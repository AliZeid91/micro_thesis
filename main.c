#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/i2c-dev.h> 
#include <sys/ioctl.h>


#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include <assert.h>




#include <linux/gpio.h>
#include <sys/poll.h>


#include "communication_bus.c"
#include "xbee.c"
#include "ads1015.c"
#include "spi.c"
#include "xbee_uart.c"
#include "xbee_spi.c"
#include "uart.c"
#include "i2c.c"
#include "xbee_process.c"


// #include "timer.c"
// #include "stopwatch.c"
#include "mcp3208.c"

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>


#define SERIAL_PORT_BAUD_RATE 38400

#define XBEE_SERIAL_PORT_PATH  "/dev/ttyUSB0" // Address of our Xbee on the UART bus
#define ADC1015_I2C_PORT_PATH   0x48         // Address of our adc converter on the I2C bus

#define I2C_BUS_FILE_DESCRIPTOR 1

#define MAX_BYTES_TO_READ	10

#define DEV_NAME "/dev/gpiochip0"


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


int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

void time_handler1(size_t timer_id, void * user_data)
{
    printf("Akshata : Single shot timer expired.(%d)\n", timer_id);
}

int main(void) {
 
    printf("Starting the application...\r\n");
    
    uint8_t tx_buffer[256]; // Buffer to store the bytes that we write to the I2C device
    uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 
    
    int package_length = 13;
    int package_frame_type = 0x00;
    int package_frame_id = 0x01; 
    unsigned char package_address[9] = {0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0x87}; //  0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0x87
    create_xbee_packet(package_length,package_frame_type,package_frame_id,package_address);
    
    int adc1015_fd;             // Device-Handle 
    int xbee_serial_fd;
    adc1015_fd  = i2c_open(I2C_BUS_FILE_DESCRIPTOR,ADC1015_I2C_PORT_PATH);
    xbee_serial_fd = serial_open(XBEE_SERIAL_PORT_PATH,SERIAL_PORT_BAUD_RATE);
    
    init_adc(adc1015_fd,tx_buffer,rx_buffer);


    uint8_t 	spi_mode = 0;
    uint32_t 	spi_speed = 500000;

    int mcp3208_chanel = 1;
    int xbee_spi_chanel = 0;
    int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, spi_speed);
    int xbee_spi_fd = init_spi_xbee(xbee_spi_chanel, spi_mode, spi_speed);

    fd_set read_fd, write_fd;
    struct timeval timeout;
    int retval;
    
    #if 0   

    while(1) 
    {
        puts("Starting Select...");
            
        /* Initialize the file descriptor set. */
        FD_ZERO(&read_fd);
        FD_SET(xbee_serial_fd, &read_fd);
        FD_SET(xbee_spi_fd, &read_fd);

        FD_ZERO(&write_fd);
        FD_SET(mcp3208_fd, &write_fd);    
            
        /* Initialize the timeout */
        timeout.tv_sec  = 2;       //2 Seconds
        timeout.tv_usec = 0;
        retval = select(FD_SETSIZE, &read_fd, &write_fd, NULL, &timeout);
            
        if( retval < 0 ) 
        {
            perror("select");
            assert(0);
        }

        if(FD_ISSET(xbee_serial_fd,&read_fd))
        {
            int buffer_length = uart_xbee_rx(xbee_serial_fd,rx_buffer);
            for(int buffer_index=0;buffer_index < buffer_length;buffer_index++)
            {
               process(rx_buffer[buffer_index]);
            }
        }

        if(FD_ISSET(xbee_spi_fd,&read_fd))
        {
            printf("Received SPI buffer...\r\n");
            int buffer_length = spi_xbee_rx(xbee_spi_fd,rx_buffer,11);
            for(int buffer_index=0;buffer_index < buffer_length;buffer_index++)
            {
                process(rx_buffer[buffer_index]);
            }
        }

        if(FD_ISSET(mcp3208_fd,&write_fd))
        {
            tx_buffer[0]= 0b00000110;
            tx_buffer[1]= 0b00000000;
            tx_buffer[2]= 0x00;
            spi_mcp3208_rx(mcp3208_fd, tx_buffer,rx_buffer); 
            //get_analog_value(rx_buffer);
        }
       
        //send_package_uart(&packet,xbee_serial_fd);
        //send_package_spi(&packet,xbee_spi_fd);
        //read_adc(adc1015_fd,tx_buffer,rx_buffer);
        
        msleep(100);
    }    
    #endif 
    
    
    int fd, ret;
    // open the device
    fd = open("/dev/gpiochip0", O_RDONLY);
    if (fd < 0)
    {
        printf("Unabled to open %s: %s", "/dev/gpiochip0", strerror(errno));
        return -1;
    }

        
    struct gpiohandle_data data;
    struct gpioevent_request rq;
   //struct gpiohandle_request req;
    struct pollfd pfd;
    
    #if 0
    req.lineoffsets[0] = 17;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_INPUT;

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
		perror("Error setting GPIO 17 to input");
		close(req.fd);
		close(fd);
		return -1;
	}
    //close(fd);

    do{
        /* Reading Xbee buffer */
        ret = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
        if( ret < 0) 
            perror("Error setting GPIO to 1");

        printf("%s\n", (data.values[0] > 0) ? "No data in buffer" : "Data in buffer");
        if(data.values[0] == 0){
            spi_xbee_rx(xbee_spi_fd,rx_buffer,255);
        }
    }while (data.values[0] != 1);

    close(req.fd);
    
    
    for(int i=0; i<100;i++){
        spi_xbee_rx(xbee_spi_fd,rx_buffer,255);
    }
    
    rq.lineoffset = 17;
    rq.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE; 
    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &rq);
    close(fd);
    
    if (ret == -1)
    {
        printf("Unable to get line event from ioctl : %s", strerror(errno));
        return -1;
    }

    pfd.fd = rq.fd;
    pfd.events = POLLIN;
    while (1)
    {
        
        if (poll(&pfd, 1, -1) == -1)
        {
            printf("Error while polling event from GPIO: %s", strerror(errno));
        }
        else if (pfd.revents & POLLIN)
        {
            printf("Falling edge event on GPIO offset: %d, of %s\n", 1, "/dev/gpiochip0");
            int buffer_length = spi_xbee_rx(xbee_spi_fd,rx_buffer,150);
            for(int buffer_index=0;buffer_index < buffer_length;buffer_index++)
            {
                process(rx_buffer[buffer_index]);
            }
            read(pfd.fd,rx_buffer,255);
        }

    }
    #endif

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

    close_serial_port();
    return 0;
}

