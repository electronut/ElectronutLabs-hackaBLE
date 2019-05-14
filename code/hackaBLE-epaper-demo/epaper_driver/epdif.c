/**
 *  @filename   :   epdif.c
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     July 7 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "epdif.h"

extern nrf_drv_spi_t spi;


EPD_Pin epd_cs_pin = {
  SPI_CS_Pin,
};

EPD_Pin epd_rst_pin = {
  RST_Pin,
};

EPD_Pin epd_dc_pin = {
  DC_Pin,
};

EPD_Pin epd_busy_pin = {
  BUSY_Pin,
};

EPD_Pin pins[4];

void EpdDigitalWriteCallback(int pin_num, int value) {
  if (value == HIGH) {
    nrf_gpio_pin_set(pins[pin_num].pin);
  } else {
    nrf_gpio_pin_clear(pins[pin_num].pin);
  }
}

int EpdDigitalReadCallback(int pin_num) {
  if (nrf_gpio_pin_read(pins[pin_num].pin)) {
    return HIGH;
  } else {
    return LOW;
  }
}

void EpdDelayMsCallback(unsigned int delaytime) {
  nrf_delay_ms(delaytime);  
}

void EpdSpiTransferCallback(unsigned char data) {
  nrf_gpio_pin_clear(pins[CS_PIN].pin);
  //- HAL_SPI_Transmit(&hspi1, &data, 1, 1000);
  nrf_drv_spi_transfer(&spi, &data, 1, NULL, 0);
  nrf_gpio_pin_set(pins[CS_PIN].pin);
}

int EpdInitCallback(void) {
  pins[CS_PIN] = epd_cs_pin;
  pins[RST_PIN] = epd_rst_pin;
  pins[DC_PIN] = epd_dc_pin;
  pins[BUSY_PIN] = epd_busy_pin;
  
  return 0;
}

