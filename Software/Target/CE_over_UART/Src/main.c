/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_LOG_MSG_SIZE 128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static uint8_t SPI_msg[MAX_LOG_MSG_SIZE] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void setGain(gain_t new_gain);
void setVref(uint16_t _new_Vref);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//From https://electronics.stackexchange.com/questions/272427/stm32-busy-flag-is-set-after-i2c-initialization
/**
1. Disable the I2C peripheral by clearing the PE bit in I2Cx_CR1 register.
2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level
(Write 1 to GPIOx_ODR).
3. Check SCL and SDA High level in GPIOx_IDR.
4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to
GPIOx_ODR).
5. Check SDA Low level in GPIOx_IDR.
6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to
GPIOx_ODR).
7. Check SCL Low level in GPIOx_IDR.
8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to
GPIOx_ODR).
9. Check SCL High level in GPIOx_IDR.
10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to
GPIOx_ODR).
11. Check SDA High level in GPIOx_IDR.
12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
13. Set SWRST bit in I2Cx_CR1 register.
14. Clear SWRST bit in I2Cx_CR1 register.
15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register.
**/

void HAL_GPIO_WRITE_ODR(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  GPIOx->ODR |= GPIO_Pin;
}

void HAL_I2C_ClearBusyFlagErrata_2_14_7(I2C_HandleTypeDef *hi2c) {

    static uint8_t resetTried = 0;
    if (resetTried == 1) {
        return ;
    }
    uint32_t SDA_PIN = GPIO_PIN_7;
    uint32_t SCL_PIN = GPIO_PIN_6;
    GPIO_InitTypeDef GPIO_InitStruct;

    // 1
    __HAL_I2C_DISABLE(hi2c);

    // 2
    GPIO_InitStruct.Pin = SDA_PIN|SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WRITE_ODR(GPIOB, SDA_PIN);
    HAL_GPIO_WRITE_ODR(GPIOB, SCL_PIN);

    // 3
    GPIO_PinState pinState;
    if (HAL_GPIO_ReadPin(GPIOB, SDA_PIN) == GPIO_PIN_RESET) {
        for(;;){}
    }
    if (HAL_GPIO_ReadPin(GPIOB, SCL_PIN) == GPIO_PIN_RESET) {
        for(;;){}
    }

    // 4
    GPIO_InitStruct.Pin = SDA_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_TogglePin(GPIOB, SDA_PIN);

    // 5
    if (HAL_GPIO_ReadPin(GPIOB, SDA_PIN) == GPIO_PIN_SET) {
        for(;;){}
    }

    // 6
    GPIO_InitStruct.Pin = SCL_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_TogglePin(GPIOB, SCL_PIN);

    // 7
    if (HAL_GPIO_ReadPin(GPIOB, SCL_PIN) == GPIO_PIN_SET) {
        for(;;){}
    }

    // 8
    GPIO_InitStruct.Pin = SDA_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WRITE_ODR(GPIOB, SDA_PIN);

    // 9
    if (HAL_GPIO_ReadPin(GPIOB, SDA_PIN) == GPIO_PIN_RESET) {
        for(;;){}
    }

    // 10
    GPIO_InitStruct.Pin = SCL_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WRITE_ODR(GPIOB, SCL_PIN);

    // 11
    if (HAL_GPIO_ReadPin(GPIOB, SCL_PIN) == GPIO_PIN_RESET) {
        for(;;){}
    }

    // 12
    GPIO_InitStruct.Pin = SDA_PIN|SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    //GPIO_InitStruct.Alternate = NUCLEO_I2C_EXPBD_SCL_SDA_AF;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   // 13
   hi2c->Instance->CR1 |= I2C_CR1_SWRST;

   // 14
   hi2c->Instance->CR1 ^= I2C_CR1_SWRST;

   // 15
   __HAL_I2C_ENABLE(hi2c);

   resetTried = 1;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  static char ADC_msg[MAX_LOG_MSG_SIZE] = {0};
  static const char channel_name[] = "LightSensor";
  static volatile uint16_t lightVal = 0;
  static volatile uint8_t zoomAndEnhance = 0;
  static const uint8_t gain[8] = {1,2,4,5,8,10,16,32};
  static volatile gain_t gain_idx = GAIN_OF_1;
  static volatile float Vin_min = 0;
  static volatile float Vin_max = 3.300;
  static volatile uint16_t Vref = 0;
  static uint8_t I2C_msg[MAX_LOG_MSG_SIZE] = {0};
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
      Error_Handler();
  }

  /*
  HAL_I2C_ClearBusyFlagErrata_2_14_7(&hi2c1);

  volatile uint8_t res = 0;
  while ( res = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(0x2A<<1), 1000, HAL_MAX_DELAY)  )
  {
  	__asm("nop");
  }
  //Reset the load scale
  //Read REG0x0, PU_CTRL. Data should be 0.
  //HAL_I2C_Mem_Read(&hi2c1, 0x54, 0x0, 0x2, &I2C_msg[0], 0x2, HAL_MAX_DELAY);
  HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)0x54, &I2C_msg[0], 2, HAL_MAX_DELAY);
  //Write 0x1 to REG0x0, PU_CTRL, to put device in reset state.
  I2C_msg[0] = 0x1;
  HAL_I2C_Mem_Write(&hi2c1, 0x54, 0x0, 0x2, &I2C_msg[0], 0x2, HAL_MAX_DELAY);
  I2C_msg[0] = 0x0;
  //Read REG0x0, PU_CTRL. Data should be 1.
  HAL_I2C_Mem_Read(&hi2c1, 0x54, 0x0, 0x2, &I2C_msg[0], 0x2, HAL_MAX_DELAY);

  //Set initial load scale register values
  //Take device out of reset, set AVDD to internal LDO, and turn on analog and digital circuits
  I2C_msg[0] = 0x85;
  HAL_I2C_Mem_Write(&hi2c1, 0x54, 0x0, 0x2, &I2C_msg[0], 0x2, HAL_MAX_DELAY);
  I2C_msg[0] = 0x0;
  //Read REG0x0, PU_CTRL. Data should be 0xAE.
  HAL_I2C_Mem_Read(&hi2c1, 0x54, 0x0, 0x2, &I2C_msg[0], 0x2, HAL_MAX_DELAY);
  */
  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //int z = 0;
    //z = 4*t;
    if (zoomAndEnhance)
    {
    	//calculate gain from Vin_max, Vin_min
    	float gain_float = 3.3 / ( Vin_max - Vin_min );
    	for ( gain_idx = GAIN_OF_32; gain_idx > 0; gain_idx--)
    	{
    		if ( gain[gain_idx] < gain_float ) break;
    	}
    	//update gain
    	setGain(gain_idx);

    	//calculate Vref from gain, Vin_min
    	float Vref_float = ( gain[gain_idx] * Vin_min ) / ( gain[gain_idx] - 1 );
    	uint16_t Vref = round( Vref_float * 4095 / 3.3f );
    	//update Vref
    	setVref(Vref);
    }

    HAL_ADC_Start(&hadc1);
    while( HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK );
    lightVal = HAL_ADC_GetValue(&hadc1);
    
    sprintf(&ADC_msg[0], "{ \"channel\" : \"%s\", \"time\" : %d, \"value\" : %d }\n\r", channel_name, (int)HAL_GetTick(), lightVal);
    HAL_UART_Transmit(&huart1, (uint8_t*)&ADC_msg[0], strlen(ADC_msg), HAL_MAX_DELAY);

    memset(&ADC_msg[0], 0, MAX_LOG_MSG_SIZE);

    HAL_Delay(5);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 460800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 4800;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CSS_PGA_Pin|CSS_DAC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CSS_PGA_Pin CSS_DAC_Pin */
  GPIO_InitStruct.Pin = CSS_PGA_Pin|CSS_DAC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void setGain(gain_t new_gain)
{
	memset(&SPI_msg[0], 0, MAX_LOG_MSG_SIZE);
	SPI_msg[0] = 0x40;
    SPI_msg[1] = new_gain;
    HAL_GPIO_WritePin(CSS_PGA_GPIO_Port, CSS_PGA_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &SPI_msg[0], 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(CSS_PGA_GPIO_Port, CSS_PGA_Pin, GPIO_PIN_SET);
    memset(&SPI_msg[0], 0, MAX_LOG_MSG_SIZE);
}

void setVref(uint16_t _new_Vref)
{
	volatile uint16_t new_Vref = _new_Vref;
	if ( new_Vref > 4095 ) new_Vref = 4095;

	memset(&SPI_msg[0], 0, MAX_LOG_MSG_SIZE);
	SPI_msg[0] = ( 0x70 | ( ( new_Vref >> 8 ) & 0xF ) );
    SPI_msg[1] = ( new_Vref & 0xFF );
    HAL_GPIO_WritePin(CSS_DAC_GPIO_Port, CSS_DAC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &SPI_msg[0], 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(CSS_DAC_GPIO_Port, CSS_DAC_Pin, GPIO_PIN_SET);
    memset(&SPI_msg[0], 0, MAX_LOG_MSG_SIZE);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	__asm("bkpt 0");
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
