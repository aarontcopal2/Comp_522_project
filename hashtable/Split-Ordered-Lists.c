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
#include <stdatomic.h>  // atomic_fetch_add
#include <string.h>     // memcpy



//******************************************************************************
// local includes
//******************************************************************************

#include "Split-Ordered-Lists.h"



//******************************************************************************
// macros
//******************************************************************************

#define UNINITIALIZED NULL
#define MAX_LOAD 5              // is a 5 node bucket fine?
#define SEGMENT_SIZE 5          // is SEGMENT_SIZE = 5 ok?

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


//******************************************************************************
// local data
//******************************************************************************

/* shared variables */
typedef MarkPtrType *segment_t; //segment_t is an array of MarkType pointers
segment_t *ST;              // buckets (2D array of Marktype pointers)
atomic_ullong count = 0;    // total nodes in hash table
uint size = 2;              // hash table size



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


static MarkPtrType get_bucket(uint bucket) {
    debug_print("get_bucket: %u\n", bucket);
    uint segment = bucket / SEGMENT_SIZE;

    // bucket not initialized, hence segment is NULL
    if (ST[segment] == NULL) {
        return UNINITIALIZED;
    }
    return ST[segment][bucket % SEGMENT_SIZE];
}


static void set_bucket(uint bucket, NodeType *head) {
    debug_print("set_bucket: %u\n", bucket);
    uint segment = bucket / SEGMENT_SIZE;
    MarkPtrType *null_segment = (MarkPtrType*)calloc(sizeof(MarkPtrType)*SEGMENT_SIZE, 0);
    // we may need to set new_segment[i-SEGMENT_SIZE] = NULL
    if (ST[segment] == NULL) {
        MarkPtrType *new_segment = (MarkPtrType*)calloc(sizeof(MarkPtrType)*SEGMENT_SIZE, 0);
        
        if(!atomic_compare_exchange_strong(&ST[segment], null_segment, new_segment)) {
            // some other thread beat us to updating the segment
            free(new_segment);
            free(null_segment);
            return;
        }
    }
    free(null_segment);
    ST[segment][bucket % SEGMENT_SIZE] = head;
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
    return 0;   //0(dummy) is the 1st node i.e parent of all nodes
}


static MarkPtrType initialize_bucket(uint bucket) {
    debug_print("initialize_bucket: %u\n", bucket);

    MarkPtrType cur;
    uint parent = get_parent(bucket);
    MarkPtrType parent_bucket_ptr = get_bucket(parent);
    if (parent_bucket_ptr == UNINITIALIZED) {
        parent_bucket_ptr = initialize_bucket(parent);
    }

    NodeType *dummy = malloc(sizeof(NodeType));
    dummy->so_key = so_dummy_key(bucket);      // is this param correct?
    dummy->key = bucket;
    dummy->isDummy = true;
    dummy->next = NULL;
    ANNOTATE_HAPPENS_BEFORE(dummy);
    // do we need to save the hash inside the node?

    /* if another thread began initialization of the same bucket, but didnt complete then adding dummy again will fail
    * if so, we delete allocated dummy node of current thread and instead use the dummy node of the successful thread(cur points to the dummy node of that thread) */
    
    /* As the table size increases, the bucket values calculated for a key will either stay same or increase.
    * and if the bucket values increase, the first initialization call for that bucket will create a link from
    * parent bucket to new bucket's dummy node. This will ensure that:
    * 1. If a thread has old value of size and peforms some operation after this insertion on old bucket,
    * it can access the elements from the appended bucket 
    * 2. Operations accessing the parent bucket will be able to insert elements in the child bucket if thats
    * needed to maintain the list order */
    if (!list_insert(parent_bucket_ptr, dummy)) {
        retire_node(dummy);
        cur = list_search(parent_bucket_ptr, so_dummy_key(dummy->key));
        dummy = cur;
    }
    set_bucket(bucket, dummy);
    /* we call get_bucket again rather than returning from set_bucket because some other thread may have called set_bucket and updated
    * the bucket pointer. get_bucket is safer.
    * TO-DO: I think the if(!list_insert()) loop will get the right dummy node and we can return dummy without calling get_bucket().*/

    return get_bucket(bucket);
}


static uint64_t fetch_and_increment_count() {
    return atomic_fetch_add(&count, 1);
}


static uint64_t fetch_and_decrement_count() {
    return atomic_fetch_add(&count, -1);
}


static void resize_hashtable() {
    uint csize = size;
    segment_t *old_ST = ST;
    segment_t *new_ST = malloc(sizeof(segment_t) * csize * 2);
    /* will this operation need to be done using a lock?
    * Else we may loose some insertions happening between memcpy and changing swapping of old table with new
    * Fix1: use hazard pointers for ST. One pointer per thread */
    memcpy(new_ST, ST, sizeof(segment_t) * csize);

    if (!atomic_compare_exchange_strong(&size, &csize, 2*csize)) {
        // a concurrent thread already incremented the size
        free(new_ST);
        return;
    }

    if (!atomic_compare_exchange_strong(&ST, old_ST, new_ST)) {
        // a concurrent thread already switched the table pointer
        free(new_ST);
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

void initialize_hashtable () {
    ST = malloc(sizeof(segment_t) * size);
    ST[0] = (MarkPtrType*)malloc(SEGMENT_SIZE * sizeof(MarkPtrType*));
    for (int i = 0; i < SEGMENT_SIZE; i++) {
        ST[0][i] = NULL;
    }
    // adding a dummy node for key = 0. Without this node, intialize bucket calls to bucket=0 will be stuck in an infinite loop
    t_key start_key = 0;
    NodeType *start_node = malloc(sizeof(NodeType));
    start_node->so_key = so_dummy_key(start_key);
    start_node->key = start_key;
    start_node->isDummy = true;
    start_node->next = NULL;
    set_bucket(start_key, start_node);
}


bool map_insert(t_key key, val_t val) {
    debug_print("map_insert: %u\n", key);
    uint bucket = key % size;

    // intialize bucket if not already done
    MarkPtrType bucket_ptr = get_bucket(bucket);
    if (!bucket_ptr) {
        bucket_ptr = initialize_bucket(bucket);
    }

    // inside the node, key is stored in split-ordered form
    NodeType *node = malloc(sizeof(NodeType));
    node->so_key = so_regular_key(key);
    node->key = key;
    node->val = val;
    node->isDummy = false;
    node->next = NULL;
    
    // do we need to save the hash inside the node?

    // list_insert will fail if the key already exists
    if (!list_insert(bucket_ptr, node)) {
        free(node);     // no issues with calling free() here, right?
        return false;
    }

    // if insertion is succesful, increment the count of nodes.
    // If the load factor of the hashtable > MAX_LOAD, resize the hash table
    if (fetch_and_increment_count(&count) / (size * SEGMENT_SIZE) > MAX_LOAD) {
        resize_hashtable();
    }
    return true;
}


// need to return value
val_t map_search(t_key key) {
    uint bucket = key % size;

    MarkPtrType bucket_ptr = get_bucket(bucket);
    // ensure that bucket is initialized
    if (bucket_ptr == UNINITIALIZED) {
        bucket_ptr = initialize_bucket(bucket);
    }
    MarkPtrType result = list_search(bucket_ptr, so_regular_key(key));
    if (result && result->val) {
        return result->val;
    }
    return NULL;
}


bool map_delete(t_key key) {
    uint bucket = key % size;

    MarkPtrType bucket_ptr = get_bucket(bucket);
    // ensure that bucket is initialized
    if (bucket_ptr == UNINITIALIZED) {
        bucket_ptr = initialize_bucket(bucket);
    }
    if (!list_delete(bucket_ptr, so_regular_key(key))) {
        return false;
    }

    // if deletion is succesful, decrement the count of nodes.
    fetch_and_decrement_count(&count);
    return true;
}
