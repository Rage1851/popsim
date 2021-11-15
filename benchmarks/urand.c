/*
 *      Filename: urand.c
 *   Description: Benchmarking different uniform PRNG.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#include "mt.h"

#define NCALLS 1000000000

/*
 *  Defining the Ran random number generator.
 *  See William H. Press et al. Numerical Recipes 3rd Edition: The Art of Scientific Computing
 *  Chapter 7. Random Numbers.
 */
typedef struct ran_t {
    unsigned long long u, v, w;
} ran_t;

unsigned long long ran(ran_t r){
    r.v = 4101842887655102017LL;
    r.w = 1;

    r.u = r.u * 2862933555777941757LL + 7046029254386353087LL;
    r.v ^= r.v >> 17; r.v ^= r.v << 31; r.v ^= r.v >> 8;
    r.w = 4294957665U*(r.w & 0xffffffff) + (r.w >> 32);
    unsigned long long x = r.u ^ (r.u << 21); x ^= x >> 35; x ^= x << 4;

    return (x + r.v) ^ r.w;
}

void sran(ran_t r, unsigned long long j){
    r.u = j ^ r.v; ran(r);
    r.v = r.u; ran(r);
    r.w = r.v; ran(r);
}

int main(int argc, char** argv) {
    clock_t start, end;
    unsigned long long r;

    // rand
    srand(time(NULL));
    start = clock();
    for(int i = 0; i < NCALLS;++i)
        r = rand() % i;
    end = clock();
    printf("rand: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    // random
    srandom(time(NULL));
    start = clock();
    for(int i = 0; i < NCALLS;++i)
        r = random() % i;
    end = clock();
    printf("random: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    // devrandom
    FILE* f = fopen("/dev/random", "r");
    start = clock();
    for(int i = 0; i < NCALLS;++i) {
        fread(&r, sizeof(unsigned long long), 1, f);
        r = r%i;
    }
    end = clock();
    printf("devrandom: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    // Ran
    ran_t ran_state;
    sran(ran_state, time(NULL));
    start = clock();
    for(int i = 0; i < NCALLS;++i)
        r = ran(ran_state) % i;
    end = clock();
    printf("Ran: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    // MT
    mt_t mt;
    mt_init(&mt, time(NULL));
    start = clock();
    for(int i = 0; i < NCALLS;++i)
        r = mt_rand(&mt) % i;
    end = clock();
    printf("MT: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    return 0;
}
