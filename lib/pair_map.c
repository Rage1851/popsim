/*
 *      Filename: pair_map.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <assert.h>

#include "pair_map.h"
#include "int_pair.h"

struct byte_pair_map {
    byte_pair** map;
};

byte_pair_map* byte_pair_map_init(unsigned char:wq
        ) {
    assert(element_count > 0);

    byte_pair_map* map = (byte_pair**) malloc(sizeof(byte_pair*) * element_count);
    map->element_count = element_count;
    for(unsigned char e = 0; e < element_count; ++e)
        map[e] = (pair*) malloc(sizeof(pair) * state_count);

    return 0;
}

int set_pair_map(pair_map* map, size_t fst, size_t scd, size_t fst_map, size_t scd_map) {
    if(map == NULL)
        return -1;
    if(fst >= map.element_count || scd >= map.element_count ||
            fst_map >= map.element_count || scd_map >= map.elment_count)
        return -2;
        
    map.map[fst][scd].fst = fst_map;
    map.map[fst][scd].scd = scd_map;

    return 0;
}

int get_pair_map(pair_map* map, pair* pair, size_t fst, size_t scd) {
    if(map == NULL)
        return -1;
    if(fst >= map.element_count || scd >= map.element_count)
        return -2;

    pair = &(map.map[fst][scd]);

    return 0;
}

int destroy_pair_map(pair_map* map) {
    if(map == NULL)
        return -1;

    for(size_t element = 0; element < map.element_count; ++element)
        free(map.map[element]);
    free(map.map);
    free(map);

    return 0;
}
