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

CREATE_MUTEX(GUI_UpdateMask);

eIsobusMask eCurrMask = DATA_MASK_INSTALLATION;

tsAcumulados GUI_sAcumulado;
tsStatus GUI_sStatus;

extern osFlagsGroupId UOS_sFlagSis;

extern uint16_t AQR_wEspacamento;

eInstallationStatus eSensorStatus[GUI_NUM_SENSOR];

sTestModeDataMaskData sGUITestModeData;
sPlanterDataMaskData  sGUIPlanterData;

sConfigurationData GUIConfigurationData;
UOS_tsConfiguracao sSISConfiguration;

GUI_tsConfig GUI_sConfig;

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
void GUI_vSetGuiTestData (event_e eEvt, void* vPayload);
void GUI_vUpdateWorkedArea (void);
void GUI_vLinesPartialPopulation (uint32_t dNumSensor, int32_t* dsAverage, uint32_t* dSeedsPerUnit, uint32_t* dSeedsPerHa, uint32_t* dTotalSeeds);
void GUI_vGetProductivity (uint32_t* pdProductivity, uint32_t* pdSeconds);
void GUI_vUptTestMode(void);
void GUI_vUptPlanter(void);

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

void GUI_vUptTestMode(void)
{
	event_e ePubEvt = EVENT_GUI_UPDATE_TEST_MODE_INTERFACE;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
}

void GUI_vRandomValuesToPlanterDataMask (void)
{
	uint8_t i, j;

	for ( i = 0, j = 1; i < 36, j < 36; i = i + 2, j = j + 2) {
		if (sGUIPlanterData.asLineStatus[i].dsLineAverage > 100)
		{
			sGUIPlanterData.asLineStatus[i].dsLineAverage = -100;
		}
		else
		{
			sGUIPlanterData.asLineStatus[i].dsLineAverage++;
			sGUIPlanterData.asLineStatus[i].dLineSemPerUnit++;
			sGUIPlanterData.asLineStatus[i].dLineSemPerHa++;
			sGUIPlanterData.asLineStatus[i].dLineTotalSeeds++;
		}

		if (sGUIPlanterData.asLineStatus[j].dsLineAverage < -100)
		{
			sGUIPlanterData.asLineStatus[j].dsLineAverage = 100;
		}
		else
		{
			sGUIPlanterData.asLineStatus[j].dsLineAverage--;
			sGUIPlanterData.asLineStatus[j].dLineSemPerUnit++;
			sGUIPlanterData.asLineStatus[j].dLineSemPerHa++;
			sGUIPlanterData.asLineStatus[j].dLineTotalSeeds++;
		}
	}

	sGUIPlanterData.dProductivity++;
	sGUIPlanterData.dWorkedTime++;
	sGUIPlanterData.dTotalSeeds++;
	sGUIPlanterData.dPartPopSemPerUnit++;
	sGUIPlanterData.dPartPopSemPerHa++;
	sGUIPlanterData.dWorkedAreaMt++;
	sGUIPlanterData.dWorkedAreaHa++;
	sGUIPlanterData.dTotalMt++;
	sGUIPlanterData.dTotalHa++;
}

#define ALL_LINES 0

void GUI_vGetValuesToPlanterDataMask (void)
{
	for (uint8_t dNumSensor = 0; dNumSensor < sSISConfiguration.sMonitor.bNumLinhas; dNumSensor++)
	{
		GUI_vLinesPartialPopulation((dNumSensor + 1), &sGUIPlanterData.asLineStatus[dNumSensor].dsLineAverage,
			&sGUIPlanterData.asLineStatus[dNumSensor].dLineSemPerUnit,
			&sGUIPlanterData.asLineStatus[dNumSensor].dLineSemPerHa,
			&sGUIPlanterData.asLineStatus[dNumSensor].dLineTotalSeeds);
	}

	GUI_vGetProductivity(&sGUIPlanterData.dProductivity, &sGUIPlanterData.dWorkedTime);

	GUI_vLinesPartialPopulation(ALL_LINES, NULL, &sGUIPlanterData.dPartPopSemPerUnit, &sGUIPlanterData.dPartPopSemPerHa,
		&sGUIPlanterData.dTotalSeeds);
	GUI_vUpdateWorkedArea();
}

//#define RANDOM_VALUES

void GUI_vUptPlanter (void)
{
	event_e ePubEvt = EVENT_GUI_UPDATE_PLANTER_INTERFACE;
#if defined (RANDOM_VALUES)
	GUI_vRandomValuesToPlanterDataMask();
#else
	GUI_vGetValuesToPlanterDataMask();
#endif
	PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
}

void GUI_vGuiPublishThread (void const *argument)
{
	osStatus status;
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

	WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	osDelay(200);
	WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

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
					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_UPDATE;
					sGUIPubMessage.vPayload = (void*)&eSensorStatus;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);

					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = RELEASE_MUTEX(GUI_UpdateMask);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
					break;
				}
				case EVENT_GUI_UPDATE_PLANTER_INTERFACE:
				{
					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_UPDATE;
					sGUIPubMessage.vPayload = (void*)&sGUIPlanterData;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);

					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = RELEASE_MUTEX(GUI_UpdateMask);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
					break;
				}
				case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
				{
					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

					GUI_vSetGuiTestData(EVENT_GUI_UPDATE_TEST_MODE_INTERFACE, (void*)&GUI_sAcumulado);
					if ((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) > 0)
					{
						sGUIPubMessage.dEvent = ePubEvt;
						sGUIPubMessage.eEvtType = EVENT_UPDATE;
						sGUIPubMessage.vPayload = (void*)&sGUITestModeData;
						MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
						PUBLISH(CONTRACT(Gui), 0);
					}

					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = RELEASE_MUTEX(GUI_UpdateMask);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
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
				case EVENT_GUI_INSTALLATION_ERASE_INSTALLATION:
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
					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = (void*)&sGUITestModeData;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);

					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = RELEASE_MUTEX(GUI_UpdateMask);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
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
					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = (void*)&GUIConfigurationData;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);

					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = RELEASE_MUTEX(GUI_UpdateMask);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
					break;
				}
				case EVENT_GUI_UPDATE_SYS_CONFIG:
				{
					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = (void*)&sSISConfiguration;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);

					WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
					status = RELEASE_MUTEX(GUI_UpdateMask);
					ASSERT(status == osOK);
					WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
					break;
				}
				case EVENT_GUI_PLANTER_CLEAR_COUNTER_TOTAL:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = NULL;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_PLANTER_CLEAR_COUNTER_SUBTOTAL:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = NULL;
					MESSAGE_PAYLOAD(Gui) = (void*)&sGUIPubMessage;
					PUBLISH(CONTRACT(Gui), 0);
					break;
				}
				case EVENT_GUI_CHANGE_ACTIVE_MASK_CONFIG_MASK:
				{
					sGUIPubMessage.dEvent = ePubEvt;
					sGUIPubMessage.eEvtType = EVENT_SET;
					sGUIPubMessage.vPayload = NULL;
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

void GUI_vSetGuiTestData(event_e eEvt, void* vPayload)
{
	switch (eEvt) {
		case EVENT_AQR_INSTALLATION_CONFIRM_INSTALLATION:
		{
			tsStatus sStatus = *((tsStatus*)vPayload);
			sGUITestModeData.dInstalledSensors = sStatus.bSementeInstalados + sStatus.bAduboInstalados + sStatus.bAdicionalInstalados + sStatus.bSensorNaoEsperado;
			sGUITestModeData.dConfiguredSensors = sStatus.bNumSensores;
			break;
		}
		case EVENT_GUI_UPDATE_TEST_MODE_INTERFACE:
		{
			sGUITestModeData.sAccumulated = *((tsAcumulados*)vPayload);
			break;
		}
		default:
			break;
	}
}

void GUI_SetGuiConfiguration(void)
{
	event_e ePublish;

	GUIConfigurationData.bNumOfRows = sSISConfiguration.sMonitor.bNumLinhas;
	GUIConfigurationData.bTolerance = sSISConfiguration.sMonitor.bTolerancia;
	GUIConfigurationData.dVehicleID = sSISConfiguration.dVeiculo;
	GUIConfigurationData.eAltType = (eAlternatedRowsType)sSISConfiguration.sMonitor.eIntercala;
	GUIConfigurationData.eAlterRows =
			(sSISConfiguration.sMonitor.eIntercala == Sem_Intercalacao) ? ALTERNATE_ROWS_DISABLED : ALTERNATE_ROWS_ENABLED;

	GUIConfigurationData.eMonitorArea = (eAreaMonitor)sSISConfiguration.sMonitor.bMonitorArea;
	GUIConfigurationData.eLanguage = sSISConfiguration.sIHM.eLanguage;
	GUIConfigurationData.eUnit = sSISConfiguration.sIHM.eUnit;

	if (GUIConfigurationData.eUnit == UNIT_INTERNATIONAL_SYSTEM)
	{
		GUIConfigurationData.wEvaluationDistance = sSISConfiguration.sMonitor.wAvalia;
		GUIConfigurationData.fMaxSpeed = sSISConfiguration.sMonitor.fLimVel;
		GUIConfigurationData.wSeedRate = sSISConfiguration.sMonitor.wSementesPorMetro;
		GUIConfigurationData.wImplementWidth = sSISConfiguration.sMonitor.wLargImpl;
		GUIConfigurationData.wDistBetweenLines = sSISConfiguration.sMonitor.wDistLinhas;
	}
	else
	{
		GUIConfigurationData.wEvaluationDistance = DM2IN(sSISConfiguration.sMonitor.wAvalia);
		GUIConfigurationData.fMaxSpeed = KMH2MLH(sSISConfiguration.sMonitor.fLimVel);
		GUIConfigurationData.wSeedRate = SDM2SP(sSISConfiguration.sMonitor.wSementesPorMetro);
		GUIConfigurationData.wImplementWidth = MM2IN(sSISConfiguration.sMonitor.wLargImpl);
		GUIConfigurationData.wDistBetweenLines = MM2IN(sSISConfiguration.sMonitor.wDistLinhas);
	}
	ePublish = EVENT_GUI_UPDATE_CONFIG;;
	PUT_LOCAL_QUEUE(GuiPublishQ, ePublish, osWaitForever);
}

void GUI_SetSisConfiguration(void)
{
	event_e ePublish;

	sSISConfiguration.sMonitor.bNumLinhas = GUIConfigurationData.bNumOfRows;
	sSISConfiguration.sMonitor.bTolerancia = GUIConfigurationData.bTolerance;
	sSISConfiguration.dVeiculo = GUIConfigurationData.dVehicleID;

	GUIConfigurationData.eAltType = (eAlternatedRowsType)sSISConfiguration.sMonitor.eIntercala;
	if (GUIConfigurationData.eAlterRows == ALTERNATE_ROWS_DISABLED)
	{
		sSISConfiguration.sMonitor.eIntercala = Sem_Intercalacao;
	}
	else
	{
		sSISConfiguration.sMonitor.eIntercala = GUIConfigurationData.eAltType;
	}

	sSISConfiguration.sMonitor.bMonitorArea = GUIConfigurationData.eMonitorArea;
	sSISConfiguration.sIHM.eLanguage = GUIConfigurationData.eLanguage;
	sSISConfiguration.sIHM.eUnit = GUIConfigurationData.eUnit;

	if (GUIConfigurationData.eUnit == UNIT_INTERNATIONAL_SYSTEM)
	{
		sSISConfiguration.sMonitor.wAvalia = GUIConfigurationData.wEvaluationDistance;
		sSISConfiguration.sMonitor.fLimVel = GUIConfigurationData.fMaxSpeed;
		sSISConfiguration.sMonitor.wSementesPorMetro = GUIConfigurationData.wSeedRate;
		sSISConfiguration.sMonitor.wLargImpl = GUIConfigurationData.wImplementWidth;
		sSISConfiguration.sMonitor.wDistLinhas = GUIConfigurationData.wDistBetweenLines;
	}
	else
	{
		sSISConfiguration.sMonitor.wAvalia = IN2DM(GUIConfigurationData.wEvaluationDistance);
		sSISConfiguration.sMonitor.fLimVel = MLH2KMH(GUIConfigurationData.fMaxSpeed);
		sSISConfiguration.sMonitor.wSementesPorMetro = SP2SDM(GUIConfigurationData.wSeedRate);
		sSISConfiguration.sMonitor.wLargImpl = IN2MM(GUIConfigurationData.wImplementWidth);
		sSISConfiguration.sMonitor.wDistLinhas = IN2MM(GUIConfigurationData.wDistBetweenLines);
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
	osStatus status;
	event_e ePubEvt = GET_PUBLISHED_EVENT(contract);
	void * pvPayload = GET_PUBLISHED_PAYLOAD(contract);

	switch (contract->eOrigin)
	{
		case MODULE_ISOBUS:
		{
			if (ePubEvt == EVENT_ISO_UPDATE_CURRENT_DATA_MASK)
			{
				eCurrMask = *((eIsobusMask*)pvPayload);

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				if (eCurrMask == DATA_MASK_TEST_MODE)
				{
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_MODO_TESTE);
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST);
				}
				else if (eCurrMask == DATA_MASK_PLANTER)
				{
					osFlagSet(UOS_sFlagSis, (UOS_SIS_FLAG_MODO_TRABALHO | UOS_SIS_FLAG_MODO_TESTE));
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST);
				}
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_ISO_UPDATE_CURRENT_CONFIGURATION)
			{
				sConfigurationData *psConfig = pvPayload;

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
				if (psConfig != NULL)
				{
					memcpy(&GUIConfigurationData, psConfig, sizeof(sConfigurationData));
					GUI_SetSisConfiguration();
					GUI_InitSensorStatus();
				}
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_ISO_CONFIG_CANCEL_UPDATE_DATA)
			{
				ePubEvt = EVENT_GUI_UPDATE_CONFIG;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
			}

			if (ePubEvt == EVENT_ISO_INSTALLATION_REPEAT_TEST)
			{
				ePubEvt = EVENT_GUI_INSTALLATION_REPEAT_TEST;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				GUI_InitSensorStatus();

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_ISO_INSTALLATION_ERASE_INSTALLATION)
			{
				ePubEvt = EVENT_GUI_INSTALLATION_ERASE_INSTALLATION;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				GUI_InitSensorStatus();

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_ISO_INSTALLATION_CONFIRM_INSTALLATION)
			{
				ePubEvt = EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION_ACK;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
			}

			if (ePubEvt == EVENT_ISO_PLANTER_CLEAR_COUNTER_TOTAL)
			{
				ePubEvt = EVENT_GUI_PLANTER_CLEAR_COUNTER_TOTAL;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
			}

			if (ePubEvt == EVENT_ISO_PLANTER_CLEAR_COUNTER_SUBTOTAL)
			{
				ePubEvt = EVENT_GUI_PLANTER_CLEAR_COUNTER_SUBTOTAL;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
			}
			break;
		}
		case MODULE_CONTROL:
		{
			if (ePubEvt == EVENT_CTL_UPDATE_CONFIG)
			{
				UOS_tsConfiguracao *psConfig = pvPayload;

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				if (psConfig != NULL)
				{
					if (memcmp(&sSISConfiguration, psConfig, sizeof(UOS_tsConfiguracao)) != 0)
					{
						memcpy(&sSISConfiguration, psConfig, sizeof(UOS_tsConfiguracao));
						GUI_SetGuiConfiguration();
						GUI_InitSensorStatus();
						if (GET_PUBLISHED_TYPE(contract) == EVENT_CLEAR)
						{
							ePubEvt = EVENT_GUI_CHANGE_ACTIVE_MASK_CONFIG_MASK;
							PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
						}
					}
				}

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}
			break;
		}
		case MODULE_ACQUIREG:
		{
			if (ePubEvt == EVENT_AQR_INSTALLATION_UPDATE_INSTALLATION)
			{
				CAN_tsLista *pPubData = pvPayload;
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				GUI_UpdateSensorStatus(pPubData);

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_AQR_INSTALLATION_CONFIRM_INSTALLATION)
			{
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				// Update test mode data mask number of sensors installed, number of sensors configured
				GUI_vSetGuiTestData(ePubEvt, pvPayload);
				ePubEvt = EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION;
				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_AQR_UPDATE_PLANT_DATA)
			{
				tsPubPlantData *psPlantData = pvPayload;

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = WAIT_MUTEX(GUI_UpdateMask, osWaitForever);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

				if (psPlantData != NULL)
				{
					if (psPlantData->AQR_sAcumulado != NULL)
					{
						memcpy(&GUI_sAcumulado, psPlantData->AQR_sAcumulado, sizeof(GUI_sAcumulado));
					}
					if(psPlantData->AQR_sStatus != NULL)
					{
						memcpy(&GUI_sStatus, psPlantData->AQR_sStatus, sizeof(GUI_sStatus));
					}
					if (eCurrMask == DATA_MASK_PLANTER)
					{
						GUI_vUptPlanter();
					}
					else if (eCurrMask == DATA_MASK_TEST_MODE)
					{
						GUI_vUptTestMode();
					}
				}

				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				status = RELEASE_MUTEX(GUI_UpdateMask);
				ASSERT(status == osOK);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

//			if (ePubEvt == EVENT_AQR_INSTALLATION_CONFIRM_INSTALLATION)
//			{
//				// Update test mode data mask number of sensors installed, number of sensors configured
//				GUI_vSetGuiTestData(ePubEvt, GET_PUBLISHED_PAYLOAD(contract));
//				ePubEvt = EVENT_GUI_UPDATE_TEST_MODE_INTERFACE;
//				PUT_LOCAL_QUEUE(GuiPublishQ, ePubEvt, osWaitForever);
//			}
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

	INITIALIZE_MUTEX(GUI_UpdateMask);

	GUI_eInitGuiPublisher();

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromIsobusID = (osThreadId)argument;
	osSignalSet(xMainFromIsobusID, MODULE_GUI);

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP, false, false, osWaitForever);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		GUI_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	GUI_eInitGuiSubs();

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

double GUI_fConvertUnit (double gValue, uint32_t dFlags)
{
	static const double agTabToMeters[] =
		{
			0.001,          // mm = 1/1000 m
			0.01,           // cm = 1/100 m
			1.0,            // 1 m
			1000.0,         // km = 1000 m
			10000.0,        // ha = 10000 m�
			4046.93,        // acre = 4046.93 m�
			1609.30,        // mi = 1,60930 km = 1609,30 m
			0.3048,         // p�s = 30,48 cm = 0,3048 m
			0.0254,         // pol = 2,54 cm = 0,0254 m
		};

	const uint32_t dInput = (dFlags >> 16) - 1;
	const uint32_t dOutput = (dFlags & 0xffff) - 1;

	if ((dInput >= GUI_dUNITS_QUANT) ||
		(dOutput >= GUI_dUNITS_QUANT))
	{
		return 0.0;
	}
	if (dInput == dOutput)
	{
		return gValue;
	}

	// Converte medida de entrada em metros:
	gValue *= agTabToMeters[dInput];

	// Converte de metros para medida de sa�da:
	gValue /= agTabToMeters[dOutput];

	return gValue;
}

void GUI_vUpdateWorkedArea (void)
{
	float fAreaTrab;

	tsLinhas* const psAcum = &GUI_sAcumulado.sTrabTotal;
	tsLinhas* const psAcumDir = &GUI_sAcumulado.sTrabTotalDir;
	tsLinhas* const psAcumEsq = &GUI_sAcumulado.sTrabTotalEsq;

	tsLinhas* const psParcial = &GUI_sAcumulado.sTrabParcial;
	tsLinhas* const psParcDir = &GUI_sAcumulado.sTrabParcDir;
	tsLinhas* const psParcEsq = &GUI_sAcumulado.sTrabParcEsq;

	tsDistanciaTrab* const psAcumDis = &GUI_sAcumulado.sDistTrabTotal;
	tsDistanciaTrab* const psAcumDisDir = &GUI_sAcumulado.sDistTrabTotalDir;
	tsDistanciaTrab* const psAcumDisEsq = &GUI_sAcumulado.sDistTrabTotalEsq;

	tsDistanciaTrab* const psParcDis = &GUI_sAcumulado.sDistTrabParcial;
	tsDistanciaTrab* const psParcDisDir = &GUI_sAcumulado.sDistTrabParcialDir;
	tsDistanciaTrab* const psParcDisEsq = &GUI_sAcumulado.sDistTrabParcialEsq;

	tsStatus *psStatus = &GUI_sStatus;
	UOS_tsCfgMonitor *psMonitor = &sSISConfiguration.sMonitor;

	//Divide por 10 porque AQR_wEspacamento esta em cm*10
	float fAreaTrabalhada = ((float)AQR_wEspacamento * (float)psParcDis->dDistancia) * 0.1f;
	float fAreaTrabalhadaDir = ((float)AQR_wEspacamento * (float)psParcDisDir->dDistancia) * 0.1f;
	float fAreaTrabalhadaEsq = ((float)AQR_wEspacamento * (float)psParcDisEsq->dDistancia) * 0.1f;

	//Converte de cm� para m�
	fAreaTrabalhada *= (1.0f / 10000.0f);
	fAreaTrabalhadaDir *= (1.0f / 10000.0f);
	fAreaTrabalhadaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			fAreaTrabalhada *= psStatus->bNumLinhasSemIntercalar;
		}
		else
		{
			fAreaTrabalhada *= psMonitor->bNumLinhas;
		}

		fAreaTrabalhadaDir *= psStatus->bNumLinhasDir;
		fAreaTrabalhadaEsq *= psStatus->bNumLinhasEsq;
	}

	//Soma area calculada acima e a acumulada
	fAreaTrabalhada +=
		(fAreaTrabalhadaDir + fAreaTrabalhadaEsq + psParcial->fArea + psParcDir->fArea + psParcEsq->fArea);

	// Converte de centimetros para ha/acre:
	fAreaTrab = (float)GUI_fConvertUnit(fAreaTrabalhada,
		GUI_dCONV(GUI_dMETERS,
			GUI_dHECTARES));

	// Partial population Ha
	sGUIPlanterData.dWorkedAreaHa = fAreaTrab * 10;

	// Converte de cent�metros para km/mi:
	fAreaTrab = (float)GUI_fConvertUnit(
		(psParcial->dDistancia +
			psParcDir->dDistancia +
			psParcEsq->dDistancia),
		GUI_dCONV( GUI_dCENTIMETERS, GUI_dMETERS));

	sGUIPlanterData.dWorkedAreaMt = fAreaTrab * 10;

	// Totals
	//--------------------------------------------------------------------------
	// Quantidade acumulada, em km/mi:

	//Divide por 10 porque AQR_wEspacamento est� em cm*10
	fAreaTrabalhada = ((float)AQR_wEspacamento * (float)psAcumDis->dDistancia) * 0.1f;
	fAreaTrabalhadaDir = ((float)AQR_wEspacamento * (float)psAcumDisDir->dDistancia) * 0.1f;
	fAreaTrabalhadaEsq = ((float)AQR_wEspacamento * (float)psAcumDisEsq->dDistancia) * 0.1f;

	//Converte de cm� para m�
	fAreaTrabalhada *= (1.0f / 10000.0f);
	fAreaTrabalhadaDir *= (1.0f / 10000.0f);
	fAreaTrabalhadaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			fAreaTrabalhada *= psStatus->bNumLinhasSemIntercalar;
		}
		else
		{
			fAreaTrabalhada *= psMonitor->bNumLinhas;
		}

		fAreaTrabalhadaDir *= psStatus->bNumLinhasDir;
		fAreaTrabalhadaEsq *= psStatus->bNumLinhasEsq;
	}
	//Soma area calaculada acima e a acumulada
	fAreaTrabalhada += (fAreaTrabalhadaDir + fAreaTrabalhadaEsq + psAcum->fArea + psAcumDir->fArea + psAcumEsq->fArea);

	// Converte de cent�metros para ha/acre:
	fAreaTrab = (float)GUI_fConvertUnit(fAreaTrabalhada,
		GUI_dCONV(GUI_dMETERS,
			GUI_dHECTARES));

	sGUIPlanterData.dTotalHa = fAreaTrab * 10;

	// Converte de cent�metros para km/mi:
	fAreaTrab = (float)GUI_fConvertUnit(
		(psAcum->dDistancia + psAcumDir->dDistancia + psAcumEsq->dDistancia),
		GUI_dCONV(GUI_dCENTIMETERS,
			GUI_dMETERS));

	sGUIPlanterData.dTotalMt = fAreaTrab * 10;
}

void GUI_vLinesPartialPopulation (uint32_t dNumSensor, int32_t* dsAverage, uint32_t* dSeedsPerUnit, uint32_t* dSeedsPerHa, uint32_t* dTotalSeeds)
{
	uint32_t dSem = 0;
	uint8_t bLinhaLevantada;
	UOS_tsCfgMonitor *psMonitor = &sSISConfiguration.sMonitor;
	tsLinhas* const psParcial = &GUI_sAcumulado.sTrabParcial;
	tsLinhas* const psParcDir = &GUI_sAcumulado.sTrabParcDir;
	tsLinhas* const psParcEsq = &GUI_sAcumulado.sTrabParcEsq;
	//Apontador para estrutura de distancia
	tsDistanciaTrab* const psDistParc = &GUI_sAcumulado.sDistTrabParcial;
	tsDistanciaTrab* const psDistParcDir = &GUI_sAcumulado.sDistTrabParcialDir;
	tsDistanciaTrab* const psDistParcEsq = &GUI_sAcumulado.sDistTrabParcialEsq;

	tsStatus *psStatus = &GUI_sStatus;

	uint32_t dFlagSis = osFlagGet(UOS_sFlagSis);

	if (((dsAverage == NULL) && (dNumSensor != 0)) || (dSeedsPerUnit == NULL) || (dSeedsPerHa == NULL)
		|| (dTotalSeeds == NULL) || (dNumSensor > 36))
		return;

	//verificacao das linhas levantadas
	if (dNumSensor < 33)
	{
		bLinhaLevantada = ((dNumSensor > 0) && ((psStatus->dLinhasLevantadas & (1 << (dNumSensor - 1))) != 0));
	}
	else
	{
		bLinhaLevantada = ((dNumSensor > 0) && ((psStatus->dLinhasLevantadasExt & (1 << (dNumSensor - 33))) != 0));
	}
	ASSERT(dNumSensor < 37);

	//Painel de popula��o deve exibir:
	//___________________________
	//|Populacao     |  |  |  |  |
	//|(ou Linha XX) |  |  |  |  |
	//|--------------------------|
	//| |--|   <destaque>        |
	//| |  |                     |
	//| |  |+   xx,x   sem/m     |
	//| |##|                     |
	//| |##|- </destaque>        |
	//| |  |                     |
	//| |  |_   xx,x   sem/ha    |
	//| |  |    xx,x   sementes  |
	//| |__|                     |
	//|__________________________|

	//Painel de linha em modo teste deve exibir:
	//___________________________
	//|Linha     /XX\|  |  |  |  |
	//|          \XX/|  |  |  |  |
	//|--------------------------|
	//|<status>  TESTE           |
	//|                          |
	//|       </destaque>        |
	//|                          |
	//|           xxxx  sem      |
	//|                          |
	//|       </destaque>        |
	//|                          |
	//|                          |
	//|__________________________|

	// sensor 0 indica que as informa��es que ser�o exibidas s�o
	// o acumulado de todos os sensores
	float fSem = 0.0f;
	float fAux = 0.0f;
	float fFator1 = 0.0f;
	float fFator2 = 0.0f;

	if (psMonitor->bMonitorArea == false)
	{
		if (dNumSensor == 0)
		{
			// Queremos n�mero de sementes por metros
			// Divide o acumulado da soma das sementes pelo n�mero de
			// linhas e multiplica pela dist�ncia para achar
			// a m�dia de sementes por metro

			// NOTA:
			// A dist�ncia percorrida est� em cent�metros, por isso
			// dividimos pelo n�mero de linhas e multiplicamos o resultado por 100

			if (psMonitor->eIntercala != Sem_Intercalacao)
			{
				fSem = ((float)psDistParc->dSomaSem / psStatus->bNumLinhasSemIntercalar);
			}
			else
			{
				fSem = ((float)psDistParc->dSomaSem / psMonitor->bNumLinhas);
			}

			if (psStatus->bNumLinhasDir > 0)
			{
				fSem += ((float)psDistParcDir->dSomaSem / psStatus->bNumLinhasDir);
			}
			if (psStatus->bNumLinhasEsq > 0)
			{
				fSem += ((float)psDistParcEsq->dSomaSem / psStatus->bNumLinhasEsq);
			}

			if ((psDistParc->dDistancia + psDistParcDir->dDistancia + psDistParcEsq->dDistancia) > 0)
			{
				fSem = (fSem / (psDistParc->dDistancia + psDistParcDir->dDistancia + psDistParcEsq->dDistancia) * 100.0f);
				//Calcula o fator para cada dist�ncia percorrida
				fFator1 = (psDistParc->dDistancia + psDistParcDir->dDistancia + psDistParcEsq->dDistancia);
				fFator1 /= (psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia);
				fSem *= fFator1;
			}
			else
			{
				fSem = 0;
			}
			//Calcula o n�mero de sementes do trecho apenas
			fAux = psParcial->dSomaSem + psParcDir->dSomaSem + psParcEsq->dSomaSem - psDistParc->dSomaSem
				- psDistParcDir->dSomaSem - psDistParcEsq->dSomaSem;

			if ((psParcial->fArea + psParcDir->fArea + psParcEsq->fArea) > 0)
			{ //Calcula o fator para cada dist�ncia percorrida
				fAux /= (psParcial->fArea + psParcDir->fArea + psParcEsq->fArea);
				fAux *= (float)psMonitor->wDistLinhas * 0.001f;
				fFator2 = (psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia - psDistParc->dDistancia
					- psDistParcDir->dDistancia - psDistParcEsq->dDistancia);
				fFator2 /= (psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia);
				fAux *= fFator2;
			}
			else
			{
				fAux = 0;
			}
			fSem += fAux;
		}
		else
		{
			// NOTA:
			// Media de sementes instantanea esta em sem/m * 100
			fSem = (float)(GUI_sStatus.awMediaSementes[dNumSensor - 1]);
			fSem /= 100.0f;
		}
	}

	// If imperial mode, convert to seeds per feet:
	if (GUI_sConfig.bSistImperial != false)
	{
		dSem = (uint32_t)(GUI_fConvertUnit(fSem, GUI_dCONV(GUI_dFEETS, GUI_dMETERS)) * 100.0f);
	}
	else
	{
		dSem = (uint32_t)(fSem * 100.0f);
	}

	//--------------------------------------------------------------------------
	// Barra lateral:
	if ((dFlagSis & UOS_SIS_FLAG_MODO_TRABALHO) != 0)
	{
		float fMedia = 0.0f;
		uint32_t dMetaPop = psMonitor->wSementesPorMetro;

		//Arredonda
		fSem *= 100.0f;
		fSem = (((uint32_t)fSem + 5) / 10) * 10;

		if (dMetaPop > 0)
		{
			// multiplica fSem por 10 para compensar dMetaPop ser sem/m * 10
			fMedia = fSem * 10.0f;
			fMedia = (fMedia / dMetaPop) - 100.0f;
			if (psMonitor->bTolerancia > 0)
			{
				fMedia = (fMedia / (float)psMonitor->bTolerancia * 20.0f);
			}
		}

		//Se nao tiver iniciado a avaliacao nao indica falha
		if ((psParcial->dDistancia +
			psParcDir->dDistancia +
			psParcEsq->dDistancia) == 0)
		{
			fMedia = 0;
		}

		if (bLinhaLevantada == 1)
		{
			fMedia = 0;
		}

		if(dsAverage != NULL)
		{
			*dsAverage = (int32_t) fMedia;
		}

		if (dNumSensor > 0)
		{
			const uint32_t dBits = (GUI_sStatus.dMemLinhaDesconectada | GUI_sStatus.dSementeFalhaIHM
				| GUI_sStatus.dAduboFalha | GUI_sStatus.dSementeZeroIHM);
			const uint32_t dBitsExt = (GUI_sStatus.dMemLinhaDesconectadaExt | GUI_sStatus.dSementeFalhaIHMExt
				| GUI_sStatus.dAduboFalhaExt | GUI_sStatus.dSementeZeroIHMExt);

			if (((dBits & (1 << (dNumSensor - 1))) && (dNumSensor < 33)) ||
				((dBitsExt & (1 << (dNumSensor - 33))) && (dNumSensor >= 33)))
			{
//        LCD_vRetanguloPreenchido( pabBuffer, 1, 32, 1 + 17, 32 + 86, DISPLAY_wXOR );
			}
		}
	}

  //--------------------------------------------------------------------------
  // Seeds per meter/feets:
  if( ( dFlagSis & UOS_SIS_FLAG_MODO_TRABALHO ) != 0 )
  {
    if( bLinhaLevantada == false )
    {
      // Arredonda unidade:
      dSem += 5;
      dSem /= 10;

      *dSeedsPerUnit = dSem;
    }
  }

  //--------------------------------------------------------------------------
  // Seeds per ha/acre:
  if( ( dFlagSis & UOS_SIS_FLAG_MODO_TRABALHO ) != 0 )
  {
    if( bLinhaLevantada == false )
    {
      // Quant Sementes
      float   fSem = 0.0f;
      float   fCm2 = 0.0f; //Alterado de uint32_t para float por solicita��o do Alexandre,
      //para n�o limitar a dist�ncia m�xima trabalhada
      //que considerando uma dist�ncia entre linhas de 1 metro
      //a dist�ncia m�xima trabalhada ficaria limitada � aprox. 43km
      float fAux = 0.0f;
      float fFator1 = 0.0f;
      float fFator2 = 0.0f;
      float fSemHa = 0.0f;

      float  fPopulTot = 0.0f;
      float  fPopulDir = 0.0f;
      float  fPopulEsq = 0.0f;
      uint8_t bNumLinhasEsq = 0;
      uint32_t dDistTotal;

      if( dNumSensor == 0 )
      {
        //Divide a soma das sementes pelo n�mero de linhas
        if( psMonitor->eIntercala != Sem_Intercalacao )
        {
          fSem = ( ( float )psDistParc->dSomaSem / psStatus->bNumLinhasSemIntercalar   );
        }
        else
        {
          fSem = ( ( float )psDistParc->dSomaSem / psMonitor->bNumLinhas   );
        }

        if( psStatus->bNumLinhasDir > 0 )
        {
          fSem += ( ( float )psDistParcDir->dSomaSem / psStatus->bNumLinhasDir );
        }
        if( psStatus->bNumLinhasEsq > 0 )
        {
          fSem += ( ( float )psDistParcEsq->dSomaSem / psStatus->bNumLinhasEsq );
        }

        fCm2 = ( ( float )( psDistParc->dDistancia +
                           psDistParcDir->dDistancia +
                           psDistParcEsq->dDistancia   ) * ( float )AQR_wEspacamento );

        if(fCm2 > 0)
        {
          fSemHa = (fSem / fCm2);
        }
        else
        {
          fSemHa = 0;
        }
        //Calcula o fator do trecho trabalhado
        fFator1 = ( psDistParc->dDistancia + psDistParcDir->dDistancia + psDistParcEsq->dDistancia);

        if((psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia) > 0)
        {
          fFator1 /= ( psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia);
        }
        else
        {
          fFator1 = 0;
        }
        fSemHa *= fFator1;

        fSem = psParcial->dSomaSem + psParcDir->dSomaSem + psParcEsq->dSomaSem - psDistParc->dSomaSem - psDistParcDir->dSomaSem - psDistParcEsq->dSomaSem;

        if((psParcial->fArea + psParcDir->fArea + psParcEsq->fArea) > 0)
        {
          fAux  = (psParcial->fArea + psParcDir->fArea + psParcEsq->fArea);
          fAux *= 100000.0f;
          if(fAux > 0)
          {
            fAux = fSem / fAux;
          }
          //Calcula o fator do trecho trabalhado
          fFator2  = (psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia - psDistParc->dDistancia - psDistParcDir->dDistancia  - psDistParcEsq->dDistancia);
          fFator2 /= (psParcial->dDistancia + psParcDir->dDistancia + psParcEsq->dDistancia);

          fAux *= fFator2;
        }
        else
        {
          fAux  = 0;
        }

        fSemHa += fAux;

        //Multiplica por 10 porque AQR_wEspacamento est� em cm*10
        fSem = (fSemHa * 10.0f );

        // mant�m a convers�o para metros por �ltimo para n�o perder precis�o
        // na quantidade de sementes
        // Sem/m�
        fSem *= 10000.0f;
      }
      else
      {
        // ATEN��O:
        // M�dia de sementes est� em sem/m * 100 e dist�ncia entre linhas
        // est� em mil�metros, ent�o:
        // sem/m� = ( sem/m / 100 ) / ( dist_entre_linhas / 1000 )
        // simplificando,
        // sem/m� = ( sem/m  / ( dist_entre_linhas / 10 ) )  ou
        // sem/m� = ( sem/m  /  dist_entre_linhas * 10 )
        //fSem = ( float )( GUI_sStatus.awMediaSementes[ dNumSensor - 1 ] );
        //fSem = ( fSem / AQR_wEspacamento * 10.0f );

         //Se a divis�o da plantadeira est� no lado esquerdo
        if( psMonitor->bDivLinhas == 0 )
        {
            //O n�mero de linhas do lado esq � o maior lado da plantadeira
            bNumLinhasEsq = ( psMonitor->bNumLinhas + 1 ) >> 1;
        }
        else //Se a divis�o da plantadeira est� no lado direito
        {
            //O n�mero de linhas do lado esq � o menor lado da plantadeira
            bNumLinhasEsq = ( psMonitor->bNumLinhas  >> 1 );
        }

        if(dNumSensor > bNumLinhasEsq)
        {
          dDistTotal = psParcial->dDistancia + psParcDir->dDistancia;
        }
        else
        {
          dDistTotal = psParcial->dDistancia + psParcEsq->dDistancia;
        }

        if(dDistTotal > 0)
        {
          if(psParcial->dDistancia > 0)
          {
            fPopulTot  = (( float )psParcial->adSementes[ dNumSensor - 1 ] * 100000.0f) /(( float )psParcial->dDistancia *( float )AQR_wEspacamento);
            fPopulTot *= ((float)psParcial->dDistancia /(float)dDistTotal);
          }
          else
          {
            fPopulTot = 0;
          }
          //Se a linha estiver do lado direito
          if(dNumSensor > bNumLinhasEsq)
          {
            if(psParcDir->dDistancia > 0)
            {
              fPopulDir  = (( float )psParcDir->adSementes[ dNumSensor - 1 ] * 100000.0f) /(( float )psParcDir->dDistancia *( float )AQR_wEspacamento);
              fPopulDir *= ((float)psParcDir->dDistancia / (float)dDistTotal);
            }
            else
            {
              fPopulDir = 0;
            }
          }
          else
          {
            if(psParcEsq->dDistancia > 0)
            {
              fPopulEsq  = (( float )psParcEsq->adSementes[ dNumSensor - 1 ]* 100000.0f) /(( float )psParcEsq->dDistancia *( float )AQR_wEspacamento);
              fPopulEsq *= ((float)psParcEsq->dDistancia / (float)dDistTotal);
            }
            else
            {
              fPopulEsq = 0;
            }

          }
        }
        // Quant Sementes/cm�
        fSem = fPopulTot + fPopulDir + fPopulEsq;

      }
      // Calcula invertendo os parametros de entrada e saida para calcular 1/medida
      fSem = ( float )GUI_fConvertUnit(fSem,
                                          GUI_dCONV(GUI_dHECTARES,
                                                    GUI_dMETERS ) );
      fSem *= 0.001f;
      *dSeedsPerHa = fSem;
    }
  }

  //--------------------------------------------------------------------------
  // Seeds planted:
  if( ( dFlagSis & UOS_SIS_FLAG_MODO_TRABALHO ) != 0 )
  {
    if( bLinhaLevantada == false )
    {
      if( dNumSensor == 0 )
      {
        dSem = psParcial->dSomaSem +
          psParcDir->dSomaSem +
            psParcEsq->dSomaSem;
      }
      else
      {
        dSem = psParcial->adSementes[ dNumSensor - 1 ] +
          psParcDir->adSementes[ dNumSensor - 1 ] +
            psParcEsq->adSementes[ dNumSensor - 1 ];
      }
      *dTotalSeeds = dSem;
    }
  }
  /*else
  {
    if( bLinhaLevantada == false )
    {
      if( dNumSensor == 0 )
      {
        dSem = GUI_sAcumulado.sManobra.dSomaSem; //TESTE BANCADA
      }
      else
      {
        dSem = GUI_sAcumulado.sManobra.adSementes[ dNumSensor - 1 ]; //TESTE BANCADA
      }
      // Test mode value
      *dTotalSeeds = dSem % 10000;
    }
  }*/
}

void GUI_vGetProductivity (uint32_t* pdProductivity, uint32_t* pdSeconds)
{
	UOS_tsCfgMonitor *psMonitor = &sSISConfiguration.sMonitor;
	tsStatus *psStatus = &GUI_sStatus;

	tsLinhas* const psParcial = &GUI_sAcumulado.sTrabParcial;
	tsLinhas* const psParcDir = &GUI_sAcumulado.sTrabParcDir;
	tsLinhas* const psParcEsq = &GUI_sAcumulado.sTrabParcEsq;

	tsDistanciaTrab* const psParcDis = &GUI_sAcumulado.sDistTrabParcial;
	tsDistanciaTrab* const psParcDisDir = &GUI_sAcumulado.sDistTrabParcialDir;
	tsDistanciaTrab* const psParcDisEsq = &GUI_sAcumulado.sDistTrabParcialEsq;

	if ((pdProductivity == NULL) || (pdSeconds == NULL))
		return;

	//Painel de rendimento deve exibir:
	//___________________________
	//|Rendimento    |  |  |  |  |
	//|______________|__|__|__|__|
	//| <destaque>               |
	//|            xxx,x   ha/h  |
	//| </destaque>              |
	//|                          |
	//|         00h00m00s        |
	//|                          |
	//|   dd/mm/aa   hh:mm:ss    |
	//|__________________________|

	//Divide por 10 porque AQR_wEspacamento esta em cm*10
	float fAreaTrabalhada = ((float)AQR_wEspacamento * (float)psParcDis->dDistancia) * 0.1f;
	float fAreaTrabalhadaDir = ((float)AQR_wEspacamento * (float)psParcDisDir->dDistancia) * 0.1f;
	float fAreaTrabalhadaEsq = ((float)AQR_wEspacamento * (float)psParcDisEsq->dDistancia) * 0.1f;

	//Converte de cm� para m�
	fAreaTrabalhada *= (1.0f / 10000.0f);
	fAreaTrabalhadaDir *= (1.0f / 10000.0f);
	fAreaTrabalhadaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			fAreaTrabalhada *= psStatus->bNumLinhasSemIntercalar;
		}
		else
		{
			fAreaTrabalhada *= psMonitor->bNumLinhas;
		}

		fAreaTrabalhadaDir *= psStatus->bNumLinhasDir;
		fAreaTrabalhadaEsq *= psStatus->bNumLinhasEsq;
	}

	fAreaTrabalhada +=
		(fAreaTrabalhadaDir + fAreaTrabalhadaEsq + psParcial->fArea + psParcDir->fArea + psParcEsq->fArea);

	// Converte de m� para ha/acre:
	float fAreaTrab = (float)GUI_fConvertUnit(fAreaTrabalhada, GUI_dCONV(GUI_dMETERS, GUI_dHECTARES));
	{
		if (GUI_sAcumulado.sTrabParcial.dSegundos > 0)
		{
			float fHoras = ((float)GUI_sAcumulado.sTrabParcial.dSegundos / 3600.0f);

			fAreaTrab /= fHoras;
		}
	}

	//Arredonda para exibir
	fAreaTrab += 0.05f;

	*pdProductivity = fAreaTrab;

	//--------------------------------------------------------------------------
	// Tempo trabalhando:

	*pdSeconds = GUI_sAcumulado.sTrabParcial.dSegundos;

}
