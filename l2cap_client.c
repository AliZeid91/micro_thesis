#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
//---hci commands and events
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
//------
//Stopwatch: 
#include <time.h>
#include "keybord_int.c"
#include <errno.h>

//Read from ADC:
#include "i2c.c"
#include "ads1015.c"
#include "mcp3208.c"
#include "spi.c"
//#include "ads1015.c"

#define I2C  1
#define SPI  2

#define ADC1015_I2C_PORT_PATH   0x48         // Address of our adc converter on the I2C bus
#define I2C_BUS_FILE_DESCRIPTOR 1



//prototypes
int delay_ms(unsigned int tms);

//Global vars
int adc_fd;             // Device-Handle 
int ads_addr = 0x48;    // Address of our adc converter on the I2C bus
int i2c_port = 1;
int16_t adcVal;
uint8_t adcWriteBuf[3];
uint8_t adcReadBuf[2];  //2 byte reading from i2c device


/********************************************
 * MAIN FUNCTION 
 *******************************************/
int main(int argc, char **argv)
{
    //adc_fd = i2c_open(i2c_port, ads_addr);
    //init_adc(adc_fd, adcWriteBuf, adcReadBuf);
    
    struct sockaddr_l2 addr = { 0 };
    int s, status;
    char dest[18] = "B8:27:EB:EE:46:E3"; //"C8:94:02:6D:D5:84" <-laptop; //B8:27:EB:EE:46:E3 <-curaspi, jacob -> 50:76:AF:5C:D7:A2

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // set the connection parameters (who to connect to)
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(0x1001);    //kind of port number.
    str2ba( dest, &addr.l2_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if( status < 0 ) {
        printf("%d %s\n", errno, strerror(errno));
        printf("No connection :(\n");
        //close(s);
        printf("TsdsdsdHIS IS ALI\n");
    } 
    else {
        int i = 0;
        uint8_t tx_buffer[256]; // Buffer to store the bytes that we write to the I2C device
        uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 
        /* Device-Handle */
        int adc1015_fd;  
        fd_set read_fd, write_fd;
        struct timeval timeout;
        int retval; 

        adc1015_fd  = i2c_open(I2C_BUS_FILE_DESCRIPTOR,ADC1015_I2C_PORT_PATH); 
        init_adc(adc1015_fd,tx_buffer,rx_buffer);

        uint8_t 	spi_mode = 0;
        uint32_t 	spi_speed = 500000;

        int mcp3208_chanel = 1;
        int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, spi_speed);

        int BUS = SPI;
        
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

        /* Initialize the timeout */
        timeout.tv_sec  = 2;       //2 Seconds
        timeout.tv_usec = 0;

        changemode(1);  //bbfb
        while (!kbhit()) { 
            printf("%d| \n", i);
            switch (BUS)
            {
                case I2C:
                    /* Initialize the timeout */
                    timeout.tv_sec  = 2;       //2 Seconds
                    timeout.tv_usec = 0;
                    retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                    if(FD_ISSET(adc1015_fd,&write_fd))
                    {
                        ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
                    }
                    break;
                case SPI:
                    retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                    if(FD_ISSET(mcp3208_fd,&write_fd))
                    {
                        tx_buffer[0]= 0b00000110;
                        tx_buffer[1]= 0b00000000;
                        tx_buffer[2]= 0x00;
                        spi_mcp3208_rx(mcp3208_fd, tx_buffer,rx_buffer); 
                        get_analog_value(rx_buffer);
                    }
                    break;

            }
            //read_adc(adc_fd, adcWriteBuf, adcReadBuf);  //Read pedal value & store in adcReadBuf
            if(BUS == SPI)
            {
                write(s, rx_buffer , 2);  //send read value across bluetooth:
            }else{
                write(s, rx_buffer , 3);  //send read value across bluetooth:
            }
           
            //write(s, "hello\0", 6);
            delay_ms(200);
            // printf("Sent message %d\n", i);
            i++;
        }
        changemode(0);
        
    }
    
    close(s);
    
    #if 0
    //ADC TEST:
    changemode(1);
        while (!kbhit()) { 
            read_adc(adc_fd, adcWriteBuf, adcReadBuf); 
            delay_ms(100);
        }
    changemode(0);
    #endif
}

/********************************************
 * delay_ms FUNCTION 
 * Produces a delay of tms milliseconds
 *******************************************/
int delay_ms(unsigned int tms) {
  return usleep(tms * 1000);
}

