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
//#include "xbee.c"
#include "ads1015.c"
#include "spi.c"
//#include "xbee_uart.c"
//#include "xbee_spi.c"
#include "uart.c"
#include "i2c.c"
//#include "xbee_process.c"
#include "stopwatch.c"

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


int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

void time_handler1(size_t timer_id, void * user_data)
{
    printf("Akshata : Single shot timer expired.(%d)\n", timer_id);
}

int main(void) {
 
    printf("Starting the application...\r\n");
    
    uint8_t tx_buffer[4]; // Buffer to store the bytes that we write to the I2C device
    uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 
    uint8_t mcp3208_tx[4];
    uint8_t mcp3208_rx[4];
    
    int adc1015_fd;             // Device-Handle 
    int xbee_serial_fd;
    adc1015_fd  = i2c_open(I2C_BUS_FILE_DESCRIPTOR,ADC1015_I2C_PORT_PATH);
    
    init_adc(adc1015_fd,tx_buffer,rx_buffer);


    uint8_t 	spi_mode = 0;
    uint32_t 	spi_speed = 125000000;

    int mcp3208_chanel = 1;
    int xbee_spi_chanel = 0;
    int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, spi_speed);


    int BUS = SPI;

    fd_set write_fd;
    struct timeval timeout;
    int retval;
    
    /* Initialize the timeout */
        timeout.tv_sec  = 2;       //2 Seconds
        timeout.tv_usec = 0;

    /* Initialize the file descriptor set. */
    if(BUS == I2C)
    {
        FD_ZERO(&write_fd);
        FD_SET(adc1015_fd, &write_fd);
    }else
    {
        FD_ZERO(&write_fd);
        FD_SET(mcp3208_fd, &write_fd);
    }    

    long test[100000];
    for(int i=0;i<10;i++){
        #if 0
        /* Test for I2C */
        retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
        if(FD_ISSET(adc1015_fd,&write_fd))
        {
            struct timespec time = timer_start();  // begin a timer'time'
            ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
            long time_elapsed_nanos = timer_end(time); 
            test[i] = time_elapsed_nanos;
            printf("Time taken (milliseconds): %ld\n", test[i]);
        }
        #else
        retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
        if(FD_ISSET(mcp3208_fd,&write_fd))
        {
            struct timespec time = timer_start();  // begin a timer'time'
            mcp3208_tx[0]= 0b00000110;
            mcp3208_tx[1]= 0b00000000;
            mcp3208_tx[2]= 0x00;
            spi_mcp3208_rx(mcp3208_fd, mcp3208_tx,mcp3208_rx);
            get_analog_value(mcp3208_rx);
            long time_elapsed_nanos = timer_end(time); 
            test[i] = time_elapsed_nanos;
            printf("Time taken (milliseconds): %d\n", test[i]);
        }
        #endif 
    }      
    close_serial_port();
    return 0;
}

