/****************************************************************************
* Title                 :   WATCHDOG Include File
* Filename              :   WATCHDOG.c
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
/** @file WATCHDOG.c
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

/******************************************************************************
* Includes
*******************************************************************************/
#include "WATCHDOG.h"
#include "WATCHDOG_config.h"
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
typedef void (*wdtCallBack) (void); //!< Watchdog callback

/**
 * This Struct holds Watchdog configuration
 */
typedef struct wdt_config_s
{
  uint32_t wTimeout;            //!< WDT Timeout in milliseconds
  wdtCallBack pfWDTCallBack;    //!< WDT Callback function
} wdt_config_s;

static bool bWDTEnabled = false;

peripheral_descriptor_p pWDT = NULL;
#endif

typedef eDEVError_s (*fpIOCTLFunction)(uint32_t wRequest, void * vpValue);
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static wdt_config_s sWDT;     //!< Watchdog Handler
static bool bStarted = false;
/******************************************************************************
* Function Prototypes
*******************************************************************************/
static eDEVError_s WDT_eIOCTLFeed(uint32_t wRequest, void * vpValue);
static eDEVError_s WDT_eIOCTLSetMode(uint32_t wRequest, void * vpValue);
static eDEVError_s WDT_eIOCTLStart(uint32_t wRequest, void * vpValue);
static eDEVError_s WDT_eIOCTLReason(uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction WDT_pIOCTLFunction[] =  //!< IOCTL array of function mapping
{
    IOCTL_WATCHDOG
};
#undef X
/******************************************************************************
* Function Definitions
*******************************************************************************/
#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);

eDEVError_s WDT_eInit(wdt_config_s * pWDT)
{
  if (!bWDTEnabled)
    {
      bWDTEnabled = true;
    }
  return DEV_ERROR_SUCCESS;
}

eDEVError_s WDT_eDeInit(const wdt_config_s * pWDT)
{
  return DEV_ERROR_LAST_ERROR;
}

eDEVError_s WDT_eFeed(const wdt_config_s * pWDT)
{
  return DEV_ERROR_SUCCESS;
}

eDEVError_s WDT_eStart(const wdt_config_s * pWDT)
{
  return DEV_ERROR_SUCCESS;
}

uint8_t WDT_bFetchResetCause(void)
{
  return true;
}

void WDT_vForceReset(void){};

void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

static eDEVError_s WDT_eIOCTLFeed(uint32_t wRequest, void * vpValue)
{
  (void)wRequest;
  (void)vpValue;
  return WDT_eFeed(&sWDT);
}

static eDEVError_s WDT_eIOCTLSetMode(uint32_t wRequest, void * vpValue)
{
  eDEVError_s eError = DEV_ERROR_SUCCESS;
  if (bStarted && (wRequest != IOCTL_WDT_FORCE_RESET))
    {
      return DEV_ERROR_WDT_ALREADY_STARTED;
    }
  switch (wRequest) {
    case IOCTL_WDT_CALLBACK_MODE:
      if (vpValue == NULL)
        {
          return DEV_ERROR_INVALID_IOCTL;
        }
      sWDT.pfWDTCallBack = (wdtCallBack)vpValue;
      break;
    case IOCTL_WDT_RESET_MODE:
      sWDT.pfWDTCallBack = NULL;
      break;
    case IOCTL_WDT_SET_TIMEOUT:
      if (vpValue == NULL)
        {
          return DEV_ERROR_INVALID_IOCTL;
        }
      sWDT.wTimeout = *(uint32_t*)vpValue;
      break;
    case IOCTL_WDT_FORCE_RESET:
      WDT_vForceReset();
      break;
    default:
      eError = DEV_ERROR_INVALID_IOCTL;
      break;
  }
  return eError;
}

static eDEVError_s WDT_eIOCTLStart(uint32_t wRequest, void * vpValue)
{
  (void)wRequest;
  eDEVError_s eError = DEV_ERROR_WDT_ALREADY_STARTED;
  if (bStarted)
    {
      return eError;
    }
  eError = (eDEVError_s)WDT_eInit(&sWDT);
  if (!eError)
    {
      eError = WDT_eStart(&sWDT);
    }

  bStarted = (!eError ? true : false);

  return eError;
}

static eDEVError_s WDT_eIOCTLReason(uint32_t wRequest, void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  (void)wRequest;
  *(uint8_t*)vpValue = WDT_bFetchResetCause();
  return DEV_ERROR_SUCCESS ;
}

eDEVError_s WDT_open(void)
{
  sWDT.wTimeout = WDT_DEFAULT_TIMEOUT_IN_MILLIS;
  sWDT.pfWDTCallBack = NULL;
  bStarted = false;
  return DEV_ERROR_SUCCESS;
}

uint32_t WDT_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize)
{
  (void)this;
  (void)vpBuffer;
  (void)tBufferSize;
  return 0;
}

uint32_t WDT_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize)
{
  (void)this;
  (void)vpBuffer;
  (void)tBufferSize;
  return 0;
}

eDEVError_s WDT_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue)
{
  if (wRequest >= IOCTL_WDT_INVALID)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  eDEVError_s eError;

  // Call specific function
  eError = WDT_pIOCTLFunction[wRequest](wRequest, vpValue);

  return eError;
}

eDEVError_s WDT_close(struct peripheral_descriptor_s* const this)
{
  eDEVError_s eError = DEV_ERROR_SUCCESS;
  eError = (eDEVError_s)WDT_eDeInit(&sWDT);

  bStarted = (!eError ? false : bStarted);

  return eError;
}

/******************************************************************************
* Unity Testing
*******************************************************************************/
#if defined(UNITY_TEST)

TEST_GROUP(WATCHDOGDeviceTest);

TEST_SETUP(WATCHDOGDeviceTest)
{
}

TEST_TEAR_DOWN(WATCHDOGDeviceTest)
{
}

/*
 * This test will <describe>
 */
TEST(WATCHDOGDeviceTest, test_Verify_IOCTLFeed)
{
  //mock test, not really usefull
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLFeed(0, NULL));

}

TEST(WATCHDOGDeviceTest, test_Verify_IOCTLSetMode)
{
  void test(void){}
  //test already started
  bStarted = true;
  TEST_ASSERT_EQUAL(DEV_ERROR_WDT_ALREADY_STARTED, WDT_eIOCTLSetMode(0, NULL));

  //test invalid input
  bStarted = false;
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, WDT_eIOCTLSetMode(0, NULL));

  //test IOCTL_WDT_CALLBACK_MODE
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, WDT_eIOCTLSetMode(IOCTL_WDT_CALLBACK_MODE, NULL));
  TEST_ASSERT_NULL(sWDT.pfWDTCallBack);
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLSetMode(IOCTL_WDT_CALLBACK_MODE, &test));
  TEST_ASSERT_NOT_NULL(sWDT.pfWDTCallBack);
  TEST_ASSERT_EQUAL_PTR(test, sWDT.pfWDTCallBack);

  //test IOCTL_WDT_RESET_MODE
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLSetMode(IOCTL_WDT_RESET_MODE, &test));
  TEST_ASSERT_NULL(sWDT.pfWDTCallBack);

  //test IOCTL_WDT_SET_TIMEOUT
  uint32_t wDummy = 2000;
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, WDT_eIOCTLSetMode(IOCTL_WDT_SET_TIMEOUT, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLSetMode(IOCTL_WDT_SET_TIMEOUT, &wDummy));
  TEST_ASSERT_EQUAL(wDummy, sWDT.wTimeout);

  //test IOCTL_WDT_FORCE_RESET (not really useful)
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLSetMode(IOCTL_WDT_FORCE_RESET, &wDummy));

  //test default return
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, WDT_eIOCTLSetMode(0, &wDummy));
}

TEST(WATCHDOGDeviceTest, test_Verify_IOCTLStart)
{
  bStarted = true;
  TEST_ASSERT_EQUAL(DEV_ERROR_WDT_ALREADY_STARTED, WDT_eIOCTLStart(0, NULL));

  bStarted = false;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLStart(0, NULL));
  TEST_ASSERT_EQUAL(true, bStarted);

  //restore
  bStarted = false;
}

TEST(WATCHDOGDeviceTest, test_Verify_IOCTLReason)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, WDT_eIOCTLReason(IOCTL_WDT_RESTART_REASON, NULL));
  uint32_t wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_eIOCTLReason(IOCTL_WDT_RESTART_REASON, &wDummy));
  TEST_ASSERT_EQUAL(1, wDummy);
  //restore

}

TEST(WATCHDOGDeviceTest, test_Verify_WDT_open)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_open());
  TEST_ASSERT_EQUAL(WDT_DEFAULT_TIMEOUT_IN_MILLIS, sWDT.wTimeout);
  TEST_ASSERT_EQUAL(false, bStarted);
  TEST_ASSERT_NULL(sWDT.pfWDTCallBack);
}

TEST(WATCHDOGDeviceTest, test_Verify_WDT_read)
{
  //Read is not used by the application
  TEST_ASSERT_EQUAL(0, WDT_read(NULL, NULL, 0));

}

TEST(WATCHDOGDeviceTest, test_Verify_WDT_write)
{
  //Read is not used by the application
  TEST_ASSERT_EQUAL(0, WDT_write(NULL, NULL, 0));
}

TEST(WATCHDOGDeviceTest, test_Verify_WDT_ioctl)
{
  void teste(void){}
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, WDT_ioctl(NULL, IOCTL_WDT_INVALID, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_ioctl(NULL, IOCTL_WDT_FEED, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_ioctl(NULL, IOCTL_WDT_RESET_MODE, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_ioctl(NULL, IOCTL_WDT_CALLBACK_MODE, &teste));
  TEST_ASSERT_NOT_NULL(sWDT.pfWDTCallBack);
  TEST_ASSERT_EQUAL_PTR(teste, sWDT.pfWDTCallBack);
  uint32_t wTimeout = 4000;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, WDT_ioctl(NULL, IOCTL_WDT_SET_TIMEOUT, &wTimeout));
  TEST_ASSERT_EQUAL(wTimeout, sWDT.wTimeout);

}

TEST(WATCHDOGDeviceTest, test_Verify_WDT_close)
{
  #define MCU_ERROR_WDT_UNABLE_TO_DISABLE 508

  bStarted = true;
  TEST_ASSERT_EQUAL(MCU_ERROR_WDT_UNABLE_TO_DISABLE, WDT_close(NULL));
  TEST_ASSERT_EQUAL(true, bStarted);

  bStarted = false;
  TEST_ASSERT_EQUAL(MCU_ERROR_WDT_UNABLE_TO_DISABLE, WDT_close(NULL));
  TEST_ASSERT_EQUAL(false, bStarted);
}

TEST(WATCHDOGDeviceTest, test_Verify_DEV_open)
{
  pWDT = DEV_open(PERIPHERAL_WDT);
  TEST_ASSERT_NOT_NULL(pWDT);
  TEST_ASSERT_EQUAL(WDT_DEFAULT_TIMEOUT_IN_MILLIS, sWDT.wTimeout);
  TEST_ASSERT_EQUAL(false, bStarted);
  TEST_ASSERT_NULL(sWDT.pfWDTCallBack);

  //try to reopen
  peripheral_descriptor_p pWDT2 = DEV_open(PERIPHERAL_WDT);
  TEST_ASSERT_NULL(pWDT2);

  //restore
  DEV_close(pWDT);
}

TEST(WATCHDOGDeviceTest, test_Verify_DEV_read)
{
  //No read in interface, not useful test (just a mock)
  TEST_ASSERT_EQUAL(0, DEV_read(pWDT, NULL, 10));
}

TEST(WATCHDOGDeviceTest, test_Verify_DEV_write)
{
  //No write in interface, not useful test (just a mock)
  TEST_ASSERT_EQUAL(0, DEV_write(pWDT, NULL, 10));
}

TEST(WATCHDOGDeviceTest, test_Verify_DEV_ioctl)
{
  void teste(void){}
  uint32_t wDummy = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(pWDT, IOCTL_WDT_INVALID, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(pWDT, IOCTL_WDT_FEED, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pWDT, IOCTL_WDT_FEED, &wDummy));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pWDT, IOCTL_WDT_RESET_MODE, &wDummy));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pWDT, IOCTL_WDT_CALLBACK_MODE, &teste));
  TEST_ASSERT_NOT_NULL(sWDT.pfWDTCallBack);
  TEST_ASSERT_EQUAL_PTR(teste, sWDT.pfWDTCallBack);
  uint32_t wTimeout = 4000;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(pWDT, IOCTL_WDT_SET_TIMEOUT, &wTimeout));
  TEST_ASSERT_EQUAL(wTimeout, sWDT.wTimeout);
}

TEST(WATCHDOGDeviceTest, test_Verify_DEV_close)
{
#define MCU_ERROR_WDT_UNABLE_TO_DISABLE 508

  //impossible to close watchdog, so it is still open from the DEV_open test
  //peripheral_descriptor_p pWDT = DEV_open(PERIPHERAL_WDT);
  //TEST_ASSERT_NOT_NULL(pWDT);

  TEST_ASSERT_EQUAL(MCU_ERROR_WDT_UNABLE_TO_DISABLE, DEV_close(pWDT));
  TEST_ASSERT_EQUAL(false, bStarted);

}

#endif
