/****************************************************************************
* Title                 :   CBACOMM_config Include File
* Filename              :   CBACOMM_config.h
* Author                :   thiago.palmieri
* Origin Date           :   18 de mar de 2016
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
*  18 de mar de 2016   1.0.0   thiago.palmieri CBACOMM_config include file Created.
*
*****************************************************************************/
/** @file CBACOMM_config.h
 *  @brief This file provides configuration data for CBACOMM Device.
 *
 */
#ifndef COMMON_CBACOMM_CONFIG_H_
#define COMMON_CBACOMM_CONFIG_H_

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
//Interface Configuration
#define CCM_INTERFACE_UART      0                 //!< UART Interface Index
#define CCM_INTERFACE_CAN       1                 //!< CAN Interface Index
#define CCM_INTERFACE_USB       2                 //!< USB Interface Index
#define CCM_INTERFACE_INVALID   3                 //!< Invalid Interface

#define CCM_DEFAULT_INTERFACE   CCM_INTERFACE_CAN //!< Default Interface

//UART Configuration
#define CCM_DEFAULT_UART_BAUD   UART_B115200      //!< Default UART Baud
#define CCM_DEFAULT_UART        UART1             //!< Default UART interface
#define CCM_DEFAULT_UART_WORD   UART_WORD_8B      //!< Default UART Word Size
#define CCM_DEFAULT_UART_STOP   UART_STOPBITS1    //!< Default UART Stopbits
#define CCM_DEFAULT_UART_PARITY UART_PARITY_NONE  //!< Default UART Parity

//CAN Configuration
#define CCM_DEFAULT_CAN         CAN0              //!< Default CAN Interface
#define CCM_DEFAULT_CAN_BITRATE CAN_BITRATE_500   //!< Default CAN Bitrate

//USB Configuration
#define CCM_DEFAULT_USB         USB0              //!< Default USR Interface
#define CCM_DEFAULT_USB_CLASS   USB_CLASS_CDC     //!< Default USB Class

//Buffer Configuration
#define CCM_DEFAULT_BUFFER_SIZE 256               //!< Default Message Buffer Size

//Multipacket Configuration
#define CCM_CAN_MESSAGE_FINAL       0xBB
#define CCM_CAN_MESSAGE_HAS_MORE    0xAA
/******************************************************************************
* Macros
*******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
//CAN, PORT, TD, RD, MODE
#define IOCTL_CBACOMM \
  X(IOCTL_CCM_CHANGE_UART_SPEED, CCM_eIOCTLChangeSpeed) \
  X(IOCTL_CCM_CHANGE_CAN_SPEED, CCM_eIOCTLChangeSpeed) \
  X(IOCTL_CCM_CHANGE_INTERFACE, CCM_eIOCTLChangeInterface) \
  X(IOCTL_CCM_CHANGE_CALLBACK, CCM_eIOCTLChangeCallBack) \
  X(IOCTL_CCM_CAN_ADD_RECEIVE_ID, CCM_eIOCTLAddID) \
  X(IOCTL_CCM_CAN_REMOVE_RECEIVE_ID, CCM_eIOCTLRemoveID) \
  X(IOCTL_CCM_CAN_NEXT_MESSAGE_ID, CCM_eIOCTLSendID) \
  X(IOCTL_CCM_GET_INTERFACE, CCM_eIOCTLGets) \
  X(IOCTL_CCM_GET_SPEED, CCM_eIOCTLGets) \
  X(IOCTL_CCM_INVALID, NULL) \


/******************************************************************************
* Typedefs
*******************************************************************************/
#if defined (UNITY_TEST)
/**
 * This enumeration is a list of possible USB ports on the Board
 */
typedef enum eUSBInterfaces_s
{
  USB0,               //!< USB0 Interface
  USB1,               //!< USB1 Interface
  USB_INVALID         //!< Invalid USB Interface
} eUSBInterfaces_s;

/*
*   @brief  UART peripherals nomenclature
*/
typedef enum uart_channel_e
{
  UART0 = 0,         /*!< UART 0 peripheral generic nomenclature   */
  UART1,             /*!< UART 1 peripheral generic nomenclature   */
  UART2,             /*!< UART 2 peripheral generic nomenclature   */
  UART3,             /*!< UART 3 peripheral generic nomenclature   */
  UART_INVALID       /*!< UART 5 peripheral generic nomenclature   */
} uart_channel_e;

/*
 * @brief UART standard baudrate values
 * */
typedef enum uart_baudrate_e
{
  UART_B9600    = 9600,
  UART_B19200   = 19200,
  UART_B38400   = 38400,
  UART_B57600   = 57600,
  UART_B115200  = 115200,
  UART_B1000000 = 1000000,
  UART_BINVALID ,
}uart_baudrate_e;

/**
 * This enumeration is a list of possible CAN ports on the Board
 */
typedef enum canChannel_s
{
  CAN0,           //!< CAN 0
  CAN1,           //!< CAN 1
  CAN_INVALID     //!< Invalid CAN indicator
}canChannel_s;

/**
 * This enumeration is a list of possible CAN bitrates that can be used
 */
typedef enum canBitrate_s
{
  CAN_BITRATE_250       = 250000,      //!< 250Kbps
  CAN_BITRATE_500       = 500000,      //!< 500Kbps
  CAN_BITRATE_1000      = 1000000,     //!< 1000Kbps
  CAN_BITRATE_INVALID                  //!< Invalid value
}canBitrate_s;

#endif

typedef eUSBInterfaces_s CBACOMUSBInterface_e;    //!< Valid USB Interfaces
typedef uart_channel_e CBACOMMUARTInterface_e;    //!< Valid UART Interfaces
typedef canChannel_s CBACOMMCANInterface_e;       //!< Valid CAN Interfaces

typedef canBitrate_s CBACOMMCANSpeed_e;           //!< Valid CAN Bitrate
typedef uart_baudrate_e CBACOMMUARTSpeed_e;       //!< Valid UART Bauds

typedef void (*CCMCallBack)(void *message, uint32_t data);    //!< CBAComm Callback

/**
 * This Typedef holds all possible IOCTL Requests for CBA Communicator
 */
#define X(a, b) a,
typedef enum IOCTL_CCM_REQUEST_e
{
  IOCTL_CBACOMM
}IOCTL_CCM_REQUEST_e;
#undef X

//typedef eUSBInterfaces_s CBACOMUSBInterface_e;    //!< Valid USB Interfaces
//typedef uart_channel_e CBACOMMUARTInterface_e;    //!< Valid UART Interfaces
//typedef canChannel_s CBACOMMCANInterface_e;       //!< Valid CAN Interfaces
//
//typedef canBitrate_s CBACOMMCANSpeed_e;           //!< Valid CAN Bitrate
//typedef uart_baudrate_e CBACOMMUARTSpeed_e;       //!< Valid UART Bauds


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

#endif /* COMMON_CBACOMM_CONFIG_H_ */

/*** End of File **************************************************************/
