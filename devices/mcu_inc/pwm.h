/****************************************************************************
* Title                 :   PWM HAL
* Filename              :   pwm.h
* Author                :   Thiago Palmieri
* Origin Date           :   03/02/2016
* Version               :   1.0.0
* Compiler              :   GCC 5.2 2015q4
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
*  03/02/16   1.0.0   Thiago Palmieri PWM HAL Created.
*
*****************************************************************************/
/** @file pwm.h
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of PWM functions.
 *
 *  In order to initialize a PWM channel, a Channel, Frequency and Duty cycle must
 *  be provided during initialization. The Frequency is MCU limited, and will return error
 *  if violated, a frequency of 0 is also seen as an error.
 *  The Duty cycle accepts variation from 0 to 100 (in percent), if a higher
 *  value is provided, the initialization will modify it to 100.
 *
 *  After initialization the PWM may be enabled and disabled at any time, the Duty and frequency
 *  may also be changed as needed using the respective functions.
 *
 *  The ReadConfig function is provided to read a specific PWM setting at any given time.
 */

#ifndef ABS_INC_PWM_H_
#define ABS_INC_PWM_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <inttypes.h>
#include "mcuerror.h"

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/
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

/******************************************************************************
* Variables
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
* Function : PWM_bInit(pwm_config_s *pPWM)
*//**
* \b Description:
*
* This is a public function used to initialize a PWM channel.
* A pwm_config_s struct pointer must be passed to the function. The struct
* must be initialized with PWM channel to be used, its frequency and duty
* cycle. If frequency is not provided or the wrong channel is used,
* the function will return error.
*
* PRE-CONDITION: Valid PWM Channel and Frequency larger than zero.
*
* POST-CONDITION: PWM Channel initialized and ready to be activated.
*
* @return     eMCUError_s see mcuerror.h
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   pwm_config_s sChannelZero;
*   sChannelZero.eChannel = PWM_CHANNEL0;
*   sChannelZero.wFrequency = 5000; //in Hz
*
*   PWM_bInit(&sChannelZero);
~~~~~~~~~~~~~~~
*
* @see PWM_vConfigDuty, PWM_vConfigFrequency, PWM_sReadConfig, PWM_vEnable and PWM_vDisable
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eMCUError_s PWM_bInit(pwm_config_s * pPWM);

/******************************************************************************
* Function : PWM_vConfigDuty(const pwm_config_s * pPWM)
*//**
* \b Description:
*
* This is a public function used to configure a PWM Duty Cycle.
* A previously initialized pwm_config_s struct pointer must be passed
* to the function. If the Channel was not initialized, the function will not
* perform the duty change. If a duty value larger than 100 is provided, the
* function will change it to 100. Duty values are in %. (from 0% to 100%).
*
* PRE-CONDITION: PWM Channel initialized.
*
* POST-CONDITION: Duty Cycle changed.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   pwm_config_s sChannelZero;
*   sChannelZero.eChannel = PWM_CHANNEL0;
*   sChannelZero.wFrequency = 5000; //in Hz
*
*   PWM_bInit(&sChannelZero);
*   PWM_vConfigDuty(&sChannelZero, 50); //Duty in %
~~~~~~~~~~~~~~~
*
* @see PWM_bInit, PWM_vConfigFrequency, PWM_sReadConfig, PWM_vEnable and PWM_vDisable
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void PWM_vConfigDuty(const pwm_config_s * pPWM);

/******************************************************************************
* Function : PWM_vConfigFrequency(const pwm_config_s * pPWM)
*//**
* \b Description:
*
* This is a public function used to configure a PWM Frequency.
* A previously initialized pwm_config_s struct pointer must be passed
* to the function. If the Channel was not initialized, the function will not
* perform the frequency change. If a frequency value equal to zero is provided,
* the function will not perform the change.
*
* PRE-CONDITION: PWM Channel initialized.
*
* POST-CONDITION: Frequency changed.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   pwm_config_s sChannelZero;
*   sChannelZero.eChannel = PWM_CHANNEL0;
*   sChannelZero.wFrequency = 5000; //in Hz
*
*   PWM_bInit(&sChannelZero);
*   PWM_vConfigFrequency(&sChannelZero, 4000); //Frequency in Hz
~~~~~~~~~~~~~~~
*
* @see PWM_bInit, PWM_vConfigDuty, PWM_sReadConfig, PWM_vEnable and PWM_vDisable
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void PWM_vConfigFrequency(const pwm_config_s * pPWM);

/******************************************************************************
* Function : PWM_vEnable(pwm_config_s * pPWM)
*//**
* \b Description:
*
* This is a public function used to Enable a PWM Channel.
* A previously initialized pwm_config_s struct pointer must be passed
* to the function. If the Channel was not initialized, the function will not
* enable the channel.
*
* PRE-CONDITION: PWM Channel initialized.
*
* POST-CONDITION: PWM Channel enabled.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   pwm_config_s sChannelZero;
*   sChannelZero.eChannel = PWM_CHANNEL0;
*   sChannelZero.wFrequency = 5000; //in Hz
*
*   PWM_bInit(&sChannelZero);
*   PWM_vConfigFrequency(&sChannelZero, 4000); //Frequency in Hz
*   PWM_vEnable(&sChannelZero);
~~~~~~~~~~~~~~~
*
* @see PWM_bInit, PWM_vConfigDuty, PWM_vConfigFrequency, PWM_sReadConfig and PWM_vDisable
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void PWM_vEnable(const pwm_config_s * pPWM);

/******************************************************************************
* Function : PWM_vDisable(pwm_config_s * pPWM)
*//**
* \b Description:
*
* This is a public function used to Disable a PWM Channel.
* A previously initialized pwm_config_s struct pointer must be passed
* to the function.
*
* PRE-CONDITION: PWM Channel initialized.
*
* POST-CONDITION: PWM Channel disabled.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   pwm_config_s sChannelZero;
*   sChannelZero.eChannel = PWM_CHANNEL0;
*   sChannelZero.wFrequency = 5000; //in Hz
*
*   PWM_bInit(&sChannelZero);
*   PWM_vConfigFrequency(&sChannelZero, 4000); //Frequency in Hz
*   PWM_vEnable(&sChannelZero);   //PWM activated
*   PWM_vDisable(&sChannelZero);  //PWM deactivated
~~~~~~~~~~~~~~~
*
* @see PWM_bInit, PWM_vConfigDuty, PWM_vConfigFrequency, PWM_sReadConfig and PWM_vEnable
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void PWM_vDisable(const pwm_config_s * pPWM);

/******************************************************************************
* Function : PWM_sReadConfig(pwm_channel_e eChannel)
*//**
* \b Description:
*
* This is a public function used to read a PWM Channel configuration.
* A PWM channel must be passed to the function as parameter, the return is
* a PWM channel configuration structure. If the channel is not initialized
* the return structure will be all zeroes. Since Duty and Channel values can
* be Zero but the frequency not when the channel is initialized, it is
* recommended to verify the Frequency, if it is zero, then the channel is
* not initialized.
*
* PRE-CONDITION: PWM Channel initialized.
*
* POST-CONDITION: PWM Channel structure configuration.
*
* @return     pwm_config_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   pwm_config_s sChannelZero;
*   sChannelZero.eChannel = PWM_CHANNEL0;
*   sChannelZero.wFrequency = 5000; //in Hz
*
*   PWM_bInit(&sChannelZero);
*   PWM_vConfigFrequency(&sChannelZero, 4000); //Frequency in Hz
*   PWM_vEnable(&sChannelZero);   //PWM activated
*   PWM_vDisable(&sChannelZero);  //PWM deactivated
*
*   if (!PWM_sReadConfig(PWM_CHANNEL0).wFrequency) PRINTF("Channel not initialized");
*   else PRINTF("Channel Initialized");
~~~~~~~~~~~~~~~
*
* @see PWM_bInit, PWM_vConfigDuty, PWM_vConfigFrequency, PWM_vDisable and PWM_vEnable
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 03/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
pwm_config_s PWM_sReadConfig(const pwm_channel_e eChannel);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ABS_INC_PWM_H_ */
