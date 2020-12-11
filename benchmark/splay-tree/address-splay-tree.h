typedef struct address_splay_entry_t address_splay_entry_t;

//******************************************************************************
// interface operations
//******************************************************************************

address_splay_entry_t*
address_splay_lookup
(
 uint64_t key
);


void
address_splay_insert
(
 uint64_t key,
 int val
);


void
address_splay_delete
(
 uint64_t key
);


int
address_splay_entry_val_get
(
 uint64_t key
);