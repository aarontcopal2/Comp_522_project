#ifndef MICHEAL_SPLAY_TREE_H
#define MICHEAL_SPLAY_TREE_H

//************************ Forward Declarations ******************************

typedef struct splay_entry_t splay_entry_t;



//******************************************************************************
// interface operations
//******************************************************************************

splay_entry_t*
splay_lookup
(
 uint64_t key
);


void
splay_insert
(
 uint64_t key,
 uint64_t val
);


void
splay_delete
(
 uint64_t key
);


uint64_t
splay_entry_val_get
(
 uint64_t key
);


void
clear_micheal_splay_tree
(
    int delete_root
);

#endif  // MICHEAL_SPLAY_TREE_H