/****************************************************************************
 * Title                 :   unity_test_broker_runner Include File
 * Filename              :   unity_test_broker_runner.c
 * Author                :   thiago.palmieri
 * Origin Date           :   9 de mar de 2016
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
 *  9 de mar de 2016   1.0.0   thiago.palmieri unity_test_broker_runner include file Created.
 *
 *****************************************************************************/
/** @file unity_test_broker_runner.c
 *  @brief This file provides Unit Testing configuration
 *
 */
#if defined(UNITY_TEST)
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "unity_fixture.h"
/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
TEST_GROUP_RUNNER(BrokerModuleTest)
{
	RUN_TEST_CASE(BrokerModuleTest, test_Verify_that_initialization_is_correct);
	RUN_TEST_CASE(BrokerModuleTest, test_that_push_is_working);
	RUN_TEST_CASE(BrokerModuleTest, test_that_double_push_is_working);
	RUN_TEST_CASE(BrokerModuleTest, test_that_double_push_into_same_topic_is_working);
	RUN_TEST_CASE(BrokerModuleTest, test_pop_from_topic);
	RUN_TEST_CASE(BrokerModuleTest, test_pop_other_than_head);
	RUN_TEST_CASE(BrokerModuleTest, test_pop_from_head_and_move_list);
	RUN_TEST_CASE(BrokerModuleTest, test_pop_from_null_pointer);
	RUN_TEST_CASE(BrokerModuleTest, test_pop_middle_node_from_list_with_three_nodes);
	RUN_TEST_CASE(BrokerModuleTest, test_search_in_list_with_three_nodes);
	RUN_TEST_CASE(BrokerModuleTest, test_adding_of_subscriber_into_a_single_topic_using_broker);
	RUN_TEST_CASE(BrokerModuleTest, test_adding_of_subscriber_into_an_invalid_topic_using_broker);
	RUN_TEST_CASE(BrokerModuleTest, test_adding_of_subscriber_into_multiple_topics_using_broker);
	RUN_TEST_CASE(BrokerModuleTest, test_do_not_add_duplicated_subscriber_into_a_single_topic_using_broker);
	RUN_TEST_CASE(BrokerModuleTest, test_do_not_add_duplicated_subscriber_into_multiple_topics_using_broker);
	RUN_TEST_CASE(BrokerModuleTest, test_of_verify_duplicate_broker_service);
	RUN_TEST_CASE(BrokerModuleTest, test_removing_subscriber_from_single_topic_using_broker_service);
	RUN_TEST_CASE(BrokerModuleTest, test_removing_subscriber_from_invalid_topic_using_broker_service);
	RUN_TEST_CASE(BrokerModuleTest, test_removing_subscriber_from_all_topics_using_broker_service);
	RUN_TEST_CASE(BrokerModuleTest, test_of_search_function_broker_service);
	RUN_TEST_CASE(BrokerModuleTest, test_of_eFindNextInList_function);
	RUN_TEST_CASE(BrokerModuleTest, test_of_eFindNextInList_function_error_conditions)

}

TEST_GROUP_RUNNER(SubscriptorModuleTest)
{
	RUN_TEST_CASE(SubscriptorModuleTest, test_Verify_DetectThread);
}

#endif
