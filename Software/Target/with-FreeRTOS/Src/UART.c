#include <string.h>
#include "main.h"
#include "cmsis_os.h"

#define NOT_FOUND		-1

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
UART_Send_String(char * string)
{
    for (; *string; string++)
    {
        //osMessageQueuePut(Queue_UART_TXHandle, *string, 0, 0);
    }
}

void
Process_Command(char * mReceiveBuffer, int32_t cmdEndline)
{
    
}

void
Thread_UART_RX(void)
{
    static char rx_msg[MAX_LOG_MSG_SIZE+1] = {0};
    static uint8_t idx = MAX_LOG_MSG_SIZE;
    
    for (;;)
    {
        uint32_t Thread_UART_RX_stack_size = osThreadGetStackSpace(UART_RXHandle);
        if ( Thread_UART_RX_stack_size > Thread_UART_RX_max_stack_size ) Thread_UART_RX_max_stack_size = Thread_UART_RX_stack_size;
        while ( HAL_UART_Receive_IT(&huart1, (uint8_t *)(&rx_msg[0]), MAX_LOG_MSG_SIZE) != HAL_OK )
        {
            //Error_Handler();
        }
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
        uint16_t UART_RX_queue_count = osMessageQueueGetCount(Queue_UART_RXHandle);
        if ( UART_RX_queue_count > UART_RX_queue_max_size ) UART_RX_queue_max_size = UART_RX_queue_count;
        
        Process_Command(&rx_msg[0], idx);
        
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
        while ( UART_TX_queue_count > 0 )
        {
            if ( UART_TX_queue_count > UART_TX_queue_max_size ) UART_TX_queue_max_size = UART_TX_queue_count;
            osMessageQueueGet(Queue_UART_TXHandle, &ch, 0, 0);
            HAL_UART_Transmit(&huart1, (uint8_t *)(&ch), 1, HAL_MAX_DELAY);
        }
        osThreadYield();
    }
}
