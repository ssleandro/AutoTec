/****************************************************************************
 * Title                 :   control_core
 * Filename              :   control_core.c
 * Author                :   Henrique Reis
 * Origin Date           :   18 de abr de 2017
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
 *  18/04/17   1.0.0     Henrique Reis         control_core.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "control_core.h"
#include "debug_tool.h"
#include "../../control/config/control_config.h"
#include "control_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of CONTROL queue
#define QUEUE_SIZEOFCONTROL 16

#define THIS_MODULE MODULE_CONTROL

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
DECLARE_QUEUE(ControlQueue, QUEUE_SIZEOFCONTROL);     //!< Declaration of Interface Queue
CREATE_SIGNATURE(ControlAcquireg);							//!< Signature Declarations
CREATE_SIGNATURE(ControlSensor);								//!< Signature Declarations
CREATE_SIGNATURE(ControlFileSys);							//!< Signature Declarations
CREATE_SIGNATURE(ControlGui);									//!< Signature Declarations
CREATE_CONTRACT(Control);                             //!< Create contract for sensor msg publication

//!< From MPA2500
/******************************************************************************
 Versão deste software com 4 campos de 8 caracteres na seguinte ordem:
 Cliente, Produto, Versão e Observacao.
 *******************************************************************************/
//Código numérico desta versão:
const UOS_tsVersaoCod UOS_sVersaoCodDef = {
	0xFFFF,           //Flag
	6,                //Modelo M2G
	0,                //Versão
	1,                //Revisão
	1006,              //Build
	0x00,             //Número de série do hardware
	0x00,             //(6 bytes)
	0x00,
	0x00,
	0x00,
	0x00,
	UOS_eTARQ_REG_GRAF, //Tipo de arquivo
	0x0000,            //Opcional 1
	0x0000,            //Opcional 2
	0x0000,            //Opcional 3
	0x0000,            //Opcional 4
	0x0000,            //Opcional 5
	0x00000000         //Offset.
	};

//Nome do arquivo de configuracao da interface:
const uint8_t UOS_abNomeConfigIHM[] = "INTERFACE.CFG";

//Estrutura com o código númerico desta versão:
UOS_tsVersaoCod UOS_sVersaoCod;

//Estrutura da configuracao operacional:
UOS_tsConfiguracao UOS_sConfiguracao;

//Flags para indicar o status do sistema:
extern osFlagsGroupId UOS_sFlagSis;
osFlagsGroupId CTL_sFlagSis;

// mutex para a hora
CREATE_MUTEX(UOS_MTX_sDataHora);

uint8_t bMem = 1;

//Variáveis para indicar modo de utilizacao da Serial
uint8_t UOS_bGPSAtivo;
uint8_t UOS_bEstadoUART0;
uint8_t UOS_bSilenciaAlarme;
FFS_sFSInfo UOS_sFSInfo;

/****************************************************************************
 Variáveis locais
 *****************************************************************************/
const UOS_tsConfiguracao UOS_sConfiguracaoDefault =
	{

	//------------------------------------------------------------------------------
//UOS_tsCfgMonitor
		/*FP32   fLimVel;*/20.0f,//Limite de velocidade km/h.
		/*INT16U wSementesPorMetro;*/15, //Meta de Sementes por Metro. (sementes/m)*10
		/*INT16U wInsensibilidade;*/100, //Distância de insensibilidade para falhas. (metros)*10
		/*INT16U wAvalia;*/1000, //Distância para avaliacao de aplicacao. (metros)*10
		/*INT16U wDistLinhas;*/100, //Distância entre linhas. (centímetros)*10
		/*INT16U wLargImpl;*/10, //Largura do implemento. (centímetros)*10
		/*uint8_t  bMonitorArea;*/false, //Se está em modo monitor de área. (0 = false, 1 = true )
		/*uint8_t  bNumLinhas;*/1, //No. Linhas (1-36)
		/*uint8_t  bDivLinhas;*/0, //Divisão da plantadeira (bNumLinhas/2 ou bNumLinhas/2+1)
		/*uint8_t  bSensorAdubo;*/false, //Indica presença de sensor de adubo. (bSensorAdubo = 1)
		/*uint8_t  bTolerancia;*/20, //Tolerância ao espaçamento entre sementes. (porcentagem)
		/*uint8_t  bTeclaPausaHab;*/false, //Habilita o uso da tecla de pausa. (bTeclaPausaHab = 1)
		/*UOS_teIntercala  eIntercala;*/Sem_Intercalacao,
		/*uint8_t  bPausaAuto;*/false, //Habilita o uso da pausa automática.(bPausaAuto = 1)
		/*uint8_t  bLinhasFalhaPausaAuto;*/0, //Número de linhas em falha para pausa automática. (1-32)
		/*uint8_t  bNumSensorAdicionais;*/0, //Número de sensores adicionais (0-6)

//tsCfgGPS
		/*INT32S      lFusoHorario;*/-10800, //Usa fuso horário padrão (Brasília -03:00 )
		/*INT16U      wDistanciaEntreFixos;*/0,
		/*INT16U      wAnguloEntreFixos;*/0,
		/*uint8_t       bHorarioVerao;*/false, //Indica se está em horário de verão (bHorarioVerao = 1)
		/*uint8_t       bSalvaRegistro;*/false, //Indica se gravacao de registros está ativada

		//------------------------------------------------------------------------------
//tsCfgIHM
		/*uint32_t abSenha*/0,
		/*eSelectedLanguage eLanguage;*/LANGUAGE_PORTUGUESE,
		/*eSelectedUnitMeasurement eUnit;*/UNIT_INTERNATIONAL_SYSTEM,

		//------------------------------------------------------------------------------
		/*uint32_t dVeiculo;*/0,

		//------------------------------------------------------------------------------
		/*INT16U wCRC16;*/0
	};

/******************************************************************************
 * Module typedef
 *******************************************************************************/
/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
CONTROL_MODULES
	};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(CONTROLPUB);//!< WDT pointer flag
WATCHDOG_CREATE(CONTROLEMY);//!< WDT pointer flag
uint8_t bCONTROLPUBThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bCONTROLEMYThreadArrayPosition = 0;                    //!< Thread position in array

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * CTL_WDTData (uint8_t * pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void CTL_vDetectThread (thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
	*bPosition = 0;
	while (THREADS_THREAD(*bPosition)!= (os_pthread)pFunc)
	{
		(*bPosition)++;
	}
	*flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

static void CTL_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever);
	}
}

/******************************************************************************
 * Function : CTL_eInitPublisher(void)
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
eAPPError_s CTL_eInitPublisher (void)
{
	//Prepare Default Contract/Message
	MESSAGE_HEADER(Control, CONTROL_DEFAULT_MSGSIZE, 1, MT_ARRAYBYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(Control, 1, THIS_MODULE, TOPIC_CONTROL);

	return APP_ERROR_SUCCESS;
}

void CTL_vControlPublishThread (void const *argument)
{
	osEvent evt;
	osFlags flags;
	osFlags UOSflags;
	uint32_t* dPayload;
	eEventType eEvType;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Control Publish Thread Created");
#endif

	CTL_vDetectThread(&WATCHDOG(CONTROLPUB), &bCONTROLPUBThreadArrayPosition, (void*)CTL_vControlPublishThread);
	WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bCONTROLPUBThreadArrayPosition)); //Task created, inform core

	WATCHDOG_STATE(CONTROLPUB, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

	while (1)
	{
		WATCHDOG_STATE(CONTROLPUB, WDT_SLEEP);
		flags = osFlagWait(CTL_sFlagSis, CTL_UPDATE_CONFIG_DATA | CTL_UPDATE_FILE_INFO | CTL_GET_FILE_INFO |
								 CTL_FORMAT_FILE | CTL_FORMAT_FILE_DONE | CTL_SW_HW_VERSION, true, false, osWaitForever);
		WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

		UOSflags = osFlagGet(UOS_sFlagSis);

		if ((flags & CTL_UPDATE_CONFIG_DATA) > 0)
		{
			if (UOSflags & UOS_SIS_FLAG_CFG_OK)
				eEvType = EVENT_SET;
			else
				eEvType = EVENT_CLEAR;
			PUBLISH_MESSAGE(Control, EVENT_CTL_UPDATE_CONFIG, eEvType, &UOS_sConfiguracao);

		}
		if ((flags & CTL_UPDATE_FILE_INFO) > 0)
		{
			PUBLISH_MESSAGE(Control, EVENT_CTL_UPDATE_FILE_INFO, eEvType, &UOS_sFSInfo);
		}

		if ((flags & CTL_GET_FILE_INFO) > 0)
		{
			PUBLISH_MESSAGE(Control, EVENT_CTL_GET_FILE_INFO, eEvType, NULL);
		}

		if ((flags & CTL_FORMAT_FILE) > 0)
		{
			PUBLISH_MESSAGE(Control, EVENT_CTL_FILE_FORMAT, eEvType, NULL);
		}

		if ((flags & CTL_FORMAT_FILE_DONE) > 0)
		{
			if (UOSflags & UOS_SIS_FLAG_CFG_OK)
				eEvType = EVENT_SET;
			else
				eEvType = EVENT_CLEAR;
			PUBLISH_MESSAGE(Control, EVENT_CTL_FILE_FORMAT_DONE, eEvType, NULL);
		}

		if ((flags & CTL_SW_HW_VERSION) > 0)
		{
			PUBLISH_MESSAGE(Control, EVENT_CTL_SW_HW_VERSION, EVENT_SET, &UOS_sVersaoCod);
		}
	}
	osThreadTerminate(NULL);
}

void CTL_vIdentifyEvent (contract_s* contract)
{
	event_e ePubEvt = GET_PUBLISHED_EVENT(contract);
	eEventType ePubEvType = GET_PUBLISHED_TYPE(contract);
	void *pvPayData = GET_PUBLISHED_PAYLOAD(contract);

	switch (contract->eOrigin)
	{
		case MODULE_ACQUIREG:
		{
			break;
		}
		case MODULE_GPS:
		{
			break;
		}
		case MODULE_FILESYS:
		{
			if (ePubEvt == EVENT_FFS_STATUS)
			{
				if (ePubEvType == EVENT_SET)
				{
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
				}
				else
				{
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
				}
			}

			if (ePubEvt == EVENT_FFS_CFG)
			{
				if (ePubEvType == EVENT_SET)
				{
					UOS_tsConfiguracao *psConfig = pvPayData;
					if (psConfig != NULL)
					{
						UOS_sConfiguracao =*psConfig;
						osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_CFG_OK);
					}
				}
				else
				{
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_CFG_OK);
				}
				osFlagSet(CTL_sFlagSis, CTL_UPDATE_CONFIG_DATA);
			}

			if (ePubEvt == EVENT_FFS_FILE_INFO)
			{

				FFS_sFSInfo *psFSInfo = pvPayData;
				if (psFSInfo != NULL)
				{
					UOS_sFSInfo = *psFSInfo;
					osFlagSet(CTL_sFlagSis, CTL_UPDATE_FILE_INFO);
				}
			}
			if (ePubEvt == EVENT_FFS_FILE_FORMAT_DONE)
			{
				if (ePubEvType == EVENT_SET)
				{
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
				}
				else
				{
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
				}
				osFlagSet(CTL_sFlagSis, CTL_FORMAT_FILE_DONE);
			}

			break;
		}
		case MODULE_GUI:
		{
			if (ePubEvt == EVENT_GUI_UPDATE_SYS_CONFIG)
			{
				UOS_tsConfiguracao *psConfig = pvPayData;
				if (psConfig != NULL)
				{
					UOS_sConfiguracao = *psConfig;
					osFlagSet(CTL_sFlagSis, CTL_UPDATE_CONFIG_DATA);
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_CFG_OK);
				}
			}
			if (ePubEvt == EVENT_GUI_CONFIG_GET_MEMORY_USED)
			{
				osFlagSet(CTL_sFlagSis, CTL_GET_FILE_INFO);
			}
			if (ePubEvt == EVENT_GUI_SYSTEM_FORMAT_FILE)
			{
				osFlagSet(CTL_sFlagSis, CTL_FORMAT_FILE);
			}
			break;
		}
		default:
			break;
	}
}

eAPPError_s CTL_eGetSwHwVersion (void)
{
	eAPPError_s eErr = APP_ERROR_ERROR;
	static peripheral_descriptor_p pIDHandle;
	static uint8_t bIDNumBuffer[8];
	uint32_t dRecvBytes = 0;
	uint8_t bRetry = 0;

	do{
		pIDHandle = DEV_open(PERIPHERAL_DS2411R);

		if (pIDHandle != NULL)
		{
			dRecvBytes = DEV_read(pIDHandle, &bIDNumBuffer, sizeof(bIDNumBuffer));

			if (dRecvBytes)
			{
				memcpy(UOS_sVersaoCod.abNumSerie, &bIDNumBuffer[1], sizeof(UOS_sVersaoCod.abNumSerie));
				eErr = APP_ERROR_SUCCESS;
			}

			DEV_close(pIDHandle);
		}
	} while((dRecvBytes != 0) && (bRetry++ < 5));

	osFlagSet(CTL_sFlagSis, CTL_SW_HW_VERSION);

	return eErr;
}

/* ************************* Main thread ************************************ */
void CTL_vControlThread (void const *argument)
{
	osStatus status;
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Control Thread Created");
#endif

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(ControlQueue);
	INITIALIZE_MUTEX(UOS_MTX_sDataHora);
#ifndef NDEBUG
	REGISTRY_QUEUE(ControlQueue, CTL_vControlThread);
	REGISTRY_QUEUE(UOS_MTX_sDataHora, UOS_MTX_sDataHora);
#endif

	status = osFlagGroupCreate(&CTL_sFlagSis);
	ASSERT(status == osOK);

	memcpy(&UOS_sConfiguracao, &UOS_sConfiguracaoDefault, sizeof(UOS_tsConfiguracao));

	CTL_eInitPublisher();

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromIsobusID = (osThreadId)argument;
	osSignalSet(xMainFromIsobusID, MODULE_CONTROL);

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP, false, false, osWaitForever);

	SIGNATURE_HEADER(ControlFileSys, THIS_MODULE, TOPIC_FILESYS, ControlQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(ControlFileSys), 0) == osOK);

	SIGNATURE_HEADER(ControlGui, THIS_MODULE, TOPIC_GUI, ControlQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(ControlGui), 0) == osOK);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		CTL_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	UOS_sVersaoCod = UOS_sVersaoCodDef;

	CTL_eGetSwHwVersion();

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on any topic */
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		osEvent evt = RECEIVE(ControlQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

		if (evt.status == osEventMessage)
		{
			CTL_vIdentifyEvent(GET_CONTRACT(evt));
		}
	}
	/* Unreachable */
	osThreadSuspend(NULL);
}

void CTL_vControlEmergencyThread (void const *argument)
{
	osFlags dFlagsSis;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Control Emergency Thread Created");
#endif

	CTL_vDetectThread(&WATCHDOG(CONTROLEMY), &bCONTROLEMYThreadArrayPosition, (void*)CTL_vControlEmergencyThread);
	WATCHDOG_STATE(CONTROLEMY, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bCONTROLEMYThreadArrayPosition));    //Task created, inform core

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(CONTROLEMY, WDT_SLEEP);
		osDelay(2000);
		WATCHDOG_STATE(CONTROLEMY, WDT_ACTIVE);
	}
	osThreadTerminate(NULL);
}
