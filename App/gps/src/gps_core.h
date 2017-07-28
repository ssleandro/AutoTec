/****************************************************************************
 * Title                 :   gps_core
 * Filename              :   gps_core.h
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
 *  21/03/17   1.0.0     Henrique Reis         gps_core.h created.
 *
 *****************************************************************************/
#ifndef GPS_SRC_GPS_CORE_H_
#define GPS_SRC_GPS_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../gps/inc/interface_gps.h"
#else
#include "interface_gps.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/
////Valores para os temporizadores de comunicação:
//#define GPS_wTICKS_WDT  (TICK/2)      //Número de ticks até o watchdog timeout.
//#define GPS_wTICKS_WAT  (TICK/8)    //Número de ticks até o fim de uma espera.
//#define GPS_wTICKS_IDL  (TICK*3)    //Número de ticks máximo em neutro.
//#define GPS_wTICKS_CNX  (TICK*30)   //Número de ticks máximo ocioso.
//
////Valores para os contadores de excessões de comunicação:
//#define GPS_wMAX_ENQS   16      //Número máximo de pedidos de confirmação de um quadro.
//#define GPS_wMAX_NAKS   16      //Número máximo de reconhecimentos negativos para um quadro.
//#define GPS_wMAX_WAITS  24      //Número máximo de esperas por buffer livre.
//
////Timeout do GPS
//
//#define GPS_TIMEOUT_10S    (TICK*10)  // 10 segundos
//#define GPS_TIMEOUT_9S    (TICK*9)  // 9 segundos
//#define GPS_TIMEOUT_8S    (TICK*8)  // 8 segundos
//#define GPS_TIMEOUT_7S    (TICK*7)  // 7 segundos
//#define GPS_TIMEOUT_6S    (TICK*6)  // 6 segundos
//#define GPS_TIMEOUT_5S    (TICK*5)  // 5 segundos
//#define GPS_TIMEOUT_4S    (TICK*4)  // 4 segundos
//#define GPS_TIMEOUT_3S    (TICK*3)  // 3 segundos
//#define GPS_TIMEOUT_2S    (TICK*2)  // 2 segundos
//#define GPS_TIMEOUT_1S    TICK      // 1 segundos
//#define GPS_TIMEOUT_S500  (TICK/2)  // 0,5 segundos
//
////Flags de controle da rotina do módulo GPS:
//#define GPS_FLAG_NENHUM             0x00000000
//#define GPS_FLAG_TIME_OUT           0x00000001
//#define GPS_FLAG_INT_TIMEPULSE      0x00000002
//#define GPS_FLAG_METRO              0x00000004
//#define GPS_FLAG_TIMEOUT_MTR        0x00000008
//#define GPS_FLAG_SEGUNDO            0x00000010
/*******************************************************************************

 Definições relacionadas à comunicação com o GPS:

 *******************************************************************************/

/*******************************************************************************

 Definições do Protocolo Ubx

 | Sync | Sync |       |    |               |     ~     |    |    |
 | Char | Char | Class | ID |    LENGTH     |  Payload  | CK | CK |
 |  1   |  2   |       |    | Little Endian |     ~     | _A | _B |
 (2 Bytes)      (n Bytes)

 *******************************************************************************/

// Definições relacionadas à máquina de estados de recepção
#define GPS_ERX_W_SYNC_1   0
#define GPS_ERX_W_SYNC_2   1
#define GPS_ERX_W_CLASS    2
#define GPS_ERX_W_ID       3
#define GPS_ERX_W_LEN_1    4
#define GPS_ERX_W_LEN_2    5
#define GPS_ERX_W_DATA     6
#define GPS_ERX_W_CK_A     7
#define GPS_ERX_W_CK_B     8

//-----------------------------------------------------------------------------//
//SINCRONISMO

#define SYNC_CHAR_1  0xB5
#define SYNC_CHAR_2  0x62

//-----------------------------------------------------------------------------//
//CLASS IDs

#define NAV  0x01 //Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used
//#define RXM  0x02 //Receiver Manager Messages: Satellite Status, RTC Status
//#define INF  0x04 //Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice
#define ACK  0x05 //Ack/Nack Messages: as replies to CFG Input Messages
#define CFG  0x06 //Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc.
#define MON  0x0A //Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status
//#define AID  0x0B //AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input
//#define TIM  0x0D //Timing Messages: Timepulse Output, Timemark Results

//-----------------------------------------------------------------------------//
//MSG IDs

//IDs CFG
#define CFG_PRT   0x00 //Port Configuration for UART
#define CFG_MSG   0x01 //Message Rate
//#define CFG_INF   0x02 //Information message configuration
//#define CFG_RST   0x04 //Reset Receiver / Clear Backup Data Structures
//#define CFG_DAT   0x06 //Standard Datum
#define CFG_TP    0x07 //TimePulse Parameters
#define CFG_TP5   0x31 //TimePulse Parameters
#define CFG_RATE  0x08 //Navigation/Measurement Rate Settings
//#define CFG_CFG   0x09 //Clear, Save and Load configurations
#define CFG_RXM   0x11 //RXM configuration
#define CFG_ANT   0x13 //Antenna Control Settings
//#define CFG_SBAS  0x16 //SBAS Configuration
//#define CFG_NMEA  0x17 //NMEA protocol configuration
//#define CFG_USB   0x1B //USB Configuration
//#define CFG_TMODE 0x1D //Time Mode Settings
#define CFG_NAVX5 0x23 //Navigation Engine Expert Settings
#define CFG_NAV5  0x24 //Navigation Engine Settings

//IDs NAV
#define NAV_POSLLH    0x02 //Geodetic Position Solution
//#define NAV_STATUS    0x03 //Receiver Navigation Status
//#define NAV_DOP       0x04 //Dilution of precision
#define NAV_SOL       0x06 //Navigation Solution Information
//#define NAV_VELECEF   0x11 //Velocity Solution in ECEF
#define NAV_VELNED    0x12 //Velocity Solution in NED
#define NAV_TIMEGPS   0x20 //GPS Time Solution
//#define NAV_TIMEUTC   0x21 //UTC Time Solution
//#define NAV_CLOCK     0x22 //Clock Solution
//#define NAV_SVINFO    0x30 //Space Vehicle Information
//#define NAV_SBAS      0x32 //SBAS Status Data

//IDs ACK
#define ACK_NACK  0x00
#define ACK_ACK   0x01

//IDs MON
#define MON_VER 0x04
#define MON_HW  0x09

//-----------------------------------------------------------------------------//

////Máscaras do Fix Status Flag
//#define GPS_FIX_OK  0x01   //i.e within DOP & ACC Masks
////#define DIFF_SOLN  0x02 //1 if DGPS used
//#define WEEK_SET    0x04  //1 if Week Number valid
//#define TOW_SET     0x08   //1 if Time of Week valid
//#define GPS_FIX_STATUS_OK ( GPS_FIX_OK | WEEK_SET | TOW_SET )
//
////Máscaras do flag do GPS Time
//#define VALID_TOW     0x01 //1=Valid Time of Week
//#define VALID_WEEK    0x02 //1=Valid Week Number
//#define VALID_UTCOFF  0x04 //1=Valid Leap Seconds, i.e. Leap Seconds already known
//#define GPS_VALID_TIME ( VALID_TOW | VALID_WEEK | VALID_UTCOFF )

/*******************************************************************************

 Definições relacionadas ao método de interpolação de coordenadas.
 O Gps envia coordenada à 1 Hz, porém o programa calcula coordenadas
 entre coordenadas reais para precisão no cálculo.
 *******************************************************************************/
// nº de amostras em um segundo. No caso de 4 amostras, o programa irá
// receber 1 amostra e interpolar outras 3, tendo assim amostras à cada 250 Hz.
#define NUM_AM_INT          (TICK/8)

/* Valor do semi-eixo maior do elipsóide WGS-84 */
//#define GPS_ELIP_EIXO_MAIOR    6378137

// dia juliano para o dia 6 de janeiro de 1980( referência do GPS )
#define JD_GPS_TIME         2444245

/*******************************************************************************

 Definições relacionadas ao sistema de navegação:

 *******************************************************************************/

/*******************************************************************************
 Estruturas públicas deste módulo:
 *******************************************************************************/
//typedef enum {
//  No_Fix,
//  Dead_Reckoning_only,
//  GPS_2D_Fix,
//  GPS_3D_Fix,
//  GPS_and_Dead_Reckoning_Combined,
//  Time_only_fix
//}GPS_teFix;
//
//typedef enum {
//  INIT,
//  DONT_KNOW,
//  OK,
//  SHORT,
//  OPEN
//}GPS_teAnt;
//
//typedef enum {
//  POWER_OFF,
//  POWER_ON,
//  DONTKNOW
//}GPS_tePWR;
//typedef struct
//{
//
////----------------------------------------------------------------------------//
////NAV-POSLLH (Geodetic Position Solution)
//  int32_t          lLon;  // Longitude (deg) 1e-7
//  int32_t          lLat;  // Latitude (deg) 1e-7
//  uint32_t          dHAcc; // Horizontal Accuracy Estimate (mm)
//  uint32_t          dVAcc; // Vertical Accuracy Estimate (mm)
//
////----------------------------------------------------------------------------//
////NAV-SOL ( Navigation Solution Information )
//  GPS_teFix       eGpsFix; //GPS fix type
//  uint8_t           bFlagsFix; //Fix Status Flags
//  uint16_t          wPDOP; //Position DOP (0.01)
//  uint8_t           bNSV;  //Number of SVs used in Nav Solution
//
////----------------------------------------------------------------------------//
////NAV-VELNED ( Velocity Solution in NED )
//  int32_t          lVelNorth;     //NED north velocity (cm/s)
//  int32_t          lVelEast;      //NED east velocity (cm/s)
////  uint32_t          dSpeed;      //Speed (3-D) (cm/s)
//  uint32_t          dGroundSpeed;  //Ground Speed (2-D) (cm/s)
//  uint32_t          dSpeedAcc;     //Speed Accuracy Estimate (cm/s)
//
////----------------------------------------------------------------------------//
////NAV_TIMEGPS ( GPS Time Solution )
//  uint32_t          dTOW;  // GPS Milisecond Time of Week (ms)
//  int16_t          iWeek; //GPS week ( GPS time )
//  int8_t           cUTCOff;  //Leap Seconds (GPS-UTC) (s)
//  uint8_t           bValid; //Validity Flags (TOW, Week, utc)
//
////----------------------------------------------------------------------------//
////MON-HW ( Hardware Status )
//  GPS_teAnt       eStsAntena;
////  GPS_tePWR       eStsPower;
//
////----------------------------------------------------------------------------//
////CFG-PRT (Port Configuration for UART)
//  uint8_t     bPorta;
//  uint32_t    dBaud;
//  uint16_t    wInProto;
//  uint16_t    wOutProto;
//
////----------------------------------------------------------------------------//
////CFG_RXM
////  uint8_t  bLowPowerMode;
//
////----------------------------------------------------------------------------//
////Valores calculados
//  float      fDistancia; //Cálculo da distância percorrida
//  uint8_t     bBateria;
//
////----------------------------------------------------------------------------//
////MON-VER ( Receiver Software Versiom )
//  uint8_t     bSwVersion[30];
////  uint8_t     bHwVersion[10];
////  uint8_t     bRomVersion[30];
//
//  uint8_t     bConfigura_FIM; //True - Se finalizou a Configuração do GPS
//
//} GPS_tsDadosGPS; // estrutura contendo os dados do gps.

//Estrutura com a hora local:
// os dois membros desta estrutura armazaenam a data/hora com o seguinte formato:
// 0 - centésimos de segundos
// 1 - seg
// 2 - min
// 3 - hora
// 4 - dia da semana ( domingo = 1 )
// 5 - dia
// 6 - mês
// 7 - ano
//
typedef struct
{
	//Hora Atual GPS
	uint8_t abHoraAtualGPS[8];

	//Hora local = hora GPS + offset GPS/UTC + diferença de fuso-horário:
	uint8_t abDataHoraBCDLocal[8];

	//Hora local HV = hora local + 1 hora, se no período do horário de verão
	//senão: Hora local HV = hora local:
	uint8_t abDataHoraBCDLocalHV[8];

	//Hora local em segundos desde 01/01/1970 sem o horário de verão:
	uint32_t dDataHoraSistema;

} GPS_tsDataHoraLocal;

/*******************************************************************************
 Variáveis públicas deste módulo:
 *******************************************************************************/
//Pilha da tarefa principal:
//extern OS_STK GPS_adTrfPrincipalStk[ GPS_TRF_PRINCIPAL_STK_SIZE ];
//Estrutura de dados do GPS
//extern GPS_tsDadosGPS         GPS_sDadosGPS;
//Flags para indicar eventos do GPS
//extern OS_FLAG_GRP            *GPS_sFlagGPS;
//Mutex para controle de acesso às estruturas de dados das entradas do GPS:
//extern OS_EVENT               *GPS_MTX_sEntradas;
//Timer do GPS para percorrer 1 metro
//extern UOS_tbTimerHandler     GPS_bTimerMtr;
//Timer do TIMEPULSE
//extern UOS_tbTimerHandler     GPS_bTimerInt;
//Flags para indicar eventos da aplicação do protocolo:
//extern OS_FLAG_GRP            *GPS_sFlagApl;
//extern uint8_t                  GPS_bEstadoRxGPS;
//extern uint8_t              GPS_bDistanciaPercorrida;
/*******************************************************************************
 Funções públicas deste módulo:
 *******************************************************************************/
//void GPS_vTrataInterrupt( void );
void GPS_vTrfPrincipal (void *p_arg);
void GPS_vTrataEINT2 (void);
void GPS_vTimeout (void);
void GPS_dDataHoraLocalHV (uint8_t *pabDataHoraLocalHV);
void GPS_dDataHoraLocal (uint8_t *pabDataHoraLocal);
uint32_t GPS_dDataHoraSistema (void);
void GPS_vCalcHoraUTC (uint8_t *pabDataHora);
void GPS_vCalcHoraLocal (void);
uint32_t GPS_dCalcHoraSistema (uint8_t *pabDataHora);

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

#endif /* GPS_SRC_GPS_CORE_H_ */
