//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/Split-Ordered-Lists.h"
#include "benchmark.h"



//******************************************************************************
// local data
//******************************************************************************

hashtable *htab;



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
        print_address(t_index, key, result_val);

        status = map_delete(htab, key);

        result_val = (address*) map_search(htab, key);
        print_address(t_index, key, result_val);   
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

void split_ordered_list_benchmark () {
    htab = hashtable_initialize();

    int THREADS = 30;
    pthread_t thr[THREADS];
    int *index = malloc(sizeof(int) * THREADS);

    for (int i = 0; i < THREADS; i++) {
        index[i] = i;
        pthread_create(&thr[i], NULL, hashtable_operations, &index[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(thr[i], NULL);
    }

    free(index);
    print_hashtable(htab);
    
    hashtable_destroy(htab);
}