/*******************************************************************************
 * Title                 :	UART support for MCULIB
 * Filename              :   uart.c
 * Author                :   Joao Paulo Martins
 * Origin Date           :   04/02/2016
 * Version               :   1.0.5
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
 *******************************************************************************/
/*************** SOURCE REVISION LOG *****************************************
 *
 *    Date    Version      Author               Description
 *  04/02/16   1.0.0  Joao Paulo Martins   First design of UART API.
 *  10/02/16   1.0.1  Joao Paulo Martins   Addition of the board config files
 *  19/02/16   1.0.2  Joao Paulo Martins   Reception by interrupt feature
 *  04/03/16   1.0.3  Joao Paulo Martins   Fixing minor details
 *  09/03/16   1.0.4  Joao Paulo Martins   Added DEINIT function
 *  31/03/16		1.0.5	 Joao Paulo Martins		Include RTOS header to use SysTick
 *
 *******************************************************************************/
/** @file uart.c
 *  @brief Source file of UART driver on top of LPCOpen implementation
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include <stdint.h>				/* For portable types */
#include <stdlib.h>
#include "uart.h"
#include "board.h"
#ifdef USE_SYSVIEW
#include "SEGGER.h"
#include "SEGGER_SYSVIEW.h"
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static uart_config_s * psUARTList[UART_INVALID];  //!< UART List

static uint8_t sUARTIndex[5] = { UART0b, UART1b, UART2b, UART3b, UART4b };  //!< UART map index
#define UART_MAP_REGISTER(x)   sUARTMap[sUARTIndex[x]].pRegister  //!< UART REGISTER map macro
#define UART_MAP_IRQn(x)       sUARTMap[sUARTIndex[x]].iIRQn    //!< UART IRQ map macro

// Ring buffer receive option
/* Transmit and receive ring buffers */
static RINGBUFF_T txring, rxring;

/* Ring buffer size */
#define UART_RB_SIZE 512

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RB_SIZE], txbuff[UART_RB_SIZE];

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static uint32_t pvt_StopBits (uart_config_s *pUART);
static uint32_t pvt_WordLenght (uart_config_s *pUART);
static uint32_t pvt_Parity (uart_config_s *pUART);
static int32_t pvt_ISR_ReadRxBuffer (uart_config_s *pUART);
static void pvt_ISR_eRxInterrupt (uart_channel_e eListIndex);
static void pvt_ISR_eRBRxInterrupt (uart_channel_e eListIndex);

extern void UART0_IRQHandler (void);
extern void UART1_IRQHandler (void);
extern void UART2_IRQHandler (void);
extern void UART3_IRQHandler (void);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

/******************************************************************************
 * Function : pvt_StopBits(uart_config_s *pUART)
 *//**
 * \b Description:
 *
 * Returns the UART LCR register mask for the number of stop bits of a UART frame
 * corresponding the uart_config_s stopbits field.
 *
 * PRE-CONDITION: valid uart_config_s handle
 *
 * POST-CONDITION: none
 *
 * @return Returns a uint32_t mask to set the UART LCR register configuring the stop bits
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uart_config_s uart_init;
 * uint32_t lcr_mask = 0x00000000;
 *
 * // Generic configuration
 * uart_init.eChannel     = UART1;         // enumerator that list UART devices
 * uart_init.bStopbits    = UART_STOPBITS2; // generic definition of 2 stopbits
 *
 * // Config the UART LCR register to set stopbits
 * lcr_mask |= pvtStopBits(uart_init);
 * LPC_UART1->LCR |= lcr_mask;
 *
 ~~~~~~~~~~~~~~~
 *
 * @see
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static uint32_t pvt_StopBits (uart_config_s *pUART)
{
	uint32_t ret = 0x00000000;

	if (pUART->eStopbits == UART_STOPBITS2)
	{
		ret |= UART_LCR_SBS_2BIT;
	}
	else
	{
		pUART->eStopbits = UART_STOPBITS1;
		ret |= UART_LCR_SBS_1BIT;
	}

	return ret;
}

/******************************************************************************
 * Function : pvt_WordLenght(uart_config_s *pUART)
 *//**
 * \b Description:
 *
 * Returns the UART LCR register mask for the correct word lenght of a UART frame
 * corresponding the uart_config_s wordlenght field.
 *
 * PRE-CONDITION: valid uart_config_s handle
 *
 * POST-CONDITION: none
 *
 * @return Returns a uint32_t mask to set the UART LCR register configuring the
 *         word lenght
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uart_config_s uart_init;
 * uint32_t lcr_mask = 0x00000000;
 *
 * // Generic configuration
 * uart_init.eChannel     = UART1;         // enumerator that list UART devices
 * uart_init.bWordLenght  = UART_WORD_8B;   // generic definition of 8-bit word lenght
 *
 * // Config the UART LCR register to word lenght
 * lcr_mask |= pvtWordLenght(uart_init);
 * LPC_UART1->LCR |= lcr_mask;
 *
 ~~~~~~~~~~~~~~~
 *
 * @see
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static uint32_t pvt_WordLenght (uart_config_s *pUART)
{
	uint32_t ret = 0x00000000;

	if (pUART->eWordLenght == UART_WORD_7B)
	{
		ret |= UART_LCR_WLEN7;
	}
	else //if (pUART->bWordLenght == UART_WORD_8B)
	{
		pUART->eWordLenght = UART_WORD_8B;
		ret |= UART_LCR_WLEN8;
	}

	return ret;
}

/******************************************************************************
 * Function : pvt_Parity(uart_config_s *pUART)
 *//**
 * \b Description:
 *
 * Returns the UART LCR register mask for the correct UART parity configuration,
 * corresponding the uart_config_s parity field.
 *
 * PRE-CONDITION: valid uart_config_s handle
 *
 * POST-CONDITION: none
 *
 * @return Returns a uint32_t mask to set the UART LCR register configuring the
 *         parity
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uart_config_s uart_init;
 * uint32_t lcr_mask = 0x00000000;
 *
 * // Generic configuration
 * uart_init.eChannel     = UART1;         // enumerator that list UART devices
 * uart_init.bParity      = PARITY_EVEN;    // generic definition of 8-bit word lenght
 *
 * // Config the UART LCR register to set parity
 * lcr_mask |= pvtParity(uart_init);
 * LPC_UART1->LCR |= lcr_mask;
 *
 ~~~~~~~~~~~~~~~
 *
 * @see
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static uint32_t pvt_Parity (uart_config_s *pUART)
{
	uint32_t ret = 0x00000000;

	if (pUART->eParity == UART_PARITY_EVEN)
	{
		ret |= (UART_LCR_PARITY_EVEN | UART_LCR_PARITY_EN);
	}
	else if (pUART->eParity == UART_PARITY_ODD)
	{
		ret |= UART_LCR_PARITY_EN;
		ret |= UART_LCR_PARITY_ODD;
	}
	else //if (pUART->bParity == PARITY_NONE)
	{
		pUART->eParity = UART_PARITY_NONE;
		ret |= UART_LCR_PARITY_DIS;
	}

	return ret;
}

/******************************************************************************
 * Function : pvt_ISR_ReadRxBuffer(uart_config_s *pUART)
 *//**
 * \b Description:
 * This routine is called by the ISR just to read data from the UART receive buffer. It will
 * transfer from the receive buffer to the uart_config_s handle buffer until there is no data
 * to be read.
 *
 * PRE-CONDITION:
 * The ISR pvt_ISR_ReadRxBuffer must be in execution.
 *
 * POST-CONDITION:
 *
 * @return
 *
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *
 ~~~~~~~~~~~~~~~
 *
 * @see
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/

static int32_t pvt_ISR_ReadRxBuffer (uart_config_s *pUART)
{
	int32_t iCont = 0;

	while (Chip_UART_ReadLineStatus(UART_MAP_REGISTER(pUART->eChannel)) & UART_LSR_RDR)
	{
		/* Read RBR n-times to get the data */
		*(pUART->bInBuffer++) = (uint8_t)(UART_MAP_REGISTER(pUART->eChannel)->RBR & UART_RBR_MASKBIT);
		iCont++;

		/* Check buffer overflow */
		if (pUART->bInBuffer == NULL)
		{
			pUART->eStatus |= UART_BUF_OVERFLOW;
			break;
		}
	}
	return iCont;
}

/******************************************************************************
 * Function : pvt_ISR_eRxInterrupt(uart_channel_e eListIndex)
 *//**
 * \b Description:
 * This function is an interrupt service routine for the UART Receive Interrupt.
 * It checks if the interrupt source was an error or new data at receive buffer.
 * When the interrupt is triggered by new data arriving, it checks if the "receive lentgh"
 * parameter of the uart_config_s struct is zero, so it calls the callback function. When
 * is greater than zero, there is more data to be received before the callback execution, so
 * it returns.
 *
 * PRE-CONDITION:
 * The uart_config_s handle must be valid and initialized, and the UARTn must be enabled.
 *
 * POST-CONDITION:
 *
 * @return
 *
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *
 ~~~~~~~~~~~~~~~
 *
 * @see
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static void pvt_ISR_eRxInterrupt (uart_channel_e eListIndex)
{
	uint32_t wTemp = 0;
	int32_t iRxBytes;

	uart_config_s *pUART;

	/* Get the UART handler */
	if (psUARTList[eListIndex] == NULL)
	{
		return;
	}
	else
	{
		pUART = psUARTList[eListIndex];
	}

	/* Identifies the interrupt */
	wTemp = (UART_MAP_REGISTER(pUART->eChannel)->IIR & UART_IIR_INTID_MASK);

	/* Check if is an Receive Line Status Interrupt */
	if (wTemp == (uint32_t) UART_IIR_INTID_RLS)
	{
		wTemp = UART_MAP_REGISTER(pUART->eChannel)->LSR;
		pUART->eStatus |= UART_ERROR;
		/* Callback called with -1 as argument TODO: return zero ?*/
		pUART->fpCallBack(pUART->bInBuffer, -1);
		return;
	}

	/* Check if is an Receive data available or CTI interrupt */
	if ((wTemp == (uint32_t) UART_IIR_INTID_RDA) || (wTemp == (uint32_t) UART_IIR_INTID_CTI))
	{
		/* Read RBR (14 times or until no char received)*/
		iRxBytes = pvt_ISR_ReadRxBuffer(pUART);

		if (pUART->eStatus == UART_FIXED_RX_IT)
		{
			pUART->iRxTferSize -= iRxBytes;
		}
	}

	/* Checks the end of transfer */
	if (pUART->iRxTferSize <= 0)
	{

		/* Fixed RX IT mode issues */
		if (pUART->eStatus == UART_FIXED_RX_IT)
		{
			/* Disable interrupt if is RX IT mode */
			pUART->eStatus = UART_READY;
			UART_eDisableIRQ(pUART);

			/* if more bytes than expected arrived:
			 * the iRxTferSize will be negative and the
			 * bInBuffer will point to the end of the buffer
			 * which can be out of space
			 * */
		}

		/* Adjust the buffer pointer to the begining of stream */
		pUART->bInBuffer -= iRxBytes;
		/* Call the user callback */
		pUART->fpCallBack(pUART->bInBuffer, iRxBytes);
	}

	return;
}

static void pvt_ISR_eRBRxInterrupt (uart_channel_e eListIndex)
{
	uint16_t wRecvBytes = 0;
	uart_config_s *pUART;

	/* Get the UART handler */
	if (psUARTList[eListIndex] == NULL)
	{
		return;
	}
	else
	{
		pUART = psUARTList[eListIndex];
	}

	Chip_UART_RXIntHandlerRB(UART_MAP_REGISTER(pUART->eChannel), &rxring);

	wRecvBytes = RingBuffer_GetCount(&rxring);

	Chip_UART_ReadRB(UART_MAP_REGISTER(pUART->eChannel), &rxring, &pUART->bInBuffer[0], wRecvBytes);

	pUART->fpCallBack(pUART->bInBuffer, wRecvBytes);

	return;
}

eMCUError_s UART_eInit (uart_config_s *pUART)
{

	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return MCU_ERROR_UART_INVALID_CHANNEL;
	}

	/* Checks if UART is in use */
	if (psUARTList[pUART->eChannel] != NULL)
	{
		return MCU_ERROR_UART_ALREADY_IN_USE;
	}

	/* Initial status is disabled */
	pUART->eStatus = UART_RESET;

	/* PinMux settings */
	BRD_UARTConfig(sUARTIndex[pUART->eChannel]); // Will send sUARTMap[] index as arg

	pUART->eStatus = UART_READY;

	/* Config word lenght, parity, stop bits and baudrate */
	UART_vReconfSettings(pUART);

	/* Enable transmission */
	Chip_UART_TXEnable(UART_MAP_REGISTER(pUART->eChannel));

	/* Add UART to a list of handlers to be used by ISR */
	//psUARTList[pUART->eChannel] = (void*) pUART;
	UART_vAddPeripheralList(pUART, pUART->eChannel);

	return MCU_ERROR_SUCCESS;
}

void UART_vDeInit (uart_config_s *pUART)
{
	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return;
	}

	/* Clears the list element */
	UART_vRemovePeripheralList(pUART->eChannel);

	/* Disable interrupt */
	UART_eDisableIRQ(pUART);

	/* Disable the chip peripheral */
	Chip_UART_DeInit(UART_MAP_REGISTER(pUART->eChannel));
}

void UART_vReconfSettings (uart_config_s *pUART)
{
	uint32_t dConfig = 0;
	uint32_t dBaud = 0;

	/* Set stopbits configurations */
	dConfig |= pvt_StopBits(pUART);

	/* Set word length configuration */
	dConfig |= pvt_WordLenght(pUART);

	/* Set parity */
	dConfig |= pvt_Parity(pUART);

	/* Enable configurations */
	Chip_UART_ConfigData(UART_MAP_REGISTER(pUART->eChannel), dConfig);

	/* Set baudrate (default value is 9600) */
	if (pUART->eBaudrate == UART_BINVALID)
	{
		pUART->eBaudrate = 9600;
	}
	dBaud = Chip_UART_SetBaudFDR(UART_MAP_REGISTER(pUART->eChannel), (uint32_t)pUART->eBaudrate);
	//Chip_UART_SetBaud(UART_MAP_REGISTER(pUART->eChannel), pUART->eBaudrate);

	pUART->eBaudrate = dBaud;

}

uint32_t UART_eSendData (uart_config_s *pUART, uint8_t *vData, int32_t iLenght)
{
	int32_t iSentBytes = 0;

	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return 0;
	}

	pUART->eStatus = UART_BUSY_TX;

	/* Transmit an array of bytes, blocking mode */
	iSentBytes = Chip_UART_SendBlocking(UART_MAP_REGISTER(pUART->eChannel),
		(uint8_t*)vData, iLenght);

	if (iSentBytes == iLenght)
	{
		pUART->eStatus = UART_READY;
	}
	else
	{
		pUART->eStatus = (UART_READY | UART_ERROR);
	}

	return iSentBytes;
}

uint32_t UART_eRecvData (uart_config_s *pUART, uint8_t *vData, int32_t wLenght)
{
	int32_t wRecvBytes = 0;
	int32_t wRemaining = 0;

	/* Parameters validation */
	if ((pUART == NULL) || (vData == NULL) || (wLenght < 1))
	{
		return 0;
	}

	pUART->eStatus = UART_BUSY_RX;

	wRemaining = wLenght;

	/* Loop until receive N=iLenght bytes*/
	while (wRecvBytes < wLenght)
	{
		int32_t wPass = 0;
		wPass = Chip_UART_Read(UART_MAP_REGISTER(pUART->eChannel), vData, wRemaining);

		wRemaining -= wPass;
		wRecvBytes += wPass;
		vData += wPass;
	}

	if (wRecvBytes == wLenght)
	{
		pUART->eStatus = UART_READY;
	}
	else
	{
		pUART->eStatus = (UART_READY | UART_TIMEOUT);
	}

	return wRecvBytes;
}

eMCUError_s UART_eRecvDataConfig (uart_config_s *pUART, int32_t iIRQPrio, int32_t iArg)
{
	/* Parameters validation */
	if ((pUART == NULL) || (iIRQPrio < 0))
	{
		return MCU_ERROR_UART_CONFIG_ERROR;
	}

	/* Disable IER */
	UART_MAP_REGISTER(pUART->eChannel)->IER = 0;

	/* UART_FCR_TRG_LEV3 (3 << 6) UART FIFO trigger level 3: 14 character */
	Chip_UART_SetupFIFOS(UART_MAP_REGISTER(pUART->eChannel),
		(UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV3));

	/* Sets the IER register to interrupt when:
	 *  bit 0 - RBR Interrupt Enable
	 */
	UART_MAP_REGISTER(pUART->eChannel)->IER = (UART_IER_RBRINT | UART_IER_RLSINT);

	/* Config transfer size */
	pUART->iRxTferSize = iArg;

	/* When iLenght = 0 interrupt is in permanent mode */
	if (iArg == UART_RX_PERMANENT)
	{
		pUART->eStatus = UART_PERMANENT_RX_IT;
	}
	else
	{
		pUART->eStatus = UART_FIXED_RX_IT;
	}

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART_MAP_IRQn(pUART->eChannel), iIRQPrio);

	return MCU_ERROR_SUCCESS;
}

eMCUError_s UART_eRecvData_IT (uart_config_s *pUART, int32_t iIRQPrio, int32_t iArg)
{
	/* Parameters validation */
	if ((pUART == NULL) || (iIRQPrio < 0))
	{
		return MCU_ERROR_UART_CONFIG_ERROR;
	}

	/* Disable IER */
	UART_MAP_REGISTER(pUART->eChannel)->IER = 0;

	/* UART_FCR_TRG_LEV3 (3 << 6) UART FIFO trigger level 3: 14 character */
	Chip_UART_SetupFIFOS(UART_MAP_REGISTER(pUART->eChannel),
		(UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV3));

	/* Before using the ring buffers, initialize them using the ring
	 buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_RB_SIZE);

	/* Sets the IER register to interrupt when:
	 *  bit 0 - RBR Interrupt Enable
	 *  bit 2 - RX Line Interrupt Enable
	 */
	UART_MAP_REGISTER(pUART->eChannel)->IER = (UART_IER_RBRINT | UART_IER_RLSINT);

	/* Config transfer size */
	pUART->iRxTferSize = iArg;

	/* When iLenght = 0 interrupt is in permanent mode */
	if (iArg == UART_RX_PERMANENT)
	{
		pUART->eStatus = UART_PERMANENT_RX_IT;
	}
	else
	{
		pUART->eStatus = UART_FIXED_RX_IT;
	}

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART_MAP_IRQn(pUART->eChannel), iIRQPrio);

	/* Enable NVIC interrupt */
	UART_eEnableIRQ(pUART);

	return MCU_ERROR_SUCCESS;
}

eMCUError_s UART_ePutChar (uart_config_s *pUART, char *vData)
{
	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return MCU_ERROR_UART_INVALID_CHANNEL;
	}

	pUART->eStatus = UART_BUSY_TX;

	/* Check FIFO status */
	if (Chip_UART_ReadLineStatus(UART_MAP_REGISTER(pUART->eChannel)) & UART_LSR_THRE)
	{
		/* Transmitter buffer is empty */
		Chip_UART_SendByte(UART_MAP_REGISTER(pUART->eChannel), (uint8_t)*vData);
		pUART->eStatus = UART_READY;
		return MCU_ERROR_SUCCESS;
	}
	else
	{
		/* Transmitter buffer is full, cancel transmission */
		pUART->eStatus = (UART_ERROR | UART_READY);
		return MCU_ERROR_UART_TRANS_FULL;
	}
}

eMCUError_s UART_eGetChar (uart_config_s *pUART, char *vData)
{
	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return MCU_ERROR_UART_INVALID_CHANNEL;
	}

	pUART->eStatus = UART_BUSY_RX;

	/* Check if a new byte has arrived */
	if (Chip_UART_ReadLineStatus(UART_MAP_REGISTER(pUART->eChannel)) & UART_LSR_RDR)
	{
		*vData = (char)Chip_UART_ReadByte(UART_MAP_REGISTER(pUART->eChannel));
		pUART->eStatus = UART_READY;
		return MCU_ERROR_SUCCESS;
	}
	else
	{
		pUART->eStatus = (UART_ERROR | UART_READY);
		return MCU_ERROR_UART_RECV_EMPTY;
	}
}

eMCUError_s UART_eEnableIRQ (uart_config_s *pUART)
{
	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return MCU_ERROR_UART_INVALID_CHANNEL;
	}

	/* Enable Interrupt for UART0 channel */
	NVIC_EnableIRQ(UART_MAP_IRQn(pUART->eChannel));
	return MCU_ERROR_SUCCESS;
}

eMCUError_s UART_eDisableIRQ (uart_config_s *pUART)
{
	/* Validation of parameters */
	if ((pUART == NULL) || (pUART->eChannel >= UART_INVALID))
	{
		return MCU_ERROR_UART_INVALID_CHANNEL;
	}
	/* Disable Interrupt for UART channel */
	NVIC_DisableIRQ(UART_MAP_IRQn(pUART->eChannel));
	return MCU_ERROR_SUCCESS;
}

/* Set every list element as NULL */
void UART_vInitPeripheralList (void)
{
	uint8_t i;
	for (i = 0; i < UART_INVALID; i++)
	{
		psUARTList[i] = NULL;
	}
}

void UART_vAddPeripheralList (uart_config_s *sElement, uint8_t bPos)
{
	if (bPos < UART_INVALID)
		psUARTList[bPos] = (void*)sElement;
}

void UART_vRemovePeripheralList (uint8_t bPos)
{
	if (bPos < UART_INVALID)
		psUARTList[bPos] = NULL;
}

void UART0_IRQHandler (void)
{
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	pvt_ISR_eRxInterrupt(UART0);
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordExitISR();
#endif
}

void UART1_IRQHandler (void)
{
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	pvt_ISR_eRxInterrupt(UART1);
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordExitISR();
#endif
}

void UART2_IRQHandler (void)
{
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	pvt_ISR_eRBRxInterrupt(UART2);
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordExitISR();
#endif
}

void UART3_IRQHandler (void)
{
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	pvt_ISR_eRxInterrupt(UART3);
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/*************** END OF FUNCTIONS ***************************************************************************/
