/****************************************************************************
 * Title                 :   isobus_core Include File
 * Filename              :   isobus_core.h
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
 *    Date    Version     Author         				Description
 *  01/07/16   1.0.0  Henrique Reis     isobus_core.h created
 *
 *****************************************************************************/
/** @file
 *  @brief
 *
 *
 */
#ifndef ISOBUS_SRC_ISOBUS_CORE_H_
#define ISOBUS_SRC_ISOBUS_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../isobus/inc/interface_isobus.h"
#else
#include "interface_isobus.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define ISO_NUM_NUMBER_VARIABLE_OBJECTS 482
#define ISO_NUM_INPUT_LIST_OBJECTS 9
#define ISO_NUM_FILL_ATTRIBUTES_OBJECTS 72
#define ISO_NUM_BAR_GRAPH_OBJECTS 72

#define ISO_NUM_LARGURA_IMPLEMENTO_INDEX 7

#define ISO_KEY_PLANTER_ID					0x3000
#define ISO_KEY_CONFIG_ID					0x3001
#define ISO_KEY_INSTALLATION_ID			0x3002
#define ISO_KEY_TEST_MODE_ID				0x3003
#define ISO_KEY_FINISH_TEST_ID			0x3004
#define ISO_KEY_REPEAT_TEST_ID			0x3005
#define ISO_KEY_REPLACE_SENSORS_ID		0x3007
#define ISO_KEY_ERASE_INSTALLATION_ID	0x3008
#define ISO_KEY_BACKTO_INSTALLATION_ID	0x3009
#define ISO_KEY_TRIMMING_ID				0x300A
#define ISO_KEY_SYSTEM_ID					0x300B
#define ISO_KEY_CLEAR_TOTAL_ID			0x300C
#define ISO_KEY_CLEAR_SUBTOTAL_ID		0x300D
#define ISO_KEY_BACKTO_PLANTER_ID		0x300E

#define ISO_BUTTON_REPEAT_TEST_ID			0x6005
#define ISO_BUTTON_ERASE_INSTALLATION_ID	0x6006
#define ISO_BUTTON_CLEAR_COUNT_CANCEL_ID	0x6034
#define ISO_BUTTON_CLEAR_COUNT_ACCEPT_ID	0x6035


/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define ISO_OBJECT_WORKING_SET_ID	  0x0000
#define ISO_OBJECT_PICTURE_GRAPHIC_ID 0x1000
#define ISO_OBJECT_SOFT_KEY_MASK_ID	  0x2000
#define ISO_OBJECT_POINTER_ID		  	  0x2100
#define ISO_OBJECT_KEY_ID			  	  0x3000
#define ISO_OBJECT_CONTAINER_ID		  0x4000
#define ISO_OBJECT_OUTPUT_STRING_ID	  0x4500
#define ISO_OBJECT_STRING_VARIABLE_ID 0x4700
#define ISO_OBJECT_DATA_MASK_ID		  0x5000
#define ISO_OBJECT_OUTPUT_NUMBER_ID   0x5500
#define ISO_OBJECT_BUTTON_ID		  	  0x6000
#define ISO_OBJECT_FONT_ID			  	  0x6500
#define ISO_OBJECT_BAR_GRAPH_ID		  0x7000
#define ISO_OBJECT_NUMBER_VARIABLE_ID 0x8000
#define ISO_OBJECT_INPUT_NUMBER_ID	  0x8200
#define ISO_OBJECT_INPUT_BOOLEAN_ID	  0x8500
#define ISO_OBJECT_OUTPUT_LINE_ID	  0x8900
#define ISO_OBJECT_INPUT_LIST_ID	     0x9000
#define ISO_OBJECT_RECTANGLE_ID		  0x9500
#define ISO_OBJECT_FILL_ATTRIBUTES_ID 0x9600
#define ISO_OBJECT_LINE_ID			 	  0x9900

#define ISO_OBJECT_LARGURA_IMPLEMENTO_ID			  0x812D

/******************************************************************************
 * Macros
 *******************************************************************************/
#define GET_INDEX_FROM_ID(id) (id & 0x0FFF)
#define GET_FLOAT_VALUE(value) ((float)(value/10.0f))
#define GET_UNSIGNED_INT_VALUE(value) ((uint32_t)(value*10))

/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef enum
{
	WAIT_GLOBAL_VT_STATUS,
	WAIT_VT_STATUS,
	WAIT_LOAD_VERSION,
	WAIT_SEND_POOL,
	OBJECT_POOL_SENDED,
	OBJECT_POOL_LOADED,
	BOOT_COMPLETED
} eBootStates;

typedef enum
{
	BOOT,
	RUNNING
} eModuleStates;

typedef enum
{
	CLEAR_TOTALS_IDLE,
	CLEAR_SUBTOTAL_WAIT_CONFIRMATION,
	CLEAR_SUBTOTAL,
	CLEAR_TOTAL_WAIT_CONFIRMATION,
	CLEAR_TOTAL,
} eClearCounterStates;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void ISO_vTimerCallbackWSMaintenance (void const *arg);

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* M2G_SRC_M2G_CORE_H_ */

/*** End of File **************************************************************/
