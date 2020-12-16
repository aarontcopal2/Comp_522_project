#ifndef benchmark_h
#define benchmark_h

//******************************************************************************
// system includes
//******************************************************************************

#include <stdint.h>
#include <time.h>       // timespec
#include <pthread.h>    // pthread_self, pthread_t, pthread_create, pthread_join



//******************************************************************************
// local data
//******************************************************************************

#define LOWER 50000
#define UPPER 20000000

// ITERATION_SIZE is a multiple of 2 since the thread counts are a factor of 2
#define ITERATION_SIZE 1048576
//4096, 32768, 524288, 1048576, 4194304, 8388608

#define LAST_THREAD 32



//******************************************************************************
// interface operations
//******************************************************************************

struct timespec timer_start();


double timer_elapsed(struct timespec start_time);

#endif      // benchmark_h
