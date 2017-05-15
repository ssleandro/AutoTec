/****************************************************************************
* Title                 :   spifi Include File
* Filename              :   spifi.h
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
/** @file spifi.h
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
#ifndef ABS_INC_SPIFI_H_
#define ABS_INC_SPIFI_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <inttypes.h>
#include "mcuerror.h"

#include "spifilib_dev.h" //TODO: commented line, change to spifi.h
#include "spifilib_api.h"

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

typedef struct spifi_private_config_s
{
  SPIFI_HANDLE_T * pThisHandle;
}spifi_private_config_s;

/**
 * @brief This Typedef holds get commands
 */
typedef enum spifi_getters_e
{
  SPIFI_GET_BASE_ADDRESS     = 0,         /**< Device physical memory address */
  SPIFI_GET_DEVSIZE          = 1,         /**< Device size in Bytes */
  SPIFI_GET_ERASE_BLOCKSIZE  = 3,         /**< Size of erase blocks */
  SPIFI_GET_PAGESIZE         = 6,         /**< Size of a page, page write size limit */
  SPIFI_GET_MAXCLOCK         = 8,         /**< Maximum device speed in Hz */

  SPIFI_GET_INVALID
} spifi_getters_e;

/**
 * This enumeration is a list of possible SPIFI ports on the Board
 */
typedef enum spifiChannel_s
{
  SPIFI0,           //!< SPIFI 0
  SPIFI_INVALID     //!< Invalid SPIFI indicator
}spifiChannel_s;
/**
 * This Struct holds SPIFI configuration for given board
 */
typedef struct spifi_config_s
{
  spifiChannel_s eSPIFI;        //!< SPIFI Channel
  uint32_t wSPIFIBaseAddress;   //!< SPIFI Base Memory address
  void * vpPrivateData;         //!< SPIFI Private data
} spifi_config_s;

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
* Function : SPF_eInit(spifi_config_s *pSPF)
*//**
* \b Description:
*
* This is a public function used to initialize a SPIFI port.
* A spifi_config_s struct pointer must be passed to the function. The struct
* must be initialized with SPIFI port and Memory Address to be used by SPIFI.
*
* PRE-CONDITION: Valid SPIFI port.
*
* POST-CONDITION: SPIFI Port initialized and ready.
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   spifi_config_s sSPIFIport;
*   sSPIFIport.eSPIFI = SPIFI0;
*   sSPIFIport.wSPIFIBaseAddress = 0x14000000;  //Memory Address
*
*   if (MCU_ERROR_SUCCESS == SPF_eInit(&sSPIFIport))
*     PRINTF("SPIFI0 Initiated");
~~~~~~~~~~~~~~~
*
* @see SPF_eInit, SPF_eDeInit, SPF_eRead, SPF_eProgram, SPF_eErase, SPF_wGet
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 12/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s SPF_eInit(spifi_config_s *pSPF);

/******************************************************************************
* Function : SPF_eDeInit(spifi_config_s *pSPF)
*//**
* \b Description:
*
* This is a public function used to de-initialize a SPIFI port.
* A spifi_config_s struct pointer already initialized must be passed to the function.
*
* PRE-CONDITION: Valid SPIFI port.
*
* POST-CONDITION: SPIFI Port de-initialized
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   spifi_config_s sSPIFIport;
*   sSPIFIport.eSPIFI = SPIFI0;
*   sSPIFIport.wSPIFIBaseAddress = 0x14000000;  //Memory Address
*
*   if (MCU_ERROR_SUCCESS == SPF_eInit(&sSPIFIport))
*     PRINTF("SPIFI0 Initiated");
*   if (MCU_ERROR_SUCCESS == SPF_DeInit(&sSPIFIport))
*     PRINTF("SPIFI0 de-Initiated");
~~~~~~~~~~~~~~~
*
* @see SPF_eInit, SPF_eDeInit, SPF_eRead, SPF_eProgram, SPF_eErase, SPF_wGet
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 12/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s SPF_eDeInit(spifi_config_s *pSPF);

/******************************************************************************
* Function : SPF_eRead(spifi_config_s *pSPF, uint32_t addr, uint32_t *readBuff, uint32_t bytes)
*//**
* \b Description:
*
* This is a public function used to read a SPIFI address.
* A spifi_config_s struct pointer already initialized must be passed to the function. A
* valid Address, Buffer and size must be passed as well.
*
* PRE-CONDITION: Valid SPIFI port, address and buffer.
*
* POST-CONDITION: none
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   spifi_config_s sSPIFIport;
*   sSPIFIport.eSPIFI = SPIFI0;
*   sSPIFIport.wSPIFIBaseAddress = 0x14000000;  //Memory Address
*
*   if (MCU_ERROR_SUCCESS == SPF_eInit(&sSPIFIport))
*     PRINTF("SPIFI0 Initiated");
*     uint8_t readBuff[10];
*   if (MCU_ERROR_SUCCESS == SPF_eRead(&sSPIFIport, 0x14000000, &readBuff, sizeof(readBuff)))
*     PRINTF("SPIFI0 Read");
~~~~~~~~~~~~~~~
*
* @see SPF_eInit, SPF_eDeInit, SPF_eRead, SPF_eProgram, SPF_eErase, SPF_wGet
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 12/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s SPF_eRead(const spifi_config_s *pSPF, uint32_t addr, uint32_t *readBuff, uint32_t bytes);

/******************************************************************************
* Function : SPF_eProgram(const spifi_config_s *pSPF, uint32_t addr, const uint32_t *writeBuff, uint32_t bytes)
*//**
* \b Description:
*
* This is a public function used to program a SPIFI address.
* A spifi_config_s struct pointer already initialized must be passed to the function. A
* valid Address, Buffer and size must be passed as well.
*
* PRE-CONDITION: Valid SPIFI port, address and buffer.
*
* POST-CONDITION: none
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   spifi_config_s sSPIFIport;
*   sSPIFIport.eSPIFI = SPIFI0;
*   sSPIFIport.wSPIFIBaseAddress = 0x14000000;  //Memory Address
*
*   if (MCU_ERROR_SUCCESS == SPF_eInit(&sSPIFIport))
*     PRINTF("SPIFI0 Initiated");
*     uint8_t writeBuff[] = "TEST";
*   if (MCU_ERROR_SUCCESS == SPF_eProgram(&sSPIFIport, 0x14000000, &writeBuff, sizeof(writeBuff)))
*     PRINTF("SPIFI0 Written");
~~~~~~~~~~~~~~~
*
* @see SPF_eInit, SPF_eDeInit, SPF_eRead, SPF_eProgram, SPF_eErase, SPF_wGet
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 12/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s SPF_eProgram(const spifi_config_s *pSPF, uint32_t addr, const uint32_t *writeBuff, uint32_t bytes);

/******************************************************************************
* Function : SPF_eErase(const spifi_config_s *pSPF, uint8_t EraseAddress, uint32_t firstAddrOrBlock, uint32_t lastAddrOrNumBlocks)
*//**
* \b Description:
*
* This is a public function used to erase a SPIFI address or block interval.
* A spifi_config_s struct pointer already initialized must be passed to the function.
* In case of Address Erase (EraseAddress = true) then a valid Address Range must be given.
* In case of Block Erase (EraseAddress = false) then a valid initial Block and number of blocks  must be given.
*
* PRE-CONDITION: Valid SPIFI port, initial address/block and final/number Address/Blocks.
*
* POST-CONDITION: none
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   spifi_config_s sSPIFIport;
*   sSPIFIport.eSPIFI = SPIFI0;
*   sSPIFIport.wSPIFIBaseAddress = 0x14000000;  //Memory Address
*
*   if (MCU_ERROR_SUCCESS == SPF_eInit(&sSPIFIport))
*     PRINTF("SPIFI0 Initiated");
*   if (MCU_ERROR_SUCCESS == SPF_eErase(&sSPIFIport, true, 0x14000000, 0x14001000)
*     PRINTF("SPIFI0 Address Interval erased");
~~~~~~~~~~~~~~~
*
* @see SPF_eInit, SPF_eDeInit, SPF_eRead, SPF_eProgram, SPF_eErase, SPF_wGet
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 12/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s SPF_eErase(const spifi_config_s *pSPF, uint8_t EraseAddress, uint32_t firstAddrOrBlock, uint32_t lastAddrOrNumBlocks);

/******************************************************************************
* Function : SPF_wGet(const spifi_config_s *pSPF, spifi_getters_e eGetter)
*//**
* \b Description:
*
* This is a public function used to Fetch the Memory information of a SPIFI connected device.
*
* PRE-CONDITION: Valid SPIFI port
*
* POST-CONDITION: none
*
* @return     Memory Size
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   spifi_config_s sSPIFIport;
*   sSPIFIport.eSPIFI = SPIFI0;
*   sSPIFIport.wSPIFIBaseAddress = 0x14000000;  //Memory Address
*
*   if (MCU_ERROR_SUCCESS == SPF_eInit(&sSPIFIport))
*     PRINTF("SPIFI0 Initiated");
*   uint32_t wTemp = SPF_wGet(&sSPIFIport, SPIFI_GET_DEVSIZE);
*     PRINTF("Memory size: %d", wTemp);
~~~~~~~~~~~~~~~
*
* @see SPF_eInit, SPF_eDeInit, SPF_eRead, SPF_eProgram, SPF_eErase, SPF_wGet
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
uint32_t SPF_wGet(const spifi_config_s *pSPF, spifi_getters_e eGetter);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ABS_INC_SPIFI_H_ */

/*** End of File **************************************************************/
