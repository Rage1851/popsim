/*
 *       Filename: popsimio.c
 *    Description: Command line UI to popsim.
 *         Author: Niklas Mamtschur
 *   Organization: Technical University of Munich (TUM)
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#include "popsim.h"
#include "ran.h"
#include "arrurn.h"
#include "linurn.h"
#include "bsturn.h"
#include "aliurn.h"
#include "intpmap.h"

typedef unsigned long long ullong;
void popsimio_printhelp(char* prog_name);

void print_ullong_arr(ullong* arr, ullong nel) {
    for(ullong i = 0; i < nel-1; ++i)
        printf("%llu ", arr[i]);
    printf("%llu\n", arr[nel-1]);
}

// Simulation variables
enum alg_t {ARRAY,LINEAR,BST,ALIAS,BATCH,MBATCH} alg;
ullong nsteps   = 1;
int    verbose  = 0;
int    hmap     = 0;
ullong nsnap    = 1;
ullong nthreads = 1;

// Protocol variables
ullong nstates  = 1;
ullong ndist    = 1;
ullong ntrans   = 0;

// Urns
arrurn_t** arrurn;
linurn_t** linurn;
bsturn_t** bsturn;
aliurn_t** aliurn;

// Global version of the lookup
ullong*    larrfst = NULL;
ullong*    larrscd = NULL;
intpmap_t* lmap    = NULL;

void (*delta)(ullong, ullong, ullong*, ullong*) = NULL;

static inline void alookup(ullong kfst, ullong kscd, ullong* vfst, ullong *vscd) {
    *vfst = larrfst[kfst*nstates+kscd]; 
    *vscd = larrscd[kfst*nstates+kscd]; 
}

static inline void hlookup(ullong kfst, ullong kscd, ullong* vfst, ullong* vscd) {
    intpmap_lookup(lmap, kfst, kscd, vfst, vscd);
    if(*vfst == ULLONG_MAX) {
        *vfst = kfst;
        *vscd = kscd;
    }
}

// Threads and output
typedef struct siminfo_t {
    ullong id;
    ullong seed1, seed2, seed3;
} siminfo_t;

pthread_t* threads;
siminfo_t* siminfo;
ullong**   conf;

void* pthread_sim(void* data) {
    siminfo_t* i = (siminfo_t*) data;
    switch(alg) {
        case ARRAY:  popsim_seqarr(arrurn[i->id], nsteps, nstates, nsnap, conf[i->id], delta); break;
        case LINEAR: popsim_seqlin(linurn[i->id], nsteps, nstates, nsnap, conf[i->id], delta); break;
        case BST:    popsim_seqbst(bsturn[i->id], nsteps, nstates, nsnap, conf[i->id], delta); break;
        case ALIAS:  popsim_seqali(aliurn[i->id], nsteps, nstates, nsnap, conf[i->id], delta); break;
        case BATCH:
            if(popsim_batch(linurn[i->id], nsteps, nstates, nsnap, conf[i->id],
                        delta, i->seed1, i->seed2, i->seed3) == 0) {
                fprintf(stderr, "Not enough memory to run the batched simulator.\n");
                abort();
            }
            break;
        case MBATCH:
            if(popsim_mbatch(bsturn[i->id], nsteps, nstates, nsnap, conf[i->id],
                        delta, i->seed1, i->seed2, i->seed3) == 0) {
                fprintf(stderr, "Not enough memory to run the multi batched simulator.\n");
                abort();
            }
            break;
        default: abort();
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    // Read command line options
    char c;
    int flag;
    while((flag = getopt(argc, argv, "hvd:s:t:")) >= 0) {
        switch(flag) {
            case 'h':
                popsimio_printhelp(argv[0]);
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 'd':
                if(strcmp(optarg, "array") == 0) {
                    hmap = 0;
                } else if(strcmp(optarg, "map") == 0) {
                    hmap = 1;
                } else {
                    fprintf(stderr, "Option -%c requires delta to be either \"array\" or \"map\".",
                            optopt);
                    return -1;
                }
                break;
            case 's':
                if((nsnap = strtoull(optarg, NULL, 10)) == 0 || errno != 0 || nsnap == ULLONG_MAX) {
                    fprintf(stderr, "Option -%c requires nsnap as an integer argument in "
                           "[1,nsteps].\n", optopt);
                    return -1;
                }
                break;
            case 't':
                if((nthreads = strtoull(optarg, NULL, 10)) == 0 || errno != 0 ||
                        nthreads == ULLONG_MAX) {
                    fprintf(stderr, "Option -%c requires nthreads as an integer argument in "
                           "[1,2^64-1).\n", optopt);
                    return -1;
                }
                break;
            case '?':
                if(optopt == 'd')
                    fprintf(stderr, "Option -%c requires delta to be either \"array\" or \"map\".",
                            optopt);
                else if(optopt == 's')
                    fprintf(stderr, "Option -%c requires nsnap as an integer argument in "
                           "[1,nsteps].\n", optopt);
                else if(optopt == 't')
                    fprintf(stderr, "Option -%c requires nthreads as an integer argument in "
                           "[1,2^64-1).\n", optopt);
                else if(isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return -1;
            default:
                abort();
        }
    }

    // Read alg and check for the correct number of arguments
    if(optind+2 != argc) {
        fprintf(stderr, "Too many or too few command line arguments.\n");
        return -1;
    }
    if(     strcmp(argv[optind], "array")  == 0) alg = ARRAY;
    else if(strcmp(argv[optind], "linear") == 0) alg = LINEAR;
    else if(strcmp(argv[optind], "bst")    == 0) alg = BST;
    else if(strcmp(argv[optind], "alias")  == 0) alg = ALIAS;
    else if(strcmp(argv[optind], "batch")  == 0) alg = BATCH;
    else if(strcmp(argv[optind], "mbatch") == 0) alg = MBATCH;
    else {
        fprintf(stderr, "The specified algorithm must be either \"array\", \"linear\", \"bst\", "
                "\"alias\",\"batch\" or \"mbatch\".\n");
        return -1;
    }
    if((nsteps = strtoull(argv[optind+1], NULL, 10)) == 0 || errno != 0 || nsteps == ULLONG_MAX) {
        fprintf(stderr, "The number of steps needs to be an integer in [1,2^64-1).\n");
        return -1;
    }
    if(nsnap > nsteps) {
        fprintf(stderr, "The number of snapshots must be smaller or equal than the number of "
                        "steps.\n");
        return -1;
    }

    // Read number of steps, states, configurations, and transitions
    if(verbose) {
        printf("Enter the number of states as well as the number of configurations and "
               "transitions given as a space separated list ended by a newline:\n");
    }
    if(scanf("%llu", &nstates) != 1 || errno != 0 || nstates == 0 ||
            (!hmap && nstates >= sqrt(ULLONG_MAX)) || nstates >= ULLONG_MAX/(nsnap+1)) {
        fprintf(stderr, "The number of states needs to be an integer in [1,(2^64-1)/(nsnap+1)] if "
                        "delta is \"map\" or in [1,min(sqrt(2^64-1),(2^64-1)/(nsnap+1))] if delta "
                        "is \"array\" or it was entered invalidly.\n");
        return -1;
    }
    if(scanf(" %llu", &ndist) != 1 || errno != 0 || ndist == 0 || ndist == ULLONG_MAX) {
        fprintf(stderr, "The number of configurations specified needs to be an integer in "
                        "[1,2^64-1) or was entered invalidly.\n");
        return -1;
    }
    c = 0;
    if(scanf(" %llu%c", &ntrans, &c) != 2 || errno != 0 || ntrans == ULLONG_MAX || c != '\n') {
        fprintf(stderr, "The number of transitions specified needs to be an integer in [0,2^64-1) "
                        "or was entered invalidly.\n");
        return -1;
    }

    // Read initial configuration and initialize the urn data structure
    if(verbose)
        printf("Enter the initial state configuration as a space separated list of state-number of "
               "agents pairs separated by a colon ended by a newline:\n");

    ullong nagents = 0;
    ullong s, q;
    ullong* dist = (ullong*) calloc(nstates, sizeof(ullong));
    if(dist == NULL) {
        fprintf(stderr, "Not enough memory for the state configuration.\n");
        return -1;
    }

    for(ullong i = 0; i < ndist; ++i) {
        c = 0;
        if(scanf("%llu:%llu%c", &s, &q, &c) != 3 || errno != 0 || s == 0 || s > nstates ||
                q == ULLONG_MAX || (i < ndist-1 && c != ' ') || (i == ndist-1 && c != '\n')) {
            fprintf(stderr, "The configuration entered was invalid.\n");
            return -1;
        }
        if(q >= ULLONG_MAX-nagents) {
            fprintf(stderr, "The total number of agents is too large.\n");
            return -1;
        }

        nagents += q;
        // Change state because io mapping does not correspond with the implementation mapping
        dist[--s] += q;
    }

    if(nagents < 2) {
        fprintf(stderr, "The total number of agents needs to be larger than 1.\n");
        return -1;
    }

    sran(time(NULL));
    switch(alg) {
        case ARRAY:
            arrurn = (arrurn_t**) malloc(nthreads * sizeof(arrurn_t*));
            if((arrurn[0] = arrurn_create(ran(), nstates, nagents)) == NULL) {
                fprintf(stderr, "Not enough memory for the urn data structure.\n");
                return -1;
            }

            arrurn_insert(arrurn[0], dist);
            for(ullong i = 1; i < nthreads; ++i) {
                if((arrurn[i] = arrurn_copy(arrurn[0], ran())) == NULL) {
                    fprintf(stderr, "Not enough memory for the urn data structure.\n");
                    return -1;
                }
            }
            break;
        case LINEAR:
            linurn = (linurn_t**) malloc(nthreads * sizeof(linurn_t*));
            if((linurn[0] = linurn_create(ran(), nstates)) == NULL) {
                fprintf(stderr, "Not enough memory for the urn data structure.\n");
                return -1;
            }

            linurn_insert(linurn[0], dist);
            for(ullong i = 1; i < nthreads; ++i) {
                if((linurn[i] = linurn_copy(linurn[0], ran())) == NULL) {
                    fprintf(stderr, "Not enough memory for the urn data structure.\n");
                    return -1;
                }
            }
            break;
        case BST:
            bsturn = (bsturn_t**) malloc(nthreads * sizeof(bsturn_t*));
            if((bsturn[0] = bsturn_create(ran(), nstates)) == NULL) {
                fprintf(stderr, "Not enough memory for the urn data structure.\n");
                return -1;
            }

            bsturn_insert(bsturn[0], dist);
            for(ullong i = 1; i < nthreads; ++i) {
                if((bsturn[i] = bsturn_copy(bsturn[0], ran())) == NULL) {
                    fprintf(stderr, "Not enough memory for the urn data structure.\n");
                    return -1;
                }
            }
            break;
        case ALIAS:
            aliurn = (aliurn_t**) malloc(nthreads * sizeof(aliurn_t*));
            if((aliurn[0] = aliurn_create(ran(), nstates, 0.8L, 1.5L)) == NULL) {
                fprintf(stderr, "Not enough memory for the urn data structure.\n");
                return -1;
            }

            aliurn_insert(aliurn[0], dist);
            for(ullong i = 1; i < nthreads; ++i) {
                if((aliurn[i] = aliurn_copy(aliurn[0], ran())) == NULL) {
                    fprintf(stderr, "Not enough memory for the urn data structure.\n");
                    return -1;
                }
            }
            break;
        case BATCH:
            linurn = (linurn_t**) malloc(nthreads * sizeof(linurn_t*));
            if((linurn[0] = linurn_create(ran(), nstates)) == NULL) {
                fprintf(stderr, "Not enough memory for the urn data structure.\n");
                return -1;
            }

            linurn_insert(linurn[0], dist);
            for(ullong i = 1; i < nthreads; ++i) {
                if((linurn[i] = linurn_copy(linurn[0], ran())) == NULL) {
                    fprintf(stderr, "Not enough memory for the urn data structure.\n");
                    return -1;
                }
            }
            break;
        case MBATCH:
            bsturn = (bsturn_t**) malloc(nthreads * sizeof(bsturn_t*));
            if((bsturn[0] = bsturn_create(ran(), nstates)) == NULL) {
                fprintf(stderr, "Not enough memory for the urn data structure.\n");
                return -1;
            }

            bsturn_insert(bsturn[0], dist);
            for(ullong i = 1; i < nthreads; ++i) {
                if((bsturn[i] = bsturn_copy(bsturn[0], ran())) == NULL) {
                    fprintf(stderr, "Not enough memory for the urn data structure.\n");
                    return -1;
                }
            }
            break;
        default:
            abort();
    }
    free(dist);

    // Read transitions
    if(verbose)
        printf("Enter the transitions as a newline separated list of two space separated state "
               "pairs in turn separated by a colon:\n");

    if(hmap) {
        delta = hlookup;
        if((lmap = intpmap_create(ntrans, nagents-1)) == NULL) {
            fprintf(stderr, "Not enough memory for the transition map.\n");
            return -1;
        }
    } else {
        delta = alookup;
        if((larrfst = (ullong*) malloc(nstates*nstates * sizeof(ullong))) == NULL) {
            fprintf(stderr, "Not enough memory for the transition array.\n");
            return -1;
        }
        if((larrscd = (ullong*) malloc(nstates*nstates * sizeof(ullong))) == NULL) {
            fprintf(stderr, "Not enough memory for the transition array.\n");
            return -1;
        }

        for(ullong i = 0; i < nstates; ++i) {
            for(ullong j = 0; j < nstates; ++j) {
                larrfst[i*nstates+j] = i;
                larrscd[i*nstates+j] = j;
            }
        }
    }

    ullong kfst, kscd, vfst, vscd;
    ullong hfst, hscd;
    for(ullong i = 0; i < ntrans; ++i) {
        c = 0;
        if(scanf("%llu:%llu %llu:%llu%c", &kfst, &kscd, &vfst, &vscd, &c) != 5 || c != '\n' ||
                errno != 0 || vfst == 0 || vscd == 0 || kfst == 0 || kscd == 0 ||
                kfst > nstates || kscd > nstates || vfst > nstates || vscd > nstates) {
            fprintf(stderr, "Transitions must be given such that s_ij are in [1,nstates] or were "
                            "entered invalidly.\n");
            return -1;
        }
        
        kfst--; kscd--; vfst--; vscd--;
        if(hmap) {
            intpmap_lookup(lmap, kfst, kscd, &hfst, &hscd);
            if(hfst == ULLONG_MAX)
                intpmap_insert(lmap, kfst, kscd, vfst, vscd);
        } else {
            larrfst[kfst*nstates+kscd] = vfst;
            larrscd[kfst*nstates+kscd] = vscd;
        }
    }

    // Allocated space for the configuration snapshots
    conf = (ullong**) malloc(nthreads * sizeof(ullong*));
    if(conf == NULL) {
        fprintf(stderr, "Not enough memory for the configuration snapshots.\n");
        return -1;
    }
    for(ullong i = 0; i < nthreads; ++i) {
        if((conf[i] = (ullong*) calloc((nsnap+1)*nstates, sizeof(ullong))) == NULL) {
            fprintf(stderr, "Not enough memory for the configuration snapshots.\n");
            return -1;
        }
    }

    // Simulation
    if(nthreads > 1) {
        if((threads = (pthread_t*) malloc(nthreads * sizeof(pthread_t))) == NULL) {
            fprintf(stderr, "Not enough memory for the thread states.\n");
            return -1;
        }

        if((siminfo = (siminfo_t*) malloc(nthreads * sizeof(siminfo_t))) == NULL) {
            fprintf(stderr, "Not enough memory for the thread ids.\n");
            return -1;
        }

        for(ullong i = 0; i < nthreads; ++i) {
            siminfo[i].id = i;
            siminfo[i].seed1 = ran(); siminfo[i].seed2 = ran(); siminfo[i].seed3 = ran();
            pthread_create(threads+i, NULL, pthread_sim, (void*) (siminfo+i));
        }
        for(ullong i = 0; i < nthreads; ++i) {
            if(pthread_join(threads[i], NULL) != 0) {
                fprintf(stderr, "Threads could not be joined.\n");
                return -1;
            }
        }
        free(siminfo);
        free(threads);
    } else {
        siminfo_t info;
        info.id = 0;
        info.seed1 = ran(); info.seed2 = ran(); info.seed3 = ran();
        pthread_sim((void*) &info);
    }

    // Print results
    if(nthreads > 1) {
        for(ullong i = 0; i < nthreads; ++i) {
            if(verbose)
                printf("Execution snapshots of thread %llu:\n", i+1);
            for(ullong j = 0; j < (nsnap+1); ++j)
                print_ullong_arr(conf[i] + j*nstates, nstates);
            if(i < nthreads-1)
                printf("\n");
        }
    } else {
        if(verbose)
            printf("Execution snapshots:\n");
        for(ullong i = 0; i < (nsnap+1); ++i)
            print_ullong_arr(conf[0] + i*nstates, nstates);
    }

    // Clean-Up
    for(ullong i = 0; i < nthreads; ++i) {
        free(conf[i]);
        switch(alg) {
            case ARRAY:  arrurn_destroy(arrurn[i]); break;
            case LINEAR: linurn_destroy(linurn[i]); break;
            case BST:    bsturn_destroy(bsturn[i]); break;
            case ALIAS:  aliurn_destroy(aliurn[i]); break;
            case BATCH:  linurn_destroy(linurn[i]); break;
            case MBATCH: bsturn_destroy(bsturn[i]); break;
            default: abort();
        }
    }
    free(conf);
    switch(alg) {
        case ARRAY:  free(arrurn); break;
        case LINEAR: free(linurn); break;
        case BST:    free(bsturn); break;
        case ALIAS:  free(aliurn); break;
        case BATCH:  free(linurn); break;
        case MBATCH: free(bsturn); break;
        default: abort();
    }
    if(hmap) {
        intpmap_destroy(lmap);
    } else {
        free(larrfst);
        free(larrscd);
    }
    return 0;
}

void popsimio_printhelp(char* prog_name) {
    printf("A program for the simulation of population protocols using the algorithms described\n"
           "in Berenbrink et al. which prints the configuration snapshots as a newline separated\n"
           "list to stdout. The configuration snapshots itself are given as a space separated list\n"
           "of integers where the position in the list corresponds to the state.\n\n"
           "Usage: %s [-h] [-v] [-d delta] [-s nsnap] [-t nthreads] sim nsteps\n\n"
           "Arguments:\n"
           "  sim         Specifies the simulator used where sim is in\n"
           "              {\"array\",\"linear\",\"bst\",\"alias\",\"batch\",\"mbatch}.\n"
           "  nsteps      Amount of interaction steps that should be simulated where nsteps in\n"
           "              [1,2^64-1).\n"
           "  -h          Print this usage statement and do not run the program.\n"
           "  -v          Prompt for input and print results with messages.\n"
           "  -d delta    Specifies how the transition function is realized where delta must be\n"
           "              in {\"array\",\"map\"} where \"array\" is the default and \"array\"\n"
           "              corresponds to a two dimensional array and \"map\" to a hash map.\n"
           "  -s nsnap    Specifies that nsnap configuration snapshots should be taken which\n"
           "              excludes the initial and includes the final configuration where nsnap\n"
           "              must be in [1,nsteps] and 1 is the default. The snapshots will be taken\n"
           "              after nsteps/nsnap floored interactions and after the simulation has\n"
           "              finished. If sim is \"batch\" or \"mbatch\" and nsteps/nsnap floored is\n"
           "              smaller than a batched step, then this snapshot will be filled by the\n"
           "              previous one.\n"
           "  -t nthreads Simulate the population protocol nthreads times on nthreads many threads\n"
           "              where nthreads needs to be in [1,2^64-1) and 1 is the default. The\n"
           "              outputs are given as a newline seperated list for multiple threads.\n\n"
           "The program then expects several non-negative integers from stdin:\n"
           "  nstates     Number of states where nstates must be in [1,(2^64-1)/(nsnap+1) if delta\n"
           "              is \"map\" or in [1,min(sqrt(2^64-1),(2^64-1)/(nsnap+1)) if delta is\n"
           "              \"array\". Each state is represented as an integer in [1,nstates].\n"
           "  ndist       Number of initial state distributions given where ndist [1,2^64-1). If\n"
           "              a state configuration is not given, then it is assumed to be zero and if\n"
           "              multiple configurations are given for the same state, then they will be\n"
           "              summed up.\n"
           "  ntrans      Number of transitions given, where ntrans is an integer in [0,2^64-1).\n"
           "              If a transiton is not given for a pair of states, then the identity\n"
           "              mapping is used. If multiple transitions are given for the same pair of\n"
           "              states, then only the first transition is considered for delta equal to\n"
           "              \"map\" and only the last transition for delta equal to \"array\".\n"
           "  s_i a_i     Initial configuration of state s_i with a_i agents, where i is an\n"
           "              integer in [1,ndist] and s_i is in [1,nstates]. The total number of\n"
           "              agents must be in [2,2^64-1).\n"
           "  s_ij        Transition mapping of (s_i1,s_i2) -> (s_i3,s_i4), where s_ij must be in\n"
           "              [1,nstates] and i and j are integers in [1,ntrans] and {1,2,3,4},\n"
           "              respectively.\n\n"
           "These parameters need to be given in exactly the following format:\n"
           "nstates ndist ntrans\n"
           "s_1:a_1 s_2:a_2 ... s_ndist:a_ndist\n"
           "s_11:s_12 s_13:s_14\n"
           "s_21:s_22 s_23:s_24\n"
           "...\n"
           "s_ntrans1:s_ntrans2 s_ntrans3:s_ntrans4\n", prog_name);
}
