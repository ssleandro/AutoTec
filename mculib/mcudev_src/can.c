/****************************************************************************
 * Title                 :   CAN HAL
 * Filename              :   can.c
 * Author                :   Thiago Palmieri
 * Origin Date           :   05/02/2016
 * Version               :   1.0.0
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
 *    Date    Version   Author         Description
 *  05/02/16   1.0.0   Thiago Palmieri CAN HAL Created.
 *
 *****************************************************************************/
/** @file can.c
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the source file for the definition of CAN functions
 */

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

#define CAN_MAP_IRQ(x) (sCANMap[x ? CAN1b : CAN0b].CANIntIRQ) //!< CAN IRQ map macro
#define CAN_MAP_REGISTER(x) (sCANMap[x ? CAN1b : CAN0b].pCAN) //!< CAN Register map macro

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
#define CAN_MAX_CHANNELS (CAN_INVALID) //!< Maximum number of CAN channels
#define CAN_MAX_CLOCK 100000000  //!< Maximum clock supported on CAN

#define CAN_STAT_LEC_MASK  (0x07)    //!< Mask for Last Error Code
#define CAN_INT_STATUS (0x8000)      //!< CAN Status Macro

#ifndef NULL
#define NULL (void*)0
#endif
/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
/**
 * This Struct holds private CAN configuration
 */
/*typedef struct can_private_config_s
{

}can_private_config_s;*/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

static bool CANInitiated[2] = {false, false}; //!< Indicates if a CAN port is initialized

static CCAN_MSG_IF_T sCANInterface[2] = { CCAN_MSG_IF1, CCAN_MSG_IF2 }; //!< Indicates which CAN interface to use

static can_config_s * CANlist[CAN_MAX_CHANNELS] = { NULL, NULL };  //!< List of channels initialized
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
extern void CAN0_IRQHandler(void);
extern void CAN1_IRQHandler(void);
/******************************************************************************
 * Function Definitions
 *******************************************************************************/
/******************************************************************************
 * Function : CAN_vTreatInterruptsISR(uint8_t bCANChannel)
 *//**
 * \b Description:
 *
 * This function must be called only from within an Interrupt. It will fetch the
 * data and post it on the callback.
 *
 * PRE-CONDITION: CAN Channel must be initialized
 *
 * POST-CONDITION: CAN interrupt sample sent via Callback
 *
 * @return     Void
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  *  //Not available.
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
static void CAN_vTreatInterruptsISR(uint8_t bCANChannel)
{
	can_config_s * pCAN = NULL;
	canMSGStruct_s sCANMessage;
	eCANStatus_s eCANStat;

	// If CANlist position has a valid structure
	if (CANlist[bCANChannel] != NULL)
	{
		pCAN = CANlist[bCANChannel];
	}

	if(pCAN != NULL)
	{
		uint32_t wCANint;
		while((wCANint = Chip_CCAN_GetIntID(CAN_MAP_REGISTER(pCAN->eCANPort))) != 0)
		{
			memset(sCANMessage.data, 0x00, 8);
			sCANMessage.dlc = 0;
			sCANMessage.id = 0;

			if (wCANint & CAN_INT_STATUS) //Error or Transmit/Reception OK
			{
				eCANStat = (Chip_CCAN_GetStatus(CAN_MAP_REGISTER(pCAN->eCANPort)) & CAN_STAT_LEC_MASK);

				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_BOFF);
				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_EPASS);
				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_EWARN);
				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_TXOK);
				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_RXOK);
				pCAN->fpCallback(eCANStat, sCANMessage);
			}
			else if (( 1 <= CCAN_INT_MSG_NUM(wCANint)) && (CCAN_INT_MSG_NUM(wCANint) <= 0x20))
			{
				Chip_CCAN_GetMsgObject(CAN_MAP_REGISTER(pCAN->eCANPort), sCANInterface[pCAN->eCANPort], wCANint, (CCAN_MSG_OBJ_T*)&sCANMessage);

				eCANStat = Chip_CCAN_GetStatus(CAN_MAP_REGISTER(pCAN->eCANPort));

				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_TXOK);
				Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CAN_STAT_RXOK);
				pCAN->fpCallback(eCANStat, sCANMessage);
			}
		}
	}
}

/******************************************************************************
 * Function : CAN0_IRQHandler(void)
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
void CAN0_IRQHandler(void)
{
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	CAN_vTreatInterruptsISR(CAN0);
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordExitISR();
#endif
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
void CAN1_IRQHandler(void)
{
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordEnterISR();
#endif
	CAN_vTreatInterruptsISR(CAN1);
#ifdef USE_SYSVIEW
	SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/******************************************************************************
 * Function : CAN_eCheckFault(adc_config_s * pCAN)
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
static eMCUError_s CAN_eCheckFault(const can_config_s *pCAN)
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


eMCUError_s CAN_eInit(can_config_s *pCAN)
{
	//Verify if it is a valid CAN port, if it is not already initiated, if it is a valid bitrate and if
	//it has a valid message id
	if ((pCAN->eCANPort < CAN_INVALID) && !CANInitiated[pCAN->eCANPort])
	{
		eMCUError_s eErrorcode = CAN_eCheckFault(pCAN);
		if (eErrorcode)
		{
			return eErrorcode;
		}

		if(BRD_CANConfig(pCAN->eCANPort) > CAN_MAX_CLOCK) //PinMux CAN on Board.c
						{
			return MCU_ERROR_CAN_INVALID_CLOCK;
						}

		CANInitiated[pCAN->eCANPort] = true; //Indicate that given CAN is initiated

		//CAN interface preparation
		Chip_CCAN_Init(CAN_MAP_REGISTER(pCAN->eCANPort));

		// Chip_CCAN_ConfigTestMode(CAN_MAP_REGISTER(pCAN->eCANPort),CCAN_TEST_TD_DOMINANT);

		if (!Chip_CCAN_SetBitRate(CAN_MAP_REGISTER(pCAN->eCANPort), pCAN->eCANBitrate))
		{
			return MCU_ERROR_CAN_INVALID_BITRATE;
		}

		CANlist[pCAN->eCANPort] = pCAN;

		return MCU_ERROR_SUCCESS;
	}
	return MCU_ERROR_CAN_INVALID_CAN_PORT;
}

void CAN_vDeInit(can_config_s *pCAN)
{
	NVIC_DisableIRQ(CAN_MAP_IRQ(pCAN->eCANPort));
	Chip_CCAN_DeInit(CAN_MAP_REGISTER(pCAN->eCANPort));
	CANInitiated[pCAN->eCANPort] = false; //Indicate that given CAN is not initiated
	CANlist[pCAN->eCANPort] = NULL;
}

void CAN_vAddMessageID(const can_config_s *pCAN, const uint16_t hCANmsgID)
{
	if ((pCAN->eCANPort < CAN_INVALID) && CANInitiated[pCAN->eCANPort]) //Valid and initiated
	{
		//Enable Interrupt
		Chip_CCAN_EnableInt(CAN_MAP_REGISTER(pCAN->eCANPort), (CCAN_CTRL_IE | CCAN_CTRL_SIE | CCAN_CTRL_EIE));
		Chip_CCAN_AddReceiveID(CAN_MAP_REGISTER(pCAN->eCANPort), sCANInterface[pCAN->eCANPort], hCANmsgID);
		//Enable Interrupt
		NVIC_EnableIRQ(CAN_MAP_IRQ(pCAN->eCANPort));
		//uint32_t wPrio = NVIC_GetPriority(CAN_MAP_IRQ(pCAN->eCANPort));
		//NVIC_SetPriority(CAN_MAP_IRQ(pCAN->eCANPort), wPrio);

	}
}

STATIC uint8_t getFreeMsgObject(LPC_CCAN_T *pCCAN)
{
	/* Return 1->32; 0 if not find free msg */
	uint32_t msg_valid;
	uint8_t i;
	msg_valid = Chip_CCAN_GetValidMsg(pCCAN);
	for (i = 0; i < CCAN_MSG_MAX_NUM; i++) {
		if (!((msg_valid >> i) & 1UL)) {
			return i + 1;
		}
	}
	return 0;	// No free object
}

void CCAN_SetMsgObject_ReceiveAll_ID(LPC_CCAN_T *pCCAN,
									 CCAN_MSG_IF_T IFSel,
									 CCAN_TRANSFER_DIR_T dir,
									 bool remoteFrame,
									 uint8_t msgNum,
									 const CCAN_MSG_OBJ_T *pMsgObj)
{
	/* Set a message into the message object in message RAM */
	uint16_t *pData;
	uint32_t msgCtrl = 0;

	if (pMsgObj == NULL) {
		return;
	}
	pData = (uint16_t *) (pMsgObj->data);

	msgCtrl |= CCAN_IF_MCTRL_UMSK | CCAN_IF_MCTRL_RMTEN(remoteFrame) | CCAN_IF_MCTRL_EOB |
					(pMsgObj->dlc & CCAN_IF_MCTRL_DLC_MSK);

	if (dir == CCAN_TX_DIR) {
		if (!remoteFrame) {
			msgCtrl |= CCAN_IF_MCTRL_TXRQ;
		}
	}
	else {
		msgCtrl |= CCAN_IF_MCTRL_RXIE;
	}

	pCCAN->IF[IFSel].MCTRL = msgCtrl;
	pCCAN->IF[IFSel].DA1 = *pData++;	/* Lower two bytes of message pointer */
	pCCAN->IF[IFSel].DA2 = *pData++;	/* Upper two bytes of message pointer */
	pCCAN->IF[IFSel].DB1 = *pData++;	/* Lower two bytes of message pointer */
	pCCAN->IF[IFSel].DB2 = *pData;	/* Upper two bytes of message pointer */

	/* Configure arbitration */
	if (!(pMsgObj->id & (0x1 << 30))) {					/* bit 30 is 0, standard frame */
		/* Mxtd: 0, Mdir: 1, Mask is 0x7FF */
		pCCAN->IF[IFSel].MSK2 = CCAN_IF_MASK2_MDIR(dir) | (~CCAN_MSG_ID_STD_MASK << 2);
		pCCAN->IF[IFSel].MSK1 = 0x0000;

		/* MsgVal: 1, Mtd: 0, Dir: 1, ID = 0x200 */
		pCCAN->IF[IFSel].ARB2 = CCAN_IF_ARB2_MSGVAL | CCAN_IF_ARB2_DIR(dir) | (pMsgObj->id << 2);
		pCCAN->IF[IFSel].ARB1 = 0x0000;
	}
	else {										/* Extended frame */            
		/* Mxtd: 1, Mdir: 1, Mask is 0x1FFFFFFF */
		pCCAN->IF[IFSel].MSK2 = CCAN_IF_MASK2_MXTD | CCAN_IF_MASK2_MDIR(dir) | (~CCAN_MSG_ID_EXT_MASK >> 16);
		pCCAN->IF[IFSel].MSK1 = ~CCAN_MSG_ID_EXT_MASK & 0x0000FFFF;

		/* MsgVal: 1, Mtd: 1, Dir: 1, ID = 0x200000 */
		pCCAN->IF[IFSel].ARB2 = CCAN_IF_ARB2_MSGVAL | CCAN_IF_ARB2_XTD | CCAN_IF_ARB2_DIR(dir) | (pMsgObj->id >> 16);
		pCCAN->IF[IFSel].ARB1 = pMsgObj->id & 0x0000FFFF;
	}    
	Chip_CCAN_TransferMsgObject(pCCAN, IFSel, CCAN_IF_CMDMSK_WR | CCAN_IF_CMDMSK_TRANSFER_ALL, msgNum);
}

void CAN_vAddAllMessageID(const can_config_s *pCAN, const uint32_t hCANmsgID)
{
	CCAN_MSG_OBJ_T temp;
	uint8_t msgNum;

	if ((pCAN->eCANPort < CAN_INVALID) && CANInitiated[pCAN->eCANPort]) //Valid and initiated
	{
		//Enable Interrupt
		Chip_CCAN_EnableInt(CAN_MAP_REGISTER(pCAN->eCANPort), (CCAN_CTRL_IE | CCAN_CTRL_SIE | CCAN_CTRL_EIE));

		msgNum = getFreeMsgObject(CAN_MAP_REGISTER(pCAN->eCANPort));
		if(!msgNum)
		{
			return;
		}
		temp.id = hCANmsgID;
		// Call CCAN_SetMsgObject_ReceiveAll_ID
		CCAN_SetMsgObject_ReceiveAll_ID(CAN_MAP_REGISTER(pCAN->eCANPort), sCANInterface[pCAN->eCANPort], CCAN_RX_DIR, false, msgNum, &temp);

		//Enable Interrupt
		NVIC_EnableIRQ(CAN_MAP_IRQ(pCAN->eCANPort));
	}
}

void CAN_vEnableLoopback(can_config_s *pCAN)
{
	Chip_CCAN_EnableTestMode(CAN_MAP_REGISTER(pCAN->eCANPort));
	Chip_CCAN_ConfigTestMode(CAN_MAP_REGISTER(pCAN->eCANPort), CCAN_TEST_LOOPBACK_MODE);
}

void CAN_vRemoveMessageID(const can_config_s *pCAN, const uint16_t hCANmsgID)
{
	if ((pCAN->eCANPort < CAN_INVALID) && CANInitiated[pCAN->eCANPort]) //Valid and initiated
	{
		Chip_CCAN_DeleteReceiveID(CAN_MAP_REGISTER(pCAN->eCANPort), sCANInterface[pCAN->eCANPort], hCANmsgID);
	}
}

uint32_t CAN_vConfigRemoteMessageObj(const can_config_s *pCAN, const canMSGStruct_s CANMessage)
{
	uint32_t obj_idx;

	if ((pCAN->eCANPort < CAN_INVALID) && CANInitiated[pCAN->eCANPort]) //Valid and initiated
	{
		obj_idx = getFreeMsgObject(CAN_MAP_REGISTER(pCAN->eCANPort));

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDMSK = CCAN_IF_CMDMSK_MASK |
						CCAN_IF_CMDMSK_ARB  |
						CCAN_IF_CMDMSK_CTRL;

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ = obj_idx;

		while((CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ & CCAN_IF_CMDREQ_BUSY) != 0);

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDMSK = CCAN_IF_CMDMSK_WR |
						CCAN_IF_CMDMSK_MASK  |
						CCAN_IF_CMDMSK_ARB |
						CCAN_IF_CMDMSK_CTRL;

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].MSK2 &= ~(1U << 14);
		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].ARB2 &= ~(1U << 13);
		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].ARB2 |= CCAN_IF_ARB2_MSGVAL;
		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].MCTRL = CCAN_IF_MCTRL_UMSK | CCAN_IF_MCTRL_EOB | CCAN_IF_MCTRL_RXIE;

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ = obj_idx;
		while((CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ & CCAN_IF_CMDREQ_BUSY) != 0);
	}
	return obj_idx;
}

void CAN_vSendRemoteMessage(const can_config_s *pCAN, const canMSGStruct_s CANMessage)
{
	// Configure an object to TX_RTR_RX_DATA and gets the object index
	uint32_t obj_idx = CAN_vConfigRemoteMessageObj(pCAN, CANMessage);

	uint32_t arb1, arb2, mctrl, size;

	if ((pCAN->eCANPort < CAN_INVALID) && CANInitiated[pCAN->eCANPort]) //Valid and initiated
	{
		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDMSK = CCAN_IF_CMDMSK_ARB  |                  // Read
						CCAN_IF_CMDMSK_CTRL |                  // Access arbitration and Access control bits
						CCAN_IF_CMDMSK_R_NEWDAT;               // Clear NEWDAT bit

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ = obj_idx;                               // Read from message object

		while((CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ & CCAN_IF_CMDREQ_BUSY) != 0);     // Wait for read to finish

		mctrl = CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].MCTRL;                                  // Store current value of MCTRL register
		arb1 = CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].ARB1;                                    // Store current value of ARB1 register
		arb2 = CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].ARB2;                                    // Store current value of ARB2 register

		// Prepare arb1 and arb2
		if(CANMessage.id & CAN_ID_IDE_Msk){     // Extended Identifier
			arb1 = CANMessage.id & (0xFFFFU << 0);
			arb2 = ((CANMessage.id >> 16) & (0x1FFFU << 0)) | CCAN_IF_ARB2_XTD | CCAN_IF_ARB2_MSGVAL;
		} else {                                // Standard Identifier
			arb1 = 0U ;
			arb2 = ((CANMessage.id << 2) & (0x1FFFU << 0)) | CCAN_IF_ARB2_MSGVAL;
		}

		// Test here if it's a remote transmission
		if(CANMessage.id & CAN_ID_RTR_Msk) {
			size = CANMessage.dlc;
		}

		if(size > 8U) {size = 8U;}

		mctrl = (mctrl & ~0xFU) | CCAN_IF_MCTRL_TXRQ | size;

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDMSK = CCAN_IF_CMDMSK_ARB  |                  // Access arbitration
						CCAN_IF_CMDMSK_CTRL |                  // Access control bits
						CCAN_IF_CMDMSK_WR;                     // Write

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].ARB1 = arb1;
		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].ARB2 = arb2;
		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].MCTRL = mctrl;

		CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ = obj_idx;                               // Write to message object
		while((CAN_MAP_REGISTER(pCAN->eCANPort)->IF[sCANInterface[pCAN->eCANPort]].CMDREQ & CCAN_IF_CMDREQ_BUSY) != 0);     // Wait for write to finish

		while (Chip_CCAN_GetTxRQST(CAN_MAP_REGISTER(pCAN->eCANPort)) >> (obj_idx - 1)) {    // blocking , wait for sending completed
		}
	}
	Chip_CCAN_SetValidMsg(CAN_MAP_REGISTER(pCAN->eCANPort), sCANInterface[pCAN->eCANPort], obj_idx, false);
}

void CAN_vSendMessage(const can_config_s *pCAN, const canMSGStruct_s CANMessage)
{
	if ((pCAN->eCANPort < CAN_INVALID) && CANInitiated[pCAN->eCANPort]) //Valid and initiated
	{
		if(!(CANMessage.id & CAN_ID_RTR_Msk))
		{
			Chip_CCAN_Send(CAN_MAP_REGISTER(pCAN->eCANPort), sCANInterface[pCAN->eCANPort], false, (CCAN_MSG_OBJ_T *)&CANMessage );
		} else {
			CAN_vSendRemoteMessage(pCAN, CANMessage);
		}
		Chip_CCAN_ClearStatus(CAN_MAP_REGISTER(pCAN->eCANPort), CCAN_STAT_TXOK);
	}
	uint32_t wIntActive = NVIC_GetActive(CAN_MAP_IRQ(pCAN->eCANPort));
	if (!wIntActive)
	{
		NVIC_EnableIRQ(CAN_MAP_IRQ(pCAN->eCANPort));
	}
}
