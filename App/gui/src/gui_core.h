/****************************************************************************
 * Title                 :   gui_core
 * Filename              :   gui_core.h
 * Author                :   Henrique Reis
 * Origin Date           :   19 de abr de 2017
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
 *  19/04/17   1.0.0     Henrique Reis         gui_core.h created.
 *
 *****************************************************************************/
#ifndef APP_GUI_SRC_GUI_CORE_H_
#define APP_GUI_SRC_GUI_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../gui/inc/interface_gui.h"
#else
#include "interface_gui.h"
#endif

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define GUI_FLGAS_ALL_FLAGS			  	  0x00FFFFFF
#define GUI_UPDATE_INSTALLATION_INTERFACE 0x00000001
#define GUI_UPDATE_PLANTER_INTERFACE 	  0x00000002
#define GUI_UPDATE_TEST_MODE_INTERFACE	  0x00000004
#define GUI_UPDATE_TRIMMING_INTERFACE	  0x00000008
#define GUI_UPDATE_SYSTEM_INTERFACE		  0x00000010
#define GUI_CHANGE_CURRENT_DATA_MASK	  0x00000020
#define GUI_CHANGE_CURRENT_CONFIGURATION  0x00000040
#define GUI_CHANGE_INSTAL_REPEAT_TEST     0x00000080
#define GUI_INSTALL_CONFIRM_INSTALLATION	0x00000100

// Flags usados para converter unidades de medidas:
#define GUI_dMILIMETERS       1
#define GUI_dCENTIMETERS      2
#define GUI_dMETERS           3
#define GUI_dKILOMETERS       4
#define GUI_dHECTARES         5
#define GUI_dACRES            6
#define GUI_dMILES            7
#define GUI_dFEETS            8
#define GUI_dINCHES	         9
#define GUI_dUNITS_QUANT	   9
#define GUI_dCONV(from,to)  (((from)<<16)|(to))

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define GUI_NUM_SENSOR 	CAN_bNUM_DE_LINHAS

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef struct
{
	//Idioma:
	uint8_t bIdioma;

	// Se sistema imperial ou internacional
	uint8_t bSistImperial;

	// Tipo de medida para velocidade do veiculo
	uint8_t bVelocidade;
	uint8_t bTxtVel;
	uint8_t bTxtVelPorHora;
	// Medida da superficie trabalhada
	uint8_t bAreaTrabalhada;
	uint8_t bTxtAreaTrab;
	uint8_t bTxtAreaTrabPorHora;
	// Sementes por cm/pol..
	uint8_t bSementes;
	uint8_t bTxtSementes;
	uint8_t bTxtSemPorDist;
	uint8_t bImgSemPorDist;
	// distancia em km/mi..
	uint8_t bDistPerc;
	uint8_t bTxtDistPerc;
	// caractere usado para fracao e milhar:
	uint8_t bCharFrac;
	uint8_t bCharMilhar;

	// CRC da estrutura
	uint16_t wCRC16;
} GUI_tsConfig;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* APP_GUI_SRC_GUI_CORE_H_ */
