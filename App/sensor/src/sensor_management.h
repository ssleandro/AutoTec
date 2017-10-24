/****************************************************************************
 * Title                 :   sensor_management
 * Filename              :   sensor_management.h
 * Author                :   Henrique Reis
 * Origin Date           :   21 de out de 2016
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
 *  21/10/16   1.0.0     Henrique Reis         sensor_management.h created.
 *
 *****************************************************************************/

/*******************************************************************************
 *
 * Arquivo   : can_api.h
 * Título    : Firmware do módulo da cpu philips do MPA2500
 * Autor     : Henrique Pérsico Rossi
 * Data      : 05/12/2007
 * Descrição : Definições relacionadas à camada de aplicação da interface CAN.
 * Notas     :
 *
 *******************************************************************************/
/*******************************************************************************
 * Tratamento de flags da camada de aplicação
 *
 * - Detecção de sensores na rede:
 *
 * 1) Os flags de manutenção e do modo de instalação devem estar setados;
 * 2) A tarefa do controlador CAN, de um em um segundo, atualiza a lista.
 *
 *   Flags que esta tarefa gera:
 *
 *******************************************************************************/
#ifndef APP_SENSOR_SRC_SENSOR_MANAGEMENT_H_
#define APP_SENSOR_SRC_SENSOR_MANAGEMENT_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
//// Listagem de comandos utilizados na rede CAN
//#define CAN_APL_CMD_PNP                           0x01 //Comando de PnP
//#define CAN_APL_CMD_LEITURA_DADOS                 0x02 //Comando de leitura de dados
//#define CAN_APL_CMD_CONFIGURA_SENSOR              0x03 //Comando de configuração do sensor
//#define CAN_APL_RESP_PNP                          0x04 //Resposta ao comando de PnP
//#define CAN_APL_RESP_LEITURA_DADOS                0x05 //Resposta ao comando de leitura de dados
//#define CAN_APL_LEITURA_VELOCIDADE                0x07 //Leitura de dados de velocidade
//#define CAN_APL_RESP_CONFIGURA_SENSOR             0x06 //Resposta ao comando de configuração do sensor
//
//#define CAN_APL_CMD_PARAMETROS_SENSOR             0x08 //Comando de envio de parâmetros de sensor
//#define CAN_APL_RESP_PARAMETROS_SENSOR            0x09
//
//#define CAN_APL_CMD_VERSAO_SW_SENSOR              0x0A //Comando de envio de Versão de Firmware de sensor
//#define CAN_APL_RESP_VERSAO_SW_SENSOR             0x0B
//
//#define CAN_APL_CMD_PARAMETROS_EXTENDED           0x0C //Comando de envio de parâmetros de sensor (Com algorítimo para correção de duplos)
//#define CAN_APL_RESP_PARAMETROS_EXTENDED          0x0D
//
//// Listagem de flags da aplicação CAN
//#define CAN_APL_FLAG_NENHUM                       0x00000000
//#define CAN_APL_FLAG_TODOS                        0xFFFFFFFF
//
//#define CAN_APL_FLAG_TMR_CMD_PNP                  0x00000001
//
//#define CAN_APL_FLAG_MSG_RESP_PNP                 0x00000002
//#define CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS       0x00000004
//#define CAN_APL_FLAG_MSG_RESP_CONFIGURACAO        0x00000008
//
//#define CAN_APL_FLAG_MSG_LEITURA_VELOCIDADE       0x00000010
//
//#define CAN_APL_FLAG_MSG_RESP_PARAMETROS          0x00000020
//#define CAN_APL_FLAG_MSG_RESP_VERSAO_SW_SENSOR    0x00000040
//
//#define CAN_APL_FLAG_PNP_TIMEOUT                  0x00000080
//#define CAN_APL_FLAG_COMANDO_CONFIGURA            0x00000100
//#define CAN_APL_FLAG_COMANDO_TIMEOUT              0x00000200
//#define CAN_APL_FLAG_TODOS_SENS_RESP_PNP          0x00000400
//#define CAN_APL_FLAG_MENSAGEM_LIDA                0x00000800
//
//
////**** A tarefa de aquisição de dados aguarda algum destes flags serem setados
//
//// Eventos relacionados à detecção de sensores
//#define CAN_APL_FLAG_DET_NOVO_SENSOR              0x00001000
//#define CAN_APL_FLAG_DET_SENSOR_RECONECTADO       0x00002000
//
//// Respostas ao comando de leitura de dados
//#define CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP    0x00004000
//#define CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP   0x00008000
//#define CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP    0x00010000
//
//// Respostas ao comando de configuração
//#define CAN_APL_FLAG_CFG_SENSOR_RESPONDEU         0x00020000
//
//// Nenhum sensor conectado à rede Resposta comum a todos os comandos
//#define CAN_APL_FLAG_SENSOR_NAO_RESPONDEU         0x00040000
//#define CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO      0x00080000
//
//// Listagem de dispositivos utilizados na rede CAN
//#define CAN_APL_SENSOR_SEMENTE                    0x00 //Sensor de semente
//#define CAN_APL_SENSOR_ADUBO                      0x01 //Sensor de adubo
//#define CAN_APL_SENSOR_SIMULADOR                  0x02 //Sensor de Velocidade do Simulador
//#define CAN_APL_SENSOR_DIGITAL_2                  0x03 //Sensor digital 2
//#define CAN_APL_SENSOR_DIGITAL_3                  0x04 //Sensor digital 3
//#define CAN_APL_SENSOR_DIGITAL_4                  0x05 //Sensor digital 4
//#define CAN_APL_SENSOR_DIGITAL_5                  0x06 //Sensor digital 5
//#define CAN_APL_SENSOR_DIGITAL_6                  0x07 //Sensor digital 6
//#define CAN_APL_SENSOR_TODOS                      0xFF //Todos os sensores
//#define CAN_APL_LINHA_TODAS                       0xFF //Todas as linhas
//
////#define TRACE_SENSOR_INSTALL
//
//#if defined(TRACE_SENSOR_INSTALL)
//#define CAN_wTICKS_PRIMEIRO_CMD_PNP           2*(3 *TICK )      //Número de ticks para o timer de timeout do comando de PnP
//#define CAN_wTICKS_CMD_PNP                    2*(3*(TICK/2))  //Número de ticks para o timer de envio do comando de PnP
//#define CAN_wTICKS_TIMEOUT_PNP                3*(TICK)      //Número de ticks para o timer de timeout do comando de PnP
//#define CAN_wTICKS_TIMEOUT                    3*(TICK)      //Número de ticks para o timer de timeout de Leitura de Dados
//#define CAN_wTICKS_TIMEOUT_CFG                3*(TICK)      //Número de ticks para o timer de timeout de Configuração
//#define CAN_wTICKS_TIMEOUT_TESTE              5*(TICK)        //Número de ticks para o timer de timeout de Leitura de Dados no Modo Teste
//
//#define CAN_wTICKS_TIMEOUT_VEL                3*(TICK)      //Número de ticks para o timer de timeout de Leitura de Velocidade
//#else
//// Valores para os temporizadores de comunicação:
//#define CAN_wTICKS_PRIMEIRO_CMD_PNP           (3 *TICK )      //Número de ticks para o timer de timeout do comando de PnP
//#define CAN_wTICKS_CMD_PNP                    (3*(TICK/2))  //Número de ticks para o timer de envio do comando de PnP
//#define CAN_wTICKS_TIMEOUT_PNP                (TICK/4)      //Número de ticks para o timer de timeout do comando de PnP
//#define CAN_wTICKS_TIMEOUT                    (TICK/4)      //Número de ticks para o timer de timeout de Leitura de Dados
//#define CAN_wTICKS_TIMEOUT_CFG                (TICK/4)      //Número de ticks para o timer de timeout de Configuração
//#define CAN_wTICKS_TIMEOUT_TESTE              (TICK)        //Número de ticks para o timer de timeout de Leitura de Dados no Modo Teste
//
//#define CAN_wTICKS_TIMEOUT_VEL                (TICK/4)      //Número de ticks para o timer de timeout de Leitura de Velocidade
//#endif
//
//#define CAN_bCONT_CMD_PNP                     1
//#define CAN_bCONT_TIMEOUT_CMD_PNP             1
//#define CAN_bCONT_TIMEOUT_CMD_DADOS           1
//
//// Tamanho do endereço físico de cada sensor na rede
//#define CAN_bTAMANHO_END_FISICO               6
//
//// Macro para calcular a quantidade de elementos de um array
//#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
//#define CAN_bNUM_DE_LINHAS                    36
//#define CAN_bNUM_SENSORES_POR_LINHA           2
//#define CAN_bNUM_SENSORES_DIGITAIS            6
//#define CAN_bNUM_SENSORES_SEMENTE_E_ADUBO     (CAN_bNUM_DE_LINHAS * CAN_bNUM_SENSORES_POR_LINHA)
//#define CAN_bTAMANHO_LISTA                    (CAN_bNUM_SENSORES_SEMENTE_E_ADUBO + CAN_bNUM_SENSORES_DIGITAIS)
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
//Estado do sensor na lista de sensores
//typedef enum {
//  Novo,
//  Conectado,
//  Desconectado,
//  Verificando,
//} CAN_teEstadoSensor;
//
//typedef enum {
//  Nenhum,
//  Reprovado,
//  Aprovado
//} CAN_teAutoTeste;
//typedef struct {
//    bool               bNovo;
//    uint8_t            bTipoSensor;
//    uint8_t            abEnderecoFisico[ CAN_bTAMANHO_END_FISICO ];
//    CAN_teAutoTeste    eResultadoAutoTeste;
//} CAN_tsNovoSensor;
//
//// Estrutura de Versão de SW do Sensor
//typedef struct {
//    uint16_t            wVer;                   //Número da versão.
//    uint16_t            wRev;                   //Número da revisão.
//    uint16_t            wBuild;                 //Número do build.
//}CAN_tsVersaoSensor;
//
//// Lista de sensores na rede CAN
//typedef struct {
//    CAN_teEstadoSensor    eEstado;
//    uint8_t               abEnderecoFisico[ CAN_bTAMANHO_END_FISICO ];
//    uint8_t               abUltimaLeitura[2];
//    CAN_teAutoTeste       eResultadoAutoTeste;
//    CAN_tsVersaoSensor    CAN_sVersaoSensor;
//} CAN_tsLista;
//
//// Estrutura de controle da lista de sensores na rede CAN
//typedef struct {
//    CAN_tsNovoSensor      sNovoSensor;
//    CAN_tsLista           asLista[ CAN_bTAMANHO_LISTA ];
//} CAN_tsCtrlListaSens;
//#pragma pack(pop)
//// Estrutura de parametros de sensores
//typedef struct {
//    uint16_t wMinimo;       //Tempo mínimo em us para contagem de uma semente
//    uint16_t wMaximo;       //Tempo máximo em us para contagem de uma semente
//    uint16_t wMaximoDuplo;  //Tempo máximo em us para contagem de duas sementes
//}CAN_tsParametrosSensor;
//
////Estrutura de parametros de sensores
//typedef struct {
//    uint8_t bMinimo;
//    uint8_t bDuplo;
//    uint8_t bMaximo;
//    uint8_t bFreqAmostra;
//    uint8_t bNAmostra;
//    uint8_t bPasso;
//    uint8_t bDivPicoOffsetMinimo;       // LSN = DivPico     / MSN = OffsetMin      (LSN = Nibble menos significativo / MSN = Nibble mais significativo)
//    uint8_t bOffsetDuploOffsetTriplo;   // LSN = OffsetDuplo / MSN = OffsetTriplo   (LSN = Nibble menos significativo / MSN = Nibble mais significativo)
//}CAN_tsParametrosExtended;
/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/
// Estrutura da lista de dispositivos na rede CAN
extern CAN_tsCtrlListaSens CAN_sCtrlLista;

// Estrutura de parametros de sensores
extern CAN_tsParametrosSensor CAN_sParametrosSensor;

// Estrutura de parametros de sensores com algorítimo para correção de duplos
extern CAN_tsParametrosExtended CAN_sParametrosExtended;

// Mutex para acesso ao buffer da lista de sensores na rede CAN
EXTERN_MUTEX(CAN_MTX_sBufferListaSensores);

// Variável para incrementar a quantidade de pulsos lida pelo sensor de velociadade
extern uint32_t CAN_dLeituraSensor;
extern uint8_t CAN_bSensorSimulador;
extern uint8_t CAN_bNumRespostasPNP;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void SEN_vObtemListaAnterior (void);
void SEN_vArquivoErros (void);
void SEN_vSalvaLista (void);
void SEN_vApagaLista (void);
void SEN_vSalvaArquivoErros (void);

void SEN_vAddNewSensor (const uint8_t bLinha);
void SEN_vReadDataFromSensors (void);
void SEN_vSensorsParameters (uint8_t bComando,
	uint8_t bLinha,
	uint8_t bTipo,
	uint8_t *pbDados,
	uint8_t bDLC);
void SEN_vGetVersion (void);

void SEN_vManagementNetwork (void);

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* APP_SENSOR_SRC_SENSOR_MANAGEMENT_H_ */
