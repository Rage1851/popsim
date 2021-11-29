/*
 *      Filename: coll.h
 *   Description: The routine coll samples from the distribution that represents how many balls we
 *                can throw into bins until we reach a collision, as in the birthday paradox.
 *                Suppose an urn of n marbles where r are red and n-r are green, now we are
 *                interested in repeatedly sampling from this urn. If we sample a green marble, then
 *                we put a red one back into the urn. If we sample a red marble, then we stop. We
 *                are interested in the number of marbles drawn, until we draw a red marble. The
 *                routine coll samples from exactly this distribution.
 *                The idea for this is taken from P. Berenbrink, D. Hammer, D. Kaaser, U. Meyer,
 *                M. Penschuck, and H. Tran.Simulating. Population Protocols in Sub-Constant Time
 *                per Interaction. 2020. arXiv:2005.03584 [cs.DS].
 *   Assumptions: The collision rng has to be seeded before use and n as well as r, r <= n have to
 *                be set before use. Additionally, 0 < n < ULLONG_MAX.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef COLL_H
#define COLL_H

#include "mt.h"

typedef unsigned long long ullong;
typedef long double        ldouble;

// Should be treated as opaque.
typedef struct coll_t {
    ullong  n, r, g;
    ldouble logn, lfacg;

    mt_t mt;
} coll_t;

/*
 *  Description: Seeds the underlying random number generator.
 */
void coll_seed(coll_t* c, ullong seed);

/*
 *  Description: Choose one of coll_bisec or coll_regulafalsi based on the number of green marbles.
 */
ullong coll_coll(coll_t* c);

/*
 *  Description: Samples from the collision distribution by using bisection.
 */
ullong coll_bisec(coll_t* c);

/*
 *  Description: Samples from the collision distribution by using a regula falsi then bisection.
 */
ullong coll_regulafalsi(coll_t* c);

/*
 *  Description: Defines a new collision distribution with n marbles where r are red.
 */
void coll_setnr(coll_t* c, ullong n, ullong r);

/*
 *  Description: Updates the total number of marbles in the urn where the amount of red marbles
 *               stays the same.
 */
void coll_setn(coll_t* c, ullong n);

/*
 *  Description: Updates the number of red marbles in the urn.
 */
void coll_setr(coll_t* c, ullong r);

#endif
