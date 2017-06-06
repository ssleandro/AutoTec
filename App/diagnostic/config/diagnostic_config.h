/****************************************************************************
 * Title                 :   diagnostic_config Include File
 * Filename              :   diagnostic_config.h
 * Author                :   Joao Paulo Martins
 * Origin Date           :   08 de abr de 2016
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
 *    Date    Version        Author         				  Description
 *  08/04/16   1.0.0    Joao Paulo Martins    diagnostic_config.h created.
 *
 *****************************************************************************/
/** @file 	diagnostic_config.h
 *  @brief
 *
 */
#ifndef DIAGNOSTIC_CONFIG_DIAGNOSTIC_CONFIG_H_
#define DIAGNOSTIC_CONFIG_DIAGNOSTIC_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "devicelib.h"
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#if defined (UNITY_TEST)
#endif
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
//Handlers
#define LOG_PAYLOAD_SIZE	100

#define DIAGNOSTIC_ID PERIPHERAL_TERMDEV  //!< Handler

// TERMDEV basic configurations
#define TERMDEV_DEV_UART	0x01	//!< Set TERMDEV to use UART
#define TERMDEV_DEV_USB		0x02	//!< Set TERMDEV to use USB
#define TERMDEV_DEV_CAN		0x04	//!< Set TERMDEV to use CAN

#define DIG_INITIAL_IO_IFACE	TERMDEV_DEV_UART

#define DIG_INIT_CAN_ID	0x2F8
#define DIG_SEND_CAN_ID	0x2F7

//Main loop wait
#define DIAGNOSTIC_QUEUE_WAIT 			(0) //!< No timeout - wait forever
#define DIAGNOSTIC_DEFAULT_MSGSIZE  16  //!< Default output format - 16 bytes

/******************************************************************************
 * Macros
 *******************************************************************************/
//Debug
#define DIG_BUFFER_SIZE           (1 << 12) //!< Buffer Size for error logging
#define DIG_BUFFER_MAX_LINE_SIZE  128       //!< Line size for error logging

#ifndef DBG_LOG_LEVEL
#define DBG_LOG_LEVEL LEVEL_CRITICAL        //!< Debug Level (defined in debug_tool.h)
#endif

#define DBG_ASSERT_STOP_LEVEL LEVEL_ERROR
/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef enum log_types_e
{
	BROKER_LOG_TYPE,
	GENERIC_LOG_TYPE,
	INVALID_LOG_TYPE
} log_types_e;

/******************************************************************************
 * Variables
 *******************************************************************************/
extern peripheral_descriptor_p pTDVHandle;   //!< TERMDEV Handler
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

#endif /* DIAGNOSTIC_CONFIG_DIAGNOSTIC_CONFIG_H_ */

/*** End of File **************************************************************/
