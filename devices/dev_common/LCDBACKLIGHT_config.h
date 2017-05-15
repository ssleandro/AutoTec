/****************************************************************************
* Title                 :   LCDBACKLIGHT_config Include File
* Filename              :   LCDBACKLIGHT_config.h
* Author                :   thiago.palmieri
* Origin Date           :   16 de mar de 2016
* Version               :   1.0.0
* Compiler              :   GCC 5.2 2015q4 / ICCARM 7.50.2.10312
* Target                :   LPC43XX M4
* Notes                 :   None
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
*    Date    Version   Author         Description
*  16 de mar de 2016   1.0.0   thiago.palmieri LCDBACKLIGHT_config include file Created.
*
*****************************************************************************/
/** @file LCDBACKLIGHT_config.h
 *  @brief This file provides LCD Backlight common configuration
 *
 */
#ifndef COMMON_LCDBACKLIGHT_CONFIG_H_
#define COMMON_LCDBACKLIGHT_CONFIG_H_

/******************************************************************************
* Includes
*******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#endif
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
#define LBK_DEFAULT_DUTY  50
#define LBK_DEFAULT_FREQ  2000
#define LBK_DEFAULT_CHANNEL PWM_CHANNEL0

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
//CAN, PORT, TD, RD, MODE
#define IOCTL_LCDBACKLIGHT \
  X(IOCTL_LBK_SET_DUTY, LBK_eChangeDuty) \
  X(IOCTL_LBK_SET_FREQ, LBK_eChangeFrequency) \
  X(IOCTL_LBK_TURN_ON_OFF, LBK_eChangeStatus) \
  X(IOCTL_LBK_READ_FREQ, LBK_eReadConfig) \
  X(IOCTL_LBK_READ_DUTY, LBK_eReadConfig) \
  X(IOCTL_LBK_INVALID, NULL) \

/******************************************************************************
* Typedefs
*******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for LCD Backlight
 */
#define X(a, b) a,
typedef enum IOCTL_LBK_REQUEST_e
{
  IOCTL_LCDBACKLIGHT
}IOCTL_LBK_REQUEST_e;
#undef X
/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMMON_LCDBACKLIGHT_CONFIG_H_ */

/*** End of File **************************************************************/
