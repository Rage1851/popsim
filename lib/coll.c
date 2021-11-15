/*
 *      Filename: coll.h
 *   Description: The routine coll samples from the distribution that represents how many balls we
 *                can throw into bins until we reach a collision, as in the birthday paradox.
 *                Suppose an urn of n marbles where r are red and n-r are green, now we are
 *                interested in repeatedly sampling from this urn. If we sample a green marble, then
 *                we put a red one back into the urn. If we sample a red marble, then we stop. We
 *                are interested in the number of marbles drawn, until we draw a red marble. The
 *                routine coll samples from exactly this distribution.
 *                The idea for this is taken from P. Berenbrink, D. Hammer, D. Kaaser, U. Meyer,
 *                M. Penschuck, and H. Tran.Simulating. Population Protocols in Sub-Constant Time
 *                per Interaction. 2020. arXiv:2005.03584 [cs.DS].
 *   Assumptions: The random number generator has to be seeded and n and r, r <= n have to be set
 *                before use.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <math.h>
#include "mt.h"
#include "lfac.h"

typedef long double ldouble;

typedef struct coll_t {
    ullong  n, r, g;
    ldouble logn, lfacg;

    mt_t mt;
} coll_t;

void coll_seed(coll_t* c, ullong seed) {
    mt_init(&(c->mt), seed);
}

void coll_setnr(coll_t* c, ullong n, ullong r) {
    c->n = n; c->r = r; c->g = n - r;
    c->logn   = log(n);
    c->lfacg = lfac(c->g + 1);
}

void coll_setn(coll_t* c, ullong n) {
    c->n = n; c->g = n - c->r;
    c->logn   = log(n);
    c->lfacg = lfac(c->g + 1);
}

void coll_setr(coll_t* c, ullong r) {
    c->r = r; c->g = c->n - r;
    c->lfacg = lfac(c->g + 1);
}

static ullong bisec(coll_t* c) {
    ldouble fixed = c->lfacg - log(1.L - mt_real1(&(c->mt)));
    ullong lo = (c->r > 0) ? 1 : 0;
    ullong hi = c->g;

    while(lo + 1 < hi) {
        ullong mi = lo + (hi-lo)/2.L;
        ldouble x = (fixed - lfac(c->g-mi)) - mi*c->logn;
        if(x < 0)  hi = mi;
        else       lo = mi;    
    }

    if(fabs((fixed - lfac(c->g-lo)) - lo*c->logn) <=
            fabs((fixed - lfac(c->g-hi)) - hi*c->logn))
        return lo;
    else
        return hi;
}

static ullong regulafalsi(coll_t* c) {
    ldouble fixed = c->lfacg - log(1.L - mt_real1(&(c->mt)));
    ullong lo = (c->r > 0) ? 1 : 0;
    ullong hi = c->g;
    ldouble xlo = (fixed - lfac(c->g-lo)) - lo*c->logn;
    ldouble xhi = (fixed - lfac(c->g-hi)) - hi*c->logn;

    while(lo+1 < hi) {
        ullong mi = (lo*xhi - hi*xlo) / (xhi-xlo);
        ldouble x = (fixed - lfac(c->g-mi)) - mi*c->logn;
        if(x < 0) {
            hi  = mi;
            xhi = x;
        } else {
            lo  = mi; 
            xlo = x;
        }
    }
    
    if(fabs(xlo) <= fabs(xhi))
        return lo;
    else
        return hi;
}

ullong coll(coll_t* c) {
    if(c->g < 1e6)
        return bisec(c);
    else
        return regulafalsi(c);
}
