/*
 *      Filename: aliurn.h
 *   Description: Urn data structure implemented via a dynamic alias table introduced by Berenbrink 
 *                et al.
 *   Assumptions: The urn data structure needs to be created before and destroyed after use and 
 *                colors are represented as numbers in [0, ncolors) and the total number of marbles
 *                must be smaller than ULLONG_MAX.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef ALIAS_URN_H
#define ALIAS_URN_H

#include <string.h>
#include <math.h>
#include "mt.h"

typedef unsigned long long ullong;
typedef long double        ldouble;

#define ALIURN_MIN(x,y) ((x)<=(y) ? (x) : (y))
#define ALIURN_MAX(x,y) ((x)>=(y) ? (x) : (y))

typedef struct aliurn_t {
    ullong ncolors;
    ullong nmarbles;
    ldouble alpha, beta;
    ullong  lbound, rbound;

    ullong* weight;
    ullong* aweight;
    ullong* alias;
    ullong  min_rweight, max_rweight;

    ullong* dist;
    ullong* small;
    ullong* large;

    mt_t mt;
} aliurn_t;

void aliurn_dist(aliurn_t* urn, ullong* qs);
/*
 *  Description: Rebuilds the internal alias table structure to conform to the invariants if
 *               they were violated.
 *       Source: M. D. Vose "A Linear Algorithm For Generating Random Numbers With a Given
 *               Distribution." In: IEEE Trans. Software Eng. 17.9 (1991), pp. 972-975.
 *               DOI: 10.1109/32.92917
 */
static inline void aliurn_rebuild(aliurn_t* u) {
    // Check if invariants are violated
    if(u->lbound <= u->min_rweight && u->max_rweight <= u->rbound)
        return;

    // Define bounds
    u->min_rweight = u->nmarbles/u->ncolors;
    u->max_rweight = ceil(u->nmarbles/(ldouble) u->ncolors);
    u->lbound      = ceil(u->alpha*u->min_rweight);
    u->rbound      = u->beta*u->min_rweight;

    // Split color distribution into small and large, where small fit into a min_rweight and
    // large do not
    ullong s = 0, l = 0;
    aliurn_dist(u, u->dist);
    for(ullong c = 0; c < u->ncolors; ++c) {
        if(u->dist[c] > u->min_rweight) u->large[l++] = c;
        else                            u->small[s++] = c;
    }

    // Fill table
    ullong nmax = u->nmarbles - u->ncolors*u->min_rweight;
    while(l > 0) {
        ullong sn = u->small[--s];
        ullong ln = u->large[--l];

        u->weight [sn] = u->dist[sn];
        u->aweight[sn] = u->min_rweight - u->weight[sn];
        u->alias  [sn] = ln;
        if(nmax > 0) {
            u->aweight[sn]++;
            nmax--;
        }

        //printf("aweight: %llu\n", u->aweight[sn]);
        u->dist[ln] -= u->aweight[sn];
        if(u->dist[ln] > u->min_rweight) ++l;
        else                             u->small[s++] = ln;
    }

    while(s > 0) {
        s--;
        u->weight [u->small[s]] = u->dist[u->small[s]];
        u->aweight[u->small[s]] = 0;
    }
}

/*
 *   Description: Intializes the urn and allocates all pointers.
 *  Return value: The newly allocated urn or NULL on error.
 *        Errors: ENOMEM if not enough memory was available and EDOM if ncolors = ULLONG_MAX or
 *                alpha <= 0 or alpha >=1 or beta <= 1.
 */
aliurn_t* aliurn_create(ullong seed, ullong ncolors, ldouble alpha, ldouble beta);

/*
 *   Description: Produces an exact, independent copy of u which needs to be destroyed on its own.
 *  Return value: The copied and independently allocated urn or NULL on error.
 *        Errors: ENOMEM if not enough memory was available.
 */
aliurn_t* aliurn_copy(aliurn_t* u, ullong seed);

/*
 *   Description: Sample with or without replacement as long as there is a marble in the urn.
 *  Return value: The sampled marble or ULLONG_MAX if the urn was empty.
 */
static inline ullong aliurn_sample(aliurn_t* u) {
    if(u->nmarbles == 0) return ULLONG_MAX;

    // Rejection Sampling
    ullong c, w;
    do {
        c = mt_urand(&(u->mt), u->ncolors);
        w = mt_urand(&(u->mt), u->max_rweight);
    } while(w >= u->weight[c] + u->aweight[c]);

    // Alias Sampling
    return (w < u->weight[c]) ? c : u->alias[c];
}

static inline ullong aliurn_draw(aliurn_t* u) {
    if(u->nmarbles == 0) return ULLONG_MAX;

    // Rejection sampling
    ullong c, w;
    do {
        c = mt_urand(&(u->mt), u->ncolors);
        w = mt_urand(&(u->mt), u->max_rweight);
    } while(w >= u->weight[c] + u->aweight[c]);

    u->min_rweight = ALIURN_MIN(u->weight[c] + u->aweight[c], u->min_rweight);

    // Alias sampling
    if(w < u->weight[c]) {
        u->weight[c]--;
    } else {
        u->aweight[c]--;
        c = u->alias[c];
    }

    u->nmarbles--;
    aliurn_rebuild(u);

    return c;
}

/*
 *   Description: Insert q marbles of color c into the urn.
 *  Assumtptions: There is enough space in the urn and c < ncolors;
 */
static inline void aliurn_cinsert(aliurn_t* u, ullong c, ullong q) {
    u->weight[c]  += q;
    u->max_rweight = ALIURN_MAX(u->weight[c] + u->aweight[c], u->max_rweight);
    u->nmarbles   += q;

    aliurn_rebuild(u);
}

/*
 *  Description: Inserts marbles of all colors into the urn.
 *  Assumptions: qs holds the color distribution where the index of each element corresponds to
 *               the color with the same value and there has to be enough space in the urn.
 */
void aliurn_insert(aliurn_t* u, ullong* qs);

/*
 *  Description: Removes all marbles, leaving an empty urn.
 */
void aliurn_empty(aliurn_t* urn);

/*
 *  Description: Returns the number of marbles with color c.
 *  Assumptions: c < ncolors.
 */
ullong aliurn_cdist(aliurn_t* u, ullong c);

/*
 *  Description: Calculates the color distribution of all marbles;
 *  Assumptions: qs holds the color distribution where the index of each element corresponds to
 *               the color with the same value.
 */
void aliurn_dist(aliurn_t* urn, ullong* qs);

/*
 *  Description: Return the number of marbles in the urn.
 */
static inline ullong aliurn_nmarbles(aliurn_t* u) {
    return u->nmarbles;
}

/*
 *  Description: Frees the urn structure and all other pointers allocated by the create function.
 */
void aliurn_destroy(aliurn_t* u);

#endif
