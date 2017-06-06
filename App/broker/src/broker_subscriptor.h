/****************************************************************************
 * Title                 :   broker_subscriptor Include File
 * Filename              :   broker_subscriptor.h
 * Author                :   thiago.palmieri
 * Origin Date           :   03/03/2016
 * Version               :   1.0.0
 * Compiler              :   GCC 5.2 2015q4 / ICCARM 7.50.2.10312
 * Target                :   LPC4357
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
 *  03/03/2016   1.0.0   thiago.palmieri broker_subscriptor include file Created.
 *
 *****************************************************************************/
/** @file broker_subscriptor.h
 *  @brief This file provides Subscription and unsubscription threads.
 *
 */
#ifndef BROKER_BROKER_SUBSCRIPTOR_H_
#define BROKER_BROKER_SUBSCRIPTOR_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>

#include "broker_threadControl.h"
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

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
 * Function : BRK_vUnSubscriptorThread (void const *argument)
 *//**
 * \b Description:
 *
 * After the Thread is created, it wait for an unsubscription message.
 * When it arrives it will try to remove the subscriber from the respective topic.
 *
 * PRE-CONDITION: Broker Initialized
 *
 * POST-CONDITION: UnSubscription handled
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * osThreadDef_t sBrokerUnSubThread;
 *
 * sBrokerUnSubThread.name = "BrokerUnSub";
 * sBrokerUnSubThread.stacksize = 500;
 * sBrokerUnSubThread.tpriority = osPriorityNormal;
 * sBrokerUnSubThread.pthread = vUnSubscriptorThread;
 *
 * osThreadCreate(&sBrokerUnSubThread, NULL);
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_vBrokerThread, BRK_vUnSubscriptorThread, BRK_vSubscriptorThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 11/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void BRK_vUnSubscriptorThread (void const *argument);

/******************************************************************************
 * Function : BRK_vSubscriptorThread (void const *argument);
 *//**
 * \b Description:
 *
 * After the Thread is created, it wait for a subscription message.
 * When it arrives it will try to insert the subscriber into the respective topic
 * for modules that need to receive the publication and forward it to them.
 *
 * PRE-CONDITION: Broker Initialized
 *
 * POST-CONDITION: Subscription handled
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * osThreadDef_t sBrokerSubThread;
 *
 * sBrokerSubThread.name = "BrokerSub";
 * sBrokerSubThread.stacksize = 500;
 * sBrokerSubThread.tpriority = osPriorityNormal;
 * sBrokerSubThread.pthread = vSubscriptorThread;
 *
 * osThreadCreate(&sBrokerSubThread, NULL);
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_vBrokerThread, BRK_vUnSubscriptorThread, BRK_vSubscriptorThread
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 11/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void BRK_vSubscriptorThread (void const *argument);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BROKER_BROKER_SUBSCRIPTOR_H_ */

/*** End of File **************************************************************/
