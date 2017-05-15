/****************************************************************************
* Title                 :   MLX75305 Include File
* Filename              :   MLX75305.h
* Author                :   thiago.palmieri
* Origin Date           :   14/03/2016
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
*  14/03/2016   1.0.0   thiago.palmieri MLX75305 include file Created.
*
*****************************************************************************/
/** @file MLX75305.h
 *  @brief This file provides the header file for light sensor MLX75305.
 *
 *  The public functions provided by this file must not be executed directly,
 *  they must be used through devicelib.h header file implementation.
 *
 */
#ifndef INC_MLX75305_H_
#define INC_MLX75305_H_

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
* Function : MLX_open(void)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to open the MLX75305 sensor. This function will
* init the ring buffer, set default initialization parameters and initialize
* the sensor interface towards mculib.
*
* PRE-CONDITION: None
*
* POST-CONDITION: Sensor initialized
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MLX75305 = DEV_open(DEVICE_MLX75305);
*   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s MLX_open(void);

/******************************************************************************
* Function : MLX_read(struct peripheral_descriptor_s* const this,
*        void * const vpBuffer,
*        const uint32_t tBufferSize)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to read the MLX75305 sensor. A Buffer/Buffer size
* must be passed to the function, this buffer will save the result of the read.
* If pooling method is used, then the function will read the sensor and return the
* result. If Interrupt/DMA without buffer method is used, then this function will
* return zero.
* In case of Buffered Interrupt/DMA, then the function will read the buffer and
* return the value. The return value is the number of bytes read.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: None
*
* @return     uint32_t number of data read
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MLX75305 = DEV_open(DEVICE_MLX75305);
*   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
*   uint16_t hBuffer[10];
*   uint32_t bytes_read = DEV_read(MLX75305, &hBuffer, 10);
*   if (bytes_read)
*     PRINTF("%d Bytes Read", bytes_read);
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t MLX_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize);

/******************************************************************************
* Function : MLX_write(peripheral_descriptor_p const this,
*    const void * vpBuffer,
*    const uint32_t tBufferSize);
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to write data to MLX75305. Since the device does not
* receive any information, then this function will always return zero.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: None
*
* @return     uint32_t number of data written (always ZERO)
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MLX75305 = DEV_open(DEVICE_MLX75305);
*   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
*   uint16_t hBuffer = 60000;
*   uint32_t bytes_write = DEV_write(MLX75305, &hBuffer, 1);
*   if (bytes_write)
*     PRINTF("%d Bytes Written", bytes_write);
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t MLX_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize);

/******************************************************************************
* Function : MLX_ioctl(peripheral_descriptor_p const this,
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
*   peripheral_descriptor_p MLX75305 = DEV_open(DEVICE_MLX75305);
*   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
*   eDEVError_s eError = DEV_ioctl(MLX75305, IOCTL_MLX_SET_RESOLUTION, (void*)ADC_RES_10BITS);
*   if (!eError) PRINTF("Resolution Changed"):
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s MLX_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue);

/******************************************************************************
* Function : MLX_close(peripheral_descriptor_p const this)
*//**
* \b Description:
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* This is a public function used to close MLX75305 sensor. The device descriptor
* must be passed. After this function is used, the sensor will be deinitialized.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device Closed
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p MLX75305 = DEV_open(DEVICE_MLX75305);
*   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
*   uint16_t hBuffer = 60000;
*   uint32_t bytes_write = DEV_write(MLX75305, &hBuffer, 1);
*   if (bytes_write)
*     PRINTF("%d Bytes Written", bytes_write);
*   DEV_close(MLX75305);
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eDEVError_s MLX_close(struct peripheral_descriptor_s* const this);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_MLX75305_H_ */

/*** End of File **************************************************************/
