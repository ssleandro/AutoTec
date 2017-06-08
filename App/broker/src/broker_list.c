/****************************************************************************
 * Title                 :   broker_list source File
 * Filename              :   broker_list.c
 * Author                :   thiago.palmieri
 * Origin Date           :   03/03/2016
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
 *  03/03/2016   1.0.0   thiago.palmieri broker_list source file Created.
 *
 *****************************************************************************/
/** @file broker_list.c
 *  @brief This file provides common list handling for broker module
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "broker_list.h"
#include <stdlib.h>
#if defined(UNITY_TEST)
#include "unity_fixture.h"
#include <stdint.h>
#endif
/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
#if defined(UNITY_TEST)
void osEnterCritical(void)
{

}
void osExitCritical(void)
{

}

#ifndef false
#define false 0
#endif
#ifndef true
#define true ~false
#endif
#endif
/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
#define HEAD(x) sSubscriptionList[x]
/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
/**
 * This typedef
 */
typedef struct node_s
{
	signature_s sSubscriber;
	struct node_s * sNext;
} node_s;

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static node_s * sSubscriptionList[TOPIC_LAST] = { NULL };   //!< Subscription list
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
/******************************************************************************
 * Function : BRK_wListLength(node_s * head)
 *//**
 * \b Description:
 *
 * This is a Linked List service to return the number of nodes into a given list.
 * The HEAD must be used as input parameter, always, otherwise the function will
 * return the wrong count.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     Size of the list
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * PRINTF("The current list has %d nodes", wListLength(HEAD));
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wListLength, BRK_sPushIntoList, BRK_sSearchInList, BRK_ePopFromList
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
static uint32_t BRK_wListLength (node_s * head)
{
	node_s * sCurrent = head;
	uint32_t wResult = 0;

	while (sCurrent != NULL)
	{
		wResult++;
		sCurrent = sCurrent->sNext;
	}
	return wResult;
}

/******************************************************************************
 * Function : BRK_sPushIntoList(node_s ** head, signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This is a Linked List service to insert a given subscriber into a list. If a NULL
 * subscriber is inserted, then the result will be NULL. If a NULL head is entered
 * the a new list will be created.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     The new node or NULL
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * node_s * newNode = sPushIntoList(NULL, &subscriber);
 * if (newNode != NULL)
 *   PRINTF("New list created");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wListLength, BRK_sPushIntoList, BRK_sSearchInList, BRK_ePopFromList
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
static node_s * BRK_sPushIntoList (node_s ** head, signature_s * sSubscriber)
{
	if (NULL == sSubscriber)
	{
		return NULL;
	}

	node_s * sNewNode = pvPortMalloc(sizeof(node_s));
	if (NULL == sNewNode)
	{
		return NULL;
	}

	if (NULL == *head) //The head is NULL
	{
		// Create Head
		sNewNode->sNext = NULL;
		sNewNode->sSubscriber = *sSubscriber;
		*head = sNewNode;
	}
	else
	{
		sNewNode->sSubscriber = *sSubscriber;
		sNewNode->sNext = *head;
		*head = sNewNode;
	}
	return *head;
}

/******************************************************************************
 * Function : BRK_sSearchInList(node_s ** head, signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This is a Linked List service to search for a given subscriber on a list. If a
 * NULL head is provided, the response will be NULL. If a subscriber is not found or
 * a NULL subscriber is provided, the answer will also be NULL.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     The node or NULL
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * node_s * subscriberNode = sSearchInList(HEAD, &subscriber);
 * if (subscriberNode != NULL)
 *   PRINTF("Subscriber Found");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wListLength, BRK_sPushIntoList, BRK_sSearchInList, BRK_ePopFromList
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
static node_s * BRK_sSearchInList (node_s ** head, signature_s * sSubscriber)
{
	node_s * spPtr = *head;
	bool bFound = false;

	while (spPtr != NULL)
	{
		if ((spPtr->sSubscriber.eDestine == sSubscriber->eDestine) &&
			(spPtr->sSubscriber.vpDestineQueue == sSubscriber->vpDestineQueue) &&
			(spPtr->sSubscriber.vpDestineQueue == sSubscriber->vpDestineQueue))
		{
			bFound = true;
			break;
		}
		else
		{
			spPtr = spPtr->sNext;
		}
	}
	if (bFound)
	{
		return spPtr;
	}
	return NULL;

}

/******************************************************************************
 * Function : BRK_ePopFromList(node_s ** head, signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This is a Linked List service used to remove a given subscriber from a list. If a
 * NULL head is provided, the function will return an error. If a subscriber is not found or
 * a NULL subscriber is provided, the answer will also be an error.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     eAPPError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (!ePopFromList(HEAD, &subscriber))
 *   PRINTF("Subscriber Removed");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wListLength, BRK_sPushIntoList, BRK_sSearchInList, BRK_ePopFromList
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
static eAPPError_s BRK_ePopFromList (node_s ** head, signature_s * sSubscriber)
{
	node_s *spDel = NULL;
	node_s *spCurr = *head;

	spDel = BRK_sSearchInList(head, sSubscriber);  //Search in list
	if (spDel == NULL) //Not found
	{
		return APP_ERROR_BROKER_LIST_NOT_FOUND;
	}
	else  //Found
	{

		if (spDel == *head)  //if head, then just move head to next
		{
			*head = spDel->sNext;
		}
		else //Find previous to relocate next pointer
		{
			while (spCurr->sNext != spDel)
			{
				spCurr = spCurr->sNext;
			}

			spCurr->sNext = spDel->sNext;
		}

	}

	free(spDel);
	spDel = NULL;

	return APP_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : BRK_wNumberOfTopicSubscribers(topic_e eTopic)
 *//**
 * \b Description:
 *
 * This function must be used to return the number of nodes into a given list.
 * The HEAD must be used as input parameter, always, otherwise the function will
 * return the wrong count.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     Size of the list
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * PRINTF("The current list has %d nodes", wNumberOfTopicSubscribers(HEAD));
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wNumberOfTopicSubscribers, BRK_eVerifyDuplicate, BRK_eSubscription, BRK_eUnsubscription
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
static uint32_t BRK_wNumberOfTopicSubscribers (topic_e eTopic)
{
	return BRK_wListLength(sSubscriptionList[eTopic]);
}

/******************************************************************************
 * Function : BRK_eVerifyDuplicate(signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This function must be used to search for a duplicate subscriber on a list.
 * If a subscriber is not found or a NULL subscriber is provided, the answer will
 * be an error.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     eAPPError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (!eVerifyDuplicate(&subscriber))
 *   PRINTF("Duplicated Subscriber not found");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wNumberOfTopicSubscribers, BRK_eVerifyDuplicate, BRK_eSubscription, BRK_eUnsubscription
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
static eAPPError_s BRK_eVerifyDuplicate (signature_s * sSubscriber)
{
	if (NULL == sSubscriber)
	{
		return APP_ERROR_BROKER_LIST_NOT_FOUND;
	}
	if (BRK_sSearchInList(&HEAD(sSubscriber->eTopic), sSubscriber) != NULL)
	{
		return APP_ERROR_BROKER_ALREADY_IN_LIST;
	}
	return APP_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : BRK_eSubscription(signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This function must be used to add a subscriber to a list. If a NULL subscriber is provided
 * or an invalid topic is used, the return will be an error code.
 *
 * PRE-CONDITION: Subscriber list initialized
 *
 * POST-CONDITION: Subscriber added to list, or not
 *
 * @return     eAPPError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (!eSubscription(&subscriber))
 *   PRINTF("Subscribed signed");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wNumberOfTopicSubscribers, BRK_eVerifyDuplicate, BRK_eSubscription, BRK_eUnsubscription
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
static eAPPError_s BRK_eSubscription (signature_s * sSubscriber)
{
	if (NULL == sSubscriber)
	{
		return APP_ERROR_BROKER_LIST_NOT_FOUND;
	}
	if (sSubscriber->eTopic == TOPIC_ALL) // subscribed into all topics
	{
		uint16_t wIndex;
		for (wIndex = 0; wIndex < TOPIC_LAST; wIndex++)
		{
			sSubscriber->eTopic = (topic_e)wIndex; //Chande subscriber topic to this iteration
			eAPPError_s eAlreadyIntoList = BRK_eVerifyDuplicate(sSubscriber);
			if (!eAlreadyIntoList)
			{
				//The linked list as defined below, will fix the HEAD into array
				//New Head                    //Previous head
				node_s * sVerify = BRK_sPushIntoList(&HEAD(wIndex), sSubscriber);
				if (sVerify == NULL)
				{
					return APP_ERROR_BROKER_LIST_NOT_FOUND;
				}
			}
		}
		sSubscriber->eTopic = TOPIC_ALL;
		return APP_ERROR_SUCCESS;
	}
	else if (sSubscriber->eTopic < TOPIC_ALL) // Single topic subscription
	{
		eAPPError_s eAlreadyIntoList = BRK_eVerifyDuplicate(sSubscriber);
		if (!eAlreadyIntoList)
		{
			//New Head                                 //Previous head
			node_s * sVerify = BRK_sPushIntoList(&HEAD(sSubscriber->eTopic), sSubscriber);
			if (sVerify != NULL)
			{
				return APP_ERROR_SUCCESS;
			}
		}
		return APP_ERROR_SUCCESS;
	}
	else  // Invalid topic
	{
		return APP_ERROR_BROKER_INVALID_TOPIC;
	}
}

/******************************************************************************
 * Function : BRK_eUnsubscription(signature_s * sSubscriber)
 *//**
 * \b Description:
 *
 * This function must be used to remove a subscriber from a list. If a NULL subscriber is provided
 * or an invalid topic is used, the return will be an error code.
 *
 * PRE-CONDITION: Subscriber list initialized
 *
 * POST-CONDITION: Subscriber removed from list, or not
 *
 * @return     eAPPError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * if (!eUnsubscription(&subscriber))
 *   PRINTF("Subscribed removed");
 ~~~~~~~~~~~~~~~
 *
 * @see BRK_wNumberOfTopicSubscribers, BRK_eVerifyDuplicate, BRK_eSubscription, BRK_eUnsubscription
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
static eAPPError_s BRK_eUnsubscription (signature_s * sSubscriber)
{
	if (NULL == sSubscriber)
	{
		return APP_ERROR_BROKER_LIST_NOT_FOUND;
	}
	if (sSubscriber->eTopic == TOPIC_ALL) // unsubscribe  from all topics
	{
		uint16_t wIndex;
		for (wIndex = 0; wIndex < TOPIC_LAST; wIndex++)
		{
			sSubscriber->eTopic = (topic_e)wIndex; //Change subscriber topic to this specific iteration
			BRK_ePopFromList(&HEAD(wIndex), sSubscriber);
		}
		sSubscriber->eTopic = TOPIC_ALL;
		return APP_ERROR_SUCCESS;
	}
	else if (sSubscriber->eTopic < TOPIC_ALL) // Single topic subscription
	{
		eAPPError_s eError = BRK_ePopFromList(&HEAD(sSubscriber->eTopic), sSubscriber);
		return eError;
	}
	else
	{
		return APP_ERROR_BROKER_INVALID_TOPIC;
	}
}

eAPPError_s BRK_eInitializeLists (void)
{
	uint8_t bIndex;
	for (bIndex = 0; bIndex < TOPIC_LAST; bIndex++)
	{
		sSubscriptionList[bIndex] = NULL;
		if (BRK_wNumberOfTopicSubscribers((topic_e)bIndex))
		{
			return APP_ERROR_ERROR;
		}
	}

	return APP_ERROR_SUCCESS;
}

eAPPError_s BRK_eBrokerListUpdate (bool add, signature_s * sSubscriber)
{
	osEnterCritical();
	//Handle list
	eAPPError_s eResult;
	if (add)  //Subscription
	{
		eResult = BRK_eSubscription(sSubscriber);
	}
	else  //Unsubscription
	{
		eResult = BRK_eUnsubscription(sSubscriber);
	}
	osExitCritical();
	return eResult;
}

signature_s * BRK_eBrokerListSearch (topic_e eTopic, signature_s * sCurrentNode)
{
	if ((sCurrentNode == NULL) || (eTopic >= TOPIC_LAST))
	{
		return NULL;
	}
	return &(BRK_sSearchInList(&sSubscriptionList[eTopic], sCurrentNode))->sSubscriber;
}

signature_s * BRK_eFindNextInList (topic_e eTopic, signature_s * sCurrentNode)
{
	if (eTopic < TOPIC_LAST) //Valid topic?
	{
		if (sCurrentNode == NULL) //I want the HEAD (start of list) as response
		{
			return &sSubscriptionList[eTopic]->sSubscriber;  //This is the HEAD
		}
		else  //Another starting point was passed
		{
			//Fetch the next node and verify it it is not null
			node_s * pReturn = (BRK_sSearchInList(&sSubscriptionList[eTopic], sCurrentNode))->sNext;
			if (pReturn != NULL)
			{
				return &pReturn->sSubscriber; //Return the subscriber data associated with it
			}
		}
	}
	return NULL;  //Invalid input data or no more itens in list;
}

/******************************************************************************
 * Unity Testing
 *******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(BrokerModuleTest);

TEST_SETUP(BrokerModuleTest)
{
	//This is run before EACH TEST
	int bIndex;
	for (bIndex = 0; bIndex < TOPIC_LAST; bIndex++)
	{
		sSubscriptionList[bIndex] = NULL;
	}
}

TEST_TEAR_DOWN(BrokerModuleTest)
{
}

/*
 * This test will verify the list initialization to all NULLs
 */
TEST(BrokerModuleTest, test_Verify_that_initialization_is_correct)
{
	//All of these should pass
	int bIndex;
	for (bIndex = 0; bIndex < TOPIC_LAST; bIndex++)
	{
		TEST_ASSERT_EQUAL(NULL, BRK_wListLength(sSubscriptionList[bIndex]));
	}
}

/*
 * This test will verify if push is working
 */
TEST(BrokerModuleTest, test_that_push_is_working)
{
	signature_s teste;
	node_s * node;
	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_AUTHENTICATION;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	TEST_ASSERT_EQUAL(1, BRK_wListLength(sSubscriptionList[teste.eTopic]));
	TEST_ASSERT_EQUAL(node->sSubscriber.eDestine, teste.eDestine);
	TEST_ASSERT_EQUAL(node->sSubscriber.eTopic, teste.eTopic);
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node);

	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste);   //Restore

}

/*
 * This test will verify if double push into two different topics is working
 */
TEST(BrokerModuleTest, test_that_double_push_is_working)
{
	signature_s teste, teste2;
	node_s * node, *node2;
	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_AUTHENTICATION;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	TEST_ASSERT_EQUAL(1, BRK_wListLength(sSubscriptionList[teste.eTopic])); //Verify size
	TEST_ASSERT_EQUAL(1, BRK_wListLength(sSubscriptionList[teste2.eTopic]));//Verify size
	TEST_ASSERT_EQUAL(node->sSubscriber.eDestine, teste.eDestine);//Compare stored information for equality
	TEST_ASSERT_EQUAL(node->sSubscriber.eTopic, teste.eTopic);//Compare stored information for equality
	TEST_ASSERT_EQUAL(node2->sSubscriber.eDestine, teste2.eDestine);//Compare stored information for equality
	TEST_ASSERT_EQUAL(node2->sSubscriber.eTopic, teste2.eTopic);//Compare stored information for equality
	TEST_ASSERT_NOT_EQUAL(node2->sSubscriber.eDestine, teste.eDestine);//cross compare, must be different
	TEST_ASSERT_NOT_EQUAL(node2->sSubscriber.eTopic, teste.eTopic);//cross compare, must be different
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node);//compare pointer
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste2.eTopic], node2);//compare pointer

	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste);//Restore
	BRK_ePopFromList(&sSubscriptionList[teste2.eTopic], &teste2);//Restore
}

/*
 * This test will push two nodes into the same topic. Then it will verify each topic length,
 * each node created and cross compare nodes for unequality
 */
TEST(BrokerModuleTest, test_that_double_push_into_same_topic_is_working)
{
	signature_s teste, teste2;
	node_s * node, *node2;
	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste2);

	TEST_ASSERT_EQUAL(2, BRK_wListLength(sSubscriptionList[teste.eTopic])); //Verify size
	TEST_ASSERT_EQUAL(2, BRK_wListLength(sSubscriptionList[teste2.eTopic]));//Verify size

	TEST_ASSERT_EQUAL_PTR(node2, sSubscriptionList[teste.eTopic]);//node2 was inserted last, thus it will be the HEAD
	TEST_ASSERT_EQUAL_PTR(node, sSubscriptionList[teste.eTopic]->sNext);//node was inserted first, so it will be next

	TEST_ASSERT_EQUAL(node->sSubscriber.eTopic, sSubscriptionList[teste.eTopic]->sNext->sSubscriber.eTopic);//Compare stored information for equality with second node
	TEST_ASSERT_EQUAL(node->sSubscriber.eDestine, sSubscriptionList[teste.eTopic]->sNext->sSubscriber.eDestine);//Compare stored information for equality with second node

	TEST_ASSERT_EQUAL(node2->sSubscriber.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);//Compare stored information for equality with HEAD
	TEST_ASSERT_EQUAL(node2->sSubscriber.eTopic, sSubscriptionList[teste.eTopic]->sSubscriber.eTopic);//Compare stored information for equality with HEAD

	TEST_ASSERT_NOT_EQUAL(node2->sSubscriber.eDestine, sSubscriptionList[teste.eTopic]->sNext->sSubscriber.eDestine);//cross compare, must be different

	//Test other topics size
	TEST_ASSERT_NOT_EQUAL(1, BRK_wListLength(sSubscriptionList[TOPIC_COMM]));
	TEST_ASSERT_NOT_EQUAL(1, BRK_wListLength(sSubscriptionList[TOPIC_AUTHENTICATION]));
	TEST_ASSERT_NOT_EQUAL(1, BRK_wListLength(sSubscriptionList[TOPIC_DIAGNOSTIC]));
	TEST_ASSERT_NOT_EQUAL(1, BRK_wListLength(sSubscriptionList[TOPIC_INPUT]));

	//Test for NULL pointers
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_COMM]);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_AUTHENTICATION]);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_DIAGNOSTIC]);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_INPUT]);

	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste);//Restore
	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste2);//Restore
}

/*
 * This test will try to pop from a list with a single node and verify the resulting list
 */
TEST(BrokerModuleTest, test_pop_from_topic)
{
	signature_s teste;
	node_s * node;
	eAPPError_s eError;
	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	TEST_ASSERT_EQUAL(1, BRK_wListLength(sSubscriptionList[teste.eTopic])); // Verify size
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node);// Verify pointer

	eError = BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &node->sSubscriber);//Pop from list

	TEST_ASSERT_EQUAL(0, BRK_wListLength(sSubscriptionList[teste.eTopic]));// Verify size
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);// Verify response
	TEST_ASSERT_NULL(sSubscriptionList[teste.eTopic]);// Verify NULL pointer
}

/*
 * This test will pop a non HEAD node and verify the response
 */
TEST(BrokerModuleTest, test_pop_other_than_head)
{
	signature_s teste, teste2;
	node_s * node, *node2;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	TEST_ASSERT_EQUAL(2, BRK_wListLength(sSubscriptionList[teste.eTopic])); // Verify size
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node2);// Verify HEAD pointer
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic]->sNext, node);// Verify pointer

	eError = BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &node->sSubscriber);//Pop from list

	TEST_ASSERT_EQUAL(1, BRK_wListLength(sSubscriptionList[teste.eTopic]));// Verify size
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);// Verify response
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node2);// Verify HEAD pointer
	TEST_ASSERT_NULL(sSubscriptionList[teste.eTopic]->sNext);// Verify NULL pointer
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);//Verify data

	BRK_ePopFromList(&sSubscriptionList[teste2.eTopic], &teste2);//Restore
}

/*
 * This test will pop the HEAD node and verify if the list moved
 */
TEST(BrokerModuleTest, test_pop_from_head_and_move_list)
{
	signature_s teste, teste2;
	node_s * node, *node2;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	TEST_ASSERT_EQUAL(2, BRK_wListLength(sSubscriptionList[teste.eTopic])); // Verify size
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node2);// Verify HEAD pointer
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic]->sNext, node);// Verify pointer

	eError = BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &node2->sSubscriber);//Pop from list

	TEST_ASSERT_EQUAL(1, BRK_wListLength(sSubscriptionList[teste.eTopic]));// Verify size
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);// Verify response
	TEST_ASSERT_EQUAL_PTR(sSubscriptionList[teste.eTopic], node);// Verify HEAD pointer
	TEST_ASSERT_NULL(sSubscriptionList[teste.eTopic]->sNext);// Verify NULL pointer
	TEST_ASSERT_EQUAL(teste.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);//Verify data

	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste);//Restore
}

/*
 * This test will try to pop a null node and verify the response
 */
TEST(BrokerModuleTest, test_pop_from_null_pointer)
{
	signature_s teste;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;

	eError = BRK_ePopFromList(&sSubscriptionList[TOPIC_ACTUATOR], &teste);  //Pop from list

	TEST_ASSERT_EQUAL(APP_ERROR_BROKER_LIST_NOT_FOUND, eError);//Assert not found

}

/*
 * This test will pop the middle node from a list with three nodes and verify if pointers moved correctly
 */
TEST(BrokerModuleTest, test_pop_middle_node_from_list_with_three_nodes)
{
	signature_s teste, teste2, teste3;
	node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ACTUATOR;
	node3 = BRK_sPushIntoList(&sSubscriptionList[teste3.eTopic], &teste3);

	TEST_ASSERT_EQUAL(3, BRK_wListLength(sSubscriptionList[teste.eTopic])); // Verify size

	nodetest = BRK_sSearchInList(&sSubscriptionList[teste.eTopic], &teste2);
	TEST_ASSERT_NOT_NULL(nodetest);
	TEST_ASSERT_EQUAL_PTR(node2, nodetest);

	eError = BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &nodetest->sSubscriber);//Pop middle node
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);// Verify response

	//Verify linked list moved pointers
	TEST_ASSERT_EQUAL_PTR(node, node3->sNext);
	TEST_ASSERT_EQUAL(node->sSubscriber.eDestine, node3->sNext->sSubscriber.eDestine);

	teste.eDestine = MODULE_INPUT_KEYPAD;//Change value to check if value or pointer to value was stored
	nodetest = BRK_sSearchInList(&sSubscriptionList[teste.eTopic], &teste);
	TEST_ASSERT_NULL(nodetest);

	teste.eDestine = MODULE_ACTUATOR;
	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste);//Restore
	BRK_ePopFromList(&sSubscriptionList[teste2.eTopic], &teste2);//Restore
	BRK_ePopFromList(&sSubscriptionList[teste3.eTopic], &teste3);//Restore
}

/*
 * This test will search the middle node from a list with three nodes
 */
TEST(BrokerModuleTest, test_search_in_list_with_three_nodes)
{
	signature_s teste, teste2, teste3;
	node_s * node, *node2, *node3, *nodetest;
	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ACTUATOR;
	node3 = BRK_sPushIntoList(&sSubscriptionList[teste3.eTopic], &teste3);

	TEST_ASSERT_EQUAL(3, BRK_wListLength(sSubscriptionList[teste.eTopic])); // Verify size

	nodetest = BRK_sSearchInList(&sSubscriptionList[teste.eTopic], &teste2);

	TEST_ASSERT_NOT_NULL(nodetest);

	TEST_ASSERT_EQUAL_PTR(node2, nodetest);

	BRK_ePopFromList(&sSubscriptionList[teste.eTopic], &teste);//Restore
	BRK_ePopFromList(&sSubscriptionList[teste2.eTopic], &teste2);//Restore
	BRK_ePopFromList(&sSubscriptionList[teste3.eTopic], &teste3);//Restore
	(void)node;
	(void)node3;
}

/*
 * This test will verify the addition of a subscriber into a single topic using broker services
 */
TEST(BrokerModuleTest, test_adding_of_subscriber_into_a_single_topic_using_broker)
{
	signature_s teste;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;

	eError = BRK_eBrokerListUpdate(true, &teste);//Insert into valid topic
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_EQUAL(teste.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);

	BRK_eBrokerListUpdate(false, &teste);//Restore

}

/*
 * This test will verify the addition of a subscriber into am invalid topic using broker services
 */
TEST(BrokerModuleTest, test_adding_of_subscriber_into_an_invalid_topic_using_broker)
{
	signature_s teste3;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ALL + 1;

	eError = BRK_eBrokerListUpdate(true, &teste3);//Insert into invalid topic
	TEST_ASSERT_EQUAL(APP_ERROR_BROKER_INVALID_TOPIC, eError);

}

/*
 * This test will verify the addition of a subscriber into all topics using broker services
 */
TEST(BrokerModuleTest, test_adding_of_subscriber_into_multiple_topics_using_broker)
{
	signature_s teste2;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ALL;

	eError = BRK_eBrokerListUpdate(true, &teste2);//Insert into all topics
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_AUTHENTICATION));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_COMM));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_DIAGNOSTIC));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_INPUT));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_MEMORY));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_OUTPUT));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_SENSOR));// Verify size
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_UPDATE));// Verify size

	//Verify list linkage
	TEST_ASSERT_NOT_NULL(sSubscriptionList[TOPIC_ACTUATOR]);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_ACTUATOR]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_AUTHENTICATION]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_COMM]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_DIAGNOSTIC]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_INPUT]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_MEMORY]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_OUTPUT]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_SENSOR]->sSubscriber.eDestine);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_UPDATE]->sSubscriber.eDestine);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_ACTUATOR]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_AUTHENTICATION]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_COMM]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_DIAGNOSTIC]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_INPUT]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_MEMORY]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_OUTPUT]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_SENSOR]->sNext);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_UPDATE]->sNext);

	BRK_eBrokerListUpdate(false, &teste2);//Restore
}

/*
 * This test will verify the addition of a duplicated subscriber into a single topic using broker services
 */
TEST(BrokerModuleTest, test_do_not_add_duplicated_subscriber_into_a_single_topic_using_broker)
{
	signature_s teste;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;

	eError = BRK_eBrokerListUpdate(true, &teste);//Insert into valid topic
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_EQUAL(teste.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);

	eError = BRK_eBrokerListUpdate(true, &teste);//Insert into valid topic
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_EQUAL(teste.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);

	BRK_eBrokerListUpdate(false, &teste);//Restore
}

/*
 * This test will verify the addition of a subscriber into all topics using broker services
 */
TEST(BrokerModuleTest, test_do_not_add_duplicated_subscriber_into_multiple_topics_using_broker)
{
	signature_s teste2;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ALL;

	eError = BRK_eBrokerListUpdate(true, &teste2);//Insert into all topics
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size

	//Verify list linkage
	TEST_ASSERT_NOT_NULL(sSubscriptionList[TOPIC_ACTUATOR]);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_ACTUATOR]->sSubscriber.eDestine);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_ACTUATOR]->sNext);

	eError = BRK_eBrokerListUpdate(true, &teste2);//Insert of duplicate into all topics
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size

	//Verify list linkage
	TEST_ASSERT_NOT_NULL(sSubscriptionList[TOPIC_ACTUATOR]);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_ACTUATOR]->sSubscriber.eDestine);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_ACTUATOR]->sNext);

	BRK_eBrokerListUpdate(false, &teste2);//Restore
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_ACTUATOR]);
}

/*
 * This test will verify that double subscription is avoided
 */
TEST(BrokerModuleTest, test_of_verify_duplicate_broker_service)
{
	signature_s teste, teste2;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;

	teste2.eDestine = MODULE_COMM_CBA;
	teste2.eTopic = TOPIC_ACTUATOR;

	eError = BRK_eBrokerListUpdate(true, &teste);//Insert into valid topic
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_EQUAL(teste.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);

	TEST_ASSERT_EQUAL(APP_ERROR_BROKER_ALREADY_IN_LIST, BRK_eVerifyDuplicate(&teste));
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, BRK_eVerifyDuplicate(&teste2));

	BRK_eBrokerListUpdate(false, &teste);//Restore
}

/*
 * This test will verify the unsubscription using broker services
 */
TEST(BrokerModuleTest, test_removing_subscriber_from_single_topic_using_broker_service)
{
	signature_s teste;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;

	eError = BRK_eBrokerListUpdate(true, &teste);//Insert into valid topic
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_EQUAL(teste.eDestine, sSubscriptionList[teste.eTopic]->sSubscriber.eDestine);

	eError = BRK_eBrokerListUpdate(false, &teste);//Removing subscriber
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(0, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
	TEST_ASSERT_NULL(sSubscriptionList[teste.eTopic]);
}

/*
 * This test will verify the unsubscription of an invalid topic using broker service
 */
TEST(BrokerModuleTest, test_removing_subscriber_from_invalid_topic_using_broker_service)
{
	signature_s teste3;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ALL + 1;

	eError = BRK_eBrokerListUpdate(false, &teste3);//Insert into invalid topic
	TEST_ASSERT_EQUAL(APP_ERROR_BROKER_INVALID_TOPIC, eError);

}

/*
 * This test will verify the unsubscription of all topics using broker service
 */
TEST(BrokerModuleTest, test_removing_subscriber_from_all_topics_using_broker_service)
{
	signature_s teste2;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ALL;

	eError = BRK_eBrokerListUpdate(true, &teste2);//Insert into all topics
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(1, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size

	//Verify list linkage
	TEST_ASSERT_NOT_NULL(sSubscriptionList[TOPIC_ACTUATOR]);
	TEST_ASSERT_EQUAL(teste2.eDestine, sSubscriptionList[TOPIC_ACTUATOR]->sSubscriber.eDestine);
	TEST_ASSERT_NULL(sSubscriptionList[TOPIC_ACTUATOR]->sNext);

	teste2.eTopic = TOPIC_ALL;
	eError = BRK_eBrokerListUpdate(false, &teste2);//Remove from all topics
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);
	TEST_ASSERT_EQUAL(0, BRK_wNumberOfTopicSubscribers(TOPIC_ACTUATOR));// Verify size
}
/*
 * This test will verify the search function using broker service
 */
TEST(BrokerModuleTest, test_of_search_function_broker_service)
{
	signature_s teste, teste2, teste3;
	//node_s * node, *node2, *node3, *nodetest;
	eAPPError_s eError;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;

	teste2.eDestine = MODULE_COMM_CBA;
	teste2.eTopic = TOPIC_ACTUATOR;

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ACTUATOR;

	eError = BRK_eBrokerListUpdate(true, &teste);//Insert
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);

	eError = BRK_eBrokerListUpdate(true, &teste2);//Insert
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);

	eError = BRK_eBrokerListUpdate(true, &teste3);//Insert
	TEST_ASSERT_EQUAL(APP_ERROR_SUCCESS, eError);

	signature_s * nodetest = BRK_eBrokerListSearch(teste2.eTopic, &teste2);
	TEST_ASSERT_EQUAL(teste2.eDestine, nodetest->eDestine);

	BRK_eBrokerListUpdate(false, &teste);//Restore
	BRK_eBrokerListUpdate(false, &teste2);//Restore
	BRK_eBrokerListUpdate(false, &teste3);//Restore
}

/*
 * This test will verify the find next subscriber using broker service
 */
TEST(BrokerModuleTest, test_of_eFindNextInList_function)
{
	signature_s teste, teste2, teste3, *testeResult;
	node_s * node, *node2, *node3;     //, *nodetest;

	testeResult = NULL;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ACTUATOR;
	node3 = BRK_sPushIntoList(&sSubscriptionList[teste3.eTopic], &teste3);

	TEST_ASSERT_NULL(testeResult);
	testeResult = BRK_eFindNextInList(teste.eTopic, testeResult);
	TEST_ASSERT_EQUAL(teste3.eDestine, testeResult->eDestine);

	testeResult = BRK_eFindNextInList(teste.eTopic, testeResult);
	TEST_ASSERT_EQUAL(teste2.eDestine, testeResult->eDestine);

	testeResult = BRK_eFindNextInList(teste.eTopic, testeResult);
	TEST_ASSERT_EQUAL(teste.eDestine, testeResult->eDestine);

	testeResult = BRK_eFindNextInList(teste.eTopic, testeResult);
	TEST_ASSERT_NULL(testeResult);

	BRK_eBrokerListUpdate(false, &teste);//Restore
	BRK_eBrokerListUpdate(false, &teste2);//Restore
	BRK_eBrokerListUpdate(false, &teste3);//Restore
	(void)node;
	(void)node2;
	(void)node3;
	//(void)nodetest;
}

/*
 * This test will verify the error conditions of find next subscriber using broker service
 */
TEST(BrokerModuleTest, test_of_eFindNextInList_function_error_conditions)
{
	signature_s teste, teste2, teste3, *testeResult;
	node_s * node, *node2, *node3;     //, *nodetest;

	testeResult = NULL;

	teste.eDestine = MODULE_ACTUATOR;
	teste.eTopic = TOPIC_ACTUATOR;
	node = BRK_sPushIntoList(&sSubscriptionList[teste.eTopic], &teste);

	teste2.eDestine = MODULE_AUTHENTICATION;
	teste2.eTopic = TOPIC_ACTUATOR;
	node2 = BRK_sPushIntoList(&sSubscriptionList[teste2.eTopic], &teste2);

	teste3.eDestine = MODULE_DIAGNOSTIC;
	teste3.eTopic = TOPIC_ACTUATOR;
	node3 = BRK_sPushIntoList(&sSubscriptionList[teste3.eTopic], &teste3);

	TEST_ASSERT_NULL(testeResult);
	testeResult = BRK_eFindNextInList(TOPIC_LAST, testeResult);
	TEST_ASSERT_NULL(testeResult);

	testeResult = BRK_eFindNextInList(TOPIC_ALL, testeResult);
	TEST_ASSERT_NULL(testeResult);

	testeResult = BRK_eFindNextInList(teste.eTopic, testeResult);
	TEST_ASSERT_EQUAL(teste3.eDestine, testeResult->eDestine);

	BRK_eBrokerListUpdate(false, &teste);//Restore
	BRK_eBrokerListUpdate(false, &teste2);//Restore
	BRK_eBrokerListUpdate(false, &teste3);//Restore
	(void)node;
	(void)node2;
	(void)node3;
	//(void)nodetest;
}
#endif
