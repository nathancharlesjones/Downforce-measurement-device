#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

extern void Error_Handler(void);
extern osMessageQueueId_t Queue_UART_TXHandle;
extern ADC_HandleTypeDef hadc1;

void
Thread_ADC_LightSensor( void )
{
    static uint16_t lightVal = 0;
    static uint16_t ADC_delay = 1000;
    static char ADC_msg[MAX_LOG_MSG_SIZE] = "ADC start-up message.\n";
    uint8_t idx = 0;
    
    for (idx = 0; ADC_msg[idx]; idx++)
    {
        osMessageQueuePut(Queue_UART_TXHandle, &ADC_msg[idx], 0, 0);
    }
    memset(&ADC_msg[0], 0, MAX_LOG_MSG_SIZE);
  
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
  
    /* Infinite loop */
    for(;;)
    {
        HAL_ADC_Start(&hadc1);
        while( HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK )
        {
            osDelay(1);
        }
    
        lightVal = HAL_ADC_GetValue(&hadc1);
        sprintf(&ADC_msg[0], "%d: %d\n", HAL_GetTick(), lightVal);
    
        for (idx = 0; ADC_msg[idx]; idx++)
        {
            osMessageQueuePut(Queue_UART_TXHandle, &ADC_msg[idx], 0, 0);
        }
        memset(&ADC_msg[0], 0, MAX_LOG_MSG_SIZE);
    
        if (lightVal > 2000u)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        }
        osDelay(ADC_delay);
    }
}
