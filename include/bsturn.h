/*
 *      Filename: bsturn.h
 *   Description: Urn data structure where the color distibrutions are the leaves of a binary search
 *                tree and each internal node is the sum of all marbles in the left sub-tree.
 *   Assumptions: The urn needs to be created before and destroyed after use and colors are
 *                represented as integers in [0,ncolors).
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef BSTURN_H
#define BSTURN_H

#include <string.h>
#include "mt.h"

typedef unsigned long long ullong;

// Tree operation macros
// Nodes start at idx 1 so that computations are easier
#define ROOT            1
#define INCLVL(lvl)     ((lvl) << 1)
#define DECLVL(lvl)     ((lvl) >> 1)
#define LCHILD(node)    ((node) << 1)
#define RCHILD(node)    (((node) << 1)+1)
#define ISLCHILD(node)  ((node) != ROOT ? (node)%2 == 0 : 0)
#define IS_RCHILD(node) ((node) != ROOT ? (node)%2 != 0 : 0)
#define PARENT(node)    (ISLCHILD(node) ? (node) >> 1 : ((node)-1) >> 1)

typedef struct bsturn_t {
    ullong* bst;
    ullong  nmarbles;

    ullong ncolors;
    ullong height;
    ullong nnodes;
    ullong cstart;

    mt_t mt;
} bsturn_t;

/*
 *   Description: Initialize and allocate the urn.
 *  Return value: Pointer to the initialized urn or NULL if an error occurred.
 *        Errors: ENOMEN if there was not enough memory for the urn.
 */
bsturn_t* bsturn_create(ullong seed, ullong ncolors);

/*
 *   Description: Create an exact copy of urn that needs to be destroyed independently.
 *  Return value: Pointer to the allocated and copied urn or NULL if an error occurred.
 *        Errors: ENOMEN if there was not enough memory for the urn.
 */
bsturn_t* bsturn_copy(bsturn_t*, ullong seed);

/*
 *   Description: Sampling with or without replacement as long as there are still marbles in the
 *                urn.
 *  Return value: The sampled color or ULONG_MAX if the urn was empty.
 */
static inline ullong bsturn_sample(bsturn_t* u) {
    if(u->nmarbles == 0) return ULLONG_MAX;

    ullong marble = mt_urand(&(u->mt), u->nmarbles);
    ullong node = ROOT;
    for(ullong lvl = 0; lvl < u->height; ++lvl) {
          if(marble <= u->bst[node]) {
              node = LCHILD(node);
          } else {
              node = RCHILD(node);
              marble -= u->bst[node];
          }
    }

    return node - u->cstart;
}

static inline ullong bsturn_draw(bsturn_t* u) {
    if(u->nmarbles == 0) return ULLONG_MAX;

    ullong marble = mt_urand(&(u->mt), u->nmarbles);
    ullong node = ROOT;
    for(ullong lvl = 0; lvl < u->height; ++lvl) {
          if(marble <= u->bst[node]) {
              u->bst[node]--;
              node = LCHILD(node);
          } else {
              node = RCHILD(node);
              marble -= u->bst[node];
          }
    }
    u->nmarbles--;

    return node - u->cstart;
}

/*
 *   Description: Inserts marbles of color c into the urn
 *   Assumptions: c < ncolors and there is enough space in the urn.
 */
static inline void bsturn_cinsert(bsturn_t* u, ullong c, ullong q) {
    ullong node     = u->cstart+c;
    u->bst[node] += q;

    do {
        if(ISLCHILD(node)) {
            node = PARENT(node);
            u->bst[node] += q;
        } else {
            node = PARENT(node);
        }
    } while(node > ROOT);
}

/*
 *   Description: Removes marbles of color c from the urn.
 *   Assumptions: c < ncolors and there are enough marbles to be removed.
 */
static inline void bsturn_cremove(bsturn_t* u, ullong c, ullong q) {
    ullong node    = u->cstart+c;
    u->bst[node] -= q;

    do {
        if(ISLCHILD(node)) {
            node = PARENT(node);
            u->bst[node] -= q;
        } else {
            node = PARENT(node);
        }
    } while(node > ROOT);
}

/*
 *   Description: Inserts marbles of all colors into the urn.
 *   Assumptions: qs needs to be allocated already and hold atleast ncolors members where
 *                the index corresponds to the color with the same value and there needs to be
 *                enough space in the urn.
 */
void bsturn_insert(bsturn_t* u, ullong* qs);

/*
 *   Description: Removes marbles of all colors from the urn.
 *   Assumptions: qs needs to be allocated already and hold atleast ncolors members where
 *                the index corresponds to the color with the same value and there needs to be
 *                enough marbles in the urn to be removed.
 */
void bsturn_remove(bsturn_t* u, ullong* qs);

/*
 *  Description: Removes all marbles, leaving an empty urn.
 */
static inline void bsturn_empty(bsturn_t* u) {
    memset(u->bst, 0, u->nnodes * sizeof(ullong));
}

/*
 *   Description: Getter function for the color distribution of a single color.
 *   Assumptions: c < ncolors.
 */
static inline ullong bsturn_cdist(bsturn_t* u, ullong c) {
    return u->bst[u->cstart+c];
}

/*
 *   Description: Getter function for the color distribution of all colors.
 */
static inline ullong* bsturn_dist(bsturn_t* u) {
    return u->bst + u->cstart;
}

/*
 *   Description: Getter function for the number of marbles currently in the urn.
 *  Return value: The number of marbles currently in the urn.
 */
static inline ullong bsturn_nmarbles(bsturn_t* u) {
    return u->nmarbles;
}

/*
 *  Description: Frees the urn structure and all other pointers allocated by the create function.
 */
void bsturn_destroy(bsturn_t* u);

#endif
