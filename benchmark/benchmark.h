#ifndef benchmark_h
#define benchmark_h

//******************************************************************************
// system includes
//******************************************************************************

#include <stdint.h>
#include <time.h>       // clock, clock_t
#include <pthread.h>    // pthread_self, pthread_t, pthread_create, pthread_join



//******************************************************************************
// local data
//******************************************************************************

#define UPPER 200000


// ITERATION_SIZE is a multiple of 2 since the thread counts are a factor of 2
#define ITERATION_SIZE 256


#define LOWER 1500


#define LAST_THREAD 32

#endif      // benchmark_h
