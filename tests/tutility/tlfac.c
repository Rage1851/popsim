/*
 *      Filename: tlfac.c
 *   Description: Test file for the log factorial utility.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include "lfac.h"

typedef unsigned long long ullong;

#define CALLS 10000000

/*
 *  Difficult to test lfac compared to lgammal because the results are floating point numbers and
 *  we do not care for that lfac is exactly equal to lgammal but rather the digits than can
 *  accurately be represented. Another issue is that for lgammal integers bigger than 2^53 will
 *  not be representable exactly, furthering the inaccuracy. To test this utility, error ranges
 *  were used and independent numbers can be picked out and can be compared to arbitrary precision
 *  results.
 */
int main(int argc, char** argv) {
    printf("LDouble digit precision: %d\n", LDBL_DIG);

    int failed = 0;
    for(ullong i = 0; i < CALLS; ++i)
        if(fabs(lgammal(i+1) - lfac(i)) >= 1e-6) {
            printf("Wrong result for %llu: lgammal %Lf != %Lf lfac.\n", i, lgammal(i+1), lfac(i));
            failed = 1;
            break;
        }
    if(failed == 0)
        printf("Passed lower range.\n");
    failed = 0;

    for(ullong i = ULLONG_MAX-CALLS; i < ULLONG_MAX; ++i)
        if(fabs(lgammal(i+1) - lfac(i)) >= 1e6) {
            printf("Wrong result for %llu: lgammal %Lf != %Lf lfac.\n", i, lgammal(i+1), lfac(i));
            break;
        }
    if(failed == 0)
        printf("Passed upper range.\n");

    printf("Highest input result: %Lf\n", lfac(ULLONG_MAX));
}
