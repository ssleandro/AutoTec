/****************************************************************************
* Title                 :   test_broker Include File
* Filename              :   test_broker.h
* Author                :   thiago.palmieri
* Origin Date           :   8 de mar de 2016
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
*  8 de mar de 2016   1.0.0   thiago.palmieri test_broker include file Created.
*
*****************************************************************************/
/** @file test_broker.h
 *  @brief This file provides some broker testing functions.
 *
 */
#ifndef BROKER_TEST_TEST_BROKER_H_
#define BROKER_TEST_TEST_BROKER_H_

/******************************************************************************
* Includes
*******************************************************************************/
//#include "../inc/interface_broker.h"
#include <M2G_app.h>
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
extern osMessageQId  TestPublish; //!< identifies the message queue (pointer to a message queue control block).
/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

extern void vTestSubscriptionThread (void const *argument);

extern void vTestUnsubscriptionThread (void const *argument);

extern void vTestPublishThread (void const *argument);

extern void vRunBrokerTestsThread (void const *argument);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BROKER_TEST_TEST_BROKER_H_ */

/*** End of File **************************************************************/
