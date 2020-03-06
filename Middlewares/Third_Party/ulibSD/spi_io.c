/*
 *  File: spi_io.c.example
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

#include <stdio.h>
#include "spi_io.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;
/*
extern TIM_HandleTypeDef htim1;
uint32_t * GPIOA_BSRR = (uint32_t *)0x40010810;
uint32_t * GPIOA_BRR = (uint32_t *)0x40010814;
uint16_t SPI_NSS_Pin_Pos = 4u;
uint32_t SPI_NSS_Pin_Mask = (0x1 << 4);
uint32_t * RCC_CFGR = (uint32_t *)0x40021004;
uint16_t PPRE2_Pos = 11u;
uint32_t PPRE2_Mask = (0x111 << 11);
uint32_t * SPI_CR1 = (uint32_t *)0x40013000;
uint16_t SPI_BR_Pos = 3u;
uint32_t SPI_BR_Mask = (0x111 << 3);
*/

uint32_t spiTimerTickStart;
uint32_t spiTimerTickDelay;


/******************************************************************************
 Module Public Functions - Low level SPI control functions
******************************************************************************/

void SPI_Init (void) {
}

BYTE SPI_RW (BYTE d) {
    uint8_t rx_data = 0, tx_data = d;
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)(&tx_data), &rx_data, 1, 0);
    return (rx_data);
}

void SPI_Release (void) {
    WORD idx;
    for (idx=512; idx && (SPI_RW(0xFF)!=0xFF); idx--);
}

inline void SPI_CS_Low (void) {
    HAL_GPIO_WritePin(SD_NSS_GPIO_Port, SD_NSS_Pin, GPIO_PIN_RESET);
}

inline void SPI_CS_High (void){
    HAL_GPIO_WritePin(SD_NSS_GPIO_Port, SD_NSS_Pin, GPIO_PIN_SET);
}

inline void SPI_Freq_High (void) {
    //De-initialize SPI
    HAL_SPI_DeInit(&hspi1);
    //Set init struct baudrate prescaler to 2
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    //Re-initialize SPI
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}

inline void SPI_Freq_Low (void) {
    //De-initialize SPI
    HAL_SPI_DeInit(&hspi1);
    //Set init struct baudrate prescaler to 256
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    //Re-initialize SPI
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
    
    /*
    if ( *RCC_CFGR & PPRE2_Mask )
    {
        //APB2 prescaler is >1 and APB2 timer clock is 2x APB2 clock
        if ( ( HAL_RCC_GetPCLK2Freq() * 2 / 256 ) > 400000 )
        {
            Error_Handler();
        }
    }
    else
    {
        //APB2 prescaler is x1 and APB2 timer clock = APB2 clock
        if ( ( HAL_RCC_GetPCLK2Freq() / 256 ) > 400000 )
        {
            Error_Handler();
        }
    }
    *SPI_CR1 |= SPI_BR_Mask;
    */
}

void SPI_Timer_On (WORD ms) {
    spiTimerTickStart = HAL_GetTick();
    spiTimerTickDelay = ms;
}

inline BOOL SPI_Timer_Status (void) {
    return ((HAL_GetTick() - spiTimerTickStart) < spiTimerTickDelay);
}

inline void SPI_Timer_Off (void) {
    
}

#ifdef SPI_DEBUG_OSC
inline void SPI_Debug_Init(void)
{
}
inline void SPI_Debug_Mark(void)
{
}
#endif

/*
The MIT License (MIT)

Copyright (c) 2015 Nelson Lombardo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
