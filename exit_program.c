#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdbool.h>

void *waitInterrupt(void *arg);
typedef void (*eventHandler)(int);
uint64_t t[20];

typedef struct {
    bool exit_program;
    int epfd;
    int fd;
    eventHandler func;
} intVec;


void *waitInterrupt(void *arg) {
    intVec *intData = (intVec*) arg;
    struct epoll_event event[1];
    char read_buff[255];
    for (;;) {

        int nfds = epoll_wait(intData->epfd, event, 1, 2000);
        if (nfds != 0) 
        {
            intData->func(intData->fd);
            read(intData->fd, read_buff,255);
            read(event[0].data.fd,read_buff,255);
        }
    }
}
