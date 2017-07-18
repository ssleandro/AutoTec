/****************************************************************************
 * Title                 :   control_core
 * Filename              :   control_core.c
 * Author                :   Henrique Reis
 * Origin Date           :   18 de abr de 2017
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
 *  18/04/17   1.0.0     Henrique Reis         control_core.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "debug_tool.h"
#include "config_files.h"
#include "filesys_core.h"
#include "file_tool.h"
#include <stdlib.h>
#include "fat_sl.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//Nome do arquivo de configuração:
const uint8_t FFS_abConfigName[] = "MPA2500.CFG";
const uint8_t FFS_abSensorCfgName[] = "SENSORES.CAN";
//Nome do arquivo de registro estático:
const uint8_t FFS_abStaticRegCfgName[] = "ESTATICO.REG";
//Nome do arquivo de registros combinados (estáticos e dinâmicos):
//const uint8_t abNomeRegDat[] = "REGISTRO.DAT";

EXTERN_MUTEX(FFS_AccesControl);

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/
extern osFlagsGroupId FFS_sFlagSis;
extern UOS_tsConfiguracao FFS_sConfiguracao;
extern const UOS_tsConfiguracao UOS_sConfiguracaoDefault;

extern AQR_tsCtrlListaSens FFS_sCtrlListaSens;

extern AQR_tsRegEstaticoCRC FFS_sRegEstaticoCRC;

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/*****************************************************************************
 void FFS_vLoadConfigFile( void )
 Descricao : funcao para carregar a configuracao do sistema a partir do
 sistema de arquivos.
 Parametros: nenhum
 Retorno   : nenhum
 ******************************************************************************/
eAPPError_s FFS_vLoadConfigFile (void)
{
	osFlags dFlagsSis;
	uint16_t wCRC16_C;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ret;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		//Procura pelo arquivo de configuracao:
		bErr = f_findfirst(FFS_abConfigName, &xFindStruct);
		ASSERT((bErr == F_NO_ERROR) || (bErr == F_ERR_NOTFOUND));

		if (bErr == F_NO_ERROR)
		{
			xFileHandle = f_open(FFS_abConfigName, "r");
			ASSERT(xFileHandle != NULL);

			//Verifica se o tamanho consiste:
			if (xFindStruct.filesize == sizeof(FFS_sConfiguracao) && (xFileHandle != NULL))
			{
				//Le o arquivo de configuracao do sistema de arquivos:
				bErr = f_read((uint8_t * )&FFS_sConfiguracao, sizeof(FFS_sConfiguracao), 1, xFileHandle);
				ASSERT(bErr == 1);

				//Confere o CRC da configuracao:
				TLS_vCalculaCRC16Bloco(&wCRC16_C, (uint8_t *)&FFS_sConfiguracao, sizeof(FFS_sConfiguracao));
				//Se o CRC esta OK:
				if (wCRC16_C == 0)
				{
					bErroCfg = false;
				}
			}
			//Fecha o arquivo de configuracao:
			bErr = f_close(xFileHandle);
			ASSERT(bErr == F_NO_ERROR);
		}
	}
	//Carrega arquivo de configuracao default
	if (bErroCfg == true)
	{
		memcpy(&FFS_sConfiguracao, &UOS_sConfiguracaoDefault, sizeof(UOS_sConfiguracaoDefault));

		osFlagClear(FFS_sFlagSis, FFS_FLAG_CFG);
		ret = APP_ERROR_ERROR;
	}
	else
	{
		osFlagSet(FFS_sFlagSis, FFS_FLAG_CFG);
		ret = APP_ERROR_SUCCESS;
	}

	RELEASE_MUTEX(FFS_AccesControl);

	return ret;
}

/*****************************************************************************

 void FFS_vSalveConfigFile( void )

 Descricao : funcao para salvar arquivo de configuracao do sistema.
 Parametros: nenhum
 Retorno   : nenhum

 ******************************************************************************/
eAPPError_s FFS_vSaveConfigFile (void)
{
	osFlags dFlagsSis;
	uint16_t wCRC16;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&FFS_sConfiguracao,
		(sizeof(FFS_sConfiguracao) - sizeof(FFS_sConfiguracao.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sConfiguracao.wCRC16 = wCRC16;

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		xFileHandle = f_open(FFS_abConfigName, "w+");
		ASSERT(xFileHandle != NULL);

		bErr = f_rewind(xFileHandle);
		ASSERT(bErr == F_NO_ERROR);

		bErr = f_write((uint8_t* )&FFS_sConfiguracao, sizeof(FFS_sConfiguracao), 1, xFileHandle);
		ASSERT(bErr == 1);

		f_close(xFileHandle);

		if (bErr == 1)
		{
			ErroReturn = APP_ERROR_SUCCESS;
		}

	}

	RELEASE_MUTEX(FFS_AccesControl);

	return ErroReturn;
}

/*****************************************************************************

 void FFS_vRemoveSensorCfg( void )

 Descricao : funcao para apagar arquivo de configuracao dos sensores.
 Parametros: nenhum
 Retorno   : nenhum

 ******************************************************************************/
eAPPError_s FFS_vRemoveSensorCfg (void)
{
	uint8_t bNumTotalSensores, bCount;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	f_delete(FFS_abSensorCfgName);

	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	// Inicia parâmetros default:
	//Numero total de sensores na rede CAN
	bNumTotalSensores = ARRAY_SIZE(FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista);

	//Ajusta todos os registros da lista com o estado "Novo",
	//uma lista anteriormente gravada no sistema de arquivos
	for (bCount = 0; bCount < bNumTotalSensores; bCount++)
	{
		FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista[bCount].eEstado = Novo;
	}

	RELEASE_MUTEX(FFS_AccesControl);

	return APP_ERROR_SUCCESS;
}

/*****************************************************************************

 void FFS_vSaveSensorCfg( void )

 Descricao : funcao para salvar arquivo de configuracao dos sensores.
 Parametros: nenhum
 Retorno   : nenhum

 ******************************************************************************/
eAPPError_s FFS_vSaveSensorCfg (void)
{
	osFlags dFlagsSis;
	uint16_t wCRC16;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&FFS_sCtrlListaSens,
		(sizeof(FFS_sCtrlListaSens) - sizeof(FFS_sCtrlListaSens.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sCtrlListaSens.wCRC16 = wCRC16;

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		xFileHandle = f_open(FFS_abSensorCfgName, "w+");
		ASSERT(xFileHandle != NULL);

		bErr = f_rewind(xFileHandle);
		ASSERT(bErr == F_NO_ERROR);

		bErr = f_write((uint8_t* )&FFS_sCtrlListaSens, sizeof(FFS_sCtrlListaSens), 1, xFileHandle);
		ASSERT(bErr == 1);

		f_close(xFileHandle);

		if (bErr == 1)
		{
			ErroReturn = APP_ERROR_SUCCESS;
		}
		else
		{
			ErroReturn = APP_ERROR_ERROR;
		}

	}

	RELEASE_MUTEX(FFS_AccesControl);

	return ErroReturn;
}

/*
 ================================================================================
 CARREGA CONFIG IHM

 Descrição:    Carrega a configuração do IHM
 Parâmetros:   Nenhum
 Retorno:      true se conseguir carregar a configuração
 Obs.:         Nenhuma
 ================================================================================
 */
eAPPError_s FFS_vLoadSensorCfg (void)
{
	osFlags dFlagsSis;
	uint16_t wCRC16_C;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ret;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		//Procura pelo arquivo de configuracao:
		bErr = f_findfirst(FFS_abSensorCfgName, &xFindStruct);
		ASSERT((bErr == F_NO_ERROR) || (bErr == F_ERR_NOTFOUND));

		if (bErr == F_NO_ERROR)
		{
			xFileHandle = f_open(FFS_abSensorCfgName, "r");
			ASSERT(xFileHandle != NULL);

			//Verifica se o tamanho consiste:
			if (xFindStruct.filesize == sizeof(FFS_sCtrlListaSens) && (xFileHandle != NULL))
			{
				//Le o arquivo de configuracao do sistema de arquivos:
				bErr = f_read((uint8_t * )&FFS_sCtrlListaSens, sizeof(FFS_sCtrlListaSens), 1, xFileHandle);
				ASSERT(bErr == 1);

				//Confere o CRC da configuracao:
				TLS_vCalculaCRC16Bloco(&wCRC16_C, (uint8_t *)&FFS_sCtrlListaSens, sizeof(FFS_sCtrlListaSens));
				//Se o CRC esta OK:
				if (wCRC16_C == 0)
				{
					bErroCfg = false;
				}
			}
			//Fecha o arquivo de configuracao:
			bErr = f_close(xFileHandle);
			ASSERT(bErr == F_NO_ERROR);
		}
	}
	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	if (bErroCfg == true)
	{
		uint8_t bNumTotalSensores, bCount;

		// Inicia parâmetros default:
		//Numero total de sensores na rede CAN
		bNumTotalSensores = ARRAY_SIZE(FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista);

		//Ajusta todos os registros da lista com o estado "Novo",
		//uma lista anteriormente gravada no sistema de arquivos
		for (bCount = 0; bCount < bNumTotalSensores; bCount++)
		{
			FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista[bCount].eEstado = Novo;
		}

		ret = APP_ERROR_ERROR;
	}
	else
	{
		ret = APP_ERROR_SUCCESS;
	}

	RELEASE_MUTEX(FFS_AccesControl);

	return ret;
}

/*
 ================================================================================
 CARREGA STATIC REGISTER

 Descrição:    Carrega a configuração do IHM
 Parâmetros:   Nenhum
 Retorno:      true se conseguir carregar a configuração
 Obs.:         Nenhuma
 ================================================================================
 */
eAPPError_s FFS_vSaveStaticReg (void)
{

	osFlags dFlagsSis;
	uint16_t wCRC16;
	uint8_t bErr = 1;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&FFS_sRegEstaticoCRC,
		(sizeof(FFS_sRegEstaticoCRC) - sizeof(FFS_sRegEstaticoCRC.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sRegEstaticoCRC.wCRC16 = wCRC16;

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{

		xFileHandle = f_open(FFS_abStaticRegCfgName, "w+");
		ASSERT(xFileHandle != NULL);

		//bErr = f_rewind(xFileHandle);
		//ASSERT(bErr == F_NO_ERROR);

				bErr = f_write((uint8_t* )&FFS_sRegEstaticoCRC, sizeof(FFS_sRegEstaticoCRC), 1, xFileHandle);
		ASSERT(bErr == 1);

		f_close(xFileHandle);

		if (bErr == 1)
		{
			ErroReturn = APP_ERROR_SUCCESS;
		}
		else
		{
			ErroReturn = APP_ERROR_ERROR;
		}
	}

	RELEASE_MUTEX(FFS_AccesControl);

	return ErroReturn;
}

/*
 ================================================================================
 CARREGA STATIC REGISTERM

 Descrição:    Carrega a configuração do IHM
 Parâmetros:   Nenhum
 Retorno:      true se conseguir carregar a configuração
 Obs.:         Nenhuma
 ================================================================================
 */
eAPPError_s FFS_vLoadStaticReg (void)
{
	osFlags dFlagsSis;
	uint16_t wCRC16_C, wCRC16;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ret;

	WAIT_MUTEX(FFS_AccesControl, osWaitForever);

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		//Procura pelo arquivo de configuracao:
		bErr = f_findfirst(FFS_abStaticRegCfgName, &xFindStruct);
		ASSERT((bErr == F_NO_ERROR) || (bErr == F_ERR_NOTFOUND));

		if (bErr == F_NO_ERROR)
		{
			xFileHandle = f_open(FFS_abStaticRegCfgName, "r");
			ASSERT(xFileHandle != NULL);

			//Verifica se o tamanho consiste:
			if (xFindStruct.filesize == sizeof(FFS_sRegEstaticoCRC) && (xFileHandle != NULL))
			{
				//Le o arquivo de configuracao do sistema de arquivos:
				bErr = f_read((uint8_t * )&FFS_sRegEstaticoCRC, sizeof(FFS_sRegEstaticoCRC), 1, xFileHandle);
				ASSERT(bErr == 1);

				//Confere o CRC da configuracao:
				TLS_vCalculaCRC16Bloco(&wCRC16_C, (uint8_t *)&FFS_sRegEstaticoCRC, sizeof(FFS_sRegEstaticoCRC));
				//Se o CRC esta OK:
				if (wCRC16_C == 0)
				{
					bErroCfg = false;
				}
			}
			//Fecha o arquivo de configuracao:
			bErr = f_close(xFileHandle);
			ASSERT(bErr == F_NO_ERROR);
		}
	}

	if (bErroCfg == true)
	{
		//Limpa a estrutura do registro estático:
		memset(&FFS_sRegEstaticoCRC, 0x00, sizeof(FFS_sRegEstaticoCRC));

		ret = APP_ERROR_ERROR;
	}
	else
	{
		ret = APP_ERROR_SUCCESS;
	}
	osFlagSet(FFS_sFlagSis, FFS_FLAG_STATIC_REG);

	RELEASE_MUTEX(FFS_AccesControl);

	return ret;
}
