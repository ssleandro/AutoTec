/****************************************************************************
* Title                 :   test_core Source File
* Filename              :   test_core.c
* Author                :   Henrique Reis
* Origin Date           :   14 de out de 2016
* Version               :   1.0.0
* Compiler              :   GCC 5.4 2016q2 / ICCARM 7.40.3.8938
* Target                :   LPC43XX M4
* Notes                 :   Qualicode Machine Technologies
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
*    Date    Version     Author           Description
*  14/10/16   1.0.0  Henrique Reis     test_core.c created
*
*****************************************************************************/
/** @file
 *  @brief
 *
 *
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "M2G_app.h"
#include "test_core.h"
#include "debug_tool.h"
#include "../../test/config/test_config.h"
#include "test_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
//!< MACRO to define the size of SENSOR queue
#define QUEUE_SIZEOFTEST 5

#define THIS_MODULE MODULE_TEST

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
DECLARE_QUEUE(TestPubQueue, QUEUE_SIZEOFTEST);      //!< Declaration of Interface Queue
CREATE_SIGNATURE(Test);                             //!< Signature Declarations
CREATE_CONTRACT(Test);                              //!< Create contract for sensor msg publication

/******************************************************************************
* Module typedef
*******************************************************************************/
typedef uint16_t (*changeTest)(uint16_t data, uint8_t step);

/**
* Module Threads
*/
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
    TEST_MODULES
};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(TESTPUB);                                   //!< WDT pointer flag
uint8_t bTESTPUBThreadArrayPosition = 0;                    //!< Thread position in array

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
uint8_t * TEST_WDTData(uint8_t * pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void TEST_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
    *bPosition = 0;
    while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
    {
        (*bPosition)++;
    }
    *flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

static void TEST_vCreateThread(const Threads_t sThread )
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
    }
}

uint16_t TEST_vIncrease(uint16_t data, uint8_t step)
{
  data += step;
  return data;
}

uint16_t TEST_vDecrease(uint16_t data, uint8_t step)
{
  data -= step;
  return data;
}

void TEST_vTestPublishThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Test Publish Thread Created");
#endif

    TEST_vDetectThread(&WATCHDOG(TESTPUB), &bTESTPUBThreadArrayPosition, (void*)TEST_vTestPublishThread);
    WATCHDOG_STATE(TESTPUB, WDT_ACTIVE);

    changeTest changeDuty;
    changeDuty = TEST_vIncrease;
    uint16_t hMessage = 0;
    MESSAGE(Test).hMessageKey = 0;
    MESSAGE(Test).hMessageSize = 1;
    MESSAGE(Test).eMessageType = MT_TWOBYTE;
    MESSAGE(Test).pvMessage = &hMessage;

    CONTRACT(Test).bVersion =1;
    CONTRACT(Test).eOrigin = MODULE_TEST;
    CONTRACT(Test).eTopic = TOPIC_SENSOR;
    CONTRACT(Test).sMessage = &MESSAGE(Test);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bTESTPUBThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    while(1)
    {
        /* Pool the device waiting for */
        hMessage = changeDuty(hMessage, 100);
        if (hMessage >= 1020) changeDuty = TEST_vDecrease;
        else if (hMessage <= 0) changeDuty = TEST_vIncrease;
        osStatus testResult = PUBLISH(CONTRACT(Test), 0);
        (void)testResult;
        WATCHDOG_STATE(TESTPUB, WDT_SLEEP);
        osDelay(2000);
        WATCHDOG_STATE(TESTPUB, WDT_ACTIVE);
    }
    osThreadTerminate(NULL);
}

/* ************************* Main thread ************************************ */
void TEST_vTestThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Test Thread Created");
#endif

    /* Init the module queue - structure that receive data from broker */
    INITIALIZE_QUEUE(TestPubQueue);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while(THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        TEST_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }

    SIGNATURE(Test).eDestine = MODULE_INPUT_KEYPAD;
    SIGNATURE(Test).eTopic = TOPIC_SENSOR;
    SIGNATURE(Test).vpDestineQueue = TestPubQueue;

    osStatus testResult = SUBSCRIBE(SIGNATURE(Test), osWaitForever);
    (void)testResult;

    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId) argument;
    osSignalSet(xMainFromIsobusID, MODULE_TEST);

    /* Start the main functions of the application */
    while (1)
    {
        /* Blocks until any message is published on any topic */
        WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
        osEvent evt = RECEIVE(TestPubQueue, osWaitForever);
        WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
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
    }
    /* Unreachable */
    osThreadSuspend(NULL);
}
