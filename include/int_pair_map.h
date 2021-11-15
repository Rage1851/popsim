/*
 *      Filename: ipmap.h
 *   Description: Hash map for integer pairs implemented with hsearch.
 *   Assumptions: Since the implementation uses hsearch and there can only be one of these hash
 *                tables at a time, this map cannot be used simultaneously with another hash table.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef INT_PAIR_MAP_H
#define INT_PAIR_MAP_H

#include <stddef.h>

typedef struct

#define EINT_PAIR_MAP_NOT_INIT -1
#define EINT_PAIR_MAP_FULL     -2

/*
 *   Description:
 *              - Initializes and allocates the map with a maximum of max_entry_count mappings and
 *                with max_entry being the largest integer that the elements of the key and value
 *                pairs can be.
 *              - If the map was already initialized, then the function calls int_pair_map_destroy
 *                before continuing with normal functionality.
 *  Return value: Non-zero if the map could be initialized and zero otherwise
 *        Errors: ENOMEM if there was not enough memory for the hash table and the book keeping
 *                structures
 */
int int_pair_map_create(size_t nel, unsigned long max_entry);

/*
 *   Description: Inserts the (key_fst,key_scd)->(value_fst,value_scd) mapping into the map as long
 *                as there is no key (key_fst,key_scd) and as long as the map is not full
 *  Return value: Non-zero if the mapping could be inserted and zero otherwise
 *        Errors:
 *              - EINT_PAIR_MAP_FULL if the map already contains nel elements,
 *              - EINT_PAIR_MAP_NOT_INIT if the map was not initialized before
 *              - EDOM if key_fst, key_scd, value_fst or value_scd are larger than max_entry
 *              - ENOMEM if there was not enough memory for the insert to be performed
 */
int int_pair_map_insert(unsigned long key_fst, unsigned long key_scd,
                        unsigned long val_fst, unsigned long val_scd);

/*
 *   Description: Fills *value_fst and *value_scd with the mapping of the key (key_fst,key_scd) as
 *                long as this mapping is in the map
 *  Return value: Non-zero if the key was found and zero otherwise
 *        Errors:
 *              - EINT_PAIR_MAP_NOT_INIT if the map was not initialized before
 *              - EDOM if key_fst or key_scd are larger than max_entry
 *              - ENOMEM if there was not enough memory for the lookup to be performed
 */
int int_pair_map_lookup(unsigned long  key_fst, unsigned long  key_scd,
                        unsigned long* val_fst, unsigned long* val_scd);

/*
 *   Description: Calls hdestroy() and frees all book keeping structures if the map was initialized
 *                before, otherwise the function performs nop
 */
void int_pair_map_destroy();

#endif
