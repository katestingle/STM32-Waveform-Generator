/*
 * DAC_Interface.h
 *
 *  Created on: May 20, 2026
 *      Author: kates
 */
#ifndef INC_DAC_INTERFACE_H_
#define INC_DAC_INTERFACE_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx_it.h"

// DAC minimum viable API:
int DAC_Init(); // initializes the SPI peripheral to communicate with the DAC, return val base don pass/fail, calls SPI_init()
int DAC_write(uint16_t write_val); // DAC_write - write a voltage 12-bit value to the DAC
uint16_t DAC_volt_conv(int voltage); // convert a voltage value(in mV) into a 12-bit value to control the DAC

// helper functions
void configure_gpio_spi1(); // configures gpios PA4,5,6,7 for AF5 mode SPI1
void DAC_configure_spi1(); // enables spi1 clock, master mode, custom interface for MCP4921 DAC
void SPI_Init(); // turns on SPI interface for Port A AF mode #5, calls DAC_configure_spi() and configure_gpio_spi()

#endif /* INC_DAC_INTERFACE_H_ */
