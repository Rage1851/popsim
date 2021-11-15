/*
 *      Filename: bsturn.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include "bsturn.h"

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <errno.h>

bsturn_t* bsturn_create(ullong seed, ullong ncolors) {
    if(ncolors == ULLONG_MAX) {
        errno = EDOM;
        return NULL;
    }

    bsturn_t* u = (bsturn_t*) malloc(sizeof(bsturn_t)); 
    if(u == NULL) return NULL;

    u->ncolors = ncolors;
    u->height  = ceil(log(ncolors)/log(2.));
    // Actually, nnodes+1
    u->nnodes  = 2 << u->height;
    u->cstart  = DECLVL(u->nnodes);

    u->nmarbles = 0;
    if((u->bst = (ullong*) calloc(u->nnodes, sizeof(ullong))) == NULL)
        return NULL;

    return u;
}

bsturn_t* bsturn_copy(bsturn_t* u, ullong seed) {
    bsturn_t* ucopy = bsturn_create(seed, u->ncolors);
    if(ucopy == NULL) return NULL;
    
    ucopy->nmarbles = u->nmarbles;
    memcpy(ucopy->bst, u->bst, u->nnodes * sizeof(ullong));

    return ucopy;
}

static inline void iupdate(bsturn_t* u) {
    for(ullong lvl = u->cstart; DECLVL(lvl) >= ROOT; lvl = DECLVL(lvl)) {
        for(ullong node = DECLVL(lvl); node < lvl; ++node) {
            ullong child = LCHILD(node);
            u->bst[node] = u->bst[child];

            for(child = RCHILD(child); child < u->nnodes; child = RCHILD(child))
                u->bst[node] += u->bst[child];
        }
    }
}

void bsturn_insert(bsturn_t* u, ullong* qs) {
    for(ullong c = 0; c < u->ncolors; ++c) {
        u->bst[u->cstart+c] += qs[c];
        u->nmarbles         += qs[c];
    }
    iupdate(u);
}

void bsturn_remove(bsturn_t* u, ullong* qs) {
    for(ullong c = 0; c < u->ncolors; ++c) {
        u->bst[u->cstart+c] -= qs[c];
        u->nmarbles         -= qs[c];
    }
    iupdate(u);
}

void bsturn_destroy(bsturn_t* u) {
    free(u->bst);
    free(u);
}
