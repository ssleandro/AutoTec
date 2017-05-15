/****************************************************************************
* Title                 :   CBACOMM Include File
* Filename              :   CBACOMM.h
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
*  18 de mar de 2016   1.0.0   thiago.palmieri CBACOMM include file Created.
*
*****************************************************************************/
/** @file CBACOMM.h
 *  @brief This file provides Control device for CBA Communication.
 *
 *  This device must be used to initiate communication towards CBA CPU. The
 *  default interface is CAN, but it can be changed to UART or USB on the fly.
 *
 *  For each interface, a proper command sequence must be used to properly initiate it.
 *
 *  CAN:
 *  - First the interface must be opened (DEV_open).
 *  - Then message IDs must be added for monitoring (DEV_ioctl - one command per message ID).
 *  - After the first ID is added, then the interface is initialized.
 *  - In order to send CAN messages, DEV_ioctl must be used previously to inform message ID.
 *
 *  USB / UART:
 *  - First the interface must be opened (DEV_open).
 *  - After opening the interface, it will be ready for usage.
 *
 *  In order to close the interface, command DEV_close must be used.
 *
 *  Commands DEV_read, DEV_write must be used to read/write data from the interface
 *  respectively.
 *
 *  The interface works via message buffers, therefore if the message is larger than
 *  the available payload for the interface, it will be split in separate messages.
 *
 *  The default reading method is pooling, if interrupt method need to be used,
 *  the DEV_ioctl must be used to add an Interrupt CallBack. If you need to return to
 *  pooling method after defining a callback, then update the callback with NULL.
 *
 *  Check the _config.h file for defaults.
 *
 */
#ifndef INC_CBACOMM_H_
#define INC_CBACOMM_H_

/******************************************************************************
* Includes
*******************************************************************************/
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

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
* Function : CCM_open(void)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to open the CBA Communicator interface.
*
* PRE-CONDITION: None
*
* POST-CONDITION: Sensor initialized
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p CBACOM = DEV_open(DEVICE_CBACOMM);
*   if (CBACOM != NULL) PRINTF("CBACOM ready !");
~~~~~~~~~~~~~~~
*
* @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
eDEVError_s CCM_open(void);

/******************************************************************************
* Function : CCM_read(struct peripheral_descriptor_s* const this,
*        void * const vpBuffer,
*        const uint32_t tBufferSize)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to read the CBA Communicator interface. The function
* requires a buffer pointer and buffer size, to store the readed data. The return value
* is the number of bytes read.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Read performed
*
* @return     uint32_t number of data read
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p CBACOM = DEV_open(DEVICE_CBACOM);
*   if (CBACOM != NULL) PRINTF("CBACOM ready !");
*   uint16_t hBuffer[10];
*   uint32_t bytes_read = DEV_read(CBACOM, &hBuffer, 10);
*   if (bytes_read)
*     PRINTF("CBA data read!");
~~~~~~~~~~~~~~~
*
* @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
uint32_t CCM_read(struct peripheral_descriptor_s* const this,
       void * const vpBuffer,
       const uint32_t tBufferSize);

/******************************************************************************
* Function : CCM_write(peripheral_descriptor_p const this,
*    const void * vpBuffer,
*    const uint32_t tBufferSize);
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to write data to CBA Communicator interface.
* The function requires a buffer containing the data to be written, and a size of
* data to write variable. The function return value is the number of bytes written.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Data written
*
* @return     uint32_t number of data written
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p CBACOM = DEV_open(DEVICE_CBACOM);
*   if (CBACOM != NULL) PRINTF("CBACOM ready !");
*   uint16_t hBuffer = 60000;
*   uint32_t bytes_write = DEV_write(CBACOM, &hBuffer, 1);
*   if (bytes_write)
*     PRINTF("Data sent !");
~~~~~~~~~~~~~~~
*
* @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
uint32_t CCM_write(struct peripheral_descriptor_s* const this,
     const void * vpBuffer,
     const uint32_t tBufferSize);

/******************************************************************************
* Function : CCM_ioctl(peripheral_descriptor_p const this,
*    uint32_t wRequest,
*    void * vpValue);
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
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device controlled
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p CBACOM = DEV_open(DEVICE_CBACOM);
*   if (CBACOM != NULL) PRINTF("CBACOM ready !");
*   uint8_t interface = 1;
*   eDEVError_s eError = DEV_ioctl(CBACOM, IOCTL_CCM_SET_INTERFACE, (void*)&interface);
*   if (!eError) PRINTF("Interface changed"):
~~~~~~~~~~~~~~~
*
* @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
eDEVError_s CCM_ioctl(struct peripheral_descriptor_s* const this,
     uint32_t wRequest,
     void * vpValue);

/******************************************************************************
* Function : CCM_close(peripheral_descriptor_p const this)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to close the CBA Communicator interface. The device descriptor
* must be passed. After this function is used, the CBA communicator interface will be closed.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device Closed
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p CBACOM = DEV_open(DEVICE_CBACOM);
*   if (CBACOM != NULL) PRINTF("CBACOM ready !");
*   DEV_close(CBACOM);
~~~~~~~~~~~~~~~
*
* @see CCM_open, CCM_write, CCM_read, CCM_ioctl and CCM_close
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
eDEVError_s CCM_close(struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_CBACOMM_H_ */

/*** End of File **************************************************************/
