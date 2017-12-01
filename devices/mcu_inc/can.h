/****************************************************************************
 * Title                 :   CAN HAL
 * Filename              :   can.h
 * Author                :   Thiago Palmieri
 * Origin Date           :   17/02/2016
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
 *  17/02/16   1.0.0   Thiago Palmieri ADC HAL Created.
 *
 *****************************************************************************/
/** @file can.h
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of CAN functions
 *
 *  The CAN interface must be initialized passing the CAN channel, a valid Bitrate
 *  and a valid Callback function.
 *
 *  After initialization, the interface will need to have at least one message ID
 *  registered before it can start, therefore in order to fully enable it, AddMessageID
 *  function must be executed. Any number of messages can be added.
 *
 *  Message reception is made through the Callback, an error code is also provided so
 *  the application must handle it accordingly.
 *
 *
 *  Message sending is made via SendMessage function, where a message struct consisting
 *  of Message ID, size (maximum 8) and payload (maximum 8 Bytes) must be passed.
 *
 *  The CAN interface may be disabled via DeInit function.
 */
#ifndef ABS_INC_CAN_H_
#define ABS_INC_CAN_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <inttypes.h>
#include "mcuerror.h"

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
/****** CAN ID Frame Format codes *****/
#define CAN_ID_IDE_Pos 30UL
#define CAN_ID_RTR_Pos 31UL
#define CAN_ID_IDE_Msk (1UL << CAN_ID_IDE_Pos)
#define CAN_ID_RTR_Msk (1UL << CAN_ID_RTR_Pos)

/****** CAN Identifier encoding *****/
#define CAN_DATA_STANDARD_ID(id) (id & 0x000007FFUL)                      ///< CAN identifier in standard format (11-bits)
#define CAN_DATA_EXTENDED_ID(id) ((id & 0x1FFFFFFFUL) | CAN_ID_IDE_Msk)   ///< CAN identifier in extended format (29-bits)

#define CAN_RTR_STANDARD_ID(id) ((id & 0x000007FFUL) | CAN_ID_RTR_Msk)                    ///< CAN identifier in standard format (11-bits)
#define CAN_RTR_EXTENDED_ID(id) ((id & 0x1FFFFFFFUL) | CAN_ID_IDE_Msk | CAN_ID_RTR_Msk)   ///< CAN identifier in extended format (29-bits)

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/
#define CAN_MAX_CHANNELS (CAN_INVALID) //!< Maximum number of CAN channels
#define CAN_MAX_CLOCK 100000000  //!< Maximum clock supported on CAN

#define CAN_STAT_LEC_MASK  (0x07)    //!< Mask for Last Error Code
#define CAN_INT_STATUS (0x8000)      //!< CAN Status Macro

#ifndef NULL
#define NULL (void*)0
#endif

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 * CAN message Status
 */
typedef enum eCANStatus_s
{
	CAN_STAT_NOERROR = 0x00,
	CAN_STAT_STUFFERROR = 0x01,
	CAN_STAT_FORMERROR = 0x02,
	CAN_STAT_ACKERROR = 0x03,
	CAN_STAT_BIT1ERROR = 0x04,
	CAN_STAT_BIT0ERROR = 0x05,
	CAN_STAT_CRCERROR = 0x06,
	CAN_STAT_UNUSED = 0x07,
	CAN_STAT_TXOK = (1 << 3),   //!< CAN Transmited message successfully
	CAN_STAT_RXOK = (1 << 4),   //!< CAN Received a message successfully
	CAN_STAT_EPASS = (1 << 5),   //!< CAN controller is in the error passive state
	CAN_STAT_EWARN = (1 << 6), //!< CAN At least one of the error counters in the EC has reached the error warning limit of 96
	CAN_STAT_BOFF = (1 << 7)    //!< CAN controller is in busoff state
} eCANStatus_s;

/**
 * CAN message object structure
 */
typedef struct canMSGStruct_s
{
	uint32_t id; /**< ID of message, if bit 30 is set then this is extended frame */
	uint32_t dlc; /**< Message data length */
	uint8_t data[8];/**< Message data */
} canMSGStruct_s;


typedef struct canStatusStruct_s
{
	uint16_t wBaudRateKbps;
	uint32_t wRxCount;
	uint8_t bRxError;
	uint32_t wTxCount;
	uint8_t bTxError;
	uint32_t dDataOverrun;
	uint32_t dArbitrationLost;
	uint32_t dErrorPassive;
	uint32_t dErrorWarning;
	uint32_t dBusError;
} canStatusStruct_s;


typedef void (*canCallBack) (eCANStatus_s eErrorCode, canMSGStruct_s CANMessage);  //!< CAN Callback typedef/

/**
 * This enumeration is a list of possible CAN ports on the Board
 */
typedef enum canChannel_s
{
	CAN0,           //!< CAN 0
	CAN1,           //!< CAN 1
	CAN_INVALID     //!< Invalid CAN indicator
} canChannel_s;

/**
 * This enumeration is a list of possible CAN bitrates that can be used
 */
typedef enum canBitrate_s
{
	CAN_BITRATE_250 = 250000,      //!< 250Kbps
	CAN_BITRATE_500 = 500000,      //!< 500Kbps
	CAN_BITRATE_1000 = 1000000,     //!< 1000Kbps
	CAN_BITRATE_INVALID                  //!< Invalid value
} canBitrate_s;

/**
 * This Struct holds CAN configuration for given board
 */
typedef struct can_config_s
{
	canChannel_s eCANPort;        //!< CAN Port
	canBitrate_s eCANBitrate;     //!< CAN BitRate
	canCallBack fpCallback;       //!< CAN Callback Function
	void * vpPrivateData;         //!< CAN Private data
	uint8_t bIDRecvObj;				//!< ID of CAN receive object
} can_config_s;

typedef enum
{
	eCAN_RX_DIR,
	eCAN_TX_DIR,
} can_transfer_id;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Function : CAN_eInit(can_config_s *pCAN)
 *//**
 * \b Description:
 *
 * This is a public function used to initialize a CAN port.
 * A can_config_s struct pointer must be passed to the function. The struct
 * must be initialized with CAN port, bitrate and callback.
 * However, the CAN port will only be active when a messageID is added.
 *
 * PRE-CONDITION: Valid CAN port.
 *
 * POST-CONDITION: CAN Port initialized and ready.
 *
 * @return     eMCUerror
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   can_config_s sCANport;
 *   sCANport.eCANPort = CAN0;
 *   sCANport.eCANBitrate = CAN_BITRATE_500;
 *   sCANport.fpCallback = CANCallback;
 *
 *   if (MCU_ERROR_SUCCESS == CAN_bInit(&sCANport))
 *     PRINTF("CAN0 Initiated");
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
eMCUError_s CAN_eInit (can_config_s *pCAN);
eMCUError_s CANISO_eInit (can_config_s *pCAN);

/******************************************************************************
 * Function : CAN_vDeInit(can_config_s *pCAN)
 *//**
 * \b Description:
 *
 * This is a public function used to de-initialize a CAN port.
 * This function will completely disable the CANport previously initialized
 *
 * PRE-CONDITION: Initialized CAN port.
 *
 * POST-CONDITION: CAN Port disconnected.
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   can_config_s sCANport;
 *   sCANport.eCANPort = CAN0;
 *   sCANport.eCANBitrate = CAN_BITRATE_500;
 *   sCANport.fpCallback = CANCallback;
 *
 *   CAN_bInit(&sCANport);  // CAN0 Initiated
 *   CAN_vDeInit(&sCANport);  // CAN0 de-initialized
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
void CAN_vDeInit (can_config_s *pCAN);
void CANISO_vDeInit (can_config_s *pCAN);

/******************************************************************************
 * Function : CAN_vAddMessageID(const can_config_s *pCAN, const uint16_t hCANmsgID)
 *//**
 * \b Description:
 *
 * This is a public function used to add another messageID to current CAN.
 * An initialized CAN port is required for this function to work properly.
 *
 * PRE-CONDITION: Initialized CAN port.
 *
 * POST-CONDITION: CAN Now can receive a new message.
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   can_config_s sCANport;
 *   sCANport.eCANPort = CAN0;
 *   sCANport.eCANBitrate = CAN_BITRATE_500;
 *   sCANport.fpCallback = CANCallback;
 *
 *   CAN_bInit(&sCANport);  // CAN0 Initiated
 *   CAN_vAddMessage(&sCANport, 500); // CAN0 now receive message 500
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
void CAN_vAddMessageID (const can_config_s *pCAN, const uint16_t hCANmsgID);
void CANISO_vAddMessageID (const can_config_s *pCAN, const uint16_t hCANmsgID);

void CAN_vAddAllMessageID (const can_config_s *pCAN, const uint32_t hCANmsgID);
void CANISO_vAddAllMessageID (can_config_s *pCAN, const uint32_t hCANmsgID);

void CAN_vEnableLoopback (can_config_s *pCAN);
void CANISO_vEnableLoopback (can_config_s *pCAN);

/******************************************************************************
 * Function : CAN_vRemoveMessageID(const can_config_s *pCAN, const uint16_t hCANmsgID)
 *//**
 * \b Description:
 *
 * This is a public function used to remove a messageID from current CAN.
 * An initialized CAN port is required for this function to work properly.
 *
 * PRE-CONDITION: Initialized CAN port.
 *
 * POST-CONDITION: Message ID removed from CAN port.
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   can_config_s sCANport;
 *   sCANport.eCANPort = CAN0;
 *   sCANport.eCANBitrate = CAN_BITRATE_500;
 *   sCANport.fpCallback = CANCallback;
 *
 *   CAN_bInit(&sCANport);  // CAN0 Initiated
 *   CAN_vAddMessage(&sCANport, 300); // CAN0 now receive message 300
 *   CAN_vAddMessage(&sCANport, 500); // CAN0 now receives message 300 and 500
 *   CAN_vRemoveMessage(&sCANport, 300); // CAN0 now only receives message 500
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
void CAN_vRemoveMessageID (const can_config_s *pCAN, const uint16_t hCANmsgID);
void CANISO_vRemoveMessageID (const can_config_s *pCAN, const uint16_t hCANmsgID);

/******************************************************************************
 * Function : CAN_vSendMessage(const can_config_s *pCAN, const canMSGStruct_s CANMessage)
 *//**
 * \b Description:
 *
 * This is a public function used to send a CAN message.
 * An initialized CAN port is required for this function to work properly.
 *
 * PRE-CONDITION: Initialized CAN port.
 *
 * POST-CONDITION: CAN message sent.
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   can_config_s sCANport;
 *   sCANport.eCANPort = CAN0;
 *   sCANport.eCANBitrate = CAN_BITRATE_500;
 *   sCANport.fpCallback = CANCallback;
 *
 *   canMSGStruct_s CANMessageOne;
 *   CANMessageOne.id = 400;   //Message ID
 *   CANMessageOne.dlc = 7;    //Message Size
 *   CANMessageOne.data = {0, 1, 2, 3, 4, 5, 6}; //Message Data
 *
 *   CAN_bInit(&sCANport);  // CAN0 Initiated
 *   CAN_vSendMessage(&sCANport, CANMessageOne);  // MessageOne sent via CAN0
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
void CAN_vSendMessage (const can_config_s *pCAN, const canMSGStruct_s CANMessage);
void CANISO_vSendMessage (const can_config_s *pCAN, const canMSGStruct_s CANMessage);

void CAN_vSendRemoteMessage (const can_config_s *pCAN, const canMSGStruct_s CANMessage);

uint8_t CAN_bGetErrCount(const can_config_s *pCCAN, can_transfer_id dir);
uint8_t CANISO_bGetErrCount(const can_config_s *pCCAN, can_transfer_id dir);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ABS_INC_CAN_H_ */
