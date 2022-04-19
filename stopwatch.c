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
    //long diffInMilli = (end_time.tv_sec - start_time.tv_sec) * (long)1e3 + ((end_time.tv_nsec - start_time.tv_nsec) / 1.0e6);
    long diffInMilli = (end_time.tv_sec - start_time.tv_sec) * (long)1e3 + ((end_time.tv_nsec - start_time.tv_nsec));
    return diffInMilli;
}

#if 0
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    char buff[100];
    strftime(buff, sizeof buff, "%T", gmtime(&ts.tv_sec));
    printf("Current time: %s:%09ld UTC\n", buff, ts.tv_nsec);
    int h, m, s;
    sscanf(buff, "%d:%d:%d", &h, &m, &s);
    printf ("\n%d, %d, %d\n", h, m, s);
#endif