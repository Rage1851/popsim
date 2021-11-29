/*
 *      Filename: tarrurn.c
 *   Description: Test file for the array urn.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include "arrurn.h"

typedef unsigned long long ullong;

#define CALLS 100000000LLU
#define NEL   10LLU

void print_ullong_arr(char* prefix, ullong* arr, ullong nel) {
    printf("%s:", prefix);
    for(ullong i = 0LLU; i < nel; ++i)
        printf(" %llu", arr[i]);
    printf("\n");
}

/*
 *  If we enter a uniform color distribution into the urn, then we expect to sample a uniform
 *  distribution as well. If we draw from the urn, we expect a randomly drawn sequence where each
 *  marble occurs exactly once.
 */
int main(int argc, char** argv) {
    arrurn_t* u    = NULL;
    arrurn_t* ucpy = NULL;
    ullong colors[NEL];
    ullong dist[NEL];
    ullong sample[NEL];
    int failed = 0;

    // Create Errors
    errno = 0;
    u = arrurn_create(time(NULL), ULLONG_MAX, 100);
    if(u == NULL && errno == EDOM)
        printf("Passed ncolors too large create.\n");
    else
        printf("Failed ncolors too large create.\n");

    errno = 0;
    u = arrurn_create(time(NULL), 100, ULLONG_MAX);
    if(u == NULL && errno == EDOM)
        printf("Passed max_nmarbles too large create.\n");
    else
        printf("Failed max_nmarbles too large create.\n");

    // Sizes, two cinsert, sample, and cdist tests
    printf("Sizes, two cinsert, sample, and cdist tests:\n");

    // Byte
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    u = arrurn_create(time(NULL), NEL+0, 2*NEL);
    for(ullong i = 0; i < NEL+0; ++i)
        arrurn_cinsert(u, i, 2);
    for(ullong i = 0; i < NEL+0; ++i)
        dist[i] = arrurn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[arrurn_sample(u)]++;
    arrurn_destroy(u);

    print_ullong_arr("Byte Dist", dist, NEL);
    print_ullong_arr("Byte Sample", sample, NEL);

    // Short
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    u = arrurn_create(time(NULL), NEL+UCHAR_MAX, 2*NEL);
    for(ullong i = UCHAR_MAX; i < NEL+UCHAR_MAX; ++i)
        arrurn_cinsert(u, i, 2);
    for(ullong i = UCHAR_MAX; i < NEL+UCHAR_MAX; ++i)
        dist[i-UCHAR_MAX] = arrurn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[arrurn_sample(u)-UCHAR_MAX]++;
    arrurn_destroy(u);

    print_ullong_arr("Short Dist", dist, NEL);
    print_ullong_arr("Short Sample", sample, NEL);

    // Int
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    u = arrurn_create(time(NULL), NEL+USHRT_MAX, 2*NEL);
    for(ullong i = USHRT_MAX; i < NEL+USHRT_MAX; ++i)
        arrurn_cinsert(u, i, 2);
    for(ullong i = USHRT_MAX; i < NEL+USHRT_MAX; ++i)
        dist[i-USHRT_MAX] = arrurn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[arrurn_sample(u)-USHRT_MAX]++;
    arrurn_destroy(u);

    print_ullong_arr("Int Dist", dist, NEL);
    print_ullong_arr("Int Sample", sample, NEL);

    // Long
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    u = arrurn_create(time(NULL), NEL+UINT_MAX, 2*NEL);
    for(ullong i = UINT_MAX; i < NEL+UINT_MAX; ++i)
        arrurn_cinsert(u, i, 2);
    for(ullong i = UINT_MAX; i < NEL+UINT_MAX; ++i)
        dist[i-UINT_MAX] = arrurn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[arrurn_sample(u)-UINT_MAX]++;
    arrurn_destroy(u);

    print_ullong_arr("Long Dist", dist, NEL);
    print_ullong_arr("Long Sample", sample, NEL);

    // ULLong and ulong are the same on my system
    
    // Insert, copy, draw, and dist tests
    printf("Insert, copy, draw, and dist tests\n");
    u = arrurn_create(time(NULL), NEL, NEL);
    for(ullong i = 0; i < NEL; ++i)
        colors[i] = 1;
    for(ullong i = 0; i < NEL; ++i)
        dist[i] = 0;
    arrurn_insert(u, colors);
    arrurn_dist(u, dist);
    print_ullong_arr("Dist", dist, NEL);

    ullong draws[NEL];
    for(ullong i = 0; i < 10; ++i) {
        for(ullong j = 0; j < NEL; ++j)
            draws[j] = arrurn_draw(u);
        arrurn_insert(u, colors);
        print_ullong_arr("Draws", draws, NEL);
    }

    ucpy = arrurn_copy(u, time(NULL));
    arrurn_dist(ucpy, dist);
    print_ullong_arr("Copy Dist", dist, NEL);
    for(ullong j = 0; j < NEL; ++j)
        draws[j] = arrurn_draw(ucpy);
    print_ullong_arr("Copy Draws", draws, NEL);


    // Empty, sample, nmarlbe, and draw test
    failed = 0;
    if(arrurn_nmarbles(u) == NEL)
        printf("Passed nmarbles filled test.\n");
    else
        printf("Failed nmarbles filled test.\n");
    arrurn_empty(u);
    if(arrurn_nmarbles(u) == 0)
        printf("Passed nmarbles empty test.\n");
    else
        printf("Failed nmarlbes empty test.\n");
    for(ullong i = 0; i < CALLS; ++i) {
        if(arrurn_sample(u) != ULLONG_MAX || arrurn_draw(u) != ULLONG_MAX)
            failed = 1;
    }

    if(failed == 0)
        printf("Passed empty, sample, and draw test.\n");
    else
        printf("Failed empty, sample, and draw test.\n");
    
}
