/*
 *      Filename: tbsturn.c
 *   Description: Test file for the bst urn.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include "bsturn.h"

typedef unsigned long long ullong;

#define CALLS 10000000LLU
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
    bsturn_t* u    = NULL;
    bsturn_t* ucpy = NULL;
    ullong colors[NEL];
    ullong dist[NEL];
    ullong sample[NEL];
    ullong draws[NEL];
    int failed = 0;

    // Create Errors
    errno = 0;
    u = bsturn_create(time(NULL), ULLONG_MAX);
    if(u == NULL && errno == EDOM)
        printf("Passed ncolors too large create test.\n");
    else
        printf("Failed ncolors too large create test.\n");

    // Two cinsert, sample, two cremove, nmarbles, and cdist test
    printf("Two cinsert, sample, two cremove, nmarbles, and cdist test:\n");
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    u = bsturn_create(time(NULL), NEL);
    for(ullong i = 0; i < NEL; ++i)
        bsturn_cinsert(u, i, 2);
    for(ullong i = 0; i < NEL; ++i)
        dist[i] = bsturn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[bsturn_sample(u)]++;

    print_ullong_arr("Dist", dist, NEL);
    print_ullong_arr("Sample", sample, NEL);    

    if(bsturn_nmarbles(u) == 2*NEL)
        printf("Passed nmarbles filled test.\n");
    else
        printf("Failed nmarbles filled test.\n");

    for(ullong i = 0; i < NEL; ++i)
        bsturn_cremove(u, i, 2);
    for(ullong i = 0; i < NEL; ++i)
        dist[i] = bsturn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[bsturn_sample(u)]++;
     
    if(bsturn_nmarbles(u) == 0)
        printf("Passed nmarbles empty test.\n");
    else
        printf("Failed nmarbles empty test.\n");

    print_ullong_arr("CRemoved Dist", dist, NEL);
    bsturn_destroy(u);

    // Insert, draw, remove, nmarbles, copy, and dist test:
    printf("Insert, draw, remove, nmarbles, copy, and dist test:\n");
    ullong* lindist;
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    for(ullong i = 0; i < NEL; ++i)
        colors[i] = 1;
    u = bsturn_create(time(NULL), NEL);
    bsturn_insert(u, colors);
    if(bsturn_nmarbles(u) == NEL)
        printf("Passed nmarbles filled test.\n");
    else
        printf("Failed nmarbles filled test.\n");
    lindist = bsturn_dist(u);
    print_ullong_arr("Dist", lindist, NEL);

    failed = 0;
    for(ullong i = 0; i < 10; ++i) {
        for(ullong j = 0; j < NEL; ++j)
            draws[j] = bsturn_draw(u);
        if(bsturn_nmarbles(u) != 0)
            failed = 1;
        bsturn_insert(u, colors);
        print_ullong_arr("Draws", draws, NEL);
    }

    if(failed == 0)
        printf("Passed nmarbles drawn empty test.\n");
    else
        printf("Failed nmarbles drawn empty test.\n");

    failed = 0;
    bsturn_remove(u, colors);
    ullong* distpointer = bsturn_dist(u);
    for(ullong i; i < NEL; ++i) {
        if(distpointer[i] != 0)
            failed = 1;
    }
    if(failed == 0 && bsturn_nmarbles(u) == 0)
        printf("Passed removed test.\n");
    else
        printf("Failed removed test.\n");
    bsturn_insert(u, colors);

    ucpy = bsturn_copy(u, time(NULL));
    lindist = bsturn_dist(ucpy);
    print_ullong_arr("Copy Dist", lindist, NEL);
    if(bsturn_nmarbles(ucpy) == NEL)
        printf("Passed copy nmarbles filled test.\n");
    else
        printf("Failed copy nmarbles filled test.\n");
    for(ullong j = 0; j < NEL; ++j)
        draws[j] = bsturn_draw(ucpy);
    print_ullong_arr("Draws", draws, NEL);
    if(bsturn_nmarbles(ucpy) == 0)
        printf("Passed copy nmarbles drawn empty test.\n");
    else
        printf("Failed copy nmarbles drawn empty test.\n");


    // Empty tests and sample/draw edge cases
    printf("Empty tests and sample/draw edge cases:\n");
    bsturn_empty(u);
    if(bsturn_nmarbles(u) == 0)
        printf("Passed marbles empty test.\n");
    else
        printf("Failed nmarbles empty test.\n");

    failed = 0;
    for(ullong i = 0; i < CALLS; ++i) {
        if(bsturn_sample(u) != ULLONG_MAX || bsturn_draw(u) != ULLONG_MAX)
            failed = 1;
    }

    if(failed == 0)
        printf("Passed Sample/draw test.\n");
    else
        printf("Failed Sample/draw test.\n");
}












