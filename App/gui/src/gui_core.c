/****************************************************************************
 * Title                 :   gui_core
 * Filename              :   gui_core.c
 * Author                :   Henrique Reis
 * Origin Date           :   19 de abr de 2017
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
 *  19/04/17   1.0.0     Henrique Reis         gui_core.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "gui_core.h"
#include "debug_tool.h"
#include "../../gui/config/gui_config.h"
#include "gui_ThreadControl.h"
#include <stdlib.h>
#include "common_app.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of CONTROL queue
#define QUEUE_SIZEOFGUI 16

#define THIS_MODULE MODULE_GUI

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
DECLARE_QUEUE(GuiQueue, QUEUE_SIZEOFGUI);      //!< Declaration of Interface Queue
CREATE_SIGNATURE(GuiIsobus);//!< Signature Declarations
CREATE_SIGNATURE(GuiAcquireg);
CREATE_SIGNATURE(GuiControl);

CREATE_CONTRACT(Gui);                              //!< Create contract for sensor msg publication

CREATE_LOCAL_QUEUE(GuiPublishQ, event_e, 16);

eIsobusMask eCurrMask = DATA_MASK_INSTALLATION;

extern osFlagsGroupId UOS_sFlagSis;
extern tsAcumulados AQR_sAcumulado;

eInstallationStatus eSensorStatus[GUI_NUM_SENSOR];
sConfigurationData GUIConfigurationData;
UOS_tsConfiguracao SISConfiguration;

/******************************************************************************
 * Module typedef
 *******************************************************************************/

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
GUI_MODULES
	};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(GUIPUB);//!< WDT pointer flag
uint8_t bGUIPUBThreadArrayPosition = 0;                    //!< Thread position in array

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void GUI_vUpdateInterfaceTimerCallback (void const *argument);

CREATE_TIMER(Gui_UptTimer, GUI_vUpdateInterfaceTimerCallback);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * GUI_WDTData (uint8_t * pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void GUI_vDetectThread (thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
	*bPosition = 0;
	while (THREADS_THREAD(*bPosition)!= (os_pthread)pFunc)
	{
		(*bPosition)++;
	}
	*flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

static void GUI_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}

eAPPError_s GUI_eInitGuiPublisher (void)
{
	//Prepare Default Contract/Message
	MESSAGE_HEADER(Gui, 1, GUI_DEFAULT_MSGSIZE, MT_ARRAYBYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(Gui, 1, THIS_MODULE, TOPIC_GUI);

	return APP_ERROR_SUCCESS;
}

void GUI_vUpdateInterfaceTimerCallback (void const *argument)
{
	event_e ePubEvt;

	ePubEvt = EVENT_GUI_UPDATE_TEST_MODE_INTERFACE;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
}

void GUI_vGuiPublishThread (void const *argument)
{
	osEvent evt;
	osFlags dFlagsSis;
	event_e ePubEvt;
	PubMessage sGUIPubMessage;

	INITIALIZE_LOCAL_QUEUE(GuiPublishQ);           //!< Initialize message queue to publish thread

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Gui Publish Thread Created");
#endif

	GUI_vDetectThread(&WATCHDOG(GUIPUB), &bGUIPUBThreadArrayPosition, (void*)GUI_vGuiPublishThread);
	WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bGUIPUBThreadArrayPosition)); //Task created, inform core

	GUI_eInitGuiPublisher();

	WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

	START_TIMER(Gui_UptTimer, 750);

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
		evt = RECEIVE_LOCAL_QUEUE(GuiPublishQ, &ePubEvt, osWaitForever);
		WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

		if (evt.status == osEventMessage)
		{
			dFlagsSis = osFlagGet(UOS_sFlagSis);

			switch (ePubEvt)
			{
				case EVENT_GUI_UPDATE_INSTALLATION_INTERFACE:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_UPDATE;
					sGUIPubMessage.vPayload = (void*)&eSensorStatus;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_UPDATE_PLANTER_INTERFACE:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_UPDATE;
					sGUIPubMessage.vPayload = NULL;		//TODO: incorrect payload
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
				{
					if ((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) > 0)
					{
						sGUIPubMessage.dEvent = ePubEvt;
						sGUIPubMessage.eEvtType = EVENT_UPDATE;
						sGUIPubMessage.vPayload = (void*)&AQR_sAcumulado;
						MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
						PUBLISH(CONTRACT(Gui), 0);
					}
					break;
				}
				case EVENT_GUI_UPDATE_TRIMMING_INTERFACE:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_UPDATE;
					sGUIPubMessage.vPayload = NULL;		//TODO: incorrect payload
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_UPDATE_SYSTEM_INTERFACE:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_UPDATE;
					sGUIPubMessage.vPayload = NULL;		//TODO: incorrect payload
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_INSTALLATION_REPEAT_TEST:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = NULL;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = NULL;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION_ACK:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_CLEAR;
					sGUIPubMessage.vPayload = NULL;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_UPDATE_CONFIG:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = (void*)&GUIConfigurationData;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_UPDATE_SYS_CONFIG:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = (void*)&SISConfiguration;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				default:
					break;
			}
		}

	}
	osThreadTerminate(NULL);
}

void GUI_SetGuiConfiguration(void)
{
	event_e ePublish;

	GUIConfigurationData.bNumOfRows = SISConfiguration.sMonitor.bNumLinhas;
	GUIConfigurationData.bTolerance = SISConfiguration.sMonitor.bTolerancia;
	GUIConfigurationData.dVehicleID = SISConfiguration.dVeiculo;
	GUIConfigurationData.eAltType = (eAlternatedRowsType)SISConfiguration.sMonitor.eIntercala;
	GUIConfigurationData.eAlterRows =
			(SISConfiguration.sMonitor.eIntercala == Sem_Intercalacao) ? ALTERNATE_ROWS_DISABLED : ALTERNATE_ROWS_ENABLED;

	GUIConfigurationData.eMonitorArea = (eAreaMonitor)SISConfiguration.sMonitor.bMonitorArea;
	GUIConfigurationData.eLanguage = SISConfiguration.sIHM.eLanguage;
	GUIConfigurationData.eUnit = SISConfiguration.sIHM.eUnit;

	if (GUIConfigurationData.eUnit == UNIT_INTERNATIONAL_SYSTEM)
	{
		GUIConfigurationData.wEvaluationDistance = SISConfiguration.sMonitor.wAvalia;
		GUIConfigurationData.fMaxSpeed = SISConfiguration.sMonitor.fLimVel;
		GUIConfigurationData.wSeedRate = SISConfiguration.sMonitor.wSementesPorMetro;
		GUIConfigurationData.wImplementWidth = SISConfiguration.sMonitor.wLargImpl;
		GUIConfigurationData.wDistBetweenLines = SISConfiguration.sMonitor.wDistLinhas;
	}
	else
	{
		GUIConfigurationData.wEvaluationDistance = DM2IN(SISConfiguration.sMonitor.wAvalia);
		GUIConfigurationData.fMaxSpeed = KMH2MLH(SISConfiguration.sMonitor.fLimVel);
		GUIConfigurationData.wSeedRate = SDM2SP(SISConfiguration.sMonitor.wSementesPorMetro);
		GUIConfigurationData.wImplementWidth = MM2IN(SISConfiguration.sMonitor.wLargImpl);
		GUIConfigurationData.wDistBetweenLines = MM2IN(SISConfiguration.sMonitor.wDistLinhas);
	}
	ePublish = EVENT_GUI_UPDATE_CONFIG;;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePublish, osWaitForever);
}

void GUI_SetSisConfiguration(void)
{
	event_e ePublish;

	SISConfiguration.sMonitor.bNumLinhas = GUIConfigurationData.bNumOfRows;
	SISConfiguration.sMonitor.bTolerancia = GUIConfigurationData.bTolerance;
	SISConfiguration.dVeiculo = GUIConfigurationData.dVehicleID;

	GUIConfigurationData.eAltType = (eAlternatedRowsType)SISConfiguration.sMonitor.eIntercala;
	if (GUIConfigurationData.eAlterRows == ALTERNATE_ROWS_DISABLED)
	{
		SISConfiguration.sMonitor.eIntercala = Sem_Intercalacao;
	}
	else
	{
		SISConfiguration.sMonitor.eIntercala = GUIConfigurationData.eAltType;
	}

	SISConfiguration.sMonitor.bMonitorArea = GUIConfigurationData.eMonitorArea;
	SISConfiguration.sIHM.eLanguage = GUIConfigurationData.eLanguage;
	SISConfiguration.sIHM.eUnit = GUIConfigurationData.eUnit;

	if (GUIConfigurationData.eUnit == UNIT_INTERNATIONAL_SYSTEM)
	{
		SISConfiguration.sMonitor.wAvalia = GUIConfigurationData.wEvaluationDistance;
		SISConfiguration.sMonitor.fLimVel = GUIConfigurationData.fMaxSpeed;
		SISConfiguration.sMonitor.wSementesPorMetro = GUIConfigurationData.wSeedRate;
		SISConfiguration.sMonitor.wLargImpl = GUIConfigurationData.wImplementWidth;
		SISConfiguration.sMonitor.wDistLinhas = GUIConfigurationData.wDistBetweenLines;
	}
	else
	{
		SISConfiguration.sMonitor.wAvalia = IN2DM(GUIConfigurationData.wEvaluationDistance);
		SISConfiguration.sMonitor.fLimVel = MLH2KMH(GUIConfigurationData.fMaxSpeed);
		SISConfiguration.sMonitor.wSementesPorMetro = SP2SDM(GUIConfigurationData.wSeedRate);
		SISConfiguration.sMonitor.wLargImpl = IN2MM(GUIConfigurationData.wImplementWidth);
		SISConfiguration.sMonitor.wDistLinhas = IN2MM(GUIConfigurationData.wDistBetweenLines);
	}
	ePublish = EVENT_GUI_UPDATE_SYS_CONFIG;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePublish, osWaitForever);
}

void GUI_InitSensorStatus (void)
{
	event_e ePubEvt;
	uint8_t bConta;

	for (bConta = 0; bConta < GUI_NUM_SENSOR; bConta++)
	{
		if (bConta < GUIConfigurationData.bNumOfRows)
		{
			eSensorStatus[bConta] = STATUS_INSTALL_WAITING;
		}
		else
		{
			eSensorStatus[bConta] = STATUS_INSTALL_NONE;
		}
	}
	ePubEvt = EVENT_GUI_UPDATE_INSTALLATION_INTERFACE;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
}

void GUI_UpdateSensorStatus (CAN_tsLista * pSensorStatus)
{
	event_e ePubEvt;
	uint8_t bConta;
	uint8_t bSensor = 0;

	for (bConta = 0; bConta < GUI_NUM_SENSOR; bConta++)
	{
		if (bSensor++ < GUIConfigurationData.bNumOfRows)
		{
			// se o sensor for par, ele é de semente
			CAN_tsLista *pSensor = &pSensorStatus[bConta * 2];

			switch (pSensor->eEstado)
			{
				case Novo:
				{
					eSensorStatus[bConta] = STATUS_INSTALL_WAITING;
					break;
				}
				case Verificando:
				{
					eSensorStatus[bConta] = STATUS_INSTALL_INSTALLING;
					break;
				}
				case Conectado:
				{
					if ((pSensor->eResultadoAutoTeste == Aprovado) ||
						(pSensor->eResultadoAutoTeste == Nenhum))
					{
						eSensorStatus[bConta] = STATUS_INSTALL_INSTALLED;
					}
					else
					{
						eSensorStatus[bConta] = STATUS_INSTALL_INSTALL_ERROR;
					}
					break;
				}
				case Desconectado:
				{
					eSensorStatus[bConta] = STATUS_INSTALL_INSTALL_ERROR;
					break;
				}
				default:
				{
					eSensorStatus[bConta] = STATUS_INSTALL_NONE;
					break;
				}
			}
		}
		else
		{
			eSensorStatus[bConta] = STATUS_INSTALL_NONE;
		}
	}
	ePubEvt = EVENT_GUI_UPDATE_INSTALLATION_INTERFACE;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);

}

void GUI_vIdentifyEvent (contract_s* contract)
{
	event_e ePubEvt = GET_PUBLISHED_EVENT(contract);

	switch (contract->eOrigin)
	{
		case MODULE_ISOBUS:
		{
			if (ePubEvt == EVENT_ISO_UPDATE_CURRENT_DATA_MASK)
			{
				eCurrMask = *((eIsobusMask*)GET_PUBLISHED_PAYLOAD(contract));

				if (eCurrMask == DATA_MASK_TEST_MODE)
				{
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_MODO_TESTE);
				}
				else if (eCurrMask == DATA_MASK_PLANTER)
				{
					osFlagSet(UOS_sFlagSis, (UOS_SIS_FLAG_MODO_TRABALHO | UOS_SIS_FLAG_MODO_TESTE));

				}
			}

			if (ePubEvt == EVENT_ISO_UPDATE_CURRENT_CONFIGURATION)
			{
				memcpy(&GUIConfigurationData, (sConfigurationData *)GET_PUBLISHED_PAYLOAD(contract), sizeof(sConfigurationData));
				GUI_SetSisConfiguration();
			}

			if (ePubEvt == EVENT_ISO_INSTALLATION_REPEAT_TEST)
			{
				ePubEvt = EVENT_GUI_INSTALLATION_REPEAT_TEST;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
				GUI_InitSensorStatus();
			}

			if (ePubEvt == EVENT_ISO_INSTALLATION_CONFIRM_INSTALLATION)
			{
				ePubEvt = EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION_ACK;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
			}
			break;
		}
		case MODULE_CONTROL:
		{
			if (ePubEvt == EVENT_CTL_UPDATE_CONFIG)
			{
				memcpy(&SISConfiguration, (UOS_tsConfiguracao *)GET_PUBLISHED_PAYLOAD(contract), sizeof(UOS_tsConfiguracao));
				GUI_SetGuiConfiguration();
				GUI_InitSensorStatus();
			}
			break;
		}
		case MODULE_ACQUIREG:
		{
			if (ePubEvt == EVENT_AQR_INSTALLATION_UPDATE_INSTALLATION)
			{
				GUI_UpdateSensorStatus((CAN_tsLista *)GET_PUBLISHED_PAYLOAD(contract));

			}

			if (ePubEvt == EVENT_AQR_INSTALLATION_CONFIRM_INSTALLATION)
			{
				ePubEvt = EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
			}
			break;
		}
		default:
			break;
	}
}

eAPPError_s GUI_eInitGuiSubs (void)
{
	/* Prepare the signature - struture that notify the broker about subscribers */
	SIGNATURE_HEADER(GuiIsobus, THIS_MODULE, TOPIC_ISOBUS, GuiQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(GuiIsobus), 0) == osOK);

	SIGNATURE_HEADER(GuiAcquireg, THIS_MODULE, TOPIC_ACQUIREG, GuiQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(GuiAcquireg), 0) == osOK);

	SIGNATURE_HEADER(GuiControl, THIS_MODULE, TOPIC_CONTROL, GuiQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(GuiControl), 0) == osOK);

	return APP_ERROR_SUCCESS;
}

/* ************************* Main thread ************************************ */
void GUI_vGuiThread (void const *argument)
{
	osStatus status;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Gui Thread Created");
#endif

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(GuiQueue);

	INITIALIZE_TIMER(Gui_UptTimer, osTimerPeriodic);

	GUI_eInitGuiSubs();

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		GUI_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromIsobusID = (osThreadId)argument;
	osSignalSet(xMainFromIsobusID, MODULE_GUI);

	GUI_InitSensorStatus();

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on any topic */
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		osEvent evt = RECEIVE(GuiQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

		if (evt.status == osEventMessage)
		{
			GUI_vIdentifyEvent(GET_CONTRACT(evt));
		}
	}
	/* Unreachable */
	osThreadSuspend(NULL);
}
