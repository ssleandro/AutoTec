/****************************************************************************
 * Title                 :   interface_broker Include File
 * Filename              :   interface_broker.h
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
 *  03/03/2016   1.0.0   thiago.palmieri interface_broker include file Created.
 *
 *****************************************************************************/
/** @file interface_broker.h
 *  @brief This file provides the broker module interface.
 *
 *  This header file must be included by all modules that need to send messages.
 *  This header provides the interface towards the broker regarding subscription,
 *  unsubscription and publishing. No other broker header file need to be included.
 *
 *  The src folder holds all code related to the previously mentioned functions.
 *  The test folder holds test code to be used for function tests and unit tests.
 *  The config folder holds all needed configuration for the broker.
 *
 *  PUB/SUB:
 *
 *  Any module can publish a message, even if it does not subscribe to any topic.
 *  For that, the module must include the interface_broker header file.
 *  In order to receive messages, the module must subscribe for a given topic,
 *  after its subscription, it will be eligible to receive publications on that topic.
 *  If the module does not want to receive any more publications for a given topic,
 *  it must perform an unsubscription of the before mentioned topic.
 *  A module can subscribe and unsubscribe to all topics.
 */
#ifndef BROKER_INC_INTERFACE_BROKER_H_
#define BROKER_INC_INTERFACE_BROKER_H_

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

/** @defgroup BROKER_MODULE
 * @ingroup BROKER_Interface
 * @{
 */

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define QUEUE_SIZEOFPUBLISH 128     //!< MACRO to define the size of PUBLISH queue
#define QUEUE_SIZEOFSUBSCRIBE 64    //!< MACRO to define the size of SUBSCRIBE queue
#define QUEUE_SIZEOFUNSUBSCRIBE 64  //!< MACRO to define the size of UNSUBSCRIBE queue

#if defined (UNITY_TEST)
typedef void * osMessageQId;
#endif
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/
//Queue MACROS
//!< Create a Message Queue Definition for broker Publishing
#define DECLARE_QUEUE(QueueName, QueueSize) \
    osMessageQDef(QueueName, QueueSize, contract_s); \
    osMessageQId  QueueName;
/*!< Create and Initialize a Message Queue */
#define INITIALIZE_QUEUE(QueueName) \
  (QueueName = osMessageCreate(osMessageQ(QueueName), NULL));  \
    ASSERT(QueueName != NULL);
/*!< Publish Contract to Broker */
#define PUBLISH(contract, time) (osMessagePut(BrokerPublish, (uint32_t)&contract, time))
/*!< Subscribe */
#define SUBSCRIBE(signature, time) (osMessagePut(BrokerSubscribe, (uint32_t)&signature, time))
/*!< Unsubscribe */
#define UNSUBSCRIBE(signature, time) (osMessagePut(BrokerUnsubscribe, (uint32_t)&signature, time))
/*!< Receive from Queue */
#define RECEIVE(fromQueue, time) (osMessageGet(fromQueue, time))

//Variable MACROS
/*!< Create Contract, Message and Lock both together */
#define CREATE_CONTRACT(name) \
      message_s s##name##Message; \
      contract_s s##name##Contract = { \
          .sMessage = &s##name##Message, \
      }

/*!< Create Signature Struct */
#define CREATE_SIGNATURE(name) \
    signature_s s##name##Signature;

/*!< Use Contract */
#define CONTRACT(name)  s##name##Contract

/*!< Update Contract Header data */
#define CONTRACT_HEADER(name, version, origin, topic) \
{ \
  CONTRACT(name).bVersion = version; \
  CONTRACT(name).eOrigin = origin; \
  CONTRACT(name).eTopic = topic; \
}

/*!< Use Message */
#define MESSAGE(name) s##name##Message

/*!< Update Message Header data */
#define MESSAGE_HEADER(name, Size, Key, Type) \
{ \
  MESSAGE(name).hMessageSize = Size; \
  MESSAGE(name).hMessageKey = Key; \
  MESSAGE(name).eMessageType = Type; \
}

/*!< Update Message Payload */
#define MESSAGE_PAYLOAD(name) MESSAGE(name).pvMessage

/*!< Use Signature */
#define SIGNATURE(name) s##name##Signature

/*!< Update Signature Header data */
#define SIGNATURE_HEADER(name, destine, topic, destinequeue) \
{ \
  SIGNATURE(name).eDestine = destine; \
  SIGNATURE(name).eTopic = topic; \
  SIGNATURE(name).vpDestineQueue = destinequeue; \
}

/*!< Get Contract from Event */
#define GET_CONTRACT(FromEvent) ((contract_s*)FromEvent.value.p)
/*!< Get topic from contract */
#define GET_TOPIC(FromContract) ((topic_e)FromContract->eTopic)
/*!< Get Message from Contract */
#define GET_MESSAGE(FromContract) ((message_s*)FromContract->sMessage)

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/**
 * This typedef defines the size of message aggregation (and not its format)
 *
 * Example: If a message Size is 3 and message type is MT_FOURBYTE, then it means
 * that the message is composed of 3 data of 32Bits size each.
 */
typedef enum messageType_e
{
	MT_BYTE = (1 << 0),  //!< 8 bits size
	MT_TWOBYTE = (1 << 1),  //!< 16 bits size
	MT_FOURBYTE = (1 << 2),  //!< 32 bits size
	MT_EIGHTBYTE = (1 << 3),  //!< 64 bits size
	MT_ARRAYBYTE = (1 << 4),  //!< Variable size
} messageType_e;

/**
 *  This Typedef defines the Message formating
 */
typedef struct message_s
{
	uint32_t hMessageKey;           //!< Message Key
	messageType_e eMessageType;     //!< Message Type
	uint16_t hMessageSize;          //!< Size of message
	void * pvMessage;               //!< Pointer to the message structure
} message_s;

/**
 *  This Typedef defines the Contracts that need to be used during publishing
 */
typedef struct contract_s
{
	origin_e eOrigin;         //!< Module that originated the message
	topic_e eTopic;           //!< Topic of the message
	uint8_t bVersion;         //!< Contract Version
	message_s * sMessage;     //!< Pointer to Message data
} contract_s;

/**
 *  This Typedef defines the Signature data needed to subscribed into a topic
 */
typedef struct signature_s
{
	destine_e eDestine;           //!< Module that is performing/canceling the subscription
	topic_e eTopic;               //!< Topic of the message
	osMessageQId vpDestineQueue;  //!< Module Queue
} signature_s;
/******************************************************************************
 * Variables
 *******************************************************************************/
#ifndef UNITY_TEST
EXTERN_QUEUE(BrokerPublish);     //!< identifies the message queue (pointer to a message queue control block).
EXTERN_QUEUE(BrokerSubscribe);   //!< identifies the message queue (pointer to a message queue control block).
EXTERN_QUEUE(BrokerUnsubscribe); //!< identifies the message queue (pointer to a message queue control block).
#endif
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
 * Function : BRK_vBrokerThread (void const *argument)
 *//**
 * \b Description:
 *
 * After the Thread is created, it will create other two threads responsible for
 * subscription and unsubscription. Then it will reach the main loop and will
 * wait for a publish message. When it arrives it will search the subscription list
 * for modules that need to receive the publication and forward it to them.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: Broker initialized
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * osThreadDef_t sBrokerThread;
 *
 * sBrokerThread.name = "Broker";
 * sBrokerThread.stacksize = 500;
 * sBrokerThread.tpriority = osPriorityNormal;
 * sBrokerThread.pthread = vBrokerThread;
 *
 * osThreadCreate(&sBrokerThread, NULL);
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
void BRK_vBrokerThread (void const *argument);

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
uint8_t * BRK_WDTData (uint8_t * pbNumberOfThreads);

#ifdef __cplusplus
} // extern "C"
#endif

/**
 * @}
 */

#endif /* BROKER_INC_INTERFACE_BROKER_H_ */

/*** End of File **************************************************************/
