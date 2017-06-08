/****************************************************************************
 * Title                 :   Watchdog Include File
 * Filename              :   wdt.c
 * Author                :   thiago.palmieri
 * Origin Date           :   12 de mai de 2016
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
 *  12 de mai de 2016   1.0.0   thiago.palmieri Watchdog include file Created.
 *
 *****************************************************************************/
/** @file wdt.c
 *  @brief This file provides basic watchdog implementation, see the wdt.h
 *  header file for implementation details.
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "debug_tool.h"
#include "wds.h"

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
static peripheral_descriptor_p pWDT = NULL;   //!< Watchdog descriptor pointer

static uint8_t* pbaWDTModuleFlagArray[MODULE_INVALID];
static uint8_t baWDTModuleFlagArraySize[MODULE_INVALID];
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
void WDS_vInterrupt (void)
{

}

#if defined(__USE_WATCHDOG)
eAPPError_s WDS_eFeed (void)
{
	uint8_t bDummy;
	return (eAPPError_s)DEV_ioctl(pWDT, IOCTL_WDT_FEED, &bDummy);
}

eAPPError_s WDS_eStart (void)
{
	memset(pbaWDTModuleFlagArray, 0x00, MODULE_INVALID);  //Clear array

	pWDT = DEV_open(PERIPHERAL_WDT);
	ASSERT_LEVEL(pWDT != NULL, LEVEL_CRITICAL);

	uint32_t wDummy = 0;
	DEV_ioctl(pWDT, IOCTL_WDT_RESTART_REASON, &wDummy);   //Fetch previous restart reason
	DBG_LOG_INFO("Reset Reason: %d", wDummy);

	ASSERT(DEV_ERROR_SUCCESS == DEV_ioctl(pWDT, IOCTL_WDT_RESET_MODE, &wDummy));       //WDT in reset mode
	wDummy = EXPIRATION_TIMER;
	ASSERT(DEV_ERROR_SUCCESS == DEV_ioctl(pWDT, IOCTL_WDT_SET_TIMEOUT, &wDummy));      //WDT Feed time expiration
	ASSERT(DEV_ERROR_SUCCESS == DEV_ioctl(pWDT, IOCTL_WDT_START, &wDummy));            //WDT start

	return APP_ERROR_SUCCESS;
}

eAPPError_s WDS_eAddModule (uint8_t* pbWDTModuleArray, uint8_t bArraySize, uint8_t bArrayPos)
{
	if (pbWDTModuleArray != NULL)
	{
		pbaWDTModuleFlagArray[bArrayPos] = pbWDTModuleArray;
		baWDTModuleFlagArraySize[bArrayPos] = bArraySize;
	}
	return APP_ERROR_SUCCESS;
}

static uint8_t WDS_eCheckFlag (uint8_t * pbModuleArray, uint8_t bQuantity)
{
	uint8_t bIndex = 0;
	while (bIndex < bQuantity)
	{
		if (pbModuleArray[bIndex] == WDT_ACTIVE)
		{
			pbModuleArray[bIndex] = WDT_UNKNOWN;
		}
		else if (pbModuleArray[bIndex] == WDT_UNKNOWN)
		{
			DBG_LOG_ERROR("Thread: %d Locked", bIndex);
			return bIndex;
		}
		bIndex++;
	}
	return (bQuantity + 1);
}

eAPPError_s WDS_eTreat (void)
{
	uint8_t bIndex = 0;
	for (bIndex = 0; bIndex < MODULE_INVALID; bIndex++)
	{
		if (pbaWDTModuleFlagArray[bIndex] != NULL)
		{
			uint8_t bThreadFault = WDS_eCheckFlag(pbaWDTModuleFlagArray[bIndex], baWDTModuleFlagArraySize[bIndex]);
			if (bThreadFault <= baWDTModuleFlagArraySize[bIndex])
			{
				osEnterCritical();
				//WDT error
				while (1)
				{
					DIG_onWDT__(GET_TIMESTAMP(), bIndex, bThreadFault);         //Log error
					DEV_ioctl(pWDT, IOCTL_WDT_FORCE_RESET, &bIndex);
				}
			}
		}
	}
	WDS_eFeed();
	return APP_ERROR_SUCCESS;
}

#endif

/******************************************************************************
 * Unity Testing
 *******************************************************************************/
