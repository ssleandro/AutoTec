/****************************************************************************
* Title                 :   spifi Include File
* Filename              :   spifi.c
* Author                :   thiago.palmieri
* Origin Date           :   11 de abr de 2016
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
*  11 de abr de 2016   1.0.0   thiago.palmieri spifi include file Created.
*
*****************************************************************************/
/** @file spifi.c
 *  @brief This file provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer for SPIFI access.
 *
 *  This is the header file for the definition of SPIFI functions
 *
 *  the SPIFI interface must be initialized prior to its usage. The initialization
 *  is made via Init interface, where a handler must be passed with initial configuration.
 *
 *  After initialization, the memory can be accessed for reading via a pointer to address.
 *
 *  In order to erase the memory, the Erase function must be used, passing the type of
 *  erase (address or block) and the relevant data.
 *
 *  To write data into a given address, the Program function must be used.
 *
 *  There is also a Read function that can be used to read a given address via command
 *  mode.
 *
 *  NOTE: Before programming a given address, it must be erased.
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include <spifi.h>
//#include "spifilib_dev.h" //TODO: commented line, change to spifi.h
//#include "spifilib_api.h" //TODO: commented line, change to spifi.h
#include "board.h"
#include <string.h>
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

/******************************************************************************
* Module Typedefs
*******************************************************************************/
/**
 * This Struct holds private CAN configuration
 */
//TODO: comment the next four line, and add this to spifi.h
//typedef struct spifi_private_config_s
//{
//  SPIFI_HANDLE_T * pThisHandle;
//}spifi_private_config_s;
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static bool bSPIFIInitialized = false;    //!< Variable indicating that SPIFI was already initialized

// TODO: static removed, just for test
spifi_private_config_s sSPIFIPrivate =
    {
        .pThisHandle = NULL,
    };                                    //!< Private SPIFI Configuration data

static uint32_t wMem[21];                 //!< Memory to be used for init
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/

eMCUError_s SPF_eInit(spifi_config_s *pSPF)
{
  if ((pSPF != NULL) && (pSPF->eSPIFI < SPIFI_INVALID) && !bSPIFIInitialized && (pSPF->wSPIFIBaseAddress != 0))
    {
      BRD_SPIFIConfig();  //Board Config

      spifiInit(sSPIFIMap[pSPF->eSPIFI].bSPIFIAddress, true);  //Initialize
      spifiRegisterFamily(spifi_REG_FAMILY_CommonCommandSet); //Register Flash Family
      //Initialize Handle
      if (sSPIFIPrivate.pThisHandle == NULL)
        {
          sSPIFIPrivate.pThisHandle = spifiInitDevice(&wMem, sizeof(wMem),
                                                      sSPIFIMap[pSPF->eSPIFI].bSPIFIAddress,
                                                      pSPF->wSPIFIBaseAddress);
        }

      if (sSPIFIPrivate.pThisHandle == NULL)
        {
          return MCU_ERROR_SPIFI_INVALID_INITIALIZATION;
        }
      spifiDevSetOpts(sSPIFIPrivate.pThisHandle, SPIFI_OPT_USE_QUAD, true); //Use QUAD SPI

      spifiDevSetMemMode(sSPIFIPrivate.pThisHandle, true);  //Set Memory mode

      pSPF->vpPrivateData = &sSPIFIPrivate;
      bSPIFIInitialized = true;


      return MCU_ERROR_SUCCESS;
    }
  return MCU_ERROR_SPIFI_INVALID_CHANNEL;
}

eMCUError_s SPF_eDeInit(spifi_config_s *pSPF)
{
  if ((pSPF != NULL) && (pSPF->eSPIFI < SPIFI_INVALID) && bSPIFIInitialized)
    {
      spifiDevDeInit(sSPIFIPrivate.pThisHandle);
      sSPIFIPrivate.pThisHandle = NULL;
      bSPIFIInitialized = false;
      pSPF->vpPrivateData = NULL;
    }
  return MCU_ERROR_SUCCESS;
}

eMCUError_s SPF_eRead(const spifi_config_s *pSPF, uint32_t addr, uint32_t *readBuff, uint32_t bytes)
{
  eMCUError_s eError = MCU_ERROR_SPIFI_READ_ERROR;
  if ((pSPF != NULL) && (pSPF->eSPIFI < SPIFI_INVALID) && bSPIFIInitialized && (readBuff != NULL))
    {
      spifi_private_config_s * sThisPrivate = (spifi_private_config_s*)pSPF->vpPrivateData;
      spifiDevSetMemMode(sThisPrivate->pThisHandle, false);  //Set Command mode

      if (spifiRead(sThisPrivate->pThisHandle, addr, readBuff, bytes) == SPIFI_ERR_NONE)
        {
          eError = MCU_ERROR_SUCCESS;
        }

      spifiDevSetMemMode(sThisPrivate->pThisHandle, true);  //Set Memory mode
    }
  return eError;
}

eMCUError_s SPF_eProgram(const spifi_config_s *pSPF, uint32_t addr, const uint32_t *writeBuff, uint32_t bytes)
{
  eMCUError_s eError = MCU_ERROR_SPIFI_PROGRAM_ERROR;
  if ((pSPF != NULL) && (pSPF->eSPIFI < SPIFI_INVALID) && bSPIFIInitialized && (writeBuff != NULL))
    {
      __disable_irq();
      spifi_private_config_s * sThisPrivate = (spifi_private_config_s*)pSPF->vpPrivateData;
      spifiDevSetMemMode(sThisPrivate->pThisHandle, false);  //Set Command mode

      if (spifiProgram(sThisPrivate->pThisHandle, addr, writeBuff, bytes) == SPIFI_ERR_NONE)
        {
          eError = MCU_ERROR_SUCCESS;
        }
      spifiDevSetMemMode(sThisPrivate->pThisHandle, true);  //Set Memory mode
      __enable_irq();
    }
  return eError;
}

eMCUError_s SPF_eErase(const spifi_config_s *pSPF, uint8_t EraseAddress, uint32_t firstAddrOrBlock, uint32_t lastAddrOrNumBlocks)
{
  eMCUError_s eError = MCU_ERROR_SPIFI_ERASE_ERROR;
  if ((pSPF != NULL) && (pSPF->eSPIFI < SPIFI_INVALID) && bSPIFIInitialized)
    {
      __disable_irq();
      spifi_private_config_s * sThisPrivate = (spifi_private_config_s*)pSPF->vpPrivateData;
      spifiDevSetOpts(sSPIFIPrivate.pThisHandle, SPIFI_OPT_USE_QUAD, false); //Disable QUAD SPI
      spifiDevSetMemMode(sThisPrivate->pThisHandle, false);  //Set Command mode

      if (EraseAddress)
        {
          if (spifiEraseByAddr(sThisPrivate->pThisHandle, firstAddrOrBlock, lastAddrOrNumBlocks) == SPIFI_ERR_NONE)
            {
              eError = MCU_ERROR_SUCCESS;
            }
        }
      else
        {
          if (spifiErase(sThisPrivate->pThisHandle, firstAddrOrBlock, lastAddrOrNumBlocks) == SPIFI_ERR_NONE)
            {
              eError = MCU_ERROR_SUCCESS;
            }
        }

      spifiDevSetMemMode(sThisPrivate->pThisHandle, true);  //Set Memory mode
      spifiDevSetOpts(sSPIFIPrivate.pThisHandle, SPIFI_OPT_USE_QUAD, true); //Use QUAD SPI
      __enable_irq();
    }
  return eError;
}

uint32_t SPF_wGet(const spifi_config_s *pSPF, spifi_getters_e eGetter)
{
  uint32_t wResult = 0;
  if ((pSPF != NULL) && (pSPF->eSPIFI < SPIFI_INVALID) && bSPIFIInitialized)
    {
      __disable_irq();
      spifi_private_config_s * sThisPrivate = (spifi_private_config_s*)pSPF->vpPrivateData;
      spifiDevSetOpts(sSPIFIPrivate.pThisHandle, SPIFI_OPT_USE_QUAD, false); //Disable QUAD SPI
      spifiDevSetMemMode(sThisPrivate->pThisHandle, false);  //Set Command mode

      wResult = spifiDevGetInfo(sSPIFIPrivate.pThisHandle, eGetter);

      spifiDevSetMemMode(sThisPrivate->pThisHandle, true);  //Set Memory mode
      spifiDevSetOpts(sSPIFIPrivate.pThisHandle, SPIFI_OPT_USE_QUAD, true); //Use QUAD SPI
      __enable_irq();
    }
  return wResult;
}


/******************************************************************************
* Unity Testing
*******************************************************************************/
