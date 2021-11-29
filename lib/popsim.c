/*
 *       Filename: popsim.c
 *         Author: Niklas Mamtschur
 *   Organization: Technical University of Munich (TUM)
 */

#include "popsim.h"
#include "arrurn.h"
#include "linurn.h"
#include "bsturn.h"
#include "aliurn.h"
#include "coll.h"
#include "hgeom.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#define POPSIM_MIN(x,y) ((x) <= (y) ? (x) : (y))
#define POPSIM_MAX(x,y) ((x) >= (y) ? (x) : (y))

typedef struct timespec timespec;

void popsim_seqarr(arrurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                   void (*delta)(ullong, ullong, ullong*, ullong*)) {
    arrurn_dist(u, conf);
    ullong cstep = nsteps / nconf;
    ullong p1, q1, p2, q2;
    for(ullong i = 1, j = 1; i <= nsteps; ++i) {
        p1 = arrurn_draw(u); q1 = arrurn_draw(u); 
        (*delta)(p1, q1, &p2, &q2); 
        arrurn_cinsert(u, p2, 1); arrurn_cinsert(u, q2, 1);

        if(j < nconf && i == j*cstep)
            arrurn_dist(u, conf + (j++)*nstates);
    }
    arrurn_dist(u, conf + nconf*nstates);
}

void popsim_seqlin(linurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                   void (*delta)(ullong, ullong, ullong*, ullong*)) {
    memcpy(conf, linurn_dist(u), nstates * sizeof(ullong));
    ullong cstep = nsteps / nconf;
    ullong p1, q1, p2, q2;
    for(ullong i = 1, j = 1; i <= nsteps; ++i) {
        p1 = linurn_draw(u); q1 = linurn_draw(u); 
        (*delta)(p1, q1, &p2, &q2); 
        linurn_cinsert(u, p2, 1); linurn_cinsert(u, q2, 1);

        if(j < nconf && i == j*cstep)
            memcpy(conf + (j++)*nstates, linurn_dist(u), nstates * sizeof(ullong));
    }
    memcpy(conf + nconf*nstates, linurn_dist(u), nstates * sizeof(ullong));
}

void popsim_seqbst(bsturn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                   void (*delta)(ullong, ullong, ullong*, ullong*)) {
    memcpy(conf, bsturn_dist(u), nstates * sizeof(ullong));
    ullong cstep = nsteps / nconf;
    ullong p1, q1, p2, q2;
    for(ullong i = 1, j = 1; i <= nsteps; ++i) {
        p1 = bsturn_draw(u); q1 = bsturn_draw(u); 
        (*delta)(p1, q1, &p2, &q2); 
        bsturn_cinsert(u, p2, 1); bsturn_cinsert(u, q2, 1);

        if(j < nconf && i == j*cstep)
            memcpy(conf + (j++)*nstates, bsturn_dist(u), nstates * sizeof(ullong));
    }
    memcpy(conf + nconf*nstates, bsturn_dist(u), nstates * sizeof(ullong));
}

void popsim_seqali(aliurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                   void (*delta)(ullong, ullong, ullong*, ullong*)) {
    aliurn_dist(u, conf);
    ullong cstep = nsteps / nconf;
    ullong p1, q1, p2, q2;
    for(ullong i = 1, j = 1; i <= nsteps; ++i) {
        p1 = aliurn_draw(u); q1 = aliurn_draw(u); 
        (*delta)(p1, q1, &p2, &q2); 
        aliurn_cinsert(u, p2, 1); aliurn_cinsert(u, q2, 1);

        if(j < nconf && i == j*cstep)
            aliurn_dist(u, conf + (j++)*nstates);
    }
    aliurn_dist(u, conf + nconf*nstates);
}

int popsim_batch(linurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                 void (*delta)(ullong, ullong, ullong*, ullong*),
                 ullong seed1, ullong seed2, ullong seed3) {
    linurn_t* un = linurn_create(seed1, nstates);
    if(un == NULL) return 0;

    ullong* ic = (ullong*) malloc(nstates * sizeof(ullong));
    if(ic == NULL) return 0;
    ullong* rc = (ullong*) malloc(nstates * sizeof(ullong));
    if(rc == NULL) return 0;

    ullong p1, p2;
    ullong q1, q2;
    
    ullong l;
    coll_t c;
    coll_seed(&c, seed2);
    coll_setnr(&c, linurn_nmarbles(u), 0);

    mt_t mt;
    mt_init(&mt, seed3);

    memcpy(conf, linurn_dist(u), nstates * sizeof(ullong));
    ullong cstep = nsteps / nconf;
    ullong j = 1;
    for(ullong i = 1; i <= nsteps;) {
        do {
            l = coll_coll(&c); 
        } while(l < 2);

        mhgeom(&mt, ic, linurn_dist(u), nstates, linurn_nmarbles(u), l/2);
        linurn_remove(u, ic);
        for(p1 = 0; p1 < nstates; ++p1) {
            mhgeom(&mt, rc, linurn_dist(u), nstates, linurn_nmarbles(u), ic[p1]);
            linurn_remove(u, rc);

            for(q1 = 0; q1 < nstates; ++q1) {
                (*delta)(p1, q1, &p2, &q2); 
                linurn_cinsert(un, p2, rc[q1]);
                linurn_cinsert(un, q2, rc[q1]);
            }
        }

        if(l%2 == 0) {
            p1 = linurn_draw(un);
            linurn_insert(u, linurn_dist(un));
            q1 = linurn_draw(u);
        } else {
            p1 = linurn_draw(u);
            q1 = linurn_draw(un);
            linurn_insert(u, linurn_dist(un));
        }

        (*delta)(p1, q1, &p2, &q2); 
        linurn_cinsert(u, p2, 1);
        linurn_cinsert(u, q2, 1);
        linurn_empty(un);

        i += l/2+1;
        while(j < nconf && i >= j*cstep)
            memcpy(conf + (j++)*nstates, linurn_dist(u), nstates * sizeof(ullong));
    }
    while(j <= nconf)
        memcpy(conf + (j++)*nstates, linurn_dist(u), nstates * sizeof(ullong));

    linurn_destroy(un);
    free(ic); free(rc);
    return 1;
}

int popsim_mbatch(bsturn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                   void (*delta)(ullong, ullong, ullong*, ullong*),
                   ullong seed1, ullong seed2, ullong seed3) {
    bsturn_t* un = bsturn_create(seed1, nstates);
    if(un == NULL) return 0;

    ullong* ic = (ullong*) malloc(nstates * sizeof(ullong));
    if(ic == NULL) return 0;
    ullong* rc = (ullong*) malloc(nstates * sizeof(ullong));
    if(rc == NULL) return 0;

    ullong p1, p2;
    ullong q1, q2;
    ullong r1, r2;

    ullong l;
    coll_t c;
    coll_seed(&c, seed2);
    coll_setn(&c, bsturn_nmarbles(u));

    mt_t mt;
    mt_init(&mt, seed3);

    ullong epoch = (nstates*(ldouble) nstates) / (log(bsturn_nmarbles(u))/log(2.L));
    epoch = POPSIM_MAX(epoch, 1);
    int dir = 1;
    timespec starttp, endtp;
    ldouble pput = 0.L, cput = 0.L;

    int fstcoll, scdcoll;
    memcpy(conf, bsturn_dist(u), nstates * sizeof(ullong));
    ullong cstep = nsteps / nconf;
    ullong j = 1;
    for(ullong i = 1, k = 0, t = 0; i <= nsteps; k = 0, t = 0) {
        pput = cput;
        clock_gettime(CLOCK_REALTIME, &starttp);

        for(ullong e = 0; e < epoch && bsturn_nmarbles(u) > 0; ++e) {
            coll_setr(&c, t + bsturn_nmarbles(un));
            do {
                l = coll_coll(&c); 
            } while((t + bsturn_nmarbles(un) == 0) && l < 2);
            t += 2*(l/2);

            fstcoll = (l%2 == 0);
            scdcoll = (fstcoll == 0) || mt_urand(&mt, bsturn_nmarbles(u)) < t;

            if(fstcoll) {
                if(mt_urand(&mt, t + bsturn_nmarbles(un)) < t) {
                    p1 = bsturn_draw(u);
                    r1 = bsturn_draw(u);
                    (*delta)(p1, r1, &p2, &r2); k++;

                    if(mt_real1(&mt) <= 0.5L) {
                        bsturn_cinsert(un, r2, 1);
                        p1 = p2;
                    } else {
                        bsturn_cinsert(un, p2, 1);
                        p1 = r2;
                    }
                    t -= 2;
                } else {
                    p1 = bsturn_draw(un);
                }
            } else {
                p1 = bsturn_draw(u);
            }

            if(scdcoll) {
                if(mt_urand(&mt, t + bsturn_nmarbles(un)) < t) {
                    q1 = bsturn_draw(u);
                    r1 = bsturn_draw(u);
                    (*delta)(r1, q1, &r2, &q2); k++;

                    if(mt_real1(&mt) <= 0.5L) {
                        bsturn_cinsert(un, r2, 1);
                        q1 = q2;
                    } else {
                        bsturn_cinsert(un, q2, 1);
                        q1 = r2;
                    }
                    t -= 2;
                } else {
                    q1 = bsturn_draw(un);
                }
            } else {
                q1 = bsturn_draw(u);
            }
            
            (*delta)(p1, q1, &p2, &q2);
            bsturn_cinsert(un, p2, 1);
            bsturn_cinsert(un, q2, 1);
            k++;
        }

        mhgeom(&mt, ic, bsturn_dist(u), nstates, bsturn_nmarbles(u), t/2);
        bsturn_remove(u, ic);
        for(p1 = 0; p1 < nstates; ++p1) {
            mhgeom(&mt, rc, bsturn_dist(u), nstates, bsturn_nmarbles(u), ic[p1]);
            bsturn_remove(u, rc);

            for(q1 = 0; q1 < nstates; ++q1) {
                (*delta)(p1, q1, &p2, &q2); 
                bsturn_cinsert(un, p2, rc[q1]);
                bsturn_cinsert(un, q2, rc[q1]);
            }
        }

        bsturn_insert(u, bsturn_dist(un));
        k += t/2;
        bsturn_empty(un);

        clock_gettime(CLOCK_REALTIME, &endtp);
        cput = k / ((endtp.tv_sec-starttp.tv_sec) + (endtp.tv_nsec-starttp.tv_nsec)*1e-9);
        if(cput < pput)
            dir *= -1;
        epoch += dir;
        epoch = POPSIM_MAX(epoch, 1);
        
        i += k;
        while(j < nconf && i >= j*cstep)
            memcpy(conf + (j++)*nstates, bsturn_dist(u), nstates * sizeof(ullong));
    }
    while(j <= nconf)
        memcpy(conf + (j++)*nstates, bsturn_dist(u), nstates * sizeof(ullong));

    bsturn_destroy(un);
    free(ic); free(rc);
    return 1;
}
