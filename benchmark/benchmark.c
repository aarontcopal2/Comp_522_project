//******************************************************************************
// local includes
//******************************************************************************

#include "benchmark.h"



//******************************************************************************
// interface operations
//******************************************************************************

struct timespec timer_start() {
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    return start_time;
}


double timer_elapsed(struct timespec start_time) {
    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    return (((double) (end_time.tv_sec - start_time.tv_sec)) +  /* sec */
            ((double)(end_time.tv_nsec - start_time.tv_nsec))/1000000000); /*nanosec */
}
