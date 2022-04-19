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
    struct epoll_event ev;
    printf("startar en thread nu");
    for (;;) {
        int nfds = epoll_wait(intData->epfd, &ev, 1, -1);
        if (nfds != 0) 
        {
            intData->func(intData->fd);
        }
    }
}
