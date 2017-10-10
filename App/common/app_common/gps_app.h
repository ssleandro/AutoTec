/****************************************************************************
 * Title                 :   gps_app
 * Filename              :   gps_app.h
 * Author                :   Henrique Reis
 * Origin Date           :   29/05/2017
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
 * 29/05/2017  1.0.0     Henrique Reis      gps_app.h created.
 *
 *****************************************************************************/
#ifndef APP_COMMON_APP_COMMON_GPS_APP_H_
#define APP_COMMON_APP_COMMON_GPS_APP_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

//Valores para os temporizadores de comunicacao:
#define GPS_wTICKS_WDT  (TICK/2)      //Numero de ticks ate o watchdog timeout.
#define GPS_wTICKS_WAT  (TICK/8)    //Numero de ticks ate o fim de uma espera.
#define GPS_wTICKS_IDL  (TICK*3)    //Numero de ticks maximo em neutro.
#define GPS_wTICKS_CNX  (TICK*30)   //Numero de ticks maximo ocioso.

//Valores para os contadores de excessoes de comunicacao:
#define GPS_wMAX_ENQS   16      //Numero maximo de pedidos de confirmação de um quadro.
#define GPS_wMAX_NAKS   16      //Numero maximo de reconhecimentos negativos para um quadro.
#define GPS_wMAX_WAITS  24      //Numero maximo de esperas por buffer livre.

//Timeout do GPS

#define GPS_TIMEOUT_10S    (TICK*10)  // 10 segundos
#define GPS_TIMEOUT_9S    (TICK*9)  // 9 segundos
#define GPS_TIMEOUT_8S    (TICK*8)  // 8 segundos
#define GPS_TIMEOUT_7S    (TICK*7)  // 7 segundos
#define GPS_TIMEOUT_6S    (TICK*6)  // 6 segundos
#define GPS_TIMEOUT_5S    (TICK*5)  // 5 segundos
#define GPS_TIMEOUT_4S    (TICK*4)  // 4 segundos
#define GPS_TIMEOUT_3S    (TICK*3)  // 3 segundos
#define GPS_TIMEOUT_2S    (TICK*2)  // 2 segundos
#define GPS_TIMEOUT_1S    TICK      // 1 segundos
#define GPS_TIMEOUT_S500  (TICK/2)  // 0,5 segundos

//Flags de controle da rotina do modulo GPS:
#define GPS_FLAG_NENHUM             0x00000000
#define GPS_FLAG_TIME_OUT           0x00000001
#define GPS_FLAG_INT_TIMEPULSE      0x00000002
#define GPS_FLAG_METRO              0x00000004
#define GPS_FLAG_TIMEOUT_MTR        0x00000008
#define GPS_FLAG_SEGUNDO            0x00000010
#define GPS_FLAG_STATUS					0x00000020

//Mascaras do Fix Status Flag
#define GPS_FIX_OK  0x01   //i.e within DOP & ACC Masks
//#define DIFF_SOLN  0x02 //1 if DGPS used
#define WEEK_SET    0x04  //1 if Week Number valid
#define TOW_SET     0x08   //1 if Time of Week valid
#define GPS_FIX_STATUS_OK ( GPS_FIX_OK | WEEK_SET | TOW_SET )

//Mascaras do flag do GPS Time
#define VALID_TOW     0x01 //1=Valid Time of Week
#define VALID_WEEK    0x02 //1=Valid Week Number
#define VALID_UTCOFF  0x04 //1=Valid Leap Seconds, i.e. Leap Seconds already known
#define GPS_VALID_TIME ( VALID_TOW | VALID_WEEK | VALID_UTCOFF )

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

typedef enum
{
	No_Fix,
	Dead_Reckoning_only,
	GPS_2D_Fix,
	GPS_3D_Fix,
	GPS_and_Dead_Reckoning_Combined,
	Time_only_fix
} GPS_teFix;

typedef enum
{
	INIT,
	DONT_KNOW,
	OK,
	SHORT,
	OPEN
} GPS_teAnt;

typedef enum
{
	POWER_OFF,
	POWER_ON,
	DONTKNOW
} GPS_tePWR;

typedef struct
{

//----------------------------------------------------------------------------//
//NAV-POSLLH (Geodetic Position Solution)
	int32_t lLon;  // Longitude (deg) 1e-7
	int32_t lLat;  // Latitude (deg) 1e-7
	uint32_t dHAcc; // Horizontal Accuracy Estimate (mm)
	uint32_t dVAcc; // Vertical Accuracy Estimate (mm)

//----------------------------------------------------------------------------//
//NAV-SOL ( Navigation Solution Information )
	GPS_teFix eGpsFix; //GPS fix type
	uint8_t bFlagsFix; //Fix Status Flags
	uint16_t wPDOP; //Position DOP (0.01)
	uint8_t bNSV;  //Number of SVs used in Nav Solution

//----------------------------------------------------------------------------//
//NAV-VELNED ( Velocity Solution in NED )
	int32_t lVelNorth;     //NED north velocity (cm/s)
	int32_t lVelEast;      //NED east velocity (cm/s)
//  uint32_t          dSpeed;      //Speed (3-D) (cm/s)
	uint32_t dGroundSpeed;  //Ground Speed (2-D) (cm/s)
	uint32_t dSpeedAcc;     //Speed Accuracy Estimate (cm/s)

//----------------------------------------------------------------------------//
//NAV_TIMEGPS ( GPS Time Solution )
	uint32_t dTOW;  // GPS Milisecond Time of Week (ms)
	int16_t iWeek; //GPS week ( GPS time )
	int8_t cUTCOff;  //Leap Seconds (GPS-UTC) (s)
	uint8_t bValid; //Validity Flags (TOW, Week, utc)

//----------------------------------------------------------------------------//
//MON-HW ( Hardware Status )
	GPS_teAnt eStsAntena;
//  GPS_tePWR       eStsPower;

//----------------------------------------------------------------------------//
//CFG-PRT (Port Configuration for UART)
	uint8_t bPorta;
	uint32_t dBaud;
	uint16_t wInProto;
	uint16_t wOutProto;

//----------------------------------------------------------------------------//
//CFG_RXM
//  uint8_t  bLowPowerMode;

//----------------------------------------------------------------------------//
//Valores calculados
	float fDistancia; //Calculo da distancia percorrida
	uint8_t bBateria;

//----------------------------------------------------------------------------//
//MON-VER ( Receiver Software Versiom )
	uint8_t bSwVersion[30];
//  uint8_t     bHwVersion[10];
//  uint8_t     bRomVersion[30];

	uint8_t bConfigura_FIM; //True - Se finalizou a Configuracao do GPS

} GPS_tsDadosGPS; // estrutura contendo os dados do gps.

typedef struct
{
	GPS_tsDadosGPS sDadosGPS;
	uint8_t  bDistanciaPercorrida;
}GPS_sPubDadosGPS;


typedef struct
{
	uint32_t dPDOP;
	uint32_t dERRV;
	uint32_t dERRP;
	uint32_t dModVel;
	uint8_t bMode[3];
	uint8_t bBBRAM[3];
	uint8_t bAnt[2];
	uint16_t wLatDgr;
	uint16_t wLatMin;
	uint16_t wLatSec;
	uint16_t wLonDgr;
	uint16_t wLonMin;
	uint16_t wLonSec;
	uint16_t vVerFW;
	uint8_t bLatDir;
	uint8_t bLonDir;
	uint8_t bNSV;
}GPS_sStatus;
/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#endif /* APP_COMMON_APP_COMMON_GPS_APP_H_ */
