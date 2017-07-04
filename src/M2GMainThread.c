/****************************************************************************
 * Title                 :   TDATaskMain Include File
 * Filename              :   TDATaskMain.c
 * Author                :   thiago.palmieri
 * Origin Date           :   8 de abr de 2016
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
 *  8 de abr de 2016   1.0.0   thiago.palmieri TDATaskMain include file Created.
 *
 *****************************************************************************/
/** @file TDATaskMain.c
 *  @brief This file provides Main thread functionality. This file is responsible to
 *  start each module main thread and to start watchdog routines.
 *
 */
#ifndef UNITY_TEST
/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#include "broker_includeall.h"
#include "devicelib.h"
#include "debug_tool.h"
#include "wds.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
osFlagsGroupId UOS_sFlagSis;

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
// The MACRO below holds all necessary information for thread creation, it must be expanded into
// a structure to be used on thread creation function
//  name            , stacksize, priority       , threadfunc              , waitfor         , WDTFunction
#define STARTING_MODULES \
        X("Diagnostic" , 512  , osPriorityLow			, DIG_vDiagnosticThread , MODULE_DIAGNOSTIC , DIG_WDTData  ) \
        X("Broker"     , 256  , osPriorityHigh			, BRK_vBrokerThread     , MODULE_BROKER     , BRK_WDTData  ) \
        X("Isobus"     , 1024  , osPriorityHigh       	, ISO_vIsobusThread     , MODULE_ISOBUS     , ISO_WDTData  ) \
		  X("Gui"        , 512  , osPriorityNormal    	, GUI_vGuiThread        , MODULE_GUI        , GUI_WDTData  ) \
		  X("Acquireg"   , 512  , osPriorityHigh    		, AQR_vAcquiregThread   , MODULE_ACQUIREG   , AQR_WDTData  ) \
		  X("Sensor"     , 512  , osPriorityHigh    		, SEN_vSensorThread     , MODULE_SENSOR     , SEN_WDTData  ) \
		  X("GPS"        , 512  , osPriorityAboveNormal , GPS_vGPSThread        , MODULE_GPS        , GPS_WDTData  ) \
		  X("Control"    , 512  , osPriorityNormal		, CTL_vControlThread    , MODULE_CONTROL    , CTL_WDTData  ) \
		  X("Filesys"    , 1024 , osPriorityNormal		, FSM_vFileSysThread    , MODULE_FILESYS    , FSM_WDTData  ) \
		  X(NULL         ,   0  , 0                     , NULL                  , 0                 , NULL         )

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
typedef uint8_t * (*WDTFunc) (uint8_t *);  //!< Watchdog function typedef

/**
 * This Typedef is used to define Threads data for initialization
 */
typedef struct startingThreads_t
{
	osThreadDef_t thisThread;       //!< Thread Definition
	uint32_t thisModule;           //!< Synchronization
	WDTFunc thisFunc;                //!< Watchdog function used to retrieve module wdt arrays
} startingThreads_t;
/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisFunc = f},
startingThreads_t sThreads[] = {
STARTING_MODULES
	};  //!< Array used to hold Initial threads definition for each module
#undef X

volatile unsigned long ulHighFrequencyTimerTicks;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

/******************************************************************************
 * Function : MAI_vCreateThread(const startingThreads_t sThread )
 *//**
 * \b Description:
 *
 * This is a private function used to create new threads. It must receive as input
 * a structure holding all necessary information for thread initialization.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: Thread created
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * startingThreads_t sThreads;
 *
 * sThreads.thisThread.name = "TestThread";          //Thread name
 * sThreads.thisThread.stacksize = 200;              //Stack size
 * sThreads.thisThread.tpriority = osPriorityNormal; //Priority
 * sThreads.thisThread.pthread = TST_Thread;         //Thread function pointer
 * sThreads.thisModule = MODULE_TEST;                //Wait signal value for synchronization
 * sThreads.thisFunc = TST_WDTData;                  //Watchdog function to return WDTarray
 *
 * MAI_vCreateThread(sThreads);
 ~~~~~~~~~~~~~~~
 *
 * @see MAI_vCreateThread, MAI_TDAMainThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 12/05/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static void MAI_vCreateThread (const startingThreads_t sThread)
{
	osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());
	ASSERT(xThreads != NULL);
	if (sThread.thisModule != 0)
	{
		osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
	}
}

void MAI_M2GMainThread (void const *argument)
{
	osStatus status;
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Start(); // Start Trace
	SEGGER_SYSVIEW_Print("MAIN Thread Created");
#endif

	// Create an flag to indicate the system status...
	status = osFlagGroupCreate(&UOS_sFlagSis);
	ASSERT(status == osOK);

	WDS_eStart(); //Start Watchdog

	uint8_t bIndex = 0;
	while (sThreads[bIndex].thisThread.pthread != NULL)
	{
		uint8_t bSize = 0;
		if (sThreads[bIndex].thisFunc != NULL)
		{
			uint8_t* pFunc = sThreads[bIndex].thisFunc(&bSize);
			WDS_eAddModule(pFunc, bSize, sThreads[bIndex].thisModule);
		}
		MAI_vCreateThread(sThreads[bIndex++]);
		WDS_eFeed();  //Feed dog
	}

	osFlagSet(UOS_sFlagSis, UOS_SIS_FLAG_SIS_UP);

//#define FUNCTION_TEST
#if defined (FUNCTION_TEST)
	osThreadDef_t sBrokerTestThread;

	sBrokerTestThread.name = "BrokerTest";
	sBrokerTestThread.stacksize = 500;
	sBrokerTestThread.tpriority = osPriorityBelowNormal;
	sBrokerTestThread.pthread = vRunBrokerTestsThread;

	osThreadCreate(&sBrokerTestThread, (void*)osThreadGetId());
	osSignalWait((1 << 0), osWaitForever); //wait for Test initialization
#endif

	while (1)
	{
		osDelay(EXPIRATION_TIMER - 500);
		WDS_eTreat(); //Handle Dog
		//osThreadSuspend(NULL);  //Too much work for me, Im going to sleep now...
	}
	osThreadTerminate(NULL);  //Ohhhh noes.....
}

#endif
/******************************************************************************
 * Unity Testing
 *******************************************************************************/
