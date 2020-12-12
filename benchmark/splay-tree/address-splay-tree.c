//******************************************************************************
// system includes
//******************************************************************************

#include <stddef.h>     // NULL
#include <assert.h>     // assert
#include <stdlib.h>     // free
#include <valgrind/helgrind.h>  // ANNOTATE_HAPPENS_AFTER, ANNOTATE_HAPPENS_BEFORE



//******************************************************************************
// local includes
//******************************************************************************

#include "../../hashtable/splay-tree/splay-uint64.h"
#include "../../hashtable/channel/lib/prof-lean/spinlock.h"
#include "../../hashtable/channel/hpcrun/gpu/gpu-splay-allocator.h"
#include "address-splay-tree.h"



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
    typed_splay_alloc(free_list, address_splay_entry_t)

#define st_free(free_list, node) \
    typed_splay_free(free_list, node)

#undef typed_splay_node
#define typed_splay_node(int) address_splay_entry_t

// we have a special case where we only need to search for keys,
// we are not concerned with the value. Can we remove the value parameter from the struct?
typedef struct typed_splay_node(int) {
    struct typed_splay_node(int) *left;
    struct typed_splay_node(int) *right;
    uint64_t key;
    address *val;
} typed_splay_node(int);


typed_splay_impl(int)



//******************************************************************************
// local data
//******************************************************************************

static address_splay_entry_t *splay_root = NULL;
static address_splay_entry_t *splay_free_list = NULL;


static spinlock_t address_splay_lock = SPINLOCK_UNLOCKED;



//******************************************************************************
// private operations
//******************************************************************************

static address_splay_entry_t *
address_splay_alloc
(
    void
)
{
    return st_alloc(&splay_free_list);
}


static address_splay_entry_t *
address_splay_new
(
    uint64_t key,
    address *val
)
{
    address_splay_entry_t *e = malloc(sizeof(address_splay_entry_t));
    e->left = e->right = NULL;
    e->key = key;
    e->val = val;
    return e;
}


static void free_splay_tree(address_splay_entry_t *node, int delete_root) {
    // we dont want to delete the root
    if (node == NULL || (node == splay_root && !delete_root)) {
        return;
    }

    /* first recur on left child */
    free_splay_tree(node->left, delete_root);

    /* now recur on right child */
    free_splay_tree(node->right, delete_root);

    // freeing the node
    free(node);
}



//******************************************************************************
// interface operations
//******************************************************************************

address_splay_entry_t*
address_splay_lookup
(
    uint64_t key
)
{
    ANNOTATE_RWLOCK_ACQUIRED(&address_splay_lock, 1);
    spinlock_lock(&address_splay_lock);
    address_splay_entry_t *result = st_lookup(&splay_root, key);
    spinlock_unlock(&address_splay_lock);
    ANNOTATE_RWLOCK_RELEASED(&address_splay_lock, 1);
    return result;
}


void
address_splay_insert
(
    uint64_t key,
    address *val
)
{
    if (address_splay_lookup(key)) {
        // Do nothing, entry for a given key should be inserted only once
    } else {
        ANNOTATE_RWLOCK_ACQUIRED(&address_splay_lock, 1);
        spinlock_lock(&address_splay_lock);
        address_splay_entry_t *entry = address_splay_new(key, val);
        st_insert(&splay_root, entry);  
        spinlock_unlock(&address_splay_lock);
        ANNOTATE_RWLOCK_RELEASED(&address_splay_lock, 1);
    }
}


void
address_splay_delete
(
    uint64_t key
)
{
    ANNOTATE_RWLOCK_ACQUIRED(&address_splay_lock, 1);
    spinlock_lock(&address_splay_lock);
    address_splay_entry_t *node = st_delete(&splay_root, key);
    //st_free(&splay_free_list, node);
    free(node);
    spinlock_unlock(&address_splay_lock);
    ANNOTATE_RWLOCK_RELEASED(&address_splay_lock, 1);

}


address*
address_splay_entry_val_get
(
    uint64_t key
)
{
    address_splay_entry_t *e = address_splay_lookup(key);
    if (!e) {
        return NULL;
    }
    return e->val;
}


uint64_t size() {
    ANNOTATE_RWLOCK_ACQUIRED(&address_splay_lock, 1);
    spinlock_lock(&address_splay_lock);
    uint64_t size = st_count(splay_root);
    spinlock_unlock(&address_splay_lock);
    ANNOTATE_RWLOCK_RELEASED(&address_splay_lock, 1);
    return size;
}


void
clear_splay_tree(int delete_root) {
    free_splay_tree(splay_root, delete_root);
}