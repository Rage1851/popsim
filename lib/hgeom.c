/*
 *      Filename: hgeom.c
 *    Changed by: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

/*
Disclaimer: This code was taken from NumPy's random number generation and was slightly modified.
https://github.com/numpy/numpy/blob/623bc1fae1d47df24e7f1e29321d0c0ba2771ce0/numpy/random/src/distributions/hgeom.c

Copyright (c) 2005-2021, NumPy Developers.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.

    * Neither the name of the NumPy Developers nor the names of any
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <math.h>
#include "lfac.h"
#include "mt.h"

// D1 = 2*sqrt(2/e)
// D2 = 3 - 2*sqrt(3/e)
#define D1 1.7155277699214135
#define D2 0.8989161620588988
#define MIN(x,y) ((x)<=(y) ? (x) : (y))
#define MAX(x,y) ((x)>=(y) ? (x) : (y))

typedef long double lldouble;

static ullong hgeom_sample(mt_t* mt, ullong good, ullong bad, ullong sample){
    ullong remaining_total, remaining_good, result, computed_sample;
    ullong total = good + bad;

    if(sample > total/2) computed_sample = total - sample;
    else                  computed_sample = sample;

    remaining_total = total;
    remaining_good = good;

    while((computed_sample > 0) && (remaining_good > 0) && (remaining_total > remaining_good)) {
        if(mt_urand(mt, remaining_total) < remaining_good)
            --remaining_good;
        --remaining_total;
        --computed_sample;
    }

    if(remaining_total == remaining_good) remaining_good -= computed_sample;
    if (sample > total/2) result = remaining_good;
    else result = good - remaining_good;

    return result;
}

static ullong hgeom_hrua(mt_t* mt, ullong good, ullong bad, ullong sample){
    ullong mingoodbad, maxgoodbad, popsize;
    ullong computed_sample;
    ldouble p, q;
    ldouble mu, var;
    ldouble a, c, b, h, g;
    ullong m, K;

    popsize = good + bad;
    computed_sample = MIN(sample, popsize - sample);
    mingoodbad = MIN(good, bad);
    maxgoodbad = MAX(good, bad);

    p = ((ldouble) mingoodbad) / popsize;
    q = ((ldouble) maxgoodbad) / popsize;

    mu = computed_sample * p;

    a = mu + 0.5;

    var = ((ldouble)(popsize - computed_sample) * computed_sample * p * q / (popsize - 1));

    c = sqrt(var + 0.5);

    h = D1*c + D2;

    m = (ullong) floor((ldouble)(computed_sample + 1) * (mingoodbad + 1) / (popsize + 2));

    g = (lfac(m) + lfac(mingoodbad - m) + lfac(computed_sample - m) +
         lfac(maxgoodbad - computed_sample + m));

    b = MIN(MIN(computed_sample, mingoodbad) + 1, floor(a + 16*c));

    while (1) {
        ldouble U, V, X, T;
        ldouble gp;
        U = mt_real3(mt);
        V = mt_real3(mt);
        X = a + h*(V - 0.5) / U;

        // fast rejection:
        if ((X < 0.0) || (X >= b)) continue;

        K = (ullong) floor(X);

        gp = (lfac(K) + lfac(mingoodbad - K) + lfac(computed_sample - K) +
             lfac(maxgoodbad - computed_sample + K));

        T = g - gp;

        // fast acceptance:
        if((U*(4.0 - U) - 3.0) <= T) break;

        // fast rejection:
        if(U*(U - T) >= 1) continue;

        // acceptance
        if(2.0*log(U) <= T) break;
    }

    if(good > bad) K = computed_sample - K;

    if(computed_sample < sample) K = good - K;

    return K;
}

ullong hgeom(mt_t* mt, ullong total, ullong good, ullong sample){
    if ((sample >= 10) && (sample <= total - 10))
        return hgeom_hrua(mt, good, total-good, sample);
    else
        return hgeom_sample(mt, good, total-good, sample);
}

void mhgeom(mt_t* mt, ullong* destdist, ullong* srcdist,
                   ullong ncolors, ullong total, ullong sample) {
    for(ullong c = 0; c < ncolors; ++c) {
        ullong x = hgeom(mt, total, srcdist[c], sample);
        destdist[c] = x;
        if((total -= x) == 0) break;
    }
}
