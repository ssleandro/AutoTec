/****************************************************************************
* Title                 :   gui_core
* Filename              :   gui_core.c
* Author                :   Henrique Reis
* Origin Date           :   19 de abr de 2017
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
*    Date    Version       Author          Description
*  19/04/17   1.0.0     Henrique Reis         gui_core.c created.
*
*****************************************************************************/

/******************************************************************************
* Includes
*******************************************************************************/
#include "M2G_app.h"
#include "gui_core.h"
#include "debug_tool.h"
#include "../../gui/config/gui_config.h"
#include "gui_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
//!< MACRO to define the size of CONTROL queue
#define QUEUE_SIZEOFGUI 5

#define THIS_MODULE MODULE_GUI

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
DECLARE_QUEUE(GuiQueue, QUEUE_SIZEOFGUI);      //!< Declaration of Interface Queue
CREATE_SIGNATURE(Gui);                             //!< Signature Declarations
CREATE_CONTRACT(Gui);                              //!< Create contract for sensor msg publication

/******************************************************************************
* Module typedef
*******************************************************************************/

/**
* Module Threads
*/
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
    GUI_MODULES
};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(GUIPUB);                                   //!< WDT pointer flag
uint8_t bGUIPUBThreadArrayPosition = 0;                    //!< Thread position in array

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
uint8_t * GUI_WDTData(uint8_t * pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void GUI_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
    *bPosition = 0;
    while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
    {
        (*bPosition)++;
    }
    *flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

static void GUI_vCreateThread(const Threads_t sThread )
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
    }
}

void GUI_vGuiPublishThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Control Publish Thread Created");
#endif

    GUI_vDetectThread(&WATCHDOG(GUIPUB), &bGUIPUBThreadArrayPosition, (void*)GUI_vGuiPublishThread);
    WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bGUIPUBThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(GUIPUB, WDT_SLEEP);
        osDelay(2000);
        WATCHDOG_STATE(GUIPUB, WDT_ACTIVE);
    }
    osThreadTerminate(NULL);
}

/* ************************* Main thread ************************************ */
void GUI_vGuiThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Gui Thread Created");
#endif

    /* Init the module queue - structure that receive data from broker */
    INITIALIZE_QUEUE(GuiQueue);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while(THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        GUI_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }

    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId) argument;
    osSignalSet(xMainFromIsobusID, MODULE_GUI);

    /* Start the main functions of the application */
    while (1)
    {
        /* Blocks until any message is published on any topic */
        WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
        osEvent evt = RECEIVE(GuiQueue, osWaitForever);
        WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
        if (evt.status == osEventMessage)
        {
            // Message arrived
            uint32_t wData = *(uint32_t*)GET_MESSAGE(GET_CONTRACT(evt))->pvMessage;
            (void)wData;
        }
    }
    /* Unreachable */
    osThreadSuspend(NULL);
}
