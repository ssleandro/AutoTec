/****************************************************************************
 * Title                 :   MCURTC
 * Filename              :   MCURTC.c
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
 * 30/10/2017  1.0.0     Henrique Reis      MCURTC.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "MCURTC.h"
#include "MCURTC_config.h"
#include <string.h>
#ifndef UNITY_TEST
#include "mculib.h"
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
#define MCURTC_STATUS_ACTIVE		0x00000001
#define MCURTC_STATUS_BUSY			0x00000002

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/
typedef eDEVError_s (*fpIOCTLFunction) (void * vpValue);

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
sRTCTime sRTCCurrentlFullTime;
static rtc_config_s sRTCConfig;
static uint8_t bRTCDeviceStatus;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#define X(a, b) b,
fpIOCTLFunction RTC_pIOCTLFunction[] =  //!< IOCTL array of function mapping
	{
	IOCTL_MCURTC
	};
#undef X

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
void vRTCCallbackFunction (sRTCTime sRTCTimeStruct)
{
	bRTCDeviceStatus = MCURTC_STATUS_BUSY;

	sRTCCurrentlFullTime = sRTCTimeStruct;

	bRTCDeviceStatus = MCURTC_STATUS_ACTIVE;
}

eDEVError_s RTC_open (void)
{
	sRTCConfig.fpRTCCallBack = vRTCCallbackFunction;

	eDEVError_s eError = (eDEVError_s) RTC_eInit(&sRTCConfig);
	return eError;
}

uint32_t RTC_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	(void)this;
	if (tBufferSize < sizeof(sRTCTime))
	{
		return 0;
	}

	if (bRTCDeviceStatus != MCURTC_STATUS_BUSY)
	{
		memcpy(vpBuffer, &sRTCCurrentlFullTime, sizeof(sRTCTime));
	}

	return sizeof(sRTCTime);
}

uint32_t RTC_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	(void)this;
	(void)tBufferSize;

	RTC_vSetFullTime((sRTCTime*)vpBuffer);

	return sizeof(sRTCTime);
}

eDEVError_s RTC_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	(void)this;
	(void)wRequest;
	(void)vpValue;
	return DEV_ERROR_SUCCESS;
}

eDEVError_s RTC_close (struct peripheral_descriptor_s* const this)
{
	RTC_eDeInit();
	return DEV_ERROR_SUCCESS;
}
