/****************************************************************************
* Title                 :   USB CDC HAL
* Filename              :   usbcdc.h
* Author                :   Thiago Palmieri
* Origin Date           :   18/02/2016
* Version               :   1.0.1
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
*    Date    Version   Author         		Description
*  18/02/16   1.0.0   Thiago Palmieri 		USB CDC HAL Created.
*  10/03/16   1.0.1   Joao Paulo Martins	Added USB_DeInit function.
*
*****************************************************************************/
/** @file usbcdc.h
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of USB CDC functions.
 *
 *  The current implementation limits the use to a single USB port and limit the
 *  use of USB as CDC Virtual COM Class.
 *
 *  The communication speed will be limited to the USB speed (FS or HS).
 *
 *  If a callback is passed during initialization, them the USB message receiving
 *  will be made via Interrupts, through the callback. If it is not passed, then
 *  it is up to the application to query the USB for messages received.
 */
#ifndef ABS_INC_USBCDC_H_
#define ABS_INC_USBCDC_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <inttypes.h>
#include "mcuerror.h"

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
typedef void (* usbMessageCallBack)(void);  //!< USB Callback typedef
/**
 * This enumeration is a list of possible USB ports on the Board
 */
typedef enum eUSBInterfaces_s
{
  USB0,               //!< USB0 Interface
  USB1,               //!< USB1 Interface
  USB_INVALID         //!< Invalid USB Interface
} eUSBInterfaces_s;

/**
 * This enumeration is a list of possible USB classes on the Board
 */
typedef enum eUSBClasses_s
{
  USB_CLASS_CDC,      //!< USB CDC Class
  USB_CLASS_HID,      //!< USB HID Class
  USB_CLASS_DFU,      //!< USB DFU Class
  USB_CLASS_MSD,      //!< USB MSD Class
  USB_CLASS_INVALID   //!< USB Invalid Class
} eUSBClasses_s;

/**
 * This Struct holds USB configuration for given board
 */
typedef struct usb_config_s
{
  eUSBInterfaces_s eUSB;      //!< USB port to use
  eUSBClasses_s eClass;       //!< USB Class to use (Currently fixed at CDC)
  usbMessageCallBack fpUSBCallBack;  //!< USB Callback
  void * vpPrivateData;       //!< Pointer to private data
} usb_config_s;


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
* Function : USB_eInit(usb_config_s * eUSB)
*//**
* \b Description:
*
* This is a public function used to initialize a USB CDC port.
* A usb_config_s struct pointer must be passed to the function.
* So far, only one USB interface can be initialized, so an error will be returned
* if a second USB port is initialized.
*
* PRE-CONDITION: Valid USB port.
*
* POST-CONDITION: USB Port initialized and operational.
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   usb_config_s sUSBport;
*   sUSBport.eUSB = USB0;
*
*   if (MCU_ERROR_SUCCESS == USB_bInit(&sUSBport))
*     PRINTF("USB0 Initiated");
~~~~~~~~~~~~~~~
*
* @see USB_eInit, USB_wBufferRead, USB_eReadRequest, USB_wReadCount, USB_wWrite, USB_wConnected
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 19/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s USB_eInit(usb_config_s * pUSB);

eMCUError_s USB_eDeInit(usb_config_s *pUSB);

/******************************************************************************
* Function : USB_wBufferRead(usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
*//**
* \b Description:
*
* This is a public function used to read a Buffer from USB CDC port.
* A buffer and it's size must be passed to the function. If there is any message
* on the read ROM Buffer then the function will return the number of bytes on it
* and copy the buffer to the one provided on the function
* If Internal buffer is empty, them the function will return Zero.
*
* PRE-CONDITION: Valid USB port and a buffer to hold the message.
*
* POST-CONDITION: Buffer will hold the message, if any.
*
* @return     Size of message read
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   usb_config_s sUSBport;
*   sUSBport.eUSB = USB0;
*
*   if (MCU_ERROR_SUCCESS == USB_bInit(&sUSBport))
*     PRINTF("USB0 Initiated");
*   uint32_t rdCnt = USB_wBufferRead(&usb0, &g_rxBuff[0], 256);
*   if (rdCnt) PRINTF("USB0 HAS MESSAGE Of SIZE %d", rdCnt);
~~~~~~~~~~~~~~~
*
* @see USB_eInit, USB_wBufferRead, USB_eReadRequest, USB_wReadCount, USB_wWrite, USB_wConnected
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 19/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t USB_wBufferRead(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len);

/******************************************************************************
* Function : USB_eReadRequest(usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
*//**
* \b Description:
*
* This is a public function used to read a USB CDC port.
* A buffer and it's size must be passed to the function. If the read was succesfull,
* then the function will return the data readed on the buffer, and a SUCCESS message.
* If there was nothing to read, them the request will be queued until data arrives.
* If there is already another Read queued, then the function will return an error.
*
* PRE-CONDITION: Valid USB port and a buffer to hold the message.
*
* POST-CONDITION: Buffer will hold the message, if any.
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   usb_config_s sUSBport;
*   sUSBport.eUSB = USB0;
*
*   if (MCU_ERROR_SUCCESS == USB_bInit(&sUSBport))
*     PRINTF("USB0 Initiated");
*   if (!USB_eReadRequest(&usb0, &g_rxBuff[0], 256))
*     PRINTF("USB0 HAS MESSAGE: %s", g_rxBuff);
~~~~~~~~~~~~~~~
*
* @see USB_eInit, USB_wBufferRead, USB_eReadRequest, USB_wReadCount, USB_wWrite, USB_wConnected
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 19/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s USB_eReadRequest(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len);

/******************************************************************************
* Function : USB_wReadCount(void)
*//**
* \b Description:
*
* This is a public function used get the Read Count of a USB CDC port.
*
* PRE-CONDITION: USB Interface initialized
*
* POST-CONDITION: None
*
* @return     Number of Bytes on Read Buffer
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   usb_config_s sUSBport;
*   sUSBport.eUSB = USB0;
*
*   if (MCU_ERROR_SUCCESS == USB_bInit(&sUSBport))
*     PRINTF("USB0 Initiated");
*   uint32_t rdCnt = USB_wReadCount();
*   if (rdCnt) PRINTF("USB0 HAS %d BYTES ON THE READ BUFFER", rdCnt);
~~~~~~~~~~~~~~~
*
* @see USB_eInit, USB_wBufferRead, USB_eReadRequest, USB_wReadCount, USB_wWrite, USB_wConnected
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 19/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t USB_wReadCount(const usb_config_s * pUSB);

/******************************************************************************
* Function : USB_wWrite(usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
*//**
* \b Description:
*
* This is a public function used to write to a USB CDC port.
* A buffer holding the message and and the size of message must be passed to the function.
* If the write was succesfull, the function Returns the number of bytes written
*
* PRE-CONDITION: Valid USB port and a buffer holding the message.
*
* POST-CONDITION: Message sent.
*
* @return     Returns the number of bytes written
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   usb_config_s sUSBport;
*   sUSBport.eUSB = USB0;
*
*   if (MCU_ERROR_SUCCESS == USB_bInit(&sUSBport))
*     PRINTF("USB0 Initiated");
*   uint8_t bMessage[] = {'H','E','L','L','O'};
*   uint32_t rdCnt = USB_wWrite(&usb0, &bMessage, 5);
*   if (rdCnt) PRINTF("USB0 HAS SENT %d BYTES OF MESSAGE", rdCnt);
~~~~~~~~~~~~~~~
*
* @see USB_eInit, USB_wBufferRead, USB_eReadRequest, USB_wReadCount, USB_wWrite, USB_wConnected
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 19/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t USB_wWrite(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len);

/******************************************************************************
* Function : uint32_t USB_wConnected(void)
*//**
* \b Description:
*
* This is a public function used to verify if VCOM is connected.
*
* PRE-CONDITION: None.
*
* POST-CONDITION: None.
*
* @return  Returns non-zero value if connected
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   usb_config_s sUSBport;
*   sUSBport.eUSB = USB0;
*
*   if (MCU_ERROR_SUCCESS == USB_bInit(&sUSBport))
*     PRINTF("USB0 Initiated");
*   uint8_t bMessage[] = {'H','E','L','L','O'};
*   if (USB_wConnected())
*   {
*     uint32_t rdCnt = USB_wWrite(&usb0, &bMessage, 5);
*     if (rdCnt) PRINTF("USB0 HAS SENT %d BYTES OF MESSAGE", rdCnt);
*   } else PRINTF ("USB Interface Disconnected");
~~~~~~~~~~~~~~~
*
* @see USB_eInit, USB_wBufferRead, USB_eReadRequest, USB_wReadCount, USB_wWrite, USB_wConnected
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 19/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint32_t USB_wConnected(void);

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* ABS_INC_USBCDC_H_ */
