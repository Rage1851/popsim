/*
 *      Filename: rand_range.c
 *   Description: Implementation of a RNG in the range of [0,n), n <= RAND_MAX + 1
 *                based on the C standard library function rand().
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <bool.h>

bool rand_range_init = false;

void rand_range_seed() {
    assert(!rand_range_init);
    srand(time(NULL));
    rand_range_init = true;
}

unsigned int rand_range(unsigned int n_exclusive) {
    assert(rand_range_init);
    assert(n_exclusive > 0);
    assert(n_exclusive <= RAND_MAX);

    unsigned int intervals = (RAND_MAX + 1u) / n_exclusive;
    unsigned int max_accept_exclusive = intervals * n_exclusive;
    unsigned int random_val;
    do {
        random_val = rand();
    } while(random_val >= max_accept_exclusive);

    return random_val / intervals;
}
