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
#include "filesys_core.h"
#include "file_tool.h"
#include <stdlib.h>
#include "fat_sl.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//Nome do arquivo de configuração:
const uint8_t UOS_abNomeConfig[] = "MPA2500.CFG";

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/
extern osFlagsGroupId FFS_sFlagSis;
extern UOS_tsConfiguracao	FFS_sConfiguracao;;
extern const UOS_tsConfiguracao UOS_sConfiguracaoDefault;

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

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet (FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		//Procura pelo arquivo de configuracao:
		bErr = f_findfirst(UOS_abNomeConfig, &xFindStruct);
		ASSERT((bErr == F_NO_ERROR) || (bErr == F_ERR_NOTFOUND));

		if (bErr == F_NO_ERROR)
		{
			xFileHandle = f_open(UOS_abNomeConfig, "r");
			ASSERT(xFileHandle == NULL);

			//Verifica se o tamanho consiste:
			if (xFindStruct.filesize == sizeof(FFS_sConfiguracao) && (xFileHandle != NULL))
			{
				//Le o arquivo de configuracao do sistema de arquivos:
				bErr = f_read((uint8_t * )&FFS_sConfiguracao, sizeof(FFS_sConfiguracao), 1, xFileHandle);
				ASSERT(bErr == 1);

				//Confere o CRC da configuracao:
				TLS_vCalculaCRC16Bloco (&wCRC16_C, (uint8_t *) &FFS_sConfiguracao, sizeof(FFS_sConfiguracao));
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
		//Carrega arquivo de configuracao default
		memcpy (&FFS_sConfiguracao, &UOS_sConfiguracaoDefault, sizeof(UOS_sConfiguracaoDefault));
		uint8_t abCodigo[] = { 0x25, 0x00, 0xA0, 0x00, 0x00, 0x00 };
		memcpy (FFS_sConfiguracao.sVeiculo.abCodigo, abCodigo, sizeof(FFS_sConfiguracao.sVeiculo.abCodigo));
		osFlagClear(FFS_sFlagSis, FFS_FLAG_CFG);
		ret = APP_ERROR_ERROR;
	} else
	{
		osFlagSet(FFS_sFlagSis, FFS_FLAG_CFG);
		ret = APP_ERROR_SUCCESS;
	}
	return ret;
}

/*****************************************************************************

 void FFS_vSalveConfigFile( void )

 Descricao : funcao para salvar arquivo de configuracao do sistema.
 Parametros: nenhum
 Retorno   : nenhum

 ******************************************************************************/
eAPPError_s FFS_vSalveConfigFile (void)
{
	osFlags dFlagsSis;
	uint16_t wCRC16;
	uint8_t bErr;
	uint8_t bErroCfg = true;
	F_FIND xFindStruct;
	F_FILE *xFileHandle;
	eAPPError_s ErroReturn = APP_ERROR_ERROR;

	//Confere o CRC da configuracao:
	TLS_vCalculaCRC16Bloco (&wCRC16, (uint8_t *) &FFS_sConfiguracao,
					(sizeof(FFS_sConfiguracao) - sizeof(FFS_sConfiguracao.wCRC16)));

	//Atualiza o valor do crc na estrutura:
	FFS_sConfiguracao.wCRC16 = wCRC16;

	//Verifica se o sistema de arquivo foi inicializado:
	dFlagsSis = osFlagGet (FFS_sFlagSis);

	if ((dFlagsSis & FFS_FLAG_STATUS) > 0)
	{
		xFileHandle = f_open(UOS_abNomeConfig, "w");

		if (xFileHandle != NULL)
		{
			bErr = f_rewind(xFileHandle);

			bErr = f_write((uint8_t* )&FFS_sConfiguracao, sizeof(FFS_sConfiguracao), 1, xFileHandle);
			ASSERT(bErr == 1);

			f_close(xFileHandle);

			if (bErr != 1)
			{
				ErroReturn = APP_ERROR_SUCCESS;
				osFlagSet(FFS_sFlagSis, FFS_FLAG_CFG);
			}
		}
	}

	return ErroReturn;
}
