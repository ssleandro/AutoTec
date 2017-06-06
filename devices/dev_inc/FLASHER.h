/****************************************************************************
 * Title                 :   FLASH Include File
 * Filename              :   FLASH.h
 * Author                :   thiago.palmieri
 * Origin Date           :   12 de abr de 2016
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
 *  12 de abr de 2016   1.0.0   thiago.palmieri FLASH include file Created.
 *
 *****************************************************************************/
/** @file FLASH.h
 *  @brief This file provides Access to External Flash for programming, reading
 *  and erasing.
 *
 *  After device initialization, the External Flash will be available for reading
 *  via pointer addresses as usual.
 *
 *  In order to program some value on the memory, three steps must be taken.
 *  1. Set the starting programming address via IOCTL command
 *  2. Erase the Interval to be programmed
 *  3. Finally, program the address via WRITE command
 *
 *  Reading can be made in one or two steps.
 *  1. Reading via pointer to address, as usual
 *  or
 *  1a.Set the Reading Address via IOCTL command
 *  2a.Use READ command to read a number of bytes
 *
 */
#ifndef INC_FLASH_H_
#define INC_FLASH_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "devicelib.h"
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#if defined (UNITY_TEST)
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
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
extern "C"
{
#endif

/******************************************************************************
 * Function : FLS_open(void)
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to open the FLASH interface.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: Flash initialized
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p pFLASH = DEV_open(PERIPHERAL_FLASH);
 *   if (pFLASH != NULL) PRINTF("FLASH ready !");
 ~~~~~~~~~~~~~~~
 *
 * @see FLS_open, FLS_write, FLS_read, FLS_ioctl and FLS_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 13/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s FLS_open (void);

/******************************************************************************
 * Function : FLS_read(struct peripheral_descriptor_s* const this,
 *        void * const vpBuffer,
 *        const uint32_t tBufferSize)
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to read the FLASH. The function
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
 *   peripheral_descriptor_p pFLASH = DEV_open(PERIPHERAL_FLASH);
 *   if (pFLASH != NULL) PRINTF("FLASH ready !");
 *   uint16_t hBuffer[10];
 *   uint32_t bytes_read = DEV_read(pFLASH, &hBuffer, 10);
 *   if (bytes_read)
 *     PRINTF("Data read!");
 ~~~~~~~~~~~~~~~
 *
 * @see FLS_open, FLS_write, FLS_read, FLS_ioctl and FLS_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 13/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t FLS_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize);

/******************************************************************************
 * Function : FLS_write(peripheral_descriptor_p const this,
 *    const void * vpBuffer,
 *    const uint32_t tBufferSize);
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to write data to FLASH interface.
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
 *   peripheral_descriptor_p pFLASH = DEV_open(PERIPHERAL_FLASH);
 *   if (pFLASH != NULL) PRINTF("FLASH ready !");
 *   uint16_t hBuffer = 60000;
 *   uint32_t bytes_write = DEV_write(pFLASH, &hBuffer, 1);
 *   if (bytes_write)
 *     PRINTF("Data sent !");
 ~~~~~~~~~~~~~~~
 *
 * @see FLS_open, FLS_write, FLS_read, FLS_ioctl and FLS_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 13/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t FLS_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize);

/******************************************************************************
 * Function : FLS_ioctl(peripheral_descriptor_p const this,
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
 *   peripheral_descriptor_p pFLASH = DEV_open(PERIPHERAL_FLASH);
 *   if (pFLASH != NULL) PRINTF("FLASH ready !");
 *   uint8_t interface = 1;
 *   eDEVError_s eError = DEV_ioctl(pFLASH, IOCTL_FLS_SET_INTERFACE, (void*)&interface);
 *   if (!eError) PRINTF("Interface changed"):
 ~~~~~~~~~~~~~~~
 *
 * @see FLS_open, FLS_write, FLS_read, FLS_ioctl and FLS_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 13/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s FLS_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue);

/******************************************************************************
 * Function : FLS_close(peripheral_descriptor_p const this)
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to close the FLASH interface. The device descriptor
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
 *   peripheral_descriptor_p pFLASH = DEV_open(PERIPHERAL_FLASH);
 *   if (pFLASH != NULL) PRINTF("FLASH ready !");
 *   DEV_close(pFLASH);
 ~~~~~~~~~~~~~~~
 *
 * @see FLS_open, FLS_write, FLS_read, FLS_ioctl and FLS_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 13/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s FLS_close (struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_FLASH_H_ */

/*** End of File **************************************************************/
