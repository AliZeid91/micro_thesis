
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/i2c-dev.h> 
#include <sys/ioctl.h>

#define SERIAL_PORT_PATH   "/dev/ttyUSB0"
struct termios g_tty;
int uart_fd;

// FILE OPERATION
static int file_open_and_get_descriptor(const char *fname) {
    int fd;

    fd = open(fname, O_RDWR );
    if(fd < 0) {
        printf("Could not open file %s...%d\r\n",fname,fd);
    }
    return fd;
}
static void open_serial_port(char serial_port_path[]) {
    uart_fd = file_open_and_get_descriptor(serial_port_path);
    if(uart_fd < 0) {
        printf("Something went wrong while opening the serial port...\r\n");
        exit(EXIT_FAILURE);
    }
}

static void configure_serial_port(void) {
    if(tcgetattr(uart_fd, &g_tty)) {
        printf("Something went wrong while getting port attributes...\r\n");
        exit(EXIT_FAILURE);
    }

    g_tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity 
    g_tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication 
    g_tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    g_tty.c_cflag |= CS8; // 8 bits per byte 
    g_tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control 
    
    /*
 * Enable the receiver and set local mode...
 */

    //g_tty.c_cflag |= (CLOCAL | CREAD);

    //g_tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    g_tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    g_tty.c_cc[VMIN]  == 10 || g_tty.c_cc[VMIN]  == 11;

    // Set in/out baud rate to be 9600
    cfsetispeed(&g_tty, B9600);
    cfsetospeed(&g_tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(uart_fd, TCSANOW, &g_tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
}


int init_serial_port(char serial_port_path[]){

    open_serial_port(serial_port_path);

    configure_serial_port();

    return uart_fd;
}




static int file_write_data(int fd, uint8_t *buff, uint32_t len_buff) {
    return write(fd,buff,len_buff);
}
static int file_read_data(int fd, uint8_t *buff, uint32_t len_buff) {
    return read(fd,buff,len_buff);
}
static int file_close(int fd) {
    return close(fd);
}

static void close_serial_port(void) {
    file_close(uart_fd);
}



int serialDataAvail (const int fd)
{
  int result ;

  if (ioctl (fd, FIONREAD, &result) == -1)
    return -1 ;

  return result ;
}