#ifndef test_splay_tree_h
#define test_splay_tree_h

//******************************************************************************
// local includes
//******************************************************************************

#include "../benchmark.h"



//******************************************************************************
// type definitions
//******************************************************************************

typedef struct test_splay_entry_t test_splay_entry_t;



//******************************************************************************
// interface operations
//******************************************************************************

test_splay_entry_t*
test_splay_lookup
(
 uint64_t key
);


void
test_splay_insert
(
 uint64_t key,
 uint64_t val
);


void
test_splay_delete
(
 uint64_t key
);


uint64_t
test_splay_entry_val_get
(
 uint64_t key
);


uint64_t
size
(
    void
);


void
clear_test_splay_tree(
    void
);

#endif  // test_splay_tree_h
