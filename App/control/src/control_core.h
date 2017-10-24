/****************************************************************************
 * Title                 :   control_core
 * Filename              :   control_core.h
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
 *  18/04/17   1.0.0     Henrique Reis         control_core.h created.
 *
 *****************************************************************************/
#ifndef APP_CONTROL_SRC_CONTROL_CORE_H_
#define APP_CONTROL_SRC_CONTROL_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../control/inc/interface_control.h"
#else
#include "interface_control.h"
#endif

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/

//Máscaras para os flags de sincronismo:
#define UOS_SINC_FLAG_NENHUM    0x00000000
#define UOS_SINC_FLAG_TODOS     0xFFFFFFFF

#define UOS_SINC_FLAG_32HZ      0x00000001
#define UOS_SINC_FLAG_16HZ      0x00000003
#define UOS_SINC_FLAG_8HZ       0x00000007
#define UOS_SINC_FLAG_4HZ       0x0000000F
#define UOS_SINC_FLAG_2HZ       0x0000001F
#define UOS_SINC_FLAG_1HZ       0x0000003F
#define UOS_SINC_FLAG_2S        0x0000007F
#define UOS_SINC_FLAG_4S        0x000000FF

//Máscaras para os flags de fase:
#define UOS_FASE_FLAG_NENHUM    0x00000000
#define UOS_FASE_FLAG_TODOS     0xFFFFFFFF

#define UOS_FASE_FLAG_00        0x00000001
#define UOS_FASE_FLAG_02        0x00000002
#define UOS_FASE_FLAG_04        0x00000004
#define UOS_FASE_FLAG_06        0x00000008
#define UOS_FASE_FLAG_08        0x00000010
#define UOS_FASE_FLAG_10        0x00000020
#define UOS_FASE_FLAG_12        0x00000040
#define UOS_FASE_FLAG_14        0x00000080
#define UOS_FASE_FLAG_16        0x00000100
#define UOS_FASE_FLAG_18        0x00000200
#define UOS_FASE_FLAG_20        0x00000400
#define UOS_FASE_FLAG_22        0x00000800
#define UOS_FASE_FLAG_24        0x00001000
#define UOS_FASE_FLAG_26        0x00002000
#define UOS_FASE_FLAG_28        0x00004000
#define UOS_FASE_FLAG_30        0x00008000
#define UOS_FASE_FLAG_32        0x00010000
#define UOS_FASE_FLAG_34        0x00020000
#define UOS_FASE_FLAG_36        0x00040000
#define UOS_FASE_FLAG_38        0x00080000
#define UOS_FASE_FLAG_40        0x00100000
#define UOS_FASE_FLAG_42        0x00200000
#define UOS_FASE_FLAG_44        0x00400000
#define UOS_FASE_FLAG_46        0x00800000
#define UOS_FASE_FLAG_48        0x01000000
#define UOS_FASE_FLAG_50        0x02000000
#define UOS_FASE_FLAG_52        0x04000000
#define UOS_FASE_FLAG_54        0x08000000
#define UOS_FASE_FLAG_56        0x10000000
#define UOS_FASE_FLAG_58        0x20000000
#define UOS_FASE_FLAG_60        0x40000000
#define UOS_FASE_FLAG_62        0x80000000

//Estados possíveis da UART0

#define UOS_SERIAL      1
#define UOS_GPS         2

#define UOS_BUZZER_OFF 0x00
#define UOS_BUZZER_ON  0x01


#define CTL_SAVE_CONFIG_DATA	 		0x000001
#define CTL_UPDATE_CONFIG_DATA	 	0x000002
#define CTL_UPDATE_FILE_INFO			0x000004
#define CTL_GET_FILE_INFO				0x000008

/******************************************************************************
 * Typedefs
 *******************************************************************************/

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

#endif /* APP_CONTROL_SRC_CONTROL_CORE_H_ */
