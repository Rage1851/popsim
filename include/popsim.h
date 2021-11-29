/*
 *      Filename: popsim.h
 *  Descriptions: Header file for the simulation of population protocols as described by Berenbrink
 *                et al.
 *        Author: Niklas Mamtschur
 *  Organization: Technical University of Munich (TUM)
 */

#ifndef POPSIM_H
#define POPSIM_H

#include "arrurn.h"
#include "linurn.h"
#include "bsturn.h"
#include "aliurn.h"

typedef unsigned long long ullong;
typedef long double        ldouble;

/*
 *  Description: Sequential simulation where each step is simulated one after the other.
 *   Parameters: 
 *             - u is a urn holding the initial state configuration with a total of nstates states
 *               and atleast two agents.
 *             - nsteps is the number of steps which need to be simulated.
 *             - nconf is the number of configuration snapshots to be taken excluding the initial
 *               and including the final configuration which are all stored in conf. The snapshots
 *               are taken at equidistant interaction steps of size nsteps/nconf floored.
 *             - conf needs to be allocated as a two dimensional array with (nconf+1) as the size of
 *               the first dimension and nstates as the size of the second one. It will be filled
 *               with the initial and final configuration as well as (nconf-1) steps in between.
 *  Assumptions:
 *             - 1 <= min(nstates,nsteps)
 *             - 1 <= nconf <= nsteps
 *             - max(nstates, nsteps, nconf) < ULLONG_MAX
 */
void popsim_seqarr(arrurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                  void (*delta)(ullong, ullong, ullong*, ullong*));
void popsim_seqlin(linurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                  void (*delta)(ullong, ullong, ullong*, ullong*));
void popsim_seqbst(bsturn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                  void (*delta)(ullong, ullong, ullong*, ullong*));
void popsim_seqali(aliurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                  void (*delta)(ullong, ullong, ullong*, ullong*));

/*
 *   Description: Batched simulation where multiple steps are simulated at once.
 *    Parameters: The configuration snapshots will be taken once the interaction steps are larger or
 *                equal than the equidistant steps. If the equidistant steps are smaller, then they
 *                will be filled up by the previous snapshot. Additionally, these functions require
 *                three random number generator seeds. For the rest, see sequential simulators.
 *   Assumptions: See sequential simulators.
 *  Return value: Non-zero if everything went alright and zero otherwise.
 *        Errors: ENOMEM if there was not enough memory for the helper data structures.
 */
int popsim_batch (linurn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                  void (*delta)(ullong, ullong, ullong*, ullong*),
                  ullong seed1, ullong seed2, ullong seed3);
int popsim_mbatch(bsturn_t* u, ullong nsteps, ullong nstates, ullong nconf, ullong* conf,
                  void (*delta)(ullong, ullong, ullong*, ullong*),
                  ullong seed1, ullong seed2, ullong seed3);

#endif
