
#ifndef SRC_TIM2_H_
#define SRC_TIM2_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include "DAC_Interface.h"

void TIM2_Init();
void GPIO_SYSCLCK_Init(); // Enable MCO for output sysclock
void GPIO_TIM2_PA2_IRQ_Tester_Init(); // config gpio output on pa2 for testing the T_update from the irq_handler of tim2
void GPIO_TIM2_PA5_Init(); // Configure MCO output on PA5, not used in this project

#endif
