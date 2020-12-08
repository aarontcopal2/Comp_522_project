// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2020, Rice University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage.
//
// ******************************************************* EndRiceCopyright *

#ifndef SOL_HT_MEMORY_MANAGER_H
#define SOL_HT_MEMORY_MANAGER_H

//******************************************************************************
// system includes
//******************************************************************************

#include <valgrind/helgrind.h>  // ANNOTATE_HAPPENS_AFTER, ANNOTATE_HAPPENS_BEFORE



//******************************************************************************
// local includes
//******************************************************************************

#include "lib/prof-lean/bistack.h"
#include "hpcrun/gpu/gpu-activity.h"



//************************ Forward Declarations ******************************

typedef struct cct_node_t cct_node_t;



//******************************************************************************
// type declarations
//******************************************************************************


typedef struct sol_ht_object_t sol_ht_object_t;
typedef unsigned int uint;
typedef uint so_key_t;
typedef uint t_key;
typedef void* val_t;


typedef struct __node NodeType;


//what is a markable pointer type
typedef NodeType* MarkPtrType;


//Node: contains key and next pointer
struct __node {
    so_key_t so_key;
    t_key key;
    val_t val;
    bool isDummy;
    sol_ht_object_t *sol_obj_ref;
    _Atomic(MarkPtrType) next;
};


typedef struct sol_ht_object_channel_t sol_ht_object_channel_t;


typedef struct gpu_activity_channel_t gpu_activity_channel_t;


typedef struct __hp_node hazard_ptr_node;

struct __hp_node {
    _Atomic(NodeType*) hp;
    sol_ht_object_t *sol_obj_ref;
    _Atomic(hazard_ptr_node*) next;
};


typedef struct __rl_node retired_list_node;

struct __rl_node {
  NodeType *thread_retired_list_head;
  _Atomic(retired_list_node*) next;
};


typedef struct sol_ht_object_details_t {
  union {
    hazard_ptr_node hpn;
    NodeType node;
  };
  gpu_activity_channel_t *initiator_channel;
} sol_ht_object_details_t;


typedef struct sol_ht_object_t {
  s_element_ptr_t next;
  sol_ht_object_channel_t *channel;
  sol_ht_object_details_t details;
} sol_ht_object_t;


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

    _Atomic(hazard_ptr_node *) hp_head;
    _Atomic(hazard_ptr_node *) hp_tail;
    _Atomic(uint) hazard_pointers_count;        // initialize to 0

    _Atomic(retired_list_node *) rl_head;
    _Atomic(retired_list_node *) rl_tail;
} hashtable;



//******************************************************************************
// interface operations
//******************************************************************************

sol_ht_object_t *
sol_ht_malloc
(
 void
);


void
sol_ht_free
(
 sol_ht_object_t *
);

#endif  // SOL_HT_MEMORY_MANAGER_H
