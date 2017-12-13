/****************************************************************************
 * Title                 :   caniso
 * Filename              :   caniso.c
 * Author                :   Henrique Reis
 * Origin Date           :   30/11/2017
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
 * 30/11/2017  1.0.0     Henrique Reis      caniso.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <can.h>
#include "board.h"
#include <string.h>
#ifdef USE_SYSVIEW
#include "SEGGER.h"
#include "SEGGER_SYSVIEW.h"
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
#define CANISO_MAP_IRQ() (sCANMap[CAN1b].CANIntIRQ) //!< CAN IRQ map macro
#define CANISO_MAP_REGISTER() (sCANMap[CAN1b].pCAN) //!< CAN Register map macro

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static bool CANISOInitiated = false; //!< Indicates if a CAN port is initialized
static can_config_s * CANISOCfg = NULL;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
static void CANISO_vTreatInterruptsISR (void)
{
	can_config_s * pCAN = NULL;
	canMSGStruct_s sCANMessage;
	eCANStatus_s eCANStat;

	// If CANlist position has a valid structure
	if (CANISOCfg != NULL)
	{
		pCAN = CANISOCfg;
	}

	if (pCAN != NULL)
	{
		uint32_t wCANint;
		while ((wCANint = Chip_CCAN_GetIntID(CANISO_MAP_REGISTER())) != 0)
		{
			memset(sCANMessage.data, 0x00, 8);
			sCANMessage.dlc = 0;
			sCANMessage.id = 0;

			if (wCANint & CAN_INT_STATUS) //Error or Transmit/Reception OK
			{
				eCANStat = Chip_CCAN_GetStatus(CANISO_MAP_REGISTER());
				pCAN->fpCallback(eCANStat, sCANMessage);
				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_BOFF);
				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_EPASS);
				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_EWARN);
				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_TXOK);
				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_RXOK);
			}
			else if ((1 <= CCAN_INT_MSG_NUM(wCANint)) && (CCAN_INT_MSG_NUM(wCANint) <= 0x20))
			{
				Chip_CCAN_GetMsgObject(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, wCANint,
					(CCAN_MSG_OBJ_T*)&sCANMessage);

				eCANStat = Chip_CCAN_GetStatus(CANISO_MAP_REGISTER());

				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_TXOK);
				Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CAN_STAT_RXOK);
				pCAN->fpCallback(eCANStat, sCANMessage);
			}
		}
	}
}

/******************************************************************************
 * Function : CAN1_IRQHandler(void)
 *//**
 * \b Description:
 *
 * This is a IRQ handling function for the CAN0. The result will be sent via Callback.
 * The IRQ  will read the value and send it through the Callback
 *
 * PRE-CONDITION: CAN Channel must be initialized
 *
 * POST-CONDITION: CAN interrupt sample sent via Callback
 *
 * @return     Void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *  //Not available, IRQ is activated via CAN_vAddMessageID
 ~~~~~~~~~~~~~~~
 *
 * @see CAN_bInit, CAN_vDeInit, CAN_vAddMessageID, CAN_vRemoveMessageID, CAN_vSendMessage
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void CAN1_IRQHandler (void)
{
#ifdef USE_SYSVIEW_ISR
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	CANISO_vTreatInterruptsISR();
#ifdef USE_SYSVIEW_ISR
	SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/******************************************************************************
 * Function : CANISO_eCheckFault(adc_config_s * pCAN)
 *//**
 * \b Description:
 *
 * This private function must be called to verify CAN input data
 *
 * PRE-CONDITION: CAN Data
 *
 * POST-CONDITION: return error
 *
 * @return     eMCUError_s error type
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *  //Not available
 ~~~~~~~~~~~~~~~
 *
 * @see CAN_bInit, CAN_vDeInit, CAN_vAddMessageID, CAN_vRemoveMessageID, CAN_vSendMessage
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static eMCUError_s CANISO_eCheckFault (const can_config_s *pCAN)
{
	if (pCAN->eCANBitrate >= CAN_BITRATE_INVALID)
	{
		return MCU_ERROR_CAN_INVALID_BITRATE;
	}
	if (pCAN->fpCallback == NULL)
	{
		return MCU_ERROR_INVALID_CALLBACK;
	}
	return MCU_ERROR_SUCCESS;
}

eMCUError_s CANISO_eInit (can_config_s *pCAN)
{
	//Verify if it is a valid CAN port, if it is not already initiated, if it is a valid bitrate and if
	//it has a valid message id
	if ((pCAN->eCANPort < CAN_INVALID) && !CANISOInitiated)
	{
		eMCUError_s eErrorcode = CANISO_eCheckFault(pCAN);
		if (eErrorcode)
		{
			return eErrorcode;
		}

		if (BRD_CANConfig(pCAN->eCANPort) > CAN_MAX_CLOCK) //PinMux CAN on Board.c
		{
			return MCU_ERROR_CAN_INVALID_CLOCK;
		}

		CANISOInitiated = true; //Indicate that given CAN is initiated

		//CAN interface preparation
		Chip_CCAN_Init(CANISO_MAP_REGISTER());

		// Chip_CCAN_ConfigTestMode(CANISO_MAP_REGISTER(),CCAN_TEST_TD_DOMINANT);

		if (!Chip_CCAN_SetBitRate(CANISO_MAP_REGISTER(), pCAN->eCANBitrate))
		{
			return MCU_ERROR_CAN_INVALID_BITRATE;
		}

		CANISOCfg = pCAN;

		return MCU_ERROR_SUCCESS;
	}
	return MCU_ERROR_CAN_INVALID_CAN_PORT;
}

void CANISO_vDeInit (can_config_s *pCAN)
{
	NVIC_DisableIRQ(CANISO_MAP_IRQ());
	Chip_CCAN_DeInit(CANISO_MAP_REGISTER());
	CANISOInitiated = false; //Indicate that given CAN is not initiated
	CANISOCfg = NULL;
}

void CANISO_vAddMessageID (const can_config_s *pCAN, const uint16_t hCANmsgID)
{
	if ((pCAN->eCANPort < CAN_INVALID) && CANISOInitiated) //Valid and initiated
	{
		//Enable Interrupt
		Chip_CCAN_EnableInt(CANISO_MAP_REGISTER(), (CCAN_CTRL_IE | CCAN_CTRL_EIE));
		Chip_CCAN_AddReceiveID(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, hCANmsgID);
		//Enable Interrupt
		NVIC_EnableIRQ(CANISO_MAP_IRQ());
	}
}

STATIC uint8_t CANISO_getFreeMsgObject (LPC_CCAN_T *pCCAN)
{
	/* Return 1->32; 0 if not find free msg */
	uint32_t msg_valid;
	uint8_t i;
	msg_valid = Chip_CCAN_GetValidMsg(pCCAN);
	for (i = 0; i < CCAN_MSG_MAX_NUM; i++)
	{
		if (!((msg_valid >> i) & 1UL))
		{
			return i + 1;
		}
	}
	return 0;	// No free object
}

STATIC void CANISO_freeMsgObject (LPC_CCAN_T *pCCAN, CCAN_MSG_IF_T IFSel, uint8_t msgNum)
{
	Chip_CCAN_SetValidMsg(pCCAN, IFSel, msgNum, false);
}

void CANISO_SetMsgObject_ReceiveAll_ID (LPC_CCAN_T *pCCAN,
	CCAN_MSG_IF_T IFSel,
	CCAN_TRANSFER_DIR_T dir,
	bool remoteFrame,
	uint8_t msgNum,
	const CCAN_MSG_OBJ_T *pMsgObj)
{
	/* Set a message into the message object in message RAM */
	uint16_t *pData;
	uint32_t msgCtrl = 0;

	if (pMsgObj == NULL)
	{
		return;
	}
	pData = (uint16_t *)(pMsgObj->data);

	msgCtrl |= CCAN_IF_MCTRL_UMSK | CCAN_IF_MCTRL_RMTEN(remoteFrame) | CCAN_IF_MCTRL_EOB |
		(pMsgObj->dlc & CCAN_IF_MCTRL_DLC_MSK);

	if (dir == CCAN_TX_DIR)
	{
		if (!remoteFrame)
		{
			msgCtrl |= CCAN_IF_MCTRL_TXRQ;
		}
	}
	else
	{
		msgCtrl |= CCAN_IF_MCTRL_RXIE;
	}

	pCCAN->IF[IFSel].MCTRL = msgCtrl;
	pCCAN->IF[IFSel].DA1 = *pData++; /* Lower two bytes of message pointer */
	pCCAN->IF[IFSel].DA2 = *pData++; /* Upper two bytes of message pointer */
	pCCAN->IF[IFSel].DB1 = *pData++; /* Lower two bytes of message pointer */
	pCCAN->IF[IFSel].DB2 = *pData; /* Upper two bytes of message pointer */

	/* Configure arbitration */
	if (!(pMsgObj->id & (0x1 << 30)))
	{ /* bit 30 is 0, standard frame */
		/* Mxtd: 0, Mdir: 1, Mask is 0x7FF */
		pCCAN->IF[IFSel].MSK2 = CCAN_IF_MASK2_MDIR(dir) | (~CCAN_MSG_ID_STD_MASK << 2);
		pCCAN->IF[IFSel].MSK1 = 0x0000;

		/* MsgVal: 1, Mtd: 0, Dir: 1, ID = 0x200 */
		pCCAN->IF[IFSel].ARB2 = CCAN_IF_ARB2_MSGVAL | CCAN_IF_ARB2_DIR(dir) | (pMsgObj->id << 2);
		pCCAN->IF[IFSel].ARB1 = 0x0000;
	}
	else
	{ /* Extended frame */
		/* Mxtd: 1, Mdir: 1, Mask is 0x1FFFFFFF */
		pCCAN->IF[IFSel].MSK2 = CCAN_IF_MASK2_MXTD | CCAN_IF_MASK2_MDIR(dir) | (~CCAN_MSG_ID_EXT_MASK >> 16);
		pCCAN->IF[IFSel].MSK1 = ~CCAN_MSG_ID_EXT_MASK & 0x0000FFFF;

		/* MsgVal: 1, Mtd: 1, Dir: 1, ID = 0x200000 */
		pCCAN->IF[IFSel].ARB2 = CCAN_IF_ARB2_MSGVAL | CCAN_IF_ARB2_XTD | CCAN_IF_ARB2_DIR(dir) | (pMsgObj->id >> 16);
		pCCAN->IF[IFSel].ARB1 = pMsgObj->id & 0x0000FFFF;
	}
	Chip_CCAN_TransferMsgObject(pCCAN, IFSel, CCAN_IF_CMDMSK_WR | CCAN_IF_CMDMSK_TRANSFER_ALL, msgNum);
}

void CANISO_vAddAllMessageID (can_config_s *pCAN, const uint32_t hCANmsgID)
{
	CCAN_MSG_OBJ_T temp;
	uint8_t msgNum;

	if ((pCAN->eCANPort < CAN_INVALID) && CANISOInitiated) //Valid and initiated
	{
		//Enable Interrupt
		Chip_CCAN_EnableInt(CANISO_MAP_REGISTER(), (CCAN_CTRL_IE | CCAN_CTRL_EIE));

		msgNum = CANISO_getFreeMsgObject(CANISO_MAP_REGISTER());
		if (!msgNum)
		{
			return;
		}
		pCAN->bIDRecvObj = msgNum;
		temp.id = hCANmsgID;
		// Call CCAN_SetMsgObject_ReceiveAll_ID
		CANISO_SetMsgObject_ReceiveAll_ID(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, CCAN_RX_DIR,
			false, msgNum, &temp);

		//Enable Interrupt
		NVIC_EnableIRQ(CANISO_MAP_IRQ());
	}
}

void CANISO_vEnableLoopback (can_config_s *pCAN)
{
	Chip_CCAN_EnableTestMode(CANISO_MAP_REGISTER());
	Chip_CCAN_ConfigTestMode(CANISO_MAP_REGISTER(), CCAN_TEST_LOOPBACK_MODE);
}

void CANISO_vRemoveMessageID (const can_config_s *pCAN, const uint16_t hCANmsgID)
{
	if ((pCAN->eCANPort < CAN_INVALID) && CANISOInitiated) //Valid and initiated
	{
		Chip_CCAN_DeleteReceiveID(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, hCANmsgID);
	}
}

inline void CANISO_vSendMessage (const can_config_s *pCAN, const canMSGStruct_s CANMessage)
{
	if ((pCAN->eCANPort < CAN_INVALID) && CANISOInitiated) //Valid and initiated
	{
		uint8_t msgNum = CANISO_getFreeMsgObject(CANISO_MAP_REGISTER());
		if (!msgNum)
		{
			for( int i=1; i<=CCAN_MSG_MAX_NUM;i++)
			{
				if (i != pCAN->bIDRecvObj)
					CANISO_freeMsgObject(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, i);
			}
		} else
		{
			CANISO_freeMsgObject(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, msgNum);
		}

		if (!(CANMessage.id & CAN_ID_RTR_Msk))
		{
			Chip_CCAN_Send(CANISO_MAP_REGISTER(), CCAN_MSG_IF2, false,
				(CCAN_MSG_OBJ_T *)&CANMessage);
		}
		Chip_CCAN_ClearStatus(CANISO_MAP_REGISTER(), CCAN_STAT_TXOK);
	}
}

uint8_t CANISO_bGetErrCount(const can_config_s *pCAN, can_transfer_id dir)
{
	return Chip_CCAN_GetErrCounter(CANISO_MAP_REGISTER(), dir);
}
