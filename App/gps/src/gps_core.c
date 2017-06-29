/****************************************************************************
 * Title                 :   gps_core
 * Filename              :   gps_core.c
 * Author                :   Henrique Reis
 * Origin Date           :   21 de mar de 2017
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
 *  21/03/17   1.0.0     Henrique Reis         gps_core.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "gps_core.h"
#include "debug_tool.h"
#include "time.h"
#include "../../gps/config/gps_config.h"
#include "gps_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of BUZZER queue
#define QUEUE_SIZEOFGPS (5)

#define THIS_MODULE MODULE_GPS

#define EXTINT_TIMEPULSE_PORT 4
#define EXTINT_TIMEPULSE_PIN 0

#define LED_DEBUG_GREEN_PORT 0x0E       // LED1
#define LED_DEBUG_GREEN_PIN  0x08

#define LED_DEBUG_RED_PORT 0x0E        // LED2
#define LED_DEBUG_RED_PIN  0x09

/* Tamanho máximo do buffer de transmissão do GPS */
#define GPS_wTAM_BUF_ANEL     256
/* máscara relacionada ao buffer em anel dos dados a serem transmitidos para o gps GPS_wTAM_BUF_ANEL-1  */
#define GPS_wMSK_BUF_ANEL     0x00FF

#define GPS_PULSOS_METRO      100.0f //95.492965f

#define GPS_MAX_TENTATIVAS    3

//-----------------------------------------------------------------------------//
//CFG_PORT (Port Configuration for UART)

//Port Identifier Number (= 1 or 2 for UART ports)
#define UART_1  0x01 //GPS UART 1
//#define UART_2  0x02 //GPS UART 2

//UART mode
//Character Length
//#define MODE_5_BITS 0x00000000 //5bit (not supported)
//#define MODE_6_BITS 0x00000020 //6bit (not supported)
//#define MODE_7_BITS 0x00000080 //7bit (supported only with parity)
#define MODE_8_BITS 0x000000C0 //8bit

//Parity
//#define EVEN_PARITY 0x00000000 //Even Parity
//#define ODD_PARITY  0x00000200 //Odd Parity
#define NO_PARITY   0x00000800 //No Parity

//Number of Stop Bits
#define STOP_BIT_1  0x00000000 //1 Stop Bit
//#define STOP_BIT_15 0x00001000 //1.5 Stop Bit
//#define STOP_BIT_2  0x00002000 //2 Stop Bit
//#define STOP_BIT_05 0x00003000 //0.5 Stop Bit

//Input/ Output active protocols
#define UBX      0x01
//#define NMEA     0x02
//#define _NMEA 0x03

//-----------------------------------------------------------------------------//
//CFG_ANTENNA (Antenna Control Settings)

//Antenna Flag Mask
#define ENABLE_SVCS 0x0001  //Enable Antenna Supply Voltage Control Signal
#define ENABLE_SCD  0x0002  //Enable Short Circuit Detection
#define ENABLE_OCD  0x0004  //Enable Open Circuit Detection
#define PWRDOWN_SCD 0x0008  //Power Down Antenna supply if Short Circuit is detected. (only in combination with Bit 1)
#define RECOVERY    0x0010  //Enable automatic recovery from short state

//-----------------------------------------------------------------------------//
//CFG_NAVIGATION_X (Navigation Engine Expert Settings)

//First Parameters bitmask
#define MIN_MAX_SVS 0x0004  //Apply min/max SVs settings
#define MIN_CNO     0x0008  //Apply minimum C/N0 setting
#define INI_3D_FIX  0x0040  //Apply initial 3D fix settings
#define WKN_ROLL    0x0200  //Apply GPS weeknumber rollover settings

//-----------------------------------------------------------------------------//
//CFG_NAVIGATION (Navigation Engine Settings)

////Parameters Bitmask. (Only the masked parameters will be applied)
#define DYN_MODEL 0x0001 //Apply dynamic model settings
#define MIN_ELEV  0x0002 //Apply minimum elevation settings
#define FIX_MODE  0x0004 //Apply fix mode settings
//#define DR_LIMIT  0x0008 //Apply DR limit settings
#define POS_MASK  0x0010 //Apply position mask settings
#define TIME_MASK 0x0020 //Apply time mask settings
#define HOLD_MASK 0x0040 //Apply static hold settings

//Dynamic Platform Model:
//#define PORTABLE   0x00
//#define STATIONARY 0x02
#define PEDESTRIAN 0x03
#define AUTOMOTIVE 0x04
//#define SEA        0x05
//#define AIRBONE_1G 0x06 //Airborne with >1g Acceleration
//#define AIRBONE_2G 0x07 //Airborne with >2g Acceleration
//#define AIRBONE_4G 0x08 //Airborne with >4g Acceleration

//Position Fixing Mode
//#define MODE_2D_ONLY    0x01
#define MODE_3D_ONLY    0x02
#define MODE_AUTO_2D_3D 0x03

//----------------------------------------------------------------------------//
//CFG_TIMEPULSE (TimePulse Parameters)

//Time pulse config setting
#define POSITIVE    1
//#define OFF         0
#define NEGATIVE   -1

//Alignment to reference time:
#define UTC_TIME    0x00
#define GPS_TIME    0x01
#define LOCAL_TIME  0x02

//Sync Mode:
#define SYNC  0x00 //Time pulse always synchronized and only available if time is valid
#define ASYNC 0x01 //Time pulse allowed to be asynchronized and available even when time is not valid

// CFG-TP5 - Time pulse configuration message
#define TIMEPULSE 0
#define TIMEPULSE2 1
#define TIMEPULSE_MESSAGE_VERSION 1

#define TP5_FLAG_ENABLE (1 << 0)
#define TP5_FLAG_LOCKGPSFREQ (1 << 1)
#define TP5_FLAG_LOCKEDOTHERSET (1 << 2)
#define TP5_FLAG_ISFREQ (1 << 3)
#define TP5_FLAG_ISLENGTH (1 << 4)
#define TP5_FLAG_ALIGNTOTOW (1 << 5)
#define TP5_FLAG_POLARITY (1 << 6)
#define TP5_FLAG_GRIDUTCGPS (1 << 7)

//Máscaras para os flags de eventos do enlace de dados:
#define GPS_ENL_FLAG_NENHUM     0x00000000
#define GPS_ENL_FLAG_TIME_OUT   0x00000001  //Temporizador chegou em zero.
#define GPS_ENL_FLAG_RX_BYTE    0x00000002  //Byte recebido no buffer em anel.

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
// Estruturas do protocolo UBX
//----------------------------------------------------------------------------//
//Estrutura de Configuração da UART
//CFG-PRT
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x00 | 20             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint8_t bPortId;         //Port Identifier Number (= 1 or 2 for UART ports)
	uint8_t bReservado0;     //Reserved
	uint16_t wReservado1;     //Reserved
	uint32_t dMode;           //A bit mask describing the UART mode
	uint32_t dBaudRate;       //Baudrate in bits/second
	uint16_t wInProtoMask;  //A mask describing which input protocols are active
	uint16_t wOutProtoMask; //A mask describing which output protocols are active
	uint16_t wFlags;          //Reserved, set to 0
	uint16_t wPad;            //Reserved, set to 0
} GPS_tsConfigUART;

//----------------------------------------------------------------------------//
//Estrutura de Configuração do Time Pulse
//CFG-TP
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x07 | 20             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint32_t dnterval;      //Time interval for time pulse (us)
	uint32_t dLength;       //Length of time pulse (us)
	int8_t cStatus; //Time pulse config setting: +1 = positive, 0 = off, -1 = negative
	uint8_t bTimeRef; //Tlignment to reference time: 0 = UTC time, 1 = GPS time, 2 = Local time
	uint8_t bSyncMode; //0=Time pulse always synchronized and only available if time is valid
//1=Time pulse allowed to be asynchronized and available even when time is not valid
	uint8_t bReservado0;   //Reserved
	int16_t iCableDelay;   //Antenna Cable Delay (ns)
	int16_t iRfGroupDelay; //Receiver RF Group Delay
	int32_t lUserDelay; //User Time Function Delay (positive delay results in earlier pulse)
} GPS_tsConfigTP;

//----------------------------------------------------------------------------//
//Estrutura de Configuração do Time Pulse
//CFG-TP
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x31 | 32             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint8_t tpIdx;                  // Time pulse selection
	uint8_t version;                // Message version
	uint16_t reserved1;             // Reserved
	int16_t antCableDelay;          // Antenna cable delay
	int16_t rfGroupDelay;           // RF group delay
	uint32_t freqPeriod; // Frequency or period time, depending on setting of bit 'isFreq'
	uint32_t freqPeriodLock; // Frequency or period time when locked to GNSS time, only used if 'lockedOtherSet' is set
	uint32_t pulseLenRatio; // Pulse length or duty cycle, depending on setting of bit 'isFreq'
	uint32_t pulseLenRatioLock; // Pulse length or duty cycle when locked to GNSS time, only used if 'lockedOtherSet' is set
	int32_t userConfigDelay; // User configurable time pulse delay (positive delay results in earlier pulse)
	uint32_t flags;                 // Configuration flags
} GPS_tsConfigTP5;

//----------------------------------------------------------------------------//
//Estrutura de Configuração do Rate
//CFG-TP
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x08 | 06             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint16_t wMeasRate; //Measurement Rate, GPS measurements are taken every measRate milliseconds
	uint16_t wNavRate;  //Navigation Rate, in number of measurement cycles.
//On u-blox 5, this parameter cannot be changed, and is always equals 1.
	uint16_t wTimeRef; //Alignment to reference time: 0 = UTC time, 1 = GPS time

} GPS_tsConfigRate;

//----------------------------------------------------------------------------//
//Estrutura de Configuração do Controle da Antena
//CFG-ANT
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x13 | 04             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint16_t wFlags;
	uint16_t wPins;
} GPS_tsConfigAnt;

//----------------------------------------------------------------------------//
//Estrutura de Navigation Engine Expert Settings
//CFG-ANT
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x23 | 40             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint16_t wVersion;  //Message version. Current version is 0.
	uint16_t wMask1;    //First Parameters Bitmask. Only the flagged parameters
//will be applied, unused bits must be set to 0.
	uint32_t wMask2; //Second Parameters Bitmask. Currently unused, must be set to 0.
	uint8_t bRes1;     //reserved, set to 0
	uint8_t bRes2;     //reserved, set to 0
	uint8_t bMinSVs;   //Minimum number of satellites for navigation
	uint8_t bMaxSVs;   //Maximum number of satellites for navigation
	uint8_t bMinCNO;   //Minimum satellite signal level for navigation
	uint8_t bRes3;     //reserved, set to 0
	uint8_t bIniFix3D; //Initial Fix must be 3D flag (0=false/1=true)
	uint8_t bRes4;     //reserved, set to 0
	uint8_t bRes5;     //reserved, set to 0
	uint8_t bRes6;     //reserved, set to 0
	uint16_t wWknRollover; //GPS week rollover number; GPS week numbers will be set
//correctly from this week up to 1024 weeks after this
//week. Setting this to 0 reverts to firmware default.
	uint32_t dRes7;     //reserved, set to 0
	uint32_t dRes8;     //reserved, set to 0
	uint32_t dRes9;     //reserved, set to 0
	uint32_t dRes10;    //reserved, set to 0
	uint32_t dRes11;    //reserved, set to 0
} GPS_tsConfigNavX;

//----------------------------------------------------------------------------//
//Estrutura de Navigation Engine Settings
//CFG-ANT
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x24 | 36             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint16_t wMask;
	uint8_t bDynModel;
	uint8_t bFixMode;
	int32_t lFixedAlt;
	uint32_t dFixedAltVar;
	int8_t cMinElev;
	uint8_t bDrLimit;
	uint16_t wPDOP;
	uint16_t wTDOP;
	uint16_t wPACC;
	uint16_t wTACC;
	uint8_t bStaticHoldThresh;
	uint8_t bReserved1;
	uint32_t dReserved2;
	uint32_t dReserved3;
	uint32_t dReserved4;
} GPS_tsConfigNav;

//----------------------------------------------------------------------------//
//Estrutura de Configuração das Mensagens
//CFG-MSG
//  | Header    | ID        | Length (Bytes) | Payload   | Checksum  |
//  | 0xB5 0x62 | 0x06 0x01 | 03             |           | CK_A CK_B |
//----------------------------------------------------------------------------//
typedef struct
{
	uint8_t bClass;
	uint8_t bMsgId;
	uint8_t bRate;
} GPS_tsConfigMsg;

//------------------------------------------------------------------------------

typedef struct
{
	uint8_t bNavPos;
	uint8_t bNavSol;
	uint8_t bNavVelNed;
	uint8_t bNavTimeGps;
	uint8_t bMonVer;
	uint8_t bMonHw;

} GPS_tsTimeoutMsg;

typedef struct
{
	uint32_t dNavPos;
	uint32_t dNavSol;
	uint32_t dNavVelNed;
	uint32_t dNavTimeGps;
	uint32_t dMonVer;
	uint32_t dMonHw;
	uint32_t dConfiguracao;
} GPS_tsGerenciaTimeoutMsg;

//Passos da configuração do módulo GPS
typedef enum
{
	SemConfiguracao,
	VerificaPort,
	CfgPort,
	CfgTimePulse,
	CfgRateSettings,
	CfgAntenna,
	CfgNavigation,
	CfgMsgNavPosllh,
	CfgMsgNavSol,
	CfgMsgNavVelned,
	CfgMsgNavTimeGps,
	CfgMsgMonHw
} GPS_teConfigura;

typedef struct
{
	osFlagsGroupId* psFlagEnl;
	osFlags dEventosEnl;
	osTimerId bTimer;
	uint32_t dTicksIDLE;
} GPS_tsCtrlEnl;

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static eAPPError_s eError;                          //!< Error variable

DECLARE_QUEUE(GPSQueue, QUEUE_SIZEOFGPS);    //!< Declaration of Interface Queue
CREATE_SIGNATURE(GPS);//!< Signature Declarations
CREATE_CONTRACT(GPS);//!< Create contract for buzzer msg publication

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = { GPS_MODULES };
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

WATCHDOG_CREATE(GPSPUB);//!< WDT pointer flag
WATCHDOG_CREATE(GPSMGT);//!< WDT pointer flag
WATCHDOG_CREATE(GPSTPS);//!< WDT pointer flag
WATCHDOG_CREATE(GPSRCV);//!< WDT pointer flag
uint8_t bGPSPUBThreadArrayPosition = 0;            //!< Thread position in array
uint8_t bGPSMGTThreadArrayPosition = 0;            //!< Thread position in array
uint8_t bGPSTPSThreadArrayPosition = 0;            //!< Thread position in array
uint8_t bGPSRCVThreadArrayPosition = 0;            //!< Thread position in array

CREATE_LOCAL_QUEUE(GPSPublishQ, uint8_t*, 10);

peripheral_descriptor_p pGPSHandle;

gpio_config_s sTimePulseInt;

/*******************************************************************************
 Variáveis públicas deste módulo:
 *******************************************************************************/

//Mutex para controle de acesso às estruturas de dados das entradas e variáveis
//da aquisição e registro:
osMutexId GPS_MTX_sEntradas;

/*******************************************************************************
 Variáveis locais
 *******************************************************************************/

//Flags para indicar eventos do enlace de dados:
osFlagsGroupId GPS_sFlagEnl;

//Flags para indicar eventos da aplicação do protocolo:
//osFlagsGroupId GPS_sFlagApl;

//Flags para indicar eventos do GPS
osFlagsGroupId GPS_sFlagGPS;

//Estrutura de controle do protocolo de comunicação:
GPS_tsCtrlEnl GPS_sCtrlCBASRL;

CREATE_RINGBUFFER(uint8_t, GPSRx, GPS_RX_BUFFER_SIZE);

//Estrutura de dados do GPS
GPS_tsDadosGPS GPS_sDadosGPS;

//Estrutura de dados de hora
GPS_tsDataHoraLocal GPS_sDataHoraLocal;

GPS_tsTimeoutMsg GPS_sTimeoutMsg;
GPS_tsGerenciaTimeoutMsg GPS_sGerenciaTimeoutMsg;

//Variáveis de Configuração
uint16_t GPS_wMsgAck;
GPS_teConfigura GPS_eConfigura;
GPS_teConfigura GPS_eStatusConfig;

uint8_t GPS_bEstadoRxGPS;
uint8_t abBufMsgProcRxGPS[GPS_RX_BUFFER_SIZE];

uint16_t wPosMsgProcRxGps;
uint16_t wDataLength;
uint16_t wDataByte;

float GPS_fDistancia;
uint8_t GPS_bDistanciaPercorrida;

uint8_t bContaSegundo;
uint8_t bTimeSync;

uint8_t bAlternaBaud = false;

float fDistanciaAcumula = 0.0f;

// External mutex from CONTROL module
EXTERN_MUTEX(UOS_MTX_sDataHora);

extern tsStatus AQR_sStatus;
extern osFlagsGroupId UOS_sFlagSis;
extern UOS_tsConfiguracao UOS_sConfiguracao;
extern uint32_t CAN_dLeituraSensor;
extern uint8_t CAN_bSensorSimulador;

GPS_tsDadosGPS GPS_sPublishDadosGPS;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

uint32_t GPS_vSendData (uint16_t wNumDados, uint8_t *pbVetorDados);
void GPS_vIdentMsgRxGPS (void);
void GPS_vProcessBufferRx (void);
uint8_t GPS_vGPSPackMsgTx (uint8_t bClass, uint8_t bId, uint8_t *pbDados,
	uint8_t bLength);

void GPS_vTimerCallbackMtr (void const*);
void GPS_vTimerCallbackTimeout (void const*);
void GPS_vTimerCallbackTimeoutEnl (void const*);
void GPS_vTimePulseIntCallback (void);

/******************************************************************************
 * Module timers
 *******************************************************************************/
CREATE_TIMER(GPS_bTimerMtr, GPS_vTimerCallbackMtr);
CREATE_TIMER(GPS_bTimerTimeout, GPS_vTimerCallbackTimeout);
CREATE_TIMER(GPS_bTimerTimeoutEnl, GPS_vTimerCallbackTimeoutEnl);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t* GPS_WDTData (uint8_t * pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY)
		/ sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t*) WATCHDOG_FLAG_ARRAY;
}

inline void GPS_vDetectThread (thisWDTFlag* flag, uint8_t* bPosition,
	void* pFunc)
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
 * Function : GPS_vCreateThread(const Threads_t sSensorThread )
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
static void GPS_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread,
		(void*)osThreadGetId());

	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}
#endif

/******************************************************************************
 * Function : GPS_eInitGPSPublisher(void)
 *//**
 * \b Description:
 *
 * This routine prepares the contract and message that the GPS_vGPSPublishThread thread
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
 * @see GPS_vGPSThread, GPS_vGPSPublishThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
eAPPError_s GPS_eInitGPSPublisher (void)
{
	/* Check if handler is already enabled */
	if (pGPSHandle == NULL)
	{
		return APP_ERROR_ERROR;
	}

	//Prepare Default Contract/Message
	MESSAGE_HEADER(GPS, GPS_DEFAULT_MSGSIZE, 1, MT_ARRAYBYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(GPS, 1, THIS_MODULE, TOPIC_GPS);

	return APP_ERROR_SUCCESS;
}

PubMessage sGPSPubMsg;

/******************************************************************************
 * Function : GPS_vGPSPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the GPS module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the GPS topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from GPS_vGPSThread,
 ~~~~~~~~~~~~~~~
 *
 * @see GPS_vGPSThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void GPS_vGPSPublishThread (void const *argument)
{
	osFlags dValorGPS;
	osStatus status;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Buzzer Publish Thread Created");
#endif

	GPS_vDetectThread(&WATCHDOG(GPSPUB), &bGPSPUBThreadArrayPosition,
		(void*)GPS_vGPSPublishThread);
	WATCHDOG_STATE(GPSPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bGPSPUBThreadArrayPosition)); //Task created, inform core


	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(GPSPUB, WDT_SLEEP);
		dValorGPS = osFlagWait(GPS_sFlagGPS, (GPS_FLAG_METRO | GPS_FLAG_SEGUNDO | GPS_FLAG_TIMEOUT_MTR), true, false,
		osWaitForever);
		WATCHDOG_STATE(GPSPUB, WDT_ACTIVE);

		status = WAIT_MUTEX(GPS_MTX_sEntradas, osWaitForever);
		ASSERT(status == osOK);

		memcpy(&GPS_sPublishDadosGPS, &GPS_sDadosGPS, sizeof(GPS_tsDadosGPS));
		sGPSPubMsg.vPayload = (void*)&GPS_sPublishDadosGPS;

		status = RELEASE_MUTEX(GPS_MTX_sEntradas);
		ASSERT(status == osOK);

		// Todas as publicacoes a seguir chegaram aos modulos que estao assinados para receber as publicacoes do modulo GPS
		if ((dValorGPS & GPS_FLAG_METRO) > 0)
		{
			// Este evento interessa ao modulo SENSOR, ACQUIREG e RECORDS
			sGPSPubMsg.dEvent = GPS_FLAG_METRO;
			MESSAGE_PAYLOAD(GPS) = (void*)&sGPSPubMsg;
			PUBLISH(CONTRACT(GPS), 0);
		}

		if ((dValorGPS & GPS_FLAG_SEGUNDO) > 0)
		{
			// Este evento interessa ao modulo ACQUIREG, mais precisamente a thread AQR_vAcquiregTimeThread
			sGPSPubMsg.dEvent = GPS_FLAG_SEGUNDO;
			MESSAGE_PAYLOAD(GPS) = (void*)&sGPSPubMsg;
			PUBLISH(CONTRACT(GPS), 0);
		}

		if ((dValorGPS & GPS_FLAG_TIMEOUT_MTR) > 0)
		{
			// Este evento interessa ao modulo ACQUIREG, AQR_vAcquiregManagementThread
			sGPSPubMsg.dEvent = GPS_FLAG_TIMEOUT_MTR;
			MESSAGE_PAYLOAD(GPS) = (void*)&sGPSPubMsg;
			PUBLISH(CONTRACT(GPS), 0);
		}
	}
	osThreadTerminate(NULL);
}
#else
void GPS_vGPSPublishThread(void const *argument)
{}
#endif

/*******************************************************************************

 void GPS_vAcumulaDistancia( void )

 Descrição : Rotina que acumula distância a cada 100ms e indica a cada metro
 percorrido.

 Parâmetros: nenhum
 Retorno   : nenhum

 *******************************************************************************/
void GPS_vAcumulaDistancia (void)
{
	osStatus status;
	uint8_t bErr;
	osFlags dFlagsSis;
	float fAux;
	float fDistanciaMeioTick;
	float fDistanciaMinima;

	//Se estiver em modo de trabalho
	dFlagsSis = osFlagGet(UOS_sFlagSis);

	if (((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) > 0)
		&& ((AQR_sStatus.bErroGPS == false) || (CAN_bSensorSimulador != false)))
	{
		//Pega o mutex antes acessar dados compartilhados:
		status = WAIT_MUTEX(GPS_MTX_sEntradas, osWaitForever);
		ASSERT(status == osOK);

		//Se velocidade no GPS ou no simulador for maior que 0,14m/s = 0,5 km/h
		if (GPS_sDadosGPS.dGroundSpeed > 14)
		{
			//Acumula a distância percorrida
			fAux = GPS_sDadosGPS.dGroundSpeed;
			fAux = (fAux / (float) NUM_AM_INT);

			//Acumula distância total percorrida
			//GPS_sDadosGPS.fDistancia = ( fAux + GPS_sDadosGPS.fDistancia );
			fDistanciaAcumula = (fAux + fDistanciaAcumula);

			//Acumula distância percorrida desde o último metro
			GPS_fDistancia = (fAux + GPS_fDistancia);

			//Calcula distância percorrida em meio tick
			fDistanciaMeioTick = (fAux / 2.0f);

			//Calcula a distância mínima para considerar 1 metro
			fDistanciaMinima = (100.0f - fDistanciaMeioTick);

			//Liga um flag a cada metro percorrido
			if (GPS_fDistancia >= fDistanciaMinima)
			{
				GPS_bDistanciaPercorrida = (uint8_t)(GPS_fDistancia);
				GPS_fDistancia = 0.0f;
				fDistanciaMinima = 0.0f;

				GPS_sDadosGPS.fDistancia = fDistanciaAcumula;
				fDistanciaAcumula = 0.0f;

				if (UOS_sConfiguracao.sMonitor.bMonitorArea == false)
				{
					//Solicita leitura de dados dos sensores
//                    CAN_vLeituraDadosSensores();
//
//                    //Aguarda resposta dos sensores
//                    OSFlagPend( CAN_psFlagApl,
//                                CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP |
//                                CAN_APL_FLAG_SENSOR_NAO_RESPONDEU|
//                                CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO,
//                                OS_FLAG_WAIT_SET_ANY,
//                                0, &bErr );
//                    __assert( bErr == OS_NO_ERR );
				}

				//Seta Flag de 1 metro percorrido, para tarefa de aquisição de dados
				osFlagSet(GPS_sFlagGPS, GPS_FLAG_METRO);
			}

			//------------------------------------------------------------------------

			//Escolhe o melhor valor de timeout, de acordo com a velocidade atual
			//Se a velocidade for maior que 1m/s
			if (GPS_sDadosGPS.dGroundSpeed > 100)
			{
				//Ajusta o timer com timeout de 1 segundos
				status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_1S);
				ASSERT(status == osOK);
			}
			else
			{
				//Se a velocidade estiver entre 0,51 m/s e 1m/s
				if (GPS_sDadosGPS.dGroundSpeed > 50)
				{
					//Ajusta o timer com timeout de 2 segundos
					status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_2S);
					ASSERT(status == osOK);
				}
				else
				{
					//Se a velocidade estiver entre 0,34 m/s e 0,5 m/s
					if (GPS_sDadosGPS.dGroundSpeed > 33)
					{
						//Ajusta o timer com timeout de 3 segundos
						status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_3S);
						ASSERT(status == osOK);
					}
					else
					{
						//Se a velocidade estiver entre 0,26 m/s e 0,34
						if (GPS_sDadosGPS.dGroundSpeed > 25)
						{
							//Ajusta o timer com timeout de 4 segundos
							status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_4S);
							ASSERT(status == osOK);
						}
						else
						{
							//Se a velocidade for menor ou igual a 0,25 m/s
							//Ajusta o timer com timeout de 8 segundos
							status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_8S);
							ASSERT(status == osOK);
						}
					}
				}
			}
		}

		//Devolve o mutex:
		status = RELEASE_MUTEX(GPS_MTX_sEntradas);
		ASSERT(status == osOK);
	}

}

void GPS_vTimePulseIntCallback (void)
{
	// Set time pulse flag
	osFlagSetIrq(GPS_sFlagGPS, GPS_FLAG_INT_TIMEPULSE);

	STOP_TIMER(GPS_bTimerTimeout);
	START_TIMER(GPS_bTimerTimeout, 125);
}

void GPS_vConfigExtInterrupt (void)
{
	sTimePulseInt.vpPrivateData = NULL;
	sTimePulseInt.bDefaultOutputHigh = false;
	sTimePulseInt.eDirection = GPIO_INPUT;
	sTimePulseInt.ePull = GPIO_PULLUP;
	sTimePulseInt.eInterrupt = GPIO_INTERRUPT_ON_FALLING;
	sTimePulseInt.fpCallBack = GPS_vTimePulseIntCallback;
	sTimePulseInt.bMPort = EXTINT_TIMEPULSE_PORT;
	sTimePulseInt.bMPin = EXTINT_TIMEPULSE_PIN;

	// Initialize time pulse external interrupt
	GPIO_eInit(&sTimePulseInt);
}

/*******************************************************************************

 void GPS_vTrfTrataTimePulse( void *p_arg )

 Descrição : Tarefa que trata a interrupção do TIMEPULSE
 Parâmetros: nenhum
 Retorno   : nenhum

 *******************************************************************************/
void GPS_vGPSTimePulseThread (void const *argument)
{
	osFlags dFlags;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("GPS Timepulse Thread Created");
#endif

	GPS_vDetectThread(&WATCHDOG(GPSTPS), &bGPSTPSThreadArrayPosition, (void*)GPS_vGPSTimePulseThread);
	WATCHDOG_STATE(GPSTPS, WDT_ACTIVE);

	osThreadId xMainID = (osThreadId)argument;
	osSignalSet(xMainID, THREADS_RETURN_SIGNAL(bGPSTPSThreadArrayPosition)); //Task created, inform core

	// Configure external interrupt 2 - Module GPS TIMEPULSE
	GPS_vConfigExtInterrupt();

	//Loop infinito da tarefa:
	while (1)
	{
		// Wait for TIMEPULSE event
		WATCHDOG_STATE(GPSTPS, WDT_SLEEP);
		dFlags = osFlagWait(GPS_sFlagGPS, (GPS_FLAG_TIME_OUT | GPS_FLAG_INT_TIMEPULSE), true, false, osWaitForever);
		WATCHDOG_STATE(GPSTPS, WDT_ACTIVE);

		if (bTimeSync != false)
		{
			// If time pulse interrupt
			if ((dFlags & GPS_FLAG_INT_TIMEPULSE) > 0)
			{
				bContaSegundo++;

				if (bContaSegundo > 3)
				{
					bContaSegundo = 0;

					osFlagSet(GPS_sFlagGPS, GPS_FLAG_SEGUNDO);
				}
			}
			//Acumula a distância percorrida.
			GPS_vAcumulaDistancia();
		}
		else
		{
			bTimeSync = true;
			bContaSegundo = 0;
		}
	}
}

/*******************************************************************************
 void GPS_vGPSPackMsgTx ( uint8_t bClass, uint8_t bId, uint8_t *pbDados, uint8_t bLength )

 Descrição : rotina de que monta a mensagem a ser enviada para o GPS.

 Parâmetros: bClass -> Id de classe da mensagem
 bId -> Id da mensagem
 *pbDados -> ponteiro para os bytes de dados.
 bLength -> quantidade de bytes de dados

 Retorno   :
 *******************************************************************************/

uint8_t GPS_vGPSPackMsgTx (uint8_t bClass, uint8_t bId, uint8_t *pbDados,
	uint8_t bLength)
{
	uint8_t abMsg[52];
	uint8_t bChecksum_A = 0;
	uint8_t bChecksum_B = 0;
	uint8_t bTamanho;
	uint8_t bNumDados = 0;

	//Bytes de Sincronismo
	abMsg[bNumDados++] = SYNC_CHAR_1;
	abMsg[bNumDados++] = SYNC_CHAR_2;

	//Ids de Classe e Menssagem
	abMsg[bNumDados++] = bClass;
	abMsg[bNumDados++] = bId;

	//Tamanho ds dados
	abMsg[bNumDados++] = bLength;
	abMsg[bNumDados++] = 0;         //Verificar

	//Tamanho da msg sem o Checksum
	bTamanho = bNumDados + bLength;

	//Coloca dados no buffer
	for (; bNumDados < bTamanho; bNumDados++)
	{
		abMsg[bNumDados] = *pbDados++;
	}

	//Tamanho da msg sem o Checksum
	//bTamanho = 4 + bLength;

	//Calcula o checksum
	for (bNumDados = 2; bNumDados < bTamanho; bNumDados++)
	{
		bChecksum_A = bChecksum_A + abMsg[bNumDados];
		bChecksum_B = bChecksum_B + bChecksum_A;
	}

	//Coloca o checksum no buffer
	abMsg[bNumDados++] = bChecksum_A;
	abMsg[bNumDados++] = bChecksum_B;

	//Transmite os dados para o GPS
	if ((GPS_vSendData(bNumDados, abMsg)) != false)
	{
		return true;
	}
	else
	{
		return false;
	}

}  //Fim da função

/*******************************************************************************
 void GPSTransmiteDados( uint8_t bNumDados, uint8_t *pbVetorDados)

 Descrição : rotina de que envia comandos para o GPS.

 Parâmetros: bNumDados -> quantidade de bytes a ser enviado para GPS
 *bVetorDados -> ponteiro para os bytes a serem enviados.

 Retorno   :
 *******************************************************************************/
uint32_t GPS_vSendData (uint16_t wNumDados, uint8_t *pbVetorDados)
{
	return DEV_write(pGPSHandle, &pbVetorDados[0], wNumDados);
}

/*******************************************************************************
 void GPS_vGPSConfig( void)

 Descrição : rotina de que configura o módulo GPS.

 Parâmetros:

 Retorno   :
 *******************************************************************************/
void GPS_vGPSConfig (void)
{
	uint8_t abCfgGps[56];
	uint32_t wBaudRate;
	osFlags dFlagsGPS;

	GPS_tsConfigUART GPS_sConfigUART;
	GPS_tsConfigTP5 GPS_sConfigTP5;
	GPS_tsConfigRate GPS_sConfigRate;
	GPS_tsConfigAnt GPS_sConfigAnt;
	GPS_tsConfigNav GPS_sConfigNav;
	GPS_tsConfigMsg GPS_sConfigMsg;

	dFlagsGPS = GPS_sCtrlCBASRL.dEventosEnl;

	//Se ocorreu um time-out volta um passo na configuração
	if ((dFlagsGPS & GPS_ENL_FLAG_TIME_OUT) > 0)
	{
		if (GPS_eStatusConfig > SemConfiguracao)
		{
			GPS_eStatusConfig--;
		}
		else
		{
			GPS_eStatusConfig = SemConfiguracao;
		}
	}

	switch (GPS_eConfigura)
	{

		case VerificaPort: //Polls the configuration for UART
		{
			if (GPS_eStatusConfig != VerificaPort)
			{
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				if (bAlternaBaud != false)
				{
					wBaudRate = 115200;
					DEV_ioctl(pGPSHandle, IOCTL_M2GGPSCOMM_CHANGE_BAUD_RATE,
						&wBaudRate);
				}
				else
				{
					wBaudRate = 9600;
					DEV_ioctl(pGPSHandle, IOCTL_M2GGPSCOMM_CHANGE_BAUD_RATE,
						&wBaudRate);
				}

				memset(&abCfgGps, 0x00, sizeof(abCfgGps));
				abCfgGps[0] = 1;

				//Polls the configuration for one I/O Port
				if ((GPS_vGPSPackMsgTx( CFG, CFG_PRT, abCfgGps, 0x01)) != false)
				{
					if (bAlternaBaud == false)
					{
						bAlternaBaud = true;
					}
					else
					{
						bAlternaBaud = false;
					}
				}
				GPS_eStatusConfig = GPS_eConfigura;
			}
			break;
		}

		case CfgPort: //Polls the configuration for UART
		{
			if (GPS_eStatusConfig != CfgPort)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				memset(&GPS_sConfigUART, 0x00, sizeof(GPS_sConfigUART));
				memset(&abCfgGps, 0x00, sizeof(abCfgGps));

				//Configurações da UART do GPS
				GPS_sConfigUART.bPortId = UART_1; // Port Identifier Number (= 1 or 2 for UART ports)
				GPS_sConfigUART.dMode = ( MODE_8_BITS | //UART Mode: Character Length
					NO_PARITY | //UART Mode: Parity
					STOP_BIT_1    //UART Mode: Number of stop bits
				);
				GPS_sConfigUART.dBaudRate = 115200;  // Baudrate (bits/s)
				GPS_sConfigUART.wInProtoMask = UBX;  // Input active protocols
				GPS_sConfigUART.wOutProtoMask = UBX; // Output active protocols

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigUART, sizeof(GPS_tsConfigUART));

				//Monta mensagem de configuração
				GPS_vGPSPackMsgTx( CFG, CFG_PRT, abCfgGps,
					sizeof(GPS_tsConfigUART));

				//Se a alteração do baud rate do GPS ainda não respondeu satisfatóriamente
				//altera o baud da UART0 para estabelecer uma comunicação
				if (GPS_sDadosGPS.dBaud != 115200)
				{
					GPS_eConfigura = VerificaPort;
					bAlternaBaud = true;
				}
				else
				{
					//Se conseguiu mudar o baud rate do GPS para 115200, altera o status
					//para prosseguir com a configuração
					GPS_eStatusConfig = GPS_eConfigura;
				}

			}
			else // alternativa para quando o GPS não enviar ACK para o comando de CFG do Port
			{
				//Verifica se está com o Baud desejado
				if (GPS_sDadosGPS.dBaud == 115200)
				{
					GPS_eConfigura = CfgTimePulse;
				}
			}
			break;
		}

		case CfgTimePulse: //Get/Set TimePulse Parameters
		{

			if (GPS_eStatusConfig != CfgTimePulse)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				memset(&GPS_sConfigTP5, 0x00, sizeof(GPS_tsConfigTP5));

				//Configura os parâmetros do Time Pulse

				GPS_sConfigTP5.tpIdx = TIMEPULSE2;
				GPS_sConfigTP5.version = TIMEPULSE_MESSAGE_VERSION;
				GPS_sConfigTP5.antCableDelay = 50;
				GPS_sConfigTP5.freqPeriod = 250000;
				GPS_sConfigTP5.freqPeriodLock = 250000;
				GPS_sConfigTP5.pulseLenRatio = 25000;

				GPS_sConfigTP5.flags = (TP5_FLAG_ENABLE | TP5_FLAG_LOCKGPSFREQ
					| TP5_FLAG_ISLENGTH | TP5_FLAG_ALIGNTOTOW
					| TP5_FLAG_GRIDUTCGPS);

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigTP5, sizeof(GPS_sConfigTP5));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_TP5, abCfgGps,
					sizeof(GPS_sConfigTP5))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}

		case CfgRateSettings: //Navigation/Measurement Rate Settings
		{

			if (GPS_eStatusConfig != CfgRateSettings)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigRate.wMeasRate = 250; //Measurement Rate, GPS measurements are
				//taken every measRate milliseconds (ms)
				GPS_sConfigRate.wNavRate = 1; //Navigation Rate, in number of measurement
				//cycles. On u-blox 5, this parameter cannot be
				//changed, and is always equals 1.
				GPS_sConfigRate.wTimeRef = GPS_TIME; //Alignment to reference time: 0 = UTC time,
				//1 = GPS time

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigRate, sizeof(GPS_sConfigRate));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_RATE, abCfgGps,
					sizeof(GPS_sConfigRate))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}

		case CfgAntenna: //Get/Set Antenna Control Settings
		{
			if (GPS_eStatusConfig != CfgAntenna)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigAnt.wFlags = ( ENABLE_SVCS //| //Enable Antenna Supply Voltage Control Signal
//                        ENABLE_SCD  | //Enable Short Circuit Detection
//                        ENABLE_OCD  | //Enable Open Circuit Detection
					//PWRDOWN_SCD | //Power Down Antenna supply if Short Circuit is detected. (only in combination with Bit 1)
//                        RECOVERY      //Enable automatic recovery from short state
					);

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigAnt, sizeof(GPS_sConfigAnt));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_ANT, abCfgGps,
					sizeof(GPS_sConfigAnt))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}

			}
			break;
		}

		case CfgNavigation: //Get/Set Navigation Engine Settings
		{

			if (GPS_eStatusConfig != GPS_eConfigura)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigNav.wMask = ( DYN_MODEL | //Apply dynamic model settings
					MIN_ELEV | //Apply minimum elevation settings
					FIX_MODE | //Apply fix mode settings
					POS_MASK | //Apply position mask settings
					TIME_MASK | //Apply time mask settings
					HOLD_MASK);

				GPS_sConfigNav.bDynModel = AUTOMOTIVE; //Dynamic Platform model.
				GPS_sConfigNav.bFixMode = MODE_3D_ONLY; //MODE_AUTO_2D_3D; //Position Fixing Mode.
				GPS_sConfigNav.cMinElev = 5; //Minimum Elevation for a GNSS satellite to be used in NAV (deg)
				GPS_sConfigNav.wPDOP = 60; //120;  //Position DOP Mask to use (Scaling 0.1)
				GPS_sConfigNav.wTDOP = 60; //120;  //Time DOP Mask to use (Scaling 0.1)
				GPS_sConfigNav.wPACC = 50;    //Position Accuracy Mask (m)
				GPS_sConfigNav.wTACC = 300;   //Time Accuracy Mask (m)
				GPS_sConfigNav.bStaticHoldThresh = 14; //Static hold threshold (cm/s)

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigNav, sizeof(GPS_sConfigNav));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_NAV5, abCfgGps,
					sizeof(GPS_sConfigNav))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}

		//----------------------------------------------------------------------------//
		case CfgMsgNavPosllh: //Geodetic Position Solution
		{
			if (GPS_eStatusConfig != CfgMsgNavPosllh)
			{

				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigMsg.bClass = NAV;        //Navigation Results
				GPS_sConfigMsg.bMsgId = NAV_POSLLH; //Geodetic Position Solution
				GPS_sConfigMsg.bRate = 1; // Recebe msg a cada 0,25 segundos

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigMsg, sizeof(GPS_sConfigMsg));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_MSG, abCfgGps,
					sizeof(GPS_sConfigMsg))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}

		case CfgMsgNavSol: //Navigation Solution Information
		{
			if (GPS_eStatusConfig != CfgMsgNavSol)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigMsg.bClass = NAV;  //Navigation Results
				GPS_sConfigMsg.bMsgId = NAV_SOL; //Navigation Solution Information
				GPS_sConfigMsg.bRate = 8; //Recebe msg a cada 2 segundos

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigMsg, sizeof(GPS_sConfigMsg));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_MSG, abCfgGps,
					sizeof(GPS_sConfigMsg))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}

		case CfgMsgNavVelned: //Velocity Solution in NED
		{
			if (GPS_eStatusConfig != CfgMsgNavVelned)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigMsg.bClass = NAV;  //Navigation Results
				GPS_sConfigMsg.bMsgId = NAV_VELNED; //Velocity Solution in NED
				GPS_sConfigMsg.bRate = 1; // Recebe mensagem a cada 0,25 segundos

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigMsg, sizeof(GPS_sConfigMsg));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_MSG, abCfgGps,
					sizeof(GPS_sConfigMsg))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}
		case CfgMsgNavTimeGps: //GPS Time Solution
		{
			if (GPS_eStatusConfig != CfgMsgNavTimeGps)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				GPS_sConfigMsg.bClass = NAV;  //Navigation Results
				GPS_sConfigMsg.bMsgId = NAV_TIMEGPS; //GPS Time Solution
				GPS_sConfigMsg.bRate = 2; // Recebe msg a cada 0,5 segundos

				//Copia mensagem para buffer de mensagem
				memcpy(abCfgGps, &GPS_sConfigMsg, sizeof(GPS_sConfigMsg));

				//Monta mensagem de configuração
				if ((GPS_vGPSPackMsgTx( CFG, CFG_MSG, abCfgGps,
					sizeof(GPS_sConfigMsg))) != false)
				{
					GPS_eStatusConfig = GPS_eConfigura;
				}
			}
			break;
		}

		case CfgMsgMonHw: //Hardware Status
		{
			if (GPS_eStatusConfig != CfgMsgMonHw)
			{
				//                UOS_vReiniciaTimer( GPS_sCtrlCBASRL.sCtrlEnl.bTimer );
				START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

				//Se já recebeu a msg com a versão de SW
				if (GPS_sTimeoutMsg.bMonVer != false)
				{
					GPS_sConfigMsg.bClass = MON;
					GPS_sConfigMsg.bMsgId = MON_HW;
					GPS_sConfigMsg.bRate = 2; //Recebe msg a cada 2 segundos

					//Copia mensagem para buffer de mensagem
					memcpy(abCfgGps, &GPS_sConfigMsg, sizeof(GPS_sConfigMsg));

					//Monta mensagem de configuração
					if ((GPS_vGPSPackMsgTx( CFG, CFG_MSG, abCfgGps,
						sizeof(GPS_sConfigMsg))) != false)
					{
						GPS_eStatusConfig = GPS_eConfigura;
					}
				}
				else //Se ainda não recebeu a msg com a versão de SW, solicita versão.
				{
					//Receiver/Software Version
					//Polls the receiver/Software Version
					GPS_vGPSPackMsgTx( MON, MON_VER, 0, 0);
				}
			}
			break;
		}

		default:
			break;
	}

}

/*******************************************************************************

 unsigned char RTCConverteGPSToRTC( int16_t iWeek, uint32_t dTempoSeg )

 Descrição : Esta rotina converte uma data baseada em nº de semanas e tempo em
 segundos, para ano, mes, dia do mes, dia da semana, hora, minuto e segundo.
 Para a conversão de ano, mes e dia, o cálculo é feito baseado na
 data juliana. O nº de semanas são multiplicados por 7, e o tempo em seg é dividido
 por 86400( 24 hrs*60 min*60 seg ), para se encontrar qtos dias se passaram desde
 o dia 6 de janeiro de 1980. A este nº de dias, é somado a data juliana do dia
 6 de janeiro de 1980. Com esta data juliana, é calculado o mes, dia e ano.
 Para o cálculo do dia da semana, basta dividir o tempo em segundos por 86400.
 Para a hora, basta subtrair do tempo em segundos os dias inteiros da semana
 que já se passaram( hora = (dTempoSeg/1000) - DiadaSemana*86400 );


 Parâmetros: int16_t iWeek  : é o nº da semana, desde o dia 6 de janeiro de 1980.
 uint32_t dTempoSeg : é o tempo em segundos que se passou desde o
 começo da semana (em milisegundo)
 Retorno   :  Nenhum.

 *******************************************************************************/
void RTCConverteGPSToRTC (int16_t iWeek, uint32_t dTempoSeg)
{
	osStatus status;
	uint32_t dL;
	uint32_t dN;
	uint32_t dI;
	uint32_t dJ;
	uint32_t dK;

	uint8_t bErr;
	uint8_t bVar;
	uint8_t bAux;
	uint16_t wAux;

	//Pega o mutex antes de acessar dados compartilhados:
	status = WAIT_MUTEX(UOS_MTX_sDataHora, osWaitForever);
	ASSERT(status == osOK);

	memset(&GPS_sDataHoraLocal.abHoraAtualGPS, 0x00,
		sizeof(GPS_sDataHoraLocal.abHoraAtualGPS));
	// encontra os dias que se passaram nesta semana
	bVar = 0;
	//converte de milisegundos para segundos
	dK = (dTempoSeg / 1000);
	while (dK >= 86400)
	{
		dK = dK - 86400;
		bVar++;
	}

	// encontra o nº de dias q se passou desde 6 de janeiro de 1980
	bAux = 6;
	dK = iWeek;
	while (bAux > 0)
	{
		dK += iWeek;
		bAux--;
	}

	// transforma em Julian Date
	dL = JD_GPS_TIME + dK + bVar;

	dL = dL + 68569;
	dN = ((4 * dL) / 146097);
	dL = dL - ((146097 * dN + 3) / 4);
	dI = (4000 * (dL + 1)) / 1461001;
	dL = dL - ((1461 * dI) / 4) + 31;
	dJ = (80 * dL) / 2447;
	dK = dL - ((2447 * dJ) / 80);
	dL = dJ / 11;
	dJ = dJ + 2 - (12 * dL);
	dI = (100 * (dN - 49)) + dI + dL;

	// Deixa somente 2 digitos no ano
	if (dI > 2000)
	{
		dI = dI - 2000;
	}

	// Transforma o ANO para BCD
	bVar = dI;
	bAux = bVar / 10;
	bVar = bVar - (bAux * 10);
	bAux <<= 4;
	bAux |= bVar;
	GPS_sDataHoraLocal.abHoraAtualGPS[7] = bAux;  // ano

	// Transforma o MÊS para BCD
	bVar = dJ;
	bAux = bVar / 10;
	bVar = bVar - (bAux * 10);
	bAux <<= 4;
	bAux |= bVar;
	GPS_sDataHoraLocal.abHoraAtualGPS[6] = bAux;  // mes

	// Transforma o DIA para BCD
	bVar = dK;
	bAux = bVar / 10;
	bVar = bVar - (bAux * 10);
	bAux <<= 4;
	bAux |= bVar;
	GPS_sDataHoraLocal.abHoraAtualGPS[5] = bAux;  // dia

	//converte de milisegundos para segundos
	dK = (dTempoSeg / 1000);

	bVar = 0;
	while (dK >= 86400)
	{
		dK = dK - 86400;
		bVar++;
	}

	// Calcula o dia da semana
	GPS_sDataHoraLocal.abHoraAtualGPS[4] = bVar + 1;  // dia da semana

	// calcula a hora do dia, q é o total de hrs da semana menos o dia da semana
	bVar = 0;
	while (dK >= 3600)
	{
		dK = dK - 3600;
		bVar++;
	}

	// transforma para BCD
	bAux = bVar / 10;   // Calcula 2º nibble
	bVar = bVar - (bAux * 10);
	bAux <<= 4;
	bVar |= bAux;
	GPS_sDataHoraLocal.abHoraAtualGPS[3] = bVar;  //hora

	wAux = dK;
	// Calcula o minuto
	bVar = 0;
	while (wAux >= 60)
	{
		wAux = wAux - 60;
		bVar++;
	}

	//Transforma para BCD
	bAux = bVar / 10;
	bVar = bVar - bAux * 10;
	bAux <<= 4;
	bAux |= bVar;

	GPS_sDataHoraLocal.abHoraAtualGPS[2] = bAux;   //minuto

	// Calcula o segundos
	bVar = wAux;

	//Transforma para BCD
	bAux = bVar / 10;
	bVar = bVar - bAux * 10;
	bAux <<= 4;
	bAux |= bVar;
	GPS_sDataHoraLocal.abHoraAtualGPS[1] = bAux;    // segundos

	//Devolve o mutex:
	status = RELEASE_MUTEX(UOS_MTX_sDataHora);
	ASSERT(status == osOK);
	(void)bErr;
}

/*******************************************************************************
 unsigned char IdentMsgRxGPS( void )

 Descrição : rotina que identifica a mensagem recebida pelo GPS. Esta rotina
 é chamada pela rotina ProcessaBufferRxGPS. A rotina processa
 a mensagem contida no buffer sBufMsgProcRxGps.abBuf. Neste
 buffer a mensagem já foi previamente processada e contêm somente
 o ID e dados, sem os caracteres de terminação e byte stuffing.
 Parâmetros: nenhum
 Retorno   : nenhum
 *******************************************************************************/
void GPS_vIdentMsgRxGPS (void)
{
	osStatus status;
	uint8_t bErr;
	uint8_t bI;
	uint8_t bTamanho;
	uint8_t bChecksum_A = 0;
	uint8_t bChecksum_B = 0;

	//Pega o mutex antes acessar dados compartilhados:
	status = WAIT_MUTEX(GPS_MTX_sEntradas, osWaitForever);
	ASSERT(status == osOK);

	//Calcula o tamanho da mensagem com os 4 byte iniciais ( 2 sinc, 1 classe e 1 ID)
	bTamanho = (4 + wDataLength);

	//Calcula o checksum
	for (bI = 0; bI < bTamanho; bI++)
	{
		bChecksum_A = bChecksum_A + abBufMsgProcRxGPS[bI];
		bChecksum_B = bChecksum_B + bChecksum_A;
	}

	//Verifica se o Checksum calculado é igual ao recebido
	if ((bChecksum_A == abBufMsgProcRxGPS[bTamanho])
		&& (bChecksum_B == abBufMsgProcRxGPS[bTamanho + 1]))
	{
		//verifica o id de Classe do pacote
		switch (abBufMsgProcRxGPS[0])
		{
			//Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used
			case NAV:
			{
				//verifica o id da mensagem
				switch (abBufMsgProcRxGPS[1])
				{
					//Geodetic Position Solution
					case NAV_POSLLH:
					{
						//Verifica se o tamanho dos dados está correto
						if (wDataLength == 28)
						{
							memcpy(&GPS_sDadosGPS.lLon,
								&(abBufMsgProcRxGPS[8]),
								sizeof(GPS_sDadosGPS.lLon));
							memcpy(&GPS_sDadosGPS.lLat,
								&(abBufMsgProcRxGPS[12]),
								sizeof(GPS_sDadosGPS.lLat));
							memcpy(&GPS_sDadosGPS.dHAcc,
								&(abBufMsgProcRxGPS[24]),
								sizeof(GPS_sDadosGPS.dHAcc));
							memcpy(&GPS_sDadosGPS.dVAcc,
								&(abBufMsgProcRxGPS[28]),
								sizeof(GPS_sDadosGPS.dVAcc));

							//Reinicia o contador
							GPS_sTimeoutMsg.bNavPos = 5;
							GPS_sGerenciaTimeoutMsg.dNavPos = 0;
						}

						break;
					}

					case NAV_SOL: //Navigation Solution Information
					{
						//Verifica se o tamanho dos dados está correto
						if (wDataLength == 52)
						{
							memcpy(&GPS_sDadosGPS.eGpsFix,
								&(abBufMsgProcRxGPS[14]),
								sizeof(GPS_sDadosGPS.eGpsFix));
							memcpy(&GPS_sDadosGPS.bFlagsFix,
								&(abBufMsgProcRxGPS[15]),
								sizeof(GPS_sDadosGPS.bFlagsFix));
							memcpy(&GPS_sDadosGPS.wPDOP,
								&(abBufMsgProcRxGPS[48]),
								sizeof(GPS_sDadosGPS.wPDOP));
							memcpy(&GPS_sDadosGPS.bNSV,
								&(abBufMsgProcRxGPS[51]),
								sizeof(GPS_sDadosGPS.bNSV));

							GPS_sTimeoutMsg.bNavSol = 10;
							GPS_sGerenciaTimeoutMsg.dNavSol = 0;
						}
						break;
					}

					case NAV_VELNED: //Velocity Solution in NED
					{
						//Verifica se o tamanho dos dados está correto
						if (wDataLength == 36)
						{
							//Se não tem sensor de velocidade do simulador instalado
							if (CAN_bSensorSimulador == false)
							{
								//Copia dados de velocidade recebidos do GPS
								memcpy(&GPS_sDadosGPS.lVelNorth,
									&(abBufMsgProcRxGPS[8]),
									sizeof(GPS_sDadosGPS.lVelNorth));
								memcpy(&GPS_sDadosGPS.lVelEast,
									&(abBufMsgProcRxGPS[12]),
									sizeof(GPS_sDadosGPS.lVelEast));
								memcpy(&GPS_sDadosGPS.dGroundSpeed,
									&(abBufMsgProcRxGPS[24]),
									sizeof(GPS_sDadosGPS.dGroundSpeed));
								memcpy(&GPS_sDadosGPS.dSpeedAcc,
									&(abBufMsgProcRxGPS[32]),
									sizeof(GPS_sDadosGPS.dSpeedAcc));
							}
							else //Senão...
							{
								//Utiliza a velocidade lida do sensor do simulador
								GPS_sDadosGPS.dGroundSpeed = CAN_dLeituraSensor;
								//CAN_dLeituraSensor = 0;
							}

							GPS_sTimeoutMsg.bNavVelNed = 5;
							GPS_sGerenciaTimeoutMsg.dNavVelNed = 0;
						}
						break;

					}

					case NAV_TIMEGPS: //GPS Time Solution
					{
						//Verifica se o tamanho dos dados está correto
						if (wDataLength == 16)
						{

							memcpy(&GPS_sDadosGPS.dTOW,
								&(abBufMsgProcRxGPS[4]),
								sizeof(GPS_sDadosGPS.dTOW));
							memcpy(&GPS_sDadosGPS.iWeek,
								&(abBufMsgProcRxGPS[12]),
								sizeof(GPS_sDadosGPS.iWeek));
							memcpy(&GPS_sDadosGPS.cUTCOff,
								&(abBufMsgProcRxGPS[14]),
								sizeof(GPS_sDadosGPS.cUTCOff));
							memcpy(&GPS_sDadosGPS.bValid,
								&(abBufMsgProcRxGPS[15]),
								sizeof(GPS_sDadosGPS.bValid));

							GPS_sTimeoutMsg.bNavTimeGps = 4;
							GPS_sGerenciaTimeoutMsg.dNavTimeGps = 0;

							// AJUSTA RTC, se os dados de TOW, Week e UTCOff forem válidos
							if ((GPS_sDadosGPS.bValid & GPS_VALID_TIME)
								== GPS_VALID_TIME)
							{
								// Converte para formato RTC
								RTCConverteGPSToRTC(GPS_sDadosGPS.iWeek,
									GPS_sDadosGPS.dTOW);

								GPS_vCalcHoraLocal();

							}
						}

						break;

					}

					default:
					{
						break;
					}
				}
				break;
			} //Fim do case NAV

			//Ack/Nack Messages: as replies to CFG Input Messages
			case ACK:
			{

				//verifica o id da mensagem
				switch (abBufMsgProcRxGPS[1])
				{

					//Message Acknowledged
					case ACK_ACK:
					{
						//Verifica se o tamanho dos dados está correto
						if (wDataLength == 2)
						{
							if (GPS_sDadosGPS.bConfigura_FIM == false)
							{
								switch (abBufMsgProcRxGPS[5])
								{

									case CFG_PRT:
									{
										if ((GPS_eStatusConfig == VerificaPort)
											&& (GPS_eConfigura
												== VerificaPort)
											&& (GPS_sDadosGPS.dBaud > 0))
										{
											GPS_eConfigura = CfgPort;
										}
										else
										{
											if ((GPS_eStatusConfig == CfgPort)
												&& (GPS_sDadosGPS.dBaud
													== 115200))
											{
												GPS_eConfigura = CfgTimePulse;
											}
										}

										break;
									}

									case CFG_TP5:
									{
										if (GPS_eStatusConfig == CfgTimePulse)
										{
											GPS_eConfigura = CfgRateSettings;
										}

										break;
									}

									case CFG_RATE:
									{
										if (GPS_eStatusConfig
											== CfgRateSettings)
										{
											GPS_eConfigura = CfgAntenna;
										}

										break;
									}

									case CFG_ANT:
									{

										if (GPS_eStatusConfig == CfgAntenna)
										{
											GPS_eConfigura = CfgNavigation;
										}
										break;
									}

									case CFG_NAV5:
									{

										if (GPS_eStatusConfig == CfgNavigation)
										{
											GPS_eConfigura = CfgMsgNavPosllh;
										}
										break;
									}

									case CFG_MSG:
									{

										switch (GPS_eConfigura)
										{
											case CfgMsgNavPosllh:
											{
												if (GPS_eStatusConfig
													== CfgMsgNavPosllh)
												{
													GPS_eConfigura =
														CfgMsgNavSol;
												}
												break;
											}
											case CfgMsgNavSol:
											{
												if (GPS_eStatusConfig
													== CfgMsgNavSol)
												{
													GPS_eConfigura =
														CfgMsgNavVelned;
												}
												break;
											}
											case CfgMsgNavVelned:
											{
												if (GPS_eStatusConfig
													== CfgMsgNavVelned)
												{
													GPS_eConfigura =
														CfgMsgNavTimeGps;
												}
												break;
											}
											case CfgMsgNavTimeGps:
											{
												if (GPS_eStatusConfig
													== CfgMsgNavTimeGps)
												{
													GPS_eConfigura =
														CfgMsgMonHw;
												}
												break;
											}
											case CfgMsgMonHw:
											{

												if (GPS_eStatusConfig
													== CfgMsgMonHw)
												{
													//Atualiza o status da Bateria
													//OneWire_LeID();  //Retirada esta função, pois utiliza o timer 0, que também é utilizado pela rotina do GPS,

													//Finaliza a configuração do GPS
													GPS_sDadosGPS.bConfigura_FIM =
													true;
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
						break;
					}

					default:
						break;
				}

				break;
			} //Fim case ACK

			//Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc.
			case CFG:
			{
				//verifica o id da mensagem
				switch (abBufMsgProcRxGPS[1])
				{
					case CFG_PRT:
					{
						//Verifica se o tamanho dos dados está correto
						if (wDataLength == 20)
						{
							memcpy(&GPS_sDadosGPS.bPorta,
								&(abBufMsgProcRxGPS[4]),
								sizeof(GPS_sDadosGPS.bPorta));
							memcpy(&GPS_sDadosGPS.dBaud,
								&(abBufMsgProcRxGPS[12]),
								sizeof(GPS_sDadosGPS.dBaud));
							memcpy(&GPS_sDadosGPS.wInProto,
								&(abBufMsgProcRxGPS[16]),
								sizeof(GPS_sDadosGPS.wInProto));
							memcpy(&GPS_sDadosGPS.wOutProto,
								&(abBufMsgProcRxGPS[18]),
								sizeof(GPS_sDadosGPS.wOutProto));
						}
						break;
					}

					default:
						break;
				}

				break;
			} //Fim case CFG

			//Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status
			case MON:
			{
				//verifica o id da mensagem
				switch (abBufMsgProcRxGPS[1])
				{

					case MON_VER: //Receiver/Software Version
					{
						//Verifica se o tamanho dos dados está correto para SW até versão 5.00 (40 bytes)ou
						//Verifica se o tamanho dos dados está correto para SW versão 6.00 (70 bytes)
						if ((wDataLength == 40) || (wDataLength == 70)
							|| (wDataLength == 250))
						{
							memcpy(&GPS_sDadosGPS.bSwVersion,
								&(abBufMsgProcRxGPS[4]),
								sizeof(GPS_sDadosGPS.bSwVersion));

							//Versão de HW e de ROM... Atualmente não são utilizadas
							/*memcpy( &GPS_sDadosGPS.bHwVersion, &(abBufMsgProcRxGPS[ 34 ]), sizeof(GPS_sDadosGPS.bHwVersion));

							 //Verifica se o tamanho dos dados está correto para SW versão 6.00
							 if( wDataLength == 70 )
							 {
							 memcpy( &GPS_sDadosGPS.bRomVersion, &(abBufMsgProcRxGPS[ 44 ]), sizeof(GPS_sDadosGPS.bRomVersion));
							 }*/

							GPS_sTimeoutMsg.bMonVer = true;
							GPS_sGerenciaTimeoutMsg.dMonVer = 0;
						}

						break;
					}

					case MON_HW: //Hardware Status
					{
						//Verifica se o tamanho dos dados está correto
						if ((wDataLength == 68) || (wDataLength == 60))
						{
							memcpy(&GPS_sDadosGPS.eStsAntena,
								&(abBufMsgProcRxGPS[24]),
								sizeof(GPS_sDadosGPS.eStsAntena));

							GPS_sTimeoutMsg.bMonHw = 10;
							GPS_sGerenciaTimeoutMsg.dMonHw = 0;
						}
						break;
					}

					default:
						break;

				}

				break;
			} //Fim case Report Packet MON

			default:
				break;

		} // Fim do switch ID mensagem recebida.
	}

	//Devolve o mutex:
	status = RELEASE_MUTEX(GPS_MTX_sEntradas);
	ASSERT(status == osOK);

} // Fim da função

/*******************************************************************************
 void GPS_vProcessBufferRx( void )

 Descrição : rotina de processamento do buffer em anel, dos dados recebidos do
 GPS.
 A rotina processa o pacote, separando o DLE, ETX, e byte stuffing
 do ID e dos dados.

 Parâmetros: nenhum
 Retorno   : nenhum
 *******************************************************************************/
void GPS_vProcessBufferRx (void)
{
	osStatus status;
	uint8_t data;

	// Process all received bytes...
	while (!RingBuffer_IsEmpty(&GPSRxBuffer))
	{
		RingBuffer_Pop(&GPSRxBuffer, &data);

		//máquina de estados de recepção:
		switch (GPS_bEstadoRxGPS)
		{
			//Aguardando receber um o 1º caracter de sincronismo.
			case GPS_ERX_W_SYNC_1:
			{
				// Se recebeu o 1º caracter de sincronismo:
				if (data == SYNC_CHAR_1)
				{
					//Vai para aguardar o 2º caracter de sincronismo:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_2;
				}
				break;
			} //Fim case GPS_ERX_W_SYNC_1.

			//Aguardando receber um o 2º caracter de sincronismo.
			case GPS_ERX_W_SYNC_2:
			{
				// Se recebeu o 2º caracter de sincronismo:
				if (data == SYNC_CHAR_2)
				{
					//Vai para aguardar o byte de Classe
					GPS_bEstadoRxGPS = GPS_ERX_W_CLASS;
				}
				break;
			} //Fim case GPS_ERX_W_SYNC_2.

			//Aguardando receber um Message CLASS:
			case GPS_ERX_W_CLASS:
			{
				//Se recebeu um ID CLASS inválido:
				if ((data == SYNC_CHAR_1) || (data == SYNC_CHAR_2))
				{
					//Volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				}
				//Se o CLASS é válido:
				else
				{
					// Reinicia o nº de dados no buffer de msg processadas.
					wPosMsgProcRxGps = 0;

					//Salva o CLASS no buffer de recepção:
					abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

					// Incrementa o nº de dados processados.
					wPosMsgProcRxGps++;

					//Vai aguardar o byte de ID:
					GPS_bEstadoRxGPS = GPS_ERX_W_ID;
				}
				break;
			} //Fim case GPS_ERX_W_CLASS.

			//Aguardando receber um Message ID:
			case GPS_ERX_W_ID:
			{
				//Se recebeu um ID inválido:
				if ((data == SYNC_CHAR_1) || (data == SYNC_CHAR_2))
				{
					//Volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;

				}
				//Se o ID é válido:
				else
				{
					//se tem espaço no buffer de mensagens processadas
					if (wPosMsgProcRxGps < GPS_wTAM_BUF_ANEL)
					{
						//Salva o ID no buffer de recepção:
						abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

						// Incrementa o nº de dados processados.
						wPosMsgProcRxGps++;

						//Vai aguardar os bytes de tamanho do pacote:
						GPS_bEstadoRxGPS = GPS_ERX_W_LEN_1;
					}
					else
					{
						//Volta a aguardar início de pacote:
						GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
					}
				}
				break;
			} //Fim case GPS_ERX_W_ID.

			//Aguardando receber um Lenght of Payload Byte1:
			case GPS_ERX_W_LEN_1:
			{
				//se tem espaço no buffer de mensagens processadas
				if (wPosMsgProcRxGps < GPS_wTAM_BUF_ANEL)
				{
					//Salva o Lenght of Payload Byte1 no buffer de recepção:
					abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

					//Armazena o tamanho dos dados
					wDataLength = data;

					// Incrementa o nº de dados processados.
					wPosMsgProcRxGps++;
					//Vai aguardar os bytes de tamanho do pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_LEN_2;
				}
				else
				{
					//Volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				}
				break;
			} //Fim case GPS_ERX_W_LEN_1.

			//Aguardando receber um Lenght of Payload Byte2:
			case GPS_ERX_W_LEN_2:
			{
				//se tem espaço no buffer de mensagens processadas
				if (wPosMsgProcRxGps < GPS_wTAM_BUF_ANEL)
				{
					//Salva o Lenght of Payload Byte2 no buffer de recepção:
					abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

					//Armazena o tamanho dos dados
					wDataLength |= ((data) << 8);
					wDataByte = 0;

					// Incrementa o nº de dados processados.
					wPosMsgProcRxGps++;
					//Vai aguardar os bytes de Dados:
					GPS_bEstadoRxGPS = GPS_ERX_W_DATA;
				}
				else
				{
					//Volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				}
				break;
			} //Fim case GPS_ERX_W_LEN_2.

			//Aguardando um byte de dados:
			case GPS_ERX_W_DATA:
			{
				//se tem espaço no buffer de mensagens processadas
				if (wPosMsgProcRxGps < GPS_wTAM_BUF_ANEL)
				{
					//Salva o dado no buffer de mensagens processadas
					abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

					// Incrementa o nº de dados processados.
					wPosMsgProcRxGps++;

					//Incrementa o número de bytes de dados
					wDataByte++;

					//Se a quantidade de bytes de dados recebidos for maior ou igual
					//ao tamanho da mensagem...
					if ((wDataByte) >= wDataLength)
					{
						//Vai para aguardar um byte de Checksum A
						GPS_bEstadoRxGPS = GPS_ERX_W_CK_A;
					}
					else
					{
						//Volta para aguardar os bytes de Dados:
						GPS_bEstadoRxGPS = GPS_ERX_W_DATA;
					}
				}
				else
				{
					//Ignora este pacote e volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				}
				break;
			} //Fim case GPS_ERX_W_DATA.

			//Aguardando um Checksum Byte1:
			case GPS_ERX_W_CK_A:
			{
				//se tem espaço no buffer de mensagens processadas
				if (wPosMsgProcRxGps < GPS_wTAM_BUF_ANEL)
				{
					//Salva o dado no buffer de mensagens processadas
					abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

					// Incrementa o nº de dados processados.
					wPosMsgProcRxGps++;

					//Vai para aguardar um byte de checksum B
					GPS_bEstadoRxGPS = GPS_ERX_W_CK_B;
				}
				else // se não tem mais espaço
				{
					//Ignora este pacote e volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				}

				break;

			} //Fim case GPS_ERX_W_CK_A

			//Aguardando um Checksum Byte2:
			case GPS_ERX_W_CK_B:
			{

				//se tem espaço no buffer de mensagens processadas
				if (wPosMsgProcRxGps < GPS_wTAM_BUF_ANEL)
				{
					//Salva o dado no buffer de mensagens processadas
					abBufMsgProcRxGPS[wPosMsgProcRxGps] = data;

					// Incrementa o nº de dados processados.
					wPosMsgProcRxGps++;

					// identifica o pacote recebido
					GPS_vIdentMsgRxGPS();

					//volta a aguardar início de um novo pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;

				}
				else // se não tem mais espaço
				{
					//Ignora este pacote e volta a aguardar início de pacote:
					GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				}

				break;
			} //Fim case GPS_ERX_W_CK_B:

			default:
			{
				//Se a máquina está perdida, volta a aguardar início de pacote:
				GPS_bEstadoRxGPS = GPS_ERX_W_SYNC_1;
				break;
			} //Fim default.
		} //Fim switch ( GPS_bEstadoRxGPS ).
	}
}  // Fim da função ProcessaBufferRxGPS

/*******************************************************************************

 void GPS_vDataHoraAdicionaOffset( uint8_t *pabDataHora, FP32 fOffset )

 Descrição : Esta função adiciona o lOffset, que pode ser positivo ou
 negativo, à hora apontada por pabDataHora.
 Parâmetros: pabDataHoraGPS - contém um ponteiro para a data/hora.
 fOffset - o número de segundos à adicionar.
 Retorno   : nenhum.

 *******************************************************************************/
void GPS_vDataHoraAdicionaOffset (uint8_t *pabDataHora, float lOffset)
{
	uint8_t bNegativo, bAno;
	uint32_t dOffset;
	int16_t iSegundos, iMinutos, iHoras, iMes, iAcc, iCarry;
	uint8_t abQuantDiasMeses[12] = { 31,  // janeiro
		28,  // fevereiro
		31,  // março
		30,  // abril
		31,  // maio
		30,  // junho
		31,  // julho
		31,  // agosto
		30,  // setembro
		31,  // outubro
		30,  // novembro
		31   // dezembro
		};

	//Verifica se o ano é bisexto
	bAno = (((pabDataHora[7] >> 4) * 10) + (pabDataHora[7] & 0x0F));
	//divisível por 4 ?
	if ((bAno & 0x03) == 0)
	{
		//Múltiplo de 100 ?
		if (((2000 + bAno) % 100) == 0)
		{
			//Múltiplo de 400 ?
			if (((2000 + bAno) % 400) == 0)
			{
				//Ano bisexto:
				abQuantDiasMeses[1] = 29;
			}
		}
		else
		{
			abQuantDiasMeses[1] = 29;
		}
	}

	//Separa o offset em horas, minutos e segundos:
	if (lOffset < 0)
	{
		bNegativo = true;
		dOffset = (uint32_t)(-lOffset);
	}
	else
	{
		bNegativo = false;
		dOffset = (uint32_t)(lOffset);
	}
	iHoras = (dOffset / 3600);
	iMinutos = ((dOffset % 3600) / 60);
	iSegundos = (dOffset % 60);
	if (bNegativo)
	{
		iHoras = (-iHoras);
		iMinutos = (-iMinutos);
		iSegundos = (-iSegundos);
	}

	// ordem dos bytes em pabDataHora[]:    e em UOS_sConfiguracao.sHoraVerao.abDataInicio
	// 0 - centésimos de segundos           n/a
	// 1 - seg                              0
	// 2 - min                              1
	// 3 - hora                             2
	// 4 - dia da semana ( domingo = 1 )    n/a
	// 5 - dia                              3
	// 6 - mês                              4
	// 7 - ano                              5

	//Calcula com se estivesse no horário de verão. Portanto
	//GPS_sDataHoraLocal.abDataHoraBCDLocalHV =
	//   GPS_sDataHoraLocal.abDataHoraBCDLocal + 1 hora;

	//Deposito para o "vai um":
	iCarry = 0;
	//Os centésimos de segundo não são alterados. Começa adicionando o
	//offset aos segundos:
	iAcc = (((pabDataHora[1] >> 4) * 10) + (pabDataHora[1] & 0x0F));
	iAcc += iSegundos;
	if (iAcc < 0)
	{
		iCarry = -1;
		iAcc += 60;
	}
	else
	{
		iCarry = (iAcc / 60);
		iAcc %= 60;
	}
	pabDataHora[1] = (((iAcc / 10) << 4) + (iAcc % 10));

	//Minutos:
	iAcc = (((pabDataHora[2] >> 4) * 10) + (pabDataHora[2] & 0x0F));
	iAcc += iMinutos + iCarry;
	if (iAcc < 0)
	{
		iCarry = -1;
		iAcc += 60;
	}
	else
	{
		iCarry = (iAcc / 60);
		iAcc %= 60;
	}
	pabDataHora[2] = (((iAcc / 10) << 4) + (iAcc % 10));

	//Horas:
	iAcc = (((pabDataHora[3] >> 4) * 10) + (pabDataHora[3] & 0x0F));
	iAcc += iHoras + iCarry;
	if (iAcc < 0)
	{
		iCarry = -1;
		iAcc += 24;
	}
	else
	{
		iCarry = (iAcc / 24);
		iAcc %= 24;
	}
	pabDataHora[3] = (((iAcc / 10) << 4) + (iAcc % 10));

	//Se mudou de dia, muda o dia da semana:
	iAcc = (((pabDataHora[4] >> 4) * 10) + (pabDataHora[4] & 0x0F));
	if (iCarry != 0)
	{
		if (iCarry > 0)
		{
			iAcc = (iAcc < 7) ? iAcc + 1 : 1;
		}
		else
		{
			iAcc = (iAcc > 1) ? iAcc - 1 : 7;
		}
	}
	pabDataHora[4] = (((iAcc / 10) << 4) + (iAcc % 10));

	//Dias:
	iAcc = (((pabDataHora[5] >> 4) * 10) + (pabDataHora[5] & 0x0F));
	iMes = (((pabDataHora[6] >> 4) * 10) + (pabDataHora[6] & 0x0F));
	iAcc += iCarry;
	if (iAcc < 1)
	{
		iCarry = -1;
		iAcc = abQuantDiasMeses[((iMes > 1) ? iMes - 1 : 12) - 1];
	}
	else
	{
		iCarry = (iAcc / (abQuantDiasMeses[iMes - 1] + 1));
		iAcc %= (abQuantDiasMeses[iMes - 1] + 1);
		if (iAcc == 0)
		{
			iAcc = 1;
		}
	}
	pabDataHora[5] = (((iAcc / 10) << 4) + (iAcc % 10));

	//Meses:
	iAcc = iMes;
	iAcc += iCarry;
	if (iAcc < 1)
	{
		iCarry = -1;
		iAcc += 12;
	}
	else
	{
		iCarry = (iAcc / 13);
		iAcc %= 13;

		if (iAcc == 0)
		{
			iAcc = 1;
		}
	}
	pabDataHora[6] = (((iAcc / 10) << 4) + (iAcc % 10));

	//Anos:
	iAcc = (((pabDataHora[7] >> 4) * 10) + (pabDataHora[7] & 0x0F));
	iAcc += iCarry;
	pabDataHora[7] = (((iAcc / 10) << 4) + (iAcc % 10));

}

/*******************************************************************************

 uint32_t GPS_dCalcHoraSistema( uint8_t *pabDataHora )

 Descrição : Esta função calcula a hora local em segundos desde 01/01/1970 a
 partir da hora dada em pabDataHora no formato:
 0 - centésimos de segundos
 1 - seg
 2 - min
 3 - hora
 4 - dia da semana ( domingo = 1 )
 5 - dia
 6 - mês
 7 - ano ( 2000 = 0 )
 Parâmetros: pabDataHora - data/hora a converter
 Retorno   : resultado da conversão em segundos.

 *******************************************************************************/
uint32_t GPS_dCalcHoraSistema (uint8_t *pabDataHora)
{
	struct tm sDataHora;
	uint32_t dDataHora;

	// ordem dos bytes em abHoraRTC[]:
	// 0 - centésimos de segundos
	// 1 - seg
	// 2 - min
	// 3 - hora
	// 4 - dia da semana ( domingo = 1 )
	// 5 - dia
	// 6 - mês
	// 7 - ano ( 2000 = 0 )

	//seconds after the minute (from 0).
	sDataHora.tm_sec = (((pabDataHora[1] & 0xF0) >> 4) * 10)
		+ (pabDataHora[1] & 0x0F);
	//minutes after the hour (from 0).
	sDataHora.tm_min = (((pabDataHora[2] & 0xF0) >> 4) * 10)
		+ (pabDataHora[2] & 0x0F);
	//hour of the day (from 0).
	sDataHora.tm_hour = (((pabDataHora[3] & 0xF0) >> 4) * 10)
		+ (pabDataHora[3] & 0x0F);
	//day of the month (from 1).
	sDataHora.tm_mday = (((pabDataHora[5] & 0xF0) >> 4) * 10)
		+ (pabDataHora[5] & 0x0F);
	//month of the year (from 0).
	sDataHora.tm_mon = (((pabDataHora[6] & 0xF0) >> 4) * 10)
		+ (pabDataHora[6] & 0x0F) - 1;
	//years since 1900 (from 0).
	sDataHora.tm_year = (((pabDataHora[7] & 0xF0) >> 4) * 10)
		+ (pabDataHora[7] & 0x0F) + 100;
	//days since Sunday (from 0).
	sDataHora.tm_wday = 0;
	//day of the year (from 0).
	sDataHora.tm_yday = 0;
	//Daylight Saving Time flag.
	sDataHora.tm_isdst = 0;

	//Converte a hora em segundos desde 01/01/1970:
	dDataHora = mktime(&sDataHora);

	return dDataHora;

} //Fim função GPS_vCalcHoraSistema.

/*******************************************************************************

 void GPS_vCalcHoraLocal( void )

 Descrição : Esta função calcula a hora local em
 GPS_sDataHoraLocal.abDataHoraBCDLocal a partir da hora GPS,
 do offset da hora GPS e da diferença de fuso-horário configurada.
 Se estivermos dentro do período de horário de verão
 GPS_sDataHoraLocal.abDataHoraBCDLocalHV será calculada como
 abDataHoraBCDLocal + 1 hora. Caso contrário, os dois valores
 serão iguais.
 Parâmetros: nenhum.
 Retorno   : nenhum.

 *******************************************************************************/
void GPS_vCalcHoraLocal (void)
{
	osStatus status;
	uint8_t bErr;
	int32_t lOffset;
	uint8_t bVerao;
	uint8_t abHoraGPS[8];
	uint8_t abHoraGPSVerao[8];
	uint32_t dDataHora;

	//Pega o mutex antes de acessar dados compartilhados:
	status = WAIT_MUTEX(UOS_MTX_sDataHora, osWaitForever);
	ASSERT(status == osOK);

	//Fuso-horario:
	lOffset = UOS_sConfiguracao.sGPS.lFusoHorario;
	//Offset entre a hora GPS e a hora UTC:
	lOffset -= GPS_sDadosGPS.cUTCOff;
	//O RTC está ajustado com a hora GPS:
	memcpy(abHoraGPS, GPS_sDataHoraLocal.abHoraAtualGPS, sizeof(abHoraGPS));

	//Soma o offset à hora atual:
	GPS_vDataHoraAdicionaOffset(abHoraGPS, lOffset);

	//Calcula a data/hora do sistema em segundos desde 01/01/1970:
	dDataHora = GPS_dCalcHoraSistema(abHoraGPS);

	//Copia este horário no horário de verão:
	memcpy(abHoraGPSVerao, abHoraGPS, sizeof(abHoraGPS));
	//Se estamos no hrário de verão, devemos somar uma hora ao relógio HV:
	bVerao = UOS_sConfiguracao.sGPS.bHorarioVerao;
	if (bVerao == true)
	{
		//Soma uma hora à hora atual:
		GPS_vDataHoraAdicionaOffset( /*uint8_t **/abHoraGPSVerao, 3600.0);
	}

	//Atualiza a estrutura da hora local:
	memcpy(GPS_sDataHoraLocal.abDataHoraBCDLocal, abHoraGPS,
		sizeof(GPS_sDataHoraLocal.abDataHoraBCDLocal));
	memcpy(GPS_sDataHoraLocal.abDataHoraBCDLocalHV, abHoraGPSVerao,
		sizeof(GPS_sDataHoraLocal.abDataHoraBCDLocalHV));
	memcpy(&GPS_sDataHoraLocal.dDataHoraSistema, &dDataHora,
		sizeof(GPS_sDataHoraLocal.dDataHoraSistema));

	//Devolve o mutex:
	status = RELEASE_MUTEX(UOS_MTX_sDataHora);
	ASSERT(status == osOK);
	(void)bErr;

} //Fim função GPS_vCalcHoraLocal

/*******************************************************************************

 void GPS_vCalcHoraUTC( uint8_t *pabDataHora )

 Descrição : Esta função corrige a hora na estrutura pabDataHora para a hora
 UTC, levando em conta se estamos dentro do período de horário de
 verão
 Parâmetros: pabDataHora.
 Retorno   : nenhum.

 *******************************************************************************/
void GPS_vCalcHoraUTC (uint8_t *pabDataHora)
{
	int32_t lOffset;
	uint8_t bVerao;

	//Verifica se este é um horário de verão:
	bVerao = UOS_sConfiguracao.sGPS.bHorarioVerao;
	//Se estamos no horário de verão, devemos subtrair uma hora:
	if (bVerao == true)
	{
		//Soma uma hora à hora atual:
		GPS_vDataHoraAdicionaOffset(pabDataHora, -3600.0);
	}

	//Fuso-horario:
	lOffset = UOS_sConfiguracao.sGPS.lFusoHorario;
	//Offset entre a hora GPS e a hora UTC:
	lOffset -= GPS_sDadosGPS.cUTCOff;
	//Na direção Local para UTC
	lOffset = -lOffset;

	//Soma o offset à hora atual:
	GPS_vDataHoraAdicionaOffset(pabDataHora, lOffset);

} //Fim função GPS_vCalcHoraUTC

/*******************************************************************************

 uint32_t GPS_dDataHoraSistema( void )

 Descrição : Esta função retorna a data/hora local sem correção para horário de
 verão, em segundos desde 01/01/1970.
 Parâmetros: nenhum.
 Retorno   : segundos desde 01/01/1970.

 *******************************************************************************/
uint32_t GPS_dDataHoraSistema (void)
{
	osStatus status;
	uint8_t bErr;
	uint32_t dDataHoraSistema;

	//Pega o mutex antes de acessar dados compartilhados:
	status = WAIT_MUTEX(UOS_MTX_sDataHora, osWaitForever);
	ASSERT(status == osOK);

	//Copia o valor da data/hora:
	dDataHoraSistema = GPS_sDataHoraLocal.dDataHoraSistema;

	//Devolve o mutex:
	status = RELEASE_MUTEX(UOS_MTX_sDataHora);
	ASSERT(status == osOK);
	(void)bErr;

	//Retorna a dadta/hora do sistema:
	return dDataHoraSistema;

} //Fim função GPS_dDataHoraSistema

/*******************************************************************************

 void GPS_dDataHoraLocal( UINT8 *pabDataHoraLocal )

 Descrição : Esta função retorna a data/hora local SEM correção para horário de
 verão, em formato BCD em um array de 8 bytes com o seguinte
 conteúdo:
 0 - centésimos de segundos
 1 - seg
 2 - min
 3 - hora
 4 - dia da semana ( domingo = 1 )
 5 - dia
 6 - mês
 7 - ano
 Parâmetros: pabDataHoraLocal - ponteiro para o array que conterá a data/hora.
 Retorno   : nenhum.

 *******************************************************************************/
void GPS_dDataHoraLocal (uint8_t *pabDataHoraLocal)
{
	osStatus status;

	//Pega o mutex antes de acessar dados compartilhados:
	status = WAIT_MUTEX(UOS_MTX_sDataHora, osWaitForever);
	ASSERT(status == osOK);

	//Copia o valor da data/hora local:
	memcpy(pabDataHoraLocal, &(GPS_sDataHoraLocal.abDataHoraBCDLocal), 8);

	//Devolve o mutex:
	status = RELEASE_MUTEX(UOS_MTX_sDataHora);
	ASSERT(status == osOK);

} //Fim função GPS_dDataHoraLocal

/*******************************************************************************

 void GPS_dDataHoraLocalHV( UINT8 *pabDataHoraLocalHV )

 Descrição : Esta função retorna a data/hora local COM correção para horário de
 verão, em formato BCD em um array de 8 bytes com o seguinte
 conteúdo:
 0 - centésimos de segundos
 1 - seg
 2 - min
 3 - hora
 4 - dia da semana ( domingo = 1 )
 5 - dia
 6 - mês
 7 - ano
 Parâmetros: pabDataHoraLocal - ponteiro para o array que conterá a data/hora.
 Retorno   : nenhum.

 *******************************************************************************/
void GPS_dDataHoraLocalHV (uint8_t *pabDataHoraLocalHV)
{
	osStatus status;

	//Pega o mutex antes de acessar dados compartilhados:
	status = WAIT_MUTEX(UOS_MTX_sDataHora, osWaitForever);
	ASSERT(status == osOK);

	//Copia o valor da data/hora local:
	memcpy(pabDataHoraLocalHV, &(GPS_sDataHoraLocal.abDataHoraBCDLocalHV[0]),
		8);

	//Devolve o mutex:
	status = RELEASE_MUTEX(UOS_MTX_sDataHora);
	ASSERT(status == osOK);

} //Fim função GPS_dDataHoraLocalHV

void GPS_vTimerCallbackMtr (void const *arg)
{
	osFlagSet(GPS_sFlagGPS, GPS_FLAG_TIMEOUT_MTR);
}

void GPS_vTimerCallbackTimeout (void const *arg)
{
	osFlagSet(GPS_sFlagGPS, GPS_FLAG_TIME_OUT);
}

void GPS_vTimerCallbackTimeoutEnl (void const *arg)
{
	osFlagSet(GPS_sFlagEnl, GPS_ENL_FLAG_TIME_OUT);
}

/******************************************************************************
 * Function : GPS_vInitDeviceLayer()
 *//**
 * \b Description:
 *
 * This function will init the M2GGPSCOMM instance used by the GPS module.
 *
 * PRE-CONDITION: GPS core initialized
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
eAPPError_s GPS_vInitDeviceLayer (void)
{
	/*Prepare the device */
	pGPSHandle = DEV_open(PERIPHERAL_M2GGPSCOMM);
	ASSERT(pGPSHandle != NULL);

	eError = APP_ERROR_SUCCESS;

	return eError;
}

/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void GPS_vGPSThread (void const *argument)
{
	osStatus status;
	eAPPError_s error;
	osFlags dValorFlag;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("GPS Thread Created");
#endif

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(GPSQueue);

	/* Init the module ring buffer - structure that receive data from device M2GGPSCOMM */
	INITIALIZE_RINGBUFFER(uint8_t, GPSRx, GPS_RX_BUFFER_SIZE);

	/* Init M2GGPSCOMM device for read and write */
	error = GPS_vInitDeviceLayer();
	ASSERT(error == APP_ERROR_SUCCESS);

	//Flags de eventos da interrupção do TIMEPULSE:
	status = osFlagGroupCreate(&GPS_sFlagGPS);
	ASSERT(status == osOK);

	//Flags para indicar o status do sistema:
	status = osFlagGroupCreate(&GPS_sFlagEnl);
	ASSERT(status == osOK);

	//Aloca um timer de sistema para aguardar 5s:
	INITIALIZE_TIMER(GPS_bTimerMtr, osTimerPeriodic);

	// Used to count 125ms after an timepulse interrupt
	INITIALIZE_TIMER(GPS_bTimerTimeout, osTimerPeriodic);

	//Mutex para controle de acesso às estruturas de dados de entrada do GPS:
	INITIALIZE_MUTEX(GPS_MTX_sEntradas);

	//Prepara o protocolo de comunicação para trabalhar:
	memset(&GPS_sCtrlCBASRL, 0x00, sizeof(GPS_tsCtrlEnl));

	GPS_sCtrlCBASRL.psFlagEnl = &GPS_sFlagEnl;
	GPS_sCtrlCBASRL.dEventosEnl = GPS_ENL_FLAG_NENHUM;
	GPS_sCtrlCBASRL.bTimer = GPS_bTimerTimeoutEnl;

//    GPS_sCtrlCBASRL.sCtrlEnl.dTicksWDT    = GPS_wTICKS_WDT;
//    GPS_sCtrlCBASRL.sCtrlEnl.dTicksWAIT   = GPS_wTICKS_WAT;
	GPS_sCtrlCBASRL.dTicksIDLE = ( TICK >> 2);

	//Aloca um timer de sistema para o protocolo de comunicação:
	INITIALIZE_TIMER(GPS_bTimerTimeoutEnl, osTimerPeriodic);

	//      //Muda a prioridade de inicialização para o valor de trabalho:
	//      bErr = OSTaskChangePrio( OS_PRIO_SELF, GPS_TRF_PRINCIPAL_PRIORIDADE );
	//      __assert( bErr == OS_NO_ERR );

	//      //Aguarda o fim da inicialização do sistema:
	//      OSFlagPend( UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, OS_FLAG_WAIT_SET_ALL, 0, &bErr );
	//      __assert( bErr == OS_NO_ERR );


	GPS_eInitGPSPublisher();

	/* Inform Main thread that initialization was a success */
	osSignalSet((osThreadId)argument, MODULE_GPS);

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP, false, false, osWaitForever);

	//Timer sempre ligado:
	START_TIMER(GPS_bTimerTimeoutEnl, GPS_sCtrlCBASRL.dTicksIDLE);

	//Inicia variáveis de configuração
	GPS_eConfigura = VerificaPort;
	GPS_sDadosGPS.bConfigura_FIM = false;
	GPS_eStatusConfig = SemConfiguracao;

	GPS_sTimeoutMsg.bNavPos = 5;
	GPS_sTimeoutMsg.bNavSol = 10;
	GPS_sTimeoutMsg.bNavVelNed = 5;
	GPS_sTimeoutMsg.bNavTimeGps = 4;
	GPS_sTimeoutMsg.bMonVer = false;
	GPS_sTimeoutMsg.bMonHw = 10;

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		GPS_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on any GPS topic or diagnostic topic*/
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		osEvent evt = RECEIVE(GPSQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

		if (evt.status == osEventMessage)
		{
		}
		osDelay(500);
	}
	/* Unreachable */
	osThreadSuspend(NULL);
}
#else
void GPS_vGPSThread (void const *argument)
{}
#endif

#ifndef UNITY_TEST
void GPS_vGPSManagementThread (void const *argument)
{
	osStatus status;
	eAPPError_s error;
	volatile osFlags dValorFlag;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("GPS Management Thread Created");
#endif

	GPS_vDetectThread(&WATCHDOG(GPSMGT), &bGPSMGTThreadArrayPosition,
		(void*)GPS_vGPSManagementThread);
	WATCHDOG_STATE(GPSMGT, WDT_ACTIVE);

	osThreadId xMainID = (osThreadId)argument;
	osSignalSet(xMainID, THREADS_RETURN_SIGNAL(bGPSMGTThreadArrayPosition)); //Task created, inform core

	/* Start the main functions of the application */

	while (1)
	{
		/* Blocks until any data link layer event is receive */
		WATCHDOG_STATE(GPSMGT, WDT_SLEEP);
		// Get the received event
		GPS_sCtrlCBASRL.dEventosEnl |= osFlagWait(
			GPS_sFlagEnl, (GPS_ENL_FLAG_TIME_OUT | GPS_ENL_FLAG_RX_BYTE),
			true,
			false, osWaitForever);
		WATCHDOG_STATE(GPSMGT, WDT_ACTIVE);

		// Read the flag value
		dValorFlag = GPS_sCtrlCBASRL.dEventosEnl;

		// If received any data from data link layer
		if ((dValorFlag & GPS_ENL_FLAG_RX_BYTE) > 0)
		{
			GPS_vProcessBufferRx();
		}

		// If installation process not finish yet
		if (GPS_sDadosGPS.bConfigura_FIM == false)
		{
			GPS_vGPSConfig();
		}

		// If any timeout occurs
		if ((dValorFlag & GPS_ENL_FLAG_TIME_OUT) > 0)
		{
			//Se finalizou a configuração
			if (GPS_sDadosGPS.bConfigura_FIM != false)
			{
				//Reseta contador
				GPS_sGerenciaTimeoutMsg.dConfiguracao = 0;

				//Se ocorreu timeout da Mensagem Geodetic Position Solution
				if (GPS_sTimeoutMsg.bNavPos < 1)
				{
					GPS_sGerenciaTimeoutMsg.dNavPos++;

					//Se o GPS nunca recebeu esta mensagem, reconfigura
					if (!(memcmp(&GPS_sDadosGPS.lLon, 0,
						sizeof(GPS_sDadosGPS.lLon))))
					{
						//Volta para a configuração
						GPS_eConfigura = CfgMsgNavPosllh;
						GPS_eStatusConfig = SemConfiguracao;
						GPS_sDadosGPS.bConfigura_FIM = false;

						//Reinicia timer
						GPS_sTimeoutMsg.bNavPos = 5;
					}
				}

				//Se ocorreu timeout da Mensagem Navigation Solution Information
				if (GPS_sTimeoutMsg.bNavSol < 1)
				{
					GPS_sGerenciaTimeoutMsg.dNavSol++;

					//Se o GPS nunca recebeu esta mensagem, reconfigura
					if (!(memcmp(&GPS_sDadosGPS.eGpsFix, 0,
						sizeof(GPS_sDadosGPS.eGpsFix))))
					{
						//Volta para a configuração
						GPS_eConfigura = CfgMsgNavSol;
						GPS_eStatusConfig = SemConfiguracao;
						GPS_sDadosGPS.bConfigura_FIM = false;

						//Reinicia timer
						GPS_sTimeoutMsg.bNavSol = 10;

					}
				}

				//Se ocorreu timeout da mensagem Velocity Solution in NED
				if (GPS_sTimeoutMsg.bNavVelNed < 1)
				{
					GPS_sGerenciaTimeoutMsg.dNavVelNed++;

					//Se o GPS nunca recebeu esta mensagem, reconfigura
					if (!(memcmp(&GPS_sDadosGPS.lVelNorth, 0, sizeof(GPS_sDadosGPS.lVelNorth))))
					{
						//Volta para a configuração
						GPS_eConfigura = CfgMsgNavVelned;
						GPS_eStatusConfig = SemConfiguracao;
						GPS_sDadosGPS.bConfigura_FIM = false;

						//Reinicia timer
						GPS_sTimeoutMsg.bNavVelNed = 5;
					}

				}

				if (GPS_sTimeoutMsg.bNavTimeGps < 1)
				{
					GPS_sGerenciaTimeoutMsg.dNavTimeGps++;

					//Se o GPS nunca recebeu esta mensagem, reconfigura
					if (!(memcmp(&GPS_sDadosGPS.dTOW, 0,
						sizeof(GPS_sDadosGPS.dTOW))))
					{
						//Volta para a configuração
						GPS_eConfigura = CfgMsgNavTimeGps;
						GPS_eStatusConfig = SemConfiguracao;
						GPS_sDadosGPS.bConfigura_FIM = false;

						//Reinicia timer
						GPS_sTimeoutMsg.bNavTimeGps = 4;
					}
				}

				if (GPS_sTimeoutMsg.bMonVer == false)
				{
					GPS_sGerenciaTimeoutMsg.dMonVer++;

					//Se o GPS nunca recebeu esta mensagem, reconfigura
					if (!(memcmp(&GPS_sDadosGPS.bSwVersion, 0,
						sizeof(GPS_sDadosGPS.bSwVersion))))
					{
						//Volta para a configuração
						GPS_eConfigura = CfgMsgMonHw;
						GPS_eStatusConfig = SemConfiguracao;
						GPS_sDadosGPS.bConfigura_FIM = false;

						//Reinicia timer
						GPS_sTimeoutMsg.bMonVer = false;

					}
				}

				if (GPS_sTimeoutMsg.bMonHw < 1)
				{
					GPS_sGerenciaTimeoutMsg.dMonHw++;

					//Se o GPS nunca recebeu esta mensagem, reconfigura
					if (!(memcmp(&GPS_sDadosGPS.eStsAntena, 0,
						sizeof(GPS_sDadosGPS.eStsAntena))))
					{
						//Volta para a configuração
						GPS_eConfigura = CfgMsgMonHw;
						GPS_eStatusConfig = SemConfiguracao;
						GPS_sDadosGPS.bConfigura_FIM = false;

						//Reinicia timer
						GPS_sTimeoutMsg.bMonHw = 10;

					}
				}

				GPS_sTimeoutMsg.bNavPos--;
				GPS_sTimeoutMsg.bNavSol--;
				GPS_sTimeoutMsg.bNavVelNed--;
				GPS_sTimeoutMsg.bNavTimeGps--;
				GPS_sTimeoutMsg.bMonHw--;

			}
			else
			{
				GPS_sGerenciaTimeoutMsg.dConfiguracao++;
			}

			//Se excedeu o número de tentativas de espera das mensagens
			if ((GPS_sGerenciaTimeoutMsg.dNavPos > GPS_MAX_TENTATIVAS)
				|| (GPS_sGerenciaTimeoutMsg.dNavSol > GPS_MAX_TENTATIVAS)
				|| (GPS_sGerenciaTimeoutMsg.dNavVelNed > GPS_MAX_TENTATIVAS)
				|| (GPS_sGerenciaTimeoutMsg.dNavTimeGps > GPS_MAX_TENTATIVAS)
				|| (GPS_sGerenciaTimeoutMsg.dMonVer > GPS_MAX_TENTATIVAS)
				|| (GPS_sGerenciaTimeoutMsg.dMonHw > GPS_MAX_TENTATIVAS)
				|| (GPS_sGerenciaTimeoutMsg.dConfiguracao > 20))
			{

				//Limpa os contadores
				GPS_sGerenciaTimeoutMsg.dNavPos = 0;
				GPS_sGerenciaTimeoutMsg.dNavSol = 0;
				GPS_sGerenciaTimeoutMsg.dNavVelNed = 0;
				GPS_sGerenciaTimeoutMsg.dNavTimeGps = 0;
				GPS_sGerenciaTimeoutMsg.dMonVer = 0;
				GPS_sGerenciaTimeoutMsg.dMonHw = 0;
				GPS_sGerenciaTimeoutMsg.dConfiguracao = 0;

				//Inicia variáveis de configuração
				GPS_eConfigura = VerificaPort;
				GPS_sDadosGPS.bConfigura_FIM = false;
				GPS_eStatusConfig = SemConfiguracao;
			}
		}
	}
	/* Unreachable */
	osThreadSuspend(NULL);
}
#else
void GPS_vGPSManagementThread (void const *argument)
{}
#endif

/******************************************************************************
 * Function : GPS_vGPSPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the GPS module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the GPS topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from GPS_vGPSThread,
 ~~~~~~~~~~~~~~~
 *
 * @see GPS_vGPSThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void GPS_vGPSRecvThread (void const *argument)
{
	osStatus status;
	uint16_t wRecvBytes = 0;
	uint8_t bPayload[M2GGPSCOMM_BUF_SIZE];

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("GPS Receive Thread Created");
#endif

	GPS_vDetectThread(&WATCHDOG(GPSRCV), &bGPSRCVThreadArrayPosition, (void*)GPS_vGPSRecvThread);
	WATCHDOG_STATE(GPSRCV, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bGPSRCVThreadArrayPosition)); //Task created, inform core

	uint32_t wTicks = osKernelSysTick();

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(GPSRCV, WDT_SLEEP);
		osDelayUntil(&wTicks, 150);
		WATCHDOG_STATE(GPSRCV, WDT_ACTIVE);

		wRecvBytes = DEV_read(pGPSHandle, &bPayload[0], sizeof(bPayload));

		if (wRecvBytes)
		{
			RingBuffer_InsertMult(&GPSRxBuffer, &bPayload[0], wRecvBytes);
			osFlagSet(GPS_sFlagEnl, GPS_ENL_FLAG_RX_BYTE);
		}
	}
	osThreadTerminate(NULL);
}
#else
void GPS_vGPSRecvThread(void const *argument)
{}
#endif
