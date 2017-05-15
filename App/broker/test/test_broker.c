/****************************************************************************
* Title                 :   test_broker Include File
* Filename              :   test_broker.c
* Author                :   thiago.palmieri
* Origin Date           :   8 de mar de 2016
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
*  8 de mar de 2016   1.0.0   thiago.palmieri test_broker include file Created.
*
*****************************************************************************/
/** @file test_broker.c
 *  @brief This file provides some broker testing functions.
 *
 */
#ifndef UNITY_TEST
/******************************************************************************
* Includes
*******************************************************************************/
#include "test_broker.h"

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
typedef uint16_t (*change)(uint16_t data, uint8_t step);
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
DECLARE_QUEUE(TestPublish, 8);
//osMessageQDef(TestPublish, 8, signature_s); //!< Create a Message Queue Definition for broker Publishing
//osMessageQId  TestPublish; //!< identifies the message queue (pointer to a message queue control block).

//signature_s test;
CREATE_CONTRACT(test);
CREATE_SIGNATURE(test);
//contract_s contract;
//message_s message;

uint16_t increase(uint16_t data, uint8_t step)
{
  data += step;
  return data;
}

uint16_t decrease(uint16_t data, uint8_t step)
{
  data -= step;
  return data;
}


void vTestSubscriptionThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Print("Subs Test Thread Created");
#endif
  INITIALIZE_QUEUE(TestPublish);

  SIGNATURE(test).eDestine = MODULE_INPUT_KEYPAD;
  SIGNATURE(test).eTopic = TOPIC_SENSOR;
  SIGNATURE(test).vpDestineQueue = TestPublish;

  osStatus testResult = SUBSCRIBE(SIGNATURE(test), osWaitForever);
  (void)testResult;
  osThreadId xTestMainID = (osThreadId)argument;
  osSignalSet(xTestMainID, (1 << 0));//Task created, inform core

  while(1)
    {
      //osStatus testResult = osMessagePut(BrokerSubscribe, (uint32_t)&test, osWaitForever);
      //osDelay(400);
      osEvent evt = RECEIVE(TestPublish, osWaitForever);
      if (evt.status == osEventMessage)
        {
          // Message arrived
          uint32_t wData = *(uint32_t*)GET_MESSAGE(GET_CONTRACT(evt))->pvMessage;
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
          char bBuffer[30];
          snprintf(bBuffer, sizeof(bBuffer), "RECEIVED: %d Level", wData);
          SEGGER_SYSVIEW_Print(bBuffer);
#endif
          (void)wData;
        }
      //osThreadSuspend(NULL);
    }
}

void vTestPublishThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Print("Pub Test Thread Created");
#endif  
  change changeDuty;
  changeDuty = increase;
  uint16_t hMessage = 0;
  MESSAGE(test).hMessageKey = 0;
  MESSAGE(test).hMessageSize = 1;
  MESSAGE(test).eMessageType = MT_TWOBYTE;
  MESSAGE(test).pvMessage = &hMessage;

  CONTRACT(test).bVersion =1;
  CONTRACT(test).eOrigin = MODULE_COMM_CBA;
  CONTRACT(test).eTopic = TOPIC_SENSOR;
  CONTRACT(test).sMessage = &MESSAGE(test);

  osThreadId xTestMainID = (osThreadId)argument;
  osSignalSet(xTestMainID, (1 << 1));//Task created, inform cor

  while(1)
    {
      hMessage = changeDuty(hMessage, 100);
      if (hMessage >= 1020) changeDuty = decrease;
      else if (hMessage <= 0) changeDuty = increase;
      osStatus testResult = PUBLISH(CONTRACT(test), 0);
      (void)testResult;
      osDelay(300);
     //osThreadSuspend(NULL);
    }
}


void vRunBrokerTestsThread (void const *argument)
{

  //TestPublish = osMessageCreate(osMessageQ(TestPublish), NULL); // Create and Initialize a Message
  //INITIALIZE_QUEUE(TestPublish);

  osThreadDef_t sTestSubscriptionThread, sTestPublishThread;


  sTestSubscriptionThread.name = "BrokerSUBSTest";
  sTestSubscriptionThread.stacksize = 200;
  sTestSubscriptionThread.tpriority = osPriorityNormal;
  sTestSubscriptionThread.pthread = vTestSubscriptionThread;

  osThreadCreate(&sTestSubscriptionThread, (void*)osThreadGetId());

  sTestPublishThread.name = "BrokerPUBLISHTest";
  sTestPublishThread.stacksize = 200;
  sTestPublishThread.tpriority = osPriorityNormal;
  sTestPublishThread.pthread = vTestPublishThread;

  osThreadCreate(&sTestPublishThread, (void*)osThreadGetId());
  (void)sTestPublishThread;

  osThreadId xMainTestID = (osThreadId)argument;
  osSignalWait((1 << 0) || (1 << 1), osWaitForever); //wait for sub threads creation
  osSignalSet(xMainTestID, (1 << 0)); //Inform Main thread that broker initialization was a success

  while(1)
    {
     osThreadSuspend(NULL);
    }
}
#endif
