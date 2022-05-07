#include <pthread.h>
#include <sys/epoll.h>
#include <poll.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

void *waitInterrupt(void *arg);
typedef void (*eventHandler)(int);
uint64_t t[20];

typedef struct {
    int gpio;
    int fd;
    eventHandler func;
} intVec;


void *waitInterrupt(void *arg) {
    intVec *intData = (intVec*) arg;
    int gpio=intData->gpio;  
    struct pollfd fdset[1];  
    fdset[0].fd = intData->fd;  
    fdset[0].events = POLLPRI;  
    fdset[0].revents = 0;  
    for (;;) {  
        int rc = poll(fdset, 1, -1);  
        if (fdset[0].revents & POLLPRI)
        {  
            intData->func(intData->fd);
        }      
        lseek(fdset[0].fd, 0, SEEK_SET);  
        //readGPIO(gpio);  
    }  
}
