#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

extern void Error_Handler(void);
extern osThreadId_t ADC_LightSensorHandle;
extern osMessageQueueId_t Queue_UART_TXHandle;
extern ADC_HandleTypeDef hadc1;

static uint32_t Thread_ADC_LightSensor_max_stack_size = 0;
static uint16_t UART_TX_queue_full_count = 0;

void
Thread_ADC_LightSensor( void )
{
    static uint16_t lightVal = 0;
    static volatile uint8_t adc_start = 0;
    static volatile uint16_t ADC_delay = 1000;
    const char channel_name[] = "LightSensor";
    static char ADC_msg[MAX_LOG_MSG_SIZE] = {0};
    uint8_t idx = 0;
    
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
  
    /* Infinite loop */
    while(1)
    {
        osThreadYield();
    }
    
    for(;;)
    {
        uint32_t Thread_ADC_LightSensor_stack_size = osThreadGetStackSpace(ADC_LightSensorHandle);
        if ( Thread_ADC_LightSensor_stack_size > Thread_ADC_LightSensor_max_stack_size ) Thread_ADC_LightSensor_max_stack_size = Thread_ADC_LightSensor_stack_size;
        
        if ( adc_start )
        {
            HAL_ADC_Start(&hadc1);
            while( HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK )
            {
                osThreadYield();
            }
    
            lightVal = HAL_ADC_GetValue(&hadc1);
            sprintf(&ADC_msg[0], "{ \"channel\" : \"%s\", \"time\" : %d, \"value\" : %d }\n", channel_name, (int)HAL_GetTick(), lightVal);
    
            for (idx = 0; ADC_msg[idx]; idx++)
            {
                if ( osMessageQueuePut(Queue_UART_TXHandle, &ADC_msg[idx], 0, 0) != osOK ) UART_TX_queue_full_count++;
            }
            memset(&ADC_msg[0], 0, MAX_LOG_MSG_SIZE);
    
            osDelay(ADC_delay);
        }
    }
}
