/*
 *      Filename: bfac.c
 *   Description: Benchmarks for the calculation of log factorial implementations.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <time.h>

#include "lfac.h"

#define CALLS 1000000000

int main(int argc, char** argv) {
    clock_t start, end;
    double r;

    start = clock();
    for(int i = 0; i < CALLS; ++i)
        r = lgamma(i);
    end = clock();
    printf("lgamma: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    start = clock();
    for(int i = 0; i < CALLS; ++i)
        lfac(i);
    end = clock();
    printf("stirling: %f\n", (double) (end-start) / CLOCKS_PER_SEC);

    return 0;
}
