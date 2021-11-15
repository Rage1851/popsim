/*
 *      Filename: array_urn.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include "arrurn.h"
#include "mt.h"

#include <limits.h>
#include <string.h>
#include <errno.h>
#include <string.h>

arrurn_t* arrurn_create(ullong seed, ullong ncolors, ullong max_nmarbles) {
    if(ncolors == ULLONG_MAX || max_nmarbles == ULLONG_MAX) {
        errno = EDOM;
        return NULL;
    }

    arrurn_t* u = (arrurn_t*) malloc(sizeof(arrurn_t));
    if(u == NULL) return NULL;

    u->nmarbles     = 0;
    u->max_nmarbles = max_nmarbles;
    u->ncolors      = ncolors;

    mt_init(&(u->mt), seed);

    if(max_nmarbles > 0) {
        if(ncolors < UCHAR_MAX) {
            u->size = BYTE;
            if((u->bcolors  = (ubyte*)  calloc(max_nmarbles, sizeof(ubyte)))  == NULL)
                return NULL;
        } else if(ncolors < USHRT_MAX){
            u->size = SHORT;
            if((u->scolors  = (ushort*) calloc(max_nmarbles, sizeof(ushort))) == NULL)
                return NULL;
        } else if(ncolors < UINT_MAX){
            u->size = INT;
            if((u->icolors  = (uint*)   calloc(max_nmarbles, sizeof(uint)))   == NULL)
                return NULL;
        } else if(ncolors < ULONG_MAX){
            u->size = LONG;
            if((u->lcolors  = (ulong*)  calloc(max_nmarbles, sizeof(ulong)))  == NULL)
                return NULL;
        } else {
            u->size = LLONG;
            if((u->llcolors = (ullong*) calloc(max_nmarbles, sizeof(ullong))) == NULL)
                return NULL;
        }
    }

    return u;
}

arrurn_t* arrurn_copy(arrurn_t* u, ullong seed) {
    arrurn_t* ucopy = arrurn_create(seed, u->ncolors, u->max_nmarbles);
    if(ucopy == NULL) return NULL;

    ucopy->nmarbles = u->nmarbles;

    if(u->ncolors > 0) {
        switch(ucopy->size) {
            case BYTE: 
                memcpy(ucopy->bcolors,  u->bcolors,  u->max_nmarbles * sizeof(ubyte));
                break;
            case SHORT:
                memcpy(ucopy->scolors,  u->scolors,  u->max_nmarbles * sizeof(ushort));
                break;
            case INT:
                memcpy(ucopy->icolors,  u->icolors,  u->max_nmarbles * sizeof(uint));
                break;
            case LONG:
                memcpy(ucopy->lcolors,  u->lcolors,  u->max_nmarbles * sizeof(ulong));
                break;
            case LLONG:
                memcpy(ucopy->llcolors, u->llcolors, u->max_nmarbles * sizeof(ullong));
                break;
            default:
                abort();
        }
    }

    return ucopy;
}

void arrurn_insert(arrurn_t* u, ullong* qs) {
    switch(u->size) {
        case BYTE:
            for(ullong c = 0; c < u->ncolors; ++c)
                while(qs[c]--) u->bcolors[u->nmarbles++] = c;
            break;
        case SHORT:
            for(ullong c = 0; c < u->ncolors; ++c)
                while(qs[c]--) u->scolors[u->nmarbles++] = c;
            break;
        case INT:
            for(ullong c = 0; c < u->ncolors; ++c)
                while(qs[c]--) u->icolors[u->nmarbles++] = c;
            break;
        case LONG:
            for(ullong c = 0; c < u->ncolors; ++c)
                while(qs[c]--) u->lcolors[u->nmarbles++] = c;
            break;
        case LLONG:
            for(ullong c = 0; c < u->ncolors; ++c)
                while(qs[c]--) u->llcolors[u->nmarbles++] = c;
            break;
        default:
            abort();
    }
}

void arrurn_empty(arrurn_t* u) {
    u->nmarbles = 0;
}

ullong arrurn_cdist(arrurn_t* u, ullong c) {
    ullong q = 0;
    ullong nmarbles = u->nmarbles;
    switch(u->size) {
        case BYTE:
            while(nmarbles--)
                if(u->bcolors[nmarbles] == c)
                    ++q;
            break;
        case SHORT:
            while(nmarbles--)
                if(u->scolors[nmarbles] == c)
                    ++q;
            break;
        case INT:
            while(nmarbles--)
                if(u->icolors[nmarbles] == c)
                    ++q;
            break;
        case LONG:
            while(nmarbles--)
                if(u->lcolors[nmarbles] == c)
                    ++q;
            break;
        case LLONG:
            while(nmarbles--)
                if(u->llcolors[nmarbles] == c)
                    ++q;
            break;
        default:
            abort();
    }
    
    return q;
}

void arrurn_dist(arrurn_t* u, ullong* dist) {
    ullong nmarbles = u->nmarbles;
    switch(u->size) {
        case BYTE:
            while(nmarbles--) ++(dist[u->bcolors [nmarbles]]);
            break;
        case SHORT:
            while(nmarbles--) ++(dist[u->scolors [nmarbles]]);
            break;
        case INT:
            while(nmarbles--) ++(dist[u->icolors [nmarbles]]);
            break;
        case LONG:
            while(nmarbles--) ++(dist[u->lcolors [nmarbles]]);
            break;
        case LLONG:
            while(nmarbles--) ++(dist[u->llcolors[nmarbles]]);
            break;
        default:
            abort();
    }
}

void arrurn_destroy(arrurn_t* u) {
    if(u->max_nmarbles > 0) {
        switch(u->size) {
            case BYTE:  free(u->bcolors);  break;
            case SHORT: free(u->scolors);  break;
            case INT:   free(u->icolors);  break;
            case LONG:  free(u->lcolors);  break;
            case LLONG: free(u->llcolors); break;
            default: abort();
        }
    }

    free(u);
}
