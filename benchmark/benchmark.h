#ifndef benchmark_h
#define benchmark_h

//******************************************************************************
// system includes
//******************************************************************************

#include <stdint.h>
#include <time.h>       // clock, clock_t
#include <pthread.h>    // pthread_self, pthread_t, pthread_create, pthread_join



//******************************************************************************
// type definitions
//******************************************************************************

typedef struct {
    char *street_name;
} address;



//******************************************************************************
// local data
//******************************************************************************

#define ADDRESS_SIZE 123


extern address addrs[ADDRESS_SIZE];



//******************************************************************************
// interface operations
//******************************************************************************

void print_address(uint64_t thread_index, int key, address *addr);

#endif      // benchmark_h
