/****************************************************************************
* Title                 :   ADC HAL
* Filename              :   adc.h
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
/** @file adc.h
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of ADC functions
 *
 *  In order to correctly use ADC functions, a adc_config_s config must be created
 *  and populated with initial ADC configuration. This struct must be initialized
 *  via ADC_eInit before ADC sampling (ADC_vStartSampling) may be used to start measurements.
 *
 *  If configured sample method is pooling, then the Sample function (ADC_hSample) must be used
 *  whenever a reading must be performed, otherwise, the reading will be provided
 *  via callback though Interrupts.
 *
 *  To stop the sampling, function ADC_vStopSampling must be used and in order to
 *  de-initialize the ADC, ADC_vDeInit must be used.
 *
 *  ATTENTION: If the Structure has a pointer to private data, it is recommended to
 *  ALWWAYS INITIALIZE IT TO NULL.
 *
 */
#ifndef ABS_INC_ADC_H_
#define ABS_INC_ADC_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <inttypes.h>
#include <stdbool.h>
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
typedef void (* adcCallBack)(uint16_t hwADCData);  //!< ADC Callback typedef

/**
 * This enumeration is a list of possible ADC sample types
 */
typedef enum sampleType_s
{
  ADC_SAMPLE_POOLING,     //!< ADC Sample via pooling
  ADC_SAMPLE_DMA,         //!< ADC Sample via DMA
  ADC_SAMPLE_INTERRUPT,   //!< ADC sample via Interrupt
  ADC_SAMPLE_INVALID      //!< ADC Invalid Sample method
}sampleType_s;

/**
 * This enumeration is a list of possible ADC channels/ports
 */
typedef enum adcChannel_s
{
  ADC0,         //!< ADC 0
  ADC1,         //!< ADC 1
  ADC_FAULT     //!< ADC Fault indicator
}adcChannel_s;

/**
 * This enumeration is a list of possible ADC resolutions
 */
typedef enum adcResolution_s
{
  ADC_RES_10BITS = 0,   /**< ADC 10 bits */
  ADC_RES_9BITS,        /**< ADC 9 bits  */
  ADC_RES_8BITS,        /**< ADC 8 bits  */
  ADC_RES_7BITS,        /**< ADC 7 bits  */
  ADC_RES_6BITS,        /**< ADC 6 bits  */
  ADC_RES_5BITS,        /**< ADC 5 bits  */
  ADC_RES_4BITS,        /**< ADC 4 bits  */
  ADC_RES_3BITS,        /**< ADC 3 bits  */
  ADC_RES_INVALID       /**< ADC INVALID RESOLUTION */
} adcResolution_s;

typedef enum adcSampleRates_s
{
  ADC_SAMPLE_100KHZ     = 100000,   //!< 100Khz Sample Rate
  ADC_SAMPLE_200KHZ     = 200000,   //!< 200Khz Sample Rate
  ADC_SAMPLE_300KHZ     = 300000,   //!< 300Khz Sample Rate
  ADC_SAMPLE_400KHZ     = 400000,   //!< 400Khz Sample Rate
  ADC_SAMPLE_MAX                    //!< Invalid Sample Rate
}adcSampleRates_s;

/**
 * This Struct holds ADC configuration
 */
typedef struct adc_config_s
{
  adcChannel_s eChannel;        //!< ADC Channel and port
  adcSampleRates_s wSampleRate; //!< ADC Sample Rate
  adcResolution_s eResolution;  //!< ADC Resolution in bits
  bool bBurstMode;              //!< ADC Burst mode
  sampleType_s eSampleType;     //!< ADC Sample type (Interrupt, DMA or Pooling)
  adcCallBack fpCallBack;       //!< ADC Callback to post result
  void * vpPrivateData;         //!< ADC Private data
} adc_config_s;


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
* Function : ADC_eInit(adc_config_s *pADC)
*//**
* \b Description:
*
* This is a public function used to initialize an ADC channel.
* A adc_config_s struct pointer must be passed to the function. The struct
* must be initialized with ADC channel to be used
*
* PRE-CONDITION: Valid ADC Channel.
*
* POST-CONDITION: ADC Channel initialized and ready to be activated.
*
* @return     Status (ERROR = 0 or SUCCESS != ERROR)
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
*   ADC_eInit(&sADCinput);  // ADC0 CH4 Initiated
~~~~~~~~~~~~~~~
*
* @see ADC_eInit, ADC_vDeInit, ADC_vStartSampling, ADC_vStopSampling and ADC_hSample
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
eMCUError_s ADC_eInit(adc_config_s *pADC);

/******************************************************************************
* Function : ADC_bDeInit(adc_config_s *pADC)
*//**
* \b Description:
*
* This is a public function used to de-initialize an ADC channel.
* A adc_config_s struct pointer must be passed to the function.
*
* PRE-CONDITION: Valid ADC Channel.
*
* POST-CONDITION: ADC Channel de-initialized.
*
* @return     void
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
*   ADC_bInit(&sADCinput);    // ADC0 CH4 Initiated
*   ADC_bDeInit(&sADCinput);  // ADC0 CH4 deInitiated
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
void ADC_vDeInit(adc_config_s *pADC);

/******************************************************************************
* Function : ADC_vStartSampling(adc_config_s *pADC)
*//**
* \b Description:
*
* This is a public function used to start an ADC channel sampling.
* A adc_config_s struct pointer must be passed to the function. The struct
* must have been previously initiated, otherwise, nothing will happen.
* If Sampling Method is Interrupt or DMA, the result will be sent via
* Callback (fpCallBack).
*
* PRE-CONDITION: Valid and initiated ADC Channel.
*
* POST-CONDITION: ADC Channel sampling started.
*
* @return     void
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
*   ADC_bInit(&sADCinput);    // ADC0 CH4 Initiated
*   ADC_vStartSampling(&sADCinput);  // Sampling Started
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
void ADC_vStartSampling(const adc_config_s *pADC);

/******************************************************************************
* Function : ADC_vStopSampling(adc_config_s *pADC)
*//**
* \b Description:
*
* This is a public function used to stop an ADC channel sampling.
* A adc_config_s struct pointer must be passed to the function. The struct
* must have been previously initiated, otherwise, nothing will happen.
* If Sampling was not activated previously, then this function will do nothing.
*
* PRE-CONDITION: Valid, initiated and started ADC Channel.
*
* POST-CONDITION: ADC Channel sampling stopped.
*
* @return     void
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
*   ADC_bInit(&sADCinput);    // ADC0 CH4 Initiated
*   ADC_vStartSampling(&sADCinput);  // Sampling Started
*   ADC_vStopSampling(&sADCinput);  // Sampling Stoped
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
void ADC_vStopSampling(const adc_config_s *pADC);

/******************************************************************************
* Function : ADC_hSample(adc_config_s *pADC)
*//**
* \b Description:
*
* This is a public function used to sample an ADC channel.
* A adc_config_s struct pointer must be passed to the function. The struct
* must have been previously initiated, otherwise, nothing will happen.
* This function must be used only if sampling method is Pooling, otherwise it
* will have no effect (return 0).
*
* PRE-CONDITION: Valid, initiated and started ADC Channel with Pooling method.
*
* POST-CONDITION: ADC Channel sample returned.
*
* @return     void
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
*   ADC_bInit(&sADCinput);    // ADC0 CH4 Initiated
*   ADC_vStartSampling(&sADCinput);  // Sampling Started
*   uint16_t sample = ADC_hSample(&sADCinput);  // ADC Sampled
*   PRINTF("The ADC sample value is: %d", sample);
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
uint16_t ADC_hSample(const adc_config_s *pADC);


#ifdef __cplusplus
} // extern "C"
#endif



#endif /* ABS_INC_ADC_H_ */
