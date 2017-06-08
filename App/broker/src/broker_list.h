/****************************************************************************
 * Title                 :   broker_list Include File
 * Filename              :   broker_list.h
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
 *  03/03/2016   1.0.0   thiago.palmieri broker_list include file Created.
 *
 *****************************************************************************/
/** @file broker_list.h
 *  @brief This file provides common list handling for broker module
 *
 */
#ifndef BROKER_BROKER_LIST_H_
#define BROKER_BROKER_LIST_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
//#include "../inc/interface_broker.h"
#include <M2G_app.h>
//#include "common_app.h"

/** @defgroup BROKER_MODULE
 * @ingroup BROKER_List
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

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
 * Function : BRK_eInitializeLists(void)
 *//**
 * \b Description:
 *
 * This function must be used to initialize the subscriber list to a known state.
 * This function must be called from the broker_core only at starting time, otherwise
 * it may cause memory leaks.
 *
 * PRE-CONDITION: Broker initialized
 *
 * POST-CONDITION: Subscription list initialized
 *
 * @return     eAPPError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * BRK_eInitializeLists();
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_eInitializeLists, BRK_eBrokerListUpdate, BRK_eBrokerListSearch, BRK_eFindNextInList
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
eAPPError_s BRK_eInitializeLists (void);

/******************************************************************************
 * Function : BRK_eBrokerListUpdate(bool add, signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This function must be used to perform or remove a subscription into a given/all topics.
 * A Boolean value must be passed to indicate the operation to perform, TRUE for subscription
 * and FALSE for Unsubscription. A valid signature struct must be passed. A valid signature
 * means that the function will only check the TOPIC subscription, the Destine and destine
 * queue correctness information must be guaranteed by the sender.
 *
 * PRE-CONDITION: Broker Subscriptor initialized
 *
 * POST-CONDITION: Subscription list updated
 *
 * @return     eAPPError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (!BRK_eBrokerListUpdate(true, &sSubscriber)) PRINTF("Subscription Performed");
 * if (!BRK_eBrokerListUpdate(false, &sSubscriber)) PRINTF("Un-Subscription Performed");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_eInitializeLists, BRK_eBrokerListUpdate, BRK_eBrokerListSearch, BRK_eFindNextInList
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
eAPPError_s BRK_eBrokerListUpdate (bool add, signature_s * sSubscriber);

/******************************************************************************
 * Function : BRK_eBrokerListSearch(topic_e eTopic, signature_s * sCurrentNode);
 *//**
 * \b Description:
 *
 * This function must be used to verify if a given subscriber is already included on the list.
 * A valid Topic and subscriber must be provided. If invalid values are entered, the the response will
 * be NULL. If no subscriber is found, the result will also be NULL.
 *
 * PRE-CONDITION: Broker Subscriptor initialized
 *
 * POST-CONDITION: None
 *
 * @return     Subscription or NULL
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (BRK_eBrokerListSearch(eTopic, &sCurrentNode) != NULL)
 *   PRINTF("Subscriber Found");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_eInitializeLists, BRK_eBrokerListUpdate, BRK_eBrokerListSearch, BRK_eFindNextInList
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
signature_s * BRK_eBrokerListSearch (topic_e eTopic, signature_s * sCurrentNode);

/******************************************************************************
 * Function : BRK_eFindNextInList(topic_e eTopic, signature_s * sCurrentNode)
 *//**
 * \b Description:
 *
 * This function must be used to fetch the next subscriber from the linked list on a given topic.
 * A subscriber must be provided, and a topic; If NULL subscriber is provided, the function
 * will start the search from the HEAD. The function will search the list for the subscriber
 * and return the next subscriber on the list, or NULL if it was the last.
 *
 * PRE-CONDITION: List Initialized
 *
 * POST-CONDITION: None
 *
 * @return     Subscriber or NULL
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (BRK_eFindNextInList(eTopic, NULL) != NULL)
 *   PRINTF("Next Subscriber Found");
 * else
 *   PRINTF("That was the last subscriber");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_eInitializeLists, BRK_eBrokerListUpdate, BRK_eBrokerListSearch, BRK_eFindNextInList
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
signature_s * BRK_eFindNextInList (topic_e eTopic, signature_s * sCurrentNode);

#ifdef __cplusplus
} // extern "C"
#endif

/**
 * @}
 */
#endif /* BROKER_BROKER_LIST_H_ */

/*** End of File **************************************************************/
