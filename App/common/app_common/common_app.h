/****************************************************************************
* Title                 :   common_app Include File
* Filename              :   common_app.h
* Author                :   thiago.palmieri
* Origin Date           :   03/03/2016
* Version               :   1.0.0
* Compiler              :   GCC 5.2 2015q4 / ICCARM 7.50.2.10312
* Target                :   LPC4357
* Notes                 :   None
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
*    Date    Version   Author         Description
*  03/03/2016   1.0.0   thiago.palmieri broker_list include file Created.
*
*****************************************************************************/
/** @file common_app.h
 *  @brief This file provides common functions and configurations for the application layer
 *
 */
#ifndef COMMON_COMMON_APP_H_
#define COMMON_COMMON_APP_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include "ring_buffer.h"
#include "debug_tool.h"
#include "sensor_app.h"
#include "isobus_app.h"
#include "acquireg_app.h"
#include "gps_app.h"

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/
#define TICK 1000
/**
 * This MACRO will Create a Ring Buffer
 */
#if defined(__IAR_SYSTEMS_ICC__)
#define CREATE_RINGBUFFER(type, name, size) \
    static RINGBUFF_T name##Buffer @ "MY_DATA"; \
    type a##name##Buffer[size] @ "MY_DATA";
#elif defined(__GNUC__) && defined (__ARMEL__)
#define CREATE_RINGBUFFER(type, name, size) \
    __attribute__((section("RAMLOC40"))) static RINGBUFF_T name##Buffer; \
    __attribute__((section("RAMLOC40"))) type a##name##Buffer[size];

//    __attribute__((section("RamLoc32"))) static RINGBUFF_T name##Buffer;
//    __attribute__((section("RamLoc32"))) type a##name##Buffer[size];
#elif defined(UNITY_TEST)
#define CREATE_RINGBUFFER(type, name, size) \
    static RINGBUFF_T name##Buffer; \
    type a##name##Buffer[size]
#endif
/**
 * This MACRO will Initialize the Ring Buffer previously created
 */
#define INITIALIZE_RINGBUFFER(type, name, size) \
    RingBuffer_Init(&name##Buffer, a##name##Buffer, sizeof(type), size);  \
    RingBuffer_Flush(&name##Buffer);

/*!< Use Buffer */
#define BUFFER(name)  name##Buffer

/*!< Extern a Message Queue */
#define EXTERN_QUEUE(QueueName) \
    extern osMessageQId  QueueName;

//!< *****************
//!< Create a Timer Definition
#define CREATE_TIMER(TimerName, TimerCallbackFunction) \
    osTimerDef(TimerName, TimerCallbackFunction); \
    osTimerId  TimerName;
/*!< Create and Initialize a Timer */
#define INITIALIZE_TIMER(TimerName, TimerType) \
  (TimerName = osTimerCreate(osTimer(TimerName), TimerType, NULL));  \
    ASSERT(TimerName != NULL);
/*!< Start Timer */
#define START_TIMER(TimerName, TimerDelay) \
  (osTimerStart(TimerName, TimerDelay))
/*!< Stop Timer */
#define STOP_TIMER(TimerName) \
  (osTimerStop(TimerName))
/*!< Extern a timer */
#define EXTERN_TIMER(TimerName) \
    extern osTimerId  TimerName

//!< *****************
//!< Create a Mutex Definition
#define CREATE_MUTEX(MutexName) \
  osMutexDef(MutexName); \
  osMutexId MutexName;
/*!< Create and Initialize a Mutex */
#define INITIALIZE_MUTEX(MutexName) \
  (MutexName = osMutexCreate(MutexName)); \
  ASSERT(MutexName != NULL);
/*!< Mutex wait */  
#define WAIT_MUTEX(MutexName, TimeToWait) \
   osMutexWait(MutexName, TimeToWait)
/*!< Release Mutex */
#define RELEASE_MUTEX(MutexName) \
   osMutexRelease(MutexName)
/*!< Delete Mutex */
#define DELETE_MUTEX(MutexName) \
   osMutexDelete(MutexName)
/*!< Extern a mutex */
#define EXTERN_MUTEX(MutexName) \
    extern osMutexId  MutexName

//!< *****************
//!< Create a Semaphore Definition
#define CREATE_SEMAPHORE(SemaphoreName) \
  osSemaphoreDef(SemaphoreName); \
  osSemaphoreId SemaphoreName;
/*!< Create and Initialize a Semaphore */
#define INITIALIZE_SEMAPHORE(SemaphoreName, Count) \
  (SemaphoreName = osSemaphoreCreate(SemaphoreName, Count)); \
  ASSERT(SemaphoreName != NULL);
/*!< Semaphore wait */
#define WAIT_SEMAPHORE(SemaphoreName, TimeToWait) \
  (osSemaphoreWait(SemaphoreName, TimeToWait))
/*!< Release Semaphore */
#define RELEASE_SEMAPHORE(SemaphoreName) \
  (osSemaphoreRelease(SemaphoreName))
/*!< Delete Semaphore */
#define DELETE_SEMAPHORE(SemaphoreName) \
  (osSemaphoreDelete(SemaphoreName))
/*!< Extern a semaphore */
#define EXTERN_SEMAPHORE(SemaphoreName) \
    extern osSemaphoreId  SemaphoreName

//!< *****************
//!< Create a Message Queue Definition
#define CREATE_LOCAL_QUEUE(QueueName, MsgType, QueueSize) \
    osMessageQDef(QueueName, QueueSize, MsgType); \
    osMessageQId  QueueName;
/*!< Create and Initialize a Message Queue */
#define INITIALIZE_LOCAL_QUEUE(QueueName) \
  (QueueName = osMessageCreate(osMessageQ(QueueName), NULL));  \
    ASSERT(QueueName != NULL);
/*!< Put message to an local queue */
#define PUT_LOCAL_QUEUE(QueueName, message, time) (osMessagePut(QueueName, (uint32_t)&message, time))
/*!< Receive from Queue */
#define RECEIVE_LOCAL_QUEUE(fromQueue, time) (osMessageGet(fromQueue, time))

#define GET_PUBLISHED_EVENT(contract)   ((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->dEvent
#define GET_PUBLISHED_TYPE(contract) 	((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->eEvtType
#define GET_PUBLISHED_PAYLOAD(contract) ((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->vPayload

/******************************************************************************
* Typedefs
*******************************************************************************/
/**
 *  This Typedef defines the valid Origins to be used into contracts
 */
typedef enum origin_e
{
    MODULE_BROKER                  = 1, //!< MODULE_BROKER
    MODULE_DIAGNOSTIC                 , //!< MODULE_DIAGNOSTIC
    MODULE_UPDATE                     , //!< MODULE_UPDATE
    MODULE_AUTHENTICATION             , //!< MODULE_AUTHENTICATION
    MODULE_COMM_CBA                   , //!< MODULE_COMM
    MODULE_ACTUATOR                   , //!< MODULE_ACTUATOR_CORE
    MODULE_ACTUATOR_LCD               , //!< MODULE_ACTUATOR_LCD
    MODULE_ACTUATOR_KEY               , //!< MODULE_ACTUATOR_KEY
    MODULE_OUTPUT_LCD                 , //!< MODULE_OUTPUT
    MODULE_INPUT_KEYPAD               , //!< MODULE_INPUT
    MODULE_SENSOR                     , //!< MODULE_SENSOR
    MODULE_SENSOR_LUMINOSITY          , //!< MODULE_SENSOR
    MODULE_SENSOR_COMPASS             , //!< MODULE_SENSOR
    MODULE_ISOBUS					  , //!< MODULE_ISOBUS
    MODULE_BUZZER                     , //!< MODULE_BUZZER
    MODULE_FILESYS                    , //!< MODULE_FILESYS
    MODULE_GPS                        , //!< MODULE_GPS
    MODULE_ACQUIREG                   , //!< MODULE_ACQUIREG
    MODULE_CONTROL                    , //!< MODULE_CONTROL
    MODULE_GUI                        , //!< MODULE_GUI
    MODULE_RECORDS                    , //!< MODULE_RECORDS
    MODULE_SIMULATOR                  , //!< MODULE_SIMULATOR
    MODULE_TEST                       , //!< MODULE_TEST
    MODULE_INVALID                    , //!< INVALID MODULE
} origin_e;

/**
 *  This Typedef defines the valid Destines to be used into signatures
 */
typedef origin_e destine_e;

/**
 * This Typedef defines the valid Topics
 */
typedef enum topic_e
{
    TOPIC_SENSOR,        //!< TOPIC_SENSOR
    TOPIC_INPUT,         //!< TOPIC_INPUT
    TOPIC_OUTPUT,        //!< TOPIC_OUTPUT
    TOPIC_ACTUATOR,      //!< TOPIC_ACTUATOR
    TOPIC_COMM,          //!< TOPIC_COMM
    TOPIC_AUTHENTICATION,//!< TOPIC_AUTHENTICATION
    TOPIC_DIAGNOSTIC,    //!< TOPIC_DIAGNOSTIC
    TOPIC_UPDATE,        //!< TOPIC_UPDATE
    TOPIC_MEMORY,        //!< TOPIC_MEMORY
    TOPIC_ISOBUS,		 //!< TOPIC_ISOBUS
    TOPIC_BUZZER,        //!< TOPIC_BUZZER
    TOPIC_FILESYS,       //!< TOPIC_FILESYS
    TOPIC_GPS,           //!< TOPIC_GPS
    TOPIC_ACQUIREG,      //!< TOPIC_ACQUIREG
    TOPIC_CONTROL,       //!< TOPIC_CONTROL
    TOPIC_GUI,           //!< TOPIC_GUI
    TOPIC_RECORDS,       //!< TOPIC_RECORDS
    TOPIC_SIMULATOR,     //!< TOPIC_SIMULATOR
    TOPIC_LAST,          //!< TOPIC_LAST
    TOPIC_ALL,           //!< TOPIC_ALL
} topic_e;

/**
 * This Typedef defines the Application Error codes
 */
typedef enum eAPPError_s
{
    APP_ERROR_SUCCESS                   = 0,    //!< APP SUCCESS
    APP_ERROR_ERROR                     = 1000, //!< APP ERROR
    APP_ERROR_BROKER_LIST_NOT_FOUND           , //!< Linked List not found
    APP_ERROR_BROKER_NOT_ENOUGH_MEMORY        , //!< Malloc error when addding subscriber
    APP_ERROR_BROKER_INVALID_TOPIC            , //!< Invalid Topic
    APP_ERROR_BROKER_ALREADY_IN_LIST          , //!< Subscriber already in list
} eAPPError_s;

/**
 * The typedef defines a Watchdog function returning the module flags
 * @param bNumberOfThreads Number of threads inside a given module
 * @return pointer to a volatile array of bytes holding each thread watchdog flag
 */
typedef volatile uint8_t * (*WDTFunction)(uint8_t * bNumberOfThreads);

/**
 * This Typedef defines the valid Watchdog flag states.
 */
typedef enum WDTStatus_e
{
    WDT_UNKNOWN         = 0,//!< UNKNOWN
    WDT_SLEEP           = 1,//!< SLEEP
    WDT_ACTIVE          = 2,//!< ACTIVE
} WDTStatus_e;

typedef enum {
	DATA_MASK_CONFIGURATION = 0x5000,
	DATA_MASK_INSTALLATION,
	DATA_MASK_PLANTER,
	DATA_MASK_TEST_MODE,
	DATA_MASK_TRIMMING,
	DATA_MASK_SYSTEM,
	DATA_MASK_INVALID
} eDataMask;

/******************************************************************************
* Publish structures
*******************************************************************************/
typedef enum {
    EVENT_SET,
    EVENT_CLEAR,
    EVENT_UPDATE,
} eEventType;

typedef struct {
    uint32_t dEvent;
    eEventType eEvtType;
    void* vPayload;
} PubMessage;

typedef enum event_e
{
    EVENT_FFS_STATUS,        				//!< EVENT FFS STATUS CHANGED
	EVENT_FFS_CFG,        					//!< EVENT FILE CFG STATUS CHANGED
	EVENT_FFS_INTERFACE_CFG,    			//!< EVENT FILE INTERFACE STATUS CHANGED
	EVENT_FFS_STATIC_REG,    				//!< EVENT FILE INTERFACE STATUS CHANGED
	EVENT_GUI_UPDATE_PLANTER_INTERFACE,
	EVENT_GUI_UPDATE_INSTALLATION_INTERFACE,
	EVENT_GUI_UPDATE_TEST_MODE_INTERFACE,
	EVENT_GUI_UPDATE_TRIMMING_INTERFACE,
	EVENT_GUI_UPDATE_SYSTEM_INTERFACE,
	EVENT_ISO_UPDATE_CURRENT_DATA_MASK,
	EVENT_ISO_UPDATE_CURRENT_CONFIGURATION,
	EVENT_AQR_FINISH_INSTALLATION,
	EVENT_AQR_UPDATE_INSTALLATION,
} event_e;

/******************************************************************************
* Conversion from MPA
*******************************************************************************/
// TODO: This variables is just for test
// TODO: common from GPS



extern gpio_config_s sEnablePS9;
#define ENABLE_PS9 GPIO_vClear(&sEnablePS9)     // Enable sensor power source
#define DISABLE_PS9 GPIO_vSet(&sEnablePS9)      // Disable sensor power source

/* CAN sensor core */
#define CAN_ALTA_PRIOR            0x00

// Value of DLC of control field
#define CAN_MSG_DLC_0             0x00
#define CAN_MSG_DLC_1             0x01
#define CAN_MSG_DLC_2             0x02
#define CAN_MSG_DLC_3             0x03
#define CAN_MSG_DLC_4             0x04
#define CAN_MSG_DLC_5             0x05
#define CAN_MSG_DLC_6             0x06
#define CAN_MSG_DLC_7             0x07
#define CAN_MSG_DLC_8             0x08

#define CAN_ENL_FLAG_NENHUM       0x00000000
#define CAN_ENL_FLAG_RX_MENSAGEM  0x00000001

#define CAN_ENL_QTDE_BYTES_MSG        8

// TODO: Resolve this dependencies
// Listagem de comandos utilizados na rede CAN
#define CAN_APL_CMD_PNP                           0x01 //Comando de PnP
#define CAN_APL_CMD_LEITURA_DADOS                 0x02 //Comando de leitura de dados
#define CAN_APL_CMD_CONFIGURA_SENSOR              0x03 //Comando de configuracao do sensor
#define CAN_APL_RESP_PNP                          0x04 //Resposta ao comando de PnP
#define CAN_APL_RESP_LEITURA_DADOS                0x05 //Resposta ao comando de leitura de dados
#define CAN_APL_LEITURA_VELOCIDADE                0x07 //Leitura de dados de velocidade
#define CAN_APL_RESP_CONFIGURA_SENSOR             0x06 //Resposta ao comando de configuracao do sensor

#define CAN_APL_CMD_PARAMETROS_SENSOR             0x08 //Comando de envio de parametros de sensor
#define CAN_APL_RESP_PARAMETROS_SENSOR            0x09

#define CAN_APL_CMD_VERSAO_SW_SENSOR              0x0A //Comando de envio de Versao de Firmware de sensor
#define CAN_APL_RESP_VERSAO_SW_SENSOR             0x0B

#define CAN_APL_CMD_PARAMETROS_EXTENDED           0x0C //Comando de envio de parametros de sensor (Com algori�timo para correcaoo de duplos)
#define CAN_APL_RESP_PARAMETROS_EXTENDED          0x0D

// Listagem de flags da aplicacao CAN
#define CAN_APL_FLAG_NENHUM                       0x00000000
#define CAN_APL_FLAG_TODOS                        0xFFFFFFFF

#define CAN_APL_FLAG_TMR_CMD_PNP                  0x00000001

#define CAN_APL_FLAG_MSG_RESP_PNP                 0x00000002
#define CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS       0x00000004
#define CAN_APL_FLAG_MSG_RESP_CONFIGURACAO        0x00000008

#define CAN_APL_FLAG_MSG_LEITURA_VELOCIDADE       0x00000010

#define CAN_APL_FLAG_MSG_RESP_PARAMETROS          0x00000020
#define CAN_APL_FLAG_MSG_RESP_VERSAO_SW_SENSOR    0x00000040

#define CAN_APL_FLAG_PNP_TIMEOUT                  0x00000080
#define CAN_APL_FLAG_COMANDO_CONFIGURA            0x00000100
#define CAN_APL_FLAG_COMANDO_TIMEOUT              0x00000200
#define CAN_APL_FLAG_TODOS_SENS_RESP_PNP          0x00000400
#define CAN_APL_FLAG_MENSAGEM_LIDA                0x00000800

//**** A tarefa de aquisicao de dados aguarda algum destes flags serem setados

// Eventos relacionados a� detecao de sensores
#define CAN_APL_FLAG_DET_NOVO_SENSOR              0x00001000
#define CAN_APL_FLAG_DET_SENSOR_RECONECTADO       0x00002000

// Respostas ao comando de leitura de dados
#define CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP    0x00004000
#define CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP   0x00008000
#define CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP    0x00010000

// Respostas ao comando de configuracao
#define CAN_APL_FLAG_CFG_SENSOR_RESPONDEU         0x00020000

// Nenhum sensor conectado a rede Resposta comum a todos os comandos
#define CAN_APL_FLAG_SENSOR_NAO_RESPONDEU         0x00040000
#define CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO      0x00080000


/******************************************************************************
* Typedefs from Control module... Just for test...
*******************************************************************************/
typedef enum {
  UOS_eTARQ_REG_OP = 0,
  UOS_eTARQ_CDC,
  UOS_eTARQ_REG_GRAF,
  UOS_eTARQ_FORCA_WORD = 0xFFFF
} UOS_eTipoArquivo;

typedef enum {
  Linhas_Pares,     //Linhas Pares levantadas
  Linhas_Impares,   //Linhas impares levantadas
  Sem_Intercalacao  //Nenhuma linha levantada

} UOS_teIntercala;

//Estrutura de identificacao da versao deste software:
typedef struct {
  uint16_t            wFlag;                  //Flag de ini�cio. Sempre 0xFFFF.
  uint16_t            wModelo;                //Numero do modelo.

  uint16_t            wVer;                   //Numero da versao.
  uint16_t            wRev;                   //Numero da revisao.

  uint16_t            wBuild;                 //Numero do build.
  uint8_t             abNumSerie[6];          //Numero de serie do hardware.
  UOS_eTipoArquivo  eTipoArq;               //Tipo de arquivo.
  uint16_t            awOp[5];                //Opcionais.
  uint32_t            dOffsetRegs;            //Offser dos registros operacionais:
} UOS_tsVersaoCod;

//Estrutura da configuracao operacional:
//Estas informacoes são configuradas pelo usuário na IHM
//é armazenada no sistema de arquivo e deve ser carregada durante o procedimento
//de inicialização ou sempre que o conteúdo do arquivo recipiente for alterado:

//Configuracao do Monitor
typedef struct {

  float   fLimVel;               //Limite de velocidade km/h.

  uint16_t wSementesPorMetro;     //Meta de Sementes por Metro. (sementes/m)*10
  uint16_t wInsensibilidade;      //Distancia de insensibilidade para falhas. (metros)*10

  uint16_t wAvalia;               //Distancia para avaliacao de aplicacao. (metros)*10
  uint16_t wDistLinhas;           //Distancia entre linhas. (centímetros)*10

  uint16_t wLargImpl;             //Largura do implemento. (centi�metros)*10
  uint8_t  bMonitorArea;          //Se estao em modo monitor de Area. (0 = false, 1 = true )
  uint8_t  bNumLinhas;            //No. Linhas (1-36)

  uint8_t  bDivLinhas;            //Divisao da plantadeira (bNumLinhas/2 ou bNumLinhas/2+1)
  uint8_t  bSensorAdubo;          //Indica presenaa de sensor de adubo. (bSensorAdubo = 1)
  uint8_t  bTolerancia;           //Tolerancia ao espaçamento entre sementes. (porcentagem)
  uint8_t  bTeclaPausaHab;             //Habilita o uso da tecla de pausa. (bTeclaPausaHab = 1)

  UOS_teIntercala  eIntercala;
  uint8_t  bPausaAuto;            //Habilita o uso da pausa automatica.(bPausaAuto = 1)
  uint8_t  bLinhasFalhaPausaAuto;      //Numero de linhas em falha para pausa automatica. (1-32)
  uint8_t  bNumSensorAdicionais;  //Numero de sensores adicionais (0-6)

} UOS_tsCfgMonitor;

//Receptor GPS interno:
typedef struct {
  int32_t        lFusoHorario;

  uint16_t      wDistanciaEntreFixos;
  uint16_t      wAnguloEntreFixos;

  uint8_t       bHorarioVerao;       //Indica se estao em horario de verao (bHorarioVerao = 1)
  uint8_t       bSalvaRegistro;      //Indica se gravacao de registros esta ativada
} tsCfgGPS;

//Identificacao do vei�culo:
typedef struct {
  uint8_t       abCodigo[6];
} tsCfgVeiculo;

typedef struct {
  //Configuracao Monitor
  UOS_tsCfgMonitor              sMonitor;

  //Receptor GPS interno:
  tsCfgGPS                      sGPS;

  //Identificacao do vei�culo:
  tsCfgVeiculo                  sVeiculo;

  //CRC16 desta estrutura:
  uint16_t                        wCRC16;

} UOS_tsConfiguracao;

/******************************************************************************
* Variables from Control module... Just for test...
*******************************************************************************/
#define UOS_SIS_FLAG_NENHUM             0x00000000
#define UOS_SIS_FLAG_TODOS              0xFFFFFFFF
#define UOS_SIS_FLAG_BOOT               0x00000001
#define UOS_SIS_FLAG_FFS_OK             0x00000002
#define UOS_SIS_FLAG_MEM_OK             0x00000004
#define UOS_SIS_FLAG_CFG_OK             0x00000008
#define UOS_SIS_FLAG_AQR_OK             0x00000010
#define UOS_SIS_FLAG_BTH_OK             0x00000020
#define UOS_SIS_FLAG_SIS_OK             0x00000040
#define UOS_SIS_FLAG_REINICIO           0x00000080
#define UOS_SIS_FLAG_COMUNICA           0x00000100
#define UOS_SIS_FLAG_NOVO_REG           0x00000200
#define UOS_SIS_FLAG_REGISTRO           0x00000400
#define UOS_SIS_FLAG_EMERGENCIA         0x00000800
#define UOS_SIS_FLAG_ALARME             0x00001000
#define UOS_SIS_FLAG_CONECTADO          0x00002000
#define UOS_SIS_FLAG_IHM                0x00004000
#define UOS_SIS_FLAG_MODO_TRABALHO      0x00008000
#define UOS_SIS_FLAG_CONFIRMA_INST      0x00010000
#define UOS_SIS_FLAG_MODO_TESTE         0x00020000
#define UOS_SIS_FLAG_VERIFICANDO        0x00040000
#define UOS_SIS_FLAG_PARAMETROS_OK      0x00080000
#define UOS_SIS_FLAG_VERSAO_SW_OK       0x00100000
#define UOS_SIS_FLAG_ALARME_TOLERANCIA  0x00200000
#define UOS_SIS_FLAG_ERRO_INST_SENSOR   0x00400000

#define UOS_SIS_FLAG_SEND_FLAG_STATUS   0x00800000

/******************************************************************************
* Typedefs from Acquireg module...
*******************************************************************************/
//Mascaras para os flags
#define AQR_FLAG_NENHUM           0x00000000
#define AQR_FLAG_TODOS            0xFFFFFFFF
#define AQR_FLAG_RECONHECE_ALARME 0x00000001
#define AQR_FLAG_ZERA_PARCIAIS    0x00000002
#define AQR_FLAG_ZERA_TOTAIS      0x00000004
#define AQR_FLAG_PAUSA            0x00000008
#define AQR_FLAG_AUTO_TESTE       0x00000010
#define AQR_FLAG_NOVO_SENSOR      0x00000020
#define AQR_FLAG_TROCA_SENSOR     0x00000040
#define AQR_FLAG_NOVO_DADO        0x00000080
#define AQR_FLAG_USO              0x00000100
#define AQR_FLAG_RESET_DESLIGA    0x00000400
#define AQR_FLAG_DESLIGA          0x00000800
#define AQR_FLAG_IMP_PARADO       0x00001000
#define AQR_FLAG_ESTATICO_REG     0x00002000

//Mascaras para identificar eventos e gerar Alarmes e registro
#define AQR_NENHUM                  0x0000 // Nenhum alarme acionado
#define AQR_PAUSA                   0x0001 //Monitor em pausa
#define AQR_EXC_VELOCIDADE          0x0002 //Excesso de Velocidade
#define AQR_FALHA_LINHA             0x0004 //Falha em alguma Linha
#define AQR_SENSOR_DESCONECTADO     0x0008 //Sensor desconectado
#define AQR_INSENSIVEL              0x0010 //
#define AQR_FALHA_INSTALACAO        0x0020 //Falha na Instalacao
#define AQR_NOVO_SENSOR             0x0040 //Novo Sensor Instalado
#define AQR_FALHA_TOLERANCIA_LINHA  0x0080 //Falha na tolerancia da linha
#define AQR_FALHA_GPS               0x0100 //Falha GPS

/******************************************************************************
* Events List
*******************************************************************************/



//------------------------------------------------------------------------------
//Causas de fim de registro:

#define AQR_wCF_DESCONHECIDO      0x0000 //Causa desconhecida.
#define AQR_wCF_LEITURA           0x0101 //Leitura de registros.
#define AQR_wCF_ZERA_PARCIAL      0x0102 //Zera Acumulados Parciais
#define AQR_wCF_ZERA_TOTAL        0x0103 //Zera Acumulados Totais
#define AQR_wCF_FALHA_ENERGIA     0x0201 //Falha de energia.
#define AQR_wCF_CONFIGURACAO      0x0202 //Modificacao de configuracao.
#define AQR_wCF_CONFIGURA_GPS     0x0203 //Modificacao de configuracao REGISTROS
#define AQR_wCF_WATCHDOGTIMER     0x0204 //Voltando de um Watchdog timer.
#define AQR_wCF_RESET_EXTERNO     0x0205 //Reset externo, Watchdog timer nao foi executado.
#define AQR_wCF_REINICIO_IHM      0x0301 //Reinicio forcado pela IHM
#define AQR_wCF_REINICIO_SRL      0x0302 //Reinicio forcado pela interface serial.
#define AQR_wCF_REINICIO_BTH      0x0303 //Reinicio forcado pela interface Bluetooth.
#define AQR_wCF_REINICIO_GPS      0x0304 //Reinicio forcado pela interface GPS
#define AQR_wCF_AUTO_DESLIGA      0x0401 //Desliga apos um tempo sem utilização
#define AQR_wCF_LIBERA_MEM        0x0402 //Liberacao de memeria.

typedef enum {
  Sem_Arremate,  //Nenhum lado levantado
  Lado_Esquerdo, //Lado esquerdo levantado
  Lado_Direito   //Lado direito levantado
} AQR_teArremate;

typedef enum {
  AguardandoEvento,    //Nenhum novo evento
  Instalado,           //Novo sensor instalado
  FalhaAutoTeste,      //Falha no Auto-Teste
  SensorNaoEsperado    //Sensor nao esperado no bus ( e� mais que o numero de linhas )
} AQR_teInstalacao;

typedef struct {

  uint8_t   bPausaAuto;           //Monitor esta em Pausa Automatica
  uint8_t   bExVel;               //Excesso de velocidade
  uint8_t   bVelZero;             //Velocidade Zero
  uint8_t   bTrabalhando;         //Estao Trabalhando

  uint8_t   bInsensivel;          //Estao area de insensibilidade
  uint8_t   bImplemento;          //Implemento Levantado
  uint8_t   bErroGPS;             //Erro no GPS

  AQR_teInstalacao   eStatusInstalacao; //Indica se o sensor foi instalado ou se houve falha na instalacao
  uint8_t   bSensorDesconectado;          //Indica que algum sensor esta desconectado
  uint8_t   bLinhasFalha;                 //Indica se ha Linhas em falha >90% e < 20%
  uint8_t   bLinhasZero;                  //Indica se ha Linhas em falha < 90%
  uint8_t   bSensorNaoEsperado;           //Indica se foi conectado sensor alem do configurado

  uint8_t   bSementeInstalados;   //Quantidade de sensores de semente instalados ate o momento
  uint8_t   bAduboInstalados;     //Quantidade de sensores de adubo instalados ate o momento
  uint8_t   bAdicionalInstalados; //Quantidade de sensores adicionais instalados ate o momento
  uint8_t   bReprovados;          //Quantidade de sensores reprovados no Auto-Teste

  uint8_t   bAdicionalDesconectado;    //Sensores Adicionais desconectados
  uint8_t   bMemAdicionalDesconectado; //Memoriza Sensores adicionais desconectados
  //uint8_t   bLinhaSementeIgnorado;     //Numero da linha que o sensor de semente esta sendo ignorado
  //uint8_t   bLinhaAduboIgnorado;       //Numero da linha que o sensor de adubo esta sendo ignorado

  uint8_t   bAutoTeste;             //Informa se esta em auto-teste
  uint8_t   bAlarmeOK;              //Informa se o alarme foi reconhecido
  uint8_t   bAlarmeGPS;              //Informa o alarme do GPS
  AQR_teArremate   eArremate;     //Informa se esta ou nao esta em Arremate
  uint8_t   bNumLinhasSemIntercalar;
//  AQR_teIntercala  eIntercala;

  uint8_t   bNumLinhasZero;            //Quantidade de linhas com Zero sementes
  uint8_t   bNumLinhasAtivas;          //Quantidade de linhas que nao estao levantadas (em arremate)
  uint8_t   bNumLinhasDir;             //Quantidade de linhas que estao ativas do lado direito
  uint8_t   bNumLinhasEsq;             //Quantidade de linhas que estao ativas do lado esquerdo

  uint8_t   bNumSensores;         //Nao total de sensores configurados pelo usuario
  uint8_t   bLinhaDisponivel;     //Informa qual linha estao disponível para instalação do novo sensor
  uint8_t   bSensorAdicionado;    //Informa qual Sensor foi adicionado
  uint8_t   bCfgSensorRespondeu;    //Informa se o sensor respondeu a configuracao

  uint32_t  dSementeFalha;          //Linha com Sensores de Semente em Falha
  uint32_t  dSementeFalhaExt;       //Linha com Sensores de Semente em Falha Extendida
  uint32_t  dSementeFalhaIHM;       //Flags que indica na IHM as falhas de semente
  uint32_t  dSementeFalhaIHMExt;    //Flags que indica na IHM as falhas de semente Extendida
  uint32_t  dAduboFalha;            //Linha com Sensores de Adubo em Falha
  uint32_t  dAduboFalhaExt;         //Linha com Sensores de Adubo em Falha Extendida
  uint32_t  dLinhaDesconectada;     //Linhas com sensor desconectado
  uint32_t  dLinhaDesconectadaExt;  //Linhas com sensor desconectado Extendida
  uint32_t  dMemLinhaDesconectada;  //Memoriza Linhas com sensor desconectado
  uint32_t  dMemLinhaDesconectadaExt;
  uint32_t  dLinhasLevantadas;      //(Linhas levantadas = 1, Linhas ativas = 0)
  uint32_t  dLinhasLevantadasExt;
  uint32_t  dSementeIgnorado;       //Linha com sensor de Semente Ignorado;
  uint32_t  dSementeIgnoradoExt;    //Extendido
  uint32_t  dAduboIgnorado;         //Linha com sensor de Adubo Ignorado;
  uint32_t  dAduboIgnoradoExt;      //Extendido
  uint32_t  dSementeZero;           //Flags que indica falha de semente proximo de zero
  uint32_t  dSementeZeroExt;        //Extendido
  uint32_t  dSementeZeroIHM;        //Flags que indica falha de semente proximo de zero para IHM
  uint32_t  dSementeZeroIHMExt;     //Extendido
  uint16_t  wMaxSementes;           //Tolerancia maxima de sementes
  uint16_t  wMinSementes;           //Tolerancia maxima de sementes

  uint16_t  wMinSementesZero;      //Tolerância mi�nima de sementes para considerar que nao estao caindo sementes

  uint16_t  awMediaSementes[36];    //Media de sementes plantadas por metro * 100 (com duas casas decimais)
  uint16_t  wAvaliaArred;

  uint8_t   bPrimeiroSegmento;
  uint8_t   bAlarmeLinhasLevantadas; //Contador de linhas de falha de linhas levantada


} tsStatus;

typedef struct{
  uint16_t  awBufDis;
  uint8_t   abBufSem[ CAN_bNUM_DE_LINHAS ];
}tsFalhaInstantanea;

typedef struct
{
	//Senha para operacoes de seguranca:
	uint8_t       abSenha[4];
	//Contraste do LCD:
	uint8_t        bContraste;
	//Brilho do LCD:
	uint8_t       bBrilho;
	//Idioma:
	uint8_t       bIdioma;

	// Se egit  sistema imperial ou internacional
	uint8_t       bSistImperial;

	// Tipo de medida para velocidade do veiculo
	uint8_t       bVelocidade;
	uint8_t       bTxtVel;
	uint8_t       bTxtVelPorHora;
	// Medida da superficie trabalhada
	uint8_t       bAreaTrabalhada;
	uint8_t       bTxtAreaTrab;
	uint8_t       bTxtAreauint8_torHora;
	// Sementes por cm/pol..
	uint8_t       bSementes;
	uint8_t       bTxtSementes;
	uint8_t       bTxtSemPorDist;
	uint8_t       bImgSemPorDist;
	// distancia em km/mi..
	uint8_t       bDistPerc;
	uint8_t       bTxtDistPerc;
	// caractere usado para fracao e milhar:
	uint8_t       bCharFrac;
	uint8_t       bCharMilhar;

	// CRC da estrutura
	uint16_t      wCRC16;
}
IHM_tsConfig;

//Estrutura do registro estático:
typedef struct {

  uint16_t          wFlagInicio;           //Flag de inicio do registro (0xFFFF)

  uint8_t           abDataHoraIni[6];      //Data/hora de inicio (SMHDMA).
  uint8_t           abDataHoraFim[6];      //Data/hora de fim (SMHDMA).

  tsVelocidade    sVelocidade;           //Valores relativos à excesso de velocidade

  tsLinhas        sTrabParcial;          //Acumulados Parcial Trabalhando
  tsLinhas        sTrabParcDir;          //Acumulados Parcial Trabalhando
  tsLinhas        sTrabParcEsq;          //Acumulados Parcial Trabalhando

} AQR_tsRegEstatico;

//Estrutura acima com CRC:
typedef struct {

  uint32_t                dEmergencia;     //Indica que a tarefa de emergência executou normalmente
  tsLinhas              sTrabTotal;       //Acumulados Total Trabalhando
  tsLinhas              sTrabTotalDir;    //Acumulados Total Trabalhando
  tsLinhas              sTrabTotalEsq;    //Acumulados Total Trabalhando

  AQR_tsRegEstatico     sReg;            //Estrutura de um registro estático.
  uint32_t                dDataHora;       //Data/hora da última atualização desta estrutura.
  uint32_t                dOffsetRegDat;   //Offset do último registro estático dentro do REGISTRO.DAT

  uint16_t                awMediaSementes[36];      //Última média calculada para cada linha

  //Declarei esta variável com 32 bits para evitar problemas de alinhamento
  //no momento do cálculo do CRC.
  uint32_t                wCRC16;         //CRC desta estrutura.

} AQR_tsRegEstaticoCRC;

#define LCD_bBRILHO_MAX        99

//Ajuste do Contraste
#define LCD_bCONTRASTE_MAX    127

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMMON_COMMON_APP_H_ */
