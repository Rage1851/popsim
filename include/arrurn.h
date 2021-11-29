/*
 *      Filename: arrurn.h
 *   Description: Urn data structure where the color of each marble is stored as an element of a
 *                fixed size array; the array is of type unsigned char, unsigned short, unsigned
 *                int, unsigned long or unsigned long long depending on the amount of colors.
 *   Assumptions: The urn needs to be created before and destroyed after use and colors are
 *                represented as integers in [0,ncolors).
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef ARRURN_H
#define ARRURN_H

#include <stdlib.h>
#include "mt.h"

typedef unsigned char      ubyte;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long ullong;

typedef enum arrurn_size_t {
    ARRURN_BYTE,
    ARRURN_SHORT,
    ARRURN_INT,
    ARRURN_LONG,
    ARRURN_LLONG,
} arrurn_size_t;

// Should be treated as opaque.
typedef struct arrurn_t {
    ullong nmarbles;
    ullong max_nmarbles;
    ullong ncolors;

    mt_t mt;

    arrurn_size_t size;
    ubyte*  bcolors;
    ushort* scolors;
    uint*   icolors;
    ulong*  lcolors;
    ullong* llcolors;
} arrurn_t;

/*
 *   Description: Initialize and allocate a new urn, where max(ncolors, max_nmarbles) < ULLONG_MAX.
 *  Return value: Pointer to the initialized urn or NULL if there was an error.
 *        Errors: ENOMEM if there was not enough memory and EDOM if
 *                max(ncolors, max_nmarbles) == ULLONG_MAX.
 */
arrurn_t* arrurn_create(ullong seed, ullong ncolors, ullong max_nmarbles);

/*
 *   Description: Create another urn which is its own entity but is initialized with the contents
 *                of another urn.
 *  Return value: Pointer to the newly allocated copy of urn or NULL if an error occurred.
 *        Errors: ENOMEM if there was not enough memory for the copy.
 */
arrurn_t* arrurn_copy(arrurn_t* u, ullong seed);

/*
 *    Description: Sample a marble with or without replacement as long as there is a marble in
 *                 the urn.
 *   Return value: The color of the sampled marble or ULLONG_MAX to indicate an empty urn.
 */
static inline ullong arrurn_sample(arrurn_t* u) {
    if(u->nmarbles == 0) return ULONG_MAX;

    switch(u->size) {
        case ARRURN_BYTE:  return u->bcolors [mt_urand(&(u->mt), u->nmarbles)];
        case ARRURN_SHORT: return u->scolors [mt_urand(&(u->mt), u->nmarbles)];
        case ARRURN_INT:   return u->icolors [mt_urand(&(u->mt), u->nmarbles)];
        case ARRURN_LONG:  return u->lcolors [mt_urand(&(u->mt), u->nmarbles)];
        case ARRURN_LLONG: return u->llcolors[mt_urand(&(u->mt), u->nmarbles)];
        default: abort();
    }
}

static inline ullong arrurn_draw(arrurn_t* u) {
    if(u->nmarbles == 0) return ULONG_MAX;

    ullong m = mt_urand(&(u->mt), u->nmarbles);
    ullong c;

    switch(u->size) {
        case ARRURN_BYTE:
            c = u->bcolors[m];
            u->bcolors[m]  = u->bcolors [--(u->nmarbles)];
            break;
        case ARRURN_SHORT:
            c = u->scolors[m];
            u->scolors[m]  = u->scolors [--(u->nmarbles)];
            break;
        case ARRURN_INT:
            c = u->icolors[m];
            u->icolors[m]  = u->icolors [--(u->nmarbles)];
            break;
        case ARRURN_LONG:
            c = u->lcolors[m];
            u->lcolors[m]  = u->lcolors [--(u->nmarbles)];
            break;
        case ARRURN_LLONG:
            c = u->llcolors[m];
            u->llcolors[m] = u->llcolors[--(u->nmarbles)];
            break;
        default: abort();
    }

    return c;
}

/*
 *    Description: Inserts new marbles of color color into the urn as long as there is space for
 *                 all of them.
 *    Assumptions: There has to be enough space in the urn for all marbles c < ncolors.
 */
static inline void arrurn_cinsert(arrurn_t* u, ullong c, ullong q) {
    switch(u->size) {
        case ARRURN_BYTE:
            while(q--) u->bcolors [u->nmarbles++] = c;
            break;
        case ARRURN_SHORT:
            while(q--) u->scolors [u->nmarbles++] = c;
            break;
        case ARRURN_INT:
            while(q--) u->icolors [u->nmarbles++] = c;
            break;
        case ARRURN_LONG:
            while(q--) u->lcolors [u->nmarbles++] = c;
            break;
        case ARRURN_LLONG:
            while(q--) u->llcolors[u->nmarbles++] = c;
            break;
        default:
            abort();
    }
}

/*
 *   Description: Inserts new marbles of all colors into the urn as long as there is space for
 *                all of them.
 *   Assumptions: qs holds the color distribution where the index of each element corresponds to
 *                the color with the same value and there has to be enough space for all marbles.
 */
void arrurn_insert(arrurn_t* u, ullong* qs);

/*
 *  Description: Removes all marbles from the urn, leaving an empty urn.
 */
void arrurn_empty(arrurn_t* u);

/*
 *   Description: Getter function for the color distribution of a single color.
 *   Assumptions: c < ncolors.
 */
ullong arrurn_cdist(arrurn_t* u, ullong c);

/*
 *   Description: Getter functions for the color distributions of all colors.
 *   Assumptions: The array qs must be allocated as well as zeroed already and hold atleast ncolors
 *                members where the index of each member corresponds to the color with the same
 *                value.
 */
void arrurn_dist(arrurn_t* u, ullong* qs);

/*
 *   Description: Getter function for the number of marbles currently in the urn.
 *  Return value: The number of marbles currently in the urn.
 */
static inline ullong arrurn_nmarbles(arrurn_t* u) {
    return u->nmarbles;
}

/* 
 *  Description: Frees the urn structure and all other pointers allocated by the init function.
 */
void arrurn_destroy(arrurn_t* u);

#endif
