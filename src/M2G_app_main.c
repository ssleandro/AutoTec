/****************************************************************************
 * Title                 :   M2G_app_main Include File
 * Filename              :   M2G_app_main.c
 * Author                :   thiago.palmieri
 * Origin Date           :   24 de mar de 2016
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
 *  24 de mar de 2016   1.0.0   thiago.palmieri M2G_app_main file Created.
 *
 *****************************************************************************/
/** @file M2G_app_main.c
 *  @brief This file provides the MAIN function.
 *
 */
#ifndef UNITY_TEST
/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#include "broker_includeall.h"
#include "devicelib.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

#include <auteq_os.h>
#include <debug_tool.h>
#include <mcu.h>

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
/******************************************************************************
 * Function : vApplicationTickHook (void)
 *//**
 * \b Description:
 *
 * This function is executed by the OS in every tick. Any code inside it must be kept very short.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //This function must be started by the OS
 ~~~~~~~~~~~~~~~
 *
 * @see vApplicationMallocFailedHook, vApplicationStackOverflowHook, vApplicationTickHook
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 14/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void vApplicationTickHook (void)
{

}

/******************************************************************************
 * Function : vApplicationIdleHook (void)
 *//**
 * \b Description:
 *
 * This function is called on each cycle of the idle task if configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //This function must be started by the OS
 ~~~~~~~~~~~~~~~
 *
 * @see vApplicationMallocFailedHook, vApplicationStackOverflowHook, vApplicationTickHook, vApplicationIdleHook
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 14/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void vApplicationIdleHook (void)
{

}

/******************************************************************************
 * Function : main(void)
 *//**
 * \b Description:
 *
 * This is the MAIN function. Its main purpose is to start all necessary Threads
 * and the OS scheduler.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //This is the MAIN function
 ~~~~~~~~~~~~~~~
 *
 * @see main
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 30/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/

int main (void)
 {
	MCU_vSysInit();
	MCU_vFFSInit();

	/* Configure and initialize SystemView */
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
	SEGGER_SYSVIEW_Conf();
#endif

	osKernelInitialize();

	//Main Task
	osThreadDef_t sMainThread;

	sMainThread.name = "Main";
	sMainThread.stacksize = 500;
	sMainThread.tpriority = osPriorityRealtime;
	sMainThread.pthread = MAI_M2GMainThread;

	ASSERT_LEVEL(osThreadCreate(&sMainThread, NULL) != NULL, LEVEL_CRITICAL);

	osKernelStart();

	while (1)
	{
	} //You Shall not PASS !!!!!!
}
#endif
/******************************************************************************
 * Unity Testing
 *******************************************************************************/
