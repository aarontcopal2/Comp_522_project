#ifndef _MICHEAL_LOCK_FREE_LIST_
#define _MICHEAL_LOCK_FREE_LIST_

//******************************************************************************
// system includes
//******************************************************************************

#include <stdbool.h>     // bool, false, true
#include <stdint.h>     // uint64_t, uintptr_t
#include <stdio.h>      // printf



//******************************************************************************
// local includes
//******************************************************************************

#include "splay-tree/splay-uint64.h"
#include "channel/hashtable-memory-manager.h"



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
    MarkPtrType *head,
    so_key_t key
);


bool list_insert
(
    hashtable *htab,
    MarkPtrType *head,
    NodeType *node
);


bool list_delete
(
    hashtable *htab,
    MarkPtrType *head,
    so_key_t key
);

#endif  //_MICHEAL_LOCK_FREE_LIST_
