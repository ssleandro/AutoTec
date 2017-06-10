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
	DATA_MASK_CONFIGURATION = 0x5000,
	DATA_MASK_INSTALLATION,
	DATA_MASK_PLANTER,
	DATA_MASK_TEST_MODE,
	DATA_MASK_TRIMMING,
	DATA_MASK_SYSTEM,
	ALARM_MASK_CONFIRM_INSTALLATION = 0x50F0,
	DATA_MASK_INVALID
} eIsobusMask;

typedef enum
{
	STATUS_INSTALL_WAITING = 0xE6,
	STATUS_INSTALL_INSTALLING = 0x0E,
	STATUS_INSTALL_INSTALLED = 0x0A,
	STATUS_INSTALL_INSTALL_ERROR = 0x0C,
	STATUS_INSTALL_NONE = 0x07,
	STATUS_INSTALL_INVALID
} eInstallationStatus;

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

typedef struct sConfigurationData
{
	eSelectedLanguage eLanguage;
	eSelectedUnitMeasurement eUnit;
	uint32_t dVehicleID;
	eAreaMonitor eMonitorArea;
	uint16_t wSeedRate;
	uint8_t bNumOfRows;
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
	eAreaMonitor eMonitor;
	uint32_t* wSeedRate;
	uint8_t* bNumOfRows;
	uint32_t* wImplementWidth;
	uint32_t* wEvaluationDistance;
	uint32_t* wDistBetweenLines;
	uint8_t* bTolerance;
	float* fMaxSpeed;
	eAlternateRows eAlterRows;
	eAlternatedRowsType eAltType;
} sConfigurationDataMask;

typedef enum
{
	STATUS_TRIMMING_NOT_TRIMMED = 0x0A,
	STATUS_TRIMMING_TRIMMED = 0x0C,
	STATUS_TRIMMING_NONE = 0xFF,
	STATUS_TRIMMING_INVALID,
} eTrimmingStatus;

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

typedef struct sPlantingVariables
{
	sNumberVariableObj* const psNumberVariable;
	uint8_t bNumOfVariables;
} sPlantingVariables;

typedef struct sBarGraphStatus
{
	uint16_t wIncBarID;
	uint16_t wDecBarID;
	uint16_t wIncOutputNumID;
	uint16_t wDecOutputNumID;
	int8_t bValue;
} sBarGraphStatus;

typedef struct sPlanterDataMask
{
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

typedef struct sTrimmingStatus
{
	sFillAttributesObj* pFillAtributte;
	uint8_t bNumOfSensor;
} sTrimmingStatus;

typedef struct sTrimmingDataMask
{
	sTrimmingStatus* psTrimmedLines;
} sTrimmingDataMask;

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
