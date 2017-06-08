/****************************************************************************
 * Title                 :   interface_test Include File
 * Filename              :   interface_test.h
 * Author                :   Henrique Reis
 * Origin Date           :   14 de out de 2016
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
 *    Date    Version        Author                        Description
 *  14/10/16   1.0.0    Henrique Reis    interface_test.h created.
 *
 *****************************************************************************/
/** @file  interface_test.h
 *  @brief This file provides the interface for the sensor software module.
 *
 *  This Header file must be included by Broker_includeall (broker) only.
 *  The main Thread is the SensorThread, and it must be started
 *  by the Main thread.
 *
 *  When this module needs information from another module, it must subscribe
 *  to the given topic, passing this module queue during the subscription phase.
 *
 */
#ifndef APP_TEST_INC_INTERFACE_TEST_H_
#define APP_TEST_INC_INTERFACE_TEST_H_

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
EXTERN_QUEUE(TestQueue);      //!< Makes the module's queue visible to broker
#endif

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

void TEST_vTestThread (void const *argument);

uint8_t * TEST_WDTData (uint8_t * pbNumberOfThreads);

WEAK extern void TEST_vOnBrokerStarted (void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* APP_TEST_INC_INTERFACE_TEST_H_ */
