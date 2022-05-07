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


#include "timer.c"
#include "stopwatch.c"
#include "mcp3208.c"

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>

#define SERIAL_PORT_BAUD_RATE 230400

#define XBEE_SERIAL_PORT_PATH  "/dev/serial0"  //"/dev/ttyUSB0" "/dev/serial0" Address of our Xbee on the UART bus
#define ADC1015_I2C_PORT_PATH   0x48         // Address of our adc converter on the I2C bus

#define I2C_BUS_FILE_DESCRIPTOR 1

#define MAX_BYTES_TO_READ	10

#define DEV_NAME "/dev/gpiochip0"

#define I2C  1
#define SPI  2

bool run_program;

struct gpiohandle_request signal, cts;
struct gpioevent_request exit_button;
struct gpiohandle_data data;

static struct epoll_event ev;
intVec intData;


int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

void myHandler(int fd) {
    run_program = false;
}

void time_handler1(size_t timer_id, void * user_data)
{
    run_program = false;
}
void set_up_signal();
void send_signal(struct gpiohandle_request* signal);
void set_up_exit_buttom(void);
void set_up_cts_gpio();
int get_cts_status();

int main(void) {
    
    printf("Starting the application...\r\n");

    uint8_t tx_buffer[4]; // Buffer to store the bytes that we write to the I2C device
    uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 
    uint8_t mcp3208_tx[10];
    uint8_t mcp3208_rx[10];

    int package_length = 13;
    int package_frame_type = 0x00;      
    int package_frame_id = 0x8B; 
    unsigned char package_address[9] = {0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0x87}; //  0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0x87 
    create_xbee_packet(package_length,package_frame_type,package_frame_id,package_address);

    int adc1015_fd;             // Device-Handle 
    int xbee_serial_fd;
    adc1015_fd  = i2c_open(I2C_BUS_FILE_DESCRIPTOR,ADC1015_I2C_PORT_PATH);
    xbee_serial_fd = serial_open(XBEE_SERIAL_PORT_PATH,SERIAL_PORT_BAUD_RATE);
    msleep(1);
    init_adc(adc1015_fd,tx_buffer,rx_buffer);


    uint8_t 	spi_mode = 0;
    uint32_t 	spi_speed = 125000000;

    float mcp3208_chanel = 1.0;
    float xbee_spi_chanel = 0.0;
    int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, 50000);


    bool new_adc_val = true;
    long time_spi;

    fd_set write_fd, send_package;
    struct timeval timeout;
    int retval;

    set_up_signal();
    set_up_cts_gpio();
    bool tx_status_received;

    struct timespec time;
    int i=0;
    int nbr_of_tests = 0;
    run_program = true;
    while (nbr_of_tests<500)
    {   
        //save(test);
        size_t timer1;
        initialize();
        timer1 = start_timer(1000, time_handler1, TIMER_SINGLE_SHOT, NULL); 
        while(run_program){
            /* Initialize the file descriptor set. */
            FD_ZERO(&send_package);
            FD_SET(xbee_serial_fd,&send_package);

            #if 0
            FD_ZERO(&write_fd);
            FD_SET(adc1015_fd, &write_fd);    
            if(new_adc_val)
            {
                /* Initialize the timeout */
                timeout.tv_sec  = 2;       //2 Seconds
                timeout.tv_usec = 0; 
                retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                if(FD_ISSET(adc1015_fd,&write_fd))
                {
                    ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
                    new_adc_val = false;
                }
            }
            #else 
            FD_ZERO(&write_fd);
            FD_SET(mcp3208_fd, &write_fd);    
            if(new_adc_val)
            {
                /* Initialize the timeout */
                timeout.tv_sec  = 2;       //2 Seconds
                timeout.tv_usec = 0; 
                /*Read pedal value & store in rx_buffer */
                retval = select(mcp3208_fd +100, NULL, &write_fd, NULL, &timeout);
                if(FD_ISSET(mcp3208_fd,&write_fd))
                {
                    tx_buffer[0]= 0b00000110;
                    tx_buffer[1]= 0b00000000;
                    tx_buffer[2]= 0x00;
                    spi_mcp3208_rx(mcp3208_fd, tx_buffer,rx_buffer); 
                    new_adc_val = false;
                }
            }
            #endif
            if(!new_adc_val){
                if(i%10==0){
                    packet.data[0] = 0x41;
                    packet.data[1] = 0x42;
                }
                else if(i%10==1){
                    packet.data[0] = 0x43;
                    packet.data[1] = 0x44; 
                }
                else if(i%10==2){
                    packet.data[0] = 0x45;
                    packet.data[1] = 0x46; 
                }
                else if(i%10==3){
                    packet.data[0] = 0x47;
                    packet.data[1] = 0x48; 
                }
                else if(i%10==4){
                    packet.data[0] = 0x49;
                    packet.data[1] = 0x50;                
                }
                else if(i%10==5){
                    packet.data[0] = 0x51;
                    packet.data[1] = 0x52;                  
                }
                else if(i%10==6){
                    packet.data[0] = 0x53;
                    packet.data[1] = 0x54; 
                }
                else if(i%10==7){
                    packet.data[0] = 0x55;
                    packet.data[1] = 0x56; 
                }
                else if(i%10==8){
                    packet.data[0] = 0x57;
                    packet.data[1] = 0x58; 
                }
                else if(i%10==9){
                    packet.data[0] = 0x59;
                    packet.data[1] = 0x5A;                
                }
                else {
                    packet.data[0] = 0x5B;
                    packet.data[1] = 0x5C;                   
                }

                if(get_cts_status() < 1){
                    send_package_uart(&packet,xbee_serial_fd);
                    new_adc_val = true;
                   // usleep(175);
                    i++;  
                }
            }  
        }
        send_signal(&signal);
        msleep(1000);
        stop_timer(timer1);
        run_program =true;
        nbr_of_tests +=1;
        printf("Antalet Skickade Packet: %d\n",i);
        i=0;
    }
    close(adc1015_fd);
    close(xbee_serial_fd);
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

void set_up_cts_gpio(){
    int fd;

	/* Open the gpiochip device file */
	fd = open("/dev/gpiochip0", O_RDWR);
	if(fd < 0) {
		perror("Error opening gpiochip0");
		exit(0);
	}

	/* Setup cts to input */
	cts.flags = GPIOHANDLE_REQUEST_INPUT;
	strcpy(cts.consumer_label, "CTS");
	memset(cts.default_values, 0, sizeof(cts.default_values));
	cts.lines = 1;
	cts.lineoffsets[0] = 23;
    cts.default_values[0] = 0; 

	if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &cts) < 0) {
		perror("Error setting GPIO 23 to input");
		close(cts.fd);
		close(fd);
		exit(0);
	}
}

int get_cts_status(){

    /* Reading button's state */
	if(ioctl(cts.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) 
		perror("Error setting GPIO to 1");
    return data.values[0];
}