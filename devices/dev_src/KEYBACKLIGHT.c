/****************************************************************************
* Title                 :   KEYBACKLIGHT Include File
* Filename              :   KEYBACKLIGHT.c
* Author                :   thiago.palmieri
* Origin Date           :   17 de mar de 2016
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
*  17 de mar de 2016   1.0.0   thiago.palmieri KEYBACKLIGHT include file Created.
*
*****************************************************************************/
/** @file KEYBACKLIGHT.c
 *  @brief This file provides Keypad Backlight Control
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "KEYBACKLIGHT.h"
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
  PWM_CHANNEL1,           //!< PWM Channel 1
  PWM_CHANNEL2,           //!< PWM Channel 2
  PWM_FAULT               //!< Invalid PWM Channel
} pwm_channel_e;

/**
 * This Struct holds PWM configuration
 */
typedef struct pwm_config_s
{
  pwm_channel_e eChannel; //!< PWM Channel
  uint8_t bDuty;          //!< PWM Duty Cycle
  uint32_t wFrequency;    //!< PWM Frequency
} pwm_config_s;

#endif
typedef eDEVError_s (*fpIOCTLFunction)(uint32_t wRequest, void * vpValue); //!< IOCTL function pointer

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
/**
 * Default Sensor Initialization Values
 */
static pwm_config_s sKEYBACK =
    {
        .eChannel = KBL_DEFAULT_CHANNEL,    //!< PWM Channel
        .bDuty = KBL_DEFAULT_DUTY,          //!< PWM Duty Cycle
        .wFrequency = KBL_DEFAULT_FREQ,     //!< PWM Frequency
    };
/******************************************************************************
* Function Prototypes
*******************************************************************************/
static eDEVError_s KBL_eChangeStatus(uint32_t wRequest, void * vpValue);
static eDEVError_s KBL_eChangeFrequency(uint32_t wRequest, void * vpValue);
static eDEVError_s KBL_eChangeDuty(uint32_t wRequest, void * vpValue);
static eDEVError_s KBL_eReadConfig(uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction KBL_pIOCTLFunction[] =  //!< IOCTL array of function mapping
{
    IOCTL_KEYBACKLIGHT
};
#undef X
/******************************************************************************
* Function Definitions
*******************************************************************************/
#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);
void PWM_vConfigDuty2(pwm_config_s * pPWM)
{
  sKEYBACK.bDuty = pPWM->bDuty;
}

void PWM_vConfigFrequency2(pwm_config_s * pPWM)
{
  sKEYBACK.wFrequency = pPWM->wFrequency;
}

eDEVError_s PWM_bInit2(pwm_config_s *pPWM)
{
  (void)pPWM;
  return DEV_ERROR_SUCCESS;
}

void PWM_vEnable2(pwm_config_s *pPWM){(void)pPWM;}

void PWM_vDisable2(pwm_config_s *pPWM){(void)pPWM;}

pwm_config_s PWM_sReadConfig2(const pwm_channel_e eChannel)
{
  (void)eChannel;
  pwm_config_s result;
  result.bDuty = 55;
  result.wFrequency = 2500;
  result.eChannel = KBL_DEFAULT_CHANNEL;
  return result;
}

#define PWM_vConfigDuty PWM_vConfigDuty2
#define PWM_vConfigFrequency PWM_vConfigFrequency2
#define PWM_bInit PWM_bInit2
#define PWM_vEnable PWM_vEnable2
#define PWM_vDisable PWM_vDisable2
#define PWM_sReadConfig PWM_sReadConfig2

void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
* Function : KBL_eChangeStatus(uint32_t wRequest, void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to Turn ON or OFF the backlight. Any non-zero value will activate the PWM, a Zero
* will deactivate it. This function must be used by KBL_ioctl ONLY !
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
* if (!KBL_eChangeStatus(0, vpValue)) PRINTF("Status changed");
~~~~~~~~~~~~~~~
*
* @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s KBL_eChangeStatus(uint32_t wRequest, void * vpValue)
{
  (void)wRequest;
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  if (*(bool*)vpValue == true)
    {
      PWM_vEnable(&sKEYBACK);
    }
  else
    {
      PWM_vDisable(&sKEYBACK);
    }
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : KBL_eChangeFrequency(uint32_t wRequest, void * vpValue)
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
* if (!KBL_eChangeFrequency(0, vpValue)) PRINTF("Frequency may have changed");
~~~~~~~~~~~~~~~
*
* @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s KBL_eChangeFrequency(uint32_t wRequest, void * vpValue)
{
  (void)wRequest;
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }

  sKEYBACK.wFrequency = *(uint32_t*)vpValue;
  PWM_vConfigFrequency(&sKEYBACK);
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : KBL_eChangeDuty(uint32_t wRequest, void * vpValue)
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
* if (!KBL_eChangeDuty(0, vpValue)) PRINTF("Duty changed");
~~~~~~~~~~~~~~~
*
* @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s KBL_eChangeDuty(uint32_t wRequest, void * vpValue)
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
  sKEYBACK.bDuty = *(uint8_t*)vpValue;
  PWM_vConfigDuty(&sKEYBACK);

  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : KBL_eReadConfig(uint32_t wRequest, void * vpValue)
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
* if (!KBL_eReadConfig(IOCTL_KBL_READ_FREQ, vpValue)) PRINTF("Frequency %d", *(uint32_t*)vpValue);
~~~~~~~~~~~~~~~
*
* @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s KBL_eReadConfig(uint32_t wRequest, void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  if (wRequest == IOCTL_KBL_READ_FREQ)
    {
      *(uint32_t*)vpValue = PWM_sReadConfig(sKEYBACK.eChannel).wFrequency;
    }
  else if (wRequest == IOCTL_KBL_READ_DUTY)
    {
      *(uint8_t*)vpValue = PWM_sReadConfig(sKEYBACK.eChannel).bDuty;
    }
  else
    {
      return DEV_ERROR_INVALID_IOCTL;
    }

  return DEV_ERROR_SUCCESS;
}

eDEVError_s KBL_open(void)
{
  sKEYBACK.bDuty = KBL_DEFAULT_DUTY;
  sKEYBACK.eChannel = KBL_DEFAULT_CHANNEL;
  sKEYBACK.wFrequency = KBL_DEFAULT_FREQ;

  eDEVError_s eError = (eDEVError_s)PWM_bInit(&sKEYBACK); //Init LCD
  PWM_vEnable(&sKEYBACK);

  return eError;
}

uint32_t KBL_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize)
{
  (void)vpBuffer;
  (void)tBufferSize;
  return 0;  //Nothing to read
}

uint32_t KBL_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize)
{
  (void)vpBuffer;
  (void)tBufferSize;
  return 0; //Nothing to write
}

eDEVError_s KBL_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue)
{
  (void)this;
  if ((wRequest >= IOCTL_KBL_INVALID) || (vpValue == NULL))
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  eDEVError_s eError;

  //Configure
  eError = KBL_pIOCTLFunction[wRequest](wRequest, vpValue);

  return eError;
}

eDEVError_s KBL_close(struct peripheral_descriptor_s* const this)
{
  (void)this;
  PWM_vDisable(&sKEYBACK);
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Unity Testing
*******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(KEYBACKLIGHTDeviceTest);

TEST_SETUP(KEYBACKLIGHTDeviceTest)
{
}

TEST_TEAR_DOWN(KEYBACKLIGHTDeviceTest)
{
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_eChangeStatus)
{
  uint32_t request = IOCTL_KBL_SET_FREQ; //Any value
  uint8_t value = 10; //Any value different than zero
  //Test NULL pointer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_eChangeStatus(request, NULL));
  //Test True
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eChangeStatus(request, &value));
  //Test false
  value = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eChangeStatus(request, &value));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_eChangeFrequency)
{
  uint32_t request = IOCTL_KBL_SET_FREQ; //Any value
  uint32_t value = 1000; //Any value different than zero
  //Test NULL pointer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_eChangeFrequency(request, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eChangeFrequency(request, &value));
  TEST_ASSERT_EQUAL(value, sKEYBACK.wFrequency);
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_eChangeDuty)
{
  uint32_t request = IOCTL_KBL_SET_FREQ; //Any value
  uint8_t value = 20; //Any value different than zero
  //Test NULL pointer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_eChangeDuty(request, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eChangeDuty(request, &value));
  TEST_ASSERT_EQUAL(value, sKEYBACK.bDuty);

  value = 200;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eChangeDuty(request, &value));
  TEST_ASSERT_EQUAL(100, sKEYBACK.bDuty);
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_eReadConfig)
{
  sKEYBACK.bDuty = 55;
  uint32_t request = IOCTL_KBL_SET_FREQ; //Any value
  uint8_t value = 20; //Any value different than zero
  //Test NULL pointer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_eReadConfig(request, NULL));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_eReadConfig(request, &value));

  request = IOCTL_KBL_READ_DUTY;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eReadConfig(request, &value));
  TEST_ASSERT_EQUAL(sKEYBACK.bDuty, value);

  sKEYBACK.wFrequency = 2500;
  uint32_t wValue = 0;
  request = IOCTL_KBL_READ_FREQ;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_eReadConfig(request, &wValue));
  TEST_ASSERT_EQUAL(sKEYBACK.wFrequency, wValue);
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_KBL_open)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_open());
  TEST_ASSERT_EQUAL(KBL_DEFAULT_CHANNEL, sKEYBACK.eChannel);
  TEST_ASSERT_EQUAL(KBL_DEFAULT_DUTY, sKEYBACK.bDuty);
  TEST_ASSERT_EQUAL(KBL_DEFAULT_FREQ, sKEYBACK.wFrequency);

}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_KBL_read)
{
  uint8_t data;
  TEST_ASSERT_EQUAL(0, KBL_read(NULL, NULL, 0));
  TEST_ASSERT_EQUAL(0, KBL_read(NULL, NULL, 10));
  TEST_ASSERT_EQUAL(0, KBL_read(NULL, &data, 1));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_KBL_write)
{
  uint8_t data = 1;
  TEST_ASSERT_EQUAL(0, KBL_write(NULL, NULL, 0));
  TEST_ASSERT_EQUAL(0, KBL_write(NULL, NULL, 10));
  TEST_ASSERT_EQUAL(0, KBL_write(NULL, &data, 1));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_KBL_close)
{
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_close(NULL));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_KBL_ioctl)
{
  uint32_t value = 0;
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_ioctl(NULL, IOCTL_KBL_INVALID, &value));
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, KBL_ioctl(NULL, IOCTL_KBL_READ_DUTY, NULL));

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_ioctl(NULL, IOCTL_KBL_READ_DUTY, &value));
  TEST_ASSERT_EQUAL(55, (uint8_t)value);

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_ioctl(NULL, IOCTL_KBL_READ_FREQ, &value));
  TEST_ASSERT_EQUAL(2500, (uint32_t)value);

  value = 35;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_ioctl(NULL, IOCTL_KBL_SET_DUTY, &value));
  TEST_ASSERT_EQUAL(sKEYBACK.bDuty, (uint8_t)value);

  value = 1500;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_ioctl(NULL, IOCTL_KBL_SET_FREQ, &value));
  TEST_ASSERT_EQUAL(sKEYBACK.wFrequency, (uint32_t)value);

  value = true;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_ioctl(NULL, IOCTL_KBL_TURN_ON_OFF, &value));

  value = false;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, KBL_ioctl(NULL, IOCTL_KBL_TURN_ON_OFF, &value));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_DEV_open)
{
  TEST_ASSERT_NULL(DEV_open(PERIPHERAL_INVALID));
  peripheral_descriptor_p KBLDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
  TEST_ASSERT_NOT_NULL(KBLDevice);
  //Try to reopen
  TEST_ASSERT_NULL(DEV_open(PERIPHERAL_LCDBACKLIGHT));
  DEV_close(KBLDevice);
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_DEV_close)
{
  //Try to close NULL
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(NULL));
  peripheral_descriptor_p KBLDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
  TEST_ASSERT_NOT_NULL(KBLDevice);
  //Try to close
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(KBLDevice));
  //Try to re-close
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(KBLDevice));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_DEV_read)
{
  uint8_t buffer = 0;

  TEST_ASSERT_EQUAL(0, DEV_read(NULL, NULL, 0));
  peripheral_descriptor_p KBLDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
  TEST_ASSERT_NOT_NULL(KBLDevice);

  TEST_ASSERT_EQUAL(0, DEV_read(KBLDevice, NULL, 0));

  TEST_ASSERT_EQUAL(0, DEV_read(KBLDevice, &buffer, sizeof(buffer)));

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(KBLDevice));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_DEV_write)
{
  //Try to write NULL
  TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
  peripheral_descriptor_p KBLDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
  TEST_ASSERT_NOT_NULL(KBLDevice);

  uint32_t buffer = 1234;
  //Try to close
  TEST_ASSERT_EQUAL(0, DEV_write(KBLDevice, &buffer, 1));
  //Try to write NULL
  TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
  //Try to close
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(KBLDevice));
}

TEST(KEYBACKLIGHTDeviceTest, test_Verify_DEV_ioctl)
{
  uint32_t value = 0;

  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_ioctl(NULL, 0, NULL));
  peripheral_descriptor_p KBLDevice = DEV_open(PERIPHERAL_LCDBACKLIGHT);
  //NULL Buffer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(KBLDevice, IOCTL_KBL_READ_FREQ, NULL));
  //Invalid Request
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(KBLDevice, IOCTL_KBL_INVALID, &value));

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(KBLDevice, IOCTL_KBL_READ_DUTY, &value));
  TEST_ASSERT_EQUAL(55, (uint8_t)value);

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(KBLDevice, IOCTL_KBL_READ_FREQ, &value));
  TEST_ASSERT_EQUAL(2500, (uint32_t)value);

  value = 35;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(KBLDevice, IOCTL_KBL_SET_DUTY, &value));
  TEST_ASSERT_EQUAL(sKEYBACK.bDuty, (uint8_t)value);

  value = 1500;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(KBLDevice, IOCTL_KBL_SET_FREQ, &value));
  TEST_ASSERT_EQUAL(sKEYBACK.wFrequency, (uint32_t)value);

  value = true;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(KBLDevice, IOCTL_KBL_TURN_ON_OFF, &value));

  value = false;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(KBLDevice, IOCTL_KBL_TURN_ON_OFF, &value));

  DEV_close(KBLDevice);
}
#endif
