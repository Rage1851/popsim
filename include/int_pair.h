/*
 *      Filename: int_pair.h
 *   Description: Header file for the definition of pair structures of different integer sizes
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef INT_PAIR_H
#define INT_PAIR_H

#include <gmp.h>

typedef struct byte_pair {
    unsigned char fst;
    unsigned char scd;
} byte_pair;

typedef struct short_pair {
    unsigned short fst;
    unsigned short scd;
} short_pair;

typedef struct int_pair {
    unsigned int fst;
    unsigned int scd;
} int_pair;

typedef struct long_pair {
    unsigned long fst;
    unsigned long scd;
} long_pair;

typedef struct long_mpz_pair {
    unsigned long fst;
    mpz_t         scd;
} long_mpz_pair;

typedef struct mpz_long_pair {
    mpz_t         fst;
    unsigned long scd;
} mpz_long_pair;

typedef struct mpz_pair {
    mpz_t fst;
    mpz_t scd;
} mpz_pair;

#endif
