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
#define PUT_LOCAL_QUEUE(QueueName, message, time) (osMessagePutValue(QueueName, (void*)&message, time))
/*!< Receive from Queue */
#define RECEIVE_LOCAL_QUEUE(fromQueue, buffer, time) (osMessageGetValue(fromQueue, buffer, time))

/*
#define GET_PUBLISHED_EVENT(contract)   ((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->dEvent
#define GET_PUBLISHED_TYPE(contract) 	((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->eEvtType
#define GET_PUBLISHED_PAYLOAD(contract) ((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->vPayload
*/

#define GET_PUBLISHED_EVENT(contract)  	((smsg_key)(GET_MESSAGE(contract)->hMessageKey)).wEvent
#define GET_PUBLISHED_TYPE(contract)  	(eEventType)((smsg_key)GET_MESSAGE(contract)->hMessageKey).wType
#define GET_PUBLISHED_PAYLOAD(contract) GET_MESSAGE(contract)->pvMessage

#define PUBLISH_MESSAGE(contract, event, type, payload) \
{ \
  ((smsg_key*)(&MESSAGE(contract).hMessageKey))->wEvent = event; \
  ((smsg_key*)(&MESSAGE(contract).hMessageKey))->wType = type; \
  MESSAGE(contract).pvMessage = (void*)payload; \
  PUBLISH(CONTRACT(contract), 0); \
}

#define MESSAGE_HEADER(name, Size, Key, Type) \
{ \
  MESSAGE(name).hMessageSize = Size; \
  MESSAGE(name).hMessageKey = Key; \
  MESSAGE(name).eMessageType = Type; \
}


// Converter Macros
#define MM2IN(value) 	((value) * 0.0393701f)
#define IN2MM(value) 	((uint16_t)(value * 25.41f))
#define DM2IN(value) 	((value) * 3.93701f)
#define IN2DM(value) 	((uint16_t)(value * 0.2541f))

#define DM2FT(value) 	((value) * 3.28084f)
#define FT2DM(value) 	((value) / 3.28084f)

#define KMH2MLH(value) 	(value * 0.621371f)
#define MLH2KMH(value) 	(value * 1.60934f)

#define SDM2SP(value) 	((value) / 3.28084f)
#define SP2SDM(value) 	((uint16_t)(value * 3.28084f))

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 *  This Typedef defines the valid Origins to be used into contracts
 */
typedef enum origin_e
{
	MODULE_BROKER = 1, //!< MODULE_BROKER
	MODULE_DIAGNOSTIC, //!< MODULE_DIAGNOSTIC
	MODULE_UPDATE, //!< MODULE_UPDATE
	MODULE_AUTHENTICATION, //!< MODULE_AUTHENTICATION
	MODULE_COMM_CBA, //!< MODULE_COMM
	MODULE_ACTUATOR, //!< MODULE_ACTUATOR_CORE
	MODULE_ACTUATOR_LCD, //!< MODULE_ACTUATOR_LCD
	MODULE_ACTUATOR_KEY, //!< MODULE_ACTUATOR_KEY
	MODULE_OUTPUT_LCD, //!< MODULE_OUTPUT
	MODULE_INPUT_KEYPAD, //!< MODULE_INPUT
	MODULE_SENSOR, //!< MODULE_SENSOR
	MODULE_SENSOR_LUMINOSITY, //!< MODULE_SENSOR
	MODULE_SENSOR_COMPASS, //!< MODULE_SENSOR
	MODULE_ISOBUS, //!< MODULE_ISOBUS
	MODULE_BUZZER, //!< MODULE_BUZZER
	MODULE_FILESYS, //!< MODULE_FILESYS
	MODULE_GPS, //!< MODULE_GPS
	MODULE_ACQUIREG, //!< MODULE_ACQUIREG
	MODULE_CONTROL, //!< MODULE_CONTROL
	MODULE_GUI, //!< MODULE_GUI
	MODULE_RECORDS, //!< MODULE_RECORDS
	MODULE_SIMULATOR, //!< MODULE_SIMULATOR
	MODULE_TEST, //!< MODULE_TEST
	MODULE_INVALID, //!< INVALID MODULE
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
	TOPIC_SEN_STATUS,        //!< TOPIC_SENSOR
	TOPIC_INPUT,         //!< TOPIC_INPUT
	TOPIC_OUTPUT,        //!< TOPIC_OUTPUT
	TOPIC_ACTUATOR,      //!< TOPIC_ACTUATOR
	TOPIC_COMM,          //!< TOPIC_COMM
	TOPIC_AUTHENTICATION,          //!< TOPIC_AUTHENTICATION
	TOPIC_DIAGNOSTIC,    //!< TOPIC_DIAGNOSTIC
	TOPIC_UPDATE,        //!< TOPIC_UPDATE
	TOPIC_MEMORY,        //!< TOPIC_MEMORY
	TOPIC_ISOBUS,		 //!< TOPIC_ISOBUS
	TOPIC_BUZZER,        //!< TOPIC_BUZZER
	TOPIC_FILESYS,       //!< TOPIC_FILESYS
	TOPIC_GPS,           //!< TOPIC_GPS
	TOPIC_GPS_METRO,           //!< TOPIC_GPS
	TOPIC_GPS_STATUS,
	TOPIC_ACQUIREG,      //!< TOPIC_ACQUIREG
	TOPIC_ACQUIREG_SAVE,      //!< TOPIC_ACQUIREG
	TOPIC_CONTROL,       //!< TOPIC_CONTROL
	TOPIC_GUI,           //!< TOPIC_GUI
	TOPIC_GUI_AQR,           //!< TOPIC_GUI
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
	APP_ERROR_SUCCESS = 0,    //!< APP SUCCESS
	APP_ERROR_ERROR = 1000, //!< APP ERROR
	APP_ERROR_BROKER_LIST_NOT_FOUND, //!< Linked List not found
	APP_ERROR_BROKER_NOT_ENOUGH_MEMORY, //!< Malloc error when addding subscriber
	APP_ERROR_BROKER_INVALID_TOPIC, //!< Invalid Topic
	APP_ERROR_BROKER_ALREADY_IN_LIST, //!< Subscriber already in list
} eAPPError_s;

/**
 * The typedef defines a Watchdog function returning the module flags
 * @param bNumberOfThreads Number of threads inside a given module
 * @return pointer to a volatile array of bytes holding each thread watchdog flag
 */
typedef volatile uint8_t * (*WDTFunction) (uint8_t * bNumberOfThreads);

/**
 * This Typedef defines the valid Watchdog flag states.
 */
typedef enum WDTStatus_e
{
	WDT_UNKNOWN = 0, //!< UNKNOWN
	WDT_SLEEP = 1, //!< SLEEP
	WDT_ACTIVE = 2, //!< ACTIVE
} WDTStatus_e;

/******************************************************************************
 * Publish structures
 *******************************************************************************/
typedef enum
{
	EVENT_SET,
	EVENT_CLEAR,
	EVENT_UPDATE,
} eEventType;

typedef enum event_e
{
	EVENT_NONE,
	EVENT_FFS_STATUS,
	EVENT_FFS_CFG,
	EVENT_FFS_SENSOR_CFG,
	EVENT_FFS_STATIC_REG,
	EVENT_FFS_CONFIG_GET_MEMORY_USED,
	EVENT_FFS_CONFIG_GET_MEMORY_USED_RESPONSE,
	EVENT_FFS_FILE_INFO,
	EVENT_FFS_FILE_FORMAT_DONE,
	EVENT_AQR_INSTALLATION_FINISH_INSTALLATION,
	EVENT_AQR_INSTALLATION_UPDATE_INSTALLATION,
	EVENT_AQR_INSTALLATION_CONFIRM_INSTALLATION,
	EVENT_AQR_INSTALLATION_SENSOR_REPLACE,
	EVENT_AQR_INSTALLATION_ENABLE_SENSOR_PNP,
	EVENT_AQR_UPDATE_PLANTER_MASK,
	EVENT_AQR_ALARM_NEW_SENSOR,
	EVENT_AQR_ALARM_DISCONNECTED_SENSOR,
	EVENT_AQR_ALARM_LINE_FAILURE,
	EVENT_AQR_ALARM_SETUP_FAILURE,
	EVENT_AQR_ALARM_EXCEEDED_SPEED,
	EVENT_AQR_ALARM_GPS_FAILURE,
	EVENT_AQR_ALARM_TOLERANCE,
	EVENT_GPS_UPDATE_GPS_STATUS,
	EVENT_GPS_METER_TRAVELED,
	EVENT_GPS_READ_SENSORS,
	EVENT_GPS_SECOND_ELAPSED,
	EVENT_GPS_METER_TIMEOUT,
	EVENT_GUI_UPDATE_CONFIGURATION_INTERFACE,
	EVENT_GUI_UPDATE_PLANTER_INTERFACE,
	EVENT_GUI_UPDATE_INSTALLATION_INTERFACE,
	EVENT_GUI_UPDATE_TEST_MODE_INTERFACE,
	EVENT_GUI_UPDATE_TRIMMING_INTERFACE,
	EVENT_GUI_UPDATE_SYSTEM_GPS_INTERFACE,
	EVENT_GUI_UPDATE_SYSTEM_CAN_INTERFACE,
	EVENT_GUI_UPDATE_SYSTEM_SENSORS_INTERFACE,
	EVENT_GUI_UPDATE_REPLACE_SENSOR,
	EVENT_GUI_INSTALLATION_FINISH,
	EVENT_GUI_INSTALLATION_REPEAT_TEST,
	EVENT_GUI_INSTALLATION_ERASE_INSTALLATION,
	EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION,
	EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION_ACK,
	EVENT_GUI_INSTALLATION_REPLACE_SENSOR,
	EVENT_GUI_INSTALLATION_CONFIRM_REPLACE_SENSOR,
	EVENT_GUI_INSTALLATION_CANCEL_REPLACE_SENSOR,
	EVENT_GUI_PLANTER_CLEAR_COUNTER_TOTAL,
	EVENT_GUI_PLANTER_CLEAR_COUNTER_SUBTOTAL,
	EVENT_GUI_PLANTER_IGNORE_SENSOR,
	EVENT_GUI_CHANGE_ACTIVE_MASK_CONFIG_MASK,
	EVENT_GUI_TRIMMING_TRIMMING_MODE_CHANGE,
	EVENT_GUI_UPDATE_CONFIG,
	EVENT_GUI_UPDATE_SYS_CONFIG,
	EVENT_GUI_AREA_MONITOR_PAUSE,
	EVENT_GUI_ALARM_NEW_SENSOR,
	EVENT_GUI_ALARM_DISCONNECTED_SENSOR,
	EVENT_GUI_ALARM_LINE_FAILURE,
	EVENT_GUI_ALARM_SETUP_FAILURE,
	EVENT_GUI_ALARM_EXCEEDED_SPEED,
	EVENT_GUI_ALARM_GPS_FAILURE,
	EVENT_GUI_ALARM_TOLERANCE,
	EVENT_GUI_CONFIG_CHECK_PASSWORD_ACK,
	EVENT_GUI_CONFIG_CHECK_PASSWORD_NACK,
	EVENT_GUI_CONFIG_CHANGE_PASSWORD_ACK,
	EVENT_GUI_CONFIG_CHANGE_PASSWORD_NACK,
	EVENT_GUI_CONFIG_GET_MEMORY_USED,
	EVENT_GUI_CONFIG_GET_MEMORY_USED_RESPONSE,
	EVENT_GUI_GET_FILE_INFO,
	EVENT_GUI_FORMAT_FILE,
	EVENT_ISO_UPDATE_CURRENT_DATA_MASK,
	EVENT_ISO_UPDATE_CURRENT_CONFIGURATION,
	EVENT_ISO_INSTALLATION_REPEAT_TEST,
	EVENT_ISO_INSTALLATION_ERASE_INSTALLATION,
	EVENT_ISO_INSTALLATION_CONFIRM_INSTALLATION,
	EVENT_ISO_INSTALLATION_REPLACE_SENSOR,
	EVENT_ISO_INSTALLATION_CONFIRM_REPLACE_SENSOR,
	EVENT_ISO_INSTALLATION_CANCEL_REPLACE_SENSOR,
	EVENT_ISO_PLANTER_CLEAR_COUNTER_TOTAL,
	EVENT_ISO_PLANTER_CLEAR_COUNTER_SUBTOTAL,
	EVENT_ISO_PLANTER_IGNORE_SENSOR,
	EVENT_ISO_TRIMMING_TRIMMING_MODE_CHANGE,
	EVENT_ISO_CONFIG_UPDATE_DATA,
	EVENT_ISO_CONFIG_CANCEL_UPDATE_DATA,
	EVENT_ISO_CONFIG_CHECK_PASSWORD,
	EVENT_ISO_CONFIG_CHANGE_PASSWORD,
	EVENT_ISO_CONFIG_GET_MEMORY_USED,
	EVENT_ISO_LANGUAGE_COMMAND,
	EVENT_ISO_AREA_MONITOR_PAUSE,
	EVENT_ISO_ALARM_CLEAR_ALARM,
	EVENT_CTL_UPDATE_CONFIG,
	EVENT_CTL_UPDATE_FILE_INFO,
	EVENT_CTL_GET_FILE_INFO,
	EVENT_CTL_FILE_FORMAT,
	EVENT_CTL_FILE_FORMAT_DONE,
	EVENT_CTL_FILE_FORMAT_STATUS,
	EVENT_SEN_PUBLISH_FLAG,
	EVENT_SEN_CAN_STATUS,
	EVENT_SEN_SYNC_READ_SENSORS,
} event_e;

typedef struct
{
	uint32_t dEvent;
	eEventType eEvtType;
	void* vPayload;
} PubMessage;

/******************************************************************************
 * Conversion from MPA
 *******************************************************************************/
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
#define CAN_APL_FLAG_CAN_STATUS				        0x00100000
#define CAN_APL_FLAG_SYNC_READ_SENSOR		        0x00200000

/******************************************************************************
 * Typedefs from Control module... Just for test...
 *******************************************************************************/
typedef enum
{
	UOS_eTARQ_REG_OP = 0,
	UOS_eTARQ_CDC,
	UOS_eTARQ_REG_GRAF,
	UOS_eTARQ_FORCA_WORD = 0xFFFF
} UOS_eTipoArquivo;

typedef enum
{
	Linhas_Pares,     //Linhas Pares levantadas
	Linhas_Impares,   //Linhas impares levantadas
	Sem_Intercalacao  //Nenhuma linha levantada

} UOS_teIntercala;

//Estrutura de identificacao da versao deste software:
typedef struct
{
	uint16_t wFlag;                  //Flag de ini�cio. Sempre 0xFFFF.
	uint16_t wModelo;                //Numero do modelo.

	uint16_t wVer;                   //Numero da versao.
	uint16_t wRev;                   //Numero da revisao.

	uint16_t wBuild;                 //Numero do build.
	uint8_t abNumSerie[6];          //Numero de serie do hardware.
	UOS_eTipoArquivo eTipoArq;               //Tipo de arquivo.
	uint16_t awOp[5];                //Opcionais.
	uint32_t dOffsetRegs;            //Offser dos registros operacionais:
} UOS_tsVersaoCod;

//Estrutura da configuracao operacional:
//Estas informacoes são configuradas pelo usuário na IHM
//é armazenada no sistema de arquivo e deve ser carregada durante o procedimento
//de inicialização ou sempre que o conteúdo do arquivo recipiente for alterado:

//Configuracao do Monitor
typedef struct
{

	float fLimVel;               //Limite de velocidade km/h.

	uint16_t wSementesPorMetro;     //Meta de Sementes por Metro. (sementes/m)*10
	uint16_t wInsensibilidade;      //Distancia de insensibilidade para falhas. (metros)*10

	uint16_t wAvalia;               //Distancia para avaliacao de aplicacao. (metros)*10
	uint16_t wDistLinhas;           //Distancia entre linhas. (centímetros)*10

	uint16_t wLargImpl;             //Largura do implemento. (centi�metros)*10
	uint8_t bMonitorArea;          //Se estao em modo monitor de Area. (0 = false, 1 = true )
	uint8_t bNumLinhas;            //No. Linhas (1-36)

	uint8_t bDivLinhas;            //Divisao da plantadeira (bNumLinhas/2 ou bNumLinhas/2+1)
	uint8_t bSensorAdubo;          //Indica presenaa de sensor de adubo. (bSensorAdubo = 1)
	uint8_t bTolerancia;           //Tolerancia ao espaçamento entre sementes. (porcentagem)
	uint8_t bTeclaPausaHab;             //Habilita o uso da tecla de pausa. (bTeclaPausaHab = 1)

	UOS_teIntercala eIntercala;
	uint8_t bPausaAuto;            //Habilita o uso da pausa automatica.(bPausaAuto = 1)
	uint8_t bLinhasFalhaPausaAuto;      //Numero de linhas em falha para pausa automatica. (1-32)
	uint8_t bNumSensorAdicionais;  //Numero de sensores adicionais (0-6)

} UOS_tsCfgMonitor;

typedef struct
{
	uint32_t wPasswd;
	eSelectedLanguage eLanguage;
	eSelectedUnitMeasurement eUnit;
} tsCfgIHM;

//Receptor GPS interno:
typedef struct
{
	int32_t lFusoHorario;

	uint16_t wDistanciaEntreFixos;
	uint16_t wAnguloEntreFixos;

	uint8_t bHorarioVerao;       //Indica se estao em horario de verao (bHorarioVerao = 1)
	uint8_t bSalvaRegistro;      //Indica se gravacao de registros esta ativada

} tsCfgGPS;


typedef struct
{
	//Configuracao Monitor
	UOS_tsCfgMonitor sMonitor;
	//Receptor GPS interno:
	tsCfgGPS sGPS;
	//Configuracao IHM
	tsCfgIHM sIHM;
	//Identificacao do veiculo:
	uint32_t dVeiculo;
	//CRC16 desta estrutura:
	uint16_t wCRC16;
} __attribute__((aligned(1), packed)) UOS_tsConfiguracao;


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
#define UOS_SIS_FLAG_SIS_UP             0x00000020
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

typedef struct
{
	uint16_t awBufDis;
	uint8_t abBufSem[CAN_bNUM_DE_LINHAS];
} tsFalhaInstantanea;

//Estrutura do registro estático:
typedef struct
{

	uint16_t wFlagInicio;           //Flag de inicio do registro (0xFFFF)

	uint8_t abDataHoraIni[6];      //Data/hora de inicio (SMHDMA).
	uint8_t abDataHoraFim[6];      //Data/hora de fim (SMHDMA).

	tsVelocidade sVelocidade;           //Valores relativos à excesso de velocidade

	tsLinhas sTrabParcial;          //Acumulados Parcial Trabalhando
	tsLinhas sTrabParcDir;          //Acumulados Parcial Trabalhando
	tsLinhas sTrabParcEsq;          //Acumulados Parcial Trabalhando

} AQR_tsRegEstatico;

//Estrutura acima com CRC:
typedef struct
{

	uint32_t dEmergencia;     //Indica que a tarefa de emergência executou normalmente
	tsLinhas sTrabTotal;       //Acumulados Total Trabalhando
	tsLinhas sTrabTotalDir;    //Acumulados Total Trabalhando
	tsLinhas sTrabTotalEsq;    //Acumulados Total Trabalhando

	AQR_tsRegEstatico sReg;            //Estrutura de um registro estático.
	uint32_t dDataHora;       //Data/hora da última atualização desta estrutura.
	uint32_t dOffsetRegDat;   //Offset do último registro estático dentro do REGISTRO.DAT

	uint16_t awMediaSementes[36];      //Última média calculada para cada linha

	//Declarei esta variável com 32 bits para evitar problemas de alinhamento
	//no momento do cálculo do CRC.
	uint32_t wCRC16;         //CRC desta estrutura.

} __attribute__((aligned(1), packed)) AQR_tsRegEstaticoCRC;


typedef union
{
	uint32_t wMsgKey;
	struct
	{
		uint32_t wEvent:29;
		uint32_t wType:3;
	};
}smsg_key;

#define LCD_bBRILHO_MAX        99

//Ajuste do Contraste
#define LCD_bCONTRASTE_MAX    127


typedef struct
{
	int8_t bFileName[20];
	int32_t FileLengh;
	int8_t bFileDateTime[17];
	void * pNext;
}FFS_sFileInfo;

typedef struct
{
	uint32_t wTotal;
	uint32_t wFree;
	uint32_t wUsed;
	uint32_t wBad;
	FFS_sFileInfo *pFirst;
	int8_t bNumFiles;
}FFS_sFSInfo;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMMON_COMMON_APP_H_ */
