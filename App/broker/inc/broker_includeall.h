/****************************************************************************
 * Title                 :   broker_includeall Include File
 * Filename              :   broker_includeall.h
 * Author                :   thiago.palmieri
 * Origin Date           :   11 de mar de 2016
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
 *  11 de mar de 2016   1.0.0   thiago.palmieri broker_includeall include file Created.
 *
 *****************************************************************************/
/** @file broker_includeall.h
 *  @brief This file must be included by the broker_core only. Any new Module must
 *  have its interface included in this header file.
 *
 */
#ifndef BROKER_INC_BROKER_INCLUDEALL_H_
#define BROKER_INC_BROKER_INCLUDEALL_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "test_broker.h"
#include "interface_diagnostic.h"
#include "interface_isobus.h"
#include "interface_sensor.h"
#include "interface_buzzer.h"
#include "interface_filesys.h"
#include "interface_gps.h"
#include "interface_acquireg.h"
#include "interface_control.h"
#include "interface_gui.h"
#include "interface_records.h"
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

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BROKER_INC_BROKER_INCLUDEALL_H_ */

/*** End of File **************************************************************/
