/****************************************************************************
 * Title                 :   diagnostic_core Source File
 * Filename              :   diagnostic_core.c
 * Author                :   Joao Paulo Martins
 * Origin Date           :   08 de abr de 2016
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
 *    Date    Version   Author         				Description
 *  08/04/16   1.0.0  Joao Paulo Martins   diagnostic_core.c file Created.
 *
 *****************************************************************************/
/** @file diagnostic_core.c
 *  @brief
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "M2G_app.h"
#include "diagnostic_core.h"
#include "diagnostic_logger.h"
#include "debug_tool.h"
#include "../../diagnostic/config/diagnostic_config.h"
#include "diagnostic_ThreadControl.h"
#include <stdlib.h>
#if defined(UNITY_TEST)
#include "unity_fixture.h"
#include <stdint.h>
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
//!< MACRO to define the size of SENSOR queue
#define QUEUE_SIZEOFDIAGNOSTIC (256)

#define MAX_LOG_REGISTERS 100

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
#if defined (UNITY_TEST)
#undef PUBLISH
#define PUBLISH(a, b) ((void)0)
#undef ASSERT
#define ASSERT(a) ((void)0)
#endif

#define THIS_MODULE MODULE_DIAGNOSTIC

#define DIAG_TASK1_SIG	(1<<0)
#define DIAG_TASK2_SIG	(2<<0)

#define ORIGIN   spContract->eOrigin
#define TOPIC    spContract->eTopic
#define VERSION  spContract->bVersion
#define MSG_KEY  spContract->sMessage->hMessageKey
#define MSG_TYPE spContract->sMessage->eMessageType
#define MSG_SIZE spContract->sMessage->hMessageSize

#define LOG_HEADER_LENGHT 6 //!< Log header bytes: 4 timestamp + 1 level and type + 1 payload size

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static eAPPError_s eError;                   						//!< Error variable

#ifndef UNITY_TEST
DECLARE_QUEUE(DiagnosticQueue, QUEUE_SIZEOFDIAGNOSTIC); //!< Declaration of Interface Queue
CREATE_SIGNATURE(Diagnostic);//!< Signature Declarations

DECLARE_QUEUE(LocalLoggerQueue, QUEUE_SIZEOFDIAGNOSTIC); //!< Local logger queue
#endif

CREATE_CONTRACT(Diagnostic); //!< Create contract for diagnostic msg publication

/**
 * Module Threads
 */
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
DIAGNOSTIC_MODULES
	};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])]; //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(DIGPUB);//!< WDT pointer flag
WATCHDOG_CREATE(DIGLOG);//!< WDT pointer flag
uint8_t bDIGPUBThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bDIGLOGThreadArrayPosition = 0;                    //!< Thread position in array

peripheral_descriptor_p pTDVHandle = NULL;   //!< TERMDEV Handler
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
//WEAK extern uint32_t DIG_wLoggerWritter(uint32_t wTimestamp, uint8_t bLevelAndType, uint8_t* pbData, uint8_t bDataSize);
WEAK extern void DIG_vLoggerReader (uint8_t* pbData, uint32_t* wDataSize);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
uint8_t * DIG_WDTData (uint8_t * pbNumberOfThreads)
{
	*pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
	return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void DIG_vDetectThread (thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
	*bPosition = 0;
	while (THREADS_THREAD(*bPosition)!= (os_pthread)pFunc)
	{
		(*bPosition)++;
	}
	*flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

#ifndef UNITY_TEST
/******************************************************************************
 * Function : DIG_vCreateThread(const Threads_t sSensorThread )
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
static void DIG_vCreateThread (const Threads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());
	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}

/******************************************************************************
 * Function : DIG_eReceivePooling(void)
 *//**
 * \b Description:
 *
 * This is the main routine of the DIG_vDiagnosticPublishThread task. It performs a call to the
 * device read function and check if there is a received message on the buffer. The received
 * message is then published to the DIAGNOSTIC mod topic.
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

uint32_t DIG_eReceivePooling (void)
{
	/* Check if the receive buffer has some data */
	uint8_t abTempBuffer[256];
	uint32_t wReceiveLenght = DEV_read(pTDVHandle, abTempBuffer, 256);

	if (wReceiveLenght)
	{
		/* Publish the array at the DIAGNOSTIC topic */
		MESSAGE_HEADER(Diagnostic, wReceiveLenght, 1, MT_ARRAYBYTE);
		MESSAGE_PAYLOAD(Diagnostic) = (void*)abTempBuffer;
		PUBLISH(CONTRACT(Diagnostic), 0);
	}

	return wReceiveLenght;
}
#endif

/******************************************************************************
 * Function : DIG_eInitDiagPublisher(void)
 *//**
 * \b Description:
 *
 * This routine prepares the contract and message that the DIG_vDiagnosticPublishThread thread
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
eAPPError_s DIG_eInitDiagPublisher (void)
{
	/* Check if handler is already enabled */
	if (pTDVHandle == NULL)
	{
		return APP_ERROR_ERROR;
	}

	//Prepare Default Contract/Message
	MESSAGE_HEADER(Diagnostic, 1, DIAGNOSTIC_DEFAULT_MSGSIZE, MT_ARRAYBYTE);
	CONTRACT_HEADER(Diagnostic, 1, THIS_MODULE, TOPIC_DIAGNOSTIC);

	return APP_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : DIG_vDiagnosticPublishThread(void const *argument)
 *//**
 * \b Description:
 *
 * This is a thread of the diagnostic module. It will poll the receive buffer of the device
 * and in case of any incoming message, it will publish on the DIAGNOSTIC topic.
 *
 * PRE-CONDITION: Diagnostic core initialized, interface enabled.
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //Created from DIG_vDiagnosticThread,
 ~~~~~~~~~~~~~~~
 *
 * @see DIG_vDiagnosticThread
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
#ifndef UNITY_TEST
void DIG_vDiagnosticPublishThread (void const *argument)
{

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Diagnostic Publish Thread Created");
#endif

	DIG_vDetectThread(&WATCHDOG(DIGPUB), &bDIGPUBThreadArrayPosition, (void*)DIG_vDiagnosticPublishThread);
	WATCHDOG_STATE(DIGPUB, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bDIGPUBThreadArrayPosition)); //Task created, inform core
	osThreadSetPriority(NULL, osPriorityLow);

	DIG_eInitDiagPublisher();

	while (1)
	{
		/* Pool the device waiting for */
		WATCHDOG_STATE(DIGPUB, WDT_SLEEP);
		osDelay(100); //Wait
		WATCHDOG_STATE(DIGPUB, WDT_ACTIVE);
		DIG_eReceivePooling();

	}
	osThreadTerminate(NULL);
}
#else
void DIG_vDiagnosticPublishThread(void const *argument)
{}
#endif

/******************************************************************************
 * Function : DIG_vInitDeviceLayer(uint32_t wSelectedInterface)
 *//**
 * \b Description:
 *
 * This function will init the TERMDEV instance used by the diagnostic module.
 * The function parameter indicates the selected interface which will output the
 * messages.
 *
 * PRE-CONDITION: Diagnostic core initialized
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
eAPPError_s DIG_vInitDeviceLayer (uint32_t wSelectedInterface)
{
	/*Prepare the device */
	pTDVHandle = DEV_open(DIAGNOSTIC_ID);
	ASSERT(pTDVHandle != NULL);

	/* Set device output to UART peripheral */
	eError = (eAPPError_s)DEV_ioctl(pTDVHandle, IOCTL_TDV_SET_ACTIVE, (void*)&wSelectedInterface);
	ASSERT(eError == APP_ERROR_SUCCESS);

	/* Configurations for CAN multipacket communications */
	if (wSelectedInterface == TERMDEV_DEV_CAN)
	{
		/* Enables the reception of frames with the DIG_INIT_CAN_ID id number (11 bits) */
		uint32_t wCANInitID = DIG_INIT_CAN_ID;
		eError = (eAPPError_s)DEV_ioctl(pTDVHandle, IOCTL_TDV_ADD_CAN_ID, (void*)&wCANInitID);
		ASSERT(eError == APP_ERROR_SUCCESS);

		/* Enables the reception of frames with the DIG_SEND_CAN_ID id number (11 bits)
		 * The output messages will be marked with this ID */
		wCANInitID = DIG_SEND_CAN_ID;
		eError = (eAPPError_s)DEV_ioctl(pTDVHandle, IOCTL_TDV_ADD_CAN_ID, (void*)&wCANInitID);
		ASSERT(eError == APP_ERROR_SUCCESS);
	}

	return eError;
}

/******************************************************************************
 * Function : DIG_vLogActionThread(void const *argument)
 *//**
 * \b Description:
 *
 * This task is responsible of taking an action on the log record, which can be
 * to send the data under UART, CAN or USB, or to save in memory. It will wait for
 * a notification signal coming from the LoggerWriter routine and then will
 * manage the information using the LoggerReader function.
 *
 *
 * PRE-CONDITION: none
 * POST-CONDITION: Log Action thread ready to run
 *
 * @return none
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * Called only once from DIG_vDiagnosticThread !
 ~~~~~~~~~~~~~~~
 *
 * @see DIG_vDiagnosticLogActionCreation, DIG_vDiagnosticThread
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
#ifndef UNITY_TEST
void DIG_vLogActionThread (void const *argument)
{

	/* Init the logger */
	ASSERT(DIG_vLoggerInit(osThreadGetId()) == 1);

	/* check this priority */
	//osThreadSetPriority(NULL, osPriorityNormal);
	DIG_vDetectThread(&WATCHDOG(DIGLOG), &bDIGLOGThreadArrayPosition, (void*)DIG_vLogActionThread);
	WATCHDOG_STATE(DIGLOG, WDT_ACTIVE);

	osThreadId xDiagMainID = (osThreadId)argument;
	osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bDIGLOGThreadArrayPosition));				//Task created, inform core

	uint32_t wTransferSize = 0;
	uint32_t wOutputBytes = 0;
	//char bMsg[128];

	uint8_t bLogTempBuffer[DIG_BUFFER_MAX_LINE_SIZE];
	memset(bLogTempBuffer, 0x00, DIG_BUFFER_MAX_LINE_SIZE);

	while (1)
	{
		WATCHDOG_STATE(DIGLOG, WDT_SLEEP);
		osSignalWait(0xDE, osWaitForever);
		WATCHDOG_STATE(DIGLOG, WDT_ACTIVE);
		/* Read the logger buffer */
		DIG_vLoggerReader(&bLogTempBuffer[0], &wTransferSize);

		do
		{
			/* Check the type (5th byte) */
			switch (bLogTempBuffer[4] & 0x0F)
			{
				case DEBUG_USER:
				case DEBUG_ASSERT:
				case DEBUG_QUEUE:
				case DEBUG_WATCHDOG:
				case DEBUG_HARDFAULT:
				case DEBUG_SUBS:
				case DEBUG_UNSUBS:
				case DEBUG_PUBS:

				/* Output the content of the log register */
				wTransferSize = LOG_HEADER_LENGHT + bLogTempBuffer[5]; //!< 6th byte holds payload size
				wOutputBytes = DEV_write(pTDVHandle, &bLogTempBuffer[0], wTransferSize);
				ASSERT(wOutputBytes == wTransferSize);
					break;

				default:
					break;
			}

			DIG_vLoggerReader(&bLogTempBuffer[0], &wTransferSize);

		} while (wTransferSize != 0);
	}

}
#else
void DIG_vLogActionThread(void const *argument)
{}
#endif

#ifndef UNITY_TEST
void DIG_vOnBrokerStarted (void)
{
	/* Prepare the signature - struture that notify the broker about subscribers */
	SIGNATURE_HEADER(Diagnostic, THIS_MODULE, TOPIC_ALL, DiagnosticQueue);
	ASSERT(SUBSCRIBE(SIGNATURE(Diagnostic), 0) == osOK);
}
#endif

/******************************************************************************
 * Function : DIG_wLOG_PackBrokerTransaction(contract_s *spContract, uint8_t *bBuffer)
 *//**
 * \b Description:
 *
 *	This is a packer to serialize the publish transaction information, in order to
 *	log the operation. It will save in a array the following data:
 *		- contract origin;
 *		- contract topic;
 *		- contract version;
 *		- message type;
 *		- message key;
 *		- message size;
 *
 * PRE-CONDITION: a valid contract and a valid buffer to hold the serialized data
 *
 * POST-CONDITION: none
 *
 * @return Number of bytes of the serial stream.
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * 	uint8_t serialized_buffer[256];
 * 	osEvent evt = RECEIVE(DiagnosticQueue, osWaitForever);
 *	  if (evt.status == osEventMessage)
 *	  	DIG_wLOG_PackBrokerTransaction( GET_CONTRACT(evt), &serialized_buffer[0] );
 *
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
uint32_t DIG_wLOG_PackBrokerTransaction (contract_s *spContract, uint8_t *bBuffer)
{
	ASSERT((spContract != NULL) && (bBuffer != NULL));

	/* Serialize the contract information */
	uint32_t i = 0;
	memcpy(&bBuffer[i], &ORIGIN, sizeof(uint8_t));
	i += sizeof(uint8_t);

	memcpy(&bBuffer[i], &TOPIC, sizeof(uint8_t));
	i += sizeof(uint8_t);

	memcpy(&bBuffer[i], &VERSION, sizeof(uint8_t));
	i += sizeof(uint8_t);

	memcpy(&bBuffer[i], &MSG_TYPE, sizeof(uint8_t));
	i += sizeof(uint8_t);

	memcpy(&bBuffer[i], &MSG_KEY, sizeof(uint16_t));
	i += sizeof(uint16_t);

	memcpy(&bBuffer[i], &MSG_SIZE, sizeof(uint16_t));
	i += sizeof(uint16_t);

	return i;
}

/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void DIG_vDiagnosticThread (void const *argument)
{

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Print("Diagnostic Thread Created");
#endif

	/* Init the module queue - structure that receive data from broker */
	INITIALIZE_QUEUE(DiagnosticQueue);
#ifndef NDEBUG
	REGISTRY_QUEUE(DiagnosticQueue, DIG_vDiagnosticThread);
#endif

	/* Prepare the signature - struture that notify the broker about subscribers */
	/*	SIGNATURE_HEADER(Diagnostic, THIS_MODULE, TOPIC_ALL, DiagnosticQueue);
	 ASSERT(SUBSCRIBE(SIGNATURE(Diagnostic), 0) == osOK);*/

	/* Init TERMDEV device for output */
	DIG_vInitDeviceLayer(DIG_INITIAL_IO_IFACE);

	//Create subthreads
	uint8_t bNumberOfThreads = 0;
	while (THREADS_THREAD(bNumberOfThreads)!= NULL)
	{
		DIG_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
	}

	/* Inform Main thread that initialization was a success */
	osThreadId xMainFromDiagnosticID = (osThreadId)argument;
	osSignalSet(xMainFromDiagnosticID, MODULE_DIAGNOSTIC);

	uint8_t abPayload[100]; //!< Buffer to hold the contract and message data
	uint32_t bLOGLen = 0;		//!< Lenght (bytes) of the serialized data (contract+message)

	/* Start the main functions of the application */
	while (1)
	{
		/* Blocks until any message is published on any topic */
		WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
		osEvent evt = RECEIVE(DiagnosticQueue, osWaitForever);
		WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
		if (evt.status == osEventMessage)
		{

			/* Register the log */
			bLOGLen = DIG_wLOG_PackBrokerTransaction(GET_CONTRACT(evt), &abPayload[0]);
			DIG_wLoggerWritter(GET_TIMESTAMP(), (LEVEL_INFO | DEBUG_PUBS),
				(uint8_t*)abPayload, bLOGLen);
		}
	}

	/* Unreachable */
	osThreadSuspend(NULL);
}
#else
void DIG_vDiagnosticThread (void const *argument)
{}
#endif
/******************************************************************************
 * Unity Testing
 *******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(DiagnosticModuleTest);

TEST_SETUP(DiagnosticModuleTest)
{
	pTDVHandle = NULL;
}

TEST_TEAR_DOWN(DiagnosticModuleTest)
{
	DEV_close(pTDVHandle);
}

TEST(DiagnosticModuleTest, test_Verify_InitDeviceLayer)
{
	uint32_t wUART = TERMDEV_DEV_UART;
	uint32_t wUSB = TERMDEV_DEV_USB;
	uint32_t wCAN = TERMDEV_DEV_CAN;

	// Test init
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, DIG_vInitDeviceLayer(wUART));
	TEST_ASSERT_NOT_NULL(pTDVHandle);
	DEV_close(pTDVHandle);

	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, DIG_vInitDeviceLayer(wUSB));
	TEST_ASSERT_NOT_NULL(pTDVHandle);
	DEV_close(pTDVHandle);

	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, DIG_vInitDeviceLayer(wCAN));
	TEST_ASSERT_NOT_NULL(pTDVHandle);
	DEV_close(pTDVHandle);
}

TEST(DiagnosticModuleTest, test_Verify_eInitDiagPublisher)
{
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, DIG_vInitDeviceLayer(TERMDEV_DEV_UART));
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, DIG_eInitDiagPublisher());
}

TEST(DiagnosticModuleTest, test_Verify_wLOG_PackBrokerTransaction)
{
	uint8_t abMessage[200];
	uint8_t abBefore[8];
	uint8_t abAfter[8];

	memset(abMessage, 0x00, 200);
	CREATE_CONTRACT(DiagTest);
	CONTRACT_HEADER(DiagTest, 1, THIS_MODULE, TOPIC_DIAGNOSTIC);
	MESSAGE_HEADER(DiagTest, 1, 200, MT_BYTE);
	MESSAGE_PAYLOAD(DiagTest) = (void*) &abMessage[0];

	abBefore[0] = (uint8_t) sDiagTestContract.eOrigin;
	abBefore[1] = (uint8_t) sDiagTestContract.eTopic;
	abBefore[2] = (uint8_t) sDiagTestContract.bVersion;
	abBefore[3] = (uint8_t) sDiagTestContract.sMessage->eMessageType;
	abBefore[4] = (uint8_t) (sDiagTestContract.sMessage->hMessageKey & 0x00ff);
	abBefore[5] = (uint8_t) (sDiagTestContract.sMessage->hMessageKey >> 8);
	abBefore[6] = (uint8_t) (sDiagTestContract.sMessage->hMessageSize & 0x00ff);
	abBefore[7] = (uint8_t) (sDiagTestContract.sMessage->hMessageSize >> 8);

	DIG_wLOG_PackBrokerTransaction( &sDiagTestContract , &abAfter[0]);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(abAfter, abBefore, 8);

	//TEST_ASSERT_EQUAL(0, DIG_wLOG_PackBrokerTransaction(NULL, NULL));
}

TEST(DiagnosticModuleTest, test_Verify_DetectThread)
{
	uint8_t bThisPositionInSubThreadArray = 0;
	uint8_t bNumberOfArrayThreads = 3; //including core thread
	uint8_t bThreadPositionOnTheList = 1;//Position on wdt Array of flags
	uint8_t bNumber = 0;

	//Verify pointer
	DIG_vDetectThread(&WATCHDOG(DIGPUB), &bDIGPUBThreadArrayPosition, DIG_vDiagnosticPublishThread);
	TEST_ASSERT_EQUAL(bThisPositionInSubThreadArray, bDIGPUBThreadArrayPosition);
	TEST_ASSERT_EQUAL(1, THREADS_WDT_POSITION(bThisPositionInSubThreadArray));
	TEST_ASSERT_EQUAL_PTR(&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(bThisPositionInSubThreadArray)), WATCHDOG(DIGPUB));

	//verify state through pointer
	WATCHDOG_STATE(DIGPUB, WDT_ACTIVE);
	TEST_ASSERT_EQUAL(WDT_ACTIVE, *WATCHDOG(DIGPUB));
	TEST_ASSERT_EQUAL(WDT_ACTIVE, WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(bThisPositionInSubThreadArray)));

	//verify remote pointer
	uint8_t * pbWDTArray = DIG_WDTData(&bNumber);

	TEST_ASSERT_EQUAL(bNumberOfArrayThreads, bNumber);
	TEST_ASSERT_EQUAL_PTR(WATCHDOG_FLAG_ARRAY, pbWDTArray);
	TEST_ASSERT_EQUAL(WDT_ACTIVE, *(pbWDTArray += bThreadPositionOnTheList));//second thread on the list
	*pbWDTArray = WDT_UNKNOWN;
	TEST_ASSERT_EQUAL(WDT_UNKNOWN, WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(bThisPositionInSubThreadArray)));
}
#endif
