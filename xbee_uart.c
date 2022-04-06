
#include "uart.h"

void send_package_uart(struct xBeePacket* p,int fd){

    char package_arr[17]; 
   
    preparing_package(p, package_arr);
    //serial_write(fd, package_arr);
    
    write(fd,package_arr,sizeof(package_arr));
}

int uart_xbee_rx(int fd, unsigned char *rx_buff){
    //return read(fd, &rx_buff, sizeof(rx_buff));
    return serial_read(fd, rx_buff);
}

