/*
 *      Filename: aliurn.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include "aliurn.h"
#include "mt.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
    u->lbound   = 0;
    u->rbound   = 0;

    u->min_rweight = 0;
    u->max_rweight = 0;

    if(ncolors > 0) {
        if((u->weight  = (ullong*) calloc(u->ncolors, sizeof(ullong)))  == NULL)
            return NULL;
        if((u->aweight = (ullong*) calloc(u->ncolors, sizeof(ullong)))  == NULL)
            return NULL;
        // Important to zero alias, as rebuild assumes this
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

aliurn_t* aliurn_copy(aliurn_t* u, ullong seed) {
    aliurn_t* ucopy = aliurn_create(seed, u->ncolors, u->alpha, u->beta);
    if(ucopy == NULL) return NULL;

    ucopy->nmarbles = u->nmarbles;
    ucopy->lbound   = u->lbound;
    ucopy->rbound   = u->rbound;
    ucopy->min_rweight = u->min_rweight;
    ucopy->max_rweight = u->max_rweight;

    if(u->ncolors > 0) {
        memcpy(ucopy->weight,  u->weight,  u->ncolors * sizeof(ullong));
        memcpy(ucopy->aweight, u->aweight, u->ncolors * sizeof(ullong));
        memcpy(ucopy->alias,   u->alias,   u->ncolors * sizeof(ullong));

        memcpy(ucopy->dist,  u->dist,  u->ncolors * sizeof(ullong));
        memcpy(ucopy->small, u->small, u->ncolors * sizeof(ullong));
        memcpy(ucopy->large, u->large, u->ncolors * sizeof(ullong));
    }
    
    return ucopy;
}

void aliurn_insert(aliurn_t* u, ullong* qs) {
    for(ullong c = 0; c < u->ncolors; ++c) {
        u->weight[c]  += qs[c];
        u->max_rweight = ALIURN_MAX(u->weight[c] + u->aweight[c], u->max_rweight);
        u->nmarbles   += qs[c];
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
    if(u->ncolors > 0) {
        free(u->weight);
        free(u->aweight);
        free(u->alias);

        free(u->dist);
        free(u->small);
        free(u->large);
    }

    free(u);
}

