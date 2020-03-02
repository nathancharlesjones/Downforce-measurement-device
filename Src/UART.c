#include <string.h>
#include "main.h"
#include "cmsis_os.h"

extern void Error_Handler(void);
extern osMessageQueueId_t Queue_UART_TXHandle;
extern osMessageQueueId_t Queue_UART_RXHandle;
extern UART_HandleTypeDef huart1;

void
Thread_UART_RX(void)
{
    static char rx_msg[MAX_LOG_MSG_SIZE+1] = {0};
    static uint8_t idx = MAX_LOG_MSG_SIZE;
    
    for (;;)
    {
        HAL_UART_Receive_IT(&huart1, (uint8_t *)(&rx_msg[0]), MAX_LOG_MSG_SIZE);
        while ( huart1.RxState != HAL_UART_STATE_READY )
        {
            for ( idx = MAX_LOG_MSG_SIZE; idx > 0; idx-- )
            {
                if ( rx_msg[idx] == '\n' )
                {
                    HAL_UART_AbortReceive_IT(&huart1);
                }
            }
            osDelay(1);
        }
      
        //Echo received msg
        for (idx = 0; rx_msg[idx]; idx++)
        {
            osMessageQueuePut(Queue_UART_TXHandle, &rx_msg[idx], 0, 0);
        }
        memset(&rx_msg[0], 0, MAX_LOG_MSG_SIZE);
      
        osDelay(1);
    }
}

void
Thread_UART_TX(void)
{
    static char ch;
    
    for (;;)
    {
        if ( osMessageQueueGetCount(Queue_UART_TXHandle) > 0 )
        {
            osMessageQueueGet(Queue_UART_TXHandle, &ch, 0, 0);
            HAL_UART_Transmit(&huart1, (uint8_t *)(&ch), 1, HAL_MAX_DELAY);
        }
        osDelay(1);
    }
}
