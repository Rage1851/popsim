/*
 *      Filename: taliurn.c
 *   Description: Test file for the dynamic alias table urn.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include "aliurn.h"

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
 *  Different distributions to build the alias table are also tested by repeatedly inserting
 *  marbles.
 */
int main(int argc, char** argv) {
    aliurn_t* u    = NULL;
    aliurn_t* ucpy = NULL;
    ullong colors[NEL];
    ullong dist[NEL];
    ullong sample[NEL];
    ullong draws[NEL];
    int failed = 0;

    // Create Errors
    errno = 0;
    u = aliurn_create(time(NULL), ULLONG_MAX, 0.8, 1.5);
    if(u == NULL && errno == EDOM)
        printf("Passed ncolors too large create test.\n");
    else
        printf("Failed ncolors too large create test.\n");

    errno = 0;
    u = aliurn_create(time(NULL), 100, 0, 1.5);
    if(u == NULL && errno == EDOM)
        printf("Passed alpha too small create test.\n");
    else
        printf("Failed alpha too small create test.\n");

    errno = 0;
    u = aliurn_create(time(NULL), 100, 1, 1.5);
    if(u == NULL && errno == EDOM)
        printf("Passed alpha too large create test.\n");
    else
        printf("Failed alpha too large create test.\n");

    errno = 0;
    u = aliurn_create(time(NULL), 100, 0.8, 1);
    if(u == NULL && errno == EDOM)
        printf("Passed beta too small create test.\n");
    else
        printf("Failed beta too small create test.\n");

    // Two cinsert, sample, nmarbles, and cdist test
    printf("Two cinsert, sample, nmarbles, and cdist test:\n");
    for(ullong i = 0; i < NEL; ++i)
        sample[i] = 0;
    u = aliurn_create(time(NULL), NEL, 0.8, 1.5);
    for(ullong i = 0; i < NEL; ++i)
        aliurn_cinsert(u, i, 2);
    for(ullong i = 0; i < NEL; ++i)
        dist[i] = aliurn_cdist(u, i);
    for(ullong i = 0; i < CALLS; ++i)
        sample[aliurn_sample(u)]++;

    print_ullong_arr("Dist", dist, NEL);
    print_ullong_arr("Sample", sample, NEL);    

    if(aliurn_nmarbles(u) == 2*NEL)
        printf("Passed nmarbles filled test.\n");
    else
        printf("Failed nmarbles filled test.\n");
    aliurn_destroy(u);

    // Insert, draw, remove, nmarbles, copy, and dist test:
    printf("Insert, draw, nmarbles, and dist test:\n");
    for(ullong i = 0; i < NEL; ++i)
        colors[i] = 1;
    u = aliurn_create(time(NULL), NEL, 0.8, 1.5);
    aliurn_insert(u, colors);
    aliurn_dist(u, dist);
    print_ullong_arr("Dist", dist, NEL);

    failed = 0;
    for(ullong i = 0; i < 10; ++i) {
        for(ullong j = 0; j < NEL; ++j)
            draws[j] = aliurn_draw(u);
        if(aliurn_nmarbles(u) != 0)
            failed = 1;
        aliurn_insert(u, colors);
        print_ullong_arr("Draws", draws, NEL);
    }

    if(failed == 0)
        printf("Passed nmarbles drawn empty test.\n");
    else
        printf("Failed nmarbles drawn empty test.\n");

    // Empty and sample/draw edge cases
    aliurn_empty(u);
    if(aliurn_nmarbles(u) == 0)
        printf("Passed nmarbles empty test.\n");
    else
        printf("Failed nmarbles empty test.\n");

    failed = 0;
    for(ullong i = 0; i < CALLS; ++i) {
        if(aliurn_sample(u) != ULLONG_MAX || aliurn_draw(u) != ULLONG_MAX)
            failed = 1;
    }

    if(failed == 0)
        printf("Passed sample/draw edge case test.\n");
    else
        printf("Failed sample/draw edge case test.\n");

}
