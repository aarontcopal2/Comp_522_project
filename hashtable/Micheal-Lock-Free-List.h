#ifndef _MICHEAL_LOCK_FREE_LIST_
#define _MICHEAL_LOCK_FREE_LIST_

//******************************************************************************
// system includes
//******************************************************************************

#include <stdbool.h>     // bool, false, true
#include <stdint.h>     // uint64_t, uintptr_t
#include <stdio.h>      // printf



//******************************************************************************
// type definitions
//******************************************************************************

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
    MarkPtrType next;
};



//******************************************************************************
// interface operations
//******************************************************************************

void retire_node
(
    NodeType *node
);


MarkPtrType list_search
(
    MarkPtrType head,
    so_key_t key
);


bool list_insert
(
    MarkPtrType head,
    NodeType *node
);


bool list_delete
(
    MarkPtrType head,
    so_key_t key
);

#endif  //_MICHEAL_LOCK_FREE_LIST_
