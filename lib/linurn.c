/*
 *      Filename: linurn.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include "linurn.h"
#include "mt.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>

linurn_t* linurn_create(ullong seed, ullong ncolors) {
    if(ncolors == ULLONG_MAX) {
        errno = EDOM;
        return NULL;
    }

    linurn_t* u = (linurn_t*) malloc(sizeof(linurn_t));
    if(u == NULL) return NULL;

    u->nmarbles = 0LLU;
    u->ncolors = ncolors;
    if(ncolors > 0) {
        if((u->colors = (ullong*) calloc(u->ncolors, sizeof(ullong))) == NULL)
            return NULL;
    }

    mt_init(&(u->mt), seed);

    return u;
}

linurn_t* linurn_copy(linurn_t* u, ullong seed) {
    linurn_t* ucopy = linurn_create(seed, u->ncolors);
    if(ucopy == NULL) return NULL;

    ucopy->nmarbles = u->nmarbles;
    if(u->ncolors > 0)
        memcpy(ucopy->colors, u->colors, u->ncolors * sizeof(ullong));

    return ucopy;
}

void linurn_insert(linurn_t* u, ullong* qs) {
    for(ullong c = 0; c < u->ncolors; ++c) {
        u->colors[c] += qs[c];
        u->nmarbles  += qs[c];
    }
}

void linurn_remove(linurn_t* u, ullong* qs) {
    for(ullong c = 0; c < u->ncolors; ++c) {
        u->colors[c] -= qs[c];
        u->nmarbles  -= qs[c];
    }
}

void linurn_empty(linurn_t* u) {
    u->nmarbles = 0;
    memset(u->colors, 0, u->ncolors * sizeof(ullong));
}

void linurn_destroy(linurn_t* u) {
    if(u->ncolors > 0) {
        free(u->colors);
    }

    free(u);
}
