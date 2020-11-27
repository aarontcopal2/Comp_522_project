/* 
* Implementation of Micheal-Lock-Free-List (reference)
* This data-structure is an enhancement over Harris-Linked-List
 */

//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // NULL
#include <stdatomic.h>  // atomic_fetch_add
#include <pthread.h>    // pthread_setspecific, pthread_key_t



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"



//******************************************************************************
// type definitions
//******************************************************************************

// #define atomic_load(p)  ({ typeof(*p) __tmp = *(p); load_barrier (); __tmp; })
typedef void* gpointer;



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


static gpointer* get_thread_hazard_pointers() {
    pthread_key_t hazard_pointers_key;
    gpointer *hp = pthread_getspecific(hazard_pointers_key);

    if (!hp) {
        hp = malloc(sizeof(gpointer)*3);
        pthread_setspecific(hazard_pointers_key, hp);
    }
    return hp;
}


static NodeType* get_hazard_pointer(int index) {
    return (NodeType*)get_thread_hazard_pointers()[index];
}


static gpointer set_hazard_pointer(NodeType* node, int index) {
    get_thread_hazard_pointers()[index] = node;
}


static void clear_hazard_pointers() {
    gpointer *hp = get_thread_hazard_pointers();
    hp[0] = NULL;
    hp[1] = NULL;
    hp[2] = NULL;
}


static MarkPtrType list_find(NodeType *head, so_key_t key, MarkPtrType *out_prev) {
    printf("list_find: %u\n", key);
    MarkPtrType prev, cur, next;

    try_again:
        prev = head;
        cur = get_node(prev)->next;
        /*set_hazard_pointer(cur, 1);
        if (atomic_load(prev) != create_mark_pointer(get_node(cur), 0)) {
            goto try_again;
        }*/
        while(true) {
            NodeType *temp = get_node(cur);
            if (temp == NULL) {
                printf("get_node(cur) is null\n");
            }
            if (get_node(cur) == NULL) {
                goto done;
            }
            bool cmark = get_mask_bit(cur);
            next = get_node(cur)->next;
            so_key_t ckey = cur->so_key;
            set_hazard_pointer(next, 0);
            if (atomic_load(&cur) != create_mark_pointer(get_node(next), cmark)) {
                goto try_again;
            }
            if (!cmark) {
                if (ckey >= key) {
                    goto done;
                }
                prev = get_node(cur)->next;
                set_hazard_pointer(cur, 2);
            } else {
                MarkPtrType expected = create_mark_pointer(get_node(cur), 0);
                if (atomic_compare_exchange_strong(&prev,
                        &expected, create_mark_pointer(get_node(next), 0))) {
                    retire_node(cur);
                } else {
                    goto try_again;
                }
            }
            cur = next;
            set_hazard_pointer(next, 1);
        }
    done:
        *out_prev = prev;
        return cur;
}



//******************************************************************************
// interface operations
//******************************************************************************

void retire_node(NodeType *node) {
    // rlist,rcount is a thread private list
    // RETIRE_THRESHOLD = H + omega(H); where H is total no. of hazard pointers
    /*rlist.push(node);
    rcount++;
    if (rcount >= RETIRE_THRESHOLD) {
        Scan(array of hazard pointers);
    }
    */
}


MarkPtrType list_search(MarkPtrType head, so_key_t key) {
    MarkPtrType *prev;
    MarkPtrType node = list_find(head, key, prev);
    clear_hazard_pointers();
    return node;
}


bool list_insert(MarkPtrType head, NodeType *node) {
    printf("list_insert: %p\n", node);
    bool result;
    MarkPtrType cur, prev;
    so_key_t so_key = node->so_key;

    while (true) {
        cur = list_find(head, so_key, &prev);
        if (cur) {
            result = false;
            break;
        }

        // since we are calling find, we are inserting the element in a sorted order in the list
        // sort order is based on split-order i.e reversed bits
        // creating a link from prev->node and node->cur (while removing prev->cur)
        node->next = create_mark_pointer(get_node(cur), 0);
        MarkPtrType expected = create_mark_pointer(get_node(cur), 0);
        if (atomic_compare_exchange_strong(&(prev->next), &expected, create_mark_pointer(node, 0))) {
            result = true;
            break;
        }
    }
    clear_hazard_pointers();
    return result;
}


bool list_delete(MarkPtrType head, so_key_t key) {
    bool result;
    MarkPtrType cur, next, *prev;

    while (true) {
        MarkPtrType node = list_find(head, key, prev);
        if (!node) {
            result = false;
            break;
        }
        MarkPtrType expected = create_mark_pointer(next, 0);
        if (!atomic_compare_exchange_strong(&(get_node(cur)->next), &expected, create_mark_pointer(next, 1))) {
            continue;
        }
        expected = create_mark_pointer(cur, 0);
        if (atomic_compare_exchange_strong(prev, &expected, create_mark_pointer(next, 0))) {
            retire_node(cur);
        } else {
            list_find(head, key, prev); // Note: Kumpera implementation commented this
            result = true;
        }
    }
    clear_hazard_pointers();
    return result;
}