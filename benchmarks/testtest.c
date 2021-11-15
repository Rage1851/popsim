/*
 * =====================================================================================
 *
 *       Filename:  testtest.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/04/2021 16:07:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <time.h>
#include "mt.h"

int main(int argc, char** argv) {
    mt_t mt;
    //array_urn_create(100, 100);
    mt_rand(&mt);
    mt_init(&mt, time(NULL));
    return 0;
}
