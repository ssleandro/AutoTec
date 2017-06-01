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
#include "debug_tool.h"
#include "iso11783.h"
#include "../../isobus/config/isobus_config.h"
#include "isobus_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of SENSOR queue
#define QUEUE_SIZEOFISOBUS (16)

#define THIS_MODULE MODULE_ISOBUS

#define ISO_FLAG_STATE_RUNNING 0x00800000

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static eAPPError_s eError;                   		//!< Error variable

tsAcumulados* psAccumulated;

extern osFlagsGroupId UOS_sFlagSis;

extern uint32_t getID(uint32_t pgn, uint32_t sa, uint32_t da, uint32_t prio);

static sNumberVariableObj asNumVarObjects[ISO_NUM_NUMBER_VARIABLE_OBJECTS];
static sFillAttributesObj asNumFillAttributesObjects[ISO_NUM_FILL_ATTRIBUTES_OBJECTS];
static sInputListObj asConfigInputList[ISO_NUM_INPUT_LIST_OBJECTS];

static sBarGraphStatus asLinesStatus[ISO_NUM_BAR_GRAPH_OBJECTS];
static sBarGraphStatus asIndividualLineStatus[ISO_NUM_BAR_GRAPH_OBJECTS];

static eInstallationStatus eSensorsIntallStatus[36];
static sInstallSensorStatus sInstallStatus;
static sTrimmingStatus sLinesTrimmingStatus;

static sLineCountVariables sSeedsCountVar;

static sConfigurationDataMask sConfigDataMask =
{
	.eLanguage = (eSelectedLanguage*)(&asConfigInputList[0].bSelectedIndex),
	.eUnit = (eSelectedUnitMeasurement*)(&asConfigInputList[1].bSelectedIndex),
	.dVehicleID = &(asNumVarObjects[0].dValue),
	.eMonitor = AREA_MONITOR_DISABLED,
	.fSeedsPerMeter = &(asNumVarObjects[1].fValue),
	.bNumOfRows = (uint8_t*)(&asNumVarObjects[2].dValue),
	.fImplementWidth = &(asNumVarObjects[3].fValue),
	.fEvaluationDistance = &(asNumVarObjects[4].fValue),
	.bTolerance = (uint8_t*)(&asNumVarObjects[5].dValue),
	.fMaxSpeed = &(asNumVarObjects[6].fValue),
	.eAlterRows = ALTERNATE_ROWS_DISABLED,
	.eAltType = ALTERNATED_ROWS_ODD
};

const sPlantingVariables sPlantingVar =
{
	.psNumberVariable = &asNumVarObjects[0],
	.bNumOfVariables = 9
};

static sInstallationDataMask sInstallDataMask =
{
	.psLinesInstallStatus = &sInstallStatus
};

static sTrimmingDataMask sTrimminDataMask =
{
	.psTrimmedLines = &sLinesTrimmingStatus
};

#ifndef UNITY_TEST
DECLARE_QUEUE(IsobusQueue, QUEUE_SIZEOFISOBUS);     //!< Declaration of Interface Queue
CREATE_SIGNATURE(Isobus);                         	//!< Signature Declarations
#endif

CREATE_CONTRACT(Isobus);                            //!< Create contract for isobus msg publication

/*****************************
 * Local messages queue
 *****************************/
CREATE_LOCAL_QUEUE(PublishQ, event_e, 16)
CREATE_LOCAL_QUEUE(WriteQ, ISOBUSMsg, 32)
CREATE_LOCAL_QUEUE(ManagementQ, ISOBUSMsg, 32)
CREATE_LOCAL_QUEUE(UpdateQ, event_e, 16)
CREATE_LOCAL_QUEUE(BootQ, ISOBUSMsg, 32)

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

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(ISOPUB);                                    //!< WDT pointer flag
WATCHDOG_CREATE(ISORCV);                                    //!< WDT pointer flag
WATCHDOG_CREATE(ISOWRT);                                    //!< WDT pointer flag
WATCHDOG_CREATE(ISOMGT);                                   	//!< WDT pointer flag
WATCHDOG_CREATE(ISOUPDT);                                   //!< WDT pointer flag
WATCHDOG_CREATE(ISOBOOT);                                   //!< WDT pointer flag
uint8_t bISOPUBThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISORCVThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISOWRTThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISOMGTThreadArrayPosition = 0;                    	//!< Thread position in array
uint8_t bISOUPDTThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bISOBOOTThreadArrayPosition = 0;                    //!< Thread position in array

osThreadId xBootThreadId;                                  // Holds the BootThreadId
osThreadId xAuxBootThreadId;                               // Holds the AuxBootThreadId
osThreadId xUpdatePoolThreadId;							   // Holds the UpdatePoolThreadId

eBootStates eCurrState;

// Holds the current module state
eModuleStates eModCurrState;
eDataMask eCurrentDataMask = DATA_MASK_INSTALLATION;

VTStatus sVTCurrentStatus;                          //!< Holds the current VT status
peripheral_descriptor_p pISOHandle = NULL;          //!< ISO Handler

// Mutex to VT status control structure
CREATE_MUTEX(MTX_VTStatus);

extern unsigned int POOL_SIZE;
extern uint8_t pool[];

// Installation
eInstallationStatus InstallationStatus[36];


/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void ISO_vUpdateConfigurationDataMask(void);
void ISO_vUpdateInstallationDataMask(void);
void ISO_vUpdatePlanterDataMask(void);
void ISO_vUpdateTestModeDataMask(void);
void ISO_vUpdateTrimmingDataMask(void);
void ISO_vUpdateSystemDataMask(void);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * ISO_WDTData(uint8_t * pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void ISO_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
    *bPosition = 0;
    while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
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
static void ISO_vCreateThread(const Threads_t sThread )
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

    // Holds the BootThreadId, note that BootThreadId is the third position at THREADS_THISTHREAD array
    (sThread.thisWDTPosition == 3) ? xBootThreadId = xThreads : xBootThreadId;

    // Holds the AuxBootThreadId, note that AuxBootThreadId is the sixth position at THREADS_THISTHREAD array
    (sThread.thisWDTPosition == 6) ? xAuxBootThreadId = xThreads : xAuxBootThreadId;

    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
    }
}

/******************************************************************************
 * Function : ISO_eReceivePooling(void)
 *//**
 * \b Description:
 *
 * This is the main routine of the ISO_vIsobusPublishThread task. It performs a call to the
 * device read function and check if there is a received message on the buffer. The received
 * message is then published to the ISOBUS mod topic.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: none
 *
 * @return Number of bytes received by the device and published to the topic.
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * //Called from DIG_vDiagnosticPublishThread
~~~~~~~~~~~~~~~
  *
  * @see DIG_vDiagnosticThread, DIG_vDiagnosticPublishThread
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

uint32_t ISO_eReceivePooling(void)
{
    /* Check if the receive buffer has some data */
    uint8_t abTempBuffer[256];
    uint32_t wReceiveLenght = DEV_read(pISOHandle, abTempBuffer, 256);

    if (wReceiveLenght)
    {
        /* Publish the array at the DIAGNOSTIC topic */
        MESSAGE_HEADER(Isobus, wReceiveLenght, 1, MT_ARRAYBYTE);
        MESSAGE_PAYLOAD(Isobus) = (void*) abTempBuffer;
        PUBLISH(CONTRACT(Isobus), 0);
    }    
    return wReceiveLenght;
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
eAPPError_s ISO_eInitIsobusPublisher(void)
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
void ISO_vIsobusPublishThread(void const *argument)
{
	osEvent evt;
	event_e ePubEvt;
	PubMessage sISOPubMessage;

    INITIALIZE_LOCAL_QUEUE(PublishQ);           //!< Initialise message queue to publish thread

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Publish Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOPUB), &bISOPUBThreadArrayPosition, (void*)ISO_vIsobusPublishThread);
    WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bISOPUBThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    ISO_eInitIsobusPublisher();

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOPUB, WDT_SLEEP);
        evt = RECEIVE_LOCAL_QUEUE(PublishQ, &ePubEvt, osWaitForever);
        WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

        if(evt.status == osEventMessage)
        {
        	ePubEvt = (event_e) evt.value.v;
        	switch (ePubEvt) {
				case EVENT_ISO_UPDATE_CURRENT_DATA_MASK:
				{
		    		sISOPubMessage.dEvent = ePubEvt;
		    		sISOPubMessage.eEvtType = EVENT_UPDATE;
		    		sISOPubMessage.vPayload = (void*) &eCurrentDataMask;
		            MESSAGE_PAYLOAD(Isobus) = (void*) &sISOPubMessage;
		            PUBLISH(CONTRACT(Isobus), 0);
					break;
				}
				case EVENT_ISO_UPDATE_CURRENT_CONFIGURATION:
				{
		    		sISOPubMessage.dEvent = ePubEvt;
		    		sISOPubMessage.eEvtType = EVENT_UPDATE;
		    		sISOPubMessage.vPayload = (void*) &sConfigDataMask;
		            MESSAGE_PAYLOAD(Isobus) = (void*) &sISOPubMessage;
		            PUBLISH(CONTRACT(Isobus), 0);
					break;
				}
				case EVENT_ISO_INSTALLATION_REPEAT_TEST:
				{
		    		sISOPubMessage.dEvent = ePubEvt;
		    		sISOPubMessage.eEvtType = EVENT_SET;
		    		sISOPubMessage.vPayload = NULL;
		            MESSAGE_PAYLOAD(Isobus) = (void*) &sISOPubMessage;
		            PUBLISH(CONTRACT(Isobus), 0);
					break;
				}
				case EVENT_ISO_INSTALLATION_ERASE_INSTALLATION:
				{
		    		sISOPubMessage.dEvent = ePubEvt;
		    		sISOPubMessage.eEvtType = EVENT_SET;
		    		sISOPubMessage.vPayload = NULL;
		            MESSAGE_PAYLOAD(Isobus) = (void*) &sISOPubMessage;
		            PUBLISH(CONTRACT(Isobus), 0);
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
void ISO_vIsobusPublishThread(void const *argument){}
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
eAPPError_s ISO_vInitDeviceLayer(uint32_t wSelectedInterface)
{
    /*Prepare the device */
    pISOHandle = DEV_open(PERIPHERAL_M2GISOCOMM);
    ASSERT(pISOHandle != NULL);

    /* Set device output to CAN peripheral */
    eError = (eAPPError_s) DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_SET_ACTIVE, (void*)&wSelectedInterface);
    ASSERT(eError == APP_ERROR_SUCCESS);

    /* Configurations for CAN multipacket communications */
    if (wSelectedInterface == ISOBUS_DEV_CAN)
    {
        /* Enables the reception of frames with the ISO_INIT_CAN_ID id number (29 bits) */
        uint32_t wCANInitID = 0x00000000;
        eError = (eAPPError_s) DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_ADD_ALL_CAN_ID, (void*)&wCANInitID);
        ASSERT(eError == APP_ERROR_SUCCESS);
    }    
    return eError;
}

void ISO_vIdentifyEvent (contract_s* contract)
{
	event_e eEvt;

	switch (contract->eOrigin)
	{
		case MODULE_GUI:
		{
			eEvt = GET_PUBLISHED_EVENT(contract);
			switch (eEvt) {
				case EVENT_GUI_UPDATE_INSTALLATION_INTERFACE:
				{
					memcpy(&eSensorsIntallStatus, (eInstallationStatus*)GET_PUBLISHED_PAYLOAD(contract), sizeof(eSensorsIntallStatus));
					osMessagePut(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_PLANTER_INTERFACE:
				{
					osMessagePut(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
				{
					if(psAccumulated == NULL)
					{
						psAccumulated = (tsAcumulados*) GET_PUBLISHED_PAYLOAD(contract);
					}
					osMessagePut(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_TRIMMING_INTERFACE:
				{
					osMessagePut(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_UPDATE_SYSTEM_INTERFACE:
				{
					osMessagePut(UpdateQ, eEvt, osWaitForever);
					break;
				}
				case EVENT_GUI_INSTALLATION_FINISH:
				{
					osMessagePut(UpdateQ, eEvt, osWaitForever);
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

    status = osFlagGroupCreate(&ISO_sFlags);
    ASSERT(status == osOK);

    /* Prepare the signature - struture that notify the broker about subscribers */
    SIGNATURE_HEADER(Isobus, THIS_MODULE, TOPIC_GUI, IsobusQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(Isobus), 0) == osOK);

    /* Init M2GISOCOMM device for output */
    ISO_vInitDeviceLayer(ISO_INITIAL_IO_IFACE);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while(THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        ISO_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }

    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId) argument;
    osSignalSet(xMainFromIsobusID, MODULE_ISOBUS);

    /* Start the main functions of the application */
    while (1)
    {
        /* Blocks until any message is published on ISOBUS topic */
        WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
        evt = RECEIVE(IsobusQueue, osWaitForever);
        WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

        if(evt.status == osEventMessage)
        {
        	ISO_vIdentifyEvent(GET_CONTRACT(evt));
        }
    }

    /* Unreachable */
    osThreadSuspend(NULL);
}
#else
void ISO_vIsobusThread (void const *argument){}
#endif

/* *
 * Get the PGN from an message
 * */
uint32_t ISO_wGetPGN(ISOBUSMsg* RcvMsg) {
    return (RcvMsg->PF < 240) ? (((RcvMsg->frame.id >> 8) & 0x3FFFF) - RcvMsg->PS) : ((RcvMsg->frame.id >> 8) & 0x3FFFF);
} 

void ISO_vIsobusDispatcher(ISOBUSMsg* RcvMsg)
{
    uint32_t wRcvMsgPGN = ISO_wGetPGN(RcvMsg);

    // Two types of behavior...
    // Boot mode and running mode... Must be implemented...
    // This function must send the received messages to the responsably threads...
    // BootNControl Thread and UpdateOP Thread

    // On boot status, all the received message must be sent to BootNControl thread
    // On running status, this function must dispatch to the correct thread
    if((RcvMsg->PF < 240) && (wRcvMsgPGN != REQUEST_PGN_MSG_PGN))  // Destination PGN
    {
        if(RcvMsg->PS == M2G_SOURCE_ADDRESS)
        {
            if(RcvMsg->SA == VT_ADDRESS)
            {
                switch(wRcvMsgPGN)
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
                        // Send message to BootThread
                        PUT_LOCAL_QUEUE(ManagementQ, *RcvMsg, osWaitForever);
                        break;
                    default:
                        break;
                }
            } else{
            }
        } else if(RcvMsg->PS == BROADCAST_ADDRESS)
        {
            if(RcvMsg->SA == VT_ADDRESS)
            {
                switch(wRcvMsgPGN)
                {
                    case VT_TO_ECU_PGN:
                        // Send message to BootThread
                        PUT_LOCAL_QUEUE(ManagementQ, *RcvMsg, osWaitForever);
                        break;
                    case ECU_TO_GLOBAL_PGN:
                        break;
                    default:
                        break;
                }
            } else{
            }
        }
    } else if(RcvMsg->PF >= 240)        // No-Destination PGN
    {
        if(RcvMsg->SA == VT_ADDRESS)
        {
            switch(wRcvMsgPGN)
            {
                // SAVE 8 BYTES OF DATA IN DEDICATED BUFFER
                default:
                    break;
            }
        } else{
            // SAVE 12 BYTE MESSAGE (ID AND DATA) IN CIRCULAR QUEUE
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
void ISO_vIsobusRecvThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Recv Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISORCV), &bISORCVThreadArrayPosition, (void*)ISO_vIsobusRecvThread);
    WATCHDOG_STATE(ISORCV, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISORCVThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    uint8_t bIterator;
    uint8_t bRecvMessages = 0;      //!< Lenght (messages) received
    ISOBUSMsg asPayload[32];   //!< Buffer to hold the contract and message data

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISORCV, WDT_ACTIVE);
        osDelay(250); //Wait
        bRecvMessages = DEV_read(pISOHandle, &asPayload[0].frame, sizeof(asPayload));

        if(bRecvMessages)
        {
            for(bIterator = 0; bIterator < bRecvMessages; bIterator++)
            {
                // Call an function to treat ISOBUS messages
                ISO_vIsobusDispatcher(&asPayload[bIterator]);
            }
        }
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusRecvThread(void const *argument){}
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
void ISO_vIsobusWriteThread(void const *argument)
{
    ISOBUSMsg recv;
    eAPPError_s eError;

	INITIALIZE_LOCAL_QUEUE(WriteQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Write Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOWRT), &bISOWRTThreadArrayPosition, (void*)ISO_vIsobusWriteThread);
    WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOWRTThreadArrayPosition));//Task created, inform core

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOWRT, WDT_SLEEP);
        osEvent evtPub = RECEIVE_LOCAL_QUEUE(WriteQ, &recv, osWaitForever);   // Wait
        WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);

        if(evtPub.status == osEventMessage)
        {
            eError = (eAPPError_s) DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_CHANGE_SEND_ID, (void*)&(recv.frame).id);
            ASSERT(eError == APP_ERROR_SUCCESS);

            if(eError == APP_ERROR_SUCCESS)     // wSendCANID changed
            {
                DEV_write(pISOHandle, &((recv.frame).data[0]), (recv.frame).dlc);
            }
        }
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusWriteThread(void const *argument){}
#endif

void ISO_vTimerCallbackWSMaintenance(void const *arg)
{
    ISO_vSendWorkingSetMaintenance(false);
}

#ifndef UNITY_TEST
void ISO_vIsobusBootThread(void const *argument)
{
    ISOBUSMsg RcvMsg;

    INITIALIZE_LOCAL_QUEUE(BootQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Aux Boot Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOBOOT), &bISOBOOTThreadArrayPosition, (void*)ISO_vIsobusBootThread);
    WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOBOOTThreadArrayPosition));//Task created, inform core

    CREATE_TIMER(WSMaintenanceTimer, ISO_vTimerCallbackWSMaintenance);
    INITIALIZE_TIMER(WSMaintenanceTimer, osTimerPeriodic);

    // Inform BootThread that AuxBootThread already start
    WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
    osSignalSet(xBootThreadId, 0xFF);
    WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

    // Wait for an VT status message
    WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
    osSignalWait(WAIT_GLOBAL_VT_STATUS, osWaitForever);
    WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

    // Send a request address claim message
    ISO_vSendRequest(ADDRESS_CLAIM_PGN);

    // Send a address claim message
    ISO_vSendAddressClaimed();

    // Wait for an VT status message
    eCurrState = WAIT_VT_STATUS;

    while(1)
    {
        while(eCurrState != BOOT_COMPLETED)
        {
            WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
            osSignalWait(eCurrState, osWaitForever);
            WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

            switch(eCurrState)
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
                    ISO_vSendGetMemory(POOL_SIZE);
                    // Send a load version message
                    ISO_vSendLoadVersion(0xAAAAAAABAAAAAA);

                    break;
                }
                case WAIT_LOAD_VERSION:
                {
                    // Send a request to send message
                    ISO_vSendRequestToSend();
                    eCurrState = WAIT_SEND_POOL;
                    break;
                }
                case WAIT_SEND_POOL:
                {
                    ISO_vInitPointersToTranfer(pool, POOL_SIZE);
                    // Waits for a CTS message
                    // While object pool pointer not equal to NULL
                    do{
                        WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
                        osEvent evtPub = RECEIVE_LOCAL_QUEUE(BootQ, &RcvMsg, osWaitForever);   // Wait
                        WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

                        if(evtPub.status == osEventMessage)
                        {
                            switch(ISO_wGetPGN(&RcvMsg))
                            {
                                case TP_CONN_MANAGE_PGN:
                                    switch (RcvMsg.B1) {
                                        case TP_CM_CTS:
                                            ISO_vSendObjectPool(RcvMsg.B2, RcvMsg.B3, TRANSPORT_PROTOCOL);
                                            break;
                                        case TP_EndofMsgACK:
                                            eCurrState = OBJECT_POOL_SENDED;
                                            break;
                                        case TP_Conn_Abort:
                                            eCurrState = WAIT_VT_STATUS;
                                            break;
                                        case TP_BAM:
                                        case TP_CM_RTS:
                                        default:
                                            break;
                                    }
                                    break;
                                case ETP_CONN_MANAGE_PGN:
                                    switch (RcvMsg.B1) {
                                        case ETP_CM_CTS:
                                            // Send DPO message
                                            ISO_vSendETP_CM_DPO(RcvMsg.B2, (RcvMsg.B3|(RcvMsg.B4 << 8)|(RcvMsg.B5 << 16)));
                                            ISO_vSendObjectPool(RcvMsg.B2, (RcvMsg.B3|(RcvMsg.B4 << 8)|(RcvMsg.B5 << 16)), EXTENDED_TRANSPORT_PROTOCOL);
                                            break;
                                        case ETP_CM_EOMA:
                                            eCurrState = OBJECT_POOL_SENDED;
                                            break;
                                        case ETP_Conn_Abort:
                                            // We have to skip the do...while loop...
                                            eCurrState = WAIT_VT_STATUS;
                                            break;
                                        case ETP_CM_DPO:
                                        case ETP_CM_RTS:
                                        default:
                                            break;
                                    }
                                    break;
                            }
                        }
                    } while(eCurrState != OBJECT_POOL_SENDED);

                    // Create a new state in this machine, to restart the pool send
                    ISO_vSendEndObjectPool();
                    ISO_vSendWSMaintenancePoolSent();
                    ISO_vSendLoadVersion(0xAAAAAAABAAAAAA);
                    ISO_vSendStoreVersion(0xAAAAAAABAAAAAA);

                    eCurrState = OBJECT_POOL_LOADED;
                    osSignalSet(xAuxBootThreadId, OBJECT_POOL_LOADED);
                    break;
                }
                case OBJECT_POOL_LOADED:
                {
                    START_TIMER(WSMaintenanceTimer, 800);

                    // The boot process are completed, so terminate this thread
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
        WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
        osDelay(5000);
        WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);
        // TODO: Change task priority and suspend this thread. Useful only when BOOT is not completed
    } // End of while
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusBootThread(void const *argument){}
#endif

void ISO_vIsobusUpdateVTStatus(ISOBUSMsg* RcvMsg)
{
	osStatus status;

    if(RcvMsg->B1 != FUNC_VT_STATUS)
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

void ISO_vTreatBootState(ISOBUSMsg* sRcvMsg)
{
    switch(ISO_wGetPGN(sRcvMsg))
    {
        case VT_TO_ECU_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                switch(sRcvMsg->B1)
                {
                    case FUNC_LOAD_VERSION:
    					if(sRcvMsg->B6 == 0)
    					{
    						eCurrState = OBJECT_POOL_LOADED;
    						osSignalSet(xAuxBootThreadId, OBJECT_POOL_LOADED);
    					} else{
    						eCurrState = WAIT_LOAD_VERSION;
                            osSignalSet(xAuxBootThreadId, WAIT_LOAD_VERSION);
    					}
                        break;
                    case FUNC_VT_STATUS:
                        osSignalSet(xAuxBootThreadId, WAIT_VT_STATUS);
                        break;
                    default:
                        break;
                }
            } else if(sRcvMsg->PS == BROADCAST_ADDRESS){
                switch(sRcvMsg->B1)
                {
                    case FUNC_VT_STATUS:
                        osSignalSet(xAuxBootThreadId, WAIT_GLOBAL_VT_STATUS);
                        break;
                    default:
                        break;
                }
            }
            break;
        case TP_CONN_MANAGE_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                osSignalSet(xAuxBootThreadId, WAIT_SEND_POOL);
                PUT_LOCAL_QUEUE(BootQ, *sRcvMsg, osWaitForever);
            }
            break;
        case ETP_CONN_MANAGE_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                osSignalSet(xAuxBootThreadId, WAIT_SEND_POOL);
                PUT_LOCAL_QUEUE(BootQ, *sRcvMsg, osWaitForever);
            }
            break;
        default:
            break;
    }
}

void ISO_vInitObjectStruct(void)
{
	// Init number variable objects
	for (int i = 0; i < ARRAY_SIZE(asNumVarObjects); i++) {
		asNumVarObjects[i].wObjID = ISO_OBJECT_NUMBER_VARIABLE_ID + i;
	}

	// Initialize fill attributes objects
	for (int i = 0; i < ARRAY_SIZE(asNumFillAttributesObjects); i++) {
		asNumFillAttributesObjects[i].wObjID = ISO_OBJECT_FILL_ATTRIBUTES_ID + i;
	}

	// Initialize sensor install status structure
	sInstallStatus.bNumOfSensors = 36;
	sInstallStatus.pFillAttribute = &asNumFillAttributesObjects[0];

	for (int i = 0; i < sInstallStatus.bNumOfSensors; i++) {
		if(i < *sConfigDataMask.bNumOfRows)
		{
			sInstallStatus.pFillAttribute[i].bColor = STATUS_INSTALL_INSTALLING;
		} else{
			sInstallStatus.pFillAttribute[i].bColor = STATUS_INSTALL_NONE;
		}
	}

	// Initialize sensor trimming status structure
	sLinesTrimmingStatus.bNumOfSensor = 36;
	sLinesTrimmingStatus.pFillAtributte = &asNumFillAttributesObjects[36];

	for (int i = 0; i < sInstallStatus.bNumOfSensors; i++) {
		if(i < *sConfigDataMask.bNumOfRows)
		{
			sLinesTrimmingStatus.pFillAtributte[i].bColor = STATUS_TRIMMING_NOT_TRIMMED;
		} else{
			sLinesTrimmingStatus.pFillAtributte[i].bColor = STATUS_TRIMMING_NONE;
		}
	}

	// Initialize input list objects
	for (int i = 0; i < ARRAY_SIZE(asConfigInputList); i++) {
		asConfigInputList[i].wObjID = ISO_OBJECT_INPUT_LIST_ID + i;
	}

}

// Used to FUNC_CHANGE_NUMERIC_VALUE message
void ISO_vInputNumberVariableValue(uint16_t wObjectID, uint32_t dValue)
{
	uint16_t index = GET_INDEX_FROM_ID(wObjectID);

	if(index < ARRAY_SIZE(asNumVarObjects))
	{
		if(asNumVarObjects[index].wObjID == wObjectID)
		{
			asNumVarObjects[index].dValue = dValue;
			asNumVarObjects[index].fValue = GET_FLOAT_VALUE(dValue);
		}
	}
}

void ISO_vInputIndexListValue(uint16_t wObjectID, uint32_t dValue)
{
	uint16_t index = GET_INDEX_FROM_ID(wObjectID);

	if(index < ARRAY_SIZE(asConfigInputList))
	{
		if(asConfigInputList[index].wObjID == wObjectID)
		{
			asConfigInputList[index].bSelectedIndex = dValue;
		}
	}
}

void ISO_vTreatChangeNumericValueEvent(ISOBUSMsg* sRcvMsg)
{
	uint16_t wObjectID = (sRcvMsg->B3 | (sRcvMsg->B2 << 8));
	uint32_t dValue = (sRcvMsg->B5 | (sRcvMsg->B6 << 8) | (sRcvMsg->B7 << 16) | (sRcvMsg->B8 << 24));

	if((wObjectID >= 0x8000) && (wObjectID < 0x8200))
	{
		ISO_vInputNumberVariableValue(wObjectID, dValue);
	} else if((wObjectID >= 0x9000) && (wObjectID < 0x9500)){
		ISO_vInputIndexListValue(wObjectID, dValue);
	}
}

void ISO_vTreatRunningState(ISOBUSMsg* sRcvMsg)
{
	uint16_t dAux;
	event_e ePubEvt;

    switch(ISO_wGetPGN(sRcvMsg))
    {
        case VT_TO_ECU_PGN:
        {
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                switch(sRcvMsg->B1)
                {
                    case FUNC_SOFT_KEY_ACTIVATION:
                        break;
                    case FUNC_BUTTON_ACTIVATION:
                    {
                    	dAux = ((sRcvMsg->B3 << 8) | (sRcvMsg->B4));
                    	switch (dAux) {
							case ISO_BUTTON_REPEAT_TEST_ID:
							{
								ePubEvt = EVENT_ISO_INSTALLATION_REPEAT_TEST;
								PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
								break;
							}
							case ISO_BUTTON_ERASE_INSTALLATION_ID:
							{
								ePubEvt = EVENT_ISO_INSTALLATION_ERASE_INSTALLATION;
								PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
								break;
							}
							default:
								break;
						}
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
                    	dAux = ((sRcvMsg->B2 << 8) | (sRcvMsg->B3));
                    	if((dAux >= DATA_MASK_CONFIGURATION) || (dAux < DATA_MASK_INVALID))
                    	{
                    		eCurrentDataMask = (eDataMask) dAux;
                    		ePubEvt = EVENT_ISO_UPDATE_CURRENT_DATA_MASK;
                    		PUT_LOCAL_QUEUE(PublishQ, ePubEvt, osWaitForever);
                    	}
                        break;
                    }
                    default:
                        break;
                }
            } else if(sRcvMsg->PS == BROADCAST_ADDRESS){
                switch(sRcvMsg->B1)
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
        default:
            break;
    }
}

/******************************************************************************
 * Function : ISO_vIsobusBootThread(void const *argument)
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
void ISO_vIsobusManagementThread(void const *argument)
{
	osEvent evt;
    ISOBUSMsg sRcvMsg;

    INITIALIZE_LOCAL_QUEUE(ManagementQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Management Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOMGT), &bISOMGTThreadArrayPosition, (void*)ISO_vIsobusManagementThread);
    WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOMGTThreadArrayPosition));//Task created, inform core

    // Wait for auxiliary thread start
    /* Pool the device waiting for */
    WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
    osSignalWait(0xFF, osWaitForever);
    WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOMGT, WDT_SLEEP);
        evt = RECEIVE_LOCAL_QUEUE(ManagementQ, &sRcvMsg, osWaitForever);   // Wait
        WATCHDOG_STATE(ISOMGT, WDT_ACTIVE);

        if(evt.status == osEventMessage)
        {
            switch(eModCurrState)
            {
                case BOOT:
                    ISO_vTreatBootState(&sRcvMsg);
                    break;
                case RUNNING:
                    ISO_vTreatRunningState(&sRcvMsg);
                    break;
                default:
                    break;
            }
        }        
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusWriteThread(void const *argument){}
#endif

ISOBUSMsg sSendMsg;

void ISO_vUpdateNumberVariableValue(uint16_t wOutputNumberID, uint32_t dNumericValue)
{
    sSendMsg.frame.id = getID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY);;
    sSendMsg.frame.dlc = 8;
    
    sSendMsg.frame.data[0] = FUNC_CHANGE_NUMERIC_VALUE;
    sSendMsg.frame.data[1] = (wOutputNumberID & 0xFF00) >> 8;
    sSendMsg.frame.data[2] = wOutputNumberID & 0xFF;
    sSendMsg.frame.data[3] = 0xFF;
    sSendMsg.frame.data[4] = dNumericValue & 0xFF;
    sSendMsg.frame.data[5] = (dNumericValue & 0xFF00) >> 8;
    sSendMsg.frame.data[6] = (dNumericValue & 0xFF0000) >> 16;
    sSendMsg.frame.data[7] = (dNumericValue & 0xFF000000) >> 24;
    
    PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateListItemValue(uint16_t wOutputNumberID, uint8_t bListItem)
{
    sSendMsg.frame.id = getID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY);;
    sSendMsg.frame.dlc = 8;

    sSendMsg.frame.data[0] = FUNC_CHANGE_LIST_ITEM;
    sSendMsg.frame.data[1] = (wOutputNumberID & 0xFF00) >> 8;
    sSendMsg.frame.data[2] = wOutputNumberID & 0xFF;
    sSendMsg.frame.data[3] = bListItem;
    sSendMsg.frame.data[4] = 0xFF;
    sSendMsg.frame.data[5] = 0xFF;
    sSendMsg.frame.data[6] = 0xFF;
    sSendMsg.frame.data[7] = 0xFF;

    PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateBarGraphColor(uint16_t wBarGraphID, uint32_t dNumericValue)
{
    sSendMsg.frame.id = getID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY);
    sSendMsg.frame.dlc = 8;
  
    sSendMsg.frame.data[0] = FUNC_CHANGE_ATTRIBUTE;
    sSendMsg.frame.data[1] = (wBarGraphID & 0xFF00) >> 8;
    sSendMsg.frame.data[2] = wBarGraphID & 0xFF;
    sSendMsg.frame.data[3] = 0x03;
    sSendMsg.frame.data[4] = dNumericValue & 0xFF;
    sSendMsg.frame.data[5] = (dNumericValue & 0xFF00) >> 8;
    sSendMsg.frame.data[6] = (dNumericValue & 0xFF0000) >> 16;
    sSendMsg.frame.data[7] = (dNumericValue & 0xFF000000) >> 24;
    
    PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateFillAttributesValue(uint16_t wFillAttrID, uint8_t bColor)
{
    sSendMsg.frame.id = getID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY);
    sSendMsg.frame.dlc = 8;

    sSendMsg.frame.data[0] = FUNC_CHANGE_FILL_ATTRIBUTES;
    sSendMsg.frame.data[1] = (wFillAttrID & 0xFF00) >> 8;
    sSendMsg.frame.data[2] = wFillAttrID & 0xFF;
    sSendMsg.frame.data[3] = 0x02;
    sSendMsg.frame.data[4] = bColor;
    sSendMsg.frame.data[5] = 0xFF;
    sSendMsg.frame.data[6] = 0xFF;
    sSendMsg.frame.data[7] = 0xFF;

    PUT_LOCAL_QUEUE(WriteQ, sSendMsg, osWaitForever);
}

void ISO_vUpdateConfigurationDataMask(void)
{
	*sConfigDataMask.eLanguage = LANGUAGE_ENGLISH;
	*sConfigDataMask.eUnit = UNIT_IMPERIAL_SYSTEM;
	*sConfigDataMask.dVehicleID = 1234;
	sConfigDataMask.eMonitor = AREA_MONITOR_DISABLED;
	*sConfigDataMask.fSeedsPerMeter = GET_UNSIGNED_INT_VALUE(2.5f);
	*sConfigDataMask.bNumOfRows = 2;
	*sConfigDataMask.fImplementWidth = 50;
	*sConfigDataMask.fEvaluationDistance = 50;
	*sConfigDataMask.bTolerance = 50;
	*sConfigDataMask.fMaxSpeed = GET_UNSIGNED_INT_VALUE(12.0f);
	sConfigDataMask.eAlterRows = ALTERNATE_ROWS_DISABLED;

	ISO_vUpdateNumberVariableValue(0x81E3, *sConfigDataMask.eLanguage);
//	ISO_vUpdateOutputNumberValue(0x9001, *sConfigDataMask.eUnit);

	ISO_vUpdateNumberVariableValue(0x8000, *sConfigDataMask.dVehicleID);
	ISO_vUpdateNumberVariableValue(0x8001, *sConfigDataMask.fSeedsPerMeter);
	ISO_vUpdateNumberVariableValue(0x8002, *sConfigDataMask.bNumOfRows);
	ISO_vUpdateNumberVariableValue(0x8003, *sConfigDataMask.fImplementWidth);
	ISO_vUpdateNumberVariableValue(0x8004, *sConfigDataMask.fEvaluationDistance);
	ISO_vUpdateNumberVariableValue(0x8005, *sConfigDataMask.bTolerance);
	ISO_vUpdateNumberVariableValue(0x8006, *sConfigDataMask.fMaxSpeed);
}

void ISO_vUpdateInstallationDataMask(void)
{
	for (int i = 0; i < sInstallStatus.bNumOfSensors; i++) {
		sInstallStatus.pFillAttribute[i].bColor = eSensorsIntallStatus[i];
		ISO_vUpdateFillAttributesValue(sInstallStatus.pFillAttribute[i].wObjID, sInstallStatus.pFillAttribute[i].bColor);
	}
}

void ISO_vUpdatePlanterDataMask(void)
{

}

void ISO_vUpdateTestModeDataMask(void)
{
	if(psAccumulated == NULL)
		return;

	for (int i = 0; i < *sConfigDataMask.bNumOfRows; i++) {
		ISO_vUpdateNumberVariableValue((0x805C + i), psAccumulated->sTotalReg.adSementes[i]);
	}
}

void ISO_vUpdateTrimmingDataMask(void)
{

}

void ISO_vUpdateSystemDataMask(void)
{

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
void ISO_vIsobusUpdateOPThread(void const *argument)
{
	osEvent evt;
	event_e ePubEvt;

    INITIALIZE_LOCAL_QUEUE(UpdateQ);

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus UpdateOP Thread Created");
#endif

    xUpdatePoolThreadId = osThreadGetId();

    ISO_vDetectThread(&WATCHDOG(ISOUPDT), &bISOUPDTThreadArrayPosition, (void*)ISO_vIsobusUpdateOPThread);
    WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOUPDTThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    // Waiting for RUNNING module state
    WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
    osSignalWait(ISO_FLAG_STATE_RUNNING, osWaitForever);
    WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

    osThreadSetPriority(NULL, osPriorityHigh);

    ISO_vUpdateConfigurationDataMask();

    ISO_vInitObjectStruct();

//    ISO_vUpdateInstallationDataMask();

    while(1)
    {
    	/* Pool the device waiting for */
    	WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
    	evt = RECEIVE_LOCAL_QUEUE(UpdateQ, &ePubEvt, osWaitForever);
    	WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

    	if(evt.status == osEventMessage)
    	{
    		switch (ePubEvt) {
				case EVENT_GUI_UPDATE_PLANTER_INTERFACE:
				{
					ISO_vUpdatePlanterDataMask();
					break;
				}
				case EVENT_GUI_UPDATE_INSTALLATION_INTERFACE:
				{
					ISO_vUpdateInstallationDataMask();
					break;
				}
				case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
				{
					ISO_vUpdateTestModeDataMask();
					break;
				}
				case EVENT_GUI_UPDATE_TRIMMING_INTERFACE:
				{
					ISO_vUpdateTrimmingDataMask();
					break;
				}
				case EVENT_GUI_UPDATE_SYSTEM_INTERFACE:
				{
					ISO_vUpdateSystemDataMask();
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
void ISO_vIsobusWriteThread(void const *argument){}
#endif
