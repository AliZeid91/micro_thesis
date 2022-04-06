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

#include <time.h>
#include "kbhit.c"
#include <errno.h>

#include <assert.h>

// #include "timer.c"
#include "stopwatch.c"

//prototypes:
int delay_ms(unsigned int tms);
void runSelectFunc();

//global viariables:
//Select function parameters:
int s;
unsigned char  buffer[256];
fd_set read_fd, write_fd;
struct timeval timeout;
int retval;
int buffer_index;
int ret;

int success;
int count;

/********************************************
 * MAIN FUNCTION 
 *******************************************/
int main(int argc, char **argv)
{
    struct sockaddr_l2 addr = { 0 };
    //int s, status;
    int status;
    char *message = "hello!\0";
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
        #if 0
        #if 0
        write(s, message, 6);
        delay_ms(100);
        read(s, readbuff, sizeof(readbuff));
        printf("Read: %s\n", readbuff);
        #endif

        //---LATENCY TEST: Measure time - stopwatch: ---
        long timeElapsedms;
        int i = 0;
        while (i<10) {  //i<100
            struct timespec varTime = timer_start();
            write(s, message, 6);
            delay_ms(100);
            timeElapsedms = timer_end(varTime);
            printf("Time taken in ms =  %d\n", timeElapsedms);
            i++;
        }
        #endif

        // changemode(1);
        int i = 1;
        while (i<50) {  
            write(s, message, 6);
            //if (write(s, message, 6) >= 0) count++;
            delay_ms(100);
            printf("%d| ", i);
            //runSelectFunc();

            // //---RELIABILITY TEST: RSSI: ---
            // int8_t *rssi;
            // hci_read_rssi(s, 0x000b, rssi, 1000);
            // printf("RSSI read: %d\n", i, *rssi);
            i++;
        }
        // changemode(0);
    }  
    close(s);

    // //---RELIABILITY TEST: Packet loss ratio: ---
    //Wait for x ms to send packet and get a acknowledgement back
        //If packet got back - success++,
        //Packet loss = total packets/(total packets - successful packets)
    printf("Packets sent: %d\n", count);
    printf("Packets acked: %d\n", success);     //Get ack from server in selct function


}

/********************************************
 * delay_ms FUNCTION 
 * Produces a delay of tms milliseconds
 *******************************************/
int delay_ms(unsigned int tms) {
  return usleep(tms * 1000);
}

/********************************************
 * runSelectFunc FUNCTION 
 * 
 *******************************************/
void runSelectFunc() {
    puts("Starting Select...");
            
    /* Initialize the file descriptor set. */
    FD_ZERO( &read_fd );
    FD_SET(s, &read_fd);
        
        
    /* Initialize the timeout */
    timeout.tv_sec  = 2;       //2 Seconds
    timeout.tv_usec = 0;
    retval = select(FD_SETSIZE, &read_fd, NULL, NULL, &timeout);
        
    if( retval < 0 ) 
    {
        perror("select");
        assert(0);
    }else if(retval == 0)
    {
        printf("No data is waiting\n");
    }

    if(FD_ISSET(s, &read_fd))
    {
        //Stop timer here - ?
        success++;
        int buffer_length = read(s, &buffer, sizeof(buffer));
        printf("Input buffer contents: ");
        for(buffer_index=0;buffer_index < buffer_length;buffer_index++)
        {
            //process(buffer[buffer_index]);
            printf(" %x", buffer[buffer_index]);
            //41 or 41 5C 30
        }
        printf("\n");
    }
}

