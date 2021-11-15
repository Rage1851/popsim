/*
 *      Filename: int_pair_map.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */
#include "int_pair_map.h"

#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

typedef struct _ipair_t {
    unsigned char fst;
    unsigned char scd;
} _long_pair_t;

enum INT_MAP_type_t {BYTE_MAP, SHORT_MAP, INT_MAP, LONG_MAP};

typedef struct INT_MAP_t {
    size_t nel;
    size_t el;
    unsigned long max_entry;

    char** keys;
    int    max_key_size;

    int         size;
    ENTRY       lookup;
    _byte_pair_t*  byte_data;
    _short_pair_t* short_data;
    _int_pair_t*   int_data;
    _long_pair_t*  long_data;
} INT_MAP_t;

static INT_MAP_t _map;
static int _init_flag = 0;

// Conversion of int_pair to the hash table key can be optimized
void INT_MAP_pair_to_key(unsigned long fst, unsigned long scd, char* key) {
    int print_length  = sprintf(key, "%lu", fst);
    key[print_length] = ' ';
    print_length      = sprintf(key+print_length+1, "%lu", scd);
    key[print_length] = '\0';
} 

int int_pair_map_init(size_t nel, unsigned long max_entry) {
    if(_init_flag)
        int_pair_map_destroy();

    _map.nel = nel;
    _map.el  = 0;
    _map.max_entry = max_entry;
    if(hcreate(_map.nel) == 0)
        return 0;

    _map.max_key_size = 2+2*((int) ceil(log(_map.max_entry)/log(10)));
    if((_map.keys = (char**) malloc(sizeof(char*) * _map.nel)) == NULL)
        return 0;

    for(size_t entry = 0; entry < _map.nel; ++entry) {
        _map.keys[entry] = (char*) malloc(sizeof(char) * _map.max_key_size);
        _map.keys[entry][_map.max_key_size-1] = '\0';
    }

    _map.lookup.key = (char*) malloc(sizeof(char) * _map.max_key_size);
    if(max_entry <= UCHAR_MAX) {
        _map.size = BYTE_MAP;
        if((_map.byte_data = (_byte_pair_t*) malloc(sizeof(_byte_pair_t) * _map.nel))
                == NULL)
            return 0;
    } else if(max_entry <= USHRT_MAX) {
        _map.size = SHORT_MAP;
        if((_map.short_data = (_short_pair_t*) malloc(sizeof(_short_pair_t) * _map.nel))
                == NULL)
            return 0;
    } else if(max_entry <= UINT_MAX) {
        _map.size = INT_MAP;
        if((_map.int_data = (_int_pair_t*) malloc(sizeof(_int_pair_t) * _map.nel))
                == NULL)
            return 0;
    } else {
        _map.size = LONG_MAP;
        if((_map.long_data = (_long_pair_t*) malloc(sizeof(_long_pair_t) * _map.nel))
                == NULL)
            return 0;
    }

    _init_flag = 1;
    return 1;
}

int int_pair_map_insert(unsigned long key_fst, unsigned long key_scd,
                        unsigned long value_fst, unsigned long value_scd) {
    if(!_init_flag) {
        errno = EINT_PAIR_MAP_NOT_INIT;
        return 0;
    } else if(_map.el >= _map.nel) {
        errno = EINT_PAIR_MAP_FULL;
        return 0;
    } else if(key_fst > _map.max_entry || key_scd > _map.max_entry || 
              value_fst > _map.max_entry || value_scd > _map.max_entry) {
        errno = EDOM;
        return 0;
    }

    // Check if there already exists an entry for this key
    INT_MAP_pair_to_key(key_fst, key_scd, _map.keys[_map.el]);
    ENTRY entry;
    entry.key = _map.keys[_map.el];
    if(hsearch(entry, FIND) == NULL)
        return 0;

    // Fill entry data with the value and insert into the hash table
    switch(_map.size) {
        case BYTE_MAP:
            _map.byte_data[_map.el].fst = value_fst;
            _map.byte_data[_map.el].scd = value_scd;
            entry.data = (void*) (_map.byte_data + _map.el);
            break;
        case SHORT_MAP:
            _map.short_data[_map.el].fst = value_fst;
            _map.short_data[_map.el].scd = value_scd;
            entry.data = (void*) (_map.short_data + _map.el);
            break;
        case INT_MAP:
            _map.int_data[_map.el].fst = value_fst;
            _map.int_data[_map.el].scd = value_scd;
            entry.data = (void*) (_map.int_data + _map.el);
            break;
        case LONG_MAP:
            _map.long_data[_map.el].fst = value_fst;
            _map.long_data[_map.el].scd = value_scd;
            entry.data = (void*) (_map.long_data + _map.el);
            break;
        default:
            abort();
    }
    if(hsearch(entry, ENTER) == NULL)
        return 0;

    _map.el++;
    return 1;
}

int int_pair_map_lookup(unsigned long key_fst, unsigned long key_scd,
                        unsigned long* value_fst, unsigned long* value_scd) {
    if(!_init_flag) {
        errno = EINT_PAIR_MAP_NOT_INIT;
        return 0;
    } else if(key_fst > _map.max_entry || key_scd > _map.max_entry) {
        errno = EDOM;
        return 0;
    }

    // Look for key and check if it was found
    INT_MAP_pair_to_key(key_fst, key_scd, _map.lookup.key);
    ENTRY* entry = hsearch(_map.lookup, FIND);
    if(entry == NULL)
        return 0;

    // Extract value from the entry and fill return parameters
    switch(_map.size) {
        case BYTE_MAP:
            _byte_pair_t* byte_pair = (_byte_pair_t*) entry->data;
            *value_fst = byte_pair->fst;
            *value_scd = byte_pair->scd;
        case SHORT_MAP:
            _short_pair_t* short_pair = (_short_pair_t*) entry->data;
            *value_fst = short_pair->fst;
            *value_scd = short_pair->scd;
        case INT_MAP:
            _int_pair_t* int_pair = (_int_pair_t*) entry->data;
            *value_fst = int_pair->fst;
            *value_scd = int_pair->scd;
        case LONG_MAP:
            _long_pair_t* long_pair = (_long_pair_t*) entry->data;
            *value_fst = byte_pair->fst;
            *value_scd = byte_pair->scd;
        default:
            abort();
    }

    return 1;
}


void int_pair_map_destroy() {
    if(!_init_flag)
        return;

    hdestroy();

    for(size_t entry = 0; entry < _map.nel; ++entry)
        free(_map.keys[entry]);

    free(_map.lookup.key);
    free(_map.keys);
    
    free(_map.lookup.key);
    switch(_map.size) {
        case BYTE_MAP:  free(_map.byte_data);  break;
        case SHORT_MAP: free(_map.short_data); break;
        case INT_MAP:   free(_map.int_data);   break;
        case LONG_MAP:  free(_map.long_data);  break;
        default: abort();
    }

    _init_flag = 0;
}
