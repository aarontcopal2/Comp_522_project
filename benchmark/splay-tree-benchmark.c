//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // malloc



//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/splay-tree/splay-uint64.h"
#include "benchmark.h"



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


// we have a special case where we only need to search for keys,
// we are not concerned with the value. Can we remove the value parameter from the struct?
typedef struct typed_splay_node(int) {
  struct typed_splay_node(int) *left;
  struct typed_splay_node(int) *right;
  uint64_t key;
  address *val;
} typed_splay_node(int);


typedef typed_splay_node(int) splay_t;


typed_splay_impl(int)



//******************************************************************************
// local data
//******************************************************************************

splay_t *root = 0;



//******************************************************************************
// private operations
//******************************************************************************

static splay_t* splay_node(uint64_t key, address *val) {
  splay_t *node = (splay_t *) malloc(sizeof(splay_t));
  node->left = node->right = NULL;
  node->key = key;
  node->val = val;
  return node;
}


static void initialize_splay_tree() {
    // initializing the splay tree with 1k elements
    for (int i = 0; i < 1000; i++) {
        uint64_t key = i;
        address *val = (void*)&addrs[key];
        splay_t *node = splay_node(key, val);
        st_insert(&root, node);
        free(node);
    }
}


static void *splay_tree_thread_operations(void *arg) {
    bool status;
    uint t_index = pthread_self();
    int random_start_index = t_index % (ADDRESS_SIZE-20);

    for (int i = 0; i < 20; i++) {
        uint64_t key = random_start_index + i;
        address *val = (void*)&addrs[key];

        splay_t *node = splay_node(key, val);
        st_insert(&root, node);
        free(node);

        address *result_val = (address*) st_lookup(&root, key);

        status = st_delete(&root, key);

        result_val = (address*) st_lookup(&root, key);
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

void splay_tree_benchmark () {
  initialize_splay_tree();
     
  clock_t start = clock();
    
  int THREADS = 30;
  pthread_t thr[THREADS];
  int *index = malloc(sizeof(int) * THREADS);

  for (int i = 0; i < THREADS; i++) {
      index[i] = i;
      pthread_create(&thr[i], NULL, splay_tree_thread_operations, &index[i]);
  }
  for (int i = 0; i < THREADS; i++) {
      pthread_join(thr[i], NULL);
  }

  free(index);
    
  clock_t end = clock();
  double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
}
