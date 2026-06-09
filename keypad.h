/*
 * keypad.h
 *
 *  Created on: May 30, 2026
 *      Author: kates
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "stm32l4xx_hal.h"
#include "stdlib.h"

// returns the character pressed as uint8_t ascii->dec (0-9, * and #), '\0' if no press
uint8_t Keypad_Poll();

//cast struct members to pointer to create array
//set up clock in reset and clock control
void Keypad_Init();


typedef struct keypad_s{
	//GPIO ports
	GPIO_TypeDef* in_GPIO;
	GPIO_TypeDef* out_GPIO;
	//COLS are always outputs
	__IO uint32_t col_0;
	__IO uint32_t col_1;
	__IO uint32_t col_2;
	//ROWS are always inputs
	__IO uint32_t row_0;
	__IO uint32_t row_1;
	__IO uint32_t row_2;
	__IO uint32_t row_3;
	//output chars
	uint8_t one_c;
	uint8_t two_c;
	uint8_t three_c;
	uint8_t four_c;
	uint8_t five_c;
	uint8_t six_c;
	uint8_t seven_c;
	uint8_t eight_c;
	uint8_t nine_c;
	uint8_t pound_C;
	uint8_t zero_c;
	uint8_t star_c;
}*keypad;


#endif /* INC_KEYPAD_H_ */
