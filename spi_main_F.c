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
#include "exit_program.c"

// #include "timer.c"
#include "stopwatch.c"
#include "mcp3208.c"

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>


#define SERIAL_PORT_BAUD_RATE 9600

#define XBEE_SERIAL_PORT_PATH  "/dev/ttyUSB0" // Address of our Xbee on the UART bus
#define ADC1015_I2C_PORT_PATH   0x48         // Address of our adc converter on the I2C bus

#define I2C_BUS_FILE_DESCRIPTOR 1

#define MAX_BYTES_TO_READ	10

#define DEV_NAME "/dev/gpiochip0"

#define I2C  1
#define SPI  2

#define OFF  0
#define ON   1

bool run_program = true;

struct gpiohandle_request signal;
struct gpioevent_request exit_button;

struct pollfd pfd;
static struct epoll_event ev;
intVec intData;


int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

int button_pressed;

void myHandler(int fd) {
    char buff [10];
    if(!run_program){
        run_program = true;
        printf("Running Program\n");
    }else{
        run_program = false;
        printf("Stop Program\n");
    }
    read(fd,&buff,10);
}

void set_up_signal();
void send_signal(struct gpiohandle_request* signal);
void set_up_exit_buttom(void);

int main(void) {
    
    printf("Starting the application...\r\n");

    uint8_t tx_buffer[4]; // Buffer to store the bytes that we write to the I2C device
    uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 
    uint8_t mcp3208_tx[10];
    uint8_t mcp3208_rx[10];

    int package_length = 13;
    int package_frame_type = 0x00;
    int package_frame_id = 0x01; 
    unsigned char package_address[9] = {0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0xA0}; //  0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0x87 
    create_xbee_packet(package_length,package_frame_type,package_frame_id,package_address);
    
    set_up_signal();
    //set_up_exit_buttom();
    
    int adc1015_fd;             // Device-Handle 
    adc1015_fd  = i2c_open(I2C_BUS_FILE_DESCRIPTOR,ADC1015_I2C_PORT_PATH);
    init_adc(adc1015_fd,tx_buffer,rx_buffer);

    uint8_t 	spi_mode = 0;
    uint32_t 	spi_speed = 125000000;

    float mcp3208_chanel = 1.0;
    float xbee_spi_chanel = 0.0;
    int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, 50000);
    int xbee_spi_fd = init_spi_xbee(xbee_spi_chanel, spi_mode, spi_speed);

    bool new_adc_val = true;
    
    long time_spi;

    fd_set write_fd, send_package;
    struct timeval timeout;
    int retval;
    button_pressed = 0;
    struct timespec time;
    int i=0;

        while(run_program){
            /* Initialize the timeout */
            timeout.tv_sec  = 2;       //2 Seconds
            timeout.tv_usec = 0;   

            /* Initialize the file descriptor set. */
            FD_ZERO(&send_package);
            FD_SET(xbee_spi_fd,&send_package);

            FD_ZERO(&write_fd);
            FD_SET(adc1015_fd, &write_fd);
            
            if(new_adc_val)
            {   
                msleep(1000);
                send_signal(&signal);
                new_adc_val = false;
                #if 1
                /*Read pedal value & store in rx_buffer */
                retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                if(FD_ISSET(adc1015_fd,&write_fd))
                {
                    ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
                    packet.data[0] = rx_buffer[0];
                    packet.data[1] = rx_buffer[1];
                    new_adc_val = false;
                }
                #endif
            }

            if(!new_adc_val){

                i++;
                /* Initialize the timeout */
                timeout.tv_sec  = 2;       //2 Seconds
                timeout.tv_usec = 0;

                retval = select(FD_SETSIZE, NULL, &send_package, NULL, &timeout);
                if(FD_ISSET(xbee_spi_fd,&send_package))
                {
                    /*send read value across SPI bus*/
                    send_package_spi(&packet,xbee_spi_fd);
                    printf("Sänder\n");
                    new_adc_val = true;  ;
                }
            }    
        }

    
    close(mcp3208_fd);
    close(xbee_spi_fd);
    close(signal.fd);
    close(exit_button.fd);
    return 0;
}
void set_up_signal(){
    int fd;

	/* Open the gpiochip device file */
	fd = open("/dev/gpiochip0", O_RDWR);
	if(fd < 0) {
		perror("Error opening gpiochip0");
		exit(0);
	}

	/* Setup GPIO to output */
	signal.flags = GPIOHANDLE_REQUEST_OUTPUT;
	strcpy(signal.consumer_label, "SIGNAL");
	memset(signal.default_values, 0, sizeof(signal.default_values));
	signal.lines = 1;
	signal.lineoffsets[0] = 5;
    signal.default_values[0] = 1; 

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &signal) < 0) {
		perror("Error setting GPIO 16 to output");
		close(fd);
		exit(0);
	}
}

void send_signal(struct gpiohandle_request* signal){
    struct gpiohandle_data data;
    /* Send a puls */
	data.values[0] = 0;
	if(ioctl(signal->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) 
		perror("Error setting GPIO to 1");

    usleep(50);

    data.values[0] = 1;
	if(ioctl(signal->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) 
		perror("Error setting GPIO to 0");
}

void set_up_exit_buttom(void){
    int fd, ret;
    exit_button.lineoffset = 13;
    exit_button.handleflags = GPIOHANDLE_REQUEST_INPUT;
    exit_button.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
    strcpy(exit_button.consumer_label, "Event test");
    fd = open("/dev/gpiochip0", O_RDONLY);
    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &exit_button);
    if(ret < 0){
         perror("Det går inte att öppna GPIO!");
         exit(0);
    }
    close(fd);
    ev.events = EPOLLIN;
    ev.data.fd = exit_button.fd;
    int epfd = epoll_create(1);
    int res = epoll_ctl(epfd, EPOLL_CTL_ADD, exit_button.fd, &ev);

    intData.epfd = epfd;
    intData.fd = exit_button.fd;
    intData.func = &myHandler;
    pthread_t intThread;
    if (pthread_create(&intThread, NULL, waitInterrupt,
    (void*) &intData)) 
    {
        fprintf(stderr, "Error creating thread\n");
        exit(0);
    }
}