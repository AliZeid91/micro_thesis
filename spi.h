#ifndef SPI_H
#define SPI_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int spi_open(float spi_chanal, uint8_t mode, uint32_t speed);
//void spi_transfer(int fd, uint8_t *data, int length, uint32_t speed);
int spi_transfer(int fd, char *txBuf, char *rxBuf, unsigned count);
void spi_xbee_tx(int fd, uint8_t *data, int length, uint32_t speed);
int spi_close(int fd);
//int spiXfer(int fd, unsigned speed, char *txBuf, char *rxBuf, unsigned count);

#endif