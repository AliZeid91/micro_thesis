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

#include <linux/gpio.h>
#include <sys/poll.h>

#include <syscall.h>  
#include <sched.h>
#include <stdbool.h>
//Stopwatch: 
#include <time.h>
#include "keybord_int.c"
#include <errno.h>
#include <assert.h>
//Read from ADC:
#include "i2c.c"
#include "ads1015.c"
#include "mcp3208.c"
#include "spi.c"
#include "stopwatch.c"
#include "timer.c"
#define I2C  1
#define SPI  2

#define ADC1015_I2C_PORT_PATH   0x48         // Address of our adc converter on the I2C bus
#define I2C_BUS_FILE_DESCRIPTOR 1

int s, status;
bool run_program;
fd_set write_fd;
struct timeval timeout;
uint8_t tx_buffer[4]; // Buffer to store the bytes that we write to the I2C device
uint8_t rx_buffer[256];   // byte buffer to store the data read from the I2C device 

struct gpiohandle_request signal;

//prototypes
int delay_ms(unsigned int tms);

int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

void time_handler1(size_t timer_id, void * user_data)
{
    //run_program = false;
}
//Global vars
int i = 0;
int adc_fd;             // Device-Handle 
int ads_addr = 0x48;    // Address of our adc converter on the I2C bus
int i2c_port = 1;
int16_t adcVal;
uint8_t adcWriteBuf[3];
uint8_t adcReadBuf[2];  //2 byte reading from i2c device

void set_up_signal();
void send_signal(struct gpiohandle_request* signal);
void runSelectFunc();
 void set_paket_data(int i);
/********************************************
 * MAIN FUNCTION 
 *******************************************/
int main(int argc, char **argv)
{
    //adc_fd = i2c_open(i2c_port, ads_addr);
    //init_adc(adc_fd, adcWriteBuf, adcReadBuf);
    
    struct sockaddr_l2 addr = { 0 };

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
        

        /* Device-Handle */
        int adc1015_fd;  
        fd_set read_fd, write_fd;
        int retval; 

        adc1015_fd  = i2c_open(I2C_BUS_FILE_DESCRIPTOR,ADC1015_I2C_PORT_PATH); 
        init_adc(adc1015_fd,tx_buffer,rx_buffer);
        
        bool new_adc_val = true;
        uint8_t 	spi_mode = 0;
        uint32_t 	spi_speed = 125000000;

        float mcp3208_chanel = 1.0;
        float xbee_spi_chanel = 0.0;
        int mcp3208_fd = init_spi_mcp3208(mcp3208_chanel, spi_mode, 100000);
        
        /* Initialize the file descriptor set. */
        FD_ZERO(&write_fd);
        FD_SET(adc1015_fd, &write_fd);

        /* Initialize the timeout */
        timeout.tv_sec  = 2;       //2 Seconds
        timeout.tv_usec = 0;

        set_up_signal();
        const struct sched_param priority = {1};

        if (sched_setscheduler(getpid(), SCHED_FIFO, &priority) < 0 )
            fprintf(stderr, "SETSCHEDULER failed - err = %s\n", strerror(errno));
        else
            printf("Priority set to \"%d\"\n", priority);
            struct timespec time;
            int nbr_of_tests = 0;
            run_program = true;
            while (nbr_of_tests<5)
            {      
                    send_signal(&signal);
                    #if 0
                    retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                    if(FD_ISSET(adc1015_fd,&write_fd))
                    {
                        ads1015_rx(adc1015_fd,tx_buffer,rx_buffer);
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
                            retval = select(FD_SETSIZE, NULL, &write_fd, NULL, &timeout);
                            if(FD_ISSET(mcp3208_fd,&write_fd))
                            {
                                tx_buffer[0]= 0b00000110;
                                tx_buffer[1]= 0b00000000;
                                tx_buffer[2]= 0x00;
                                spi_mcp3208_rx(mcp3208_fd, tx_buffer,rx_buffer); 
                                //get_analog_value(rx_buffer);
                                new_adc_val = false;
                            }
                        }
                    #endif

                    if(!new_adc_val){

                        set_paket_data(i);
                        //write(s, rx_buffer , 3);  //send read value across bluetooth:
                        runSelectFunc();
                        run_program =false;
                        i++;
                        new_adc_val = true;
                    } 
                //usleep(500);
                msleep(5);
                run_program =true;
                nbr_of_tests +=1;
            }
        }
    printf("Antalet Skickade Packet: %d\n",i);
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

void runSelectFunc() {
    bool send_data = true;   
    while(send_data){   
        #if 1 
        /* Initialize the file descriptor set. */
        FD_ZERO(&write_fd);
        FD_SET(s, &write_fd);
            
        /* Initialize the timeout */

        timeout.tv_sec  = 2;       //2 Seconds
        timeout.tv_usec = 0;
        int retval = select(s+100, NULL, &write_fd, NULL, &timeout); //&timeout
            
        if( retval < 0 ) 
        {
            perror("select");
            assert(0);
        }else if(retval == 0)
        {
            printf("No data is waiting\n");
        }
        
        if(FD_ISSET(s,&write_fd))
        {
            //send read value across bluetooth:
            if(write(s, rx_buffer , 3) == 3){
                send_data = false;
            }
        }
        #else
            int buffer_length = read(client, &btReadBuf, 2);
            if(buffer_length == 2){
                //printf("Recived data: %s\n",btReadBuf);
                read_data = false;
            }
        #endif
    }
 }

 void set_paket_data(int i){
    if(i%10==0){
        rx_buffer[0] = 0x41;
        rx_buffer[1] = 0x42;
        rx_buffer[2] = 0x43;
    }
    else if(i%10==1){
        rx_buffer[0] = 0x43;
        rx_buffer[1] = 0x44; 
        rx_buffer[2] = 0x43;
    }
    else if(i%10==2){
        rx_buffer[0] = 0x45;
        rx_buffer[1] = 0x46; 
        rx_buffer[2] = 0x43;
    }
    else if(i%10==3){
        rx_buffer[0] = 0x47;
        rx_buffer[1] = 0x48; 
        rx_buffer[2] = 0x43;
    }
    else if(i%10==4){
        rx_buffer[0] = 0x49;
        rx_buffer[1] = 0x50; 
        rx_buffer[2] = 0x43;               
    }
    else if(i%10==5){
        rx_buffer[0] = 0x51;
        rx_buffer[1] = 0x52;  
        rx_buffer[2] = 0x43;                
    }
    else if(i%10==6){
        rx_buffer[0] = 0x53;
        rx_buffer[1] = 0x54;
        rx_buffer[2] = 0x43; 
    }
    else if(i%10==7){
        rx_buffer[0] = 0x55;
        rx_buffer[1] = 0x56; 
        rx_buffer[2] = 0x43;
    }
    else if(i%10==8){
        rx_buffer[0] = 0x57;
        rx_buffer[1] = 0x58; 
        rx_buffer[2] = 0x43;
    }
    else if(i%10==9){
        rx_buffer[0] = 0x59;
        rx_buffer[1] = 0x5A; 
        rx_buffer[2] = 0x43;               
    }
    else {
        rx_buffer[0] = 0x5B;
        rx_buffer[1] = 0x5C;
        rx_buffer[2] = 0x43;                   
    }
 }

