/****************************************************************************
* Title                 :   FLASHER Include File
* Filename              :   FLASHER.c
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
*  12 de abr de 2016   1.0.0   thiago.palmieri FLASHER include file Created.
*
*****************************************************************************/
/** @file FLASHER.c
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

/******************************************************************************
* Includes
*******************************************************************************/
#include "FLASHER.h"
#include "FLASHER_config.h"
#include <string.h>
#include "ring_buffer.h"
#ifndef UNITY_TEST
#include "mculib.h"
#else
#include "unity_fixture.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
#if defined (UNITY_TEST)

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

#endif

typedef eDEVError_s (*fpIOCTLFunction)(uint32_t wRequest, void * vpValue);
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
/**
 * Default Sensor Initialization Values
 */
static spifi_config_s sFLASHER =
    {
        .eSPIFI = FLS_DEFAULT_SPIFI_INTERFACE,
        .wSPIFIBaseAddress  = FLS_STARTING_MEMORY_ADDRESS,
    };

static uint32_t wReadAddress = FLS_STARTING_MEMORY_ADDRESS;
/******************************************************************************
* Function Prototypes
*******************************************************************************/
static eDEVError_s FLS_eIOCTLErase(uint32_t wRequest, void * vpValue);
static eDEVError_s FLS_eIOCTLSetAddress(uint32_t wRequest, void * vpValue);
static eDEVError_s FLS_eIOCTLgets(uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction FLS_pIOCTLFunction[] =  //!< IOCTL array of function mapping
{
    IOCTL_FLASHER
};
#undef X
/******************************************************************************
* Function Definitions
*******************************************************************************/
#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);

eDEVError_s SPF_eRead(spifi_config_s * pSPIFI, uint32_t wReadAddress, void * vpBuffer, uint32_t tBufferSize)
{
  (void)pSPIFI;
  uint32_t wIndex = 0;
  if (wReadAddress == 0x14000000)
    {
      for (wIndex = 0; wIndex < tBufferSize; wIndex++)
        {
          *((uint32_t*)vpBuffer + wIndex) = (0x123456 + wIndex);
        }

    }
  else
    {
      for (wIndex = 0; wIndex < tBufferSize; wIndex++)
        {
          *((uint32_t*)vpBuffer + wIndex) = (0x987654 + wIndex);
        }
    }
  return DEV_ERROR_SUCCESS;
}

eDEVError_s SPF_eErase(spifi_config_s * pSPIFI, uint8_t EraseAddress, uint32_t firstAddrOrBlock, uint32_t lastAddrOrNumBlocks)
{
  (void)pSPIFI;
  (void)firstAddrOrBlock;
  (void)lastAddrOrNumBlocks;
  return DEV_ERROR_SUCCESS + EraseAddress;
}

eDEVError_s SPF_eProgram(spifi_config_s *pSPIFI, uint32_t addr, const uint32_t *writeBuff, uint32_t bytes)
{
  (void)pSPIFI;
  (void)addr;
  (void)bytes;
  (void)writeBuff;
  return DEV_ERROR_SUCCESS;
}

eDEVError_s SPF_eInit(spifi_config_s * pSPIFI)
{
  (void)pSPIFI;
  return DEV_ERROR_SUCCESS;
}

eDEVError_s SPF_eDeInit(spifi_config_s * pSPIFI)
{
  (void)pSPIFI;
  return DEV_ERROR_SUCCESS;
}

uint32_t SPF_wGet(const spifi_config_s *pSPF, spifi_getters_e eGetter)
{
  (void)pSPF;
  (void)eGetter;
  return 0x12345678;
}
void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
* Function : FLS_eIOCTLErase(uint32_t wRequest, void * vpValue)
*//**
* \b Description:
*
* ATTENTION: This function must be called by the FLS_ioctl interface.
*
* This private function is used to erase an address or block interval from SPIFI
* Flash memory.
* The wRequest parameter indicates if Block or Address erase must be used, the vpValue
* pointer must hold two values; in case of block erase, the first block and the number
* of blocks to erase, in case of address erase, the first and last addresses.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: memory erased
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   uint32_t vpValue[] = {0x14000000, 0x14001000};
*   if (FLS_eIOCTLErase(IOCTL_FLS_ERASE_ADDRESS, &vpValue[0]) == DEV_ERROR_SUCCESS)
*     PRINTF("Address Erased");
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
static eDEVError_s FLS_eIOCTLErase(uint32_t wRequest, void * vpValue)
{
  if(vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  uint8_t bRequest = false;
  if (wRequest == IOCTL_FLS_ERASE_ADDRESS)
    {
      bRequest = true;
    }
  return (eDEVError_s)SPF_eErase(&sFLASHER, bRequest, *(uint32_t*)vpValue, *((uint32_t*)vpValue + 1));
}

/******************************************************************************
* Function : FLS_eIOCTLSetAddress(uint32_t wRequest, void * vpValue)
*//**
* \b Description:
*
* ATTENTION: This function must be called by the FLS_ioctl interface.
*
* This private function is used to set an address to be used for read or write on SPIFI
* Flash memory.
* The vpValue pointer must hold the starting Address to be used for read/program;
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: memory address set
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   uint32_t vpValue = 0x14000000;
*   if (FLS_eIOCTLSetAddress(IOCTL_FLS_SET_PROGRAM_ADDRESS, &vpValue) == DEV_ERROR_SUCCESS)
*     PRINTF("Address Set");
*   uint32_t wReadBuffer[10];
*   FLS_read(pFLASHER, &wReadBuffer, sizeof(wReadBuffer)); //read (10 * 4) bytes from position vpValue
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
static eDEVError_s FLS_eIOCTLSetAddress(uint32_t wRequest, void * vpValue)
{
  (void)wRequest;
  if(vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  wReadAddress = *(uint32_t*)vpValue;
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : FLS_eIOCTLgets(uint32_t wRequest, void * vpValue)
*//**
* \b Description:
*
* ATTENTION: This function must be called by the FLS_ioctl interface.
*
* This private function is used to get information from a SPIFI Flash memory.
* The vpValue pointer must not be NULL, it will hold the result value in 32Bit size.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: memory information fetched
*
* @return     eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   uint32_t vpValue = 0;
*   if (FLS_eIOCTLgets(IOCTL_FLS_GET_MEM_SIZE, &vpValue) == DEV_ERROR_SUCCESS)
*     PRINTF("Memory Size: %d", vpValue);
~~~~~~~~~~~~~~~
*
* @see FLS_open, FLS_write, FLS_read, FLS_ioctl and FLS_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 18/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s FLS_eIOCTLgets(uint32_t wRequest, void * vpValue)
{
  if(vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  spifi_getters_e eGetter = SPIFI_GET_INVALID;
  switch (wRequest) {
    case IOCTL_FLS_GET_MEM_SIZE:
      eGetter = SPIFI_GET_DEVSIZE;
      break;
    case IOCTL_FLS_GET_BLOCK_SIZE:
      eGetter = SPIFI_GET_ERASE_BLOCKSIZE;
      break;
    case IOCTL_FLS_GET_PAGE_SIZE:
      eGetter = SPIFI_GET_PAGESIZE;
      break;
    case IOCTL_FLS_GET_MAX_CLOCK:
      eGetter = SPIFI_GET_MAXCLOCK;
      break;
    default:
      return DEV_ERROR_INVALID_IOCTL;
      break;
  }
  *(uint32_t*)vpValue = SPF_wGet(&sFLASHER, eGetter);
  return DEV_ERROR_SUCCESS;
}

eDEVError_s FLS_open(void)
{
  sFLASHER.eSPIFI = FLS_DEFAULT_SPIFI_INTERFACE;
  sFLASHER.wSPIFIBaseAddress  = FLS_STARTING_MEMORY_ADDRESS;
  sFLASHER.vpPrivateData = NULL;

  eDEVError_s eError = (eDEVError_s)SPF_eInit(&sFLASHER);
  return eError;
}

uint32_t FLS_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize)
{
  (void)this;
  uint32_t wResult = 0;
  if (vpBuffer != NULL)
    {
      if ((eDEVError_s)SPF_eRead(&sFLASHER, wReadAddress, vpBuffer, tBufferSize) == DEV_ERROR_SUCCESS)
        {
          wResult = tBufferSize;
        }
    }
  return wResult;
}

uint32_t FLS_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize)
{
  (void)this;
  uint32_t wResult = 0;
  if ((vpBuffer != NULL) && (tBufferSize > 0))
    {
      if ((eDEVError_s)SPF_eProgram(&sFLASHER, wReadAddress, vpBuffer, tBufferSize) == DEV_ERROR_SUCCESS)
        {
          wResult = tBufferSize;
        }
    }
  return wResult;
}

eDEVError_s FLS_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue)
{
  (void)this;
  if ((wRequest >= IOCTL_FLS_INVALID) || (vpValue == NULL))
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  eDEVError_s eError;

  //Configure
  eError = FLS_pIOCTLFunction[wRequest](wRequest, vpValue);

  return eError;
}

eDEVError_s FLS_close(struct peripheral_descriptor_s* const this)
{
  (void)this;
  return (eDEVError_s)SPF_eDeInit(&sFLASHER);
}

/******************************************************************************
* Unity Testing
*******************************************************************************/
#if defined(UNITY_TEST)

TEST_GROUP(FLASHERDeviceTest);

TEST_SETUP(FLASHERDeviceTest)
{
}

TEST_TEAR_DOWN(FLASHERDeviceTest)
{
}

/*
 * This test will test Erase ioctl function
 */
TEST(FLASHERDeviceTest, test_Verify_that_Erase)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_eIOCTLErase(IOCTL_FLS_ERASE_ADDRESS, NULL));
  uint32_t wDummy;
  //The SPF_eErase mockup function will return success + 1 in case of erase address, or success
  // in case of erase block. The real function will return success only in both cases.
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS + 1, FLS_eIOCTLErase(IOCTL_FLS_ERASE_ADDRESS, &wDummy));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLErase(IOCTL_FLS_ERASE_BLOCK, &wDummy));
}

/*
 * This test will test SetAddress ioctl function
 */
TEST(FLASHERDeviceTest, test_Verify_that_SetAddress)
{
  //Error conditions (NULL pointer)
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_eIOCTLSetAddress(IOCTL_FLS_ERASE_ADDRESS, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_eIOCTLSetAddress(IOCTL_FLS_SET_PROGRAM_ADDRESS, NULL));

  uint32_t wDummy = 0x15000000;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLSetAddress(IOCTL_FLS_SET_PROGRAM_ADDRESS, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, wReadAddress);
  //restore
  wDummy = 0x14000000;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLSetAddress(IOCTL_FLS_SET_PROGRAM_ADDRESS, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, wReadAddress);
}

/*
 * This test will test SetAddress ioctl function
 */
TEST(FLASHERDeviceTest, test_Verify_that_Gets)
{
  //Error conditions (NULL pointer)
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_eIOCTLgets(IOCTL_FLS_GET_BLOCK_SIZE, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_eIOCTLgets(IOCTL_FLS_INVALID, NULL));

  uint32_t wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLgets(IOCTL_FLS_GET_MEM_SIZE, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, 0x12345678);
  wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLgets(IOCTL_FLS_GET_BLOCK_SIZE, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, 0x12345678);
  wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLgets(IOCTL_FLS_GET_PAGE_SIZE, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, 0x12345678);
  wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_eIOCTLgets(IOCTL_FLS_GET_MAX_CLOCK, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, 0x12345678);
  wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_eIOCTLgets(IOCTL_FLS_SET_PROGRAM_ADDRESS, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, 0);
  //restore

}

/*
 * This test will test FLS_open
 */
TEST(FLASHERDeviceTest, test_Verify_that_FLS_open)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_open());
  TEST_ASSERT_EQUAL(FLS_DEFAULT_SPIFI_INTERFACE, sFLASHER.eSPIFI);
  TEST_ASSERT_EQUAL(FLS_STARTING_MEMORY_ADDRESS, sFLASHER.wSPIFIBaseAddress);
}

/*
 * This test will test FLS_read
 */
TEST(FLASHERDeviceTest, test_Verify_that_FLS_read)
{
  uint32_t wBuff[2] = {0, 0};
  //Error conditions
  TEST_ASSERT_EQUAL(0, FLS_read(NULL, NULL, 0));
  TEST_ASSERT_EQUAL(0, FLS_read(NULL, &wBuff, 0));
  TEST_ASSERT_EQUAL(0, FLS_read(NULL, NULL, sizeof(wBuff)));

  TEST_ASSERT_EQUAL(sizeof(wBuff) / sizeof(wBuff[0]), FLS_read(NULL, &wBuff, sizeof(wBuff) / sizeof(wBuff[0])));
  TEST_ASSERT_EQUAL(0x123456, wBuff[0]);
  TEST_ASSERT_EQUAL(0x123457, wBuff[1]);
}

/*
 * This test will test FLS_write
 */
TEST(FLASHERDeviceTest, test_Verify_that_FLS_write)
{
  uint32_t wBuff[2] = {0, 0};
  //Error conditions
  TEST_ASSERT_EQUAL(0, FLS_write(NULL, NULL, 0));
  TEST_ASSERT_EQUAL(0, FLS_write(NULL, &wBuff, 0));
  TEST_ASSERT_EQUAL(0, FLS_write(NULL, NULL, sizeof(wBuff)));

  TEST_ASSERT_EQUAL(sizeof(wBuff) / sizeof(wBuff[0]), FLS_write(NULL, &wBuff, sizeof(wBuff) / sizeof(wBuff[0])));
}

/*
 * This test will test FLS_ioctl
 */
TEST(FLASHERDeviceTest, test_Verify_that_FLS_ioctl)
{
  uint32_t wBuff = 0x16000000;
  //Error conditions
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_ioctl(NULL, IOCTL_FLS_INVALID, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, FLS_ioctl(NULL, IOCTL_FLS_ERASE_BLOCK, NULL));

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_ioctl(NULL, IOCTL_FLS_SET_PROGRAM_ADDRESS, &wBuff));
  TEST_ASSERT_EQUAL(wBuff, wReadAddress);

  uint32_t wBuff2[] = {0x14000000, 0x14001000};
  //The SPF_eErase mockup function will return success + 1 in case of erase address, or success
  // in case of erase block. The real function will return success only in both cases.
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS + 1, FLS_ioctl(NULL, IOCTL_FLS_ERASE_ADDRESS, &wBuff2));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_ioctl(NULL, IOCTL_FLS_ERASE_BLOCK, &wBuff2));

  //restore
  wBuff = 0x14000000;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, FLS_ioctl(NULL, IOCTL_FLS_SET_PROGRAM_ADDRESS, &wBuff));
  TEST_ASSERT_EQUAL(wBuff, wReadAddress);
}

/*
 * This test will test FLS_close
 */
TEST(FLASHERDeviceTest, test_Verify_that_FLS_close)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, SPF_eDeInit(NULL));
}

/*
 * This test will test DEV_open
 */
TEST(FLASHERDeviceTest, test_Verify_that_DEV_open)
{
  //NULL ID
  peripheral_descriptor_p pFLASHER = DEV_open(PERIPHERAL_INVALID);
  TEST_ASSERT_NULL(pFLASHER);
  pFLASHER = DEV_open(PERIPHERAL_FLASHER);
  TEST_ASSERT_NOT_NULL(pFLASHER);
  //Try to reopen
  peripheral_descriptor_p pFLASHER2 = DEV_open(PERIPHERAL_FLASHER);
  TEST_ASSERT_NULL(pFLASHER2);

  //restore
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(pFLASHER));
}

/*
 * This test will test DEV_read
 */
TEST(FLASHERDeviceTest, test_Verify_that_DEV_read)
{
  peripheral_descriptor_p pFLASHER = DEV_open(PERIPHERAL_FLASHER);
  TEST_ASSERT_NOT_NULL(pFLASHER);

  uint32_t wBuff[2] = {0, 0};
  //Error conditions
  TEST_ASSERT_EQUAL(0, DEV_read(pFLASHER, NULL, 0));
  TEST_ASSERT_EQUAL(0, DEV_read(pFLASHER, &wBuff, 0));
  TEST_ASSERT_EQUAL(0, DEV_read(pFLASHER, NULL, sizeof(wBuff)));

  TEST_ASSERT_EQUAL(sizeof(wBuff) / sizeof(wBuff[0]), DEV_read(pFLASHER, &wBuff, sizeof(wBuff) / sizeof(wBuff[0])));
  TEST_ASSERT_EQUAL(0x123456, wBuff[0]);
  TEST_ASSERT_EQUAL(0x123457, wBuff[1]);

  //restore
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(pFLASHER));
}

/*
 * This test will test DEV_write
 */
TEST(FLASHERDeviceTest, test_Verify_that_DEV_write)
{
  peripheral_descriptor_p pFLASHER = DEV_open(PERIPHERAL_FLASHER);
  TEST_ASSERT_NOT_NULL(pFLASHER);

  uint32_t wBuff[2] = {0, 0};
  //Error conditions
  TEST_ASSERT_EQUAL(0, DEV_write(pFLASHER, NULL, 0));
  TEST_ASSERT_EQUAL(0, DEV_write(pFLASHER, &wBuff, 0));
  TEST_ASSERT_EQUAL(0, DEV_write(pFLASHER, NULL, sizeof(wBuff)));

  TEST_ASSERT_EQUAL(sizeof(wBuff) / sizeof(wBuff[0]), DEV_write(pFLASHER, &wBuff, sizeof(wBuff) / sizeof(wBuff[0])));

  //restore
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(pFLASHER));
}

/*
 * This test will test DEV_ioctl
 */
TEST(FLASHERDeviceTest, test_Verify_that_DEV_ioctl)
{
  peripheral_descriptor_p pFLASHER = DEV_open(PERIPHERAL_FLASHER);
  TEST_ASSERT_NOT_NULL(pFLASHER);

  uint32_t wBuff = 0x16000000;
  //Error conditions
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(pFLASHER, IOCTL_FLS_INVALID, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(pFLASHER, IOCTL_FLS_ERASE_BLOCK, NULL));

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pFLASHER, IOCTL_FLS_SET_PROGRAM_ADDRESS, &wBuff));
  TEST_ASSERT_EQUAL(wBuff, wReadAddress);

  uint32_t wBuff2[] = {0x14000000, 0x14001000};
  //The SPF_eErase mockup function will return success + 1 in case of erase address, or success
  // in case of erase block. The real function will return success only in both cases.
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS + 1, DEV_ioctl(pFLASHER, IOCTL_FLS_ERASE_ADDRESS, &wBuff2));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pFLASHER, IOCTL_FLS_ERASE_BLOCK, &wBuff2));

  //restore
  wBuff = 0x14000000;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pFLASHER, IOCTL_FLS_SET_PROGRAM_ADDRESS, &wBuff));
  TEST_ASSERT_EQUAL(wBuff, wReadAddress);
  //restore
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(pFLASHER));
}

/*
 * This test will test DEV_close
 */
TEST(FLASHERDeviceTest, test_Verify_that_DEV_close)
{
  //open
  peripheral_descriptor_p pFLASHER = DEV_open(PERIPHERAL_FLASHER);
  TEST_ASSERT_NOT_NULL(pFLASHER);

  //close
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(pFLASHER));
}

#endif
