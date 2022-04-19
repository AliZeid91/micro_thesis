
#include <inttypes.h>
#include "spi.h"
 
static int V_REF  =  5;

static int rx_length = 3;


int init_spi_mcp3208(float spi_chanal, uint8_t mode, uint32_t speed){
	return spi_open(spi_chanal, mode, speed);
}

float get_analog_value(uint8_t *rx_buffer){
   	printf("Received SPI buffer...\r\n");
	rx_buffer[1]&=0b00001111;
	int result = (rx_buffer[1]<<8)|rx_buffer[2];
	printf("Voltage Reading %f V \n", (float)(5*result)/4096.0);
  	printf("\r\n");
   	return result;
}

void spi_mcp3208_rx(int fd, uint8_t *tx_buffer, uint8_t *rx_buffer){
	spi_transfer(fd,tx_buffer,rx_buffer,rx_length);
}

