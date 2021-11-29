/*
 *      Filename: turand.c
 *   Description: Test file for the urand utility.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "ran.h"
#include "mt.h"

typedef unsigned long long ullong;

#define CALLS 1000000000
#define NEL   10

void print_llong_arr(char* prefix, long long* arr, ullong nel) {
    printf("%s:", prefix);
    for(ullong i = 0; i < nel; ++i)
        printf(" %lld", arr[i]);
    printf("\n");
}

/*
 *  We expect the numbers from 0 to NEL inclusive to be roughly equidistributed and if the modulo
 *  bias is not accounted for then we expect the array to be split into two roughly equidistributed
 *  halves where the first half has higher numbers.
 */
int main(int argc, char** argv) {
    long long arr[] = {0,0,0,0,0,0,0,0,0,0};
    sran(time(NULL));
    mt_t mt;
    mt_init(&mt, time(NULL));

    // Ran
    // Here, modulo bias is not accounted for
    for(ullong i = 0; i < CALLS; ++i)
        arr[ran() % NEL]++;
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = arr[i] - CALLS/NEL;
    print_llong_arr("Ran", arr, NEL);
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = 0;

    // MT
    for(ullong i = 0; i < CALLS; ++i)
        arr[mt_urand(&mt, NEL)]++;
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = arr[i] - CALLS/NEL;
    print_llong_arr("MT", arr, NEL);

    // MT urand n=1 edge case test
    ullong not_zero_count = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(mt_urand(&mt, 1) != 0)
            not_zero_count++;
    if(not_zero_count == 0)
        printf("Passed MT Edge Case n=0.\n");
    else
        printf("Failed MT Edge Case n=0.\n");
}
