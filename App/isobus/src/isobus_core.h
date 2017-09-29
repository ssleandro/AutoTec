/****************************************************************************
 * Title                 :   isobus_core Include File
 * Filename              :   isobus_core.h
 * Author                :   Henrique Reis
 * Origin Date           :   01 de jul de 2016
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
 *    Date    Version     Author         				Description
 *  01/07/16   1.0.0  Henrique Reis     isobus_core.h created
 *
 *****************************************************************************/
/** @file
 *  @brief
 *
 *
 */
#ifndef ISOBUS_SRC_ISOBUS_CORE_H_
#define ISOBUS_SRC_ISOBUS_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../isobus/inc/interface_isobus.h"
#else
#include "M2GPlus.iop.h"
#include "M2GPlus.c.h"
#include "interface_isobus.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define ISO_FLAG_LANGUAGE_UPDATED	0x0001

#define ISO_NUM_NUMBER_VARIABLE_OBJECTS 276
#define ISO_NUM_INPUT_LIST_OBJECTS 4
#define ISO_NUM_FILL_ATTRIBUTES_OBJECTS 36

#define ISO_KEY_PLANTER_ID					SoftKey_Planter
#define ISO_KEY_INFO_ID						SoftKey_Info
#define ISO_KEY_SPEED_ID					SoftKey_Speed
#define ISO_KEY_CONFIG_ID					SoftKey_Config
#define ISO_KEY_INSTALLATION_ID				SoftKey_Setup
#define ISO_KEY_TEST_MODE_ID				SoftKey_TestMode
#define ISO_KEY_FINISH_TEST_ID				SoftKey_EndTest
#define ISO_KEY_REPLACE_SENSORS_ID			SoftKey_ReplaceSensor
#define ISO_KEY_BACKTO_INSTALLATION_ID		SoftKey_BackInstall
#define ISO_KEY_BACKTO_PLANTER_ID			SoftKey_BackCfgChanged
#define ISO_KEY_BACKTO_PLANT_FROM_CFG_ID	SoftKeyMask_ConfigToPlanter
#define ISO_KEY_TRIMMING_ID					SoftKey_Trimming
#define ISO_KEY_SYSTEM_ID					SoftKey_System
#define ISO_KEY_CLEAR_TOTAL_ID				SoftKey_ClearTotal
#define ISO_KEY_CLEAR_SUBTOTAL_ID			SoftKey_ClearSubTotal
#define ISO_KEY_BACKTO_TRIMMING_CHANGES_ID	SoftKey_BackTrimChanged
#define ISO_KEY_PAUSE_ID					SoftKey_Pause

#define ISO_BUTTON_REPEAT_TEST_ID							BU_SETUP_REPEAT_TEST
#define ISO_BUTTON_ERASE_INSTALLATION_ID					BU_SETUP_CLEAR_SETUP
#define ISO_BUTTON_CLEAR_COUNT_CANCEL_ID					BU_CLEAR_COUNT_CANCEL
#define ISO_BUTTON_CLEAR_COUNT_ACCEPT_ID					BU_CLEAR_COUNT_ACCEPT
#define ISO_BUTTON_CLEAR_SETUP_CANCEL_ID					BU_CLEAR_SETUP_CANCEL
#define ISO_BUTTON_CLEAR_SETUP_ACCEPT_ID					BU_CLEAR_SETUP_ACCEPT
#define ISO_BUTTON_CONFIG_CHANGES_CANCEL_RET_INSTALL_ID		BU_CFG_CHANGE_CANCEL_RET_SETUP
#define ISO_BUTTON_CONFIG_CHANGES_ACCEPT_ID					BU_CFG_CHANGE_ACCEPT
#define ISO_BUTTON_CONFIG_CHANGES_CANCEL_RET_CONFIG_ID		BU_CFG_CHANGE_CANCEL_RET_CONFIG
#define ISO_BUTTON_CONFIG_CHANGES_CANCEL_RET_PLANTER_ID		BU_CFG_CHANGE_CANCEL_RET_PLANTER
#define ISO_BUTTON_TRIMM_CHANGES_CANCEL_ID					BU_TRIM_CHANGE_CANCEL
#define ISO_BUTTON_TRIMM_CHANGES_ACCEPT_ID					BU_TRIM_CHANGE_ACCEPT
#define ISO_BUTTON_CHANGE_PASSWORD_ID						BU_CFG_CHANGE_PASSWD
#define ISO_BUTTON_PASSWORD_CANCEL_ID						BU_PASSWD_CANCEL
#define ISO_BUTTON_PASSWORD_ACCEPT_ID						BU_PASSWD_ACCEPT
#define ISO_BUTTON_REPLACE_SENSOR_CANCEL_ID					BU_REPLACE_SENSOR_CANCEL
#define ISO_BUTTON_REPLACE_SENSOR_ACCEPT_ID					BU_REPLACE_SENSOR_ACCEPT

#define ISO_INPUT_LIST_LANGUAGUE_ID			IL_CFG_LANGUAGE
#define ISO_INPUT_LIST_UNIT_SYSTEM_ID		IL_CFG_UNIT_SYSTEM
#define ISO_INPUT_LIST_AREA_MONITOR_ID		IL_CFG_AREA_MONITOR
#define ISO_INPUT_LIST_CENT_LINE_SIDE_ID	IL_CFG_CENTER_ROW_SIDE
#define ISO_INPUT_LIST_ALTERNATE_ROW_ID		IL_CFG_ALTERNATE_ROWS
#define ISO_INPUT_LIST_ALTER_ROW_TYPE_ID	IL_CFG_RAISED_ROWS

#define ISO_INPUT_LIST_OPTION_ATTRIBUTE		0x06

#define ISO_INPUT_LIST_ENABLE				0x01
#define ISO_INPUT_LIST_DISABLE				0x00

#define ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE	0x0F
#define ISO_INPUT_NUMBER_ENABLE				0x01
#define ISO_INPUT_NUMBER_DISABLE			0x00

#define ISO_RECTANGLE_LINE_ATTRIBUTE		0x01
#define ISO_BAR_GRAPH_COLOUR_ATTRIBUTE		0x03

#define ISO_INPUT_BOOLEAN_SET				0x01
#define ISO_INPUT_BOOLEAN_CLEAR				0x00

#define ISO_BUTTON_ACTIVATION_PRESSED		0x01

#define ISO_OBJECT_WORKING_SET_ID		WorkingSet_0
#define ISO_OBJECT_PICTURE_GRAPHIC_ID	PG_AUTEQ_TELEMATICA
#define ISO_OBJECT_SOFT_KEY_MASK_ID		SoftKeyMask_Installation
#define ISO_OBJECT_POINTER_ID			OP_PLANTER_IND_LINE_NUM
#define ISO_OBJECT_KEY_ID				SoftKey_Config
#define ISO_OBJECT_CONTAINER_ID			Container_3000
#define ISO_OBJECT_OUTPUT_STRING_ID		OutputString_11000
#define ISO_OBJECT_DATA_MASK_ID			DataMask_Installation
#define ISO_OBJECT_OUTPUT_NUMBER_ID		ON_TEST_MODE_L01
#define ISO_OBJECT_BUTTON_ID			BU_CFG_TAB_HOME
#define ISO_OBJECT_FONT_ID				FontAttributes_23000
#define ISO_OBJECT_BAR_GRAPH_ID			BG_PLANT_UP_L01
#define ISO_OBJECT_NUMBER_VARIABLE_ID	NV_TEST_MODE_L01
#define ISO_OBJECT_INPUT_NUMBER_ID		IN_CFG_VEHICLE_CODE
#define ISO_OBJECT_OUTPUT_LINE_ID		Line_13000
#define ISO_OBJECT_INPUT_LIST_ID		IL_CFG_AREA_MONITOR
#define ISO_OBJECT_RECTANGLE_ID			Rectangle_14000
#define ISO_OBJECT_FILL_ATTRIBUTES_ID	FA_SETUP_STAT_L01
#define ISO_OBJECT_LINE_ATTRIBUTES_ID	LineAttributes_24000

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
//#define ISO_ALARM_FREQUENCY_HZ	2900
#define ISO_ALARM_FREQUENCY_HZ	800

// Alarms
#define ISO_ALARM_SETUP_NEW_SENSOR_ACTIVATIONS 	1
#define ISO_ALARM_SETUP_NEW_SENSOR_FREQUENCY_HZ	ISO_ALARM_FREQUENCY_HZ
#define ISO_ALARM_SETUP_NEW_SENSOR_ON_TIME_MS 	250
#define ISO_ALARM_SETUP_NEW_SENSOR_OFF_TIME_MS 	750

#define ISO_ALARM_SETUP_FINISHED_ACTIVATIONS 	3
#define ISO_ALARM_SETUP_FINISHED_FREQUENCY_HZ 	ISO_ALARM_FREQUENCY_HZ
#define ISO_ALARM_SETUP_FINISHED_ON_TIME_MS 	250
#define ISO_ALARM_SETUP_FINISHED_OFF_TIME_MS 	250

#define ISO_ALARM_TOLERANCE_ACTIVATIONS 	2
#define ISO_ALARM_TOLERANCE_FREQUENCY_HZ 	ISO_ALARM_FREQUENCY_HZ
#define ISO_ALARM_TOLERANCE_ON_TIME_MS 		250
#define ISO_ALARM_TOLERANCE_OFF_TIME_MS 	250

#define ISO_ALARM_EXCEEDED_SPEED_ACTIVATIONS 	1
#define ISO_ALARM_EXCEEDED_SPEED_FREQUENCY_HZ 	ISO_ALARM_FREQUENCY_HZ
#define ISO_ALARM_EXCEEDED_SPEED_ON_TIME_MS 	250
#define ISO_ALARM_EXCEEDED_SPEED_OFF_TIME_MS 	750

#define ISO_ALARM_LINE_FAILURE_ACTIVATIONS		1
#define ISO_ALARM_LINE_FAILURE_FREQUENCY_HZ 	ISO_ALARM_FREQUENCY_HZ
#define ISO_ALARM_LINE_FAILURE_ON_TIME_MS 		850
#define ISO_ALARM_LINE_FAILURE_OFF_TIME_MS 		150

#define ISO_ALARM_DEACTIVATE	0

#define TIMER_PERIOD_MS_WS_MAINTENANCE	850

/******************************************************************************
 * Macros
 *******************************************************************************/
#define INPUT_LIST_GET_INDEX_FROM_ID(id) (id - IL_CFG_AREA_MONITOR)
#define BARGRAPH_UP_GET_ID_FROM_LINE_NUMBER(line) (uint16_t)((line > 0) ? (BG_PLANT_UP_L02 + (2 * (line - 1))) : BG_PLANT_UP_L01)
#define BARGRAPH_DOWN_GET_ID_FROM_LINE_NUMBER(line) (uint16_t)((line > 0) ? (BG_PLANT_DOWN_L02 + (2 * (line - 1))) : BG_PLANT_DOWN_L01)
#define RECTANGLE_PLANT_GET_ID_FROM_LINE_NUMBER(line) (uint16_t)(RT_PLANT_L01 + line)
#define GET_FLOAT_VALUE(value) ((float)(value/10.0f))
#define GET_UNSIGNED_INT_VALUE(value) ((uint32_t)(value*10))

/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef enum
{
	WAIT_GLOBAL_VT_STATUS,
	WAIT_VT_STATUS,
	WAIT_LOAD_VERSION,
	WAIT_SEND_POOL,
	OBJECT_POOL_SENT,
	OBJECT_POOL_LANG_PKG_SENT,
	OBJECT_POOL_LOADED,
	BOOT_COMPLETED
} eBootStates;

typedef enum
{
	BOOT,
	RUNNING,
	UPDATING_LANGUAGE,
	UPDATING_STRING,
} eModuleStates;

typedef enum
{
	CLEAR_TOTALS_IDLE,
	CLEAR_SUBTOTAL_WAIT_CONFIRMATION,
	CLEAR_SUBTOTAL,
	CLEAR_TOTAL_WAIT_CONFIRMATION,
	CLEAR_TOTAL,
} eClearCounterStates;

typedef enum
{
	CLEAR_SETUP_IDLE,
	CLEAR_SETUP_WAIT_CONFIRMATION,
	CLEAR_SETUP,
} eClearSetupStates;

typedef enum
{
	TRIMM_CHANGE_IDLE,
	TRIMM_CHANGE_WAIT_CONFIRMATION,
	TRIMM_CHANGE,
} eChangeTrimmingState;

typedef enum
{
	PASSWD_IDLE,
	PASSWD_ENTER_PASSWORD,
	PASSWD_ACCEPTED,
	PASSWD_NOT_ACCEPTED,
	PASSWD_CHANGE_PASSWD_CURRENT_PASSWD,
	PASSWD_CHANGE_PASSWD_NEW_PASSWD,
	PASSWD_CHANGE_ACCEPTED,
	PASSWD_CHANGE_NOT_ACCEPTED,
} eTreatPasswordState;

typedef enum
{
	UPDATE_PLANTER_NO_ALARM,
	UPDATE_PLANTER_ALARM,
} eUpdatePlanterMaskStates;

typedef struct
{
	eUpdatePlanterMaskStates eUpdateState;
	event_e eAlarmEvent;
} sUpdatePlanterMaskStates;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void ISO_vTimerCallbackWSMaintenance (void const *arg);
void ISO_vTimerCallbackAlarmTimeout (void const *arg);

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* M2G_SRC_M2G_CORE_H_ */

/*** End of File **************************************************************/
