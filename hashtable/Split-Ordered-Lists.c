/* 
* "Split-Ordered Lists: Lock-Free Extensible Hash Tables" by Ori Shalev & Nir Shavit
* 
* Split-Order-Lists is implemented on top of Micheal-Lock-Free-List
* 2 datastructures: linkedlist with nodes + expanding array of pointers into the list
* bucket array initially has size 2, and is doubled when no. of items in the table exceeds size*L (size is size after previous resize, L is no.of items in a bucket)
* each bucket is uninitialized at start, initialization happens when a bucket is first accessed. AFter initialization of a bucket, it will point to some node in the list.
* hash is computed for a key for insert, delete and search operations. Hash function = (k % size). Table size equals some power 2^i, i >= 1
* if nodes that are entry points of a bucket are deleted, it will result in complications that require special handling. The way we handle that is adding dummy nodes which will be pointed to by the bucket array. Dummy nodes will not be deleted.
*/

//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // NULL
#include <string.h>     // memcpy
#include <assert.h>     // assert
#include <unistd.h>     // write



//******************************************************************************
// local includes
//******************************************************************************

#include "Split-Ordered-Lists.h"



//******************************************************************************
// macros
//******************************************************************************

#define INITIAL_SEGMENTS 2
#define SEGMENT_SIZE 2          // is SEGMENT_SIZE = 5 ok?
#define MAX_LOAD 2              // is an average load of (5 nodes/bucket) fine?


#define NON_BLOCKING 0
#define BLOCKING 1


#define NO_RESIZING 0u
#define ALLOCATING_MEMORY 1u
#define MOVING_DATA 3u
#define CLEANING 2u


#define STATE_BITS 2u
#define STATE_INCREMENT (1u << STATE_BITS)
#define STATE_MASK (STATE_INCREMENT - 1)
#define GET_STATE(r) ((r) & STATE_MASK)
#define GET_ACTIVE_REPLICAS(r) (r >> STATE_BITS)
#define IS_NO_RESIZE_OR_CLEANING(r) (((r) & 0x1u) == 0)


/* each thread will take up initialization work of 64 segments at a time
* each segment has 5 buckets inside it.
So, a block will contain 64 * 5 = 320 memory locations */
#define BLOCK_SIZE 64
#define MAX(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
        (_a >= _b) ? a : b; })


#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// private operations
//******************************************************************************

static uint reverse_bits(t_key key) {
    // reversing the key bits and inserting them in order in the linkedlist removes the additional sorting step during splitting of buckets
    // TO-DO: need to use a lookup table
    t_key reverse_key = 0;
    for (int i = 0; i < 32; ++i) {
        t_key t = (key & (1 << i)) >> i;
        reverse_key |= (t << (31 - i));
    }
    return reverse_key;
}


static so_key_t so_regular_key(t_key key) {
    // assuming we are dealing with 32bit keys(TO-DO: should be using 64bit keys)
    // setting MSB to 1 for regular keys
    return reverse_bits(key | 0x80000000);
}


static so_key_t so_dummy_key(t_key key) {
    return reverse_bits(key);
}


static bool is_dummy_node(so_key_t key) {
    return (key & 0x01) == 0;
}


static MarkPtrType get_bucket(hashtable *htab, uint bucket) {
    debug_print("get_bucket: %u\n", bucket);
    uint segment = bucket / SEGMENT_SIZE;

    MarkPtrType *seg = atomic_load(&htab->ST)[segment];
    // bucket not initialized, hence segment is NULL
    if (seg == NULL) {
        return NULL;
    }
    return seg[bucket % SEGMENT_SIZE];
    ANNOTATE_HAPPENS_AFTER(seg[bucket % SEGMENT_SIZE]);     // get_bucket is always called after set_bucket
}


static void set_bucket(hashtable *htab, uint bucket, NodeType *head) {
    debug_print("set_bucket: %u\n", bucket);
    uint segment = bucket / SEGMENT_SIZE;
    MarkPtrType *seg = atomic_load(&htab->ST)[segment];
    ANNOTATE_HAPPENS_BEFORE(seg[bucket % SEGMENT_SIZE]);
    seg[bucket % SEGMENT_SIZE] = head;
}


static uint get_parent(uint bucket) {
    debug_print("get_parent: %u\n", bucket);
    // parent will differ with child bucket at 1st 1bit of child from left
    // parent will have that bit set to 0
    for (int i = 31; i >= 0; ++i) {
        if (bucket & (1 << i)) {
            return (bucket & ~(1 << i));
        }
    }
    return 0;   // 0(dummy) is the 1st node i.e parent of all nodes
}


static MarkPtrType initialize_bucket(hashtable *htab, uint bucket) {
    debug_print("initialize_bucket: %u\n", bucket);

    MarkPtrType cur;
    uint parent = get_parent(bucket);

    try_again: ;
    MarkPtrType parent_bucket_ptr = get_bucket(htab, parent);
    if (parent_bucket_ptr == NULL) {
        parent_bucket_ptr = initialize_bucket(htab, parent);
        if (parent_bucket_ptr == NULL) {
            goto try_again;
        }
    }

    NodeType *dummy = malloc(sizeof(NodeType));
    /* sol_ht_object_t *sol_obj = sol_ht_malloc();
    NodeType *dummy = &(sol_obj->details.node); */
    dummy->so_key = so_dummy_key(bucket);      // is this param correct?
    dummy->key = bucket;
    dummy->isDummy = true;
    // dummy->sol_obj_ref = sol_obj;
    atomic_init(&dummy->next, NULL);
    // ANNOTATE_HAPPENS_BEFORE(dummy);
    // do we need to save the hash inside the node?

    /* if another thread began initialization of the same bucket, and completed before current thread, then dummy insertion will fail
    * if so, we delete allocated dummy node of current thread and instead use the dummy node of the successful thread(cur points to the dummy node of that thread) */
    
    /* As the table size increases, the bucket values calculated for a key will either stay same or increase.
    * and if the bucket values increase, the first initialization call for that bucket will create a link from
    * parent bucket to new bucket's dummy node. This will ensure that:
    * 1. If a thread has old value of size and peforms some operation after this insertion on old bucket,
    * it can access the elements from the appended bucket 
    * 2. Operations accessing the parent bucket will be able to insert elements in the child bucket if thats
    * needed to maintain the list order */
    if (!list_insert(htab, &parent_bucket_ptr, dummy)) {
        cur = list_search(htab, &parent_bucket_ptr, so_dummy_key(bucket));
        retire_node(htab, dummy);
        return NULL;
    }
    set_bucket(htab, bucket, dummy);
    /* we call get_bucket again rather than returning from set_bucket because some other thread may have called set_bucket and updated
    * the bucket pointer. get_bucket is safer.
    * TO-DO: I think the if(!list_insert()) loop will get the right dummy node and we can return dummy without calling get_bucket().*/

    return get_bucket(htab, bucket);
}


static size_t fetch_and_increment_count(hashtable *htab) {
    return atomic_fetch_add(&htab->count, 1);
}


static size_t fetch_and_decrement_count(hashtable *htab) {
    return atomic_fetch_add(&htab->count, -1);
}


static void resize_task(hashtable *htab, int blocking) {
    // data structure is divided into blocks, keep picking block that next needs to be worked upon
    size_t old_size = atomic_load(&htab->old_size);
    size_t size = atomic_load(&htab->size);
    size_t num_old_blocks = MAX(old_size / BLOCK_SIZE, 1);
    size_t num_new_blocks = MAX(size / BLOCK_SIZE, 1);
    size_t my_block;
    size_t num_finished_blocks = 0;

    // initialize blocks
    while ((my_block = atomic_fetch_add(&htab->next_init_block, 1)) < num_new_blocks) {
        size_t block_start = my_block * BLOCK_SIZE;
        size_t block_end = (my_block + 1) * BLOCK_SIZE;
        if (block_end > size) {
            block_end = size;
        }

        for (int i = block_start; i < block_end; i++) {
            for (int j = 0; j < SEGMENT_SIZE; j++) {
                atomic_load(&htab->ST)[i][j] = NULL;
            }
        }
        num_finished_blocks++;
    }

    // wait for all blocks to be initialized
    atomic_fetch_add(&htab->num_initialized_blocks, num_finished_blocks);
    // is there a need for this loop? maybe yes, if some other thread takes up the last block
    while (atomic_load(&htab->num_initialized_blocks) != num_new_blocks);

    // move old block contents to new block
    num_finished_blocks = 0;
    while ((my_block = atomic_fetch_add(&htab->next_move_block, 1)) < num_old_blocks) {
        size_t block_start = my_block * BLOCK_SIZE;
        size_t block_end = (my_block + 1) * BLOCK_SIZE;
        size_t old_size = atomic_load(&htab->old_size);
        if (block_end > old_size) {
            block_end = old_size;
        }

        for (int i = block_start; i < block_end; i++) {
            for (int j = 0; j < SEGMENT_SIZE; j++) {
                MarkPtrType *tmp = atomic_load(&htab->ST)[i];
                tmp[j] = atomic_load(&htab->old_ST)[i][j];
            }
        }
        num_finished_blocks++;
    }

    atomic_fetch_add(&htab->num_moved_blocks, num_finished_blocks);

    if (blocking) {
        while (atomic_load(&htab->num_moved_blocks) != num_old_blocks);
    }
}


static void resize_replica(hashtable *htab) {
    // get resize state for hashtable
    size_t resize_state = atomic_load(&htab->resizing_state); 

    // if state = (cleaning | no_resizing), resize has finished
    if (IS_NO_RESIZE_OR_CLEANING(resize_state)) {
        return;
    }

    // register as replica and check again if resize has finished
    resize_state = atomic_fetch_add(&htab->resizing_state, STATE_INCREMENT);
    if (IS_NO_RESIZE_OR_CLEANING(resize_state)) {
        atomic_fetch_sub(&htab->resizing_state, STATE_INCREMENT);
        return;
    }

    // wait for new table allocation to complete
    while (GET_STATE(resize_state) == ALLOCATING_MEMORY) {
        resize_state = atomic_load(&htab->resizing_state);
    }

    // check if resize is done
    assert(GET_STATE(resize_state) != NO_RESIZING);
    if (GET_STATE(resize_state == CLEANING)) {
        atomic_fetch_sub(&htab->resizing_state, STATE_INCREMENT);
        return;
    }

    // resize_task with blocking=0
    resize_task(htab, NON_BLOCKING);

    // deregister replica
    atomic_fetch_sub(&htab->resizing_state, STATE_INCREMENT);
}


static void resize_primary(hashtable *htab) {
    // initialize values
    size_t old_size = atomic_load(&htab->size);
    atomic_store(&htab->old_size, old_size);
    atomic_store(&htab->old_ST, atomic_load(&htab->ST));
    atomic_store(&htab->size, old_size * 2);
    size_t size = atomic_load(&htab->size);

    // malloc new table.
    segment_t *ST = malloc(sizeof(segment_t) * size);
    for (int i = 0; i < size; i++) {
        ST[i] = (MarkPtrType*)malloc(SEGMENT_SIZE * sizeof(MarkPtrType*));
    }
    atomic_store(&htab->ST, ST);
    assert(atomic_load(&htab->ST));

    // change state from allocation to moving data
    size_t resize_state = atomic_fetch_xor(&htab->resizing_state, ALLOCATING_MEMORY ^ MOVING_DATA);       // what was the purpose of XOR? Could we simply store 3?

    // resize_task with blocking=1
    resize_task(htab, BLOCKING);

    // change state from moving data to cleaning
    resize_state = atomic_fetch_xor(&htab->resizing_state, MOVING_DATA ^ CLEANING);

    // wait for active replica threads to be zero
    while (GET_ACTIVE_REPLICAS(resize_state) != 0) {
        resize_state = atomic_load(&htab->resizing_state);
    }

    // no more active replicas
    // reinitialize values for future resize
    atomic_store(&htab->next_init_block, 0);
    atomic_store(&htab->num_initialized_blocks, 0);
    atomic_store(&htab->next_move_block, 0);
    atomic_store(&htab->num_moved_blocks, 0);

    // freeing old table
    segment_t *old_ST = atomic_load(&htab->old_ST);
    // free child segments first
    for (int i = 0; i < old_size; i++) {
        free(old_ST[i]);
    }
    free(old_ST);

    // change state from cleaning to no_resizing
    resize_state = atomic_fetch_xor(&htab->resizing_state, CLEANING ^ NO_RESIZING);
}


static void resize_hashtable(hashtable *htab) {
    debug_print("resize_hashtable()\n");
    printf("resize_hashtable()\n");

    /* resizing requires mallocing a new memory region and copying old values to this new region and swapping the data structure pointers
    * This operation needs to be atomic, else we may loose some insertions happening between memcpy and swapping of old table with new
    * there will be 3 helper functions for atomic resizing: resize_primary, resize_replica, resize_task
    * resize_primary: first thread that calls resize will invoke be the master thread for resizing and invoke this function
    * resize_replica: during resize, insert-search-delete should be paused. Any thread trying to do such operations(or invoking resize a 2nd time) while a resize is
    * ongoing is marked as a resize_replica. Such threads will invoke resize_replica()
    * actual resizing work happens in the resize_task function */

   size_t resizing_state = atomic_load(&htab->resizing_state);
    if (resizing_state == 0 &&
        atomic_compare_exchange_strong(&htab->resizing_state, &resizing_state, ALLOCATING_MEMORY)) {
        // Primary thread
        pthread_rwlock_wrlock(&htab->resize_rwl);
        resize_primary(htab);
        pthread_rwlock_unlock(&htab->resize_rwl);
    } else {
        // Replica(Secondary) thread
        resize_replica(htab);
    }
}


static void free_all_nodes(NodeType *start_node) {
    // There are 2 types of nodes in the LinkedList (regular nodes externally inserted and dummy nodes for internal use)
    // will free ALL nodes in this function.
    // To-Do: Think of a parallel free solution.
    // This function should be called only from hashtable destroy()
    NodeType *node = start_node, *next;

    while(node) {
        if (node != NULL && atomic_load(&node->next)) {
            next = atomic_load(&node->next);
            free(node);
            node = next;
        } else {
            free(node);
            break;
        }
    }
}


static void free_retired_nodes(retired_list_node *rl_head) {
    retired_list_node *rl_ref = rl_head, *next;

    while (rl_ref) {
        NodeType *thread_rl_head = rl_ref->thread_retired_list_head;
        free_all_nodes(thread_rl_head);
        next = atomic_load(&rl_ref->next);
        free(rl_ref);
        rl_ref = next;
    }
}


static void free_hazard_pointers(hazard_ptr_node * start_hp, uint hp_count) {
    // This function should be called only from hashtable destroy()
    hazard_ptr_node *hp = start_hp, *next;

    while (hp_count != 0) {
        // hazard pointer is initialized as an array of size 3. The next thread's hazard pointer is thus linked to hp[2].next
        next = atomic_load(&hp[2].next);
        free(hp);
        hp = next;
        // each hp is an array of size 3
        hp_count -= 3;
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

hashtable* hashtable_initialize () {
    hashtable *htab = malloc(sizeof(hashtable));
    atomic_init(&htab->count, 0);
    atomic_init(&htab->size, INITIAL_SEGMENTS);
    atomic_init(&htab->old_size, 0);
    atomic_init(&htab->old_ST, NULL);
    atomic_init(&htab->next_init_block, 0);
    atomic_init(&htab->next_move_block, 0);
    atomic_init(&htab->num_initialized_blocks, 0);
    atomic_init(&htab->num_moved_blocks, 0);
    atomic_init(&htab->resizing_state, 0);
    pthread_rwlock_init(&htab->resize_rwl, NULL);
    atomic_init(&htab->hp_head, NULL);
    atomic_init(&htab->hp_tail, NULL);
    atomic_init(&htab->hazard_pointers_count, 0);
    atomic_init(&htab->rl_head, NULL);
    atomic_init(&htab->rl_tail, NULL);

    segment_t *ST = malloc(sizeof(segment_t) * INITIAL_SEGMENTS);
    for (int i = 0; i < INITIAL_SEGMENTS; i++) {
        ST[i] = (MarkPtrType*)malloc(SEGMENT_SIZE * sizeof(MarkPtrType*));
        for (int j = 0; j < SEGMENT_SIZE; j++) {
            ST[i][j] = NULL;
        }
    }
    atomic_init(&htab->ST, ST);
    // adding a dummy node for key = 0. Without this node, intialize bucket calls to bucket=0 will be stuck in an infinite loop
    t_key start_key = 0;

    NodeType *start_node = malloc(sizeof(NodeType));
    /* sol_ht_object_t *sol_obj = sol_ht_malloc();
    NodeType *start_node = &(sol_obj->details.node); */
    start_node->so_key = so_dummy_key(start_key);
    start_node->key = start_key;
    start_node->isDummy = true;
    // start_node->sol_obj_ref = sol_obj;
    atomic_init(&start_node->next, NULL);
    set_bucket(htab, start_key, start_node);

    return htab;
}


void hashtable_destroy(hashtable *htab) {
    segment_t *ST = atomic_load(&htab->ST);
    size_t size = atomic_load(&htab->size);

    // free dummy and regular nodes
    NodeType *start_node = ST[0][0];
    free_all_nodes(start_node);

    // free retired nodes
    retired_list_node *rl_head = atomic_load(&htab->rl_head);
    free_retired_nodes(rl_head);

    // free hazard pointers
    hazard_ptr_node *start_hp = atomic_load(&htab->hp_head);
    uint hp_count = atomic_load(&htab->hazard_pointers_count);
    free_hazard_pointers(start_hp, hp_count);
    
    // free child segments
    for (int i = 0; i < size; i++) {
        free(ST[i]);
    }

    // free segment array
    free(ST);
    pthread_rwlock_destroy(&htab->resize_rwl);
    free(htab);
}


void print_hashtable(hashtable *htab) {
    char buffer[275];
    int stdout = 1;
    int char_count;

    NodeType *node = atomic_load(&htab->ST)[0][0];
    while (node) {
        if (node->isDummy) {
            char_count = sprintf(buffer, "dummy(%d) -> ", node->key);
            write(stdout, buffer, char_count);
        } else {
            char_count = sprintf(buffer, "%d -> ", node->key);
            write(stdout, buffer, char_count);
        }
        node = atomic_load(&node->next);
    }
    char_count = sprintf(buffer, "NULL\n");
    write(stdout, buffer, char_count);
}


bool map_insert(hashtable *htab, t_key key, val_t val) {
    debug_print("map_insert: %u\n", key);

    // if a resize is in progress(initiated by another thread), block operations and make current thread a resize helper
    while (pthread_rwlock_tryrdlock(&htab->resize_rwl) != 0) {
        resize_replica(htab);
    }
    pthread_rwlock_unlock(&htab->resize_rwl);

    uint bucket = key % atomic_load(&htab->size);

    // intialize bucket if not already done
    try_again: ;
    MarkPtrType bucket_ptr = get_bucket(htab, bucket);
    if (!bucket_ptr) {
        bucket_ptr = initialize_bucket(htab, bucket);
        if (bucket_ptr == NULL) {
            goto try_again;
        }
    }

    // inside the node, key is stored in split-ordered form
    NodeType *node = malloc(sizeof(NodeType));
    /* sol_ht_object_t *sol_obj = sol_ht_malloc();
    NodeType *node = &(sol_obj->details.node); */
    node->so_key = so_regular_key(key);
    node->key = key;
    node->val = val;
    node->isDummy = false;
    // node->sol_obj_ref = sol_obj;
    atomic_init(&node->next, NULL);
    
    // do we need to save the hash inside the node?

    // list_insert will fail if the key already exists
    if (!list_insert(htab, &bucket_ptr, node)) {
        retire_node(htab, node);
        return false;
    }
    // print_hashtable(htab);

    // if insertion is succesful, increment the count of nodes.
    // If the load factor of the hashtable > MAX_LOAD, resize the hash table
    size_t count = fetch_and_increment_count(htab);
    size_t size = atomic_load(&htab->size);
    size_t load =  count / (size * SEGMENT_SIZE);
    if (load > MAX_LOAD) {
        resize_hashtable(htab);
    }
    return true;
}


// need to return value
val_t map_search(hashtable *htab, t_key key) {
    // if a resize is in progress(initiated by another thread), block operations and make current thread a resize helper
    while (pthread_rwlock_tryrdlock(&htab->resize_rwl) != 0) {
        resize_replica(htab);
    }
    pthread_rwlock_unlock(&htab->resize_rwl);
    uint bucket = key % atomic_load(&htab->size);

    // ensure that bucket is initialized
    try_again: ;
    MarkPtrType bucket_ptr = get_bucket(htab, bucket);
    if (bucket_ptr == NULL) {
        bucket_ptr = initialize_bucket(htab, bucket);
        if (bucket_ptr == NULL) {
            goto try_again;
        }
    }

    MarkPtrType result = list_search(htab, &bucket_ptr, so_regular_key(key));
    if (result && result->val) {
        return result->val;
    }
    return NULL;
}


bool map_delete(hashtable *htab, t_key key) {
    // if a resize is in progress(initiated by another thread), block operations and make current thread a resize helper
    while (pthread_rwlock_tryrdlock(&htab->resize_rwl) != 0) {
        resize_replica(htab);
    }
    pthread_rwlock_unlock(&htab->resize_rwl);
    uint bucket = key % atomic_load(&htab->size);

    // ensure that bucket is initialized
    try_again: ;
    MarkPtrType bucket_ptr = get_bucket(htab, bucket);
    if (bucket_ptr == NULL) {
        bucket_ptr = initialize_bucket(htab, bucket);
        if (bucket_ptr == NULL) {
            goto try_again;
        }
    }
    if (!list_delete(htab, &bucket_ptr, so_regular_key(key))) {
        return false;
    }

    // if deletion is succesful, decrement the count of nodes.
    fetch_and_decrement_count(htab);
    return true;
}
