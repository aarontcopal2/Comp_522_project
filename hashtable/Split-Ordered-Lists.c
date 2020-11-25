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



//******************************************************************************
// local data
//******************************************************************************

/* shared variables */
typedef MarkPtrType* segment_t[SEGMENT_SIZE];
segment_t ST[2];      // buckets (Note that the 100 is harcoded and should be made dynamic)
atomic_ullong count = 0;    // total nodes in hash table
uint size = 2;              // hash table size

/* thread private variables
MarkPtrType *prev;
MarkPtrType <pmark, cur>;
MarkPtrType <cmark, next>;
*/



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
    // assuming we are dealing with 64bit keys
    // setting MSB to 1 for regular keys
    return reverse_bits(key | 0x8000000000000000);
}


static so_key_t so_dummy_key(t_key key) {
    return reverse_bits(key);
}


static MarkPtrType* get_bucket(uint bucket) {
    uint segment = bucket / SEGMENT_SIZE;

    // bucket not initialized, hence segment is NULL
    if (ST[segment] == NULL) {
        return UNINITIALIZED;
    }
    return ST[segment][bucket % SEGMENT_SIZE];
}


static void set_bucket(uint bucket, NodeType *head) {
    uint segment = bucket / SEGMENT_SIZE;
    MarkPtrType *null_segment = (MarkPtrType*)calloc(sizeof(MarkPtrType)*SEGMENT_SIZE, 0);

    if (ST[segment] == NULL) {
        MarkPtrType *new_segment = (MarkPtrType*)calloc(sizeof(MarkPtrType)*SEGMENT_SIZE, 0);
        
        /*for (int i = 0; i < SEGMENT_SIZE; i++) {
            new_segment[i] = UNINITIALIZED;
        }*/
        
        if(!atomic_compare_exchange_strong(ST[segment], null_segment, new_segment)) {
            free(new_segment);
        }
    }
    ST[segment][bucket % SEGMENT_SIZE] = &head;
}


static uint get_parent(uint bucket) {
    // parent will differ with child bucket at 1st 1bit of child from left
    // parent will have that bit set to 0
    for (int i = 31; i >= 0; ++i) {
        if (bucket & (1 << i)) {
            return (bucket & ~(1 << i));
        }
    }
    return 0;   //0 is the 1st node
}


static void initialize_bucket(uint bucket) {
    MarkPtrType cur;
    uint parent = get_parent(bucket);
    MarkPtrType *parent_bucket_ptr = get_bucket(bucket);
    if (parent_bucket_ptr == UNINITIALIZED) {
        initialize_bucket(parent);
    }
    NodeType *dummy = malloc(sizeof(NodeType));
    dummy->key = so_dummy_key(bucket);      // is this param correct?
    // do we need to save the hash inside the node?

    /* if another thread began initialization of the same bucket, but didnt complete then adding dummy again will fail
    * if so, we delete allocated dummy node of current thread and instead use the dummy node of the successful thread(cur points to the dummy node of that thread) */
    if (!list_insert(parent_bucket_ptr, dummy)) {
        free(dummy);
        dummy = cur;
    }
    set_bucket(bucket, dummy);
}


static uint64_t fetch_and_increment_count() {
    return atomic_fetch_add(&count, 1);
}


static uint64_t fetch_and_decrement_count() {
    return atomic_fetch_add(&count, -1);
}



//******************************************************************************
// interface operations
//******************************************************************************

bool map_insert(t_key key, val_t val) {
    // inside the node, key is stored in split-ordered form
    NodeType *node = malloc(sizeof(NodeType));
    node->key = so_regular_key(key);
    node->val = val;
    // do we need to save the hash inside the node?

    uint bucket = key % size;

    // intialize bucket if not already done
    MarkPtrType *bucket_ptr = get_bucket(bucket);
    if (bucket_ptr == UNINITIALIZED) {
        initialize_bucket(bucket);
    }

    // in what scenarios will insert fail?
    if (!list_insert(bucket_ptr, node)) {
        free(node);     // no issues with calling free() here, right?
        return false;
    }

    uint csize = size;
    // if insertion is succesful, increment the count of nodes.
    // If the load factor of the hashtable > MAX_LOAD, double the hash table size
    if (fetch_and_increment_count(&count) / csize > MAX_LOAD) {
        atomic_compare_exchange_strong(&size, &csize, 2*csize);
    }
    return true;
}


// need to return value
bool map_find(t_key key) {
    uint bucket = key % size;

    MarkPtrType *bucket_ptr = get_bucket(bucket);
    // ensure that bucket is initialized
    if (bucket_ptr == UNINITIALIZED) {
        initialize_bucket(bucket);
    }
    return list_search(bucket_ptr, so_regular_key(key));
}


bool map_delete(t_key key) {
    uint bucket = key % size;

    MarkPtrType *bucket_ptr = get_bucket(bucket);
    // ensure that bucket is initialized
    if (bucket_ptr == UNINITIALIZED) {
        initialize_bucket(bucket);
    }
    if (!list_delete(bucket_ptr, so_regular_key(key))) {
        return false;
    }

    // if deletion is succesful, decrement the count of nodes.
    fetch_and_decrement_count(&count);
    return true;
}
