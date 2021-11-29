/*
 *       Filename: thgeom.c
 *    Description: Test file for sampling from the (multivariate) hypergeometric distribution.
 *         Author: Niklas Mamtschur
 *   Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "hgeom.h"
#include "mt.h"

typedef unsigned long long ullong;
typedef long long          llong;

#define CALLS 10000000
#define NEL 101

void print_ullong_arr(char* prefix, ullong* arr, ullong nel) {
    printf("%s:", prefix);
    for(ullong i = 0; i < nel; ++i)
        printf(" %llu", arr[i]);
    printf("\n");
}

void print_llong_arr(char* prefix, llong* arr, ullong nel) {
    printf("%s:", prefix);
    for(ullong i = 0; i < nel; ++i)
        printf(" %lld", arr[i]);
    printf("\n");
}

/*
 *  To test the hgeom sampling the labels and results can be used to plot a bar graph to see if
 *  the sampled values populate around the mean and have an adequate curve. This can for example be
 *  done at https://www.rapidtables.com/tools/bar-graph.html. Additionally, edge cases are tested.
 *
 *  mhgeom is tested by taking a equidistributed marble distribution and expecting the result to be
 *  roughly equidsitributed as well. Edge cases are tested as well.
 */
int main(int argc, char** argv) {
    mt_t mt;
    mt_init(&mt, time(NULL));
    ullong arr[NEL];
    ullong label[NEL];
    int failed = 0;


    // Print label
    for(ullong i = 0; i < NEL; ++i)
        label[i] = i;
    print_ullong_arr("Label", label, NEL);

    // hyp
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = 0;
    for(ullong i = 0; i < CALLS; ++i)
        arr[hgeom(&mt, 100, 50, 10)]++;
    print_ullong_arr("hyp", arr, NEL);

    // hrua
    for(ullong i = 0; i < NEL; ++i)
        arr[i] = 0;
    for(ullong i = 0; i < CALLS; ++i)
        arr[hgeom(&mt, 100, 50, 50)]++;
    print_ullong_arr("hrua", arr, NEL);

    // Total=1
    for(ullong i = 0; i < CALLS; ++i)
        if(hgeom(&mt, 1, 1, 1) != 1 || hgeom(&mt, 1, 0, 1) != 0 ||
                hgeom(&mt, 1, 1, 0) != 0 || hgeom(&mt, 1, 0, 0) != 0)
            failed = 1;
    if(failed == 0)
        printf("Passed total=1 edge case tests.\n");
    else
        printf("Failed total=1 edge case tests.\n");

    // Sample=0
    failed = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(hgeom(&mt, 10, 5, 0) != 0 || hgeom(&mt, 100, 50, 0) != 0) {
            failed = 1;
            break;
        }
    if(failed == 0)
        printf("Passed sample=0 edge case tests.\n");
    else
        printf("Failed sample=0 edge case tests.\n");

    // good=0
    failed = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(hgeom(&mt, 10, 0, 5) != 0 || hgeom(&mt, 100, 0, 50) != 0) {
            failed = 1;
            break;
        }
    if(failed == 0)
        printf("Passed good=0 edge case tests.\n");
    else
        printf("Failed good=0 edge case tests.\n");

    // total=good
    failed = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(hgeom(&mt, 10, 10, 5) != 5 || hgeom(&mt, 100, 100, 50) != 50) {
            failed = 1;
            break;
        }
    if(failed == 0)
        printf("Passed total=good edge case tests.\n");
    else
        printf("Failed total=good edge case tests.\n");

    // total=sample
    failed = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(hgeom(&mt, 10, 5, 10) != 5 || hgeom(&mt, 100, 50, 100) != 50) {
            failed = 1;
            break;
        }
    if(failed == 0)
        printf("Passed total=sample edge case tests.\n");
    else
        printf("Failed total=sample edge case tests.\n");

    // total=good=sample
    failed = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(hgeom(&mt, 10, 10, 10) != 10 || hgeom(&mt, 100, 100, 100) != 100) {
            failed = 1;
            break;
        }
    if(failed == 0)
        printf("Passed total=good=sample edge case tests.\n");
    else
        printf("Failed total=good=sample edge case tests.\n");

    // hgeom
    ullong src[] = {100,100,100,100,100,100,100,100,100,100};
    ullong dest[10];
    llong marr[] = {0,0,0,0,0,0,0,0,0,0};

    for(ullong i = 0; i < CALLS/10; ++i) {
        mhgeom(&mt, dest, src, 10, 100*10, 100*5);
        for(ullong j = 0; j < 10; ++j)
            marr[j] += dest[j];
    }

    for(ullong i = 0; i < 10; ++i)
        marr[i] -= (CALLS/10)*50;
    print_llong_arr("mhgeom", marr, 10);

    // Check sum=sample
    llong sum = 0;
    for(ullong i = 0; i < 10; ++i)
        sum += marr[i];
    if(sum == 0)
        printf("Passed sum=sample test.\n");
    else
        printf("Failed sum=sample test.\n");

    // One dimensional src test.
    failed = 0;
    ullong d1src[] = {100};
    ullong d1dest[1];

    for(ullong i = 0; i < CALLS/10; ++i) {
        mhgeom(&mt, d1dest, d1src, 1, 100, 50);
        if(d1dest[0] != 50)
            failed = 1;
    }

    if(failed == 0)
        printf("Passed one dimensional src edge case tests.\n");
    else
        printf("Failed one dimensional src edge case tests.\n");

    // Left skewed test
    failed = 0;
    ullong leftsrc[10] = {100,100,100,100,100,0,0,0,0,0};
    ullong leftdest[10];

    for(ullong i = 0; i < CALLS/10; ++i) {
        mhgeom(&mt, leftdest, leftsrc, 10, 100*5, 250);
        for(ullong j = 0; j < 5; ++j)
            if(leftdest[j] == 0)
                failed = 1;
        for(ullong j = 5; j < 10; ++j)
            if(leftdest[j] != 0)
                failed = 1;
    }

    if(failed == 0)
        printf("Passed left skewed edge case tests.\n");
    else
        printf("Failed left skewed edge case tests.\n");
}







