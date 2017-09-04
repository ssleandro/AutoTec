/****************************************************************************
 * Title                 :   isobus_app
 * Filename              :   isobus_app.h
 * Author                :   Henrique Reis
 * Origin Date           :   29/05/2017
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
 * 29/05/2017  1.0.0     Henrique Reis      isobus_app.h created.
 *
 *****************************************************************************/
#ifndef APP_COMMON_APP_COMMON_ISOBUS_APP_H_
#define APP_COMMON_APP_COMMON_ISOBUS_APP_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "acquireg_app.h"
#include "M2GPlus.iop.h"

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef enum
{
	SOFT_KEY_MASK_INSTALLATION = SoftKeyMask_Installation,
	SOFT_KEY_MASK_INSTALLATION_FINISH,
	SOFT_KEY_MASK_PLANTER,
	SOFT_KEY_MASK_CONFIG_TO_SETUP,
	SOFT_KEY_MASK_CONFIGURATION_CHANGES,
	SOFT_KEY_MASK_TEST_MODE,
	SOFT_KEY_MASK_TRIMMING,
	SOFT_KEY_MASK_SYSTEM,
	SOFT_KEY_MASK_PLANTER_INFO,
	SOFT_KEY_MASK_PLANTER_MOVING,
	SOFT_KEY_MASK_PLANTER_INFO_MOVING,
	SOFT_KEY_MASK_CONFIG_TO_PLANTER,
	SOFT_KEY_MASK_TRIMMING_CHANGES,
	SOFT_KEY_MASK_INVALID
} eIsobusSoftKeyMask;

typedef enum
{
	DATA_MASK_INSTALLATION = DataMask_Installation,
	DATA_MASK_CONFIGURATION,
	DATA_MASK_PLANTER,
	DATA_MASK_TEST_MODE,
	DATA_MASK_TRIMMING,
	DATA_MASK_SYSTEM,
	DATA_MASK_CONFIRM_CLEAR_COUNTER,
	DATA_MASK_CONFIRM_CONFIG_CHANGES,
	DATA_MASK_CONFIRM_CLEAR_SETUP,
	DATA_MASK_CONFIRM_TRIMMING_CHANGES,
	DATA_MASK_INVALID
} eIsobusMask;

typedef enum
{
	MASK_TYPE_DATA_MASK = 0x01,
	MASK_TYPE_ALARM_MASK,
	MASK_TYPE_INVALID
} eIsobusMaskType;

typedef enum
{
	STATUS_INSTALL_WAITING = 0xE6,
	STATUS_INSTALL_INSTALLING = 0x0E,
	STATUS_INSTALL_INSTALLED = 0x0A,
	STATUS_INSTALL_INSTALL_ERROR = 0x0C,
	STATUS_INSTALL_NONE = 0x07,
	STATUS_INSTALL_INVALID
} eInstallationStatus;

typedef struct
{
	uint8_t bLineNum;
	bool bLineIgnored;
} sIgnoreLineStatus;

typedef enum
{
	LANGUAGE_PORTUGUESE,
	LANGUAGE_ENGLISH,
	LANGUAGE_SPANISH,
	LANGUAGE_RUSSIAN,
	LANGUAGE_INVALID
} eSelectedLanguage;

typedef enum
{
	UNIT_INTERNATIONAL_SYSTEM,
	UNIT_IMPERIAL_SYSTEM,
	UNIT_SYSTEM_INVALID
} eSelectedUnitMeasurement;

typedef enum
{
	AREA_MONITOR_DISABLED,
	AREA_MONITOR_ENABLED,
	AREA_MONITOR_INVALID
} eAreaMonitor;

typedef enum
{
	CENTRAL_ROW_SIDE_LEFT,
	CENTRAL_ROW_SIDE_RIGHT,
	CENTRAL_ROW_SIDE_INVALID
} eCentralRowSide;

typedef enum
{
	ALTERNATE_ROWS_DISABLED,
	ALTERNATE_ROWS_ENABLED,
	ALTERNATE_ROWS_INVALID
} eAlternateRows;

typedef enum
{
	ALTERNATED_ROWS_ODD,
	ALTERNATED_ROWS_EVEN,
	ALTERNATED_ROWS_INVALID
} eAlternatedRowsType;

typedef enum
{
	STATUS_TRIMMING_NOT_TRIMMED = 0x0A,
	STATUS_TRIMMING_TRIMMED = 0x0C,
	STATUS_TRIMMING_NONE = 0xFF,
	STATUS_TRIMMING_INVALID,
} eTrimmingStatus;

typedef enum
{
	TRIMMING_NOT_TRIMMED,
	TRIMMING_LEFT_SIDE,
	TRIMMING_RIGHT_SIDE,
} eTrimming;

typedef struct
{
	eTrimming eTrimmState;
	eTrimming eNewTrimmState;
} sTrimmingState;

typedef struct sConfigurationData
{
	eSelectedLanguage eLanguage;
	eSelectedUnitMeasurement eUnit;
	uint32_t dVehicleID;
	eAreaMonitor eMonitorArea;
	uint16_t wSeedRate;
	uint8_t bNumOfRows;
	eCentralRowSide eCentralRowSide;
	uint32_t wImplementWidth;
	uint32_t wEvaluationDistance;
	uint32_t wDistBetweenLines;
	uint8_t bTolerance;
	float fMaxSpeed;
	eAlternateRows eAlterRows;
	eAlternatedRowsType eAltType;
} sConfigurationData;

typedef struct sConfigurationDataMask
{
	eSelectedLanguage* eLanguage;
	eSelectedUnitMeasurement* eUnit;
	uint32_t* dVehicleID;
	eAreaMonitor* eMonitor;
	uint32_t* wSeedRate;
	uint8_t* bNumOfRows;
	eCentralRowSide* eCentralRowSide;
	uint32_t* wImplementWidth;
	uint32_t* wEvaluationDistance;
	uint32_t* wDistBetweenLines;
	uint8_t* bTolerance;
	float* fMaxSpeed;
	eAlternateRows* eAlterRows;
	eAlternatedRowsType* eAltType;
} sConfigurationDataMask;

typedef struct sNumberVariableObj
{
	uint16_t wObjID;
	uint32_t dValue;
	uint64_t lValue;
	float fValue;
} sNumberVariableObj;

typedef struct sInputListObj
{
	uint16_t wObjID;
	uint8_t bSelectedIndex;
} sInputListObj;

typedef struct sFillAtributtesObj
{
	uint16_t wObjID;
	uint8_t bColor;
} sFillAttributesObj;

typedef struct sInstallSensorStatus
{
	sFillAttributesObj* pFillAttribute;
	uint8_t bNumOfSensors;
} sInstallSensorStatus;

typedef struct sInstallationDataMask
{
	sInstallSensorStatus* psLinesInstallStatus;
} sInstallationDataMask;


typedef struct sPlanterIndividualLinesData
{
	int32_t dsLineAverage;
	uint32_t dLineSemPerUnit;
	uint32_t dLineSemPerHa;
	uint32_t dLineTotalSeeds;
} sPlanterIndividualLinesData;

typedef struct sPlanterDataMaskData
{
	sPlanterIndividualLinesData asLineStatus[CAN_bNUM_DE_LINHAS];
	uint32_t dProductivity;
	uint32_t dWorkedTime;
	uint32_t dTotalSeeds;
	uint32_t dPartPopSemPerUnit;
	uint32_t dPartPopSemPerHa;
	uint32_t dWorkedAreaMt;
	uint32_t dWorkedAreaHa;
	uint32_t dTotalMt;
	uint32_t dTotalHa;
	uint32_t dSpeedKm;
	uint32_t dSpeedHa;
	uint32_t dTEV;
	uint32_t dMTEV;
	uint32_t dMaxSpeed;
} sPlanterDataMaskData;

typedef struct sPlanterIndividualLines
{
	sNumberVariableObj* psLineAverage;
	sNumberVariableObj* psLineSemPerUnit;
	sNumberVariableObj* psLineSemPerHa;
	sNumberVariableObj* psLineTotalSeeds;
} sPlanterIndividualLines;

typedef struct sPlanterDataMask
{
	sPlanterIndividualLines* psLineStatus;
	sNumberVariableObj* psProductivity;
	sNumberVariableObj* psWorkedTime;
	sNumberVariableObj* psTotalSeeds;
	sNumberVariableObj* psPartPopSemPerUnit;
	sNumberVariableObj* psPartPopSemPerHa;
	sNumberVariableObj* psWorkedAreaMt;
	sNumberVariableObj* psWorkedAreaHa;
	sNumberVariableObj* psTotalMt;
	sNumberVariableObj* psTotalHa;
	sNumberVariableObj* psSpeedKm;
	sNumberVariableObj* psSpeedHa;
	sNumberVariableObj* psTEV;
	sNumberVariableObj* psMTEV;
	sNumberVariableObj* psMaxSpeed;
} sPlanterDataMask;

typedef struct sTestModeDataMaskData
{
	tsAcumulados sAccumulated;
	uint32_t dInstalledSensors;
	uint32_t dConfiguredSensors;
} sTestModeDataMaskData;

typedef struct sTestModeDataMask
{
	sNumberVariableObj* psSeedsCount;
	sNumberVariableObj* pdInstalledSensors;
	sNumberVariableObj* pdConfiguredSensors;
} sTestModeDataMask;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#endif /* APP_COMMON_APP_COMMON_ISOBUS_APP_H_ */
