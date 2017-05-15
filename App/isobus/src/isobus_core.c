/****************************************************************************
* Title                 :   isobus_core Source File
* Filename              :   isobus_core.c
* Author                :   Henrique Reis
* Origin Date           :   01 de jul de 2016
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
*  01/07/16   1.0.0      Henrique Reis    isobus_core.c created.
*
*****************************************************************************/
/** @file   isobus_core.c
 *  @brief
 *
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "isobus_core.h"
#include "debug_tool.h"
#include "iso11783.h"
#include "../../isobus/config/isobus_config.h"
#include "isobus_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of SENSOR queue
#define QUEUE_SIZEOFISOBUS (5)

#define THIS_MODULE MODULE_ISOBUS
/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static eAPPError_s eError;                   		//!< Error variable

extern osFlagsGroupId UOS_sFlagSis;

//osThreadId startUpdate;

#ifndef UNITY_TEST
DECLARE_QUEUE(IsobusQueue, QUEUE_SIZEOFISOBUS);     //!< Declaration of Interface Queue
CREATE_SIGNATURE(Isobus);                         	//!< Signature Declarations
#endif

CREATE_CONTRACT(Isobus);                            //!< Create contract for isobus msg publication

/*****************************
 * Local messages queue
 *****************************/
CREATE_LOCAL_QUEUE(PublishQ, ISOBUSMsg, 10)
CREATE_LOCAL_QUEUE(WriteQ, ISOBUSMsg, 10)
CREATE_LOCAL_QUEUE(BootQ, ISOBUSMsg, 10)
CREATE_LOCAL_QUEUE(UpdateQ, ISOBUSMsg, 10)
CREATE_LOCAL_QUEUE(AuxBootQ, ISOBUSMsg, 10)

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
        ISOBUS_MODULES
};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(ISOPUB);                                    //!< WDT pointer flag
WATCHDOG_CREATE(ISORCV);                                    //!< WDT pointer flag
WATCHDOG_CREATE(ISOWRT);                                    //!< WDT pointer flag
WATCHDOG_CREATE(ISOBOOT);                                   //!< WDT pointer flag
WATCHDOG_CREATE(ISOUPDT);                                   //!< WDT pointer flag
uint8_t bISOPUBThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISORCVThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISOWRTThreadArrayPosition = 0;                     //!< Thread position in array
uint8_t bISOBOOTThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bISOUPDTThreadArrayPosition = 0;                    //!< Thread position in array

WATCHDOG_CREATE(ISOAUX);                                   //!< WDT pointer flag
uint8_t bISOAUXThreadArrayPosition = 0;                    //!< Thread position in array

osThreadId xBootThreadId;                                  // Holds the BootThreadId
osThreadId xAuxBootThreadId;                               // Holds the AuxBootThreadId

// Holds the current module state
eModuleStates eModCurrState;
VTStatus sVTCurrentStatus;                          //!< Holds the current VT status
peripheral_descriptor_p pISOHandle = NULL;          //!< ISO Handler

extern unsigned int POOL_SIZE;
extern uint8_t pool[];

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * ISO_WDTData(uint8_t * pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void ISO_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
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
 * Function : ISO_vCreateThread(const Threads_t sSensorThread )
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
  * @see ISO_vCreateThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  *******************************************************************************/
static void ISO_vCreateThread(const Threads_t sThread )
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());

    // Holds the BootThreadId, note that BootThreadId is the third position at THREADS_THISTHREAD array
    (sThread.thisWDTPosition == 3) ? xBootThreadId = xThreads : xBootThreadId;

    // Holds the AuxBootThreadId, note that AuxBootThreadId is the sixth position at THREADS_THISTHREAD array
    (sThread.thisWDTPosition == 6) ? xAuxBootThreadId = xThreads : xAuxBootThreadId;

    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
    }
}

/******************************************************************************
 * Function : ISO_eReceivePooling(void)
 *//**
 * \b Description:
 *
 * This is the main routine of the ISO_vIsobusPublishThread task. It performs a call to the
 * device read function and check if there is a received message on the buffer. The received
 * message is then published to the ISOBUS mod topic.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: none
 *
 * @return Number of bytes received by the device and published to the topic.
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * //Called from DIG_vDiagnosticPublishThread
~~~~~~~~~~~~~~~
  *
  * @see DIG_vDiagnosticThread, DIG_vDiagnosticPublishThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  * <table align="left" style="width:800px">
  * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
  * <tr><td> 04/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
  * </table><br><br>
  * <hr>
  *
  *******************************************************************************/

uint32_t ISO_eReceivePooling(void)
{
    /* Check if the receive buffer has some data */
    uint8_t abTempBuffer[256];
    uint32_t wReceiveLenght = DEV_read(pISOHandle, abTempBuffer, 256);

    if (wReceiveLenght)
    {
        /* Publish the array at the DIAGNOSTIC topic */
        MESSAGE_HEADER(Isobus, wReceiveLenght, 1, MT_ARRAYBYTE);
        MESSAGE_PAYLOAD(Isobus) = (void*) abTempBuffer;
        PUBLISH(CONTRACT(Isobus), 0);
    }    
    return wReceiveLenght;
}
#endif

/******************************************************************************
 * Function : ISO_eInitIsobusPublisher(void)
 *//**
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
  * //Called from ISO_vIsobusPublishThread
~~~~~~~~~~~~~~~
  *
  * @see ISO_vIsobusThread, ISO_vIsobusPublishThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  *
  *******************************************************************************/
eAPPError_s ISO_eInitIsobusPublisher(void)
{
    /* Check if handler is already enabled */
    if (pISOHandle == NULL)
    {
        return APP_ERROR_ERROR;
    }

    //Prepare Default Contract/Message
    MESSAGE_HEADER(Isobus, 1, ISOBUS_DEFAULT_MSGSIZE, MT_BYTE); // MT_ARRAYBYTE
    CONTRACT_HEADER(Isobus, 1, THIS_MODULE, TOPIC_ISOBUS);

    return APP_ERROR_SUCCESS;
}


/******************************************************************************
 * Function : ISO_vIsobusPublishThread(void const *argument)
 *//**
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
void ISO_vIsobusPublishThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Publish Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOPUB), &bISOPUBThreadArrayPosition, (void*)ISO_vIsobusPublishThread);
    WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bISOPUBThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    INITIALIZE_LOCAL_QUEUE(PublishQ);           //!< Initialise message queue to publish thread

    ISOBUSMsg* recv;

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOPUB, WDT_SLEEP);
        osEvent evtPub = RECEIVE_LOCAL_QUEUE(PublishQ, osWaitForever);   // Wait
        WATCHDOG_STATE(ISOPUB, WDT_ACTIVE);

        if(evtPub.status == osEventMessage)
        {
            recv = (ISOBUSMsg*) evtPub.value.p;
            // Send messages received to the other threads
            PUT_LOCAL_QUEUE(WriteQ, *((ISOBUSMsg*)recv), 0);
        }
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusPublishThread(void const *argument){}
#endif

/******************************************************************************
 * Function : ISO_vInitDeviceLayer(uint32_t wSelectedInterface)
 *//**
 * \b Description:
 *
 * This function will init the M2GISOCOMM instance used by the Isobus module.
 * The function parameter indicates the selected interface which will output the
 * messages.
 *
 * PRE-CONDITION: Isobus core initialized
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * #define UART_IFACE 0x01
  * #define USB_IFACE  0x02
  * #define CAN_IFACE  0x04
  *
  *	if (DIG_vInitDeviceLayer(UART_IFACE) != DEV_ERROR_SUCCESS)
  *		printf("error");
~~~~~~~~~~~~~~~
  *
  * @see COM_vCBACommThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  * <table align="left" style="width:800px">
  * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
  * <tr><td> 04/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
  * </table><br><br>
  * <hr>
  *
  *******************************************************************************/
eAPPError_s ISO_vInitDeviceLayer(uint32_t wSelectedInterface)
{
    /*Prepare the device */
    pISOHandle = DEV_open(PERIPHERAL_M2GISOCOMM);
    ASSERT(pISOHandle != NULL);

    /* Set device output to CAN peripheral */
    eError = (eAPPError_s) DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_SET_ACTIVE, (void*)&wSelectedInterface);
    ASSERT(eError == APP_ERROR_SUCCESS);

    /* Configurations for CAN multipacket communications */
    if (wSelectedInterface == ISOBUS_DEV_CAN)
    {
        /* Enables the reception of frames with the ISO_INIT_CAN_ID id number (29 bits) */
        uint32_t wCANInitID = 0x00000000;
        eError = (eAPPError_s) DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_ADD_ALL_CAN_ID, (void*)&wCANInitID);
        ASSERT(eError == APP_ERROR_SUCCESS);
    }    
    return eError;
}

/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void ISO_vIsobusThread (void const *argument)
{

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Thread Created");
#endif
    eModCurrState = BOOT;       // Module is in boot process

    /* Init the module queue - structure that receive data from broker */
    INITIALIZE_QUEUE(IsobusQueue);

    /* Prepare the signature - struture that notify the broker about subscribers */
//    SIGNATURE_HEADER(Diagnostic, THIS_MODULE, TOPIC_ALL, DiagnosticQueue);
//    ASSERT(SUBSCRIBE(SIGNATURE(Diagnostic), 0) == osOK);

    /* Init M2GISOCOMM device for output */
    ISO_vInitDeviceLayer(ISO_INITIAL_IO_IFACE);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while(THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        ISO_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }

    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId) argument;
    osSignalSet(xMainFromIsobusID, MODULE_ISOBUS);

    /* Start the main functions of the application */
    while (1)
    {
        /* Blocks until any message is published on ISOBUS topic */
        WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
        osDelay(2000);
        //        osEvent evt = RECEIVE(IsobusQueue, osWaitForever);
        WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
    }

    /* Unreachable */
    osThreadSuspend(NULL);
}
#else
void ISO_vIsobusThread (void const *argument){}
#endif

/* *
 * Get the PGN from an message
 * */
uint32_t ISO_wGetPGN(ISOBUSMsg* RcvMsg) {
    return (RcvMsg->PF < 240) ? (((RcvMsg->frame.id >> 8) & 0x3FFFF) - RcvMsg->PS) : ((RcvMsg->frame.id >> 8) & 0x3FFFF);
} 

void ISO_vIsobusDispatcher(ISOBUSMsg* RcvMsg)
{
    uint32_t wRcvMsgPGN = ISO_wGetPGN(RcvMsg);

    // Two types of behavior...
    // Boot mode and running mode... Must be implemented...
    // This function must send the received messages to the responsably threads...
    // BootNControl Thread and UpdateOP Thread

    // On boot status, all the received message must be sent to BootNControl thread
    // On running status, this function must dispatch to the correct thread
    if((RcvMsg->PF < 240) && (wRcvMsgPGN != REQUEST_PGN_MSG_PGN))  // Destination PGN
    {
        if(RcvMsg->PS == M2G_SOURCE_ADDRESS)
        {
            if(RcvMsg->SA == VT_ADDRESS)
            {
                switch(wRcvMsgPGN)
                {
                    case ETP_DATA_TRANSFER_PGN:
                    case ETP_CONN_MANAGE_PGN:
                    case VT_TO_ECU_PGN:
                    case PROCESS_DATA_PGN:
                    case ACKNOWLEDGEMENT_PGN:
                    case TP_DATA_TRANSFER_PGN:
                    case TP_CONN_MANAGE_PGN:
                    case ADDRESS_CLAIM_PGN:
                    case PROPRIETARY_A_PGN:
                    case PROPRIETARY_A2_PGN:
                        // Send message to BootThread
                        PUT_LOCAL_QUEUE(BootQ, *RcvMsg, 0);
                        break;
                    default:
                        break;
                }
            } else{
            }
        } else if(RcvMsg->PS == BROADCAST_ADDRESS)
        {
            if(RcvMsg->SA == VT_ADDRESS)
            {
                switch(wRcvMsgPGN)
                {
                    case VT_TO_ECU_PGN:
                        // Send message to BootThread
                        PUT_LOCAL_QUEUE(BootQ, *RcvMsg, 0);
                        break;
                    case ECU_TO_GLOBAL_PGN:
                        break;
                    default:
                        break;
                }
            } else{
            }
        }
    } else if(RcvMsg->PF >= 240)        // No-Destination PGN
    {
        if(RcvMsg->SA == VT_ADDRESS)
        {
            switch(wRcvMsgPGN)
            {
                // SAVE 8 BYTES OF DATA IN DEDICATED BUFFER
                default:
                    break;
            }
        } else{
            // SAVE 12 BYTE MESSAGE (ID AND DATA) IN CIRCULAR QUEUE
        }
    }
}

/******************************************************************************
 * Function : ISO_vIsobusPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will send to the responsible thread on module.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * //Created from ISO_vIsobusThread,
~~~~~~~~~~~~~~~
  *
  * @see ISO_vIsobusThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  *
  *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusRecvThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Recv Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISORCV), &bISORCVThreadArrayPosition, (void*)ISO_vIsobusRecvThread);
    WATCHDOG_STATE(ISORCV, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISORCVThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    uint8_t bIterator;
    uint8_t bRecvMessages = 0;      //!< Lenght (messages) received
    ISOBUSMsg asPayload[32];   //!< Buffer to hold the contract and message data

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISORCV, WDT_ACTIVE);
        osDelay(250); //Wait
        bRecvMessages = DEV_read(pISOHandle, &asPayload[0].frame, sizeof(asPayload));

        if(bRecvMessages)
        {
            for(bIterator = 0; bIterator < bRecvMessages; bIterator++)
            {
                // Call an function to treat ISOBUS messages
                ISO_vIsobusDispatcher(&asPayload[bIterator]);
            }
        }
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusRecvThread(void const *argument){}
#endif

/******************************************************************************
 * Function : ISO_vIsobusWriteThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will write the incoming messages to the device.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * //Created from ISO_vIsobusThread,
~~~~~~~~~~~~~~~
  *
  * @see ISO_vIsobusThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  *
  *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusWriteThread(void const *argument)
{    
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Write Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOWRT), &bISOWRTThreadArrayPosition, (void*)ISO_vIsobusWriteThread);
    WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOWRTThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    ISOBUSMsg* recv;
    eAPPError_s eError;

    INITIALIZE_LOCAL_QUEUE(WriteQ);

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOWRT, WDT_SLEEP);
        osEvent evtPub = RECEIVE_LOCAL_QUEUE(WriteQ, osWaitForever);   // Wait
        WATCHDOG_STATE(ISOWRT, WDT_ACTIVE);

        if(evtPub.status == osEventMessage)
        {
            recv = (ISOBUSMsg*) evtPub.value.p;

            eError = (eAPPError_s) DEV_ioctl(pISOHandle, IOCTL_M2GISOCOMM_CHANGE_SEND_ID, (void*)&(recv->frame).id);
            ASSERT(eError == APP_ERROR_SUCCESS);

            if(eError == APP_ERROR_SUCCESS)     // wSendCANID changed
            {
                DEV_write(pISOHandle, &((recv->frame).data[0]), (recv->frame).dlc);
            }
        }
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusWriteThread(void const *argument){}
#endif

void ISO_vTimerCallbackWSMaintenance(void const *arg)
{
    ISO_vSendWorkingSetMaintenance(false);
}

#ifndef UNITY_TEST
void ISO_vIsobusAuxBootThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Aux Boot Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOAUX), &bISOAUXThreadArrayPosition, (void*)ISO_vIsobusAuxBootThread);
    WATCHDOG_STATE(ISOAUX, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOAUXThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    ISOBUSMsg* RcvMsg;
    eBootStates eCurrState;

    INITIALIZE_LOCAL_QUEUE(AuxBootQ);

    CREATE_TIMER(WSMaintenanceTimer, ISO_vTimerCallbackWSMaintenance);
    INITIALIZE_TIMER(WSMaintenanceTimer, osTimerPeriodic);

    // Inform BootThread that AuxBootThread already start
    WATCHDOG_STATE(ISOAUX, WDT_SLEEP);
    osSignalSet(xBootThreadId, 0xFF);
    WATCHDOG_STATE(ISOAUX, WDT_ACTIVE);

    // Wait for an VT status message
    WATCHDOG_STATE(ISOAUX, WDT_SLEEP);
    osSignalWait(WAIT_GLOBAL_VT_STATUS, osWaitForever);
    WATCHDOG_STATE(ISOAUX, WDT_ACTIVE);

    // Send a request address claim message
    ISO_vSendRequest(ADDRESS_CLAIM_PGN);

    // Send a address claim message
    ISO_vSendAddressClaimed();

    // Wait for an VT status message
    eCurrState = WAIT_VT_STATUS;

    while(1)
    {
        while(eCurrState != BOOT_COMPLETED)
        {
            WATCHDOG_STATE(ISOAUX, WDT_SLEEP);
            osSignalWait(eCurrState, osWaitForever);
            WATCHDOG_STATE(ISOAUX, WDT_ACTIVE);

            switch(eCurrState)
            {
                case WAIT_VT_STATUS:
                    // Send a working set master message
                    ISO_vSendWorkingSetMaster();
                    // Send a working set maintenance (first)
                    // Send a proprietary A message
                    ISO_vSendProprietaryA();
                    // Send a command language message
                    ISO_vSendRequest(LANGUAGE_PGN);
                    // Send a get hardware message
                    ISO_vSendGetHardware();
                    // Send a proprietary A message
                    ISO_vSendProprietaryA();
                    // Send a proprietary A message
                    ISO_vSendProprietaryA();
                    // Send a get memory message
                    ISO_vSendGetMemory(POOL_SIZE);
                    // Send a load version message
                    ISO_vSendLoadVersion(0xAAAAAAABAAAAAA);

                    eCurrState = WAIT_LOAD_VERSION;
                    break;
                case WAIT_LOAD_VERSION:
                    // Send a request to send message
                    ISO_vSendRequestToSend();
                    eCurrState = WAIT_SEND_POOL;
                    break;
                case WAIT_SEND_POOL:
                    ISO_vInitPointersToTranfer(pool, POOL_SIZE);
                    // Waits for a CTS message
                    // While object pool pointer not equal to NULL
                    do{
                        WATCHDOG_STATE(ISOAUX, WDT_SLEEP);
                        osEvent evtPub = RECEIVE_LOCAL_QUEUE(AuxBootQ, osWaitForever);   // Wait
                        WATCHDOG_STATE(ISOAUX, WDT_ACTIVE);

                        if(evtPub.status == osEventMessage)
                        {
                            RcvMsg = (ISOBUSMsg*) evtPub.value.p;

                            switch(ISO_wGetPGN(RcvMsg))
                            {
                                case TP_CONN_MANAGE_PGN:
                                    switch (RcvMsg->B1) {
                                        case TP_CM_CTS:
                                            ISO_vSendObjectPool(RcvMsg->B2, RcvMsg->B3, TRANSPORT_PROTOCOL);
                                            break;
                                        case TP_EndofMsgACK:
                                            eCurrState = OBJECT_POOL_SENDED;
                                            break;
                                        case TP_Conn_Abort:
                                            eCurrState = WAIT_VT_STATUS;
                                            break;
                                        case TP_BAM:
                                        case TP_CM_RTS:
                                        default:
                                            break;
                                    }
                                    break;
                                case ETP_CONN_MANAGE_PGN:
                                    switch (RcvMsg->B1) {
                                        case ETP_CM_CTS:
                                            // Send DPO message
                                            ISO_vSendETP_CM_DPO(RcvMsg->B2, (RcvMsg->B3|(RcvMsg->B4 << 8)|(RcvMsg->B5 << 16)));
                                            ISO_vSendObjectPool(RcvMsg->B2, (RcvMsg->B3|(RcvMsg->B4 << 8)|(RcvMsg->B5 << 16)), EXTENDED_TRANSPORT_PROTOCOL);
                                            break;
                                        case ETP_CM_EOMA:
                                            eCurrState = OBJECT_POOL_SENDED;
                                            break;
                                        case ETP_Conn_Abort:
                                            // We have to skip the do...while loop...
                                            eCurrState = WAIT_VT_STATUS;
                                            break;
                                        case ETP_CM_DPO:
                                        case ETP_CM_RTS:
                                        default:
                                            break;
                                    }
                                    break;
                            }
                        }
                    } while(eCurrState != OBJECT_POOL_SENDED);

                    // Create a new state in this machine, to restart the pool send
                    ISO_vSendEndObjectPool();
                    ISO_vSendWSMaintenancePoolSent();
                    ISO_vSendLoadVersion(0xAAAAAAABAAAAAA);

                    START_TIMER(WSMaintenanceTimer, 1000);

                    // The boot process are completed, so terminate this thread
                    eCurrState = BOOT_COMPLETED;
                    eModCurrState = RUNNING;
//                    osSignalSet(startUpdate, 0xAF);
                    osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK);
                    break;
                default:
                    break;
            } // End of switch statement
        } // End of while
        WATCHDOG_STATE(ISOAUX, WDT_SLEEP);
        osDelay(5000);
        WATCHDOG_STATE(ISOAUX, WDT_ACTIVE);
        // TODO: Change task priority and suspend this thread. Useful only when BOOT is not completed
    } // End of while
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusAuxBootThread(void const *argument){}
#endif

void ISO_vIsobusUpdateVTStatus(ISOBUSMsg* RcvMsg)
{
    if(RcvMsg->B1 != FUNC_VT_STATUS)
    {
        return;
    }

    sVTCurrentStatus.bActiveWSM = RcvMsg->B2;
    sVTCurrentStatus.wMaskActWS = RcvMsg->B3 | (RcvMsg->B4 << 8);
    sVTCurrentStatus.wSoftActWS = RcvMsg->B5 | (RcvMsg->B6 << 8);
    sVTCurrentStatus.bBusyUpdMask = RcvMsg->B7 & 0x01;
    sVTCurrentStatus.bBusySavMem = RcvMsg->B7 & 0x02;
    sVTCurrentStatus.bBusyExcCmd = RcvMsg->B7 & 0x04;
    sVTCurrentStatus.bBusyExcMac = RcvMsg->B7 & 0x08;
    sVTCurrentStatus.bBusyParsOP = RcvMsg->B7 & 0x10;
    sVTCurrentStatus.bBusyReserv = RcvMsg->B7 & 0x20;
    sVTCurrentStatus.bAuxCtrlAct = RcvMsg->B7 & 0x40;
    sVTCurrentStatus.bVTOutMemory = RcvMsg->B7 & 0x80;
    sVTCurrentStatus.bVTFuncCode = RcvMsg->B8;
}

void ISO_vTreatBootState(ISOBUSMsg* sRcvMsg)
{
    switch(ISO_wGetPGN(sRcvMsg))
    {
        case VT_TO_ECU_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                switch(sRcvMsg->B1)
                {
                    case FUNC_LOAD_VERSION:
                        osSignalSet(xAuxBootThreadId, WAIT_LOAD_VERSION);
                        break;
                    case FUNC_VT_STATUS:
                        osSignalSet(xAuxBootThreadId, WAIT_VT_STATUS);
                        break;
                    default:
                        break;
                }
            } else if(sRcvMsg->PS == BROADCAST_ADDRESS){
                switch(sRcvMsg->B1)
                {
                    case FUNC_VT_STATUS:
                        osSignalSet(xAuxBootThreadId, WAIT_GLOBAL_VT_STATUS);
                        break;
                    default:
                        break;
                }
            }
            break;
        case TP_CONN_MANAGE_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                osSignalSet(xAuxBootThreadId, WAIT_SEND_POOL);
                PUT_LOCAL_QUEUE(AuxBootQ, *sRcvMsg, 0);
            }
            break;
        case ETP_CONN_MANAGE_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                osSignalSet(xAuxBootThreadId, WAIT_SEND_POOL);
                PUT_LOCAL_QUEUE(AuxBootQ, *sRcvMsg, 0);
            }
            break;
        default:
            break;
    }
}

void ISO_vTreatRunningState(ISOBUSMsg* sRcvMsg)
{
    switch(ISO_wGetPGN(sRcvMsg))
    {
        case VT_TO_ECU_PGN:
            if(sRcvMsg->PS == M2G_SOURCE_ADDRESS)
            {
                switch(sRcvMsg->B1)
                {
                    case FUNC_SOFT_KEY_ACTIVATION:
                        break;
                    case FUNC_BUTTON_ACTIVATION:
                        break;
                    case FUNC_POINTING_EVENT:
                        break;
                    case FUNC_SELECT_INPUT_OBJECT:
                        break;
                    case FUNC_ESC_COMMAND:
                        break;
                    case FUNC_CHANGE_NUMERIC_VALUE:
                        break;
                    case FUNC_CHANGE_ACTIVE_MASK:
                        break;
                    default:
                        break;
                }
            } else if(sRcvMsg->PS == BROADCAST_ADDRESS){
                switch(sRcvMsg->B1)
                {
                    case FUNC_VT_STATUS:
                        ISO_vIsobusUpdateVTStatus(sRcvMsg);
                        break;
                    default:
                        break;
                }
            }
            break;
        case TP_CONN_MANAGE_PGN:
        case ETP_CONN_MANAGE_PGN:
            break;
        default:
            break;
    }
}

/******************************************************************************
 * Function : ISO_vIsobusBootThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * //Created from ISO_vIsobusThread,
~~~~~~~~~~~~~~~
  *
  * @see ISO_vIsobusThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  *
  *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusBootThread(void const *argument)
{    
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Boot Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOBOOT), &bISOBOOTThreadArrayPosition, (void*)ISO_vIsobusBootThread);
    WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOBOOTThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    INITIALIZE_LOCAL_QUEUE(BootQ);    

    ISOBUSMsg* sRcvMsg;
    osEvent evt;

    // Wait for auxiliary thread start
    /* Pool the device waiting for */
    WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
    osSignalWait(0xFF, osWaitForever);
    WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOBOOT, WDT_SLEEP);
        evt = RECEIVE_LOCAL_QUEUE(BootQ, osWaitForever);   // Wait
        WATCHDOG_STATE(ISOBOOT, WDT_ACTIVE);

        if(evt.status == osEventMessage)
        {
            sRcvMsg = (ISOBUSMsg*) evt.value.p;

            switch(eModCurrState)
            {
                case BOOT:
                    ISO_vTreatBootState(sRcvMsg);
                    break;
                case RUNNING:
                    ISO_vTreatRunningState(sRcvMsg);
                    break;
                default:
                    break;
            }
        }        
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusWriteThread(void const *argument){}
#endif

#define N_ROWS 36
#define N_COLUMNS 2

uint16_t dBarGraphXOutputNumber[N_ROWS][N_COLUMNS] = 
{
    {0x7000, 0x8009},
    {0x7001, 0x800A},
    {0x7002, 0x800B},
    {0x7003, 0x800C},
    {0x7004, 0x800D},
    {0x7005, 0x800E},
    {0x7006, 0x800F},
    {0x7007, 0x8010}    
};


void ISO_UpdateBarGraph(uint16_t wOutputNumberID, uint32_t dNumericValue)
{
    ISOBUSMsg sSendMsg;    
    
    sSendMsg.frame.id = 0x18E72687;
    sSendMsg.frame.dlc = 8;
    
    sSendMsg.frame.data[0] = 0xA8;
    sSendMsg.frame.data[1] = (wOutputNumberID & 0xFF00) >> 8;
    sSendMsg.frame.data[2] = wOutputNumberID & 0xFF;
    sSendMsg.frame.data[3] = 0xFF;
    sSendMsg.frame.data[4] = dNumericValue & 0xFF;
    sSendMsg.frame.data[5] = (dNumericValue & 0xFF00) >> 8;
    sSendMsg.frame.data[6] = (dNumericValue & 0xFF0000) >> 16;
    sSendMsg.frame.data[7] = (dNumericValue & 0xFF000000) >> 24;
    
    PUT_LOCAL_QUEUE(WriteQ, sSendMsg, 0);
}

void ISO_UpdateBarGraphColor(uint16_t wBarGraphID, uint32_t dNumericValue)
{
    ISOBUSMsg sSendMsg;    
    
    sSendMsg.frame.id = getID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, VT_ADDRESS, PRIORITY);
    sSendMsg.frame.dlc = 8;
  
    sSendMsg.frame.data[0] = FUNC_CHANGE_ATTRIBUTE;
    sSendMsg.frame.data[1] = (wBarGraphID & 0xFF00) >> 8;
    sSendMsg.frame.data[2] = wBarGraphID & 0xFF;
    sSendMsg.frame.data[3] = 0x03;
    sSendMsg.frame.data[4] = dNumericValue & 0xFF;
    sSendMsg.frame.data[5] = (dNumericValue & 0xFF00) >> 8;
    sSendMsg.frame.data[6] = (dNumericValue & 0xFF0000) >> 16;
    sSendMsg.frame.data[7] = (dNumericValue & 0xFF000000) >> 24;
    
    PUT_LOCAL_QUEUE(WriteQ, sSendMsg, 0);
}

/******************************************************************************
 * Function : ISO_vIsobusUpdateOPThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the Isobus module. It will refresh the object pool.
 *
 * PRE-CONDITION: Isobus core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
~~~~~~~~~~~~~~~{.c}
  * //Created from ISO_vIsobusThread,
~~~~~~~~~~~~~~~
  *
  * @see ISO_vIsobusThread
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  *
  *******************************************************************************/
#ifndef UNITY_TEST
void ISO_vIsobusUpdateOPThread(void const *argument)
{    
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus UpdateOP Thread Created");
#endif

    ISO_vDetectThread(&WATCHDOG(ISOUPDT), &bISOUPDTThreadArrayPosition, (void*)ISO_vIsobusUpdateOPThread);
    WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

    osThreadId xIsoMainID = (osThreadId) argument;
    osSignalSet(xIsoMainID, THREADS_RETURN_SIGNAL(bISOUPDTThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);

    INITIALIZE_LOCAL_QUEUE(UpdateQ);    
    uint16_t dBarGraphID = 0x7000;
    uint16_t dOutputNumberID = 0x8009;
    uint8_t bIterator;
    uint8_t gap = 0;
    uint8_t random = 0x00;
    
    (void) dBarGraphID;
    
//    startUpdate = osThreadGetId();

//    WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
//    osSignalWait(0xAF, osWaitForever);
//    WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
    
    // Waiting for RUNNING module state
    WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
//    while(eModCurrState != RUNNING){};
    WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);

    while(1)
    {
        // Receive a message...

        if(eModCurrState == RUNNING)
        {
            /* Pool the device waiting for */
            WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
            osDelay(10000);
            WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
        }
        /* Pool the device waiting for */
        WATCHDOG_STATE(ISOUPDT, WDT_SLEEP);
        osDelay(10000);
        WATCHDOG_STATE(ISOUPDT, WDT_ACTIVE);
    }
    osThreadTerminate(NULL);
}
#else
void ISO_vIsobusWriteThread(void const *argument){}
#endif
