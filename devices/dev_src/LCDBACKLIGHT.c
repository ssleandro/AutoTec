/****************************************************************************
 * Title                 :   LCDBACKLIGHT Include File
 * Filename              :   LCDBACKLIGHT.c
 * Author                :   thiago.palmieri
 * Origin Date           :   16 de mar de 2016
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
 *  16 de mar de 2016   1.0.0   thiago.palmieri LCDBACKLIGHT include file Created.
 *
 *****************************************************************************/
/** @file LCDBACKLIGHT.c
 *  @brief This file provides LCD Backlight Control
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "LCDBACKLIGHT.h"
#include <stddef.h>
#include <stdbool.h>
#ifndef UNITY_TEST
#include "mculib.h"
#else
#include "unity_fixture.h"
#include <stdint.h>
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
#if defined (UNITY_TEST)

/**
 * This enumeration is a list of available PWM channels
 */
typedef enum pwm_channel_e
{
	PWM_CHANNEL0,           //!< PWM Channel 0
	PWM_CHANNEL1,//!< PWM Channel 1
	PWM_CHANNEL2,//!< PWM Channel 2
	PWM_FAULT//!< Invalid PWM Channel
}pwm_channel_e;

/**
 * This Struct holds PWM configuration
 */
typedef struct pwm_config_s
{
	pwm_channel_e eChannel; //!< PWM Channel
	uint8_t bDuty;//!< PWM Duty Cycle
	uint32_t wFrequency;//!< PWM Frequency
}pwm_config_s;

#endif
typedef eDEVError_s (*fpIOCTLFunction) (uint32_t wRequest, void * vpValue); //!< IOCTL function pointer

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
/**
 * Default Sensor Initialization Values
 */
static pwm_config_s sLCDBACK =
	{
		.eChannel = LBK_DEFAULT_CHANNEL,    //!< PWM Channel
		.bDuty = LBK_DEFAULT_DUTY,          //!< PWM Duty Cycle
		.wFrequency = LBK_DEFAULT_FREQ,     //!< PWM Frequency
	};
/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static eDEVError_s LBK_eChangeStatus (uint32_t wRequest, void * vpValue);
static eDEVError_s LBK_eChangeFrequency (uint32_t wRequest, void * vpValue);
static eDEVError_s LBK_eChangeDuty (uint32_t wRequest, void * vpValue);
static eDEVError_s LBK_eReadConfig (uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction LBK_pIOCTLFunction[] =  //!< IOCTL array of function mapping
	{
	IOCTL_LCDBACKLIGHT
	};
#undef X
/******************************************************************************
 * Function Definitions
 *******************************************************************************/
#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);
void PWM_vConfigDuty1(pwm_config_s * pPWM)
{
	sLCDBACK.bDuty = pPWM->bDuty;
}

void PWM_vConfigFrequency1(pwm_config_s * pPWM)
{
	sLCDBACK.wFrequency = pPWM->wFrequency;
}

eDEVError_s PWM_bInit1(pwm_config_s *pPWM)
{
	(void)pPWM;
	return DEV_ERROR_SUCCESS;
}

void PWM_vEnable1(pwm_config_s *pPWM)
{}

void PWM_vDisable1(pwm_config_s *pPWM)
{}

pwm_config_s PWM_sReadConfig1(const pwm_channel_e eChannel)
{
	pwm_config_s result;
	result.bDuty = 55;
	result.wFrequency = 2500;
	result.eChannel = LBK_DEFAULT_CHANNEL;
	return result;
}

#define PWM_vConfigDuty PWM_vConfigDuty1
#define PWM_vConfigFrequency PWM_vConfigFrequency1
#define PWM_bInit PWM_bInit1
#define PWM_vEnable PWM_vEnable1
#define PWM_vDisable PWM_vDisable1
#define PWM_sReadConfig PWM_sReadConfig1

void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
 * Function : LBK_eChangeStatus(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to Turn ON or OFF the backlight. Any non-zero value will activate the PWM, a Zero
 * will deactivate it. This function must be used by MLX_ioctl ONLY !
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *
 * if (!LBK_eChangeStatus(0, vpValue)) PRINTF("Status changed");
 ~~~~~~~~~~~~~~~
 *
 * @see LBK_open, LBK_write, LBK_read, LBK_ioctl and LBK_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static eDEVError_s LBK_eChangeStatus (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	if (*(bool*)vpValue == true)
	{
		PWM_vEnable(&sLCDBACK);
	}
	else
	{
		PWM_vDisable(&sLCDBACK);
	}
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : LBK_eChangeFrequency(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to change PWM frequency. Since the hardware device may be frequency limited,
 * executing this function is not assured that the change was performed.
 * If the user needs to know if the frequency changed, then a IOCTL read must be
 * performed.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *
 * if (!LBK_eChangeFrequency(0, vpValue)) PRINTF("Frequency may have changed");
 ~~~~~~~~~~~~~~~
 *
 * @see LBK_open, LBK_write, LBK_read, LBK_ioctl and LBK_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static eDEVError_s LBK_eChangeFrequency (uint32_t wRequest, void * vpValue)
{
	(void)wRequest;
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	sLCDBACK.wFrequency = *(uint32_t*)vpValue;
	PWM_vConfigFrequency(&sLCDBACK);
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : LBK_eChangeDuty(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to change PWM duty. The duty ranges from 0 - 100%, any value larger than that
 * will be changed to 100.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *
 * if (!LBK_eChangeDuty(0, vpValue)) PRINTF("Duty changed");
 ~~~~~~~~~~~~~~~
 *
 * @see LBK_open, LBK_write, LBK_read, LBK_ioctl and LBK_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static eDEVError_s LBK_eChangeDuty (uint32_t wRequest, void * vpValue)
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
	sLCDBACK.bDuty = *(uint8_t*)vpValue;
	PWM_vConfigDuty(&sLCDBACK);

	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Function : LBK_eReadConfig(uint32_t wRequest, void * vpValue)
 *//**
 * \b Description:
 *
 * This is a private function representing a IOCTL request, in this case the request
 * to read a specific PWM configuration.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *
 * if (!LBK_eReadConfig(IOCTL_LBK_READ_FREQ, vpValue)) PRINTF("Frequency %d", *(uint32_t*)vpValue);
 ~~~~~~~~~~~~~~~
 *
 * @see LBK_open, LBK_write, LBK_read, LBK_ioctl and LBK_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 16/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static eDEVError_s LBK_eReadConfig (uint32_t wRequest, void * vpValue)
{
	if (vpValue == NULL)
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	if (wRequest == IOCTL_LBK_READ_FREQ)
	{
		*(uint32_t*)vpValue = PWM_sReadConfig(sLCDBACK.eChannel).wFrequency;
	}
	else if (wRequest == IOCTL_LBK_READ_DUTY)
	{
		*(uint8_t*)vpValue = PWM_sReadConfig(sLCDBACK.eChannel).bDuty;
	}
	else
	{
		return DEV_ERROR_INVALID_IOCTL;
	}

	return DEV_ERROR_SUCCESS;
}

eDEVError_s LBK_open (void)
{
	sLCDBACK.bDuty = LBK_DEFAULT_DUTY;
	sLCDBACK.eChannel = LBK_DEFAULT_CHANNEL;
	sLCDBACK.wFrequency = LBK_DEFAULT_FREQ;

	eDEVError_s eError = (eDEVError_s)PWM_bInit(&sLCDBACK); //Init LCD
	PWM_vEnable(&sLCDBACK);

	return eError;
}

uint32_t LBK_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{

	return 0;  //Nothing to read
}

uint32_t LBK_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	return 0; //Nothing to write
}

eDEVError_s LBK_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	if ((wRequest >= IOCTL_LBK_INVALID) || (vpValue == NULL))
	{
		return DEV_ERROR_INVALID_IOCTL;
	}
	eDEVError_s eError;

	//Configure
	eError = LBK_pIOCTLFunction[wRequest](wRequest, vpValue);

	return eError;
}

eDEVError_s LBK_close (struct peripheral_descriptor_s* const this)
{
	PWM_vDisable(&sLCDBACK);
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
 * Unity Testing
 *******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(LCDBACKLIGHTDeviceTest);

TEST_SETUP(LCDBACKLIGHTDeviceTest)
{
}

TEST_TEAR_DOWN(LCDBACKLIGHTDeviceTest)
{
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_eChangeStatus)
{
	uint32_t request = IOCTL_LBK_SET_FREQ; //Any value
	uint8_t value = 10;//Any value different than zero
	//Test NULL pointer
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_eChangeStatus(request, NULL));
	//Test True
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eChangeStatus(request, &value));
	//Test false
	value = 0;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eChangeStatus(request, &value));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_eChangeFrequency)
{
	uint32_t request = IOCTL_LBK_SET_FREQ; //Any value
	uint32_t value = 1000;//Any value different than zero
	//Test NULL pointer
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_eChangeFrequency(request, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eChangeFrequency(request, &value));
	TEST_ASSERT_EQUAL(value, sLCDBACK.wFrequency);
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_eChangeDuty)
{
	uint32_t request = IOCTL_LBK_SET_FREQ; //Any value
	uint8_t value = 20;//Any value different than zero
	//Test NULL pointer
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_eChangeDuty(request, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eChangeDuty(request, &value));
	TEST_ASSERT_EQUAL(value, sLCDBACK.bDuty);

	value = 200;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eChangeDuty(request, &value));
	TEST_ASSERT_EQUAL(100, sLCDBACK.bDuty);
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_eReadConfig)
{
	sLCDBACK.bDuty = 55;
	uint32_t request = IOCTL_LBK_SET_FREQ; //Any value
	uint8_t value = 20;//Any value different than zero
	//Test NULL pointer
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_eReadConfig(request, NULL));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_eReadConfig(request, &value));

	request = IOCTL_LBK_READ_DUTY;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eReadConfig(request, &value));
	TEST_ASSERT_EQUAL(sLCDBACK.bDuty, value);

	sLCDBACK.wFrequency = 2500;
	uint32_t wValue = 0;
	request = IOCTL_LBK_READ_FREQ;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_eReadConfig(request, &wValue));
	TEST_ASSERT_EQUAL(sLCDBACK.wFrequency, wValue);
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_LBK_open)
{
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_open());
	TEST_ASSERT_EQUAL(LBK_DEFAULT_CHANNEL, sLCDBACK.eChannel);
	TEST_ASSERT_EQUAL(LBK_DEFAULT_DUTY, sLCDBACK.bDuty);
	TEST_ASSERT_EQUAL(LBK_DEFAULT_FREQ, sLCDBACK.wFrequency);

}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_LBK_read)
{
	uint8_t data;
	TEST_ASSERT_EQUAL(0, LBK_read(NULL, NULL, 0));
	TEST_ASSERT_EQUAL(0, LBK_read(NULL, NULL, 10));
	TEST_ASSERT_EQUAL(0, LBK_read(NULL, &data, 1));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_LBK_write)
{
	uint8_t data = 1;
	TEST_ASSERT_EQUAL(0, LBK_write(NULL, NULL, 0));
	TEST_ASSERT_EQUAL(0, LBK_write(NULL, NULL, 10));
	TEST_ASSERT_EQUAL(0, LBK_write(NULL, &data, 1));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_LBK_close)
{
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_close(NULL));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_LBK_ioctl)
{
	uint32_t value = 0;
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_ioctl(NULL, IOCTL_LBK_INVALID, &value));
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, LBK_ioctl(NULL, IOCTL_LBK_READ_DUTY, NULL));

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_ioctl(NULL, IOCTL_LBK_READ_DUTY, &value));
	TEST_ASSERT_EQUAL(55, (uint8_t)value);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_ioctl(NULL, IOCTL_LBK_READ_FREQ, &value));
	TEST_ASSERT_EQUAL(2500, (uint32_t)value);

	value = 35;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_ioctl(NULL, IOCTL_LBK_SET_DUTY, &value));
	TEST_ASSERT_EQUAL(sLCDBACK.bDuty, (uint8_t)value);

	value = 1500;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_ioctl(NULL, IOCTL_LBK_SET_FREQ, &value));
	TEST_ASSERT_EQUAL(sLCDBACK.wFrequency, (uint32_t)value);

	value = true;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_ioctl(NULL, IOCTL_LBK_TURN_ON_OFF, &value));

	value = false;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, LBK_ioctl(NULL, IOCTL_LBK_TURN_ON_OFF, &value));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_DEV_open)
{
	TEST_ASSERT_NULL(DEV_open(PERIPHERAL_INVALID));
	peripheral_descriptor_p LBKDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
	TEST_ASSERT_NOT_NULL(LBKDevice);
	//Try to reopen
	TEST_ASSERT_NULL(DEV_open(PERIPHERAL_LCDBACKLIGHT));
	DEV_close(LBKDevice);
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_DEV_close)
{
	//Try to close NULL
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(NULL));
	peripheral_descriptor_p LBKDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
	TEST_ASSERT_NOT_NULL(LBKDevice);
	//Try to close
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(LBKDevice));
	//Try to re-close
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(LBKDevice));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_DEV_read)
{
	uint8_t buffer = 0;

	TEST_ASSERT_EQUAL(0, DEV_read(NULL, NULL, 0));
	peripheral_descriptor_p LBKDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
	TEST_ASSERT_NOT_NULL(LBKDevice);

	TEST_ASSERT_EQUAL(0, DEV_read(LBKDevice, NULL, 0));

	TEST_ASSERT_EQUAL(0, DEV_read(LBKDevice, &buffer, sizeof(buffer)));

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(LBKDevice));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_DEV_write)
{
	//Try to write NULL
	TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
	peripheral_descriptor_p LBKDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
	TEST_ASSERT_NOT_NULL(LBKDevice);

	uint32_t buffer = 1234;
	//Try to close
	TEST_ASSERT_EQUAL(0, DEV_write(LBKDevice, &buffer, 1));
	//Try to write NULL
	TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
	//Try to close
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(LBKDevice));
}

TEST(LCDBACKLIGHTDeviceTest, test_Verify_DEV_ioctl)
{
	uint32_t value = 0;

	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_ioctl(NULL, 0, NULL));
	peripheral_descriptor_p LBKDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
	//NULL Buffer
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(LBKDevice, IOCTL_LBK_READ_FREQ, NULL));
	//Invalid Request
	TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(LBKDevice, IOCTL_LBK_INVALID, &value));

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(LBKDevice, IOCTL_LBK_READ_DUTY, &value));
	TEST_ASSERT_EQUAL(55, (uint8_t)value);

	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(LBKDevice, IOCTL_LBK_READ_FREQ, &value));
	TEST_ASSERT_EQUAL(2500, (uint32_t)value);

	value = 35;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(LBKDevice, IOCTL_LBK_SET_DUTY, &value));
	TEST_ASSERT_EQUAL(sLCDBACK.bDuty, (uint8_t)value);

	value = 1500;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(LBKDevice, IOCTL_LBK_SET_FREQ, &value));
	TEST_ASSERT_EQUAL(sLCDBACK.wFrequency, (uint32_t)value);

	value = true;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(LBKDevice, IOCTL_LBK_TURN_ON_OFF, &value));

	value = false;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(LBKDevice, IOCTL_LBK_TURN_ON_OFF, &value));

	DEV_close(LBKDevice);
}
#endif
