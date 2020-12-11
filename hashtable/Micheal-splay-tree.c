//******************************************************************************
// system includes
//******************************************************************************

#include <stddef.h>     // NULL
#include <assert.h>     // assert



//******************************************************************************
// local includes
//******************************************************************************

#include "splay-tree/splay-uint64.h"
#include "channel/lib/prof-lean/spinlock.h"
#include "channel/hpcrun/gpu/gpu-splay-allocator.h"



//******************************************************************************
// splay-tree definitions
//******************************************************************************

#define st_insert				\
  typed_splay_insert(int)

#define st_lookup				\
  typed_splay_lookup(int)

#define st_delete				\
  typed_splay_delete(int)

#define st_forall				\
  typed_splay_forall(int)

#define st_count				\
  typed_splay_count(int)

#define st_alloc(free_list) \
  typed_splay_alloc(free_list, splay_entry_t)

#define st_free(free_list, node) \
  typed_splay_free(free_list, node)


// we have a special case where we only need to search for keys,
// we are not concerned with the value. Can we remove the value parameter from the struct?
typedef struct typed_splay_node(int) {
  struct typed_splay_node(int) *left;
  struct typed_splay_node(int) *right;
  uint64_t key;
  int val;
} typed_splay_node(int);


typedef typed_splay_node(int) splay_entry_t;


typed_splay_impl(int)



//******************************************************************************
// local data
//******************************************************************************

static __thread splay_entry_t *splay_root = NULL;
static __thread splay_entry_t *splay_free_list = NULL;


static spinlock_t splay_lock = SPINLOCK_UNLOCKED;


//******************************************************************************
// private operations
//******************************************************************************

static splay_entry_t *
splay_alloc()
{
  return st_alloc(&splay_free_list);
}


static splay_entry_t *
splay_new
(
 uint64_t key,
 int val
)
{
  splay_entry_t *e = splay_alloc();
  e->key = key;
  e->val = val;
  return e;
}



//******************************************************************************
// interface operations
//******************************************************************************

splay_entry_t*
splay_lookup
(
 uint64_t key
)
{
  spinlock_lock(&splay_lock);
  splay_entry_t *result = st_lookup(&splay_root, key);
  spinlock_unlock(&splay_lock);
  return result;
}


void
splay_insert
(
 uint64_t key,
 int val
)
{
  if (st_lookup(&splay_root, key)) {
    assert(0);  // entry for a given key should be inserted only once
  } else {
    spinlock_lock(&splay_lock);
    splay_entry_t *entry = splay_new(key, val);
    st_insert(&splay_root, entry);  
    spinlock_unlock(&splay_lock);
  }
}


void
splay_delete
(
 uint64_t key
)
{
  splay_entry_t *node = st_delete(&splay_root, key);
  st_free(&splay_free_list, node);
}


int
splay_entry_val_get
(
    uint64_t key
)
{
    splay_entry_t *e = splay_lookup(key);
    if (!e) {
      return -1;
    }
    return e->val;
}