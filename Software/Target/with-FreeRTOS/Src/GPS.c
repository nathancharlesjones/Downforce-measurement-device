#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

#define MAX_GPS_DATA_CHARS 100

extern void Error_Handler(void);
extern osThreadId_t GPS_RXHandle;
extern osThreadId_t GPS_ProcessHandle;
extern osMessageQueueId_t Queue_UART_TXHandle;
extern osMessageQueueId_t Queue_GPSHandle;
extern UART_HandleTypeDef huart2;

static uint32_t Thread_GPS_RX_max_stack_size = 0;
static uint16_t UART_TX_queue_full_count = 0;
static uint16_t GPS_queue_max_size = 0;

void
Thread_GPS_RX(void)
{
    char GPS_data[MAX_GPS_DATA_CHARS+1];
    char* base = 0;
    uint8_t idx = 0;
    char* valid_data = NULL;
    char* speed_string = NULL;
    const char channel_name[] = "GPS";
    static char GPS_msg[MAX_LOG_MSG_SIZE+1] = {0};
    char ch;
    
    for (;;)
    {
        uint32_t Thread_GPS_RX_stack_size = osThreadGetStackSpace(GPS_RXHandle);
        if ( Thread_GPS_RX_stack_size > Thread_GPS_RX_max_stack_size ) Thread_GPS_RX_max_stack_size = Thread_GPS_RX_stack_size;
        
        while ( HAL_UART_Receive_IT(&huart2, (uint8_t *)(&ch), 1) != HAL_OK )
        {
            //Error_Handler();
        }
        
        while ( huart2.RxState != HAL_UART_STATE_READY )
        {
            osThreadYield();
        }
        
        if ( ch == '$' )
        {
            idx = 0;
            memset(&GPS_data[0], 0, MAX_GPS_DATA_CHARS);
        }
        
        if ( ch == '\r' ) //Process data
        {
            char* rest = GPS_data;
            char* token = strtok_r( rest, ",", &rest );
            if ( !strcmp( "$GPRMC", token ) )
            {
                speed_string = strtok_r( rest, ",", &rest );
                valid_data = strtok_r( rest, ",", &rest );
                if ( *valid_data == 'A' )
                {
                    sprintf(&GPS_msg[0], "{ \"channel\" : \"%s\", \"time\" : %d, \"value\" : %c%c }\n", channel_name, (int)HAL_GetTick(), speed_string[4], speed_string[5]);
    
                    for (idx = 0; GPS_msg[idx]; idx++)
                    {
                        if ( osMessageQueuePut(Queue_UART_TXHandle, &GPS_msg[idx], 0, 0) != osOK ) UART_TX_queue_full_count++;
                    }
                }
            }
                
        }
        else if ( ch != '\n' ) GPS_data[idx++] = ch;
        
        osThreadYield();
    }
}
        
        
/*
            while ( *GPS_data[idx] != '$' )
            {
                if ( *GPS_data[idx] != '\0' )
                {
                    if ( ++idx > MAX_GPS_DATA_CHARS )
                    {
                        idx = 0;
                    }
                }
                osThreadYield();
            }
            
            char* token = GPS_data[idx];
            
            while ( *GPS_data[++idx] != ',' )
            {
                temp_string
            
            if ( GPS_data[0] != "$" )
            {
                HAL_UART_AbortReceive_IT(&huart2);
                break;
            }
            
            for ( idx = MAX_GPS_DATA_CHARS; idx > 0; idx-- )
            {
                if ( GPS_data[idx] == '\n' )
                {
                    HAL_UART_AbortReceive_IT(&huart2);
                }
            }
            osThreadYield();
        }
        
        char* token = NULL;
        char* rest = GPS_data;
        while ( token = strtok_r( rest, ",\n\r", &rest ) )
        {
            if ( !strcmp( "$GPRMC", token ) )
            {
                speed_string = strtok_r( rest, ",\n\r", &rest );
                valid_data = strtok_r( rest, ",\n\r", &rest );
                if ( valid_data[0] == 'A' )
                {
                    sprintf(&GPS_msg[0], "{ \"channel\" : \"%s\", \"time\" : %d, \"value\" : %c%c }\n", channel_name, (int)HAL_GetTick(), speed_string[4], speed_string[5]);
    
                    for (idx = 0; GPS_msg[idx]; idx++)
                    {
                        if ( osMessageQueuePut(Queue_UART_TXHandle, &GPS_msg[idx], 0, 0) != osOK ) UART_TX_queue_full_count++;
                    }
                    memset(&GPS_msg[0], 0, MAX_LOG_MSG_SIZE);
                }
            }
        }
        
        osThreadYield();
    }
}

void
Thread_GPS_Process(void)
{
    char ch;
    uint8_t start = 0;
    char temp_string[100];
    uint8_t idx = 0;
    
    for (;;)
    {
        uint16_t GPS_queue_count = osMessageQueueGetCount(Queue_GPSHandle);
        while ( GPS_queue_count > 0 )
        {
            if ( GPS_queue_count > GPS_queue_max_size ) GPS_queue_max_size = GPS_queue_count;
            osMessageQueueGet(Queue_UART_TXHandle, &ch, 0, 0);
            if ( ch == '$' )
            {
                start = 1;
            }
            if ( start )
            {
                temp_string[idx++] = ch;
            }
            if ( ch == '\n' )
            {
                
}
*/
