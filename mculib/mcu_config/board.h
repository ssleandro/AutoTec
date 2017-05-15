/****************************************************************************
* Title                 :   BOARD CONFIG FILE
* Filename              :   board.h
* Author                :   Thiago Palmieri
* Origin Date           :   10/02/2016
* Version               :   1.0.1
* Compiler              :   GCC 5.2 2015q4
* Target                :   LPC43XX M4
* Notes                 :   None
*
* THIS SOFTWARE IS PROVIDED BY AUTEQ TELEMATICA "AS IS" AND ANY EXPRESSED
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL AUTEQ TELEMATICA OR ITS CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
/*************** INTERFACE CHANGE LIST **************************************
*
*     Date     Version         Author                Description
*  10/02/2016   1.0.0      Thiago Palmieri       BOARD CONFIG Created.
*  25/02/2016   1.0.1      Joao Paulo Martins    SDRAM and LCD feature
*
*****************************************************************************/
/** @file board.h
 *  @brief This module provides a Board configuration for the HAL layer
 */

#ifndef CONFIG_BOARD_H_
#define CONFIG_BOARD_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <inttypes.h>
#include "chip.h"
#include "chip_lpc43xx.h"
#include "stopwatch.h"
#include "flash_defs.h"
#include "mcuerror.h"

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
//**************************** CAN ****************************
#define CAN_MAX_BITRATE 1000000

//**************************** ADC ****************************
#define ADC_MAX_CHANNELS 2

//**************************** USB ****************************
#define USB_MAX_CHANNELS 2

/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define SDRAM_ADDR_BASE   0x28000000 // !< Base address for external SDRAM

/******************************************************************************
* Macros
*******************************************************************************/
#ifndef ASM
  #if defined(__IAR_SYSTEMS_ICC__)
    #define ASM asm
  #elif defined (__GNUC__)
    #define ASM __asm
  #endif
#endif

/* Board Timer Specification --------------------------------------------- */
#define BRD_Timer_IRQHandler  TIMER1_IRQHandler
#define BRD_Timer             LPC_TIMER1
#define BRD_Timer_IRQn        TIMER1_IRQn
#define BRD_Timer_RST         RGU_TIMER1_RST
#define BRD_Timer_Clk         CLK_MX_TIMER1


//**************************** CAN PIN MAPPING ****************************
/**
 * This private MACRO holds all the mapping between chip Port/Pin to CAN Port/Pin
 */
//CAN, PORT, TD, RD, MODE
#define CAN_PIN_MAPPING \
  X(LPC_C_CAN0, 0x3, 2, 1, SCU_MODE_FUNC2, C_CAN0_IRQn) \
  X(LPC_C_CAN0, 0xE, 3, 2, SCU_MODE_FUNC1, C_CAN0_IRQn) \
  X(LPC_C_CAN1, 0x1, 17, 18, SCU_MODE_FUNC5, C_CAN1_IRQn) \
  X(LPC_C_CAN1, 0x4, 8, 9, SCU_MODE_FUNC6, C_CAN1_IRQn) \
  X(LPC_C_CAN1, 0xE, 0, 1, SCU_MODE_FUNC5, C_CAN1_IRQn)

//**************************** PWM PIN MAPPING ****************************

/**
 * This private MACRO holds all the mapping between chip Port/Pin to PWM Port/Pin
 */
//PORT, PIN, MODE
#define PWM_PIN_MAPPING \
  X(0x4, 0, SCU_MODE_FUNC1) \
  X(0x5, 0, SCU_MODE_FUNC1) \
  X(0x5, 4, SCU_MODE_FUNC1) \
  X(0x5, 5, SCU_MODE_FUNC1) \
  X(0x5, 6, SCU_MODE_FUNC1) \
  X(0x5, 7, SCU_MODE_FUNC1) \
  X(0x9, 1, SCU_MODE_FUNC1) \
  X(0x9, 2, SCU_MODE_FUNC1) \
  X(0x9, 3, SCU_MODE_FUNC1) \
  X(0x9, 4, SCU_MODE_FUNC1) \
  X(0x9, 5, SCU_MODE_FUNC1) \
  X(0x9, 6, SCU_MODE_FUNC1) \

//**************************** SPIFI PIN MAPPING ****************************

/**
 * This private MACRO holds all the mapping between chip Port/Pin to SPIFI Port/Pin
 */
//Port, CLK, D3, D2, D1, D0, CS, HN
#define SPIFI_PIN_MAPPING \
  X(0x3, 3, 4, 5, 6, 7, 8, LPC_SPIFI_BASE) \

//**************************** ADC PIN MAPPING ****************************
/**
 * This private MACRO holds all the mapping between chip Port/Pin to ADC Port/Pin
 */
//ADC, CH, PORT, PIN, MODE
#define ADC_PIN_MAPPING \
  X(LPC_ADC0, 0, 0xc, 3, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 1, 0xc, 0, SCU_MODE_FUNC1, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 2, 0xf, 9, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 3, 0xf, 6, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 4, 0xf, 5, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 4, 0x7, 4, SCU_MODE_FUNC0, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 5, 0xf, 11, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 5, 0xf, 10, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 6, 0x7, 7, SCU_MODE_FUNC0, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC0, 7, 0xf, 7, SCU_MODE_FUNC4, ADC0_IRQn, GPDMA_CONN_ADC_0) \
  X(LPC_ADC1, 0, 0xc, 3, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 1, 0xc, 0, SCU_MODE_FUNC1, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 2, 0xf, 9, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 3, 0xf, 6, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 4, 0xf, 5, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 4, 0x7, 4, SCU_MODE_FUNC0, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 5, 0xf, 11, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 5, 0xf, 10, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 6, 0x7, 7, SCU_MODE_FUNC0, ADC1_IRQn, GPDMA_CONN_ADC_1) \
  X(LPC_ADC1, 7, 0xf, 7, SCU_MODE_FUNC4, ADC1_IRQn, GPDMA_CONN_ADC_1)

//************************** UART PIN MAPPING ****************************
/**
 * This private MACRO holds all the mapping between chip Port/Pin to UART Port/Pin
 */
//UART, PORT, TXPIN, RXPIN, MODE
#define UART_PIN_MAPPING                          \
  X(LPC_USART0, 0x02, 0x00, 0x01, SCU_MODE_FUNC1, USART0_IRQn) \
  X(LPC_USART0, 0x06, 0x04, 0x05, SCU_MODE_FUNC2, USART0_IRQn) \
  X(LPC_USART0, 0x09, 0x05, 0x06, SCU_MODE_FUNC7, USART0_IRQn) \
  X(LPC_USART0, 0x0F, 0x0A, 0x0B, SCU_MODE_FUNC1, USART0_IRQn) \
  X(LPC_UART1,  0x01, 0x0D, 0x0E, SCU_MODE_FUNC1, UART1_IRQn ) \
  X(LPC_UART1,  0x03, 0x04, 0x05, SCU_MODE_FUNC4, UART1_IRQn ) \
  X(LPC_UART1,  0x05, 0x06, 0x07, SCU_MODE_FUNC4, UART1_IRQn ) \
  X(LPC_UART1,  0x0C, 0x0D, 0x0E, SCU_MODE_FUNC2, UART1_IRQn ) \
  X(LPC_UART1,  0x0E, 0x0B, 0x0C, SCU_MODE_FUNC2, UART1_IRQn ) \
  X(LPC_USART2, 0x01, 0x0F, 0x10, SCU_MODE_FUNC1, USART2_IRQn) \
  X(LPC_USART2, 0x02, 0x0A, 0x0B, SCU_MODE_FUNC2, USART2_IRQn) \
  X(LPC_USART2, 0x07, 0x01, 0x02, SCU_MODE_FUNC6, USART2_IRQn) \
  X(LPC_USART2, 0x0A, 0x01, 0x02, SCU_MODE_FUNC3, USART2_IRQn) \
  X(LPC_USART3, 0x02, 0x03, 0x04, SCU_MODE_FUNC2, USART3_IRQn) \
  X(LPC_USART3, 0x04, 0x01, 0x02, SCU_MODE_FUNC6, USART3_IRQn) \
  X(LPC_USART3, 0x09, 0x03, 0x04, SCU_MODE_FUNC7, USART3_IRQn) \
  X(LPC_USART3, 0x0F, 0x02, 0x03, SCU_MODE_FUNC1, USART3_IRQn) \

//**************************** GPIO PIN MAPPING ****************************
/**
 * This private MACRO holds all the mapping between chip Port/Pin to GPIO Port/Pin
 */
#define GPIO_PIN_MAPPING \
  X(0, 0, 0, 0, SCU_MODE_FUNC0) \
  X(0, 1, 0, 1, SCU_MODE_FUNC0) \
  X(1, 0, 0, 4, SCU_MODE_FUNC0) \
  X(1, 1, 0, 8, SCU_MODE_FUNC0) \
  X(1, 2, 0, 9, SCU_MODE_FUNC0) \
  X(1, 3, 0, 10, SCU_MODE_FUNC0) \
  X(1, 4, 0, 11, SCU_MODE_FUNC0) \
  X(1, 5, 1, 8, SCU_MODE_FUNC0) \
  X(1, 6, 1, 9, SCU_MODE_FUNC0) \
  X(1, 7, 1, 0, SCU_MODE_FUNC0) \
  X(1, 8, 1, 1, SCU_MODE_FUNC0) \
  X(1, 9, 1, 2, SCU_MODE_FUNC0) \
  X(1, 10, 1, 3, SCU_MODE_FUNC0) \
  X(1, 11, 1, 4, SCU_MODE_FUNC0) \
  X(1, 12, 1, 5, SCU_MODE_FUNC0) \
  X(1, 13, 1, 6, SCU_MODE_FUNC0) \
  X(1, 14, 1, 7, SCU_MODE_FUNC0) \
  X(1, 15, 0, 2, SCU_MODE_FUNC0) \
  X(1, 16, 0, 3, SCU_MODE_FUNC0) \
  X(1, 17, 0, 12, SCU_MODE_FUNC0) \
  X(1, 18, 0, 13, SCU_MODE_FUNC0) \
  X(1, 20, 0, 15, SCU_MODE_FUNC0) \
  X(2, 0, 5, 0, SCU_MODE_FUNC4) \
  X(2, 1, 5, 1, SCU_MODE_FUNC4) \
  X(2, 2, 5, 2, SCU_MODE_FUNC4) \
  X(2, 3, 5, 3, SCU_MODE_FUNC4) \
  X(2, 4, 5, 4, SCU_MODE_FUNC4) \
  X(2, 5, 5, 5, SCU_MODE_FUNC4) \
  X(2, 6, 5, 6, SCU_MODE_FUNC4) \
  X(2, 7, 0, 7, SCU_MODE_FUNC0) \
  X(2, 8, 5, 7, SCU_MODE_FUNC4) \
  X(2, 9, 1, 10, SCU_MODE_FUNC0) \
  X(2, 10, 0, 14, SCU_MODE_FUNC0) \
  X(2, 11, 1, 11, SCU_MODE_FUNC0) \
  X(2, 12, 1, 12, SCU_MODE_FUNC0) \
  X(2, 13, 1, 13, SCU_MODE_FUNC0) \
  X(3, 1, 5, 8, SCU_MODE_FUNC4) \
  X(3, 2, 5, 9, SCU_MODE_FUNC4) \
  X(3, 4, 1, 14, SCU_MODE_FUNC0) \
  X(3, 5, 1, 15, SCU_MODE_FUNC0) \
  X(3, 6, 0, 6, SCU_MODE_FUNC0) \
  X(3, 7, 5, 10, SCU_MODE_FUNC4) \
  X(3, 8, 5, 11, SCU_MODE_FUNC4) \
  X(4, 0, 2, 0, SCU_MODE_FUNC0) \
  X(4, 1, 2, 1, SCU_MODE_FUNC0) \
  X(4, 2, 2, 2, SCU_MODE_FUNC0) \
  X(4, 3, 2, 3, SCU_MODE_FUNC0) \
  X(4, 4, 2, 4, SCU_MODE_FUNC0) \
  X(4, 5, 2, 5, SCU_MODE_FUNC0) \
  X(4, 6, 2, 6, SCU_MODE_FUNC0) \
  X(4, 8, 5, 12, SCU_MODE_FUNC4) \
  X(4, 9, 5, 13, SCU_MODE_FUNC4) \
  X(4, 10, 5, 14, SCU_MODE_FUNC4) \
  X(5, 0, 2, 9, SCU_MODE_FUNC0) \
  X(5, 1, 2, 10, SCU_MODE_FUNC0) \
  X(5, 2, 2, 11, SCU_MODE_FUNC0) \
  X(5, 3, 2, 12, SCU_MODE_FUNC0) \
  X(5, 4, 2, 13, SCU_MODE_FUNC0) \
  X(5, 5, 2, 14, SCU_MODE_FUNC0) \
  X(5, 6, 2, 15, SCU_MODE_FUNC0) \
  X(5, 7, 2, 7, SCU_MODE_FUNC0) \
  X(6, 1, 3, 0, SCU_MODE_FUNC0) \
  X(6, 2, 3, 1, SCU_MODE_FUNC0) \
  X(6, 3, 3, 2, SCU_MODE_FUNC0) \
  X(6, 4, 3, 3, SCU_MODE_FUNC0) \
  X(6, 5, 3, 4, SCU_MODE_FUNC0) \
  X(6, 6, 0, 5, SCU_MODE_FUNC0) \
  X(6, 7, 5, 15, SCU_MODE_FUNC4) \
  X(6, 8, 5, 16, SCU_MODE_FUNC4) \
  X(6, 9, 3, 5, SCU_MODE_FUNC0) \
  X(6, 10, 3, 6, SCU_MODE_FUNC0) \
  X(6, 11, 3, 7, SCU_MODE_FUNC0) \
  X(6, 12, 2, 8, SCU_MODE_FUNC0) \
  X(7, 0, 3, 8, SCU_MODE_FUNC0) \
  X(7, 1, 3, 9, SCU_MODE_FUNC0) \
  X(7, 2, 3, 10, SCU_MODE_FUNC0) \
  X(7, 3, 3, 11, SCU_MODE_FUNC0) \
  X(7, 4, 3, 12, SCU_MODE_FUNC0) \
  X(7, 5, 3, 13, SCU_MODE_FUNC0) \
  X(7, 6, 3, 14, SCU_MODE_FUNC0) \
  X(7, 7, 3, 15, SCU_MODE_FUNC0) \
  X(8, 0, 4, 0, SCU_MODE_FUNC0) \
  X(8, 1, 4, 1, SCU_MODE_FUNC0) \
  X(8, 2, 4, 2, SCU_MODE_FUNC0) \
  X(8, 3, 4, 3, SCU_MODE_FUNC0) \
  X(8, 4, 4, 4, SCU_MODE_FUNC0) \
  X(8, 5, 4, 5, SCU_MODE_FUNC0) \
  X(8, 6, 4, 6, SCU_MODE_FUNC0) \
  X(8, 7, 4, 7, SCU_MODE_FUNC0) \
  X(9, 0, 4, 12, SCU_MODE_FUNC0) \
  X(9, 1, 4, 13, SCU_MODE_FUNC0) \
  X(9, 2, 4, 14, SCU_MODE_FUNC0) \
  X(9, 3, 4, 15, SCU_MODE_FUNC0) \
  X(9, 4, 5, 17, SCU_MODE_FUNC4) \
  X(9, 5, 5, 18, SCU_MODE_FUNC4) \
  X(9, 6, 4, 12, SCU_MODE_FUNC0) \
  X(0xA, 1, 4, 8, SCU_MODE_FUNC0) \
  X(0xA, 2, 4, 9, SCU_MODE_FUNC0) \
  X(0xA, 3, 4, 10, SCU_MODE_FUNC0) \
  X(0xA, 4, 5, 19, SCU_MODE_FUNC4) \
  X(0xB, 0, 5, 20, SCU_MODE_FUNC4) \
  X(0xB, 1, 5, 21, SCU_MODE_FUNC4) \
  X(0xB, 2, 5, 22, SCU_MODE_FUNC4) \
  X(0xB, 3, 5, 23, SCU_MODE_FUNC4) \
  X(0xB, 4, 5, 24, SCU_MODE_FUNC4) \
  X(0xB, 5, 5, 25, SCU_MODE_FUNC4) \
  X(0xB, 6, 5, 26, SCU_MODE_FUNC4) \
  X(0xC, 1, 6, 0, SCU_MODE_FUNC4) \
  X(0xC, 2, 6, 1, SCU_MODE_FUNC4) \
  X(0xC, 3, 6, 2, SCU_MODE_FUNC4) \
  X(0xC, 4, 6, 3, SCU_MODE_FUNC4) \
  X(0xC, 5, 6, 4, SCU_MODE_FUNC4) \
  X(0xC, 6, 6, 5, SCU_MODE_FUNC4) \
  X(0xC, 7, 6, 6, SCU_MODE_FUNC4) \
  X(0xC, 8, 6, 7, SCU_MODE_FUNC4) \
  X(0xC, 9, 6, 8, SCU_MODE_FUNC4) \
  X(0xC, 10, 6, 9, SCU_MODE_FUNC4) \
  X(0xC, 11, 6, 10, SCU_MODE_FUNC4) \
  X(0xC, 12, 6, 11, SCU_MODE_FUNC4) \
  X(0xC, 13, 6, 12, SCU_MODE_FUNC4) \
  X(0xC, 14, 6, 13, SCU_MODE_FUNC4) \
  X(0xD, 0, 6, 14, SCU_MODE_FUNC4) \
  X(0xD, 1, 6, 15, SCU_MODE_FUNC4) \
  X(0xD, 2, 6, 16, SCU_MODE_FUNC4) \
  X(0xD, 3, 6, 17, SCU_MODE_FUNC4) \
  X(0xD, 4, 6, 18, SCU_MODE_FUNC4) \
  X(0xD, 5, 6, 19, SCU_MODE_FUNC4) \
  X(0xD, 6, 6, 20, SCU_MODE_FUNC4) \
  X(0xD, 7, 6, 21, SCU_MODE_FUNC4) \
  X(0xD, 8, 6, 22, SCU_MODE_FUNC4) \
  X(0xD, 9, 6, 23, SCU_MODE_FUNC4) \
  X(0xD, 10, 6, 24, SCU_MODE_FUNC4) \
  X(0xD, 11, 6, 25, SCU_MODE_FUNC4) \
  X(0xD, 12, 6, 26, SCU_MODE_FUNC4) \
  X(0xD, 13, 6, 27, SCU_MODE_FUNC4) \
  X(0xD, 14, 6, 28, SCU_MODE_FUNC4) \
  X(0xD, 15, 6, 29, SCU_MODE_FUNC4) \
  X(0xD, 16, 6, 30, SCU_MODE_FUNC4) \
  X(0xE, 0, 7, 0, SCU_MODE_FUNC4) \
  X(0xE, 1, 7, 1, SCU_MODE_FUNC4) \
  X(0xE, 2, 7, 2, SCU_MODE_FUNC4) \
  X(0xE, 3, 7, 3, SCU_MODE_FUNC4) \
  X(0xE, 4, 7, 4, SCU_MODE_FUNC4) \
  X(0xE, 5, 7, 5, SCU_MODE_FUNC4) \
  X(0xE, 6, 7, 6, SCU_MODE_FUNC4) \
  X(0xE, 7, 7, 7, SCU_MODE_FUNC4) \
  X(0xE, 8, 7, 8, SCU_MODE_FUNC4) \
  X(0xE, 9, 7, 9, SCU_MODE_FUNC4) \
  X(0xE, 10, 7, 10, SCU_MODE_FUNC4) \
  X(0xE, 11, 7, 11, SCU_MODE_FUNC4) \
  X(0xE, 12, 7, 12, SCU_MODE_FUNC4) \
  X(0xE, 13, 7, 13, SCU_MODE_FUNC4) \
  X(0xE, 14, 7, 14, SCU_MODE_FUNC4) \
  X(0xE, 15, 7, 15, SCU_MODE_FUNC4) \
  X(0xF, 1, 7, 16, SCU_MODE_FUNC4) \
  X(0xF, 2, 7, 17, SCU_MODE_FUNC4) \
  X(0xF, 3, 7, 18, SCU_MODE_FUNC4) \
  X(0xF, 5, 7, 19, SCU_MODE_FUNC4) \
  X(0xF, 6, 7, 20, SCU_MODE_FUNC4) \
  X(0xF, 7, 7, 21, SCU_MODE_FUNC4) \
  X(0xF, 8, 7, 22, SCU_MODE_FUNC4) \
  X(0xF, 9, 7, 23, SCU_MODE_FUNC4) \
  X(0xF, 10, 7, 24, SCU_MODE_FUNC4) \
  X(0xF, 11, 7, 25, SCU_MODE_FUNC4)

//**************************** USB PIN MAPPING ****************************
/**
 * This private MACRO holds all the mapping between chip Port/Pin to USB Port/Pin
 */
#define USB_PIN_MAPPING \
  X(LPC_USB0_BASE, LPC_USB0, USB0_IRQn, Chip_USB0_Init) \
  X(LPC_USB1_BASE, LPC_USB1, USB1_IRQn, Chip_USB1_Init) \


/* ************************ LCD PIN MAPPING ******************************* */
/**
 * This private MACRO holds all the mapping between chip Port/Pin to EMC peripheral
 */
#define LCD_PIN_MAPPING \
  X(4,  1, SCU_MODE_FUNC2) \
  X(4,  4, SCU_MODE_FUNC2) \
  X(4,  3, SCU_MODE_FUNC2) \
  X(4,  2, SCU_MODE_FUNC2) \
  X(8,  7, SCU_MODE_FUNC3) \
  X(8,  6, SCU_MODE_FUNC3) \
  X(8,  5, SCU_MODE_FUNC3) \
  X(8,  4, SCU_MODE_FUNC3) \
  X(7,  5, SCU_MODE_FUNC3) \
  X(4,  8, SCU_MODE_FUNC2) \
  X(4,  10,SCU_MODE_FUNC2) \
  X(4,  9, SCU_MODE_FUNC2) \
  X(8,  3, SCU_MODE_FUNC3) \
  X(11, 6, SCU_MODE_FUNC2) \
  X(11, 5, SCU_MODE_FUNC2) \
  X(11, 4, SCU_MODE_FUNC2) \
  X(7,  4, SCU_MODE_FUNC3) \
  X(7,  3, SCU_MODE_FUNC3) \
  X(7,  2, SCU_MODE_FUNC3) \
  X(7,  1, SCU_MODE_FUNC3) \
  X(11, 3, SCU_MODE_FUNC2) \
  X(11, 2, SCU_MODE_FUNC2) \
  X(11, 1, SCU_MODE_FUNC2) \
  X(11, 0, SCU_MODE_FUNC2) \
  X(4,  5, SCU_MODE_FUNC2) \
  X(4,  7, SCU_MODE_FUNC0) \
  X(8,  1, SCU_MODE_FUNC0) \
  X(7,  7, SCU_MODE_FUNC3) \
  X(4,  6, SCU_MODE_FUNC2) \
  X(7,  6, SCU_MODE_FUNC3)


/* ************************ SDRAM PIN MAPPING ******************************* */
/**
 * EMC PIN MAPPING PROTOG
 *
 * External data lines D0 .. D15
 * P1_7 - D0 / P1_8 = D1 / P1_9 = D2 / P1_10 = D3 / P1_11 = D4 / P1_12 = D5 / P1_13 = D6 / P1_14 = D7
 * P1_15 - D8 / P1_16 = D9 / P1_18 = D10 / P1_20 = D11 / P5_0 = D12 / P5_1 = D13 / P5_2 = D14 / P5_3 = D15
 *
 * Address lines A0 .. A12
 * P2_9 - A0 / P2_10 = A1 / P2_11 = A2 / P2_12 = A3 / P2_13 = A4 / P1_0 = A5 / P1_1 = A6 / P1_2 = A7
 * P2_8 - A8 / P2_7 = A9 / P2_6 = A10 / P2_2 = A11 / P2_1 = A12
 *
 * EMC control signals
 * P2_0 - A13/BA0 / P6_8 = A14/BA1 / P6_12 = DQM0 / P6_10 = DQM1 / P6_11 = CKE0 / CLK0 = CLK0 / P1_6 = nWE / P6_9 = DYCS0
 * P6_4 - nCAS / P6_5 = nRAS
 *
 */
/**
 * This private MACRO holds all the mapping between chip Port/Pin to EMC peripheral
 */
#if defined (M2G_PROTOB) | defined (TDA3500_PROTO1)
#define EMC_SDRAM_PIN_MAPPING \
  X(1, 0, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(1, 1, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(1, 2, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(1, 6, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 7, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 8, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 9, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 10, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 11, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 12, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 13, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 14, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(1, 15, (SCU_PINIO_FAST | SCU_MODE_FUNC6)) \
  X(1, 16, (SCU_PINIO_FAST | SCU_MODE_FUNC6)) \
  X(1, 18, (SCU_PINIO_FAST | SCU_MODE_FUNC7)) \
  X(1, 20, (SCU_PINIO_FAST | SCU_MODE_FUNC7)) \
  X(2, 0, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(2, 1, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(2, 2, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(2, 6, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(2, 7, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(2, 8, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(2, 9, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(2, 10, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(2, 11, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(2, 12, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(2, 13, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(5, 0, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(5, 1, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(5, 2, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(5, 3, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(6, 4, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(6, 5, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(6, 8, (SCU_PINIO_FAST | SCU_MODE_FUNC1)) \
  X(6, 9, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(6, 10, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(6, 11, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(6, 12, (SCU_PINIO_FAST | SCU_MODE_FUNC3))
#elif defined (TEST_BOARD)
#error "Pinmux to test board kit not defined."
#elif defined (EA_LPC4357)
#define EMC_SDRAM_PIN_MAPPING \
  X(0x1,  7, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1,  8, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1,  9, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1, 10, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1, 11, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1, 12, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1, 13, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1, 14, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x5,  4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  6, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  7, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  0, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  1, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  2, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x5,  3, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  2, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  3, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  6, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  7, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  8, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD,  9, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xE,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  6, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  7, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  8, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  9, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE, 10, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE, 11, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE, 12, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2,  9, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2, 10, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2, 11, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2, 12, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2, 13, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1,  0, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x1,  1, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x1,  2, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x2,  8, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2,  7, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x2,  6, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x2,  2, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x2,  1, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x2,  0, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x6,  8, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC1)) \
  X(0x6,  7, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC1)) \
  X(0xD, 16, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD, 15, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xE,  0, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  1, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  2, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  3, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xE,  4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xA,  4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1,  4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x6,  6, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC1)) \
  X(0xD, 13, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xD, 10, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0x6,  9, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1,  6, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x6,  4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x6,  5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x6, 11, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x6, 12, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x6, 10, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xD,  0, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xE, 13, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0x1,  3, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(0x1,  5, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(0x6,  3, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(0xD, 12, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(0xD, 11, (SCU_PINIO_FAST | SCU_MODE_FUNC2)) \
  X(0x6,  1, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC1)) \
  X(0xD, 14, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC2)) \
  X(0xF, 14, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3)) \
  X(0xD, 13, (SCU_PINIO_FAST | SCU_MODE_FUNC3)) \
  X(0xE, 14, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC3))
#else
#error "Pinmux to EMC dynamic controller not defined."
#endif


/******************************************************************************
* Typedefs
*******************************************************************************/
//**************************** UART ****************************

/**
 * This enum defines which UART is connected into the board and what position
 * to use as reference configuration from the UART_PIN_MAPPING MACRO
 */
typedef enum eUARTs
{
#if defined (TEST_BOARD)
  UART0b = 3,
  UART1b = 8,
  UART2b = 17,
  UART3b = 13,
  UART4b = 17,
#elif defined (TDA3500_PROTO1)
  UART0b = 3,
  UART1b = 8,
  UART2b = 17,
  UART3b = 13,
  UART4b = 17,
#elif defined (M2G_PROTOB)
  UART0b = 3,
  UART1b = 6,
  UART2b = 12,
  UART3b = 13,
  UART4b = 17,
#elif defined (EA_LPC4357)
#define USART3_TXD
  UART0b = 3,
  UART1b = 8,
  UART2b = 12,
#if defined (USART3_TXD)
  UART3b = 15,
#elif defined (USART3_RXD)
  UART3b = 13,
#else
  UART3b = 17,
#endif
  UART4b = 17,
#endif
} eUARTs;

//**************************** CAN ****************************

/**
 * This enum defines which CAN is connected into the board and what position
 * to use as reference configuration from the CAN_PIN_MAPPING MACRO
 */
typedef enum eCANs  //See CAN_PIN_MAPPING
{
#if defined (TEST_BOARD)
  CAN0b = 0,              //!< CAN0 position on the CAN_PIN_MAPPING
  CAN1b = 4,              //!< CAN1 position on the CAN_PIN_MAPPING
#elif defined (TDA3500_PROTO1)
  CAN0b = 1,              //!< CAN0 position on the CAN_PIN_MAPPING
  CAN1b = 3,              //!< CAN1 position on the CAN_PIN_MAPPING
#elif defined (M2G_PROTOB)
  CAN0b = 1,              //!< CAN0 position on the CAN_PIN_MAPPING
  CAN1b = 4,              //!< CAN1 position on the CAN_PIN_MAPPING
#elif defined (EA_LPC4357)
  CAN0b = 1,              //!< CAN0 position on the CAN_PIN_MAPPING
  CAN1b = 3,              //!< CAN1 position on the CAN_PIN_MAPPING
#endif
} eCANs;

/**
 * This private Struct defines mapping between chip pins and CAN
 */
typedef struct CANConverter_s
{
  LPC_CCAN_T *pCAN;      //!< CAN pointer to Memory structure
  uint8_t bCANPort;      //!< Chip Port
  uint8_t bCANPinTD;       //!< Chip TD Pin
  uint8_t bCANPinRD;       //!< Chip RD Pin
  uint8_t bModeFunc;     //!< CAN Pin Function
  IRQn_Type CANIntIRQ;   //!< CAN Interrupt IRQ
} CANConverter_s;

//**************************** PWM ****************************

/**
 * This enum defines which PWM is connected into the board and what position
 * to use as reference configuration from the PWM_PIN_MAPPING MACRO
 */
typedef enum ePWMs
{
#if defined (TEST_BOARD)
  PWM0b = 8,      // Adjusted to schematics
  PWM1b = 10,     // Adjusted to schematics
  PWM2b = 6,      // Adjusted to schematics
#elif defined (TDA)
  PWM0b = 2,    // Adjusted to schematics
  PWM1b = 11,   // Adjusted to schematics
  PWM2b = 7,    // Adjusted to schematics
#elif defined (M2G_PROTOB)
  PWM0b = 8,    // Adjusted to schematics
  PWM1b = 10,   // Adjusted to schematics
  PWM2b = 7,    // Adjusted to schematics
#elif defined (EA_LPC4357)
  PWM0b = 2,    // Adjusted to schematics
  PWM1b = 11,   // Adjusted to schematics
  PWM2b = 7,    // Adjusted to schematics
#endif
} ePWMs;
/**
 * This private Struct defines mapping between chip pins and PWM
 */
typedef struct PWMConverter_s
{
  uint8_t bPWMPort;     //!< PWM port
  uint8_t bPWMPin;      //!< PWM Pin
  uint8_t bModeFunc;    //!< PWM Pin Function
} PWMConverter_s;

//**************************** SPIFI ****************************
/**
 * This private Struct defines mapping between chip pins and SPIFI
 */
typedef struct SPIFIConverter_s
{
  uint8_t bSPIFIPort;     //!< SPIFI port
  uint8_t bSPIFICLK;      //!< SPIFI Clock
  uint8_t bSPIFID3;       //!< SPIFI D3
  uint8_t bSPIFID2;       //!< SPIFI D2
  uint8_t bSPIFID1;       //!< SPIFI D1
  uint8_t bSPIFID0;       //!< SPIFI D0
  uint8_t bSPIFICS;       //!< SPIFI CS
  uint32_t bSPIFIAddress; //!< SPIFI Register Address
} SPIFIConverter_s;

//**************************** GPIO ****************************
/**
 * This private Struct defines mapping between chip pins and GPIO
 */
typedef struct gpioConverter_s
{
  uint8_t bMainPort;  //!< Chip Port
  uint8_t bMainPin;   //!< Chip Pin
  uint8_t bGPIOPort;  //!< GPIO Port
  uint8_t bGPIOPin;   //!< GPIO Pin
  uint8_t bModeFunc;  //!< GPIO Pin Function
} gpioConverter_s;

//**************************** ADC ****************************
/**
 * This enum defines a list of valid ADC Channels to be used with pins
 * that are not pinmuxed.
 */
typedef enum eChannels
{
  ADC_CHANNEL0  = 0,  //!< ADC Channel 0
  ADC_CHANNEL1  = 1,  //!< ADC Channel 1
  ADC_CHANNEL2  = 2,  //!< ADC Channel 2
  ADC_CHANNEL3  = 3,  //!< ADC Channel 3
  ADC_CHANNEL4  = 4,  //!< ADC Channel 4
  ADC_CHANNEL5  = 5,  //!< ADC Channel 5
  ADC_CHANNEL6  = 6,  //!< ADC Channel 6
  ADC_CHANNEL7  = 7   //!< ADC Channel 7
} eChannels;

/**
 * This array is used to decide if PinMux is required for each ADC channel.
 * If not required, then each channel should have a value larger than the size
 * of ADC_PIN_MAPPING
 */
//uint8_t bUsedADC[ADC_MAX_CHANNELS];

/**
 * This enum defines which ADC/Channel is connected into the board and what position
 * to use as reference configuration from the ADC_PIN_MAPPING MACRO
 */
typedef enum eADCs  //See ADC_PIN_MAPPING
{
#if defined (TEST_BOARD)
  ADC0b = 1,              //!< ADC0 position on the ADC_PIN_MAPPING
  ADC1b = 14,              //!< ADC1 position on the ADC_PIN_MAPPING
#elif defined (TDA3500_PROTO1)
  ADC0b = 1,                //!< ADC0 position on the ADC_PIN_MAPPING
  ADC1b = 11,               //!< ADC1 position on the ADC_PIN_MAPPING
#elif defined (M2G_PROTOB)
  ADC0b = 1,                //!< ADC0 position on the ADC_PIN_MAPPING
  ADC1b = 11,               //!< ADC1 position on the ADC_PIN_MAPPING
#elif defined (EA_LPC4357)
  ADC0b = 1,                //!< ADC0 position on the ADC_PIN_MAPPING
  ADC1b = 11,               //!< ADC1 position on the ADC_PIN_MAPPING
#endif
} eADCs;

/**
 * This private Struct defines mapping between chip pins and ADC
 */
typedef struct ADCConverter_s
{
  LPC_ADC_T *pADC;      //!< ADC pointer to Memory structure
  uint8_t bADCChannel;  //!< ADC channel
  uint8_t bADCPort;     //!< Chip Port
  uint8_t bADCPin;      //!< Chip Pin
  uint8_t bModeFunc;    //!< ADC Pin Function
  IRQn_Type ADCIntIRQ;  //!< ADC Interrupt IRQ
  uint32_t wGPDMA;      //!< ADC GPDMA Connection
} ADCConverter_s;

//**************************** SPIFI ****************************

/**
 * This enum defines which SPIFI is connected into the board and what position
 * to use as reference configuration from the SPIFI_PIN_MAPPING MACRO
 */
typedef enum eSPIFIs  //See SPIFI_PIN_MAPPING
{
  SPIFI0b = 0,              //!< SPIFI position on the SPIFI_PIN_MAPPING
} eSPIFIs;
//**************************** USB ****************************

/**
 * This enum defines which USB/Channel is connected into the board and what position
 * to use as reference configuration from the USB_PIN_MAPPING MACRO
 */
typedef enum eUSBs  //See USB_PIN_MAPPING
{
  USB0b = 0,              //!< USB0 position on the USB_PIN_MAPPING
  USB1b = 1,              //!< USB1 position on the USB_PIN_MAPPING
} eUSBs;

/**
 * This private Struct defines mapping between chip pins and USB
 */
typedef struct USBConverter_s
{
  uint32_t wLPCUSBBase;           //!< USB Memory Base Address
  LPC_USBHS_T * pUSB;             //!< USB pointer to Memory structure
  IRQn_Type eLPCUSBIRQ;           //!< USB Interrupt IRQ
  void (*USBInitPinClk)(void);    //!< USB Init function pointer
} USBConverter_s;

//**************************** UART ****************************

/**
 * This private Struct defines mapping between chip pins and UART devices
 */
typedef struct uartConverter_s
{
  LPC_USART_T *pRegister; //!< UART Register Base
  uint8_t     bPort;      //!< Chip Port
  uint8_t     bTxPin;     //!< TX Pin
  uint8_t     bRxPin;     //!< RX Pin
  uint16_t    iModeFunc;  //!< PinMux Mask
  int         iIRQn;      //!< IRQn
} uartConverter_s;



/******************************************************************************
* Variables
*******************************************************************************/
//uint32_t SystemCoreClock; //!< System Clock Frequency (Core Clock)

extern CANConverter_s sCANMap[];    //!< PIN MUX mapping array for CAN
extern PWMConverter_s sPWMMap[];    //!< PIN MUX mapping array for PWM
extern SPIFIConverter_s sSPIFIMap[];    //!< PIN MUX mapping array for SPIFI
extern ADCConverter_s sADCMap[];    //!< PIN MUX mapping array for ADC
extern gpioConverter_s sGPIOMap[];  //!< PIN MUX mapping array for GPIO
extern USBConverter_s sUSBMap[];    //!< PIN MUX mapping array for USB
extern uartConverter_s sUARTMap[];  //!< PIN MUX mapping array for UART
extern PINMUX_GRP_T sSDRAMMap[];    //!< PIN MUX mapping array for SDRAM
extern PINMUX_GRP_T sLCDMap[];      //!< PIN MUX mapping array for LCD

extern uint32_t bGPIOMapSize; //!<  Size of map struct for GPIO
extern uint32_t bADCMapSize;  //!<  Size of map struct for ADC

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
* Function : BRD_vWait_ms (uint32_t val);
*//**
* \b Description:
*
* This is a public function used to block the execution of the firmware for
* a specific time in milliseconds. This is done using a peripheral timer.
*
* PRE-CONDITION: Timer peripheral avaliable;
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Defines that selects the timer to be used at the function
* #define BRD_Timer_IRQHandler  TIMER1_IRQHandler
* #define BRD_Timer             LPC_TIMER1
* #define BRD_Timer_IRQn        TIMER1_IRQn
* #define BRD_Timer_RST         RGU_TIMER1_RST
* #define BRD_Timer_Clk         CLK_MX_TIMER1
*
* BRD_vWait_ms(100); //block execution during 100 ms
*
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_vWait_ms (uint32_t ms);

/******************************************************************************
* Function : BRD_vWait_us (uint32_t dMicroSecs)
*//**
* \b Description:
*
* This is a public function used to block the execution of the firmware for
* a specific time in microseconds. This is done using a NOP instructions.
*
* ATTENTION: Need to review implementation when clock changes.
*
* PRE-CONDITION: Timer peripheral avaliable;
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* BRD_vWait_us(500); //block execution during 500 us
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_vWait_us (uint32_t us);

/******************************************************************************
* Function : SystemInit(void)
*//**
* \b Description:
*
* This is a public function used to configure a board vector list.
* This function must be called just after BRD_vSystemCoreClockUpdate().
* This function calls BRD_vSetupClocking to setup initial clocking.
*
* PRE-CONDITION: None
*
* POST-CONDITION: Vector array mapped
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void SystemInit(void);

/******************************************************************************
* Function : BRD_vSystemCoreClockUpdate(void)
*//**
* \b Description:
*
* This is a public function used update SystemCoreClock variable with the
* clock value currently being used by the MCU.
*
* PRE-CONDITION: None
*
* POST-CONDITION: SystemCoreClock updated with system clock value
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   PRINTF("The system clock is %l", SystemCoreClock);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_vSystemCoreClockUpdate(void);

/******************************************************************************
* Function : BRD_vSetupClocking(void)
*//**
* \b Description:
*
* This is a public function used to setup all clocks needed by the system.
* This function is called from the BRD_BoardConfig.
*
* PRE-CONDITION: None
*
* POST-CONDITION: System clocks started
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   PRINTF("The system clock is %l", SystemCoreClock);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_vSetupClocking(void);

void BRD_SetupMuxing(void);

void BRD_SystemInit(void);

/******************************************************************************
* Function : BRD_UARTConfig(void)
*//**
* \b Description:
*
* This is a public function used to initialize and pinmux UART interfaces.
*
* PRE-CONDITION: A valid PIN MUX UART array defined
*
* POST-CONDITION: UART initialized
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_UARTConfig();
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, UART_PIN_MAPPING, UARTConverter_s
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_UARTConfig(uint8_t UART_Channel);

/******************************************************************************
* Function : BRD_SPIFIConfig(void)
*//**
* \b Description:
*
* This is a public function used to initialize and pinmux a given SPIFI interfaces.
*
* PRE-CONDITION: A Valid SPIFI channel
*
* POST-CONDITION: SPIFI PINMUX executed
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_SPIFIConfig(SPIFI0);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, SPIFI_PIN_MAPPING, SPIFIConverter_s
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 11/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_SPIFIConfig(void);

/******************************************************************************
* Function : BRD_PWMConfig(uint8_t PWMChannel)
*//**
* \b Description:
*
* This is a public function used to initialize and pinmux a given PWM interfaces.
*
* PRE-CONDITION: A Valid PWM channel, and a valid PIN MUX PWM array defined
*
* POST-CONDITION: PWM Channel initialized
*
* @return     The base clock rate used by PWM interface
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_PWMConfig(PWM0);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, PWM_PIN_MAPPING, PWMConverter_s
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t BRD_PWMConfig(uint8_t PWMChannel);

/******************************************************************************
* Function : BRD_CANConfig(uint8_t CANChannel)
*//**
* \b Description:
*
* This is a public function used to initialize and pinmux a given CAN port.
*
* PRE-CONDITION: A Valid CAN port, and a valid PIN MUX CAN array defined
*
* POST-CONDITION: CAN port initialized
*
* @return     CAN clock rate
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_CANConfig(CAN0);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, CAN_PIN_MAPPING, CANConverter_s
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t BRD_CANConfig(uint8_t CANChannel);

/******************************************************************************
* Function : BRD_ADCConfig(uint8_t ADCChannel)
*//**
* \b Description:
*
* This is a public function used to initialize and pinmux a given ADC port.
*
* PRE-CONDITION: A Valid ADC port, and a valid PIN MUX ADC array defined
*
* POST-CONDITION: ADC port initialized
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_ADCConfig(ADC0);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, ADC_PIN_MAPPING, ADCConverter_s
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_ADCConfig(uint8_t ADCChannel);

/******************************************************************************
* Function : BRD_GPIOConfig(uint8_t GPIOChannel, uint8_t bPull)
*//**
* \b Description:
*
* This is a public function used to initialize and pinmux a given GPIO port.
*
* PRE-CONDITION: A Valid GPIO port and pull, and a valid PIN MUX GPIO array defined
*
* POST-CONDITION: GPIO port initialized
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_GPIOConfig(GPIO0, GPIO_PULL_UP);
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, GPIO_PIN_MAPPING, GPIOConverter_s
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_GPIOConfig(uint8_t GPIOChannel, uint8_t bPull);

/******************************************************************************
* Function : void BRD_LCDConfig(void);
*//**
* \b Description:
*
* This is a public function to initialize the hardware and pinmux of
* the LCD controller peripheral. This function also enables an external RAM,
* configuring the pinmux and parameters for proper operation.
*
* PRE-CONDITION: A LCD connected to the board via LCD controller interface, an
* external RAM connected to the processor.
*
* POST-CONDITION: LCD pinmux configured. External Memory pinmux configured and running.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* int main(void)
* {
*   BRD_vSystemCoreClockUpdate();
*   BRD_BoardConfig();
*   BRD_LCDConfig();
* }
~~~~~~~~~~~~~~~
*
* @see BRD_vSystemCoreClockUpdate, BRD_vSetupClocking, BRD_BoardConfig, LCD_PIN_MAPPING, EMC_SDRAM_PIN_MAPPING
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_LCDConfig(void);


/******************************************************************************
* Function : BRD_ExtMemoryConfig(void)
*//**
* \b Description:
*
* This function initializes the pins connected to an external SDRAM memory and
* enables the device for operation.
*
* PRE-CONDITION: An array of PINMUX_GRP_T elements, one for each of SDRAM controller pins.
*
* POST-CONDITION: An external SDRAM device enabled and operational, acessible by the system
* with base address 0x28000000.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* PINMUX_GRP_T sSDRAMMap[] = {.pingrp = 1, .pinnum = 0, .modefunc = (SCU_PINIO_FAST | SCU_MODE_FUNC2 | SCU_MODE_PULLUP)}, \
  {.pingrp = 1, .pinnum = 1, .modefunc = (SCU_PINIO_FAST | SCU_MODE_FUNC2 | SCU_MODE_PULLUP)}, \
  {.pingrp = 1, .pinnum = 2, .modefunc = (SCU_PINIO_FAST | SCU_MODE_FUNC2 | SCU_MODE_PULLUP)}, \
  {.pingrp = 1, .pinnum = 3, .modefunc = (SCU_PINIO_FAST | SCU_MODE_FUNC2 | SCU_MODE_PULLUP)}, \
  {.pingrp = 1, .pinnum = 4, .modefunc = (SCU_PINIO_FAST | SCU_MODE_FUNC2 | SCU_MODE_PULLUP)}, \
  {.pingrp = 1, .pinnum = 5, .modefunc = (SCU_PINIO_FAST | SCU_MODE_FUNC2 | SCU_MODE_PULLUP)};

  pvt_BRD_SDRAMConfig();
*
*
*
~~~~~~~~~~~~~~~
*
* @see PINMUX_GRP_T, EMC_SDRAM_PIN_MAPPING, pvt_BRD_dNano2Clock
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void BRD_ExtMemoryConfig(void);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* CONFIG_BOARD_H_ */
