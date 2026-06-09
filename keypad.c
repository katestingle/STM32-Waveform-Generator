/*
 * keypad.c
 *
 *  Created on: May 30, 2026
 *      Author: kates
 */

#include "keypad.h"
#define GPIO_PORT(port) (((uint32_t)(port) - (uint32_t)(AHB2PERIPH_BASE)) >> 9)
/*These pound defines that use ffs are going to be constant numbers during runtime.
* The compiler should detect this and turn them into constants to avoid wasting CPU time.
* Not the most elegant solution I know, but it allows the greatest flexibility
* and ease of use of this module.
* I haven't tested this, but it should work across most STM32 MCUs.
*/
#define BIT_0(pin) (pin << ((uint32_t)ffs(pin) - 1))
#define BIT_1(pin) (pin << (uint32_t)ffs(pin))
#define IN_MODER_MSK(keypad) (BIT_0(keypad -> row_0) | BIT_1(keypad -> row_0) | BIT_0(keypad -> row_1) | BIT_1(keypad -> row_1) | BIT_0(keypad -> row_2) | BIT_1(keypad -> row_2)  | BIT_0(keypad -> row_3) | BIT_1(keypad -> row_3))
#define IN_PUPDR_MSK(keypad) IN_MODER_MSK(keypad)
#define IN_PUPDR(keypad) (BIT_1(keypad -> row_0) | BIT_1(keypad -> row_1) | BIT_1(keypad -> row_2) | BIT_1(keypad -> row_3))
#define IN_IDR_MSK(keypad) (keypad -> row_0 | keypad -> row_1 | keypad -> row_2 | keypad -> row_3)
#define OUT_MODER_MSK(keypad) (BIT_0(keypad -> col_0) | BIT_1(keypad -> col_0) | BIT_0(keypad -> col_1) | BIT_1(keypad -> col_1) | BIT_0(keypad -> col_2) | BIT_1(keypad -> col_2))
#define OUT_PUPDR_MSK(keypad) OUT_MODER_MSK(keypad)
#define OUT_MODER(keypad) (BIT_0(keypad -> col_0) | BIT_0(keypad -> col_1) | BIT_0(keypad -> col_2))
#define OUT_OTYPER_MSK(keypad) (keypad -> col_0 | keypad -> col_1 | keypad -> col_2)
#define OUT_OSPEEDR_MSK(keypad) OUT_MODER_MSK(keypad)
#define OUT_ODR_MSK(keypad) (keypad -> col_0 | keypad -> col_1 | keypad -> col_2)
//define global struct containing definitions for the keypad
const struct keypad_s my_keypad_s = {GPIOB,GPIOB,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6,GPIO_PIN_13,GPIO_PIN_14,GPIO_PIN_15,GPIO_PIN_1,'1','2','3','4','5','6','7','8','9','*','0','#'};
//pointer to said struct
keypad my_keypad = &my_keypad_s;
//pointers to point to specific parts of the struct
const uint32_t *keypad_row_arr = &my_keypad_s.row_0;
const uint32_t *keypad_col_arr = &my_keypad_s.col_0;
const uint8_t *keypad_data_arr = &my_keypad_s.one_c;


void Keypad_Init(){
	//cast struct members to pointer to create array
	//set up clock in reset and clock control
	RCC -> AHB2ENR |= (GPIO_PORT(my_keypad -> in_GPIO) | GPIO_PORT(my_keypad -> out_GPIO));
	//clear (and set) in GPIO MODER
	my_keypad ->  in_GPIO -> MODER   &= ~(IN_MODER_MSK(my_keypad));
	//clear in GPIO PUPDR
	my_keypad ->  in_GPIO -> PUPDR   &= ~(IN_PUPDR_MSK(my_keypad));
	//set in GPIO PUPDR
	my_keypad ->  in_GPIO -> PUPDR   |=  (IN_PUPDR(my_keypad));
	//clear out GPIO MODER
	my_keypad -> out_GPIO -> MODER   &= ~(OUT_MODER_MSK(my_keypad));
	//set out GPIO MODER
	my_keypad -> out_GPIO -> MODER   |=  (OUT_MODER(my_keypad));
	//clear(and set) out GPIO PUPDR
	my_keypad -> out_GPIO -> PUPDR   &= ~(OUT_PUPDR_MSK(my_keypad));
	//clear (and set) out GPIO OTYPER
	my_keypad -> out_GPIO -> OTYPER  &= ~(OUT_OTYPER_MSK(my_keypad));
	//clear out GPIO OSPEEDR
	my_keypad -> out_GPIO -> OSPEEDR &= ~(OUT_OSPEEDR_MSK(my_keypad));
	//set out GPIO OSPEEDR
	my_keypad -> out_GPIO -> OSPEEDR |=  (OUT_OSPEEDR_MSK(my_keypad));
	//clear out GPIO ODR
	my_keypad -> out_GPIO -> ODR     &= ~(OUT_ODR_MSK(my_keypad));
	//set out GPIO ODR (now ready to start polling)
	my_keypad -> out_GPIO -> ODR     |=  (OUT_ODR_MSK(my_keypad));
}

uint8_t Keypad_Poll(){
	if(my_keypad -> in_GPIO -> IDR & IN_IDR_MSK(my_keypad)){
		//button pressed. Which button?
		for(int cols = 0; cols < 3; cols++){
			//poll columns
			//clear out GPIO ODR (clear columns)
			my_keypad -> out_GPIO -> ODR &= ~(OUT_ODR_MSK(my_keypad));
			//set one column high
			my_keypad -> out_GPIO -> ODR |=  (keypad_col_arr[cols]);
			for(int rows = 0; rows < 4; rows++){
				//poll rows
				if(my_keypad -> in_GPIO -> IDR & keypad_row_arr[rows]){
					//set out GPIO ODR to ready for next poll cycle
					my_keypad -> out_GPIO -> ODR     |=  (OUT_ODR_MSK(my_keypad));
					//which key?
					return keypad_data_arr[(rows * 3 + cols)];
				}
			}
		}
		//set columns high again for next poll cycle
		my_keypad -> out_GPIO -> ODR     |=  (OUT_ODR_MSK(my_keypad));
	}
	return '\0';
}
