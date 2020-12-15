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

#define ADDRESS_SIZE 2702


#define UPPER 200000


// ITERATION_SIZE is a multiple of 2 since the thread counts are a factor of 2
#define ITERATION_SIZE 256


#define LOWER 1500


#define LAST_THREAD 32


extern address addrs[ADDRESS_SIZE];



//******************************************************************************
// interface operations
//******************************************************************************

void print_address(uint64_t thread_index, int key, address *addr);

#endif      // benchmark_h
