/*
* main.c
* This file will be the micro-benchmark for the Split-Ordered-Lists datastructure
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <pthread.h>    // pthread_create



//******************************************************************************
// local includes
//******************************************************************************

#include "benchmark/splay-tree/splay-tree-benchmark.h"
#include "benchmark/Split-Ordered-Lists/Split-Ordered-Lists-benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    //splay_tree_benchmark();
    split_ordered_list_benchmark();
}
