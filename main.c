/*
* main.c
* This file will be the micro-benchmark for the Split-Ordered-Lists datastructure
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <pthread.h>    // pthread_create
#include <stdlib.h>



//******************************************************************************
// local includes
//******************************************************************************

#include "hashtable/Split-Ordered-Lists.h"



//******************************************************************************
// type definitions
//******************************************************************************

typedef struct {
    int apartment_no;
    char *street_name;
} address;

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// private operations
//******************************************************************************

static void print_address(address *addr) {
    if (!addr) {
        printf("address passed is NULL\n");
        return;
    }
    printf("address: {\n\tapartment_no: %d,\n\tstreet_name: %s\n}\n", 
        addr->apartment_no, addr->street_name);
}


void *hashtable_operations(void *arg) {
    bool status;
    t_key keys[5] = {0, 1, 2, 3, 4};
    address addrs[5] = {{232, "Maroneal Street"}, {705, "LBS Road"}, {402, "Kensington SEZ"},
                    {0, "MGLR"},{0, "JVLR"}};

    uint t_index = pthread_self();
    int index = *(int*)arg;
    ANNOTATE_HAPPENS_BEFORE(arg);
    debug_print("thread: %u. index: %d\n", t_index, index);
    
    t_key key = keys[index];
    val_t val = (void*)&addrs[index];
    debug_print("\n***********map_insert***********\n");
    status = map_insert(key, val);
    debug_print("***********status: %d***********\n", status);

    debug_print("***********map_search***********\n");
    address *result_val = (address*) map_search(key);
    print_address(result_val);

    debug_print("***********map_delete***********\n");
    status = map_delete(key);
    debug_print("***********status: %d***********\n", status);

    debug_print("***********map_search***********\n");
    result_val = (address*) map_search(key);
    print_address(result_val);
}



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    initialize_hashtable();

    pthread_t thr;
    int indices[5] = {0, 1, 2, 3, 4};
    for (int i = 0; i < 2; i++) {
        pthread_create(&thr, NULL, hashtable_operations, &indices[i]);
        ANNOTATE_HAPPENS_BEFORE(indices[i]);
    }
    pthread_join(thr, NULL);
}