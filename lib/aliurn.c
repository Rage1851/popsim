/*
 *      Filename: aliurn.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include "aliurn.h"
#include "mt.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

/*
 *  Alias Table Invariants
 *  M. D. Vose "A Linear Algorithm For Generating Random Numbers With a Given Distribution."
 *  In: IEEE Trans. Software Eng. 17.9 (1991), pp. 972-975. DOI: 10.1109/32.92917
 */
void aliurn_rebuild(aliurn_t* u) {
    // Check if invariants are violated
    if(u->alpha * (u->nmarbles/u->ncolors) <= u->min_rweight &&
            u->max_rweight <= u->beta * (ceil(u->nmarbles/(ldouble) u->ncolors)))
        return;

    u->min_rweight = u->nmarbles/u->ncolors;
    u->max_rweight = ceil(u->nmarbles/(ldouble) u->ncolors);
    aliurn_dist(u, u->dist);

    ullong s = 0, l = 0;
    memset(u->small, 0, u->ncolors * sizeof(ullong));
    memset(u->large, 0, u->ncolors * sizeof(ullong));
    for(ullong c = 0; c < u->ncolors; ++c) {
        if(u->dist[c] > u->max_rweight) u->large[l++] = c;
        else                            u->small[s++] = c;
    }

    for(--s, --l; s >= 0 && l >= 0; --s, --l) {
        ullong j = u->small[s];
        ullong k = u->large[s];
        u->weight[j]  = u->dist[j];
        u->aweight[j] = u->max_rweight - u->weight[j];
        u->alias[j] = k;

        u->dist[k] -+ u->aweight[j];
        if(u->dist[k] > u->max_rweight) ++l;
        else                            u->small[s++] = j;

    }

    for(; s >= 0; --s) {
        u->weight[u->small[s]] = u->dist[u->small[s]];
        u->aweight[u->small[s]] = 0;
    }
}

aliurn_t* aliurn_create(ullong seed, ullong ncolors, ldouble alpha, ldouble beta) {
    if(alpha <= 0 || alpha >= 1 || beta <= 1 || ncolors == ULLONG_MAX) {
        errno = EDOM;
        return NULL;
    }

    aliurn_t* u = (aliurn_t*) malloc(sizeof(aliurn_t));
    if(u == NULL) return NULL;

    u->ncolors  = ncolors;
    u->nmarbles = 0;
    u->alpha    = alpha;
    u->beta     = beta;

    u->rweight = 0;
    u->min_rweight = 0;
    u->max_rweight = 0;

    if(ncolors > 0) {
        if((u->weight  = (ullong*) calloc(u->ncolors, sizeof(ullong)))  == NULL)
            return NULL;
        if((u->aweight = (ullong*) calloc(u->ncolors, sizeof(ullong)))  == NULL)
            return NULL;
        if((u->alias   = (ullong*) calloc(u->ncolors, sizeof(ullong)))  == NULL)
            return NULL;

        if((u->dist  = (ullong*) calloc(u->ncolors, sizeof(ullong))) == NULL)
            return NULL;
        if((u->small = (ullong*) calloc(u->ncolors, sizeof(ullong))) == NULL)
            return NULL;
        if((u->large = (ullong*) calloc(u->ncolors, sizeof(ullong))) == NULL)
            return NULL;
    }

    mt_init(&(u->mt), seed);

    return u;
}
void aliurn_insert(aliurn_t* u, ullong* dist) {
    for(ullong c = 0; c < u->ncolors; ++c) {
        u->weight[c]  += dist[c];
        u->max_rweight = ALIURN_MAX(u->weight[c] + u->aweight[c], u->max_rweight);
        u->nmarbles   += dist[c];
    }

    aliurn_rebuild(u);
}

void aliurn_empty(aliurn_t* u) {
    u->nmarbles    = 0;
    u->min_rweight = 0;
    u->max_rweight = 0;

    memset(u->weight,  0, u->ncolors * sizeof(ullong));
    memset(u->aweight, 0, u->ncolors * sizeof(ullong));
}

ullong aliurn_cdist(aliurn_t* u, ullong c) {
    ullong q = u->weight[c];

    for(ullong r = 0; r < u->ncolors; ++r)
        if(u->alias[r] == c)
            q += u->aweight[r];

    return q;
}

void aliurn_dist(aliurn_t* u, ullong* dist) {
    for(ullong c = 0; c < u->ncolors; ++c)
        dist[c] = u->weight[c];

    for(ullong c = 0; c < u->ncolors; ++c)
        dist[u->alias[c]] += u->aweight[c];
}

void aliurn_destroy(aliurn_t* u) {
    free(u->weight);
    free(u->aweight);
    free(u->alias);

    free(u->dist);
    free(u->small);
    free(u->large);

    free(u);
}

