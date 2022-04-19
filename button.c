
#if 0
void *waitInterrupt(void *arg);
typedef void (*eventHandler)(int);
uint64_t t[20];

typedef struct {
 int epfd;
 int fd;
 eventHandler func;
} intVec;

void myHandler(int fd) {

    struct gpioevent_data data;
    read(fd, &data, sizeof data);
    printf("%u,%llu \n\r", data.id, data.timestamp);
    fflush(stdout);
    printf("Den FUNGERAR!!!!");
}

void *waitInterrupt(void *arg) {
    intVec *intData = (intVec*) arg;
    struct epoll_event ev;
    for (;;) {
        int nfds = epoll_wait(intData->epfd, &ev, 1, 20000);
        if (nfds != 0) 
        {
            intData->func(intData->fd);
            usleep(1000);
        }
        
    }
}

 
read(pfd.fd,rx_buffer,255);
    struct gpioevent_request req;
    req.lineoffset = 20;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
    req.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
    strcpy(req.consumer_label, "Event test");

    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
    close(fd);
    static struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = req.fd;
    int epfd = epoll_create(1);
    int res = epoll_ctl(epfd, EPOLL_CTL_ADD, req.fd, &ev);
    intVec intData;
    intData.epfd = epfd;
    intData.fd = req.fd;
    intData.func = &myHandler;
    pthread_t intThread;
    
    if (pthread_create(&intThread, NULL, waitInterrupt,
    (void*) &intData)) 
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    for (;;) {
        printf("Working\n\r");
        fflush(stdout);
        sleep(2);
    }
#endif