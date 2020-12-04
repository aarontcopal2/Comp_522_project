#ifndef _SPLIT_ORDERED_LISTS_
#define _SPLIT_ORDERED_LISTS_

//******************************************************************************
// system includes
//******************************************************************************

#include <stdint.h>     // uint64_t, uintptr_t
#include <pthread.h>    // pthread_rwlock_t, pthread_rwlock_init
#include "channel/lib/prof-lean/stdatomic.h"  // atomic_fetch_add, atomic_size_t



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"



//******************************************************************************
// type definitions
//******************************************************************************

typedef MarkPtrType *segment_t;     // segment_t is an array of MarkType pointers


typedef struct {
    _Atomic(segment_t*) ST;                          // buckets (2D array of Marktype pointers)
    _Atomic(segment_t*) old_ST;

    atomic_size_t size;                            // hash table size
    atomic_size_t old_size;

    atomic_size_t count;                    // total nodes in hash table
    
    atomic_size_t resizing_state;
    atomic_size_t next_init_block;
    atomic_size_t num_initialized_blocks;
    atomic_size_t next_move_block;
    atomic_size_t num_moved_blocks;
    pthread_rwlock_t resize_rwl;
} hashtable;



//******************************************************************************
// interface operations
//******************************************************************************

hashtable* hashtable_initialize
(
    void
);


void hashtable_destroy
(
    hashtable *htab
);


bool map_insert
(
    hashtable *htab,
    t_key key,
    val_t val
);


val_t map_search
(
    hashtable *htab,
    t_key key
);


bool map_delete
(
    hashtable *htab,
    t_key key
);

#endif  //_SPLIT_ORDERED_LISTS_
