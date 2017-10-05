/****************************************************************************
 * Title                 :   isobus_core Source File
 * Filename              :   isobus_core.c
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
 *    Date    Version        Author              Description
 *  01/07/16   1.0.0      Henrique Reis    isobus_core.c created.
 *
 *****************************************************************************/
/** @file   isobus_core.c
 *  @brief
 *
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "isobus_core.h"
#include "isobus_tools.h"
#include "debug_tool.h"
#include "iso11783.h"
#include "M2GPlus.c.h"
#include "../../isobus/config/isobus_config.h"
#include "isobus_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of SENSOR queue
#define QUEUE_SIZEOFISOBUS (32)

#define THIS_MODULE MODULE_ISOBUS

#define ISO_FLAG_STATE_RUNNING 0x00800000

#define TO_UPPER_INDEX(index) ((0x8100 + index) - 0x8000)
#define IGNORE_LINE_GET_LINE_NUMBER_FROM_ID(id) ((uint8_t)((id - NV_IND_LINE_IGNORE_L01) + 1))
#define IGNORE_LINE_GET_RECT_ID_FROM_NV_ID(id) ((uint8_t)(id - NV_IND_LINE_IGNORE_L01) + RT_PLANT_L01)
#define IGNORE_LINE_GET_BG_UP_ID_FROM_NV_ID(id) ((uint8_t)((id - NV_IND_LINE_IGNORE_L01)*2) + BG_PLANT_UP_L01)
#define IGNORE_LINE_GET_BG_DOWN_ID_FROM_NV_ID(id) ((uint8_t)((id - NV_IND_LINE_IGNORE_L01)*2) + BG_PLANT_DOWN_L01)

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static eAPPError_s eError;                   		//!< Error variable

extern osFlagsGroupId UOS_sFlagSis;

static sNumberVariableObj asNumVarObjects[ISO_NUM_NUMBER_VARIABLE_OBJECTS];
static sFillAttributesObj asNumFillAttributesObjects[ISO_NUM_FILL_ATTRIBUTES_OBJECTS];
static sInputListObj asConfigInputList[ISO_NUM_INPUT_LIST_OBJECTS];
static eLineAlarm aeLineAlarmStatus[CAN_bNUM_DE_LINHAS];
static uint8_t bClearAlarmLineX;
static bool bKeepLineHighPrioAlarm = false;
static bool bKeepLineMediumPrioAlarm = false;
static bool bHighPrioAudioInProcess = false;
static bool bMediumPrioAudioInProcess = false;
static uint8_t bPasswsNumDigits;
static uint8_t bLanguageChRcv = 0;

// Installation
static eInstallationStatus eSensorsIntallStatus[CAN_bNUM_DE_LINHAS];
static sInstallSensorStatus sInstallStatus;
eInstallationStatus InstallationStatus[36];

static sIgnoreLineStatus sIgnoreStatus;
static sLanguageCommandData sCommandLanguage;

static sTrimmingState sTrimmState = {
		.eTrimmState = TRIMMING_NOT_TRIMMED,
		.eNewTrimmState = TRIMMING_NOT_TRIMMED,
};

static sUpdatePlanterMaskStates sUptPlanterMask = {
		.eUpdateState = UPDATE_PLANTER_NO_ALARM,
		.eAlarmEvent = EVENT_GUI_ALARM_TOLERANCE,
};

static sConfigurationDataMask sConfigDataMask =
	{
		.eLanguage = (eSelectedLanguage*)(&asConfigInputList[0].bSelectedIndex),
		.eUnit = (eSelectedUnitMeasurement*)(&asConfigInputList[1].bSelectedIndex),
		.dVehicleID = &(asNumVarObjects[38].dValue),
		.eMonitor = (eAreaMonitor*)(&asConfigInputList[2].bSelectedIndex),
		.wSeedRate = &(asNumVarObjects[40].dValue),
		.bNumOfRows = (uint8_t*)(&asNumVarObjects[41].dValue),
		.eCentralRowSide = (eCentralRowSide*)(&asConfigInputList[3].bSelectedIndex),
		.wDistBetweenLines = &(asNumVarObjects[42].dValue),
		.wEvaluationDistance = &(asNumVarObjects[43].dValue),
		.bTolerance = (uint8_t*)(&asNumVarObjects[44].dValue),
		.fMaxSpeed = &(asNumVarObjects[45].fValue),
		.wImplementWidth = &(asNumVarObjects[39].dValue),
		.eAlterRows = (eAlternateRows*)(&asConfigInputList[4].bSelectedIndex),
		.eAltType = (eAlternatedRowsType*)(&asConfigInputList[5].bSelectedIndex)
	};

static sTestModeDataMask sTestDataMask =
	{
		.psSeedsCount = &(asNumVarObjects[0]),
		.pdInstalledSensors = &(asNumVarObjects[36]),
		.pdConfiguredSensors = &(asNumVarObjects[37])
	};

static sPlanterIndividualLines sPlanterLines =
	{
		.psLineAverage = &(asNumVarObjects[46]),
		.psLineSemPerUnit = &(asNumVarObjects[118]),
		.psLineSemPerHa = &(asNumVarObjects[154]),
		.psLineTotalSeeds = &(asNumVarObjects[190]),
		.peLineAlarmStatus = &(aeLineAlarmStatus[0]),
	};

const sPlanterDataMask sPlanterMask =
	{
		.psLineStatus = &sPlanterLines,
		.psProductivity = &(asNumVarObjects[228]),
		.psWorkedTime = &(asNumVarObjects[227]),
		.psTotalSeeds = &(asNumVarObjects[226]),
		.psPartPopSemPerUnit = &(asNumVarObjects[229]),
		.psPartPopSemPerHa = &(asNumVarObjects[230]),
		.psWorkedAreaMt = &(asNumVarObjects[232]),
		.psWorkedAreaHa = &(asNumVarObjects[231]),
		.psTotalMt = &(asNumVarObjects[234]),
		.psTotalHa = &(asNumVarObjects[233]),
		.psSpeedKm = &(asNumVarObjects[235]),
		.psSpeedHa = &(asNumVarObjects[236]),
		.psTEV = &(asNumVarObjects[237]),
		.psMTEV = &(asNumVarObjects[238]),
		.psMaxSpeed = &(asNumVarObjects[239]),
	};

static eSelectedCANStatus* eCANSelectedStatus = &asConfigInputList[4].bSelectedIndex;
static canStatusStruct_s sCANIsobusStatus;
static canStatusStruct_s sCANSensorStatus;
static GPS_sStatus sISOGPSStatus;

#ifndef UNITY_TEST
DECLARE_QUEUE(IsobusQueue, QUEUE_SIZEOFISOBUS);     //!< Declaration of Interface Queue
CREATE_SIGNATURE(Isobus);//!< Signature Declarations
#endif

CREATE_CONTRACT(Isobus);                            //!< Create contract for isobus msg publication

/*****************************
 * Local messages queue
 *****************************/
CREATE_LOCAL_QUEUE(PublishQ, event_e, 32)
CREATE_LOCAL_QUEUE(WriteQ, ISOBUSMsg, 64)
CREATE_LOCAL_QUEUE(ManagementQ, ISOBUSMsg, 64)
CREATE_LOCAL_QUEUE(UpdateQ, event_e, 32)
CREATE_LOCAL_QUEUE(TranspProtocolQ, ISOBUSMsg, 64)

/*****************************
 * Local flag group
 *****************************/
osFlagsGroupId ISO_sFlags;

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
ISOBUS_MODULES
	};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(ISOPUB);//!< WDT pointer flag
WATCHDOG_CREATE(ISORCV);//!< WDT pointer flag
WATCHDOG_CREATE(ISOWRT);//!< WDT pointer flag
WATCHDOG_CREATE(ISOMGT);//!< WDT pointer flag
WATCHDOG_CREATE(ISOUPDT);//!< WDT pointer flag
WATCHDOG_CREATE(ISOTPT);//!< WDT pointer flag
uint8_t bISOPUBThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISORCVThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISOWRTThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISOMGTThreadArrayPosition = 0;                    	//!< Thread position in array
uint8_t bISOUPDTThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bISOTPTThreadArrayPosition = 0;                    //!< Thread position in array

osThreadId xManagementThreadId;                           	// Holds the BootThreadId
osThreadId xBootThreadId;                               	// Holds the AuxBootThreadId
osThreadId xUpdatePoolThreadId;							   	// Holds the UpdatePoolThreadId

eBootStates eCurrState;

// Holds the current module state
eModuleStates eModCurrState;
eIsobusMask eCurrentMask = DATA_MASK_INSTALLATION;
eIsobusMask ePrevMask = DATA_MASK_INSTALLATION;
eClearCounterStates ePlanterCounterCurrState = CLEAR_TOTALS_IDLE;
eClearSetupStates eClearSetupCurrState = CLEAR_SETUP_IDLE;
eChangeTrimmingState eChangeTrimmCurrState = TRIMM_CHANGE_IDLE;
eIsobusMask eConfigMaskFromX;
eIsobusMask ePasswdMaskFromX;
eTreatPasswordState ePasswordManager = PASSWD_IDLE;
uint32_t wPubPasswd = 0;

bool bCfgClearTotals = false;
bool bCfgClearSetup = false;
bool bCOPlanterLineInfo = true;
bool bCOPlanterSpeedInfo = false;

VTStatus sVTCurrentStatus;                          //!< Holds the current VT status
peripheral_descriptor_p pISOHandle = NULL;          //!< ISO Handler

// Mutex to VT status control structure
CREATE_MUTEX(MTX_VTStatus);
CREATE_MUTEX(ISO_UpdateMask);

extern unsigned int POOL_SIZE;
extern unsigned int PT_PACKAGE_SIZE;
extern unsigned int EN_PACKAGE_SIZE;
extern unsigned int ES_PACKAGE_SIZE;
extern const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_en[];
extern const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_pt[];
extern const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_es[];

CREATE_TIMER(AlarmTimeoutTimer, ISO_vTimerCallbackAlarmTimeout);
CREATE_TIMER(WSMaintenanceTimer, ISO_vTimerCallbackWSMaintenance);
CREATE_TIMER(IsobusCANStatusTimer, ISO_vTimerCallbackIsobusCANStatus);

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void ISO_vUpdateConfigData (sConfigurationData *psCfgDataMask);
void ISO_vUpdateTestModeData (event_e eEvt, void* vPayload);
void ISO_vUpdatePlanterMaskData (sPlanterDataMaskData *psPlanterData);
void ISO_vUpdatePlanterDataMaskLines (void);
void ISO_vTreatUpdateReplaceSensorEvent (tsPubSensorReplacement);
void ISO_vTreatUpdateDataEvent (event_e ePubEvt);
void ISO_vEnableDisableObjCommand (uint16_t wObjID, bool bIsEnable);
void ISO_vChangeAttributeCommand (uint16_t wObjID, uint8_t bObjAID, uint32_t dNewValue);
void ISO_vChangeSoftKeyMaskCommand (eIsobusMask eMask, eIsobusMaskType eMaskType, eIsobusSoftKeyMask eNewSoftKeyMask);
void ISO_vChangeNumericValue (uint16_t wOutputNumberID, uint32_t dNumericValue);
void ISO_vHideShowContainerCommand (uint16_t wObjID, bool bShow);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * ISO_WDTData (uint8_t * pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void ISO_vDetectThread (thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
	*bPosition = 0;
	while (THREADS_THREAD(*bPosition)!= (os_pthread)pFunc)
	{
		(*bPosition)++;
	}
	*flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

#ifndef UNITY_TEST
/******************************************************************************
 * Function : ISO_vCreateThread(const Threads_t sSensorThread )
 *//**
 * \b Description:
 *
 * Function used to create threads.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: Threads created
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * osThreadDef_t sKEYBackThread;
 *
 * sKEYBackThread.name = "KEYBackThread";
 * sKEYBackThread.stacksize = 500;
 * sKEYBackThread.tpriority = osPriorityNormal;
 * sKEYBackThread.pthread = SEN_vKEYBackLightThread;
 *
 * osThreadCreate(&sKEYBackThread, NULL);
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vCreateThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *******************************************************************************/
static void ISO_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

	// Holds the ManagementThreadId, note that ManagementThreadId is the third position at THREADS_THISTHREAD array
	(sThread.thisWDTPosition == 3) ? xManagementThreadId = xThreads : xManagementThreadId;

	// Holds the BootThreadId, note that BootThreadId is the sixth position at THREADS_THISTHREAD array
	(sThread.thisWDTPosition == 6) ? xBootThreadId = xThreads : xBootThreadId;

	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}
#endif

/******************************************************************************
 * Function : ISO_eInitIsobusPublisher(void)
 *//**
 * \b Description:
 *
 * This routine prepares the contract and message that the ISO_vIsobusPublishThread thread
 * will publish to the broker.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Called from ISO_vIsobusPublishThread
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread, ISO_vIsobusPublishThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
eAPPError_s ISO_eInitIsobusPublisher (void)
{
	/* Check if handler is already enabled */
	if (pISOHandle == NULL)
	{
		return APP_ERROR_ERROR;
	}

	//Prepare Default Contract/Message
	MESSAGE_HEADER(Isobus, 1, ISOBUS_DEFAULT_MSGSIZE, MT_ARRAYBYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(Isobus, 1, THIS_MODULE, TOPIC_ISOBUS);

	return APP_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : ISO_vIsobusPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the ISOBUS topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vDiagnosticThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusPublishThread (void const *argument)
{
	osEvent evt;
	event_e ePubEvt;

	INITIALIZE_LOCAL_QUEUE(PublishQ);           //!< Initialise message queue to publish thread

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus Publish Thread Created");
#endif

	ISO_vDetectThread(&WATCHDOG(ISOPUB), &bISOPUBThreadArrayPosition, (void*)ISO_vIsobusPublishThread);
	WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bISOPUBThreadArrayPosition));           //Task created, inform core

	WATCHDOG_STATE(ISOPUB, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(ISOPUB, WDT_SLEEP);
		evt = RECEIVE_LOCAL_QUEUE(PublishQ, &ePubEvt, osWaitForever);
		WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

		if (evt.status == osEventMessage)
		{
			switch (ePubEvt)
			{
				case EVENT_ISO_UPDATE_CURRENT_DATA_MASK:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_UPDATE, &eCurrentMask);
					break;
				}
				case EVENT_ISO_INSTALLATION_REPEAT_TEST:
				case EVENT_ISO_INSTALLATION_ERASE_INSTALLATION:
				case EVENT_ISO_CONFIG_CANCEL_UPDATE_DATA:
				case EVENT_ISO_PLANTER_CLEAR_COUNTER_TOTAL:
				case EVENT_ISO_PLANTER_CLEAR_COUNTER_SUBTOTAL:
				case EVENT_ISO_AREA_MONITOR_PAUSE:
				case EVENT_ISO_INSTALLATION_REPLACE_SENSOR:
				case EVENT_ISO_INSTALLATION_CONFIRM_REPLACE_SENSOR:
				case EVENT_ISO_INSTALLATION_CANCEL_REPLACE_SENSOR:
				case EVENT_ISO_CONFIG_GET_MEMORY_USED:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, NULL);
					break;
				}
				case EVENT_ISO_INSTALLATION_CONFIRM_INSTALLATION:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_CLEAR, NULL);
					break;
				}
				case EVENT_ISO_CONFIG_UPDATE_DATA:
				{
					ISO_vTreatUpdateDataEvent(ePubEvt);
					break;
				}
				case EVENT_ISO_PLANTER_IGNORE_SENSOR:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, &sIgnoreStatus);
					break;
				}
				case EVENT_ISO_TRIMMING_TRIMMING_MODE_CHANGE:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, &sTrimmState);
					break;
				}
				case EVENT_ISO_ALARM_CLEAR_ALARM:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, &bClearAlarmLineX);
					break;
				}
				case EVENT_ISO_CONFIG_CHECK_PASSWORD:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, &wPubPasswd);
					break;
				}
				case EVENT_ISO_CONFIG_CHANGE_PASSWORD:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, &wPubPasswd);
					break;
				}

				case EVENT_ISO_LANGUAGE_COMMAND:
				{
					PUBLISH_MESSAGE(Isobus, ePubEvt, EVENT_SET, &sCommandLanguage);
					break;
				}
				default:
					break;
			}
		}
	}
	osThreadTerminate(NULL);
}
#else
void ISO_vIsobusPublishThread(void const *argument)
{}
#endif

/******************************************************************************
 * Function : ISO_vInitDeviceLayer(uint32_t wSelectedInterface)
 *//**
 * \b Description:
 *
 * This function will init the M2GISOCOMM instance used by the Isobus module.
 * The function parameter indicates the selected interface which will output the
 * messages.
 *
 * PRE-CONDITION: Isobus core initialized
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define UART_IFACE 0x01
 * #define USB_IFACE  0x02
 * #define CAN_IFACE  0x04
 *
 *	if (DIG_vInitDeviceLayer(UART_IFACE) != DEV_ERROR_SUCCESS)
 *		printf("error");
 ~~~~~~~~~~~~~~~
 *
 * @see COM_vCBACommThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 04/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eAPPError_s ISO_vInitDeviceLayer (uint32_t wSelectedInterface)
{
	/*Prepare the device */
	pISOHandle = DEV_open(PERIPHERAL_M2GISOCOMM);
	ASSERT(pISOHandle != NULL);

	/* Set device output to CAN peripheral */
	eError = (eAPPError_s)DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_SET_ACTIVE, (void*)&wSelectedInterface);
	ASSERT(eError == APP_ERROR_SUCCESS);

	/* Configurations for CAN multipacket communications */
	if (wSelectedInterface == ISOBUS_DEV_CAN)
	{
		/* Enables the reception of frames with the ISO_INIT_CAN_ID id number (29 bits) */
		uint32_t wCANInitID = 0x00000000;
		eError = (eAPPError_s)DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_ADD_ALL_CAN_ID, (void*)&wCANInitID);
		ASSERT(eError == APP_ERROR_SUCCESS);
	}
	START_TIMER(IsobusCANStatusTimer, ISO_TIMER_PERIOD_MS_CAN_STATUS);
	return eError;
}

void ISO_vIdentifyEvent (contract_s* contract)
{
	event_e eEvt =  GET_PUBLISHED_EVENT(contract);
	eEventType eEvtType= GET_PUBLISHED_TYPE(contract);
	void * pvPayData = GET_PUBLISHED_PAYLOAD(contract);

	switch (contract->eOrigin)
	{
		case MODULE_GUI:
		{
			switch (eEvt)
			{
				case EVENT_GUI_UPDATE_INSTALLATION_INTERFACE:
				{
					if (memcmp(&eSensorsIntallStatus, (eInstallationStatus*)pvPayData, sizeof(eSensorsIntallStatus)) != 0)
					{
						memcpy(&eSensorsIntallStatus, (eInstallationStatus*)pvPayData, sizeof(eSensorsIntallStatus));
						PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					}
					break;
				}
				case EVENT_GUI_UPDATE_PLANTER_INTERFACE:
				{
					ISO_vUpdatePlanterMaskData((sPlanterDataMaskData*)pvPayData);
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
				{
					ISO_vUpdateTestModeData(eEvt, pvPayData);
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_TRIMMING_INTERFACE:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_SYSTEM_GPS_INTERFACE:
				{
					if (pvPayData == NULL)
						return;

					sISOGPSStatus = *((GPS_sStatus*) pvPayData);
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_SYSTEM_CAN_INTERFACE:
				{
					canStatusStruct_s *psCANSensorStatus = pvPayData;
					memcpy(&sCANSensorStatus, psCANSensorStatus, sizeof(canStatusStruct_s));
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_SYSTEM_SENSORS_INTERFACE:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_INSTALLATION_FINISH:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION:
				{
					ISO_vUpdateTestModeData(eEvt, pvPayData);
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_CONFIG:
				{
					ISO_vUpdateConfigData((sConfigurationData *)pvPayData);
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_CHANGE_ACTIVE_MASK_CONFIG_MASK:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_ALARM_NEW_SENSOR:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_ALARM_DISCONNECTED_SENSOR:
				case EVENT_GUI_ALARM_LINE_FAILURE:
				case EVENT_GUI_ALARM_SETUP_FAILURE:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_ALARM_EXCEEDED_SPEED:
				case EVENT_GUI_ALARM_GPS_FAILURE:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_ALARM_TOLERANCE:
				{
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_CONFIG_CHECK_PASSWORD_ACK:
				{
					if (ePasswordManager == PASSWD_ENTER_PASSWORD)
					{
						ePasswordManager = PASSWD_ACCEPTED;
					} else if (ePasswordManager == PASSWD_CHANGE_PASSWD_CURRENT_PASSWD)
					{
						ePasswordManager = PASSWD_CHANGE_PASSWD_NEW_PASSWD;
					}
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_CONFIG_CHECK_PASSWORD_NACK:
				{
					if (ePasswordManager == PASSWD_ENTER_PASSWORD)
					{
						ePasswordManager = PASSWD_NOT_ACCEPTED;
					} else if (ePasswordManager == PASSWD_CHANGE_PASSWD_CURRENT_PASSWD)
					{
						ePasswordManager = PASSWD_CHANGE_NOT_ACCEPTED;
					}
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_CONFIG_CHANGE_PASSWORD_ACK:
				{
					if (ePasswordManager == PASSWD_CHANGE_PASSWD_NEW_PASSWD)
					{
						ePasswordManager = PASSWD_CHANGE_ACCEPTED;
					}
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_CONFIG_CHANGE_PASSWORD_NACK:
				{
					if (ePasswordManager == PASSWD_CHANGE_PASSWD_NEW_PASSWD)
					{
						ePasswordManager = PASSWD_CHANGE_NOT_ACCEPTED;
					}
					PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_REPLACE_SENSOR:
				{
					tsPubSensorReplacement *psReplacement = pvPayData;
					ISO_vTreatUpdateReplaceSensorEvent(*psReplacement);
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void ISO_vIsobusThread (void const *argument)
{
	osEvent evt;
	osStatus status;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus Thread Created");
#endif
	eModCurrState = BOOT;       // Module is in boot process

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(IsobusQueue);

	INITIALIZE_MUTEX(MTX_VTStatus);
	INITIALIZE_MUTEX(ISO_UpdateMask);

	INITIALIZE_TIMER(IsobusCANStatusTimer, osTimerPeriodic);

	status = osFlagGroupCreate(&ISO_sFlags);
	ASSERT(status == osOK);

	/* Init M2GISOCOMM device for output */
	ISO_vInitDeviceLayer(ISO_INITIAL_IO_IFACE);

	ISO_eInitIsobusPublisher();

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromIsobusID = (osThreadId)argument;
	osSignalSet(xMainFromIsobusID, MODULE_ISOBUS);

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP, false, false, osWaitForever);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		ISO_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	/* Prepare the signature - struture that notify the broker about subscribers */
	SIGNATURE_HEADER(Isobus, THIS_MODULE, TOPIC_GUI, IsobusQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(Isobus), 0) == osOK);

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on ISOBUS topic */
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		evt = RECEIVE(IsobusQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

		if (evt.status == osEventMessage)
		{
			ISO_vIdentifyEvent(GET_CONTRACT(evt));
		}
	}

	/* Unreachable */
	osThreadSuspend(NULL);
}
#else
void ISO_vIsobusThread (void const *argument)
{}
#endif

/* *
 * Get the PGN from an message
 * */
uint32_t ISO_wGetPGN (ISOBUSMsg* RcvMsg)
{
	return (RcvMsg->PF < 240) ? (((RcvMsg->frame.id >> 8) & 0x3FFFF) - RcvMsg->PS) : ((RcvMsg->frame.id >> 8) & 0x3FFFF);
}

void ISO_vIsobusDispatcher (ISOBUSMsg* RcvMsg)
{
	uint32_t wRcvMsgPGN = ISO_wGetPGN(RcvMsg);

	if ((RcvMsg->PF < 240) && (wRcvMsgPGN != REQUEST_PGN_MSG_PGN))  // Destination PGN
	{
		if (RcvMsg->PS == M2G_SOURCE_ADDRESS)
		{
			if (RcvMsg->SA == VT_ADDRESS)
			{
				switch (wRcvMsgPGN)
				{
					case ETP_DATA_TRANSFER_PGN:
					case ETP_CONN_MANAGE_PGN:
					case VT_TO_ECU_PGN:
					case PROCESS_DATA_PGN:
					case ACKNOWLEDGEMENT_PGN:
					case TP_DATA_TRANSFER_PGN:
					case TP_CONN_MANAGE_PGN:
					case ADDRESS_CLAIM_PGN:
					case PROPRIETARY_A_PGN:
					case PROPRIETARY_A2_PGN:
					{
						WATCHDOG_STATE(ISORCV, WDT_SLEEP);
						PUT_LOCAL_QUEUE(ManagementQ, *RcvMsg, osWaitForever);
						WATCHDOG_STATE(ISORCV, WDT_ACTIVE);
						break;
					}
					default:
						break;
				}
			}
			else
			{
			}
		}
		else if (RcvMsg->PS == BROADCAST_ADDRESS)
		{
			if (RcvMsg->SA == VT_ADDRESS)
			{
				switch (wRcvMsgPGN)
				{
					case VT_TO_ECU_PGN:
					{
						WATCHDOG_STATE(ISORCV, WDT_SLEEP);
						PUT_LOCAL_QUEUE(ManagementQ, *RcvMsg, osWaitForever);
						WATCHDOG_STATE(ISORCV, WDT_ACTIVE);
						break;
					}
					case ECU_TO_GLOBAL_PGN:
						break;
					default:
						break;
				}
			}
			else
			{
			}
		}
	}
	else if (RcvMsg->PF >= 240)        // No-Destination PGN
	{
		switch (wRcvMsgPGN)
		{
			case LANGUAGE_PGN:
			{
				if (eCurrState == BOOT_COMPLETED)
					eModCurrState = UPDATING_LANGUAGE;
				bLanguageChRcv = 1;
				WATCHDOG_STATE(ISORCV, WDT_SLEEP);
				PUT_LOCAL_QUEUE(ManagementQ, *RcvMsg, osWaitForever);
				WATCHDOG_STATE(ISORCV, WDT_ACTIVE);
				break;
			}
			default:
				break;
		}
	}
}

/******************************************************************************
 * Function : ISO_vIsobusPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will send to the responsible thread on module.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vIsobusThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusRecvThread (void const *argument)
{
	uint8_t bIterator;
	uint8_t bRecvMessages = 0;      //!< Lenght (messages) received
	uint32_t dTicks;
	ISOBUSMsg asPayload[64];   //!< Buffer to hold the contract and message data

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus Recv Thread Created");
#endif

	ISO_vDetectThread(&WATCHDOG(ISORCV), &bISORCVThreadArrayPosition, (void*)ISO_vIsobusRecvThread);
	WATCHDOG_STATE(ISORCV, WDT_ACTIVE);

	osThreadId xIsoMainID = (osThreadId)argument;
	osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISORCVThreadArrayPosition));   //Task created, inform core

	dTicks = osKernelSysTick();

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(ISORCV, WDT_SLEEP);
		osDelayUntil(&dTicks, 25);
		bRecvMessages = DEV_read(pISOHandle, &asPayload[0].frame, ARRAY_SIZE(asPayload));
		WATCHDOG_STATE(ISORCV, WDT_ACTIVE);

		if (bRecvMessages)
		{
			for (bIterator = 0; bIterator < bRecvMessages; bIterator++)
			{
				// Call an function to treat ISOBUS messages
				ISO_vIsobusDispatcher(&asPayload[bIterator]);
			}
		}
	}
	osThreadTerminate(NULL);
}
#else
void ISO_vIsobusRecvThread(void const *argument)
{}
#endif

/******************************************************************************
 * Function : ISO_vIsobusWriteThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will write the incoming messages to the device.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vIsobusThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusWriteThread (void const *argument)
{
	ISOBUSMsg recv;
	eAPPError_s eError;

	INITIALIZE_LOCAL_QUEUE(WriteQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus Write Thread Created");
#endif

	ISO_vDetectThread(&WATCHDOG(ISOWRT), &bISOWRTThreadArrayPosition, (void*)ISO_vIsobusWriteThread);
	WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);

	osThreadId xIsoMainID = (osThreadId)argument;
	osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOWRTThreadArrayPosition));   //Task created, inform core

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(ISOWRT, WDT_SLEEP);
		osEvent evtPub = RECEIVE_LOCAL_QUEUE(WriteQ, &recv, osWaitForever);   // Wait
		WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);

		if (evtPub.status == osEventMessage)
		{
			eError = (eAPPError_s)DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_CHANGE_SEND_ID, (void*)&(recv.frame).id);
			ASSERT(eError == APP_ERROR_SUCCESS);

			if (eError == APP_ERROR_SUCCESS)     // wSendCANID changed
			{
				WATCHDOG_STATE(ISOWRT, WDT_SLEEP);
				DEV_write(pISOHandle, &((recv.frame).data[0]), (recv.frame).dlc);
				WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);
			}
		}
	}
	osThreadTerminate(NULL);
}
#else
void ISO_vIsobusWriteThread(void const *argument)
{}
#endif

void ISO_vTransportProtocolManagement (eBootStates eIfAck, eBootStates eIfNack)
{
	ISOBUSMsg RcvMsg;

	WATCHDOG_STATE(ISOTPT, WDT_SLEEP);
	osEvent evtPub = RECEIVE_LOCAL_QUEUE(TranspProtocolQ, &RcvMsg, osWaitForever);
	WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);

	if (evtPub.status == osEventMessage)
	{
		switch (ISO_wGetPGN(&RcvMsg))
		{
			case TP_CONN_MANAGE_PGN:
			{
				switch (RcvMsg.B1)
				{
					case TP_CM_CTS:
						ISO_vSendBytesToVT(RcvMsg.B2, RcvMsg.B3, TRANSPORT_PROTOCOL);
						break;
					case TP_EndofMsgACK:
						eCurrState = eIfAck;
						break;
					case TP_Conn_Abort:
						eCurrState = eIfNack;
						break;
					case TP_BAM:
					case TP_CM_RTS:
					default:
						break;
				}
				break;
			}
			case ETP_CONN_MANAGE_PGN:
			{
				switch (RcvMsg.B1)
				{
					case ETP_CM_CTS:
						// Send DPO message
						ISO_vSendETP_CM_DPO(RcvMsg.B2, (RcvMsg.B3 | (RcvMsg.B4 << 8) | (RcvMsg.B5 << 16)));
						ISO_vSendBytesToVT(RcvMsg.B2, (RcvMsg.B3 | (RcvMsg.B4 << 8) | (RcvMsg.B5 << 16)),
										   EXTENDED_TRANSPORT_PROTOCOL);
						break;
					case ETP_CM_EOMA:
						eCurrState = eIfAck;
						break;
					case ETP_Conn_Abort:
						eCurrState = eIfNack;
						break;
					case ETP_CM_DPO:
					case ETP_CM_RTS:
					default:
						break;
				}
				break;
			}
			default:
				break;
		}
	}
}

void ISO_vObjectPoolMemoryRequired (void)
{
	switch (sCommandLanguage.eLanguage) {
		case LANGUAGE_PORTUGUESE:
		{
			ISO_vSendGetMemory(POOL_SIZE + PT_PACKAGE_SIZE);
			break;
		}
		case LANGUAGE_SPANISH:
		case LANGUAGE_RUSSIAN:
		case LANGUAGE_ENGLISH:
		default:
		{
			ISO_vSendGetMemory(POOL_SIZE + EN_PACKAGE_SIZE);
			break;
		}
	}
}

void ISO_vObjectPoolRequestToSend (void)
{
	switch (sCommandLanguage.eLanguage) {
		case LANGUAGE_PORTUGUESE:
		{
			ISO_vSendRequestToSend(PT_PACKAGE_SIZE);
			ISO_vInitPointersToTranfer(isoOP_M2GPlus_pt, PT_PACKAGE_SIZE);
			break;
		}
		case LANGUAGE_SPANISH:
		{
			ISO_vSendRequestToSend(ES_PACKAGE_SIZE);
			ISO_vInitPointersToTranfer(isoOP_M2GPlus_es, ES_PACKAGE_SIZE);
			break;
		}
		case LANGUAGE_RUSSIAN:
		case LANGUAGE_ENGLISH:
		default:
		{
			ISO_vSendRequestToSend(EN_PACKAGE_SIZE);
			ISO_vInitPointersToTranfer(isoOP_M2GPlus_en, EN_PACKAGE_SIZE);
			break;
		}
	}
}

void ISO_vTimerCallbackWSMaintenance (void const *arg)
{
	ISO_vSendWorkingSetMaintenance(false);
}

#ifndef UNITY_TEST
void ISO_vIsobusTransportProtocolThread (void const *argument)
{
	ISOBUSMsg RcvMsg;
	bool bUpdateObjectPool = false;

	INITIALIZE_LOCAL_QUEUE(TranspProtocolQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus Transport Protocol Thread Created");
#endif

	ISO_vDetectThread(&WATCHDOG(ISOTPT), &bISOTPTThreadArrayPosition, (void*)ISO_vIsobusTransportProtocolThread);
	WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);

	osThreadId xIsoMainID = (osThreadId)argument;
	osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOTPTThreadArrayPosition));     //Task created, inform core

	INITIALIZE_TIMER(WSMaintenanceTimer, osTimerPeriodic);

	// Inform Management Thread that BootThread already start
	WATCHDOG_STATE(ISOTPT, WDT_SLEEP);
	osSignalSet(xManagementThreadId, 0xFF);
	WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);

	// Wait for an VT status message
	WATCHDOG_STATE(ISOTPT, WDT_SLEEP);
	osSignalWait(WAIT_GLOBAL_VT_STATUS, osWaitForever);
	WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);

	// Send a request address claim message
	ISO_vSendRequest(ADDRESS_CLAIM_PGN);

	// Send a address claim message
	ISO_vSendAddressClaimed();

	// Wait for an VT status message
	eCurrState = WAIT_VT_STATUS;

	while (1)
	{
		switch (eModCurrState)
		{
			case BOOT:
			{
				while (eCurrState != BOOT_COMPLETED)
				{
					WATCHDOG_STATE(ISOTPT, WDT_SLEEP);
					osSignalWait(eCurrState, osWaitForever);
					WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);

					switch (eCurrState)
					{
						case WAIT_VT_STATUS:
						{
							// Send a working set master message
							ISO_vSendWorkingSetMaster();
							// Send a working set maintenance (first)
							// Send a proprietary A message
							ISO_vSendProprietaryA();
							// Send a command language message
							ISO_vSendRequest(LANGUAGE_PGN);
							// Send a get hardware message
							ISO_vSendGetHardware();
							// Send a proprietary A message
							ISO_vSendProprietaryA();
							// Send a proprietary A message
							ISO_vSendProprietaryA();
							// Send a get memory message
							ISO_vObjectPoolMemoryRequired();
							// Send a load version message
							ISO_vSendLoadVersion(OBJECT_POOL_VERSION);

							break;
						}
						case WAIT_LOAD_VERSION:
						{
							// Send a request to send message
							ISO_vSendRequestToSend(POOL_SIZE);
							eCurrState = WAIT_SEND_POOL;
							break;
						}
						case WAIT_SEND_POOL:
						{
							ISO_vInitPointersToTranfer(isoOP_M2GPlus, POOL_SIZE);

							do
							{
								ISO_vTransportProtocolManagement(OBJECT_POOL_SENT, WAIT_VT_STATUS);
							} while (eCurrState != OBJECT_POOL_SENT);

							ISO_vObjectPoolRequestToSend();

							do
							{
								ISO_vTransportProtocolManagement(OBJECT_POOL_LANG_PKG_SENT, WAIT_VT_STATUS);
							} while (eCurrState != OBJECT_POOL_LANG_PKG_SENT);

							ISO_vSendEndObjectPool();
							ISO_vSendWSMaintenancePoolSent();
							ISO_vSendLoadVersion(OBJECT_POOL_VERSION);
//							ISO_vSendStoreVersion(OBJECT_POOL_VERSION);

							eCurrState = OBJECT_POOL_LOADED;
							osSignalSet(xBootThreadId, OBJECT_POOL_LOADED);
							break;
						}
						case OBJECT_POOL_LOADED:
						{
							START_TIMER(WSMaintenanceTimer, ISO_TIMER_PERIOD_MS_WS_MAINTENANCE);

							eCurrState = BOOT_COMPLETED;
							eModCurrState = RUNNING;
							osSignalSet(xUpdatePoolThreadId, ISO_FLAG_STATE_RUNNING);
							osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK);
							break;
						}
						default:
							break;
					} // End of switch statement
				} // End of while
				break;
			}
			case UPDATING_LANGUAGE:
			{
//				ISO_vObjectPoolRequestToSend();
//
//				do
//				{
//					WATCHDOG_STATE(ISOTPT, WDT_SLEEP);
//					osEvent evtPub = RECEIVE_LOCAL_QUEUE(TranspProtocolQ, &RcvMsg, osWaitForever);   // Wait
//					WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);
//
//					if (evtPub.status == osEventMessage)
//					{
//						switch (ISO_wGetPGN(&RcvMsg))
//						{
//							case TP_CONN_MANAGE_PGN:
//							{
//								switch (RcvMsg.B1)
//								{
//									case TP_CM_CTS:
//									ISO_vSendBytesToVT(RcvMsg.B2, RcvMsg.B3, TRANSPORT_PROTOCOL);
//										break;
//									case TP_EndofMsgACK:
//										bUpdateObjectPool = true;
//										break;
//									case TP_Conn_Abort:
//										break;
//									case TP_BAM:
//									case TP_CM_RTS:
//									default:
//										break;
//								}
//								break;
//							}
//							case ETP_CONN_MANAGE_PGN:
//							{
//								switch (RcvMsg.B1)
//								{
//									case ETP_CM_CTS:
//									// Send DPO message
//									ISO_vSendETP_CM_DPO(RcvMsg.B2, (RcvMsg.B3 | (RcvMsg.B4 << 8) | (RcvMsg.B5 << 16)));
//									ISO_vSendBytesToVT(RcvMsg.B2, (RcvMsg.B3 | (RcvMsg.B4 << 8) | (RcvMsg.B5 << 16)),
//													   EXTENDED_TRANSPORT_PROTOCOL);
//										break;
//									case ETP_CM_EOMA:
//										bUpdateObjectPool = true;
//										break;
//									case ETP_Conn_Abort:
//										break;
//									case ETP_CM_DPO:
//									case ETP_CM_RTS:
//									default:
//										break;
//								}
//								break;
//							}
//						}
//					}
//				} while (!bUpdateObjectPool);
//
//				ISO_vSendEndObjectPool();
				osFlagSet(ISO_sFlags, ISO_FLAG_LANGUAGE_UPDATED);
				eModCurrState = RUNNING;
				bUpdateObjectPool = false;
				break;
			}
			default:
				break;
		}

		WATCHDOG_STATE(ISOTPT, WDT_SLEEP);
		osDelay(50);
		WATCHDOG_STATE(ISOTPT, WDT_ACTIVE);
	} // End of while
	osThreadTerminate(NULL);
}
#else
void ISO_vIsobusTransportProtocolThread(void const *argument)
{}
#endif

void ISO_vIsobusUpdateVTStatus (ISOBUSMsg* RcvMsg)
{
	osStatus status;

	if (RcvMsg->B1 != FUNC_VT_STATUS)
	{
		return;
	}

	status = WAIT_MUTEX(MTX_VTStatus, osWaitForever);
	ASSERT(status == osOK);

	sVTCurrentStatus.bActiveWSM = RcvMsg->B2;
	sVTCurrentStatus.wMaskActWS = RcvMsg->B3 | (RcvMsg->B4 << 8);
	sVTCurrentStatus.wSoftActWS = RcvMsg->B5 | (RcvMsg->B6 << 8);
	sVTCurrentStatus.bBusyUpdMask = RcvMsg->B7 & 0x01;
	sVTCurrentStatus.bBusySavMem = RcvMsg->B7 & 0x02;
	sVTCurrentStatus.bBusyExcCmd = RcvMsg->B7 & 0x04;
	sVTCurrentStatus.bBusyExcMac = RcvMsg->B7 & 0x08;
	sVTCurrentStatus.bBusyParsOP = RcvMsg->B7 & 0x10;
	sVTCurrentStatus.bBusyReserv = RcvMsg->B7 & 0x20;
	sVTCurrentStatus.bAuxCtrlAct = RcvMsg->B7 & 0x40;
	sVTCurrentStatus.bVTOutMemory = RcvMsg->B7 & 0x80;
	sVTCurrentStatus.bVTFuncCode = RcvMsg->B8;

	status = RELEASE_MUTEX(MTX_VTStatus);
	ASSERT(status == osOK);
}

void ISO_vTreatLanguageCommandMessage (ISOBUSMsg sRcvMsg)
{
	if ((sRcvMsg.B1 == 'r') && (sRcvMsg.B2 == 'u'))
	{
		sCommandLanguage.eLanguage = LANGUAGE_ENGLISH;
	} else if ((sRcvMsg.B1 == 'e') && (sRcvMsg.B2 == 's'))
	{
		sCommandLanguage.eLanguage = LANGUAGE_ENGLISH;
	} else if ((sRcvMsg.B1 == 'p') && (sRcvMsg.B2 == 't'))
	{
		sCommandLanguage.eLanguage = LANGUAGE_PORTUGUESE;
	} else
	{
		sCommandLanguage.eLanguage = LANGUAGE_ENGLISH;
	}

	switch ((sRcvMsg.B6 & 3)) {
		case 0:
		{
			sCommandLanguage.eUnit = UNIT_INTERNATIONAL_SYSTEM;
			break;
		}
		case 1:
		case 2:
		{
			sCommandLanguage.eUnit = UNIT_IMPERIAL_SYSTEM;
			break;
		}
		default:
			break;
	}
}

void ISO_vTreatBootState (ISOBUSMsg* sRcvMsg)
{
	switch (ISO_wGetPGN(sRcvMsg))
	{
		case VT_TO_ECU_PGN:
		{
			if (sRcvMsg->PS == M2G_SOURCE_ADDRESS)
			{
				switch (sRcvMsg->B1)
				{
					case FUNC_LOAD_VERSION:
						if (sRcvMsg->B6 == 0)
						{
							if (eModCurrState == BOOT)
							{
								eCurrState = OBJECT_POOL_LOADED;
								osSignalSet(xBootThreadId, OBJECT_POOL_LOADED);
							}
						}
						else
						{
							if (eModCurrState == BOOT)
							{
								eCurrState = WAIT_LOAD_VERSION;
								osSignalSet(xBootThreadId, WAIT_LOAD_VERSION);
							}
						}
						break;
					case FUNC_VT_STATUS:
						osSignalSet(xBootThreadId, WAIT_VT_STATUS);
						break;
					default:
						break;
				}
			}
			else if (sRcvMsg->PS == BROADCAST_ADDRESS)
			{
				switch (sRcvMsg->B1)
				{
					case FUNC_VT_STATUS:
						osSignalSet(xBootThreadId, WAIT_GLOBAL_VT_STATUS);
						break;
					default:
						break;
				}
			}
			break;
		}
		case TP_CONN_MANAGE_PGN:
		{
			if (sRcvMsg->PS == M2G_SOURCE_ADDRESS)
			{
				osSignalSet(xBootThreadId, WAIT_SEND_POOL);
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(TranspProtocolQ, *sRcvMsg, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			}
			break;
		}
		case ETP_CONN_MANAGE_PGN:
		{
			if (sRcvMsg->PS == M2G_SOURCE_ADDRESS)
			{
				osSignalSet(xBootThreadId, WAIT_SEND_POOL);
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(TranspProtocolQ, *sRcvMsg, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			}
			break;
		}
		case LANGUAGE_PGN:
		{
			ISO_vTreatLanguageCommandMessage (*sRcvMsg);

			event_e ePubEvt = EVENT_ISO_LANGUAGE_COMMAND;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		default:
			break;
	}
}

void ISO_vInitObjectStruct (void)
{
	// Init number variable objects
	for (int i = 0; i < ARRAY_SIZE(asNumVarObjects); i++)
	{
		asNumVarObjects[i].wObjID = ISO_OBJECT_NUMBER_VARIABLE_ID + i;
	}

	// Initialize fill attributes objects
	for (int i = 0; i < ARRAY_SIZE(asNumFillAttributesObjects); i++)
	{
		asNumFillAttributesObjects[i].wObjID = ISO_OBJECT_FILL_ATTRIBUTES_ID + i;
	}

	// Initialize sensor install status structure
	sInstallStatus.bNumOfSensors = 36;
	sInstallStatus.pFillAttribute = &asNumFillAttributesObjects[0];

	for (int i = 0; i < sInstallStatus.bNumOfSensors; i++)
	{
		if (i < *sConfigDataMask.bNumOfRows)
		{
			sInstallStatus.pFillAttribute[i].bColor = STATUS_INSTALL_INSTALLING;
		}
		else
		{
			sInstallStatus.pFillAttribute[i].bColor = STATUS_INSTALL_NONE;
		}
	}

	// Initialize input list objects
	for (int i = 0; i < ARRAY_SIZE(asConfigInputList); i++)
	{
		asConfigInputList[i].wObjID = ISO_OBJECT_INPUT_LIST_ID + i;
	}

}

// Used to FUNC_CHANGE_NUMERIC_VALUE message
void ISO_vInputNumberVariableValue (uint16_t wObjectID, uint32_t dValue)
{
	uint16_t index;
	for (index = 0; index < ARRAY_SIZE(asNumVarObjects); index++)
	{
		if (asNumVarObjects[index].wObjID == wObjectID)
		{
			asNumVarObjects[index].dValue = dValue;
			asNumVarObjects[index].fValue = GET_FLOAT_VALUE(dValue);
			break;
		}
	}
}

void ISO_vInputIndexListValue (uint16_t wObjectID, uint32_t dValue)
{
	uint16_t index = INPUT_LIST_GET_INDEX_FROM_ID(wObjectID);

	if (index < ARRAY_SIZE(asConfigInputList))
	{
		if (asConfigInputList[index].wObjID == wObjectID)
		{
			asConfigInputList[index].bSelectedIndex = dValue;
		}
	}
}

void ISO_vTreatChangeNumericValueEvent (ISOBUSMsg* sRcvMsg)
{
	if (sRcvMsg == NULL)
		return;

	event_e ePubEvt;
	uint16_t wObjectID = (sRcvMsg->B2 | (sRcvMsg->B3 << 8));
	uint32_t dValue = (sRcvMsg->B5 | (sRcvMsg->B6 << 8) | (sRcvMsg->B7 << 16) | (sRcvMsg->B8 << 24));

	if ((wObjectID >= NV_TEST_MODE_L01) && (wObjectID <= NV_TRIM_RIGHT_SIDE))
	{
		switch (wObjectID)
		{
			case NV_CFG_N_ROWS:
			{
				if ((dValue % 2) != 0)
				{
					ISO_vEnableDisableObjCommand(IL_CFG_CENTER_ROW_SIDE, true);
				} else
				{
					ISO_vEnableDisableObjCommand(IL_CFG_CENTER_ROW_SIDE, false);
				}
				bCfgClearTotals = true;
				bCfgClearSetup = true;
				break;
			}
			case NV_CFG_ROW_SPACING:
			case NV_CFG_IMP_WIDTH:
			{
				bCfgClearTotals = true;
				break;
			}
			case NV_TRIM_NO_TRIMMING:
			{
				sTrimmState.eNewTrimmState = TRIMMING_NOT_TRIMMED;
				ISO_vChangeNumericValue(NV_TRIM_NO_TRIMMING, ISO_INPUT_BOOLEAN_SET);
				ISO_vChangeNumericValue(NV_TRIM_LEFT_SIDE, ISO_INPUT_BOOLEAN_CLEAR);
				ISO_vChangeNumericValue(NV_TRIM_RIGHT_SIDE, ISO_INPUT_BOOLEAN_CLEAR);
				ISO_vHideShowContainerCommand(CO_TRIMMING_LEFT_SIDE, false);
				ISO_vHideShowContainerCommand(CO_TRIMMING_RIGHT_SIDE, false);
				break;
			}
			case NV_TRIM_LEFT_SIDE:
			{
				sTrimmState.eNewTrimmState = TRIMMING_LEFT_SIDE;
				ISO_vChangeNumericValue(NV_TRIM_LEFT_SIDE, ISO_INPUT_BOOLEAN_SET);
				ISO_vChangeNumericValue(NV_TRIM_NO_TRIMMING, ISO_INPUT_BOOLEAN_CLEAR);
				ISO_vChangeNumericValue(NV_TRIM_RIGHT_SIDE, ISO_INPUT_BOOLEAN_CLEAR);
				ISO_vHideShowContainerCommand(CO_TRIMMING_LEFT_SIDE, true);
				ISO_vHideShowContainerCommand(CO_TRIMMING_RIGHT_SIDE, false);
				break;
			}
			case NV_TRIM_RIGHT_SIDE:
			{
				sTrimmState.eNewTrimmState = TRIMMING_RIGHT_SIDE;
				ISO_vChangeNumericValue(NV_TRIM_RIGHT_SIDE, ISO_INPUT_BOOLEAN_SET);
				ISO_vChangeNumericValue(NV_TRIM_LEFT_SIDE, ISO_INPUT_BOOLEAN_CLEAR);
				ISO_vChangeNumericValue(NV_TRIM_NO_TRIMMING, ISO_INPUT_BOOLEAN_CLEAR);
				ISO_vHideShowContainerCommand(CO_TRIMMING_LEFT_SIDE, false);
				ISO_vHideShowContainerCommand(CO_TRIMMING_RIGHT_SIDE, true);
				break;
			}
			default:
			{
				if ((wObjectID >= NV_IND_LINE_IGNORE_L01) && (wObjectID <= NV_IND_LINE_IGNORE_L36))
				{
					sIgnoreStatus.bLineIgnored = dValue;
					sIgnoreStatus.bLineNum = IGNORE_LINE_GET_LINE_NUMBER_FROM_ID(wObjectID);

					if (dValue)
					{
						ISO_vChangeAttributeCommand(IGNORE_LINE_GET_RECT_ID_FROM_NV_ID(wObjectID), ISO_RECTANGLE_LINE_ATTRIBUTE, LA_PLANT_IGNORED_LINE);
						ISO_vChangeAttributeCommand(IGNORE_LINE_GET_BG_UP_ID_FROM_NV_ID(wObjectID), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, COLOR_GREY);
						ISO_vChangeAttributeCommand(IGNORE_LINE_GET_BG_DOWN_ID_FROM_NV_ID(wObjectID), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, COLOR_GREY);
					} else
					{
						ISO_vChangeAttributeCommand(IGNORE_LINE_GET_RECT_ID_FROM_NV_ID(wObjectID), ISO_RECTANGLE_LINE_ATTRIBUTE, LA_PLANT_DEFAULT_LINE);
						ISO_vChangeAttributeCommand(IGNORE_LINE_GET_BG_UP_ID_FROM_NV_ID(wObjectID), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, COLOR_BLACK);
						ISO_vChangeAttributeCommand(IGNORE_LINE_GET_BG_DOWN_ID_FROM_NV_ID(wObjectID), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, COLOR_BLACK);
					}

					ePubEvt = EVENT_ISO_PLANTER_IGNORE_SENSOR;
					WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
					PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
					WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				}
				break;
			}
		}
		ISO_vInputNumberVariableValue(wObjectID, dValue);
	}
	else if ((wObjectID >= IL_CFG_AREA_MONITOR) && (wObjectID <= IL_SYSTEM_CAN_BUS_SELECT))
	{
		switch (wObjectID)
		{
			case ISO_INPUT_LIST_AREA_MONITOR_ID:
			{
				if (dValue)
				{
					ISO_vChangeAttributeCommand(IN_CFG_SEEDS_P_M, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
					ISO_vChangeAttributeCommand(IN_CFG_N_ROWS, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
					ISO_vChangeAttributeCommand(IN_CFG_ROW_SPACING, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
					ISO_vChangeAttributeCommand(IN_CFG_EVAL_DISTANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
					ISO_vChangeAttributeCommand(IN_CFG_TOLERANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
					ISO_vChangeAttributeCommand(IN_CFG_IMP_WIDTH, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
					ISO_vEnableDisableObjCommand(IL_CFG_ALTERNATE_ROWS, false);
					ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, false);
				}
				else
				{
					ISO_vChangeAttributeCommand(IN_CFG_SEEDS_P_M, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
					ISO_vChangeAttributeCommand(IN_CFG_N_ROWS, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
					ISO_vChangeAttributeCommand(IN_CFG_ROW_SPACING, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
					ISO_vChangeAttributeCommand(IN_CFG_EVAL_DISTANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
					ISO_vChangeAttributeCommand(IN_CFG_TOLERANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
					ISO_vChangeAttributeCommand(IN_CFG_IMP_WIDTH, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
					ISO_vEnableDisableObjCommand(IL_CFG_ALTERNATE_ROWS, true);
					ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, true);
					eConfigMaskFromX = DATA_MASK_INSTALLATION;
				}
				ISO_vInputIndexListValue(wObjectID, dValue);
				bCfgClearTotals = true;
				break;
			}
			case ISO_INPUT_LIST_ALTERNATE_ROW_ID:
			{
				if (dValue)
				{
					ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, true);
					*sConfigDataMask.eAlterRows = ALTERNATE_ROWS_ENABLED;
				}
				else
				{
					ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, false);
					*sConfigDataMask.eAlterRows = ALTERNATE_ROWS_DISABLED;
				}
				ISO_vInputIndexListValue(wObjectID, dValue);
				ISO_vUpdatePlanterDataMaskLines();
				break;
			}
			case ISO_INPUT_LIST_ALTER_ROW_TYPE_ID:
			{
				if (dValue)
				{
					*sConfigDataMask.eAltType = ALTERNATED_ROWS_ODD;
				}
				else
				{
					*sConfigDataMask.eAltType = ALTERNATED_ROWS_EVEN;
				}
				ISO_vInputIndexListValue(wObjectID, dValue);
				ISO_vUpdatePlanterDataMaskLines();
				break;
			}
			case ISO_INPUT_LIST_CAN_STATUS_ID:
			{
				ePubEvt = EVENT_GUI_UPDATE_SYSTEM_CAN_INTERFACE;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(UpdateQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				ISO_vInputIndexListValue(wObjectID, dValue);
				break;
			}
			default:
			{
				ISO_vInputIndexListValue(wObjectID, dValue);
				break;
			}
		}
	}

	if(eCurrentMask == DATA_MASK_CONFIGURATION)
	{
		ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIGURATION_CHANGES);

		if (bCfgClearTotals)
		{
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CLEAR_TOTALS, true);
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_ONLY, false);
		}
	} else if (eCurrentMask == DATA_MASK_TRIMMING)
	{
		if (sTrimmState.eNewTrimmState != sTrimmState.eTrimmState)
		{
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_TRIMMING, MASK_TYPE_DATA_MASK,
					SOFT_KEY_MASK_TRIMMING_CHANGES);
		}
	}
}

void ISO_vTreatAcknowledgementMessage (ISOBUSMsg sRcvMsg)
{
	event_e eEvt;

	STOP_TIMER(WSMaintenanceTimer);
	ISO_vSendLoadVersion(OBJECT_POOL_VERSION);
	START_TIMER(WSMaintenanceTimer, ISO_TIMER_PERIOD_MS_WS_MAINTENANCE);

//	switch (eCurrentMask) {
//		case DATA_MASK_CONFIGURATION:
//		{
//			eEvt = EVENT_ISO_UPDA
//			break;
//		}
//		default:
//			break;
//	}
	PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
}

void ISO_vTreatSoftKeyActivation (uint16_t wObjectID)
{
	event_e ePubEvt;

	switch (wObjectID)
	{
		case ISO_KEY_PLANTER_ID:
		{
			break;
		}
		case ISO_KEY_CONFIG_ID:
		{
			ePasswdMaskFromX = eCurrentMask;
			ePasswordManager = PASSWD_ENTER_PASSWORD;
			ISO_vHideShowContainerCommand(CO_PASSWD_ENTER_PASSWD, true);
			ISO_vHideShowContainerCommand(CO_PASSWD_NEW_PASSWD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_CURRENT_PASSWD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_INCORRECT_PASSWORD, false);
			ISO_vChangeActiveMask(DATA_MASK_PASSWORD);
			break;
		}
		case ISO_KEY_INSTALLATION_ID:
		{
			ISO_vChangeActiveMask(DATA_MASK_INSTALLATION);
			break;
		}
		case ISO_KEY_TEST_MODE_ID:
		{
			break;
		}
		case ISO_KEY_FINISH_TEST_ID:
		{
			break;
		}
		case ISO_KEY_REPLACE_SENSORS_ID:
		{
			ISO_vChangeActiveMask(DATA_MASK_REPLACE_SENSOR);
			ePubEvt = EVENT_ISO_INSTALLATION_REPLACE_SENSOR;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		case ISO_KEY_BACKTO_INSTALLATION_ID:
		{
			ISO_vChangeActiveMask(DATA_MASK_INSTALLATION);
			break;
		}
		case ISO_KEY_TRIMMING_ID:
		{
			break;
		}
		case ISO_KEY_SYSTEM_ID:
		{
			break;
		}
		case ISO_KEY_CLEAR_TOTAL_ID:
		{
			ePlanterCounterCurrState = CLEAR_TOTAL_WAIT_CONFIRMATION;
			ISO_vHideShowContainerCommand(CO_CLEAR_TOTAL, true);
			ISO_vHideShowContainerCommand(CO_CLEAR_SUB_TOTAL, false);
			break;
		}
		case ISO_KEY_CLEAR_SUBTOTAL_ID:
		{
			ePlanterCounterCurrState = CLEAR_SUBTOTAL_WAIT_CONFIRMATION;
			ISO_vHideShowContainerCommand(CO_CLEAR_TOTAL, false);
			ISO_vHideShowContainerCommand(CO_CLEAR_SUB_TOTAL, true);
			break;
		}
		case ISO_KEY_BACKTO_PLANTER_ID:
		{
			break;
		}
		case ISO_KEY_INFO_ID:
		{
			if (sPlanterMask.psSpeedKm->dValue > 0)
			{
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER_MOVING);
			} else
			{
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER);
			}
			bCOPlanterLineInfo = true;
			bCOPlanterSpeedInfo = false;
			break;
		}
		case ISO_KEY_SPEED_ID:
		{
			if (sPlanterMask.psSpeedKm->dValue > 0)
			{
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER_INFO_MOVING);
			} else
			{
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER_INFO);
			}
			bCOPlanterSpeedInfo = true;
			bCOPlanterLineInfo = false;
			break;
		}
		case ISO_KEY_BACKTO_TRIMMING_CHANGES_ID:
		{
			eChangeTrimmCurrState = TRIMM_CHANGE_WAIT_CONFIRMATION;
			break;
		}
		case ISO_KEY_PAUSE_ID:
		{
			ePubEvt = EVENT_ISO_AREA_MONITOR_PAUSE;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		default:
			break;
	}
}

void ISO_vUpdateStringVariable (uint16_t wStrVarID, uint8_t* pbNewStrValue, uint16_t wNumBytes)
{
	ISOBUSMsg sSendMsg;

	if (pbNewStrValue == NULL)
		return;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_STRING_VALUE;
	sSendMsg.frame.data[1] = wStrVarID & 0xFF;
	sSendMsg.frame.data[2] = (wStrVarID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = wNumBytes & 0xFF;
	sSendMsg.frame.data[4] = (wNumBytes & 0xFF00) >> 8;
	sSendMsg.frame.data[5] = pbNewStrValue[0];
	sSendMsg.frame.data[6] = pbNewStrValue[1];
	sSendMsg.frame.data[7] = pbNewStrValue[2];

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vTreatButtonActivation (uint16_t wObjectID)
{
	static uint8_t bPasswd[4];
	event_e ePubEvt;

	switch (wObjectID)
	{
		case ISO_BUTTON_REPEAT_TEST_ID:
		{
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_INSTALLATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_INSTALLATION);
			ePubEvt = EVENT_ISO_INSTALLATION_REPEAT_TEST;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		case ISO_BUTTON_ERASE_INSTALLATION_ID:
		{
			eClearSetupCurrState = CLEAR_SETUP_WAIT_CONFIRMATION;
			break;
		}
		case ISO_BUTTON_CLEAR_COUNT_CANCEL_ID:
		{
			if ((ePlanterCounterCurrState == CLEAR_TOTAL_WAIT_CONFIRMATION)
				|| (ePlanterCounterCurrState == CLEAR_SUBTOTAL_WAIT_CONFIRMATION))
			{
				ePlanterCounterCurrState = CLEAR_TOTALS_IDLE;
			}
			break;
		}
		case ISO_BUTTON_CLEAR_COUNT_ACCEPT_ID:
		{
			if (ePlanterCounterCurrState == CLEAR_TOTAL_WAIT_CONFIRMATION)
			{
				ePubEvt = EVENT_ISO_PLANTER_CLEAR_COUNTER_TOTAL;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				ePlanterCounterCurrState = CLEAR_TOTALS_IDLE;
			}
			else if (ePlanterCounterCurrState == CLEAR_SUBTOTAL_WAIT_CONFIRMATION)
			{
				ePubEvt = EVENT_ISO_PLANTER_CLEAR_COUNTER_SUBTOTAL;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				ePlanterCounterCurrState = CLEAR_TOTALS_IDLE;
			}
			break;
		}
		case ISO_BUTTON_CLEAR_SETUP_CANCEL_ID:
		{
			if (eClearSetupCurrState == CLEAR_SETUP_WAIT_CONFIRMATION)
			{
				eClearSetupCurrState = CLEAR_SETUP_IDLE;
			}
			break;
		}
		case ISO_BUTTON_CLEAR_SETUP_ACCEPT_ID:
		{
			if (eClearSetupCurrState == CLEAR_SETUP_WAIT_CONFIRMATION)
			{
				ePubEvt = EVENT_ISO_INSTALLATION_ERASE_INSTALLATION;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				eClearSetupCurrState = CLEAR_SETUP_IDLE;
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_INSTALLATION, MASK_TYPE_DATA_MASK,
											  SOFT_KEY_MASK_INSTALLATION);
			}
			break;
		}
		case ISO_BUTTON_CONFIG_CHANGES_CANCEL_RET_INSTALL_ID:
		{
			ePubEvt = EVENT_ISO_CONFIG_CANCEL_UPDATE_DATA;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIG_TO_SETUP);
			break;
		}
		case ISO_BUTTON_CONFIG_CHANGES_ACCEPT_ID:
		{
			ePubEvt = EVENT_ISO_CONFIG_UPDATE_DATA;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

			if (ePasswordManager != PASSWD_CHANGE_PASSWD_NEW_PASSWD)
			{
				if (eConfigMaskFromX == DATA_MASK_INSTALLATION)
				{
					ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIG_TO_SETUP);
					ISO_vChangeActiveMask(DATA_MASK_INSTALLATION);
				}
				else if ((eConfigMaskFromX == DATA_MASK_PLANTER) && ((*sConfigDataMask.eMonitor) != AREA_MONITOR_ENABLED))
				{
					ISO_vHideShowContainerCommand(CO_PLANTER_LINES_INFO, true);
					ISO_vHideShowContainerCommand(CO_PLANTER_LINES_MASTER, true);
					ISO_vHideShowContainerCommand(CO_PLANTER_LINES_DISABLE_ALL, true);
					ISO_vHideShowContainerCommand(CO_PLANTER_AREA_MONITOR, false);
					ISO_vHideShowContainerCommand(CO_PLANTER_SPEED_INFO, false);
					ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIG_TO_PLANTER);

					(bCfgClearSetup) ? ISO_vChangeActiveMask(DATA_MASK_INSTALLATION) : ISO_vChangeActiveMask(DATA_MASK_PLANTER);
				}

				if (bCfgClearSetup)
				{
					ePubEvt = EVENT_ISO_INSTALLATION_ERASE_INSTALLATION;
					WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
					PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
					WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				}

				if ((*sConfigDataMask.eMonitor) == AREA_MONITOR_ENABLED)
				{
					ISO_vHideShowContainerCommand(CO_PLANTER_AREA_MONITOR, true);
					ISO_vHideShowContainerCommand(CO_PLANTER_SPEED_INFO, true);
					ISO_vHideShowContainerCommand(CO_PLANTER_LINES_INFO, false);
					ISO_vHideShowContainerCommand(CO_PLANTER_LINES_MASTER, false);
					ISO_vHideShowContainerCommand(CO_PLANTER_LINES_DISABLE_ALL, false);
					ISO_vChangeActiveMask(DATA_MASK_PLANTER);
				}

				if ((*sConfigDataMask.eAlterRows) == ALTERNATE_ROWS_DISABLED)
				{
					ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, false);
				}
				else
				{
					ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, true);
				}

				if (bCfgClearTotals)
				{
					ePubEvt = EVENT_ISO_PLANTER_CLEAR_COUNTER_TOTAL;
					WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
					PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
					WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
					bCfgClearTotals = false;
				}

				if (bCfgClearSetup)
				{
					ePubEvt = EVENT_ISO_INSTALLATION_ERASE_INSTALLATION;
					WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
					PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
					WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
					bCfgClearSetup = false;
					ISO_vChangeSoftKeyMaskCommand(DATA_MASK_INSTALLATION, MASK_TYPE_DATA_MASK,
												  SOFT_KEY_MASK_INSTALLATION);
				}

			} else
			{
				ePubEvt = EVENT_ISO_CONFIG_CHANGE_PASSWORD;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			}
			break;
		}
		case ISO_BUTTON_CONFIG_CHANGES_CANCEL_RET_CONFIG_ID:
		{
			bPasswsNumDigits = 0;
			ePasswordManager = PASSWD_IDLE;
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_1, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_2, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_3, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_4, false);
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIGURATION_CHANGES);
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_CONFIG, true);
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_SETUP, false);
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_PLANTER, false);
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_ONLY, true);
			ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CLEAR_TOTALS, false);
			ePubEvt = EVENT_ISO_CONFIG_CANCEL_UPDATE_DATA;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		case ISO_BUTTON_CONFIG_CHANGES_CANCEL_RET_PLANTER_ID:
		{
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIG_TO_PLANTER);
			ePubEvt = EVENT_ISO_CONFIG_CANCEL_UPDATE_DATA;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		case ISO_BUTTON_TRIMM_CHANGES_CANCEL_ID:
		{
			if (eChangeTrimmCurrState == TRIMM_CHANGE_WAIT_CONFIRMATION)
			{
				sTrimmState.eNewTrimmState = sTrimmState.eTrimmState;
				switch (sTrimmState.eTrimmState)
				{
					case TRIMMING_NOT_TRIMMED:
					{
						ISO_vChangeNumericValue(NV_TRIM_NO_TRIMMING, true);
						ISO_vChangeNumericValue(NV_TRIM_LEFT_SIDE, false);
						ISO_vChangeNumericValue(NV_TRIM_RIGHT_SIDE, false);
						break;
					}
					case TRIMMING_LEFT_SIDE:
					{
						ISO_vChangeNumericValue(NV_TRIM_LEFT_SIDE, true);
						ISO_vChangeNumericValue(NV_TRIM_NO_TRIMMING, false);
						ISO_vChangeNumericValue(NV_TRIM_RIGHT_SIDE, false);
						break;
					}
					case TRIMMING_RIGHT_SIDE:
					{
						ISO_vChangeNumericValue(NV_TRIM_RIGHT_SIDE, true);
						ISO_vChangeNumericValue(NV_TRIM_LEFT_SIDE, false);
						ISO_vChangeNumericValue(NV_TRIM_NO_TRIMMING, false);
						break;
					}
					default:
						break;
				}
				eChangeTrimmCurrState = TRIMM_CHANGE_IDLE;
			}
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_TRIMMING, MASK_TYPE_DATA_MASK,
					SOFT_KEY_MASK_TRIMMING);
			break;
		}
		case ISO_BUTTON_TRIMM_CHANGES_ACCEPT_ID:
		{
			if (eChangeTrimmCurrState == TRIMM_CHANGE_WAIT_CONFIRMATION)
			{
				sTrimmState.eTrimmState = sTrimmState.eNewTrimmState;
				ePubEvt = EVENT_ISO_TRIMMING_TRIMMING_MODE_CHANGE;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
				eChangeTrimmCurrState = TRIMM_CHANGE_IDLE;
			}
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_TRIMMING, MASK_TYPE_DATA_MASK,
					SOFT_KEY_MASK_TRIMMING);
			break;
		}
		case ISO_BUTTON_CHANGE_PASSWORD_ID:
		{
			ePasswdMaskFromX = eCurrentMask;
			ePasswordManager = PASSWD_CHANGE_PASSWD_CURRENT_PASSWD;
			ISO_vHideShowContainerCommand(CO_PASSWD_CURRENT_PASSWD, true);
			ISO_vHideShowContainerCommand(CO_PASSWD_ENTER_PASSWD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_NEW_PASSWD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_INCORRECT_PASSWORD, false);
			ISO_vChangeActiveMask(DATA_MASK_PASSWORD);
			break;
		}
		case ISO_BUTTON_PASSWORD_CANCEL_ID:
		{
			wPubPasswd = 0;
			bPasswsNumDigits = 0;
			ePasswordManager = PASSWD_IDLE;
			ISO_vHideShowContainerCommand(CO_PASSWD_ENTER_PASSWD, true);
			ISO_vHideShowContainerCommand(CO_PASSWD_CURRENT_PASSWD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_NEW_PASSWD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_INCORRECT_PASSWORD, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_1, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_2, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_3, false);
			ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_4, false);
			ISO_vChangeActiveMask(ePasswdMaskFromX);
			break;
		}
		case ISO_BUTTON_PASSWORD_ACCEPT_ID:
		{
			wPubPasswd = atoi(bPasswd);
			switch (ePasswordManager)
			{
				case PASSWD_ENTER_PASSWORD:
				{
					ePubEvt = EVENT_ISO_CONFIG_CHECK_PASSWORD;
					WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
					PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
					WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
					break;
				}
				case PASSWD_CHANGE_PASSWD_CURRENT_PASSWD:
				{
					ePubEvt = EVENT_ISO_CONFIG_CHECK_PASSWORD;
					WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
					PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
					WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
					break;
				}
				case PASSWD_CHANGE_PASSWD_NEW_PASSWD:
				{
					ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_CONFIG, true);
					ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_SETUP, false);
					ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_PLANTER, false);
					ISO_vHideShowContainerCommand(CO_CFG_CHANGE_ONLY, true);
					ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CLEAR_TOTALS, false);
					ISO_vChangeActiveMask(DATA_MASK_CONFIRM_CONFIG_CHANGES);
					break;
				}
				default:
					break;
			}
			break;
		}
		case ISO_BUTTON_REPLACE_SENSOR_CANCEL_ID:
		case ISO_BUTTON_REPLACE_SENSOR_ACCEPT_ID:
		{
			ISO_vChangeActiveMask(DATA_MASK_INSTALLATION);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_WAIT, true);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NOT_ALLOWED, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NO_SENSORS, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_DESCRIPTION, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ACCEPT, false);
			ePubEvt = (wObjectID == ISO_BUTTON_REPLACE_SENSOR_ACCEPT_ID) ? EVENT_ISO_INSTALLATION_CONFIRM_REPLACE_SENSOR :
																		   EVENT_ISO_INSTALLATION_CANCEL_REPLACE_SENSOR;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		case ISO_BUTTON_CONFIG_MEMORY_TAB_ID:
		{
			ePubEvt = EVENT_ISO_CONFIG_GET_MEMORY_USED;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		default:
		{
			if ((wObjectID >= BU_PLANTER_L01) && (wObjectID <= BU_PLANTER_L36))
			{
				bClearAlarmLineX = (wObjectID - BU_PLANTER_L01);
				ISO_vChangeAttributeCommand(BARGRAPH_UP_GET_ID_FROM_LINE_NUMBER(bClearAlarmLineX), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, COLOR_BLACK);
				ISO_vChangeAttributeCommand(BARGRAPH_DOWN_GET_ID_FROM_LINE_NUMBER(bClearAlarmLineX), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, COLOR_BLACK);
				ISO_vChangeAttributeCommand(RECTANGLE_PLANT_GET_ID_FROM_LINE_NUMBER(bClearAlarmLineX), ISO_RECTANGLE_LINE_ATTRIBUTE, COLOR_BLACK);
				ePubEvt = EVENT_ISO_ALARM_CLEAR_ALARM;
				WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
				PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
				WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

			}

			if ((wObjectID >= BU_PASSWD_DIGIT_0) && (wObjectID <= BU_PASSWD_DIGIT_9))
			{
				if (bPasswsNumDigits < 4)
				{
					bPasswd[bPasswsNumDigits] = (wObjectID - BU_PASSWD_DIGIT_0) + 48; // To ascii
					ISO_vHideShowContainerCommand((CO_PASSWD_DIGIT_1 + bPasswsNumDigits), true);
					bPasswsNumDigits++;

					if (bPasswsNumDigits == 4)
					{
						ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, true);
					}
				}

			} else if ((wObjectID == BU_PASSWD_BACKSPACE))
			{
				if (bPasswsNumDigits > 0)
				{
					bPasswsNumDigits--;
					ISO_vHideShowContainerCommand((CO_PASSWD_DIGIT_1 + bPasswsNumDigits), false);

					if (bPasswsNumDigits < 4)
					{
						ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, false);
					}
				}
			}
			break;
		}
	}
}

void ISO_vTreatRunningState (ISOBUSMsg* sRcvMsg)
{
	uint16_t dAux;
	event_e ePubEvt;


	if (sRcvMsg == NULL)
		return;

	switch (ISO_wGetPGN(sRcvMsg))
	{
		case VT_TO_ECU_PGN:
		{
			if (sRcvMsg->PS == M2G_SOURCE_ADDRESS)
			{
				dAux = ((sRcvMsg->B4 << 8) | (sRcvMsg->B3));
				switch (sRcvMsg->B1)
				{
					case FUNC_SOFT_KEY_ACTIVATION:
					{
						ISO_vTreatSoftKeyActivation(dAux);
						break;
					}
					case FUNC_BUTTON_ACTIVATION:
					{
						if (sRcvMsg->B2 != ISO_BUTTON_ACTIVATION_PRESSED)
							break;

						ISO_vTreatButtonActivation(dAux);
						break;
					}
					case FUNC_POINTING_EVENT:
						break;
					case FUNC_VT_SELECT_INP_OBJECT:
						break;
					case FUNC_VT_ESC:
						break;
					case FUNC_VT_CHANGE_NUMERIC_VALUE:
					{
						ISO_vTreatChangeNumericValueEvent(sRcvMsg);
						break;
					}
					case FUNC_VT_CHANGE_ACTIVE_MASK:
					{
						dAux = ((sRcvMsg->B3 << 8) | (sRcvMsg->B2));
						if ((dAux >= DATA_MASK_INSTALLATION) && (dAux < DATA_MASK_INVALID))
						{
							if (eCurrentMask == (eIsobusMask)dAux)
							{
								bLanguageChRcv = 0;
							}
							ePrevMask = eCurrentMask;
							eCurrentMask = (eIsobusMask)dAux;

							if (eCurrentMask == DATA_MASK_INSTALLATION)
							{
								// Ao trocar as configurações regionais do GS3, o mesmo força uma mudança de tela
								// para a tela default do pool. A ideia é monitorar esse comando e voltar a tela
								// anterior.
								if (bLanguageChRcv)
								{
									bLanguageChRcv = 0;
									ISO_vChangeActiveMask(ePrevMask);
								}
								else
								{
									eConfigMaskFromX = DATA_MASK_INSTALLATION;
									ISO_vChangeSoftKeyMaskCommand(DATA_MASK_INSTALLATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_INSTALLATION);
									ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIG_TO_SETUP);
									ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_SETUP, true);
									ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_CONFIG, false);
									ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_PLANTER, false);
									ISO_vHideShowContainerCommand(CO_CFG_CHANGE_ONLY, true);
									ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CLEAR_TOTALS, false);

									ePubEvt = EVENT_ISO_INSTALLATION_REPEAT_TEST;
								}
								WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
								PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
								WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
							} else if (eCurrentMask == DATA_MASK_PLANTER)
							{
								eConfigMaskFromX = DATA_MASK_PLANTER;
								ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_CONFIG_TO_PLANTER);
								ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_PLANTER, true);
								ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_CONFIG, false);
								ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_SETUP, false);
								ISO_vHideShowContainerCommand(CO_CFG_CHANGE_ONLY, true);
								ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CLEAR_TOTALS, false);
							}

							ePubEvt = EVENT_ISO_UPDATE_CURRENT_DATA_MASK;
							WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
							PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
							WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
						}
						break;
					}
					default:
						break;
				}
			}
			else if (sRcvMsg->PS == BROADCAST_ADDRESS)
			{
				switch (sRcvMsg->B1)
				{
					case FUNC_VT_STATUS:
					ISO_vIsobusUpdateVTStatus(sRcvMsg);
						break;
					default:
						break;
				}
			}
			break;
		}
		case TP_CONN_MANAGE_PGN:
		case ETP_CONN_MANAGE_PGN:
		{
			break;
		}
		case LANGUAGE_PGN:
		{
			ISO_vTreatLanguageCommandMessage(*sRcvMsg);
			event_e ePubEvt = EVENT_ISO_LANGUAGE_COMMAND;
			WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
			PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
			WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);
			break;
		}
		case ACKNOWLEDGEMENT_PGN:
		{
//			ISO_vTreatAcknowledgementMessage (*sRcvMsg);
			break;
		}
		default:
			break;
	}
}

/******************************************************************************
 * Function : ISO_vIsobusTransportProtocolThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vIsobusThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusManagementThread (void const *argument)
{
	osEvent evt;
	ISOBUSMsg sRcvMsg;

	INITIALIZE_LOCAL_QUEUE(ManagementQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus Management Thread Created");
#endif

	ISO_vDetectThread(&WATCHDOG(ISOMGT), &bISOMGTThreadArrayPosition, (void*)ISO_vIsobusManagementThread);
	WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

	osThreadId xIsoMainID = (osThreadId)argument;
	osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOMGTThreadArrayPosition)); //Task created, inform core

	// Wait for auxiliary thread start
	/* Pool the device waiting for */
	WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
	osSignalWait(0xFF, osWaitForever);
	WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
		evt = RECEIVE_LOCAL_QUEUE(ManagementQ, &sRcvMsg, osWaitForever);   // Wait
		WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

		if (evt.status == osEventMessage)
		{
			switch (eModCurrState)
			{
				case UPDATING_LANGUAGE:
				case BOOT:
				{
					ISO_vTreatBootState(&sRcvMsg);
					break;
				}
				case RUNNING:
				{
					ISO_vTreatRunningState(&sRcvMsg);
					break;
				}
				default:
					break;
			}
		}
	}
	osThreadTerminate(NULL);
}
#else
void ISO_vIsobusManagementThread(void const *argument)
{}
#endif

void ISO_vChangeNumericValue (uint16_t wOutputNumberID, uint32_t dNumericValue)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_NUMERIC_VALUE;
	sSendMsg.frame.data[1] = wOutputNumberID & 0xFF;
	sSendMsg.frame.data[2] = (wOutputNumberID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = 0xFF;
	sSendMsg.frame.data[4] = dNumericValue & 0xFF;
	sSendMsg.frame.data[5] = (dNumericValue & 0xFF00) >> 8;
	sSendMsg.frame.data[6] = (dNumericValue & 0xFF0000) >> 16;
	sSendMsg.frame.data[7] = (dNumericValue & 0xFF000000) >> 24;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateListItemValue (uint16_t wOutputNumberID, uint8_t bListItem)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_LIST_ITEM;
	sSendMsg.frame.data[1] = wOutputNumberID & 0xFF;
	sSendMsg.frame.data[2] = (wOutputNumberID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = bListItem;
	sSendMsg.frame.data[4] = 0xFF;
	sSendMsg.frame.data[5] = 0xFF;
	sSendMsg.frame.data[6] = 0xFF;
	sSendMsg.frame.data[7] = 0xFF;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateBarGraphColor (uint16_t wBarGraphID, uint32_t dNumericValue)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_ATTRIBUTE;
	sSendMsg.frame.data[1] = wBarGraphID & 0xFF;
	sSendMsg.frame.data[2] = (wBarGraphID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = 0x03;
	sSendMsg.frame.data[4] = dNumericValue & 0xFF;
	sSendMsg.frame.data[5] = (dNumericValue & 0xFF00) >> 8;
	sSendMsg.frame.data[6] = (dNumericValue & 0xFF0000) >> 16;
	sSendMsg.frame.data[7] = (dNumericValue & 0xFF000000) >> 24;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateFillAttributesValue (uint16_t wFillAttrID, uint8_t bColor)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_FILL_ATTRIBUTES;
	sSendMsg.frame.data[1] = wFillAttrID & 0xFF;
	sSendMsg.frame.data[2] = (wFillAttrID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = 0x02;
	sSendMsg.frame.data[4] = bColor;
	sSendMsg.frame.data[5] = 0xFF;
	sSendMsg.frame.data[6] = 0xFF;
	sSendMsg.frame.data[7] = 0xFF;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vControlAudioSignalCommand (uint8_t bNumActivations, uint16_t wFrequency, uint16_t wOnTimeMs,
	uint16_t wOffTimeMs)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CONTROL_AUDIO_SIGNAL;
	sSendMsg.frame.data[1] = bNumActivations;
	sSendMsg.frame.data[2] = wFrequency & 0xFF;
	sSendMsg.frame.data[3] = (wFrequency & 0xFF00) >> 8;
	sSendMsg.frame.data[4] = wOnTimeMs & 0xFF;
	sSendMsg.frame.data[5] = (wOnTimeMs & 0xFF00) >> 8;
	sSendMsg.frame.data[6] = wOffTimeMs & 0xFF;
	sSendMsg.frame.data[7] = (wOffTimeMs & 0xFF00) >> 8;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vChangeActiveMask (eIsobusMask eNewMask)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_ACTIVE_MASK;
	sSendMsg.frame.data[1] = (ISO_OBJECT_WORKING_SET_ID & 0xFF00) >> 8;
	sSendMsg.frame.data[2] = ISO_OBJECT_WORKING_SET_ID & 0xFF;
	sSendMsg.frame.data[3] = eNewMask & 0xFF;
	sSendMsg.frame.data[4] = (eNewMask & 0xFF00) >> 8;
	sSendMsg.frame.data[5] = 0xFF;
	sSendMsg.frame.data[6] = 0xFF;
	sSendMsg.frame.data[7] = 0xFF;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vChangeAttributeCommand (uint16_t wObjID, uint8_t bObjAID, uint32_t dNewValue)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_ATTRIBUTE;
	sSendMsg.frame.data[1] = wObjID & 0xFF;
	sSendMsg.frame.data[2] = (wObjID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = bObjAID;
	sSendMsg.frame.data[4] = dNewValue & 0xFF;
	sSendMsg.frame.data[5] = (dNewValue & 0xFF00) >> 8;
	sSendMsg.frame.data[6] = (dNewValue & 0xFF0000) >> 16;
	sSendMsg.frame.data[7] = (dNewValue & 0xFF000000) >> 24;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vEnableDisableObjCommand (uint16_t wObjID, bool bIsEnable)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_ENABLE_DISABLE_OBJECT;
	sSendMsg.frame.data[1] = wObjID & 0xFF;
	sSendMsg.frame.data[2] = (wObjID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = bIsEnable;
	sSendMsg.frame.data[4] = 0xFF;
	sSendMsg.frame.data[5] = 0xFF;
	sSendMsg.frame.data[6] = 0xFF;
	sSendMsg.frame.data[7] = 0xFF;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vChangeSoftKeyMaskCommand (eIsobusMask eMask, eIsobusMaskType eMaskType, eIsobusSoftKeyMask eNewSoftKeyMask)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_CHANGE_SOFT_KEY_MASK;
	sSendMsg.frame.data[1] = eMaskType;
	sSendMsg.frame.data[2] = eMask & 0xFF;
	sSendMsg.frame.data[3] = (eMask & 0xFF00) >> 8;
	sSendMsg.frame.data[4] = eNewSoftKeyMask & 0xFF;
	sSendMsg.frame.data[5] = (eNewSoftKeyMask & 0xFF00) >> 8;
	sSendMsg.frame.data[6] = 0xFF;
	sSendMsg.frame.data[7] = 0xFF;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vHideShowContainerCommand (uint16_t wObjID, bool bShow)
{
	ISOBUSMsg sSendMsg;

	sSendMsg.frame.id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY_6);
	sSendMsg.frame.dlc = 8;

	sSendMsg.frame.data[0] = FUNC_HIDE_SHOW_OBJECT;
	sSendMsg.frame.data[1] = wObjID & 0xFF;
	sSendMsg.frame.data[2] = (wObjID & 0xFF00) >> 8;
	sSendMsg.frame.data[3] = bShow;
	sSendMsg.frame.data[4] = 0xFF;
	sSendMsg.frame.data[5] = 0xFF;
	sSendMsg.frame.data[6] = 0xFF;
	sSendMsg.frame.data[7] = 0xFF;

	PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateConfigurationDataMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

//	ISO_vChangeNumericValue(IL_CFG_LANGUAGE, *sConfigDataMask.eLanguage);
//	ISO_vChangeNumericValue(IL_CFG_UNIT_SYSTEM, *sConfigDataMask.eUnit);
	ISO_vChangeNumericValue(NV_CFG_VEHICLE_CODE, *sConfigDataMask.dVehicleID);

	ISO_vChangeNumericValue(IL_CFG_AREA_MONITOR, *sConfigDataMask.eMonitor);
	ISO_vChangeNumericValue(NV_CFG_IMP_WIDTH, *sConfigDataMask.wImplementWidth);

	if ((*sConfigDataMask.eMonitor) == AREA_MONITOR_DISABLED)
	{
		ISO_vChangeAttributeCommand(IN_CFG_SEEDS_P_M, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
		ISO_vChangeAttributeCommand(IN_CFG_N_ROWS, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
		ISO_vChangeAttributeCommand(IN_CFG_ROW_SPACING, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
		ISO_vChangeAttributeCommand(IN_CFG_EVAL_DISTANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
		ISO_vChangeAttributeCommand(IN_CFG_TOLERANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
		ISO_vChangeAttributeCommand(IN_CFG_IMP_WIDTH, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
		ISO_vEnableDisableObjCommand(IL_CFG_ALTERNATE_ROWS, true);
		ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, true);
		ISO_vHideShowContainerCommand(CO_PLANTER_LINES_INFO, true);
		ISO_vHideShowContainerCommand(CO_PLANTER_LINES_MASTER, true);
		ISO_vHideShowContainerCommand(CO_PLANTER_LINES_DISABLE_ALL, true);
		ISO_vHideShowContainerCommand(CO_PLANTER_AREA_MONITOR, false);
		ISO_vHideShowContainerCommand(CO_PLANTER_SPEED_INFO, false);
		ISO_vChangeActiveMask(DATA_MASK_INSTALLATION);
	} else
	{
		ISO_vChangeAttributeCommand(IN_CFG_SEEDS_P_M, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
		ISO_vChangeAttributeCommand(IN_CFG_N_ROWS, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
		ISO_vChangeAttributeCommand(IN_CFG_ROW_SPACING, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
		ISO_vChangeAttributeCommand(IN_CFG_EVAL_DISTANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
		ISO_vChangeAttributeCommand(IN_CFG_TOLERANCE, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_DISABLE);
		ISO_vChangeAttributeCommand(IN_CFG_IMP_WIDTH, ISO_INPUT_NUMBER_OPTION2_ATTRIBUTE, ISO_INPUT_NUMBER_ENABLE);
		ISO_vEnableDisableObjCommand(IL_CFG_ALTERNATE_ROWS, false);
		ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, false);
		ISO_vHideShowContainerCommand(CO_PLANTER_AREA_MONITOR, true);
		ISO_vHideShowContainerCommand(CO_PLANTER_SPEED_INFO, true);
		ISO_vHideShowContainerCommand(CO_PLANTER_LINES_INFO, false);
		ISO_vHideShowContainerCommand(CO_PLANTER_LINES_MASTER, false);
		ISO_vHideShowContainerCommand(CO_PLANTER_LINES_DISABLE_ALL, false);
		ISO_vChangeActiveMask(DATA_MASK_PLANTER);
	}

	ISO_vChangeNumericValue(NV_CFG_SEEDS_P_M, *sConfigDataMask.wSeedRate);
	ISO_vChangeNumericValue(NV_CFG_N_ROWS, *sConfigDataMask.bNumOfRows);

	if ((((*sConfigDataMask.bNumOfRows) % 2) != 0) && ((*sConfigDataMask.bNumOfRows) > 1))
	{
		ISO_vEnableDisableObjCommand(IL_CFG_CENTER_ROW_SIDE, true);
	} else
	{
		ISO_vEnableDisableObjCommand(IL_CFG_CENTER_ROW_SIDE, false);
	}
	ISO_vChangeNumericValue(IL_CFG_CENTER_ROW_SIDE, *sConfigDataMask.eCentralRowSide);

	ISO_vChangeNumericValue(NV_CFG_ROW_SPACING, *sConfigDataMask.wDistBetweenLines);
	ISO_vChangeNumericValue(NV_CFG_EVAL_DISTANCE, *sConfigDataMask.wEvaluationDistance);
	ISO_vChangeNumericValue(NV_CFG_TOLERANCE, *sConfigDataMask.bTolerance);
	ISO_vChangeNumericValue(NV_CFG_MAX_SPEED, GET_UNSIGNED_INT_VALUE(*sConfigDataMask.fMaxSpeed));

	ISO_vChangeNumericValue(IL_CFG_ALTERNATE_ROWS, *sConfigDataMask.eAlterRows);

	if ((*sConfigDataMask.eAlterRows) == ALTERNATE_ROWS_ENABLED)
	{
		ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, true);
	} else
	{
		ISO_vEnableDisableObjCommand(IL_CFG_RAISED_ROWS, false);
	}

	ISO_vChangeNumericValue(IL_CFG_RAISED_ROWS, *sConfigDataMask.eAltType);

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateInstallationDataMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	for (int i = 0; i < sInstallStatus.bNumOfSensors; i++)
	{
		sInstallStatus.pFillAttribute[i].bColor = eSensorsIntallStatus[i];
		ISO_vUpdateFillAttributesValue(sInstallStatus.pFillAttribute[i].wObjID,
				sInstallStatus.pFillAttribute[i].bColor);
	}

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateAlarmStatus (uint8_t bNumLine, eLineAlarm eAlarmStatus)
{
	uint8_t bColor = (eAlarmStatus == LINE_ALARM_NO_SEED) ? COLOR_RED : ((eAlarmStatus == LINE_ALARM_TOLERANCE) ? COLOR_OLIVE : COLOR_BLACK);
	uint16_t wRectID = (bColor == COLOR_BLACK) ? LA_PLANT_DEFAULT_LINE : ((bColor == COLOR_RED) ? LA_PLANT_ALARM_RED : LA_PLANT_ALARM_YELLOW);
	ISO_vChangeAttributeCommand(BARGRAPH_UP_GET_ID_FROM_LINE_NUMBER(bNumLine), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, bColor);
	ISO_vChangeAttributeCommand(BARGRAPH_DOWN_GET_ID_FROM_LINE_NUMBER(bNumLine), ISO_BAR_GRAPH_COLOUR_ATTRIBUTE, bColor);
	ISO_vChangeAttributeCommand(RECTANGLE_PLANT_GET_ID_FROM_LINE_NUMBER(bNumLine), ISO_RECTANGLE_LINE_ATTRIBUTE, wRectID);
}

void ISO_vUpdatePlanterDataMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	if (sPlanterMask.psSpeedKm->dValue > 0)
	{
		if ((*sConfigDataMask.eMonitor) == AREA_MONITOR_DISABLED)
		{
			if (bCOPlanterLineInfo) {
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER_MOVING);
			} else if (bCOPlanterSpeedInfo) {
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER_INFO_MOVING);
			}
		} else
		{
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
					MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_AREA_MONITOR_MOVING);
		}
	} else
	{
		if ((*sConfigDataMask.eMonitor) == AREA_MONITOR_DISABLED)
		{
			if (bCOPlanterLineInfo) {
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER);
			} else if (bCOPlanterSpeedInfo) {
				ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
						MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_PLANTER_INFO);
			}
		} else {
			ISO_vChangeSoftKeyMaskCommand(DATA_MASK_PLANTER,
					MASK_TYPE_DATA_MASK, SOFT_KEY_MASK_AREA_MONITOR);
		}
	}

	if ((*sConfigDataMask.eMonitor) == AREA_MONITOR_DISABLED)
	{
		for (int i = 0; i < ((*sConfigDataMask.bNumOfRows) * 2); i++)
		{
			ISO_vChangeNumericValue(sPlanterMask.psLineStatus->psLineAverage[i].wObjID,
				sPlanterMask.psLineStatus->psLineAverage[i].dValue);

			if (i < (*sConfigDataMask.bNumOfRows))
			{
				ISO_vChangeNumericValue(sPlanterMask.psLineStatus->psLineSemPerUnit[i].wObjID,
					sPlanterMask.psLineStatus->psLineSemPerUnit[i].dValue);
				ISO_vChangeNumericValue(sPlanterMask.psLineStatus->psLineSemPerHa[i].wObjID,
					sPlanterMask.psLineStatus->psLineSemPerHa[i].dValue);
				ISO_vChangeNumericValue(sPlanterMask.psLineStatus->psLineTotalSeeds[i].wObjID,
					sPlanterMask.psLineStatus->psLineTotalSeeds[i].dValue);
				ISO_vUpdateAlarmStatus(i, sPlanterMask.psLineStatus->peLineAlarmStatus[i]);
			}
		}

		ISO_vChangeNumericValue(sPlanterMask.psProductivity->wObjID, sPlanterMask.psProductivity->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psWorkedTime->wObjID, sPlanterMask.psWorkedTime->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTotalSeeds->wObjID, sPlanterMask.psTotalSeeds->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psPartPopSemPerUnit->wObjID, sPlanterMask.psPartPopSemPerUnit->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psPartPopSemPerHa->wObjID, sPlanterMask.psPartPopSemPerHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psWorkedAreaMt->wObjID, sPlanterMask.psWorkedAreaMt->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psWorkedAreaHa->wObjID, sPlanterMask.psWorkedAreaHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTotalMt->wObjID, sPlanterMask.psTotalMt->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTotalHa->wObjID, sPlanterMask.psTotalHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psSpeedKm->wObjID, sPlanterMask.psSpeedKm->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psSpeedHa->wObjID, sPlanterMask.psSpeedHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTEV->wObjID, sPlanterMask.psTEV->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psMTEV->wObjID, sPlanterMask.psMTEV->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psMaxSpeed->wObjID, sPlanterMask.psMaxSpeed->dValue);
	} else
	{
		ISO_vChangeNumericValue(sPlanterMask.psWorkedAreaMt->wObjID, sPlanterMask.psWorkedAreaMt->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psWorkedAreaHa->wObjID, sPlanterMask.psWorkedAreaHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTotalMt->wObjID, sPlanterMask.psTotalMt->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTotalHa->wObjID, sPlanterMask.psTotalHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psSpeedKm->wObjID, sPlanterMask.psSpeedKm->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psSpeedHa->wObjID, sPlanterMask.psSpeedHa->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psTEV->wObjID, sPlanterMask.psTEV->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psMTEV->wObjID, sPlanterMask.psMTEV->dValue);
		ISO_vChangeNumericValue(sPlanterMask.psMaxSpeed->wObjID, sPlanterMask.psMaxSpeed->dValue);
	}

	if (sUptPlanterMask.eUpdateState == UPDATE_PLANTER_ALARM)
	{
		switch (sUptPlanterMask.eAlarmEvent) {
			case EVENT_GUI_ALARM_TOLERANCE:
			{
				ISO_vControlAudioSignalCommand(ISO_ALARM_TOLERANCE_ACTIVATIONS,
						ISO_ALARM_TOLERANCE_FREQUENCY_HZ,
						ISO_ALARM_TOLERANCE_ON_TIME_MS,
						ISO_ALARM_TOLERANCE_OFF_TIME_MS);
				break;
			}
			default:
				break;
		}
		sUptPlanterMask.eUpdateState = UPDATE_PLANTER_NO_ALARM;
	}

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateTestModeDataMask (event_e eEvt)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	switch (eEvt)
	{
		case EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION:
		{
			ISO_vChangeNumericValue(sTestDataMask.pdInstalledSensors->wObjID,
				sTestDataMask.pdInstalledSensors->dValue);
			ISO_vChangeNumericValue(sTestDataMask.pdConfiguredSensors->wObjID,
				sTestDataMask.pdConfiguredSensors->dValue);
			break;
		}
		case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
		{
			for (int i = 0; i < (*sConfigDataMask.bNumOfRows); i++)
			{
				ISO_vChangeNumericValue(sTestDataMask.psSeedsCount[i].wObjID, sTestDataMask.psSeedsCount[i].dValue);
			}
			break;
		}
		default:
			break;
	}

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateTrimmingDataMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateSystemGPSMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	ISO_vUpdateStringVariable(SV_SYSTEM_GPS_LAT_DIRECTION, &sISOGPSStatus.bLatDir, sizeof(sISOGPSStatus.bLatDir));
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_LAT_DEGREES, sISOGPSStatus.wLatDgr);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_LAT_MINUTES, sISOGPSStatus.wLatDgr);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_LAT_SECONDS, sISOGPSStatus.wLatDgr);
	ISO_vUpdateStringVariable(SV_SYSTEM_GPS_LNG_DIRECTION, &sISOGPSStatus.bLonDir, sizeof(sISOGPSStatus.bLonDir));
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_LNG_DEGREES, sISOGPSStatus.wLonDgr);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_LNG_MINUTES, sISOGPSStatus.wLonDgr);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_LNG_SECONDS, sISOGPSStatus.wLonDgr);

	ISO_vChangeNumericValue(ON_SYSTEM_GPS_PDOP, sISOGPSStatus.dPDOP);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_NSV, sISOGPSStatus.bNSV);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_ERRP, sISOGPSStatus.dERRP);
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_ERRV, sISOGPSStatus.dERRV);

	ISO_vUpdateStringVariable(SV_SYSTEM_GPS_MODE, sISOGPSStatus.bMode, sizeof(sISOGPSStatus.bMode));
	ISO_vUpdateStringVariable(SV_SYSTEM_GPS_ANT, sISOGPSStatus.bAnt, sizeof(sISOGPSStatus.bAnt));
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_SPEED, sISOGPSStatus.dModVel);

	ISO_vUpdateStringVariable(SV_SYSTEM_GPS_ANT, sISOGPSStatus.bBBRAM, sizeof(sISOGPSStatus.bBBRAM));
	ISO_vChangeNumericValue(ON_SYSTEM_GPS_VER_FW, sISOGPSStatus.vVerFW);

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateSystemCANMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	switch (*eCANSelectedStatus)
	{
		case CAN_STATUS_ISOBUS:
		{
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_BAUD_RATE, sCANIsobusStatus.wBaudRateKbps);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_TX_COUNT, sCANIsobusStatus.wTxCount);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_RX_COUNT, sCANIsobusStatus.wRxCount);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_TOTAL_COUNT, (sCANIsobusStatus.wTxCount + sCANIsobusStatus.wRxCount));
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_OVER_ERR_COUNT, sCANIsobusStatus.dDataOverrun);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_PASS_ERR_COUNT, sCANIsobusStatus.dErrorPassive);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_WARN_ERR_COUNT, sCANIsobusStatus.dErrorWarning);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_BUS_OFF_ERR_COUNT, sCANIsobusStatus.dBusError);
			break;
		}
		case CAN_STATUS_SENSORS:
		{
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_BAUD_RATE, sCANSensorStatus.wBaudRateKbps);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_TX_COUNT, sCANSensorStatus.wTxCount);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_RX_COUNT, sCANSensorStatus.wRxCount);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_TOTAL_COUNT, (sCANSensorStatus.wTxCount + sCANSensorStatus.wRxCount));
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_OVER_ERR_COUNT, sCANSensorStatus.dDataOverrun);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_PASS_ERR_COUNT, sCANSensorStatus.dErrorPassive);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_WARN_ERR_COUNT, sCANSensorStatus.dErrorWarning);
			ISO_vChangeNumericValue(ON_SYSTEM_CAN_BUS_OFF_ERR_COUNT, sCANSensorStatus.dBusError);
			break;
		}
		default:
			break;
	}

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateSystemSensorsMask (void)
{
	osStatus status;

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
}

void ISO_vUpdateSisConfigData (sConfigurationData *psCfgDataMask)
{
	if (psCfgDataMask == NULL)
		return;

	psCfgDataMask->eLanguage = *sConfigDataMask.eLanguage;
	psCfgDataMask->eUnit = *sConfigDataMask.eUnit;
	psCfgDataMask->dVehicleID = *sConfigDataMask.dVehicleID;
	psCfgDataMask->eMonitorArea = *sConfigDataMask.eMonitor;
	psCfgDataMask->wSeedRate = *sConfigDataMask.wSeedRate;
	psCfgDataMask->bNumOfRows = *sConfigDataMask.bNumOfRows;
	psCfgDataMask->eCentralRowSide = *sConfigDataMask.eCentralRowSide;
	psCfgDataMask->wDistBetweenLines = *sConfigDataMask.wDistBetweenLines;
	psCfgDataMask->wImplementWidth = *sConfigDataMask.wImplementWidth;
	psCfgDataMask->wEvaluationDistance = *sConfigDataMask.wEvaluationDistance;
	psCfgDataMask->bTolerance = *sConfigDataMask.bTolerance;
	psCfgDataMask->fMaxSpeed = *sConfigDataMask.fMaxSpeed;
	psCfgDataMask->eAlterRows = *sConfigDataMask.eAlterRows;
	psCfgDataMask->eAltType = *sConfigDataMask.eAltType;
}

void ISO_vUpdateConfigData (sConfigurationData *psCfgDataMask)
{
	osStatus status;

	if (psCfgDataMask == NULL)
		return;

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

	*sConfigDataMask.eLanguage = psCfgDataMask->eLanguage;
	*sConfigDataMask.eUnit = psCfgDataMask->eUnit;
	*sConfigDataMask.dVehicleID = psCfgDataMask->dVehicleID;
	*sConfigDataMask.eMonitor = psCfgDataMask->eMonitorArea;
	*sConfigDataMask.wSeedRate = psCfgDataMask->wSeedRate;
	*sConfigDataMask.bNumOfRows = psCfgDataMask->bNumOfRows;
	*sConfigDataMask.eCentralRowSide = psCfgDataMask->eCentralRowSide;
	*sConfigDataMask.wDistBetweenLines = psCfgDataMask->wDistBetweenLines;
	*sConfigDataMask.wImplementWidth = psCfgDataMask->wImplementWidth;
	*sConfigDataMask.wEvaluationDistance = psCfgDataMask->wEvaluationDistance;
	*sConfigDataMask.bTolerance = psCfgDataMask->bTolerance;
	*sConfigDataMask.fMaxSpeed = psCfgDataMask->fMaxSpeed;
	*sConfigDataMask.eAlterRows = psCfgDataMask->eAlterRows;
	*sConfigDataMask.eAltType = psCfgDataMask->eAltType;

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
}

void ISO_vUpdateTestModeData (event_e eEvt, void* vPayload)
{
	osStatus status;
	sTestModeDataMaskData sTestUpdate;

	if (vPayload == NULL)
		return;

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

	switch (eEvt)
	{
		case EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION:
		{
			sTestUpdate = *((sTestModeDataMaskData*)vPayload);
			sTestDataMask.pdInstalledSensors->dValue = sTestUpdate.dInstalledSensors;
			sTestDataMask.pdConfiguredSensors->dValue = sTestUpdate.dConfiguredSensors;
			break;
		}
		case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
		{
			sTestUpdate = *((sTestModeDataMaskData*)vPayload);
			for (int i = 0; i < 36; i++)
			{
				sTestDataMask.psSeedsCount[i].dValue = (sTestUpdate.sAccumulated.sTotalReg.adSementes[i] % 1000);
			}
			break;
		}
		default:
			break;
	}

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
}

void ISO_vUpdatePlanterMaskData (sPlanterDataMaskData *psPlanterData)
{
	osStatus status;
	uint8_t bI, bJ;

	if(psPlanterData == NULL)
		return;

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	status = WAIT_MUTEX(ISO_UpdateMask, osWaitForever);
	ASSERT(status == osOK);
	WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

	for (bI = 0, bJ = 0; bI < *(sConfigDataMask.bNumOfRows); bI++, bJ += 2)
	{

		if (psPlanterData->asLineStatus[bI].dsLineAverage == 0)
		{
			sPlanterMask.psLineStatus->psLineAverage[bJ].dValue = 0;
			sPlanterMask.psLineStatus->psLineAverage[bJ + 1].dValue = 0;
		}
		else if (psPlanterData->asLineStatus[bI].dsLineAverage > 0)
		{
			sPlanterMask.psLineStatus->psLineAverage[bJ].dValue = psPlanterData->asLineStatus[bI].dsLineAverage;
			sPlanterMask.psLineStatus->psLineAverage[bJ + 1].dValue = 0;
		}
		else if (psPlanterData->asLineStatus[bI].dsLineAverage < 0)
		{
			sPlanterMask.psLineStatus->psLineAverage[bJ].dValue = 0;
			sPlanterMask.psLineStatus->psLineAverage[bJ + 1].dValue =
				(psPlanterData->asLineStatus[bI].dsLineAverage * (-1));
		}

		sPlanterMask.psLineStatus->psLineSemPerUnit[bI].dValue = psPlanterData->asLineStatus[bI].dLineSemPerUnit;
		sPlanterMask.psLineStatus->psLineSemPerHa[bI].dValue = psPlanterData->asLineStatus[bI].dLineSemPerHa;
		sPlanterMask.psLineStatus->psLineTotalSeeds[bI].dValue = psPlanterData->asLineStatus[bI].dLineTotalSeeds;
		sPlanterMask.psLineStatus->peLineAlarmStatus[bI] = psPlanterData->asLineStatus[bI].eLineAlarmStatus;
	}

	sPlanterMask.psProductivity->dValue = psPlanterData->dProductivity;
	sPlanterMask.psWorkedTime->dValue = psPlanterData->dWorkedTime;
	sPlanterMask.psTotalSeeds->dValue = psPlanterData->dTotalSeeds;
	sPlanterMask.psPartPopSemPerUnit->dValue = psPlanterData->dPartPopSemPerUnit;
	sPlanterMask.psPartPopSemPerHa->dValue = psPlanterData->dPartPopSemPerHa;
	sPlanterMask.psWorkedAreaMt->dValue = psPlanterData->dWorkedAreaMt;
	sPlanterMask.psWorkedAreaHa->dValue = psPlanterData->dWorkedAreaHa;
	sPlanterMask.psTotalMt->dValue = psPlanterData->dTotalMt;
	sPlanterMask.psTotalHa->dValue = psPlanterData->dTotalHa;
	sPlanterMask.psSpeedKm->dValue = psPlanterData->dSpeedKm;
	sPlanterMask.psSpeedHa->dValue = psPlanterData->dSpeedHa;
	sPlanterMask.psTEV->dValue = psPlanterData->dTEV;
	sPlanterMask.psMTEV->dValue = psPlanterData->dMTEV;
	sPlanterMask.psMaxSpeed->dValue = psPlanterData->dMaxSpeed;

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	status = RELEASE_MUTEX(ISO_UpdateMask);
	ASSERT(status == osOK);
	WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
}

void ISO_vUpdatePlanterDataMaskLines (void)
{
	if ((*sConfigDataMask.eAlterRows) == ALTERNATE_ROWS_ENABLED)
	{
		for (uint8_t i = 0; i < CAN_bNUM_DE_LINHAS; i++)
		{
			if (i < (*sConfigDataMask.bNumOfRows))
			{
				if ((*sConfigDataMask.eAltType) == ALTERNATED_ROWS_ODD)
				{
					if ((i % 2) == 0)
					{
						ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, true);
						ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, false);
					} else
					{
						ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, false);
						ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, true);
					}

				} else if ((*sConfigDataMask.eAltType) == ALTERNATED_ROWS_EVEN)
				{
					if ((i % 2) == 0)
					{
						ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, false);
						ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, true);
					} else
					{
						ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, true);
						ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, false);
					}
				}
			} else
			{
				ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, false);
				ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, true);
			}
		}

	} else if ((*sConfigDataMask.eAlterRows) == ALTERNATE_ROWS_DISABLED)
	{
		for (uint8_t i = 0; i < CAN_bNUM_DE_LINHAS; i++)
		{
			if (i < (*sConfigDataMask.bNumOfRows))
			{
				ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, true);
				ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, false);
			} else
			{
				ISO_vHideShowContainerCommand(CO_PLANTER_L01 + i, false);
				ISO_vHideShowContainerCommand(CO_LINE_DISABLE_L01 + i, true);
			}
		}
	}
	ISO_vHideShowContainerCommand(CO_PLANTER_LINES_DISABLE_ALL, true);
}

void ISO_vTimerCallbackAlarmTimeout (void const *arg)
{
	if (bKeepLineHighPrioAlarm && bHighPrioAudioInProcess)
	{
		ISO_vControlAudioSignalCommand(
				ISO_ALARM_LINE_FAILURE_ACTIVATIONS,
				ISO_ALARM_LINE_FAILURE_FREQUENCY_HZ,
				ISO_ALARM_LINE_FAILURE_ON_TIME_MS,
				ISO_ALARM_LINE_FAILURE_OFF_TIME_MS);
		START_TIMER(AlarmTimeoutTimer,
			((ISO_ALARM_LINE_FAILURE_ON_TIME_MS + ISO_ALARM_LINE_FAILURE_OFF_TIME_MS)*ISO_ALARM_LINE_FAILURE_ACTIVATIONS) - 5);
		bHighPrioAudioInProcess = true;
		bKeepLineHighPrioAlarm = false;
	} else if (bKeepLineMediumPrioAlarm && bMediumPrioAudioInProcess)
	{
		ISO_vControlAudioSignalCommand(
				ISO_ALARM_EXCEEDED_SPEED_ACTIVATIONS,
				ISO_ALARM_EXCEEDED_SPEED_FREQUENCY_HZ,
				ISO_ALARM_EXCEEDED_SPEED_ON_TIME_MS,
				ISO_ALARM_EXCEEDED_SPEED_OFF_TIME_MS);
		START_TIMER(AlarmTimeoutTimer,
			((ISO_ALARM_EXCEEDED_SPEED_ON_TIME_MS + ISO_ALARM_EXCEEDED_SPEED_OFF_TIME_MS)*ISO_ALARM_EXCEEDED_SPEED_ACTIVATIONS) - 5);
		bMediumPrioAudioInProcess = true;
		bKeepLineMediumPrioAlarm = false;
	} else
	{
		bHighPrioAudioInProcess = false;
		bMediumPrioAudioInProcess = false;
		bKeepLineHighPrioAlarm = false;
		bKeepLineMediumPrioAlarm = false;
	}
}

void ISO_vTimerCallbackIsobusCANStatus (void const *arg)
{
	event_e eEvt;

	DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_GET_STATUS, (void*) &sCANIsobusStatus);

	eEvt = EVENT_GUI_UPDATE_SYSTEM_CAN_INTERFACE;
	PUT_LOCAL_QUEUE(UpdateQ, eEvt, osWaitForever);
}

/******************************************************************************
 * Function : ISO_vIsobusUpdateOPThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will refresh the object pool.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vIsobusThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusUpdateOPThread (void const *argument)
{
	osEvent evt;
	event_e eRecvPubEvt;

	INITIALIZE_LOCAL_QUEUE(UpdateQ);
	INITIALIZE_TIMER(AlarmTimeoutTimer, osTimerOnce);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Isobus UpdateOP Thread Created");
#endif

	xUpdatePoolThreadId = osThreadGetId();

	ISO_vDetectThread(&WATCHDOG(ISOUPDT), &bISOUPDTThreadArrayPosition, (void*)ISO_vIsobusUpdateOPThread);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	osThreadId xIsoMainID = (osThreadId)argument;
	osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOUPDTThreadArrayPosition));   //Task created, inform core

	// Waiting for RUNNING module state
	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
	osSignalWait(ISO_FLAG_STATE_RUNNING, osWaitForever);
	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

	osThreadSetPriority(NULL, osPriorityHigh);

	ISO_vInitObjectStruct();

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
		evt = RECEIVE_LOCAL_QUEUE(UpdateQ, &eRecvPubEvt, osWaitForever);
		if(eModCurrState == UPDATING_LANGUAGE)
		{
			osFlagWait(ISO_sFlags, ISO_FLAG_LANGUAGE_UPDATED, true, false, osWaitForever);
		}
		WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

		if (evt.status == osEventMessage)
		{
			switch (eModCurrState)
			{
				case RUNNING:
				{
					switch (eRecvPubEvt)
					{
						case EVENT_GUI_UPDATE_PLANTER_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdatePlanterDataMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_UPDATE_INSTALLATION_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateInstallationDataMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateTestModeDataMask(eRecvPubEvt);
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_UPDATE_TRIMMING_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateTrimmingDataMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_UPDATE_SYSTEM_GPS_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateSystemGPSMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_UPDATE_SYSTEM_CAN_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateSystemCANMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_UPDATE_SYSTEM_SENSORS_INTERFACE:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateSystemSensorsMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION:
						{
							ISO_vUpdateTestModeDataMask(eRecvPubEvt);
							ISO_vChangeSoftKeyMaskCommand(DATA_MASK_INSTALLATION, MASK_TYPE_DATA_MASK,
								SOFT_KEY_MASK_INSTALLATION_FINISH);

							if (eCurrentMask == DATA_MASK_INSTALLATION)
							{
								ISO_vControlAudioSignalCommand(
										ISO_ALARM_SETUP_FINISHED_ACTIVATIONS,
										ISO_ALARM_SETUP_FINISHED_FREQUENCY_HZ,
										ISO_ALARM_SETUP_FINISHED_ON_TIME_MS,
										ISO_ALARM_SETUP_FINISHED_OFF_TIME_MS);
							}

							if ((*sConfigDataMask.bNumOfRows) != 36)
							{
								WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
								ISO_vUpdatePlanterDataMaskLines();
								WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							}

							event_e ePubEvt = EVENT_ISO_INSTALLATION_CONFIRM_INSTALLATION;
							PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
							break;
						}
						case EVENT_GUI_UPDATE_CONFIG:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vUpdateConfigurationDataMask();
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_CHANGE_ACTIVE_MASK_CONFIG_MASK:
						{
							WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
							ISO_vChangeActiveMask(DATA_MASK_CONFIGURATION);
							ISO_vChangeSoftKeyMaskCommand(DATA_MASK_CONFIGURATION, MASK_TYPE_DATA_MASK,
								SOFT_KEY_MASK_CONFIGURATION_CHANGES);
							ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_CONFIG, true);
							ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_SETUP, false);
							ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CANCEL_RET_PLANTER, false);
							ISO_vHideShowContainerCommand(CO_CFG_CHANGE_ONLY, true);
							ISO_vHideShowContainerCommand(CO_CFG_CHANGE_CLEAR_TOTALS, false);
							WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
							break;
						}
						case EVENT_GUI_ALARM_NEW_SENSOR:
						{
							ISO_vControlAudioSignalCommand(
									ISO_ALARM_SETUP_NEW_SENSOR_ACTIVATIONS,
									ISO_ALARM_SETUP_NEW_SENSOR_FREQUENCY_HZ,
									ISO_ALARM_SETUP_NEW_SENSOR_ON_TIME_MS,
									ISO_ALARM_SETUP_NEW_SENSOR_OFF_TIME_MS);
							break;
						}
						case EVENT_GUI_ALARM_DISCONNECTED_SENSOR:
						case EVENT_GUI_ALARM_LINE_FAILURE:
						case EVENT_GUI_ALARM_SETUP_FAILURE:
						{
							if (!bHighPrioAudioInProcess)
							{
								if (bMediumPrioAudioInProcess)
								{
									ISO_vControlAudioSignalCommand(
											ISO_ALARM_DEACTIVATE,
											ISO_ALARM_EXCEEDED_SPEED_FREQUENCY_HZ,
											ISO_ALARM_EXCEEDED_SPEED_ON_TIME_MS,
											ISO_ALARM_EXCEEDED_SPEED_OFF_TIME_MS);
								}
								ISO_vControlAudioSignalCommand(
										ISO_ALARM_LINE_FAILURE_ACTIVATIONS,
										ISO_ALARM_LINE_FAILURE_FREQUENCY_HZ,
										ISO_ALARM_LINE_FAILURE_ON_TIME_MS,
										ISO_ALARM_LINE_FAILURE_OFF_TIME_MS);
								STOP_TIMER(AlarmTimeoutTimer);
								START_TIMER(AlarmTimeoutTimer,
									((ISO_ALARM_LINE_FAILURE_ON_TIME_MS + ISO_ALARM_LINE_FAILURE_OFF_TIME_MS)*ISO_ALARM_LINE_FAILURE_ACTIVATIONS) - 5);
								bHighPrioAudioInProcess = true;
								bKeepLineHighPrioAlarm = false;
							} else
							{
								bKeepLineHighPrioAlarm = true;
							}
							break;
						}
						case EVENT_GUI_ALARM_EXCEEDED_SPEED:
						case EVENT_GUI_ALARM_GPS_FAILURE:
						{
							if (!bHighPrioAudioInProcess && !bMediumPrioAudioInProcess)
							{
								ISO_vControlAudioSignalCommand(
										ISO_ALARM_EXCEEDED_SPEED_ACTIVATIONS,
										ISO_ALARM_EXCEEDED_SPEED_FREQUENCY_HZ,
										ISO_ALARM_EXCEEDED_SPEED_ON_TIME_MS,
										ISO_ALARM_EXCEEDED_SPEED_OFF_TIME_MS);
								STOP_TIMER(AlarmTimeoutTimer);
								START_TIMER(AlarmTimeoutTimer,
									((ISO_ALARM_EXCEEDED_SPEED_ON_TIME_MS + ISO_ALARM_EXCEEDED_SPEED_OFF_TIME_MS)*ISO_ALARM_EXCEEDED_SPEED_ACTIVATIONS) - 5);
								bMediumPrioAudioInProcess = true;
								bKeepLineMediumPrioAlarm = false;
							} else
							{
								bKeepLineMediumPrioAlarm = true;
							}
							break;
						}
						case EVENT_GUI_ALARM_TOLERANCE:
						{
							if (!bHighPrioAudioInProcess && !bMediumPrioAudioInProcess)
							{
								sUptPlanterMask.eUpdateState = UPDATE_PLANTER_ALARM;
								sUptPlanterMask.eAlarmEvent = EVENT_GUI_ALARM_TOLERANCE;
							}
							break;
						}
						case EVENT_GUI_CONFIG_CHECK_PASSWORD_ACK:
						{
							if (ePasswordManager == PASSWD_ACCEPTED)
							{
								bPasswsNumDigits = 0;
								ePasswordManager = PASSWD_IDLE;
								ISO_vChangeActiveMask(DATA_MASK_CONFIGURATION);
								ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_1, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_2, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_3, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_4, false);
							} else if (ePasswordManager == PASSWD_CHANGE_PASSWD_NEW_PASSWD)
							{
								bPasswsNumDigits = 0;
								ISO_vHideShowContainerCommand(CO_PASSWD_NEW_PASSWD, true);
								ISO_vHideShowContainerCommand(CO_PASSWD_CURRENT_PASSWD, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_INCORRECT_PASSWORD, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_ENTER_PASSWD, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_1, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_2, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_3, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_4, false);
							}
							break;
						}
						case EVENT_GUI_CONFIG_CHECK_PASSWORD_NACK:
						{
							if ((ePasswordManager == PASSWD_NOT_ACCEPTED) || (ePasswordManager == PASSWD_CHANGE_NOT_ACCEPTED))
							{
								ISO_vHideShowContainerCommand(CO_PASSWD_INCORRECT_PASSWORD, true);
								ISO_vHideShowContainerCommand(CO_PASSWD_ENTER_PASSWD, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_CURRENT_PASSWD, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_NEW_PASSWD, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_1, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_2, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_3, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_4, false);
								bPasswsNumDigits = 0;
								ePasswordManager = (ePasswordManager == PASSWD_NOT_ACCEPTED) ? PASSWD_ENTER_PASSWORD : PASSWD_CHANGE_PASSWD_CURRENT_PASSWD;
							}
							break;
						}
						case EVENT_GUI_CONFIG_CHANGE_PASSWORD_ACK:
						{
							if (ePasswordManager == PASSWD_CHANGE_ACCEPTED)
							{
								bPasswsNumDigits = 0;
								ePasswordManager = PASSWD_IDLE;
								ISO_vChangeActiveMask(DATA_MASK_CONFIGURATION);
								ISO_vHideShowContainerCommand(CO_PASSWD_ACCEPT_BUTTON, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_1, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_2, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_3, false);
								ISO_vHideShowContainerCommand(CO_PASSWD_DIGIT_4, false);
							}
							break;
						}
						default:
							break;
					}
					break;
				}
				default:
					break;
			}
		}
	}
	osThreadTerminate(NULL);
}
#else
void ISO_vIsobusUpdateOPThread(void const *argument)
{}
#endif

void ISO_vTreatUpdateDataEvent (event_e ePubEvt)
{
	static sConfigurationData GUIConfigurationData;
	switch (eCurrentMask)
	{
		case DATA_MASK_CONFIRM_CONFIG_CHANGES:
		{
			ISO_vUpdateSisConfigData(&GUIConfigurationData);
			PUBLISH_MESSAGE(Isobus, EVENT_ISO_UPDATE_CURRENT_CONFIGURATION, EVENT_CLEAR, &GUIConfigurationData);
			break;
		}
		default:
			break;
	}
}

void ISO_vTreatUpdateReplaceSensorEvent (tsPubSensorReplacement sReplacement)
{
	switch (sReplacement.eReplacState) {
		case REPLACEMENT_NO_ERROR:
		{
			ISO_vChangeNumericValue(ON_REPLACE_NUMBER_LINE_NUMBER, sReplacement.bAvailableLine);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_DESCRIPTION, true);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ACCEPT, true);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NOT_ALLOWED, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NO_SENSORS, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_WAIT, false);
			break;
		}
		case REPLACEMENT_ERR_NOT_ALLOWED:
		{
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NOT_ALLOWED, true);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NO_SENSORS, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_DESCRIPTION, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_WAIT, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ACCEPT, false);
			break;
		}
		case REPLACEMENT_ERR_NO_SENSOR:
		{
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NO_SENSORS, true);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ERR_NOT_ALLOWED, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_DESCRIPTION, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_WAIT, false);
			ISO_vHideShowContainerCommand(CO_REPLACE_SENSOR_ACCEPT, false);
			break;
		}
		default:
			break;
	}
}
