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

bool run_program;


struct gpiohandle_data data;

struct gpiohandle_request signal, stop_led, start_led;
struct gpioevent_request exit_button, start_button;

struct pollfd pfd;
static struct epoll_event ev;
intVec intData;
pthread_t start_thread, stop_thread;


int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}


void set_up_signal();
void send_signal(struct gpiohandle_request* signal);
void set_up_exit_buttom(void);
void set_up_start_buttom(void);
void set_up_stop_led();
void set_up_start_led();

int button_pressed;
int count=0;
void stop_program(int fd) {

    run_program = false;
    close(exit_button.fd);
    /* Turn Off Stop LED */
    close(start_led.fd);
    set_up_start_buttom();
    set_up_stop_led();
    struct gpioevent_data edata;  
    read(fd, &edata, sizeof edata); 
    pthread_cancel(stop_thread);
}

void start_program(int fd) {

    
    close(start_button.fd);
    /* Turn Off Stop LED */
    close(stop_led.fd);
    set_up_exit_buttom();
    set_up_start_led();
    run_program = true;    
    pthread_cancel(start_thread);

    #if 0
    if(!run_program){
        run_program = true;    
        close(start_button.fd);
        /* Turn Off Stop LED */
        close(stop_led.fd);
        set_up_exit_buttom();
        set_up_start_led();
        pthread_cancel(start_thread);
    }

    if(run_program){
        run_program = false;
        close(exit_button.fd);
        /* Turn Off Stop LED */
        close(start_led.fd);
        set_up_start_buttom();
        set_up_stop_led();
        struct gpioevent_data edata;  
        read(fd, &edata, sizeof edata); 
        pthread_cancel(stop_thread);      
    }
    #endif

}

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
    

    //set_up_exit_buttom();
    set_up_start_buttom();
    run_program = false;

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

    while(1){

        if(run_program){

            if(new_adc_val)
            {   
                msleep(1000);
                new_adc_val = false;
                ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
                packet.data[0] = rx_buffer[0];
                packet.data[1] = rx_buffer[1];
                new_adc_val = false;
            }

            if(!new_adc_val){

                i++;
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
    intData.func = &stop_program;
    pthread_t intThread;
    if (pthread_create(&stop_thread, NULL, waitInterrupt,
    (void*) &intData)) 
    {
        fprintf(stderr, "Error creating thread\n");
        exit(0);
    }
}

void set_up_start_buttom(void){

    int fd, ret;
    start_button.lineoffset = 26;
    start_button.handleflags = GPIOHANDLE_REQUEST_INPUT;
    start_button.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
    strcpy(exit_button.consumer_label, "Event test");
    fd = open("/dev/gpiochip0", O_RDONLY);
    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &start_button);
    if(ret < 0){
         perror("Det går inte att öppna GPIO!");
         exit(0);
    }
    close(fd);
    
    ev.events = EPOLLIN;
    ev.data.fd = start_button.fd;
    int epfd = epoll_create(1);
    int res = epoll_ctl(epfd, EPOLL_CTL_ADD, start_button.fd, &ev);

    intData.epfd = epfd;
    intData.fd = start_button.fd;
    intData.func = &start_program;
    if (pthread_create(&start_thread, NULL, waitInterrupt,
    (void*) &intData)) 
    {
        fprintf(stderr, "Error creating thread\n");
        exit(0);
    }
}


void set_up_start_led(){
    
    /* Open the gpiochip device file */
	int fd = open("/dev/gpiochip0", O_RDWR);
	if(fd < 0) {
		perror("Error opening gpiochip0");
        exit(0);
	}
	/* Setup oscilloscope signal to output */
	start_led.flags = GPIOHANDLE_REQUEST_OUTPUT;
	strcpy(start_led.consumer_label, "Start LED");
	memset(start_led.default_values, 0, sizeof(start_led.default_values));
	start_led.lines = 1;
	start_led.lineoffsets[0] = 24;

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &start_led) < 0) {
		perror("Error setting GPIO 16 to output");
		close(fd);
        exit(0);
	}
    /* Set EXIT LED */
    data.values[0] = 1;
    if(ioctl(start_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) 
        perror("Error setting GPIO to 1");
}

void set_up_stop_led(){
    
    /* Open the gpiochip device file */
	int fd = open("/dev/gpiochip0", O_RDWR);
	if(fd < 0) {
		perror("Error opening gpiochip0");
        exit(0);
	}
	/* Setup oscilloscope signal to output */
	stop_led.flags = GPIOHANDLE_REQUEST_OUTPUT;
	strcpy(stop_led.consumer_label, "Stop LED");
	memset(stop_led.default_values, 0, sizeof(stop_led.default_values));
	stop_led.lines = 1;
	stop_led.lineoffsets[0] = 25;

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &stop_led) < 0) {
		perror("Error setting GPIO 16 to output");
		close(fd);
        exit(0);
	}
    /* Set EXIT LED */
    data.values[0] = 1;
    if(ioctl(stop_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) 
        perror("Error setting GPIO to 1");
}
