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

static uint64_t random_key() {
    return (rand() % 
           (UPPER - LOWER)) + LOWER;
}


static void *benchmark1_operations(void *arg) {
    // 50% inserts, 0% finds, 50% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 2;
    
    t_key key, temp_val;
    val_t val;

    for (int i = 0; i < iterations; i++) {
        key = random_key();
        temp_val = key >> 1;
        val = (void*)&temp_val;

        status = map_insert(htab, key, val);
        status = map_delete(htab, key);
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
    double benchmark_time = ((double) (end - start)) / CLOCKS_PER_SEC;
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


static void *benchmark2_operations(void *arg) {
    // 33% inserts, 33% finds, 33% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 3;
    
    t_key key, temp_val;
    val_t val, result_val;

    for (int i = 0; i < iterations; i++) {
        key = random_key();
        temp_val = key >> 1;
        val = (void*)&temp_val;

        status = map_insert(htab, key, val);
        result_val = map_search(htab, key);
        status = map_delete(htab, key);
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
    double benchmark_time = ((double) (end - start)) / CLOCKS_PER_SEC;
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


static void *benchmark3_operations(void *arg) {
    // 25% inserts, 50% finds, 25% deletes
    bool status;
    int iterations = *(int*)arg;
    iterations /= 4;
    
    t_key key1, key2, temp_val;
    val_t val, result_val1, result_val2;

    for (int i = 0; i < iterations; i++) {
        key1 = random_key();
        key2 = random_key();
        temp_val = key1 >> 1;
        val = (void*)&temp_val;

        status = map_insert(htab, key1, val);
        result_val1 = map_search(htab, key1);
        result_val2 = map_search(htab, key2);
        status = map_delete(htab, key1);
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
    double benchmark_time = ((double) (end - start)) / CLOCKS_PER_SEC;
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


static void *benchmark4_operations(void *arg) {
    // 15% inserts, 70% finds, 15% deletes
    bool status;
    int iterations = *(int*)arg;
    
    t_key key1, key2, key3, key4, key5, temp_val;
    val_t val, result_val1, result_val2, result_val3, result_val4, result_val5;

    for (int i = 0; i < iterations; i++) {
        key1 = random_key();
        key2 = random_key();
        key3 = random_key();
        key4 = random_key();
        key5 = random_key();
        temp_val = key1 >> 1;
        val = (void*)&temp_val;

        status = map_insert(htab, key1, val);
        result_val1 = map_search(htab, key1);
        result_val2 = map_search(htab, key2);
        result_val3 = map_search(htab, key3);
        result_val4 = map_search(htab, key4);
        result_val5 = map_search(htab, key5);
        status = map_delete(htab, key1);
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
    double benchmark_time = ((double) (end - start)) / CLOCKS_PER_SEC;
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


static void *benchmark5_operations(void *arg) {
    // 5% inserts, 90% finds, 5% deletes
    bool status;
    int iterations = *(int*)arg;
    
    t_key key, temp_val;
    val_t val, result_val;

    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 17; j++) {
            key = random_key();
            result_val = map_search(htab, key);
        }
        key = random_key();
        temp_val = key >> 1;
        val = (void*)&temp_val;

        status = map_insert(htab, key, val);
        result_val = map_search(htab, key);
        status = map_delete(htab, key);
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
    double benchmark_time = ((double) (end - start)) / CLOCKS_PER_SEC;
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
        benchmark1(t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 2: 33%% inserts, 33%% finds, 33%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        benchmark2(t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 3: 25%% inserts, 50%% finds, 25%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        benchmark3(t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 4: 15%% inserts, 70%% finds, 15%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        benchmark4(t);
        hashtable_destroy(htab);
    }
    debug_print("\nBenchmark 5: 5%% inserts, 90%% finds, 5%% deletes\n====================================================\n");
    printf("\n");
    for (int t=1; t <= LAST_THREAD; t*=2) {
        htab = hashtable_initialize();
        benchmark5(t);
        hashtable_destroy(htab);
    }
}
