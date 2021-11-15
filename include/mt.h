/*
 *      Filename: mt.h
 *   Description: Reentrant and renamed version of the Mersenne Twister random number generator
 *                implementation mt19937-64 by Makoto Matsumoto and Takuji Nishimura.
 *   Assumptions: Init needs to be called before any of the generator functions.
 *    Changed by: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

/* 
   A C-program for MT19937-64 (2004/9/29 version).
   Coded by Takuji Nishimura and Makoto Matsumoto.

   This is a 64-bit version of Mersenne Twister pseudorandom number
   generator.

   Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   References:
   T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
     ACM Transactions on Modeling and 
     Computer Simulation 10. (2000) 348--357.
   M. Matsumoto and T. Nishimura,
     ``Mersenne Twister: a 623-dimensionally equidistributed
       uniform pseudorandom number generator''
     ACM Transactions on Modeling and 
     Computer Simulation 8. (Jan. 1998) 3--30.

   Any feedback is very welcome.
   http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)
*/

#ifndef MT_H
#define MT_H

#include <stdio.h>
#include <limits.h>

#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */

typedef unsigned long long ullong;
typedef long double        ldouble;

// Should be treated as opaque.
typedef struct mt_t {
    ullong mt[NN];
    int mti;
} mt_t;

static const ullong mt_mag01[2]={0ULL, MATRIX_A};

/*
 *  Description: Initialize the mt state object with a seed.
 */
static void mt_init(mt_t* mt, ullong seed) {
    mt->mt[0] = seed;
    for(mt->mti=1; mt->mti<NN; mt->mti++) {
        mt->mt[mt->mti] = (6364136223846793005ULL*
                (mt->mt[mt->mti-1] ^ (mt->mt[mt->mti-1] >> 62)) + mt->mti);
    }
}

/*
 *  Description: Generates integer random numbers in [0,ULLONG_MAX).
 *
 */
static inline ullong mt_rand(mt_t* mt) {
    int i;
    ullong x;

    if(mt->mti >= NN) { // generate NN words at one time
        for (i=0;i<NN-MM;i++) {
            x = (mt->mt[i]&UM)|(mt->mt[i+1]&LM);
            mt->mt[i] = mt->mt[i+MM] ^ (x>>1) ^ mt_mag01[(int)(x&1ULL)];
        }
        for (;i<NN-1;i++) {
            x = (mt->mt[i]&UM)|(mt->mt[i+1]&LM);
            mt->mt[i] = mt->mt[i+(MM-NN)] ^ (x>>1) ^ mt_mag01[(int)(x&1ULL)];
        }
        x = (mt->mt[NN-1]&UM)|(mt->mt[0]&LM);
        mt->mt[NN-1] = mt->mt[MM-1] ^ (x>>1) ^ mt_mag01[(int)(x&1ULL)];

        mt->mti = 0;
    }
  
    x = mt->mt[mt->mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}

/*
 *  Description: Generate integer random numbers in [0,n).
 *  Assumptions: n > 0.
 */
static inline ullong mt_urand(mt_t* mt, ullong n) {
    const ullong min = ULLONG_MAX % n;
    ullong x = mt_rand(mt);
    while(x <= min)
        x = mt_rand(mt);
    return x;
}

/*
 *  Description: Generate real random numbers in [0,1].
 */
static inline ldouble mt_real1(mt_t* mt) {
    return (mt_rand(mt) >> 11) * (1.0/9007199254740991.0);
}

/*
 *  Description: Generate real random numbers in [0,1).
 */
static inline ldouble mt_real2(mt_t* mt) {
    return (mt_rand(mt) >> 11) * (1.0/9007199254740992.0);
}

/*
 *  Description: Generate real random numbers in (0,1).
 */
static inline ldouble mt_real3(mt_t* mt) {
    return ((mt_rand(mt) >> 12) + 0.5) * (1.0/4503599627370496.0);
}

#endif
