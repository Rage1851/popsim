/*
 *      Filename: ran.h
 *   Description: Random number generator introduced in W. H. Press, S. A. Teukolsky, W. T.
 *                Vetterling, and B. P. Flannery. Numerical Recipes 3rd Edition: The Art of
 *                Scientific Computing. 3rd ed. USA: Cambridge University Press, 2007. Chap. 7:
 *                Random Numbers. isbn: 0521880688.
 *   Assumptions: Ran needs to be seeded before use.
 *    Changed by: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef RAN_H
#define RAN_H

typedef unsigned long long ullong;

static ullong ran_u, ran_v, ran_w;

static inline ullong ran(){
    ran_v = 4101842887655102017LLU;
    ran_w = 1;

    ran_u = ran_u * 2862933555777941757LLU + 7046029254386353087LLU;
    ran_v ^= ran_v >> 17; ran_v ^= ran_v << 31; ran_v ^= ran_v >> 8;
    ran_w = 4294957665LLU*(ran_w & 0xffffffff) + (ran_w >> 32);
    ullong x = ran_u ^ (ran_u << 21); x ^= x >> 35; x ^= x << 4;

    return (x + ran_v) ^ ran_w;
}

static void sran(ullong j){
    ran_u = j ^ ran_v; ran();
    ran_v = ran_u; ran();
    ran_w = ran_v; ran();
}

#endif
