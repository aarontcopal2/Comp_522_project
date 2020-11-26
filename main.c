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
    
}