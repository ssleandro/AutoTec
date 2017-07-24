/****************************************************************************
 * Title                 :   acquireg_app
 * Filename              :   acquireg_app.h
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
 * 29/05/2017  1.0.0     Henrique Reis      acquireg_app.h created.
 *
 *****************************************************************************/
#ifndef APP_COMMON_APP_COMMON_ACQUIREG_APP_H_
#define APP_COMMON_APP_COMMON_ACQUIREG_APP_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

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

typedef struct
{

	//uint8_t  abAdubo[32];      //Adubo por linha
	uint32_t dSomaSem;         //Sementes em todas as Linhas
	uint32_t dDistancia;       //Distância Percorrida  em centimetros
	uint32_t dSegundos;        //Tempo em segundos
	float fArea;            //Area
	uint32_t adSementes[36];   //Sementes por linha

} tsLinhas;

typedef struct
{

	uint32_t dDistancia;      //Distância Percorrida  em centimetros - após config
	uint32_t dSomaSem;       //Soma de sementes parcial de todas as linhas

} tsDistanciaTrab;

typedef struct
{

	uint32_t dTEV;          //Tempo total em excesso de velocidade (em trabalho)
	uint32_t dMTEV;         //Máximo intervalo de Tempo em Excesso de Velocidade
	float fVelMax;       //Velocidade Máxima Atingida em excesso de velocidade

} tsVelocidade;

typedef struct
{

	tsLinhas sTrabTotal;     //Acumula valores trabalhando (Total)
	tsLinhas sTrabTotalDir;  //Acumula valores trabalhando em Arremate do lado direito
	tsLinhas sTrabTotalEsq;  //Acumula valores trabalhando em Arremate do lado esquerdo

	tsLinhas sTrabParcial;  //Acumula valores trabalhando (Parcial)
	tsLinhas sTrabParcDir;  //Acumula valores trabalhando em Arremate do lado direito
	tsLinhas sTrabParcEsq;  //Acumula valores trabalhando em Arremate do lado esquerdo

	tsLinhas sManobra;      //Acumula valores manobra (não trabalhando)

	tsLinhas sTotalReg;     //Acumula valores totais para Registro(trabalhando + manobra)

	tsLinhas sAvalia;       //Acumula valores em Avaliação (cálculo da média)

	tsDistanciaTrab sDistTrabTotal;     //Distância parcial - o qual zera após a configuração
	tsDistanciaTrab sDistTrabTotalEsq;  //Distância parcial - o qual zera após a configuração
	tsDistanciaTrab sDistTrabTotalDir;  //Distância parcial - o qual zera após a configuração

	tsDistanciaTrab sDistTrabParcial;     //Distância parcial - o qual zera após a configuração
	tsDistanciaTrab sDistTrabParcialEsq;  //Distância parcial - o qual zera após a configuração
	tsDistanciaTrab sDistTrabParcialDir;  //Distância parcial - o qual zera após a configuração

} tsAcumulados;

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

typedef enum
{
	Sem_Arremate,  //Nenhum lado levantado
	Lado_Esquerdo, //Lado esquerdo levantado
	Lado_Direito   //Lado direito levantado
} AQR_teArremate;

typedef enum
{
	AguardandoEvento,    //Nenhum novo evento
	Instalado,           //Novo sensor instalado
	FalhaAutoTeste,      //Falha no Auto-Teste
	SensorNaoEsperado    //Sensor nao esperado no bus ( e� mais que o numero de linhas )
} AQR_teInstalacao;

typedef struct
{

	uint8_t bPausaAuto;           //Monitor esta em Pausa Automatica
	uint8_t bExVel;               //Excesso de velocidade
	uint8_t bVelZero;             //Velocidade Zero
	uint8_t bTrabalhando;         //Estao Trabalhando

	uint8_t bInsensivel;          //Estao area de insensibilidade
	uint8_t bImplemento;          //Implemento Levantado
	uint8_t bErroGPS;             //Erro no GPS

	AQR_teInstalacao eStatusInstalacao; //Indica se o sensor foi instalado ou se houve falha na instalacao
	uint8_t bSensorDesconectado;          //Indica que algum sensor esta desconectado
	uint8_t bLinhasFalha;                 //Indica se ha Linhas em falha >90% e < 20%
	uint8_t bLinhasZero;                  //Indica se ha Linhas em falha < 90%
	uint8_t bSensorNaoEsperado;           //Indica se foi conectado sensor alem do configurado

	uint8_t bSementeInstalados;   //Quantidade de sensores de semente instalados ate o momento
	uint8_t bAduboInstalados;     //Quantidade de sensores de adubo instalados ate o momento
	uint8_t bAdicionalInstalados; //Quantidade de sensores adicionais instalados ate o momento
	uint8_t bReprovados;          //Quantidade de sensores reprovados no Auto-Teste

	uint8_t bAdicionalDesconectado;    //Sensores Adicionais desconectados
	uint8_t bMemAdicionalDesconectado; //Memoriza Sensores adicionais desconectados
	//uint8_t   bLinhaSementeIgnorado;     //Numero da linha que o sensor de semente esta sendo ignorado
	//uint8_t   bLinhaAduboIgnorado;       //Numero da linha que o sensor de adubo esta sendo ignorado

	uint8_t bAutoTeste;             //Informa se esta em auto-teste
	uint8_t bAlarmeOK;              //Informa se o alarme foi reconhecido
	uint8_t bAlarmeGPS;              //Informa o alarme do GPS
	AQR_teArremate eArremate;     //Informa se esta ou nao esta em Arremate
	uint8_t bNumLinhasSemIntercalar;
//  AQR_teIntercala  eIntercala;

	uint8_t bNumLinhasZero;            //Quantidade de linhas com Zero sementes
	uint8_t bNumLinhasAtivas;          //Quantidade de linhas que nao estao levantadas (em arremate)
	uint8_t bNumLinhasDir;             //Quantidade de linhas que estao ativas do lado direito
	uint8_t bNumLinhasEsq;             //Quantidade de linhas que estao ativas do lado esquerdo

	uint8_t bNumSensores;         //Nao total de sensores configurados pelo usuario
	uint8_t bLinhaDisponivel;     //Informa qual linha estao disponível para instalação do novo sensor
	uint8_t bSensorAdicionado;    //Informa qual Sensor foi adicionado
	uint8_t bCfgSensorRespondeu;    //Informa se o sensor respondeu a configuracao

	uint32_t dSementeFalha;          //Linha com Sensores de Semente em Falha
	uint32_t dSementeFalhaExt;       //Linha com Sensores de Semente em Falha Extendida
	uint32_t dSementeFalhaIHM;       //Flags que indica na IHM as falhas de semente
	uint32_t dSementeFalhaIHMExt;    //Flags que indica na IHM as falhas de semente Extendida
	uint32_t dAduboFalha;            //Linha com Sensores de Adubo em Falha
	uint32_t dAduboFalhaExt;         //Linha com Sensores de Adubo em Falha Extendida
	uint32_t dLinhaDesconectada;     //Linhas com sensor desconectado
	uint32_t dLinhaDesconectadaExt;  //Linhas com sensor desconectado Extendida
	uint32_t dMemLinhaDesconectada;  //Memoriza Linhas com sensor desconectado
	uint32_t dMemLinhaDesconectadaExt;
	uint32_t dLinhasLevantadas;      //(Linhas levantadas = 1, Linhas ativas = 0)
	uint32_t dLinhasLevantadasExt;
	uint32_t dSementeIgnorado;       //Linha com sensor de Semente Ignorado;
	uint32_t dSementeIgnoradoExt;    //Extendido
	uint32_t dAduboIgnorado;         //Linha com sensor de Adubo Ignorado;
	uint32_t dAduboIgnoradoExt;      //Extendido
	uint32_t dSementeZero;           //Flags que indica falha de semente proximo de zero
	uint32_t dSementeZeroExt;        //Extendido
	uint32_t dSementeZeroIHM;        //Flags que indica falha de semente proximo de zero para IHM
	uint32_t dSementeZeroIHMExt;     //Extendido
	uint16_t wMaxSementes;           //Tolerancia maxima de sementes
	uint16_t wMinSementes;           //Tolerancia maxima de sementes

	uint16_t wMinSementesZero;      //Tolerância mi�nima de sementes para considerar que nao estao caindo sementes

	uint16_t awMediaSementes[36];    //Media de sementes plantadas por metro * 100 (com duas casas decimais)
	uint16_t wAvaliaArred;

	uint8_t bPrimeiroSegmento;
	uint8_t bAlarmeLinhasLevantadas; //Contador de linhas de falha de linhas levantada

} tsStatus;

typedef struct
{
	tsAcumulados *AQR_sAcumulado;
	tsStatus *AQR_sStatus;
}tsPubPlantData;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#endif /* APP_COMMON_APP_COMMON_ACQUIREG_APP_H_ */
