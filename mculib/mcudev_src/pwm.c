/****************************************************************************
* Title                 :   PWM HAL
* Filename              :   pwm.c
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
/** @file pwm.c
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the source file for the definition of PWM functions
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include <pwm.h>
#include "board.h"

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define FREQ_MULTIPLIER(x) ((uint32_t)((wClockRate / x) / 2))   //!< Frequency multiplier macro
#define DUTY(x) ((sPwmChannelConfig[pPWM->eChannel].bDutyMultiplier * x)) //!< Duty conversion macro
#define MAX_FREQ (wClockRate / 200)   //!< Maximum allowed frequency macro
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/
/**
 * This Struct holds PWM HAL configuration
 */
typedef struct
{
  MCPWM_CHANNEL_CFG_Type sPwmType;  //!< PWM HAL configuration struct
  en_MCPWM_Channel_Id eChannelId;   //!< PWM channel ID
  bool bStarted;                    //!< Channel Started
  uint8_t bDutyMultiplier;          //!< Duty Multiplier value
} tsPwmConfig;
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static tsPwmConfig sPwmChannelConfig[3]; //!< PWM channel configuration array

static uint32_t wClockRate;              //!< Base Clock Rate for PWM
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/

/******************************************************************************
* Function : PWM_vCommit(tsPwmConfig *pPWM)
*//**
* \b Description:
*
* This is a private function used to commit the PWM changes made by other
* functions.
*
* PRE-CONDITION: PWM Channel must be initialized
*
* POST-CONDITION: Changes performed
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
*   PWM_vCommit(&pwmChannel);
~~~~~~~~~~~~~~~
*
* @see PWM_vConfigDuty and PWM_vConfigFrequency
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
static void PWM_vCommit(tsPwmConfig *pPWM)
{
  MCPWM_WriteToShadow(LPC_MCPWM, pPWM->eChannelId, &pPWM->sPwmType);
}

/******************************************************************************
* Function : PWM_vCheckError(pwm_config_s *pPWM)
*//**
* \b Description:
*
* This is a private function used to verify if any PWM Configuration error
* occurred
*
* PRE-CONDITION: PWM Channel
*
* POST-CONDITION: error code
*
* @return     eMCUError_s see mcuerror.h
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
*   if (!PWM_vCheckError(&pwmChannel)) PRINTF("No Error !");
~~~~~~~~~~~~~~~
*
* @see PWM_bInit
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
static eMCUError_s PWM_vCheckError(const pwm_config_s *pPWM)
{
  // Wrong Channel or channel already started
  if ((pPWM->eChannel >= PWM_FAULT) || (sPwmChannelConfig[pPWM->eChannel].bStarted))
    {
      return MCU_ERROR_PWM_INVALID_CHANNEL;
    }
  return MCU_ERROR_SUCCESS;
}

eMCUError_s PWM_bInit(pwm_config_s *pPWM)
{
  eMCUError_s eErrorCode = PWM_vCheckError(pPWM);

  if (!eErrorCode)
    {
      //Init Board
      wClockRate = BRD_PWMConfig(pPWM->eChannel);

      if((pPWM->wFrequency == 0) || (pPWM->wFrequency > MAX_FREQ))
          {
            return MCU_ERROR_PWM_INVALID_FREQUENCY;
          }

      MCPWM_Init(LPC_MCPWM);
      MCPWM_DCMode(LPC_MCPWM, DISABLE, DISABLE, (0));
      MCPWM_ACMode(LPC_MCPWM, DISABLE);

      // Duty may not be larger than 100%
      if (pPWM->bDuty > 100)
        {
          pPWM->bDuty = 100;
        }
      //204Mhz / 5 = 40.8Mhz (base clock)
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPeriodValue = FREQ_MULTIPLIER(pPWM->wFrequency);     // Hz

      sPwmChannelConfig[pPWM->eChannel].eChannelId = pPWM->eChannel;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelType = MCPWM_CHANNEL_CENTER_MODE;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPolarity = MCPWM_CHANNEL_PASSIVE_HI;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelDeadtimeEnable = DISABLE;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelDeadtimeValue = 0;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelUpdateEnable = ENABLE;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelTimercounterValue = 0;
      sPwmChannelConfig[pPWM->eChannel].bDutyMultiplier = FREQ_MULTIPLIER(pPWM->wFrequency) / 100;
      sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPulsewidthValue = DUTY(pPWM->bDuty);
      sPwmChannelConfig[pPWM->eChannel].bStarted = true;
    }

  MCPWM_ConfigChannel(LPC_MCPWM, pPWM->eChannel, &sPwmChannelConfig[pPWM->eChannel].sPwmType);
  PWM_vCommit(&sPwmChannelConfig[pPWM->eChannel]);

  return eErrorCode;
}

void PWM_vConfigDuty(const pwm_config_s * pPWM)
{
  uint8_t bDuty;
  if (!sPwmChannelConfig[pPWM->eChannel].bStarted)
    {
      return;
    }
  bDuty = pPWM->bDuty;
  if (pPWM->bDuty > 100)
    {
      bDuty = 100;
    }
  //SET DUTY
  sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPulsewidthValue = DUTY(bDuty);
  //COMMIT CHANGES
  PWM_vCommit(&sPwmChannelConfig[pPWM->eChannel]);
}

void PWM_vConfigFrequency(const pwm_config_s * pPWM)
{
  if ((!sPwmChannelConfig[pPWM->eChannel].bStarted) || (pPWM->wFrequency == 0) || (pPWM->wFrequency > MAX_FREQ))
    {
      return;
    }
  uint8_t bDuty = (sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPulsewidthValue / sPwmChannelConfig[pPWM->eChannel].bDutyMultiplier);
  //Update Duty Step
  sPwmChannelConfig[pPWM->eChannel].bDutyMultiplier = FREQ_MULTIPLIER(pPWM->wFrequency) / 100;
  //Update frequency
  sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPeriodValue = FREQ_MULTIPLIER(pPWM->wFrequency);
  //Update Duty
  sPwmChannelConfig[pPWM->eChannel].sPwmType.channelPulsewidthValue = DUTY(bDuty);
  //COMMIT CHANGES
  PWM_vCommit(&sPwmChannelConfig[pPWM->eChannel]);
}

void PWM_vEnable(const pwm_config_s * pPWM)
{
  if (!sPwmChannelConfig[pPWM->eChannel].bStarted)
    {
      return;
    }

  uint32_t val[] = {0, 0, 0};

  if (pPWM->eChannel == 0)
  {
    val[0] = 1;
  } else if (pPWM->eChannel == 1)
  {
    val[1] = 1;
  }else if (pPWM->eChannel == 2)
  {
    val[2] = 1;
  }

  MCPWM_Start(LPC_MCPWM, val[0], val[1], val[2]);
}

void PWM_vDisable(const pwm_config_s * pPWM)
{
  if (!sPwmChannelConfig[pPWM->eChannel].bStarted)
    {
      return;
    }

  uint32_t val[] = {0, 0, 0};

  if (pPWM->eChannel == 0)
  {
    val[0] = 1;
  } else if (pPWM->eChannel == 1)
  {
    val[1] = 1;
  }else if (pPWM->eChannel == 2)
  {
    val[2] = 1;
  }

  MCPWM_Stop(LPC_MCPWM, val[0], val[1], val[2]);
}

pwm_config_s PWM_sReadConfig(const pwm_channel_e eChannel)
{
  pwm_config_s result;
  result.eChannel = eChannel;
  result.bDuty = (sPwmChannelConfig[eChannel].sPwmType.channelPulsewidthValue / sPwmChannelConfig[eChannel].bDutyMultiplier); //((sPwmChannelConfig[pPWM->eChannel].bDutyMultiplier * pPWM->bDuty))
  result.wFrequency = (uint32_t)(wClockRate / (sPwmChannelConfig[eChannel].sPwmType.channelPeriodValue * 2)); //((wClockRate / pPWM->wFrequency) / 2)) * 2 *wClockRate
  return result;
}
