
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "uart.h"

int set_baud_rate(const int baud);


int serial_open (const char *device, const int baud)
{
    struct termios options ;
    speed_t myBaud ;
    int     status, fd ;

    myBaud = set_baud_rate(baud);

    if ((fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1)
        return -1 ;

    fcntl (fd, F_SETFL, O_RDWR) ;

    /* Get and modify current options: */

    if(tcgetattr(fd, &options)) {
        printf("Something went wrong while getting port attributes...\r\n");
        exit(EXIT_FAILURE);
    }

    cfmakeraw   (&options) ;
    /* Set in/out baud rate */
    cfsetispeed (&options, myBaud) ;
    cfsetospeed (&options, myBaud) ;

    /* Enable the receiver */
    options.c_cflag |= (CLOCAL | CREAD) ; 
    /* Clear parity bit, disabling parity */
    options.c_cflag &= ~PARENB ; 
    /* Clear stop field, only one stop bit used in communication */
    options.c_cflag &= ~CSTOPB ;  
    /* Clear all bits that set the data size */
    options.c_cflag &= ~CSIZE ;   
    /* 8 bits per byte */
    options.c_cflag |= CS8 ;     
    /* set local mode */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;  
    options.c_oflag &= ~OPOST ;
    /* Disable RTS/CTS hardware flow control */
    options.c_cflag &= ~CRTSCTS; // 


    options.c_cc [VMIN]  = 0 ;
    options.c_cc [VTIME] = 1 ;	// 50 milliseconds (1/2 deciseconds)
    tcflush(fd, TCIFLUSH);

    /* Save tty settings, also checking for error */
    if (tcsetattr(fd, TCSANOW, &options)  != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    ioctl (fd, TIOCMGET, &status);

    status |= TIOCM_DTR ;
    status |= TIOCM_RTS ;

    ioctl (fd, TIOCMSET, &status);

    usleep (10000) ;	// 10mS

    return fd ;
}


int set_baud_rate(const int baud){
    unsigned int myBaud;
    switch (baud)
    {
        case      50:	myBaud =      B50 ; break ;
        case      75:	myBaud =      B75 ; break ;
        case     110:	myBaud =     B110 ; break ;
        case     134:	myBaud =     B134 ; break ;
        case     150:	myBaud =     B150 ; break ;
        case     200:	myBaud =     B200 ; break ;
        case     300:	myBaud =     B300 ; break ;
        case     600:	myBaud =     B600 ; break ;
        case    1200:	myBaud =    B1200 ; break ;
        case    1800:	myBaud =    B1800 ; break ;
        case    2400:	myBaud =    B2400 ; break ;
        case    4800:	myBaud =    B4800 ; break ;
        case    9600:	myBaud =    B9600 ; break ;
        case   19200:	myBaud =   B19200 ; break ;
        case   38400:	myBaud =   B38400 ; break ;
        case   57600:	myBaud =   B57600 ; break ;
        case  115200:	myBaud =  B115200 ; break ;
        case  230400:	myBaud =  B230400 ; break ;
        case  460800:	myBaud =  B460800 ; break ;
        case  500000:	myBaud =  B500000 ; break ;
        case  576000:	myBaud =  B576000 ; break ;
        case  921600:	myBaud =  B921600 ; break ;
        case 1000000:	myBaud = B1000000 ; break ;
        case 1152000:	myBaud = B1152000 ; break ;
        case 1500000:	myBaud = B1500000 ; break ;
        case 2000000:	myBaud = B2000000 ; break ;
        case 2500000:	myBaud = B2500000 ; break ;
        case 3000000:	myBaud = B3000000 ; break ;
        case 3500000:	myBaud = B3500000 ; break ;
        case 4000000:	myBaud = B4000000 ; break ;

        default:
        return -2 ;
    }
    return myBaud;
}


void serial_write(int fd, unsigned char *package_arr){
    write(fd,package_arr,17);
}

int serial_read(int fd, unsigned char *rx_buff){
    return read(fd,rx_buff,256);
}
