/*
 * DAC_Interface.c
 *
 *  Created on: May 20, 2026
 *      Author: kates
 */
#include "DAC_Interface.h"

void SPI_Init(){ // *** TO DO : FILL IN GPIO & SPI REG VALS **
	/* On Alternate Function Mode 5:  AF5
	 * SPI1_SCK = PA5,
	 * SPI1_MOSI = PA7,
	 * SPI1_NSS = PA4
	 */

	/* AFR info found in table 17 of the datasheet (not tech reference)
	 * Configure SPI Pins PA4 – SPI_1_NSS PA5 – SPI_1_SCK
	 * PA6 – SPI_1_MISO PA7 = SPI_1_MOSI
	 */
	configure_gpio_spi1();
	DAC_configure_spi1();
}

// configures gpios PA4,5,6,7 for AF5 mode SPI1
void configure_gpio_spi1(){
	/* On Alternate Function Mode 5:  AF5
	 * SPI1_SCK = PA5,
	 * SPI1_MOSI = PA7,
	 * SPI1_NSS = PA4
	 */

	/* AFR info found in table 17 of the datasheet (not tech reference)
	 * Configure SPI Pins PA4 – SPI_1_NSS PA5 – SPI_1_SCK
	 * PA6 – SPI_1_MISO PA7 = SPI_1_MOSI
	 */
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN);
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL5 |GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL7); // mask AF selection, using AFRL(alternate function low - low pins of port a 0-7
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL5_0 | GPIO_AFRL_AFSEL5_2 | GPIO_AFRL_AFSEL6_0 | GPIO_AFRL_AFSEL6_2 | GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL7_0 | GPIO_AFRL_AFSEL7_2; // select SPI_1 (AF5) set 0101
	GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE7 | GPIO_MODER_MODE6); // mask function on pa4, pa5, pa7, pa6
	GPIOA->MODER |= GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1 | GPIO_MODER_MODE7_1 | GPIO_MODER_MODE6_1; //enable alternate function mode on pa4, pa5, pa6, pa7
}

// enables spi1 clock, master mode, custom interface for MCP4921 DAC
void DAC_configure_spi1(){
	RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN); // enable SPI1 clock
	SPI1->CR1 = 0xc806; // enable master mode, fck/2, hardware CS, MSB first, full duplex = 0x12_1100.8_1000.0_0000.6_0110
	SPI1->CR2 = 0x1F08;  // enable CS output, create CS pulse, 16-bit data frames
	SPI1->CR1 |= SPI_CR1_SPE; //enable SPI
}

// initializes SPI interface for MCP4921 DAC
int DAC_Init(){ // initializes the SPI peripheral to communicate with the DAC, return val base don pass/fail
	SPI_Init();
	return 0;
}

// DAC_write - write a 12-bit value to the DAC
int DAC_write(uint16_t write_val){
	// 16 bit frame interface, [12:15] bits are constant, [0:11] describe data
	while(!(SPI1->SR & SPI_SR_TXE)); // ensure room in TXFIFO before writing
	SPI1->DR = write_val; // WHY 32-bit?? on datasheet says it's 16 bit
	return 0;
}

uint16_t DAC_volt_conv(int voltage){ // convert a 32-bit voltage value in mV into a 12-bit value to control the DAC
	// 16 bit frame interface, [12:15] bits are constant, [0:11] describe data
	// converts voltage --> 12 bit data val
	// Sets [12:15] const. val each time: gain = 1(13th bit), shdwn = 1(12th bit), unbuffered?(14th bit), 15th bit = 0
	uint16_t const_frame_vals = 0x3000; // set upper [12:15] = 0011
	if(voltage > 3000){ // max V is 3.0V, so if voltage is greater, then just set the max voltage.
		// Vout = Vref*({12b}/4096) makes {12b} = Vout * 4096/Vref = Vout *4096/3300
		return const_frame_vals | (uint16_t)(0xFFF);
	}
	// otherwise, do the math
	uint16_t calculated_12b = (uint16_t)((voltage * 4096) / 3750); // NEED TO RECALIBRATE
	return const_frame_vals | calculated_12b;
}
