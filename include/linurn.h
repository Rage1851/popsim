/*
 *      Filename: linurn.h
 *   Description: Urn data structure which keeps the color distribution in a linear data structure.
 *   Assumptions: The urn needs to be created before and destroyed after use and colors are
 *                represented as integers in [0,ncolors). The total number of marbles in the urn
 *                needs to be smaller than ULLONG_MAX.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#ifndef LINEAR_URN_H
#define LINEAR_URN_H

#include <limits.h>
#include "mt.h"

typedef unsigned long long ullong;

// Should be treated as opaque.
typedef struct linurn_t {
    ullong* colors;
    ullong  nmarbles;
    ullong  ncolors;

    mt_t mt;
} linurn_t;

/*
 *   Description: Initialize and allocate the urn, where ncolors < ULLONG_MAX.
 *  Return value: Pointer to the initialized urn or NULL if an error occurred.
 *        Errors: ENOMEM if there was not enough memory for the urn and EDOM if
 *                ncolors == ULLONG_MAX.
 */
linurn_t* linurn_create(ullong seed, ullong ncolors);

/*
 *   Description: Create and allocate an exact copy of urn which needs to be destroyed
 *                independently.
 *  Return value: Pointer to the copied and allocated urn or NULL if an error occurred.
 *        Errors: ENOMEM if there was not enough memory for the urn.
 */
linurn_t* linurn_copy(linurn_t* u, ullong seed);

/*
 *   Description: Sampling with or without replacement as long as there are still marbles in the
 *                urn.
 *  Return value: The sampled color or ULONG_MAX if the urn was empty.
 */
static inline ullong linurn_sample(linurn_t* u) {
    if(u->nmarbles > 0) {
        ullong x = mt_urand(&(u->mt), u->nmarbles);
        for(ullong c = 0; c < u->ncolors; ++c) {
            if(x < u->colors[c])
                return c;
            x -= u->colors[c];
        }
    }

    return ULLONG_MAX;
}

static inline ullong linurn_draw(linurn_t* u) {
    if(u->nmarbles > 0) {
        ullong x = mt_urand(&(u->mt), u->nmarbles);
        for(ullong c = 0; c < u->ncolors; ++c) {
            if(x < u->colors[c]) {
                u->colors[c]--;
                u->nmarbles--;
                return c;
            }
            x -= u->colors[c];
        }
    }

    return ULLONG_MAX;
}
/*
 *   Description: Inserts marbles of color c into the urn.
 *   Assumptions: c < ncolors and all marbles have to fit in the urn.
 */
static inline void linurn_cinsert(linurn_t* u, ullong c, ullong q) {
    u->colors[c] += q;
    u->nmarbles  += q;
}

/*
 *   Description: Removes marbles of color c from the urn.
 *   Assumptions: c < ncolors and there have to be enough marbles to be removed.
 */
static inline void linurn_cremove(linurn_t* u, ullong c, ullong q) {
    u->colors[c] -= q;
    u->nmarbles  -= q;
}
/*
 *   Description: Inserts marbles of every color into the urn.
 *   Assumptions: qs holds the color distribution where the index of each element corresponds to
 *                the color with the same value and there has to be enough space in the urn.
 */
void linurn_insert(linurn_t* u, ullong* qs);

/*
 *   Description: Removes marbles of color c from the urn.
 *   Assumptions: qs holds the color distribution where the index of each element corresponds to
 *                the color with the same value and there have to be enough marbles to be removed.
 */
void linurn_remove(linurn_t* u, ullong* qs);

/*
 *  Description: Removes all marbles, leaving an empty urn.
 */
void linurn_empty(linurn_t* u);

/*
 *    Description: Getter functions for the color distribution of a single color.
 *   Return value: The number of marbles of color c.
 *    Assumptions: c < ncolors.
 */
static inline ullong linurn_cdist(linurn_t* u, ullong c) {
    return u->colors[c];
}

/*
 *    Description: Getter functions for the underlying data structure.
 *   Return value: Pointer to the underlying data structure.
 */
static inline ullong* linurn_dist(linurn_t* u) {
    return u->colors;
}

/*
 *   Description: Getter function for the number of marbles currently in the urn.
 *  Return value: The number of marbles currently in the urn.
 */
static inline ullong linurn_nmarbles(linurn_t* u) {
    return u->nmarbles;
}

/*
 *  Description: Frees the urn structure and all other pointers allocated by the create function.
 */
void linurn_destroy(linurn_t* u);

#endif
