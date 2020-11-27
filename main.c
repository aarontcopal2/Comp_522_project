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
// interface operations
//******************************************************************************

int main () {
    bool status;

    initialize_hashtable();

    so_key_t key = 1;
    address addr = {232, "Maroneal Street"}; 
    val_t val = (void*)&addr;
    status = map_insert(key, val);
    printf("status map_insert:1: %d\n", status);

    status = map_search(key);
    printf("status map_search:1: %d\n", status);

}