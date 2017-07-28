/****************************************************************************
 * Title                 :   sensor_app
 * Filename              :   sensor_app.h
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
 * 29/05/2017  1.0.0     Henrique Reis      sensor_app.h created.
 *
 *****************************************************************************/
#ifndef APP_COMMON_APP_COMMON_SENSOR_APP_H_
#define APP_COMMON_APP_COMMON_SENSOR_APP_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

// Listagem de dispositivos utilizados na rede CAN
#define CAN_APL_SENSOR_SEMENTE                    0x00 //Sensor de semente
#define CAN_APL_SENSOR_ADUBO                      0x01 //Sensor de adubo
#define CAN_APL_SENSOR_SIMULADOR                  0x02 //Sensor de Velocidade do Simulador
#define CAN_APL_SENSOR_DIGITAL_2                  0x03 //Sensor digital 2
#define CAN_APL_SENSOR_DIGITAL_3                  0x04 //Sensor digital 3
#define CAN_APL_SENSOR_DIGITAL_4                  0x05 //Sensor digital 4
#define CAN_APL_SENSOR_DIGITAL_5                  0x06 //Sensor digital 5
#define CAN_APL_SENSOR_DIGITAL_6                  0x07 //Sensor digital 6
#define CAN_APL_SENSOR_TODOS                      0xFF //Todos os sensores
#define CAN_APL_LINHA_TODAS                       0xFF //Todas as linhas

//#define TRACE_SENSOR_INSTALL

#if defined(TRACE_SENSOR_INSTALL)
#define CAN_wTICKS_PRIMEIRO_CMD_PNP           2*(3 *TICK )      //Numero de ticks para o timer de timeout do comando de PnP
#define CAN_wTICKS_CMD_PNP                    2*(3*(TICK/2))  //Numero de ticks para o timer de envio do comando de PnP
#define CAN_wTICKS_TIMEOUT_PNP                3*(TICK)      //Numero de ticks para o timer de timeout do comando de PnP
#define CAN_wTICKS_TIMEOUT                    3*(TICK)      //Numero de ticks para o timer de timeout de Leitura de Dados
#define CAN_wTICKS_TIMEOUT_CFG                3*(TICK)      //Numero de ticks para o timer de timeout de Configuração
#define CAN_wTICKS_TIMEOUT_TESTE              5*(TICK)        //Numero de ticks para o timer de timeout de Leitura de Dados no Modo Teste

#define CAN_wTICKS_TIMEOUT_VEL                3*(TICK)      //Numero de ticks para o timer de timeout de Leitura de Velocidade
#else
// Valores para os temporizadores de comunicacao:
#define CAN_wTICKS_PRIMEIRO_CMD_PNP           (3 *TICK )      //Numero de ticks para o timer de timeout do comando de PnP
#define CAN_wTICKS_CMD_PNP                    (3*(TICK/2))  //Numero de ticks para o timer de envio do comando de PnP
#define CAN_wTICKS_TIMEOUT_PNP                (TICK/4)      //Numero de ticks para o timer de timeout do comando de PnP
#define CAN_wTICKS_TIMEOUT                    (TICK/4)      //Numero de ticks para o timer de timeout de Leitura de Dados
#define CAN_wTICKS_TIMEOUT_CFG                (TICK/4)      //Numero de ticks para o timer de timeout de Configuração
#define CAN_wTICKS_TIMEOUT_TESTE              (TICK)        //Numero de ticks para o timer de timeout de Leitura de Dados no Modo Teste

#define CAN_wTICKS_TIMEOUT_VEL                (TICK/4)      //Numero de ticks para o timer de timeout de Leitura de Velocidade
#endif

#define CAN_bCONT_CMD_PNP                     1
#define CAN_bCONT_TIMEOUT_CMD_PNP             1
#define CAN_bCONT_TIMEOUT_CMD_DADOS           1

// Tamanho do endereco fi�sico de cada sensor na rede
#define CAN_bTAMANHO_END_FISICO               6

// Macro para calcular a quantidade de elementos de um array
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

#define CAN_bNUM_DE_LINHAS                    36
#define CAN_bNUM_SENSORES_POR_LINHA           2
#define CAN_bNUM_SENSORES_DIGITAIS            6
#define CAN_bNUM_SENSORES_SEMENTE_E_ADUBO     (CAN_bNUM_DE_LINHAS * CAN_bNUM_SENSORES_POR_LINHA)
#define CAN_bTAMANHO_LISTA                    (CAN_bNUM_SENSORES_SEMENTE_E_ADUBO + CAN_bNUM_SENSORES_DIGITAIS)

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/
// Pin to enable/disable sensor power source
gpio_config_s sEnablePS9;

#define ENABLE_PS9 GPIO_vClear(&sEnablePS9)     // Enable sensor power source
#define DISABLE_PS9 GPIO_vSet(&sEnablePS9)      // Disable sensor power source

/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef enum
{
	Novo,
	Conectado,
	Desconectado,
	Verificando,
} CAN_teEstadoSensor;

typedef enum
{
	Nenhum,
	Reprovado,
	Aprovado
} CAN_teAutoTeste;

typedef struct
{
	bool bNovo;
	uint8_t bTipoSensor;
	uint8_t abEnderecoFisico[6];
	CAN_teAutoTeste eResultadoAutoTeste;
} CAN_tsNovoSensor;

// Estrutura de Versao de SW do Sensor
typedef struct
{
	uint16_t wVer;                   //Numero da versao.
	uint16_t wRev;                   //Numero da revisao.
	uint16_t wBuild;                 //Numero do build.
} CAN_tsVersaoSensor;

// Lista de sensores na rede CAN
typedef struct
{
	CAN_teEstadoSensor eEstado;
	uint8_t abEnderecoFisico[6];
	uint8_t abUltimaLeitura[2];
	CAN_teAutoTeste eResultadoAutoTeste;
	CAN_tsVersaoSensor CAN_sVersaoSensor;
} CAN_tsLista;

// Estrutura de controle da lista de sensores na rede CAN
typedef struct
{
	CAN_tsNovoSensor sNovoSensor;
	CAN_tsLista asLista[CAN_bTAMANHO_LISTA];
} CAN_tsCtrlListaSens;

// Estrutura de parametros de sensores
typedef struct
{
	uint16_t wMinimo;       //Tempo minimo em us para contagem de uma semente
	uint16_t wMaximo;       //Tempo maximo em us para contagem de uma semente
	uint16_t wMaximoDuplo;  //Tempo maximo em us para contagem de duas sementes
} CAN_tsParametrosSensor;

//Estrutura de parametros de sensores
typedef struct
{
	uint8_t bMinimo;
	uint8_t bDuplo;
	uint8_t bMaximo;
	uint8_t bFreqAmostra;
	uint8_t bNAmostra;
	uint8_t bPasso;
	uint8_t bDivPicoOffsetMinimo; // LSN = DivPico     / MSN = OffsetMin      (LSN = Nibble menos significativo / MSN = Nibble mais significativo)
	uint8_t bOffsetDuploOffsetTriplo; // LSN = OffsetDuplo / MSN = OffsetTriplo   (LSN = Nibble menos significativo / MSN = Nibble mais significativo)
} CAN_tsParametrosExtended;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#endif /* APP_COMMON_APP_COMMON_SENSOR_APP_H_ */
