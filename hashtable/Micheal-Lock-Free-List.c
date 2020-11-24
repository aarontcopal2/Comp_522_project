/* 
* Implementation of Micheal-Lock-Free-List (reference)
* This data-structure is an enhancement over Harris-Linked-List
 */

//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // NULL
#include <stdatomic.h>  // atomic_fetch_add



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"



//******************************************************************************
// type definitions
//******************************************************************************

// #define atomic_load(p)  ({ typeof(*p) __tmp = *(p); load_barrier (); __tmp; })



//******************************************************************************
// local data
//******************************************************************************

/* thread private variables
MarkPtrType *prev;
MarkPtrType <pmark, cur>;
MarkPtrType <cmark, next>;
*/



//******************************************************************************
// private operations
//******************************************************************************

// function to create Markable pointer type
static MarkPtrType create_mark_pointer(NodeType *node, uintptr_t mask_bit) {
    return (MarkPtrType) ((uintptr_t)node | mask_bit);
}


static NodeType* get_node(MarkPtrType m_ptr) {
    return (NodeType*) (((uintptr_t)m_ptr) & ~((uintptr_t)(0x1)));
}


static uintptr_t get_mask_bit(MarkPtrType m_ptr) {
    return (uintptr_t)m_ptr & 0x1;
}


// P.S: Arent hazard pointers missing?
static bool list_find(NodeType **head, so_key_t key) {
    MarkPtrType *prev, cur, next;

    try_again:
        prev = head;
        cur = get_node(*prev)->next;
        // *hp1 = cur;
        if (atomic_load(prev) != create_mark_pointer(get_node(cur), 0)) {
            goto try_again;
        }

        while(1) {
            if (get_node(cur) == NULL) {
                return false;
            }
            bool cmark = get_mask_bit(cur);
            next = get_node(cur)->next;
            // *hp0 = next;
            so_key_t ckey = cur->key;
            if (atomic_load(prev) != create_mark_pointer(get_node(cur), 0)) {
                goto try_again;
            }
            if (!cmark) {
                if (ckey >= key) {
                    return ckey == key;
                }
                prev = &get_node(cur)->next;
                // *hp2 = cur;
            } else {
                MarkPtrType expected = create_mark_pointer(get_node(cur), 0);
                if (atomic_compare_exchange_strong(prev,
                        &expected, create_mark_pointer(get_node(next), 0))) {
                    free(cur);      // is it ok to free cur?
                } else {
                    goto try_again;
                }
            }
            cur = next;
            // *hp1 = next;
        }
}



//******************************************************************************
// interface operations
//******************************************************************************

bool list_search(MarkPtrType *head, so_key_t key) {
    bool result = list_find(head, key);
    /*
    *hp0 = NULL;
    *hp1 = NULL;
    *hp2 = NULL;
    */
    return result;
}


bool list_insert(MarkPtrType *head, NodeType *node) {
    bool result;
    MarkPtrType cur, *prev;
    so_key_t key = node->key;

    while (1) {
        if (list_find(head, key)) {
            result = false;
            break;
        }

        // creating a link from prev->node and node->cur (while removing prev->cur)
        node->next = create_mark_pointer(get_node(cur), 0);
        MarkPtrType expected = create_mark_pointer(get_node(cur), 0);
        if (atomic_compare_exchange_strong(prev, &expected, create_mark_pointer(node, 0))) {
            result = true;
            break;
        }
    }
    /*
        *hp0 = NULL;
        *hp1 = NULL;
        *hp2 = NULL;
    */
    return result;
}


bool list_delete(MarkPtrType *head, so_key_t key) {
    bool result;
    MarkPtrType cur, next, *prev;

    while (1) {
        if (!list_find(head, key)) {
            result = false;
            break;
        }
        MarkPtrType expected = create_mark_pointer(next, 0);
        if (!atomic_compare_exchange_strong(&(get_node(cur)->next), &expected, create_mark_pointer(next, 1))) {
            continue;
        }
        expected = create_mark_pointer(cur, 0);
        if (atomic_compare_exchange_strong(prev, &expected, create_mark_pointer(next, 0))) {
            free(cur);       // is it ok to free cur?
        } else {
            list_find(head, key); // Note: Kumpera implementation commented this
            result = true;
        }
    }
    /*
        *hp0 = NULL;
        *hp1 = NULL;
        *hp2 = NULL;
    */
   return result;
}