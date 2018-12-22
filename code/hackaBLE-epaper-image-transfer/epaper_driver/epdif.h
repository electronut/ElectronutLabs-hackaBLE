/** @file epdif.h
 *
 * @brief Header file of epdif.c providing EPD interface functions
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2018 Electronut Labs.
 * All rights reserved. 
*/

#ifndef EPDIF_H
#define EPDIF_H

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"

#define EPD_BUSY_PIN 2
#define EPD_RST_PIN 3
#define EPD_DC_PIN 4
#define EPD_SPI_CS_PIN      5
#define EPD_SPI_SCK_PIN     7
#define EPD_SPI_MOSI_PIN    8

// Pin definition
#define CS_PIN           0
#define RST_PIN          1
#define DC_PIN           2
#define BUSY_PIN         3

// Pin level definition
#define LOW             0
#define HIGH            1

typedef struct {
  int pin;
} EPD_Pin;

void epaper_GPIO_Init(void);
void spi_epd_init();

int  EpdInitCallback(void);
void EpdDigitalWriteCallback(int pin, int value);
int  EpdDigitalReadCallback(int pin);
void EpdDelayMsCallback(unsigned int delaytime);
void EpdSpiTransferCallback(unsigned char data);

#endif /* EPDIF_H */
