#include <string.h>
#include "main.h"
#include "cmsis_os.h"

extern void Error_Handler(void);
extern osThreadId_t UART_TXHandle;
extern osThreadId_t UART_RXHandle;
extern osMessageQueueId_t Queue_UART_TXHandle;
extern osMessageQueueId_t Queue_UART_RXHandle;
extern UART_HandleTypeDef huart1;

static uint32_t Thread_UART_RX_max_stack_size = 0;
static uint32_t Thread_UART_TX_max_stack_size = 0;
static uint16_t UART_TX_queue_max_size = 0;
static uint16_t UART_RX_queue_max_size = 0;

void
Thread_UART_RX(void)
{
    static char rx_msg[MAX_LOG_MSG_SIZE+1] = {0};
    static uint8_t idx = MAX_LOG_MSG_SIZE;
    
    for (;;)
    {
        uint32_t Thread_UART_RX_stack_size = osThreadGetStackSpace(UART_RXHandle);
        if ( Thread_UART_RX_stack_size > Thread_UART_RX_max_stack_size ) Thread_UART_RX_max_stack_size = Thread_UART_RX_stack_size;
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
            osThreadYield();
        }
      
        //Echo received msg
        for (idx = 0; rx_msg[idx]; idx++)
        {
            osMessageQueuePut(Queue_UART_TXHandle, &rx_msg[idx], 0, 0);
        }
        memset(&rx_msg[0], 0, MAX_LOG_MSG_SIZE);
      
        osThreadYield();
    }
}

void
Thread_UART_TX(void)
{
    static char ch;
    
    for (;;)
    {
        uint32_t Thread_UART_TX_stack_size = osThreadGetStackSpace(UART_TXHandle);
        if ( Thread_UART_TX_stack_size > Thread_UART_TX_max_stack_size ) Thread_UART_TX_max_stack_size = Thread_UART_TX_stack_size;
        
        uint16_t UART_TX_queue_count = osMessageQueueGetCount(Queue_UART_TXHandle);
        if ( UART_TX_queue_count > 0 )
        {
            if ( UART_TX_queue_count > UART_TX_queue_max_size ) UART_TX_queue_max_size = UART_TX_queue_count;
            osMessageQueueGet(Queue_UART_TXHandle, &ch, 0, 0);
            HAL_UART_Transmit(&huart1, (uint8_t *)(&ch), 1, HAL_MAX_DELAY);
        }
        osThreadYield();
    }
}
