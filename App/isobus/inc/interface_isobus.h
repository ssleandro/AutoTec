/****************************************************************************
 * Title                 :   interface_isobus.h Include File
 * Filename              :   interface_isobus.h
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
 *    Date    Version        Author         				  Description
 *  01/07/16   1.0.0    Henrique Reis    interface_isobus.h created.
 *
 *****************************************************************************/
/** @file  interface_isobus.h
 *  @brief This file provides the interface for the isobus software module.
 *
 *  This Header file must be included by Broker_includeall (broker) only.
 *  The main Thread is the IsobusThread, and it must be started
 *  by the Main thread.
 *
 *  When this module needs information from another module, it must subscribe
 *  to the given topic, passing this module queue during the subscription phase.
 *
 */
#ifndef ISOBUS_INC_INTERFACE_ISOBUS_H_
#define ISOBUS_INC_INTERFACE_ISOBUS_H_

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

/** @defgroup ISOBUS_MODULE
 * @ingroup ISO_Interface
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
EXTERN_QUEUE(IsobusQueue);      //!< Makes the module's queue visible to broker
#endif

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Function : ISO_vIsobusThread (void const *argument);
 *//**
 * \b Description:
 *
 * This thread is responsible to receive the messages from all other modules
 * and send the content to an responsably thread.
 *
 * PRE-CONDITION: The peripheral used as input/output must not be used by another module
 *
 * POST-CONDITION: Isobus thread running
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * osThreadDef_t sIsobusThread;
 *
 * sIsobusThread.name = "IsobusThread";
 * sIsobusThread.stacksize = 500;
 * sIsobusThread.tpriority = osPriorityLow;
 * sIsobusThread.pthread = ISO_vIsobusThread;
 *
 * osThreadCreate(&sIsobusThread, NULL);
 ~~~~~~~~~~~~~~~
 *
 * @see ISO_vIsobusThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 *******************************************************************************/
void ISO_vIsobusThread (void const *argument);

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
uint8_t * ISO_WDTData (uint8_t * pbNumberOfThreads);

/******************************************************************************
 * Function : DIG_vOnBrokerStarted(void)
 *//**
 * \b Description:
 *
 * This is a public function used by the broker to inform Isobus module that it has
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
WEAK extern void ISO_vOnBrokerStarted (void);

#ifdef __cplusplus
} // extern "C"
#endif

/**
 * @}
 */
#endif /* ISOBUS_INC_INTERFACE_ISOBUS_H_ */

/*** End of File **************************************************************/
