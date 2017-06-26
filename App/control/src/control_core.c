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
#define QUEUE_SIZEOFCONTROL 5

#define THIS_MODULE MODULE_CONTROL

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
DECLARE_QUEUE(ControlQueue, QUEUE_SIZEOFCONTROL);      //!< Declaration of Interface Queue
CREATE_SIGNATURE(ControlAcquireg);//!< Signature Declarations
CREATE_SIGNATURE(ControlSensor);//!< Signature Declarations
CREATE_SIGNATURE(ControlFileSys);
CREATE_SIGNATURE(ControlGui);
CREATE_CONTRACT(Control);                              //!< Create contract for sensor msg publication

//!< From MPA2500
/******************************************************************************
 Versão deste software com 4 campos de 8 caracteres na seguinte ordem:
 Cliente, Produto, Versão e Observacao.
 *******************************************************************************/
//Código numérico desta versão:
const UOS_tsVersaoCod UOS_sVersaoCodDef = {
	0xFFFF,           //Flag
	5,                //Modelo MPA2500
	1,                //Versão
	1,                //Revisão
	1001,              //Build
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

//Estrutura da configuracao IHM:
IHM_tsConfig IHM_sConfig;

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

/****************************************************************************
 Variáveis locais
 *****************************************************************************/
PubMessage sControlPubMsg;

const UOS_tsConfiguracao UOS_sConfiguracaoDefault =
	{

	//------------------------------------------------------------------------------
//UOS_tsCfgMonitor
		/*FP32   fLimVel;*/20.0f,//Limite de velocidade km/h.
		/*INT16U wSementesPorMetro;*/15, //Meta de Sementes por Metro. (sementes/m)*10
		/*INT16U wInsensibilidade;*/100, //Distância de insensibilidade para falhas. (metros)*10
		/*INT16U wAvalia;*/1000, //Distância para avaliacao de aplicacao. (metros)*10
		/*INT16U wDistLinhas;*/0, //Distância entre linhas. (centímetros)*10
		/*INT16U wLargImpl;*/10, //Largura do implemento. (centímetros)*10
		/*uint8_t  bMonitorArea;*/false, //Se está em modo monitor de área. (0 = false, 1 = true )
		/*uint8_t  bNumLinhas;*/36, //No. Linhas (1-36)
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
		/*uint32_t dVeiculo;*/2500,

		//------------------------------------------------------------------------------
		/*INT16U wCRC16;*/0
	};

/******************************************************************************
 * Module typedef
 *******************************************************************************/
typedef uint16_t (*changeTest) (uint16_t data, uint8_t step);

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

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Control Publish Thread Created");
#endif

	CTL_vDetectThread(&WATCHDOG(CONTROLPUB), &bCONTROLPUBThreadArrayPosition, (void*)CTL_vControlPublishThread);
	WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bCONTROLPUBThreadArrayPosition)); //Task created, inform core

	//CTL_eInitPublisher();

	while (1)
	{
        WATCHDOG_STATE(CONTROLPUB, WDT_SLEEP);
        flags = osFlagWait(CTL_sFlagSis, CTL_UPDATE_CONFIG_DATA, true, false, osWaitForever);
        WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

        UOSflags = osFlagGet(UOS_sFlagSis);

			if ((flags & CTL_UPDATE_CONFIG_DATA) > 0)
			{
				sControlPubMsg.dEvent = EVENT_CTL_UPDATE_CONFIG;

				if (UOSflags & UOS_SIS_FLAG_FFS_OK)
					sControlPubMsg.eEvtType = EVENT_SET;
				else
					sControlPubMsg.eEvtType = EVENT_CLEAR;
				sControlPubMsg.vPayload = (void*)&UOS_sConfiguracao;
				MESSAGE_PAYLOAD(Control) = (void*)&sControlPubMsg;
				PUBLISH(CONTRACT(Control), 0);
        }
	}
	osThreadTerminate(NULL);
}

void CTL_vIdentifyEvent (contract_s* contract)
{
	event_e ePubEvt = GET_PUBLISHED_EVENT(contract);
	eEventType ePubEvType = GET_PUBLISHED_TYPE(contract);

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
			if (ePubEvt == EVENT_FFS_CFG)
			{
				if (ePubEvType == EVENT_SET)
				{
					if (GET_PUBLISHED_PAYLOAD(contract) != NULL)
					{
					memcpy(&UOS_sConfiguracao, (UOS_tsConfiguracao*)(GET_PUBLISHED_PAYLOAD(contract)),
						sizeof(UOS_tsConfiguracao));
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
					}
					else
						ePubEvt = EVENT_FFS_CFG;
				}
				else
				{
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
				}
				osFlagSet(CTL_sFlagSis, CTL_UPDATE_CONFIG_DATA);
			}
			break;
		}
		case MODULE_GUI:
		{
			if (ePubEvt == EVENT_GUI_UPDATE_SYS_CONFIG)
			{
				if (GET_PUBLISHED_PAYLOAD(contract) != NULL)
				{
					memcpy(&UOS_sConfiguracao, (UOS_tsConfiguracao*)(GET_PUBLISHED_PAYLOAD(contract)),
								sizeof(UOS_tsConfiguracao));
					osFlagSet(CTL_sFlagSis, CTL_UPDATE_CONFIG_DATA);
				}
			}
			break;
		}
		default:
			break;
	}
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

	// TODO: This is executed after file system initialization
	// Copy default configurations to start; because we don't have file system
	memcpy(&UOS_sConfiguracao, &UOS_sConfiguracaoDefault, sizeof(UOS_sConfiguracao));

	status = osFlagGroupCreate(&CTL_sFlagSis);
	ASSERT(status == osOK);


	CTL_eInitPublisher();

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromIsobusID = (osThreadId)argument;
	osSignalSet(xMainFromIsobusID, MODULE_CONTROL);

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP, false, false, osWaitForever);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		CTL_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}


	SIGNATURE_HEADER(ControlAcquireg, THIS_MODULE, TOPIC_ACQUIREG, ControlQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(ControlAcquireg), 0) == osOK);

	SIGNATURE_HEADER(ControlSensor, THIS_MODULE, TOPIC_SENSOR, ControlQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(ControlSensor), 0) == osOK);

	SIGNATURE_HEADER(ControlFileSys, THIS_MODULE, TOPIC_FILESYS, ControlQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(ControlFileSys), 0) == osOK);

	SIGNATURE_HEADER(ControlGui, THIS_MODULE, TOPIC_GUI, ControlQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(ControlGui), 0) == osOK);

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

	// At this point this thread will be suspended by ControlThread
	// Waiting for and osThreadResume
	WATCHDOG_STATE(CONTROLEMY, WDT_SLEEP);
	osSignalWait(0xAAAA, osWaitForever);
	WATCHDOG_STATE(CONTROLEMY, WDT_ACTIVE);

	// Turn off interrupts
	__disable_irq();

	// Turn off peripheral and sensors power source
	// DISABLE PS9
	// DISABLE PS5
	// DISABLE GPS??

	//Verifica os flags de sistema:
	dFlagsSis = osFlagGet(UOS_sFlagSis);

	//Se o sistema de arquivos está rodando:
	if ((dFlagsSis & UOS_SIS_FLAG_FFS_OK) > 0)
	{
		//Acumula os valores de áreas trabalhadas
//        AQR_vAcumulaArea();

		//Finaliza os arquivos da aquisicao:
//        AQR_vEmergencia();

//        CAN_vSalvaArquivoErros();

		//Finaliza o sistema de arquivos:
//        FFS_vEmergencia();
	}

//      if( REBOOT == TRF_EMERGENCIA )
//      {
//        //Reajusta watchdog timer para aguardar a descarga dos super capacitores:
//        WDTC = UOS_WDT_dTMP_EMERGENCIA;
//      }
//      else
//      {
//        //Reajusta o watchdog timer para aguardar reinicializacao forçada:
//        WDTC = UOS_WDT_dTMP_REBOOT;
//      }

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(CONTROLEMY, WDT_SLEEP);
		osDelay(2000);
		WATCHDOG_STATE(CONTROLEMY, WDT_ACTIVE);
	}
	osThreadTerminate(NULL);
}
