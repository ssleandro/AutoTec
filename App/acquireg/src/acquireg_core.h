/****************************************************************************
 * Title                 :   acquireg_core
 * Filename              :   acquireg_core.h
 * Author                :   Henrique Reis
 * Origin Date           :   17 de abr de 2017
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
 *  17/04/17   1.0.0     Henrique Reis         acquireg_core.h created.
 *
 *****************************************************************************/
#ifndef ACQUIREG_SRC_ACQUIREG_CORE_H_
#define ACQUIREG_SRC_ACQUIREG_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../acquireg/inc/interface_acquireg.h"
#else
#include "interface_acquireg.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
/*!<< From MPA2500  */

//// TODO: Resolve this dependencies
//#define CAN_bNUM_DE_LINHAS                    36
//#define CAN_bNUM_SENSORES_POR_LINHA           2
//#define CAN_bNUM_SENSORES_DIGITAIS            6
//#define CAN_bNUM_SENSORES_SEMENTE_E_ADUBO     (CAN_bNUM_DE_LINHAS * CAN_bNUM_SENSORES_POR_LINHA)
//#define CAN_bTAMANHO_LISTA                    (CAN_bNUM_SENSORES_SEMENTE_E_ADUBO + CAN_bNUM_SENSORES_DIGITAIS)
//Tempo para auto-desligamento do MPA2500
#define AQR_TIMEOUT_30MIN       (30*60*TICK)

//Tempo para implemento parado do MPA2500
#define AQR_TIMEOUT_10SEGS      (10*TICK)

#define AQR_APL_FLAG_FINISH_INSTALLATION 	0x0001
#define AQR_APL_FLAG_SAVE_STATIC_REG 		0x0002
#define AQR_APL_FLAG_UPDATE_INSTALLATION	0x0004
#define AQR_APL_FLAG_CONFIRM_INSTALLATION	0x0008
#define AQR_APL_FLAG_SAVE_LIST				0x0010
#define AQR_APL_FLAG_ERASE_LIST				0x0020
#define AQR_APL_FLAG_SEND_TOTAL				0x0040
#define AQR_SIS_FLAG_ALARME				 	0x0080
#define AQR_SIS_FLAG_ALARME_TOLERANCIA		0x0100
#define AQR_APL_FLAG_ERASE_INSTALLATION		0x0200
#define AQR_APL_FLAG_SENSOR_CHANGE			0x0400

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

//Sequencia de códigos para a variável AQR_bStatus, usados na inicialização:
#define AQR_bSTS_NAO_INICIADO           0xFF
#define AQR_bSTS_OK                     0x00
#define AQR_bSTS_ERRO                   0x01
#define AQR_bSTS_FFS_OK                 0x02
#define AQR_bSTS_SALVA_REG              0x03
#define AQR_bSTS_NOVO_REG               0x04

#define ARQ_SAVE_ESTATIC_DATA_TIMEOUT	10
/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/*******************************************************************************
 Tipos relacionados às funções deste módulo:
 *******************************************************************************/
/*!<< From MPA2500  */
//typedef enum {
//  Sem_Arremate,  //Nenhum lado levantado
//  Lado_Esquerdo, //Lado esquerdo levantado
//  Lado_Direito   //Lado direito levantado
//}
//AQR_teArremate;
//
//typedef enum {
//  AguardandoEvento,    //Nenhum novo evento
//  Instalado,           //Novo sensor instalado
//  FalhaAutoTeste,      //Falha no Auto-Teste
//  SensorNaoEsperado    //Sensor não esperado no bus ( à mais que o nº de linhas )
//}
//AQR_teInstalacao;
//
//typedef struct {
//
//  uint8_t   bPausaAuto;           //Monitor está em Pausa Automática
//  uint8_t   bExVel;               //Excesso de velocidade
//  uint8_t   bVelZero;             //Velocidade Zero
//  uint8_t   bTrabalhando;         //Está Trabalhando
//
//  uint8_t   bInsensivel;          //Está área de insensibilidade
//  uint8_t   bImplemento;          //Implemento Levantado
//  uint8_t   bErroGPS;             //Erro no GPS
//
//  AQR_teInstalacao   eStatusInstalacao; //Indica se o sensor foi instalado ou se houve falha na instalação
//  uint8_t   bSensorDesconectado;          //Indica que algum sensor está desconectado
//  uint8_t   bLinhasFalha;                 //Indica se há Linhas em falha >90% e < 20%
//  uint8_t   bLinhasZero;                  //Indica se há Linhas em falha < 90%
//  uint8_t   bSensorNaoEsperado;           //Indica se foi conectado sensor além do configurado
//
//  uint8_t   bSementeInstalados;   //Quantidade de sensores de semente instalados até o momento
//  uint8_t   bAduboInstalados;     //Quantidade de sensores de adubo instalados até o momento
//  uint8_t   bAdicionalInstalados; //Quantidade de sensores adicionais instalados até o momento
//  uint8_t   bReprovados;          //Quantidade de sensores reprovados no Auto-Teste
//
//  uint8_t   bAdicionalDesconectado;    //Sensores Adicionais desconectados
//  uint8_t   bMemAdicionalDesconectado; //Memoriza Sensores adicionais desconectados
//  //uint8_t   bLinhaSementeIgnorado;     //Número da linha que o sensor de semente está sendo ignorado
//  //uint8_t   bLinhaAduboIgnorado;       //Número da linha que o sensor de adubo está sendo ignorado
//
//  uint8_t   bAutoTeste;             //Informa se está em auto-teste
//  uint8_t   bAlarmeOK;              //Informa se o alarme foi reconhecido
//  uint8_t   bAlarmeGPS;              //Informa o alarme do GPS
//  AQR_teArremate   eArremate;     //Informa se está ou não está em Arremate
//  uint8_t   bNumLinhasSemIntercalar;
////  AQR_teIntercala  eIntercala;
//
//  uint8_t   bNumLinhasZero;            //Quantidade de linhas com Zero sementes
//  uint8_t   bNumLinhasAtivas;          //Quantidade de linhas que não estão levantadas (em arremate)
//  uint8_t   bNumLinhasDir;             //Quantidade de linhas que estão ativas do lado direito
//  uint8_t   bNumLinhasEsq;             //Quantidade de linhas que estão ativas do lado esquerdo
//
//  uint8_t   bNumSensores;         //Nº total de sensores configurados pelo usuário
//  uint8_t   bLinhaDisponivel;     //Informa qual linha está disponível para instalação do novo sensor
//  uint8_t   bSensorAdicionado;    //Informa qual Sensor foi adicionado
//  uint8_t   bCfgSensorRespondeu;    //Informa se o sensor respondeu a configuração
//
//  uint32_t  dSementeFalha;          //Linha com Sensores de Semente em Falha
//  uint32_t  dSementeFalhaExt;       //Linha com Sensores de Semente em Falha Extendida
//  uint32_t  dSementeFalhaIHM;       //Flags que indica na IHM as falhas de semente
//  uint32_t  dSementeFalhaIHMExt;    //Flags que indica na IHM as falhas de semente Extendida
//  uint32_t  dAduboFalha;            //Linha com Sensores de Adubo em Falha
//  uint32_t  dAduboFalhaExt;         //Linha com Sensores de Adubo em Falha Extendida
//  uint32_t  dLinhaDesconectada;     //Linhas com sensor desconectado
//  uint32_t  dLinhaDesconectadaExt;  //Linhas com sensor desconectado Extendida
//  uint32_t  dMemLinhaDesconectada;  //Memoriza Linhas com sensor desconectado
//  uint32_t  dMemLinhaDesconectadaExt;
//  uint32_t  dLinhasLevantadas;      //(Linhas levantadas = 1, Linhas ativas = 0)
//  uint32_t  dLinhasLevantadasExt;
//  uint32_t  dSementeIgnorado;       //Linha com sensor de Semente Ignorado;
//  uint32_t  dSementeIgnoradoExt;    //Extendido
//  uint32_t  dAduboIgnorado;         //Linha com sensor de Adubo Ignorado;
//  uint32_t  dAduboIgnoradoExt;      //Extendido
//  uint32_t  dSementeZero;           //Flags que indica falha de semente proximo de zero
//  uint32_t  dSementeZeroExt;        //Extendido
//  uint32_t  dSementeZeroIHM;        //Flags que indica falha de semente proximo de zero para IHM
//  uint32_t  dSementeZeroIHMExt;     //Extendido
//  uint16_t  wMaxSementes;           //Tolerância máxima de sementes
//  uint16_t  wMinSementes;           //Tolerância mínima de sementes
//
//  uint16_t  wMinSementesZero;      //Tolerância mínima de sementes para considerar que não estão caindo sementes
//
//  uint16_t  awMediaSementes[36];    //Média de sementes plantadas por metro * 100 (com duas casas decimais)
//  uint16_t  wAvaliaArred;
//
//  uint8_t   bPrimeiroSegmento;
//  uint8_t   bAlarmeLinhasLevantadas; //Contador de linhas de falha de linhas levantada
//
//
//} tsStatus;
//
//typedef struct{
//  uint16_t  awBufDis;
//  uint8_t   abBufSem[ CAN_bNUM_DE_LINHAS ];
//}tsFalhaInstantanea;
/******************************************************************************
 Estrutura do registro:
 *******************************************************************************/

#pragma pack(push,1)

#pragma pack(pop)

//Tamanho da estrutura acima:
#define AQR_wTAM_REG_ESTATICO sizeof( AQR_tsRegEstatico )

//Tamanho da estrutura acima:
#define AQR_wTAM_REG_ESTATICO_CRC sizeof( AQR_tsRegEstaticoCRC )

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/
/*!<< From MPA2500  */
////Flag indicador da atualização das informações de Aquisição:
//extern OS_FLAG_GRP            *AQR_sFlagREG;
//
////Mutex para controle de acesso às estruturas de dados das entradas e variáveis
////da aquisição e registro:
//extern OS_EVENT               *AQR_MTX_sEntradas;
//
////Variável que indica o que ocasionou o alarme:
//extern uint16_t                  AQR_wAlarmes;
//
////Variável que indica Status para registro
//extern uint8_t                  AQR_bStatusReg;
//
////Estrutura de dados do GPS
//extern GPS_tsDadosGPS         AQR_sDadosGPS;
//
////Estrutura de valores Acumulados
//extern tsAcumulados           AQR_sAcumulado;
//
////Estrutura de valores relativos à velocidade
//extern tsVelocidade           AQR_sVelocidade;
//
////Estrutura de Status do Monitor
//extern tsStatus               AQR_sStatus;
//
////Estrutura de dados do CAN
//extern CAN_tsCtrlListaSens    AQR_sDadosCAN;
//
//extern tsFalhaInstantanea     AQR_sFalhaInstantanea;
//
////Último número usado para renomear um arquivo de registro:
//extern uint16_t                 AQR_wNumArqReg;
//
////Última causa de encerramento de registro.
//extern uint16_t                 AQR_wCausaFim;
//
////Variável auxiliar para Largura do Implemento/ Distância entre linhas
//extern uint16_t                 AQR_wEspacamento;
//
////Variável para auxiliar para indicar se gravação de registros está ativada
//extern uint8_t                  AQR_bSalvaRegistro;
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
/*!<< From MPA2500  */
void AQR_vEmergencia (void);
void AQR_vTrfPrincipal (void *p_arg);
void AQR_vAcumulaDistancia (void);
void AQR_vApagaListaSensores (void);
void AQR_vZeraRegs (uint8_t bTudo);
void AQR_vApagaArquivos (void);
void AQR_vParametros (void);
void AQR_vAcumulaArea (void);
void AQR_SetStaticRegData (void);
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ACQUIREG_SRC_ACQUIREG_CORE_H_ */
