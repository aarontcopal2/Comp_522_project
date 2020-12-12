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
#include "address-splay-tree.h"
#include "benchmark.h"



//******************************************************************************
// local data
//******************************************************************************

// ITERATION_SIZE is a multiple of 2 since the thread counts are a factor of 2
#define ITERATION_SIZE 1024
#define LOWER 1500



//******************************************************************************
// private operations
//******************************************************************************

static void initialize_splay_tree() {
    // initializing the splay tree with 1k elements
    for (int i = 0; i < LOWER; i++) {
        uint64_t key = i;
        address *val = (void*)&addrs[key];
        address_splay_insert(key, val);
    }
}


static uint64_t random_key() {
    return (rand() % 
           (ADDRESS_SIZE - LOWER)) + LOWER;
}


static void *benchmark1_operations(void *arg) {
    // 50% inserts, 0% finds, 50% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 2;
    
    uint64_t key;
    address *val, *result_val;

    for (int i = 0; i < iterations; i++) {
        key = random_key();
        val = (void*)&addrs[key];

        address_splay_insert(key, val);
        address_splay_delete(key);
    }
}


static void benchmark1(int thread_count) {
    // 50% inserts, 0% finds, 50% deletes
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

	// clock start
    clock_t start = clock();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark1_operations, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

	// clock end
    clock_t end = clock();
    // print time
    double benchmark1_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("benchmark1: threads: %d, time: %f\n", thread_count, benchmark1_time);
}


static void *benchmark2_operations(void *arg) {
    // 33% inserts, 33% finds, 33% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 3;
    
    uint64_t key;
    address *val, *result_val;

    for (int i = 0; i < iterations; i++) {
        key = random_key();
        val = (void*)&addrs[key];

        address_splay_insert(key, val);
        result_val = (address*) address_splay_entry_val_get(key);
        address_splay_delete(key);
    }
}


static void benchmark2(int thread_count) {
    // 33% inserts, 33% finds, 33% deletes
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

	// clock start
    clock_t start = clock();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark2_operations, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

	// clock end
    clock_t end = clock();
    // print time
    double benchmark2_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("benchmark2: threads: %d, time: %f\n", thread_count, benchmark2_time);
}


static void *benchmark3_operations(void *arg) {
    // 25% inserts, 50% finds, 25% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 4;
    
    uint64_t key1, key2;
    address *val, *result_val1, *result_val2;

    for (int i = 0; i < iterations; i++) {
        key1 = random_key();
        key2 = random_key();
        val = (void*)&addrs[key1];

        address_splay_insert(key1, val);
        result_val1 = (address*) address_splay_entry_val_get(key1);
        result_val2 = (address*) address_splay_entry_val_get(key2);
        address_splay_delete(key1);
    }
}


static void benchmark3(int thread_count) {
    // 25% inserts, 50% finds, 25% deletes
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

	// clock start
    clock_t start = clock();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark3_operations, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

	// clock end
    clock_t end = clock();
    // print time
    double benchmark3_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("benchmark3: threads: %d, time: %f\n", thread_count, benchmark3_time);
}


static void *benchmark4_operations(void *arg) {
    // 15% inserts, 70% finds, 15% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 7;
    
    uint64_t key1, key2, key3, key4, key5;
    address *val, *result_val1, *result_val2, *result_val3, *result_val4, *result_val5;

    for (int i = 0; i < iterations; i++) {
        key1 = random_key();
        key2 = random_key();
        key3 = random_key();
        key4 = random_key();
        key5 = random_key();
        val = (void*)&addrs[key1];

        address_splay_insert(key1, val);
        result_val1 = (address*) address_splay_entry_val_get(key1);
        result_val2 = (address*) address_splay_entry_val_get(key2);
        result_val3 = (address*) address_splay_entry_val_get(key3);
        result_val4 = (address*) address_splay_entry_val_get(key4);
        result_val5 = (address*) address_splay_entry_val_get(key5);
        address_splay_delete(key1);
    }
}


static void benchmark4(int thread_count) {
    // 15% inserts, 70% finds, 15% deletes
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

	// clock start
    clock_t start = clock();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark4_operations, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

	// clock end
    clock_t end = clock();
    // print time
    double benchmark4_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("benchmark4: threads: %d, time: %f\n", thread_count, benchmark4_time);
}


static void *benchmark5_operations(void *arg) {
    // 5% inserts, 90% finds, 5% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 20;
    
    uint64_t key;
    address *val, *result_val;

    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 17; j++) {
            key = random_key();
            result_val = (address*) address_splay_entry_val_get(key);
        }
        key = random_key();
        val = (void*)&addrs[key];
        address_splay_insert(key, val);
        result_val = (address*) address_splay_entry_val_get(key);
        address_splay_delete(key);
    }
}


static void benchmark5(int thread_count) {
    // 5% inserts, 90% finds, 5% deletes
    int iterations = ITERATION_SIZE / thread_count;
    pthread_t thr[thread_count];

	// clock start
    clock_t start = clock();

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark5_operations, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }

	// clock end
    clock_t end = clock();
    // print time
    double benchmark5_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("benchmark5: threads: %d, time: %f\n", thread_count, benchmark5_time);
}



//******************************************************************************
// interface operations
//******************************************************************************

void splay_tree_benchmark () {
    printf("Benchmark 1: 50%% inserts, 0%% finds, 50%% deletes\n====================================================\n");
    for (int t=1; t <=32; t*=2) {
        initialize_splay_tree();
        benchmark1(t);
        clear_splay_tree();
    }
    printf("\nBenchmark 2: 33%% inserts, 33%% finds, 33%% deletes\n====================================================\n");
    for (int t=1; t <=32; t*=2) {
        initialize_splay_tree();
        benchmark2(t);
        clear_splay_tree();
    }
    printf("\nBenchmark 3: 25%% inserts, 50%% finds, 25%% deletes\n====================================================\n");
    for (int t=1; t <=32; t*=2) {
        initialize_splay_tree();
        benchmark3(t);
        clear_splay_tree();
    }
    printf("\nBenchmark 4: 15%% inserts, 70%% finds, 15%% deletes\n====================================================\n");
    for (int t=1; t <=32; t*=2) {
        initialize_splay_tree();
        benchmark4(t);
        clear_splay_tree();
    }
    printf("\nBenchmark 5: 5%% inserts, 90%% finds, 5%% deletes\n====================================================\n");
    for (int t=1; t <=32; t*=2) {
        initialize_splay_tree();
        benchmark5(t);
        clear_splay_tree();
    }
}
