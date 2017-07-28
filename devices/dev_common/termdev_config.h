/****************************************************************************
 * Title                 :	TERMDEV Device - board specific configurations
 * Filename              :	termdev_config.h
 * Author                :	Joao Paulo Martins
 * Origin Date           :	16/03/2016
 * Version               :	1.0.0
 * Compiler              : GCC 5.2 2015q4
 * Target                : LPC43XX M4
 * Notes                 : None
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
 *     Date      Version      Author         		Description
 *  16/03/2016	 1.0.0	   Joao Paulo Martins	  First version of file
 *
 *****************************************************************************/
/** @file termdev_config.h
 *  @brief
 *
 */
#ifndef COMMON_TERMDEV_CONFIG_H_
#define COMMON_TERMDEV_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#else
#endif

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/* Initial configurations for the devices */
#define	TERMDEV_IFACE_UART 0x01								 /*!< TERMDEV messages over UART interface */
#define	TERMDEV_IFACE_USB  0x02								 /*!< TERMDEV messages over USB interface */
#define	TERMDEV_IFACE_CAN  0x04								 /*!< TERMDEV messages over CAN interface */

#define RESERVED_INTERFACES	TERMDEV_IFACE_UART /*!< Init the UART peripheral as default */
#define ACTIVE_INTERFACE	TERMDEV_IFACE_UART /*!< Activate UART peripheral as default */

#define TERMDEV_UART_CHANNEL	CHANNEL1 				 /*!< UART channel to be used by the device */
#define TERMDEV_UART_BAUD		UART_B115200 		     /*!< Standard baudrate of UART */
#define TERMDEV_USB_CHANNEL		CHANNEL0 				 /*!< USB  channel to be used by the device */
#define TERMDEV_CAN_CHANNEL		CAN0 						 /*!< CAN  channel to be used by the device */
#define TERMDEV_CAN_BITRATE		CAN_BITRATE_500  /*!< Standard baudrate to be used in CAN */

#define TERMDEV_BUF_SIZE			50	/*!< Fixed receive buffer size in bytes */

/******************************************************************************
 * Macros
 *******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
#define IOCTL_termdev \
  X(IOCTL_TDV_SET_ACTIVE, TDV_eSetActive) \
	X(IOCTL_TDV_DISABLE, TDV_eDisable) \
	X(IOCTL_TDV_ADD_CAN_ID, TDV_eCANAddID) \
  X(IOCTL_TDV_INVALID, NULL) \

/**
 * This Typedef holds all possible IOCTL Requests for termdev
 */
#define X(a, b) a,
typedef enum IOCTL_TDV_REQUEST_e
{
	IOCTL_termdev
} IOCTL_TDV_REQUEST_e;
#undef X

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/******************************************************************************
 * Variables
 *******************************************************************************/
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
//#endif /* UNITY_TEST */
#endif /* COMMON_TERMDEV_CONFIG_H_ */
