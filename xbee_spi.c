#include <stdint.h>

#include "spi.h"


int init_spi_xbee(float spi_chanal, uint8_t mode, uint32_t speed){
	return spi_open(spi_chanal, mode, speed);
}


void send_package_spi(struct xBeePacket* p,int fd){

	uint8_t tx_buffer[p->length+4];
	uint8_t rx_buffer[p->length+4];
	for(int i=0;i<p->length+4;i++){
		rx_buffer[i] = 0xff;
	}
   	preparing_package(p, tx_buffer);
	spi_transfer(fd,tx_buffer,rx_buffer,p->length+4);
}


int spi_xbee_rx(int fd, unsigned char *rx_buffer,int length){
	int nbr_of_recived_byte;

	uint8_t tx_buffer[length];

	for(int i=0;i<11;i++){
		rx_buffer[i] = 0xff;
		tx_buffer[i] = 0xff;
	}
	nbr_of_recived_byte = spi_transfer(fd, tx_buffer,rx_buffer,length);
	return nbr_of_recived_byte;
}


#if 0
void send_package_spi(struct xBeePacket* p,int fd, uint32_t speed){

    char package[17]; 
   
    preparing_package(p, package);
    spi_xbee_tx(fd, package, 17, speed);
}
#endif