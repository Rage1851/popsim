/*
 *      Filename: aliurn.h
 *   Description: Urn data structure implemented via a dynamic alias table introduced by Berenbrink 
 *                et al.
 *   Assumptions: The urn data structure needs to be created before and destroyed after use and 
 *                colors are represented as numbers in [0, ncolors).
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef ALIAS_URN_H
#define ALIAS_URN_H

#include "mt.h"

typedef unsigned long long ullong;
typedef long double ldouble;

#define ALIURN_MIN(x,y) ((x)<=(y) ? (x) : (y))
#define ALIURN_MAX(x,y) ((x)>=(y) ? (x) : (y))

typedef struct aliurn_t {
    unsigned long ncolors;
    ullong nmarbles;
    ldouble alpha, beta;

    ullong* weight;
    ullong* aweight;
    ullong* alias;
    ullong rweight, min_rweight, max_rweight;

    ullong* dist;
    ullong* small;
    ullong* large;

    mt_t mt;
} aliurn_t;

/*
 *  Description: Rebuilds the internal alias table structure to conform to the invariants if
 *               they were violated.
 */
void aliurn_rebuild(aliurn_t* u);

/*
 *   Description: Intializes the urn and allocates all pointers.
 */
aliurn_t* aliurn_create(ullong seed, ullong ncolors, ldouble alpha, ldouble beta);

/*
 *   Description: Sample with or without replacement as long as there is a marble in the urn.
 */
static inline ullong aliurn_sample(aliurn_t* u) {
    ullong c, w;
    do {
        c = mt_urand(&(u->mt), u->ncolors);
        w = mt_urand(&(u->mt), u->max_rweight);
    } while(w > u->weight[c] + u->aweight[c]);

    return (w < u->weight[c]) ? c : u->alias[c];
}

static inline ullong aliurn_draw(aliurn_t* u) {
    // Rejection sampling
    ullong c, w;
    do {
        c = mt_urand(&(u->mt), u->ncolors);
        w = mt_urand(&(u->mt), u->max_rweight);
    } while(w > u->weight[c] + u->aweight[c]);

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
 */
void aliurn_insert(aliurn_t* u, ullong* dist);

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
 */
void aliurn_dist(aliurn_t* urn, ullong* dist);

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
