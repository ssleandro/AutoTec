/****************************************************************************
 * Title                 :   M2GGPSCOMM
 * Filename              :   M2GGPSCOMM.c
 * Author                :   Henrique Reis
 * Origin Date           :
 * Version               :   1.0.0
 * Compiler              :   GCC 5.4 2016q2 / ICCARM 7.40.3.8938
 * Target                :   LPC43XX M4
 * Notes                 :   Qualicode Machine Technologies
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
 *    Date    Version       Author          Description
 *  XX/XX/XX   1.0.0    Henrique Reis    M2GGPSCOMM.c created.
 *
 *****************************************************************************/
/** @file   M2GGPSCOMM.c
 *  @brief
 *
 */
/* Includes */
#include <string.h>
#include <M2GGPSCOMM_config.h>
#include <M2GGPSCOMM.h>
#include "ring_buffer.h"

#ifndef UNITY_TEST
#include "mculib.h"
#include <auteq_os.h>
#else
#include "unity_fixture.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/

/* Module preprocessor macros */
#ifndef NULL
#define NULL (void*)0
#endif

#ifndef UNITY_TEST
#define DEFAULT_TIMEOUT 500
#else
#define DEFAULT_TIMEOUT 2
#endif

/*
 * M2GGPSCOMM device status variable
 * The status is a word with each bit means a status/option
 */
#define M2GGPSCOMM_STATUS_ENABLED   0x00000001 /*!< Status flag - bit 0 - Device instance enabled */
#define M2GGPSCOMM_STATUS_DISABLED  0x00000000 /*!< Status flag - bit 0 - Device instance disabled */
#define M2GGPSCOMM_STATUS_BUSY      0x00000002 /*!< Status flag - bit 1 - Device instance busy */
#define M2GGPSCOMM_STATUS_WR_ERROR  0x00000004 /*!< Status flag - bit 2 - Device instance write op error */
#define M2GGPSCOMM_STATUS_RD_ERROR  0x00000008 /*!< Status flag - bit 3 - Device instance read op error */

/*****************************************
 * Module typedefs
 *****************************************/
typedef eDEVError_s (*fpIOCTLFunction) (uint32_t wRequest, void * vpValue); /*!< IOCTL function pointer */

/*****************************************
 * Module variable definitions
 *****************************************/
/* Ring buffer structures */
static uint8_t bGPSRecvBuffer[M2GGPSCOMM_BUF_SIZE];         //!< Ring buffer array
static RINGBUFF_T rbM2GGPSHandle;                           //!< Ring buffer control handle

static uint8_t abGPSUARTBuffer[M2GGPSCOMM_BUF_SIZE];        //!< UART preliminar buffer

/*!< Device health status */
static uint8_t bM2GGPSCOMMDevStatus;

/* Private handler used to access UART peripheral under MCULIB layer */
static uart_config_s sMCU_UARTGPS_Handle =
	{
		.eChannel = (uart_channel_e) M2GGPSCOMM_UART_CHANNEL,
		.eBaudrate = M2GGPSCOMM_UART_BAUD,
		.eWordLenght = UART_WORD_8B,
		.eParity = UART_PARITY_NONE,
		.eStopbits = UART_STOPBITS1,
		.eStatus = UART_RESET,
		.fpCallBack = NULL,
		.bInBuffer = NULL,
	};

/*****************************************
 * Function prototypes
 *****************************************/
static void M2GGPS_vRBSafeInsert (uint8_t *pbData, uint32_t wNumItens);
static uint32_t M2GGPS_wReadBufferProcedure (uint8_t *pbOutput);
static void M2GGPS_UARTCallback (uint8_t * bBuffer, uint32_t wLen);
static eDEVError_s M2GGPS_eChangeBaudRate (uint32_t wRequest, void * vpValue);
static eDEVError_s M2GGPS_eChangeIntStatus (uint32_t wRequest, void * vpValue);

//!< IOCTL array of function mapping
#define X(a, b) b,
fpIOCTLFunction M2GGPS_pIOCTLFunction[] =
	{
	IOCTL_M2GGPSCOMM
	};
#undef X

/*****************************************
 * Function definitions
 *****************************************/
void M2GGPS_vRBSafeInsert (uint8_t *pbData, uint32_t wNumItens)
{
	/* Fill the ring buffer with bytes */
	if (!RingBuffer_InsertMult(&rbM2GGPSHandle, pbData, wNumItens))  // Ring buffer full
	{
		// Pop from tail and the insert into head
		uint8_t abGarbage[M2GGPSCOMM_BUF_SIZE];
		RingBuffer_PopMult(&rbM2GGPSHandle, abGarbage, wNumItens);
		RingBuffer_InsertMult(&rbM2GGPSHandle, pbData, wNumItens);
	}
}

uint32_t M2GGPS_wReadBufferProcedure (uint8_t *pbOutput)
{
	/* Wait until ISR callbacks are completed */
	if (bM2GGPSCOMMDevStatus == M2GGPSCOMM_STATUS_BUSY)
	{
		return 0;
	}
	uint32_t wReturn = 0;

	/* Loop until the end of ring buffer */
	while (!RingBuffer_IsEmpty(&rbM2GGPSHandle))
	{
		RingBuffer_Pop(&rbM2GGPSHandle, &pbOutput[wReturn]);
		wReturn++;
	}
	return wReturn;
}

eDEVError_s M2GGPS_eChangeBaudRate (uint32_t wRequest, void * vpValue)
{
	uart_baudrate_e eBaudRate;

	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	eBaudRate = *(uint32_t*)vpValue;

	switch (eBaudRate)
	{
		case UART_B9600:
		sMCU_UARTGPS_Handle.eBaudrate = UART_B9600;
			break;
		case UART_B115200:
		sMCU_UARTGPS_Handle.eBaudrate = UART_B115200;
			break;
		case UART_B230400:
		sMCU_UARTGPS_Handle.eBaudrate = UART_B230400;
			break;
		case UART_B460800:
		sMCU_UARTGPS_Handle.eBaudrate = UART_B460800;
			break;
		default:
		return DEV_ERROR_M2GGPSCOMM_SETTING_ERROR;
			break;
	}
	UART_vReconfSettings(&sMCU_UARTGPS_Handle);

	return DEV_ERROR_SUCCESS;
}

void M2GGPS_UARTCallback (uint8_t *bBuffer, uint32_t wLen)
{
	/* State: transferring to buffer  */
	bM2GGPSCOMMDevStatus = M2GGPSCOMM_STATUS_BUSY;

	/* Anything received goes to ring buffer */
	M2GGPS_vRBSafeInsert(bBuffer, wLen);

	/* State: transfer completed */
	bM2GGPSCOMMDevStatus = M2GGPSCOMM_STATUS_ENABLED;
}

eDEVError_s M2GGPS_eChangeIntStatus (uint32_t wRequest, void * vpValue)
{
	bool bInterruptEnable = false;

	(void)wRequest;

	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	bInterruptEnable = *(bool*)vpValue;

	if (bInterruptEnable)
	{
		if (sMCU_UARTGPS_Handle.bInBuffer == NULL || sMCU_UARTGPS_Handle.fpCallBack == NULL)
		{
			return DEV_ERROR_INVALID_IOCTL;
		}
		/* Start interruption on reception */
		UART_eEnableIRQ(&sMCU_UARTGPS_Handle);

	}
	else
	{
		UART_eDisableIRQ(&sMCU_UARTGPS_Handle);
	}

	return DEV_ERROR_SUCCESS;
}

eDEVError_s M2GGPS_open (void)
{
	eDEVError_s eInitErr = DEV_ERROR_SUCCESS;

	/* Init the receive buffer */
	RingBuffer_Init(&rbM2GGPSHandle, bGPSRecvBuffer, sizeof(uint8_t), M2GGPSCOMM_BUF_SIZE);
	RingBuffer_Flush(&rbM2GGPSHandle);

	/* Call the UART initializer */
	eInitErr = (eDEVError_s)UART_eInit(&sMCU_UARTGPS_Handle);
	if (eInitErr != DEV_ERROR_SUCCESS)
	{
		/* Return if error */
		return eInitErr;
	}
	else
	{
		sMCU_UARTGPS_Handle.bInBuffer = abGPSUARTBuffer;
		sMCU_UARTGPS_Handle.fpCallBack = M2GGPS_UARTCallback;

		/* Start interruption on reception */
		UART_eRecvData_IT(&sMCU_UARTGPS_Handle, 5, UART_RX_PERMANENT);
//        UART_eRecvDataConfig(&sMCU_UARTGPS_Handle, 5, UART_RX_PERMANENT);
	}

	bM2GGPSCOMMDevStatus = M2GGPSCOMM_STATUS_ENABLED;

	return eInitErr;
}

uint32_t M2GGPS_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	uint8_t *pbAuxPointer = (uint8_t*)vpBuffer;

	/* The received data is on the ring buffer */
	return M2GGPS_wReadBufferProcedure(pbAuxPointer);
}

uint32_t M2GGPS_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	/* Check if data is corrupted */
	if ((vpBuffer == NULL) || (tBufferSize == 0))  //Wrong inputs
	{
		return 0;
	}

	return UART_eSendData(&sMCU_UARTGPS_Handle, (uint8_t*)vpBuffer, tBufferSize);
}

eDEVError_s M2GGPS_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if (wRequest >= IOCTL_M2GGPSCOMM_INVALID)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	/* Call the request related function */
	return M2GGPS_pIOCTLFunction[wRequest](wRequest, vpValue);
}

eDEVError_s M2GGPS_close (struct peripheral_descriptor_s* const this)
{
	(void)this;
	/* Close UART on MCULIB */
	UART_vDeInit(&sMCU_UARTGPS_Handle);

	return DEV_ERROR_SUCCESS;
}
