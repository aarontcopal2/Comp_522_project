/*
* main.c
* This file will be the micro-benchmark for the Split-Ordered-Lists datastructure
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <pthread.h>    // pthread_create
#include <unistd.h>     // write


//******************************************************************************
// local includes
//******************************************************************************

#include "hashtable/Split-Ordered-Lists.h"
#include "main.h"



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

#define ADDRESS_SIZE 123


//******************************************************************************
// private operations
//******************************************************************************

static void print_address(uint thread_index, int key, address *addr) {
    if (!addr) {
        printf("thread: %u key: %d: address passed is NULL\n", thread_index, key);
        return;
    }
    /* We use sprintf + write instead of printf. Helgrind reports data-races for the latter */
    char buffer[75];
    int stdout = 1;
    int char_count = sprintf(buffer, "thread: %u key: %d: address:  %s\n", thread_index, key, addr->street_name);
    write(stdout, buffer, char_count);
}


void *small_hashtable_operations(void *arg) {
    bool status;

    uint t_index = pthread_self();
    int index = *(int*)arg;
    ANNOTATE_HAPPENS_BEFORE(arg);
    debug_print("thread: %u. index: %d\n", t_index, index);
    
    t_key key = index;
    val_t val = (void*)&addrs[index];
    status = map_insert(htab, key, val);
    address *result_val = (address*) map_search(htab, index);
    print_address(t_index, key, result_val);

    //print_hashtable(htab);
    status = map_delete(htab, key);

    result_val = (address*) map_search(htab, key);
    print_address(t_index, key, result_val);
    //print_hashtable(htab);
}


void *hashtable_operations(void *arg) {
    bool status;
    uint t_index = pthread_self();
    int random_start_index = t_index % (ADDRESS_SIZE-20);
    debug_print("thread: %u. index: %d\n", t_index, random_start_index);

    for (int i = 0; i < 20; i++) {
        t_key key = random_start_index + i;
        val_t val = (void*)&addrs[key];
        status = map_insert(htab, key, val);

        address *result_val = (address*) map_search(htab, key);
        /* print_address(key, result_val);

        status = map_delete(htab, key);

        result_val = (address*) map_search(htab, key);
        print_address(key, result_val);    */
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    htab = hashtable_initialize();

    pthread_t thr[10];
    for (int i = 0; i < 11; i++) {
        int *index = malloc(sizeof(int) * 1);
        *index = i;
        pthread_create(&thr[i], NULL, small_hashtable_operations, index);
    }
    for (int i = 0; i < 11; i++) {
        pthread_join(thr[i], NULL);
    }

    print_hashtable(htab);
    
    hashtable_destroy(htab);
}
