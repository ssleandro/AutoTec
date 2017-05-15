/****************************************************************************
* Title                 :   broker_core Include File
* Filename              :   broker_core
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
*  03/03/2016   1.0.0   thiago.palmieri broker_core include file Created.
*
*****************************************************************************/
/** @file broker_core.h
 *  @brief This file provides the basic broker message handling
 *
 *  The broker core holds a single task and one of the most important features, which
 *  is to republish messages only to subscribed modules.
 *
 *  The broker task must be the first task initiated on the system. After its initialization
 *  it will create other two tasks, responsible to handle the subscription and unsubscription.
 *
 *  The broker will sit waiting indefinitely for a publish message, when it arrives
 *  it sill search its subscription list and republish the message to all subscribed
 *  modules.
 */
#ifndef BROKER_BROKER_CORE_H_
#define BROKER_BROKER_CORE_H_

/******************************************************************************
* Includes
*******************************************************************************/
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

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

extern void BRK_vUnSubscriptorThread (void const *argument);

extern void BRK_vSubscriptorThread (void const *argument);

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* BROKER_BROKER_CORE_H_ */
/*** End of File **************************************************************/
