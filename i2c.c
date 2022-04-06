
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h> 
#include <stdio.h>


#include <stdio.h>

int i2c_open(int i2c_channal, int ads_addr){
    int i2c_fd;
    char  i2c_device[32];

	/* Open the I2C bus file descriptor */
	sprintf(i2c_device, "/dev/i2c-%d", i2c_channal);
    /*open the i2c device*/
    if ((i2c_fd = open( "/dev/i2c-1", O_RDWR)) < 0)
    {
        printf("Error: Couldn't open device! %d\n", i2c_fd);
        return 1;
    }
    /* connect to ads1015 as i2c slave */
     if (ioctl(i2c_fd, I2C_SLAVE, ads_addr) < 0)
    {
        printf("Error: Couldn't find device on address!\n");
        return 1;
    }
    return i2c_fd;
}