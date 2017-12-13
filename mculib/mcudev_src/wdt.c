/****************************************************************************
 * Title                 :   wdt Include File
 * Filename              :   wdt.c
 * Author                :   thiago.palmieri
 * Origin Date           :   6 de mai de 2016
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
 *  6 de mai de 2016   1.0.0   thiago.palmieri wdt include file Created.
 *
 *****************************************************************************/
/** @file wdt.c
 *  @brief This file provides provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of Watchdo functions
 *  TODO: Fix text
 *  In order to correctly use ADC functions, a adc_config_s config must be created
 *  and populated with initial ADC configuration. This struct must be initialized
 *  via ADC_eInit before ADC sampling (ADC_vStartSampling) may be used to start measurements.
 *
 *  If configured sample method is pooling, then the Sample function (ADC_hSample) must be used
 *  whenever a reading must be performed, otherwise, the reading will be provided
 *  via callback though Interrupts.
 *
 *  To stop the sampling, function ADC_vStopSampling must be used and in order to
 *  de-initialize the ADC, ADC_vDeInit must be used.
 *
 *  ATTENTION: If the Structure has a pointer to private data, it is recommended to
 *  ALWWAYS INITIALIZE IT TO NULL.
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "wdt.h"
#include <stdlib.h>
#include "board.h"
#ifdef USE_SYSVIEW
#include "SEGGER.h"
#include "SEGGER_SYSVIEW.h"
#endif
/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
#define EVT_WDT  7
#define EVT_RESET  19
/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static bool bWDTEnabled = false;  //!< Inidicates if watchdog is enabled
static wdtCallBack pfCallback = NULL; //!< Callback to be used inside interruption
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
void WDT_IRQHandler (void)
{
	Chip_WWDT_ClearStatusFlag(LPC_WWDT, WWDT_WDMOD_WDINT); //Clear Interrupt flag
	Chip_WWDT_Feed(LPC_WWDT);  //Restart watchdog
	if (pfCallback != NULL)
	{
		pfCallback();
	}
	Chip_WWDT_Feed(LPC_WWDT);  //Restart watchdog
}
// Timeout formula:
// time = (4 / CLOCK) * TCvalue
// so for a minimum of 0.001 second, using the internal oscillator (12MHz):
// ( 0.001 / (4 / 12MHz) ) = TCValue = 0xBB8
// Therefore every millisecond equals 0xBB8 to be loaded in TC
eMCUError_s WDT_eInit (wdt_config_s * pWDT)
{
	/* WWDT input to event router is active high. */
	LPC_EVRT->HILO |= (1u << EVT_WDT);

	if ((pWDT == NULL) || (pWDT->wTimeout == 0))
	{
		return MCU_ERROR_WDT_UNABLE_TO_INITIALIZE;
	}
	if (!bWDTEnabled)
	{
		Chip_WWDT_Init(LPC_WWDT);
		if (pWDT->wTimeout >= 5000) //Max of 5000 milliseconds (or 5 seconds or 1/12 of a minute !!!)
		{
			pWDT->wTimeout = 5000;
		}

		Chip_WWDT_SetTimeOut(LPC_WWDT, (pWDT->wTimeout * 0xBB8)); //See formula calculation above (may the force be with you)
		Chip_WWDT_ClearStatusFlag(LPC_WWDT, (WWDT_WDMOD_WDTOF | WWDT_WDMOD_WDINT)); //clear events

		if (pWDT->pfWDTCallBack != NULL)
		{
			Chip_WWDT_SetWarning(LPC_WWDT, 0x3FF);  //Set interrupt on timer expiration
			Chip_WWDT_UnsetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);  //deactivate reset mode
			pfCallback = pWDT->pfWDTCallBack;
		}
		else
		{
			//Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDRESET); //activate reset mode
			Chip_WWDT_UnsetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);  //deactivate reset mode
			pfCallback = pWDT->pfWDTCallBack;
		}
		bWDTEnabled = true;
	}
	return MCU_ERROR_SUCCESS;
}

eMCUError_s WDT_eDeInit (const wdt_config_s * pWDT)
{
	Chip_WWDT_DeInit(LPC_WWDT);
	return MCU_ERROR_WDT_UNABLE_TO_DISABLE;
}

eMCUError_s WDT_eFeed (const wdt_config_s * pWDT)
{
	if ((pWDT == NULL) || !bWDTEnabled)
	{
		return MCU_ERROR_WDT_NOT_INITIALIZED;
	}
	Chip_WWDT_Feed(LPC_WWDT);
	return MCU_ERROR_SUCCESS;
}

eMCUError_s WDT_eStart (const wdt_config_s * pWDT)
{
	if ((pWDT == NULL) || !bWDTEnabled)
	{
		return MCU_ERROR_WDT_NOT_INITIALIZED;
	}
	Chip_WWDT_Start(LPC_WWDT);
	NVIC_EnableIRQ(WWDT_IRQn);
	if (pWDT->pfWDTCallBack != NULL)
	{
		NVIC_SetPriority(WWDT_IRQn, 0);
		NVIC_EnableIRQ(WWDT_IRQn);
	}
	return MCU_ERROR_SUCCESS;
}

uint8_t WDT_bFetchResetCause (void)
{
	/*
	 * We look for the event router STATUS register instead:
	 * - After power up, both WWDT_ST and RESET_ST will be set.
	 * - After an external reset, RESET_ST will be set.
	 * - After a watchdog reset, WWDT_ST will be set, while RESET_ST is cleared.
	 * Therefore, the condition to detect a watchdog reset is to look for
	 * WWDT_ST=1 and RESET_ST=0 in the Event Router STATUS register.
	 */
	if ((LPC_EVRT->STATUS & (1u << EVT_WDT)) && !(LPC_EVRT->STATUS & (1u << EVT_RESET)))
	{
		return (LPC_EVRT->STATUS & (1u << EVT_WDT)); /* Watchdog reset */
	}
	/* Clear all event indicators */
	LPC_EVRT->CLR_STAT = (0 | (1u << EVT_WDT) | (1u << EVT_RESET));
	return 0;
}

void WDT_vForceReset (void)
{
	//Chip_RGU_TriggerReset(RGU_WWDT_RST);  //Peripheral Reset not working on '-' revision of chip
	Chip_RGU_TriggerReset(RGU_CORE_RST);
}

/******************************************************************************
 * Unity Testing
 *******************************************************************************/
