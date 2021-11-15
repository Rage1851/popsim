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
 *                be set before use.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef COLL_H
#define COLL_H

typedef unsigned long long ullong;

typedef struct coll_t coll_t;

/*
 *  Description: Seeds the underlying random number generator.
 */
ullong seed(coll_t* c, ullong seed);

/*
 *  Description: Samples from the collision distribution.
 */
ullong coll(coll_t* c);

/*
 *  Description: Defines a new collision distribution with n marbles where r are red.
 */
void coll_setnr(coll_t* c, ullong n, ullong r);

/*
 *  Description: Updates the total number of marbles in the urn where the amount of red marbles.
 */
void coll_setn(coll_t* c, ullong n);

/*
 *  Description: Updates the number of red marbles in the urn.
 */
void coll_setr(coll_t* c, ullong r);

#endif
