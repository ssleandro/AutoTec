/****************************************************************************
 * Title                 :   CBACOMM Include File
 * Filename              :   CBACOMM.c
 * Author                :   thiago.palmieri
 * Origin Date           :   18 de mar de 2016
 * Version               :   1.0.0
 * Compiler              :   GCC 5.2 2015q4 / ICCARM 7.50.2.10312
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
 *    Date    Version   Author         Description
 *  18 de mar de 2016   1.0.0   thiago.palmieri CBACOMM include file Created.
 *
 *****************************************************************************/
/** @file CBACOMM.c
 *  @brief This file provides Control device for CBA Communication.
 *
 *  This device must be used to initiate communication towards CBA CPU. The
 *  default interface is CAN, but it can be changed to UART or USB on the fly.
 *
 *  For each interface, a proper command sequence must be used to properly initiate it.
 *
 *  CAN:
 *  - First the interface must be opened (DEV_open).
 *  - Then message IDs must be added for monitoring (DEV_ioctl - one command per message ID).
 *  - After the first ID is added, then the interface is initialized.
 *  - In order to send CAN messages, DEV_ioctl must be used previously to inform message ID.
 *
 *  USB / UART:
 *  - First the interface must be opened (DEV_open).
 *  - After opening the interface, it will be ready for usage.
 *
 *  In order to close the interface, command DEV_close must be used.
 *
 *  Commands DEV_read, DEV_write must be used to read/write data from the interface
 *  respectively.
 *
 *  The interface works via message buffers, therefore if the message is larger than
 *  the available payload for the interface, it will be split in separate messages.
 *
 *  The default reading method is pooling, if interrupt method need to be used,
 *  the DEV_ioctl must be used to add an Interrupt CallBack. If you need to return to
 *  pooling method after defining a callback, then update the callback with NULL.
 *
 *  Check the _config.h file for defaults.
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "CBACOMM.h"
#include "CBACOMM_config.h"
#include <string.h>
#include "ring_buffer.h"
#ifndef UNITY_TEST
#include "mculib.h"
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

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
#if defined (UNITY_TEST)
uint8_t UNITY_TESTING_VARIABLES;
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

canMSGStruct_s sUnityMessage =
{
	.id = 0,
	.dlc = 0,
	.data =
	{	0,0,0,0,0,0,0,0},
};
uint8_t END_OF_UNITY_TESTING_VARIABLES;
#endif

typedef eDEVError_s (*fpIOCTLFunction) (uint32_t wRequest, void * vpValue);
/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static struct
{
	uint8_t type;                             //!Interface being used, See _config.h file
	union
	{
		usb_config_s sUSB;          //!< USB Config handler
		can_config_s sCAN;          //!< CAN config handler
		uart_config_s sUART;        //!< UART config handler
	};
} sCBACOMM;             //!< Struct holding the interface configuration

static uint8_t bBufferArray[CCM_DEFAULT_BUFFER_SIZE];           //!< RingBuffer array
//static canMSGStruct_s bCANBufferArray[CCM_DEFAULT_BUFFER_SIZE]; //!< RingBuffer array for CAN
static RINGBUFF_T ccmBuffer;                                    //!< RingBuffer Control handle
CCMCallBack fpCCMUserCallBack = NULL;                           //!< Default User Callback
static uint32_t wCANSendID = 0;                                 //!< Next CAN message ID
volatile uint8_t g_rxBuff[CCM_DEFAULT_BUFFER_SIZE];

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static eDEVError_s CCM_eIOCTLChangeCallBack (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eIOCTLAddID (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eIOCTLChangeSpeed (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eIOCTLChangeInterface (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eIOCTLRemoveID (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eIOCTLSendID (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eIOCTLGets (uint32_t wRequest, void * vpValue);
static eDEVError_s CCM_eInternalOpen (void);

#define X(a, b) b,
fpIOCTLFunction CCM_pIOCTLFunction[] =  //!< IOCTL array of function mapping
	{
	IOCTL_CBACOMM
	};
#undef X
/******************************************************************************
 * Function Definitions
 *******************************************************************************/
#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);
uint32_t USB_wWrite(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{
	(void)pUSB;
	(void)pBuf;
	return buf_len;
}
uint32_t UART_eSendData(uart_config_s *pUART, uint8_t *vData, int32_t iLenght)
{	return iLenght;}
uint32_t USB_wBufferRead(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{
	*(pBuf + 0) = 'T';
	*(pBuf + 1) = 'E';
	*(pBuf + 2) = 'S';
	*(pBuf + 3) = 'T';
	*(pBuf + 4) = 'E';
	return 5;
}
void CAN_vSendMessage(const can_config_s *pCAN, const canMSGStruct_s CANMessage)
{}
eDEVError_s USB_eInit(usb_config_s * pUSB)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s CAN_eInit(can_config_s *pCAN)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s UART_eInit(uart_config_s *pUART)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s USB_eDeInit(usb_config_s *pUSB)
{	return DEV_ERROR_SUCCESS;}
void CAN_vDeInit(can_config_s *pCAN)
{}
void UART_vDeInit(uart_config_s *pUART)
{}
void CAN_vAddMessageID(const can_config_s *pCAN, const uint16_t hCANmsgID)
{}
void CAN_vRemoveMessageID(const can_config_s *pCAN, const uint16_t hCANmsgID)
{}
eDEVError_s UART_eDisableIRQ(uart_config_s *pUART)
{	return DEV_ERROR_SUCCESS;}
eDEVError_s UART_eRecvData_IT(uart_config_s *pUART, int32_t iIRQPrio, int32_t iArg)
{	return DEV_ERROR_SUCCESS;}
void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
 * Function : CCM_vthisUSBCallBack(void)
 *//**
 * \b Description:
 *
 * This is a private function used as callback for USB interrupt routines.
 * This function will read the USB buffer and save the result into a circular buffer
 * if there is no user callback defined. If a User callback is defined then it will
 * be executed sending the data read.
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
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
void CCM_vthisUSBCallBack (void)
{
//  uint8_t g_rxBuff[256];
	uint8_t bSize = USB_wBufferRead(&sCBACOMM.sUSB, (uint8_t*)&g_rxBuff[0], CCM_DEFAULT_BUFFER_SIZE);
	if (fpCCMUserCallBack != NULL)
	{
		fpCCMUserCallBack((void*)&g_rxBuff[0], (uint32_t)bSize);
	}
	else
	{
		//Store data into Ring Buffer
		if (!RingBuffer_InsertMult(&ccmBuffer, (uint8_t*)&g_rxBuff[0], bSize)) //RingBuffer Full
		{
			uint8_t bGarbage[CCM_DEFAULT_BUFFER_SIZE];
			//Pop from tail and then insert into head
			RingBuffer_PopMult(&ccmBuffer, &bGarbage[0], bSize);
			RingBuffer_InsertMult(&ccmBuffer, (uint8_t*)&g_rxBuff[0], bSize);
		}
	}

}

/******************************************************************************
 * Function : CCM_vthisCANCallBack(eCANStatus_s eErrorCode, canMSGStruct_s CANMessage)
 *//**
 * \b Description:
 *
 * This is a private function used as callback for CAN interrupt routines.
 * This function will save the read data into a circular buffer
 * if there is no user callback defined. If a User callback is defined then it will
 * be executed sending the data read.
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
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
void CCM_vthisCANCallBack (eCANStatus_s eErrorCode, canMSGStruct_s CANMessage)
{
	if ((CANMessage.dlc == 0) || (CANMessage.dlc > 8))
	{
		return;
	}

	//Decide if callback can be triggered
	bool bTriggerCallback = false;
	if (CANMessage.data[0] == CCM_CAN_MESSAGE_FINAL)
	{
		bTriggerCallback = true;
	}

	// Extract multipacket byte and store it on buffer
	uint8_t bGarbage = 0;
	uint8_t bIndex = 1;
	for (bIndex = 1; bIndex < CANMessage.dlc; bIndex++)
	{
		//Store data into Ring Buffer
		if (!RingBuffer_Insert(&ccmBuffer, &CANMessage.data[bIndex])) //RingBuffer Full
		{
			//Pop from tail and then insert into head
			RingBuffer_Pop(&ccmBuffer, &bGarbage);
			RingBuffer_Insert(&ccmBuffer, &CANMessage.data[bIndex]);
		}
	}

	//If it has continuation, do not trigger callback
	if ((fpCCMUserCallBack != NULL) && bTriggerCallback)
	{
		uint32_t wRingCount = RingBuffer_GetCount(&ccmBuffer);
		if (wRingCount && (wRingCount <= CCM_DEFAULT_BUFFER_SIZE)) //Check if buffer size is enough
		{
			RingBuffer_PopMult(&ccmBuffer, (uint8_t*)&g_rxBuff[0], wRingCount);
			fpCCMUserCallBack((void*)&g_rxBuff[0], wRingCount);
		}
	}
}

/******************************************************************************
 * Function : CCM_vthisUARTCallBack(uint8_t *bBuffer, uint32_t bLen)
 *//**
 * \b Description:
 *
 * This is a private function used as callback for UART interrupt routines.
 * This function will save the read data into a circular buffer
 * if there is no user callback defined. If a User callback is defined then it will
 * be executed sending the data read.
 *
 * This function must only be used as callback for UART interrupt handler from mculib.
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
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
void CCM_vthisUARTCallBack (uint8_t *bBuffer, uint32_t bLen)
{
	if (fpCCMUserCallBack != NULL)
	{
		fpCCMUserCallBack(bBuffer, bLen);
	}
	else //Store data into Ring Buffer
	{
		if (bLen > 1)
		{
			if (!RingBuffer_InsertMult(&ccmBuffer, bBuffer, bLen)) //RingBuffer Full
			{
				//Pop from tail and then insert into head
				uint8_t abGarbage[CCM_DEFAULT_BUFFER_SIZE];
				RingBuffer_PopMult(&ccmBuffer, &abGarbage, bLen);
				RingBuffer_InsertMult(&ccmBuffer, bBuffer, bLen);
			}
		}
		else
		{
			uint8_t bGarbage = 0;
			if (!RingBuffer_Insert(&ccmBuffer, bBuffer)) //RingBuffer Full
			{
				//Pop from tail and then insert into head
				RingBuffer_Pop(&ccmBuffer, &bGarbage);
				RingBuffer_Insert(&ccmBuffer, bBuffer);
			}
		}
	}
}

/******************************************************************************
 * Function : CCM_eIOCTLChangeCallBack(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to change the User Callback. This function must be used by CCM_ioctl ONLY !
 * A NULL input is also accepted, and it is used to unregister any callback
 * previously registered, thus making the data read buffered.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_CHANGE_CALLBACK;
 * if (!CCM_eIOCTLChangeCallBack(wRequest, vpValue)) PRINTF("Callback changed");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eIOCTLChangeCallBack (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	// NULL Callback is valid.
	fpCCMUserCallBack = (CCMCallBack)vpValue;
	return DEV_ERROR_SUCCESS;

}

/******************************************************************************
 * Function : CCM_eIOCTLAddID(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to add message reception ID for CAN. This function must be used by CCM_ioctl ONLY !
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_CAN_ADD_RECEIVE_ID;
 * if (!CCM_eIOCTLAddID(wRequest, vpValue)) PRINTF("CAN ID added");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eIOCTLAddID (uint32_t wRequest, void * vpValue)
{
	if (sCBACOMM.type != CCM_INTERFACE_CAN)
	{
		return DEV_ERROR_INVALID_PERIPHERAL;
	}
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	CAN_vAddMessageID(&sCBACOMM.sCAN, *(uint16_t*)vpValue);
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : CCM_eIOCTLChangeSpeed(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to change CAN or UART speeds. This function must be used by CCM_ioctl ONLY !
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_CHANGE_CAN_SPEED; // or IOCTL_CCM_CHANGE_UART_SPEED
 * if (!CCM_eIOCTLChangeSpeed(wRequest, vpValue)) PRINTF("Speed changed");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eIOCTLChangeSpeed (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	eDEVError_s eError = DEV_ERROR_INVALID_IOCTL;
	switch ((IOCTL_CCM_REQUEST_e)wRequest)
	{
		case IOCTL_CCM_CHANGE_CAN_SPEED:
		if (sCBACOMM.type == CCM_INTERFACE_CAN) //The active interface is CAN
		{
			//change speed, reinit it
			if (*(canBitrate_s*)vpValue >= CAN_BITRATE_INVALID)
			{
				return DEV_ERROR_INVALID_IOCTL;
			}
			eError = CCM_close(NULL);
			sCBACOMM.sCAN.eCANPort = CCM_DEFAULT_CAN;
			sCBACOMM.sCAN.eCANBitrate = *(canBitrate_s*)vpValue;
			sCBACOMM.sCAN.fpCallback = CCM_vthisCANCallBack;
			eError = (eDEVError_s)CAN_eInit(&sCBACOMM.sCAN);
		}
			break;
		case IOCTL_CCM_CHANGE_UART_SPEED:
		if (sCBACOMM.type == CCM_INTERFACE_UART)  //The active interface is UART
		{
			//change speed, reinit it
			if (*(uart_baudrate_e*)vpValue >= UART_BINVALID)
			{
				return DEV_ERROR_INVALID_IOCTL;
			}
			eError = CCM_close(NULL);
			sCBACOMM.sUART.eChannel = CCM_DEFAULT_UART;
			sCBACOMM.sUART.eBaudrate = *(uart_baudrate_e*)vpValue;
			;
			sCBACOMM.sUART.eWordLenght = CCM_DEFAULT_UART_WORD;
			sCBACOMM.sUART.eStopbits = CCM_DEFAULT_UART_STOP;
			sCBACOMM.sUART.eParity = CCM_DEFAULT_UART_PARITY;
			sCBACOMM.sUART.fpCallBack = CCM_vthisUARTCallBack;
			eError = (eDEVError_s)UART_eInit(&sCBACOMM.sUART);
		}
			break;
		default:
		eError = DEV_ERROR_INVALID_IOCTL;
			break;
	}
	return eError;

}

/******************************************************************************
 * Function : CCM_eIOCTLChangeInterface(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to change used interface between (CAN, USB and UART). This function must be
 * used by CCM_ioctl ONLY !
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_CHANGE_INTERFACE;
 * if (!CCM_eIOCTLChangeInterface(wRequest, vpValue)) PRINTF("Interface changed");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eIOCTLChangeInterface (uint32_t wRequest, void * vpValue)
{
	if ((vpValue == NULL) || (*(uint8_t*)vpValue >= CCM_INTERFACE_INVALID))
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	//Stop Interface and initialize the new one
	eDEVError_s eError;

	eError = CCM_close(NULL);

	sCBACOMM.type = *(uint8_t*)vpValue;

	eError = CCM_eInternalOpen();

	return eError;
}

/******************************************************************************
 * Function : CCM_eIOCTLRemoveID(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to remove a receive ID from CAN. This function must be used by CCM_ioctl ONLY !
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_CAN_REMOVE_RECEIVE_ID;
 * if (!CCM_eIOCTLRemoveID(wRequest, vpValue)) PRINTF("ID removed changed");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eIOCTLRemoveID (uint32_t wRequest, void * vpValue)
{
	if (sCBACOMM.type != CCM_INTERFACE_CAN)
	{
		return DEV_ERROR_INVALID_PERIPHERAL;
	}
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	CAN_vRemoveMessageID(&sCBACOMM.sCAN, *(uint16_t*)vpValue);
	return DEV_ERROR_SUCCESS;

}

/******************************************************************************
 * Function : CCM_eIOCTLSendID(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to change the sending ID for CAN. This function must be used by CCM_ioctl ONLY !
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_CAN_NEXT_MESSAGE_ID;
 * if (!CCM_eIOCTLSendID(wRequest, vpValue)) PRINTF("send ID changed");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eIOCTLSendID (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	wCANSendID = *(uint32_t*)vpValue;
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : CCM_eIOCTLGets(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to get Speed, callback and interface being used. A speed equal to Zero means that
 * USB is being used.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * wRequest = IOCTL_CCM_GET_INTERFACE;
 * if (!CCM_eIOCTLGets(wRequest, vpValue))
 * {
 *  *(uint32_t*)vpValue == 0 ? "UART" : (*(uint32_t*)vpValue == 1 ? "CAN" : "USB" );
 * }
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static eDEVError_s CCM_eIOCTLGets (uint32_t wRequest, void * vpValue)
{

	switch (wRequest)
	{
		case IOCTL_CCM_GET_INTERFACE:
		*(uint32_t*)vpValue = (uint32_t)sCBACOMM.type;
			break;
		case IOCTL_CCM_GET_SPEED:
		if (sCBACOMM.type == CCM_INTERFACE_CAN)
		{
			*(uint32_t*)vpValue = sCBACOMM.sCAN.eCANBitrate;
		}
		else if (sCBACOMM.type == CCM_INTERFACE_UART)
		{
			*(uint32_t*)vpValue = sCBACOMM.sUART.eBaudrate;
		}
		else
		{
			*(uint32_t*)vpValue = 0;
		}
			break;
		default:
		return DEV_ERROR_INVALID_IOCTL;
			break;
	}
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : CCM_eInternalOpen(void)
 *//**
 * \b Description:
 *
 * This is a private function used to initialize a given interface defined by sCBACOMM.type.
 * It will initiate the interface with default values and also initiate a circular buffer.
 * This function WILL NOT DE INITIALIZE previously initialized interfaces.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: Interface Initialized
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (!CCM_eInternalOpen()) PRINTF("Interface opened");
 ~~~~~~~~~~~~~~~
 *
 * @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
static eDEVError_s CCM_eInternalOpen (void)
{
	eDEVError_s eError;
	switch (sCBACOMM.type)
	{
		case CCM_INTERFACE_USB:
		RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
		RingBuffer_Flush(&ccmBuffer);
		sCBACOMM.sUSB.eUSB = CCM_DEFAULT_USB;
		sCBACOMM.sUSB.eClass = CCM_DEFAULT_USB_CLASS;
		sCBACOMM.sUSB.fpUSBCallBack = CCM_vthisUSBCallBack;
		sCBACOMM.sUSB.vpPrivateData = NULL;
		eError = (eDEVError_s)USB_eInit(&sCBACOMM.sUSB);
			break;
		case CCM_INTERFACE_CAN:
		RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
		RingBuffer_Flush(&ccmBuffer);
		sCBACOMM.sCAN.eCANPort = CCM_DEFAULT_CAN;
		sCBACOMM.sCAN.eCANBitrate = CCM_DEFAULT_CAN_BITRATE;
		sCBACOMM.sCAN.fpCallback = CCM_vthisCANCallBack;
		sCBACOMM.sCAN.vpPrivateData = NULL;
		eError = (eDEVError_s)CAN_eInit(&sCBACOMM.sCAN);
			break;
		case CCM_INTERFACE_UART:
		RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
		RingBuffer_Flush(&ccmBuffer);
		sCBACOMM.sUART.eChannel = CCM_DEFAULT_UART;
		sCBACOMM.sUART.eBaudrate = CCM_DEFAULT_UART_BAUD;
		sCBACOMM.sUART.eWordLenght = CCM_DEFAULT_UART_WORD;
		sCBACOMM.sUART.eStopbits = CCM_DEFAULT_UART_STOP;
		sCBACOMM.sUART.eParity = CCM_DEFAULT_UART_PARITY;
		sCBACOMM.sUART.fpCallBack = CCM_vthisUARTCallBack;
		sCBACOMM.sUART.vpPrivate = NULL;
		eError = (eDEVError_s)UART_eInit(&sCBACOMM.sUART);
		eError = (eDEVError_s)UART_eRecvData_IT(&sCBACOMM.sUART, 3, UART_RX_PERMANENT); //Interrupt on 1 byte
			break;
		default:
		eError = DEV_ERROR_INVALID_PERIPHERAL;
			break;
	}
	return eError;
}

eDEVError_s CCM_open (void)
{
	//Set Initial conditions
	sCBACOMM.type = CCM_DEFAULT_INTERFACE;
	fpCCMUserCallBack = NULL;
	wCANSendID = 0;
	return CCM_eInternalOpen();
}

uint32_t CCM_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	if ((vpBuffer == NULL) || (tBufferSize == 0))
	{
		return 0;
	}
	uint32_t wResult = 0;
	if (fpCCMUserCallBack == NULL)  //No user Callback defined, read from buffer
	{
		uint32_t wRingCount = RingBuffer_GetCount(&ccmBuffer);
		if (wRingCount && (wRingCount <= tBufferSize)) //Check if buffer size is enough
		{
			RingBuffer_PopMult(&ccmBuffer, (void*)vpBuffer, wRingCount);
			wResult = wRingCount;
		}
	}
	return wResult;
}

uint32_t CCM_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	uint32_t wReturn = 0;
	if ((vpBuffer == NULL) || (tBufferSize == 0))  //Wrong inputs
	{
		return wReturn;
	}

	uint8_t bCounter = 0;
	switch (sCBACOMM.type)
	{
		case CCM_INTERFACE_USB:   //USB Message
		wReturn = USB_wWrite(&sCBACOMM.sUSB, (uint8_t *)vpBuffer, tBufferSize);
			break;
		case CCM_INTERFACE_UART:  //UART MEssage
		wReturn = UART_eSendData(&sCBACOMM.sUART, (uint8_t *)vpBuffer, tBufferSize);
			break;
		case CCM_INTERFACE_CAN:   //CAN Message
		if (!wCANSendID)        //No ID provided
		{
			return 0;
		}

		canMSGStruct_s sCANMessage;
		sCANMessage.id = wCANSendID;
		wReturn = tBufferSize;
		//The WHILE below will chop the CAN message to 8Byte size chunks and format
		//each message accordingly
		memset(sCANMessage.data, 0x00, 8);
		while (wReturn)          //While Buffer Size > 0
		{
			if (wReturn > 7)  //If larger than 8, chop message
			{
				sCANMessage.dlc = 8;
				sCANMessage.data[0] = CCM_CAN_MESSAGE_HAS_MORE; //More messages will follow
			}
			else  //Message has less than 8 Bytes
			{
				sCANMessage.dlc = wReturn + 1;
				sCANMessage.data[0] = CCM_CAN_MESSAGE_FINAL; //Final Message
			}

			uint8_t bIndex = 0;
			while (wReturn && (bIndex < 7))
			{
				wReturn--;
				sCANMessage.data[bIndex + 1] = *((uint8_t*)vpBuffer + bIndex + (bCounter * 7));
				bIndex++;
			}
			CAN_vSendMessage(&sCBACOMM.sCAN, sCANMessage);
#if defined (UNITY_TEST)
			sUnityMessage = sCANMessage;
#endif
			memset(sCANMessage.data, 0x00, 8);
			bCounter++;
		}
		wReturn = tBufferSize;
			break;
		default:
			break;
	}
	return wReturn;
}

eDEVError_s CCM_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if ((wRequest >= IOCTL_CCM_INVALID) || (vpValue == NULL))
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	eDEVError_s eError;

	// Call specific function
	eError = CCM_pIOCTLFunction[wRequest](wRequest, vpValue);

	return eError;
}

eDEVError_s CCM_close (struct peripheral_descriptor_s* const this)
{
	eDEVError_s eError;
	RingBuffer_Flush(&ccmBuffer);

	switch (sCBACOMM.type)
	{
		case CCM_INTERFACE_USB:
		eError = (eDEVError_s)USB_eDeInit(&sCBACOMM.sUSB);
			break;
		case CCM_INTERFACE_CAN:
		eError = DEV_ERROR_SUCCESS;
		CAN_vDeInit(&sCBACOMM.sCAN);
			break;
		case CCM_INTERFACE_UART:
		eError = DEV_ERROR_SUCCESS;
		eError = (eDEVError_s)UART_eDisableIRQ(&sCBACOMM.sUART);
		UART_vDeInit(&sCBACOMM.sUART);
			break;
		default:
		eError = DEV_ERROR_INVALID_PERIPHERAL;
			break;
	}
	return eError;
}

/******************************************************************************
 * Unity Testing
 *******************************************************************************/
#if defined(UNITY_TEST)

TEST_GROUP(CBACOMMDeviceTest);

TEST_SETUP(CBACOMMDeviceTest)
{
}

TEST_TEAR_DOWN(CBACOMMDeviceTest)
{
}

/*
 * This test will verify the list initialization to all NULLs
 */
TEST(CBACOMMDeviceTest, test_Verify_thisUSBCallback)
{
	uint8_t buffer[10];

	void thisCallback(void *message, uint32_t data)
	{
		uint32_t dataSize = 0;
		dataSize = data;
		TEST_ASSERT_EQUAL(5, dataSize);
		TEST_ASSERT_EQUAL('T', *(uint8_t*)message);
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('E', *((uint8_t*)message + 1));
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('S', *((uint8_t*)message + 2));
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('T', *((uint8_t*)message + 3));
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('E', *((uint8_t*)message + 4));
	}
	fpCCMUserCallBack = NULL;
	RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
	RingBuffer_Flush(&ccmBuffer);

	CCM_vthisUSBCallBack();
	TEST_ASSERT_EQUAL(5, RingBuffer_GetCount(&ccmBuffer));
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('T', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('E', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('S', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('T', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('E', buffer[0]);

	fpCCMUserCallBack = thisCallback;
	CCM_vthisUSBCallBack();
}

TEST(CBACOMMDeviceTest, test_Verify_thisCANCallback)
{
	fpCCMUserCallBack = NULL;
	eCANStatus_s eError = 0;
	canMSGStruct_s sCANm;
	sCANm.id = 10;
	sCANm.dlc = 6;
	sCANm.data[0] = CCM_CAN_MESSAGE_FINAL;
	sCANm.data[1] = 'T';
	sCANm.data[2] = 'E';
	sCANm.data[3] = 'S';
	sCANm.data[4] = 'T';
	sCANm.data[5] = 'E';
	uint8_t buffer[CCM_DEFAULT_BUFFER_SIZE];

	void thisCallback(void *message, uint32_t data)
	{
		uint8_t * this = (uint8_t*)message;
		TEST_ASSERT_EQUAL(5, data);
		TEST_ASSERT_EQUAL('T', this[0]);
		TEST_ASSERT_EQUAL('E', this[1]);
		TEST_ASSERT_EQUAL('S', this[2]);
		TEST_ASSERT_EQUAL('T', this[3]);
		TEST_ASSERT_EQUAL('E', this[4]);
	}
	RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
	RingBuffer_Flush(&ccmBuffer);

	CCM_vthisCANCallBack(eError, sCANm);
	TEST_ASSERT_EQUAL(5, RingBuffer_GetCount(&ccmBuffer));
	RingBuffer_PopMult(&ccmBuffer, &buffer,5);
	TEST_ASSERT_EQUAL('T', buffer[0]);
	TEST_ASSERT_EQUAL('E', buffer[1]);
	TEST_ASSERT_EQUAL('S', buffer[2]);
	TEST_ASSERT_EQUAL('T', buffer[3]);
	TEST_ASSERT_EQUAL('E', buffer[4]);

	fpCCMUserCallBack = thisCallback;
	CCM_vthisCANCallBack(eError, sCANm);
}

TEST(CBACOMMDeviceTest, test_Verify_thisCANCallback_Multi_packet)
{
	fpCCMUserCallBack = NULL;
	eCANStatus_s eError = 0;
	canMSGStruct_s sCANm, sCANm2;
	sCANm.id = 10;
	sCANm.dlc = 8;
	sCANm.data[0] = CCM_CAN_MESSAGE_HAS_MORE;
	sCANm.data[1] = 'T';
	sCANm.data[2] = 'E';
	sCANm.data[3] = 'S';
	sCANm.data[4] = 'T';
	sCANm.data[5] = 'E';
	sCANm.data[6] = 'L';
	sCANm.data[7] = 'A';
	sCANm2.id = 10;
	sCANm2.dlc = 4;
	sCANm2.data[0] = CCM_CAN_MESSAGE_FINAL;
	sCANm2.data[1] = 'R';
	sCANm2.data[2] = 'G';
	sCANm2.data[3] = 'E';

	uint8_t buffer[CCM_DEFAULT_BUFFER_SIZE];

	void thisCallback(void *message, uint32_t data)
	{
		uint8_t * this = (uint8_t*)message;
		TEST_ASSERT_EQUAL(10, data);
		TEST_ASSERT_EQUAL('T', this[0]);
		TEST_ASSERT_EQUAL('E', this[1]);
		TEST_ASSERT_EQUAL('S', this[2]);
		TEST_ASSERT_EQUAL('T', this[3]);
		TEST_ASSERT_EQUAL('E', this[4]);
		TEST_ASSERT_EQUAL('L', this[5]);
		TEST_ASSERT_EQUAL('A', this[6]);
		TEST_ASSERT_EQUAL('R', this[7]);
		TEST_ASSERT_EQUAL('G', this[8]);
		TEST_ASSERT_EQUAL('E', this[9]);
	}
	RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
	RingBuffer_Flush(&ccmBuffer);

	CCM_vthisCANCallBack(eError, sCANm);
	CCM_vthisCANCallBack(eError, sCANm2);
	TEST_ASSERT_EQUAL(10, RingBuffer_GetCount(&ccmBuffer));
	RingBuffer_PopMult(&ccmBuffer, &buffer,CCM_DEFAULT_BUFFER_SIZE);
	TEST_ASSERT_EQUAL('T', buffer[0]);
	TEST_ASSERT_EQUAL('E', buffer[1]);
	TEST_ASSERT_EQUAL('S', buffer[2]);
	TEST_ASSERT_EQUAL('T', buffer[3]);
	TEST_ASSERT_EQUAL('E', buffer[4]);
	TEST_ASSERT_EQUAL('L', buffer[5]);
	TEST_ASSERT_EQUAL('A', buffer[6]);
	TEST_ASSERT_EQUAL('R', buffer[7]);
	TEST_ASSERT_EQUAL('G', buffer[8]);
	TEST_ASSERT_EQUAL('E', buffer[9]);

	fpCCMUserCallBack = thisCallback;
	CCM_vthisCANCallBack(eError, sCANm);
}

TEST(CBACOMMDeviceTest, test_Verify_thisUARTCallback)
{
	uint8_t buffer[10];
	uint8_t buffer2[10];

	buffer2[0] = 'T';
	buffer2[1] = 'E';
	buffer2[2] = 'S';
	buffer2[3] = 'T';
	buffer2[4] = 'E';

	void thisCallback(void *message, uint32_t data)
	{
		uint32_t dataSize = 0;
		dataSize = data;
		TEST_ASSERT_EQUAL(5, dataSize);
		TEST_ASSERT_EQUAL('T', *(uint8_t*)message);
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('E', *((uint8_t*)message + 1));
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('S', *((uint8_t*)message + 2));
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('T', *((uint8_t*)message + 3));
		RingBuffer_Pop(&ccmBuffer, &buffer);
		TEST_ASSERT_EQUAL('E', *((uint8_t*)message + 4));
	}

	fpCCMUserCallBack = NULL;
	RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
	RingBuffer_Flush(&ccmBuffer);

	CCM_vthisUARTCallBack((uint8_t*)&buffer2, (uint32_t)5);
	TEST_ASSERT_EQUAL(5, RingBuffer_GetCount(&ccmBuffer));
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('T', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('E', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('S', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('T', buffer[0]);
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('E', buffer[0]);

	fpCCMUserCallBack = thisCallback;
	CCM_vthisUARTCallBack((uint8_t*)&buffer2, 5);
	RingBuffer_Flush(&ccmBuffer);

	fpCCMUserCallBack = NULL;
	CCM_vthisUARTCallBack((uint8_t*)&buffer2, 1);
	TEST_ASSERT_EQUAL(1, RingBuffer_GetCount(&ccmBuffer));
	RingBuffer_Pop(&ccmBuffer, &buffer);
	TEST_ASSERT_EQUAL('T', buffer[0]);
}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_changeCallback)
{
	fpCCMUserCallBack = NULL;
	void thisCallback(void *message, uint32_t data)
	{}
	TEST_ASSERT_EQUAL_PTR(NULL, fpCCMUserCallBack);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLChangeCallBack(IOCTL_CCM_CHANGE_CALLBACK, thisCallback));
	TEST_ASSERT_EQUAL_PTR(thisCallback, fpCCMUserCallBack);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLChangeCallBack(IOCTL_CCM_CHANGE_CALLBACK, NULL));
	TEST_ASSERT_EQUAL_PTR(NULL, fpCCMUserCallBack);
}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_AddID)
{
	sCBACOMM.type = CCM_INTERFACE_UART;
	uint16_t data = 100;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, CCM_eIOCTLAddID(IOCTL_CCM_CAN_ADD_RECEIVE_ID, &data));
	sCBACOMM.type = CCM_INTERFACE_USB;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, CCM_eIOCTLAddID(IOCTL_CCM_CAN_ADD_RECEIVE_ID, &data));
	sCBACOMM.type = CCM_INTERFACE_INVALID;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, CCM_eIOCTLAddID(IOCTL_CCM_CAN_ADD_RECEIVE_ID, &data));
	sCBACOMM.type = CCM_INTERFACE_CAN;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLAddID(IOCTL_CCM_CAN_ADD_RECEIVE_ID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLAddID(IOCTL_CCM_CAN_ADD_RECEIVE_ID, &data));
}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_ChangeSpeed)
{
	IOCTL_CCM_REQUEST_e eRequest = CCM_DEFAULT_UART_BAUD;
	CCM_open();
	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN_BITRATE, sCBACOMM.sCAN.eCANBitrate);
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLChangeSpeed(IOCTL_CCM_CHANGE_CAN_SPEED, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLChangeSpeed(IOCTL_CCM_CHANGE_UART_SPEED, &eRequest));
	canBitrate_s bit = CAN_BITRATE_INVALID;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLChangeSpeed(IOCTL_CCM_CHANGE_CAN_SPEED, &bit));
	bit = CAN_BITRATE_1000;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLChangeSpeed(IOCTL_CCM_CHANGE_CAN_SPEED, &bit));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN, sCBACOMM.sCAN.eCANPort);
	TEST_ASSERT_EQUAL(CAN_BITRATE_1000, sCBACOMM.sCAN.eCANBitrate);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisCANCallBack, sCBACOMM.sCAN.fpCallback);

	sCBACOMM.type = CCM_INTERFACE_UART;
	uart_baudrate_e baud = UART_BINVALID;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLChangeSpeed(IOCTL_CCM_CHANGE_UART_SPEED, &baud));
	baud = UART_B1000000;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLChangeSpeed(IOCTL_CCM_CHANGE_UART_SPEED, &baud));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART, sCBACOMM.sUART.eChannel);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_WORD, sCBACOMM.sUART.eWordLenght);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_STOP, sCBACOMM.sUART.eStopbits);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_PARITY, sCBACOMM.sUART.eParity);
	TEST_ASSERT_EQUAL(baud, sCBACOMM.sUART.eBaudrate);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisUARTCallBack, sCBACOMM.sUART.fpCallBack);

}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_ChangeInterface)
{
	CCM_open();
	TEST_ASSERT_EQUAL(CCM_INTERFACE_CAN ,sCBACOMM.type);
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLChangeInterface(IOCTL_CCM_CHANGE_INTERFACE, NULL));
	uint8_t bInterface = CCM_INTERFACE_INVALID;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLChangeInterface(IOCTL_CCM_CHANGE_INTERFACE, &bInterface));
	bInterface = CCM_INTERFACE_USB;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLChangeInterface(IOCTL_CCM_CHANGE_INTERFACE, &bInterface));
	TEST_ASSERT_EQUAL(CCM_INTERFACE_USB ,sCBACOMM.type);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_USB_CLASS ,sCBACOMM.sUSB.eClass);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_USB ,sCBACOMM.sUSB.eUSB);

	bInterface = CCM_INTERFACE_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLChangeInterface(IOCTL_CCM_CHANGE_INTERFACE, &bInterface));
	TEST_ASSERT_EQUAL(CCM_INTERFACE_UART ,sCBACOMM.type);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART, sCBACOMM.sUART.eChannel);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_WORD, sCBACOMM.sUART.eWordLenght);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_STOP, sCBACOMM.sUART.eStopbits);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_PARITY, sCBACOMM.sUART.eParity);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_BAUD, sCBACOMM.sUART.eBaudrate);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisUARTCallBack, sCBACOMM.sUART.fpCallBack);

}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_RemoveID)
{
	uint16_t data = 10;
	sCBACOMM.type = CCM_INTERFACE_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, CCM_eIOCTLRemoveID(IOCTL_CCM_CAN_REMOVE_RECEIVE_ID, &data));
	sCBACOMM.type = CCM_INTERFACE_CAN;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLRemoveID(IOCTL_CCM_CAN_REMOVE_RECEIVE_ID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLRemoveID(IOCTL_CCM_CAN_REMOVE_RECEIVE_ID, &data));

}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_sendID)
{
	wCANSendID = 10;
	uint32_t data = 30;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLSendID(IOCTL_CCM_CAN_NEXT_MESSAGE_ID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLSendID(IOCTL_CCM_CAN_NEXT_MESSAGE_ID, &data));
	TEST_ASSERT_EQUAL(data, wCANSendID);
}

TEST(CBACOMMDeviceTest, test_Verify_IOCTL_Gets)
{
	CCM_eInternalOpen();

	uint32_t * data = NULL;
	//Test Wrong Invocation
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_eIOCTLGets(IOCTL_CCM_CAN_NEXT_MESSAGE_ID, data));
	uint32_t wResult = 0;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLGets(IOCTL_CCM_GET_INTERFACE, &wResult));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_INTERFACE, wResult);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eIOCTLGets(IOCTL_CCM_GET_SPEED, &wResult));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN_BITRATE, wResult);
}

TEST(CBACOMMDeviceTest, test_Verify_InternalOpen)
{
	sCBACOMM.type = CCM_INTERFACE_INVALID;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_PERIPHERAL, CCM_eInternalOpen());

	sCBACOMM.type = CCM_INTERFACE_CAN;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eInternalOpen());

	TEST_ASSERT_EQUAL(true, RingBuffer_IsEmpty(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetFree(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetSize(&ccmBuffer));
	TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&ccmBuffer));

	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN, sCBACOMM.sCAN.eCANPort);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN_BITRATE, sCBACOMM.sCAN.eCANBitrate);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisCANCallBack, sCBACOMM.sCAN.fpCallback);

	sCBACOMM.type = CCM_INTERFACE_USB;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eInternalOpen());

	TEST_ASSERT_EQUAL(true, RingBuffer_IsEmpty(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetFree(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetSize(&ccmBuffer));
	TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&ccmBuffer));

	TEST_ASSERT_EQUAL(CCM_DEFAULT_USB, sCBACOMM.sUSB.eUSB);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_USB_CLASS, sCBACOMM.sUSB.eClass);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisUSBCallBack, sCBACOMM.sUSB.fpUSBCallBack);

	sCBACOMM.type = CCM_INTERFACE_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_eInternalOpen());

	TEST_ASSERT_EQUAL(true, RingBuffer_IsEmpty(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetFree(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetSize(&ccmBuffer));
	TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&ccmBuffer));

	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART, sCBACOMM.sUART.eChannel);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_WORD, sCBACOMM.sUART.eWordLenght);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_STOP, sCBACOMM.sUART.eStopbits);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_PARITY, sCBACOMM.sUART.eParity);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_UART_BAUD, sCBACOMM.sUART.eBaudrate);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisUARTCallBack, sCBACOMM.sUART.fpCallBack);
}

TEST(CBACOMMDeviceTest, test_Verify_CCM_open)
{
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_open());
	TEST_ASSERT_EQUAL(CCM_DEFAULT_INTERFACE, sCBACOMM.type);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN, sCBACOMM.sCAN.eCANPort);
	TEST_ASSERT_EQUAL(CCM_DEFAULT_CAN_BITRATE, sCBACOMM.sCAN.eCANBitrate);
	TEST_ASSERT_EQUAL_PTR(CCM_vthisCANCallBack, sCBACOMM.sCAN.fpCallback);

	TEST_ASSERT_EQUAL(true, RingBuffer_IsEmpty(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetFree(&ccmBuffer));
	TEST_ASSERT_EQUAL(CCM_DEFAULT_BUFFER_SIZE, RingBuffer_GetSize(&ccmBuffer));
	TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&ccmBuffer));
}

TEST(CBACOMMDeviceTest, test_Verify_CCM_read)
{
	//Default error conditions
	uint8_t data;
	TEST_ASSERT_EQUAL(0, CCM_read(NULL, NULL, 0));
	TEST_ASSERT_EQUAL(0, CCM_read(NULL, NULL, 10));
	TEST_ASSERT_EQUAL(0, CCM_read(NULL, &data, 1));

	//Prepare Buffer
	fpCCMUserCallBack = NULL;
	RingBuffer_Init(&ccmBuffer, bBufferArray, sizeof(uint8_t), CCM_DEFAULT_BUFFER_SIZE);
	RingBuffer_Flush(&ccmBuffer);

	//Check Buffer
	CCM_vthisUSBCallBack();
	TEST_ASSERT_EQUAL(5, RingBuffer_GetCount(&ccmBuffer));

	//Read Buffer
	uint8_t buffer[10];
	TEST_ASSERT_EQUAL(5, CCM_read(NULL, &buffer, 10));

	//Pass a small buffer, then a large buffer and then read again
	CCM_vthisUSBCallBack();
	TEST_ASSERT_EQUAL(5, RingBuffer_GetCount(&ccmBuffer));
	TEST_ASSERT_EQUAL(0, CCM_read(NULL, &buffer, 3));
	TEST_ASSERT_EQUAL(5, CCM_read(NULL, &buffer, 10));
	TEST_ASSERT_EQUAL(0, CCM_read(NULL, &buffer, 10));

	//Assign a callback, should return 0
	void thisCallback(void *message, uint32_t data)
	{}
	CCM_vthisUSBCallBack();
	fpCCMUserCallBack = thisCallback;
	TEST_ASSERT_EQUAL(0, CCM_read(NULL, &buffer, 10));
}

TEST(CBACOMMDeviceTest, test_Verify_CCM_write)
{
	//Basic wrong conditions
	uint8_t data = 1;
	TEST_ASSERT_EQUAL(0, CCM_write(NULL, NULL, 0));
	TEST_ASSERT_EQUAL(0, CCM_write(NULL, NULL, 10));
	TEST_ASSERT_EQUAL(0, CCM_write(NULL, &data, 0));

	//Test 10Byte write
	wCANSendID = 0;
	sCBACOMM.type = CCM_INTERFACE_USB;
	TEST_ASSERT_EQUAL(10, CCM_write(NULL, &data, 10));

	sCBACOMM.type = CCM_INTERFACE_UART;
	TEST_ASSERT_EQUAL(10, CCM_write(NULL, &data, 10));

	sCBACOMM.type = CCM_INTERFACE_CAN;
	TEST_ASSERT_EQUAL(0, CCM_write(NULL, &data, 10));

	//Test Buffers lower and larger than 8
	uint8_t data2[] =
	{	'T', 'E', 'S', 'T', 'E'};
	canMSGStruct_s sUnityResult =
	{
		.id = 100,
		.dlc = 6,
		.data =
		{	0xBB, 'T', 'E', 'S', 'T', 'E', 0, 0},
	};
	wCANSendID = 100;
	TEST_ASSERT_EQUAL(5, CCM_write(NULL, &data2, 5));
	TEST_ASSERT_EQUAL(sUnityResult.id, sUnityMessage.id);
	TEST_ASSERT_EQUAL(sUnityResult.dlc, sUnityMessage.dlc);
	TEST_ASSERT_EQUAL(sUnityResult.data[0], sUnityMessage.data[0]);
	TEST_ASSERT_EQUAL(sUnityResult.data[1], sUnityMessage.data[1]);
	TEST_ASSERT_EQUAL(sUnityResult.data[2], sUnityMessage.data[2]);
	TEST_ASSERT_EQUAL(sUnityResult.data[3], sUnityMessage.data[3]);
	TEST_ASSERT_EQUAL(sUnityResult.data[4], sUnityMessage.data[4]);
	TEST_ASSERT_EQUAL(sUnityResult.data[5], sUnityMessage.data[5]);

	sUnityResult.dlc = 4;
	sUnityResult.data[0] = 0xBB;
	sUnityResult.data[1] = 'R';
	sUnityResult.data[2] = 'G';
	sUnityResult.data[3] = 'E';
	sUnityResult.data[4] = 0;
	sUnityResult.data[5] = 0;
	sUnityResult.data[6] = 0;
	sUnityResult.data[7] = 0;

	uint8_t data3[] =
	{	'T', 'E', 'S', 'T', 'E', 'L', 'A', 'R', 'G', 'E'};
	TEST_ASSERT_EQUAL(10, CCM_write(NULL, &data3, 10));
	TEST_ASSERT_EQUAL(sUnityResult.id, sUnityMessage.id);
	TEST_ASSERT_EQUAL(sUnityResult.dlc, sUnityMessage.dlc);
	TEST_ASSERT_EQUAL(sUnityResult.data[0], sUnityMessage.data[0]);
	TEST_ASSERT_EQUAL(sUnityResult.data[1], sUnityMessage.data[1]);
	TEST_ASSERT_EQUAL(sUnityResult.data[2], sUnityMessage.data[2]);
	TEST_ASSERT_EQUAL(sUnityResult.data[3], sUnityMessage.data[3]);
	TEST_ASSERT_EQUAL(sUnityResult.data[4], sUnityMessage.data[4]);
	TEST_ASSERT_EQUAL(sUnityResult.data[5], sUnityMessage.data[5]);
	TEST_ASSERT_EQUAL(sUnityResult.data[6], sUnityMessage.data[6]);
	TEST_ASSERT_EQUAL(sUnityResult.data[7], sUnityMessage.data[7]);
}

TEST(CBACOMMDeviceTest, test_Verify_CCM_close)
{
	CCM_open();
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_close(NULL));
}

TEST(CBACOMMDeviceTest, test_Verify_CCM_ioctl)
{
	//Test invalid conditions
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_ioctl(NULL, IOCTL_CCM_INVALID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, CCM_ioctl(NULL, IOCTL_CCM_CAN_NEXT_MESSAGE_ID, NULL));

	//Test each request
	sCBACOMM.type = CCM_INTERFACE_CAN;
	uint16_t hID = 200;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CAN_ADD_RECEIVE_ID, &hID));

	uint32_t hID2 = 300;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CAN_NEXT_MESSAGE_ID, &hID2));
	TEST_ASSERT_EQUAL(hID2, wCANSendID);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CAN_REMOVE_RECEIVE_ID, &hID));

	canBitrate_s eCAN = CAN_BITRATE_250;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CHANGE_CAN_SPEED, &eCAN));
	TEST_ASSERT_EQUAL(eCAN, sCBACOMM.sCAN.eCANBitrate);

	void thisCallback(void *message, uint32_t data)
	{}
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CHANGE_CALLBACK, thisCallback));
	TEST_ASSERT_EQUAL_PTR(thisCallback, fpCCMUserCallBack);

	uint32_t bInterface = CCM_INTERFACE_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CHANGE_INTERFACE, &bInterface));
	TEST_ASSERT_EQUAL(bInterface, sCBACOMM.type);

	uart_baudrate_e eBaud = UART_B9600;
	sCBACOMM.type = CCM_INTERFACE_UART;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CCM_ioctl(NULL, IOCTL_CCM_CHANGE_UART_SPEED, &eBaud));
	TEST_ASSERT_EQUAL(eBaud, sCBACOMM.sUART.eBaudrate);
}

TEST(CBACOMMDeviceTest, test_Verify_DEV_open)
{
	TEST_ASSERT_NULL(DEV_open(PERIPHERAL_INVALID));
	peripheral_descriptor_p CCMDevice = DEV_open(PERIPHERAL_CBACOMM);
	TEST_ASSERT_NOT_NULL(CCMDevice);
	//Try to reopen
	TEST_ASSERT_NULL(DEV_open(PERIPHERAL_CBACOMM));
	DEV_close(CCMDevice);
}

TEST(CBACOMMDeviceTest, test_Verify_DEV_close)
{
	//Try to close NULL
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(NULL));
	peripheral_descriptor_p CCMDevice = DEV_open(PERIPHERAL_CBACOMM);
	TEST_ASSERT_NOT_NULL(CCMDevice);
	//Try to close
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(CCMDevice));
	//Try to re-close
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(CCMDevice));
}

TEST(CBACOMMDeviceTest, test_Verify_DEV_read)
{

	TEST_ASSERT_EQUAL(0, DEV_read(NULL, NULL, 0));
	peripheral_descriptor_p CCMDevice = DEV_open(PERIPHERAL_CBACOMM);
	TEST_ASSERT_NOT_NULL(CCMDevice);

	TEST_ASSERT_EQUAL(0, DEV_read(CCMDevice, NULL, 0));

	//Check Buffer
	eCANStatus_s eError = 0;
	canMSGStruct_s sCANm;
	uint8_t sCANRead[CCM_DEFAULT_BUFFER_SIZE];
	sCANm.id = 10;
	sCANm.dlc = 6;
	sCANm.data[0] = CCM_CAN_MESSAGE_FINAL;
	sCANm.data[1] = 'T';
	sCANm.data[2] = 'E';
	sCANm.data[3] = 'S';
	sCANm.data[4] = 'T';
	sCANm.data[5] = 'E';
	CCM_vthisCANCallBack(eError, sCANm);
	TEST_ASSERT_NULL(fpCCMUserCallBack);
	TEST_ASSERT_EQUAL(5, RingBuffer_GetCount(&ccmBuffer));

	//Read Buffer
	TEST_ASSERT_EQUAL(5, DEV_read(CCMDevice, &sCANRead, CCM_DEFAULT_BUFFER_SIZE));
	TEST_ASSERT_EQUAL(sCANm.data[1], sCANRead[0]);
	TEST_ASSERT_EQUAL(sCANm.data[2], sCANRead[1]);
	TEST_ASSERT_EQUAL(sCANm.data[3], sCANRead[2]);
	TEST_ASSERT_EQUAL(sCANm.data[4], sCANRead[3]);
	TEST_ASSERT_EQUAL(sCANm.data[5], sCANRead[4]);

	TEST_ASSERT_EQUAL(0, DEV_read(CCMDevice, &sCANRead, 1));

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(CCMDevice));
}

TEST(CBACOMMDeviceTest, test_Verify_DEV_write)
{
	//Try to write NULL
	TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
	peripheral_descriptor_p CCMDevice = DEV_open(PERIPHERAL_CBACOMM);
	TEST_ASSERT_NOT_NULL(CCMDevice);

	//Try to write NULL
	TEST_ASSERT_EQUAL(0, DEV_write(CCMDevice, NULL, 0));

	uint8_t data2[] =
	{	'T', 'E', 'S', 'T', 'E'};
	uint8_t data3[] =
	{	'T', 'E', 'S', 'T', 'E', 'L', 'A', 'R', 'G', 'E'};
	uint32_t messageID = 5;
	//Did not set message ID, so it must return 0
	TEST_ASSERT_EQUAL(0, DEV_write(CCMDevice, &data2, 5));
	//Now set ID and try again
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(CCMDevice, IOCTL_CCM_CAN_NEXT_MESSAGE_ID, (void*)&messageID));
	TEST_ASSERT_EQUAL(5, DEV_write(CCMDevice, &data2, 5));
	TEST_ASSERT_EQUAL(10, DEV_write(CCMDevice, &data3, 10));

	//Try to close
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(CCMDevice));
}

TEST(CBACOMMDeviceTest, test_Verify_DEV_ioctl)
{
	//Try to write NULL
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_ioctl(NULL, 0, NULL));
	peripheral_descriptor_p CCMDevice = DEV_open(PERIPHERAL_CBACOMM);
	TEST_ASSERT_NOT_NULL(CCMDevice);
	//Wrong conditions
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(CCMDevice, IOCTL_CCM_INVALID, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(CCMDevice, IOCTL_CCM_CAN_ADD_RECEIVE_ID, NULL));

	//Other conditions
	uint32_t wSpeed = CAN_BITRATE_1000;
	TEST_ASSERT_EQUAL(CAN_BITRATE_500, sCBACOMM.sCAN.eCANBitrate);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(CCMDevice, IOCTL_CCM_CHANGE_CAN_SPEED, (void*)&wSpeed));
	TEST_ASSERT_EQUAL(wSpeed, sCBACOMM.sCAN.eCANBitrate);

	uint32_t wMID = 50;
	TEST_ASSERT_EQUAL(0, wCANSendID);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(CCMDevice, IOCTL_CCM_CAN_NEXT_MESSAGE_ID, (void*)&wMID));
	TEST_ASSERT_EQUAL(wMID, wCANSendID);
	DEV_close(CCMDevice);
}

#endif
