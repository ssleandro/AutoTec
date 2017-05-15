/****************************************************************************
* Title                 :   ADC HAL
* Filename              :   adc.c
* Author                :   Thiago Palmieri
* Origin Date           :   04/02/2016
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
*  04/02/16   1.0.0   Thiago Palmieri ADC HAL Created.
*
*****************************************************************************/
/** @file adc.c
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the source file for the definition of ADC functions
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "adc.h"
#include <stdlib.h>
#include "board.h"
#ifdef USE_SYSVIEW
#include "SEGGER.h"
#include "SEGGER_SYSVIEW.h"
#endif

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define ADC_MAP_REGISTER(x) (sADCMap[x ? ADC1b : ADC0b].pADC)   //!< ADC Register map Macro
#define ADC_MAP_CHANNEL(x) (sADCMap[x ? ADC1b : ADC0b].bADCChannel) //!< ADC Channel map macro
#define ADC_MAP_IRQ(x) (sADCMap[x ? ADC1b : ADC0b].ADCIntIRQ)   //!< ADC IRQ map macro
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/


/******************************************************************************
* Module Typedefs
*******************************************************************************/
/**
 * This private Struct holding the ADC private data
 */
typedef struct adc_private_config_s
{
  volatile uint8_t bDMAChannelNum;  //!< DMA Channel Number from Chip_GPDMA_GetFreeChannel
  volatile uint32_t wDMABuffer;     //!< DMA Buffer to be used when fetching data
           bool bStarted;           //!< DMA Channel is started
}adc_private_config_s;

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static adc_config_s * ADClist[ADC_MAX_CHANNELS]; //!< List of enabled ADC channels for use inside interrupts
/******************************************************************************
* Function Prototypes
*******************************************************************************/
extern void ADC0_IRQHandler(void);
extern void ADC1_IRQHandler(void);
extern void DMA_IRQHandler(void);
/******************************************************************************
* Function Definitions
*******************************************************************************/

/******************************************************************************
* Function : ADC_vStartDMA(const adc_config_s * pADC)
*//**
* \b Description:
*
* This is a private function used to start the ADC in DMA mode
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC DMA sampling started
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   adc_config_s sADCinput;
*   sADCinput.eChannel = ADC0_CH4_P74;
*   sADCinput.wSampleRate = ADC_SAMPLE_400KHZ_MAX;
*   sADCinput.eResolution = ADC_RES_10BITS;
*   sADCinput.bBurstMode = false;
*   sADCinput.eSampleType = ADC_SAMPLE_POOLING;
*
*   ADC_bInit(&sADCinput);  // ADC0 CH4 Initiated
*   ADC_vStartDMA(&sADCinput);
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
static void ADC_vStartDMA(const adc_config_s * pADC)
{
  //Init DMA and configure IRQ
  Chip_GPDMA_Init(LPC_GPDMA);
  NVIC_DisableIRQ(DMA_IRQn);
  NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
  NVIC_EnableIRQ(DMA_IRQn);

  Chip_ADC_Int_SetChannelCmd(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), ENABLE);

  //Get Free Channel for DMA transfer
  adc_private_config_s *pPrivate = pADC->vpPrivateData;
  pPrivate->bDMAChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, sADCMap[pADC->eChannel].wGPDMA);

  if (pADC->bBurstMode)
    {
      Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), ENABLE);
    }
  else
    {
      Chip_ADC_SetStartMode(ADC_MAP_REGISTER(pADC->eChannel), ADC_START_NOW, ADC_TRIGGERMODE_RISING);
    }
  Chip_GPDMA_Transfer(LPC_GPDMA,
                      pPrivate->bDMAChannelNum,
                      sADCMap[pADC->eChannel].wGPDMA,
                      (uint32_t) &pPrivate->wDMABuffer,
                      GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
                      1);
}

/******************************************************************************
* Function : ADC_vStartInterrupt(const adc_config_s * pADC)
*//**
* \b Description:
*
* This is a private function used to start the ADC in interrupt mode
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC interrupt sampling started
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   adc_config_s sADCinput;
*   sADCinput.eChannel = ADC0_CH4_P74;
*   sADCinput.wSampleRate = ADC_SAMPLE_400KHZ_MAX;
*   sADCinput.eResolution = ADC_RES_10BITS;
*   sADCinput.bBurstMode = false;
*   sADCinput.eSampleType = ADC_SAMPLE_POOLING;
*
*   ADC_bInit(&sADCinput);  // ADC0 CH4 Initiated
*   ADC_vStartInterrupt(&sADCinput);
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
static void ADC_vStartInterrupt(const adc_config_s * pADC)
{
  //NVIC_SetPriority(ADC_MAP_IRQ(pADC->eChannel), 5);
  NVIC_EnableIRQ(ADC_MAP_IRQ(pADC->eChannel));
  Chip_ADC_Int_SetChannelCmd(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), ENABLE);
  if (pADC->bBurstMode)
    {
      Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), ENABLE);
    }
  else
    {
      Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), DISABLE);
      Chip_ADC_SetStartMode(ADC_MAP_REGISTER(pADC->eChannel), ADC_START_NOW, ADC_TRIGGERMODE_RISING);
    }

}

/******************************************************************************
* Function : ADC_vStartPooling(const adc_config_s * pADC)
*//**
* \b Description:
*
* This is a private function used to start the ADC pooling mode
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC ready for pooling
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   adc_config_s sADCinput;
*   sADCinput.eChannel = ADC0_CH4_P74;
*   sADCinput.wSampleRate = ADC_SAMPLE_400KHZ_MAX;
*   sADCinput.eResolution = ADC_RES_10BITS;
*   sADCinput.bBurstMode = false;
*   sADCinput.eSampleType = ADC_SAMPLE_POOLING;
*
*   ADC_bInit(&sADCinput);  // ADC0 CH4 Initiated
*   ADC_vStartPooling(&sADCinput);
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
static void ADC_vStartPooling(const adc_config_s * pADC)
{
  if (pADC->bBurstMode)
    {
      Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), ENABLE);
    }
  else
    {
      Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), DISABLE);
      Chip_ADC_SetStartMode(ADC_MAP_REGISTER(pADC->eChannel), ADC_START_NOW, ADC_TRIGGERMODE_RISING);
    }

}

/******************************************************************************
* Function : ADC_vTreatInterruptsISR(uint8_t bADCChannel)
*//**
* \b Description:
*
* This function must be called only from within an Interrupt. It will fetch the
* data and post it on the callback.
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC interrupt sample sent via Callback
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available, IRQ is activated via ADC_vStartInterrupt
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
static void ADC_vTreatInterruptsISR(uint8_t bADCChannel)
{
  uint16_t hADCdata;
  adc_config_s * pADC = NULL;

  // If ADClist position has a valid structure
  if (ADClist[bADCChannel] != NULL)
    {
      pADC = ADClist[bADCChannel];
      if (ADC_SAMPLE_INTERRUPT == pADC->eSampleType)
        {
          Chip_ADC_Int_SetChannelCmd(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), DISABLE);
          Chip_ADC_ReadValue(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), &hADCdata);
          pADC->fpCallBack(hADCdata);
        }
    }
  if (pADC != NULL)
    {
      Chip_ADC_Int_SetChannelCmd(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), ENABLE);
    }
}

/******************************************************************************
* Function : ADC_eCheckFault(adc_config_s * pADC)
*//**
* \b Description:
*
* This private function must be called to verify ADC input data
*
* PRE-CONDITION: ADC Data
*
* POST-CONDITION: return error
*
* @return     eMCUError_s error type
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
static eMCUError_s ADC_eCheckFault(adc_config_s * pADC)
{
  if (pADC->eSampleType >= ADC_SAMPLE_INVALID)
    {
      return MCU_ERROR_ADC_INVALID_SAMPLE_TYPE;
    }
  //If Interrupt or DMA mode, then a callback must be provided.
  if ((pADC->eSampleType != ADC_SAMPLE_POOLING) && (NULL == pADC->fpCallBack))
    {
      return MCU_ERROR_INVALID_CALLBACK;
    }
  if (pADC->eResolution >= ADC_RES_INVALID)
    {
      return MCU_ERROR_ADC_INVALID_RESOLUTION;
    }
  if (pADC->wSampleRate >= ADC_SAMPLE_MAX)
    {
      return MCU_ERROR_ADC_INVALID_SAMPLE_RATE;
    }
  return MCU_ERROR_SUCCESS;
}

/******************************************************************************
* Function : ADC0_IRQHandler(void)
*//**
* \b Description:
*
* This is a IRQ handling function for the ADC0. The result will be sent via Callback.
* The IRQ  will read the value and send it through the Callback
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC interrupt sample sent via Callback
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available, IRQ is activated via ADC_vStartInterrupt
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
void ADC0_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  NVIC_DisableIRQ(ADC0_IRQn);

  ADC_vTreatInterruptsISR(ADC0);

  NVIC_EnableIRQ(ADC0_IRQn);

  Chip_ADC_SetStartMode(LPC_ADC0, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/******************************************************************************
* Function : ADC1_IRQHandler(void)
*//**
* \b Description:
*
* This is a IRQ handling function for the ADC1. The result will be sent via Callback.
* The IRQ  will read the value and send it through the Callback
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC interrupt sample sent via Callback
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available, IRQ is activated via ADC_vStartInterrupt
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
void ADC1_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  NVIC_DisableIRQ(ADC1_IRQn);

  ADC_vTreatInterruptsISR(ADC1);

  NVIC_EnableIRQ(ADC1_IRQn);

  Chip_ADC_SetStartMode(LPC_ADC1, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

/******************************************************************************
* Function : ADC0_IRQHandler(void)
*//**
* \b Description:
*
* This is a IRQ handling function for the DMA. The result will be sent via Callback.
* The IRQ will Iterate through list to find which ADC channel triggered, it will then
* read the value and send it through the Callback
*
* PRE-CONDITION: ADC Channel must be initialized
*
* POST-CONDITION: ADC interrupt sample sent via Callback
*
* @return     Void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available, IRQ is activated via ADC_vStartInterrupt
~~~~~~~~~~~~~~~
*
* @see ADC_bInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
void DMA_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  uint8_t bIndex = 0;
  adc_config_s * pADC;
  //Iterate through list to find which DMA triggered
  for(bIndex = 0; bIndex < ADC_MAX_CHANNELS; bIndex++)
    {
      // If ADClist position has a valid structure
      if (ADClist[bIndex] != NULL)
        {
          pADC = ADClist[bIndex];
          if (ADC_SAMPLE_DMA == pADC->eSampleType)
            {
              adc_private_config_s *pPrivate = pADC->vpPrivateData;
              //If Success, then send data back through the callback
              if (Chip_GPDMA_Interrupt(LPC_GPDMA, pPrivate->bDMAChannelNum) == SUCCESS)
                {
                uint16_t hADCResult = ADC_DR_RESULT(pPrivate->wDMABuffer);
                pADC->fpCallBack(hADCResult);
                }
            }

          if (pADC->bBurstMode)
            {
              Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), ENABLE);
            }
          else
            {
              Chip_ADC_SetStartMode(ADC_MAP_REGISTER(pADC->eChannel), ADC_START_NOW, ADC_TRIGGERMODE_RISING);
            }
          adc_private_config_s *pPrivate = pADC->vpPrivateData;
          Chip_GPDMA_Transfer(LPC_GPDMA,
                              pPrivate->bDMAChannelNum,
                              sADCMap[pADC->eChannel].wGPDMA,
                              (uint32_t) &pPrivate->wDMABuffer,
                              GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
                              1);
        }
    }
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

eMCUError_s ADC_eInit(adc_config_s * pADC)
{
  //Verify if valid ADC/channel
  if ((pADC->eChannel < ADC_FAULT) && (pADC->vpPrivateData == NULL))
    {
      eMCUError_s eErrorcode = ADC_eCheckFault(pADC);
      if (eErrorcode)
        {
          return eErrorcode;
        }

      ADC_CLOCK_SETUP_T sADCSetup;
      sADCSetup.adcRate = pADC->wSampleRate;
      sADCSetup.bitsAccuracy = pADC->eResolution;
      sADCSetup.burstMode = pADC->bBurstMode;

      //Board Configuration
      BRD_ADCConfig(pADC->eChannel);

      Chip_ADC_Init(ADC_MAP_REGISTER(pADC->eChannel), &sADCSetup);
      Chip_ADC_EnableChannel(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), ENABLE);
      Chip_ADC_SetSampleRate(ADC_MAP_REGISTER(pADC->eChannel), &sADCSetup, pADC->wSampleRate);
      Chip_ADC_SetResolution(ADC_MAP_REGISTER(pADC->eChannel), &sADCSetup, pADC->eResolution);

      //Create private struct, populate it and then reference it on the public struct
      adc_private_config_s * psPrivate = malloc(sizeof(adc_private_config_s));
      if (psPrivate != NULL)
        {
          psPrivate->bStarted = true;
          pADC->vpPrivateData = psPrivate;

          //Store ADC reference into a list for use within Interrupts
          ADClist[pADC->eChannel] = pADC;

          return MCU_ERROR_SUCCESS;
        }
      else
        {
          return MCU_ERROR_MEMORY_ALLOCATION;
        }
    }
  return MCU_ERROR_ADC_INVALID_CHANNEL;
}

void ADC_vDeInit(adc_config_s *pADC)
{
  if ((pADC->eChannel < ADC_FAULT) && (pADC->vpPrivateData != NULL))
    {
      ADC_vStopSampling(pADC);
      Chip_ADC_DeInit(ADC_MAP_REGISTER(pADC->eChannel));

      free(pADC->vpPrivateData);
      pADC->vpPrivateData = NULL;
      ADClist[pADC->eChannel] = NULL;

      //Iterate through list and disable interrupts if no channel is available
      uint8_t bIndex, bNumInterrupts = 0, bNumDMA = 0;
      for (bIndex = 0; bIndex < ADC_MAX_CHANNELS; bIndex++)
        {
          if (ADClist[bIndex] != NULL)
            {
              adc_config_s * pADC = ADClist[bIndex];
              if (pADC->eSampleType == ADC_SAMPLE_DMA)
                {
                  bNumDMA++;
                }
              else if (pADC->eSampleType == ADC_SAMPLE_INTERRUPT)
                {
                  bNumInterrupts++;
                }
            }
        }
      //Disable interrupts
      if (!bNumInterrupts)
        {
          NVIC_DisableIRQ(ADC0_IRQn);
          NVIC_DisableIRQ(ADC1_IRQn);
        }
      if (!bNumDMA)
        {
          NVIC_DisableIRQ(DMA_IRQn);
        }
    }
}

void ADC_vStartSampling(const adc_config_s * pADC)
{
  if ((pADC->eChannel >= ADC_FAULT) || (pADC->vpPrivateData == NULL))
    {
      return;
    }
  if (pADC->eSampleType == ADC_SAMPLE_INTERRUPT)
    {
      ADC_vStartInterrupt(pADC);
    }
  else if (pADC->eSampleType == ADC_SAMPLE_DMA)
    {
      ADC_vStartDMA(pADC);
    }
  else if (pADC->eSampleType == ADC_SAMPLE_POOLING)
    {
      ADC_vStartPooling(pADC);
    }

}

void ADC_vStopSampling(const adc_config_s * pADC)
{

  if ((pADC->eChannel >= ADC_FAULT) || (pADC->vpPrivateData == NULL))
    {
      return;
    }

  if (pADC->bBurstMode)
    {
      Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), DISABLE);
    }
  if (pADC->eSampleType == ADC_SAMPLE_INTERRUPT)
    {
      Chip_ADC_Int_SetChannelCmd(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), DISABLE);
      NVIC_DisableIRQ(ADC_MAP_IRQ(pADC->eChannel));
    }
  else if (pADC->eSampleType == ADC_SAMPLE_DMA)
    {
      Chip_ADC_Int_SetChannelCmd(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), DISABLE);
      adc_private_config_s * psPrivate = (adc_private_config_s *)pADC->vpPrivateData;
      Chip_GPDMA_Stop(LPC_GPDMA, psPrivate->bDMAChannelNum);
    }
}

uint16_t ADC_hSample(const adc_config_s * pADC)
{
  if ((pADC->vpPrivateData != NULL) && (pADC->eSampleType == ADC_SAMPLE_POOLING))
    {
      ADC_vStartPooling(pADC);
      uint16_t hADCData;
      while (Chip_ADC_ReadStatus(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), ADC_DR_DONE_STAT) != SET) {}
      Chip_ADC_ReadValue(ADC_MAP_REGISTER(pADC->eChannel), ADC_MAP_CHANNEL(pADC->eChannel), &hADCData);
      if (pADC->bBurstMode)
          {
            Chip_ADC_SetBurstCmd(ADC_MAP_REGISTER(pADC->eChannel), DISABLE);
          }
      return hADCData;
    }
  return 0;

}
