/****************************************************************************
 * Title                 :   Watchdog Include File
 * Filename              :   wds.h
 * Author                :   thiago.palmieri
 * Origin Date           :   12 de mai de 2016
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
 *  12 de mai de 2016   1.0.0   thiago.palmieri Watchdog include file Created.
 *
 *****************************************************************************/
/** @file wds.h
 *  @brief This file provides Watchdog functionality in a multithreaded environment.
 *
 *  The watchdog purpose is to:
 *  * Detect a properly operating system;
 *  * Detect an infinite loop in any of the tasks
 *  * Detect if any lower priority task is not getting run time because of higher
 *  priority tasks hogging the CPU
 *
 *  This watchdog implementation needs a thread dedicated to the watchdog. This thread
 *  waked up at a regular interval and checks the sanity of all other threads in the system.
 *  If all threads pass the test, then the watchdog is kicked. The watchdog monitor thread
 *  must run at a higher priority than any task it is monitoring.
 *
 *  Threads can be split into two groups.
 *
 *  A thread may run in reaction to a timer that occurs at regular interval.
 *  These threads have a start point through which they pass in each execution loop.
 *  These are referred as REGULAR THREADS.
 *
 *  A thread that respond to outside events, the frequency of which cannot be predicted
 *  is referred as WAITING THREAD.
 *
 *  \b REGULAR \b THREADS:
 *
 *  The watchdog timeout can be chosen to be the maximum time during which all regular
 *  threads have had a chance to run from their starting point through one full loop.
 *  Each thread has a flag which can have two values, ALIVE and UNKNOWN. This flag is
 *  read and written by the watchdog monitor.
 *
 *  The monitor's job is to wake up before the watchdog timer expires and check the
 *  status of each flag. If all flags contain the value ALIVE, the watchdog may be kicked.
 *  Some thread may have executed several loops and set their flag value to ALIVE
 *  several times, this is acceptable. After kicking the dog, the monitor sets all flags
 *  to UNKNOWN. By the time the monitor thread executes again, all of the UNKNOWN flags
 *  should have been overwitten with ALIVE. If any flag remained in UNKNOWN state, then
 *  the thread may be seen as locked and proper action must be taken by the monitor.
 *
 *  \b WAITING \b THREADS:
 *
 *  These threads normally have one or more points at which they are waiting on an
 *  external event. At those points, the flags are set to the value WAITING. After
 *  the wait, the flag is set to ALIVE and the process continues as described for
 *  REGULAR THREADS. If the monitor checks the flags and sees the value WAITING, it
 *  considers it as valid state. So, if all flags are either WAITING or ALIVE then
 *  the watchdog can be kicked. The monitor however does not changes the state of
 *  the WAITING threads, they remain as waiting where the ALIVE threads are set to UNKNOWN.
 *
 *  Therefore the rule is that the line of code following the line where the flag is set
 *  to WAITING must perform the wait, normally using one of the blocking function calls
 *  from the operating system. The instruction which follows the wait must set the flag to
 *  ALIVE. Example:
 *
 ~~~~~~~~~~~~~~~{.c}
 * uint8_t bFlag = WAITING;
 * osEvent evt = RECEIVE(ActuatorQueue, osWaitForever);
 * bFlag = ALIVE;
 * if (evt.status == osEventMessage)
 * //continue
 ~~~~~~~~~~~~~~~
 *
 *  Because there are no conditions or branches in this sequence, no set of circumstances
 *  allow the task to continue with the flag in the WAITING state.
 *
 *  Threads being monitored never read the flag, they only write to it.
 *
 *  This file provides functions to START the watchdog, to FEED it and thus restarting the
 *  timer, to ADD a given flag to the monitoring group and to TREAT the flag group hence verifying
 *  if any flag is in the correct state.
 *
 *  Each module implementing this watchdog scheme, must provide a function returning a pointer
 *  to an array of bytes, where each byte represents a sub-thread flag and the number of
 *  threads to be monitored.
 *
 *  \b ACTIVATION:
 *
 *  In order to activate the watchdog implementation use following symbol during compilation: \b __USE_WATCHDOG
 *
 */
#ifndef WDS_H_
#define WDS_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#include "devicelib.h"
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define EXPIRATION_TIMER 5000
/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

	/******************************************************************************
	 * Function : WDS_eStart(void)
	 *//**
	 * \b Description:
	 *
	 * Function used to start the watchdog service.
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: Watchdog service started
	 *
	 * @return     eAPPError
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * WDS_eStart();
	 ~~~~~~~~~~~~~~~
	 *
	 * @see WDS_eStart, WDS_eAddModule, WDS_eFeed, WDS_eTreat
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
	WEAK eAPPError_s WDS_eStart (void);

	/******************************************************************************
	 * Function : WDS_eAddModule(uint8_t* pbWDTModuleArray, uint8_t bArraySize, uint8_t bArrayPos)
	 *//**
	 * \b Description:
	 *
	 * Function used to add a flag array to be monitored. A pointer to the flag array, its size and
	 * a position on the internal array reference must be provided.
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: Module Watchdog flag array added to the monitoring array
	 *
	 * @return     eAPPError
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * WDS_eStart();
	 * WDS_eAddModule(pbWDTModuleArray, 3, MODULE_ACTUATOR);
	 ~~~~~~~~~~~~~~~
	 *
	 * @see WDS_eStart, WDS_eAddModule, WDS_eFeed, WDS_eTreat
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
	WEAK eAPPError_s WDS_eAddModule (uint8_t* pbWDTModuleArray, uint8_t bArraySize, uint8_t bArrayPos);

	/******************************************************************************
	 * Function : WDS_eFeed(void)
	 *//**
	 * \b Description:
	 *
	 * Function used to feed the dog, thus restarting the watchdog timer.
	 *
	 * PRE-CONDITION: Watchdog started
	 *
	 * POST-CONDITION: Watchdog timer restarted
	 *
	 * @return     eAPPError
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * WDS_eStart();
	 * WDS_eAddModule(pbWDTModuleArray, 3, MODULE_ACTUATOR);
	 * WDS_eFeed();
	 ~~~~~~~~~~~~~~~
	 *
	 * @see WDS_eStart, WDS_eAddModule, WDS_eFeed, WDS_eTreat
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
	WEAK eAPPError_s WDS_eFeed (void);

	/******************************************************************************
	 * Function : WDS_eTreat(void)
	 *//**
	 * \b Description:
	 *
	 * Function used to check registered flags status. If all flags are valid, then
	 * the dog is fed, otherwise an endless loop is entered.
	 *
	 * PRE-CONDITION: Watchdog started
	 *
	 * POST-CONDITION: Watchdog timer restarted
	 *
	 * @return     eAPPError
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * WDS_eStart();
	 * WDS_eAddModule(pbWDTModuleArray, 3, MODULE_ACTUATOR);
	 * WDS_eFeed();
	 * while(1)
	 * {
	 *   osDelay(1000);
	 *   WDS_eTreat();
	 * }
	 ~~~~~~~~~~~~~~~
	 *
	 * @see WDS_eStart, WDS_eAddModule, WDS_eFeed, WDS_eTreat
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
	WEAK eAPPError_s WDS_eTreat (void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WDS_H_ */

/*** End of File **************************************************************/
