/****************************************************************************
* Title                 :	Terminal Device API
* Filename              :	termdev.c
* Author                :	Joao Paulo Martins
* Origin Date           :	07/03/2016
* Version               :	1.0.3
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
*  07/03/2016	 1.0.0	   Joao Paulo Martins	  First version of file
*  16/03/2016  1.0.1		 Joao Paulo Martins		Adaptation to devicelib
*  31/03/2016	 1.0.2		 Joao Paulo Martins   Unit tests and new read functions
*  05/04/2016	 1.0.3     Joao Paulo Martins   New IOCTL functions and modifications
*
*****************************************************************************/
/** @file termdev.h
 *  @brief This file provides a device to get and send messages over different interfaces
 *  of the system, in order to provide debug and control of the program execution.
 *
 *  The avaliable interfaces of the device are: UART, USB and CAN. In order to use
 *  one of these, the device must be initiated and the desired interface must be
 *  activated using ioctl.
 *
 *  The current interface used can be changed at runtime by calling an ioctl operation. The
 *  enabled interfaces will remain open until the device closes or a ioctl disable a specific
 *  peripheral.
 *
 *  The details of specific interfaces are the following:
 *
 *	UART:
 *	- 8-N-1 frame configuration;
 *	- 115200 bps baudrate;
 *	- Read and send operation are blocking, default timeout is 500 ms;
 *
 *	USB:
 *	- Autobaud - will get the baudrate of the remote peer;
 *	- Read and send operation are blocking, default timeout is 500 ms;
 *
 *  CAN:
 *  - Bitrate: 1 Mbps;
 *  - After the first ID is added, then the interface is ready to receive messages;
 *  - In order to send CAN messages, DEV_ioctl must be used previously to inform message ID;
 *  - Messages greater than 7 bytes will be splited in many packets, but is transparent for the API;
 *  - Read and send operation are blocking, default timeout is 500 ms;
 *
 *  In order to close the interface, command DEV_close must be used.
 *
 *  Commands DEV_read, DEV_write must be used to read/write data from the interface
 *  respectively.
 *
 *  The default reading method is pooling. The default timeout of the operations is 500 ms.
 *
 *  Check the termdev_config.h file for defaults.
 *
 */
#ifndef INC_TERMDEV_H_
#define INC_TERMDEV_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <inttypes.h>
#include "devicelib.h"

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
#if defined (UNITY_TEST)
typedef signed char int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int  int32_t;
typedef unsigned   uint32_t;
typedef long long  int64_t;
typedef unsigned long long   uint64_t;
#endif

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/
typedef enum termdev_interface_e
{
	TERMDEV_UART 		= 1,	/*!< TERMDEV messages over UART interface */
	TERMDEV_USB  		= 2,	/*!< TERMDEV messages over USB interface */
	TERMDEV_CAN  		= 4,	/*!< TERMDEV messages over CAN interface */
	TERMDEV_LCD  		= 8,	/*!< TERMDEV messages over LCD display */
	TERMDEV_INVALID = 0,	/*!< TERMDEV invalid interface */
}termdev_interface_e;

typedef enum termdev_pchannel_e
{
	CHANNEL0,					/*!< Individual channel 0 for selected peripheral */
	CHANNEL1,					/*!< Individual channel 1 for selected peripheral */
	CHANNEL2,					/*!< Individual channel 2 for selected peripheral */
	CHANNEL3,					/*!< Individual channel 3 for selected peripheral */
	CHANNEL4,					/*!< Individual channel 4 for selected peripheral */
	INVALID_CHANNEL, 	/*!< Invalid channel */
}termdev_pchannel_e;

typedef enum termdev_msglevel_e
{
	MSG_LEVEL1, /*!< Message prioritization level - high priority */
	MSG_LEVEL2, /*!< Message prioritization level - high priority */
	MSG_LEVEL3, /*!< Message prioritization level - mid priority */
	MSG_LEVEL4, /*!< Message prioritization level - low priority */
	MSG_LEVEL5  /*!< Message prioritization level - low priority */
}termdev_msglevel_e;

typedef void (* termdev_Callback)(uint8_t *bpInputBuffer, uint32_t dSize);//!< TERMDEV receive callback typedef

typedef struct termdev_handle_s
{
	uint32_t						dReservedInterfaces; /*!< param: interfaces reserved for use (UART, USB, etc) */
	termdev_interface_e eActiveInterface;	 	 /*!< param: active input/output (UART, USB, etc) 		    */
	termdev_msglevel_e  eMessagesLevel;		   /*!< TERMDEV parameter: select what messages to show     */
	uint32_t    				bDeviceStatus;		   /*!< TERMDEV parameter: device health status             */
	termdev_Callback		fpCallback;			 		 /*!< Callback function to receive messages 			        */
} termdev_handle_s;

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function : TDV_open(void)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to open the TERMDEV device.
*
* PRE-CONDITION: None
*
* POST-CONDITION: TERMDEV instance created.
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MyTermdev = DEV_open(PERIPHERAL_TERMDEV);
*   if (MyTermdev != NULL) PRINTF("MyTermdev ready !");
~~~~~~~~~~~~~~~
*
* @see TDV_open, TDV_write, TDV_read, TDV_ioctl and TDV_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s TDV_open(void);

/******************************************************************************
* Function : TDV_read(struct peripheral_descriptor_s* const this,
*        void * const vpBuffer,
*        const uint32_t tBufferSize)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to read remote messages coming from different interfaces.
* The function requires a buffer pointer and buffer size, to store the readed data. The return value
* is the number of bytes read.
*
* PRE-CONDITION: Interface (UART or USB or CAN) activated using ioctl;
*
* POST-CONDITION: Read performed
*
* @return     uint32_t number of data read
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MyTermdev = DEV_open(PERIPHERAL_TERMDEV);
*   if (MyTermdev != NULL) PRINTF("MyTermdev ready !");
*
*   uint32_t wMyInterface = TERMDEV_IFACE_UART;
*   DEV_ioctl(IOCTL_TDV_SET_ACTIVE, (void*)&wMyInterface);
*
*   uint8_t bBuffer[100];
*   uint32_t bytes_read = DEV_read(MyTermdev, &bBuffer, 100);
*   if (bytes_read)
*     PRINTF("data read!");
~~~~~~~~~~~~~~~
*
* @see TDV_open, TDV_write, TDV_read, TDV_ioctl and TDV_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t TDV_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize);

/******************************************************************************
* Function : TDV_write(peripheral_descriptor_p const this,
*    const void * vpBuffer,
*    const uint32_t tBufferSize)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to write generic data to a selected interface.
* The function requires a buffer containing the data to be written, and a size of
* data to write variable. The function return value is the number of bytes written.
*
* PRE-CONDITION: Interface (UART or USB or CAN) activated using ioctl;
*
* POST-CONDITION: Data written
*
* @return     uint32_t number of data written
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MyTermdev = DEV_open(PERIPHERAL_TERMDEV);
*   if (MyTermdev != NULL) PRINTF("MyTermdev ready !");
*
*   uint32_t wMyInterface = TERMDEV_IFACE_UART;
*   DEV_ioctl(IOCTL_TDV_SET_ACTIVE, (void*)&wMyInterface);
*
*   uint8_t bBuffer[] = {'t','e','s','t'};
*   uint32_t bytes_sent = DEV_write(MyTermdev, &bBuffer, 4);
*   if (bytes_read)
*     PRINTF("data read!");
~~~~~~~~~~~~~~~
*
* @see TDV_open, TDV_write, TDV_read, TDV_ioctl and TDV_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t TDV_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize);

/******************************************************************************
* Function : TDV_ioctl(peripheral_descriptor_p const this,
*    uint32_t wRequest,
*    void * vpValue)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to control a device (IO ConTroL). The device descriptor,
* a control request and control value must be passed into the function. Each device will have
* its own set of control requests, therefore the user must verify on each device which control
* is valid.
*
* For the TERMDEV device, the following IOCTL operations are possible:
* - IOCTL_TDV_SET_ACTIVE: Select which peripheral will be used by TERMDEV to perform input/output;
* - IOCTL_TDV_DISABLE: Disable a peripheral interface previously activated;
* - IOCTL_TDV_ADD_CAN_ID: Specific for use when CAN is active, adds a message ID number to
* 												CAN input filter and use this ID in the output messages;
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device controlled
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MyTermdev = DEV_open(PERIPHERAL_TERMDEV);
*   if (MyTermdev != NULL) PRINTF("MyTermdev ready !");
*
*   uint32_t wMyInterface = TERMDEV_IFACE_UART;
*   DEV_ioctl(IOCTL_TDV_SET_ACTIVE, (void*)&wMyInterface);
*
*   wMyInterface = TERMDEV_IFACE_CAN;
*   DEV_ioctl(IOCTL_TDV_SET_ACTIVE, (void*)&wMyInterface);
*
*   wMyInterface = TERMDEV_IFACE_UART;
*   DEV_ioctl(IOCTL_TDV_DISABLE, (void*)&wMyInterface);
*
*   uint16_t hCAN_id = 0x515;
*		DEV_ioctl(IOCTL_TDV_CAN_ADD_ID, (void*)&hCAN_id);
~~~~~~~~~~~~~~~
*
* @see TDV_open, TDV_write, TDV_read, TDV_ioctl and TDV_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s TDV_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue);


/******************************************************************************
* Function : TDV_close(struct peripheral_descriptor_s* const this)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to close TERMDEV instance, which, in turns,
* closes all active peripheral interfaces. The device descriptor must be passed.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device Closed
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p CBACOM = DEV_open(PERIPHERAL_TERMDEV);
*   if (MyTermdev != NULL) PRINTF("MyTermdev ready !");
*   DEV_close(MyTermdev);
~~~~~~~~~~~~~~~
*
* @see TDV_open, TDV_write, TDV_read, TDV_ioctl and TDV_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s TDV_close(struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
extern "C"{
#endif


#endif /* INC_TERMDEV_H_ */
