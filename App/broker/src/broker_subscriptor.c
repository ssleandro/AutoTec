/****************************************************************************
* Title                 :   broker_subscriptor source File
* Filename              :   broker_subscriptor.c
* Author                :   thiago.palmieri
* Origin Date           :   03/03/2016
* Version               :   1.0.0
* Compiler              :   GCC 5.2 2015q4 / ICCARM 7.50.2.10312
* Target                :   LPC4357
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
*  03/03/2016   1.0.0   thiago.palmieri broker_subscriptor source file Created.
*
*****************************************************************************/
/** @file broker_subscriptor.c
 *  @brief This file provides common list handling for broker module.
 *
 */


/******************************************************************************
* Includes
*******************************************************************************/
#include "broker_subscriptor.h"
#include "broker_list.h"
#include "broker_threadControl.h"
#if defined(UNITY_TEST)
#include "unity_fixture.h"
#include <stdint.h>
#endif
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define ADD true      //!< MACRO that defines SUBSCRIBE
#define REMOVE false   //!< MACRO that defines UNSUBSCRIBE
/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
//Thread Control
WATCHDOG_CREATE(SUBS);                                   //!< WDT pointer flag
WATCHDOG_CREATE(UNSUBS);                                   //!< WDT pointer flag
uint8_t bUNSUBSThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bSUBSThreadArrayPosition = 0;                    //!< Thread position in array

#ifndef UNITY_TEST
osMessageQDef(BrokerSubscribe, QUEUE_SIZEOFSUBSCRIBE, signature_s); //!< Create a Message Queue Definition for broker Publishing
osMessageQDef(BrokerUnsubscribe, QUEUE_SIZEOFUNSUBSCRIBE, signature_s); //!< Create a Message Queue Definition for broker Publishing

osMessageQId  BrokerSubscribe;    //!< Subscribe Message Queue
osMessageQId  BrokerUnsubscribe;  //!< Unsubscribe Message Queue

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/

void BRK_vUnSubscriptorThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Print("UN-SUBSCRIPTOR Thread Created");
#endif

  BRK_vDetectThread(&WATCHDOG(UNSUBS), &bUNSUBSThreadArrayPosition, (void*)BRK_vUnSubscriptorThread);
  WATCHDOG_STATE(UNSUBS, WDT_ACTIVE);

  // Create and Initialize a Message Queue.
  BrokerUnsubscribe = osMessageCreate(osMessageQ(BrokerUnsubscribe), NULL);

  osThreadId xBrokerMainID = (osThreadId)argument;
  osSignalSet(xBrokerMainID, THREADS_RETURN_SIGNAL(bUNSUBSThreadArrayPosition));//Task created, inform broker

  eAPPError_s eError;

  while(1)
    {
      WATCHDOG_STATE(UNSUBS, WDT_SLEEP);
      osEvent evt = osMessageGet(BrokerUnsubscribe, osWaitForever);
      WATCHDOG_STATE(UNSUBS, WDT_ACTIVE);
      if (evt.status == osEventMessage)
        {
          // Unsubscribe arrived
          signature_s * sig = (signature_s*)evt.value.p;
          eError = BRK_eBrokerListUpdate(REMOVE, sig);
          ASSERT_LEVEL(eError == APP_ERROR_SUCCESS, LEVEL_INFO);
          if (!eError)
            {
              DIG_onUnSubs__(GET_TIMESTAMP(), sig->eDestine, sig->eTopic);
            }
        }
    }
  osThreadTerminate(NULL);
}
#else
void BRK_vUnSubscriptorThread (void const *argument){}
#endif

#ifndef UNITY_TEST
void BRK_vSubscriptorThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Print("SUBSCRIPTOR Thread Created");
#endif

  BRK_vDetectThread(&WATCHDOG(SUBS), &bSUBSThreadArrayPosition, (void*)BRK_vSubscriptorThread);
  WATCHDOG_STATE(SUBS, WDT_ACTIVE);

  // Create and Initialize a Message Queue.
  BrokerSubscribe = osMessageCreate(osMessageQ(BrokerSubscribe), NULL);

  osThreadId xBrokerMainID = (osThreadId)argument;
  osSignalSet(xBrokerMainID, THREADS_RETURN_SIGNAL(bSUBSThreadArrayPosition));//Task created, inform broker

  eAPPError_s eError;

  while(1)
    {
      WATCHDOG_STATE(SUBS, WDT_SLEEP);
      osEvent evt = osMessageGet(BrokerSubscribe, osWaitForever);
      WATCHDOG_STATE(SUBS, WDT_ACTIVE);
      if (evt.status == osEventMessage)
        {
          //subscribe arrived
          signature_s * sig = (signature_s*)evt.value.p;
          eError = BRK_eBrokerListUpdate(ADD, sig);
          ASSERT_LEVEL(eError == APP_ERROR_SUCCESS, LEVEL_INFO);
          if (!eError)
            {
              DIG_onSubs__(GET_TIMESTAMP(), sig->eDestine, sig->eTopic);
            }
        }
    }
  osThreadTerminate(NULL);
}
#else
void BRK_vSubscriptorThread (void const *argument){}
#endif


/******************************************************************************
* Unity Testing
*******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(SubscriptorModuleTest);

TEST_SETUP(SubscriptorModuleTest)
{

}

TEST_TEAR_DOWN(SubscriptorModuleTest)
{

}

TEST(SubscriptorModuleTest, test_Verify_DetectThread)
{
  uint8_t bThisPositionInSubThreadArray = 0;
  uint8_t bNumberOfArrayThreads = 3; //including core thread
  uint8_t bThreadPositionOnTheList = 1; //Position on wdt Array of flags
  uint8_t bNumber = 0;

  //Verify pointer
  BRK_vDetectThread(&WATCHDOG(SUBS), &bSUBSThreadArrayPosition, BRK_vSubscriptorThread);
  TEST_ASSERT_EQUAL(bThisPositionInSubThreadArray, bSUBSThreadArrayPosition);
  TEST_ASSERT_EQUAL(1, THREADS_WDT_POSITION(bThisPositionInSubThreadArray));
  TEST_ASSERT_EQUAL_PTR(&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(bThisPositionInSubThreadArray)), WATCHDOG(SUBS));

  //verify state through pointer
  WATCHDOG_STATE(SUBS, WDT_ACTIVE);
  TEST_ASSERT_EQUAL(WDT_ACTIVE, *WATCHDOG(SUBS));
  TEST_ASSERT_EQUAL(WDT_ACTIVE, WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(bThisPositionInSubThreadArray)));

  //verify remote pointer
  uint8_t * pbWDTArray = BRK_WDTData(&bNumber);

  TEST_ASSERT_EQUAL(bNumberOfArrayThreads, bNumber);
  TEST_ASSERT_EQUAL_PTR(WATCHDOG_FLAG_ARRAY, pbWDTArray);
  TEST_ASSERT_EQUAL(WDT_ACTIVE, *(pbWDTArray += bThreadPositionOnTheList)); //second thread on the list
  *pbWDTArray = WDT_UNKNOWN;
  TEST_ASSERT_EQUAL(WDT_UNKNOWN, WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(bThisPositionInSubThreadArray)));
}

#endif
