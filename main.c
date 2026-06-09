/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/*
 * saw tooth wave (use timer)
 * square wave w/ variable duty cycle
 * sine wave
 *
 * The keypad will be used to select:
 * 	variable freq
 * 	output waveform type,
 *  duty cycle of the square wave
 *  triangle waveform
 *
 *  DAC_write() frequency remains constant with variable wave frequency, this may req. using a 2nd fixed timer for sampling?
 *
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  Keypad_Init();
  GPIO_SYSCLCK_Init(); // turn on pa8 system clock output, used to verify 80MHz system clock
  Wave_Generator_Init(); // enable dac spi interface, tim2, gpios for tim2 testing,set initial wave generator conditions

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  // Initialize the lookup table as a constant array (stored in Flash memory)
  // map character input to given waveform generator commands
  const uint8_t ascii_command_lookup[58] = {
      ['1'] = CMD_SET_FREQ_100HZ,
      ['2'] = CMD_SET_FREQ_200HZ,
      ['3'] = CMD_SET_FREQ_300HZ,
      ['4'] = CMD_SET_FREQ_400HZ,
      ['5'] = CMD_SET_FREQ_500HZ,
      ['6'] = CMD_SET_TYPE_SINE,
      ['7'] = CMD_SET_TYPE_TRIANGLE,
      ['8'] = CMD_SET_TYPE_SAWTOOTH,
      ['9'] = CMD_SET_TYPE_SQUARE,
      ['*'] = CMD_DECREMENT_DUTY,
      ['0'] = CMD_RESET_DUTY,
      ['#'] = CMD_INCREMENT_DUTY,
	  ['\0'] = CMD_NONE
  };
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  uint8_t pressed_key = Keypad_Poll();
	  if (pressed_key > 0 && pressed_key < 58) {
		Update_waveform((KeyCommand_t)ascii_command_lookup[pressed_key]);
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	__HAL_RCC_PWR_CLK_ENABLE();
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
		Error_Handler();
	}
	// 1. Configure the MSI clock as the source (completely internal, rock solid)
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	// Range 6 is 4 MHz (Perfect PLL input)
	RCC_OscInitStruct.MSICalibrationValue = 0;
	// 2. Drive the PLL from the 4 MHz MSI internal clock
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI; // Input = 4 MHz
	RCC_OscInitStruct.PLL.PLLM = 1;
	// 4 MHz / 1 = 4 MHz VCO input
	RCC_OscInitStruct.PLL.PLLN = 40;
	// 4 MHz * 40 = 160 MHz VCO output
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2; RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	// 160 MHz / 2 = 80 MHz SYSCLK
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	} /** After the PLL stabilizes at 80 MHz, we shift the MSI range to 32 MHz * to satisfy your project requirement constraint! */
	__HAL_RCC_MSI_RANGE_CONFIG(RCC_MSIRANGE_10); // Change MSI to 32 MHz safely post-lock
	/** Initializes the CPU, AHB and APB buses clocks */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	//if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		//Error_Handler();
	//}
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
