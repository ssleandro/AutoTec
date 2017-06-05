/****************************************************************************
* Title                 :   USB CDC HAL
* Filename              :   usbcdc.c
* Author                :   Thiago Palmieri
* Origin Date           :   17/02/2016
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
/** @file usbcdc.c
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the source file for the definition of USB CDC functions
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include <usbcdc.h>
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "usbd_rom_api.h"
#include "cdc_vcom.h"
#ifdef USE_SYSVIEW
#include "SEGGER.h"
#include "SEGGER_SYSVIEW.h"
#endif

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define LPC_USB_BASE(x)            (sUSBMap[x].wLPCUSBBase)
#define LPC_USB(x)                 (sUSBMap[x].pUSB)
#define LPC_USB_IRQ(x)             (sUSBMap[x].eLPCUSBIRQ)
/******************************************************************************
* Module Typedefs
*******************************************************************************/
/**
 * This Struct holds private USB configuration
 */
typedef struct usb_private_config_s
{
  bool bStarted;                   //!< USB Channel is started
}usb_private_config_s;

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static USBD_API_INIT_PARAM_T usb_param; //!< This data structure is used to pass initialization parameters to the USB device stack's init function.
static USB_CORE_DESCS_T desc; //!< This structure contains pointers to various descriptor arrays needed by the stack
static USBD_HANDLE_T g_hUsb; //!< USB device stack/module handle

static usb_config_s * USBlist[USB_MAX_CHANNELS]; //!< List of enabled USB channels for use inside interrupts

/* Endpoint 0 patch that prevents nested NAK event processing */
static uint32_t g_ep0RxBusy = 0;        //!< Flag indicating whether EP0 OUT/RX buffer is busy.
static USB_EP_HANDLER_T g_Ep0BaseHdlr;  //!< variable to store the pointer to base EP0 handler

static usbMessageCallBack gCallback = NULL;

const USBD_API_T * g_pUsbApi; //!< Main entry point for Methods exposed by the ROM
/******************************************************************************
* Function Prototypes
*******************************************************************************/
extern void USB0_IRQHandler(void);
extern void USB1_IRQHandler(void);
/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : USB0_IRQHandler(void)
*//**
* \b Description:
*
* This is a IRQ handling function for the USB0.
*
* PRE-CONDITION: USB Channel must be initialized
*
* POST-CONDITION: None
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void USB0_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  USBD_API->hw->ISR(g_hUsb);
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/******************************************************************************
* Function : USB1_IRQHandler(void)
*//**
* \b Description:
*
* This is a IRQ handling function for the USB0.
*
* PRE-CONDITION: USB Channel must be initialized
*
* POST-CONDITION: None
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void USB1_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  USBD_API->hw->ISR(g_hUsb);
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/******************************************************************************
* Function : vUSBInternallCallback(void)
*//**
* \b Description:
*
* This function will be called everytime a message arrives at USB_EVT_OUT interface
* meaning that a message has arrived.
* This function handles a global function pointer, so only ONE USB port must be active.
*
* PRE-CONDITION: USB Channel must be initialized and a callback provided
*
* POST-CONDITION: Callback executed
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 29/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void vUSBInternallCallback(void)
{
  eUSBInterfaces_s eIterator = USB0;
  while(eIterator < USB_INVALID)
    {
      if ((USBlist[eIterator] != NULL) && (gCallback != NULL))
        {
          gCallback();
        }
      eIterator++;
    }

}
/******************************************************************************
* Function : ErrorCode_t EP0_patch(USBD_HANDLE_T hUsb, void *data, uint32_t event)
*//**
* \b Description:
*
* This is a private EP0_patch part of WORKAROUND for artf45032.
* WORKAROUND for artf45032 ROM driver BUG:
*            Due to a race condition there is the chance that a second NAK event will
*            occur before the default endpoint0 handler has completed its preparation
*            of the DMA engine for the first NAK event. This can cause certain fields
*            in the DMA descriptors to be in an invalid state when the USB controller
*            reads them, thereby causing a hang.
*
* PRE-CONDITION: Used in initialization only.
*
* POST-CONDITION: ROM BUG Fixed.
*
* @return     ErrorCode_t
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   //Not Available
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
/* EP0_patch part of WORKAROUND for artf45032. */
static ErrorCode_t EP0_patch(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
  switch (event) {
  case USB_EVT_OUT_NAK:
    if (g_ep0RxBusy) {
      /* we already queued the buffer so ignore this NAK event. */
      return LPC_OK;
    }
    else {
      /* Mark EP0_RX buffer as busy and allow base handler to queue the buffer. */
      g_ep0RxBusy = 1;
    }
    break;

  case USB_EVT_SETUP: /* reset the flag when new setup sequence starts */
  case USB_EVT_OUT:
    /* we received the packet so clear the flag. */
    g_ep0RxBusy = 0;
    break;
  }
  return g_Ep0BaseHdlr(hUsb, data, event);
}

eMCUError_s USB_eInit(usb_config_s * pUSB)
{
  //Verify if valid ADC/channel
  if ((pUSB->eUSB < USB_INVALID) && (pUSB->vpPrivateData == NULL) && (pUSB->eClass == USB_CLASS_CDC))
    {
      for (int i = 0; i < USB_MAX_CHANNELS; i++) //Verify if any USB was already initialized. (Limitation)
        {
          if (USBlist[i] != NULL)
            {
              return MCU_ERROR_USB_INVALID_USB;
            }
        }

      ErrorCode_t ret = LPC_OK;
      USB_CORE_CTRL_T *pCtrl;

      /* enable clocks and pinmux */
      if (pUSB->eUSB)
        {
          Chip_USB1_Init();
        }
      else
        {
          Chip_USB0_Init();
        }

      /* Init USB API structure */
      g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

      /* initialize call back structures */
      memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));

      usb_param.usb_reg_base = LPC_USB_BASE(pUSB->eUSB);
      usb_param.max_num_ep = 4;
      usb_param.mem_base = USB_STACK_MEM_BASE;
      usb_param.mem_size = USB_STACK_MEM_SIZE;

      /* Set the USB descriptors */
      desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
      desc.string_desc = (uint8_t *) USB_StringDescriptor;

      if (!pUSB->eUSB)
        {
          desc.high_speed_desc = USB_HsConfigDescriptor;
          desc.full_speed_desc = USB_FsConfigDescriptor;
          desc.device_qualifier = (uint8_t *) USB_DeviceQualifier;
        }
      else
        {
          /* Note, to pass USBCV test full-speed only devices should have both
         * descriptor arrays point to same location and device_qualifier set
         * to 0.
         */
          desc.high_speed_desc = USB_FsConfigDescriptor;
          desc.full_speed_desc = USB_FsConfigDescriptor;
          desc.device_qualifier = 0;
        }

      /* USB Initialization */
      ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
      if (ret == LPC_OK)
        {

          /*  WORKAROUND for artf45032 ROM driver BUG:
              Due to a race condition there is the chance that a second NAK event will
              occur before the default endpoint0 handler has completed its preparation
              of the DMA engine for the first NAK event. This can cause certain fields
              in the DMA descriptors to be in an invalid state when the USB controller
              reads them, thereby causing a hang.
           */
          pCtrl = (USB_CORE_CTRL_T *) g_hUsb; /* convert the handle to control structure */
          g_Ep0BaseHdlr = pCtrl->ep_event_hdlr[0];/* retrieve the default EP0_OUT handler */
          pCtrl->ep_event_hdlr[0] = EP0_patch;/* set our patch routine as EP0_OUT handler */

          /* Init VCOM interface */
          ret = vcom_init(pUSB->eUSB, g_hUsb, &desc, &usb_param);
          if (ret == LPC_OK)
            {
              /*  enable USB interrupts */
            	NVIC_SetPriority(LPC_USB_IRQ(pUSB->eUSB), 1);
              NVIC_EnableIRQ(LPC_USB_IRQ(pUSB->eUSB));
              /* now connect */
              USBD_API->hw->Connect(g_hUsb, 1);

              usb_private_config_s * psPrivate = malloc(sizeof(usb_private_config_s));
              psPrivate->bStarted = true;
              pUSB->vpPrivateData = psPrivate;
              if (pUSB->fpUSBCallBack)
                {
                  gCallback = pUSB->fpUSBCallBack;
                }

              USBlist[pUSB->eUSB] = pUSB;
              return MCU_ERROR_SUCCESS;
            }
        }
      return MCU_ERROR_USB_UNABLE_TO_INITIALIZE;
    }
  return MCU_ERROR_USB_INVALID_USB;
}

uint32_t USB_wBufferRead(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{
  if ((pUSB->eUSB < USB_INVALID) && (pUSB->vpPrivateData != NULL) && (pUSB->eClass == USB_CLASS_CDC))
    {
      return vcom_bread(pUSB->eUSB, pBuf, buf_len);
    }
  return 0;
}

eMCUError_s USB_eReadRequest(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{
  if ((pUSB->eUSB < USB_INVALID) && (pUSB->vpPrivateData != NULL) && (pUSB->eClass == USB_CLASS_CDC))
    {
      if (!vcom_read_req(pUSB->eUSB, pBuf, buf_len))
        {
          return MCU_ERROR_SUCCESS;
        }
      return MCU_ERROR_USB_BUSY;
    }
  return MCU_ERROR_USB_INVALID_USB;
}

uint32_t USB_wReadCount(const usb_config_s * pUSB)
{
  if ((pUSB->eUSB < USB_INVALID) && (pUSB->vpPrivateData != NULL) && (pUSB->eClass == USB_CLASS_CDC))
    {
      return vcom_read_cnt();
    }
  return 0;
}

uint32_t USB_wWrite(const usb_config_s * pUSB, uint8_t *pBuf, uint32_t buf_len)
{
  if ((pUSB->eUSB < USB_INVALID) && (pUSB->vpPrivateData != NULL) && (pUSB->eClass == USB_CLASS_CDC))
    {
      return vcom_write(pUSB->eUSB, pBuf, buf_len);
    }
  return 0;
}

uint32_t USB_wConnected()
{
  return vcom_connected();
}

/* TODO: disable mcu USB registers */
eMCUError_s USB_eDeInit(usb_config_s *pUSB)
{
	/* Remove the selected USB from USBList */
	USBlist[pUSB->eUSB] = NULL;

	/* Calls the API function to disable USB */
	USBD_API->hw->ResetEP(&g_hUsb, USB_CDC_IN_EP);
	USBD_API->hw->ResetEP(&g_hUsb, USB_CDC_OUT_EP);

	/* Clears the private data */
	free((usb_private_config_s*) pUSB->vpPrivateData);
	pUSB->vpPrivateData = NULL;

	return MCU_ERROR_SUCCESS;
}
