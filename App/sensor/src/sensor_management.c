/****************************************************************************
 * Title                 :   sensor_management
 * Filename              :   sensor_management.c
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
 *  21/10/16   1.0.0     Henrique Reis         sensor_management.c created.
 *
 *****************************************************************************/
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "sensor_core.h"
#include "sensor_management.h"
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

/******************************************************************************
 * Public Variables
 *******************************************************************************/
//Flags para indicar eventos da aplicação do protocolo:
//OS_FLAG_GRP             *CAN_psFlagApl;
//Estrutura da lista de dispositivos na rede CAN
CAN_tsCtrlListaSens CAN_sCtrlLista;

//Estrutura de parametros de sensores
CAN_tsParametrosSensor CAN_sParametrosSensor;

//Estrutura de parametros de sensores com algorítimo para correção de duplos
CAN_tsParametrosExtended CAN_sParametrosExtended;

//Mutex para acesso ao buffer da lista de sensores na rede CAN
CREATE_MUTEX(CAN_MTX_sBufferListaSensores);

//Mutex para acesso ao arquivo da lista de sensores na rede CAN
//OS_EVENT                *CAN_MTX_sArquivoListaSensores;

extern osFlagsGroupId CAN_psFlagApl;

/******************************************************************************
 * Variables
 *******************************************************************************/
uint32_t dSensorSementeDescMem;
uint32_t dSensorSementeDescMemExt;
uint32_t dSensorAduboDescMem;
uint32_t dSensorAduboDescMemExt;
uint32_t dSensorAdicionalDescMem;
uint32_t CAN_dLeituraSensor;
uint8_t CAN_bSensorSimulador;

uint8_t bDesliga;

//Contador de sensores novos
uint8_t CAN_bNumRespostasPNP = 0;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static CAN_teEstadoSensor SEN_vSearchSensorsInQueue (CAN_tsIDAuteq* psIDAuteq);
static bool SEN_bUpdateConnectedSensors (void);
static void SEN_vVerifyUnconnectedSensors (void);
static void SEN_vSendCommand (uint8_t bComando,
	uint8_t bLinha,
	uint8_t bTipo,
	uint8_t *pbDados,
	uint8_t bDLC);
static void SEN_vConfigSensor (uint8_t bLinha,
	uint8_t bTipo,
	uint8_t *pbEndFisico,
	uint8_t bDLC);
static void SEN_vDetectSensors (void);

/*****************************************************************************
 static CAN_teEstadoSensor SEN_vSearchSensorsInQueue( CAN_tsIDAuteq* psIDAuteq )

 Descrição :
 Parâmetros:
 Retorno   : nenhum

 ******************************************************************************/
static CAN_teEstadoSensor SEN_vSearchSensorsInQueue (CAN_tsIDAuteq* psIDAuteq)
{
	uint8_t bSensorSementeAdubo;
	uint8_t bNumTotalLinhas;
	uint8_t bLoopLinha;
	uint8_t bPosicaoB;
	uint8_t bOffset;
	uint32_t dErr;
	canMSGStruct_s psMessage;
	CAN_teEstadoSensor eEstado = Verificando;
	CAN_tsLista* psSensor;
	CAN_tsCtrlApl* psCtrlApl;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Mensagem recebida
	psMessage = CAN_sCtrlMPA.sMensagemRecebida;

	//Caso o sensor detectado seja digital, o ponteiro de sensor é ajustado para
	//somente referenciar registros deste tipo de sensor
	if ((psIDAuteq->sID_bits.bTipo != CAN_APL_SENSOR_SEMENTE) &&
		(psIDAuteq->sID_bits.bTipo != CAN_APL_SENSOR_ADUBO))
	{
		//Ponteiro de trabalho da lista
		psSensor = CAN_sCtrlLista.asLista + CAN_bNUM_SENSORES_SEMENTE_E_ADUBO;

		//Número total de linhas a serem pesquisadas na lista
		bNumTotalLinhas = CAN_bNUM_SENSORES_DIGITAIS;

		//Offset para procura na lista de sensores digitais
		bOffset = 1;

		//Indica que é sensor digital
		bSensorSementeAdubo = false;
	}
	else
	{
		//Ponteiro de trabalho da lista
		psSensor = CAN_sCtrlLista.asLista + psIDAuteq->sID_bits.bTipo;

		//Número total de linhas a serem pesquisadas na lista
		//Obs: A divisão é feita por 2 porque há sensores de sementes e de adubo
		//em cada linha
		bNumTotalLinhas = ( CAN_bNUM_SENSORES_SEMENTE_E_ADUBO >> 1);

		//Offset para procura na lista
		bOffset = CAN_bNUM_SENSORES_POR_LINHA;

		//Indica que é um sensor de semente ou de adubo
		bSensorSementeAdubo = true;
	}

	//Procura por um registro na lista que contenha o endereço físico da mensagem
	for (bLoopLinha = 0; (eEstado == Verificando) && (bLoopLinha < bNumTotalLinhas); bLoopLinha++)
	{
		switch (psSensor->eEstado)
		{
			case Conectado:
			//Verifica se o sensor já está conectado à rede CAN
			if (!memcmp(psMessage.data,
				psSensor->abEnderecoFisico,
				sizeof(psSensor->abEnderecoFisico)))
			{
				//Testa que tipo de sensor está sendo procurado
				if (bSensorSementeAdubo) //Sensor de semente ou de adubo
				{
					//Verifica se há coerência de linha
					if (bLoopLinha == psIDAuteq->sID_bits.bLinha)
					{
						//Indica que o sensor está conectado
						eEstado = Conectado;

						//Informa resultado do auto-teste dos sensores
						psSensor->eResultadoAutoTeste = (CAN_teAutoTeste)psMessage.data[7];
					}
				}
				else //Sensor digital
				{
					//Para sensor adicional devem ser testados o seu tipo e endereço físico
					if ((bLoopLinha + CAN_APL_SENSOR_SIMULADOR) == psIDAuteq->sID_bits.bTipo)
					{
						//Indica que o sensor está conectado
						eEstado = Conectado;
					}
				}
			}

				break;

			case Desconectado:
			//Verifica se o sensor já está conectado à rede CAN
			if (!memcmp(psMessage.data,
				psSensor->abEnderecoFisico,
				sizeof(psSensor->abEnderecoFisico)))
			{
				//Testa que tipo de sensor está sendo procurado
				if (bSensorSementeAdubo) //Sensor de semente ou de adubo
				{
					//Verifica se há coerência de linha
					if (bLoopLinha == psIDAuteq->sID_bits.bLinha)
					{
						//Indica que o sensor está reconectado
						eEstado = Conectado;

						//Atualiza o estado do sensor, reconectando-o
						psSensor->eEstado = Conectado;

						//Atualiza os flags que indicam quais sensores estão conectados
						//à rede CAN
						if (psIDAuteq->sID_bits.bTipo == CAN_APL_SENSOR_SEMENTE)
						{
							//Verifica se o loop é menor que 32, se não, usa os flags extendidos
							if (bLoopLinha < 32)
							{
								psCtrlApl->dSensSementeConectados |= 0x00000001 << bLoopLinha;
							}
							else
							{
								psCtrlApl->dSensSementeConectadosExt |= 0x00000001 << (bLoopLinha - 32);
							}
						}
						else
						{
							//Verifica se o loop é menor que 32, se não, usa os flags extendidos
							if (bLoopLinha < 32)
							{
								psCtrlApl->dSensAduboConectados |= 0x00000001 << bLoopLinha;
							}
							else
							{
								psCtrlApl->dSensAduboConectadosExt |= 0x00000001 << (bLoopLinha - 32);
							}
						}

						//Informa resultado do auto-teste dos sensores
						psSensor->eResultadoAutoTeste = (CAN_teAutoTeste)psMessage.data[7];

						osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_DET_SENSOR_RECONECTADO);
					}
				}
				else //Sensor digital
				{
					//Para sensor adicional devem ser testados o seu tipo e endereço físico
					if ((bLoopLinha + CAN_APL_SENSOR_SIMULADOR) == psIDAuteq->sID_bits.bTipo)
					{
						//Indica que o sensor está reconectado
						eEstado = Conectado;

						//Atualiza o estado do sensor, reconectando-o
						psSensor->eEstado = Conectado;

						//Atualiza os flags que indicam quais sensores estão conectados
						//à rede CAN
						psCtrlApl->dSensDigitalConectados |= 0x00000001 << (bLoopLinha);

						//Informa resultado do auto-teste dos sensores
						psSensor->eResultadoAutoTeste = (CAN_teAutoTeste)psMessage.data[7];

						osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_DET_SENSOR_RECONECTADO);
					}
				}
			}

				break;

			case Novo: //Registro vazio

			eEstado = Verificando;
				break;

			default:

			eEstado = Verificando;
				break;
		}

		//Atualiza o ponteiro de sensores
		psSensor += bOffset;

	} //Fim do for(...)

	//Verifica se o sensor já está conectado à rede. Em caso negativo é indicada
	//a sua presença.
	if ((eEstado == Verificando) && (bLoopLinha == bNumTotalLinhas))
	{
		//Armazena os dados do novo sensor
		CAN_sCtrlLista.sNovoSensor.bNovo = true;
		CAN_sCtrlLista.sNovoSensor.bTipoSensor = psIDAuteq->sID_bits.bTipo;

		for (bPosicaoB = 0; (eEstado == Verificando) && (bPosicaoB < CAN_bNUM_SENSORES_SEMENTE_E_ADUBO); bPosicaoB++)
		{
			if (memcmp(CAN_sCtrlLista.asLista[bPosicaoB].abEnderecoFisico,
				psMessage.data,
				sizeof(CAN_sCtrlLista.asLista[bPosicaoB].abEnderecoFisico)))
			{
				//Copia o endereço recebido na mensagem, para a estrutura de novo sensor
				memcpy(CAN_sCtrlLista.sNovoSensor.abEnderecoFisico,
					psMessage.data,
					sizeof(CAN_sCtrlLista.sNovoSensor.abEnderecoFisico));

				//Informa resultado do auto-teste dos sensores
                CAN_sCtrlLista.sNovoSensor.eResultadoAutoTeste = ( CAN_teAutoTeste ) ( psMessage.data[1] >> 24 );
				CAN_sCtrlLista.sNovoSensor.eResultadoAutoTeste = (CAN_teAutoTeste)psMessage.data[7];
				CAN_sCtrlLista.sNovoSensor.eResultadoAutoTeste = (CAN_teAutoTeste)(psMessage.data[1] >> 24);

				osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_DET_NOVO_SENSOR);

				//Atribui o estado de Novo sensor
				eEstado = Novo;
			}
			else
			{
				eEstado = Conectado; //VERIFICAR
			}
		}
	}
	return (eEstado);
}

/*****************************************************************************

 static bool SEN_bUpdateConnectedSensors( void )

 Descrição : Verifica se há algum sensor conectado na rede via lista de
 sensores
 Parâmetros: nenhum
 Retorno   : TRUE  - Há algum sensor conectado na lista de sensores
 FALSE - Nenhum sensor conectado na lista de sensores

 ******************************************************************************/

static bool SEN_bUpdateConnectedSensors (void)
{
	osStatus status;
	bool fAlgumSensorConectado;
	uint8_t bCont, bNumSensSementeAdubo, bNumTotalSensores;
	CAN_tsCtrlApl *psCtrlApl;
	CAN_tsLista *psLista;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Ponteiro de trabalho da lista
	psLista = CAN_sCtrlLista.asLista;

	//Limpa os indicadores dos sensores conectados na rede
	psCtrlApl->dSensSementeConectados = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dSensAduboConectados = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dSensSementeConectadosExt = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dSensAduboConectadosExt = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dSensDigitalConectados = CAN_APL_FLAG_NENHUM;

	//Número de sensores de sementes e de adubo
	bNumSensSementeAdubo = CAN_bNUM_DE_LINHAS * CAN_bNUM_SENSORES_POR_LINHA;

	//Número total de sensores na rede CAN
	bNumTotalSensores = ARRAY_SIZE(CAN_sCtrlLista.asLista);

	fAlgumSensorConectado = false;

	//Mutex para acesso exclusivo ao buffer da lista de dispositivos na rede CAN
	status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
	ASSERT(status == osOK);

	//Varre a lista de sensores para localizar aqueles que estão conectados
	for (bCont = 0; bCont < bNumTotalSensores; bCont++)
	{
		//Atualiza os flags de leitura de dados referentes a cada sensor conectado
		//à rede CAN
		if (psLista->eEstado == Conectado)
		{
			//Se for um sensor de semente ou de adubo...
			if (bCont < bNumSensSementeAdubo)
			{
				if (bCont % 2) //Sensor de adubo
				{
					//Verifica se o loop é menor que 64, se não, usa os flags extendidos
					if (bCont < 64)
					{
						psCtrlApl->dSensAduboConectados |= 0x00000001 << (bCont / 2);
					}
					else
					{
						psCtrlApl->dSensAduboConectadosExt |= 0x00000001 << ((bCont - 64) / 2);
					}
				}
				else //Sensor de sementes
				{
					//Verifica se o loop é menor que 64, se não, usa os flags extendidos
					if (bCont < 64)
					{
						psCtrlApl->dSensSementeConectados |= 0x00000001 << (bCont / 2);
					}
					else
					{
						psCtrlApl->dSensSementeConectadosExt |= 0x00000001 << ((bCont - 64) / 2);
					}
				}
			}
			else //Caso seja um sensor digital...
			{
				psCtrlApl->dSensDigitalConectados |= 0x00000001 << (bCont - bNumSensSementeAdubo);
			}

			fAlgumSensorConectado = true;
		}
		//Aponta para o próximo sensor na lista
		psLista++;
	}

	//Libera Mutex para acesso exclusivo ao buffer da lista de dispositivos na rede CAN
	status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
	ASSERT(status == osOK);

	return (fAlgumSensorConectado);
}

/*****************************************************************************

 static void SEN_vVerifyUnconnectedSensors( void )

 Descrição : Atualiza a lista de sensores indicando aqueles que foram
 desconectados da rede
 Parâmetros: nenhum
 Retorno   : nenhum

 ******************************************************************************/

static void SEN_vVerifyUnconnectedSensors (void)
{
	uint8_t bCont;
	uint32_t dLinhasNaoResponderam, dMascara;
	uint32_t dLinhasNaoResponderamExt, dMascaraExt;
	CAN_tsCtrlApl *psCtrlApl;
	CAN_tsLista *psSensor;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Ponteiro de trabalho da lista de sensores
	psSensor = CAN_sCtrlLista.asLista;

	//----------------------------------------------------------------------------
	//Inicializa a máscara de bits
	dMascara = 0x00000001;
	dMascaraExt = 0x00000001;

	//Verifica se todos os sensores de semente responderam
	if ((!psCtrlApl->bTodosSensSementeRsp) || (!psCtrlApl->bTodosSensSementeRspExt))
	{
		//Linha(s) cujo sensor(es) não respondeu(ram)
		dLinhasNaoResponderam = psCtrlApl->dSensSementeConectados ^ psCtrlApl->dRespostaSemente;
		dLinhasNaoResponderamExt = psCtrlApl->dSensSementeConectadosExt ^ psCtrlApl->dRespostaSementeExt;

		//Verifica se sensor(es) permanece(m) sem resposta
		dSensorSementeDescMem &= dLinhasNaoResponderam;
		dSensorSementeDescMemExt &= dLinhasNaoResponderamExt;

		//Varre as 36 linhas de sensores (32 bits da variável)
		for (bCont = 0; bCont < 36; bCont++)
		{
			//Verifica se o loop é menor que 32, se não, usa os flags extendidos
			if (bCont < 32)
			{
				//Testa a linha do sensor
				if ((dSensorSementeDescMem & dMascara))
				{
					//Desconecta o sensor
					psSensor[bCont * CAN_bNUM_SENSORES_POR_LINHA + CAN_APL_SENSOR_SEMENTE].eEstado = Desconectado;
				}
				//Atualiza a máscara
				dMascara <<= 1;
			}
			else
			{
				//Testa a linha do sensor
				if ((dSensorSementeDescMemExt & dMascaraExt))
				{
					//Desconecta o sensor
					psSensor[bCont * CAN_bNUM_SENSORES_POR_LINHA + CAN_APL_SENSOR_SEMENTE].eEstado = Desconectado;
				}
				//Atualiza a máscara
				dMascaraExt <<= 1;
			}
		}
		//Memoriza linha(s) cujo sensor(es) não respondeu(ram)
		dSensorSementeDescMem = dLinhasNaoResponderam;
		dSensorSementeDescMemExt = dLinhasNaoResponderamExt;
	}

	//----------------------------------------------------------------------------
	//Inicializa a máscara de bits
	dMascara = 0x00000001;
	dMascaraExt = 0x00000001;

	//Verifica se todos os sensores de adubo responderam
	if ((!psCtrlApl->bTodosSensAduboRsp) || (!psCtrlApl->bTodosSensAduboRspExt))
	{
		//Linha(s) cujo sensor(es) não respondeu(ram)
		dLinhasNaoResponderam = psCtrlApl->dSensAduboConectados ^ psCtrlApl->dRespostaAdubo;
		dLinhasNaoResponderamExt = psCtrlApl->dSensAduboConectadosExt ^ psCtrlApl->dRespostaAduboExt;
		dSensorAduboDescMem &= dLinhasNaoResponderam;
		dSensorAduboDescMemExt &= dLinhasNaoResponderamExt;

		//Varre as 36 linhas de sensores (2 x 32 bits da variável)
		for (bCont = 0; bCont < 36; bCont++)
		{
			//Verifica se o loop é menor que 32, se não, usa os flags extendidos
			if (bCont < 32)
			{
				//Testa a linha do sensor
				if (dSensorAduboDescMem & dMascara)
				{
					//Desconecta o sensor
					psSensor[bCont * CAN_bNUM_SENSORES_POR_LINHA + CAN_APL_SENSOR_ADUBO].eEstado = Desconectado;
				}

				//Atualiza a máscara
				dMascara <<= 1;
			}
			else
			{
				//Testa a linha do sensor
				if (dSensorAduboDescMemExt & dMascaraExt)
				{
					//Desconecta o sensor
					psSensor[bCont * CAN_bNUM_SENSORES_POR_LINHA + CAN_APL_SENSOR_ADUBO].eEstado = Desconectado;
				}

				//Atualiza a máscara
				dMascaraExt <<= 1;
			}
		}
		dSensorAduboDescMem = dLinhasNaoResponderam;
		dSensorAduboDescMemExt = dLinhasNaoResponderamExt;
	}

	//----------------------------------------------------------------------------
	//Inicializa a máscara de bits
	dMascara = 0x00000001;

	//Verifica se todos os sensores digitais responderam
	if (!psCtrlApl->bTodosSensDigitalRsp)
	{
		//Linha(s) cujo sensor(es) não respondeu(ram)
		dLinhasNaoResponderam = psCtrlApl->dSensDigitalConectados ^ psCtrlApl->dRespostaDigital;
		dSensorAdicionalDescMem &= dLinhasNaoResponderam;

		//Varre os 6 sensores aux.
		for (bCont = 0; bCont < 6; bCont++)
		{
			//Testa a linha do sensor
			if (dSensorAdicionalDescMem & dMascara)
			{
				//Desconecta o sensor
				psSensor[CAN_bNUM_SENSORES_SEMENTE_E_ADUBO + bCont].eEstado = Desconectado;
			}
			//Atualiza a máscara
			dMascara <<= 1;
		}
		dSensorAdicionalDescMem = dLinhasNaoResponderam;
	}
	//Atualiza o array de sensores conectados
	SEN_bUpdateConnectedSensors();
}

/*****************************************************************************

 static void SEN_vSendCommand( uint8_t bComando,
 uint8_t bLinha,
 uint8_t bTipo,
 uint8_t *pbDados,
 uint8_t bDLC )

 Descrição : Envia um comando para a rede de sensores, ou para um sensor em
 específico
 Parâmetros: bComando - Comando a ser enviado
 bLinha   - Linha da plantadeira na qual o sensor está instalado
 bTipo    - Tipo do sensor (semente, adubo ou digital)
 pbDados  - Ponteiro para o buffer de dados a ser inserido no
 campo de dados da mensagem CAN
 bDLC     - Quantidade de bytes a serem inseridos no campo de
 dados da mensagem CAN
 Retorno   : nenhum

 Comando PnP enviado:

 Remote Frame, RTR = 1
 Linha:                0xFF
 Pacote atual:         1
 Total de pacotes:     1
 Tipo de dados:        0
 Tipo de dispositivo:  0
 Dados:                nenhum

 ******************************************************************************/

static void SEN_vSendCommand (uint8_t bComando,
	uint8_t bLinha,
	uint8_t bTipo,
	uint8_t *pbDados,
	uint8_t bDLC)
{
	static canMSGStruct_s pSensorMsg;         // Holds the actual message
	CAN_tsIDAuteq sIDAuteq;

	//Limpa a estrutura do ID da Auteq
	sIDAuteq.dID = 0UL;

	//Atualiza o registro do último comando enviado
	CAN_sCtrlMPA.sCtrlApl.bUltimoCmdEnviado = bComando;

	//Preenchimento do identificador Auteq
	sIDAuteq.sID_bits.bLinha = bLinha;        //ID.7  - ID.0
	sIDAuteq.sID_bits.bPacoteAtual = 1;             //ID.11 - ID.8
	sIDAuteq.sID_bits.bTotalPacotes = 1;             //ID.15 - ID.12
	sIDAuteq.sID_bits.bComando = bComando;      //ID.22 - ID.16
	sIDAuteq.sID_bits.bTipo = bTipo;         //ID.28 - ID.23

	//Máscara para serem obtidos os 29 bits do campo "Identifier"
	sIDAuteq.dID &= 0x1FFFFFFF;

	//Armazena a mensagem no buffer se transmissão
	pSensorMsg.dlc = bDLC;
	pSensorMsg.id = (bDLC) ? CAN_DATA_EXTENDED_ID(sIDAuteq.dID) : CAN_RTR_EXTENDED_ID(sIDAuteq.dID);

	if (bDLC > 0)
	{
		memcpy(pSensorMsg.data, pbDados, bDLC);
	}

	PUT_LOCAL_QUEUE(SensorWriteQ, pSensorMsg, osWaitForever);
}

/*****************************************************************************

 static void SEN_vConfigSensor( uint8_t bLinha,
 uint8_t bTipo,
 uint8_t *pbEndFisico,
 uint8_t bDLC )

 Descrição :
 Parâmetros:
 Retorno   : nenhum

 Após esta função a aplicação aguarda so seguintes flags:

 CAN_APL_FLAG_TODOS_SENS_RESPONDERAM
 CAN_APL_FLAG_LEITURA_TIMEOUT

 ******************************************************************************/

static void SEN_vConfigSensor (uint8_t bLinha,
	uint8_t bTipo,
	uint8_t *pbEndFisico,
	uint8_t bDLC)
{
	osStatus status;
	CAN_tsCtrlApl *psCtrlApl;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Liga o timer de controle de timeout do comando de configuração de um
	//sensor conectado à rede CAN
	//    UOS_vReiniciaTimer( psCtrlApl->bTimerTimeoutConfigura );
	status = START_TIMER(psCtrlApl->wTimerTimeoutConfigura, psCtrlApl->dTicksTimeoutConfigura);
	ASSERT(status == osOK);

	//Envia comando de configuração no barramento CAN
	SEN_vSendCommand( CAN_APL_CMD_CONFIGURA_SENSOR,
		bLinha,
		bTipo,
		pbEndFisico,
		bDLC);
}

/*****************************************************************************

 void SEN_vAddNewSensor( const uint8_t bLinha )

 Descrição : Adiciona um novo sensor, cujo tipo e linha na qual será inserido
 são passados como argumentos da função.
 Parâmetros: bLinha - Linha da plantadeira na qual o novo sensor deve ser
 instalado
 Retorno   : nenhum

 ******************************************************************************/

void SEN_vAddNewSensor (const uint8_t bLinha)
{
	//Verifica se há realmente um novo sensor na rede
	if (CAN_sCtrlLista.sNovoSensor.bNovo != false)
	{
		//Envia comando de configuração de sensor
		SEN_vConfigSensor(bLinha,
			CAN_sCtrlLista.sNovoSensor.bTipoSensor,
			CAN_sCtrlLista.sNovoSensor.abEnderecoFisico,
			CAN_bTAMANHO_END_FISICO);

		//O recebimento da resposta do sensor a este comando faz com este seja
		//adicionado à lista de sensores.
	}
}

/*****************************************************************************

 void SEN_vReadDataFromSensors( void )

 Descrição : Envia comando de leitura de dados dos sensores instalados na rede
 Parâmetros: nenhum
 Retorno   : nenhum

 ******************************************************************************/

void SEN_vReadDataFromSensors (void)
{
	osStatus status;
	CAN_tsCtrlApl *psCtrlApl;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Limpa os flags de resposta ao comando de leitura de dados dos sensores
	psCtrlApl->dRespostaSemente = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaAdubo = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaSementeExt = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaAduboExt = CAN_APL_FLAG_NENHUM;

	//Liga o timer de controle de timeout do comando de leitura de dados dos
	//sensores conectados à rede CAN
	status = START_TIMER(psCtrlApl->wTimerTimeoutComando, psCtrlApl->dTicksTimeoutComando);
	ASSERT(status == osOK);

	//Indica que o timer de timeout deste comando deve ser reiniciado
	psCtrlApl->bReiniciaTimeout = true;

	//Envia comando de leitura de dados dos sensores
	SEN_vSendCommand( CAN_APL_CMD_LEITURA_DADOS,
	CAN_APL_LINHA_TODAS,
	CAN_APL_SENSOR_TODOS,
		(void*) NULL,
		CAN_MSG_DLC_0);
}

/*****************************************************************************

 void SEN_vSensorsParameters( uint8_t bComando,
 uint8_t bLinha,
 uint8_t bTipo,
 uint8_t *pbDados,
 uint8_t bDLC )

 Descrição : Envia comando com parâmetro de sensores, atualizando a lista
 de sensores armazenada em memória RAM
 Parâmetros: bLinha - Número da linha
 bTipo - Tipo de Sensor
 pbDados - Buffer de dados a serem enviados
 bDLC  - Tamanho de dados a serem enviados

 Retorno   : nenhum

 ******************************************************************************/

void SEN_vSensorsParameters (uint8_t bComando,
	uint8_t bLinha,
	uint8_t bTipo,
	uint8_t *pbDados,
	uint8_t bDLC)
{
	CAN_tsCtrlApl *psCtrlApl;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Limpa os flags de resposta ao comando de detecção dos sensores
	psCtrlApl->dResSemParametros = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResAduParametros = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResDigParametros = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResSemParametrosExt = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResAduParametrosExt = CAN_APL_FLAG_NENHUM;

	//Zera os flags que indicam se os sensores responderam ao comando de PnP
	psCtrlApl->bTodosSensSemRspParametros = false;
	psCtrlApl->bTodosSensAduRspParametros = false;
	psCtrlApl->bTodosSensSemRspParametrosExt = false;
	psCtrlApl->bTodosSensAduRspParametrosExt = false;
	psCtrlApl->bTodosSensDigRspParametros = false;

	//Envia comando de PnP no barramento CAN
	SEN_vSendCommand(bComando,
		bLinha,
		bTipo,
		pbDados,
		bDLC);
}

/*****************************************************************************

 void SEN_vGetVersion( void )

 Descrição : Envia comando de Solicitação de Versão de Software dos Sensores
 Parâmetros: nenhum
 Retorno   : nenhum

 ******************************************************************************/

void SEN_vGetVersion (void)
{
	CAN_tsCtrlApl *psCtrlApl;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Limpa os flags de resposta ao comando de Solicitação de Versão
	psCtrlApl->dResSemVersaoSW = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResAduVersaoSW = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResDigVersaoSW = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResSemVersaoSWExt = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dResAduVersaoSWExt = CAN_APL_FLAG_NENHUM;

	//Zera os flags que indicam se os sensores responderam ao comando de Solicitação de Versão
	psCtrlApl->bTodosSensSemRspVersao = false;
	psCtrlApl->bTodosSensAduRspVersao = false;
	psCtrlApl->bTodosSensSemRspVersaoExt = false;
	psCtrlApl->bTodosSensAduRspVersaoExt = false;
	psCtrlApl->bTodosSensDigRspVersao = false;

	//Envia comando de Solicitação de Versão no barramento CAN
	SEN_vSendCommand( CAN_APL_CMD_VERSAO_SW_SENSOR,
	CAN_APL_LINHA_TODAS,
	CAN_APL_SENSOR_TODOS,
		(void*) NULL,
		CAN_MSG_DLC_0);
}

/*****************************************************************************

 static void SEN_vDetectSensors( void )

 Descrição : Envia comando de detecção de sensores na rede, atualizando a lista
 de sensores armazenada em memória RAM
 Parâmetros: nenhum
 Retorno   : nenhum

 ******************************************************************************/

static void SEN_vDetectSensors (void)
{
	CAN_tsCtrlApl *psCtrlApl;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	//Limpa os flags de resposta ao comando de detecção dos sensores
	psCtrlApl->dRespostaSemente = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaAdubo = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaDigital = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaSementeExt = CAN_APL_FLAG_NENHUM;
	psCtrlApl->dRespostaAduboExt = CAN_APL_FLAG_NENHUM;

	//Zera os flags que indicam se os sensores responderam ao comando de PnP
	psCtrlApl->bTodosSensSementeRsp = false;
	psCtrlApl->bTodosSensSementeRspExt = false;
	psCtrlApl->bTodosSensAduboRsp = false;
	psCtrlApl->bTodosSensAduboRspExt = false;
	psCtrlApl->bTodosSensDigitalRsp = false;

	//Limpa contador de respostas ao comando PNP
	CAN_bNumRespostasPNP = 0;

	//Verifica se deve ajustar o timer de intervalo dos comandos PNP,
	//o primeiro comando tem um intervalo maior para dar tempo dos
	//sensores executarem a inicialização e o auto-teste.
	if (psCtrlApl->dTicksCmdPnP != CAN_wTICKS_CMD_PNP)
	{
		psCtrlApl->dTicksCmdPnP = CAN_wTICKS_CMD_PNP;
	}

	//Envia comando de PnP no barramento CAN
	SEN_vSendCommand( CAN_APL_CMD_PNP,
	CAN_APL_LINHA_TODAS,
	CAN_APL_SENSOR_TODOS,
		(void*) NULL,
		CAN_MSG_DLC_0);
}

/******************************************************************************


 static void SEN_vManagementNetwork( void )

 Descrição : Identifica o evento ocorrido na camada de aplicação CAN
 Parâmetros: nenhum
 Retorno   : nenhum

 ******************************************************************************/
void SEN_vManagementNetwork (void)
{
	osStatus status;
	CAN_tsCtrlApl *psCtrlApl;
	CAN_tsCtrlEnl *psCtrlEnl;
	CAN_tsIDAuteq sIDAuteq;
	CAN_tsLista *psSensor;
	osFlags dFlagsSis;
	canMSGStruct_s psMessage;
	CAN_teEstadoSensor eEstado;
	uint32_t wLastSig;

	//Ponteiro de trabalho da aplicação CAN
	psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;

	psCtrlEnl = &CAN_sCtrlMPA.sCtrlEnl;

	//O ID da mensagem recebida é armazenado, sendo filtrados somente 29 bits
	//do campo "identifier".
	sIDAuteq.dID = 0x1FFFFFFF & CAN_sCtrlMPA.sMensagemRecebida.id;

	//Mensagem recebida
	psMessage = CAN_sCtrlMPA.sMensagemRecebida;

	//Ponteiro de trabalho do sensor que enviou a mensagem
	psSensor = CAN_sCtrlLista.asLista + //Posição inicial da lista
		(sIDAuteq.sID_bits.bLinha * CAN_bNUM_SENSORES_POR_LINHA) + //Posição da linha desejada
		sIDAuteq.sID_bits.bTipo;                                     //Posição do sensor

	if (sIDAuteq.sID_bits.bTipo >= CAN_APL_SENSOR_SIMULADOR)
	{
		psSensor -= sIDAuteq.sID_bits.bTipo;
	}

	//Verifica qual evento foi responsável por fazer esta tarefa ser executada

	//----------------------------------------------------------------------------
	//Testa o evento "Comando de detecção de sensores"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_TMR_CMD_PNP)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_TMR_CMD_PNP);

		//Se está em modo tarbalho
		//Se finalizou a instalação

		dFlagsSis = osFlagGet(UOS_sFlagSis);

		if (((dFlagsSis & UOS_SIS_FLAG_MODO_TRABALHO) > 0) ||
			((dFlagsSis & UOS_SIS_FLAG_MODO_TESTE) > 0))
		{
			if ((bDesliga < 2) && (UOS_sConfiguracao.sMonitor.bMonitorArea == true))
			{
				bDesliga++;

				//Envia comando de detecção de sensores na rede CAN
				SEN_vDetectSensors();
			}
			else
			{
				status = STOP_TIMER(psCtrlApl->wTimerCmdPnP);
				ASSERT(status == osOK);
			}
		}
		else
		{
			//Verifica de quais sensores devem ser esperadas respostas a qualquer
			//comando e se há algum sensor na rede CAN
			if (SEN_bUpdateConnectedSensors())
			{
				//Liga o timer de controle de timeout do comando de PnP dos sensores
				//conectados à rede CAN
				status = START_TIMER(psCtrlApl->wTimerTimeoutPnP, psCtrlApl->dTicksTimeoutPnP);
				ASSERT(status == osOK);

				//Indica que o timer de timeout do comando de detecção deve ser reiniciado
				psCtrlApl->bReiniciaTimeout = true;
			}
			//Envia comando de detecção de sensores na rede CAN
			SEN_vDetectSensors();
		}
	}

	//----------------------------------------------------------------------------
	//Testa o evento "Resposta ao comando de PnP"

	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_PNP)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_MSG_RESP_PNP);

		//Verifica se deve ser computado o timeout do comando de detecção de sensores
		if (psCtrlApl->bReiniciaTimeout != false)
		{
			//Reinicia o timer de controle de timeout do comando de detecção de sensores
			status = START_TIMER(psCtrlApl->wTimerTimeoutPnP, psCtrlApl->dTicksTimeoutPnP);
			ASSERT(status == osOK);
		}

		//Mutex para acesso exclusivo ao buffer da lista de dispositivos na rede CAN
		status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
		ASSERT(status == osOK);

		//Verifica se a reposta é originada por um novo sensor e atualiza a lista
		//de sensores na rede CAN
		eEstado = SEN_vSearchSensorsInQueue(&sIDAuteq);

		//Libera Mutex para acesso exclusivo à lista de dispositivos na rede CAN
		status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
		ASSERT(status == osOK);

		if (eEstado == Conectado)
		{
			//Verifica o tipo de sensor
			switch (sIDAuteq.sID_bits.bTipo)
			{
				case CAN_APL_SENSOR_SEMENTE: //Sensor de sementes
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (sIDAuteq.sID_bits.bLinha < 32)
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaSemente |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
					}
					else
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaSementeExt |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
					}
					break;
				}

				case CAN_APL_SENSOR_ADUBO:    //Sensor de adubo
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (sIDAuteq.sID_bits.bLinha < 32)
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaAdubo |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
					}
					else
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaAduboExt |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
					}

					break;
				}

				case CAN_APL_SENSOR_SIMULADOR:  //Sensor de Velocidade do Simulador
				CAN_bSensorSimulador = true;
				//no break
				case CAN_APL_SENSOR_DIGITAL_2:  //Sensor digital 2
				case CAN_APL_SENSOR_DIGITAL_3:  //Sensor digital 3
				case CAN_APL_SENSOR_DIGITAL_4:  //Sensor digital 4
				case CAN_APL_SENSOR_DIGITAL_5:  //Sensor digital 5
				case CAN_APL_SENSOR_DIGITAL_6:  //Sensor digital 6
				{
					//Indica que resposta ao comando de leitura de dados foi recebida
					psCtrlApl->dRespostaDigital |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - CAN_bNUM_DE_LINHAS);

					break;
				}

				default:
				;
			}

			//Verifica se todos os sensores de semente responderam ao comando de PnP
			psCtrlApl->bTodosSensSementeRsp = psCtrlApl->dRespostaSemente == psCtrlApl->dSensSementeConectados;
			//Verifica tambem os flags extendidos
			psCtrlApl->bTodosSensSementeRspExt = psCtrlApl->dRespostaSementeExt == psCtrlApl->dSensSementeConectadosExt;

			//Verifica se todos os sensores de adubo responderam ao comando de PnP
			psCtrlApl->bTodosSensAduboRsp = psCtrlApl->dRespostaAdubo == psCtrlApl->dSensAduboConectados;

			//Verifica tambem os flags extendidos
			psCtrlApl->bTodosSensAduboRspExt = psCtrlApl->dRespostaAduboExt == psCtrlApl->dSensAduboConectadosExt;

			//Verifica se todos os sensores digitais responderam ao comando de PnP
			psCtrlApl->bTodosSensDigitalRsp = psCtrlApl->dRespostaDigital == psCtrlApl->dSensDigitalConectados;

			//Testa se todos os sensores responderam ao comando de PnP
			if (psCtrlApl->bTodosSensSementeRsp &&
				psCtrlApl->bTodosSensSementeRspExt &&
				psCtrlApl->bTodosSensAduboRsp &&
				psCtrlApl->bTodosSensAduboRspExt &&
				psCtrlApl->bTodosSensDigitalRsp)
			{
				//Desliga o timer de timeout do comando de PnP dos sensores
				status = STOP_TIMER(psCtrlApl->wTimerTimeoutPnP);
				ASSERT(status == osOK);

				//Indica que o timeout do comando de detecção de sensores não deve ser
				//mais computado
				psCtrlApl->bReiniciaTimeout = false;

				dSensorSementeDescMem = 0x00000000;
				dSensorAdicionalDescMem = 0x00000000;
				dSensorAduboDescMem = 0x00000000;
				dSensorSementeDescMemExt = 0x00000000;
				dSensorAduboDescMemExt = 0x00000000;

				osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_TODOS_SENS_RESP_PNP);
			}
		}
		//Indica que a aplicação leu a mensagem recebida
		wLastSig = osSignalSet(*(psCtrlEnl->psFlagEnl), CAN_APL_FLAG_MENSAGEM_LIDA);
		ASSERT(wLastSig != 0x80000000);
	} //Fim do if ( psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_PNP )

	//----------------------------------------------------------------------------
	//Testa o evento "Mensagem de leitura de Velocidade"

	if ((psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_LEITURA_VELOCIDADE) > 0)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_MSG_LEITURA_VELOCIDADE);

		if (psSensor->eEstado == Desconectado)
		{
			//Reconecta o sensor da lista
			psSensor->eEstado = Conectado;
			SEN_bUpdateConnectedSensors();
		}

		if (psSensor->eEstado == Conectado)
		{

			memcpy(psSensor->abUltimaLeitura, &psMessage.data[0], sizeof(psSensor->abUltimaLeitura));

			//Verifica o tipo de sensor
			switch (sIDAuteq.sID_bits.bTipo)
			{
				case CAN_APL_SENSOR_SIMULADOR:  //Sensor de Velocidade do Simulador
				{
					uint32_t dAux;

					dAux = (uint32_t)psSensor->abUltimaLeitura[0];

					dAux |= (uint32_t)(psSensor->abUltimaLeitura[1] << 8);

					CAN_dLeituraSensor = dAux;

					//Indica que resposta ao comando de leitura de dados foi recebida
					psCtrlApl->dRespostaDigital |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - CAN_bNUM_DE_LINHAS);
					break;
				}
				default:
					break;
			}
			//Verifica se todos os sensores digitais responderam ao comando
			psCtrlApl->bTodosSensDigitalRsp = psCtrlApl->dRespostaDigital == psCtrlApl->dSensDigitalConectados;
		}
		//Indica que a aplicação leu a mensagem recebida
		wLastSig = osSignalSet(*(psCtrlEnl->psFlagEnl), CAN_APL_FLAG_MENSAGEM_LIDA);
		ASSERT(wLastSig != 0x80000000);
	} //Fim do if ( psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS )

	//----------------------------------------------------------------------------
	//Testa o evento "Resposta ao comando de leitura de dados dos sensores"

	if ((psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS) > 0)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS);

		//Verifica se deve ser computado o timeout do comando de leitura de dados
		if (psCtrlApl->bReiniciaTimeout != false)
		{
			//Reinicia o timer de controle de timeout de comando
			status = START_TIMER(psCtrlApl->wTimerTimeoutComando, CAN_wTICKS_TIMEOUT);
			ASSERT(status == osOK);
		}

		if (psSensor->eEstado == Desconectado)
		{
			//Reconecta o sensor da lista
			psSensor->eEstado = Conectado;
			SEN_bUpdateConnectedSensors();
		}

		if (psSensor->eEstado == Conectado)
		{

			memcpy(psSensor->abUltimaLeitura, &psMessage.data[0], sizeof(psSensor->abUltimaLeitura));

			//Verifica o tipo de sensor
			switch (sIDAuteq.sID_bits.bTipo)
			{
				case CAN_APL_SENSOR_SEMENTE: //Sensor de sementes
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (sIDAuteq.sID_bits.bLinha < 32)
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaSemente |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
					}
					else
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaSementeExt |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
					}
					break;
				}
				case CAN_APL_SENSOR_ADUBO:    //Sensor de adubo
				{
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (sIDAuteq.sID_bits.bLinha < 32)
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaAdubo |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
					}
					else
					{
						//Indica que resposta ao comando de leitura de dados foi recebida
						psCtrlApl->dRespostaAduboExt |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
					}
					break;
				}
				default:
					break;
			}

			//Verifica se todos os sensores de semente responderam ao comando
			psCtrlApl->bTodosSensSementeRsp = (psCtrlApl->dRespostaSemente == psCtrlApl->dSensSementeConectados);
			psCtrlApl->bTodosSensSementeRspExt = (psCtrlApl->dRespostaSementeExt == psCtrlApl->dSensSementeConectadosExt);

			//Verifica se todos os sensores de adubo responderam ao comando
			psCtrlApl->bTodosSensAduboRsp = (psCtrlApl->dRespostaAdubo == psCtrlApl->dSensAduboConectados);
			psCtrlApl->bTodosSensAduboRspExt = (psCtrlApl->dRespostaAduboExt == psCtrlApl->dSensAduboConectadosExt);

			//Verifica se todos os sensores digitais responderam ao comando
			psCtrlApl->bTodosSensDigitalRsp = (psCtrlApl->dRespostaDigital == psCtrlApl->dSensDigitalConectados);

			//Testa se todos os sensores responderam ao comando de PnP
			if (psCtrlApl->bTodosSensSementeRsp &&
				psCtrlApl->bTodosSensSementeRspExt &&
				psCtrlApl->bTodosSensAduboRsp &&
				psCtrlApl->bTodosSensAduboRspExt &&
				psCtrlApl->bTodosSensDigitalRsp)
			{

				dSensorSementeDescMem = 0x00000000;
				dSensorAdicionalDescMem = 0x00000000;
				dSensorAduboDescMem = 0x00000000;
				dSensorSementeDescMemExt = 0x00000000;
				dSensorAduboDescMemExt = 0x00000000;

				//Indica que o timeout do comando de detecção de sensores não deve ser
				//mais computado
				psCtrlApl->bReiniciaTimeout = false;

				//Desliga o timer de timeout do comando de leitura de dados dos
				//sensores
				status = STOP_TIMER(psCtrlApl->wTimerTimeoutComando);
				ASSERT(status == osOK);

				osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP);
			}
		}
		//Indica que a aplicação leu a mensagem recebida
		wLastSig = osSignalSet(*(psCtrlEnl->psFlagEnl), CAN_APL_FLAG_MENSAGEM_LIDA);
		ASSERT(wLastSig != 0x80000000);
	} //Fim do if ( psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS )

	//----------------------------------------------------------------------------

	//Testa o evento "Resposta ao comando de configuração do sensor"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_CONFIGURACAO)
	{
		CAN_tsLista *psSensor;

		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_MSG_RESP_CONFIGURACAO);

		//Desliga o timer de timeout do comando enviado
		status = STOP_TIMER(psCtrlApl->wTimerTimeoutConfigura);
		ASSERT(status == osOK);

		//Ponteiro de trabalho da lista
		psSensor = CAN_sCtrlLista.asLista +                   //Posição inicial da lista
			(sIDAuteq.sID_bits.bLinha * CAN_bNUM_SENSORES_POR_LINHA) + //Posição da linha desejada
			sIDAuteq.sID_bits.bTipo;                                     //Posição do sensor

		if (sIDAuteq.sID_bits.bTipo >= CAN_APL_SENSOR_SIMULADOR)
		{
			psSensor -= sIDAuteq.sID_bits.bTipo;
		}

		//Verifica se a mesagem recebida é realmente originária do novo sensor
		if (!memcmp(psMessage.data,
			CAN_sCtrlLista.sNovoSensor.abEnderecoFisico,
			sizeof(CAN_sCtrlLista.sNovoSensor.abEnderecoFisico)))
		{

			//Mutex para acesso exclusivo ao buffer da lista de dispositivos na rede CAN
			status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
			ASSERT(status == osOK);

			//Indica que o sensor foi inserido na lista de sensores
			CAN_sCtrlLista.sNovoSensor.bNovo = false;

			//Conecta o sensor à lista de sensores
			psSensor->eEstado = Conectado;

			//Copia o endereço do novo sensor para a posição da lista
			memcpy(psSensor->abEnderecoFisico,
				CAN_sCtrlLista.sNovoSensor.abEnderecoFisico,
				sizeof(CAN_sCtrlLista.sNovoSensor.abEnderecoFisico));

			psSensor->eResultadoAutoTeste = CAN_sCtrlLista.sNovoSensor.eResultadoAutoTeste;

			//Libera Mutex para acesso exclusivo ao buffer da lista de dispositivos na rede CAN
			status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
			ASSERT(status == osOK);
		}

		osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_CFG_SENSOR_RESPONDEU);

		//Indica que a aplicação leu a mensagem recebida
		wLastSig = osSignalSet(*(psCtrlEnl->psFlagEnl), CAN_APL_FLAG_MENSAGEM_LIDA);
		ASSERT(wLastSig != 0x80000000);
	}

	//----------------------------------------------------------------------------
	//Testa o evento "Resposta ao comando de Parâmetros do sensor"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_PARAMETROS)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_MSG_RESP_PARAMETROS);

		if (psSensor->eEstado == Conectado)
		{
			//Verifica o tipo de sensor
			switch (sIDAuteq.sID_bits.bTipo)
			{

				case CAN_APL_SENSOR_SEMENTE: //Sensor de sementes
				{
					//Se o parâmetro recebido na resposta do sensor for
					//igual ao parâmetro enviado para o sensor
					if (!(memcmp(&CAN_sParametrosSensor,
						psMessage.data,
						sizeof(CAN_sParametrosSensor))) ||
						!(memcmp(&CAN_sParametrosExtended,
							psMessage.data,
							sizeof(CAN_sParametrosExtended))))
					{
						//Verifica se o loop é menor que 32, se não, usa os flags extendidos
						if (sIDAuteq.sID_bits.bLinha < 32)
						{
							//Indica que resposta ao comando Parâmetros do sensor foi recebida
							psCtrlApl->dResSemParametros |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
						}
						else
						{
							//Indica que resposta ao comando Parâmetros do sensor foi recebida
							psCtrlApl->dResSemParametrosExt |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
						}
					}
					break;
				}

				default:
					break;
			}

			//Verifica se todos os sensores de semente responderam ao comando
			psCtrlApl->bTodosSensSemRspParametros = (psCtrlApl->dResSemParametros == psCtrlApl->dSensSementeConectados);
			psCtrlApl->bTodosSensSemRspParametrosExt = (psCtrlApl->dResSemParametrosExt
				== psCtrlApl->dSensSementeConectadosExt);

			//Testa se todos os sensores responderam ao comando de PnP
			if (psCtrlApl->bTodosSensSemRspParametros &&
				psCtrlApl->bTodosSensSemRspParametrosExt)
			{
				osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP);
			}

		}
		//Indica que a aplicação leu a mensagem recebida
		wLastSig = osSignalSet(*(psCtrlEnl->psFlagEnl), CAN_APL_FLAG_MENSAGEM_LIDA);
		ASSERT(wLastSig != 0x80000000);
	}

	//----------------------------------------------------------------------------
	//Testa o evento "Resposta ao comando de Parâmetros do sensor"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_MSG_RESP_VERSAO_SW_SENSOR)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_MSG_RESP_VERSAO_SW_SENSOR);

		if (psSensor->eEstado == Conectado)
		{
			//Verifica o tipo de sensor
			switch (sIDAuteq.sID_bits.bTipo)
			{
				case CAN_APL_SENSOR_SEMENTE: //Sensor de sementes
				{

					memcpy(&psSensor->CAN_sVersaoSensor,
						psMessage.data,
						sizeof(psSensor->CAN_sVersaoSensor));

					//Verifica se o loop é menor que 32, se não usa os flags extendidos
					if (sIDAuteq.sID_bits.bLinha < 32)
					{
						//Indica que resposta ao comando Parâmetros do sensor foi recebida
						psCtrlApl->dResSemVersaoSW |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
					}
					else
					{
						//Indica que resposta ao comando Parâmetros do sensor foi recebida
						psCtrlApl->dResSemVersaoSWExt |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
					}
					break;
				}

				case CAN_APL_SENSOR_ADUBO:    //Sensor de adubo
				{

					memcpy(&psSensor->CAN_sVersaoSensor,
						psMessage.data,
						sizeof(psSensor->CAN_sVersaoSensor));
					//Verifica se o loop é menor que 32, se não, usa os flags extendidos
					if (sIDAuteq.sID_bits.bLinha < 32)
					{
						//Indica que resposta ao comando Parâmetros do sensor foi recebida
						psCtrlApl->dResAduVersaoSW |= 0x00000001 << sIDAuteq.sID_bits.bLinha;
					}
					else
					{
						//Indica que resposta ao comando Parâmetros do sensor foi recebida
						psCtrlApl->dResAduVersaoSW |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - 32);
					}
					break;
				}
				case CAN_APL_SENSOR_SIMULADOR:  //Sensor de Velocidade do Simulador
				case CAN_APL_SENSOR_DIGITAL_2:  //Sensor digital 2
				case CAN_APL_SENSOR_DIGITAL_3:  //Sensor digital 3
				case CAN_APL_SENSOR_DIGITAL_4:  //Sensor digital 4
				case CAN_APL_SENSOR_DIGITAL_5:  //Sensor digital 5
				case CAN_APL_SENSOR_DIGITAL_6:  //Sensor digital 6
				{
					memcpy(&psSensor->CAN_sVersaoSensor,
						psMessage.data,
						sizeof(psSensor->CAN_sVersaoSensor));

					//Indica que resposta ao comando de leitura de dados foi recebida
					psCtrlApl->dResDigVersaoSW |= 0x00000001 << (sIDAuteq.sID_bits.bLinha - CAN_bNUM_DE_LINHAS);

					break;
				}
				default:
					break;
			}

			//Verifica se todos os sensores de semente responderam ao comando
			psCtrlApl->bTodosSensSemRspVersao = (psCtrlApl->dResSemVersaoSW == psCtrlApl->dSensSementeConectados);
			psCtrlApl->bTodosSensSemRspVersaoExt = (psCtrlApl->dResSemVersaoSWExt == psCtrlApl->dSensSementeConectadosExt);

			//Verifica se todos os sensores de adubo responderam ao comando
			psCtrlApl->bTodosSensAduRspVersao = (psCtrlApl->dResAduVersaoSW == psCtrlApl->dSensAduboConectados);
			psCtrlApl->bTodosSensAduRspVersaoExt = (psCtrlApl->dResAduVersaoSWExt == psCtrlApl->dSensAduboConectadosExt);

			//Verifica se todos os sensores digitais responderam ao comando
			psCtrlApl->bTodosSensDigRspVersao = (psCtrlApl->dResDigVersaoSW == psCtrlApl->dSensDigitalConectados);

			//Testa se todos os sensores responderam ao comando de PnP
			if (psCtrlApl->bTodosSensSemRspVersao &&
				psCtrlApl->bTodosSensAduRspVersao &&
				psCtrlApl->bTodosSensSemRspVersaoExt &&
				psCtrlApl->bTodosSensAduRspVersaoExt)
			{
				osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP);
			}

		}
		//Indica que a aplicação leu a mensagem recebida
		wLastSig = osSignalSet(*(psCtrlEnl->psFlagEnl), CAN_APL_FLAG_MENSAGEM_LIDA);
		ASSERT(wLastSig != 0x80000000);
	}

	//----------------------------------------------------------------------------
	//Testa os eventos "Timeout do comando de PnP" e "Timeout do comando de
	//leitura de dados"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_PNP_TIMEOUT)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_PNP_TIMEOUT);

		//Desliga o timer de timeout do comando de PnP
		status = STOP_TIMER(psCtrlApl->wTimerTimeoutPnP);
		ASSERT(status == osOK);

		//Verifica qual(s) sensor(es) não respondeu(ram) ao comando de detecção
		SEN_vVerifyUnconnectedSensors();

		osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_SENSOR_NAO_RESPONDEU);
	}

	//----------------------------------------------------------------------------
	//Testa os eventos "Timeout do comando de Configuração"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_COMANDO_CONFIGURA)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_COMANDO_CONFIGURA);

		//Desliga o timer de timeout do comando enviado
		status = STOP_TIMER(psCtrlApl->wTimerTimeoutConfigura);
		ASSERT(status == osOK);

		//Pega o mutex antes acessar dados compartilhados:
		status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
		ASSERT(status == osOK);

		//Prepara a cópia de trabalho da estrutura com dados CAN:
		memset(&CAN_sCtrlLista.sNovoSensor, 0, sizeof(CAN_sCtrlLista.sNovoSensor));

		//Devolve o mutex:
		status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
		ASSERT(status == osOK);

		osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_SENSOR_NAO_RESPONDEU);
	}

	//----------------------------------------------------------------------------
	//Testa os eventos "Timeout do comando de PnP" e "Timeout do comando de
	//leitura de dados"
	if (psCtrlApl->dEventosApl & CAN_APL_FLAG_COMANDO_TIMEOUT)
	{
		//Limpa o indicador do evento:
		psCtrlApl->dEventosApl &= (~CAN_APL_FLAG_COMANDO_TIMEOUT);

		//Desliga o timer de timeout do comando enviado
		status = STOP_TIMER(psCtrlApl->wTimerTimeoutComando);
		ASSERT(status == osOK);

		//-----------------------------------------------------------------------------//
		if (psSensor->eEstado == Conectado)
		{
			//Verifica se todos os sensores de semente responderam ao comando
			psCtrlApl->bTodosSensSementeRsp = (psCtrlApl->dRespostaSemente == psCtrlApl->dSensSementeConectados);
			psCtrlApl->bTodosSensSementeRspExt = (psCtrlApl->dRespostaSementeExt == psCtrlApl->dSensSementeConectadosExt);

			//Verifica se todos os sensores de adubo responderam ao comando
			psCtrlApl->bTodosSensAduboRsp = (psCtrlApl->dRespostaAdubo == psCtrlApl->dSensAduboConectados);
			psCtrlApl->bTodosSensAduboRspExt = (psCtrlApl->dRespostaAduboExt == psCtrlApl->dSensAduboConectadosExt);

			//Verifica se todos os sensores digitais responderam ao comando
			psCtrlApl->bTodosSensDigitalRsp = (psCtrlApl->dRespostaDigital == psCtrlApl->dSensDigitalConectados);
		}
		//-----------------------------------------------------------------------------//
		//Verifica qual(s) sensor(es) não respondeu(ram) ao comando
		SEN_vVerifyUnconnectedSensors();

		osFlagSet(CAN_psFlagApl, CAN_APL_FLAG_SENSOR_NAO_RESPONDEU);
	}
}
