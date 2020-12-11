//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/Split-Ordered-Lists.h"
#include "benchmark.h"



//******************************************************************************
// local data
//******************************************************************************

hashtable *htab;


// SIZE is a multiple of 2 since the thread counts are a factor of 2
#define SIZE 1024



//******************************************************************************
// private operations
//******************************************************************************

static void *small_hashtable_operations(void *arg) {
    bool status;

    uint t_index = pthread_self();
    int index = *(int*)arg;
    ANNOTATE_HAPPENS_BEFORE(arg);
    
    t_key key = index;
    val_t val = (void*)&addrs[index];
    status = map_insert(htab, key, val);
    address *result_val = (address*) map_search(htab, index);
    print_address(t_index, key, result_val);

    status = map_delete(htab, key);

    result_val = (address*) map_search(htab, key);
    print_address(t_index, key, result_val);
}


static void *hashtable_operations(void *arg) {
    bool status;
    uint t_index = pthread_self();
    int random_start_index = t_index % (ADDRESS_SIZE-20);

    for (int i = 0; i < 20; i++) {
        t_key key = random_start_index + i;
        val_t val = (void*)&addrs[key];
        status = map_insert(htab, key, val);

        address *result_val = (address*) map_search(htab, key);
        //print_address(t_index, key, result_val);

        status = map_delete(htab, key);

        result_val = (address*) map_search(htab, key);
        //print_address(t_index, key, result_val);   
    }
}


static void *benchmark2_operations(void *arg) {
    bool status;
    int iterations = *(int*)arg;
    
    // clock start
    clock_t start = clock();
    t_key key;
    val_t val;
    address *result_val;

    for (int i = 0; i < iterations; i++) {
        key = (rand() % ADDRESS_SIZE);
        val = (void*)&addrs[key];

        status = map_insert(htab, key, val);
        // result_val = (address*) map_search(htab, key);
        // status = map_delete(htab, key);
    }

    // clock end
    clock_t end = clock();
    // print time
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
}


static void benchmark1(int thread_count) {
    int iterations = SIZE / thread_count;
    // 50% inserts, 0% finds, 50% deletes
    
}


static void benchmark2(int thread_count) {
    int iterations = SIZE / thread_count;
    pthread_t thr[thread_count];

    // 33% inserts, 33% finds, 33% deletes
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thr[i], NULL, benchmark2_operations, &iterations);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thr[i], NULL);
    }
}


static void benchmark3() {
    // 25% inserts, 50% finds, 25% deletes
}


static void benchmark4() {
    // 15% inserts, 70% finds, 15% deletes
}


static void benchmark5() {
    // 5% inserts, 90% finds, 5% deletes
}



//******************************************************************************
// interface operations
//******************************************************************************

void split_ordered_list_benchmark () {
    htab = hashtable_initialize();

    // seeding so that different random sequences get generated
    srand (time(NULL));
    benchmark2(1);

    /* int THREADS = 30;
    pthread_t thr[THREADS];
    int *index = malloc(sizeof(int) * THREADS);

    for (int i = 0; i < THREADS; i++) {
        index[i] = i;
        pthread_create(&thr[i], NULL, hashtable_operations, &index[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(thr[i], NULL);
    }

    free(index); */
    print_hashtable(htab);
    
    hashtable_destroy(htab);
}