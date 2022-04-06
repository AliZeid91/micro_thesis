#include <time.h>

// call this function to start a nanosecond-resolution timer
struct timespec timer_start(){
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    return start_time;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(struct timespec start_time){
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    // Convert nanoseconds to milliseconds
    long diffInMilli = (end_time.tv_sec - start_time.tv_sec) * (long)1e3 + ((end_time.tv_nsec - start_time.tv_nsec) / 1.0e6);
    return diffInMilli;
}