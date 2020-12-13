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


static __thread spinlock_t splay_lock = SPINLOCK_UNLOCKED;


static __thread splay_tree_pointer *local_spt_head;



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


static void free_splay_tree(splay_entry_t *node, int delete_root) {
    // we dont want to delete the root
    if (node == NULL) {
        return;
    }

    /* first recur on left child */
    free_splay_tree(node->left, delete_root);

    /* now recur on right child */
    free_splay_tree(node->right, delete_root);

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
    ANNOTATE_RWLOCK_ACQUIRED(&splay_lock, 1);
    spinlock_lock(&splay_lock);
    splay_entry_t *result = st_lookup(&splay_root, key);
    spinlock_unlock(&splay_lock);
    ANNOTATE_RWLOCK_RELEASED(&splay_lock, 1);
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
        ANNOTATE_RWLOCK_ACQUIRED(&splay_lock, 1);
        spinlock_lock(&splay_lock);
        splay_entry_t *entry = splay_new(key, val);
        st_insert(&splay_root, entry);  
        spinlock_unlock(&splay_lock);
        ANNOTATE_RWLOCK_RELEASED(&splay_lock, 1);
    }
}


void
splay_delete
(
    uint64_t key
)
{
    ANNOTATE_RWLOCK_ACQUIRED(&splay_lock, 1);
    spinlock_lock(&splay_lock);
    splay_entry_t *node = st_delete(&splay_root, key);
    // st_free(&splay_free_list, node);
    free(node);
    spinlock_unlock(&splay_lock);
    ANNOTATE_RWLOCK_RELEASED(&splay_lock, 1);
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
clear_micheal_splay_tree(int delete_root) {
    free_splay_tree(splay_root, delete_root);
}


static void add_entry_to_global_splay_tree_pointer(hashtable *htab) {
    splay_tree_pointer *null_spt = NULL;

    if (atomic_compare_exchange_strong(&htab->spt_head, &null_spt, local_spt_head)) {
        atomic_compare_exchange_strong(&htab->spt_tail, &null_spt, local_spt_head);
    } else {
        try_again: ;
        splay_tree_pointer *current_tail = atomic_load(&htab->spt_tail);
        if (current_tail == NULL) {
            goto try_again;
        }
        if(atomic_compare_exchange_strong(&htab->spt_tail, &current_tail, local_spt_head)) {
            // adding this helgrind annotation because we are always CAS'ing a NULL pointer
            VALGRIND_HG_DISABLE_CHECKING(&current_tail->next, sizeof(splay_tree_pointer));
            atomic_store(&current_tail->next, local_spt_head);
            VALGRIND_HG_ENABLE_CHECKING(&current_tail->next, sizeof(splay_tree_pointer));
        } else {
            goto try_again;
        }
    }
}


void update_global_splay_tree_pointer(hashtable *htab) {
    if (!local_spt_head) {
        local_spt_head = malloc(sizeof(splay_tree_pointer));
        local_spt_head->root = splay_root;
        add_entry_to_global_splay_tree_pointer(htab);
    } else {
        local_spt_head->root = splay_root;
    }
}
