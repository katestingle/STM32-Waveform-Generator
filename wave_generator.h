/*
 * calc_waveforms.h
 *
 *  Created on: May 31, 2026
 *      Author: kates
 */

#ifndef INC_WAVE_GENERATOR_H_
#define INC_WAVE_GENERATOR_H_

#include "stm32l4xx_hal.h"
#include "DAC_Interface.h"
#include "tim2.h"
#include <math.h>
#include <stdio.h>

// for sin lookup table
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// commands for directing the waveform generator
typedef enum {
    CMD_NONE = 0,
    CMD_SET_FREQ_100HZ = 1,
    CMD_SET_FREQ_200HZ,
    CMD_SET_FREQ_300HZ,
    CMD_SET_FREQ_400HZ,
    CMD_SET_FREQ_500HZ,
    CMD_SET_TYPE_SINE,
    CMD_SET_TYPE_TRIANGLE,
    CMD_SET_TYPE_SAWTOOTH,
    CMD_SET_TYPE_SQUARE,
    CMD_DECREMENT_DUTY,
    CMD_RESET_DUTY,
    CMD_INCREMENT_DUTY
} KeyCommand_t;

typedef enum {
    SINE = 1,
    TRIANGLE = 2,
    SAWTOOTH = 3,
    SQUARE = 4
} WaveType_t;

typedef struct {
    WaveType_t wave_type;
    uint32_t frequency;     // 100, 200, 300, 400, 500
    uint32_t duty_cycle;    // 10 to 90 (for square wave)
    uint32_t step_size;     // Calculated phase step per timer tick
} WaveformState_t;

#define DEFAULT_WAVEFORM_STATE ((WaveformState_t){.wave_type = SQUARE, .frequency = 100, .duty_cycle = 50, .step_size = 10})

#define UPDATES_PER_PERIOD 10000 // TO DO : CALIBRATE THIS
// f_update = f_s = fclk/arr+1 = 32,000,000Hz/(3200) = 10,000Hz
// f_period = f_update*step_size/UPDATES_PER_PERIOD = 10,000Hz
// therefor step_size = f_period * UPDATES_PER_PERIOD/f_update

void Wave_Generator_Init(); // initializes timer and DAC interface for wave generator module

void gen_sin_table(); // generates sine lookup table
void Update_Waveform(KeyCommand_t command); // takes a and updates waveform state, no update if '\0'


void TIM2_IRQHandler(void); // specialized IRQ_Handler for contuous DAC writing, specific to the current waveform state
#endif /* INC_WAVE_GENERATOR_H_ */
