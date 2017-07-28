/****************************************************************************
 * Title                 :   WATCHDOG_config Include File
 * Filename              :   WATCHDOG_config.h
 * Author                :   thiago.palmieri
 * Origin Date           :   6 de mai de 2016
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
 *  6 de mai de 2016   1.0.0   thiago.palmieri WATCHDOG_config include file Created.
 *
 *****************************************************************************/
/** @file WATCHDOG_config.h
 *  @brief This file provides configurations values to watchdog interface.
 *
 */
#ifndef DEVICES_DEV_COMMON_WATCHDOG_CONFIG_H_
#define DEVICES_DEV_COMMON_WATCHDOG_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define WDT_DEFAULT_TIMEOUT_IN_MILLIS   5000    //!< Watchdog default expiration time
/******************************************************************************
 * Macros
 *******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
//CAN, PORT, TD, RD, MODE
#define IOCTL_WATCHDOG \
  X(IOCTL_WDT_FEED, WDT_eIOCTLFeed) \
  X(IOCTL_WDT_CALLBACK_MODE, WDT_eIOCTLSetMode) \
  X(IOCTL_WDT_RESET_MODE, WDT_eIOCTLSetMode) \
  X(IOCTL_WDT_SET_TIMEOUT, WDT_eIOCTLSetMode) \
  X(IOCTL_WDT_FORCE_RESET, WDT_eIOCTLSetMode) \
  X(IOCTL_WDT_START, WDT_eIOCTLStart) \
  X(IOCTL_WDT_RESTART_REASON, WDT_eIOCTLReason) \
  X(IOCTL_WDT_INVALID, NULL) \

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for CBA Communicator
 */
#define X(a, b) a,
typedef enum IOCTL_WDT_REQUEST_e
{
	IOCTL_WATCHDOG
} IOCTL_WDT_REQUEST_e;
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

#endif /* DEVICES_DEV_COMMON_WATCHDOG_CONFIG_H_ */

/*** End of File **************************************************************/
