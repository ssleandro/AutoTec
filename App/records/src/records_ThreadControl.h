/****************************************************************************
 * Title                 :   records_ThreadControl
 * Filename              :   records_ThreadControl.h
 * Author                :   Henrique Reis
 * Origin Date           :   18 de abr de 2017
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
 *  18/04/17   1.0.0     Henrique Reis         records_ThreadControl.h created.
 *
 *****************************************************************************/
#ifndef APP_RECORDS_SRC_RECORDS_THREADCONTROL_H_
#define APP_RECORDS_SRC_RECORDS_THREADCONTROL_H_

#ifndef UNITY_TEST
#include "interface_records.h"
#endif

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
//  name            , stacksize, priority       , threadfunc              , waitfor  , WDTPosition
#define RECORDS_MODULES \
    X("RecordsPublish"     , 150 , osPriorityNormal , REC_vRecordsPublishThread     , (1 << 0) , 1 ) \
    X(NULL                 ,   0 , 0                , NULL                          , 0        , 2 ) \

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define WATCHDOG_FLAG_ARRAY baRecordsWatchdogFlags

#define THREADS_THISTHREAD  sRecordsThreads
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
/**
 * This Typedef is used to define Threads data for initialization
 */
typedef struct Threads_t
{
	osThreadDef_t thisThread;       //!< Thread Definition
	uint32_t thisModule;       //!< Synchronization
	uint8_t thisWDTPosition;  //!< WDT position in flag array
} Threads_t;

typedef uint8_t* thisWDTFlag;      //!< Watchdog Control flag pointer typedef

/******************************************************************************
 * Variables
 *******************************************************************************/
extern Threads_t THREADS_THISTHREAD[];    //!< Actuator Thread control array of structure

extern volatile uint8_t WATCHDOG_FLAG_ARRAY[];   //!< Threads Watchdog flag holder

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

extern void REC_vRecordsPublishThread (void const *argument);

extern void REC_vDetectThread (thisWDTFlag* flag, uint8_t* bCounter, void* pFunc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* APP_RECORDS_SRC_RECORDS_THREADCONTROL_H_ */
