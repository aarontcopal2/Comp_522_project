#ifndef _MICHEAL_LOCK_FREE_LIST_
#define _MICHEAL_LOCK_FREE_LIST_

//******************************************************************************
// system includes
//******************************************************************************

#include <stdbool.h>     // bool, false, true
#include <stdint.h>     // uint64_t, uintptr_t
#include <stdio.h>      // printf
#include <valgrind/helgrind.h>  // ANNOTATE_HAPPENS_AFTER, ANNOTATE_HAPPENS_BEFORE



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"
#include "splay-tree/splay-uint64.h"
#include "channel/hashtable-memory-manager.h"



//******************************************************************************
// type definitions
//******************************************************************************



//******************************************************************************
// interface operations
//******************************************************************************

void retire_node
(
    hashtable *htab,
    NodeType *node
);


MarkPtrType list_search
(
    hashtable *htab,
    MarkPtrType head,
    so_key_t key
);


bool list_insert
(
    hashtable *htab,
    MarkPtrType head,
    NodeType *node
);


bool list_delete
(
    hashtable *htab,
    MarkPtrType head,
    so_key_t key
);

#endif  //_MICHEAL_LOCK_FREE_LIST_
