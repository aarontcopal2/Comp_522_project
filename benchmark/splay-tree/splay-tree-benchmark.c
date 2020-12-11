//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // malloc



//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/splay-tree/splay-uint64.h"
#include "address-splay-tree.h"
#include "benchmark.h"



//******************************************************************************
// private operations
//******************************************************************************

static void initialize_splay_tree() {
    // initializing the splay tree with 1k elements
    for (int i = 0; i < 1000; i++) {
        uint64_t key = i;
        address *val = (void*)&addrs[key];
        address_splay_insert(key, val);
    }
}


static void *splay_tree_thread_operations(void *arg) {
    bool status;
    uint t_index = pthread_self();
    int random_start_index = t_index % (ADDRESS_SIZE-20);

    for (int i = 0; i < 20; i++) {
        uint64_t key = random_start_index + i;
        address *val = (void*)&addrs[key];

        address_splay_insert(key, val);
        address *result_val = (address*) address_splay_entry_val_get(key);
        address_splay_delete(key);
        // result_val = (address*) address_splay_entry_val_get(key);
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
