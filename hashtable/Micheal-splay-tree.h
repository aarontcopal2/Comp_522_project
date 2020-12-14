#ifndef MICHEAL_SPLAY_TREE_H
#define MICHEAL_SPLAY_TREE_H

//************************ Forward Declarations ******************************

typedef struct hashtable hashtable;


//******************************************************************************
// splay-tree definitions
//******************************************************************************

#define st_insert				\
    typed_splay_insert(int)

#define st_lookup				\
    typed_splay_lookup(int)

#define st_delete				\
    typed_splay_delete(int)

#define st_forall				\
    typed_splay_forall(int)

#define st_count				\
    typed_splay_count(int)

#define st_alloc(free_list) \
    typed_splay_alloc(free_list, splay_entry_t)

#define st_free(free_list, node) \
    typed_splay_free(free_list, node)


#undef typed_splay_node
#define typed_splay_node(int) splay_entry_t

// we have a special case where we only need to search for keys,
// we are not concerned with the value. Can we remove the value parameter from the struct?
typedef struct typed_splay_node(int) {
    struct typed_splay_node(int) *left;
    struct typed_splay_node(int) *right;
    uint64_t key;
    uint64_t val;
} typed_splay_node(int);



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
    void
);

#endif  // MICHEAL_SPLAY_TREE_H