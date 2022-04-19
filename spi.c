#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>


#include "spi.h"
 
int spi_open(float spi_chanal, uint8_t mode, uint32_t speed)
{
    
	int fd;
	int bits_per_word = 8;
	char  device[32];
    
	/* Open the SPI bus file descriptor */
	sprintf(device, "/dev/spidev%.1f", spi_chanal);;
	if ((fd = open(device, O_RDWR)) < 0)
   	{
		perror("Error opening SPI Bus");
      	return -1;
  	}
	
	/* Setup of the SPI Bus */
	if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
		perror("Error setting the SPI mode");
		close(fd);
		return -1;
	}
    
	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		perror("Error setting the SPI speed");
		close(fd);
		return -1;
	}
	if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
		perror("Error setting the SPI wordlength");
		close(fd);
		return -1;
	}
 
    return fd;
}



/**
 * @brief function makes a SPI transfer
 *
 * @param fd		File descriptor to SPI bus
 * @param data		Data array with output (write) data
 *					will be overwritten with the received input data
 * @param length	Length of the data array
 */
void spi_transfer_1(int fd, uint8_t *data, int length, uint32_t speed){
	
	struct spi_ioc_transfer spi[length];	

	/* Setup transfer struct */
	for (int i=0; i<length; i++) {
		memset(&spi[i], 0, sizeof(struct spi_ioc_transfer));
		spi[i].tx_buf = (unsigned long) (data+i);
		spi[i].rx_buf = (unsigned long) (data+i);
		spi[i].len = 1;
		spi[i].speed_hz = speed;
		spi[i].bits_per_word = 8;
	}

	/* Transfer */
	int ret = ioctl(fd, SPI_IOC_MESSAGE(length), spi);
	if(ret < 0) {
		perror("Error transfering data over SPI bus");
		close(fd);
		exit(-1);
	}

}
#if 1
int spi_transfer(int fd, char *txBuf, char *rxBuf, unsigned count)
{
	int ret;
	int speed = 500000;
	struct spi_ioc_transfer spi;

	memset(&spi, 0, sizeof(spi));

	spi.tx_buf        = (unsigned long) txBuf;
	spi.rx_buf        = (unsigned long) rxBuf;
	spi.len           = count;
	spi.speed_hz      = speed;
	spi.delay_usecs   = 0;
	spi.bits_per_word = 8;
	spi.cs_change     = 0;

	/* Transfer */
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);
	if(ret < 0) 
	{
		perror("Error transfering data over SPI bus");
		close(fd);
		exit(-1);
	}

	return ret;
}
#endif


int spi_close(int fd)
{
   return close(fd);
}

