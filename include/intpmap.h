/*
 *      Filename: intpmap.h
 *   Description: Hash map for integer pairs implemented with xxhash and a sorted linked list as
 *                the chaining resolution. It uses different integer sizes based on the largest
 *                possible value for kfst, kscd, vfst, and vscd.
 *   Assumptions: kfst, kscd, vfst, and vscd all have to be smaller than the respective max element
 *                of their data type.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef INT_PAIR_MAP_H
#define INT_PAIR_MAP_H

#ifndef XXH_INLINE_ALL
#define XXH_INLINE_ALL
#endif
#include "xxhash.h"

typedef unsigned char      ubyte;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long ullong;

typedef struct bentry_t {
    ubyte  k[2];
    ubyte  v[2];
    ullong n;
} bentry_t;

typedef struct sentry_t {
    ushort k[2];
    ushort v[2];
    ullong n;
} sentry_t;

typedef struct ientry_t {
    uint   k[2];
    uint   v[2];
    ullong n;
} ientry_t;

typedef struct lentry_t {
    ulong  k[2];
    ulong  v[2];
    ullong n;
} lentry_t;

typedef struct llentry_t {
    ullong k[2];
    ullong v[2];
    ullong n;
} llentry_t;

typedef enum intpmap_size_t {
    INTPMAP_BYTE,
    INTPMAP_SHORT,
    INTPMAP_INT,
    INTPMAP_LONG,
    INTPMAP_LLONG,
} intpmap_size_t;

// Should be treated as opaque.
typedef struct intpmap_t {
    ullong         nel;
    ullong         nchain;
    ullong         max_nchain;
    intpmap_size_t size;

    bentry_t*  bvals;
    sentry_t*  svals;
    ientry_t*  ivals;
    lentry_t*  lvals;
    llentry_t* llvals;

    bentry_t*  bchain;
    sentry_t*  schain;
    ientry_t*  ichain;
    lentry_t*  lchain;
    llentry_t* llchain;
} intpmap_t;

/*
 *  Description: Return the next prime number p >= n. Note that if n is prime, then n will be
 *               returned.
 *  Assumptions: n is not allowed to be larger than the largest prime that a 64 bit integer can
 *               represent.
 */
ullong nextprime(ullong n);

/*
 *   Description: Initializes and allocates a new hash map where nel represents the maximum number
 *                of elements that will be inserted and max_entry is the largest value that any
 *                of the first or second members of any of the keys or values will have.
 *  Return value: A pointer to the hash map or NULL on error.
 *        Errors: ENOMEM if there was not enough memory for the hash map or EDOM if
 *                nel is greater than the largest prime that fits in ullong or EDOM if
 *                max_entry == ULLONG_MAX.
 */
intpmap_t* intpmap_create(ullong nel, ullong max_entry);

/*
 *    Description: Inserts the (kfst,kscd)->(vfst,vscd) mapping into the map.
 *   Return value: Zero if there were errors enlarging the dynamic array holding the chaining list
 *                 and non-zero otherwise.
 *    Assumptions: No duplicate keys shall be inserted.
 */
int intpmap_insert(intpmap_t* m, ullong kfst, ullong kscd, ullong vfst, ullong vscd);

/*
 *   Description: Fills *vfst and *vscd with the mapping of the key (kfst,kscd). If the mapping was
 *                not in the hash map, then *vfst and *vscd are filled with ULLONG_MAX.
 */
static inline void intpmap_lookup(intpmap_t* m, ullong  kfst, ullong  kscd,
                                                ullong* vfst, ullong* vscd) {
    // Its faster to have a constant key size, see github of xxhash
    ullong pair[2]; pair[0] = kfst; pair[1] = kscd;
    ullong hash = XXH3_64bits(pair, 2*sizeof(ullong)) % m->nel;

    switch(m->size) {
        case INTPMAP_BYTE:
            bentry_t* be = m->bvals + hash;
            if(be->k[0] == kfst && be->k[1] == kscd) {
                *vfst = be->v[0];
                *vscd = be->v[1];
                return;
            }
            if(be->n == ULLONG_MAX)
                break;

            be = m->bchain + be->n;
            while(be->n != ULLONG_MAX && (kfst > be->k[0] || (kfst == be->k[0] && kscd > be->k[1])))
                be = m->bchain + be->n;

            if(kfst == be->k[0] && kscd == be->k[1]) {
                *vfst = be->v[0];
                *vscd = be->v[1];
                return;
            }
            break;
        case INTPMAP_SHORT:
            sentry_t* se = m->svals + hash;
            if(se->k[0] == kfst && se->k[1] == kscd) {
                *vfst = se->v[0];
                *vscd = se->v[1];
                return;
            }
            if(se->n == ULLONG_MAX)
                break;

            se = m->schain + se->n;
            while(se->n != ULLONG_MAX && (kfst > se->k[0] || (kfst == se->k[0] && kscd > se->k[1])))
                se = m->schain + se->n;

            if(kfst == se->k[0] && kscd == se->k[1]) {
                *vfst = se->v[0];
                *vscd = se->v[1];
                return;
            }
            break;
        case INTPMAP_INT:
            ientry_t* ie = m->ivals + hash;
            if(ie->k[0] == kfst && ie->k[1] == kscd) {
                *vfst = ie->v[0];
                *vscd = ie->v[1];
                return;
            }
            if(ie->n == ULLONG_MAX)
                break;

            ie = m->ichain + ie->n;
            while(ie->n != ULLONG_MAX && (kfst > ie->k[0] || (kfst == ie->k[0] && kscd > ie->k[1])))
                ie = m->ichain + ie->n;

            if(kfst == ie->k[0] && kscd == ie->k[1]) {
                *vfst = ie->v[0];
                *vscd = ie->v[1];
                return;
            }
            break;
        case INTPMAP_LONG:
            lentry_t* le = m->lvals + hash;
            if(le->k[0] == kfst && le->k[1] == kscd) {
                *vfst = le->v[0];
                *vscd = le->v[1];
                return;
            }
            if(le->n == ULLONG_MAX)
                break;

            le = m->lchain + le->n;
            while(le->n != ULLONG_MAX && (kfst > le->k[0] || (kfst == le->k[0] && kscd > le->k[1])))
                le = m->lchain + le->n;

            if(kfst == le->k[0] && kscd == le->k[1]) {
                *vfst = le->v[0];
                *vscd = le->v[1];
                return;
            }
            break;
        case INTPMAP_LLONG:
            llentry_t* lle = m->llvals + hash;
            if(lle->k[0] == kfst && lle->k[1] == kscd) {
                *vfst = lle->v[0];
                *vscd = lle->v[1];
                return;
            }
            if(lle->n == ULLONG_MAX)
                break;

            lle = m->llchain + lle->n;
            while(lle->n != ULLONG_MAX &&
                    (kfst > lle->k[0] || (kfst == lle->k[0] && kscd > lle->k[1])))
                lle = m->llchain + lle->n;

            if(kfst == lle->k[0] && kscd == lle->k[1]) {
                *vfst = lle->v[0];
                *vscd = lle->v[1];
                return;
            }
            break;
    }

    *vfst = ULLONG_MAX;
    *vscd = ULLONG_MAX;
}

/*
 *   Description: Frees the memory kept by the hash table.
 */
void intpmap_destroy(intpmap_t* m);

#endif
