/****************************************************************************
 * Title                 :   EXTBUZZER
 * Filename              :   EXTBUZZER.c
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
 *  30/09/16   1.0.0    Henrique Reis    EXTBUZZER.c created.
 *
 *****************************************************************************/
/** @file   EXTBUZZER.c
 *  @brief
 *
 */
/******************************************************************************
 * Includes
 *******************************************************************************/
#include "EXTBUZZER.h"
#include "EXTBUZZER_config.h"
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
static pwm_config_s sEXTBUZZER =
	{
		.eChannel = (pwm_channel_e) EXB_BUZZER_CHANNEL,
		.bDuty = EXB_BUZZER_DUTY,
		.wFrequency = EXB_BUZZER_FREQ,
	};

static eDEVError_s EXB_eChangeDuty (uint32_t wRequest, void * vpValue);
static eDEVError_s EXB_eChangeFrequency (uint32_t wRequest, void * vpValue);
static eDEVError_s EXB_eChangeStatus (uint32_t wRequest, void * vpValue);
static eDEVError_s EXB_eReadConfig (uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction EXB_pIOCTLFunction[] =  //!< IOCTL array of function mapping
	{
	IOCTL_EXTBUZZER
	};
#undef X

static eDEVError_s EXB_eChangeDuty (uint32_t wRequest, void * vpValue)
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

	sEXTBUZZER.bDuty = *(uint8_t*)vpValue;
	PWM_vConfigDuty(&sEXTBUZZER);
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s EXB_eChangeFrequency (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	sEXTBUZZER.wFrequency = *(uint32_t*)vpValue;
	PWM_vConfigFrequency(&sEXTBUZZER);
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s EXB_eChangeStatus (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	if (*(bool*)vpValue == true)
	{
		PWM_vEnable(&sEXTBUZZER);
	}
	else
	{
		PWM_vDisable(&sEXTBUZZER);
	}
	return DEV_ERROR_SUCCESS;
}

static eDEVError_s EXB_eReadConfig (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	if (wRequest == IOCTL_EXB_READ_FREQ)
	{
		*(uint32_t*)vpValue = PWM_sReadConfig(sEXTBUZZER.eChannel).wFrequency;
	}
	else if (wRequest == IOCTL_EXB_READ_DUTY)
	{
		*(uint8_t*)vpValue = PWM_sReadConfig(sEXTBUZZER.eChannel).bDuty;
	}
	else
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	return DEV_ERROR_SUCCESS;
}

eDEVError_s EXB_open (void)
{
	sEXTBUZZER.eChannel = (pwm_channel_e)EXB_BUZZER_CHANNEL;
	sEXTBUZZER.bDuty = EXB_BUZZER_DUTY;
	sEXTBUZZER.wFrequency = EXB_BUZZER_FREQ;

	eDEVError_s eError = (eDEVError_s)PWM_bInit(&sEXTBUZZER); //Init PWM

	return eError;
}

uint32_t EXB_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	return 0;
}

uint32_t EXB_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	return 0; //Nothing to write
}

eDEVError_s EXB_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if ((wRequest >= IOCTL_EXB_INVALID) || (vpValue == NULL))
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	eDEVError_s eError;

	//Configure
	eError = EXB_pIOCTLFunction[wRequest](wRequest, vpValue);

	return eError;
}

eDEVError_s EXB_close (struct peripheral_descriptor_s* const this)
{
	PWM_vDisable(&sEXTBUZZER);
	return DEV_ERROR_SUCCESS;
}
