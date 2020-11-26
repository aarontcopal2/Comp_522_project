#ifndef _SPLIT_ORDERED_LISTS_
#define _SPLIT_ORDERED_LISTS_

//******************************************************************************
// system includes
//******************************************************************************

#include <stdint.h>     // uint64_t, uintptr_t



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"



//******************************************************************************
// interface operations
//******************************************************************************

void initialize_hashtable
(
    void
);


bool map_insert
(
    t_key key,
    val_t val
);


bool map_find
(
    t_key key
);


bool map_delete
(
    t_key key
);

#endif  //_SPLIT_ORDERED_LISTS_
