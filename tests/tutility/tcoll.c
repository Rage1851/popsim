/*
 *      Filename: tcoll.c
 *   Description: Test file for the collision sampling.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "coll.h"

typedef unsigned long long ullong;

#define CALLS 10000000
#define NEL   101

void print_ullong_arr(char* prefix, ullong* arr, ullong nel) {
    printf("%s:", prefix);
    for(ullong i = 0; i < nel; ++i)
        printf(" %llu", arr[i]);
    printf("\n");
}

int main(int argc, char** argv) {
    coll_t c;
    coll_seed(&c, time(NULL));
    ullong label[NEL];
    ullong arr[NEL];
    int failed = 0;

    for(ullong i = 0; i < NEL; ++i) {
        arr[i]   = 0;
        label[i] = i;
    }

    // Print labels
    print_ullong_arr("Label", label, NEL);

    // General bisec r=0 run
    coll_setnr(&c, NEL-1, 0);
    for(ullong i = 0; i < CALLS; ++i)
        arr[coll_bisec(&c)]++;
    print_ullong_arr("bisec r=0", arr, NEL);

    // General regula falsi r=0 run
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = 0;
    for(ullong i = 0; i < CALLS; ++i)
        arr[coll_regulafalsi(&c)]++;
    print_ullong_arr("regula falsi r=0", arr, NEL);

    // General bisec r large run
    coll_setnr(&c, 101, 99);
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = 0;
    for(ullong i = 0; i < CALLS; ++i)
        arr[coll_bisec(&c)]++;
    print_ullong_arr("bisec r large", arr, NEL);

    // General regula falsi r large run
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = 0;
    for(ullong i = 0; i < CALLS; ++i)
        arr[coll_regulafalsi(&c)]++;
    print_ullong_arr("regula falsi r large", arr, NEL);

    // n = 1 test case
    coll_setn(&c, 1);
    coll_setr(&c, 0);
    for(ullong i = 0; i < CALLS; ++i) {
        coll_setr(&c, 0);
        if(coll(&c) != 1)
            failed = 1;
        coll_setr(&c, 1);
        if(coll(&c) != 0)
            failed = 1;
    }

    if(failed == 0)
        printf("Passed n=1 edge case test.\n");
    else
        printf("Failed n=1 edge case test.\n");

    // r=n test case
    failed = 0;
    coll_setn(&c, 100);
    coll_setr(&c, 100);
    for(ullong i = 0; i < CALLS; ++i) {
        if(coll(&c) != 0)
            failed = 1;
    }

    if(failed == 0)
        printf("Passed r=n edge case test.\n");
    else
        printf("Failed r=n edge case test.\n");
    
}
