/****************************************************************************
 * Title                 :   sensor_config Include File
 * Filename              :   sensor_config.h
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
 *  01/07/16   1.0.0    Henrique Reis    sensor_config.h created.
 *
 *****************************************************************************/
/** @file 	sensor_config.h
 *  @brief
 *
 */
#ifndef SENSOR_CONFIG_H_
#define SENSOR_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "devicelib.h"
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#if defined (UNITY_TEST)
#endif
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
//Handlers
#define SENSOR_ID               PERIPHERAL_M2GSENSORCOMM  //!< Handler

// M2GSENSORCOMM basic configurations
#define SENSOR_DEV_CAN	0x01	//!< Set M2GSENSORCOMM to use CAN

#define SENSOR_INITIAL_IO_IFACE	SENSOR_DEV_CAN

#define SENSOR_INIT_CAN_ID	        0x2F8
#define SENSOR_SEND_CAN_ID          0x2F7

//Main loop wait
#define SENSOR_QUEUE_WAIT 	0           //!< No timeout - wait forever
#define SENSOR_DEFAULT_MSGSIZE  (sizeof(uint32_t) + sizeof(CAN_tsCtrlListaSens))       //!< Default output format - 16 bytes

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Variables
 *******************************************************************************/
extern peripheral_descriptor_p pSENSORHandle;   //!< SENSOR Handler
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SENSOR_CONFIG_H_ */

/*** End of File **************************************************************/
