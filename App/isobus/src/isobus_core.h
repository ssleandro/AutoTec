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

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

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
	DATA_MASK_CONFIGURATION = 0x5000,
	DATA_MASK_INSTALLATION,
	DATA_MASK_PLANTER,
	DATA_MASK_TEST_MODE,
	DATA_MASK_TRIMMING,
	DATA_MASK_SYSTEM,
	DATA_MASK_INVALID
} eDataMask;

typedef enum {
	STATUS_INSTALL_INSTALLING 	 = 0x0E,
	STATUS_INSTALL_INSTALLED 	 = 0x0A,
	STATUS_INSTALL_INSTALL_ERROR = 0x0C,
	STATUS_INSTALL_NONE			 = 0xFF,
	STATUS_INSTALL_INVALID
} eInstallationStatus;

typedef enum {
	STATUS_TRIMMING_NOT_TRIMMED = 0x0A,
	STATUS_TRIMMING_TRIMMED		= 0x0C,
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

typedef struct sConfigurationDataMask {
	eSelectedLanguage eLanguage;
	eSelectedUnitMeasurement eUnit;
	uint32_t* dVehicleID;
	eAreaMonitor eMonitor;
	float* fSeedsPerMeter;
	uint8_t* bNumOfRows;
	float* fImplementWidth;
	float* fEvaluationDistance;
	uint8_t* fTolerance;
	float* fMaxSpeed;
	eAlternateRows eAlterRows;
	eAlternatedRowsType eAltType;
} sConfigurationDataMask;

typedef struct sNumberVariableObj {
	uint16_t wObjID;
	uint32_t dValue;
	float fValue;
} sNumberVariableObj;

#define N_NUMBER_VARIABLE_OBJECTS 7

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
