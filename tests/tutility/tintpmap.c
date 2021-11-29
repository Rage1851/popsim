/*
 *      Filename: tintpmap.c
 *   Description: Test file for the integer pair hash map.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM) 
 */

#include "intpmap.h"
#include "mt.h"

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#define NEL       10000LLU
#define MAX_PRIME 18446744073709551557LLU

typedef unsigned char      ubyte;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long ullong;

int main(int argc, char** argv) {
    printf("Next prime test:\n");
    for(ullong i = 0; i < 100; i += 3) {
        printf("%llu: %llu, %llu: %llu, %llu: %llu,\n",
                i, nextprime(i), i+1, nextprime(i+1), i+2, nextprime(i+2));
     }

    errno = 0;
    if(intpmap_create(NEL, ULLONG_MAX) == NULL && errno == EDOM)
        printf("Passed create max_entry too large test.\n");
    else
        printf("Failed create max_entry too large test.\n");
    errno = 0;
    if(intpmap_create(MAX_PRIME+1, 100) == NULL && errno == EDOM)
        printf("Passed create nel too large test.\n");
    else
        printf("Failed create nel too large test.\n");

    printf("NEL: %llu, nextprime(NEL): %llu\n", NEL, nextprime(NEL));

    intpmap_t* bm = intpmap_create(NEL, UCHAR_MAX-1LL);
    intpmap_t* sm = intpmap_create(NEL, USHRT_MAX-1LL);
    intpmap_t* im = intpmap_create(NEL, UINT_MAX -1LL);
    intpmap_t* lm = intpmap_create(NEL, ULONG_MAX-1LL);

    if(bm != NULL && sm != NULL && im != NULL && lm != NULL)
        printf("Hash map creation worked.\n");
    else
        printf("Hash map creation did not work.\n");

    int failed = 0;
    for(ullong i = 0; i < NEL; ++i) {
        if(i+3 < UCHAR_MAX) {
            if(intpmap_insert(bm, i, i+1, i+2, i+3) == 0)
                failed = 1;
        }
        if(intpmap_insert(sm, i+UCHAR_MAX, i+1+UCHAR_MAX, i+2+UCHAR_MAX, i+3+UCHAR_MAX) == 0)
            failed = 1;
        if(intpmap_insert(im, i+USHRT_MAX, i+1+USHRT_MAX, i+2+USHRT_MAX, i+3+USHRT_MAX) == 0)
            failed = 1;
        if(intpmap_insert(lm, i+UINT_MAX, i+1+UINT_MAX, i+2+UINT_MAX, i+3+UINT_MAX) == 0)
            failed = 1;
    }

    for(ullong i = NEL; i < 2*NEL; ++i) {
        if(i < UCHAR_MAX) {
            if(intpmap_insert(bm, i, i-1, i-2, i-3) == 0)
                failed = 1;
        }
        if(intpmap_insert(sm, i+UCHAR_MAX, i-1+UCHAR_MAX, i-2+UCHAR_MAX, i-3+UCHAR_MAX) == 0)
            failed = 1;
        if(intpmap_insert(im, i+USHRT_MAX, i-1+USHRT_MAX, i-2+USHRT_MAX, i-3+USHRT_MAX) == 0)
            failed = 1;
        if(intpmap_insert(lm, i+UINT_MAX, i-1+UINT_MAX, i-2+UINT_MAX, i-3+UINT_MAX) == 0)
            failed = 1;
    }

    if(failed == 0)
        printf("Inserting elements worked.\n");
    else
        printf("Inserting elements did not work.\n");

    failed = 0;
    ullong vfst, vscd;
    for(ullong i = 0; i < NEL; ++i) {
        if(i+3 < UCHAR_MAX) {
            intpmap_lookup(bm, i, i+1, &vfst, &vscd);
            if(vfst != i+2 && vscd != i+3) {
                printf("False result in bm. Expected (%llu,%llu), actually (%llu,%llu).\n",
                        i+2, i+3, vfst, vscd);
                failed = 1;
            }
        }

        intpmap_lookup(sm, i+UCHAR_MAX, i+1+UCHAR_MAX, &vfst, &vscd);
        if(vfst != i+2+UCHAR_MAX && vscd != i+3+UCHAR_MAX) {
            printf("False result in sm. Expected (%llu,%llu), actually (%llu,%llu).\n",
                    i+2+UCHAR_MAX, i+3+UCHAR_MAX, vfst, vscd);
            failed = 1;
        }

        intpmap_lookup(im, i+USHRT_MAX, i+1+USHRT_MAX, &vfst, &vscd);
        if(vfst != i+2+USHRT_MAX && vscd != i+3+USHRT_MAX) {
            printf("False result in im. Expected (%llu,%llu), actually (%llu,%llu).\n",
                    i+2+USHRT_MAX, i+3+USHRT_MAX, vfst, vscd);
            failed = 1;
        }

        intpmap_lookup(lm, i+UINT_MAX, i+1+UINT_MAX, &vfst, &vscd);
        if(vfst != i+2+UINT_MAX && vscd != i+3+UINT_MAX) {
            printf("False result in lm. Expected (%llu,%llu), actually (%llu,%llu).\n",
                    i+2+UINT_MAX, i+3+UINT_MAX, vfst, vscd);
            failed = 1;
        }
    }

    for(ullong i = NEL; i < 2*NEL; ++i) {
        if(i < UCHAR_MAX) {
            intpmap_lookup(bm, i, i-1, &vfst, &vscd);
            if(vfst != i-2 && vscd != i-3) {
                printf("False result in bm. Expected (%llu,%llu), actually (%llu,%llu).\n",
                        i-2, i-3, vfst, vscd);
                failed = 1;
            }
        }

        intpmap_lookup(sm, i+UCHAR_MAX, i-1+UCHAR_MAX, &vfst, &vscd);
        if(vfst != i-2+UCHAR_MAX && vscd != i-3+UCHAR_MAX) {
            printf("False result in sm. Expected (%llu,%llu), actually (%llu,%llu).\n",
                    i-2+UCHAR_MAX, i-3+UCHAR_MAX, vfst, vscd);
            failed = 1;
        }

        intpmap_lookup(im, i+USHRT_MAX, i-1+USHRT_MAX, &vfst, &vscd);
        if(vfst != i-2+USHRT_MAX && vscd != i-3+USHRT_MAX) {
            printf("False result in im. Expected (%llu,%llu), actually (%llu,%llu).\n",
                    i-2+USHRT_MAX, i-3+USHRT_MAX, vfst, vscd);
            failed = 1;
        }

        intpmap_lookup(lm, i+UINT_MAX, i-1+UINT_MAX, &vfst, &vscd);
        if(vfst != i-2+UINT_MAX && vscd != i-3+UINT_MAX) {
            printf("False result in lm. Expected (%llu,%llu), actually (%llu,%llu).\n",
                    i-2+UINT_MAX, i-3+UINT_MAX, vfst, vscd);
            failed = 1;
        }
    }

    if(failed == 0)
        printf("Passed insert/lookup test.\n");
    else
        printf("Failed insert/lookup test.\n");

    failed = 0;
    for(ullong i = 0; i < 2*NEL; ++i) {
        if(i < UCHAR_MAX) {
            intpmap_lookup(bm, i, i, &vfst, &vscd);
            if(vfst != ULLONG_MAX || vscd != ULLONG_MAX) {
                printf("bm failed for i: %llu\n", i);
                failed = 1;
            }
        }

        intpmap_lookup(sm, i, i, &vfst, &vscd);
        if(vfst != ULLONG_MAX || vscd != ULLONG_MAX) {
            printf("sm failed for i: %llu\n", i);
            failed = 1;
        }

        intpmap_lookup(im, i, i, &vfst, &vscd);
        if(vfst != ULLONG_MAX || vscd != ULLONG_MAX) {
            printf("im failed for i: %llu\n", i);
            failed = 1;
        }

        intpmap_lookup(lm, i, i, &vfst, &vscd);
        if(vfst != ULLONG_MAX || vscd != ULLONG_MAX) {
            printf("lm failed for i: %llu\n", i);
            failed = 1;
        }
    }

    if(failed == 0)
        printf("Passed not inserted, not found test.\n");
    else
        printf("Failed not inserted, not found test.\n");

    intpmap_destroy(bm);
    intpmap_destroy(sm);
    intpmap_destroy(im);
    intpmap_destroy(lm);

    if(nextprime(MAX_PRIME) == MAX_PRIME && nextprime(MAX_PRIME-1) == MAX_PRIME)
        printf("Passed max prime test.\n");
    else
        printf("Failed max prime test.\n");
}

