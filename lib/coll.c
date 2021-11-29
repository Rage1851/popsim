/*
 *      Filename: coll.c
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <math.h>
#include "coll.h"
#include "mt.h"
#include "lfac.h"

void coll_seed(coll_t* c, ullong seed) {
    mt_init(&(c->mt), seed);
}

void coll_setnr(coll_t* c, ullong n, ullong r) {
    c->n = n; c->r = r; c->g = n - r;
    c->logn  = log(n);
    c->lfacg = lfac(c->g);
}

void coll_setn(coll_t* c, ullong n) {
    c->n = n; c->g = n - c->r;
    c->logn  = log(n);
    c->lfacg = lfac(c->g);
}

void coll_setr(coll_t* c, ullong r) {
    c->r = r; c->g = c->n - r;
    c->lfacg = lfac(c->g);
}

ullong coll_bisec(coll_t* c) {
    ldouble fixed = log(mt_real1(&(c->mt))) - c->lfacg;
    ullong lo = (c->r > 0) ? 0 : 1;
    ullong hi = c->g+1;
    ldouble x;

    while(lo + 1 < hi) {
        ullong mi = lo + (hi-lo)/2.L;
        x = fixed + lfac(c->g-mi) + mi*c->logn;
        if(x > 0.L) hi = mi;
        else        lo = mi;    
    }

    return lo;
}

ullong coll_regulafalsi(coll_t* c) {
    ldouble fixed = log(mt_real1(&(c->mt))) - c->lfacg;
    ullong lo = (c->r > 0) ? 0 : 1;
    ullong hi = c->g+1;
    ldouble xlo = fixed + lfac(c->g-lo) + lo*c->logn;
    ldouble xhi = fixed + lfac(c->g-hi) + hi*c->logn;
    ldouble x;

    for(ullong i = 0; i < 15; ++i) {
        ullong mi = (lo*xhi - hi*xlo) / (xhi-xlo);
        x = fixed + lfac(c->g-mi) + mi*c->logn;
        if(x > 0.L) {
            xhi = x;
            hi = mi;
        } else {
            xlo = x;
            lo = mi; 
        }
    }
    
    while(lo + 1 < hi) {
        ullong mi = lo + (hi-lo)/2.L;
        x = fixed + lfac(c->g-mi) + mi*c->logn;
        if(x > 0.L) hi = mi;
        else        lo = mi;    
    }

    return lo;
}

ullong coll_coll(coll_t* c) {
    if(c->g < 1e6)
        return coll_bisec(c);
    else
        return coll_regulafalsi(c);
}
