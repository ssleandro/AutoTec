/****************************************************************************
 * Title                 :   KEYBACKLIGHT_config Include File
 * Filename              :   KEYBACKLIGHT_config.h
 * Author                :   thiago.palmieri
 * Origin Date           :   17 de mar de 2016
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
 *  17 de mar de 2016   1.0.0   thiago.palmieri KEYBACKLIGHT_config include file Created.
 *
 *****************************************************************************/
/** @file KEYBACKLIGHT_config.h
 *  @brief This file provides Keypad Backlight common configuration
 *
 */
#ifndef COMMON_KEYBACKLIGHT_CONFIG_H_
#define COMMON_KEYBACKLIGHT_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define KBL_DEFAULT_DUTY  50
#define KBL_DEFAULT_FREQ  2000
#define KBL_DEFAULT_CHANNEL PWM_CHANNEL1
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
#define IOCTL_KEYBACKLIGHT \
  X(IOCTL_KBL_SET_DUTY, KBL_eChangeDuty) \
  X(IOCTL_KBL_SET_FREQ, KBL_eChangeFrequency) \
  X(IOCTL_KBL_TURN_ON_OFF, KBL_eChangeStatus) \
  X(IOCTL_KBL_READ_FREQ, KBL_eReadConfig) \
  X(IOCTL_KBL_READ_DUTY, KBL_eReadConfig) \
  X(IOCTL_KBL_INVALID, NULL) \

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for Keypad Backlight
 */
#define X(a, b) a,
typedef enum IOCTL_KBL_REQUEST_e
{
	IOCTL_KEYBACKLIGHT
} IOCTL_KBL_REQUEST_e;
#undef X
/******************************************************************************
 * Variables
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

#endif /* COMMON_KEYBACKLIGHT_CONFIG_H_ */

/*** End of File **************************************************************/
