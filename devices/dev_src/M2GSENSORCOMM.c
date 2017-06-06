/****************************************************************************
 * Title                 :   M2GSENSORCOMM
 * Filename              :   M2GSENSORCOMM.c
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
 *  XX/XX/XX   1.0.0    Henrique Reis    M2GSENSORCOMM.c created.
 *
 *****************************************************************************/
/** @file   M2GSENSORCOMM.c
 *  @brief
 *
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include <string.h>
#include <M2GSENSORCOMM_config.h>
#include <M2GSENSORCOMM.h>
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

/* Module Preprocessor Macros */
#ifndef NULL
#define NULL (void*)0
#endif

#define RECV_SENSORBUFSIZE		32

#ifndef UNITY_TEST
#define DEFAULT_TIMEOUT		500
#else
#define DEFAULT_TIMEOUT		2
#endif

/*
 * @brief M2GSENSORCOMM device status variable
 * The status is a word with each bit means a status/option
 * */
#define M2GSENSORCOMM_STATUS_ENABLED	0x00000001 /*!< Status flag - bit 0 - Device instance enabled */
#define M2GSENSORCOMM_STATUS_DISABLED	0x00000000 /*!< Status flag - bit 0 - Device instance enabled */
#define M2GSENSORCOMM_STATUS_BUSY		0x00000002 /*!< Status flag - bit 1 - Device instance busy */
#define M2GSENSORCOMM_STATUS_WR_ERROR	0x00000004 /*!< Status flag - bit 2 - Device instance write op error */
#define M2GSENSORCOMM_STATUS_RD_ERROR 	0x00000008 /*!< Status flag - bit 3 - Device instance read op error */

#define M2GSENSORCOMM_STATUS_USE_MASK	0x00000001 /*!< Status flag - Perihperals usage mask */
#define M2GSENSORCOMM_STATUS_USE_CAN	0x00000001 /*!< Status flag - CAN reserved for use */

#define _IS_INTERFACES_NOT_VALID(__HANDLE__) \
(!((__HANDLE__).dReservedInterfaces & M2GSENSORCOMM_STATUS_USE_MASK))	/*!< Macro to check if reserved interfaces are valid */

#define _IS_M2GSENSORCOMM_USING_CAN(__HANDLE__) \
((__HANDLE__).dReservedInterfaces & M2GSENSORCOMM_STATUS_USE_CAN)	/*!< Macro to check if CAN is enabled */

#define _IS_M2GSENSORCOMM_VALID_IFACE(__IFACE__) \
(__IFACE__ == M2GSENSORCOMM_CAN)					/*!< Macro to check if any interface is activated */

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
typedef eDEVError_s (*fpIOCTLFunction) (uint32_t wRequest, void * vpValue); /*!< IOCTL function pointer */

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

/* Ring buffer structures */
static canMSGStruct_s sRecvBuffer[RECV_SENSORBUFSIZE];  		//!< RingBuffer array
static RINGBUFF_T rbM2GSENHandle;             	                //!< RingBuffer Control handle

/*
 * @brief Private handler used to access CAN peripheral under MCULIB layer
 * */
static can_config_s sMCU_CAN_Handle =
	{
		.eCANPort = M2GSENSORCOMM_CAN_CHANNEL,
		.eCANBitrate = M2GSENSORCOMM_CAN_BITRATE,
		.fpCallback = NULL,
		.vpPrivateData = NULL
	};

/*
 * @brief Private handler used to manage the TERMDEV instance
 * */
static M2GSENSORCOMM_Handle_s M2GSENSORCOMM_Handle =
	{
		.dReservedInterfaces = 0,
		.eActiveInterface = M2GSENSORCOMM_INVALID,
		.bDeviceStatus = M2GSENSORCOMM_STATUS_DISABLED,
	};

static uint32_t wCANSendID = 0; /* !< Holds the current output CAN message ID */

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void M2GSEN_vRBSafeInsert (canMSGStruct_s *psData);
static uint32_t M2GSEN_wReadBufferProcedure (canMSGStruct_s *psOutput, uint32_t dBufferSize);
static eDEVError_s M2GSEN_eSetActive (uint32_t wRequest, void * vpValue);
static eDEVError_s M2GSEN_eDisable (uint32_t wRequest, void * vpValue);
static eDEVError_s M2GSEN_eCANAddID (uint32_t wRequest, void * vpValue);
static eDEVError_s M2GSEN_eCANAddAllID (uint32_t wRequest, void * vpValue);
static eDEVError_s M2GSEN_eCANChangeSendID (uint32_t wRequest, void * vpValue);
static void M2GSEN_CANCallback (eCANStatus_s eErrorCode, canMSGStruct_s CANMessage);

#define X(a, b) b,
fpIOCTLFunction M2GSEN_pIOCTLFunction[] = //!< IOCTL array of function mapping
	{
	IOCTL_M2GSENSORCOMM
	};
#undef X

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
/******************************************************************************
 * Function : M2GSEN_vRBSafeInsert(uint8_t *pbData, uint32_t wNumItens)
 *//**
 * \b Description:
 *
 * This function is just a procedure to check if the module's global ring buffer (rbM2GISOHandle)
 * is full, if yes, it will overwrite the oldest items in order to insert the new data.
 *
 * PRE-CONDITION: The global ring buffer (rbM2GSENHandle) has to be already enabled.
 *
 * POST-CONDITION: Data items saved in ring buffer.
 *
 * @return nothing
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint8_t abTest[] = ['t','e','s','t'];
 * M2GSEN_vRBSafeInsert(abTest, 4);
 ~~~~~~~~~~~~~~~
 *
 * @see rbM2GSENHandle
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 * <hr>
 *
 *******************************************************************************/
void M2GSEN_vRBSafeInsert (canMSGStruct_s *psData)
{
	/* Fill the ringbuffer with bytes */
	if (!RingBuffer_Insert(&rbM2GSENHandle, psData)) //RingBuffer Full
	{
		//Pop from tail and then insert into head
		canMSGStruct_s asGarbage;
		RingBuffer_Pop(&rbM2GSENHandle, &asGarbage);
		RingBuffer_Insert(&rbM2GSENHandle, psData);
	}
}

/******************************************************************************
 * Function : M2GSEN_wReadBufferProcedure(uint8_t *pbOutput)
 *//**
 * \b Description:
 *
 * This function is called whenever a M2GSEN_read call occurs. It will check if the ring buffer
 * has new data and will copy this data to the destination area. By doing this, the TDV_read
 * procedure performs a non-blocking reading operation. If using CAN and the function is called
 * in the middle of a multi-packet transfer, it will return zero until all packets are received.
 *
 * PRE-CONDITION: The global ring buffer (rbM2GSENHandle) has to be already enabled.
 *
 * POST-CONDITION: Valid items of the ring buffer will be transfered to pbOutput.
 *
 * @return nothing
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint8_t buffer[256];
 * uint8_t bBytesReceived = M2GSEN_wReadBufferProcedure(buffer);
 ~~~~~~~~~~~~~~~
 *
 * @see M2GSEN_read
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 * <hr>
 *
 *******************************************************************************/
static uint32_t M2GSEN_wReadBufferProcedure (canMSGStruct_s *psOutput, uint32_t dBufferSize)
{
	uint32_t wReturn;
	/* Wait until ISR callbacks are completed */
	if (M2GSENSORCOMM_Handle.bDeviceStatus == M2GSENSORCOMM_STATUS_BUSY)
	{
		return 0;
	}

	wReturn = RingBuffer_PopMult(&rbM2GSENHandle, psOutput, dBufferSize);

	return wReturn;
}

/******************************************************************************
 * Function : M2GSEN_CANCallback(eCANStatus_s eErrorCode, canMSGStruct_s CANMessage)
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
 * @see M2GSEN_open, M2GSEN_write, M2GSEN_read, M2GSEN_ioctl, M2GSEN_close and M2GSEN_eCANAddID
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 * <hr>
 *
 *******************************************************************************/
void M2GSEN_CANCallback (eCANStatus_s eErrorCode, canMSGStruct_s CANMessage)
{
	if ((CANMessage.dlc < 1) || (CANMessage.dlc > 8))
		return;

	/* Ringbuffer implementation */
	M2GSENSORCOMM_Handle.bDeviceStatus = M2GSENSORCOMM_STATUS_BUSY;
	/* Insert message to RB */
	M2GSEN_vRBSafeInsert(&CANMessage);
	/* Signals that the handler is not busy anymore */
	M2GSENSORCOMM_Handle.bDeviceStatus = M2GSENSORCOMM_STATUS_ENABLED;
}

/******************************************************************************
 * Function : M2GSEN_eSetActive(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request.
 * This function will set the active input/output interface of the TERMDEV device, that must be UART or
 * USB or CAN. If the interface is not enabled, this routine will call the init procedure using the
 * MCULIB layer. The interface will remain enabled until the disable function is called.
 *
 * When setting an interface as active, any call on M2GSEN_read and M2GSEN_write will use the active interface.
 *
 * PRE-CONDITION: M2GSENCOMM initiated and avaliable interface
 *
 * POST-CONDITION: None
 *
 * @return eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wActiveInterface = TERMDEV_IFACE_UART;
 * M2GSEN_eSetActive(IOCTL_M2GSENSORCOMM_SET_ACTIVE, &wActiveInterface);
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
eDEVError_s M2GSEN_eSetActive (uint32_t wRequest, void * vpValue)
{
	eDEVError_s eInitErr;

	/* Validation */
	if (vpValue == NULL)
	{
		return DEV_ERROR_TERMDEV_SETTING_ERROR;
	}

	uint32_t wActiveIface = *(uint32_t*)vpValue;
	if (!(_IS_M2GSENSORCOMM_VALID_IFACE(wActiveIface)))
	{
		return DEV_ERROR_INVALID_PERIPHERAL;
	}

	switch (wActiveIface)
	{
		case M2GSENSORCOMM_CAN:
		if (!(_IS_M2GSENSORCOMM_USING_CAN(M2GSENSORCOMM_Handle)))
		{
			// Initializes CAN peripheral
			sMCU_CAN_Handle.fpCallback = M2GSEN_CANCallback; //Set the callback
			eInitErr = (eDEVError_s)CAN_eInit(&sMCU_CAN_Handle);
			if (eInitErr != DEV_ERROR_SUCCESS)
			{
				/* Return if error */
				return eInitErr;
			}
			wCANSendID = 0;
			M2GSENSORCOMM_Handle.dReservedInterfaces |= M2GSENSORCOMM_CAN;
		}
		/* CAN is already enabled, set as active */
		M2GSENSORCOMM_Handle.eActiveInterface = (M2GSENSORCOMM_interface_e)wActiveIface;
			break;
		default:
			break;
	}
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : M2GSEN_eDisable(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request.
 * This function will perform the MCULIB DeInit routine of the requested interface (UART, CAN
 * or USB). After the de-initialization, the interface is disabled and can be used by other modules
 * of the firmware.
 *
 * PRE-CONDITION: M2GSENSORCOMM initiated and avaliable interface
 *
 * POST-CONDITION: Selected interface disabled.
 *
 * @return eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wInterface = M2GSENSORCOMM_IFACE_UART;
 * M2GSEN_eDisable(IOCTL_M2GSENSORCOMM_DISABLE, &wInterface);
 ~~~~~~~~~~~~~~~
 *
 * @see M2GSEN_open, M2GSEN_write, M2GSEN_read, M2GSEN_ioctl, M2GSEN_close
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
eDEVError_s M2GSEN_eDisable (uint32_t wRequest, void * vpValue)
{
	/* Validation */
	if (vpValue == NULL)
	{
		return DEV_ERROR_TERMDEV_SETTING_ERROR;
	}

	uint32_t wActiveIface = *(uint32_t*)vpValue;
	if (!(_IS_M2GSENSORCOMM_VALID_IFACE(wActiveIface)))
	{
		return DEV_ERROR_INVALID_PERIPHERAL;
	}

	if ((_IS_M2GSENSORCOMM_USING_CAN(M2GSENSORCOMM_Handle)) && (wActiveIface == M2GSENSORCOMM_CAN))
	{
		/* Close CAN on MCULIB */
		CAN_vDeInit(&sMCU_CAN_Handle);
		M2GSENSORCOMM_Handle.dReservedInterfaces &= ~M2GSENSORCOMM_CAN;
		return DEV_ERROR_SUCCESS;
	}

	return DEV_ERROR_INVALID_PERIPHERAL;
}

/******************************************************************************
 * Function : M2GSEN_eCANAddID(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request.
 * This function will enable the CAN peripheral to receive messages with the requested ID number. It also
 * saves the ID number as the current ID of the outgoing CAN messages.
 *
 * PRE-CONDITION: M2GSENSOR initiated and CAN interface enabled.
 *
 * POST-CONDITION: Any CAN output message will have the ID number passed at the vpValue argument.
 *
 * @return eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wNewCANID = 0x515;
 * M2GSEN_eCANAddID(IOCTL_M2GSENSORCOMM_ADD_CAN_ID, &wNewCANID);
 ~~~~~~~~~~~~~~~
 *
 * @see M2GSEN_open, M2GSEN_write, M2GSEN_read, M2GSEN_ioctl, M2GSEN_close and M2GSEN_eSetActive
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 * <hr>
 *
 *******************************************************************************/
static eDEVError_s M2GSEN_eCANAddID (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	CAN_vAddMessageID(&sMCU_CAN_Handle, *(uint16_t*)vpValue);
	wCANSendID = *(uint32_t*)vpValue;
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s M2GSEN_eCANAddAllID (uint32_t wRequest, void * vpValue)
{
	(void)vpValue;
	uint32_t wRecvAllID = 0x00000000 | CAN_ID_IDE_Msk;

	CAN_vAddAllMessageID(&sMCU_CAN_Handle, wRecvAllID);

	return DEV_ERROR_SUCCESS;
}

static eDEVError_s M2GSEN_eCANChangeSendID (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	wCANSendID = *(uint32_t*)vpValue;
	return DEV_ERROR_SUCCESS;
}

/* ************************************************
 *	Public functions 
 ** ************************************************ */
eDEVError_s M2GSEN_open (void)
{
	/* M2GISOCOMM starts disabled */
	M2GSENSORCOMM_Handle.bDeviceStatus = M2GSENSORCOMM_STATUS_ENABLED;
	uint32_t wDefaultActive = M2GSENSORCOMM_ACTIVE_INTERFACE; // default interface defined in _config.h

	/* Init the receive buffer */
	RingBuffer_Init(&rbM2GSENHandle, sRecvBuffer, sizeof(canMSGStruct_s), RECV_SENSORBUFSIZE);
	RingBuffer_Flush(&rbM2GSENHandle);

	/* Try to init the default interface */
	return M2GSEN_eSetActive(IOCTL_M2GSENSORCOMM_SET_ACTIVE, &wDefaultActive);
}

uint32_t M2GSEN_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	canMSGStruct_s *psAuxPointer = (canMSGStruct_s*)vpBuffer;

	/* The received data is on the ring buffer */
	return M2GSEN_wReadBufferProcedure(psAuxPointer, tBufferSize);
}

uint32_t M2GSEN_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	uint32_t wSentBytes = 0;
	static canMSGStruct_s sCANMessage;
	uint8_t bIterator = 0;
	uint8_t *pbAuxPointer;

	/* Check if data is corrupted */
	if ((vpBuffer == NULL) || ((tBufferSize == 0) && !(wCANSendID & CAN_ID_RTR_Msk)))  //Wrong inputs
	{
		return 0;
	}
	else if (wCANSendID & CAN_ID_RTR_Msk)
	{
		sCANMessage.id = wCANSendID | CAN_ID_IDE_Msk;
		sCANMessage.dlc = 0;
		/* Send CAN frame */
		CAN_vSendMessage(&sMCU_CAN_Handle, sCANMessage);
		return 0;
	}

	pbAuxPointer = (uint8_t*)vpBuffer;

	/* Basic protocol to send more than 8 bytes over CAN */
	if ((M2GSENSORCOMM_Handle.eActiveInterface == M2GSENSORCOMM_CAN) && (wCANSendID != 0)
		&& (M2GSENSORCOMM_Handle.dReservedInterfaces & M2GSENSORCOMM_CAN))
	{
		sCANMessage.id = wCANSendID | CAN_ID_IDE_Msk;

		/* Loop until all bytes have been sent */
		while (wSentBytes < tBufferSize)
		{
			uint32_t dBytesToSend = (tBufferSize - wSentBytes);
			if (dBytesToSend <= 8)
			{
				sCANMessage.dlc = dBytesToSend;

				for (bIterator = 0; bIterator < dBytesToSend; bIterator++)
				{
					sCANMessage.data[bIterator] = *(pbAuxPointer++);
				}
				wSentBytes += dBytesToSend;
			}
			else
			{
				for (bIterator = 0; bIterator < 8; bIterator++)
				{
					sCANMessage.data[bIterator] = *(pbAuxPointer++);
				}
				sCANMessage.dlc = 8;
				wSentBytes += 8;
			}
			/* Send CAN frame */
			CAN_vSendMessage(&sMCU_CAN_Handle, sCANMessage);
		}
	}
	return wSentBytes;
}

eDEVError_s M2GSEN_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if (wRequest >= IOCTL_M2GSENSORCOMM_INVALID)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	/* Call the request related function */
	return M2GSEN_pIOCTLFunction[wRequest](wRequest, vpValue);
}

eDEVError_s M2GSEN_close (struct peripheral_descriptor_s* const this)
{
	if (_IS_M2GSENSORCOMM_USING_CAN(M2GSENSORCOMM_Handle))
	{
		/* Close CAN on MCULIB */
		CAN_vDeInit(&sMCU_CAN_Handle);
		M2GSENSORCOMM_Handle.dReservedInterfaces &= ~M2GSENSORCOMM_CAN;
	}
	return DEV_ERROR_SUCCESS;
}
