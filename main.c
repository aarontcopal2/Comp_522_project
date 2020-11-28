/*
* main.c
* This file will be the micro-benchmark for the Split-Ordered-Lists datastructure
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <omp.h>



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



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    bool status;
    initialize_hashtable();

    t_key keys[5] = {1, 2, 3, 4, 5};
    address addrs[5] = {{232, "Maroneal Street"}, {705, "LBS Road"}, {402, "Kensington SEZ"},
                    {0, "MGLR"},{0, "JVLR"}};

    omp_set_num_threads(5);

    #pragma omp parallel
    {
        int t_index = omp_get_thread_num();
        printf("thread %d\n", t_index);

        t_key key = keys[t_index];
        val_t val = (void*)&addrs[t_index];
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
    
}