/****************************************************************************
* Title                 :   sensor_core Include File
* Filename              :   sensor_core.h
* Author                :   Henrique Reis
* Origin Date           :   04 de jul de 2016
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
*    Date    Version     Author         				Description
*  07/07/16   1.0.0  Henrique Reis     sensor_core.h created
*
*****************************************************************************/
/** @file
 *  @brief
 *
 *
 */
#ifndef SENSOR_SRC_SENSOR_CORE_H_
#define SENSOR_SRC_SENSOR_CORE_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../sensor/inc/interface_sensor.h"
#else
#include "interface_sensor.h"
#endif
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants from MPA2500 (can.h)
*******************************************************************************/
// Priorities
//#define CAN_ALTA_PRIOR            0x00
//
//// Value of DLC of control field
//#define CAN_MSG_DLC_0             0x00
//#define CAN_MSG_DLC_1             0x01
//#define CAN_MSG_DLC_2             0x02
//#define CAN_MSG_DLC_3             0x03
//#define CAN_MSG_DLC_4             0x04
//#define CAN_MSG_DLC_5             0x05
//#define CAN_MSG_DLC_6             0x06
//#define CAN_MSG_DLC_7             0x07
//#define CAN_MSG_DLC_8             0x08
//
//#define CAN_ENL_FLAG_NENHUM       0x00000000
//#define CAN_ENL_FLAG_RX_MENSAGEM  0x00000001
//
//#define CAN_ENL_QTDE_BYTES_MSG        8

/******************************************************************************
* Macros
*******************************************************************************/
    
/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* Typedefs from MPA2500 (can.h)
*******************************************************************************/

// Auteq Identifier
typedef union {
    struct {
        uint32_t bLinha        : 8; //ID.7  - ID.0
        uint32_t bPacoteAtual  : 4; //ID.11 - ID.8
        uint32_t bTotalPacotes : 4; //ID.15 - ID.12
        uint32_t bComando      : 7; //ID.22 - ID.16
        uint32_t bTipo         : 6; //ID.28 - ID.23
        uint32_t pad           : 3; //ID.31 - ID.29
    } sID_bits;
    uint32_t dID;
} CAN_tsIDAuteq;

// CAN network control structure
typedef struct {
    osThreadId* psFlagEnl;
    uint32_t dEventosEnl;     //Flags de eventos do enlace.
} CAN_tsCtrlEnl;

// CAN application control structure
typedef struct {
//    osThreadId* psFlagApl;
    osFlagsGroupId* psFlagApl;
//    osThreadId* psFlagAplAqrReg;
    osFlagsGroupId* psFlagAplAqr;
//    uint32_t dEventosApl;               //Flags de eventos do aplicação
    osFlags dEventosApl;
//    uint32_t dEventosAplAqrReg;         //Flags de eventos do aplicação
    osFlags dEventosAplAqr;
    uint32_t dSensSementeConectados;    //Conjunto de sensores de semente conectados à rede
    uint32_t dSensSementeConectadosExt; //Conjunto de sensores de semente conectados à rede extendido
    uint32_t dSensAduboConectados;      //Conjunto de sensores de adubo conectados à rede
    uint32_t dSensAduboConectadosExt;   //Conjunto de sensores de adubo conectados à rede extendido
    uint32_t dSensDigitalConectados;    //Conjunto de sensores digitais conectados à rede
    uint32_t dRespostaSemente;          //Flags de respostas a comandos - sensores de semente
    uint32_t dRespostaSementeExt;       //Flags de respostas a comandos - sensores de semente Extendido  //***********************************
    uint32_t dRespostaAdubo;            //Flags de respostas a comandos - sensores de adubo
    uint32_t dRespostaAduboExt;         //Flags de respostas a comandos - sensores de adubo Extendido
    uint32_t dRespostaDigital;          //Flags de respostas a comandos - sensores digitais

    uint32_t dResSemParametros;
    uint32_t dResSemParametrosExt;
    uint32_t dResAduParametros;
    uint32_t dResAduParametrosExt;
    uint32_t dResDigParametros;

    uint32_t dResSemVersaoSW;
    uint32_t dResSemVersaoSWExt;
    uint32_t dResAduVersaoSW;
    uint32_t dResAduVersaoSWExt;
    uint32_t dResDigVersaoSW;

    uint32_t dTicksCmdPnP;            //Número de ticks para o timer de envio do comando de PnP
    uint32_t dTicksTimeoutPnP;        //Número de ticks para o timer de timeout do comando de PnP
    uint32_t dTicksTimeoutConfigura;  //Número de ticks para o timer de timeout de comando de configuração
    uint32_t dTicksTimeoutComando;    //Número de ticks para o timer de timeout dos outros comandos
    uint8_t bTodosSensSementeRsp;    //Indica que todos os sensores de semente responderam ao comando de PnP
    uint8_t bTodosSensAduboRsp;      //Indica que todos os sensores de adubo responderam ao comando de PnP
    uint8_t bTodosSensSementeRspExt; //Indica que todos os sensores de semente responderam ao comando de PnP Extendido
    uint8_t bTodosSensAduboRspExt;   //Indica que todos os sensores de adubo responderam ao comando de PnP Extendido
    uint8_t bTodosSensDigitalRsp;    //Indica que todos os sensores digitais responderam ao comando de PnP
    uint8_t bHandler;                //Handler do arquivo manipulado pela aplicação

    uint8_t bTodosSensSemRspParametros;    //Indica que todos os sensores de semente responderam ao comando
    uint8_t bTodosSensAduRspParametros;    //Indica que todos os sensores de adubo responderam ao comando
    uint8_t bTodosSensSemRspParametrosExt; //Indica que todos os sensores de semente responderam ao comando extendido
    uint8_t bTodosSensAduRspParametrosExt; //Indica que todos os sensores de adubo responderam ao comando  extendido
    uint8_t bTodosSensDigRspParametros;    //Indica que todos os sensores digitais responderam ao comando

    uint8_t bTodosSensSemRspVersao;    //Indica que todos os sensores de semente responderam ao comando
    uint8_t bTodosSensAduRspVersao;    //Indica que todos os sensores de adubo responderam ao comando
    uint8_t bTodosSensSemRspVersaoExt; //Indica que todos os sensores de semente responderam ao comando
    uint8_t bTodosSensAduRspVersaoExt; //Indica que todos os sensores de adubo responderam ao comando
    uint8_t bTodosSensDigRspVersao;    //Indica que todos os sensores digitais responderam ao comando

    uint8_t bUltimoCmdEnviado;          //Último comando enviado
    uint8_t bReiniciaTimeout;           //Indica que o timer de timeout do comando de detecção deve ser reiniciado
    uint8_t bReiniciaTimeoutVel;        //Indica que o timer de timeout do comando de detecção deve ser reiniciado

    osTimerId wTimerCmdPnP;            //Timer de sistema para envio do comando de PnP
    osTimerId wTimerTimeoutPnP;        //Timer de sistema para controle de timeout do comando de PnP
    osTimerId wTimerTimeoutConfigura;  //Timer de sistema para controle de timeout do comando de configuração
    osTimerId wTimerTimeoutComando;    //Timer de sistema para controle de timeout do comando de leitura de dados
} CAN_tsCtrlApl;

// CAN controller structure
typedef struct {
    canMSGStruct_s sMensagemRecebida;     // Received message on CAN network - 16 bytes
    CAN_tsCtrlEnl sCtrlEnl;               // Network control  - 8 bytes
    CAN_tsCtrlApl sCtrlApl;               // Application control - 139 bytes
} CAN_tsCtrlMPA;

typedef struct
{
    uint32_t dRxMsg;           // Total of received messages
    uint32_t dTxMsg;           // Total of received messages
    uint32_t dRxErr;           // The current value of the Rx Error Counter
    uint32_t dTxErr;           // The current value of the Tx Error Counter
    uint32_t dErrorWarning;    // one or both of the Transmit and Receive
    // Error Counters has reached the limit set
    // in the Error Warning Limit register.
    // Or the CAN controller is currently prohibited
    // from bus activity because the Transmit Error
    // Counter reached its limiting value of 255
    
    uint32_t dDataOverrun;     // A message was lost because the preceding
    // message to this CAN controller was not read
    // and released quickly enough
    
    uint32_t dErrorPassive;    // CAN controller switches between Error Passive
    // and Error Active mode in either direction.
    
    uint32_t dArbitrationLost; // CAN controller loses arbitration while
    // attempting to transmit.
    
    uint32_t dBusError;        // CAN controller detects an error on the bus.
} CAN_tsStatus;

// System flag
extern osFlagsGroupId UOS_sFlagSis;

// CAN controller structure
extern CAN_tsCtrlMPA CAN_sCtrlMPA;

// CAN network status
extern CAN_tsStatus CAN_sStatus;

// 
extern osMessageQId SensorWriteQ;

// CAN network sensors list name
extern const uint8_t CAN_abNomeConfigLista[];
extern const uint8_t CAN_abNomeArquivoErros[];

/******************************************************************************
* Typedefs from Control module... Just for test...
*******************************************************************************/
//typedef enum {
//  Linhas_Pares,     //Linhas Pares levantadas
//  Linhas_Impares,   //Linhas Ímpares levantadas
//  Sem_Intercalacao  //Nenhuma linha levantada
//
//} UOS_teIntercala;
//
////Configuração do Monitor
//typedef struct {
//
//  float   fLimVel;               //Limite de velocidade km/h.
//
//  uint16_t wSementesPorMetro;     //Meta de Sementes por Metro. (sementes/m)*10
//  uint16_t wInsensibilidade;      //Distância de insensibilidade para falhas. (metros)*10
//
//  uint16_t wAvalia;               //Distância para avaliação de aplicação. (metros)*10
//  uint16_t wDistLinhas;           //Distância entre linhas. (centímetros)*10
//
//  uint16_t wLargImpl;             //Largura do implemento. (centímetros)*10
//  uint8_t  bMonitorArea;          //Se está em modo monitor de área. (0 = false, 1 = true )
//  uint8_t  bNumLinhas;            //No. Linhas (1-36)
//
//  uint8_t  bDivLinhas;            //Divisão da plantadeira (bNumLinhas/2 ou bNumLinhas/2+1)
//  uint8_t  bSensorAdubo;          //Indica presença de sensor de adubo. (bSensorAdubo = 1)
//  uint8_t  bTolerancia;           //Tolerância ao espaçamento entre sementes. (porcentagem)
//  uint8_t  bTeclaPausaHab;             //Habilita o uso da tecla de pausa. (bTeclaPausaHab = 1)
//
//  UOS_teIntercala  eIntercala;
//  uint8_t  bPausaAuto;            //Habilita o uso da pausa automática.(bPausaAuto = 1)
//  uint8_t  bLinhasFalhaPausaAuto;      //Número de linhas em falha para pausa automática. (1-32)
//  uint8_t  bNumSensorAdicionais;  //Número de sensores adicionais (0-6)
//
//} UOS_tsCfgMonitor;
//
////Receptor GPS interno:
//typedef struct {
//  int32_t        lFusoHorario;
//
//  uint16_t      wDistanciaEntreFixos;
//  uint16_t      wAnguloEntreFixos;
//
//  uint8_t       bHorarioVerao;       //Indica se está em horário de verão (bHorarioVerao = 1)
//  uint8_t       bSalvaRegistro;      //Indica se gravação de registros está ativada
//} tsCfgGPS;
//
////Identificação do veículo:
//typedef struct {
//  uint8_t       abCodigo[6];
//} tsCfgVeiculo;
//
//typedef struct {
//  //Configuração Monitor
//  UOS_tsCfgMonitor              sMonitor;
//
//  //Receptor GPS interno:
//  tsCfgGPS                      sGPS;
//
//  //Identificação do veículo:
//  tsCfgVeiculo                  sVeiculo;
//
//  //CRC16 desta estrutura:
//  uint16_t                        wCRC16;
//
//} UOS_tsConfiguracao;

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

extern UOS_tsConfiguracao      UOS_sConfiguracao;

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

#endif /* SENSOR_SRC_SENSOR_CORE_H_ */

/*** End of File **************************************************************/
