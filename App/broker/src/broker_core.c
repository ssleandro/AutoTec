/****************************************************************************
* Title                 :   broker_core source File
* Filename              :   broker_core.c
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
*  03/03/2016   1.0.0   thiago.palmieri broker_core include file Created.
*
*****************************************************************************/
/** @file broker_core.c
 *  @brief This file provides common queue handling for broker module
 *
 *  The broker core holds a single task and one of the most important features, which
 *  is to republish messages only to subscribed modules.
 *
 *  The broker task must be the first task initiated on the system. After its initialization
 *  it will create other two tasks, responsible to handle the subscription and unsubscription.
 *
 *  The broker will sit waiting indefinitely for a publish message, when it arrives
 *  it sill search its subscription list and republish the message to all subscribed
 *  modules.
 */


/******************************************************************************
* Includes
*******************************************************************************/
#include "broker_core.h"
#include "broker_list.h"
#ifndef UNITY_TEST
#include "broker_includeall.h"
#endif
#include "broker_threadControl.h"
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
#ifndef UNITY_TEST
osMessageQDef(BrokerPublish, QUEUE_SIZEOFPUBLISH, contract_s); //!< Create a Message Queue Definition for broker Publishing
osMessageQId  BrokerPublish;  //!< Publish Queue
#endif
/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
    BROKER_MODULES
};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
uint8_t * BRK_WDTData(uint8_t * pbNumberOfThreads)
{
  *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
  return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void BRK_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
  *bPosition = 0;
  while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
    {
      (*bPosition)++;
    }
  *flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

#ifndef UNITY_TEST
/******************************************************************************
* Function : BRK_vCreateThread(const Threads_t sSensorThread )
*//**
* \b Description:
*
* Function used to create threads.
*
* PRE-CONDITION: None
*
* POST-CONDITION: Threads created
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* osThreadDef_t sKEYBackThread;
*
* sKEYBackThread.name = "KEYBackThread";
* sKEYBackThread.stacksize = 500;
* sKEYBackThread.tpriority = osPriorityNormal;
* sKEYBackThread.pthread = SEN_vKEYBackLightThread;
*
* osThreadCreate(&sKEYBackThread, NULL);
~~~~~~~~~~~~~~~
*
* @see ACT_vCreateThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 11/05/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static void BRK_vCreateThread(const Threads_t sThread )
{
  osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());
  ASSERT(xThreads != NULL);
  if (sThread.thisModule != 0)
  {
    osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
  }
}

void BRK_vBrokerThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Print("BROKER Thread Created");
#endif

  BRK_eInitializeLists();

  BrokerPublish = osMessageCreate(osMessageQ(BrokerPublish), NULL); // Create and Initialize a Message Queue.
  ASSERT(BrokerPublish != NULL);

  //Create subthreads
  uint8_t bNumberOfThreads = 0;
  while(THREADS_THREAD(bNumberOfThreads) != NULL)
  {
      BRK_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
  }

  osThreadId xMainID = (osThreadId)argument;
  osSignalSet(xMainID, MODULE_BROKER); //Inform Main thread that broker initialization was a success

  DIG_vOnBrokerStarted(); //Inform Diagnostic that broker has finished startup sequence

  while(1)
    {
      //Do nothing until a publish arrives
      WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
      osEvent evt = osMessageGet(BrokerPublish, osWaitForever);
      WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
      if (evt.status == osEventMessage)
        {
          //Fetch Publisher topic
          contract_s * pSig = (contract_s*)evt.value.p;
          signature_s * pCurrent = NULL;
          //Fetch subscribers from list
          do {
              pCurrent = BRK_eFindNextInList(pSig->eTopic, pCurrent); // Iterate through list
              if (pCurrent != NULL)
                {
                  // Send Origin Message (contract) to pCurrent Queue
                  osMessagePut(pCurrent->vpDestineQueue, (uint32_t)pSig, 0);
                }
          } while (pCurrent != NULL);

        }
      else
        {
          //TODO: Log Publish Error
        }
    }
  osThreadTerminate(NULL); /* Unreachable */
}
#endif
