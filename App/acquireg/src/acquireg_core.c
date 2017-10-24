/****************************************************************************
 * Title                 :   acquireg_core
 * Filename              :   acquireg_core.c
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
 *  17/04/17   1.0.0     Henrique Reis         acquireg_core.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "acquireg_core.h"
#include "file_tool.h"
#include "debug_tool.h"
#include "../../acquireg/config/acquireg_config.h"
#include "acquireg_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of BUZZER queue
#define QUEUE_SIZEOFBUZZER (5)
#define THIS_MODULE MODULE_ACQUIREG

/*!<< From MPA2500  */
// Tamanho do Buffer em anel para média de distãncia e sementes.
#define AQR_wTAM_BUF_ANEL 256
#define AQR_wMSK_BUF_ANEL 0x00FF

#define AQR_DISTANCIA_LIMPA_FALHA  10

#define AQR_dTAMANHO_HEADER  ( uint32_t )( sizeof( UOS_sVersaoCod        ) + \
        sizeof( UOS_sConfiguracao     )     )

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/
extern osFlagsGroupId UOS_sFlagSis;
osFlagsGroupId xAQR_sFlagSis;
osFlagsGroupId xGPS_sFlagGPS;
osFlagsGroupId xSEN_sFlagApl;

extern GPS_tsDadosGPS GPS_sDadosGPS;

extern osMutexId GPS_MTX_sEntradas;
extern uint8_t GPS_bDistanciaPercorrida;
uint8_t AQR_bDistanciaPercorrida;
EXTERN_TIMER(GPS_bTimerMtr);

extern void GPS_dDataHoraLocal (uint8_t*);
extern uint32_t GPS_dDataHoraSistema (void);

extern uint8_t CAN_bSensorSimulador;
extern uint8_t CAN_bNumRespostasPNP;
extern CAN_tsCtrlListaSens CAN_sCtrlLista;
EXTERN_MUTEX(CAN_MTX_sBufferListaSensores);
extern void SEN_vAddNewSensor (const uint8_t);
extern void SEN_vReadDataFromSensors (void);

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/*!<< From MPA2500  */
//Buffer em anel para handler:
typedef struct
{
	volatile uint16_t wPosIns;        //Posição de inserção no buffer.
	volatile uint16_t wPosRet;        //Posição de retirada no buffer.
	uint16_t wMskBufAnel;    //Mascara para fechamento do buffer.
	uint16_t awBufDis[AQR_wTAM_BUF_ANEL];  // buffer para distância
	uint8_t abBufSem[CAN_bNUM_DE_LINHAS][AQR_wTAM_BUF_ANEL]; // buffer para sensores
	uint8_t abBufAdu[CAN_bNUM_DE_LINHAS][AQR_wTAM_BUF_ANEL]; // buffer para sensores
	int32_t alBufSemFrac[CAN_bNUM_DE_LINHAS]; //Fração de sementes para sensores ignorados
} tsBufSegmentos;

/******************************************************************************
 * Public Variables
 *******************************************************************************/

extern UOS_tsConfiguracao UOS_sConfiguracao;

/*!<< From MPA2500  */
//Mutex para controle de acesso às estruturas de dados das entradas e variáveis
//da aquisição e registro:
CREATE_MUTEX(AQR_MTX_sEntradas);
CREATE_MUTEX(AQR_MTX_sBufferListaSensores);
CREATE_MUTEX(AQR_MTX_sBufferAcumulado);

//flag indicador da atualização das informações de Aquisição:
osFlagsGroupId AQR_sFlagREG;
//Estrutura de dados do GPS
GPS_tsDadosGPS AQR_sDadosGPS;
//Estrutura de dados do CAN
CAN_tsCtrlListaSens AQR_sDadosCAN;
//Estrutura de Status do Monitor
tsStatus AQR_sStatus;

//Estrutura de valores Acumulados
tsAcumulados AQR_sAcumulado;
//Estrutura de valores relativos à velocidade
tsVelocidade AQR_sVelocidade;
//Variável que indica Status para registro
uint8_t AQR_bStatusReg;
//Status da aquisição e registro:
uint8_t AQR_bStatus = AQR_bSTS_NAO_INICIADO;
//Variável para auxiliar para indicar se gravação de registros está ativada
uint8_t AQR_bSalvaRegistro;
//Variável que indica o que ocasionou o alarme:
uint16_t AQR_wAlarmes;
//Último número usado para renomear um arquivo de registro:
uint16_t AQR_wNumArqReg;
//Última causa de encerramento de registro.
uint16_t AQR_wCausaFim;
//Variável auxiliar para Largura do Implemento/ Distância entre linhas
uint16_t AQR_wEspacamento;

uint8_t bLimpaFalhas = false;

AQR_teArremate eMemArremate;

CAN_tsCtrlListaSens AQR_sPubCtrlLista;

tsAcumulados AQR_sPubAcumulado;
tsStatus AQR_sPubStatus;
tsPubPlantData AQR_sPubPlantData;
tsPubSensorReplacement AQR_sPubTrocaSensor;

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
//static eAPPError_s eError; //!< Error variable
DECLARE_QUEUE(AcquiregQueue, QUEUE_SIZEOFBUZZER);   //!< Declaration of Interface Queue
CREATE_SIGNATURE(AcquiregControl);//!< Signature Declarations
CREATE_SIGNATURE(AcquiregSensor);//!< Signature Declarations
CREATE_SIGNATURE(AcquiregGPS);//!< Signature Declarations
CREATE_SIGNATURE(AcquiregGPSMetro);//!< Signature Declarations
CREATE_SIGNATURE(AcquiregFileSys);
CREATE_SIGNATURE(AcquiregGUI);
CREATE_CONTRACT(Acquireg);   //!< Create contract for buzzer msg publication
CREATE_CONTRACT(AcquiregSave);   //!< Create contract for buzzer msg publication

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
ACQUIREG_MODULES };
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

WATCHDOG_CREATE(AQRPUB);//!< WDT pointer flag
WATCHDOG_CREATE(AQRTIM);//!< WDT pointer flag
WATCHDOG_CREATE(AQRMGT);//!< WDT pointer flag
uint8_t bAQRPUBThreadArrayPosition = 0; //!< Thread position in array
uint8_t bAQRTIMThreadArrayPosition = 0; //!< Thread position in array
uint8_t bAQRMGTThreadArrayPosition = 0; //!< Thread position in array

/*!<< From MPA2500  */

//Registro estático com CRC:
AQR_tsRegEstaticoCRC sRegEstaticoCRC;
//buffer de dados em trabalho
tsBufSegmentos sSegmentos;
tsFalhaInstantanea AQR_sFalhaInstantanea;

uint8_t bLinhasOffset;
uint8_t bApagaSensorReprovado;
uint8_t abDataHoraCiclo[8]; //Buffer para a data/hora de cada ciclo de aquisição e registro:
uint32_t dDataHoraSisCiclo;
uint32_t dDistInsens; //Distância de Insensibilidade
uint32_t dLinhaAtual;
uint32_t dLinhaAtualExt;
uint32_t dPEV; //Excesso de Velocidade Parcial

uint32_t dLinhasArremate;
uint32_t dLinhasArremateExt;
uint32_t dLinhasIntercaladas;
uint32_t dLinhasIntercaladasExt;
uint8_t abTimeoutAlarme[CAN_bNUM_DE_LINHAS];

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

void AQR_vTimerCallbackTurnOff (void const*);
void AQR_vTimerCallbackImpStopped (void const*);
void AQR_vRepeteTesteSensores (void);
void AQR_vApagaInstalacao (void);
void AQR_vIgnoreSensors(uint8_t bLineNum, bool bIgnored);
void AQR_vTrimmingLines(eTrimming eTrimmStatus);
void AQR_vTrocaSensores(eEventType);
void AQR_vVerificarTrocaSensores(void);

/******************************************************************************
 * Module timers
 *******************************************************************************/
//Timer do Auto Shutdown  30 min
//osTimerId      bTimerDesliga;
//osTimerId      bTimerImpParado;
CREATE_TIMER(AQR_bTimerTurnOff, AQR_vTimerCallbackTurnOff);
CREATE_TIMER(AQR_bTimerImpStopped, AQR_vTimerCallbackImpStopped);

/******************************************************************************
 * Function Definitions from MPA2500
 *******************************************************************************/

/*******************************************************************************

 void AQR_vZeraRegs( uint8_t bTudo )

 Descrição : rotina para reinicialização das variáveis e contadores do
 trabalho.
 Parâmetros: bTudo - indica se deve zerar o Odômetro e tempo total trabalhado.
 Retorno   : nenhum.

 *******************************************************************************/
void AQR_vZeraRegs (uint8_t bTudo)
{
	uint8_t bErr;

	//Copia Valores Acumulados Totais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sTrabTotal, &AQR_sAcumulado.sTrabTotal, sizeof(AQR_sAcumulado.sTrabTotal));

	//Copia Valores Acumulados Totais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sTrabTotalDir, &AQR_sAcumulado.sTrabTotalDir, sizeof(AQR_sAcumulado.sTrabTotalDir));

	//Copia Valores Acumulados Totais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sTrabTotalEsq, &AQR_sAcumulado.sTrabTotalEsq, sizeof(AQR_sAcumulado.sTrabTotalEsq));

	//Copia Valores Acumulados Parciais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sReg.sTrabParcial, &AQR_sAcumulado.sTrabParcial, sizeof(AQR_sAcumulado.sTrabParcial));

	//Copia Valores Acumulados Parciais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sReg.sTrabParcDir, &AQR_sAcumulado.sTrabParcDir, sizeof(AQR_sAcumulado.sTrabParcDir));

	//Copia Valores Acumulados Parciais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sReg.sTrabParcEsq, &AQR_sAcumulado.sTrabParcEsq, sizeof(AQR_sAcumulado.sTrabParcEsq));

	//Copia Valores relativos à velocidade
	memcpy(&sRegEstaticoCRC.sReg.sVelocidade, &AQR_sVelocidade, sizeof(AQR_sVelocidade));

	//Copia Valores da última média calculada em cada linha
	memcpy(&sRegEstaticoCRC.awMediaSementes, &AQR_sStatus.awMediaSementes, sizeof(AQR_sStatus.awMediaSementes));

	//Ajusta a causa de fim:
	AQR_wCausaFim = AQR_wCF_ZERA_PARCIAL;

	//Pede a criação de um novo registro:
	osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_NOVO_REG);

	//----------------------------------------------------------------------------
	//Zera variáveis de excesso de velocidade
	AQR_sVelocidade.dTEV = 0; //Zera Tempo total em excesso de velocidade (em trabalho)
	AQR_sVelocidade.dMTEV = 0; //Zera Máximo intervalo de Tempo em Excesso de Velocidade
	AQR_sVelocidade.fVelMax = 0.0f; //Zera Velocidade Máxima Atingida em excesso de velocidade
	dPEV = 0;

	//Zera distância de insensibilidade (entra em insensibilidade)
	dDistInsens = 0;

	//----------------------------------------------------------------------------
	//Zera valores parciais em trabalho:

	WAIT_MUTEX(AQR_MTX_sBufferAcumulado, osWaitForever);

	// Zera acumulados parciais
	memset(&AQR_sAcumulado.sTrabParcial, 0, sizeof(AQR_sAcumulado.sTrabParcial));
	memset(&AQR_sAcumulado.sTrabParcDir, 0, sizeof(AQR_sAcumulado.sTrabParcDir));
	memset(&AQR_sAcumulado.sTrabParcEsq, 0, sizeof(AQR_sAcumulado.sTrabParcEsq));

	memset(&AQR_sAcumulado.sAvalia, 0, sizeof(AQR_sAcumulado.sAvalia));
	memset(&AQR_sStatus.awMediaSementes, 0, sizeof(AQR_sStatus.awMediaSementes));

	//Zera buffer de segmentos
	memset(&sSegmentos, 0, sizeof(sSegmentos));

	//Ajusta Mascára Buffer em Anel
	sSegmentos.wMskBufAnel = AQR_wMSK_BUF_ANEL;

	//Limpa Linhas pra pausa automática
	AQR_sStatus.bNumLinhasZero = 0;

	//Limpa área e distância parcial
	memset(&AQR_sAcumulado.sDistTrabParcial, 0, sizeof(AQR_sAcumulado.sDistTrabParcial));
	memset(&AQR_sAcumulado.sDistTrabParcialEsq, 0, sizeof(AQR_sAcumulado.sDistTrabParcial));
	memset(&AQR_sAcumulado.sDistTrabParcialDir, 0, sizeof(AQR_sAcumulado.sDistTrabParcial));

	//Se foi solicitado para zerar totais
	if (bTudo != false)
	{
		//Ajusta a causa de fim:
		AQR_wCausaFim = AQR_wCF_ZERA_TOTAL;

		//--------------------------------------------------------------------------
		//Zera valores totais acumulados:

		memset(&AQR_sAcumulado.sTrabTotal, 0, sizeof(AQR_sAcumulado.sTrabTotal)); //trabalhando
		memset(&AQR_sAcumulado.sTrabTotalDir, 0, sizeof(AQR_sAcumulado.sTrabTotalDir)); //trabalhando
		memset(&AQR_sAcumulado.sTrabTotalEsq, 0, sizeof(AQR_sAcumulado.sTrabTotalEsq)); //trabalhando

		//Limpa área e distância total
		memset(&AQR_sAcumulado.sDistTrabTotal, 0, sizeof(AQR_sAcumulado.sDistTrabTotal));
		memset(&AQR_sAcumulado.sDistTrabTotalEsq, 0, sizeof(AQR_sAcumulado.sDistTrabTotalEsq));
		memset(&AQR_sAcumulado.sDistTrabTotalDir, 0, sizeof(AQR_sAcumulado.sDistTrabTotalDir));

		memset(&AQR_sAcumulado.sTotalReg, 0, sizeof(AQR_sAcumulado.sTotalReg)); //registro
		memset(&AQR_sAcumulado.sManobra, 0, sizeof(AQR_sAcumulado.sManobra)); // manobra

	}

	RELEASE_MUTEX(AQR_MTX_sBufferAcumulado);

} //Fim da função AQR_vZeraRegs.

/*******************************************************************************

 void AQR_vParametros( void )

 Descrição : Calcula o valor mínimo e máximo de sementes por 100 metros
 de acordo com a configuração inserida na IHM pelo usuário.
 Arredonda distância para avaliação
 Calcula espaçamento se for monitor de área ou de sementes
 Durante a insensibilidade a tolerância admitida deverá ser no mínimo 90%.
 Parâmetros: nenhum.
 Retorno   : nenhum.

 *******************************************************************************/
void AQR_vParametros (void)
{
	uint32_t dDelta;
	uint16_t wPopulacao;
	UOS_tsCfgMonitor *psMonitor;

	//Ponteiro de trabalho da Configuração do Monitor
	psMonitor = &UOS_sConfiguracao.sMonitor;

	//----------------------------------------------------------------------------
	//Calcula tolerância máxima e mínima de sementes por 100 metros

	//Semente por metro
	wPopulacao = (psMonitor->wSementesPorMetro);

	//Delta da tolerancia sem dividir por 100
	dDelta = ((psMonitor->bTolerancia * wPopulacao));

	//Quantidade máximo tolerável de sementes por 100 metros
	AQR_sStatus.wMaxSementes = (((uint32_t)wPopulacao * 100) + dDelta) / 10;

	//Quantidade mínima tolerável de sementes por 100 metros
	AQR_sStatus.wMinSementes = (((uint32_t)wPopulacao * 100) - dDelta) / 10;

	//Verifica se a tolerância configurada é maior que 90%
	if (psMonitor->bTolerancia > 90)
	{ //Se sim, a tolerância minima para se considerar que não estão caindo sementes será a configurada do MPA
		AQR_sStatus.wMinSementesZero = AQR_sStatus.wMinSementes;
	}
	else
	{ //Se não, A tolerância minima para se considerar que não estão caindo sementes será de 90%
	  //Quantidade máxima tolerável sem dividir por 100
		AQR_sStatus.wMinSementesZero = wPopulacao;
	}

	/*
	 bMinAdubo = ( (psMonitor->bNumLinhas * ( 100 - psMonitor->bTolAdubo) ) /100 );
	 */

	//--------------------------------------------------------------------------
	//Calcula o Espaçamento
	//Se não estiver em modo Monitor de Área
	if (psMonitor->bMonitorArea == false)
	{
		//Se tem intercalação de linhas, dobra o tamanho do espaçamento
		//entre linhas
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			//O espaçamento é a distância entre linhas x 2
			AQR_wEspacamento = (psMonitor->wDistLinhas * 2);
		}
		else
		{
			//O espaçamento é a distância entre linhas
			AQR_wEspacamento = psMonitor->wDistLinhas;
		}
	}
	else
	{
		//O espaçamento é a largura do implemento
		AQR_wEspacamento = psMonitor->wLargImpl;
	}

	//--------------------------------------------------------------------------
	//Define quantas linhas tem de cada lado da plantadeira
	//Se a divisão da plantadeira está no lado esquerdo
	if (psMonitor->bDivLinhas == 0)
	{
		//O número de linhas do lado esq é o maior lado da plantadeira
		AQR_sStatus.bNumLinhasEsq = (psMonitor->bNumLinhas + 1) >> 1;
	}
	else //Se a divisão da plantadeira está no lado direito
	{
		//O número de linhas do lado esq é o menor lado da plantadeira
		AQR_sStatus.bNumLinhasEsq = (psMonitor->bNumLinhas >> 1);
	}

	AQR_sStatus.bNumLinhasDir = psMonitor->bNumLinhas - AQR_sStatus.bNumLinhasEsq;
}

/*******************************************************************************

 void AQR_vAcumulaArea( void )

 Descrição : Calcula área
 Parâmetros: psAcum    - Acumulado Total ou parcial para toda plantadeira
 psAcumDir - Acumulado Total ou parcial para o lado direito
 psAcumEsq - Acumulado Total ou parcial para o lado Esquerdo
 Retorno   : nenhum.

 *******************************************************************************/
void AQR_vAcumulaArea (void)
{
	tsStatus *psStatus = &AQR_sStatus;
	UOS_tsCfgMonitor *psMonitor = &UOS_sConfiguracao.sMonitor;
	//Aponta para estrutura dos acumulados
	tsLinhas* const psAcum = &AQR_sAcumulado.sTrabTotal;
	tsLinhas* const psAcumDir = &AQR_sAcumulado.sTrabTotalDir;
	tsLinhas* const psAcumEsq = &AQR_sAcumulado.sTrabTotalEsq;
	//Aponta para estrutura dos parciais
	tsLinhas* const psParcial = &AQR_sAcumulado.sTrabParcial;
	tsLinhas* const psParcDir = &AQR_sAcumulado.sTrabParcDir;
	tsLinhas* const psParcEsq = &AQR_sAcumulado.sTrabParcEsq;

	float fArea;
	float fAreaDir;
	float fAreaEsq;
	//Aponta para estrutura dos acumulados de distância totais
	tsDistanciaTrab* const psAcumDis = &AQR_sAcumulado.sDistTrabTotal;
	tsDistanciaTrab* const psAcumDisDir = &AQR_sAcumulado.sDistTrabTotalDir;
	tsDistanciaTrab* const psAcumDisEsq = &AQR_sAcumulado.sDistTrabTotalEsq;
	//Aponta para estrutura dos acumulados de distância parciais
	tsDistanciaTrab* const psParcDis = &AQR_sAcumulado.sDistTrabParcial;
	tsDistanciaTrab* const psParcDisDir = &AQR_sAcumulado.sDistTrabParcialDir;
	tsDistanciaTrab* const psParcDisEsq = &AQR_sAcumulado.sDistTrabParcialEsq;


	//Acumula área parcial
	//Divide por 10 porque AQR_wEspacamento está em cm*10
	fArea = ((float)AQR_wEspacamento * (float)psParcDis->dDistancia) * 0.1f;
	fAreaDir = ((float)AQR_wEspacamento * (float)psParcDisDir->dDistancia) * 0.1f;
	fAreaEsq = ((float)AQR_wEspacamento * (float)psParcDisEsq->dDistancia) * 0.1f;

	//Converte de cm² para m²
	fArea *= (1.0f / 10000.0f);
	fAreaDir *= (1.0f / 10000.0f);
	fAreaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			psParcial->fArea += psStatus->bNumLinhasSemIntercalar * fArea;
		}
		else
		{
			psParcial->fArea += psMonitor->bNumLinhas * fArea;
		}

		psParcDir->fArea += psStatus->bNumLinhasDir * fAreaDir;
		psParcEsq->fArea += psStatus->bNumLinhasEsq * fAreaEsq;
		sRegEstaticoCRC.sReg.sTrabParcDir.fArea = psParcDir->fArea;
		sRegEstaticoCRC.sReg.sTrabParcEsq.fArea = psParcEsq->fArea;

	}
	else
	{
		psParcial->fArea += fArea;
	}

	//Inclui o valor no registro
	sRegEstaticoCRC.sReg.sTrabParcial.fArea = psParcial->fArea;

	//Acumula área total
	//Divide por 10 porque AQR_wEspacamento está em cm*10
	fArea = ((float)AQR_wEspacamento * (float)psAcumDis->dDistancia) * 0.1f;
	fAreaDir = ((float)AQR_wEspacamento * (float)psAcumDisDir->dDistancia) * 0.1f;
	fAreaEsq = ((float)AQR_wEspacamento * (float)psAcumDisEsq->dDistancia) * 0.1f;

	//Converte de cm² para m²
	fArea *= (1.0f / 10000.0f);
	fAreaDir *= (1.0f / 10000.0f);
	fAreaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			psAcum->fArea += psStatus->bNumLinhasSemIntercalar * fArea;
		}
		else
		{
			psAcum->fArea += psMonitor->bNumLinhas * fArea;
		}

		psAcumDir->fArea += psStatus->bNumLinhasDir * fAreaDir;
		psAcumEsq->fArea += psStatus->bNumLinhasEsq * fAreaEsq;

		sRegEstaticoCRC.sTrabTotalDir.fArea = psAcumDir->fArea;
		sRegEstaticoCRC.sTrabTotalEsq.fArea = psAcumEsq->fArea;
	}
	else
	{
		psAcum->fArea += fArea;
	}

	//Inclui o valor no registro
	sRegEstaticoCRC.sTrabTotal.fArea = psAcum->fArea;

	//Zera as estruturas de distância totais e parciais
	memset(&AQR_sAcumulado.sDistTrabParcial, 0, sizeof(AQR_sAcumulado.sDistTrabParcial));
	memset(&AQR_sAcumulado.sDistTrabParcialDir, 0, sizeof(AQR_sAcumulado.sDistTrabParcialDir));
	memset(&AQR_sAcumulado.sDistTrabParcialEsq, 0, sizeof(AQR_sAcumulado.sDistTrabParcialEsq));

	memset(&AQR_sAcumulado.sDistTrabTotal, 0, sizeof(AQR_sAcumulado.sDistTrabTotal));
	memset(&AQR_sAcumulado.sDistTrabTotalDir, 0, sizeof(AQR_sAcumulado.sDistTrabTotalDir));
	memset(&AQR_sAcumulado.sDistTrabTotalEsq, 0, sizeof(AQR_sAcumulado.sDistTrabTotalEsq));

}

void AQR_vPubAcumulaArea (void)
{
	tsStatus *psStatus = &AQR_sStatus;
	UOS_tsCfgMonitor *psMonitor = &UOS_sConfiguracao.sMonitor;

	float fArea = AQR_sAcumulado.sTrabParcial.fArea;
	float fAreaEsq = AQR_sAcumulado.sTrabParcEsq.fArea;
	float fAreaDir = AQR_sAcumulado.sTrabParcDir.fArea;

	float fIncArea;
	float fIncAreaDir;
	float fIncAreaEsq;

	uint32_t dDist = AQR_sAcumulado.sDistTrabParcial.dDistancia;
	uint32_t dDistEsq = AQR_sAcumulado.sDistTrabParcialEsq.dDistancia;
	uint32_t dDistDir = AQR_sAcumulado.sDistTrabParcialDir.dDistancia;

	//Acumula área parcial
	//Divide por 10 porque AQR_wEspacamento está em cm*10
	fIncArea = ((float)AQR_wEspacamento * (float)dDist) * 0.1f;
	fIncAreaDir = ((float)AQR_wEspacamento * (float)dDistDir) * 0.1f;
	fIncAreaEsq = ((float)AQR_wEspacamento * (float)dDistEsq) * 0.1f;

	//Converte de cm² para m²
	fIncArea *= (1.0f / 10000.0f);
	fIncAreaDir *= (1.0f / 10000.0f);
	fIncAreaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			fArea += psStatus->bNumLinhasSemIntercalar * fIncArea;
		}
		else
		{
			fArea += psMonitor->bNumLinhas * fIncArea;
		}

		fAreaDir += psStatus->bNumLinhasDir * fIncAreaDir;
		fAreaEsq += psStatus->bNumLinhasEsq * fIncAreaEsq;
		sRegEstaticoCRC.sReg.sTrabParcDir.fArea = fAreaDir;
		sRegEstaticoCRC.sReg.sTrabParcEsq.fArea = fAreaEsq;

	}
	else
	{
		fArea += fIncArea;
	}

	//Inclui o valor no registro
	sRegEstaticoCRC.sReg.sTrabParcial.fArea = fArea;

	//Acumula área total

	fArea = AQR_sAcumulado.sTrabTotal.fArea;
	fAreaEsq = AQR_sAcumulado.sTrabTotalEsq.fArea;
	fAreaDir = AQR_sAcumulado.sTrabTotalDir.fArea;

	dDist = AQR_sAcumulado.sDistTrabTotal.dDistancia;
	dDistEsq = AQR_sAcumulado.sDistTrabTotalEsq.dDistancia;
	dDistDir = AQR_sAcumulado.sDistTrabTotalDir.dDistancia;

	//Divide por 10 porque AQR_wEspacamento está em cm*10
	fIncArea = ((float)AQR_wEspacamento * (float)dDist) * 0.1f;
	fIncAreaDir = ((float)AQR_wEspacamento * (float)dDistDir) * 0.1f;
	fIncAreaEsq = ((float)AQR_wEspacamento * (float)dDistEsq) * 0.1f;

	//Converte de cm² para m²
	fIncArea *= (1.0f / 10000.0f);
	fIncAreaDir *= (1.0f / 10000.0f);
	fIncAreaEsq *= (1.0f / 10000.0f);

	if (psMonitor->bMonitorArea == false)
	{
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{
			fArea += psStatus->bNumLinhasSemIntercalar * fIncArea;
		}
		else
		{
			fArea += psMonitor->bNumLinhas * fIncArea;
		}

		fAreaDir += psStatus->bNumLinhasDir * fIncAreaDir;
		fAreaEsq += psStatus->bNumLinhasEsq * fIncAreaEsq;

		sRegEstaticoCRC.sTrabTotalDir.fArea = fAreaDir;
		sRegEstaticoCRC.sTrabTotalEsq.fArea = fAreaEsq;
	}
	else
	{
		fArea += fIncArea;
	}

	//Inclui o valor no registro
	sRegEstaticoCRC.sTrabTotal.fArea = fArea;
}

/*******************************************************************************

 void AQR_vVerificaFalha( void )

 Descrição : Verifica se existe linha com falha de semente
 Parâmetros: nenhum.
 Retorno   : nenhum.

 *******************************************************************************/
void AQR_vVerificaFalha (void)
{
	uint8_t bI;
	UOS_tsCfgMonitor *psMonitor = &UOS_sConfiguracao.sMonitor;
	tsLinhas *psAvalia = &AQR_sAcumulado.sAvalia;
	tsStatus *psStatus = &AQR_sStatus;

	//Calcula de quantidade de sementes conforme configuração de avaliação:
	for (bI = 0; bI < psMonitor->bNumLinhas; bI++)
	{
		//Verifica se o loop é menor que 32, se não, usa os flags extendidos
		if (bI < 32)
		{
			dLinhaAtual = 1 << bI;
			if ((dLinhaAtual & psStatus->dLinhasLevantadas) == 0)
			{
				//Se a linha atual não está com sementes próximo de zero (falha > 90% pra baixo)
				if ((dLinhaAtual & psStatus->dSementeZero) == 0)
				{
					//Calcula média de sementes e converte para metro x 100 para evitar
					//utilização de float
					AQR_sStatus.awMediaSementes[bI] = (uint16_t)((psAvalia->adSementes[bI] * 100000L)
						/ psAvalia->dDistancia);

					//Se o número de sementes está fora dos limites configurados
					if ((AQR_sStatus.awMediaSementes[bI] < AQR_sStatus.wMinSementes)
						|| (AQR_sStatus.awMediaSementes[bI] > AQR_sStatus.wMaxSementes))
					{
						//Indica a falha de sensor de semente nesta linha:
						AQR_sStatus.dSementeFalha |= 0x00000001 << bI;
						abTimeoutAlarme[bI] = AQR_DISTANCIA_LIMPA_FALHA;

						//Indica falha de sensor - usado pela IHM
						//AQR_sStatus.dSementeFalhaIHM |= 0x00000001 << bI;
					}
					else
					{
						//Limpa falha de sensor de semente neste linha
						AQR_sStatus.dSementeFalha &= ~(0x00000001 << bI);

						//Após 10 metros que parou o alarme sonoro, limpa o alarme visual
						if (abTimeoutAlarme[bI] > 0)
						{
							abTimeoutAlarme[bI]--;
						}
						else
						{
							AQR_sStatus.dSementeFalhaIHM &= ~(0x00000001 << bI);
						}
					}
				}
				else
				{
					//Se estiver com sementes próximo de zero (falha > 90% pra baixo)
					//exibe valor instantâneo de sementes por metro
					AQR_sStatus.awMediaSementes[bI] = (uint16_t)((AQR_sFalhaInstantanea.abBufSem[bI] * 100000L)
						/ AQR_sFalhaInstantanea.awBufDis);
				}
			}                    // Fim da verificação de linha levantada
		}
		else // usando flags extendidos
		{
			dLinhaAtualExt = 1 << (bI - 32);
			if ((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0)
			{
				//Se a linha atual não está com sementes próximo de zero (falha > 90% pra baixo)
				if ((dLinhaAtualExt & psStatus->dSementeZeroExt) == 0)
				{
					//Calcula média de sementes e converte para metro x 100 para evitar
					//utilização de float
					AQR_sStatus.awMediaSementes[bI] = (uint16_t)((psAvalia->adSementes[bI] * 100000L)
						/ psAvalia->dDistancia);

					//Se o número de sementes está fora dos limites configurados
					if ((AQR_sStatus.awMediaSementes[bI] < AQR_sStatus.wMinSementes)
						|| (AQR_sStatus.awMediaSementes[bI] > AQR_sStatus.wMaxSementes))
					{
						//Indica a falha de sensor de semente nesta linha:
						AQR_sStatus.dSementeFalhaExt |= 0x00000001 << (bI - 32);
						abTimeoutAlarme[bI] = AQR_DISTANCIA_LIMPA_FALHA;

						//Indica falha de sensor - usado pela IHM
						//AQR_sStatus.dSementeFalhaIHM |= 0x00000001 << bI;
					}
					else
					{
						//Limpa falha de sensor de semente neste linha
						AQR_sStatus.dSementeFalhaExt &= ~(0x00000001 << (bI - 32));

						//Após 10 metros que parou o alarme sonoro, limpa o alarme visual
						if (abTimeoutAlarme[bI] > 0)
						{
							abTimeoutAlarme[bI]--;
						}
						else
						{
							AQR_sStatus.dSementeFalhaIHMExt &= ~(0x00000001 << (bI - 32));
						}
					}
				}
				else
				{
					//Se estiver com sementes próximo de zero (falha > 90% pra baixo)
					//exibe valor instantâneo de sementes por metro
					AQR_sStatus.awMediaSementes[bI] = (uint16_t)((AQR_sFalhaInstantanea.abBufSem[bI] * 100000L)
						/ AQR_sFalhaInstantanea.awBufDis);
				}
			}                    // Fim da verificação de linha levantada
		}                    // Fim do if  do flag extendido
	}                    // Fim do for

	//Indica existe linha(s) em falha de semente
	if ((AQR_sStatus.dSementeFalha > 0) || (AQR_sStatus.dSementeFalhaExt > 0))
	{
		AQR_sStatus.bLinhasFalha = true;
	}
	else
	{
		AQR_sStatus.bLinhasFalha = false;
	}

	//Indica existe linha(s) em falha de semente
	if ((AQR_sStatus.dSementeZero > 0) || (AQR_sStatus.dSementeZeroExt > 0))
	{
		AQR_sStatus.bLinhasZero = true;
		AQR_sStatus.dSementeZeroIHM |= AQR_sStatus.dSementeZero;
		AQR_sStatus.dSementeZeroIHMExt |= AQR_sStatus.dSementeZeroExt;
	}
	else
	{
		AQR_sStatus.bLinhasZero = false;
	}

	//Indica existe linha(s) em falha de adubo:
	if ((AQR_sStatus.dAduboFalha > 0) || (AQR_sStatus.dAduboFalhaExt > 0))
	{
		asm("nop");
	}
	else
	{
		asm("nop");
	}
}

/*******************************************************************************

 uint8_t AQR_vAdicionaSensor( uint8_t bConta, CAN_teEstadoSensor  eEstado )

 Descrição : Tarefa que escolhe posição do sensor na lista e chama rotina
 CAN_vAdicionaNovoSensor.
 Parâmetros: bConta - posição livre para instalação do sensor.
 eEstado - estado atual da posição (desconectado ou novo).
 Retorno   : Posição no loop de verificação dos sensores se o sensor foi
 adicionado, sai do loop, caso contrário continua na mesma posição.

 *******************************************************************************/
uint8_t AQR_vAdicionaSensor (uint8_t bConta, CAN_teEstadoSensor eEstado)
{
	uint8_t bPosicao = bConta;
	osFlags dFlagsIMH;

	//Se estiver em troca de sensor...
	if ((eEstado == Desconectado) && (AQR_sStatus.bSementeInstalados >= UOS_sConfiguracao.sMonitor.bNumLinhas))
	{
		dFlagsIMH = osFlagGet(AQR_sFlagREG);

		//Se a troca do sensor foi autorizada...
		if ((dFlagsIMH & AQR_FLAG_TROCA_SENSOR) > 0)
		{
			//Adiciona sensor de semente
			SEN_vAddNewSensor(bPosicao);

			//Limpa flag de troca de sensor...
			osFlagClear(AQR_sFlagREG, AQR_FLAG_TROCA_SENSOR | AQR_FLAG_NOVO_SENSOR);
		}
		else
		{
			//Informa o número da linha disponível para troca de sensor
			AQR_sStatus.bLinhaDisponivel = bPosicao + 1;

			//Seta flag de novo sensor para IHM:
			osFlagSet(AQR_sFlagREG, AQR_FLAG_NOVO_SENSOR);
		}
	}
	else //Se estiver em instalação...
	{
		//Adiciona sensor...
		SEN_vAddNewSensor(bPosicao);
	}

	bPosicao = 0;

	//Sai do loop
	return ( CAN_bTAMANHO_LISTA);

} //Fim da função AQR_vAdicionaSensor

/*******************************************************************************

 uint8_t AQR_vContaSensores( CAN_teEstadoSensor  eEstado )

 Descrição : Conta quantidade de sensores com um determinado estado (Novo,
 Conectado, Desconectado, Verificando ) e verifica quantos sensores
 de cada tipo foram incluidos na lista, mesmo que estejam
 desconectados no momento;
 Parâmetros: eEstado = (Novo, Conectado, Desconectado, Verificando )
 Retorno   : nenhum.

 *******************************************************************************/
uint8_t AQR_vContaSensores (CAN_teEstadoSensor eEstado)
{
	CAN_tsLista *psAQR_Sensor = AQR_sDadosCAN.asLista;
	UOS_tsCfgMonitor *psMonitor = &UOS_sConfiguracao.sMonitor;

	uint8_t bContaSensor = 0;
	uint8_t bContaTeste = 0;
	uint8_t bAdicionalInstalados = 0;
	uint8_t bAduboInstalados = 0;
	uint8_t bSementeInstalados = 0;
	uint8_t bConta;
	uint8_t bLinha;
	uint8_t bLimpaAux = false;

	uint32_t dAduboDesconectado = 0;
	uint32_t dAduboDesconectadoExt = 0;
	uint32_t dSementeDesconectado = 0;
	uint32_t dSementeDesconectadoExt = 0;
	uint32_t dAux, dAuxExt;

	for (bConta = 0; bConta < CAN_bTAMANHO_LISTA; bConta++)
	{
		//Conta sensores em um determinado estado
		if (psAQR_Sensor[bConta].eEstado == eEstado)
		{
			bContaSensor++;

			if (eEstado == Desconectado)
			{

				if (AQR_sStatus.bSensorAdicionado == bConta)
				{
					bLimpaAux = true;
				}
				//Se o sensor desconectado é sensor adicional
				if ((bConta >= CAN_bNUM_SENSORES_SEMENTE_E_ADUBO)
						&& (CAN_bSensorSimulador != false))
				{
					//Encontra o número do sensor
					bLinha = (bConta - CAN_bNUM_SENSORES_SEMENTE_E_ADUBO);

					//Indica qual sensor está desconectado
					AQR_sStatus.bAdicionalDesconectado |= 0x01 << bLinha;
				}
				else
				{
					//SÓ VARRE A LISTA ATÉ A QUANTIDADE DE SENSORES CONFIGURADOS
					if (bConta < (psMonitor->bNumLinhas * 2))
					{
						//Encontra o número da linha
						bLinha = (bConta >> 1);

						//Se o sensor for ímpar, ele é de adubo
						if ((bConta % 2) && (psMonitor->bSensorAdubo != false))
						{
							//Verifica se o loop é menor que 32, se não, usa os flags extendidos
							if (bLinha < 32)
							{
								//Indica sensor de adubo desconectado nesta linha
								dAduboDesconectado |= 0x00000001 << bLinha;

								dAux = (dAduboDesconectado ^ AQR_sStatus.dAduboIgnorado);
								dAduboDesconectado &= dAux;
							}
							else
							{
								//Indica sensor de adubo desconectado nesta linha
								dAduboDesconectadoExt |= 0x00000001 << (bLinha - 32);

								dAuxExt = (dAduboDesconectadoExt ^ AQR_sStatus.dAduboIgnoradoExt);
								dAduboDesconectadoExt &= dAuxExt;
							}
						}
						else // se o sensor for par, ele é de semente
						{
							//Verifica se o loop é menor que 32, se não, usa os flags extendidos
							if (bLinha < 32)
							{
								//Indica sensor de semente desconectado nesta linha
								dSementeDesconectado |= 0x00000001 << bLinha;

								dAux = (dSementeDesconectado ^ AQR_sStatus.dSementeIgnorado);
								dSementeDesconectado &= dAux;
							}
							else
							{
								//Indica sensor de semente desconectado nesta linha
								dSementeDesconectadoExt |= 0x00000001 << (bLinha - 32);

								dAuxExt = (dSementeDesconectadoExt ^ AQR_sStatus.dSementeIgnoradoExt);
								dSementeDesconectadoExt &= dAuxExt;
							}

						}

						//Indica sensor desconectado nesta linha
						AQR_sStatus.dLinhaDesconectada = (dAduboDesconectado | dSementeDesconectado);
						AQR_sStatus.dLinhaDesconectadaExt = (dAduboDesconectadoExt | dSementeDesconectadoExt);
					}
				}
			}
		}

		//Se o estado desejado é conectado, conta os sensores ignorados como conectados.
		if (eEstado == Conectado)
		{
			//if( bConta < CAN_bNUM_SENSORES_SEMENTE_E_ADUBO )
			//SÓ VARRE A LISTA ATÉ 36 SENSORES DE SEMENTE OU ADUBO
			if (bConta < (psMonitor->bNumLinhas * 2))
			{
				//Encontra o número da linha
				bLinha = (bConta >> 1);

				//Se o sensor for ímpar, ele é de adubo
				if ((bConta % 2) && (psMonitor->bSensorAdubo != false))
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (bLinha < 32)
					{
						bContaSensor += ((AQR_sStatus.dAduboIgnorado >> bLinha) & 0x00000001);
					}
					else
					{
						bContaSensor += ((AQR_sStatus.dAduboIgnoradoExt >> (bLinha - 32)) & 0x00000001);
					}
				}
				else // se o sensor for par, ele é de semente
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (bLinha < 32)
					{
						bContaSensor += ((AQR_sStatus.dSementeIgnorado >> bLinha) & 0x00000001);
					}
					else
					{
						bContaSensor += ((AQR_sStatus.dSementeIgnoradoExt >> (bLinha - 32)) & 0x00000001);
					}
				}

			}
		}

		//Verifica quantos sensores foram reprovados no auto-teste
		if (psAQR_Sensor[bConta].eResultadoAutoTeste == Reprovado)
		{
			bContaTeste++;

			if ((AQR_sStatus.bSensorAdicionado == bConta) && (bLimpaAux != false))
			{
				bApagaSensorReprovado = true;
			}

			//Se for sensor de semente ou adubo...
			//SÓ VARRE A LISTA ATÉ A QUANTIDADE DE SENSORES CONFIGURADOS
			if (bConta < (psMonitor->bNumLinhas * 2))
			{
				//Encontra o número da linha
				bLinha = (bConta >> 1);

				//Se o sensor for ímpar, ele é de adubo
				if ((bConta % 2) && (psMonitor->bSensorAdubo != false))
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (bLinha < 32)
					{
						//Sensor ignorado não conta como reprovado
						bContaTeste -= ((AQR_sStatus.dAduboIgnorado >> bLinha) & 0x00000001);
					}
					else
					{
						//Sensor ignorado não conta como reprovado
						bContaTeste -= ((AQR_sStatus.dAduboIgnoradoExt >> (bLinha - 32)) & 0x00000001);
					}
				}
				else // se o sensor for par, ele é de semente
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (bLinha < 32)
					{
						//Sensor ignorado não conta como reprovado
						bContaTeste -= ((AQR_sStatus.dSementeIgnorado >> bLinha) & 0x00000001);
					}
					else
					{
						//Sensor ignorado não conta como reprovado
						bContaTeste -= ((AQR_sStatus.dSementeIgnoradoExt >> (bLinha - 32)) & 0x00000001);
					}
				}
			}
		}
		//Verifica quantos sensores de cada tipo estão na lista de sensores,
		//mesmo que estejam desconectados no momento
		if (psAQR_Sensor[bConta].eEstado != Novo)
		{
			//Conta qtde de Sensores Adicionais
			if ((bConta >= CAN_bNUM_SENSORES_SEMENTE_E_ADUBO)
					&& (CAN_bSensorSimulador != false))
			{
				bAdicionalInstalados++;
			}
			else
			{
				//SÓ VARRE A LISTA ATÉ A QUANTIDADE DE SENSORES CONFIGURADOS
				if (bConta < (psMonitor->bNumLinhas * 2))
				{
					//Conta qtde de sensores de Adubo
					if ((bConta % 2) && (psMonitor->bSensorAdubo != false))
					{
						bAduboInstalados++;
					}
					else
					{
						//Conta qtde de sensores de semente
						bSementeInstalados++;
					}
				}
			}
		}
	}

	//Atualiza a quantidade de sensores reprovados no auto-teste
	AQR_sStatus.bReprovados = bContaTeste;

	//Atualizar a quantidade de sensores já instalados
	AQR_sStatus.bAdicionalInstalados = bAdicionalInstalados;
	AQR_sStatus.bAduboInstalados = bAduboInstalados;
	AQR_sStatus.bSementeInstalados = bSementeInstalados;

	//Retorna a quantidade de sensores contados, no estado escolhido
	return (bContaSensor);
}

/*******************************************************************************

 void AQR_vApagaListaSensores( void )

 Descrição : Apaga lista de sensores
 Parâmetros: nenhum.
 Retorno   : nenhum.

 *******************************************************************************/
void AQR_vApagaListaSensores (void)
{
	uint8_t bErr;
	osStatus status;

	//Mutex para acesso exclusivo ao arquivo da lista de sensores na rede CAN
	//Pega o mutex antes acessar dados compartilhados:
	status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
	ASSERT(status == osOK);

	//Limpa a estrutura de novo sensor
	memset(&CAN_sCtrlLista.sNovoSensor, 0,	sizeof(CAN_sCtrlLista.sNovoSensor));

	//Limpa a lista armazenada em buffer
	memset( &CAN_sCtrlLista.asLista, 0x00, sizeof( CAN_sCtrlLista.asLista ) );

	//Prepara a cópia de trabalho da estrutura com dados CAN:
	AQR_sDadosCAN = CAN_sCtrlLista;

	AQR_sStatus.bAdicionalInstalados = 0;
	AQR_sStatus.bAduboInstalados = 0;
	AQR_sStatus.bSementeInstalados = 0;

	//Devolve o mutex:
	status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
	ASSERT(status == osOK);

	osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_ERASE_LIST);
}

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * AQR_WDTData (uint8_t *pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t *) WATCHDOG_FLAG_ARRAY;
}

inline void AQR_vDetectThread (thisWDTFlag *flag, uint8_t *bPosition, void *pFunc)
{
	*bPosition = 0;
	while (THREADS_THREAD(*bPosition)!= (os_pthread)pFunc)
	{
		(*bPosition)++;
	}
	*flag = (uint8_t *)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}


void AQR_GetCANsCrtlLista(CAN_tsCtrlListaSens *sOut)
{
	osStatus status;
//Pega o mutex antes acessar dados compartilhados:
	status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
	ASSERT(status == osOK);

//Prepara a cópia de trabalho da estrutura com dados CAN:
	*sOut = CAN_sCtrlLista;

//Devolve o mutex:
	status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
	ASSERT(status == osOK);
}

void AQR_SetCANsCrtlLista(CAN_tsCtrlListaSens *sIn)
{
	osStatus status;
//Pega o mutex antes acessar dados compartilhados:
	status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
	ASSERT(status == osOK);

//Prepara a cópia de trabalho da estrutura com dados CAN:
	CAN_sCtrlLista = *sIn;

//Devolve o mutex:
	status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
	ASSERT(status == osOK);
}

#ifndef UNITY_TEST

/******************************************************************************
 * Function : AQR_vCreateThread(const Threads_t sSensorThread )
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
 * @see ISO_vCreateThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *******************************************************************************/
static void AQR_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void *)osThreadGetId());

	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}
#endif

/******************************************************************************
 * Function : AQR_eInitAcquiregPublisher(void)
 *//**
 * \b Description:
 *
 * This routine prepares the contract and message that the ISO_vIsobusPublishThread thread
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
 * @see ISO_vIsobusThread, ISO_vIsobusPublishThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
eAPPError_s AQR_eInitAcquiregPublisher (void)
{
	//Prepare Default Contract/Message
	MESSAGE_HEADER(Acquireg, ACQUIREG_DEFAULT_MSGSIZE, 1, MT_ARRAYBYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(Acquireg, 1, THIS_MODULE, TOPIC_ACQUIREG);

	//Prepare Default Contract/Message
	MESSAGE_HEADER(AcquiregSave, ACQUIREG_DEFAULT_MSGSIZE, 1, MT_ARRAYBYTE); // MT_ARRAYBYTE
	CONTRACT_HEADER(AcquiregSave, 1, THIS_MODULE, TOPIC_ACQUIREG_SAVE);

	return APP_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : AQR_vAcquiregPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the ISOBUS topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vDiagnosticThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void AQR_vAcquiregPublishThread (void const *argument)
{

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Acquireg Publish Thread Created");
#endif

	AQR_vDetectThread(&WATCHDOG(AQRPUB), &bAQRPUBThreadArrayPosition, (void *)AQR_vAcquiregPublishThread);
	WATCHDOG_STATE(AQRPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bAQRPUBThreadArrayPosition)); //Task created, inform core

	WATCHDOG_STATE(AQRPUB, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	WATCHDOG_STATE(AQRPUB, WDT_ACTIVE);

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(AQRPUB, WDT_SLEEP);
		osFlags dFlags = osFlagWait(xAQR_sFlagSis,
			AQR_APL_FLAG_FINISH_INSTALLATION | AQR_APL_FLAG_SAVE_STATIC_REG | AQR_APL_FLAG_UPDATE_INSTALLATION
			| AQR_APL_FLAG_CONFIRM_INSTALLATION | AQR_APL_FLAG_SAVE_LIST | AQR_APL_FLAG_ERASE_LIST
			| AQR_APL_FLAG_SEND_TOTAL | AQR_SIS_FLAG_ALARME | AQR_SIS_FLAG_ALARME_TOLERANCIA
			| AQR_APL_FLAG_SENSOR_CHANGE | AQR_APL_FLAG_ENABLE_SENSOR_PNP, true, false, osWaitForever);
		WATCHDOG_STATE(AQRPUB, WDT_ACTIVE);

		if ((dFlags & AQR_APL_FLAG_FINISH_INSTALLATION) > 0)
		{
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_INSTALLATION_FINISH_INSTALLATION, EVENT_SET, NULL);
		}
		if ((dFlags & AQR_APL_FLAG_SAVE_STATIC_REG) > 0)
		{
			PUBLISH_MESSAGE(AcquiregSave, EVENT_FFS_STATIC_REG, EVENT_SET, &sRegEstaticoCRC);
		}
		if ((dFlags & AQR_APL_FLAG_UPDATE_INSTALLATION) > 0)
		{
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_INSTALLATION_UPDATE_INSTALLATION, EVENT_SET, AQR_sDadosCAN.asLista);
		}
		if ((dFlags & AQR_APL_FLAG_CONFIRM_INSTALLATION) > 0)
		{
			AQR_sPubStatus = AQR_sStatus;
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_INSTALLATION_CONFIRM_INSTALLATION, EVENT_SET, &AQR_sPubStatus);
		}
		if ((dFlags & AQR_APL_FLAG_SAVE_LIST) > 0)
		{
			AQR_GetCANsCrtlLista(&AQR_sPubCtrlLista);
			PUBLISH_MESSAGE(AcquiregSave, EVENT_FFS_SENSOR_CFG, EVENT_SET, &AQR_sPubCtrlLista);
		}
		if ((dFlags & AQR_APL_FLAG_ERASE_LIST) > 0)
		{
			PUBLISH_MESSAGE(AcquiregSave, EVENT_FFS_SENSOR_CFG, EVENT_CLEAR, NULL);
		}
		if ((dFlags & AQR_APL_FLAG_SEND_TOTAL) > 0)
		{
			AQR_sPubPlantData.AQR_sAcumulado = &AQR_sPubAcumulado;
			AQR_sPubPlantData.AQR_sStatus = &AQR_sPubStatus;
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_UPDATE_PLANT_DATA, EVENT_SET, &AQR_sPubPlantData);
		}
		if ((dFlags & AQR_SIS_FLAG_ALARME) > 0)
		{
			if ((AQR_wAlarmes & AQR_SENSOR_DESCONECTADO) > 0)
			{
				PUBLISH_MESSAGE(Acquireg, EVENT_AQR_ALARM_DISCONNECTED_SENSOR, EVENT_SET, &AQR_sStatus);
			} else if ((AQR_wAlarmes & AQR_FALHA_LINHA) > 0)
			{
				PUBLISH_MESSAGE(Acquireg, EVENT_AQR_ALARM_LINE_FAILURE, EVENT_SET, &AQR_sStatus);
			} else if ((AQR_wAlarmes & AQR_FALHA_INSTALACAO) > 0)
			{
				PUBLISH_MESSAGE(Acquireg, EVENT_AQR_ALARM_SETUP_FAILURE, EVENT_SET, &AQR_sStatus);
			} else
			{
				if ((AQR_wAlarmes & AQR_EXC_VELOCIDADE) > 0)
				{
					PUBLISH_MESSAGE(Acquireg, EVENT_AQR_ALARM_EXCEEDED_SPEED, EVENT_SET, &AQR_sStatus);
				} else if ((AQR_wAlarmes & AQR_FALHA_GPS) > 0)
				{
					PUBLISH_MESSAGE(Acquireg, EVENT_AQR_ALARM_GPS_FAILURE, EVENT_SET, &AQR_sStatus);
				}
			}
		}
		if ((dFlags & AQR_SIS_FLAG_ALARME_TOLERANCIA) > 0)
		{
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_ALARM_TOLERANCE, EVENT_SET, &AQR_sStatus);
		}
		
		if ((dFlags & AQR_APL_FLAG_SENSOR_CHANGE) > 0)
		{
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_INSTALLATION_SENSOR_REPLACE, EVENT_SET, &AQR_sPubTrocaSensor);
		}

		if ((dFlags & AQR_APL_FLAG_ENABLE_SENSOR_PNP) > 0)
		{
			PUBLISH_MESSAGE(Acquireg, EVENT_AQR_INSTALLATION_ENABLE_SENSOR_PNP, EVENT_SET, NULL);
		}
	}
	osThreadTerminate(NULL);
}
#else
void AQR_vAcquiregPublishThread(void const *argument)
{
}
#endif

void AQR_vIdentifyEvent (contract_s* contract)
{
	osStatus status;
	event_e ePubEvt = GET_PUBLISHED_EVENT(contract);
	eEventType ePubEvType = GET_PUBLISHED_TYPE(contract);
	void *pvPubData = GET_PUBLISHED_PAYLOAD(contract);

	switch (contract->eOrigin)
	{
		case MODULE_CONTROL:
		{
			if (ePubEvt == EVENT_CTL_UPDATE_CONFIG)
			{
				//Se não está em modo monitor de área
				if (UOS_sConfiguracao.sMonitor.bMonitorArea == false)
				{
					//Se tiver sensor de Adubo Configurado
					if (UOS_sConfiguracao.sMonitor.bSensorAdubo != false)
					{
						//O número de sensores é duas vezes o número de linhas + sensores adicionais
						AQR_sStatus.bNumSensores = (UOS_sConfiguracao.sMonitor.bNumLinhas * 2);

					}
					else    //Se não tiver sensor de adubo
					{
						//O número de sensores é o número de linhas + sensores adicionais
						AQR_sStatus.bNumSensores = UOS_sConfiguracao.sMonitor.bNumLinhas;
					}
				}
				AQR_vParametros();
			}
			break;
		}
		case MODULE_SENSOR:
		{
			// Treat an event receive from MODULE_SENSOR
			osFlags sPubFlag = *((osFlags*)pvPubData);
			osStatus stat = osFlagSet(xSEN_sFlagApl, sPubFlag);
			break;
		}
		case MODULE_GPS:
		{
			GPS_sPubDadosGPS *psGPSDados = pvPubData;

			// Treat an event receive from MODULE_GPS
			osFlagSet(xGPS_sFlagGPS, (osFlags)ePubEvt);

			if (psGPSDados != NULL)
			{
				status = WAIT_MUTEX(AQR_MTX_sEntradas, osWaitForever);
				ASSERT(status == osOK);
				AQR_sDadosGPS = psGPSDados->sDadosGPS;
				if (ePubEvt == GPS_FLAG_METRO)
				{
					AQR_bDistanciaPercorrida = psGPSDados->bDistanciaPercorrida;
				}

				status = RELEASE_MUTEX(AQR_MTX_sEntradas);
				ASSERT(status == osOK);
			}

			break;
		}
		case MODULE_FILESYS:
		{
			if (ePubEvt == EVENT_FFS_STATIC_REG)
			{
				if (ePubEvType == EVENT_SET)
				{
					AQR_tsRegEstaticoCRC *pAQRRegData = pvPubData;
					if (pAQRRegData != NULL)
					{
						sRegEstaticoCRC = *pAQRRegData;
						osFlagSet(AQR_sFlagREG, AQR_FLAG_ESTATICO_REG);
					}
				}
				else
				{
					osFlagClear(AQR_sFlagREG, AQR_FLAG_ESTATICO_REG);
				}
			}
			if (ePubEvt == EVENT_FFS_SENSOR_CFG)
			{
				CAN_tsCtrlListaSens *pCtrlListaSens = pvPubData;
				if (pCtrlListaSens != NULL)
				{
					AQR_SetCANsCrtlLista(pCtrlListaSens);

					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_VERIFICANDO);
				}
			}

			break;
		}
		case MODULE_GUI:
		{
			if (ePubEvt == EVENT_GUI_INSTALLATION_REPEAT_TEST)
			{
				AQR_vRepeteTesteSensores();
				osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_ENABLE_SENSOR_PNP);
			}

			if (ePubEvt == EVENT_GUI_INSTALLATION_ERASE_INSTALLATION)
			{
				AQR_vApagaInstalacao();
				osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_ENABLE_SENSOR_PNP);
			}

			if (ePubEvt == EVENT_GUI_INSTALLATION_CONFIRM_INSTALLATION_ACK)
			{
				osFlagClear(xAQR_sFlagSis, AQR_APL_FLAG_CONFIRM_INSTALLATION);
				osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_SAVE_LIST);
			}

			if (ePubEvt == EVENT_GUI_PLANTER_CLEAR_COUNTER_TOTAL)
			{
				osFlagSet(AQR_sFlagREG, AQR_FLAG_ZERA_TOTAIS);
			}

			if (ePubEvt == EVENT_GUI_PLANTER_CLEAR_COUNTER_SUBTOTAL)
			{
				osFlagSet(AQR_sFlagREG, AQR_FLAG_ZERA_PARCIAIS);
			}

			if (ePubEvt == EVENT_GUI_PLANTER_IGNORE_SENSOR)
			{
				sIgnoreLineStatus *psIgnoreLine = pvPubData;
				if (psIgnoreLine != NULL)
				{
					WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
					AQR_vIgnoreSensors(psIgnoreLine->bLineNum, psIgnoreLine->bLineIgnored);
					WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
				}
			}

			if (ePubEvt == EVENT_GUI_TRIMMING_TRIMMING_MODE_CHANGE)
			{
				sTrimmingState* psTrimm = pvPubData;
				if (psTrimm != NULL)
				{
					WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
					AQR_vTrimmingLines(psTrimm->eTrimmState);
					WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
				}
			}

			if (ePubEvt == EVENT_GUI_AREA_MONITOR_PAUSE)
			{
				if (UOS_sConfiguracao.sMonitor.bTeclaPausaHab != false) {
					uint32_t dFlags = osFlagGet(AQR_sFlagREG);

					if ((dFlags & AQR_FLAG_PAUSA) > 0) {
						osFlagClear(AQR_sFlagREG, AQR_FLAG_PAUSA);
					} else {
						osFlagSet(AQR_sFlagREG, AQR_FLAG_PAUSA);
					}
				}
			}

			if (ePubEvt == EVENT_GUI_INSTALLATION_REPLACE_SENSOR)
			{
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				AQR_vVerificarTrocaSensores();
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_GUI_INSTALLATION_CONFIRM_REPLACE_SENSOR)
			{
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				AQR_vTrocaSensores(EVENT_SET);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}

			if (ePubEvt == EVENT_GUI_INSTALLATION_CANCEL_REPLACE_SENSOR)
			{
				WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
				AQR_vTrocaSensores(EVENT_CLEAR);
				WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
			}
			break;
		}
		default:
			break;
	}
}

/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void AQR_vAcquiregThread (void const *argument)
{
	osStatus status;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Acquireg Thread Created");
#endif

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(AcquiregQueue);

	// Internal flags
	status = osFlagGroupCreate(&xSEN_sFlagApl);
	ASSERT(status == osOK);

	status = osFlagGroupCreate(&xGPS_sFlagGPS);
	ASSERT(status == osOK);

	status = osFlagGroupCreate(&xAQR_sFlagSis);
	ASSERT(status == osOK);

	AQR_eInitAcquiregPublisher();

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromIsobusID = (osThreadId)argument;
	osSignalSet(xMainFromIsobusID, MODULE_ACQUIREG);

	WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP, false, false, osWaitForever);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		AQR_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	/* Prepare the signature - struture that notify the broker about subscribers */
	SIGNATURE_HEADER(AcquiregControl, THIS_MODULE, TOPIC_CONTROL, AcquiregQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(AcquiregControl), 0) == osOK);

	SIGNATURE_HEADER(AcquiregSensor, THIS_MODULE, TOPIC_SENSOR, AcquiregQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(AcquiregSensor), 0) == osOK);

	SIGNATURE_HEADER(AcquiregGPS, THIS_MODULE, TOPIC_GPS, AcquiregQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(AcquiregGPS), 0) == osOK);

	SIGNATURE_HEADER(AcquiregGPSMetro, THIS_MODULE, TOPIC_GPS_METRO, AcquiregQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(AcquiregGPSMetro), 0) == osOK);

	SIGNATURE_HEADER(AcquiregFileSys, THIS_MODULE, TOPIC_FILESYS, AcquiregQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(AcquiregFileSys), 0) == osOK);

	SIGNATURE_HEADER(AcquiregGUI, THIS_MODULE, TOPIC_GUI_AQR, AcquiregQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(AcquiregGUI), 0) == osOK);

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on any topic */
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		osEvent evt = RECEIVE(AcquiregQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;

		if (evt.status == osEventMessage)
		{
			AQR_vIdentifyEvent(GET_CONTRACT(evt));
		}
	}
	/* Unreachable */
	osThreadSuspend(NULL);
}
#else
void AQR_vAcquiregThread(void const *argument)
{
}
#endif

/******************************************************************************
 * Function : AQR_vAcquiregTimeThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the ISOBUS topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vDiagnosticThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void AQR_vAcquiregTimeThread (void const *argument)
{
	uint8_t bErr;
	uint8_t bSaveEstaticData = 0;
	float fVelocidade;
	tsLinhas *psTrabTotal = &AQR_sAcumulado.sTrabTotal;
	tsLinhas *psTrabParcial = &AQR_sAcumulado.sTrabParcial;
	tsLinhas *psTotalReg = &AQR_sAcumulado.sTotalReg;
	tsStatus *psStatus = &AQR_sStatus;
	osFlags dFlagsSis;

	(void)argument;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Acquireg Time Thread Created");
#endif

	AQR_vDetectThread(&WATCHDOG(AQRTIM), &bAQRTIMThreadArrayPosition, (void *)AQR_vAcquiregTimeThread);
	WATCHDOG_STATE(AQRTIM, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bAQRTIMThreadArrayPosition)); //Task created, inform core

	WATCHDOG_STATE(AQRTIM, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	WATCHDOG_STATE(AQRTIM, WDT_ACTIVE);

	while (1)
	{
		/* Wait a flag sent on each second */
		WATCHDOG_STATE(AQRTIM, WDT_SLEEP);
		osFlagWait(xGPS_sFlagGPS, GPS_FLAG_SEGUNDO, true, false, osWaitForever);
		WATCHDOG_STATE(AQRTIM, WDT_ACTIVE);

		//Contador de Segundos Total
		psTotalReg->dSegundos++;

		if (psStatus->bTrabalhando != false)
		{
			//Contador de Segundos Trabalhados
			psTrabTotal->dSegundos++;

			//Contador de Segundos Trabalhados parcial
			psTrabParcial->dSegundos++;

			//Converte velocidade de cm/s para km/h
			fVelocidade = (AQR_sDadosGPS.dGroundSpeed / 100.0f);
			fVelocidade *= 3.6f;

			// Verifica Excesso de Velocidade a cada segundo
			if (fVelocidade > (UOS_sConfiguracao.sMonitor.fLimVel + 0.01f))
			{
				//Indica que está em excesso de velocidade
				psStatus->bExVel = true;

				//Se a velocidade atual for maior que a velocidade máxima armazenada
				//salva valor atual em km/h
				if (fVelocidade > AQR_sVelocidade.fVelMax)
				{
					AQR_sVelocidade.fVelMax = fVelocidade;
				}

				//Incrementa Tempo em excesso de velocidade em segundos
				AQR_sVelocidade.dTEV++;

				//Conta tempo parcial em excesso de velocidade
				dPEV++;
			}
			else
			{
				//Indica que não está em excesso de velocidade
				psStatus->bExVel = false;

				//Zera contador de tempo parcial em excesso de velocidade
				dPEV = 0;

			}
		}
		else
		{
			//Limpa variável excesso de velocidade
			psStatus->bExVel = false;

			//Zera contador de tempo parcial em excesso de velocidade
			dPEV = 0;
		}

		//Se o tempo parcial em excesso de velocidade for maior que o
		//anteriormente armazenado salva valor atual
		if (dPEV > AQR_sVelocidade.dMTEV)
		{
			//Máximo Tempo em Excesso de Velocidade em segundos
			AQR_sVelocidade.dMTEV = dPEV;
		}

		dFlagsSis = osFlagGet(UOS_sFlagSis);
		if ((dFlagsSis & (UOS_SIS_FLAG_MODO_TRABALHO | UOS_SIS_FLAG_MODO_TESTE)) != 0)
		{
			WAIT_MUTEX(AQR_MTX_sBufferAcumulado, osWaitForever);
			AQR_sPubStatus = AQR_sStatus;
			AQR_sPubAcumulado = AQR_sAcumulado;
			RELEASE_MUTEX(AQR_MTX_sBufferAcumulado);
			osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_SEND_TOTAL);
		}

		if (((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) != 0) && (bSaveEstaticData++ > ARQ_SAVE_ESTATIC_DATA_TIMEOUT))
		{
			bSaveEstaticData = 0;
			AQR_SetStaticRegData();
			AQR_vPubAcumulaArea();
			osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_SAVE_STATIC_REG);
		}
	}
	osThreadTerminate(NULL);
}
#else
void AQR_vAcquiregTimeThread(void const *argument)
{
}
#endif

void AQR_vTimerCallbackTurnOff (void const* argument)
{
	osFlagSet(AQR_sFlagREG, AQR_FLAG_DESLIGA);
}

void AQR_vTimerCallbackImpStopped (void const* argument)
{
	osFlagSet(AQR_sFlagREG, AQR_FLAG_IMP_PARADO);
}

uint8_t AQR_GetStaticRegData (void)
{
	uint16_t wResult;
	uint16_t wCRC16;        //Para calculo do CRC dos registros estáticos.
	uint8_t sRet;

	WAIT_MUTEX(AQR_MTX_sBufferAcumulado, osWaitForever);

	//Verifica se o registro estático não está corrompido:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&sRegEstaticoCRC,
	AQR_wTAM_REG_ESTATICO_CRC);

	//Se os registros estão ok:
	if (wCRC16 == 0)
	{
		//-----------------------------------------------------------------------------//
		//Copia da estrutura de registro para estrutura de Acumulados Totais
		memcpy(&AQR_sAcumulado.sTrabTotal, &sRegEstaticoCRC.sTrabTotal, sizeof(AQR_sAcumulado.sTrabTotal));

		//Copia da estrutura de registro para estrutura de Acumulados Totais
		memcpy(&AQR_sAcumulado.sTrabTotalDir, &sRegEstaticoCRC.sTrabTotalDir, sizeof(AQR_sAcumulado.sTrabTotalDir));

		//Copia da estrutura de registro para estrutura de Acumulados Totais
		memcpy(&AQR_sAcumulado.sTrabTotalEsq, &sRegEstaticoCRC.sTrabTotalEsq, sizeof(AQR_sAcumulado.sTrabTotalEsq));

		//Copia Valores Acumulados Parciais para estrutura de registro
		memcpy(&AQR_sAcumulado.sTrabParcial, &sRegEstaticoCRC.sReg.sTrabParcial, sizeof(AQR_sAcumulado.sTrabParcial));

		//Copia Valores Acumulados Parciais para estrutura de registro
		memcpy(&AQR_sAcumulado.sTrabParcDir, &sRegEstaticoCRC.sReg.sTrabParcDir, sizeof(AQR_sAcumulado.sTrabParcDir));

		//Copia Valores Acumulados Parciais para estrutura de registro
		memcpy(&AQR_sAcumulado.sTrabParcEsq, &sRegEstaticoCRC.sReg.sTrabParcEsq, sizeof(AQR_sAcumulado.sTrabParcEsq));

		//Copia Valores relativos à velocidade
		memcpy(&AQR_sVelocidade, &sRegEstaticoCRC.sReg.sVelocidade, sizeof(AQR_sVelocidade));

		//Copia Valores da última média calculada em cada linha
		memcpy(&AQR_sStatus.awMediaSementes, &sRegEstaticoCRC.awMediaSementes, sizeof(AQR_sStatus.awMediaSementes));
		//-----------------------------------------------------------------------------//

		//Indica que vai salvar o registro existente:
		sRet = AQR_bSTS_SALVA_REG;

	} //Fim se( crc do registro está ok )
	else
	{
		//Limpa a estrutura do registro estático:
		memset(&sRegEstaticoCRC, 0x00, AQR_wTAM_REG_ESTATICO_CRC);

		//Data/hora da atualização:
		sRegEstaticoCRC.dDataHora = dDataHoraSisCiclo;

		//Calcula o crc da estrutura do registro estático:
		TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&sRegEstaticoCRC,
		AQR_wTAM_REG_ESTATICO_CRC - sizeof(sRegEstaticoCRC.wCRC16));
		//Atualiza o valor do crc na estrutura combinada:
		sRegEstaticoCRC.wCRC16 = wCRC16;

		//Indica que vai criar um novo registro:
		sRet = AQR_bSTS_NOVO_REG;
	}
	RELEASE_MUTEX(AQR_MTX_sBufferAcumulado);
	return sRet;
}

void AQR_SetStaticRegData (void)
{
	uint8_t bErr;
	uint16_t wResult;
	uint16_t wCRC16;

	//Ponteiro para o registro estático dentro da estrutura acima:
	AQR_tsRegEstatico * const AQR_psReg = &sRegEstaticoCRC.sReg;

	WAIT_MUTEX(AQR_MTX_sBufferAcumulado, osWaitForever);

	//--------------------------------------------------------------------------
	//Registro Estático:

	//Atualiza a data/hora de fim de registro:
	memcpy((uint8_t *)&AQR_psReg->abDataHoraFim[0], &abDataHoraCiclo[1], 3);
	memcpy((uint8_t *)&AQR_psReg->abDataHoraFim[3], &abDataHoraCiclo[5], 3);

	//--------------------------------------------------------------------------
	// Atualiza Estruturas de Registro:
	//Copia Valores Acumulados Totais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sTrabTotal, &AQR_sAcumulado.sTrabTotal, sizeof(AQR_sAcumulado.sTrabTotal));

	memcpy(&sRegEstaticoCRC.sTrabTotalDir, &AQR_sAcumulado.sTrabTotalDir, sizeof(AQR_sAcumulado.sTrabTotalDir));

	memcpy(&sRegEstaticoCRC.sTrabTotalEsq, &AQR_sAcumulado.sTrabTotalEsq, sizeof(AQR_sAcumulado.sTrabTotalEsq));

	//Copia Valores Acumulados Parciais para estrutura de registro
	memcpy(&sRegEstaticoCRC.sReg.sTrabParcial, &AQR_sAcumulado.sTrabParcial, sizeof(AQR_sAcumulado.sTrabParcial));

	memcpy(&sRegEstaticoCRC.sReg.sTrabParcDir, &AQR_sAcumulado.sTrabParcDir, sizeof(AQR_sAcumulado.sTrabParcDir));

	memcpy(&sRegEstaticoCRC.sReg.sTrabParcEsq, &AQR_sAcumulado.sTrabParcEsq, sizeof(AQR_sAcumulado.sTrabParcEsq));

	//Copia Valores relativos à velocidade
	memcpy(&sRegEstaticoCRC.sReg.sVelocidade, &AQR_sVelocidade, sizeof(AQR_sVelocidade));

	//Copia Valores da última média calculada em cada linha
	memcpy(&sRegEstaticoCRC.awMediaSementes, &AQR_sStatus.awMediaSementes, sizeof(AQR_sStatus.awMediaSementes));

	//Data/hora da atualização:
	sRegEstaticoCRC.dDataHora = dDataHoraSisCiclo;

	//Calcula o crc da estrutura do registro estático:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&sRegEstaticoCRC,
	AQR_wTAM_REG_ESTATICO_CRC - sizeof(sRegEstaticoCRC.wCRC16));
	//Atualiza o valor do crc na estrutura combinada:
	sRegEstaticoCRC.wCRC16 = wCRC16;

	RELEASE_MUTEX(AQR_MTX_sBufferAcumulado);
}

void AQR_vRepeteTesteSensores (void)
{
	osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST);
	osFlagClear(xAQR_sFlagSis, AQR_APL_FLAG_CONFIRM_INSTALLATION);

	osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);

	osFlagClear(UOS_sFlagSis,
		(UOS_SIS_FLAG_MODO_TESTE | UOS_SIS_FLAG_MODO_TRABALHO));

	osFlagSet(AQR_sFlagREG, AQR_FLAG_AUTO_TESTE);
}

void AQR_vApagaInstalacao (void)
{
	AQR_vApagaListaSensores();

	osFlagClear(xAQR_sFlagSis, AQR_APL_FLAG_CONFIRM_INSTALLATION);

	osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST | UOS_SIS_FLAG_ERRO_INST_SENSOR |
		UOS_SIS_FLAG_PARAMETROS_OK | UOS_SIS_FLAG_VERSAO_SW_OK | UOS_SIS_FLAG_ALARME_TOLERANCIA |
		UOS_SIS_FLAG_MODO_TESTE | UOS_SIS_FLAG_MODO_TRABALHO);

	osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_VERIFICANDO);

}

void AQR_vIgnoreSensors(uint8_t bLineNum, bool bIgnored)
{
	osStatus status;
	uint32_t dCurrLine, dCurrLineExt;

	if (bLineNum < 33) {
		dCurrLine = 1 << (bLineNum - 1);
	} else if (bLineNum < 37) {
		dCurrLineExt = 1 << (bLineNum - 33);
	}

	if (bIgnored) {
		if (bLineNum < 33) {
			AQR_sStatus.dSementeIgnorado |= dCurrLine;
		} else if (bLineNum < 37) {
			AQR_sStatus.dSementeIgnoradoExt |= dCurrLineExt;
		}
	} else {
		if (bLineNum < 33) {
			AQR_sStatus.dSementeIgnorado &= ~dCurrLine;
			AQR_sStatus.dAduboIgnorado &= ~dCurrLine;
		} else if (bLineNum < 37) {
			AQR_sStatus.dSementeIgnoradoExt &= ~dCurrLineExt;
			AQR_sStatus.dAduboIgnoradoExt &= ~dCurrLineExt;
		}
		osFlagSet(AQR_sFlagREG, AQR_FLAG_AUTO_TESTE);
	}
}

void AQR_vTrimmingLines (eTrimming eTrimmStatus)
{
	tsStatus *psStatus = &AQR_sStatus;

	psStatus->eArremate = (AQR_teArremate) eTrimmStatus;
}

void AQR_vVerificarTrocaSensores (void)
{
	osFlags dFlags;
	AQR_vRepeteTesteSensores();

	while( AQR_sStatus.bAutoTeste != false )
	{
		osDelay(50);
	}

	dFlags = osFlagGet(AQR_sFlagREG);

	if( dFlags & AQR_FLAG_NOVO_SENSOR )
	{
		AQR_sPubTrocaSensor.eReplacState = REPLACEMENT_NO_ERROR;
		switch( AQR_sDadosCAN.sNovoSensor.bTipoSensor )
		{

			case CAN_APL_SENSOR_ADUBO:
			{
				AQR_sPubTrocaSensor.eType = FERTILIZER_SENSOR;
				break;
			}
			case CAN_APL_SENSOR_SIMULADOR:
			case CAN_APL_SENSOR_DIGITAL_2:
			case CAN_APL_SENSOR_DIGITAL_3:
			case CAN_APL_SENSOR_DIGITAL_4:
			case CAN_APL_SENSOR_DIGITAL_5:
			case CAN_APL_SENSOR_DIGITAL_6:
			{
				AQR_sPubTrocaSensor.eType = ADDITIONAL_SENSOR;
				break;
			}
			case CAN_APL_SENSOR_SEMENTE:
			default:
			{
				AQR_sPubTrocaSensor.eType = SEED_SENSOR;
			}
		}

		AQR_sPubTrocaSensor.bAvailableLine = AQR_sStatus.bLinhaDisponivel;
	}
	else
	{
		if (AQR_sDadosCAN.sNovoSensor.bNovo != false)
		{
			AQR_sPubTrocaSensor.eReplacState = REPLACEMENT_ERR_NOT_ALLOWED;
		}
		else
		{
			AQR_sPubTrocaSensor.eReplacState = REPLACEMENT_ERR_NO_SENSOR;
		}
	}
	osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_SENSOR_CHANGE);
}

void AQR_vTrocaSensores (eEventType ev)
{
	if (ev == EVENT_SET)
	{
      osFlagSet( AQR_sFlagREG, AQR_FLAG_TROCA_SENSOR );
      osFlagClear( UOS_sFlagSis, UOS_SIS_FLAG_VERSAO_SW_OK);
	}
	else
	{
		osFlagClear(AQR_sFlagREG, (AQR_FLAG_TROCA_SENSOR |	AQR_FLAG_NOVO_SENSOR));
	}
}

/******************************************************************************
 * Function : AQR_vAcquiregManagementThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the ISOBUS topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from ISO_vDiagnosticThread,
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *
 *******************************************************************************/
#ifndef UNITY_TEST
void AQR_vAcquiregManagementThread (void const *argument)
{
	osStatus status;
	uint8_t bErr;
	uint8_t bConta;
	uint8_t bContaSensor;
	uint8_t bTentativas;
	uint8_t bLinhasFalhaPausaAuto;
	uint16_t wResult;
	uint16_t wCRC16;

	osFlags dFlagsSis;
	osFlags dValorFlag;
	osFlags dValorGPS;
	osFlags dFlagCAN;
	osFlags dFlagSensor;

	tsStatus *psStatus = &AQR_sStatus;
	CAN_tsLista *psAQR_Sensor = AQR_sDadosCAN.asLista;
	UOS_tsCfgMonitor *psMonitor = &UOS_sConfiguracao.sMonitor;

	tsLinhas *psTrabTotal;   //= &AQR_sAcumulado.sTrabTotal;
	tsLinhas *psTrabParcial; //= &AQR_sAcumulado.sTrabParcial;
	tsDistanciaTrab *psDistTrabTotal;
	tsDistanciaTrab *psDistTrabParcial;

	tsLinhas *psTotalReg = &AQR_sAcumulado.sTotalReg;
	tsLinhas *psAvalia = &AQR_sAcumulado.sAvalia;
	tsLinhas *psManobra = &AQR_sAcumulado.sManobra;

	//Ponteiro para o registro estático dentro da estrutura acima:
	AQR_tsRegEstatico * const AQR_psReg = &sRegEstaticoCRC.sReg;

	// To prevent warning
	(void)argument;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Acquireg Time Thread Created");
#endif

	AQR_vDetectThread(&WATCHDOG(AQRMGT), &bAQRMGTThreadArrayPosition, (void *)AQR_vAcquiregManagementThread);
	WATCHDOG_STATE(AQRMGT, WDT_ACTIVE);

	//----------------------------------------------------------------------------
	//Preparação variáveis da aquisição e registro:

	// Initialize flag group to indicate events
	status = osFlagGroupCreate(&AQR_sFlagREG);
	ASSERT(status == osOK);

#ifndef TESTE_TECLADO
	INITIALIZE_TIMER(AQR_bTimerTurnOff, osTimerPeriodic);
#endif

	// Timer to wait for 10 seconds
	INITIALIZE_TIMER(AQR_bTimerImpStopped, osTimerPeriodic);

	// Mutex to access control to input data structures, acquire variables and records
	INITIALIZE_MUTEX(AQR_MTX_sEntradas);
	INITIALIZE_MUTEX(AQR_MTX_sBufferListaSensores);
	INITIALIZE_MUTEX(AQR_MTX_sBufferAcumulado);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bAQRMGTThreadArrayPosition)); //Task created, inform core

	//Aguarda o fim da inicialização do sistema:
	WATCHDOG_STATE(AQRMGT, WDT_SLEEP);
	osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
	WATCHDOG_STATE(AQRMGT, WDT_ACTIVE);

	// TODO: extern variable
	//    UOS_sVersaoCod.wFlag = 0xFFFF;

	// TODO: Copia para variável auxiliar a opção de configuração que ativa gravação de registros
	AQR_bSalvaRegistro = UOS_sConfiguracao.sGPS.bSalvaRegistro;

	//Prepara a data/hora do ciclo agora pois será usada na
	//recuperação do registro anterior à falha de energia:
	//Formato BCD:
	GPS_dDataHoraLocal(abDataHoraCiclo);
	//Formato do sistema:
	dDataHoraSisCiclo = GPS_dDataHoraSistema();

	//----------------------------------------------------------------------------
	//Preparação dos arquivos de registro:

	dValorFlag = osFlagGet(AQR_sFlagREG);
	if ((dValorFlag & AQR_FLAG_ESTATICO_REG) == 0)
	{
		//Limpa a estrutura do registro estático:
		memset(&sRegEstaticoCRC, 0x00, AQR_wTAM_REG_ESTATICO_CRC);

		//Data/hora da atualização:
		sRegEstaticoCRC.dDataHora = dDataHoraSisCiclo;

		//Indicação de status de uso local:
		AQR_bStatus = AQR_bSTS_ERRO;
	}
	else
	{
		//Indicação de status de uso local:
		AQR_bStatus = AQR_GetStaticRegData();
		//Se havia um registro nos arquivos:
		if (AQR_bStatus == AQR_bSTS_SALVA_REG)
		{
			if (sRegEstaticoCRC.dEmergencia != 0)
			{
				//Salva o registro anterior com causa de fim falha de energia:
				AQR_wCausaFim = AQR_wCF_FALHA_ENERGIA;
			}

			//Indicação de status de uso local:
			AQR_bStatus = AQR_bSTS_OK;
		}
	}

	//----------------------------------------------------------------------------
	//Prioridade de trabalho:

	//Muda a prioridade de inicialização para o valor de trabalho antes
	//de se pendurar no flag enviado a cada metro percorrido:
	//      bErr = OSTaskChangePrio( OS_PRIO_SELF, AQR_TRF_PRINCIPAL_PRIORIDADE );
	//      __assert( bErr == OS_NO_ERR );

	//----------------------------------------------------------------------------
	//Prepara o valor inicial:

	//Se não está em modo monitor de área
	if (psMonitor->bMonitorArea == false)
	{
		//Se tiver sensor de Adubo Configurado
		if (psMonitor->bSensorAdubo != false)
		{
			//O número de sensores é duas vezes o número de linhas + sensores adicionais
			psStatus->bNumSensores = (psMonitor->bNumLinhas * 2);

		}
		else    //Se não tiver sensor de adubo
		{
			//O número de sensores é o número de linhas + sensores adicionais
			psStatus->bNumSensores = psMonitor->bNumLinhas;
		}
	}

	//Seta variáveis
	bTentativas = 0;
	psStatus->bVelZero = true;
	//Zera distância de insensibilidade
	dDistInsens = 0;

	//Garante que a insensibilidade será sempre fixa em 10 metros
	psMonitor->wInsensibilidade = 100;

	//Zera buffer de segmentos
	memset(&sSegmentos, 0, sizeof(sSegmentos));

	// Wait mutex
	status = WAIT_MUTEX(GPS_MTX_sEntradas, osWaitForever);
	ASSERT(status == osOK);

	//Prepara a cópia de trabalho da estrutura com as entradas:
	AQR_sDadosGPS = GPS_sDadosGPS;

	// Release mutex
	status = RELEASE_MUTEX(GPS_MTX_sEntradas);
	ASSERT(status == osOK);

	//Mascára Buffer em Anel
	sSegmentos.wMskBufAnel = AQR_wMSK_BUF_ANEL;

	//Calcula os parametros de tolerância, avaliação e espaçamento
	AQR_vParametros();

	//----------------------------------------------------------------------------
	//Finaliza a inicialização da aquisição e registro:

	//Ajusta o timer com timeout de 1 segundo
	status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_1S);
	ASSERT(status == osOK);

#ifndef TESTE_TECLADO
	// Ajusta o timer com timeout de 30 Min.
	status = START_TIMER(AQR_bTimerTurnOff, AQR_TIMEOUT_30MIN);
	ASSERT(status == osOK);
#endif

	//Ajusta o timer com timeout de 30 Min.
	//O MPA2500 desligará automaticamente após 30 minutos sem velocidade ou atividade no teclado
	//Esta funcionalidade serve para modo de teste, instalação ou trabalho
	//UOS_vAjustaTimer( bTimerDesliga, AQR_TIMEOUT_30MIN, true );

	CAN_bNumRespostasPNP = 0;

	while (1)
	{
		// Wait for flag GPS_FLAG_METRO or timeout
		WATCHDOG_STATE(AQRMGT, WDT_SLEEP);
		dValorGPS = osFlagWait(xGPS_sFlagGPS, (GPS_FLAG_METRO | GPS_FLAG_TIMEOUT_MTR), true, false, osWaitForever);
		WATCHDOG_STATE(AQRMGT, WDT_ACTIVE);

		// Mutex wait
		status = WAIT_MUTEX(AQR_MTX_sEntradas, osWaitForever);
		ASSERT(status == osOK);

		//Avisa que novas informações vindas do GPS estão disponíveis:
		osFlagSet(AQR_sFlagREG, AQR_FLAG_NOVO_DADO);

		//Limpa a variável de linhas intercaladas
		dLinhasIntercaladas = 0x00000000;
		dLinhasIntercaladasExt = 0x00000000;

		//Verifica linhas intercaladas
		switch (psMonitor->eIntercala)
		{
			case Linhas_Impares:
			{
				uint8_t bI;

				for (bI = 0; bI < psMonitor->bNumLinhas; bI++)
				{
					if (!(bI % 2))
					{
						//Verifica se o loop é menor que 32, se não usa os flags extendidos
						if (bI < 32)
						{
							dLinhasIntercaladas |= 0x00000001 << bI;
						}
						else
						{
							dLinhasIntercaladasExt |= 0x00000001 << (bI - 32);
						}
					}
				}
				break;
			}

			case Linhas_Pares:
			{

				uint8_t bI;

				for (bI = 0; bI < psMonitor->bNumLinhas; bI++)
				{
					if (bI % 2)
					{
						//Verifica se o loop é menor que 32, se não, usa os flags extendidos
						if (bI < 32)
						{
							dLinhasIntercaladas |= 0x00000001 << bI;
						}
						else
						{
							dLinhasIntercaladasExt |= 0x00000001 << (bI - 32);
						}
					}
				}
				break;
			}

			default:
				break;
		}

		//Verifica arremate
		switch (psStatus->eArremate)
		{
			case Sem_Arremate: //Se não está em arremate (nenhum lado levantado)
			{
				//Aponta para estrutura de plantadeira inteira
				psTrabTotal = &AQR_sAcumulado.sTrabTotal;
				psTrabParcial = &AQR_sAcumulado.sTrabParcial;
				//Aponta para estrutura da area parcial
				psDistTrabTotal = &AQR_sAcumulado.sDistTrabTotal;
				psDistTrabParcial = &AQR_sAcumulado.sDistTrabParcial;

				//O número de linhas ativas é igual ao número de linhas configuradas
				psStatus->bNumLinhasAtivas = psMonitor->bNumLinhas;

				//Offset de linha é zero
				bLinhasOffset = 0;

				//Se estava em arremate
				if ((dLinhasArremate > 0) || (dLinhasArremateExt > 0))
				{
					//Limpa linhas em arremate
					dLinhasArremate = 0x00000000;
					dLinhasArremateExt = 0x00000000;

					//Zera distância de insensibilidade (entra em insensibilidade)
					dDistInsens = 0;

					osFlagSet(AQR_sFlagREG, AQR_FLAG_RECONHECE_ALARME);
				}

				break;
			}

			case Lado_Direito: //Se está em arremate no lado direito (Lado direito levantado)
			{
				//Se o arremate nao estava onfigurado para o lado direito
				//Zera distancia pra avaliaçao  limpa os alarmes
				if (eMemArremate != Lado_Direito)
				{
					//Zera distância de insensibilidade (entra em insensibilidade)
					dDistInsens = 0;

					osFlagSet(AQR_sFlagREG, AQR_FLAG_RECONHECE_ALARME);
				}

				//Aponta para estrutura de plantadeira do lado esquerdo
				psTrabTotal = &AQR_sAcumulado.sTrabTotalEsq;
				psTrabParcial = &AQR_sAcumulado.sTrabParcEsq;

				// Aponta para estrutura da área parcial
				psDistTrabTotal = &AQR_sAcumulado.sDistTrabTotalEsq;
				psDistTrabParcial = &AQR_sAcumulado.sDistTrabParcialEsq;

				//Se a divisão da plantadeira está no lado esquerdo
				if (psMonitor->bDivLinhas == 0)
				{
					//O número de linhas ativas é o maior lado da plantadeira
					psStatus->bNumLinhasAtivas = (psMonitor->bNumLinhas + 1) >> 1;
				}
				else //Se a divisão da plantadeira está no lado direito
				{
					//O número de linhas ativas é o menor lado da plantadeira
					psStatus->bNumLinhasAtivas = (psMonitor->bNumLinhas >> 1);
				}

				//Offset de linha é zero
				bLinhasOffset = 0;

				break;
			}

			case Lado_Esquerdo: //Se está em arremate no lado esquerdo (Lado esquerdo levantado)
			{

				//Se o arremate nao estava onfigurado para o lado direito
				//Zera distancia pra avaliaçao  limpa os alarmes
				if (eMemArremate != Lado_Esquerdo)
				{
					//Zera distância de insensibilidade (entra em insensibilidade)
					dDistInsens = 0;

					osFlagSet(AQR_sFlagREG, AQR_FLAG_RECONHECE_ALARME);
				}

				//Aponta para estrutura de plantadeira do lado direito
				psTrabTotal = &AQR_sAcumulado.sTrabTotalDir;
				psTrabParcial = &AQR_sAcumulado.sTrabParcDir;

				//Aponta para estrutura da área parcial
				psDistTrabTotal = &AQR_sAcumulado.sDistTrabTotalDir;
				psDistTrabParcial = &AQR_sAcumulado.sDistTrabParcialDir;

				//Se a divisão da plantadeira está no lado esquerdo
				if (psMonitor->bDivLinhas == 0)
				{
					//O número de linhas ativas é  o menor lado da plantadeira
					psStatus->bNumLinhasAtivas = psMonitor->bNumLinhas >> 1;
				}
				else //Se a divisão da plantadeira está no lado direito
				{
					//O número de linhas ativas é o maior lado da plantadeira
					psStatus->bNumLinhasAtivas = (psMonitor->bNumLinhas + 1) >> 1;
				}

				//Offset de linha é o número de linhas
				bLinhasOffset = psMonitor->bNumLinhas - psStatus->bNumLinhasAtivas;

				break;
			}

			default:
				break;
		}

		//Memoriza o estado do arremate
		eMemArremate = psStatus->eArremate;

		//Se estiver em Arremate, coloca na Estrutura de Manobra as eventuais
		//sementes que possam cair nas linhas levantadas
		if (psStatus->eArremate != Sem_Arremate)
		{
			uint8_t bOffset;
			uint8_t bLinhasArremate = psMonitor->bNumLinhas - psStatus->bNumLinhasAtivas;
			uint8_t bI;

			dLinhasArremate = 0x00000000;
			dLinhasArremateExt = 0x00000000;

			//Se Offset de linhas for maior que zero
			if (bLinhasOffset > 0)
			{
				//Offset de linhas em arremate é zero
				bOffset = 0;
			}
			else //Se offset de linhas em arremate for zero
			{
				//O Offset de linhas em arremate é igual ao número de linhas ativas
				bOffset = psStatus->bNumLinhasAtivas;
			}

			bLinhasArremate += bOffset;

			for (bI = bOffset; bI < bLinhasArremate; bI++)
			{
				//Verifica se o loop é menor que 32, se não, usa os flags extendidos
				if (bI < 32)
				{
					dLinhasArremate |= 0x000000001 << bI;
				}
				else
				{
					dLinhasArremateExt |= 0x000000001 << (bI - 32);
				}
			}
		}

		//As linhas levantadas são as em Arremate e as Intercaladas
		psStatus->dLinhasLevantadas = (dLinhasArremate | dLinhasIntercaladas);
		psStatus->dLinhasLevantadasExt = (dLinhasArremateExt | dLinhasIntercaladasExt);

		//------------------------------------------------------------------------------
		//Limpa as Linhas em falha para pausa automática
		bLinhasFalhaPausaAuto = 0;
		psStatus->bNumLinhasAtivas = 0;
		psStatus->bNumLinhasSemIntercalar = 0;
		uint8_t bNumLinhasSemIntercalarDir = 0;
		uint8_t bNumLinhasSemIntercalarEsq = 0;
		uint8_t bNumLinhasEsq = 0;

		//Varre todas as linhas
		for (uint8_t bI = 0; bI < psMonitor->bNumLinhas; bI++)
		{
			//Verifica se o loop é menor que 32, se não usa os flags extendidos
			if (bI < 32)
			{
				dLinhaAtual = 1 << bI;

				//Verifica quantas linhas não estão levantadas ou ignoradas
				if ((dLinhaAtual & (psStatus->dLinhasLevantadas | psStatus->dSementeIgnorado)) == 0)
				{
					bLinhasFalhaPausaAuto++;
				}

				//Verifica quantas linhas estão ativas
				if ((dLinhaAtual & psStatus->dLinhasLevantadas) == 0)
				{
					psStatus->bNumLinhasAtivas++;
				}

				//Verifica quantas linhas estão sem intercalar, mesmo que esteja em arremate
				if ((dLinhaAtual & dLinhasIntercaladas) == 0)
				{
					psStatus->bNumLinhasSemIntercalar++;

					if (psMonitor->bDivLinhas == 0)
					{
						//O número de linhas do lado esq é o maior lado da plantadeira
						bNumLinhasEsq = (psMonitor->bNumLinhas + 1) >> 1;
					}
					else //Se a divisão da plantadeira está no lado direito
					{
						//O número de linhas do lado esq é o menor lado da plantadeira
						bNumLinhasEsq = (psMonitor->bNumLinhas >> 1);
					}

					if (bI < bNumLinhasEsq)
					{
						bNumLinhasSemIntercalarEsq++;
					}
					else
					{
						bNumLinhasSemIntercalarDir++;
					}
				}
			}
			else
			{
				dLinhaAtualExt = 1 << (bI - 32);

				//Verifica quantas linhas não estão levantadas ou ignoradas
				if ((dLinhaAtualExt & (psStatus->dLinhasLevantadasExt | psStatus->dSementeIgnoradoExt)) == 0)
				{
					bLinhasFalhaPausaAuto++;
				}

				//Verifica quantas linhas estão ativas
				if ((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0)
				{
					psStatus->bNumLinhasAtivas++;
				}

				//Verifica quantas linhas estão sem intercalar, mesmo que esteja em arremate
				if ((dLinhaAtualExt & dLinhasIntercaladasExt) == 0)
				{
					psStatus->bNumLinhasSemIntercalar++;

					if (psMonitor->bDivLinhas == 0)
					{
						//O número de linhas do lado esq é o maior lado da plantadeira
						bNumLinhasEsq = (psMonitor->bNumLinhas + 1) >> 1;
					}
					else //Se a divisão da plantadeira está no lado direito
					{
						//O número de linhas do lado esq é o menor lado da plantadeira
						bNumLinhasEsq = (psMonitor->bNumLinhas >> 1);
					}

					if (bI < bNumLinhasEsq)
					{
						bNumLinhasSemIntercalarEsq++;
					}
					else
					{
						bNumLinhasSemIntercalarDir++;
					}
				}
			}
		}

		//Se a quantidade de linhas ativas for maior que a configuração para
		//para pausa automática
		if (bLinhasFalhaPausaAuto > psMonitor->bLinhasFalhaPausaAuto)
		{
			bLinhasFalhaPausaAuto = psMonitor->bLinhasFalhaPausaAuto;
		}

		//Se intercalação de linhas está configurada...
		if (psMonitor->eIntercala != Sem_Intercalacao)
		{

			//Se NÃO está em arremate
			if (psStatus->eArremate == Sem_Arremate)
			{
				psStatus->bNumLinhasEsq = bNumLinhasSemIntercalarEsq++;
				psStatus->bNumLinhasDir = bNumLinhasSemIntercalarDir++;
			}
			else //se está em arremate
			{
				//Se está com o lado esquerdo levantado
				if (psStatus->eArremate == Lado_Esquerdo)
				{
					psStatus->bNumLinhasDir = psStatus->bNumLinhasAtivas;
					psStatus->bNumLinhasEsq = psStatus->bNumLinhasSemIntercalar - psStatus->bNumLinhasDir;

				}
				else //if( psStatus->eArremate == Lado_Direito )
				{
					psStatus->bNumLinhasEsq = psStatus->bNumLinhasAtivas;
					psStatus->bNumLinhasDir = psStatus->bNumLinhasSemIntercalar - psStatus->bNumLinhasEsq;
				}
			}
		}

		//------------------------------------------------------------------------------
		//Finaliza a instalação

		//Se não terminou a instalação
		//Se não está em Auto Teste
		//Se não ligou flag de fim de instalação
		if (((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) == 0) && (psStatus->bAutoTeste == false)
			&& ((dFlagsSis & UOS_SIS_FLAG_CONFIRMA_INST) == 0))
		{
			//Verifica quantos sensores estão conectados e
			//quantos de cada tipo foram instalados
			bContaSensor = AQR_vContaSensores(Conectado);

			//Se o nº de sensores conectados for igual ao nº de sensores esperados
			//Se o nº de sensores adicionais instalados for igual ao configurado
			//Se não houver sensor reprovado no Auto Teste
			if ((bContaSensor >= (psStatus->bNumSensores + CAN_bSensorSimulador)) && (psStatus->bReprovados == 0)
				&& (psStatus->bSensorNaoEsperado == false))
			{
				//Se os parâmetros dos sensores estão OK e
				//Se o nº de sensores de semente instalados é igual ao configurado ou
				//Se o nº de sensores de adubo instalados é igual ao configurado ou
				//Se está em modo monitor de área (em caso de haver sensor adicional)
				if ((((psStatus->bSementeInstalados == psMonitor->bNumLinhas)
					|| ((psStatus->bAduboInstalados == psMonitor->bNumLinhas) && (psMonitor->bSensorAdubo != false)))
					|| (psMonitor->bMonitorArea != false)) && ((dFlagsSis & UOS_SIS_FLAG_PARAMETROS_OK) > 0)
					&& ((dFlagsSis & UOS_SIS_FLAG_VERSAO_SW_OK) > 0))
				{
					//Liga flag de fim de instalação
					osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST);
					osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_CONFIRM_INSTALLATION);

					for (bConta = 0; bConta < CAN_bTAMANHO_LISTA; bConta++)
					{
						//Conecta sensores
						psAQR_Sensor[bConta].eEstado = Conectado;
					}
				}

				osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_FINISH_INSTALLATION);
			}
		}

		WATCHDOG_STATE(AQRMGT, WDT_SLEEP);
		dFlagSensor = osFlagWait(xSEN_sFlagApl, (CAN_APL_FLAG_TODOS_SENS_RESP_PNP |
		CAN_APL_FLAG_DET_NOVO_SENSOR |
		CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP |
		CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP |
		CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP |
		CAN_APL_FLAG_SENSOR_NAO_RESPONDEU |
		CAN_APL_FLAG_DET_SENSOR_RECONECTADO |
		CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO |
		CAN_APL_FLAG_CFG_SENSOR_RESPONDEU |
		CAN_APL_FLAG_CAN_STATUS),
		true, false, 0);
		WATCHDOG_STATE(AQRMGT, WDT_ACTIVE);

		if (dFlagSensor != CAN_APL_FLAG_NENHUM)
		{

			AQR_GetCANsCrtlLista(&AQR_sDadosCAN);

			//------------------------------------------------------------------------
			//Se algum sensor não respondeu ou e nenhum sensor respondeu...
			if ((dFlagSensor & ( CAN_APL_FLAG_SENSOR_NAO_RESPONDEU | CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO)) > 0)
			{
				if (((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) == 0) && //Se não terminou a instalação...
					((dFlagsSis & UOS_SIS_FLAG_ERRO_INST_SENSOR) != 0) && //Se a flag estiver ligada
					(CAN_bNumRespostasPNP <= (psStatus->bSementeInstalados + CAN_bSensorSimulador))) //Se não houver sensores a mais
				{
					osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);
				}

				//Se não estiver em auto-teste
				if (psStatus->bAutoTeste == false)
				{
					//Identifica quantos sensores estão desconectados
					bContaSensor = AQR_vContaSensores(Desconectado);

					//Se o número de sensores desconectados maior que zero aciona o alarme.
					if ((psStatus->bAdicionalDesconectado > 0) || (psStatus->dLinhaDesconectada > 0)
						|| (psStatus->dLinhaDesconectadaExt > 0))
					{
						psStatus->bSensorDesconectado = true;

						if ((bApagaSensorReprovado != false) && ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0))
						{
							bApagaSensorReprovado = false;

							//Pega o mutex antes acessar dados compartilhados:
							status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
							ASSERT(status == osOK);

							memset(&CAN_sCtrlLista.asLista[psStatus->bSensorAdicionado], 0x00,
								sizeof(CAN_sCtrlLista.asLista[psStatus->bSensorAdicionado]));
							memset(&AQR_sDadosCAN.asLista[psStatus->bSensorAdicionado], 0x00,
								sizeof(AQR_sDadosCAN.asLista[psStatus->bSensorAdicionado]));

							//Devolve o mutex:
							status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
							ASSERT(status == osOK);
						}
					}
					else
					{
						//Só limpa o flag de sensor desconectado se não estiver em modo
						//trabalho, pois em modo trabalho o flag apenas pode ser zerado
						//ao precionar a tecla de reconhecimento de falha
						if ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0)
						{
							psStatus->bSensorDesconectado = false;
							psStatus->bAdicionalDesconectado = 0x00;
							psStatus->bMemAdicionalDesconectado = 0x00;
							psStatus->dLinhaDesconectada = 0x00000000;
							psStatus->dMemLinhaDesconectada = 0x00000000;
							psStatus->dLinhaDesconectadaExt = 0x00000000;
							psStatus->dMemLinhaDesconectadaExt = 0x00000000;
						}
					}
				}
				//Publish Event Update Installation
				osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_UPDATE_INSTALLATION);
			}

			//------------------------------------------------------------------------
			//Se todos os sensores responderam ao comando de detecção...
			if ((dFlagSensor & CAN_APL_FLAG_TODOS_SENS_RESP_PNP) > 0)
			{
				//Se não terminou a instalação...
				if ((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) == 0)
				{

					if ((CAN_bNumRespostasPNP <= (psStatus->bSementeInstalados + CAN_bSensorSimulador))
						&& ((dFlagsSis & UOS_SIS_FLAG_ERRO_INST_SENSOR) != 0)) //Se a flag estiver ligada
					{
						osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);
					}

					//Le o valor do flag
					dFlagCAN = osFlagGet(xSEN_sFlagApl);

					//Se houve resposta ao comando de configuracao...
					if ((dFlagCAN & CAN_APL_FLAG_CFG_SENSOR_RESPONDEU) > 0)
					{
						//Limpa o flag
						osFlagClear(xSEN_sFlagApl, CAN_APL_FLAG_CFG_SENSOR_RESPONDEU);

						//Seta flag de Sensor respondeu Configuracao
						psStatus->bCfgSensorRespondeu = true;

						//Faz um auto-teste dos sensores:
						osFlagSet(AQR_sFlagREG, AQR_FLAG_AUTO_TESTE);

						bTentativas = 0;
					}

					if (psStatus->bAutoTeste != false)
					{
						//Verifica se há sensores reprovados...
						bContaSensor = AQR_vContaSensores(Conectado);

						//Se algum sensor foi reprovado no teste, repete o teste
						if (psStatus->bReprovados > 0)
						{
							psStatus->bReprovados = 0;
							if (bTentativas <= 3)
							{
								bTentativas++;
								//Faz um auto-teste dos sensores:
								osFlagSet(AQR_sFlagREG, AQR_FLAG_AUTO_TESTE);

								//limpa flag de novo sensor instalado, para alarme
								psStatus->eStatusInstalacao = AguardandoEvento;
							}
							else
							{
								//Se mesmo após repetição do teste o sensor permanecer
								//reprovado liga alarme
								psStatus->eStatusInstalacao = FalhaAutoTeste;

								//Informa que finalizou o auto-teste
								psStatus->bAutoTeste = false;
							}
						}
						else ////Se não houver sensor reprovado...
						{
							//Limpa flag de falha de instalação
							psStatus->eStatusInstalacao = AguardandoEvento;

							//Informa que finalizou o auto-teste
							psStatus->bAutoTeste = false;

							if (psStatus->bCfgSensorRespondeu != false)
							{
								//Limpa flag
								psStatus->bCfgSensorRespondeu = false;

								//Seta Status de novo sensor instalado, para alarme
								psStatus->eStatusInstalacao = Instalado;

							}
						}
					}

					//Publish Event Update Installation
					osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_UPDATE_INSTALLATION);
				}
			}

			//------------------------------------------------------------------------
			//Se foi encontrado novo sensor...
			if ((dFlagSensor & CAN_APL_FLAG_DET_NOVO_SENSOR) > 0)
			{

				uint8_t bAdiciona = false;

				//Varre a lista de sensores...
				for (bConta = 0; bConta < CAN_bTAMANHO_LISTA; bConta++)
				{
					//Se o novo sensor for sensor adicional...
					if ((AQR_sDadosCAN.sNovoSensor.bTipoSensor >= CAN_APL_SENSOR_SIMULADOR)
						&& (CAN_bSensorSimulador != false))
					{
						//Adiciona offset para sensores digitais (a partir da posição 72)
						bConta += CAN_bNUM_SENSORES_SEMENTE_E_ADUBO;
					}

					//Se encontrar uma posição livre...
					if ((psAQR_Sensor[bConta].eEstado == Novo)
						|| ((psAQR_Sensor[bConta].eEstado == Desconectado) && (psStatus->bAutoTeste == false)))
					{
						//Se o novo sensor for sensor adicional...
						if ((AQR_sDadosCAN.sNovoSensor.bTipoSensor >= CAN_APL_SENSOR_SIMULADOR)
							&& (bConta >= CAN_bNUM_SENSORES_SEMENTE_E_ADUBO)
							&& ((psAQR_Sensor[bConta].eEstado == Novo) || (psAQR_Sensor[bConta].eEstado == Desconectado)))
						{
							psStatus->bSensorAdicionado = bConta;

							//Encontra o número da linha
							bConta = (bConta >> 1);
							bAdiciona = true;

						}
						else
						{
							//SÓ VARRE A LISTA ATÉ A QUANTIDADE DE SENSORES CONFIGURADOS
							//Se estiver em uma posição coerente com número de linhas configuradas
							//Se não estiver em modo monitor de área
							if ((bConta < (psMonitor->bNumLinhas * 2)) && (psMonitor->bMonitorArea == false))
							{
								//Se posição livre for ímpar...
								if (bConta % 2)
								{
									//Se o tipo do sensor encontrado for ADUBO e
									//Se houver sensor de adubo configurado e
									//Se o sensor for novo ou está desconectado
									if ((AQR_sDadosCAN.sNovoSensor.bTipoSensor == CAN_APL_SENSOR_ADUBO)
										&& (psMonitor->bSensorAdubo != false)
										&& ((psAQR_Sensor[bConta].eEstado == Novo)
											|| (psAQR_Sensor[bConta].eEstado == Desconectado)))
									{
										//Testa se o número de respostas PNP é maior que o número
										//de sensores no barramento CAN mais 1. Não será permitido
										//instalar mais que 1 sensor ao mesmo tempo.
										if (CAN_bNumRespostasPNP
											> (psStatus->bAduboInstalados + 1 + CAN_bSensorSimulador))
										{
											if ((dFlagsSis & UOS_SIS_FLAG_ERRO_INST_SENSOR) == 0)
											{
												osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);
											}
										}
										else
										{
											if ((dFlagsSis & UOS_SIS_FLAG_ERRO_INST_SENSOR) != 0)
											{
												osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);
											}

											//Informa o número do sensor a ser adicionado
											psStatus->bSensorAdicionado = bConta;

											//Encontra o número da linha
											bConta = ((bConta - 1) >> 1);
											bAdiciona = true;
										}
									}
								}
								else
								{
									//Se o tipo do sensor encontrado for SEMENTE e
									//Se o sensor for novo ou está desconectado
									if ((AQR_sDadosCAN.sNovoSensor.bTipoSensor == CAN_APL_SENSOR_SEMENTE)
										&& ((psAQR_Sensor[bConta].eEstado == Novo)
											|| (psAQR_Sensor[bConta].eEstado == Desconectado)))
									{
										//Testa se o número de respostas PNP é maior que o número
										//de sensores no barramento CAN mais 1. Não será permitido
										//instalar mais que 1 sensor ao mesmo tempo.
										if (CAN_bNumRespostasPNP
											> (psStatus->bSementeInstalados + 1 + CAN_bSensorSimulador))
										{
											if ((dFlagsSis & UOS_SIS_FLAG_ERRO_INST_SENSOR) == 0)
											{
												osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);
											}
										}
										else
										{
											if ((dFlagsSis & UOS_SIS_FLAG_ERRO_INST_SENSOR) != 0)
											{
												osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ERRO_INST_SENSOR);
											}

											//Informa o número do sensor a ser adicionado
											psStatus->bSensorAdicionado = bConta;

											//Encontra o número da linha
											bConta = (bConta >> 1);
											bAdiciona = true;
										}
									}
								}
							}
						}

						//Adiciona o sensor na primeira linha vaga
						if (bAdiciona != false)
						{
							bConta = (AQR_vAdicionaSensor(bConta, psAQR_Sensor[psStatus->bSensorAdicionado].eEstado));
							osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_UPDATE_INSTALLATION);
						}

					}

				} //fim do for(...)

				//Se foi encontrado um sensor não esperado
				if (bAdiciona == false)
				{
					if (psStatus->bSensorNaoEsperado == false)
					{
						//Liga flag de falha de instalação
						psStatus->eStatusInstalacao = SensorNaoEsperado;

						//Indica que foi conectado um sensor além dos esperados
						psStatus->bSensorNaoEsperado = true;

						//Limpa o flag de fim de instalação
						osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST);
						osFlagClear(xAQR_sFlagSis, AQR_APL_FLAG_CONFIRM_INSTALLATION);

						//Limpa flag de sensor instalado
						//                        IHM_bConfirmaInstSensores = eSensoresNaoInstalados;
					}
				}
			}
			else
			{
				//Limpa variável que indica sensor não esperado, para permitir finalizar instalação,
				//caso o sensor tenha sido retirado do barramento.

				psStatus->bSensorNaoEsperado = false;

				if (AQR_sDadosCAN.sNovoSensor.bNovo != false)
				{
					//Pega o mutex antes acessar dados compartilhados:
					status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
					ASSERT(status == osOK);

					//Limpa indicação de novo sensor
					CAN_sCtrlLista.sNovoSensor.bNovo = false;

					//Devolve o mutex:
					status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
					ASSERT(status == osOK);

					AQR_sDadosCAN.sNovoSensor.bNovo = false;

					//Limpa flag de troca de sensor...
					osFlagClear(AQR_sFlagREG, (AQR_FLAG_TROCA_SENSOR | AQR_FLAG_NOVO_SENSOR));
				}
			}

			//------------------------------------------------------------------------
			//Se todos os sensores responderam
			if ((dFlagSensor & CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP) > 0)
			{
				//Pega o mutex antes acessar dados compartilhados:
				status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
				ASSERT(status == osOK);

				// TODO: Clean last data receive from sensors CAN_sCtrlLista. This should be done using broker.
				//Limpa a última leitura de cada sensor
				for (bConta = 0; bConta < CAN_bTAMANHO_LISTA; bConta++)
				{
					CAN_sCtrlLista.asLista[bConta].abUltimaLeitura[0] = 0x00;
					CAN_sCtrlLista.asLista[bConta].abUltimaLeitura[1] = 0x00;
				}

				//Devolve o mutex:
				status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
				ASSERT(status == osOK);

				//Identifica quantos sensores estão desconectados
				bContaSensor = AQR_vContaSensores(Desconectado);

				//Se o número de sensores desconectados maior que zero aciona o alarme.
				if ((AQR_sStatus.bAdicionalDesconectado > 0) || (AQR_sStatus.dLinhaDesconectada > 0)
					|| (AQR_sStatus.dLinhaDesconectadaExt > 0))
				{
					psStatus->bSensorDesconectado = true;

				}
				else
				{
					//Só limpa o flag de sensor desconectado se não estiver em modo
					//trabalho, pois em modo trabalho o flag apenas pode ser zerado
					//ao precionar a tecla de reconhecimento de falha
					if ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0)
					{
						psStatus->bSensorDesconectado = false;
						psStatus->bAdicionalDesconectado = 0x00;
						psStatus->bMemAdicionalDesconectado = 0x00;
						psStatus->dLinhaDesconectada = 0x00000000;
						psStatus->dMemLinhaDesconectada = 0x00000000;
						psStatus->dLinhaDesconectadaExt = 0x00000000;
						psStatus->dMemLinhaDesconectadaExt = 0x00000000;
					}
				}
			}

			//------------------------------------------------------------------------
			//Se todos os sensores responderam parâmetros
			if ((dFlagSensor & CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP) > 0)
			{
				//Liga flag de fim de instalação
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_PARAMETROS_OK);
			}

			//------------------------------------------------------------------------
			//Se todos os sensores responderam versão de Software
			if ((dFlagSensor & CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP) > 0)
			{
				//Liga flag de fim de instalação
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_VERSAO_SW_OK);
			}
		}
		else
		{
			//Identifica quantos sensores estão conectados
			bContaSensor = AQR_vContaSensores(Conectado);

			//Se não tem nenhum sensor conectado
			//Se não finalizou a instalação
			if ((bContaSensor == 0) && ((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) == 0))
			{
				//Limpa o flag de auto teste
				psStatus->bAutoTeste = false;
			}
		}

		WATCHDOG_STATE(AQRMGT, WDT_SLEEP);
		dValorFlag = osFlagWait(AQR_sFlagREG,
			(AQR_FLAG_RECONHECE_ALARME | AQR_FLAG_ZERA_PARCIAIS | AQR_FLAG_ZERA_TOTAIS | AQR_FLAG_AUTO_TESTE),
			true,
			false, 0);
		WATCHDOG_STATE(AQRMGT, WDT_ACTIVE);

		if (dValorFlag != CAN_APL_FLAG_NENHUM)
		{
			//Reconhece Alarmes
			if ((dValorFlag & AQR_FLAG_RECONHECE_ALARME) > 0)
			{
				psStatus->bAlarmeOK = true;
			}

			//Se foi solicitado Zerar parciais
			if ((dValorFlag & AQR_FLAG_ZERA_PARCIAIS) > 0)
			{
				AQR_vZeraRegs( false);
				AQR_sStatus.bAlarmeOK = true;
			}

			//Se foi solicitado Zerar Totais
			if ((dValorFlag & AQR_FLAG_ZERA_TOTAIS) > 0)
			{
				//Ajusta a causa de fim para "leitura de registros":
				AQR_wCausaFim = AQR_wCF_ZERA_TOTAL;
				AQR_sStatus.bAlarmeOK = true;

				//Pede a criação de um novo registro:
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_NOVO_REG);
			}

			//Se foi solicitado auto teste dos sensores
			if ((dValorFlag & AQR_FLAG_AUTO_TESTE) > 0)
			{
				//Liga flag para informar que está em auto teste
				psStatus->bAutoTeste = true;

				//Limpa o flag de fim de instalação
				osFlagClear(xAQR_sFlagSis, AQR_APL_FLAG_CONFIRM_INSTALLATION);

				//Limpa alguma eventual falha de sensor desconectado,
				//para evitar que o icone de falha seja ligado durante o teste
				psStatus->bSensorDesconectado = false;

				//Desabilita fonte de alimentação dos sensores CAN
				DISABLE_PS9;

				//Aguarda meio segundo
				osDelay(500);

				//Habilita fonte de alimentação dos sensores CAN
				ENABLE_PS9;
			}
		}

		//--------------------------------------------------------------------------
		//Data/Hora atual:

		//Data/hora a ser utilizada em todas as operações a seguir:
		//Formato BCD:
		GPS_dDataHoraLocal(abDataHoraCiclo);
		//Formato do sistema:
		dDataHoraSisCiclo = GPS_dDataHoraSistema();

		//--------------------------------------------------------------------------
		//Controle de criação de registros operacionais:

		dValorFlag = osFlagGet(AQR_sFlagREG);
		if ((dValorFlag & AQR_FLAG_ESTATICO_REG) == 0)
		{
			//Indicação de status de uso local:
			AQR_bStatus = AQR_bSTS_ERRO;
		}
		else
		{
			//Se no ciclo anterior estava parado:
			if (AQR_bStatus == AQR_bSTS_ERRO)
			{
				//Indicação de status de uso local:
				AQR_bStatus = AQR_GetStaticRegData();
				//Se havia um registro nos arquivos:
				if (AQR_bStatus == AQR_bSTS_SALVA_REG)
				{
					//Ajusta a causa de fim:
					AQR_wCausaFim = AQR_wCF_LIBERA_MEM;

					//Indica que deve criar um novo registro:
					dFlagsSis |= UOS_SIS_FLAG_NOVO_REG;
					//osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_NOVO_REG);
				}

				//Indicação de status de uso local:
				AQR_bStatus = AQR_bSTS_OK;
			}
		}

		//--------------------------------------------------------------------------
		// CRIAÇÃO DO REGISTRO ESTÁTICO:

		//Se alguém está pedindo para criar um novo registro:
		if (((dFlagsSis & UOS_SIS_FLAG_NOVO_REG) > 0) || ((dFlagsSis & UOS_SIS_FLAG_REINICIO) > 0))
		{
			//Se estamos rodando normalmente:
			if (AQR_bStatus != AQR_bSTS_ERRO)
			{
				uint8_t bVaiReiniciar = false;

				AQR_bStatus = AQR_bSTS_OK;

				//Se o novo registro foi pedido por que está sendo
				//realizada a leitura do arquivo combinado ou
				//se houve mudança de configuração:
				if ((AQR_wCausaFim == AQR_wCF_CONFIGURACAO) || (AQR_wCausaFim == AQR_wCF_REINICIO_SRL)
					|| (AQR_wCausaFim == AQR_wCF_REINICIO_BTH) || (AQR_wCausaFim == AQR_wCF_LEITURA)
					|| (AQR_wCausaFim == AQR_wCF_ZERA_TOTAL))
				{
					//Memoriza a Causa de Fim
					//          INT16U wCausaFim = AQR_wCausaFim;
					//Zera o total  - somente quando solicitado pelo menu
					if (AQR_wCausaFim == AQR_wCF_ZERA_TOTAL)
					{
						//Zera Total
						AQR_vZeraRegs( true);
					}

					//Ajusta a Causa de fim
					AQR_wCausaFim = AQR_wCF_DESCONHECIDO;

					//Se a causa do fim do registro gera um reinicio geral do MPA2500
					//não é necessário que seja criado um novo arquivo de Registros.
					if ((dFlagsSis & UOS_SIS_FLAG_REINICIO) == 0)
					{
						bVaiReiniciar = false;
					}
					else
					{ //Muda a indicação que o CBA será reiniciado.
						bVaiReiniciar = true;
					}

					if (bVaiReiniciar == false)
					{
						//Prepara o par de arquivos de registro estático
						//e registros dinâmicos com um novo registro:
						AQR_SetStaticRegData();
						osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_SAVE_STATIC_REG);
					}
				}

				//Ativa o flag de interface:
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_REGISTRO);
			}
			else //Senão, não estamos rodando.
			{
				//Se estamos somente sem memória:
				if ((dFlagsSis & UOS_SIS_FLAG_FFS_OK) > 0)
				{
					//Se o novo registro foi pedido por que está sendo
					//realizada a leitura do arquivo combinado:
					if (AQR_wCausaFim == AQR_wCF_LEITURA)
					{
						//Ajusta a causa de fim:
						AQR_wCausaFim = AQR_wCF_LIBERA_MEM;
					}

				} //Fim se( estamos somente sem memória )

			} //Fim senão( não estamos rodando normalmente )

			//Se alguem está pedindo reinício do sistema:
			if ((dFlagsSis & UOS_SIS_FLAG_REINICIO) > 0)
			{

				//Ativa a execução da tarefa de emergencia. Isso fara com que todos
				//os arquivos pendentes sejam salvos e provoca um Watchdog Timeout:
				//OSTaskResume(UOS_TRF_EMERGENCIA_PRIORIDADE);
			}

			//Reconhece o flag:
			osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_NOVO_REG);

		} //Fim se( deve criar um novo registro operacional )

		//--------------------------------------------------------------------------
		//Tratamento da Pausa
		//Se a pausa automática está habilitada...
		if (psMonitor->bPausaAuto != false)
		{
			//Se não estiver em modo trabalho, sai da pausa automática
			if ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0)
			{
				//Limpa Linhas pra pausa automática
				AQR_sStatus.bNumLinhasZero = 0;
			}

			//Se o número de linhas com zero de sementes é menor que o mínimo
			//configurado para pausa automática
			if (psStatus->bNumLinhasZero < bLinhasFalhaPausaAuto)
			{
				if (psStatus->bPausaAuto != false)
				{
					psStatus->bPausaAuto = false;

					//Indica que não está em pausa
					osFlagClear(AQR_sFlagREG, AQR_FLAG_PAUSA);
				}
			}
			else //if( psStatus->bNumLinhasZero >= psMonitor->bLinhasFalhaPausaAuto )
			{
				if (psStatus->bPausaAuto == false)
				{
					psStatus->bPausaAuto = true;

					//Decrementa Ponteiros do Buffer em anel e move valores acumulados
					//no momento da pausa automática para manobra
					sSegmentos.wPosIns = (sSegmentos.wPosIns - 1) & sSegmentos.wMskBufAnel;
					sSegmentos.wPosRet = (sSegmentos.wPosRet - 1) & sSegmentos.wMskBufAnel;

					psManobra->dDistancia += (uint32_t)((sSegmentos.awBufDis[sSegmentos.wPosIns] + 5) / 10);
					psTrabTotal->dDistancia -= (uint32_t)((sSegmentos.awBufDis[sSegmentos.wPosIns] + 5) / 10);
					psTrabParcial->dDistancia -= (uint32_t)((sSegmentos.awBufDis[sSegmentos.wPosIns] + 5) / 10);
					// Distância parcial para calculo da área parcial
					psDistTrabParcial->dDistancia -= (uint32_t)((sSegmentos.awBufDis[sSegmentos.wPosIns] + 5) / 10);
					psDistTrabTotal->dDistancia -= (uint32_t)((sSegmentos.awBufDis[sSegmentos.wPosIns] + 5) / 10);

					for (uint8_t bI = 0; bI < psMonitor->bNumLinhas; bI++)
					{
						//Verifica se o loop é menor que 32, se não, usa os flags extendidos
						if (bI < 32)
						{
							dLinhaAtual = 1 << bI;

							//Se a linha não estiver levantada (Arremate ou intercalação)
							if ((dLinhaAtual & psStatus->dLinhasLevantadas) == 0)
							{
								psManobra->adSementes[bI] += sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psManobra->dSomaSem += sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabTotal->adSementes[bI] -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabParcial->adSementes[bI] -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabTotal->dSomaSem -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabParcial->dSomaSem -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psDistTrabParcial->dSomaSem -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
							}
						}
						else
						{
							dLinhaAtualExt = 1 << (bI - 32);

							//Se a linha não estiver levantada (Arremate ou intercalação)
							if ((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0)
							{
								psManobra->adSementes[bI] += sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psManobra->dSomaSem += sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabTotal->adSementes[bI] -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabParcial->adSementes[bI] -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabTotal->dSomaSem -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psTrabParcial->dSomaSem -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
								psDistTrabParcial->dSomaSem -= sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
							}
						}
					}

					//Indica que está em Pausa
					osFlagSet(AQR_sFlagREG, AQR_FLAG_PAUSA);

					//Indica no registro que está em pausa
					AQR_bStatusReg |= AQR_PAUSA;
				}
			}
		}
		//Fim do Tratamento pausa
		//--------------------------------------------------------------------------

		//--------------------------------------------------------------------------
		//Indica Erro no GPS

		//Se nova mensagem for inválida, liga evento de erro de GPS:
		if (((AQR_sDadosGPS.bFlagsFix & GPS_FIX_STATUS_OK) != GPS_FIX_STATUS_OK) ||  //Se não tiver fixos
			((AQR_sDadosGPS.bValid & GPS_VALID_TIME) != GPS_VALID_TIME) || //Se não tiver hora válida
			(AQR_sDadosGPS.eStsAntena != OK) ||  //Se não tiver antena OK
			(AQR_sDadosGPS.dSpeedAcc > 150) || //Se a precisão de velocidade for maior que 1,5 m/s
			(AQR_sDadosGPS.bConfigura_FIM == false)) //Se a configuração do GPS não estiver finalizada
		{
			psStatus->bErroGPS = true;

			//Se não estiver usando o simulador e houver erro de GPS
			//Considera a velocidade igual a zero;
			if (CAN_bSensorSimulador == false)
			{
				//Força velocidade zero, pois está com erro de GPS
				psStatus->bVelZero = true;
				AQR_sDadosGPS.dGroundSpeed = 0;
			}
		}
		else
		{
			psStatus->bErroGPS = false;
			psStatus->bAlarmeGPS = false;
		}

		//--------------------------------------------------------------------------
		// Tratamento do parâmetro Velocidade Zero:

		//Se não houver erro de GPS ou
		//Se houver Sensor de velocidade do simulador
		if ((psStatus->bErroGPS == false) || (CAN_bSensorSimulador != false))
		{
			//se velocidade no GPS for menor que 0,14m/s = 0,5 km/h
			if (AQR_sDadosGPS.dGroundSpeed < 14)
			{
				//está parado
				psStatus->bVelZero = true;
				AQR_sDadosGPS.dGroundSpeed = 0;
			}
			else
			{
				//não está parado
				psStatus->bVelZero = false;

				//Ajusta o timer com timeout de 10 segundo
				status = START_TIMER(AQR_bTimerImpStopped, AQR_TIMEOUT_10SEGS);
				ASSERT(status == osOK);

				//Indica que o reset do Auto Desligamento deve ser resetado:
				osFlagSet(AQR_sFlagREG, AQR_FLAG_RESET_DESLIGA);

				//--------------------------------------------------------------------------------------//
				// RETIRAR COMENTÁRIO QUANDO O PROBLEMA DE VELOCIDADE DO GPS ESTIVER SOLUCIONADO
				//O problema do GPS é que mesmo parado ele às vezes indica velocidade
				/*
				 //Se os sensores já foram instalados e ainda não está em modo trabalho,
				 //porém é detectado velocidade, entra no modo trabalho
				 if( ( ( dFlagsSis & UOS_SIS_FLAG_CONFIRMA_INST ) > 0  ) &&
				 ( ( dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO ) == 0 )    )
				 {

				 // Grava a lista de sensores instalados:
				 CAN_vSalvaLista( );

				 OSFlagPost( UOS_sFlagSis, UOS_SIS_FLAG_CONFIRMA_INST, OS_FLAG_CLR, &bErr );
				 __assert( bErr == OS_NO_ERR );

				 OSFlagPost( UOS_sFlagSis,
				 UOS_SIS_FLAG_MODO_TESTE |
				 UOS_SIS_FLAG_MODO_TRABALHO, OS_FLAG_SET, &bErr );
				 __assert( bErr == OS_NO_ERR );
				 }
				 */
				//-------------------------------------------------------------------------------------//
			}
		}

		dFlagsSis = osFlagGet(UOS_sFlagSis);

		//Se houver timeout
		//Se não estiver em modo de trabalho
		//Se a instalação estiver concluída
		if (((dValorGPS & GPS_FLAG_TIMEOUT_MTR) > 0) && ((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) > 0))
		{
			if (psMonitor->bMonitorArea == false)
			{
				SEN_vReadDataFromSensors();
			}
		}

		//--------------------------------------------------------------------------
		//Trata o Auto Desligamento do MPA2500
		WATCHDOG_STATE(AQRMGT, WDT_SLEEP);
		uint32_t dValorFlagREG = osFlagWait(AQR_sFlagREG,
			(AQR_FLAG_DESLIGA | AQR_FLAG_RESET_DESLIGA | AQR_FLAG_IMP_PARADO), true, false, 0);
		WATCHDOG_STATE(AQRMGT, WDT_ACTIVE);

		if (dValorFlagREG != AQR_FLAG_NENHUM)
		{
			//Se passou o tempo de 30 minutos...
			if ((dValorFlagREG & AQR_FLAG_DESLIGA) > 0)
			{
				//Ajusta causa de fim
				AQR_wCausaFim = AQR_wCF_AUTO_DESLIGA;

				//Solicita o desligamento do MPA2500.
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_REINICIO);
			}
#ifndef TESTE_TECLADO
			//Se foi pressionado alguma tecla ou se tem velocidade no MPA...
			if ((dValorFlagREG & AQR_FLAG_RESET_DESLIGA) > 0)
			{
				//Ajusta o timer com timeout de 30 Min.
				status = START_TIMER(AQR_bTimerTurnOff, AQR_TIMEOUT_30MIN);
				ASSERT(status == osOK);
			}
#endif

			//Se está parado há mais de 10 segundos, zera insensibilidade
			if ((dValorFlagREG & AQR_FLAG_IMP_PARADO) > 0)
			{
				if (dDistInsens > 0)
				{
					//Zera distância de insensibilidade
					dDistInsens = 0;
				}
			}
		}

		//--------------------------------------------------------------------------
		// Tratamento do parâmetro Trabalhando:

		//Se não está em Pausa e
		//Se não tiver Implemento Levantado e
		//Se não tiver Velocidade Zero e
		//Se está em modo de trabalho e
		//Se não tiver Erro de GPS ou houver simulador de velocidade

		dValorFlag = osFlagGet(AQR_sFlagREG);

		if (((dValorFlag & AQR_FLAG_PAUSA) == 0) && //( psStatus->bImplemento == false) &&
			(psStatus->bVelZero == false) && ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) > 0)
			&& ((psStatus->bErroGPS == false) || (CAN_bSensorSimulador != false)))
		{
			if (psStatus->bTrabalhando == false)
			{
				psStatus->bPrimeiroSegmento = true;

				//Está trabalhando
				psStatus->bTrabalhando = true;
			}
		}
		else //Senão...
		{
			//Não está trabalhando
			psStatus->bTrabalhando = false;
		}

		//--------------------------------------------------------------------------
		// Tratamento do parâmetro Insensível:

		//Se estiver trabalhando e não estiver como monitor de área
		if ((psStatus->bTrabalhando != false) && (psMonitor->bMonitorArea == false))
		{
			//Se a insensibilidade configurada for maior que a distância percorrida
			if ((uint32_t)(psMonitor->wInsensibilidade * 10) > dDistInsens)
			{
				//Liga flag de insensibilidade
				psStatus->bInsensivel = true;

				//Indica no registro que está insensível
				AQR_bStatusReg |= AQR_INSENSIVEL;
			}
			else
			{
				//Desliga o flag de insensibilidade
				psStatus->bInsensivel = false;
			}
		}

		//Se estiver com velocidade zero ou
		//Se não estiver em modo trabalho
		//Se não for monitor de área
		if ((psStatus->bVelZero != false) || ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0))
		{
			//Ajusta o timer com timeout de 1 segundo
			status = START_TIMER(GPS_bTimerMtr, GPS_TIMEOUT_1S);
			ASSERT(status == osOK);
		}

		//--------------------------------------------------------------------------
		//Tratamento dos dados coletados no último metro percorrido:

		//Se está trabalhando e passou 1 metro:
		if (psStatus->bTrabalhando != false)
		{
			//Só acumula dados se o motivo de acordar a tarefa for percorrer 1 metro
			if ((dValorGPS & GPS_FLAG_METRO) > 0)
			{
				uint16_t wAux;
				uint16_t wPtrAux;
				uint8_t bI;
				uint8_t bOffSeg;
				uint8_t bAdubo;

				//Aponta para a posição livre corrente do buffer.
				wAux = sSegmentos.wPosIns;
				psStatus->bNumLinhasZero = 0x00;

				//Coloca no Buffer a distância exata percorrida no último metro (em milímetros)
				sSegmentos.awBufDis[wAux] = (uint16_t)((AQR_sDadosGPS.fDistancia * 10.0f) + 0.5f);

				//Acumula distância trabalhada total e parcial em m:
				psTrabTotal->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);
				psTrabParcial->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);
				// Area parcial
				psDistTrabParcial->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);
				psDistTrabTotal->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);

				//Acumula distância para verificar insensibilidade
				dDistInsens += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);

				//Se não estiver trabalhando como modo monitor de área
				if (psMonitor->bMonitorArea == false)
				{

					//Verifica se existe linha com falha de semente
					AQR_vVerificaFalha();

					//Coloca no Buffer de cada linha a quantidade de sementes e informação de
					//adubo por metro
					for (bI = 0; bI < psMonitor->bNumLinhas; bI++)
					{
						//Verifica se o loop é menor que 32, se não, usa os flags extendidos
						if (bI < 32)
						{
							dLinhaAtual = 1 << bI;
						}
						else
						{
							dLinhaAtualExt = 1 << (bI - 32);
						}

						//Números Pares - Sensores de Semente
						uint8_t bI1 = (bI * 2);

						//Números ímpares - Sensores de Adubo
						uint8_t bI2 = (bI1 + 1);

						//Se a linha está sendo ignorada e  verificando se está
						//Utilizando os flags de extensão, verificando se é maior que 32 bits
						if ((((dLinhaAtual & psStatus->dSementeIgnorado) > 0) && (bI < 32))
							|| (((dLinhaAtualExt & psStatus->dSementeIgnoradoExt) > 0) && (bI >= 32)))
						{
							//Se a linha não estiver levantada (Arremate ou intercalação)
							//Utilizando os flags de extensão, verificando se é maior que 32 bits
							if ((((dLinhaAtual & psStatus->dLinhasLevantadas) == 0) && (bI < 32))
								|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0) && (bI >= 32)))
							{
								//Devido à necessidade de trabalharmos com valores inteiros de semente,
								//devemos arredondar o valor configurado, para acumular na linha ignorada
								//o valor equivalente ao real, sem perder a parte fracionária.
								uint8_t bArredondamento = (uint8_t)(psMonitor->wSementesPorMetro * 0.1);
								bArredondamento = psMonitor->wSementesPorMetro - (bArredondamento * 10);
								bArredondamento = 10 - bArredondamento;

								//Coloca no buffer informações sobre sensores de semente
								sSegmentos.abBufSem[bI][wAux] = (uint8_t)((psMonitor->wSementesPorMetro
									+ bArredondamento) * 0.1f);

								//Soma o total de sementes por linha em trabalho
								psTrabTotal->adSementes[bI] += (uint32_t)((psMonitor->wSementesPorMetro
									+ bArredondamento) * 0.1f);
								psTrabParcial->adSementes[bI] += (uint32_t)((psMonitor->wSementesPorMetro
									+ bArredondamento) * 0.1f);

								//Soma o total de sementes em todas as linha em trabalho
								psTrabTotal->dSomaSem += (uint32_t)((psMonitor->wSementesPorMetro + bArredondamento)
									* 0.1f);
								psTrabParcial->dSomaSem += (uint32_t)((psMonitor->wSementesPorMetro + bArredondamento)
									* 0.1f);
								psDistTrabParcial->dSomaSem += (uint32_t)((psMonitor->wSementesPorMetro
									+ bArredondamento) * 0.1f);
								//Sementes por metro * 10 - Distância em m
								//utilizamos para esta conta o valor sem arredondamento, pois precisamos extrair a fração real que está
								//sendo "perdida" nos cálculos em função da distância do segmento não ser exatamente 1 metro
								int32_t lFracaoSemente = (uint32_t)(psMonitor->wSementesPorMetro
									* sSegmentos.awBufDis[wAux] * 0.1);
								//Subtraimos deste valor a quantidade de sementes que já havia sido acumulada (com arredondamento)
								//desta forma acumulamos o valor fracionário, com 3 casas decimais de precisão
								sSegmentos.alBufSemFrac[bI] += lFracaoSemente
									- ((psMonitor->wSementesPorMetro + bArredondamento) * 100);

								if ((sSegmentos.alBufSemFrac[bI] >= 1000) || (sSegmentos.alBufSemFrac[bI] <= -1000))
								{
									lFracaoSemente = (int32_t)(sSegmentos.alBufSemFrac[bI] * 0.001f);

									sSegmentos.alBufSemFrac[bI] -= lFracaoSemente * 1000.0f;
									sSegmentos.abBufSem[bI][wAux] += lFracaoSemente;
									//Soma o total de sementes por linha em trabalho
									psTrabTotal->adSementes[bI] += lFracaoSemente;
									psTrabParcial->adSementes[bI] += lFracaoSemente;
									//Soma o total de sementes em todas as linha em trabalho
									psTrabTotal->dSomaSem += lFracaoSemente;
									psTrabParcial->dSomaSem += lFracaoSemente;
									psDistTrabParcial->dSomaSem += lFracaoSemente;
								}
							}
						}
						else
						{
							//Se a linha atual não estiver levantada verificando se está
							//Utilizando os flags de extensão, verificando se é maior que 32 bits
							if ((((dLinhaAtual & psStatus->dLinhasLevantadas) == 0) && (bI < 32))
								|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0) && (bI >= 32)))
							{
								//Coloca no buffer informações sobre sensores de semente
								sSegmentos.abBufSem[bI][wAux] = psAQR_Sensor[bI1].abUltimaLeitura[0];

								//Soma o total de sementes por linha em trabalho
								psTrabTotal->adSementes[bI] += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
								psTrabParcial->adSementes[bI] += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];

								//Se não caiu semente na linha
								if ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100)
									< psStatus->wMinSementesZero)
								{
									//Incrementa a quantidade de linhas com zero sementes
									psStatus->bNumLinhasZero++;

									//Verifica se está em insensibilidade
									if (AQR_sStatus.bInsensivel == false)
									{
										//Indica a falha de sensor de semente próximo de zero
										//Verifica qual flag irá sinalizar
										if (bI < 32)
										{
											AQR_sStatus.dSementeZero |= 0x00000001 << bI;
										}
										else
										{
											AQR_sStatus.dSementeZeroExt |= 0x00000001 << (bI - 32);
										}

										abTimeoutAlarme[bI] =
										AQR_DISTANCIA_LIMPA_FALHA;

										AQR_sFalhaInstantanea.abBufSem[bI] = sSegmentos.abBufSem[bI][wAux];
										AQR_sFalhaInstantanea.awBufDis = (uint16_t)((AQR_sDadosGPS.fDistancia * 10.0f)
											+ 0.5f);
										//Indica falha de sensor - usado pela IHM
										//AQR_sStatus.dSementeZeroIHM |= 0x00000001 << bI;
									}                      //fim insensibilidade
								}
								else
								{
									//Limpa falha de sensor de semente próximo de zero
									//Verifica qual flag irá sinalizar
									if (bI < 32)
									{
										AQR_sStatus.dSementeZero &= ~(0x00000001 << bI);
									}
									else
									{
										AQR_sStatus.dSementeZeroExt &= ~(0x00000001 << (bI - 32));
									}

									if (abTimeoutAlarme[bI] == 0)
									{
										//Verifica qual flag irá sinalizar
										if (bI < 32)
										{
											AQR_sStatus.dSementeZeroIHM &= ~(0x00000001 << bI);
										}
										else
										{
											AQR_sStatus.dSementeZeroIHMExt &= ~(0x00000001 << (bI - 32));
										}
									}
								}

								//Soma o total de sementes em todas as linha em trabalho
								psTrabTotal->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
								psTrabParcial->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
								psDistTrabParcial->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
							}
						}

						//Se o sensor foi marcado como ignorado pelo usuário verifica se
						//Utiliza as flags de extensão, verificando se é maior que 32 bits
						if ((((dLinhaAtual & psStatus->dAduboIgnorado) > 0) && (bI < 32))
							|| (((dLinhaAtualExt & psStatus->dAduboIgnoradoExt) > 0) && (bI >= 32)))
						{
							//Coloca no buffer informações ideais
							sSegmentos.abBufAdu[bI][wAux] = 1;
						}
						else
						{
							//Coloca no buffer informações sobre sensores de Adubo
							sSegmentos.abBufAdu[bI][wAux] = psAQR_Sensor[bI2].abUltimaLeitura[0];
							bAdubo = psAQR_Sensor[bI2].abUltimaLeitura[0];

							//Se existir sensor de adubo instalado e não tiver caído adubo neste metro
							if ((psMonitor->bSensorAdubo) && (bAdubo == 0)) //<<----------o VERIFICAR
							{
								//Verifica se o loop é menor que 32, se não, usa os flags extendidos
								if (bI < 32)
								{
									//Indica a falha de sensor de adubo nesta linha:
									psStatus->dAduboFalha |= 0x00000001 << bI;
								}
								else
								{
									//Indica a falha de sensor de adubo nesta linha:
									psStatus->dAduboFalhaExt |= 0x00000001 << (bI - 32);
								}
							}
							/*
							 else
							 {
							 //Limpa a falha de sensor de adubo nesta linha:
							 psStatus->dAduboFalha &= ~( 0x00000001 << bI );
							 }
							 */
						}
					}

					//Se não vai entrar em Pausa
					if (psStatus->bNumLinhasZero < bLinhasFalhaPausaAuto)
					{
						psStatus->bAlarmeLinhasLevantadas = 0x00;
						for (bI = 0; bI < psMonitor->bNumLinhas; bI++)
						{
							//Verifica se o loop é menor que 32, se não, usa os flags extendidos
							if (bI < 32)
							{
								dLinhaAtual = 1 << bI;
							}
							else
							{
								dLinhaAtualExt = 1 << (bI - 32);
							}

							//Números Pares - Sensores de Semente
							uint8_t bI1 = (bI * 2);

							if ((((dLinhaAtual & psStatus->dLinhasLevantadas) > 0) && (bI < 32))
								|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) > 0) && (bI >= 32)))

							{
								//Conta quantas linhas que estão levantadas estão contando semente
								if ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100)
									> psStatus->wMinSementesZero)
								{
									psStatus->bAlarmeLinhasLevantadas++;
								}
							}
						}
					}

					//----------------------------------------------------------------------------

					if (GPS_bDistanciaPercorrida==0 || AQR_bDistanciaPercorrida == 0)
					{
						wPtrAux = 0;
					}
					//Arredonda Distância para Avaliação
					//Arredonda variável de avaliação para um número inteiro.
					AQR_sStatus.wAvaliaArred = (psMonitor->wAvalia * 100) / AQR_bDistanciaPercorrida;
					AQR_sStatus.wAvaliaArred = ((AQR_sStatus.wAvaliaArred + 5) / 10);
					GPS_bDistanciaPercorrida = 0;
					AQR_bDistanciaPercorrida = 0;

					//Se a posição de retirado do buffer for zero, Aponta para o fim
					if (sSegmentos.wPosRet == 0)
					{
						wPtrAux = sSegmentos.wMskBufAnel;
					}
					else
					{
						//Aponta para a posição do buffer
						wPtrAux = (sSegmentos.wPosRet - 1);
					}

					//--------------------------------------------------------------------
					//Calcula a média dos últimos metros, de acordo com a configuração para
					//avaliação

					//Zera Acumuladores de distância, Adubo, Sementes:
					memset(psAvalia, 0x00, sizeof(AQR_sAcumulado.sAvalia));

					//Acumula valores dos n últimos segmentos, onde n é o valor da
					//configuração de avaliação:
					for (bOffSeg = 0; bOffSeg < psStatus->wAvaliaArred; bOffSeg++)
					{
						//Acumula distância exata percorrida em milímetros:
						psAvalia->dDistancia += (uint32_t)sSegmentos.awBufDis[wPtrAux];

						for (bI = 0; bI < psMonitor->bNumLinhas; bI++)
						{
							//Verifica se o loop é maior que 32, para usar os flags extendidos
							if (bI < 32)
							{
								dLinhaAtual = 1 << bI;
							}
							else
							{
								dLinhaAtualExt = 1 << (bI - 32);
							}

							//Se a linha atual não estiver levantada e verificando se está
							//Utilizando os flags de extensão, verificando se é maior que 32 bits
							if ((((dLinhaAtual & psStatus->dLinhasLevantadas) == 0) && (bI < 32))
								|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0) && (bI >= 32)))
							{
								//Acumula Quantidade de Sementes Plantadas:
								psAvalia->adSementes[bI] += (uint32_t)sSegmentos.abBufSem[bI][wPtrAux];

								//Acumula Adubo lançado
								//psAvalia->abAdubo[bI] += sSegmentos.abBufAdu[bI][wPtrAux];//<<---o VERIFICAR
							}
						}

						//Decrementa o ponteiro de retirada e quando o ponteiro chegar
						//a zero, aponta para última posição do buffer
						if (wPtrAux-- <= 0)
							wPtrAux = sSegmentos.wMskBufAnel;
					}

					//Verifica se existe linha com falha de semente
					//AQR_vVerificaFalha();

					//Incrementa ponteiro de inserção
					sSegmentos.wPosIns = ((wAux + 1) & sSegmentos.wMskBufAnel);

					//Incrementa ponteiro de Retirada
					sSegmentos.wPosRet = sSegmentos.wPosIns;
				}
			}
		}
		else //se não está trabalhando
		{
			uint8_t bLinhasErroGPS = 0x00;

			//Se percorreu um metro
			if ((dValorGPS & GPS_FLAG_METRO) > 0)
			{
				//Zera Acumuladores de distância, Adubo, Sementes:
				//MemSet( psAvalia,0x00 ,sizeof( AQR_sAcumulado.sAvalia ) );
				psStatus->bNumLinhasZero = 0x00;
			}

			//Se não estiver em modo teste
			if ((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) > 0)
			{
				//Não estamos acumulando a distância quando não passou 1 metro, pois na
				//rotina que acumula distância, esta informação apenas estará disponível quando passar 1 metro
				//Caso haja necessidade de utilizar este valor, a rotina GPS_vAcumulaDistancia() deverá ser revista.
				//Soma a distância percorrida em manobra
				//psManobra->dDistancia += ( uint32_t )( AQR_sDadosGPS.fDistancia + 0.5f );
				//GPS_sDadosGPS.fDistancia = 0;

				for (uint8_t bI = 0; bI < psMonitor->bNumLinhas; bI++)
				{
					//Verifica se o loop é maior que 32, para usar os flags extendidos
					if (bI < 32)
					{
						dLinhaAtual = 1 << bI;
					}
					else
					{
						dLinhaAtualExt = 1 << (bI - 32);
					}

					//Números Pares - Sensores de Semente
					uint8_t bI1 = (bI * 2);

					if (((dValorGPS & GPS_FLAG_METRO) > 0) || ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0))
					{
						//Soma o total de sementes por linha em manobra
						psManobra->adSementes[bI] += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];

						//Soma o total de sementes em todas as Linhas em manobra
						psManobra->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
					}

					//Se percorreu um metro
					if ((dValorGPS & GPS_FLAG_METRO) > 0)
					{
						//Se está em pausa automática
						//E a linha atual não está sendo ignorada
						//E a linha atual não está intercalada
						//E a linha atual não está em arremate
						//E a última leitura do sensor foi zero
						//Verificando se está utilizando os flags de extensão, verificando se é maior que 32 bits
						if (((psStatus->bPausaAuto != false) && ((dLinhaAtual & psStatus->dSementeIgnorado) == 0)
							&& ((dLinhaAtual & psStatus->dLinhasLevantadas) == 0)
							&& ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100) < psStatus->wMinSementesZero)
							&& (bI < 32))
							|| ((psStatus->bPausaAuto != false)
								&& ((dLinhaAtualExt & psStatus->dSementeIgnoradoExt) == 0)
								&& ((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0)
								&& ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100) < psStatus->wMinSementesZero)
								&& (bI >= 32)))
						{
							//Incrementa o número de linhas que não está caindo semente
							psStatus->bNumLinhasZero++;
						}
					}

					//Verifica quantas linhas está caindo semente para erro de GPS
					if ((((dLinhaAtual & psStatus->dSementeIgnorado) == 0)
						&& ((dLinhaAtual & psStatus->dLinhasLevantadas) == 0)
						&& ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100) > psStatus->wMinSementesZero))
						|| (((dLinhaAtualExt & psStatus->dSementeIgnoradoExt) == 0)
							&& ((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0)
							&& ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100) > psStatus->wMinSementesZero)))
					{
						//Incrementa o número de linhas que estão caindo semente
						bLinhasErroGPS++;
					}
				}

				//Se estiver caindo sementes nas Linhas.
				//Se houver erro no gps .
				//Se não estiver no modo simulador.
				if ( //( bLinhasErroGPS >= bLinhasFalhaPausaAuto )&&
				(bLinhasErroGPS > 0) && (psStatus->bErroGPS != false) && (CAN_bSensorSimulador == false))
				{
					//Aciona alarme do GPS
					psStatus->bAlarmeGPS = true;
				}
				else
				{
					psStatus->bAlarmeGPS = false;
				}

				//Se não passou 1 metro, desconsidera linhas para sair de pausa
				if ((dValorGPS & GPS_FLAG_METRO) > 0) // Se passou 1 metro
				{

					//Se está em pausa automática e vai sair da pausa
					if ((psStatus->bNumLinhasZero < bLinhasFalhaPausaAuto) && (psStatus->bPausaAuto != false))
					{
						//Sai da Pausa
						psStatus->bPausaAuto = false;

						//Indica que não está em pausa
						osFlagClear(AQR_sFlagREG, AQR_FLAG_PAUSA);

						//Coloca no Buffer a distância exata percorrida no último metro (em milímetros)
						sSegmentos.awBufDis[sSegmentos.wPosIns] =
							(uint16_t)((AQR_sDadosGPS.fDistancia * 10.0f) + 0.5f);

						//Acumula distância trabalhada total e parcial em m:
						psTrabTotal->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);
						psTrabParcial->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);
						// Area parcial
						psDistTrabParcial->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);
						psDistTrabTotal->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);

						//Acumula distância para verificar insensibilidade
						dDistInsens += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);

						for (uint8_t bI = 0; bI < psMonitor->bNumLinhas; bI++)
						{
							//Verifica se o loop é maior que 32, para usar os flags extendidos
							if (bI < 32)
							{
								dLinhaAtual = 1 << bI;
							}
							else
							{
								dLinhaAtualExt = 1 << (bI - 32);
							}

							//Números Pares - Sensores de Semente
							uint8_t bI1 = (bI * 2);

							//Soma o total de sementes por linha em manobra
							psManobra->adSementes[bI] -= (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];

							//Soma o total de sementes em todas as Linhas em manobra
							psManobra->dSomaSem -= (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];

							//Se a linha está sendo ignorada ou
							//Se a linha está sendo ignorada e são os flags extendidos
							if (((((dLinhaAtual & psStatus->dSementeIgnorado) > 0) && (bI < 32)))
								|| (((dLinhaAtualExt & psStatus->dSementeIgnoradoExt) > 0) && (bI >= 32)))
							{
								//Se a linha não estiver levantada (Arremate ou intercalação) ou
								//Se a linha não estiver levantada e forem os flags de extendidos
								if ((((dLinhaAtual & psStatus->dLinhasLevantadas) == 0) && (bI < 32))
									|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0) && (bI >= 32)))
								{
									//Devido à necessidade de trabalharmos com valores inteiros de semente,
									//devemos arredondar o valor configurado, para acumular na linha ignorada
									//o valor equivalente ao real, sem perder a parte fracionária.
									uint8_t bArredondamento = (uint8_t)(psMonitor->wSementesPorMetro * 0.1);
									bArredondamento = psMonitor->wSementesPorMetro - (bArredondamento * 10);
									bArredondamento = 10 - bArredondamento;

									//Coloca no buffer informações sobre sensores de semente
									sSegmentos.abBufSem[bI][sSegmentos.wPosIns] =
										(uint8_t)((psMonitor->wSementesPorMetro + bArredondamento) * 0.1f);

									//Soma o total de sementes por linha em trabalho
									psTrabTotal->adSementes[bI] += (uint32_t)((psMonitor->wSementesPorMetro
										+ bArredondamento) * 0.1f);
									psTrabParcial->adSementes[bI] += (uint32_t)((psMonitor->wSementesPorMetro
										+ bArredondamento) * 0.1f);

									//Soma o total de sementes em todas as linha em trabalho
									psTrabTotal->dSomaSem +=
										(uint32_t)((psMonitor->wSementesPorMetro + bArredondamento) * 0.1f);
									psTrabParcial->dSomaSem += (uint32_t)((psMonitor->wSementesPorMetro
										+ bArredondamento) * 0.1f);
									psDistTrabParcial->dSomaSem += (uint32_t)((psMonitor->wSementesPorMetro
										+ bArredondamento) * 0.1f);
									//Sementes por metro * 10 - Distância em m
									//utilizamos para esta conta o valor sem arredondamento, pois precisamos extrair a fração real que está
									//sendo "perdida" nos cálculos em função da distância do segmento não ser exatamente 1 metro
									int32_t lFracaoSemente = (uint32_t)(psMonitor->wSementesPorMetro
										* sSegmentos.awBufDis[sSegmentos.wPosIns] * 0.1);
									//Subtraimos deste valor a quantidade de sementes que já havia sido acumulada (com arredondamento)
									//desta forma acumulamos o valor fracionário, com 3 casas decimais de precisão
									sSegmentos.alBufSemFrac[bI] += lFracaoSemente
										- ((psMonitor->wSementesPorMetro + bArredondamento) * 100);

									if ((sSegmentos.alBufSemFrac[bI] >= 1000) || (sSegmentos.alBufSemFrac[bI] <= -1000))
									{
										lFracaoSemente = (int32_t)(sSegmentos.alBufSemFrac[bI] * 0.001f);

										sSegmentos.alBufSemFrac[bI] -= lFracaoSemente * 1000.0f;
										sSegmentos.abBufSem[bI][sSegmentos.wPosIns] += lFracaoSemente;
										//Soma o total de sementes por linha em trabalho
										psTrabTotal->adSementes[bI] += lFracaoSemente;
										psTrabParcial->adSementes[bI] += lFracaoSemente;
										//Soma o total de sementes em todas as linha em trabalho
										psTrabTotal->dSomaSem += lFracaoSemente;
										psTrabParcial->dSomaSem += lFracaoSemente;
										psDistTrabParcial->dSomaSem += lFracaoSemente;

									}
								}
							}
							else
							{
								//Se a linha atual não estiver levantada ou
								//Se a linha atual não estiver levantada dos flags extendidos
								if ((((dLinhaAtual & psStatus->dLinhasLevantadas) == 0) && (bI < 32))
									|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0) && (bI >= 32)))
								{
									//Coloca no buffer informações sobre sensores de semente
									sSegmentos.abBufSem[bI][sSegmentos.wPosIns] = psAQR_Sensor[bI1].abUltimaLeitura[0];

									//Soma o total de sementes por linha em trabalho
									psTrabTotal->adSementes[bI] += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
									psTrabParcial->adSementes[bI] += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];

									//Se não caiu semente na linha
									if ((uint16_t)(psAQR_Sensor[bI1].abUltimaLeitura[0] * 100)
										< psStatus->wMinSementesZero)
									{
										//Verifica se está em insensibilidade
										if (AQR_sStatus.bInsensivel == false)
										{
											//Indica a falha de sensor de semente próximo de zero
											//Verifica se o loop é maior que 32, para usar, os flags extendidos
											if (bI < 32)
											{
												AQR_sStatus.dSementeZero |= 0x00000001 << bI;
											}
											else
											{
												AQR_sStatus.dSementeZeroExt |= 0x00000001 << (bI - 32);
											}
											abTimeoutAlarme[bI] =
											AQR_DISTANCIA_LIMPA_FALHA;

											AQR_sFalhaInstantanea.abBufSem[bI] =
												sSegmentos.abBufSem[bI][sSegmentos.wPosIns];
											AQR_sFalhaInstantanea.awBufDis = (uint16_t)((AQR_sDadosGPS.fDistancia
												* 10.0f) + 0.5f);
											//Indica falha de sensor - usado pela IHM
											//AQR_sStatus.dSementeZeroIHM |= 0x00000001 << bI;
										}                  //fim insensibilidade
									}
									else
									{
										//Limpa falha de sensor de semente próximo de zero
										//Verifica se o loop é maior que 32, para usar, os flags extendidos
										if (bI < 32)
										{
											AQR_sStatus.dSementeZero &= ~(0x00000001 << bI);
										}
										else
										{
											AQR_sStatus.dSementeZeroExt &= ~(0x00000001 << (bI - 32));
										}

										//Após 10 metros que parou o alarme sonoro, limpa o alarme visual
										//Este timeout esta sendo decrementado na função que verifca a falha de tolerancia,
										//Porque ela sempre ocorrerá quando a falha instantanea ocorrer
										if (abTimeoutAlarme[bI] == 0)
										{
											//Verifica se o loop é maior que 32, para usar os flags extendidos
											if (bI < 32)
											{
												AQR_sStatus.dSementeZeroIHM &= ~(0x00000001 << bI);
											}
											else
											{
												AQR_sStatus.dSementeZeroIHMExt &= ~(0x00000001 << (bI - 32));
											}
										}
									}

									//Soma o total de sementes em todas as linha em trabalho
									psTrabTotal->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
									psTrabParcial->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
									psDistTrabParcial->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
								}
							}
						}
						//--------------------------------------------------------------------
					}
				}
			}
		}

		//--------------------------------------------------------------------------
		//Tratamento de parâmetros para Registro:
		psTotalReg->dDistancia += (uint32_t)(AQR_sDadosGPS.fDistancia + 0.5f);

		//total de Sementes
		for (uint8_t bI = 0; bI < psMonitor->bNumLinhas; bI++)
		{
			//Verifica se o loop é maior que 32, para usar os flags extendidos
			if (bI < 32)
			{
				dLinhaAtual = 1 << bI;
			}
			else
			{
				dLinhaAtualExt = 1 << (bI - 32);
			}

			//Números Pares - Sensores de Semente
			uint8_t bI1 = (bI * 2);

			//Se a linha atual não estiver levantada ou
			//Se a linha atual não estiver levantada e estiver usando os flags extendidos
			if ((((dLinhaAtual & psStatus->dLinhasLevantadas) == 0) && (bI < 32))
				|| (((dLinhaAtualExt & psStatus->dLinhasLevantadasExt) == 0) && (bI >= 32)))
			{
				//Soma o total de sementes por Linha
				psTotalReg->adSementes[bI] += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];

				//Soma o total de sementes em todas as Linhas
				psTotalReg->dSomaSem += (uint32_t)psAQR_Sensor[bI1].abUltimaLeitura[0];
			}
		}

		//--------------------------------------------------------------------------
		//Registro Estático:
		//Salva Registro
		//
		//osFlagSet(xAQR_sFlagSis, AQR_APL_FLAG_SAVE_STATIC_REG);

		//--------------------------------------------------------------------------
		// Tratamento do parâmetro Alarmes:

		uint8_t bAlarmes = false;

		//Se está em modo trabalho e
		//Não está em Auto Teste
		if ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) > 0)
		{
			if (bLimpaFalhas == true)
			{
				bLimpaFalhas = false;

				//Limpa as falhas:
				psStatus->dSementeFalha = 0x00000000;
				psStatus->dSementeZero = 0x00000000;
				psStatus->dSementeFalhaIHM = 0x00000000;
				psStatus->dSementeZeroIHM = 0x00000000;
				psStatus->dAduboFalha = 0x00000000;

				psStatus->dSementeFalhaExt = 0x00000000;
				psStatus->dSementeZeroExt = 0x00000000;
				psStatus->dSementeFalhaIHMExt = 0x00000000;
				psStatus->dSementeZeroIHMExt = 0x00000000;
				psStatus->dAduboFalhaExt = 0x00000000;

				psStatus->bAdicionalDesconectado = 0x00;
				psStatus->bMemAdicionalDesconectado = 0x00;
				psStatus->dLinhaDesconectada = 0x00000000;
				psStatus->dMemLinhaDesconectada = 0x00000000;
				psStatus->dLinhaDesconectadaExt = 0x00000000;
				psStatus->dMemLinhaDesconectadaExt = 0x00000000;
				psStatus->bLinhasFalha = false;
				psStatus->bLinhasZero = false;
				psStatus->bSensorDesconectado = false;
			}

			AQR_wAlarmes = 0;

			// se houver linha em falha aciona o alarme
			if (psStatus->bLinhasZero != false)
			{
				bAlarmes = true;
				AQR_wAlarmes |= AQR_FALHA_LINHA;
				AQR_bStatusReg |= AQR_FALHA_LINHA; //Indica no registro que há falha em 1 ou mais linhas
			}

			// se houver linha em falha aciona o alarme
			if (psStatus->bLinhasFalha != false)
			{
				//Se a falha é em outro sensor, então sinaliza com um novo alarme.
				if ((((AQR_sStatus.dSementeFalha ^ AQR_sStatus.dSementeFalhaIHM) & AQR_sStatus.dSementeFalha) > 0)
					|| (((AQR_sStatus.dSementeFalhaExt ^ AQR_sStatus.dSementeFalhaIHMExt) & AQR_sStatus.dSementeFalhaExt)
						> 0))
				{
					bAlarmes = true;
					AQR_sStatus.dSementeFalhaIHM |= AQR_sStatus.dSementeFalha;
					AQR_sStatus.dSementeFalhaIHMExt |= AQR_sStatus.dSementeFalhaExt;
					AQR_wAlarmes |= AQR_FALHA_TOLERANCIA_LINHA;
					AQR_bStatusReg |= AQR_FALHA_TOLERANCIA_LINHA; //Indica no registro que há falha em 1 ou mais linhas
				}
			}

			//Se houver algum sensor desconectado aciona o alarme
			if (psStatus->bSensorDesconectado != false)
			{
				if (psStatus->bTrabalhando != false)
				{
					AQR_wAlarmes |= AQR_SENSOR_DESCONECTADO;
					bAlarmes = true;

					//indica sensor desconectado
					AQR_bStatusReg |= AQR_SENSOR_DESCONECTADO;
				}
				else
				{
					if (((psStatus->dLinhaDesconectada | psStatus->dMemLinhaDesconectada)
						!= psStatus->dMemLinhaDesconectada)
						|| ((psStatus->bAdicionalDesconectado | psStatus->bMemAdicionalDesconectado)
							!= psStatus->bMemAdicionalDesconectado)
						|| ((psStatus->dLinhaDesconectadaExt | psStatus->dMemLinhaDesconectadaExt)
							!= psStatus->dMemLinhaDesconectadaExt))
					{
						AQR_wAlarmes |= AQR_SENSOR_DESCONECTADO;

						bAlarmes = true;
					}
					else
					{
						//Limpa variável de Alarme
						bAlarmes = false;
					}
				}

				psStatus->dMemLinhaDesconectada |= psStatus->dLinhaDesconectada;
				psStatus->dMemLinhaDesconectadaExt |= psStatus->dLinhaDesconectadaExt;
				psStatus->bMemAdicionalDesconectado |= psStatus->bAdicionalDesconectado;
			}

			//Se tiver que tocar alarme de falha de GPS
			if (psStatus->bAlarmeGPS != false)
			{
				AQR_wAlarmes |= AQR_FALHA_GPS;
				bAlarmes = true;
			}
		}
		else
		{

			bLimpaFalhas = true;

			//Limpa as falhas:
			psStatus->dSementeFalha = 0x00000000;
			psStatus->dSementeZero = 0x00000000;
			psStatus->dSementeFalhaIHM = 0x00000000;
			psStatus->dSementeZeroIHM = 0x00000000;
			psStatus->dAduboFalha = 0x00000000;

			psStatus->dSementeFalhaExt = 0x00000000;
			psStatus->dSementeZeroExt = 0x00000000;
			psStatus->dSementeFalhaIHMExt = 0x00000000;
			psStatus->dSementeZeroIHMExt = 0x00000000;
			psStatus->dAduboFalhaExt = 0x00000000;

			psStatus->bAdicionalDesconectado = 0x00;
			psStatus->bMemAdicionalDesconectado = 0x00;
			psStatus->dLinhaDesconectada = 0x00000000;
			psStatus->dMemLinhaDesconectada = 0x00000000;
			psStatus->dLinhaDesconectadaExt = 0x00000000;
			psStatus->dMemLinhaDesconectadaExt = 0x00000000;
			psStatus->bLinhasFalha = false;
			psStatus->bLinhasZero = false;
			psStatus->bSensorDesconectado = false;
			psStatus->bAlarmeGPS = false;
		}

		//se houver reconhecimento de falha...
		if (psStatus->bAlarmeOK != false)
		{
			//Limpa variável de reconhecimento de falha
			psStatus->bAlarmeOK = false;

			//Limpa variável de Alarme
			bAlarmes = false;

			//Limpa as falhas:
			psStatus->dSementeFalha = 0x00000000;
			psStatus->dSementeFalhaIHM = 0x00000000;
			psStatus->dSementeZero = 0x00000000;
			psStatus->dSementeZeroIHM = 0x00000000;
			psStatus->dAduboFalha = 0x00000000;

			psStatus->dSementeFalhaExt = 0x00000000;
			psStatus->dSementeFalhaIHMExt = 0x00000000;
			psStatus->dSementeZeroExt = 0x00000000;
			psStatus->dSementeZeroIHMExt = 0x00000000;
			psStatus->dAduboFalhaExt = 0x00000000;

			psStatus->bAdicionalDesconectado = 0x00;
			psStatus->bMemAdicionalDesconectado = 0x00;
			psStatus->dLinhaDesconectada = 0x00000000;
			psStatus->dMemLinhaDesconectada = 0x00000000;

			psStatus->dLinhaDesconectadaExt = 0x00000000;
			psStatus->dMemLinhaDesconectadaExt = 0x00000000;

			psStatus->bLinhasFalha = false;
			psStatus->bLinhasZero = false;
			psStatus->bSensorDesconectado = false;
			psStatus->bAlarmeLinhasLevantadas = 0x00;

		}

		//Se estiver Insensivel
		if ((psStatus->bInsensivel != false) || (psStatus->bPrimeiroSegmento != false))
		{
			psStatus->bPrimeiroSegmento = false;

			//Somente limpa alarme sonoro se não houver sensor desconectado e não houver falha de GPS
			if ((psStatus->bSensorDesconectado == false) && (psStatus->bAlarmeGPS == false))
			{
				//Limpa variável de Alarme
				bAlarmes = false;
			}

			//Limpa as falhas:
			psStatus->dSementeZero = 0x00000000;
			psStatus->dSementeZeroIHM = 0x00000000;
			psStatus->dSementeFalha = 0x00000000;
			psStatus->dSementeFalhaIHM = 0x00000000;
			psStatus->dAduboFalha = 0x00000000;

			psStatus->dSementeZeroExt = 0x00000000;
			psStatus->dSementeZeroIHMExt = 0x00000000;
			psStatus->dSementeFalhaExt = 0x00000000;
			psStatus->dSementeFalhaIHMExt = 0x00000000;
			psStatus->dAduboFalhaExt = 0x00000000;

			psStatus->bLinhasFalha = false;
			psStatus->bLinhasZero = false;
		}

		if (psStatus->bAlarmeLinhasLevantadas > 0)
		{
			uint8_t bLinhasLevantadas = psMonitor->bNumLinhas - psStatus->bNumLinhasAtivas;
			if (psStatus->bAlarmeLinhasLevantadas >= ((bLinhasLevantadas + 1) >> 1))
			{
				psStatus->dSementeZeroIHM |= psStatus->dLinhasLevantadas;
				psStatus->dSementeZeroIHMExt |= psStatus->dLinhasLevantadasExt;

				AQR_wAlarmes |= AQR_FALHA_LINHA;
				bAlarmes = true;

				//Indica no registro que há falha em 1 ou mais linhas
				AQR_bStatusReg |= AQR_FALHA_LINHA;
			}
		}

		//Se não estiver trabalhando,
		//Se não houver sensor desconectado,
		//Se não houver alarme de GPS, para de tocar o alarme
		if ((psStatus->bTrabalhando == false) && (psStatus->bSensorDesconectado == false)
			&& (psStatus->bAlarmeGPS == false))
		{
			//Limpa variável de Alarme
			bAlarmes = false;
		}

		// Se estiver em excesso de velocidade
		if (psStatus->bExVel != false)
		{
			//Liga o flag de alarme
			AQR_wAlarmes |= AQR_EXC_VELOCIDADE;
			bAlarmes = true;

			//Indica no registro excesso de velocidade
			AQR_bStatusReg |= AQR_EXC_VELOCIDADE;
		}

		//Se ainda não está em modo trabalho
		//verifica falhas de instalação
		if ((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) == 0)
		{
			//Verifica o status da instalação do último sensor
			switch (psStatus->eStatusInstalacao)
			{
				case Instalado: //Se um novo sensor foi instalado com sucesso
				{
					psStatus->eStatusInstalacao = AguardandoEvento;

					//aciona aviso sonoro
					AQR_wAlarmes = AQR_NOVO_SENSOR;

					bAlarmes = true;
					break;
				}
				case FalhaAutoTeste: //Se houve falha na instalacao
				case SensorNaoEsperado: //Se foi encontrado mais sensores do que o nº de linhas configuradas
				{
					psStatus->eStatusInstalacao = AguardandoEvento;

					//aciona aviso sonoro
					AQR_wAlarmes = AQR_FALHA_INSTALACAO;

					bAlarmes = true;
					break;
				}

				default:
					break;
			}
		}

		//Se tem alarme...
		if (bAlarmes != false)
		{
			//Limpa variável que indica alarme
			bAlarmes = false;

			//Se houve alguma destas falhas, então emite um alarme contínuo.
			if ((AQR_wAlarmes & ( AQR_FALHA_LINHA | AQR_SENSOR_DESCONECTADO |
				AQR_EXC_VELOCIDADE | AQR_NOVO_SENSOR | AQR_FALHA_INSTALACAO |
				AQR_FALHA_GPS)) > 0)
			{
				//Se algum está ativo, aciona o alarme:
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_ALARME);
				osFlagSet(xAQR_sFlagSis, AQR_SIS_FLAG_ALARME);
			}

			//Se houve apenas falha na tolerância do sensor, então emite 2 beeps.
			if (((AQR_wAlarmes & AQR_FALHA_LINHA) == 0) && ((AQR_wAlarmes & AQR_FALHA_TOLERANCIA_LINHA) > 0))
			{
				//Aciona o alarme de linha abaixo da tolerância
				osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_ALARME_TOLERANCIA);
				osFlagSet(xAQR_sFlagSis, AQR_SIS_FLAG_ALARME_TOLERANCIA);
			}
		}
		else
		{
			if ((dFlagsSis & UOS_SIS_FLAG_ALARME) > 0)
			{
				//Este flag deve ser reconhecido aqui:
				osFlagClear(UOS_sFlagSis, UOS_SIS_FLAG_ALARME);
				osFlagClear(xAQR_sFlagSis, AQR_SIS_FLAG_ALARME);
			}
		}

		// devolve mutex
		status = RELEASE_MUTEX(AQR_MTX_sEntradas);
		ASSERT(status == osOK);
	}
	osThreadTerminate(NULL);
}
#else
void AQR_vAcquiregManagementThread(void const *argument)
{
}
#endif
