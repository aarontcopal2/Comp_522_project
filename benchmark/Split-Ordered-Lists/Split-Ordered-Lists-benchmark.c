//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h> // rand, srand



//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/Split-Ordered-Lists.h"
#include "benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// local data
//******************************************************************************

hashtable *htab;



//******************************************************************************
// private operations
//******************************************************************************

static void *add_initial_elements(void *arg) {
    // initializing the hash table with 50k elements
    bool status;
    t_key key, temp_val;
    val_t val;

    for (int i = 0; i < LOWER; i++) {
        key = i;
        temp_val = key >> 1;
        val = (void*)&temp_val;
        status = map_insert(htab, key, val);
    }
}

static void initial_hashtable_population() {
    pthread_t thr;
    pthread_create(&thr, NULL, add_initial_elements, NULL);
    pthread_join(thr, NULL);
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
    
    t_key key_i, key_d, temp_val;
    val_t val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = map_insert(htab, key_i, val);
        key_d = random_delete_key();
        status = map_delete(htab, key_d);
    }
}


static void *benchmark2_operations(void *arg) {
    // 33% inserts, 33% finds, 33% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 3;
    
    t_key key_i, key_s, key_d, temp_val;
    val_t val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = map_insert(htab, key_i, val);
        key_s = random_search_key();
        result_val = map_search(htab, key_s);
        key_d = random_delete_key();
        status = map_delete(htab, key_d);
    }
}


static void *benchmark3_operations(void *arg) {
    // 25% inserts, 50% finds, 25% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 4;
    
    t_key key_i, key_s, key_d, temp_val;
    val_t val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = map_insert(htab, key_i, val);
        for (int j = 0; j < 2; j++) {
            key_s = random_search_key();
            result_val = map_search(htab, key_s);
        }
        key_d = random_delete_key();
        status = map_delete(htab, key_d);
    }
}


static void *benchmark4_operations(void *arg) {
    // 15% inserts, 70% finds, 15% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 7;
    
    t_key key_i, key_s, key_d, temp_val;
    val_t val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = map_insert(htab, key_i, val);
        for (int j = 0; j < 5; j++) {
            key_s = random_search_key();
            result_val = map_search(htab, key_s);
        }
        key_d = random_delete_key();
        status = map_delete(htab, key_d);
    }
}


static void *benchmark5_operations(void *arg) {
    // 5% inserts, 90% finds, 5% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 20;
    
    t_key key_i, key_s, key_d, temp_val;
    val_t val, result_val;

    for (int i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = map_insert(htab, key_i, val);
        for (int j = 0; j < 18; j++) {
            key_s = random_search_key();
            result_val = map_search(htab, key_s);
        }
        key_d = random_delete_key();
        status = map_delete(htab, key_d);
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

void split_ordered_list_benchmark () {
    // seeding so that different random sequences get generated
    srand (time(NULL));

    debug_print("Benchmark 1: 50%% inserts, 0%% finds, 50%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        initial_hashtable_population();
        benchmark(1, t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 2: 33%% inserts, 33%% finds, 33%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        initial_hashtable_population();
        benchmark(2, t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 3: 25%% inserts, 50%% finds, 25%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        initial_hashtable_population();
        benchmark(3, t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 4: 15%% inserts, 70%% finds, 15%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        initial_hashtable_population();
        benchmark(4, t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 5: 5%% inserts, 90%% finds, 5%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        initial_hashtable_population();
        benchmark(5, t);
        hashtable_destroy(htab);
    }
}
