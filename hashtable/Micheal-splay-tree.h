typedef struct splay_entry_t splay_entry_t;

//******************************************************************************
// interface operations
//******************************************************************************

splay_entry_t*
splay_lookup
(
 uint64_t key
);


void
splay_insert
(
 uint64_t key,
 int val
);


void
splay_delete
(
 uint64_t key
);


int
splay_entry_val_get
(
 uint64_t key
);