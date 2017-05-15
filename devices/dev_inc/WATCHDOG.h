/****************************************************************************
* Title                 :   WATCHDOG Include File
* Filename              :   WATCHDOG.h
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
*  6 de mai de 2016   1.0.0   thiago.palmieri WATCHDOG include file Created.
*
*****************************************************************************/
/** @file WATCHDOG.h
 *  @brief This file provides Watchdog implementation.
 *
 *  In order to properly use the watchdog the following steps must be taken:
 *  1. Open the watchdog interface
 *  2. Configure it
 *  3. Start the interface
 *  4. keep feeding the dog
 *
 *  The current watchdog implementation cannot be disabled once started.
 *
 *  When the interface is opened, the default configuration is applied, meaning that
 *  the default timeout time (WDT_DEFAULT_TIMEOUT_IN_MILLIS) will be used and
 *  when watchdog expires, a reset will take place.
 *
 *  If the default implementation need to be changed, then IOCTL interface can be used
 *  to set a new timeout value and to change the expiration mode action.
 *
 *  If callback mode is chosen, then a callback must be provided and when the timer
 *  expires, the callback is executed and the watchdog is restarted. If the user needs
 *  to restart the chip in this mode, then it is up to the user to perform such action.
 *
 *  The reset can be forced via IOCTL interface.
 *
 *  To feed the dog, the IOCTL interface need to be used.
 *
 *  The user can check a previous restart reason (watchdog restart or user restart) via
 *  IOCTL interface as well.
 *
 */
#ifndef DEVICES_DEV_INC_WATCHDOG_H_
#define DEVICES_DEV_INC_WATCHDOG_H_

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
* Function : WDT_open(void)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to open the watchdog.
*
* PRE-CONDITION: None
*
* POST-CONDITION: Sensor initialized
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p pWDT = DEV_open(PERIPHERAL_WATCHDOG);
*   if (pWDT != NULL) PRINTF("WATCHDOG ready !");
~~~~~~~~~~~~~~~
*
* @see WDT_open, WDT_write, WDT_read, WDT_ioctl and WDT_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td>        </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s WDT_open(void);

/******************************************************************************
* Function : WDT_read(struct peripheral_descriptor_s* const this,
*        void * const vpBuffer,
*        const uint32_t tBufferSize)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to read the watchdog. The function
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
*   peripheral_descriptor_p pWDT = DEV_open(PERIPHERAL_WATCHDOG);
*   if (pWDT != NULL) PRINTF("WATCHDOG ready !");
*   uint16_t hBuffer[10];
*   uint32_t bytes_read = DEV_read(pWDT, &hBuffer, 10);
*   if (bytes_read)
*     PRINTF("Data read!");
~~~~~~~~~~~~~~~
*
* @see WDT_open, WDT_write, WDT_read, WDT_ioctl and WDT_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td>        </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t WDT_read(struct peripheral_descriptor_s* const this,
       void * const vpBuffer,
       const uint32_t tBufferSize);

/******************************************************************************
* Function : WDT_write(peripheral_descriptor_p const this,
*    const void * vpBuffer,
*    const uint32_t tBufferSize);
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to write data to watchdog interface.
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
*   peripheral_descriptor_p pWDT = DEV_open(PERIPHERAL_WATCHDOG);
*   if (pWDT != NULL) PRINTF("WATCHDOG ready !");
*   uint16_t hBuffer = 60000;
*   uint32_t bytes_write = DEV_write(pWDT, &hBuffer, 1);
*   if (bytes_write)
*     PRINTF("Data sent !");
~~~~~~~~~~~~~~~
*
* @see WDT_open, WDT_write, WDT_read, WDT_ioctl and WDT_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td>        </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t WDT_write(struct peripheral_descriptor_s* const this,
     const void * vpBuffer,
     const uint32_t tBufferSize);

/******************************************************************************
* Function : WDT_ioctl(peripheral_descriptor_p const this,
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
*   peripheral_descriptor_p pWDT = DEV_open(PERIPHERAL_WATCHDOG);
*   if (pWDT != NULL) PRINTF("WATCHDOG ready !");
*   uint8_t interface = 1;
*   eDEVError_s eError = DEV_ioctl(pWDT, IOCTL_WDT_SET_INTERFACE, (void*)&interface);
*   if (!eError) PRINTF("Interface changed"):
~~~~~~~~~~~~~~~
*
* @see WDT_open, WDT_write, WDT_read, WDT_ioctl and WDT_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td>        </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s WDT_ioctl(struct peripheral_descriptor_s* const this,
     uint32_t wRequest,
     void * vpValue);

/******************************************************************************
* Function : WDT_close(peripheral_descriptor_p const this)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to close the watchdog interface. The device descriptor
* must be passed. After this function is used, the interface will be closed.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device Closed
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p pWDT = DEV_open(PERIPHERAL_WATCHDOG);
*   if (pWDT != NULL) PRINTF("WATCHDOG ready !");
*   DEV_close(pWDT);
~~~~~~~~~~~~~~~
*
* @see WDT_open, WDT_write, WDT_read, WDT_ioctl and WDT_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td>        </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s WDT_close(struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DEVICES_DEV_INC_WATCHDOG_H_ */

/*** End of File **************************************************************/
