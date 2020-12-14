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

#include "splay-tree/splay-uint64.h"
#include "channel/lib/prof-lean/spinlock.h"
#include "channel/hpcrun/gpu/gpu-splay-allocator.h"
#include "channel/hashtable-memory-manager.h"
#include "Micheal-splay-tree.h"



//******************************************************************************
// splay-tree definitions
//******************************************************************************

typed_splay_impl(int)



//******************************************************************************
// local data
//******************************************************************************

static __thread splay_entry_t *splay_root = NULL;
static __thread splay_entry_t *splay_free_list = NULL;



//******************************************************************************
// private operations
//******************************************************************************

static splay_entry_t *
splay_alloc()
{
    return st_alloc(&splay_free_list);
}


static splay_entry_t *
splay_new
(
    uint64_t key,
    uint64_t val
)
{
    splay_entry_t *e = malloc(sizeof(splay_entry_t));
    e->left = e->right = NULL;
    e->key = key;
    e->val = val;
    return e;
}


static void free_splay_tree(splay_entry_t *node) {
    // we dont want to delete the root
    if (node == NULL) {
        return;
    }

    /* first recur on left child */
    free_splay_tree(node->left);

    /* now recur on right child */
    free_splay_tree(node->right);

    if (node == splay_root) {
        splay_root = NULL;
    }
    // freeing the node
    free(node);
}



//******************************************************************************
// interface operations
//******************************************************************************

splay_entry_t*
splay_lookup
(
    uint64_t key
)
{
    splay_entry_t *result = st_lookup(&splay_root, key);
    return result;
}


void
splay_insert
(
    uint64_t key,
    uint64_t val
)
{
    if (splay_lookup(key)) {
        // entry for a given key should be inserted only once
    } else {
        splay_entry_t *entry = splay_new(key, val);
        st_insert(&splay_root, entry);  
    }
}


void
splay_delete
(
    uint64_t key
)
{
    splay_entry_t *node = st_delete(&splay_root, key);
    // st_free(&splay_free_list, node);
    free(node);
}


uint64_t
splay_entry_val_get
(
    uint64_t key
)
{
    splay_entry_t *e = splay_lookup(key);
    if (!e) {
      return -1;
    }
    return e->val;
}


void
clear_micheal_splay_tree() {
    free_splay_tree(splay_root);
}
