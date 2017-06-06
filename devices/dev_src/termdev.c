/****************************************************************************
 * Title                 :	Terminal Device API
 * Filename              :	termdev.c
 * Author                :	Joao Paulo Martins
 * Origin Date           :	07/03/2016
 * Version               :	1.0.3
 * Compiler              : GCC 5.2 2015q4
 * Target                : LPC43XX M4
 * Notes                 : None
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
 ******************************************************************************/
/*************** INTERFACE CHANGE LIST **************************************
 *
 *     Date      Version      Author         		Description
 *  07/03/2016	 1.0.0	   Joao Paulo Martins	  First version of file
 *  16/03/2016  1.0.1		 Joao Paulo Martins		Adaptation to devicelib
 *  31/03/2016	 1.0.2		 Joao Paulo Martins   Unit tests and new read functions
 *  05/04/2016	 1.0.3     Joao Paulo Martins   New IOCTL functions and modifications
 *
 *****************************************************************************/
/** @file termdev.c
 *  @brief This device implements the feature of sending/receiving debug
 *  and command messages.
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <string.h>
#include <TERMDEV.h>
#include <TERMDEV_config.h>
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
#ifndef NULL
#define NULL (void*)0
#endif

#define RECV_BUFSIZE 				256 /*!< Size (bytes) of the receive buffer */
#ifndef UNITY_TEST
#define DEFAULT_TIMEOUT		500 /*!< 500 ms timeout */
#else
#define DEFAULT_TIMEOUT		2 /*!< 2 secs timeout */
#endif

#ifdef UNITY_TEST
#define USB_wWrite 						USB_wWrite2
#define UART_eSendData 				UART_eSendData2
#define USB_wBufferRead 			USB_wBufferRead2
#define CAN_vSendMessage 			CAN_vSendMessage2
#define USB_eInit 						USB_eInit2
#define CAN_eInit 						CAN_eInit2
#define UART_eInit				 		UART_eInit2
#define USB_eDeInit 					USB_eDeInit2
#define CAN_vDeInit 					CAN_vDeInit2
#define UART_vDeInit 					UART_vDeInit2
#define CAN_vAddMessageID 		CAN_vAddMessageID2
#define CAN_vRemoveMessageID 	CAN_vRemoveMessageID2
#define UART_eDisableIRQ 			UART_eDisableIRQ2
#define UART_eRecvData_IT 		UART_eRecvData_IT2
#define UART_eRecvData				UART_eRecvData2

/**
 * CAN message Status
 */
typedef enum eCANStatus_s
{
	CAN_STAT_TXOK = (1 << 3),   //!< CAN Transmited message successfully
	CAN_STAT_RXOK = (1 << 4),//!< CAN Received a message successfully
	CAN_STAT_EPASS = (1 << 5),//!< CAN controller is in the error passive state
	CAN_STAT_EWARN = (1 << 6),//!< CAN At least one of the error counters in the EC has reached the error warning limit of 96
	CAN_STAT_BOFF = (1 << 7)//!< CAN controller is in busoff state
}eCANStatus_s;

/**
 * CAN message object structure
 */
typedef struct canMSGStruct_s
{
	uint32_t id; /**< ID of message, if bit 30 is set then this is extended frame */
	uint32_t dlc; /**< Message data length */
	uint8_t data[8];/**< Message data */
}canMSGStruct_s;

typedef void (* canCallBack)(eCANStatus_s eErrorCode, canMSGStruct_s CANMessage);  //!< CAN Callback typedef

/**
 * This Struct holds CAN configuration for given board
 */
typedef struct can_config_s
{
	canChannel_s eCANPort;        //!< CAN Port
	canBitrate_s eCANBitrate;//!< CAN BitRate
	canCallBack fpCallback;//!< CAN Callback Function
	void * vpPrivateData;//!< CAN Private data
}can_config_s;

typedef void (* usbMessageCallBack)(void);  //!< USB Callback typedef

/**
 * This enumeration is a list of possible USB classes on the Board
 */
typedef enum eUSBClasses_s
{
	USB_CLASS_CDC,      //!< USB CDC Class
	USB_CLASS_HID,//!< USB HID Class
	USB_CLASS_DFU,//!< USB DFU Class
	USB_CLASS_MSD,//!< USB MSD Class
	USB_CLASS_INVALID//!< USB Invalid Class
}eUSBClasses_s;

/**
 * This Struct holds USB configuration for given board
 */
typedef struct usb_config_s
{
	eUSBInterfaces_s eUSB;      //!< USB port to use
	eUSBClasses_s eClass;//!< USB Class to use (Currently fixed at CDC)
	usbMessageCallBack fpUSBCallBack;//!< USB Callback
	void * vpPrivateData;//!< Pointer to private data
}usb_config_s;

#define UART_RX_PERMANENT 0

/**
 * @brief  Callback definition for UART receive interrupts.
 * @param  bBuffer: pointer to the top of a buffer thst store the
 *                  received data;
 * @param  bLen: Amount of data received at the last UART RX interruption;
 */
typedef void (* uartCallback)(uint8_t *bBuffer, uint32_t bLen);

/**
 * @brief UART config parameter: word lenght (8 or 7 bits)
 */
typedef enum uart_wordlen_e
{
	UART_WORD_8B = 0,
	UART_WORD_7B,
}uart_wordlen_e;

/**
 * @brief UART config parameter: stop bits (1 or 2 bits)
 */
typedef enum uart_stopbits_e
{
	UART_STOPBITS1 = 0,
	UART_STOPBITS2,
}uart_stopbits_e;

/**
 * @brief UART config parameter: parity (none, even or odd)
 */
typedef enum uart_parity_e
{
	UART_PARITY_NONE = 0,
	UART_PARITY_ODD,
	UART_PARITY_EVEN,
}uart_parity_e;

/**
 * @brief UART State definition
 */
typedef enum uart_state_e
{
	UART_RESET = 0x00, /*!< Peripheral is not yet Initialized                  */
	UART_READY = 0x01, /*!< Peripheral Initialized and ready for use           */
	UART_PERMANENT_RX_IT = 0x02, /*!< Interrupt on any receive data arrival              */
	UART_FIXED_RX_IT = 0x42, /*!< Receive N bytes by interruption                    */
	UART_BUSY_TX = 0x12, /*!< Data Transmission process is ongoing               */
	UART_BUSY_RX = 0x22, /*!< Data Reception process is ongoing                  */
	UART_BUSY_TX_RX = 0x32, /*!< Data Transmission and Reception process is ongoing */
	UART_TIMEOUT = 0x03, /*!< Timeout state                                      */
	UART_ERROR = 0x04, /*!< Error on any basic operation                       */
	UART_BUF_OVERFLOW = 0x05, /*!< RX Buffer overflow                                 */
}uart_state_e;

/*
 *   @brief  UART peripheral basic info needed to init
 */
typedef struct uart_config_s
{
	uart_channel_e eChannel;             //!< UART instance (UART1, UART2, etc..)
	uart_baudrate_e eBaudrate;//!< Baudrate as defined number
	uart_wordlen_e eWordLenght;//!< Lenght of the data field
	uart_stopbits_e eStopbits;//!< Stop bits
	uart_parity_e eParity;//!< Parity
	uart_state_e eStatus;//!< Status of the peripheral
	uartCallback fpCallBack;//!< Callback to handle rx interrupt
	uint8_t *bInBuffer;//!< Pointer to incoming data buffer
	int iRxTferSize;//!< Size of the reception transfer
	void *vpPrivate;//!< Private data
}uart_config_s;

#define MCU_ERROR_INVALID_CALLBACK	1

#endif /* UNITY_TEST */

/*
 * @brief TERMDEV device status variable
 * The status is a word with each bit means a status/option
 * */
#define TERMDEV_STATUS_ENABLED	0x00000001 /*!< Status flag - bit 0 - Device instance enabled */
#define TERMDEV_STATUS_DISABLED	0x00000000 /*!< Status flag - bit 0 - Device instance enabled */
#define TERMDEV_STATUS_BUSY			0x00000002 /*!< Status flag - bit 1 - Device instance busy */
#define TERMDEV_STATUS_WR_ERROR	0x00000004 /*!< Status flag - bit 2 - Device instance write op error */
#define TERMDEV_STATUS_RD_ERROR 0x00000008 /*!< Status flag - bit 3 - Device instance read op error */

#define TERMDEV_STATUS_USE_MASK	0x00000007 /*!< Status flag - perihperals usage mask */
#define TERMDEV_STATUS_USE_UART	0x00000001 /*!< Status flag - UART reserved for use */
#define TERMDEV_STATUS_USE_USB	0x00000002 /*!< Status flag - USB reserved for use */
#define TERMDEV_STATUS_USE_CAN	0x00000004 /*!< Status flag - CAN reserved for use */

#define _IS_INTERFACES_NOT_VALID(__HANDLE__) \
	(!((__HANDLE__).dReservedInterfaces & TERMDEV_STATUS_USE_MASK))/*!< Macro to check if reserved interfaces are valid */

#define _IS_TERMDEV_USING_UART(__HANDLE__) \
	((__HANDLE__).dReservedInterfaces & TERMDEV_STATUS_USE_UART)/*!< Macro to check if UART is enabled */

#define _IS_TERMDEV_USING_USB(__HANDLE__) \
	((__HANDLE__).dReservedInterfaces & TERMDEV_STATUS_USE_USB)/*!< Macro to check if USB is enabled */

#define _IS_TERMDEV_USING_CAN(__HANDLE__) \
	((__HANDLE__).dReservedInterfaces & TERMDEV_STATUS_USE_CAN)/*!< Macro to check if CAN is enabled */

#define _IS_TERMDEV_VALID_IFACE(__IFACE__) \
	((__IFACE__ == TERMDEV_UART)||(__IFACE__ == TERMDEV_USB)||(__IFACE__ == TERMDEV_CAN))	/*!< Macro to check if any interface is activated */

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
typedef eDEVError_s (*fpIOCTLFunction) (uint32_t wRequest, void * vpValue); /*!< IOCTL function pointer */

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

/* Ringbuffer structures */
static uint8_t bRecvBuffer[RECV_BUFSIZE];  //!< RingBuffer array
static RINGBUFF_T rbTDVHandle;             //!< RingBuffer Control handle

static uint8_t abUARTBuffer[RECV_BUFSIZE];  //!< UART preliminar buffer

/*
 * @brief Private handler used to access UART peripheral under MCULIB layer
 * */
static uart_config_s sMCU_UART_Handle =
	{
		.eChannel = (uart_channel_e) TERMDEV_UART_CHANNEL,
		.eBaudrate = TERMDEV_UART_BAUD,
		.eWordLenght = UART_WORD_8B,
		.eParity = UART_PARITY_NONE,
		.eStopbits = UART_STOPBITS1,
		.eStatus = UART_RESET,
		.fpCallBack = NULL,
		.bInBuffer = NULL,
	};

/*
 * @brief Private handler used to access USB peripheral under MCULIB layer
 * */
static usb_config_s sMCU_USB_Handle =
	{
		.eUSB = (eUSBInterfaces_s) TERMDEV_USB_CHANNEL,
		.eClass = USB_CLASS_CDC,
		.fpUSBCallBack = NULL,
	};

/*
 * @brief Private handler used to access CAN peripheral under MCULIB layer
 * */
static can_config_s sMCU_CAN_Handle =
	{
		.eCANPort = TERMDEV_CAN_CHANNEL,
		.eCANBitrate = TERMDEV_CAN_BITRATE,
		.fpCallback = NULL,
	};

/*
 * @brief Private handler used to manage the TERMDEV instance
 * */
static termdev_handle_s TERMDEV_Handle =
	{
		.dReservedInterfaces = 0,
		.eActiveInterface = TERMDEV_INVALID,
		.eMessagesLevel = MSG_LEVEL5,
		.bDeviceStatus = TERMDEV_STATUS_DISABLED,
	};

static uint32_t wCANSendID = 0; /* !< Holds the current output CAN message ID */

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void TDV_vRBSafeInsert (uint8_t *pbData, uint32_t wNumItens);
static uint32_t TDV_wReadBufferProcedure (uint8_t *pbOutput);
static eDEVError_s TDV_eSetActive (uint32_t wRequest, void * vpValue);
static eDEVError_s TDV_eDisable (uint32_t wRequest, void * vpValue);
static eDEVError_s TDV_eCANAddID (uint32_t wRequest, void * vpValue);
static void TDV_CANCallback (eCANStatus_s eErrorCode, canMSGStruct_s CANMessage);
static void TDV_USBCallback (void);
static void TDV_UARTCallback (uint8_t * bBuffer, uint32_t wLen);

#define X(a, b) b,
fpIOCTLFunction TDV_pIOCTLFunction[] = //!< IOCTL array of function mapping
	{
	IOCTL_termdev
	};
#undef X

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);
uint32_t USB_wWrite2(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{	return buf_len;}
uint32_t UART_eSendData2(uart_config_s *pUART, uint8_t *vData, int32_t iLenght)
{	return iLenght;}
uint32_t USB_wBufferRead2(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{
	*(pBuf + 0) = 'T';
	*(pBuf + 1) = 'E';
	*(pBuf + 2) = 'S';
	*(pBuf + 3) = 'T';
	*(pBuf + 4) = 'E';
	return 5;
}
void CAN_vSendMessage2(const can_config_s *pCAN, const canMSGStruct_s CANMessage)
{}
eDEVError_s USB_eInit2(usb_config_s * pUSB)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s CAN_eInit2(can_config_s *pCAN)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s UART_eInit2(uart_config_s *pUART)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s USB_eDeInit2(usb_config_s *pUSB)
{	return DEV_ERROR_SUCCESS;}
void CAN_vDeInit2(can_config_s *pCAN)
{}
void UART_vDeInit2(uart_config_s *pUART)
{}
void CAN_vAddMessageID2(const can_config_s *pCAN, const uint16_t hCANmsgID)
{}
void CAN_vRemoveMessageID2(const can_config_s *pCAN, const uint16_t hCANmsgID)
{}
eDEVError_s UART_eDisableIRQ2(uart_config_s *pUART)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s UART_eRecvData_IT2(uart_config_s *pUART, int32_t iIRQPrio, int32_t iArg)
{	return DEV_ERROR_SUCCESS;}
uint32_t UART_eRecvData2(uart_config_s *pUART, uint8_t *vData, int32_t wLenght, uint32_t wTimeout)
{	return wLenght;};

uint32_t osKernelSysTick(void)
{
	struct tm *str_time;
	time_t rawtime;

	time(&rawtime);
	str_time = localtime(&rawtime);

	return (uint32_t)((str_time->tm_hour * 3600) + (str_time->tm_min * 60) + str_time->tm_sec);
}

void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
 * Function : TDV_vRBSafeInsert(uint8_t *pbData, uint32_t wNumItens)
 *//**
 * \b Description:
 *
 * This function is just a procedure to check if the module's global ring buffer (rbTDVHandle)
 * is full, if yes, it will overwrite the oldest items in order to insert the new data.
 *
 * PRE-CONDITION: The global ring buffer (rbTDVHandle) has to be already enabled.
 *
 * POST-CONDITION: Data items saved in ring buffer.
 *
 * @return nothing
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint8_t abTest[] = ['t','e','s','t'];
 * TDV_vRBSafeInsert(abTest, 4);
 ~~~~~~~~~~~~~~~
 *
 * @see rbTDVHandle
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void TDV_vRBSafeInsert (uint8_t *pbData, uint32_t wNumItens)
{
	/* Fill the ringbuffer with bytes */
	if (!RingBuffer_InsertMult(&rbTDVHandle, pbData, wNumItens)) //RingBuffer Full
	{
		//Pop from tail and then insert into head
		uint8_t abGarbage[256];
		RingBuffer_PopMult(&rbTDVHandle, abGarbage, wNumItens);
		RingBuffer_InsertMult(&rbTDVHandle, pbData, wNumItens);
	}
}

/******************************************************************************
 * Function : TDV_wReadBufferProcedure(uint8_t *pbOutput)
 *//**
 * \b Description:
 *
 * This function is called whenever a TDV_read call occurs. It will check if the ring buffer
 * has new data and will copy this data to the destination area. By doing this, the TDV_read
 * procedure performs a non-blocking reading operation. If using CAN and the function is called
 * in the middle of a multi-packet transfer, it will return zero until all packets are received.
 *
 * PRE-CONDITION: The global ring buffer (rbTDVHandle) has to be already enabled.
 *
 * POST-CONDITION: Valid items of the ring buffer will be transfered to pbOutput.
 *
 * @return nothing
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint8_t buffer[256];
 * uint8_t bBytesReceived = TDV_wReadBufferProcedure(buffer);
 ~~~~~~~~~~~~~~~
 *
 * @see TDV_read
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static uint32_t TDV_wReadBufferProcedure (uint8_t *pbOutput)
{
	/* Wait until ISR callbacks are completed */
	if (TERMDEV_Handle.bDeviceStatus == TERMDEV_STATUS_BUSY)
	{
		return 0;
	}

	uint32_t wReturn = 0;

	/* Loop until the end of ring buffer */
	while (!RingBuffer_IsEmpty(&rbTDVHandle))
	{
		RingBuffer_Pop(&rbTDVHandle, &pbOutput[wReturn]);
		wReturn++;
	}

	return wReturn;
}

/******************************************************************************
 * Function : TDV_CANCallback(eCANStatus_s eErrorCode, canMSGStruct_s CANMessage)
 *//**
 * \b Description:
 *
 * This is a private function used as callback for CAN interrupt routines.
 * This function is called whenever a valid CAN message is received. It will copy the received
 * bytes to the input buffer (sRecvBuffer), following a basic protocol to get multi-packet CAN
 * messages.
 *
 * If the CAN message data field starts with 0xAA, there are more packets to arrive. The callback will
 * copy the 7 bytes of the message and keep the TERMDEV_Handle status as BUSY. When the last packet arrive
 * (1st byte 0xBB) the callback will copy the remaining bytes and set the TERMDEV_Status as ENABLED,
 * to notify the TDV_read function that data has arrived.
 *
 * This function must only be used as callback for CAN interrupt handler from mculib.
 *
 * PRE-CONDITION: CAN interface initiated
 *
 * POST-CONDITION: None
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * Do not use it !
 ~~~~~~~~~~~~~~~
 *
 * @see TDV_open, TDV_write, TDV_read, TDV_ioctl, TDV_close and TDV_eCANAddID
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void TDV_CANCallback (eCANStatus_s eErrorCode, canMSGStruct_s CANMessage)
{
	//uint32_t i;

	/* CAN protocol - first byte of message controls flow
	 * 0xAA -> Initial or intermediate packet
	 * 0xBB -> Last packet
	 * */

	if ((CANMessage.dlc < 1) || (CANMessage.dlc > 8))
		return;

	/* Ringbuffer implementation */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_BUSY;
	TDV_vRBSafeInsert(&CANMessage.data[1], (CANMessage.dlc - 1));

	if (CANMessage.data[0] == 0xBB)
	{
		/* Signals that the handler is not busy anymore */
		TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_ENABLED;
	}
}

/******************************************************************************
 * Function : TDV_USBCallback(void)
 *//**
 * \b Description:
 *
 * This is a private function used as callback for USB interrupt routines.
 * This function will read the USB buffer and save the result into the input buffer (sRecvBuffer),
 * setting the TERMDEV_Handler status as ENABLED, to notify the TDV_read function that data has
 * arrived.
 *
 * This function must only be used as callback for USB interrupt handler from mculib.
 *
 * PRE-CONDITION: USB interface initiated
 *
 * POST-CONDITION: None
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * Do not use it !
 ~~~~~~~~~~~~~~~
 *
 * @see @see TDV_open, TDV_write, TDV_read, TDV_ioctl, TDV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void TDV_USBCallback (void)
{
	uint8_t bUSBAuxBuffer[256];

	/* State: transfering to buffer */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_BUSY;

	/* Gets the message */
	uint32_t wLenght = (uint32_t)USB_wBufferRead(&sMCU_USB_Handle, bUSBAuxBuffer, 256);

	/* Insert into ring buffer */
	TDV_vRBSafeInsert(bUSBAuxBuffer, wLenght);

	/* State: transfer completed */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_ENABLED;
}

/******************************************************************************
 * Function : TDV_UARTCallback(void)
 *//**
 * \b Description:
 *
 * This private callback is called whenever any amout of data arrive at UART.
 * It will simply put the received data into sRecvBuffer, and notify the TERMDEV_Handler
 * using the bStatus field.
 *
 * PRE-CONDITION: UART interface initiated
 *
 * POST-CONDITION: sRecvBuffer with new data at bBufData and a new tail indicator (bValidDataLenght)
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * Do not use it !
 ~~~~~~~~~~~~~~~
 *
 * @see @see TDV_open, TDV_write, TDV_read, TDV_ioctl, TDV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void TDV_UARTCallback (uint8_t *bBuffer, uint32_t wLen)
{
	/* State: transfering to buffer */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_BUSY;

	/* Anything received goes to ring buffer */
	TDV_vRBSafeInsert(bBuffer, wLen);

	/* State: transfer completed */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_ENABLED;
}

/******************************************************************************
 * Function : TDV_eSetActive(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request.
 * This function will set the active input/output interface of the TERMDEV device, that must be UART or
 * USB or CAN. If the interface is not enabled, this routine will call the init procedure using the
 * MCULIB layer. The interface will remain enabled until the disable function is called.
 *
 * When setting an interface as active, any call on TDV_read and TDV_write will use the active interface.
 *
 * PRE-CONDITION: TERMDEV initiated and avaliable interface
 *
 * POST-CONDITION: None
 *
 * @return eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wActiveInterface = TERMDEV_IFACE_UART;
 * TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wActiveInterface);
 ~~~~~~~~~~~~~~~
 *
 * @see TDV_open, TDV_write, TDV_read, TDV_ioctl, TDV_close and TDV_eDisable
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s TDV_eSetActive (uint32_t wRequest, void * vpValue)
{
	eDEVError_s eInitErr;

	/* Validation */
	if (vpValue == NULL)
	{
		return DEV_ERROR_TERMDEV_SETTING_ERROR;
	}

	uint32_t wActiveIface = *(uint32_t*)vpValue;
	if (!(_IS_TERMDEV_VALID_IFACE(wActiveIface)))
	{
		return DEV_ERROR_INVALID_PERIPHERAL;
	}

	switch (wActiveIface)
	{
		case TERMDEV_UART:
		/* Is UART not enabled? */
		if (!(_IS_TERMDEV_USING_UART(TERMDEV_Handle)))
		{
			/* Call the UART initializer */
			eInitErr = (eDEVError_s)UART_eInit(&sMCU_UART_Handle);
			if (eInitErr != DEV_ERROR_SUCCESS)
			{
				/* Return if error */
				return eInitErr;
			}
			else
			{
				/* UART initialized - set as reserved interface */
				TERMDEV_Handle.dReservedInterfaces |= TERMDEV_UART;
				sMCU_UART_Handle.bInBuffer = abUARTBuffer;
				sMCU_UART_Handle.fpCallBack = TDV_UARTCallback;

				/* Start interruption on reception */
				UART_eRecvData_IT(&sMCU_UART_Handle, 5, UART_RX_PERMANENT);
			}
		}
		/* UART is already enabled, set as active */
		TERMDEV_Handle.eActiveInterface = (termdev_interface_e)wActiveIface;
			break;

		case TERMDEV_USB:
		if (!(_IS_TERMDEV_USING_USB(TERMDEV_Handle)))
		{
			// Call the USB initializer
			eInitErr = (eDEVError_s)USB_eInit(&sMCU_USB_Handle);
			if (eInitErr != DEV_ERROR_SUCCESS)
			{
				/* Return if error */
				return eInitErr;
			}
			else
			{
				/* USB initialized - set as reserved interface */
				TERMDEV_Handle.dReservedInterfaces |= TERMDEV_USB;
				sMCU_USB_Handle.fpUSBCallBack = TDV_USBCallback;
			}
		}
		/* UART is already enabled, set as active */
		TERMDEV_Handle.eActiveInterface = (termdev_interface_e)wActiveIface;
			break;

		case TERMDEV_CAN:
		if (!(_IS_TERMDEV_USING_CAN(TERMDEV_Handle)))
		{
			// Initializes CAN peripheral
			sMCU_CAN_Handle.fpCallback = TDV_CANCallback; //Set the callback
			eInitErr = (eDEVError_s)CAN_eInit(&sMCU_CAN_Handle);
			if (eInitErr != DEV_ERROR_SUCCESS)
			{
				/* Return if error */
				return eInitErr;
			}
			wCANSendID = 0;
			TERMDEV_Handle.dReservedInterfaces |= TERMDEV_CAN;
		}

		/* UART is already enabled, set as active */
		TERMDEV_Handle.eActiveInterface = (termdev_interface_e)wActiveIface;
			break;

		default:
			break;
	}
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : TDV_eDisable(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request.
 * This function will perform the MCULIB DeInit routine of the requested interface (UART, CAN
 * or USB). After the de-initialization, the interface is disabled and can be used by other modules
 * of the firmware.
 *
 * PRE-CONDITION: TERMDEV initiated and avaliable interface
 *
 * POST-CONDITION: Selected interface disabled.
 *
 * @return eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wInterface = TERMDEV_IFACE_UART;
 * TDV_eSetActive(IOCTL_TDV_DISABLE, &wInterface);
 ~~~~~~~~~~~~~~~
 *
 * @see TDV_open, TDV_write, TDV_read, TDV_ioctl, TDV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s TDV_eDisable (uint32_t wRequest, void * vpValue)
{
	/* Validation */
	if (vpValue == NULL)
	{
		return DEV_ERROR_TERMDEV_SETTING_ERROR;
	}

	uint32_t wActiveIface = *(uint32_t*)vpValue;
	if (!(_IS_TERMDEV_VALID_IFACE(wActiveIface)))
	{
		return DEV_ERROR_INVALID_PERIPHERAL;
	}

	if ((_IS_TERMDEV_USING_UART(TERMDEV_Handle)) && (wActiveIface == TERMDEV_UART))
	{
		/* Close UART on MCULIB */
		UART_vDeInit(&sMCU_UART_Handle);
		TERMDEV_Handle.dReservedInterfaces &= ~TERMDEV_UART;
		return DEV_ERROR_SUCCESS;
	}

	if ((_IS_TERMDEV_USING_CAN(TERMDEV_Handle)) && (wActiveIface == TERMDEV_CAN))
	{
		/* Close CAN on MCULIB */
		CAN_vDeInit(&sMCU_CAN_Handle);
		TERMDEV_Handle.dReservedInterfaces &= ~TERMDEV_CAN;
		return DEV_ERROR_SUCCESS;
	}

	/* TODO: develop the USB deinit routine */

	return DEV_ERROR_INVALID_PERIPHERAL;
}

/******************************************************************************
 * Function : TDV_eCANAddID(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request.
 * This function will enable the CAN peripheral to receive messages with the requested ID number. It also
 * saves the ID number as the current ID of the outgoing CAN messages.
 *
 * PRE-CONDITION: TERMDEV initiated and CAN interface enabled.
 *
 * POST-CONDITION: Any CAN output message will have the ID number passed at the vpValue argument.
 *
 * @return eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wNewCANID = 0x515;
 * TDV_eCANAddID(IOCTL_TDV_DISABLE, &wNewCANID);
 ~~~~~~~~~~~~~~~
 *
 * @see TDV_open, TDV_write, TDV_read, TDV_ioctl, TDV_close and TDV_eSetActive
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 23/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s TDV_eCANAddID (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	CAN_vAddMessageID(&sMCU_CAN_Handle, *(uint16_t*)vpValue);
	wCANSendID = *(uint32_t*)vpValue;
	return DEV_ERROR_SUCCESS;
}

eDEVError_s TDV_open (void)
{
	/* TERMDEV starts disabled */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_ENABLED;
	uint32_t wDefaultActive = ACTIVE_INTERFACE; // default interface defined in _config.h

	/* Init the receive buffer */
	RingBuffer_Init(&rbTDVHandle, bRecvBuffer, sizeof(uint8_t), RECV_BUFSIZE);
	RingBuffer_Flush(&rbTDVHandle);

	/* Try to init the default interface */
	return TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wDefaultActive);
}

/* ********************************** Public functions *******************************************/

uint32_t TDV_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	uint8_t *pbAuxPointer = (uint8_t*)vpBuffer;

	/* The received data is on the ring buffer */
	return TDV_wReadBufferProcedure(pbAuxPointer);
}

uint32_t TDV_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	uint32_t wSentBytes = 0;
	static canMSGStruct_s sCANMessage;
	uint8_t bIterator = 0;
	uint8_t *pbAuxPointer;

	/* Check if data is corrupted */
	if ((vpBuffer == NULL) || (tBufferSize == 0))  //Wrong inputs
	{
		return 0;
	}

	pbAuxPointer = (uint8_t*)vpBuffer;

	/* Checks the peripheral used by the handle */
	if ((TERMDEV_Handle.eActiveInterface == TERMDEV_UART) && (TERMDEV_Handle.dReservedInterfaces & TERMDEV_UART))
	{
		return UART_eSendData(&sMCU_UART_Handle, (uint8_t*)vpBuffer, tBufferSize);
	}

	if ((TERMDEV_Handle.eActiveInterface == TERMDEV_USB) && (TERMDEV_Handle.dReservedInterfaces & TERMDEV_USB))
	{
		return USB_wWrite(&sMCU_USB_Handle, (uint8_t*)vpBuffer, tBufferSize);
	}

	/* Basic protocol to send more than 8 bytes over CAN */
	if ((TERMDEV_Handle.eActiveInterface == TERMDEV_CAN) && (wCANSendID != 0)
		&& (TERMDEV_Handle.dReservedInterfaces & TERMDEV_CAN))
	{
		sCANMessage.id = wCANSendID;

		/* Loop until all bytes have been sent */
		while (wSentBytes < tBufferSize)
		{
			if ((tBufferSize - wSentBytes) <= 7)
			{
				/* Mark the firstbyte as 0xBB () */
				sCANMessage.data[0] = 0xBB;
				sCANMessage.dlc = (tBufferSize - wSentBytes) + 1;

				for (bIterator = 0; bIterator < (tBufferSize - wSentBytes); bIterator++)
				{
					//sCANMessage.data[bIterator+1] = *(uint8_t*)(vpBuffer++);
					sCANMessage.data[bIterator + 1] = *(pbAuxPointer++);
				}
				wSentBytes += (tBufferSize - wSentBytes);
			}
			else
			{
				/* Mark the firstbyte as 0xAA () */
				sCANMessage.data[0] = 0xAA;
				for (bIterator = 1; bIterator < 8; bIterator++)
				{
					//sCANMessage.data[bIterator] = *((uint8_t*)(vpBuffer++));
					sCANMessage.data[bIterator] = *(pbAuxPointer++);
				}
				sCANMessage.dlc = 8;
				wSentBytes += 7;
			}

			/* Send CAN frame */
			CAN_vSendMessage(&sMCU_CAN_Handle, sCANMessage);
		}
		return wSentBytes;
	}
	return 0;
}

eDEVError_s TDV_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if (wRequest >= IOCTL_TDV_INVALID)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	/* Call the request related function */
	return TDV_pIOCTLFunction[wRequest](wRequest, vpValue);
}

eDEVError_s TDV_close (struct peripheral_descriptor_s* const this)
{

	if (_IS_TERMDEV_USING_UART(TERMDEV_Handle))
	{
		/* Close UART on MCULIB */
		UART_vDeInit(&sMCU_UART_Handle);
		TERMDEV_Handle.dReservedInterfaces &= ~TERMDEV_UART;
	}

	if (_IS_TERMDEV_USING_CAN(TERMDEV_Handle))
	{
		/* Close CAN on MCULIB */
		CAN_vDeInit(&sMCU_CAN_Handle);
		TERMDEV_Handle.dReservedInterfaces &= ~TERMDEV_CAN;
	}

	if (_IS_TERMDEV_USING_USB(TERMDEV_Handle))
	{
		/* TODO: Implement the correct USB DeInit function */
	}

	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Unity Testing
 *******************************************************************************/
#if defined(UNITY_TEST)

void vSetDefaultHandler(void)
{
	sMCU_UART_Handle.eChannel = TERMDEV_UART_CHANNEL;
	sMCU_UART_Handle.eBaudrate = TERMDEV_UART_BAUD;
	sMCU_UART_Handle.eWordLenght = UART_WORD_8B;
	sMCU_UART_Handle.eParity = UART_PARITY_NONE;
	sMCU_UART_Handle.eStopbits = UART_STOPBITS1;
	sMCU_UART_Handle.eStatus = UART_RESET;
	sMCU_UART_Handle.fpCallBack = NULL;
	sMCU_UART_Handle.bInBuffer = NULL;

	sMCU_USB_Handle.eUSB = TERMDEV_USB_CHANNEL;
	sMCU_USB_Handle.eClass = USB_CLASS_CDC;
	sMCU_USB_Handle.fpUSBCallBack = NULL;

	sMCU_CAN_Handle.eCANPort = TERMDEV_CAN_CHANNEL;
	sMCU_CAN_Handle.eCANBitrate = TERMDEV_CAN_BITRATE;
	sMCU_CAN_Handle.fpCallback = NULL;

	TERMDEV_Handle.dReservedInterfaces = 0;
	TERMDEV_Handle.eActiveInterface = ACTIVE_INTERFACE;
	TERMDEV_Handle.eMessagesLevel = MSG_LEVEL5;
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_DISABLED;
	TERMDEV_Handle.fpCallback = NULL;

	RingBuffer_Init(&rbTDVHandle, bRecvBuffer, sizeof(uint8_t), RECV_BUFSIZE);
	RingBuffer_Flush(&rbTDVHandle);

}

TEST_GROUP(TERMDEVDeviceTest);

TEST_SETUP(TERMDEVDeviceTest)
{
	vSetDefaultHandler();
}

TEST_TEAR_DOWN(TERMDEVDeviceTest)
{
	vSetDefaultHandler();
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_vRBSafeInsert)
{
	uint8_t abTestBuffer[512];
	uint8_t abOutBuffer[512];
	uint32_t bTestCounter = 0;

	for (bTestCounter=0; bTestCounter < 512; bTestCounter++)
	{
		if (bTestCounter > 255)
		abTestBuffer[bTestCounter] = 1;
		else
		abTestBuffer[bTestCounter] = 2;
	}

	/* Insert 200 bytes */
	TDV_vRBSafeInsert(abTestBuffer, 200);
	RingBuffer_PopMult(&rbTDVHandle, abOutBuffer, 200);
	TEST_ASSERT_EQUAL_INT8_ARRAY(abTestBuffer, abOutBuffer, 200);

	RingBuffer_Flush(&rbTDVHandle);

	/* Insert 512 bytes in two blocks */
	TDV_vRBSafeInsert(abTestBuffer, 200);
	TDV_vRBSafeInsert(&abTestBuffer[300], 200);

	RingBuffer_PopMult(&rbTDVHandle, abOutBuffer, 256);

	/* check if the ringbuffer retained the most recent data */
	//TEST_ASSERT_EQUAL_INT8_ARRAY(&abTestBuffer[256], abOutBuffer, 256);
	RingBuffer_Flush(&rbTDVHandle);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_wReadBufferProcedure)
{
	uint8_t abTestBuffer[256];
	uint8_t abDummyBuffer[256];
	uint32_t bTestCounter = 0;

	/* Test empty ringbuffer */
	RingBuffer_Flush(&rbTDVHandle);
	TEST_ASSERT_EQUAL(0, TDV_wReadBufferProcedure(abTestBuffer));

	/*Insert 200 bytes into ringbuffer */
	for (bTestCounter=0; bTestCounter < 200; bTestCounter++)
	{
		TDV_vRBSafeInsert((uint8_t*) &bTestCounter, 1);
		abDummyBuffer[bTestCounter] = bTestCounter;
	}

	TEST_ASSERT_EQUAL(200, TDV_wReadBufferProcedure(abTestBuffer));
	TEST_ASSERT_EQUAL_INT8_ARRAY(abDummyBuffer, abTestBuffer, 200);

}

TEST(TERMDEVDeviceTest, test_Verify_TDV_CANCallback)
{
	canMSGStruct_s sCANMsg;
	uint8_t bDummyData[] =
	{	0xAA,1,2,3,4,5,6,7,1,2,3,4,5,6,7};

	sCANMsg.id = 0x515;
	sCANMsg.dlc = 0;
	strncpy((char*) sCANMsg.data, (const char*) bDummyData, 8);

	RingBuffer_Flush(&rbTDVHandle);

	/* Test if nothing happens when the interruption is not valid */
	TERMDEV_Handle.bDeviceStatus = TERMDEV_STATUS_ENABLED;
	TDV_CANCallback(0, sCANMsg);
	TEST_ASSERT_EQUAL(1, RingBuffer_IsEmpty(&rbTDVHandle));
	TEST_ASSERT_EQUAL(TERMDEV_STATUS_ENABLED, TERMDEV_Handle.bDeviceStatus);

	/* Test if data is passed to ringbuffer */
	sCANMsg.dlc = 8;
	TDV_CANCallback(0, sCANMsg);
	TEST_ASSERT_EQUAL_INT8_ARRAY(&bDummyData[1], bRecvBuffer, 7);
	TEST_ASSERT_EQUAL(TERMDEV_STATUS_BUSY, TERMDEV_Handle.bDeviceStatus);

	sCANMsg.dlc = 8;
	sCANMsg.data[0] = 0xBB; /* Signals the end of message */
	TDV_CANCallback(0, sCANMsg);
	TEST_ASSERT_EQUAL_INT8_ARRAY(&bDummyData[1], bRecvBuffer, 14);
	TEST_ASSERT_EQUAL(TERMDEV_STATUS_ENABLED, TERMDEV_Handle.bDeviceStatus);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_USBCallback)
{
	uint8_t bDummyData[] =
	{	'T', 'E', 'S', 'T', 'E'};

	RingBuffer_Flush(&rbTDVHandle);

	TDV_USBCallback();
	TEST_ASSERT_EQUAL(TERMDEV_STATUS_ENABLED, TERMDEV_Handle.bDeviceStatus);
	TEST_ASSERT_EQUAL_INT8_ARRAY(bDummyData, bRecvBuffer, 5);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_UARTCallback)
{
	uint8_t abTestBuffer[256];
	uint32_t bTestCounter = 0;

	for (bTestCounter=0; bTestCounter<256; bTestCounter++)
	abTestBuffer[bTestCounter] = bTestCounter;

	RingBuffer_Flush(&rbTDVHandle);

	TDV_UARTCallback(abTestBuffer, 256);
	TEST_ASSERT_EQUAL(1, RingBuffer_IsFull(&rbTDVHandle));
	TEST_ASSERT_EQUAL_INT8_ARRAY(abTestBuffer, bRecvBuffer, 256);
}

/* Test different settings for active interface */
TEST(TERMDEVDeviceTest, test_Verify_TDV_eSetActive)
{
	uint32_t wUARTSelected = TERMDEV_UART;
	uint32_t wUSBSelected = TERMDEV_USB;
	uint32_t wCANSelected = TERMDEV_CAN;
	uint32_t wInvalid = TERMDEV_INVALID;

	TERMDEV_Handle.dReservedInterfaces = 0;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wInvalid));

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUARTSelected));
	TEST_ASSERT_EQUAL(TERMDEV_UART, TERMDEV_Handle.dReservedInterfaces);
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_UART);
	TEST_ASSERT_EQUAL_PTR(sMCU_UART_Handle.fpCallBack, TDV_UARTCallback);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUSBSelected));
	TEST_ASSERT_EQUAL((TERMDEV_UART|TERMDEV_USB), TERMDEV_Handle.dReservedInterfaces);
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_USB);
	TEST_ASSERT_EQUAL_PTR(sMCU_USB_Handle.fpUSBCallBack, TDV_USBCallback);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wCANSelected));
	TEST_ASSERT_EQUAL((TERMDEV_UART|TERMDEV_USB|TERMDEV_CAN), TERMDEV_Handle.dReservedInterfaces);
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_CAN);
	TEST_ASSERT_EQUAL_PTR(sMCU_CAN_Handle.fpCallback, TDV_CANCallback);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUARTSelected));
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_UART);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUSBSelected));
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_USB);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wCANSelected));
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_CAN);

	TEST_ASSERT_EQUAL((TERMDEV_UART|TERMDEV_USB|TERMDEV_CAN), TERMDEV_Handle.dReservedInterfaces);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_eDisable)
{
	uint32_t wUARTSelected = TERMDEV_UART;
	uint32_t wUSBSelected = TERMDEV_USB;
	uint32_t wCANSelected = TERMDEV_CAN;
	uint32_t wInvalid = TERMDEV_INVALID;

	TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUARTSelected);
	TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUSBSelected);
	TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wCANSelected);

	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, TDV_eDisable(IOCTL_TDV_DISABLE, &wInvalid));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, TDV_eDisable(IOCTL_TDV_DISABLE, &wUSBSelected));

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eDisable(IOCTL_TDV_DISABLE, &wUARTSelected));
	TEST_ASSERT_EQUAL((TERMDEV_CAN|TERMDEV_USB), TERMDEV_Handle.dReservedInterfaces);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_eDisable(IOCTL_TDV_DISABLE, &wCANSelected));
	TEST_ASSERT_EQUAL(TERMDEV_USB, TERMDEV_Handle.dReservedInterfaces);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_eCANAddID)
{
	uint32_t value = 0x515;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL,TDV_eCANAddID(IOCTL_TDV_ADD_CAN_ID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,TDV_eCANAddID(IOCTL_TDV_ADD_CAN_ID, (void*) &value));
	TEST_ASSERT_EQUAL(0x515, wCANSendID);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_open)
{
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_open());
	TEST_ASSERT_EQUAL(TERMDEV_Handle.bDeviceStatus, TERMDEV_STATUS_ENABLED);

	TEST_ASSERT_EQUAL(TERMDEV_UART, TERMDEV_Handle.dReservedInterfaces);
	TEST_ASSERT_EQUAL(TERMDEV_Handle.eActiveInterface , TERMDEV_UART);
	TEST_ASSERT_EQUAL(1, RingBuffer_IsEmpty(&rbTDVHandle));
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_write)
{
	uint32_t wUARTSelected = TERMDEV_UART;
	uint32_t wUSBSelected = TERMDEV_USB;
	uint32_t wCANSelected = TERMDEV_CAN;
	uint32_t value = 0x515;

	uint8_t data1[] =
	{	'T', 'E', 'S', 'T', 'E'};
	uint8_t data2[] =
	{	'T', 'E', 'S', 'T', 'E', 'L', 'A', 'R', 'G', 'E'};

	TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUARTSelected);
	TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wUSBSelected);
	TDV_eSetActive(IOCTL_TDV_SET_ACTIVE, &wCANSelected);

	/* Try to write without select interface */
	TERMDEV_Handle.eActiveInterface = TERMDEV_INVALID;
	TEST_ASSERT_EQUAL(0, TDV_write(NULL, data1, 5));
	TEST_ASSERT_EQUAL(0, TDV_write(NULL, data2, 10));

	TERMDEV_Handle.eActiveInterface = TERMDEV_UART;
	TEST_ASSERT_EQUAL(5, TDV_write(NULL, data1, 5));
	TEST_ASSERT_EQUAL(10, TDV_write(NULL, data2, 10));
	TEST_ASSERT_EQUAL(0, TDV_write(NULL, NULL, 10));

	TERMDEV_Handle.eActiveInterface = TERMDEV_USB;
	TEST_ASSERT_EQUAL(5, TDV_write(NULL, data1, 5));
	TEST_ASSERT_EQUAL(10, TDV_write(NULL, data2, 10));

	/* No CAN ID was assigned yet */
	wCANSendID = 0;
	TERMDEV_Handle.eActiveInterface = TERMDEV_CAN;
	TEST_ASSERT_EQUAL(0, TDV_write(NULL, data1, 5));
	TEST_ASSERT_EQUAL(0, TDV_write(NULL, data2, 10));

	TDV_eCANAddID(IOCTL_TDV_ADD_CAN_ID, (void*) &value);
	TERMDEV_Handle.eActiveInterface = TERMDEV_CAN;
	TEST_ASSERT_EQUAL(5, TDV_write(NULL, data1, 5));
	TEST_ASSERT_EQUAL(10, TDV_write(NULL, data2, 10));
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_read)
{
	uint8_t bAuxDataBuffer[256];
	uint8_t bTestDataBuffer[256];
	uint32_t i = 0;

	for (i=0; i<256; i++)
	bAuxDataBuffer[i] = i;

	memset(bTestDataBuffer, 0, 256);
	RingBuffer_Flush(&rbTDVHandle);

	/* UART callback will fill the ring buffer */
	TDV_UARTCallback(bAuxDataBuffer, 256);
	TERMDEV_Handle.eActiveInterface = TERMDEV_UART;
	TEST_ASSERT_EQUAL(256, TDV_read(NULL, bTestDataBuffer, 256));
	TEST_ASSERT_EQUAL(TERMDEV_Handle.bDeviceStatus, TERMDEV_STATUS_ENABLED);

	memset(bTestDataBuffer, 0, 256);
	RingBuffer_Flush(&rbTDVHandle);

	/* Test USB timeout */
	TDV_USBCallback();
	TERMDEV_Handle.eActiveInterface = TERMDEV_USB;
	TEST_ASSERT_EQUAL(5, TDV_read(NULL, bAuxDataBuffer, 10));

	/* Test CAN timeout */
	uint8_t bDummyData[] =
	{	0xAA,1,2,3,4,5,6,7};
	canMSGStruct_s sCANMsg;

	sCANMsg.id = 0x515;
	sCANMsg.dlc = 8;
	strncpy((char*) sCANMsg.data, (const char*) bDummyData, 8);

	for (i=0; i<10; i++)
	TDV_CANCallback(0, sCANMsg);

	sCANMsg.data[0] = 0xBB;
	TDV_CANCallback(0, sCANMsg);

	TERMDEV_Handle.eActiveInterface = TERMDEV_CAN;
	TEST_ASSERT_EQUAL((11*7), TDV_read(NULL, bAuxDataBuffer, 256));

//	/* Test invalid */
//	TERMDEV_Handle.eActiveInterface = TERMDEV_INVALID;
//	TEST_ASSERT_EQUAL(0, TDV_read(NULL, bAuxDataBuffer, 10));
//
//	/* Test if buffer was transferred */
//	for (i=0; i<255; i++)
//	{
//		sRecvBuffer.bBufData[i] = i;
//		sRecvBuffer.bValidDataLenght++;
//	}
//
//	TERMDEV_Handle.eActiveInterface = TERMDEV_INVALID;
//	TEST_ASSERT_EQUAL(255, TDV_read(NULL, bAuxDataBuffer, 255));
//	TEST_ASSERT_EQUAL(TERMDEV_Handle.bDeviceStatus, TERMDEV_STATUS_ENABLED);
//
//	for (i=0; i<255; i++)
//	{
//		sRecvBuffer.bBufData[i] = i;
//		sRecvBuffer.bValidDataLenght++;
//	}
//
//	TEST_ASSERT_EQUAL_UINT8_ARRAY(sRecvBuffer.bBufData, bAuxDataBuffer, 255);
}

TEST(TERMDEVDeviceTest, test_Verify_TDV_ioctl)
{
	uint32_t wUARTSelected = TERMDEV_UART;
	uint32_t wUSBSelected = TERMDEV_USB;
	uint32_t wCANSelected = TERMDEV_CAN;
	uint32_t wExampleID = 0x515;

	/* Invalid IOCTL command - returning error can be different */
	TEST_ASSERT_NOT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_INVALID, NULL));
	TEST_ASSERT_NOT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_SET_ACTIVE, NULL));

	/* Test ioctl set active options */
	TERMDEV_Handle.dReservedInterfaces = 0;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_SET_ACTIVE, &wUARTSelected));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_SET_ACTIVE, &wUSBSelected ));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_SET_ACTIVE, &wCANSelected ));

	/* Test ioctl disable option */
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_DISABLE, &wUARTSelected));
	TEST_ASSERT_NOT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_DISABLE, &wUSBSelected ));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_DISABLE, &wCANSelected ));

	/* Test ioctl add ID option */
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, TDV_ioctl(NULL, IOCTL_TDV_ADD_CAN_ID, &wExampleID));
	TEST_ASSERT_EQUAL(wCANSendID, wExampleID);
}

TEST(TERMDEVDeviceTest, test_Verify_DEV_open)
{
	TEST_ASSERT_NULL(DEV_open(PERIPHERAL_INVALID));
	peripheral_descriptor_p TDVDevice = DEV_open(PERIPHERAL_TERMDEV);
	TEST_ASSERT_NOT_NULL(TDVDevice);

	//Try to reopen
	TEST_ASSERT_NULL(DEV_open(PERIPHERAL_TERMDEV));
	DEV_close(TDVDevice);
}

TEST(TERMDEVDeviceTest, test_Verify_DEV_ioctl)
{
	peripheral_descriptor_p TDVDevice = DEV_open(PERIPHERAL_TERMDEV);
	TEST_ASSERT_NOT_NULL(TDVDevice);

	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL,
			DEV_ioctl(TDVDevice, IOCTL_TDV_INVALID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL,
			DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, NULL));

	uint32_t wArgument = TERMDEV_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wArgument));
	wArgument = TERMDEV_USB;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wArgument));
	wArgument = TERMDEV_CAN;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wArgument));

	wArgument = TERMDEV_CAN | TERMDEV_UART | TERMDEV_USB;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL,
			DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wArgument));

	wArgument = TERMDEV_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_DISABLE, &wArgument));
	wArgument = TERMDEV_USB;
	TEST_ASSERT_NOT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_DISABLE, &wArgument));
	wArgument = TERMDEV_CAN;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_DISABLE, &wArgument));

	wArgument = 0x510;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS,
			DEV_ioctl(TDVDevice, IOCTL_TDV_ADD_CAN_ID, &wArgument));
	TEST_ASSERT_EQUAL(wCANSendID, wArgument);

	DEV_close(TDVDevice);
}

TEST(TERMDEVDeviceTest, test_Verify_DEV_write)
{
	TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
	vSetDefaultHandler();

	peripheral_descriptor_p TDVDevice = DEV_open(PERIPHERAL_TERMDEV);
	TEST_ASSERT_NOT_NULL(TDVDevice);
	TEST_ASSERT_EQUAL(0, DEV_write(TDVDevice, NULL, 0));

	uint8_t data[] =
	{	'T', 'E', 'S', 'T', ' ', 'L', 'A', 'R', 'G', 'E'};
	uint32_t wIface = TERMDEV_UART;
	DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wIface);
	TEST_ASSERT_EQUAL(10, DEV_write(TDVDevice, data, 10));

	wIface = TERMDEV_USB;
	DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wIface);
	TEST_ASSERT_EQUAL(10, DEV_write(TDVDevice, data, 10));

	wIface = TERMDEV_CAN;
	uint32_t wId = 0x10;
	DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wIface);
	TEST_ASSERT_EQUAL(0, DEV_write(TDVDevice, data, 10));
	DEV_ioctl(TDVDevice, IOCTL_TDV_ADD_CAN_ID, &wId);
	TEST_ASSERT_EQUAL(10, DEV_write(TDVDevice, data, 10));

	TERMDEV_Handle.eActiveInterface = TERMDEV_INVALID;
	TEST_ASSERT_EQUAL(0, DEV_write(TDVDevice, data, 10));

	/* Test if disabled interfaces return zero on write */

	wIface = TERMDEV_UART;
	DEV_ioctl(TDVDevice, IOCTL_TDV_DISABLE, &wIface);
	wIface = TERMDEV_CAN;
	DEV_ioctl(TDVDevice, IOCTL_TDV_DISABLE, &wIface);

	TEST_ASSERT_EQUAL(TERMDEV_Handle.dReservedInterfaces, TERMDEV_USB);

	TERMDEV_Handle.eActiveInterface = TERMDEV_UART;
	TEST_ASSERT_EQUAL(0, DEV_write(TDVDevice, data, 10));

	TERMDEV_Handle.eActiveInterface = TERMDEV_CAN;
	TEST_ASSERT_EQUAL(0, DEV_write(TDVDevice, data, 10));

	DEV_close(TDVDevice);
}

TEST(TERMDEVDeviceTest, test_Verify_DEV_read)
{
	uint32_t wUARTSelected = TERMDEV_UART;
	uint32_t wUSBSelected = TERMDEV_USB;
	uint32_t wCANSelected = TERMDEV_CAN;
	uint32_t wExampleID = 0x515;

	uint8_t bTestBuffer[RECV_BUFSIZE];
	uint8_t bDummyBuffer[RECV_BUFSIZE];

	memset(bDummyBuffer, 0x0a, RECV_BUFSIZE);

	peripheral_descriptor_p TDVDevice = DEV_open(PERIPHERAL_TERMDEV);
	TEST_ASSERT_NOT_NULL(TDVDevice);

	/* No interface active */
	TEST_ASSERT_EQUAL(0, DEV_read(TDVDevice, bTestBuffer, 200));

	/* Try to read UART */
	DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wUARTSelected);
	TDV_UARTCallback(bDummyBuffer, RECV_BUFSIZE);
	TEST_ASSERT_EQUAL(RECV_BUFSIZE, DEV_read(TDVDevice, bTestBuffer, RECV_BUFSIZE));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(bDummyBuffer, bTestBuffer, RECV_BUFSIZE);

	RingBuffer_Flush(&rbTDVHandle);

	/* Try to read USB - callback will reach timeout */
	DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wUSBSelected);
	TEST_ASSERT_EQUAL(0, DEV_read(TDVDevice, bTestBuffer, 200));

	/* Try to read CAN */
	DEV_ioctl(TDVDevice, IOCTL_TDV_SET_ACTIVE, &wCANSelected);
	TEST_ASSERT_EQUAL(0, DEV_read(TDVDevice, bTestBuffer, 200));

	DEV_ioctl(TDVDevice, IOCTL_TDV_ADD_CAN_ID, &wExampleID);
	TEST_ASSERT_EQUAL(0, DEV_read(TDVDevice, bTestBuffer, 200));

	RingBuffer_Flush(&rbTDVHandle);

	/* Test CAN */
	uint8_t bDummyCAN[] =
	{	0xAA,1,2,3,4,5,6,7};
	uint8_t bDummyCAN2[] =
	{	1,2,3,4,5,6,7,1,2,3,4,5,6,7};
	canMSGStruct_s sCANMsg;

	sCANMsg.id = 0x515;
	sCANMsg.dlc = 8;
	strncpy((char*) sCANMsg.data, (const char*) bDummyCAN, 8);

	TDV_CANCallback(0, sCANMsg);

	TEST_ASSERT_EQUAL(0, DEV_read(TDVDevice, bTestBuffer, 14));

	sCANMsg.data[0] = 0xBB;
	TDV_CANCallback(0, sCANMsg);

	/* Read 14 bytes from CAN */
	TEST_ASSERT_EQUAL(14, DEV_read(TDVDevice, bTestBuffer, 14));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(bDummyCAN2, bTestBuffer, 14);

	DEV_close(TDVDevice);
}

#endif

