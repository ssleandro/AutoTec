/****************************************************************************
 * Title                 :   CBT09427
 * Filename              :   CBT09427.c
 * Author                :   Henrique Reis
 * Origin Date           :   30 de set de 2016
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
 *  30/09/16   1.0.0    Henrique Reis    CBT09427.c created.
 *
 *****************************************************************************/
/** @file   CBT09427.c
 *  @brief
 *
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "CBT09427.h"
#include "CBT09427_config.h"
#include <string.h>
#ifndef UNITY_TEST
#include "mculib.h"
#else
#include "unity_fixture.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/
typedef eDEVError_s (*fpIOCTLFunction) (uint32_t wRequest, void * vpValue);

/**
 * Default Sensor Initialization Values
 */
static pwm_config_s sCBT09427 =
	{
		.eChannel = (pwm_channel_e) CBT_BUZZER_CHANNEL,
		.bDuty = CBT_BUZZER_DUTY,
		.wFrequency = CBT_BUZZER_FREQ,
	};

static eDEVError_s CBT_eChangeDuty (uint32_t wRequest, void * vpValue);
static eDEVError_s CBT_eChangeFrequency (uint32_t wRequest, void * vpValue);
static eDEVError_s CBT_eChangeStatus (uint32_t wRequest, void * vpValue);
static eDEVError_s CBT_eReadConfig (uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction CBT_pIOCTLFunction[] =  //!< IOCTL array of function mapping
	{
	IOCTL_CBT09427
	};
#undef X

static eDEVError_s CBT_eChangeDuty (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	if (*(uint8_t*)vpValue > 100)
	{
		*(uint8_t*)vpValue = 100;
	}

	sCBT09427.bDuty = *(uint8_t*)vpValue;
	PWM_vConfigDuty(&sCBT09427);
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s CBT_eChangeFrequency (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	sCBT09427.wFrequency = *(uint32_t*)vpValue;
	PWM_vConfigFrequency(&sCBT09427);
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s CBT_eChangeStatus (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	if (*(bool*)vpValue == true)
	{
		PWM_vEnable(&sCBT09427);
	}
	else
	{
		PWM_vDisable(&sCBT09427);
	}
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s CBT_eReadConfig (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	if (wRequest == IOCTL_CBT_READ_FREQ)
	{
		*(uint32_t*)vpValue = PWM_sReadConfig(sCBT09427.eChannel).wFrequency;
	}
	else if (wRequest == IOCTL_CBT_READ_DUTY)
	{
		*(uint8_t*)vpValue = PWM_sReadConfig(sCBT09427.eChannel).bDuty;
	}
	else
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	return DEV_ERROR_SUCCESS;
}

eDEVError_s CBT_open (void)
{
	sCBT09427.eChannel = (pwm_channel_e)CBT_BUZZER_CHANNEL;
	sCBT09427.bDuty = CBT_BUZZER_DUTY;
	sCBT09427.wFrequency = CBT_BUZZER_FREQ;

	eDEVError_s eError = (eDEVError_s)PWM_bInit(&sCBT09427); //Init PWM

	return eError;
}

uint32_t CBT_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	return 0;
}

uint32_t CBT_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	return 0; //Nothing to write
}

eDEVError_s CBT_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if ((wRequest >= IOCTL_CBT_INVALID) || (vpValue == NULL))
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	eDEVError_s eError;

	//Configure
	eError = CBT_pIOCTLFunction[wRequest](wRequest, vpValue);

	return eError;
}

eDEVError_s CBT_close (struct peripheral_descriptor_s* const this)
{
	PWM_vDisable(&sCBT09427);
	return DEV_ERROR_SUCCESS;
}
