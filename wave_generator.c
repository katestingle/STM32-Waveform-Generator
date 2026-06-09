/*
 * calc_wavefoms.c
 *
 *  Created on: May 31, 2026
 *      Author: kates
 */
#include <wave_generator.h>

// Global state variable accessed by main loop (write) and ISR (read)
volatile WaveformState_t current_state = DEFAULT_WAVEFORM_STATE;
volatile uint32_t phase_accumulator = 0;

uint16_t sin_table[UPDATES_PER_PERIOD];
uint16_t process_duty = 0;

// enable dac spi interface, tim2, gpios for tim2 testing,set initial wave generator conditions
void Wave_Generator_Init(){
	DAC_Init();
	TIM2_Init();
	GPIO_TIM2_PA2_IRQ_Tester_Init();
}

// generates sine lookup table
void gen_sin_table(){
	for (int i = 0; i < UPDATES_PER_PERIOD; i++) {
	        // Calculate normalized angle (0 to 2*PI)
	        double angle = (2.0 * M_PI * i) / UPDATES_PER_PERIOD;
	        double sine_out = sin(angle) + 1.0;
	        // scale 0.0-2.0 range to fit 12b bounds (0 to 4095)
	        sin_table[i] = (uint16_t)((sine_out * 3000) / 2.0);
	    }
}

// takes a command and updates waveform state, no update if '\0'
void Update_waveform(KeyCommand_t command){
	if(command != CMD_NONE){
		__disable_irq();// global interrupt EN
		switch (command) {
			// --- FREQUENCY CONFIGURATIONS ---
			case CMD_SET_FREQ_100HZ:
				current_state.frequency = 100;
				current_state.step_size = 10;  // 100 Hz / 10
				break;
			case CMD_SET_FREQ_200HZ:
				current_state.frequency = 200;
				current_state.step_size = 20;  // 200 Hz / 10
				break;
			case CMD_SET_FREQ_300HZ:
				current_state.frequency = 300;
				current_state.step_size = 30;  // 300 Hz / 10
				break;
			case CMD_SET_FREQ_400HZ:
				current_state.frequency = 400;
				current_state.step_size = 40;  // 400 Hz / 10
				break;
			case CMD_SET_FREQ_500HZ:
				current_state.frequency = 500;
				current_state.step_size = 50;  // 500 Hz / 10
				break;
			// --- WAVEFORM TYPE CONFIGURATIONS ---
			case CMD_SET_TYPE_SINE:
				gen_sin_table();
				current_state.wave_type = SINE;
				break;
			case CMD_SET_TYPE_TRIANGLE:
				current_state.wave_type = TRIANGLE;
				break;
			case CMD_SET_TYPE_SAWTOOTH:
				current_state.wave_type = SAWTOOTH;
				break;
			case CMD_SET_TYPE_SQUARE:
				current_state.wave_type = SQUARE;
				break;
			// --- SQUARE WAVE DUTY CYCLE CONFIGURATIONS ---
			case CMD_DECREMENT_DUTY:
				// Only allow modifications if currently outputting a square wave
				// and keep it bounded above the 10% system minimum
				if(process_duty == 0){// only change duty once we've waited enough for the previous inc/dec to debounce
					if (current_state.wave_type == SQUARE && current_state.duty_cycle > 10) {
						current_state.duty_cycle -= 10;
						process_duty = 1;
					}
				}
				break;
			case CMD_RESET_DUTY:
				if (current_state.wave_type == SQUARE) {
					current_state.duty_cycle = 50;
					process_duty = 0;
				}
				break;
			case CMD_INCREMENT_DUTY:
				// Keep it bounded below the 90% system maximum
				if(process_duty == 0){// only change duty once we've waited enough for the previous inc/dec to debounce
					if (current_state.wave_type == SQUARE && current_state.duty_cycle < 90) {
						current_state.duty_cycle += 10;
						process_duty = 1;
					}
				}
				break;
			default:
				// Safeguard against undefined commands
				break;
		}
		__enable_irq(); // global interrupt EN
	}
}

// specialized IRQ_Handler for contuous DAC writing, specific to the current waveform state(above)
void TIM2_IRQHandler(void){
	GPIOA->BSRR = GPIO_BSRR_BS2;
	if (TIM2->SR & TIM_SR_UIF) { // Check update interrupt flag
	        TIM2->SR &= ~TIM_SR_UIF; // Clear flag
	        phase_accumulator += current_state.step_size;
	        if (phase_accumulator >= UPDATES_PER_PERIOD) {
	            phase_accumulator -= UPDATES_PER_PERIOD; // Safe wrap-around
	        }
	        uint16_t dac_value = 1500; // Default to mid-scale(1.5V)
	        switch (current_state.wave_type) {
	            case SQUARE: {
	                // Duty cycle boundary line
	            	if(process_duty >0 && process_duty < 100000){
	            		// that is, if the duty increment or decrement button was recently pressed
	            		process_duty += 1; // increment process_duty counter
	            	} else if(process_duty >= 100000){ // if duty inc/dec counter was waited long enough
	            		process_duty = 0; // set process duty = 0 so inc/dec is now allowed
	            	}
	                uint32_t switch_point = (current_state.duty_cycle * UPDATES_PER_PERIOD) / 100;
	                dac_value = (phase_accumulator < switch_point) ? 3000 : 0;
	                break;
	            }
	            case SAWTOOTH: {
	                // Linear ramp from 0 to 4095 over the entire 0-1199 phase range
	                dac_value = (phase_accumulator * 3000) / (UPDATES_PER_PERIOD - 1);
	                break;
	            }
	            case TRIANGLE: {
	                // First half ramps up, second half ramps down
	                if (phase_accumulator < (UPDATES_PER_PERIOD / 2)) {
	                    dac_value = (phase_accumulator * 3000) / (UPDATES_PER_PERIOD / 2);
	                } else {
	                    dac_value = 3000 - (((phase_accumulator - (UPDATES_PER_PERIOD / 2)) * 3000) / (UPDATES_PER_PERIOD / 2));
	                }
	                break;
	            }
	            case SINE: {
	                // Fast array access, zero real-time math overhead
	                dac_value = sin_table[phase_accumulator];
	                break;
	            }
	        }

            dac_value = DAC_volt_conv((int)dac_value);
	        DAC_write(dac_value);
	    }
	//if(TIM2->SR & TIM_SR_CC1IF){
//		TIM2->SR &= ~TIM_SR_CC1IF; // Disable the CC1 Interrupt Flag
//		// GPIOA->BRR = (1 << 5);   // Turn off PA5 --> spi_set_1V
//		uint16_t spi_voltage_12b = DAC_volt_conv(1000); // convert 1V --> 12b
//		DAC_write(spi_voltage_12b);   // SET DAC 1V
//	}
	GPIOA->BRR = GPIO_BRR_BR2;
}
