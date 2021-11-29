/*
 *      Filename: hgeom.h
 *   Description: Sampling from the (multivariate) hypergeometric distribution. The code for mhgeom
 *                was written by Niklas Mamtschur whereas the rest of the code was taken from
 *                NumPy.
 *   Assumptions: The mt state needs to be initialized before being passed and the number of good
 *                marbles as well as the number of samples must be smaller than the total.
 *    Changed by: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef HGEOM_H
#define HGEOM_H

#include "mt.h"

typedef unsigned long long ullong;

/*
 *  Description: Samples from the hypergeometric distribution.
 *  Assumptions: max(good,sample) <= total and total >= 1
 */
ullong hgeom(mt_t* mt, ullong total, ullong good, ullong sample);

/*
 *  Description: Samples from the multivariate hypergeometric distribution.
 *  Assumptions: destdist and srcdist must hold ncolors elements where the index corresponds to
 *               a color. Additionally, sample <= total, total >= 1, ncolors >= 1, and all colors
 *               of srcdist must add up to total.
 */
void mhgeom(mt_t* mt, ullong* destdist, ullong* srcdist,
            ullong ncolors, ullong total, ullong sample);

#endif
