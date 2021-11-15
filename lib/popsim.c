/*
 *       Filename: popsim.c
 *         Author: Niklas Mamtschur
 *   Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <math.h>
#include <dyn_ali_tab.h>

typedef struct state_pair {
    int init;
    int resp;
} state_pair;

int sim_seq(agent_pair** map, int** dist, int state_count,
        double alpha, double beta, int steps, int snapshot_count)
{
    if(steps < 1 || snapshot_count > steps)
        return -1;
    if(state_count < 1)
        return -2;
    if(alpha < 0 || alpha >= 1 || beta <= 1)
        return -3;

    dyn_ali_tab* table = alloc_dyn_ali_tab(dist[0], state_count, alpha, beta);

    double step_width = (double) steps / snapshots;
    int snapshot = 1;
    for(int step = 0; step < step_count; ++step) {
        int state1 = sample_dyn_ali_tab(table); 
        int state2 = sample_dyn_ali_tab(table);
        update_dyn_ali_tab(table, map[state1][state2].init_state);
        update_dyn_ali_tab(table, map[state1][state2].resp_state);

        if((int) step_width == step) {
            dist[snapshot++] = dist_dyn_ali_tab(table);
            step_width += step_width;
        }
    }

    free_dyn_ali_tab(table);
    return 0;
}


int sim_batch(state_pair (*trans)(int, int), int* init_state_dist, int* final_state_dist,
                          int state_count, int step_count) {
    if(dist == NULL)     return -1;
    if(state_count < 1)  return -2;
    if(step_count  < 1)  return -3;

    linear_urn* urn_old = NULL;
    linear_urn* urn_new = NULL;
    build_from_dist_linear_urn(urn_old, dist, state_count);
    build_empty_linear_urn(urn_new, dist, state_count);

    int* state_dist = (int*) malloc(state_count * sizeof(int));
    int* q_row_dist = (int*) malloc(state_count * sizeof(int));
    int* q_col_dist = (int*) malloc(state_count * sizeof(int));
    size_t state1, state2;

    for(int step = 0, coll; step < step_count; step += coll+1) {
        coll = sample_coll(get_marble_count_linear_urn(urn_old), 0);
        get_color_dist_linear_urn(urn_old, state_dist);

        // Cannot group all together because this would require O(Q^2logn) bits of space
        sample_multi_hypergeom(state_dist, q_row_dist, agent_count, state_count, coll);
        for(size_t row_state = 0; row_state < state_count; ++row_state) {
            remove_marbles_linear_urn(urn, row_state, q_row_dist[row_state]);

            sample_multi_hypergeom(q_col_dist, agent_count, state_count, q_row_dist[state]);
            remove_list_of_marbles_linear_urn(urn_old, q_col_dist);
            memset(state_dist, 0, state_count * sizeof(int));
            for(size_t col_state = 0; col_state < state_count; ++col_state) {
                state_pair res = trans(row_state, col_state);
                ++state_dist[res.fst];
                ++state_dist[res.scd];
            }
            add_list_of_marbles_linear_urn(urn_old, state_dist);
        }

        if(coll % 2 == 0) {
            sample_linear_urn(urn_new, &state1);

            // Merging urn_new into urn_old
            get_color_dist_linear_urn(urn_new, state_dist);
            add_list_of_marbles_linear_urn(urn_old, state_dist);
            remove_all_marbles_linear_urn(urn_new);

            sample_linear_urn(urn_old, &state2);
        } else {
            sample_linear_urn(urn_old, &state1);
            sample_linear_urn(urn_new, &state2);

            // Merging urn_new into urn_old
            get_color_dist_linear_urn(urn_new, state_dist);
            add_list_of_marbles_linear_urn(urn_old, state_dist);
            remove_all_marbles_linear_urn(urn_new);
        }

        state_pair res = trans(state1, state2);
        add_marbles_linear_urn(linear_urn* urn, state1, state2);
    }

    destroy_linear_urn(urn_old);
    destroy_linear_urn(urn_new);
    free(state_dist);
    free(q_row_dist);
    free(q_col_dist);
    return 0;
}

void sim_multibatch(pair_map** map, int) {
    // TODO
}
