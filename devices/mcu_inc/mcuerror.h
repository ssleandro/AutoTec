/****************************************************************************
 * Title                 :   MCUerror Include File
 * Filename              :   mcuerror.h
 * Author                :   Thiago Palmieri
 * Origin Date           :   11/02/2016
 * Version               :   1.0.0
 * Compiler              :   GCC 5.2 2015q4
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
 *  11/02/16   1.0.0   Thiago Palmieri MCUerror include file Created.
 *
 *****************************************************************************/
/** @file mcuerror.h
 *  @brief This file provides all necessary return error codes for the MCUlib
 *
 */
#ifndef ABS_INC_MCUERROR_H_
#define ABS_INC_MCUERROR_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

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
typedef enum eMCUError_s
{
	MCU_ERROR_SUCCESS = 0,   //!< SUCCESS
	MCU_ERROR_MEMORY_ALLOCATION,   //!< Memory allocation fault
	MCU_ERROR_INVALID_CALLBACK,   //!< Callback invalid or not provided
	MCU_ERROR_ADC_INVALID_CHANNEL,   //!< ADC channel invalid or not provided
	MCU_ERROR_ADC_INVALID_RESOLUTION,   //!< ADC Invalid resolution provided
	MCU_ERROR_ADC_INVALID_SAMPLE_RATE,   //!< ADC Invalid Sample Rate provided
	MCU_ERROR_ADC_INVALID_SAMPLE_TYPE,   //!< ADC Invalid Sample type provided (Interrupt / DMA / Pooling)
	MCU_ERROR_PWM_INVALID_CHANNEL,   //!< PWM Invalid Channel provided
	MCU_ERROR_PWM_INVALID_FREQUENCY,   //!< PWM Invalid Frequency provided
	MCU_ERROR_GPIO_INVALID_PORT_OR_PIN,   //!< GPIO Invalid combination of port/pin provided
	MCU_ERROR_GPIO_INVALID_INITIALIZATION,   //!< GPIO Invalid Initialization, pin already initialized
	MCU_ERROR_GPIO_INVALID_DIRECTION,   //!< GPIO Invalid direction provided (both INPUT and OUTPUT or NONE)
	MCU_ERROR_GPIO_INVALID_INTERRUPTION,   //!< GPIO Invalid Interruption
	MCU_ERROR_CAN_INVALID_BITRATE,   //!< CAN Invalid Bitrate provided, too high
	MCU_ERROR_CAN_INVALID_CAN_PORT,   //!< CAN Invalid port provided
	MCU_ERROR_CAN_INVALID_CLOCK,   //!< CAN Invalid Clock Rate setting
	MCU_ERROR_USB_INVALID_USB,   //!< USB Invalid USB Interface
	MCU_ERROR_USB_BUSY,   //!< USB Interface is Busy
	MCU_ERROR_USB_UNABLE_TO_INITIALIZE,   //!< USB Unable to Initialize interface
	MCU_ERROR_UART_INVALID_CHANNEL,   //!< UART Invalid Channel
	MCU_ERROR_UART_ALREADY_IN_USE,   //!< UART already in use by the upper layer
	MCU_ERROR_UART_CONFIG_ERROR,   //!< UART Invalid configuration
	MCU_ERROR_UART_RECV_EMPTY,   //!< UART Invalid configuration
	MCU_ERROR_UART_RECV_FULL,   //!< UART Invalid configuration
	MCU_ERROR_UART_RECV_ERROR,   //!< UART Invalid configuration
	MCU_ERROR_UART_TRANS_FULL,   //!< UART Invalid configuration
	MCU_ERROR_UART_TRANS_ERROR,   //!< UART Invalid configuration
	MCU_ERROR_LCD_INVALID_CONFIG,   //!< Invalid LCD configuration struct
	MCU_ERROR_SPIFI_INVALID_CHANNEL,   //!< SPIFI Invalid Channel
	MCU_ERROR_SPIFI_INVALID_INITIALIZATION,   //!< SPIFI Invalid Initialization
	MCU_ERROR_SPIFI_READ_ERROR,   //!< SPIFI Unable to read device
	MCU_ERROR_SPIFI_PROGRAM_ERROR,   //!< SPIFI Unable to program device
	MCU_ERROR_SPIFI_ERASE_ERROR,   //!< SPIFI Unable to erase device
	MCU_ERROR_WDT_UNABLE_TO_INITIALIZE,   //!< WDT Unable to initialize service
	MCU_ERROR_WDT_NOT_INITIALIZED,   //!< WDT Not initialized
	MCU_ERROR_WDT_UNABLE_TO_DISABLE, //!< WDT Unable to disable service  MCU_ERROR_IAP_CMD_SUCCESS             	,	  //!< IAP command is executed successfully */
	MCU_ERROR_IAP_INVALID_COMMAND,   //!< IAP invalid command */
	MCU_ERROR_IAP_SRC_ADDR_ERROR,	  //!< IAP sector source address is not on word boundary */
	MCU_ERROR_IAP_DST_ADDR_ERROR,	  //!< IAP sector destination address is not on a correct boundary */
	MCU_ERROR_IAP_SRC_ADDR_NOT_MAPPED,	  //!< IAP source address is not mapped in the memory map */
	MCU_ERROR_IAP_DST_ADDR_NOT_MAPPED,	  //!< IAP destination address is not mapped in the memory map */
	MCU_ERROR_IAP_COUNT_ERROR,	  //!< IAP byte count is not multiple of 4 or is not a permitted value */
	MCU_ERROR_IAP_INVALID_SECTOR,	//!< IAP sector number is invalid or end sector number is greater than start sector number */
	MCU_ERROR_IAP_SECTOR_NOT_BLANK,	  //!< IAP sector is not blank */
	MCU_ERROR_IAP_SECTOR_NOT_PREPARED,	  //!< IAP command to prepare sector for write operation was not executed */
	MCU_ERROR_IAP_COMPARE_ERROR,	  //!< IAP source and destination data not equal */
	MCU_ERROR_IAP_BUSY,	  //!< IAP flash programming hardware interface is busy */
	MCU_ERROR_IAP_PARAM_ERROR,	  //!< IAP insufficient number of parameters or invalid parameter */
	MCU_ERROR_IAP_ADDR_ERROR,	  //!< IAP address is not on word boundary */
	MCU_ERROR_IAP_ADDR_NOT_MAPPED,	  //!< IAP address is not mapped in the memory map */
	MCU_ERROR_IAP_CMD_LOCKED,	  //!< IAP command is locked */
	MCU_ERROR_IAP_INVALID_CODE,	  //!< IAP unlock code is invalid */
	MCU_ERROR_IAP_INVALID_BAUD_RATE,	  //!< IAP invalid baud rate setting */
	MCU_ERROR_IAP_INVALID_STOP_BIT,	  //!< IAP invalid stop bit setting */
	MCU_ERROR_IAP_CRP_ENABLED,	  //!< IAP code read protection enabled */
} eMCUError_s;

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

#endif /* ABS_INC_MCUERROR_H_ */

/*** End of File **************************************************************/
