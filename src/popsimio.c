/*
 *       Filename: popsimio.c
 *    Description: Command line UI to popsim
 *         Author: Niklas Mamtschur
 *   Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <search.h>
#include <gmp.h>
#include <pthread.h>

int main(int argc, char* argv[]) {
    // Read command line options
    enum alg_t {ARRAY, SEQ, BST, ALIAS, BATCH, MBATCH} alg;
    enum form_t {MAP, LIST} form = LIST;
    int verbose = 0;
    unsigned long step = 0;
    unsigned long nthreads = 1;
    
    int flag;
    while((flag = getopt(argc, argv, "hvs:f:t:")) >= 0) {
        switch(flag) {
            case 'h':
                popsim_print_help(argv[0]);
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 's':
                if((step = strtoul(optarg, NULL, 10)) == 0 || errno != 0) {
                    fprintf(stderr, "Option -%c requires steps as an integer argument in "
                           "[1,%llu].\n", optopt, ULLONG_MAX);
                    return -1;
                }
                break;
            case 'f':
                if(strcmp(optarg, "map")) {
                    form = MAP;
                    break;
                } else if(strcmp(optarg, "list")) {
                    form = LIST;
                    break
                } else {
                    fprintf(stderr, "Option -%c requires form as either \"map\" or \"list\"
                            as an argument.\n", optopt);
                    return -1;
                }
            case 't':
                if((nthreads = strtoul(optarg, NULL, 10)) == 0 || errno != 0) {
                    fprintf(stderr, "Option -%c requires nthreads as an integer argument in "
                           "[1,%lu].\n", optopt, ULLONG_MAX);
                    return -1;
                }
                break;
            case '?':
                if(optopt == 's')
                    fprintf(stderr, "Option -%c requires steps as an integer argument in "
                           "[1,%lu].\n", optopt, ULLONG_MAX);
                else if(optopt == 'f')
                    fprintf(stderr, "Option -%c requires form as either \"map\" or \"list\"
                            as an argument.\n", optopt);
                else if(optopt == 't')
                    fprintf(stderr, "Option -%c requires nthreads as an integer argument in "
                           "[1,%lu].\n", optopt, ULLONG_MAX);
                else if(isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return -1;
            default:
                abort();
        }
    }

    // Read alg and check for the correct number of arguments
    if(optind+1 != argc) {
        fprintf(stderr, "Too many or too few command line arguments.\n");
        return -1;
    }

    if(strcmp(argv[optind], "array") == 0)
        alg = ARRAY;
    else if(strcmp(argv[optind], "seq") == 0)
        alg = SEQ;
    else if(strcmp(argv[optind], "bst") == 0)
        alg = BST;
    else if(strcmp(argv[optind], "alias") == 0)
        alg = ALIAS;
    else if(strcmp(argv[optind], "batch") == 0)
        alg = BATCH;
    else if(strcmp(argv[optind], "mbatch") == 0)
        alg = MBATCH;
    else {
        fprintf(stderr, "The specified algorithm must be either \"array\", \"seq\", \"bst\", "
                "\"alias\",\"batch\" or \"mbatch\".\n");
        return -1;
    }

    // Read number of steps, states, and agents
    if(verbose)
        printf("Number of steps and states:\n");

    mpz_t nsteps;
    if(mpz_inp_str(nsteps, NULL, 10) == 0 || mpz_cmp(nsteps, 0) > 0) {
        fprintf(stderr, "nsteps needs to be a positive integer or was entered invalidly\n");
        return -1;
    }

    unsigned long nstates;
    const char* state_format = (alg != ARRAY) ? " %lu" : " %lu\n"
    if(scanf(state_format, &nstates) != 1 || errno != 0 ||
            nstates == 0 || nstates == ULONG_MAX) {
        fprintf(stderr, "nstates needs to be an integer int  [1,%lu] or was entered invalidly.\n",
                        ULONG_MAX);
        return -1;
    }

    unsigned long nagents;
    if(alg != ARRAY) {
        if(scanf(" %lu\n", &nagents) != 1 || errno != 0 ||
                nagents == 0 || nagents == ULONG_MAX) {
            fprintf(stderr, "nagents needs to be an integer in [1,%lu) for the case that "
                            "alg is \"array\" or was entered invalidly.\n", ULONG_MAX);
            return -1;
        }
    }

    // Read initial configuration and initialize the urn data structure
    urn_t* urn;
    switch(alg) {
        case ARRAY:  urn = urn_init_array(nagents, nstates); break;
        case SEQ:    urn = urn_init_type(LINEAR_URN, nstates);   break;
        case BST:    urn = urn_init_type(BST_URN, nstates);      break;
        case ALIAS:  urn = urn_init_alias(0.5, 1.5, nstates);    break;
        case BATCH:  urn = urn_init_type(URN_LINEAR, nstates);   break;
        case MBATCH: urn = urn_init_type(URN_LINEAR, nstates);   break;
        default: abort();
    }
    if(urn == NULL) {
        fprintf(stderr, "Not enough memory for the urn data structure.\n");
        return -1;
    }

    mpz_t nagents_per_state;
    mpz_init(nagents_per_state);
    if(form == LIST) {

        for(unsigned long state = 0; state < nstates; ++state) {
            if(mpz_inp_str(nagents_per_state, NULL, 10) == 0 || mpz_cmp(nagents_per_state, 0) >= 0) {
                fprintf(stderr, "State distributions need to be non-negative integers or were "
                                "entered invalidly.\n");
                return -1;
            }
            if(!urn_insert(urn, state, nagents_per_state)) {
                fprintf(stderr, "The number of agents needs to be equal to nagents.\n");
                return -1;
            }
        }

        char tmp = 0;
        if(scanf("%c", &tmp) != 1 || tmp != '\n') {
            fprintf(stderr, "Initial configuration was entered invalidly.\n");
            return -1;
        }

    } else {
        
    }

    mpz_t nagents_in_urn;
    mpz_init(nagents_in_urn);
    urn_get_nmarbles(urn, nagents_in_urn);
    if(mpz_cmp(nagents, nagents_in_urn) != 0) {
        fprintf(stderr, "The sum of agents of the initial configuration needs to be equal to "
                        "nagents.\n");
        return -1;
    }
    mpz_clear(nagents_in_urn);

    // Transitions
    unsigned long ntrans;
    if(scanf("%lu\n", &ntrans) != 1 || errno != 0 || ntrans == ULONG_MAX) {
        fprintf(stderr, "Amount of transitions given needs to be a non-negative integer smaller "
                        "than %lu or was entered invalidly.\n", ULONG_MAX);
        return -1;
    }

    if(int_pair_map_create(ntrans, nagents) == 0) {
        fprintf(stderr, "Not enough memory for all the transitions.\n");
        return -1;
    }

    unsigned long init_key, resp_key, init_val, resp_val;
    for(unsigned long trans = 0; trans < ntrans; ++trans) {
        if(scanf("%lu %lu %lu %lu\n", &init_key, &resp_key, &init_val, &resp_val) != 4 ||
                errno != 0 || init_key == 0 || resp_key == 0 || init_val == 0 || resp_val == 0 ||
                init_key > nstates || resp_key > nstates ||init_val > nstates ||
                resp_val > nstates) {
            fprintf(stderr, "Transitions must be given such that s_ij are in [1,nstates] or were "
                            "entered invalidly.\n");
            return -1;
        }

        int_pair_map_insert(init_key-1, resp_key-1, init_val-1, resp_val-1);
    }

    // Algorithms
    switch(alg) {
        case ARRAY:  //TODO
        case SEQ:    //TODO
        case BST:    //TODO
        case ALIAS:  //TODO
        case BATCH:  //TODO
        case MBATCH: //TODO
        default: abort();
    }

    // Clean-Up
    urn_destroy(urn);
    int_pair_map_destroy();
    return 0;
}

void popsimio_print_help(char* prog_name) {
    printf("A program for the simulation of population protocols using the algorithms described in "
           "Berenbrink et al which prints the final configuration to stdout.\n\n"
           "Usage: %s [-h] [-v] [-s step] [-f form] [-t nthreads] alg\n\n"
           "Arguments:\n"
           "  alg         Specifies the algorithm used where alg is in "
           "              {\"array\",\"seq\",\"bst\",\"alias\",\"batch\",\"mbatch}.\n"
           "  -h          Print this usage statement and do not run the program.\n"
           "  -v          Prompt for input and print results with messages.\n"
           "  -s step     Additionally to the final configuration, also print the configuration "
           "              after every step interactions where step is an integer in [1,%lu].\n"
           "  -f form     Specifies how the configuration may be put in and how it is returned "
           "              where form must be in {\"map\",\"list\"} and \"list\" is the default.\n"
           "  -t nthreads Simulate the population protocol nthreads times on nthreads many threads "
           "              where nthreads needs to be in [1,%lu] and 1 is the default. The output "
           "              is given as a newline seperated list.\n\n"
           "The program then expects several non-negative integers from stdin, where the "
           "parameters denoted in brackets should only be given under certain conditions. Round "
           "brackets must be given for when alg is equal to \"array\", square brackets for when "
           "form is \"list\", and curly brackets for when form is \"map\":\n"
           "  nsteps    Amount of interaction steps that should be simulated where nsteps > 0.\n"
           "  nstates   Number of states where nstates <= %lu and each state is represented as an "
           "            integer in [1,nstates].\n"
           TODO alpha and beta
           "  (nagents) Number of agents, where nagents must be an integer in [1,%lu).\n"
           "  [d_i]     Initial configuration where d_i represents the initial distribution of "
           "            state i and i is in [1,nstates]. If \"alg\" is \"array\", then d_i < %lu.\n"
           "  {nconf}   Number of initial state configurations given. If a state configuration is "
           "            not given, then it is assumed to be zero and if multiple configurations are "
           "            given for the same state, then they will be summed up. If \"alg\" is "
           "            \"array\", then the sum of all configurations must be equal to nagents.\n"
           "  {s_i a_i} Initial configuration of state s_i with a_i agents, where i is an integer "
           "            in [1,nconf] and s_i is in [1,nstates].\n"
           "  ntrans    Number of transitions that are given, where ntrans is an integer in "
           "            [1,%lu]. If a transiton is not given for a pair of states, then the "
           "            identity mapping is used. If multiple transitions are given for the same "
           "            pair of states, then only the first transition is considered.\n"
           "  s_ij      Transition mapping of (s_i1,s_i2) -> (s_i3,s_i4), where s_ij must be in "
           "            [1,nstates] and i and j are integers in [1,ntrans] and {1,2,3,4} "
           "            respectively.\n\n"
           "These parameters need to be given in exactly the following format, ignoring the "
           "brackets and only condering the relevant parameters:\n"
           "nsteps nstates (nagents)"
           "[d_1 d_2 ... d_nstates]\n"
           "{nconf}\n"
           "{s_1 a_1}\n"
           "{s_2 a_2}\n"
           "{...}\n"
           "{s_nconf a_nconf}\n"
           "ntrans\n"
           "s_11 s_12 s_13 s_14\n"
           "s_21 s_22 s_23 s_24\n"
           "...\n"
           "s_ntrans1 s_ntrans2 s_ntrans3 s_ntrans4\n\n"
        prog_name, ULONG_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX);
}
