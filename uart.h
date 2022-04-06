#ifndef UART_H
#define UART_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void serial_write(int fd, unsigned char *package_arr);

int serial_read(int fd, unsigned char *rx_buff);

#endif