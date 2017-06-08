/****************************************************************************
 * Title                 :   tda3500_app Include File
 * Filename              :   tda3500_app.h
 * Author                :   thiago.palmieri
 * Origin Date           :   14/03/2016
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
 *  14/03/2016   1.0.0   thiago.palmieri tda3500_app include file Created.
 *
 *****************************************************************************/
/** @file tda3500_app.h
 *  @brief This file provides the default include header for the modules
 *
 */
#ifndef INC_M2G_APP_H_
#define INC_M2G_APP_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "cr_section_macros.h"
#include "devicelib.h"
#include <stdio.h>
#ifndef UNITY_TEST
#include "common_app.h"
#include <auteq_os.h>
#include <interface_broker.h>
#else
#include "../common/app_common/common_app.h"
#include "../broker/inc/interface_broker.h"
#endif
#include <string.h>
#include <math.h>

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#ifndef ASM
#if defined(__IAR_SYSTEMS_ICC__)
#define ASM asm
#elif defined (__GNUC__)
#define ASM __asm
#endif

#endif

#ifndef UNITY_TEST
#define GET_TIMESTAMP()      (*(volatile uint32_t *)(0xE0001004))     // Retrieve a system timestamp. Cortex-M cycle counter.
#else
#define GET_TIMESTAMP() 123456789
#endif
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
 * Function : MAI_M2GMainThread (void const *argument)
 *//**
 * \b Description:
 *
 * This is the main thread, it must be called from MAIN function. This thread is responsible
 * to start the system accordingly, including modules and watchdog routines. In order to use
 * watchdog, this thread must have the highest priority on the system.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: System started
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * osThreadDef_t sMainThread;
 *
 * sMainThread.name = "MainThread";
 * sMainThread.stacksize = 500;
 * sMainThread.tpriority = osPriorityRealTime;
 * sMainThread.pthread = MAI_M2GMainThread;
 *
 * osThreadCreate(&sMainThread, NULL);
 ~~~~~~~~~~~~~~~
 *
 * @see MAI_TDAMainThread, main
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 12/05/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
extern void MAI_M2GMainThread (void const *argument);

extern void MAI_vM2GHardwareTestThread (void const *argument);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_M2G_APP_H_ */

/*** End of File **************************************************************/
