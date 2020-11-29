/*
* main.c
* This file will be the micro-benchmark for the Split-Ordered-Lists datastructure
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <valgrind/helgrind.h>
#include <pthread.h>    // pthread_create



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


void *pthread_print(void *arg) {
    bool status;
    t_key keys[5] = {1, 2, 3, 4, 5};
    address addrs[5] = {{232, "Maroneal Street"}, {705, "LBS Road"}, {402, "Kensington SEZ"},
                    {0, "MGLR"},{0, "JVLR"}};

    uint t_index = pthread_self();
    int index = *(int*)arg;
    printf("thread: %u. index: %d\n", t_index, index);

    
    t_key key = keys[index];
    val_t val = (void*)&addrs[index];
    printf("\n***********map_insert***********\n");
    status = map_insert(key, val);
    printf("***********status: %d***********\n", status);

    printf("***********map_search***********\n");
    address *result_val = (address*) map_search(key);
    print_address(result_val);

    printf("***********map_delete***********\n");
    status = map_delete(key);
    printf("***********status: %d***********\n", status);

    printf("***********map_search***********\n");
    result_val = (address*) map_search(key);
    print_address(result_val);
}



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    initialize_hashtable();

    pthread_t thr;
    for (int i = 0; i < 5; i++) {
        pthread_create(&thr, NULL, pthread_print, &i);
    }
    pthread_join(thr, NULL);
}