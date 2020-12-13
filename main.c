/*
* main.c
* This file will be the micro-benchmark for the Split-Ordered-Lists datastructure
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <pthread.h>    // pthread_create
#include <stdio.h>      // printf



//******************************************************************************
// local includes
//******************************************************************************

#include "benchmark/splay-tree/splay-tree-benchmark.h"
#include "benchmark/Split-Ordered-Lists/Split-Ordered-Lists-benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

#define DEBUG 1
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    /* debug_print("splay_tree_benchmark\n");
    splay_tree_benchmark(); */
    debug_print("\nsplit_ordered_list_benchmark\n");
    split_ordered_list_benchmark();
}
