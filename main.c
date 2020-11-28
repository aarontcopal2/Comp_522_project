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
    printf("address: {\n\tapartment_no: %d,\n\tstreet_name: %s\n}\n", 
        addr->apartment_no, addr->street_name);
}



//******************************************************************************
// interface operations
//******************************************************************************

int main () {
    bool status;

    initialize_hashtable();

    so_key_t key = 1;
    address addr = {232, "Maroneal Street"}; 
    val_t val = (void*)&addr;
    status = map_insert(key, val);
    printf("status map_insert(1) %d\n", status);
    printf("map_search(1)\n");
    address *result_val = (address*) map_search(key);
    print_address(result_val);
}