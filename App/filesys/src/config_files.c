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
#include "api_mdriver_span.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//Nome do arquivo de configuracao:
const uint8_t FFS_abConfigName[] = "MPA2500.CFG";
const uint8_t FFS_abSensorCfgName[] = "SENSORES.CAN";
//Nome do arquivo de registro estatico:
const uint8_t FFS_abStaticRegCfgName[] = "ESTATICO.REG";

EXTERN_MUTEX(FFS_AccessControl);

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

uint8_t FFS_bSaveFile(uint8_t const *bFileName, uint8_t *bData, uint32_t wLen)
{
	osFlags dFlagsSis;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	uint8_t bErr = 1;
	uint8_t bRet = 0;

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{

		xFileHandle = f_open(bFileName, "w+");
		ASSERT(xFileHandle != NULL);

		bErr = f_rewind(xFileHandle);
		ASSERT(bErr == F_NO_ERROR);

		bErr = f_write(bData, wLen, 1, xFileHandle);
		ASSERT(bErr == 1);

		f_close(xFileHandle);

		if (bErr == 1)
		{
			bRet = 1;
		}
	}
	return bRet;
}

uint8_t FFS_bReadFile(uint8_t const *bFileName, uint8_t *bData, uint32_t wLen)
{
	osFlags dFlagsSis;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	uint8_t bRet = F_ERR_READ;

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet(FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		//Procura pelo arquivo de configuracao:
		bErr = f_findfirst(bFileName, &xFindStruct);
		ASSERT((bErr == F_NO_ERROR) || (bErr == F_ERR_NOTFOUND));

		if (bErr == F_NO_ERROR)
		{
			xFileHandle = f_open(bFileName, "r");
			ASSERT(xFileHandle != NULL);

			//Verifica se o tamanho consiste:
			if ((xFindStruct.filesize == wLen) && (xFileHandle != NULL))
			{
				//Le o arquivo de configuracao do sistema de arquivos:
				bErr = f_read(bData, wLen, 1, xFileHandle);
				if (bErr == 1)
				{
					bRet = F_NO_ERROR;
				}
				ASSERT(bErr == 1);
			}

			//Fecha o arquivo de configuracao:
			bErr = f_close(xFileHandle);
			ASSERT(bErr == F_NO_ERROR);
		}
	}
	return bRet;
}

/*****************************************************************************
 void FFS_vLoadConfigFile( void )
 Descricao : funcao para carregar a configuracao do sistema a partir do
 sistema de arquivos.
 Parametros: nenhum
 Retorno   : nenhum
 ******************************************************************************/
eAPPError_s FFS_vLoadConfigFile (void)
{
	uint16_t wCRC16_C;
	uint8_t bErr;
	eAPPError_s eRet;

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	bErr = FFS_bReadFile(FFS_abConfigName, (uint8_t * )&FFS_sConfiguracao, sizeof(FFS_sConfiguracao));

	if (bErr == F_NO_ERROR)
	{
		//Confere o CRC da configuracao:
		TLS_vCalculaCRC16Bloco(&wCRC16_C, (uint8_t *)&FFS_sConfiguracao, sizeof(FFS_sConfiguracao));
		//Se o CRC esta OK:
		if ((wCRC16_C == 0) && (FFS_sConfiguracao.sMonitor.bNumLinhas != 0))
		{
			osFlagSet(FFS_sFlagSis, FFS_FLAG_CFG);
			eRet = APP_ERROR_SUCCESS;
		}
		else
		{
			bErr = F_ERR_READ;
		}
	}

	if (bErr != F_NO_ERROR)
	{
		//Carrega arquivo de configuracao default
		memcpy(&FFS_sConfiguracao, &UOS_sConfiguracaoDefault, sizeof(UOS_sConfiguracaoDefault));

		osFlagClear(FFS_sFlagSis, FFS_FLAG_CFG);
		eRet = APP_ERROR_ERROR;
	}

	RELEASE_MUTEX(FFS_AccessControl);

	return eRet;
}

/*****************************************************************************

 void FFS_vSalveConfigFile( void )

 Descricao : funcao para salvar arquivo de configuracao do sistema.
 Parametros: nenhum
 Retorno   : nenhum

 ******************************************************************************/
eAPPError_s FFS_vSaveConfigFile (void)
{
	uint8_t bErr;
	uint16_t wCRC16;
	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&FFS_sConfiguracao,
		(sizeof(FFS_sConfiguracao) - sizeof(FFS_sConfiguracao.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sConfiguracao.wCRC16 = wCRC16;

	bErr = FFS_bSaveFile(FFS_abConfigName, (uint8_t*) &FFS_sConfiguracao,
			sizeof(FFS_sConfiguracao));
	ASSERT(bErr == 1);
	if (bErr == 1)
	{
		ErroReturn = APP_ERROR_SUCCESS;
	}

	RELEASE_MUTEX(FFS_AccessControl);

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

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	f_delete(FFS_abSensorCfgName);

	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	// Inicia par??metros default:
	//Numero total de sensores na rede CAN
	bNumTotalSensores = ARRAY_SIZE(FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista);

	//Ajusta todos os registros da lista com o estado "Novo",
	//uma lista anteriormente gravada no sistema de arquivos
	for (bCount = 0; bCount < bNumTotalSensores; bCount++)
	{
		FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista[bCount].eEstado = Novo;
	}

	RELEASE_MUTEX(FFS_AccessControl);

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
	uint8_t bErr;
	uint16_t wCRC16;
	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&FFS_sCtrlListaSens,
		(sizeof(FFS_sCtrlListaSens) - sizeof(FFS_sCtrlListaSens.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sCtrlListaSens.wCRC16 = wCRC16;

	bErr = FFS_bSaveFile(FFS_abSensorCfgName, (uint8_t* )&FFS_sCtrlListaSens, sizeof(FFS_sCtrlListaSens));
	ASSERT(bErr == 1);
	if (bErr == 1)
	{
		ErroReturn = APP_ERROR_SUCCESS;
	}

	RELEASE_MUTEX(FFS_AccessControl);

	return ErroReturn;
}

/*
 ================================================================================
 CARREGA CONFIG IHM

 Descri????o:    Carrega a configura????o do IHM
 Par??metros:   Nenhum
 Retorno:      true se conseguir carregar a configura????o
 Obs.:         Nenhuma
 ================================================================================
 */
eAPPError_s FFS_vLoadSensorCfg (void)
{
	uint16_t wCRC16_C;
	uint8_t bErr;
	eAPPError_s eRet;

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	bErr = FFS_bReadFile(FFS_abSensorCfgName, (uint8_t * )&FFS_sCtrlListaSens, sizeof(FFS_sCtrlListaSens));

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16_C, (uint8_t *)&FFS_sCtrlListaSens, sizeof(FFS_sCtrlListaSens));

	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	//Se o CRC esta OK:
	if ((wCRC16_C == 0) && (bErr == F_NO_ERROR))
	{
		eRet = APP_ERROR_SUCCESS;
	}
	else
	{
		uint8_t bNumTotalSensores, bCount;

		// Inicia par??metros default:
		//Numero total de sensores na rede CAN
		bNumTotalSensores = ARRAY_SIZE(FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista);

		//Ajusta todos os registros da lista com o estado "Novo",
		//uma lista anteriormente gravada no sistema de arquivos
		for (bCount = 0; bCount < bNumTotalSensores; bCount++)
		{
			FFS_sCtrlListaSens.CAN_sCtrlListaSens.asLista[bCount].eEstado = Novo;
		}

		eRet = APP_ERROR_ERROR;
	}

	RELEASE_MUTEX(FFS_AccessControl);

	return eRet;
}


/*
 ================================================================================
 CARREGA STATIC REGISTER

 Descri????o:    Carrega a configura????o do IHM
 Par??metros:   Nenhum
 Retorno:      true se conseguir carregar a configura????o
 Obs.:         Nenhuma
 ================================================================================
 */
eAPPError_s FFS_vSaveStaticReg (void)
{

	uint16_t wCRC16;
	uint8_t bErr;
	uint8_t bErroCfg = true;

	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16, (uint8_t *)&FFS_sRegEstaticoCRC,
		(sizeof(FFS_sRegEstaticoCRC) - sizeof(FFS_sRegEstaticoCRC.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sRegEstaticoCRC.wCRC16 = wCRC16;

	bErr = FFS_bSaveFile(FFS_abStaticRegCfgName, (uint8_t* )&FFS_sRegEstaticoCRC, sizeof(FFS_sRegEstaticoCRC));
	ASSERT(bErr == 1);

	if (bErr > 0)
	{
		ErroReturn = APP_ERROR_SUCCESS;
	}

	RELEASE_MUTEX(FFS_AccessControl);

	return ErroReturn;
}

/*
 ================================================================================
 CARREGA STATIC REGISTERM

 Descri????o:    Carrega a configura????o do IHM
 Par??metros:   Nenhum
 Retorno:      true se conseguir carregar a configura????o
 Obs.:         Nenhuma
 ================================================================================
 */
eAPPError_s FFS_vLoadStaticReg (void)
{

	uint16_t wCRC16_C;
	uint8_t bErr;
	uint8_t bCount;
	eAPPError_s eRet;
	uint8_t const *Filename = FFS_abStaticRegCfgName;

	WAIT_MUTEX(FFS_AccessControl, osWaitForever);

	bErr = FFS_bReadFile(Filename, (uint8_t *) &FFS_sRegEstaticoCRC, sizeof(FFS_sRegEstaticoCRC));
	ASSERT(bErr == F_NO_ERROR);

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco(&wCRC16_C, (uint8_t *) &FFS_sCtrlListaSens,
			sizeof(FFS_sCtrlListaSens));

	//Limpa a estrutura de novo sensor
	memset(&FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor, 0,
				sizeof(FFS_sCtrlListaSens.CAN_sCtrlListaSens.sNovoSensor));

	//Se o CRC esta OK:
	if((bErr == F_NO_ERROR) && (wCRC16_C == 0))
	{
		osFlagSet(FFS_sFlagSis, FFS_FLAG_STATIC_REG);
		eRet = APP_ERROR_SUCCESS;
	}
	else
	{
		//Limpa a estrutura do registro est??tico:
		memset(&FFS_sRegEstaticoCRC, 0x00, sizeof(FFS_sRegEstaticoCRC));
		osFlagClear(FFS_sFlagSis, FFS_FLAG_STATIC_REG);
		eRet = APP_ERROR_ERROR;
	}

	RELEASE_MUTEX(FFS_AccessControl);

	return eRet;
}

void FFS_sGetFSInfo(FFS_sFSInfo *pSFInfo)
{
	osFlags dFlagsSis;
	uint8_t bErr;
	F_SPACE xSpace;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	uint8_t bRet = F_ERR_READ;
	FFS_sFileInfo **psFileInfo = NULL;
	TLS_FreeFSInfo(pSFInfo);

	/* Get space information on current embedded FAT file system drive. */
	bErr = f_getfreespace( &xSpace );
	if( bErr == F_NO_ERROR )
	{
		pSFInfo->wUsed = xSpace.used;
		pSFInfo->wFree = xSpace.free;
		pSFInfo->wTotal = xSpace.total;
		pSFInfo->wBad = xSpace.bad;
	}

	pSFInfo->bNumFiles = 0;

	//Procura pelo arquivo de configuracao:
	bErr = f_findfirst("*.*", &xFindStruct);
	if( bErr == F_NO_ERROR )
	{
		psFileInfo = &pSFInfo->pFirst;
		do
		{
			pSFInfo->bNumFiles++;
			*psFileInfo = pvPortMalloc(sizeof(FFS_sFileInfo));
			(*psFileInfo)->FileLengh = (int)xFindStruct.filesize;

			strcpy((*psFileInfo)->bFileName, xFindStruct.filename);

			TLS_convertDateTime((*psFileInfo)->bFileDateTime, xFindStruct.ctime,xFindStruct.cdate);
			(*psFileInfo)->pNext = NULL;
			psFileInfo = (FFS_sFileInfo **)&((*psFileInfo)->pNext);
		}while( f_findnext( &xFindStruct ) == F_NO_ERROR );
	}
}

eAPPError_s FFS_FormatFS(void)
{
	eAPPError_s ret = APP_ERROR_ERROR;
	uint8_t bStatus;
	uint8_t bRetries = 3;

	// Format
	span_format();

	bStatus = f_initvolume(initfunc_span);
	if (bStatus == F_ERR_NOTFORMATTED)
	{
		f_format(F_FAT16_MEDIA);
	}

	do
	{
		bStatus = f_initvolume(initfunc_span);
		if (bStatus == F_ERR_NOTFORMATTED)
		{
			f_format(F_FAT16_MEDIA);
		}
	} while ((bStatus != F_NO_ERROR) && (bRetries-- > 0));

	if (bStatus == F_NO_ERROR)
	{
		ret = APP_ERROR_SUCCESS;
	}
	return ret;
}
