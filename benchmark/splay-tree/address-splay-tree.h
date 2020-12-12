#ifndef address_splay_tree_h
#define address_splay_tree_h

//******************************************************************************
// local includes
//******************************************************************************

#include "../benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

typedef struct address_splay_entry_t address_splay_entry_t;



//******************************************************************************
// interface operations
//******************************************************************************

address_splay_entry_t*
address_splay_lookup
(
 uint64_t key
);


void
address_splay_insert
(
 uint64_t key,
 address *val
);


void
address_splay_delete
(
 uint64_t key
);


address*
address_splay_entry_val_get
(
 uint64_t key
);


uint64_t
size
(
    void
);


void
clear_splay_tree(
    int delete_root
);

#endif  // address_splay_tree_h