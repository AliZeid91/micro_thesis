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

#define I2C  1
#define SPI  2

bool run_program;

struct gpiohandle_request signal;
struct gpioevent_request exit_button;

static struct epoll_event ev;
intVec intData;


int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}


void time_handler1(size_t timer_id, void * user_data)
{
    printf("Akshata : Single shot timer expired.(%d)\n", timer_id);
}

void myHandler(int fd) {
    run_program = false;
}

void set_up_signal();
void send_signal(struct gpiohandle_request* signal);
void set_up_brake_buttom(void);

int main(void) {
    
    printf("Starting the application...\r\n");

    uint8_t tx_buffer[4]; // Buffer to store the bytes that we write to the I2C device
    uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 
    uint8_t mcp3208_tx[10];
    uint8_t mcp3208_rx[10];

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

    set_up_signal();
    
    set_up_brake_buttom();
    

    uint8_t 	spi_mode = 0;
    uint32_t 	spi_speed = 125000000;

    float mcp3208_chanel = 0.1;
    float xbee_spi_chanel = 0.0;
    int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, spi_speed);
    int xbee_spi_fd = init_spi_xbee(xbee_spi_chanel, spi_mode, spi_speed);

    int BUS = SPI;
    bool get_adc_val = true;

    fd_set write_fd, send_package;
    struct timeval timeout;
    int retval;
    
    /* Initialize the timeout */
        timeout.tv_sec  = 2;       //2 Seconds
        timeout.tv_usec = 0;   


    #if 1
        /* Initialize the file descriptor set. */
        FD_ZERO(&send_package);
        FD_SET(xbee_serial_fd,&send_package);
        FD_ZERO(&write_fd);
        FD_SET(adc1015_fd, &write_fd);
        while(1){
            msleep(200);
            
            if(get_adc_val)
            {
                send_signal(&signal);
                retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                if(FD_ISSET(adc1015_fd,&write_fd))
                {
                    ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
                    get_adc_val = false;
                }
            }
           
            if(!get_adc_val){
                packet.data[0] = 0x4D;
                packet.data[1] = 0x4E;
                retval = select(FD_SETSIZE, NULL, &send_package, NULL, &timeout);
                if(FD_ISSET(xbee_serial_fd,&send_package))
                {
                    /*send read value across bluetooth*/
                    send_package_uart(&packet,xbee_serial_fd);
                    get_adc_val = true; 
                } 
            }  
        } 
    #else 
        /* Initialize the file descriptor set. */
        FD_ZERO(&send_package);
        FD_SET(xbee_spi_fd,&send_package);

        FD_ZERO(&write_fd);
        FD_SET(mcp3208_fd, &write_fd);
        while(0){
            send_signal(&signal);
            if(get_adc_val)
            {
                /*Read pedal value & store in rx_buffer */
                retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                if(FD_ISSET(mcp3208_fd,&write_fd))
                {
                    tx_buffer[0]= 0b00000110;
                    tx_buffer[1]= 0b00000000;
                    tx_buffer[2]= 0x00;
                    spi_mcp3208_rx(mcp3208_fd, tx_buffer,rx_buffer); 
                    get_analog_value(rx_buffer);
                    get_adc_val = false;
                }
            }

            msleep(1000);

            if(!get_adc_val){
                packet.data[0] = 0x4D;
                packet.data[1] = 0x4E;
                retval = select(FD_SETSIZE, NULL, &send_package, NULL, &timeout);

            if(FD_ISSET(xbee_spi_fd,&send_package))
            {
                /*send read value across bluetooth*/
                send_package_spi(&packet,xbee_spi_fd);
                get_adc_val = true;  
            }
            msleep(2);
        }    
    }

    #endif
    

    close_serial_port();
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

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &signal) < 0) {
		perror("Error setting GPIO 16 to output");
		close(fd);
		exit(0);
	}
}

void send_signal(struct gpiohandle_request* signal){
    struct gpiohandle_data data;
    /* Send a puls */
	data.values[0] = 1;
	if(ioctl(signal->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) 
		perror("Error setting GPIO to 1");

    usleep(2);

    data.values[0] = 0;
	if(ioctl(signal->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) 
		perror("Error setting GPIO to 1");
}

void set_up_brake_buttom(void){
    run_program = true;
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
    printf("ska starta en thread \n");
    if (pthread_create(&intThread, NULL, waitInterrupt,
    (void*) &intData)) 
    {
        fprintf(stderr, "Error creating thread\n");
        exit(0);
    }
}