/****************************************************************************
* Title                 :   buzzer_core Source File
* Filename              :   buzzer_core.c
* Author                :   Henrique Reis
* Origin Date           :
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
*    Date    Version        Author              Description
*  XX/XX/XX   1.0.0      Henrique Reis    buzzer_core.c created.
*
*****************************************************************************/
/** @file   buzzer_core.c
 *  @brief
 *
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "M2G_app.h"
#include "buzzer_core.h"
#include "debug_tool.h"
#include "../../buzzer/config/buzzer_config.h"
#include "buzzer_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
//!< MACRO to define the size of BUZZER queue
#define QUEUE_SIZEOFBUZZER (5)

#define THIS_MODULE MODULE_BUZZER

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static eAPPError_s eError; //!< Error variable

DECLARE_QUEUE(BuzzerQueue, QUEUE_SIZEOFBUZZER); //!< Declaration of Interface Queue
CREATE_SIGNATURE(Buzzer);                       //!< Signature Declarations
CREATE_CONTRACT(Buzzer);                        //!< Create contract for buzzer msg publication

/**
* Module Threads
*/
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
    BUZZER_MODULES};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

WATCHDOG_CREATE(BUZPUB);                //!< WDT pointer flag
uint8_t bBUZPUBThreadArrayPosition = 0; //!< Thread position in array

peripheral_descriptor_p pIntBuzzerHandle = NULL; //!< Internal BUZZER Handler
peripheral_descriptor_p pExtBuzzerHandle = NULL; //!< External BUZZER Handler

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
uint8_t *BUZ_WDTData(uint8_t *pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t *)WATCHDOG_FLAG_ARRAY;
}

inline void BUZ_vDetectThread(thisWDTFlag *flag, uint8_t *bPosition, void *pFunc)
{
    *bPosition = 0;
    while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
    {
        (*bPosition)++;
    }
    *flag = (uint8_t *)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

#ifndef UNITY_TEST
/******************************************************************************
* Function : BUZ_vCreateThread(const Threads_t sSensorThread )
*/ /**
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
* @see ISO_vCreateThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*******************************************************************************/
static void BUZ_vCreateThread(const Threads_t sThread)
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void *)osThreadGetId());

    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
    }
}
#endif

/******************************************************************************
* Function : BUZ_eInitBuzzerPublisher(void)
*/ /**
* \b Description:
*
* This routine prepares the contract and message that the ISO_vIsobusPublishThread thread
* will publish to the broker.
*
* PRE-CONDITION: none
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Called from
~~~~~~~~~~~~~~~
*
* @see ISO_vIsobusThread, ISO_vIsobusPublishThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
eAPPError_s BUZ_eInitBuzzerPublisher(void)
{
    /* Check if handler is already enabled */
    if ((pIntBuzzerHandle == NULL) || (pExtBuzzerHandle == NULL))
    {
        return APP_ERROR_ERROR;
    }

    //Prepare Default Contract/Message
    MESSAGE_HEADER(Buzzer, 1, BUZZER_DEFAULT_MSGSIZE, MT_BYTE); // MT_ARRAYBYTE
    CONTRACT_HEADER(Buzzer, 1, THIS_MODULE, TOPIC_BUZZER);

    return APP_ERROR_SUCCESS;
}

    /******************************************************************************
* Function : BUZ_vIsobusPublishThread(void const *argument)
*/ /**
* \b Description:
*
* This is a thread of the Isobus module. It will poll the receive buffer of the device
* and in case of any incoming message, it will publish on the ISOBUS topic.
*
* PRE-CONDITION: Diagnostic core initialized, interface enabled.
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Created from ISO_vDiagnosticThread,
~~~~~~~~~~~~~~~
*
* @see ISO_vIsobusThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
#ifndef UNITY_TEST
void BUZ_vBuzzerPublishThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Buzzer Publish Thread Created");
#endif

    BUZ_vDetectThread(&WATCHDOG(BUZPUB), &bBUZPUBThreadArrayPosition, (void *)BUZ_vBuzzerPublishThread);
    WATCHDOG_STATE(BUZPUB, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId)argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bBUZPUBThreadArrayPosition)); //Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    BUZ_eInitBuzzerPublisher();

    while (1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(BUZPUB, WDT_SLEEP);
        osDelay(500);
        WATCHDOG_STATE(BUZPUB, WDT_ACTIVE);
    }
    osThreadTerminate(NULL);
}
#else
void BUZ_vBuzzerPublishThread(void const *argument)
{
}
#endif

/******************************************************************************
* Function : BUZ_vInitDeviceLayer()
*/ /**
* \b Description:
*
* This function will init the CBT09427 instance used by the Buzzer module.
*
* PRE-CONDITION: Buzzer core initialized
*
* POST-CONDITION: none
*
* @return     void
*
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
eAPPError_s BUZ_vInitDeviceLayer()
{
#if BUZZER_DUAL
    /*Prepare the internal device */
    pIntBuzzerHandle = DEV_open(BUZZER_INT);
    ASSERT(pIntBuzzerHandle != NULL);

    /*Prepare the external device */
    pExtBuzzerHandle = DEV_open(BUZZER_EXT);
    ASSERT(pExtBuzzerHandle != NULL);

    eError = APP_ERROR_SUCCESS;
#elif BUZZER_INTERNAL_ONLY
    /*Prepare the device */
    pIntBuzzerHandle = DEV_open(BUZZER_INT);
    ASSERT(pIntBuzzerHandle != NULL);

    eError = APP_ERROR_SUCCESS;
#elif BUZZER_EXTERNAL_ONLY
    /*Prepare the device */
    pExtBuzzerHandle = DEV_open(BUZZER_EXT);
    ASSERT(pExtBuzzerHandle != NULL);

    eError = APP_ERROR_SUCCESS;
#else
#error "An buzzer operation mode should be selected..."
#endif
    return eError;
}

/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void BUZ_vBuzzerThread(void const *argument)
{

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Buzzer Thread Created");
#endif

    /* Init the module queue - structure that receive data from broker */
    INITIALIZE_QUEUE(BuzzerQueue);

    /* Init buzzer device for output */
    BUZ_vInitDeviceLayer();

    /* Prepare the signature - struture that notify the broker about subscribers */
    SIGNATURE_HEADER(Buzzer, THIS_MODULE, TOPIC_DIAGNOSTIC, BuzzerQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(Buzzer), 0) == osOK);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while (THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        BUZ_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }
    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId)argument;
    osSignalSet(xMainFromIsobusID, MODULE_BUZZER);

    uint8_t *pRecvBuffer;
    messageType_e eMsgType;
    uint16_t hMsgSize;
    bool bOnBuzzer = false;

    /* Start the main functions of the application */
    while (1)
    {
        /* Blocks until any message is published on any topic */
        WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
        osEvent evt = RECEIVE(BuzzerQueue, osWaitForever);
        WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
        if (evt.status == osEventMessage)
        {
            eMsgType = GET_MESSAGE(GET_CONTRACT(evt))->eMessageType;

            hMsgSize = GET_MESSAGE(GET_CONTRACT(evt))->hMessageSize;

            pRecvBuffer = (uint8_t *)GET_MESSAGE(GET_CONTRACT(evt))->pvMessage;

            switch (pRecvBuffer[0])
            {
                case 'A':
                    bOnBuzzer = true;
                    DEV_ioctl(pIntBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    break;
                case 'B':
                    bOnBuzzer = false;
                    DEV_ioctl(pIntBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    break;
                case 'C':
                    bOnBuzzer = true;
                    DEV_ioctl(pExtBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    break;
                case 'D':
                    bOnBuzzer = false;
                    DEV_ioctl(pExtBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    break;
                case 'F':
                    bOnBuzzer = true;
                    DEV_ioctl(pIntBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    DEV_ioctl(pExtBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    break;
                case 'G':
                    bOnBuzzer = false;
                    DEV_ioctl(pIntBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    DEV_ioctl(pExtBuzzerHandle, IOCTL_CBT_TURN_ON_OFF, &bOnBuzzer);
                    break;
                default:
                    break;
            }
            (void)evt;
        }
    }
    /* Unreachable */
    osThreadSuspend(NULL);
}
#else
void BUZ_vBuzzerThread(void const *argument)
{
}
#endif
