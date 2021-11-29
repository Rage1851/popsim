/*
 *      Filename: intpmap.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 *     Important: The implementation for the nextprime function is derived from
 *     https://stackoverflow.com/questions/4475996/given-prime-number-n-compute-the-next-prime.
 */
#include "intpmap.h"

#ifndef XXH_INLINE_ALL
#define XXH_INLINE_ALL
#endif
#include "xxhash.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

#define MAX_PRIME        18446744073709551557LLU
#define CHAIN_START_SIZE 256LLU
#define PSTEP            30LLU
#define NPLOOKUP         10LLU
#define NPCAND           8LLU
#define PCAND_SKIP       3LLU

// Primes smaller than 30
static const ullong plookup[] = {
    2LLU,
    3LLU,
    5LLU,
    7LLU,
    11LLU,
    13LLU,
    17LLU,
    19LLU,
    23LLU,
    29LLU,
};

// Potential prime candidates which are not divisible by 2,3 or 5
static const ullong pcand[] = {
    1LLU,
    7LLU,
    11LLU,
    13LLU,
    17LLU,
    19LLU,
    23LLU,
    29LLU,
};

static int isprime(ullong n) {
    ullong p,q;
    for(ullong i = PCAND_SKIP; i < NPLOOKUP; ++i) {
        p = plookup[i];
        q = n/p;
        if(p > q)    return 1;
        if(n == p*q) return 0;
    }

    for(ullong i = PSTEP;; i += PSTEP) {
        for(ullong j = 0; j < NPCAND; ++j) {
            p = i+pcand[j];
            q = n/p;
            if(p > q)    return 1;
            if(n == p*q) return 0;
        }
    }

    return 1;
}

ullong nextprime(ullong n) {
    ullong i = 0;
    if(n < PSTEP) {
        while(plookup[i] < n) ++i;
        return plookup[i];
    }

    ullong j = PSTEP*(n/PSTEP);
    while(j + pcand[i] < n) ++i;

    n = j + pcand[i];
    while(!isprime(n)) {
        if(++i == NPCAND) {
            j += PSTEP;
            i = 0;
        }
        n = j + pcand[i];
    }

    return n;
}

intpmap_t* intpmap_create(ullong nel, ullong max_entry) {
    if(nel > MAX_PRIME || max_entry == ULLONG_MAX) {
        errno = EDOM;
        return NULL;
    }

    intpmap_t* m = (intpmap_t*) malloc(sizeof(intpmap_t));
    if(m == NULL) return NULL;

    m->nel        = nextprime(nel);
    m->nchain     = 0;
    m->max_nchain = CHAIN_START_SIZE;

    if(max_entry < UCHAR_MAX) {
        m->size = INTPMAP_BYTE;
        if((m->bvals  = (bentry_t*) malloc(m->nel * sizeof(bentry_t))) == NULL) return NULL;
        for(ullong i = 0; i < m->nel; ++i) {
            m->bvals[i].k[0] = UCHAR_MAX;
            m->bvals[i].n    = ULLONG_MAX;
        }

        if((m->bchain = (bentry_t*) malloc(CHAIN_START_SIZE * sizeof(bentry_t))) == NULL)
            return NULL;
    } else if(max_entry < USHRT_MAX) {
        m->size = INTPMAP_SHORT;
        if((m->svals  = (sentry_t*) malloc(m->nel * sizeof(sentry_t))) == NULL) return NULL;
        for(ullong i = 0; i < m->nel; ++i) {
            m->svals[i].k[0] = USHRT_MAX;
            m->svals[i].n    = ULLONG_MAX;
        }

        if((m->schain = (sentry_t*) malloc(CHAIN_START_SIZE * sizeof(sentry_t))) == NULL)
            return NULL;
    } else if(max_entry < UINT_MAX) {
        m->size = INTPMAP_INT;
        if((m->ivals  = (ientry_t*) malloc(m->nel * sizeof(ientry_t))) == NULL) return NULL;
        for(ullong i = 0; i < m->nel; ++i) {
            m->ivals[i].k[0] = UINT_MAX;
            m->ivals[i].n    = ULLONG_MAX;
        }

        if((m->ichain = (ientry_t*) malloc(CHAIN_START_SIZE * sizeof(ientry_t))) == NULL)
            return NULL;
    } else if(max_entry < ULONG_MAX){
        m->size = INTPMAP_LONG;
        if((m->lvals  = (lentry_t*) malloc(m->nel * sizeof(lentry_t))) == NULL) return NULL;
        for(ullong i = 0; i < m->nel; ++i) {
            m->lvals[i].k[0] = ULONG_MAX;
            m->lvals[i].n    = ULLONG_MAX;
        }

        if((m->lchain = (lentry_t*) malloc(CHAIN_START_SIZE * sizeof(lentry_t))) == NULL)
            return NULL;
    } else {
        m->size = INTPMAP_LLONG;
        if((m->llvals  = (llentry_t*) malloc(m->nel * sizeof(llentry_t)))) return NULL;
        for(ullong i = 0; i < m->nel; ++i) {
            m->llvals[i].k[0] = ULLONG_MAX;
            m->llvals[i].n    = ULLONG_MAX;
        }

        if((m->llchain = (llentry_t*) malloc(CHAIN_START_SIZE * sizeof(llentry_t))) == NULL)
            return NULL;
    }

    return m;
}

static inline int resize_bchain(intpmap_t* m) {
    bentry_t* new_bchain = (bentry_t*) realloc(m->bchain, 2*m->max_nchain * sizeof(bentry_t));
    if(new_bchain == NULL)
        return 0;
    m->max_nchain = 2*m->max_nchain;
    m->bchain = new_bchain;
}

static inline int resize_schain(intpmap_t* m) {
    sentry_t* new_schain = (sentry_t*) realloc(m->schain, 2*m->max_nchain * sizeof(sentry_t));
    if(new_schain == NULL)
        return 0;
    m->max_nchain = 2*m->max_nchain;
    m->schain = new_schain;
    return 1;
}

static inline int resize_ichain(intpmap_t* m) {
    ientry_t* new_ichain = (ientry_t*) realloc(m->ichain, 2*m->max_nchain * sizeof(ientry_t));
    if(new_ichain == NULL)
        return 0;
    m->max_nchain = 2*m->max_nchain;
    m->ichain = new_ichain;
    return 1;
}

static inline int resize_lchain(intpmap_t* m) {
    lentry_t* new_lchain = (lentry_t*) realloc(m->lchain, 2*m->max_nchain * sizeof(lentry_t));
    if(new_lchain == NULL)
        return 0;
    m->max_nchain = 2*m->max_nchain;
    m->lchain = new_lchain;
    return 1;
}

static inline int resize_llchain(intpmap_t* m) {
    llentry_t* new_llchain = (llentry_t*) realloc(m->llchain, 2*m->max_nchain * sizeof(llentry_t));
    if(new_llchain == NULL)
        return 0;
    m->max_nchain = 2*m->max_nchain;
    m->llchain = new_llchain;
    return 1;
}

int intpmap_insert(intpmap_t* m, ullong kfst, ullong kscd, ullong vfst, ullong vscd) {
    // Its faster to have a constant key size, see github of xxhash
    ullong pair[2]; pair[0] = kfst; pair[1] = kscd;
    ullong hash = XXH3_64bits(pair, 2*sizeof(ullong)) % m->nel;
    
    int end = 0;
    switch(m->size) {
        case INTPMAP_BYTE:
            bentry_t* be = m->bvals + hash;
            // Bucket was empty
            if(be->k[0] == UCHAR_MAX) {
                be->k[0] = kfst; be->k[1] = kscd;
                be->v[0] = vfst; be->v[1] = vscd;
                return 1; 
            }

            if(m->nchain >= m->max_nchain)
                if(resize_bchain(m) == 0)
                    return 0;

            bentry_t* bnew = m->bchain + m->nchain; 
            bnew->k[0] = kfst; bnew->k[1] = kscd;
            bnew->v[0] = vfst; bnew->v[1] = vscd;

            // First chain element was empty
            if(be->n == ULLONG_MAX) {
                bnew->n = ULLONG_MAX;
                be->n   = m->nchain++;
                return 1;
            }

            bentry_t* bn = m->bchain + be->n;
            while(kfst > bn->k[0] || (kfst == bn->k[0] && kscd > bn->k[1])) {
                if(bn->n == ULLONG_MAX) {
                    end = 1;
                    break;
                }
                be = bn;
                bn = m->bchain + bn->n;
            }

            if(end) {
                bnew->n = ULLONG_MAX;
                bn->n   = m->nchain++;
            } else {
                bnew->n = be->n;
                be->n = m->nchain++;
            }

            return 1;
        case INTPMAP_SHORT:
            sentry_t* se = m->svals + hash;
            // Bucket was empty
            if(se->k[0] == USHRT_MAX) {
                se->k[0] = kfst; se->k[1] = kscd;
                se->v[0] = vfst; se->v[1] = vscd;
                return 1; 
            }

            if(m->nchain >= m->max_nchain)
                if(resize_schain(m) == 0)
                    return 0;

            sentry_t* snew = m->schain + m->nchain; 
            snew->k[0] = kfst; snew->k[1] = kscd;
            snew->v[0] = vfst; snew->v[1] = vscd;

            // First chain element was empty
            if(se->n == ULLONG_MAX) {
                snew->n = ULLONG_MAX;
                se->n   = m->nchain++;
                return 1;
            }

            sentry_t* sn = m->schain + se->n;
            while(kfst > sn->k[0] || (kfst == sn->k[0] && kscd > sn->k[1])) {
                if(sn->n == ULLONG_MAX) {
                    end = 1;
                    break;
                }
                se = sn;
                sn = m->schain + sn->n;
            }

            if(end) {
                snew->n = ULLONG_MAX;
                sn->n   = m->nchain++;
            } else {
                snew->n = se->n;
                se->n   = m->nchain++;
            }

            return 1;
        case INTPMAP_INT:
            ientry_t* ie = m->ivals + hash;
            // Bucket was empty
            if(ie->k[0] == UINT_MAX) {
                ie->k[0] = kfst; ie->k[1] = kscd;
                ie->v[0] = vfst; ie->v[1] = vscd;
                return 1; 
            }

            if(m->nchain >= m->max_nchain)
                if(resize_ichain(m) == 0)
                    return 0;

            ientry_t* inew = m->ichain + m->nchain; 
            inew->k[0] = kfst; inew->k[1] = kscd;
            inew->v[0] = vfst; inew->v[1] = vscd;

            // First chain element was empty
            if(ie->n == ULLONG_MAX) {
                inew->n = ULLONG_MAX;
                ie->n   = m->nchain++;
                return 1;
            }

            ientry_t* in = m->ichain + ie->n;
            while(kfst > in->k[0] || (kfst == in->k[0] && kscd > in->k[1])) {
                if(in->n == ULLONG_MAX) {
                    end = 1;
                    break;
                }
                ie = in;
                in = m->ichain + in->n;
            }

            if(end) {
                inew->n = ULLONG_MAX;
                in->n   = m->nchain++;
            } else {
                inew->n = ie->n;
                ie->n   = m->nchain++;
            }

            return 1;
        case INTPMAP_LONG:
            lentry_t* le = m->lvals + hash;
            // Bucket was empty
            if(le->k[0] == ULONG_MAX) {
                le->k[0] = kfst; le->k[1] = kscd;
                le->v[0] = vfst; le->v[1] = vscd;
                return 1; 
            }

            if(m->nchain >= m->max_nchain)
                if(resize_lchain(m) == 0)
                    return 0;

            lentry_t* lnew = m->lchain + m->nchain; 
            lnew->k[0] = kfst; lnew->k[1] = kscd;
            lnew->v[0] = vfst; lnew->v[1] = vscd;

            // First chain element was empty
            if(le->n == ULLONG_MAX) {
                lnew->n = ULLONG_MAX;
                le->n   = m->nchain++;
                return 1;
            }

            lentry_t* ln = m->lchain + le->n;
            while(kfst > ln->k[0] || (kfst == ln->k[0] && kscd > ln->k[1])) {
                if(ln->n == ULLONG_MAX) {
                    end = 1;
                    break;
                }
                le = ln;
                ln = m->lchain + ln->n;
            }

            if(end) {
                lnew->n = ULLONG_MAX;
                ln->n   = m->nchain++;
            } else {
                lnew->n = le->n;
                le->n   = m->nchain++;
            }

            return 1;
        case INTPMAP_LLONG:
            llentry_t* lle = m->llvals + hash;
            // Bucket was empty
            if(lle->k[0] == ULONG_MAX) {
                lle->k[0] = kfst; lle->k[1] = kscd;
                lle->v[0] = vfst; lle->v[1] = vscd;
                return 1; 
            }

            if(m->nchain >= m->max_nchain)
                if(resize_llchain(m) == 0)
                    return 0;

            llentry_t* llnew = m->llchain + m->nchain; 
            llnew->k[0] = kfst; llnew->k[1] = kscd;
            llnew->v[0] = vfst; llnew->v[1] = vscd;

            // First chain element was empty
            if(lle->n == ULLONG_MAX) {
                llnew->n = ULLONG_MAX;
                lle->n   = m->nchain++;
                return 1;
            }

            llentry_t* lln = m->llchain + lle->n;
            while(kfst > lln->k[0] || (kfst == lln->k[0] && kscd > lln->k[1])) {
                if(lln->n == ULLONG_MAX) {
                    end = 1;
                    break;
                }
                lle = lln;
                lln = m->llchain + lln->n;
            }

            if(end) {
                llnew->n = ULLONG_MAX;
                lln->n   = m->nchain++;
            } else {
                llnew->n = lle->n;
                lle->n   = m->nchain++;
            }

            return 1;
    }
}

void intpmap_destroy(intpmap_t* m) {
    switch(m->size) {
        case INTPMAP_BYTE:  free(m->bvals);  free(m->bchain);  break;
        case INTPMAP_SHORT: free(m->svals);  free(m->schain);  break;
        case INTPMAP_INT:   free(m->ivals);  free(m->ichain);  break;
        case INTPMAP_LONG:  free(m->lvals);  free(m->lchain);  break;
        case INTPMAP_LLONG: free(m->llvals); free(m->llchain); break;
    }

    free(m);
}
