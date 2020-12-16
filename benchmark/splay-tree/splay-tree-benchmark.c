//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // malloc
#include <stdio.h>      // printf
#include <inttypes.h>   // PRIu64



//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/splay-tree/splay-uint64.h"
#include "test-splay-tree.h"
#include "benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// private operations
//******************************************************************************

static void initialize_splay_tree() {
    // initializing the splay tree with 50k elements
    uint64_t key, val;
    for (int i = 0; i < LOWER; i++) {
        key = i;
        val = key >> 1;
        test_splay_insert(key, val);
    }
}


static uint64_t random_search_key() {
    return rand() % UPPER;
}


static uint64_t random_insert_key() {
    return (rand() % 
           (UPPER - LOWER)) + LOWER;
}


static uint64_t random_delete_key() {
    return rand() % LOWER;
}


static void *benchmark1_operations(void *arg) {
    // 50% inserts, 0% finds, 50% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 2;
    
    uint64_t key_i, key_d, val1;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        val1 = key_i >> 1;
        test_splay_insert(key_i, val1);
        key_d = random_delete_key();
        test_splay_delete(key_d);
    }
}


static void *benchmark2_operations(void *arg) {
    // 33% inserts, 33% finds, 33% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 3;
    
    uint64_t key_i, key_s, key_d, val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        val = key_i >> 1;
        test_splay_insert(key_i, val);
        key_s = random_search_key();
        result_val = test_splay_entry_val_get(key_s);
        key_d = random_delete_key();        
        test_splay_delete(key_d);
    }
}


static void *benchmark3_operations(void *arg) {
    // 25% inserts, 50% finds, 25% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 4;
    
    uint64_t key_i, key_s, key_d, val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        val = key_i >> 1;
        test_splay_insert(key_i, val);
        for (int j = 0; j < 2; j++) {
            key_s = random_search_key();
            result_val = test_splay_entry_val_get(key_s);
        }
        key_d = random_delete_key();
        test_splay_delete(key_d);
    }
}


static void *benchmark4_operations(void *arg) {
    // 15% inserts, 70% finds, 15% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 7;
    
    uint64_t key_i, key_s, key_d, val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        val = key_i >> 1;
        test_splay_insert(key_i, val);
        for (int j = 0; j < 5; j++) {
            key_s = random_search_key();
            result_val = test_splay_entry_val_get(key_s);
        }
        key_d = random_delete_key();
        test_splay_delete(key_d);
    }
}


static void *benchmark5_operations(void *arg) {
    // 5% inserts, 90% finds, 5% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 20;
    
    uint64_t key_i, key_s, key_d, val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        val = key_i >> 1;
        test_splay_insert(key_i, val);
        for (int j = 0; j < 18; j++) {
            key_s = random_search_key();
            result_val = test_splay_entry_val_get(key_s);
        }
        key_d = random_delete_key();
        test_splay_delete(key_d);
    }
}


static void benchmark(int benchmark_no, int thread_count) {
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

    void *(*benchmark_fn_ptr)(void*);
    switch (benchmark_no) {
    case 1:
        benchmark_fn_ptr = benchmark1_operations;
        break;
    case 2:
        benchmark_fn_ptr = benchmark2_operations;
        break;
    case 3:
        benchmark_fn_ptr = benchmark3_operations;
        break;
    case 4:
        benchmark_fn_ptr = benchmark4_operations;
        break;
    case 5:
        benchmark_fn_ptr = benchmark5_operations;
        break;
    default:
        printf("benchmark not found\n");
        break;
    }

	// clock start
    struct timespec start = timer_start();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark_fn_ptr, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

    // print time
    double benchmark_time = timer_elapsed(start);
    if (!DEBUG) {
        if (thread_count != LAST_THREAD) {
            printf("%f, ", benchmark_time);
        } else {
            printf("%f", benchmark_time);
        }
    } else {
        debug_print("threads: %d, time: %f\n", thread_count, benchmark_time);
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

void splay_tree_benchmark () {
    // seeding so that different random sequences get generated
    srand (time(NULL));

    debug_print("Benchmark 1: 50%% inserts, 0%% finds, 50%% deletes\n====================================================\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(1, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 2: 33%% inserts, 33%% finds, 33%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(2, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 3: 25%% inserts, 50%% finds, 25%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(3, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 4: 15%% inserts, 70%% finds, 15%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(4, t);
        clear_test_splay_tree();
    }
    debug_print("\nBenchmark 5: 5%% inserts, 90%% finds, 5%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        initialize_splay_tree();
        benchmark(5, t);
        clear_test_splay_tree();
    }
}
