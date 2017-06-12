/****************************************************************************
 * Title                 :   buzzer_core Source File
 * Filename              :   buzzer_core.c
 * Author                :   Henrique Reis
 * Origin Date           :
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
 *  XX/XX/XX   1.0.0      Henrique Reis    buzzer_core.c created.
 *
 *****************************************************************************/
/** @file   filesys_core.c
 *  @brief
 *
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "filesys_core.h"

#include <config_files.h>
#include "api_mdriver_span.h"
#include "debug_tool.h"
#include "../../filesys/config/filesys_config.h"
#include "filesys_ThreadControl.h"
#include "fat_sl.h"
#include "config_files.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of BUZZER queue
#define QUEUE_SIZEOFFILESYS (5)

#define THIS_MODULE MODULE_FILESYS

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/
extern osFlagsGroupId UOS_sFlagSis;

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static eAPPError_s eError;                          //!< Error variable

DECLARE_QUEUE(FileSysQueue, QUEUE_SIZEOFFILESYS);    //!< Declaration of Interface Queue
CREATE_SIGNATURE(FileSysControl);//!< Signature Declarations
CREATE_SIGNATURE(FileSysDiag);//!< Signature Declarations
CREATE_CONTRACT(FileSys);//!< Create contract for buzzer msg publication

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
FILESYS_MODULES
	};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

WATCHDOG_CREATE(FSMPUB);//!< WDT pointer flag
uint8_t bFSMPUBThreadArrayPosition = 0;                 //!< Thread position in array

peripheral_descriptor_p pFileSysHandle;          //!< SPIFI Handler

osThreadId xPbulishThreadID;

osFlagsGroupId FFS_sFlagSis;

PubMessage sFileSysPubMsg;

/******************************************************************************
 * Module Internal DATABASE
 *******************************************************************************/

UOS_tsConfiguracao FFS_sConfiguracao;
IHM_tsConfig FFS_sConfig;
AQR_tsRegEstaticoCRC FFS_sRegEstaticoCRC;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * FSM_WDTData (uint8_t * pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void FSM_vDetectThread (thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
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
 * Function : FSM_vCreateThread(const Threads_t sSensorThread )
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
 * @see FSM_vCreateThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *******************************************************************************/
static void FSM_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());
	ASSERT(xThreads != NULL);

	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}
#endif

/******************************************************************************
 * Function : BUZ_eInitBuzzerPublisher(void)
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
 * //Called from
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread, ISO_vIsobusPublishThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
eAPPError_s FSM_eInitFileSysPublisher (void)
{
	/* Check if handler is already enabled */

	//Prepare Default Contract/Message
	MESSAGE_HEADER(FileSys, 1, FILESYS_DEFAULT_MSGSIZE, MT_BYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(FileSys, 1, THIS_MODULE, TOPIC_FILESYS);

	return APP_ERROR_SUCCESS;
}

void FSM_ePublishEvent (uint32_t event, eEventType eventType, void *payload)
{
	sFileSysPubMsg.dEvent = event;
	sFileSysPubMsg.eEvtType = eventType;
	sFileSysPubMsg.vPayload = payload;
	MESSAGE_PAYLOAD(FileSys) = (void*)&sFileSysPubMsg;
	PUBLISH(CONTRACT(FileSys), 0);
}

/******************************************************************************
 * Function : FSM_vFileSysPublishThread(void const *argument)
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
void FSM_vFileSysPublishThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Buzzer Publish Thread Created");
#endif

	FSM_vDetectThread(&WATCHDOG(FSMPUB), &bFSMPUBThreadArrayPosition, (void*)FSM_vFileSysPublishThread);
	WATCHDOG_STATE(FSMPUB, WDT_ACTIVE);

	xPbulishThreadID = osThreadGetId();

	osSignalSet((osThreadId)argument, THREADS_RETURN_SIGNAL(bFSMPUBThreadArrayPosition)); //Task created, inform core
	osThreadSetPriority(NULL, osPriorityLow);

	FSM_eInitFileSysPublisher();

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(FSMPUB, WDT_SLEEP);
		osEvent sEvent = osSignalWait(FFS_FLAG_ALL, osWaitForever);
		WATCHDOG_STATE(FSMPUB, WDT_ACTIVE);

		osFlags dFlags = osFlagGet(FFS_sFlagSis);
		uint32_t tSignalBit = sEvent.value.v;

		if (tSignalBit & FFS_FLAG_STATUS)
		{
			if (dFlags & FFS_FLAG_STATUS)
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
			else
				osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
		}
		if (tSignalBit & FFS_FLAG_CFG)
		{
			if (dFlags & FFS_FLAG_CFG)
				FSM_ePublishEvent(EVENT_FFS_CFG, EVENT_SET, (void*)&FFS_sConfiguracao);
			else
				FSM_ePublishEvent(EVENT_FFS_CFG, EVENT_CLEAR, NULL);
		}
		if (tSignalBit & FFS_FLAG_INTERFACE_CFG)
		{
			if (dFlags & FFS_FLAG_INTERFACE_CFG)
				FSM_ePublishEvent(EVENT_FFS_INTERFACE_CFG, EVENT_SET, (void*)&FFS_sConfig);
			else
				FSM_ePublishEvent(EVENT_FFS_INTERFACE_CFG, EVENT_CLEAR, NULL);
		}
		if (tSignalBit & FFS_FLAG_STATIC_REG)
		{
			if (dFlags & FFS_FLAG_STATIC_REG)
				FSM_ePublishEvent(EVENT_FFS_STATIC_REG, EVENT_SET, (void*)&FFS_sRegEstaticoCRC);
			else
				FSM_ePublishEvent(EVENT_FFS_STATIC_REG, EVENT_CLEAR, NULL);
		}

	}

	osThreadTerminate(NULL);
	osThreadTerminate(NULL);
}
#else
void FSM_vFileSysPublishThread(void const *argument)
{}
#endif

/******************************************************************************
 * Function : FSM_vInitDeviceLayer()
 *//**
 * \b Description:
 *
 * This function will init the CBT09427 instance used by the Buzzer module.
 *
 * PRE-CONDITION: Buzzer core initialized
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 ~~~~~~~~~~~~~~~
 *
 * @see
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
eAPPError_s FSM_vInitDeviceLayer (void)
{
	uint8_t ucStatus;
	eAPPError_s error = APP_ERROR_SUCCESS;
	uint8_t retries = 3;

	do
	{
		ucStatus = f_initvolume(initfunc_span);
		if (ucStatus == F_ERR_NOTFORMATTED)
		{
			f_format(F_FAT16_MEDIA);
		}

	} while ((ucStatus != F_NO_ERROR) && (retries-- > 0));

	if (ucStatus == F_NO_ERROR)
	{
		eError = APP_ERROR_SUCCESS;
		osFlagSet(FFS_sFlagSis, FFS_FLAG_STATUS);
	}
	else
	{
		osFlagClear(FFS_sFlagSis, FFS_FLAG_STATUS);
	}

	return eError;
}

void FFS_vIdentifyEvent (contract_s* contract)
{
	event_e ePubEvt = GET_PUBLISHED_EVENT(contract);
	eEventType ePubEvType = GET_PUBLISHED_TYPE(contract);;

	switch (contract->eOrigin)
	{
		case MODULE_CONTROL:
		{
			if (ePubEvt == EVENT_CTL_UPDATE_CONFIG)
			{
				if (GET_PUBLISHED_TYPE(contract) == EVENT_SET)
				{
				memcpy(&FFS_sConfiguracao, (UOS_tsConfiguracao*)(GET_PUBLISHED_PAYLOAD(contract)),
					sizeof(UOS_tsConfiguracao));

					eAPPError_s error = FFS_vSaveConfigFile();
					ASSERT(error == APP_ERROR_SUCCESS);
				}

			}
			if (GET_PUBLISHED_EVENT(contract) == EVENT_FFS_INTERFACE_CFG)
			{
				memcpy(&FFS_sConfig, (IHM_tsConfig*)(GET_PUBLISHED_PAYLOAD(contract)), sizeof(IHM_tsConfig));
				if (GET_PUBLISHED_TYPE(contract) == EVENT_SET)
				{
					eAPPError_s error = FFS_vSaveInterfaceCfgFile();
					ASSERT(error == APP_ERROR_SUCCESS);
				}
			}
			break;
		}
		case MODULE_ACQUIREG:
		{
			if (ePubEvt == EVENT_FFS_STATIC_REG)
			{
				memcpy(&FFS_sRegEstaticoCRC, (AQR_tsRegEstaticoCRC*)(GET_PUBLISHED_PAYLOAD(contract)),
							sizeof(AQR_tsRegEstaticoCRC));
				if (GET_PUBLISHED_TYPE(contract) == EVENT_SET)
				{
					eAPPError_s error = FFS_vSaveStaticReg();
					ASSERT(error == APP_ERROR_SUCCESS);
				}
			}
			break;
		}

		default:
			break;
	}
}
/* ************************* Main thread ************************************ */

extern UOS_tsConfiguracao UOS_sConfiguracaoDefault;

#ifndef UNITY_TEST
void FSM_vFileSysThread (void const *argument)
{
	osStatus status;
	eAPPError_s error;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("FileSys Thread Created");
#endif

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(FileSysQueue);

	SIGNATURE_HEADER(FileSysControl, THIS_MODULE, TOPIC_CONTROL, FileSysQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(FileSysControl), 0) == osOK);

	osFlagGroupCreate(&FFS_sFlagSis);
	error = FSM_vInitDeviceLayer();
	ASSERT(error == APP_ERROR_SUCCESS);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		FSM_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromID = (osThreadId)argument;
	osSignalSet(xMainFromID, MODULE_FILESYS);

	osSignalSet(xPbulishThreadID, FFS_FLAG_STATUS);

	error = FFS_vLoadConfigFile();
	ASSERT(error == APP_ERROR_SUCCESS);
	osSignalSet(xPbulishThreadID, FFS_FLAG_CFG);

	error = FFS_vLoadStaticReg();
	ASSERT(error == APP_ERROR_SUCCESS);
	osSignalSet(xPbulishThreadID, FFS_FLAG_STATIC_REG);

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on any filesys topic or diagnostic topic*/
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		osEvent evt = RECEIVE(FileSysQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

		if (evt.status == osEventMessage)
		{
			FFS_vIdentifyEvent(GET_CONTRACT(evt));
		}
	}
	/* Unreachable */
	osThreadSuspend(NULL);
}
#else
void FSM_vFileSysThread (void const *argument)
{}
#endif

/* ************************* Management thread ************************************ */
void FSM_vFileSysManagementThread (void const *argument)
{
	eAPPError_s error;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("FileSys Thread Created");
#endif

	FSM_vDetectThread(&WATCHDOG(FSMPUB), &bFSMPUBThreadArrayPosition, (void*)FSM_vFileSysManagementThread);
	WATCHDOG_STATE(FSMPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bFSMPUBThreadArrayPosition)); //Task created, inform core
	osThreadSetPriority(NULL, osPriorityLow);

	while (1)
	{

	}
}
