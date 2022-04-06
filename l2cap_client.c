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
#include "kbhit.c"
#include <errno.h>

//Read from ADC:
#include "i2c.c"
//#include "ads1015.c"

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
    } 
    else {
        int i = 0;
        changemode(1);  //bbfb
        while (!kbhit()) { 
            printf("%d| \n", i);
            //read_adc(adc_fd, adcWriteBuf, adcReadBuf);  //Read pedal value & store in adcReadBuf
            //write(s, adcReadBuf , sizeof(adcReadBuf));  //send read value across bluetooth:
            write(s, "hello\0", 6);
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

