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
CREATE_SIGNATURE(ControlAcquireg);                             //!< Signature Declarations
CREATE_SIGNATURE(ControlSensor);                             //!< Signature Declarations
CREATE_SIGNATURE(ControlFileSys);
CREATE_CONTRACT(Control);                              //!< Create contract for sensor msg publication

osThreadId xCtlEmyThreadId;                            // Holds the control emergency thread id
osThreadId xCtlPubThreadId;                            // Holds the control publish thread id

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
const uint8_t             UOS_abNomeConfigIHM[] = "INTERFACE.CFG";

//Contador de tick do sistema:
volatile uint32_t         UOS_dTicks;

//Estrutura com o código númerico desta versão:
UOS_tsVersaoCod         UOS_sVersaoCod;

//Estrutura da configuracao operacional:
UOS_tsConfiguracao      UOS_sConfiguracao;

//Estrutura da configuracao IHM:
IHM_tsConfig IHM_sConfig;

//Flags que representas os bits do contador de ticks:
osFlagsGroupId UOS_sFlagTicks;

//Flags que representam a diferença de fase entre o início de um ciclo de um
//segundo e o momento atual, de 2 em dois ticks (0 a 62):
osFlagsGroupId UOS_sFlagFase;

//Flags para indicar o status do sistema:
osFlagsGroupId UOS_sFlagSis;

//Timers de sistema a serem utilizados pelas tarefas:
//UOS_tsTimer             asListaTimers[ UOS_bMAX_TIMERS ];

// mutex para a hora
//OS_EVENT  *UOS_MTX_sDataHora;
CREATE_MUTEX(UOS_MTX_sDataHora);

// Semaphores
//Semáforo para sincronizar a tarefa UOS_vTrfSincronismo com o tick:
CREATE_SEMAPHORE(UOS_sSemSincronismo);
//Semáforo para controle de acesso à exibicao de mensagens de alerta:
CREATE_SEMAPHORE(UOS_sSemAlerta);

//Buffer para mensagens do sistema de controle:
//uint8_t UOS_abLCD[ 2560 ];

uint8_t    bMem = 1;

//Variáveis para indicar modo de utilizacao da Serial
uint8_t    UOS_bGPSAtivo;
uint8_t    UOS_bEstadoUART0;
uint8_t    UOS_bSilenciaAlarme;

extern uint16_t AQR_wAlarmes;

/****************************************************************************
  Variáveis locais
*****************************************************************************/

const UOS_tsConfiguracao UOS_sConfiguracaoDefault =
{
//------------------------------------------------------------------------------
//UOS_tsCfgMonitor
  /*FP32   fLimVel;*/                         20.0f, //Limite de velocidade km/h.
  /*INT16U wSementesPorMetro;*/                  15, //Meta de Sementes por Metro. (sementes/m)*10
  /*INT16U wInsensibilidade;*/                  100, //Distância de insensibilidade para falhas. (metros)*10
  /*INT16U wAvalia;*/                          1000, //Distância para avaliacao de aplicacao. (metros)*10
  /*INT16U wDistLinhas;*/                         0, //Distância entre linhas. (centímetros)*10
  /*INT16U wLargImpl;*/                          10, //Largura do implemento. (centímetros)*10
  /*uint8_t  bMonitorArea;*/                    false, //Se está em modo monitor de área. (0 = false, 1 = true )
  /*uint8_t  bNumLinhas;*/                          36, //No. Linhas (1-36)
  /*uint8_t  bDivLinhas;*/                          0, //Divisão da plantadeira (bNumLinhas/2 ou bNumLinhas/2+1)
  /*uint8_t  bSensorAdubo;*/                    false, //Indica presença de sensor de adubo. (bSensorAdubo = 1)
  /*uint8_t  bTolerancia;*/                        20, //Tolerância ao espaçamento entre sementes. (porcentagem)
  /*uint8_t  bTeclaPausaHab;*/                  false, //Habilita o uso da tecla de pausa. (bTeclaPausaHab = 1)
  /*UOS_teIntercala  eIntercala;*/ Sem_Intercalacao,
  /*uint8_t  bPausaAuto;*/                      false, //Habilita o uso da pausa automática.(bPausaAuto = 1)
  /*uint8_t  bLinhasFalhaPausaAuto;*/               0,//Número de linhas em falha para pausa automática. (1-32)
  /*uint8_t  bNumSensorAdicionais;*/                0,//Número de sensores adicionais (0-6)

//------------------------------------------------------------------------------
//tsCfgGPS
  /*INT32S      lFusoHorario;*/               -10800, //Usa fuso horário padrão (Brasília -03:00 )
  /*INT16U      wDistanciaEntreFixos;*/            0,
  /*INT16U      wAnguloEntreFixos;*/               0,
  /*uint8_t       bHorarioVerao;*/               false, //Indica se está em horário de verão (bHorarioVerao = 1)
  /*uint8_t       bSalvaRegistro;*/              false, //Indica se gravacao de registros está ativada

//------------------------------------------------------------------------------
//tsCfgBluetooth
  /*uint8_t bInstalado;*/                        false,
  /*uint8_t bTamPIN;*/                               0,
  /*uint8_t abNomeDispositivo[30];*/                 0,
  /*uint8_t abPIN[16];*/                             0,

//------------------------------------------------------------------------------
//tsCfgVeiculo
  /*uint8_t abCodigo[6];*/                           0,

//------------------------------------------------------------------------------
  /*INT16U wCRC16;*/                               0
};

/******************************************************************************
* Module typedef
*******************************************************************************/
typedef uint16_t (*changeTest)(uint16_t data, uint8_t step);

/**
* Module Threads
*/
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
    CONTROL_MODULES
};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(CONTROLPUB);                                   //!< WDT pointer flag
WATCHDOG_CREATE(CONTROLMGT);                                   //!< WDT pointer flag
WATCHDOG_CREATE(CONTROLEMY);                                   //!< WDT pointer flag
uint8_t bCONTROLPUBThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bCONTROLMGTThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bCONTROLEMYThreadArrayPosition = 0;                    //!< Thread position in array

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
uint8_t * CTL_WDTData(uint8_t * pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void CTL_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
    *bPosition = 0;
    while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
    {
        (*bPosition)++;
    }
    *flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

static void CTL_vCreateThread(const Threads_t sThread )
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever);
    }
}

PubMessage sControlPubMsg;

void CTL_vControlPublishThread(void const *argument)
{
    osEvent evt;
    osFlags flags;
    uint32_t* dPayload;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Control Publish Thread Created");
#endif

    CTL_vDetectThread(&WATCHDOG(CONTROLPUB), &bCONTROLPUBThreadArrayPosition, (void*)CTL_vControlPublishThread);
    WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bCONTROLPUBThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    while(1)
    {
//        WATCHDOG_STATE(CONTROLPUB, WDT_SLEEP);
//        flags = osFlagWait(UOS_sFlagSis, (UOS_SIS_FLAG_SEND_FLAG_STATUS | UOS_SIS_FLAG_SIS_OK), true, false, 100);
//        WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);
//
//        if(flags != UOS_SIS_FLAG_NENHUM)
//        {
//            if((flags & UOS_SIS_FLAG_SEND_FLAG_STATUS) > 0)
//            {
//                osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_SEND_FLAG_STATUS);
//
//                sControlPubMsg.eEvtType = EVENT_UPDATE;
//                sControlPubMsg.dEvent = osFlagGet(UOS_sFlagSis);
//                sControlPubMsg.vPayload = (void*) &UOS_sConfiguracao;
//                MESSAGE_PAYLOAD(Control) = (void*) &sControlPubMsg;
//                PUBLISH(CONTRACT(Control), 0);
//            }
//
//            if((flags & UOS_SIS_FLAG_SIS_OK) > 0)
//            {
//                sControlPubMsg.eEvtType = EVENT_SET;
//                sControlPubMsg.dEvent = UOS_SIS_FLAG_SIS_OK;
//                sControlPubMsg.vPayload = (void*) &UOS_sConfiguracao;
//                MESSAGE_PAYLOAD(Control) = (void*) &sControlPubMsg;
//                PUBLISH(CONTRACT(Control), 0);
//            }
//        }

        /* Pool the device waiting for */
        WATCHDOG_STATE(CONTROLPUB, WDT_SLEEP);
  //      evt = osSignalWait ((UOS_BUZZER_ON | UOS_BUZZER_OFF), 100);
        osDelay(2000);
        WATCHDOG_STATE(CONTROLPUB, WDT_ACTIVE);

 /*       if(evt.status == osEventSignal)
        {
            switch(evt.value.signals)
            {
                case UOS_BUZZER_ON:

                    *dPayload = 'A';

                     Publish the array at the CONTROL topic
                    MESSAGE_HEADER(Control, 1, 1, MT_ARRAYBYTE);
                    MESSAGE_PAYLOAD(Control) = (void*) dPayload;
                    PUBLISH(CONTRACT(Control), 0);

                    break;
                case UOS_BUZZER_OFF:

                    *dPayload = 'B';

                     Publish the array at the CONTROL topic
                    MESSAGE_HEADER(Control, 1, 1, MT_ARRAYBYTE);
                    MESSAGE_PAYLOAD(Control) = (void*) dPayload;
                    PUBLISH(CONTRACT(Control), 0);

                    break;
                default:
                    break;
            }
        }*/
    }
    osThreadTerminate(NULL);
}

void CTL_vIdentifyEvent(contract_s* contract)
{
    osStatus status;

    switch(contract->eOrigin)
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
        	if (GET_PUBLISHED_EVENT(contract) == EVENT_FFS_CFG)
        	{
    			if(GET_PUBLISHED_TYPE(contract) == EVENT_SET)
    			{
    				memcpy(&UOS_sConfiguracao, (UOS_tsConfiguracao*)(GET_PUBLISHED_PAYLOAD(contract)), sizeof(UOS_tsConfiguracao));
    				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
    			}
    			else
    			{
    				osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_FFS_OK);
    			}
        	}
        	if (GET_PUBLISHED_EVENT(contract) == EVENT_FFS_INTERFACE_CFG)
        	{
    			if(GET_PUBLISHED_TYPE(contract) == EVENT_SET)
    			{
    				memcpy(&IHM_sConfig, (IHM_tsConfig*)(GET_PUBLISHED_PAYLOAD(contract)), sizeof(IHM_sConfig));
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

    xCtlPubThreadId = osThreadGetId();

    // TODO: This is executed after file system initialization
    // Copy default configurations to start; because we don't have file system
    memcpy( &UOS_sConfiguracao, &UOS_sConfiguracaoDefault, sizeof( UOS_sConfiguracao ) );
    uint8_t abCodigo[] = { 0x25, 0x00, 0xA0, 0x00, 0x00, 0x00 };
    memcpy( UOS_sConfiguracao.sVeiculo.abCodigo, abCodigo ,sizeof( UOS_sConfiguracao.sVeiculo.abCodigo ) );

    // Create an flag to indicate the system status...
    status = osFlagGroupCreate(&UOS_sFlagSis);
    ASSERT(status == osOK);

    SIGNATURE_HEADER(ControlAcquireg, THIS_MODULE, TOPIC_ACQUIREG, ControlQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(ControlAcquireg), 0) == osOK);

    SIGNATURE_HEADER(ControlSensor, THIS_MODULE, TOPIC_SENSOR, ControlQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(ControlSensor), 0) == osOK);

    SIGNATURE_HEADER(ControlFileSys, THIS_MODULE, TOPIC_FILESYS, ControlQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(ControlFileSys), 0) == osOK);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while(THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        CTL_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }

    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId) argument;
    osSignalSet(xMainFromIsobusID, MODULE_CONTROL);

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

void CTL_vControlEmergencyThread(void const *argument)
{
    osFlags dFlagsSis;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Control Emergency Thread Created");
#endif

    xCtlEmyThreadId = osThreadGetId();

    CTL_vDetectThread(&WATCHDOG(CONTROLEMY), &bCONTROLEMYThreadArrayPosition, (void*)CTL_vControlEmergencyThread);
    WATCHDOG_STATE(CONTROLEMY, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bCONTROLEMYThreadArrayPosition));//Task created, inform core

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
      if ( ( dFlagsSis & UOS_SIS_FLAG_FFS_OK ) > 0 )
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

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(CONTROLEMY, WDT_SLEEP);
        osDelay(2000);
        WATCHDOG_STATE(CONTROLEMY, WDT_ACTIVE);
    }
    osThreadTerminate(NULL);
}

void CTL_vControlManagementThread(void const *argument)
{
#define EI_BOOT       0    //Interface no estado inicial.
#define EI_ERRO_FFS   1    //Interface no estado memoria cheia.
#define EI_COMUNICA   2    //Interface no estado comunicacao em andamento.
#define EI_ERRO_CFG   3    //Interface no estado erro na configuracao.
#define EI_NOVO_REG   4    //Interface no estado novo registro.
#define EI_MEMO_CHEIA 5    //Interface no estado memoria cheia.
#define EI_EMERGENCIA 6    //Interface no estado de emergencia.
#define EI_ALARME     7    //Interface no estado de alarme.
#define EI_IHM        8    //Interface no estado de alerta.
#define EI_TOLERANCIA 9    //Interface no estado de alarme por falha na tolerancia
#define EI_NORMAL     0xFF //Interface no estado normal.

    osStatus status;
    osFlags dFlagSis;
    uint8_t bAlarme;
    uint8_t bEstadoInterface;
    uint8_t bI;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Control Management Thread Created");
#endif

    CTL_vDetectThread(&WATCHDOG(CONTROLMGT), &bCONTROLMGTThreadArrayPosition, (void*)CTL_vControlManagementThread);
    WATCHDOG_STATE(CONTROLMGT, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bCONTROLMGTThreadArrayPosition));//Task created, inform core

    //Semaforo para sincronizar esta tarefa com o tick:
    INITIALIZE_SEMAPHORE(UOS_sSemSincronismo, 1);

    //Semaforo para controle de acesso a� exibicao de mensagens de alerta:
    INITIALIZE_SEMAPHORE(UOS_sSemAlerta, 1);

    // Initialize flag group to indicate events
    //Flags que representam os bits do contador de ticks:
    status = osFlagGroupCreate(&UOS_sFlagTicks);
    ASSERT(status == osOK);

    //Flags que a diferenca de fase entre o inicio de um ciclo de um
    //segundo e o momento atual, de 2 em dois ticks (0 a 62):
    status = osFlagGroupCreate(&UOS_sFlagFase);
    ASSERT(status == osOK);

      // cria um mutex para a data e hora
    INITIALIZE_MUTEX(UOS_MTX_sDataHora);

    //Inicia o contador de ticks:
    UOS_dTicks = 0;

//    //Verifica pino que seleciona GPS ou Serial
//    if( GPS_ATIVO > 0 )
//    {
//      //Inicia o estado da UART0 como GPS
//      UOS_bEstadoUART0 = UOS_GPS;
//    }
//    else
//    {
//      //Inicia o estado da UART0 como SERIAL
//      UOS_bEstadoUART0 = UOS_SERIAL;
//    }

    while(1)
    {
        /* Pool the device waiting for */
//        WATCHDOG_STATE(CONTROLMGT, WDT_SLEEP);
//        osDelay(2000);
//        WATCHDOG_STATE(CONTROLMGT, WDT_ACTIVE);

        //        //Verifica pino que seleciona GPS ou Serial
        //        if( GPS_ATIVO > 0 )
        //        {
        //            UOS_bGPSAtivo = true;
        //        }
        //        else
        //        {
        //            UOS_bGPSAtivo = false;
        //        }

        //--------------------------------------------------------------------------
        //Define o estado da interface em funcao dos flags de status:

        //Flags de status:
        WATCHDOG_STATE(CONTROLMGT, WDT_SLEEP);
        dFlagSis = osFlagGet(UOS_sFlagSis);
        WATCHDOG_STATE(CONTROLMGT, WDT_ACTIVE);

        //Atualizacao da variável de controle da interface
        //Se o sistema já está iniciado:
        if ( ( dFlagSis & UOS_SIS_FLAG_SIS_OK ) > 0 )
        {
            if ( ( dFlagSis & UOS_SIS_FLAG_FFS_OK ) == 0 ) //Erro no sistema de arquivos tem a prioridade mais alta.
            {
                bEstadoInterface = EI_ERRO_FFS;
            }
            else
            {
                if ( ( dFlagSis & UOS_SIS_FLAG_REGISTRO ) > 0 )  //Seguido da criacao de registro.
                {
                    //Este flag deve ser reconhecido aqui:
                    osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_REGISTRO);

                    bEstadoInterface = EI_NOVO_REG;
                }
                else
                {
                    if ( ( dFlagSis & UOS_SIS_FLAG_COMUNICA ) > 0 ) //Seguido de comunicaçao em andamento
                    {
                        //Este flag deve ser reconhecido aqui:
                        osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_COMUNICA);

                        bEstadoInterface = EI_COMUNICA;
                    }
                    else
                    {
                        if ( ( dFlagSis & UOS_SIS_FLAG_CFG_OK ) == 0 ) //Seguido do erro na configuracao.
                        {
                            if( UOS_bSilenciaAlarme == false )
                            {
                                bEstadoInterface = EI_ERRO_CFG;
                            }
                            else
                            {
                                bEstadoInterface = EI_NORMAL;
                            }

                        }
                        else
                        {
                            if ( ( dFlagSis & UOS_SIS_FLAG_MEM_OK ) == 0 ) //Seguido de memoria cheia.
                            {
                                bEstadoInterface = EI_MEMO_CHEIA;
                            }
                            else
                            {
                                if ( ( dFlagSis & UOS_SIS_FLAG_EMERGENCIA ) > 0 ) //Seguido das emergenncias.
                                {
                                    bEstadoInterface = EI_EMERGENCIA;
                                }
                                else
                                {
                                    if ( ( dFlagSis & UOS_SIS_FLAG_ALARME ) > 0 ) //Seguido dos alarmes.
                                    {
                                        bEstadoInterface = EI_ALARME;
                                    }
                                    else
                                    {
                                        if ( ( dFlagSis & UOS_SIS_FLAG_ALARME_TOLERANCIA ) > 0 )
                                        {
                                            //Este flag deve ser reconhecido aqui:
                                            osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ALARME_TOLERANCIA);

                                            bEstadoInterface = EI_TOLERANCIA;
                                        }
                                        else
                                        {
                                            bEstadoInterface = EI_NORMAL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else //Senão, não completou a inicializacao:
        {
            //Verifica se ainda está aguardando estabilizacao da fonte de energia:
            if ( ( dFlagSis & UOS_SIS_FLAG_BOOT ) > 0 )
            {
                bEstadoInterface = EI_BOOT;
            }
            else
            {
                bEstadoInterface = EI_NORMAL;
            }
        }


        WATCHDOG_STATE(CONTROLMGT, WDT_ACTIVE);
        //--------------------------------------------------------------------------
        //Todas as sinalizações devem ter um ciclo mínimo de 1 segundo.
        //Para tanto, a variavel bEstadoInterface é atualizada acima, fora do
        //loop abaixo que dura 1/64 segundo * 64 = 1 segundo;
        for ( bI = 0; bI < TICK; bI++)
        {

            //------------------------------------------------------------------------
            //Atualiza os flags de ticks e fases:

            //Atualiza os flags de frequência de acordo com os bits do tick:
//            osFlagClear(UOS_sFlagTicks, UOS_SINC_FLAG_TODOS);  // TODO: verify this line
            osFlagSet(UOS_sFlagTicks, UOS_dTicks);

            //Só nos ticks pares:
            if ( ( UOS_dTicks & 0x01 ) == 0 )
            {
                //Atualiza os flags de fase de acordo com os bits do tick:
//                osFlagClear(UOS_sFlagFase, UOS_FASE_FLAG_TODOS);  // TODO: verify this line
//                osFlagSet(UOS_sFlagFase, ( 1UL << ( ( UOS_dTicks >> 1 ) & 0x1F ) ));  // TODO: verify this line
            }


            //------------------------------------------------------------------------
            //Define o estado do led vermelho e da buzina:

            /*
              Bit 0 = UOS_SINC_FLAG_32HZ 0101010101010101010101010101010101010101010101010101010101010101
              Bit 1 = UOS_SINC_FLAG_16HZ 0011001100110011001100110011001100110011001100110011001100110011
              Bit 2 = UOS_SINC_FLAG_8HZ  0000111100001111000011110000111100001111000011110000111100001111
              Bit 3 = UOS_SINC_FLAG_4HZ  0000000011111111000000001111111100000000111111110000000011111111
              Bit 4 = UOS_SINC_FLAG_2HZ  0000000000000000111111111111111100000000000000001111111111111111
              Bit 5 = UOS_SINC_FLAG_1HZ  0000000000000000000000000000000011111111111111111111111111111111
             */
            //Inicialmente sem alarme:
            bAlarme      = false;

            //Controle do LED Vermelho e do Buzzer:
            switch ( bEstadoInterface )
            {
                case EI_BOOT: //Se o sistema está iniciando:
                    if ( ( UOS_dTicks & 0x0038 ) == 0 )
                    {
                        bAlarme      = true;
                    }
                    break;
                case EI_ERRO_FFS:
                    //Sem alarme e led sempre aceso.
                    bAlarme      = false;
                    break;
                case EI_NOVO_REG: //Se um novo registro de viagem está sendo criado:
                    if ( ( UOS_dTicks & 0x0024 ) == 0 )
                    {
                        bAlarme      = true;
                    }
                    break;
                case EI_COMUNICA: //Se temos comunicacao em andamento:
                    if ( ( UOS_dTicks & 0x000F ) == 0 )
                    {
                        bAlarme      = true;

                    }
                    break;
                case EI_ERRO_CFG: //Se o sistema está com erro nos configuracao:
                    if ( ( UOS_dTicks & 0x0034 ) == 0 )
                    {
                        bAlarme      = true;

                    }
                    break;
                case EI_MEMO_CHEIA: //Se está com a memória cheia:
                    //Sem alarme
                    bAlarme      = false;

                    break;
                case EI_EMERGENCIA: //Se existe uma situacao de emergência:
                    if ( ( UOS_dTicks & 0x0004 ) == 0 )
                    {
                        bAlarme      = true;
                    }
                    break;

                case EI_ALARME: //Se existe uma situacao de alarme:
                    if( (AQR_wAlarmes & AQR_NOVO_SENSOR) > 0 )
                    {

                        if ( ( UOS_dTicks & 0x0004 ) == 0 )
                        {
                            bAlarme      = true;
                        }
                    }
                    else
                    {
                        if( ( ( AQR_wAlarmes & AQR_SENSOR_DESCONECTADO ) > 0 )||
                                ( ( AQR_wAlarmes & AQR_FALHA_LINHA         ) > 0 )||
                                ( ( AQR_wAlarmes & AQR_FALHA_INSTALACAO    ) > 0 )  )
                        {
                            if ( ( UOS_dTicks & 0x0000 ) == 0 )
                            {
                                bAlarme      = true;
                            }
                        }
                        else
                        {
                            if( ( (AQR_wAlarmes & AQR_EXC_VELOCIDADE) > 0 )||
                                    ( (AQR_wAlarmes & AQR_FALHA_GPS     ) > 0 )  )

                            {
                                if ( ( UOS_dTicks & 0x0010 ) == 0 ) // 2 Hz
                                {
                                    bAlarme      = true;
                                }
                            }
                        }
                    }
                    break;

                case EI_TOLERANCIA: //Se existe uma falha na tolerancia da linha
                    if ( ( UOS_dTicks & 0x0028 ) == 0 ) //Apenas 2 beeps
                    {
                        bAlarme = true;
                    }
                    break;

                default:
                {
                    //Sem alarme.
                    bAlarme      = false;

                    break;
                }
            } //Fim switch ( bEstadoInterface )

            //------------------------------------------------------------------------
            //            //Realimentacao acústica do teclado:
            //
            //            //Realimentacao acústica do teclado:
            //            if ( KBD_bIHM > 0 )
            //            {
            //                KBD_bIHM--;
            //                bAlarme = true;
            //            }

            //------------------------------------------------------------------------
            //Atua na buzina:

            //Controle da saída de alarme (buzzer):
            if ( bAlarme != false )
            {
                // Send buffer on to buzzer module
                osSignalSet(xCtlPubThreadId, UOS_BUZZER_ON);
            }
            else
            {
                // Send buffer off to buzzer module
                osSignalSet(xCtlPubThreadId, UOS_BUZZER_OFF);
            }

            //------------------------------------------------------------------------
            //Fim do ciclo. Até o próximo tick:

            //Aguarda a ocorrência de um tick:
            WATCHDOG_STATE(CONTROLMGT, WDT_SLEEP);
            status = WAIT_SEMAPHORE(UOS_sSemSincronismo, osWaitForever);
            ASSERT(status == osOK);
            WATCHDOG_STATE(CONTROLMGT, WDT_ACTIVE);

            //            //Se está em meio a um acesso à memória:
            //            if ( FFS_SPI_bWatchdogTimer > 0 )
            //            {
            //                //Decrementa o contador do watchdog timer:
            //                FFS_SPI_bWatchdogTimer--;
            //            }

            //Incrementa contador de ticks:
            UOS_dTicks++;

        }//Fim do loop de 1 segundo.

    }
    osThreadTerminate(NULL);
}
