/** @file epdif.c
 *
 * @brief Implements EPD interface functions
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2018 Electronut Labs.
 * All rights reserved. 
*/

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "epdif.h"

EPD_Pin epd_cs_pin = {
  EPD_SPI_CS_PIN,
};

EPD_Pin epd_rst_pin = {
  EPD_RST_PIN,
};

EPD_Pin epd_dc_pin = {
  EPD_DC_PIN,
};

EPD_Pin epd_busy_pin = {
  EPD_BUSY_PIN,
};

EPD_Pin pins[4];

void epaper_GPIO_Init(void)
{
  nrf_gpio_pin_clear(EPD_DC_PIN);
  nrf_gpio_pin_clear(EPD_RST_PIN);
  nrf_gpio_pin_clear(EPD_SPI_CS_PIN);

  nrf_gpio_cfg_output(EPD_DC_PIN);
  nrf_gpio_cfg_output(EPD_RST_PIN);
  nrf_gpio_cfg_output(EPD_SPI_CS_PIN);
  nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
}

#define SPI_INSTANCE  1 /**< SPI instance index. */
const nrf_drv_spi_t spi_epd = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

void spi_epd_init()
{
  nrf_drv_spi_config_t spi_config_epd = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config_epd.frequency = NRF_DRV_SPI_FREQ_1M;
  spi_config_epd.mode = NRF_DRV_SPI_MODE_0;
  spi_config_epd.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  spi_config_epd.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config_epd.mosi_pin = EPD_SPI_MOSI_PIN;
  spi_config_epd.sck_pin = EPD_SPI_SCK_PIN;
  spi_config_epd.ss_pin = EPD_SPI_CS_PIN;

  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_epd, &spi_config_epd, NULL, NULL));
}

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

void EpdSpiTransferCallback(unsigned char data)
{
  nrf_gpio_pin_clear(pins[CS_PIN].pin);
  nrf_drv_spi_transfer(&spi_epd, &data, 1, NULL, 0);
  nrf_gpio_pin_set(pins[CS_PIN].pin);
}

int EpdInitCallback(void) {
  pins[CS_PIN] = epd_cs_pin;
  pins[RST_PIN] = epd_rst_pin;
  pins[DC_PIN] = epd_dc_pin;
  pins[BUSY_PIN] = epd_busy_pin;
  
  return 0;
}

