
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>


struct xBeePacket{
    unsigned int length;
    unsigned char frame_type;
    unsigned char frame_id;
    unsigned char dest_address[9];
    unsigned char options;
    unsigned char data[255];  // Length 
    unsigned char package[17];
}packet;


void create_xbee_packet(unsigned char length, unsigned char frame_type, unsigned char frame_id, unsigned char *address){
    packet.length = length;
    packet.frame_type = frame_type;
    packet.frame_id = frame_id;
    //packet.options = 0;
    for (int i = 0; i < 8; i++)
    {
        packet.dest_address[i] = (unsigned char) address[i];
    }
}


#if 0
void xbee_packet(unsigned char length, unsigned char frame_type, unsigned char frame_id, unsigned char *address){
    packet.length = 13;
    packet.frame_type = 0x00;
    packet.frame_id = 0x01;
    unsigned int address[9] ={0x00,0x13,0xA2,0x00,0x41,0xC7,0x20,0x87};
    for (int i = 0; i < 8; i++)
    {
        packet.dest_address[i] = address[i];
    }
    packet.data[0] = 0x41;
    packet.data[1] = 0x42;
}
#endif

void preparing_package(struct xBeePacket* p, unsigned char *tx_buff){
    
    unsigned char checksum = 0;

    tx_buff[0]=0x7E;			// send start bit
    tx_buff[1]= (unsigned char)(p->length>>8); // send the most significant byte (MSB).
    tx_buff[2]= (unsigned char)(p->length & 0x00FF); // send the least significant byte (LSB)

    tx_buff[3]=(p->frame_type);
    checksum += p->frame_type;	// add frame type to checksum
    tx_buff[4]=(p->frame_id);
    checksum += p->frame_id; // add frame ID to checksum

    // the mac address is now sent, 6 byte
    for (int i = 5; i < 13; i++)
    {
        tx_buff[i]= (unsigned char) (p->dest_address[i-5]);
        checksum += p->dest_address[i-5]; //add to checksum
    }

    tx_buff[13] = (p->options);
    checksum += p->options;

    //The actual message/data is sent. In this case, we only have two char/bytes to send
    /*
    tx_buff[14]= p->data[0];
    checksum += p->data[0];
    tx_buff[15] = p->data[1];
    checksum += p->data[1];
    */
    int tx_index = 14;
    for(int i = 0;i<p->length-11; i++){
        printf("tx_index %d\n",tx_index);
        tx_buff[tx_index]= p->data[i];
        checksum += p->data[i];
        tx_index +=1;
    }
    printf("tx_index %d\n",tx_index);
    //calculate the proper checksum
    tx_buff[tx_index] = (0xFF-checksum);

}

#if 0
int init_spi_xbee(unsigned spi_chanal, uint8_t mode, uint32_t speed){
	return spi_open(spi_chanal, mode, speed);
}

void send_package_uart(struct xBeePacket* p,int fd){

    char package[17]; 
   
    preparing_package(p, package);
    write(fd,package,17);
}

void send_package_spi(struct xBeePacket* p,int fd, uint32_t speed){

    char package[17]; 
   
    preparing_package(p, package);
    spi_xbee_tx(fd, package, 17, speed);
}


void spi_xbee_tx(int fd, uint8_t *data, int length, uint32_t speed){
	spi_transfer(fd, data,length, speed);
}

void spi_xbee_rx(int fd, int length, uint32_t speed){
	uint8_t rx_buffer[11];
	for(int i=0;i<11;i++){
		rx_buffer[i] = 0xff;
	}
	spi_transfer(fd, rx_buffer,length, speed);
	printf("Received SPI buffer...\r\n");
    printf("%c%c",rx_buffer[7],rx_buffer[8]);
    printf("\r\n");
}
#endif