//******************************************************************************
// system includes
//******************************************************************************

#include <stdio.h>
#include <unistd.h>     // write



//******************************************************************************
// local includes
//******************************************************************************

#include "benchmark.h"



//******************************************************************************
// interface operations
//******************************************************************************

void print_address(uint64_t thread_index, int key, address *addr) {
  if (!addr) {
      printf("thread: %u key: %d: address passed is NULL\n", thread_index, key);
      return;
  }
  /* We use sprintf + write instead of printf. Helgrind reports data-races for the latter */
  char buffer[75];
  int stdout = 1;
  int char_count = sprintf(buffer, "thread: %u key: %d: address:  %s\n", thread_index, key, addr->street_name);
  write(stdout, buffer, char_count);
}