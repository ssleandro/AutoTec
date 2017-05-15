/****************************************************************************
* Title                 :   interface_diagnostic Include File
* Filename              :   interface_diagnostic.h
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
*    Date    Version        Author         				  Description
*  08/04/16   1.0.0    Joao Paulo Martins    interface_diagnostic.h created.
*
*****************************************************************************/
/** @file  interface_diagnostic.h
 *  @brief This file provides the interface for the diagnostic software module.
 *
 *  This Header file must be included by Broker_includeall (broker) only.
 *  The main Thread is the DiagnosticThread, and it must be started
 *  by the Main thread.
 *
 *  When this module needs information from another module, it must subscribe
 *  to the given topic, passing this module queue during the subscription phase.
 *
 *  The diagnostic module is subscribed to the COMM_TOPIC and will capture all
 *  messages to this topic and output as a stream via selected interface
 *  (UART, CAN or USB).
 *
 */
#ifndef DIAGNOSTIC_INC_INTERFACE_DIAGNOSTIC_H_
#define DIAGNOSTIC_INC_INTERFACE_DIAGNOSTIC_H_

/******************************************************************************
* Includes
*******************************************************************************/
#ifndef UNITY_TEST
#include <auteq_os.h>
#include "common_app.h"
#else
#include "../common/app_common/common_app.h"
#include <stdint.h>
#endif
#include <inttypes.h>
#include <stdbool.h>

/** @defgroup DIAGNOSTIC_MODULE
 * @ingroup DIAGNOSTIC_Interface
 * @{
 */
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* Variables
*******************************************************************************/
#ifndef UNITY_TEST
EXTERN_QUEUE(DiagnosticQueue); //!< Makes the module's queue visible to broker
#endif

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
* Function : DIG_vDiagnosticThread (void const *argument);
*//**
* \b Description:
*
* This thread is responsible to receive the diagnostic messages from all other modules
* and output the content on a selected peripheral interface.
*
* PRE-CONDITION: The peripheral used as input/output must not be used by another module
*
* POST-CONDITION: Diagnostic thread running
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* osThreadDef_t sDiagnosticThread;
*
* sCommunicatorThread.name = "DiagnosticThread";
* sCommunicatorThread.stacksize = 500;
* sCommunicatorThread.tpriority = osPriorityLow;
* sCommunicatorThread.pthread = DIG_vDiagnosticThread;
*
* osThreadCreate(&sDiagnosticThread, NULL);
~~~~~~~~~~~~~~~
*
* @see DIG_vDiagnosticThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 29/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void DIG_vDiagnosticThread (void const *argument);

/******************************************************************************
* Function : XXX_WDTData(uint8_t * pbNumberOfThreads)
*//**
* \b Description:
*
* This is a publik function used by the watchdog interface to fetch the watchdog thread
* array and number os threads runing on the module.
*
* PRE-CONDITION: Watchdog interface activated
*
* POST-CONDITION: none
*
* @return void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* uint8_t bNumberOfThreads = 0;
* uint8_t * pbArray = XXX_WDTData(&bNumberOfThreads);
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 16/05/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint8_t * DIG_WDTData(uint8_t * pbNumberOfThreads);

/******************************************************************************
* Function : DIG_vOnBrokerStarted(void)
*//**
* \b Description:
*
* This is a public function used by the broker to inform Diagnostic module that it has
* completed initialization. This function will then perform the diagnostic subscription
* to receive publishes.
*
* PRE-CONDITION: Broker completed initialization
*
* POST-CONDITION: none
*
* @return void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* DIG_vOnBrokerStarted();
~~~~~~~~~~~~~~~
*
* @see DIG_vOnBrokerStarted
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 16/05/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
WEAK extern void DIG_vOnBrokerStarted(void);

#ifdef __cplusplus
} // extern "C"
#endif

/**
 * @}
 */
#endif /* DIAGNOSTIC_INC_INTERFACE_DIAGNOSTIC_H_ */

/*** End of File **************************************************************/
