/****************************************************************************
 * Title                 :   rtc
 * Filename              :   rtc.c
 * Author                :   Henrique Reis
 * Origin Date           :   30/10/2017
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
 *    Date    Version       Author          Description
 * 30/10/2017  1.0.0     Henrique Reis      rtc.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/

#include "rtc.h"
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
 * Variables from others modules
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static bool bRTCEnabled = false;  			//!< Inidicates if RTC is enabled
static rtcCallBack fpRTCCallback = NULL; 	//!< Callback to be used inside interruption

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

void RTC_IRQHandler (void)
{
	sRTCTime sRTCStruct;

	if (Chip_RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE)) {

		Chip_RTC_GetFullTime(LPC_RTC, (RTC_TIME_T*) &sRTCStruct);

		if (fpRTCCallback != NULL)
		{
			fpRTCCallback(sRTCStruct);
		}

		/* Clear pending interrupt */
		Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
	}
}

eMCUError_s RTC_eInit (const rtc_config_s* pRTC)
{
	if (pRTC == NULL)
	{
		return MCU_ERROR_INVALID_CALLBACK;
	} else
	{
		fpRTCCallback = pRTC->fpRTCCallBack;
	}

	Chip_RTC_Init(LPC_RTC);

	/* Set the RTC to generate an interrupt on each second */
	Chip_RTC_CntIncrIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC, (fpRTCCallback != NULL) ? ENABLE : DISABLE);

	/* Enable matching for alarm for second, minute, hour fields only */
	Chip_RTC_AlarmIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC | RTC_AMR_CIIR_IMMIN | RTC_AMR_CIIR_IMHOUR, DISABLE);

	/* Clear interrupt pending */
	Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE | RTC_INT_ALARM);

	/* Enable RTC interrupt in NVIC */
	NVIC_EnableIRQ((IRQn_Type) RTC_IRQn);

	/* Enable RTC (starts increase the tick counter and second counter register) */
	Chip_RTC_Enable(LPC_RTC, ENABLE);
	bRTCEnabled = true;

	return MCU_ERROR_SUCCESS;
}

eMCUError_s RTC_eDeInit (void)
{
	Chip_RTC_DeInit(LPC_RTC);
	bRTCEnabled = false;

	return MCU_ERROR_SUCCESS;
}

void RTC_vGetFullTime (const sRTCTime* pTime)
{
	Chip_RTC_GetFullTime(LPC_RTC, (RTC_TIME_T*)pTime);
}

void RTC_vSetFullTime (const sRTCTime* pTime)
{
	Chip_RTC_SetFullTime(LPC_RTC, (RTC_TIME_T*)pTime);
}
