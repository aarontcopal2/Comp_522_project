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

static void print_address(address *addr) {
    if (!addr) {
        printf("address passed is NULL\n");
        return;
    }
    /* We use sprintf + write instead of printf. Helgrind reports data-races for the latter */
    char buffer[75];
    int stdout = 1;
    int char_count = sprintf(buffer, "address:  %s\n", addr->street_name);
    write(stdout, buffer, char_count);
}


void *small_hashtable_operations(void *arg) {
    bool status;
    /* address addrs[11] = {{232, "Maroneal Street"}, {705, "LBS Road"}, {402, "Kensington SEZ"},
                    {0, "MGLR"},{0, "JVLR"}, {489, "E. Edgefield Street"}, {839, "NW. Mountainview St"},
                    {298, "Cambridge Lane"}, {8166, "John Road"}, {88, "Brickyard Rd"},
                    {180, "Pleasant Dr."}}; */

    uint t_index = pthread_self();
    int index = *(int*)arg;
    ANNOTATE_HAPPENS_BEFORE(arg);
    debug_print("thread: %u. index: %d\n", t_index, index);
    
    t_key key = index;
    val_t val = (void*)&addrs[index];
    status = map_insert(htab, key, val);

    address *result_val = (address*) map_search(htab, index);
    print_address(result_val);

    status = map_delete(htab, key);

    result_val = (address*) map_search(htab, key);
    print_address(result_val);
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
        print_address(result_val);

        status = map_delete(htab, key);

        result_val = (address*) map_search(htab, key);
        print_address(result_val);   
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    htab = hashtable_initialize();

    pthread_t thr;
    int indices[11] = {0, 1, 2, 3, 4, 5, 6, 7 ,8, 9, 10};
    for (int i = 0; i < 11; i++) {
        pthread_create(&thr, NULL, small_hashtable_operations, &indices[i]);
        ANNOTATE_HAPPENS_BEFORE(indices[i]);
    }
    pthread_join(thr, NULL);

    print_hashtable(htab);
    
    hashtable_destroy(htab);
}
