/****************************************************************************
* Title                 :   broker_threadControl Include File
* Filename              :   broker_threadControl.h
* Author                :   thiago.palmieri
* Origin Date           :   13 de mai de 2016
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
*  13 de mai de 2016   1.0.0   thiago.palmieri broker_threadControl include file Created.
*
*****************************************************************************/
/** @file broker_threadControl.h
 *  @brief This file provides Thread and watchdog control structures to be used by the module
 *
 */
#ifndef APP_BROKER_SRC_BROKER_THREADCONTROL_H_
#define APP_BROKER_SRC_BROKER_THREADCONTROL_H_

/******************************************************************************
* Includes
*******************************************************************************/
#if defined (UNITY_TEST)
#include "../broker/inc/interface_broker.h"
#else
#include "interface_broker.h"
#endif
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
//  name            , stacksize, priority       , threadfunc              , waitfor  , WDTPosition
#define BROKER_MODULES \
  X("BrokerSubs "   , 500 , osPriorityAboveNormal , BRK_vSubscriptorThread    , (1 << 0) , 1 ) \
  X("BrokerUnsubs " , 500 , osPriorityAboveNormal , BRK_vUnSubscriptorThread  , (1 << 1) , 2 ) \
  X(NULL            ,   0 , 0                     , NULL                      , 0        , 3 ) \


/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define WATCHDOG_FLAG_ARRAY babrokerWatchdogFlags

#define THREADS_THISTHREAD  sbrokerThreads
/******************************************************************************
* Macros
*******************************************************************************/
#define WATCHDOG_CREATE(thread) \
  static thisWDTFlag p##thread##WDTFlag = NULL

#define WATCHDOG_STATE(thread, state) (*p##thread##WDTFlag = state)

#define WATCHDOG(thread) p##thread##WDTFlag

#define WATCHDOG_FLAGPOS(position) WATCHDOG_FLAG_ARRAY[position]

#define THREADS_THREAD(position) THREADS_THISTHREAD[position].thisThread.pthread

#define THREADS_WDT_POSITION(position) THREADS_THISTHREAD[position].thisWDTPosition

#define THREADS_RETURN_SIGNAL(position) THREADS_THISTHREAD[position].thisModule
/******************************************************************************
* Typedefs
*******************************************************************************/
#if defined(UNITY_TEST)
typedef void (*os_pthread) (void const *argument);

typedef enum  {
  osPriorityIdle          = -3,          ///< priority: idle (lowest)
  osPriorityLow           = -2,          ///< priority: low
  osPriorityBelowNormal   = -1,          ///< priority: below normal
  osPriorityNormal        =  0,          ///< priority: normal (default)
  osPriorityAboveNormal   = +1,          ///< priority: above normal
  osPriorityHigh          = +2,          ///< priority: high
  osPriorityRealtime      = +3,          ///< priority: realtime (highest)
  osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} osPriority;

typedef struct os_thread_def  {
  char                   *name;        ///< Thread name
  os_pthread             pthread;      ///< start address of thread function
  osPriority             tpriority;    ///< initial thread priority
  uint32_t               instances;    ///< maximum number of instances of that thread function
  uint32_t               stacksize;    ///< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;
#endif

/**
 * This Typedef is used to define Threads data for initialization
 */
typedef struct Threads_t
{
  osThreadDef_t   thisThread;       //!< Thread Definition
  uint32_t        thisModule;       //!< Synchronization
  uint8_t         thisWDTPosition;  //!< WDT position in flag array
}Threads_t;

typedef uint8_t*        thisWDTFlag;      //!< Watchdog Control flag pointer typedef

/******************************************************************************
* Variables
*******************************************************************************/
extern Threads_t THREADS_THISTHREAD[];    //!< Actuator Thread control array of structure

extern volatile uint8_t WATCHDOG_FLAG_ARRAY[];   //!< Threads Watchdog flag holder

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

extern void BRK_vSubscriptorThread (void const *argument);
extern void BRK_vUnSubscriptorThread (void const *argument);

extern void BRK_vDetectThread(thisWDTFlag* flag, uint8_t* bCounter, void* pFunc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* APP_BROKER_SRC_BROKER_THREADCONTROL_H_ */

/*** End of File **************************************************************/
