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
#include "interface_isobus.h"
#endif
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
#define ISO_NUM_NUMBER_VARIABLE_OBJECTS 7
#define ISO_NUM_INPUT_LIST_OBJECTS 9
#define ISO_NUM_FILL_ATTRIBUTES_OBJECTS 72
#define ISO_NUM_BAR_GRAPH_OBJECTS 72

#define ISO_UPDATE_CURRENT_DATA_MASK 		0x00000001
#define ISO_UPDATE_CURRENT_CONFIGURATION    0x00000002
#define ISO_UPDATE_INSTALLATION_INTERFACE	0x00000004
#define ISO_UPDATE_PLANTER_INTERFACE		0x00000008
#define ISO_UPDATE_TEST_MODE_INTERFACE		0x00000010
#define ISO_UPDATE_TRIMMING_INTERFACE		0x00000020
#define ISO_UPDATE_SYSTEM_INTERFACE			0x00000040
#define ISO_INSTALLATION_REPEAT_TEST		0x00000080
#define ISO_INSTALLATION_ERASE_INSTALLATION	0x00000100

#define ISO_BUTTON_REPEAT_TEST_ID			0x6005
#define ISO_BUTTON_ERASE_INSTALLATION_ID	0x6006

/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define ISO_OBJECT_PICTURE_GRAPHIC_ID 0x1000
#define ISO_OBJECT_SOFT_KEY_MASK_ID	  0x2000
#define ISO_OBJECT_POINTER_ID		  0x2100
#define ISO_OBJECT_KEY_ID			  0x3000
#define ISO_OBJECT_CONTAINER_ID		  0x4000
#define ISO_OBJECT_OUTPUT_STRING_ID	  0x4500
#define ISO_OBJECT_STRING_VARIABLE_ID 0x4700
#define ISO_OBJECT_DATA_MASK_ID		  0x5000
#define ISO_OBJECT_OUTPUT_NUMBER_ID   0x5500
#define ISO_OBJECT_BUTTON_ID		  0x6000
#define ISO_OBJECT_FONT_ID			  0x6500
#define ISO_OBJECT_BAR_GRAPH_ID		  0x7000
#define ISO_OBJECT_NUMBER_VARIABLE_ID 0x8000
#define ISO_OBJECT_INPUT_NUMBER_ID	  0x8200
#define ISO_OBJECT_INPUT_BOOLEAN_ID	  0x8500
#define ISO_OBJECT_OUTPUT_LINE_ID	  0x8900
#define ISO_OBJECT_INPUT_LIST_ID	  0x9000
#define ISO_OBJECT_RECTANGLE_ID		  0x9500
#define ISO_OBJECT_FILL_ATTRIBUTES_ID 0x9600
#define ISO_OBJECT_LINE_ID			  0x9900

/******************************************************************************
* Macros
*******************************************************************************/
#define GET_INDEX_FROM_ID(id) (id & 0x0FFF)
#define GET_FLOAT_VALUE(value) ((float)(value/10.0f))
#define GET_UNSIGNED_INT_VALUE(value) ((uint32_t)(value*10))

/******************************************************************************
* Typedefs
*******************************************************************************/
typedef enum {
	WAIT_GLOBAL_VT_STATUS,
	WAIT_VT_STATUS,
	WAIT_LOAD_VERSION,
	WAIT_SEND_POOL,
	OBJECT_POOL_SENDED,
	OBJECT_POOL_LOADED,
	BOOT_COMPLETED
} eBootStates;

typedef enum {
	BOOT,
	RUNNING
} eModuleStates;

typedef enum {
	STATUS_TRIMMING_NOT_TRIMMED = 0x0A,
	STATUS_TRIMMING_TRIMMED		= 0x0C,
	STATUS_TRIMMING_NONE	    = 0xFF,
	STATUS_TRIMMING_INVALID,
} eTrimmingStatus;

typedef enum {
	LANGUAGE_PORTUGUESE,
	LANGUAGE_ENGLISH,
	LANGUAGE_SPANISH,
	LANGUAGE_RUSSIAN,
	LANGUAGE_INVALID
} eSelectedLanguage;

typedef enum {
	UNIT_INTERNATIONAL_SYSTEM,
	UNIT_IMPERIAL_SYSTEM,
	UNIT_SYSTEM_INVALID
} eSelectedUnitMeasurement;

typedef enum {
	AREA_MONITOR_DISABLED,
	AREA_MONITOR_ENABLED,
	AREA_MONITOR_INVALID
} eAreaMonitor;

typedef enum {
	ALTERNATE_ROWS_DISABLED,
	ALTERNATE_ROWS_ENABLED,
	ALTERNATE_ROWS_INVALID
} eAlternateRows;

typedef enum {
	ALTERNATED_ROWS_ODD,
	ALTERNATED_ROWS_EVEN,
	ALTERNATED_ROWS_INVALID
} eAlternatedRowsType;

typedef struct sNumberVariableObj {
	uint16_t wObjID;
	uint32_t dValue;
	float fValue;
} sNumberVariableObj;

typedef struct sInputListObj {
	uint16_t wObjID;
	uint8_t bSelectedIndex;
} sInputListObj;

typedef struct sFillAtributtesObj {
	uint16_t wObjID;
	uint8_t bColor;
} sFillAttributesObj;

typedef struct sConfigurationDataMask {
	eSelectedLanguage* eLanguage;
	eSelectedUnitMeasurement* eUnit;
	uint32_t* dVehicleID;
	eAreaMonitor eMonitor;
	float* fSeedsPerMeter;
	uint8_t* bNumOfRows;
	float* fImplementWidth;
	float* fEvaluationDistance;
	uint8_t* bTolerance;
	float* fMaxSpeed;
	eAlternateRows eAlterRows;
	eAlternatedRowsType eAltType;
} sConfigurationDataMask;

typedef struct sInstallSensorStatus {
	sFillAttributesObj* pFillAttribute;
	uint8_t bNumOfSensors;
} sInstallSensorStatus;

typedef struct sInstallationDataMask {
	sInstallSensorStatus* psLinesInstallStatus;
} sInstallationDataMask;

typedef struct sPlantingVariables {
	sNumberVariableObj* const psNumberVariable;
	uint8_t bNumOfVariables;
} sPlantingVariables;

typedef struct sBarGraphStatus {
	uint16_t wIncBarID;
	uint16_t wDecBarID;
	uint16_t wIncOutputNumID;
	uint16_t wDecOutputNumID;
	int8_t bValue;
} sBarGraphStatus;

typedef struct sPlanterDataMask {
	sBarGraphStatus* psLinesStatus;
	sBarGraphStatus* psIndividualLineStatus;
	uint32_t* pdPartPopSemPerMt;
	uint32_t* pdPartPopSemPetHa;
	uint32_t* pdWorkedAreaMt;
	uint32_t* pdWorkedAreaHa;
	uint32_t* pdTotalMt;
	uint32_t* pdTotalHa;
	uint32_t* pdProductivity;
	uint32_t* pdWorkedTime;
	uint32_t* pdTotalSeeds;
} sPlanterDataMask;

typedef struct sLineCountVariables {
	sNumberVariableObj* psNumberVariable;
	uint8_t bNumOfSensors;
	uint8_t bNumOfVariables;
} sLineCountVariables;

typedef struct sTestModeDataMask {
	sLineCountVariables* psSeedsCount;
	uint32_t* pdInstalledSensors;
	uint32_t* pdConfiguredSensors;
} sTestModeDataMask;

typedef struct sTrimmingStatus {
	sFillAttributesObj* pFillAtributte;
	uint8_t bNumOfSensor;
} sTrimmingStatus;

typedef struct sTrimmingDataMask {
	sTrimmingStatus* psTrimmedLines;
} sTrimmingDataMask;

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
void ISO_vTimerCallbackWSMaintenance(void const *arg);

#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* M2G_SRC_M2G_CORE_H_ */

/*** End of File **************************************************************/
