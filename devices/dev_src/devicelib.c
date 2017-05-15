/****************************************************************************
* Title                 :   devicelib Include File
* Filename              :   devicelib.c
* Author                :   thiago.palmieri
* Origin Date           :   15 de mar de 2016
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
*  15 de mar de 2016   1.0.0   thiago.palmieri devicelib include file Created.
*
*****************************************************************************/
/** @file devicelib.c
 *  @brief This file provides implementation of device control layer. For each device
 *  created, include its header file here.
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "devicelib.h"
#include <stdbool.h>
#include <TERMDEV.h>
#include "MLX75305.h"
#include "LCDBACKLIGHT.h"
#include "KEYBACKLIGHT.h"
#include "CBACOMM.h"
#include "FLASHER.h"
#include "WATCHDOG.h"
#include "M2GISOCOMM.h"
#include "M2GSENSORCOMM.h"
#include "M2GGPSCOMM.h"
#include "CBT09427.h"
#include "EXTBUZZER.h"
#include "CHIPPROG.h"

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#ifndef NULL
#define NULL (void*)0
#endif
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
/**
 * This Typedef holds all peripheral IDs
 */
#define X(a, fopen, fread, fwrite, fioctl, fclose) {.open = fopen, .read = fread, .write = fwrite, \
    .ioctl = fioctl, .close = fclose, .bOpen = false},
peripheral_descriptor_s sDescriptor[] =
{
    DEVICE_MAPPING
};
#undef X
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
peripheral_descriptor_p DEV_open(peripheralID_e eID)
{
  if (eID >= PERIPHERAL_INVALID)
    {
      return NULL;
    }
  if (sDescriptor[eID].bOpen) //If it is open, return NULL
    {
      return NULL;
    }
  if (sDescriptor[eID].open()) //If it did not open, return NULL
    {
      return NULL;
    }
  sDescriptor[eID].bOpen = true;
  return &sDescriptor[eID];
}

uint32_t DEV_read(peripheral_descriptor_p this,
      void * const vpBuffer,
      const uint32_t tBufferSize)
{
  if ((this == NULL) || (vpBuffer == NULL))
    {
      return 0;
    }
  return this->read(this, vpBuffer, tBufferSize);
}

uint32_t DEV_write(peripheral_descriptor_p this,
    const void * vpBuffer,
    const uint32_t tBufferSize)
{
  if (this == NULL)
    {
      return 0;
    }
  return this->write(this, vpBuffer, tBufferSize);
}

eDEVError_s DEV_ioctl(peripheral_descriptor_p this,
    uint32_t wRequest,
    void * vpValue)
{
  if (this == NULL)
    {
      return DEV_ERROR_INVALID_DESCRIPTOR;
    }
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  return this->ioctl(this, wRequest, vpValue);
}

eDEVError_s DEV_close(peripheral_descriptor_p this)
{
  eDEVError_s eError = DEV_ERROR_INVALID_DESCRIPTOR;
  if (this == NULL)
    {
      return eError;
    }

  if (this->bOpen)
    {
      eError = this->close(this);
      if (!eError)
        {
          //Release ID
          this->bOpen = false;
        }
    }
  return eError;
}

/******************************************************************************
* Unity Testing
*******************************************************************************/
