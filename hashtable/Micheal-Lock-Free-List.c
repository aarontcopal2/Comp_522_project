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


typedef struct __hp_node hazard_ptr_node;

struct __hp_node {
    NodeType *hp;
    hazard_ptr_node *next;
};


#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// local data
//******************************************************************************

hazard_ptr_node *hp_head;
hazard_ptr_node *hp_tail;
__thread hazard_ptr_node *local_hp_head;



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


static hazard_ptr_node* get_thread_hazard_pointers() {
    if (!local_hp_head) {
        hazard_ptr_node *hp = malloc(sizeof(hazard_ptr_node)*3);
        hp[0].next = &hp[1];
        hp[1].next = &hp[2];
        hazard_ptr_node *null_hp = NULL;

        if (!atomic_compare_exchange_strong(&local_hp_head, &null_hp, hp)) {
            free(hp);
        } else {
            if (atomic_compare_exchange_strong(&hp_head, &null_hp, local_hp_head)) {
                // do we need to check if this returns true?
                atomic_compare_exchange_strong(&hp_tail, &null_hp, &local_hp_head[2]);
            } else {
                hazard_ptr_node *current_tail = hp_tail;
                if(atomic_compare_exchange_strong(&(hp_tail->next), &null_hp, local_hp_head)) {
                    // do we need to check if this returns true?
                    atomic_compare_exchange_strong(&hp_tail, &current_tail, &local_hp_head[2]);
                }
            }
        }
    }
    return local_hp_head;
}


static NodeType* get_hazard_pointer(int index) {
    return get_thread_hazard_pointers()[index].hp;
}


static void set_hazard_pointer(NodeType* node, int index) {
    get_thread_hazard_pointers()[index].hp = node;
}


static void clear_hazard_pointers() {
    hazard_ptr_node *hp = get_thread_hazard_pointers();
    hp[0].hp = NULL;
    hp[1].hp = NULL;
    hp[2].hp = NULL;
}


static MarkPtrType list_find(NodeType *head, so_key_t so_key, MarkPtrType *out_prev) {
    debug_print("list_find: %u\n", so_key);
    MarkPtrType prev, cur, next;

    try_again:
        prev = head;
        cur = get_node(prev)->next;
        /*set_hazard_pointer(cur, 1);
        if (atomic_load(prev) != create_mark_pointer(get_node(cur), 0)) {
            goto try_again;
        }*/
        while(true) {
            if (get_node(cur) == NULL) {
                goto done;
            }
            bool cmark = get_mask_bit(cur);
            ANNOTATE_HAPPENS_AFTER(cur);
            next = get_node(cur)->next;
            // ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(cur);
            ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(get_node(cur));
            so_key_t ckey = cur->so_key;
            set_hazard_pointer(next, 0);
            /* commented because cant see in kumpera implementation
            if (atomic_load(&cur) != create_mark_pointer(get_node(next), cmark)) {
                goto try_again;
            }*/
            if (prev->next != create_mark_pointer(get_node(cur), 0)) {
               goto try_again;
            }
            if (!cmark) {
                if (ckey >= so_key) {
                    goto done;
                }
                prev = cur; // get_node(cur)->next; why does commented code work in paper and Kumpera?
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
    debug_print("retiring node: %u\n", node->key);
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
    MarkPtrType prev;
    MarkPtrType node = list_find(head, key, &prev);
    clear_hazard_pointers();
    return node;
}


bool list_insert(MarkPtrType head, NodeType *node) {
    debug_print("list_insert: %p\n", node);
    bool result;
    MarkPtrType cur, prev;
    so_key_t so_key = node->so_key;

    while (true) {
        cur = list_find(head, so_key, &prev);
        if (cur && cur->so_key == so_key) {
            result = false;
            break;
        }

        // since we are calling find, we are inserting the element in a sorted order in the list
        // sort order is based on split-order i.e reversed bits
        // creating a link from prev->node and node->cur (while removing prev->cur)
        node->next = create_mark_pointer(get_node(cur), 0);
        ANNOTATE_HAPPENS_AFTER(node->next);
        //ANNOTATE_HAPPENS_AFTER(node);
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
    MarkPtrType cur, next, prev;

    while (true) {
        cur = list_find(head, key, &prev);
        if (!cur || cur->so_key != key) {
            result = false;
            break;
        }
        next = get_hazard_pointer(0);
        MarkPtrType expected = create_mark_pointer(next, 0);
        if (!atomic_compare_exchange_strong(&(cur->next), &expected, create_mark_pointer(next, 1))) {
            continue;
        }
        expected = create_mark_pointer(cur, 0);
        if (atomic_compare_exchange_strong(&(prev->next), &expected, create_mark_pointer(next, 0))) {
            retire_node(cur);
        } else {
            list_find(head, key, &prev); // Note: Kumpera implementation commented this
        }
        result = true;
        break;
    }
    clear_hazard_pointers();
    return result;
}
