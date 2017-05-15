/****************************************************************************
* Title                 :   MLX75305 Include File
* Filename              :   MLX75305.c
* Author                :   thiago.palmieri
* Origin Date           :   14/03/2016
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
*  14/03/2016   1.0.0   thiago.palmieri MLX75305 include file Created.
*
*****************************************************************************/
/** @file MLX75305.c
 *  @brief This file provides the interface towards light sensor MLX75305.
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "MLX75305.h"
#include "MLX75305_config.h"
#include <string.h>
#include "ring_buffer.h"
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
#if defined (UNITY_TEST)

#define MCU_ERROR_INVALID_CALLBACK 1
#define MCU_ERROR_ADC_INVALID_SAMPLE_TYPE 2
#define MCU_ERROR_ADC_INVALID_RESOLUTION 3
#define MCU_ERROR_ADC_INVALID_SAMPLE_RATE 4

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

typedef void (* adcCallBack)(uint16_t hwADCData);  //!< ADC Callback typedef

typedef struct adc_config_s
{
  uint32_t eChannel;        //!< ADC Channel and port
  uint32_t wSampleRate; //!< ADC Sample Rate
  uint32_t eResolution;  //!< ADC Resolution in bits
  uint8_t bBurstMode;              //!< ADC Burst mode
  uint32_t eSampleType;     //!< ADC Sample type (Interrupt, DMA or Pooling)
  adcCallBack fpCallBack;       //!< ADC Callback to post result
}adc_config_s;

typedef enum adcChannel_s
{
  ADC0,         //!< ADC 0
  ADC1,         //!< ADC 1
  ADC_FAULT     //!< ADC Fault indicator
}adcChannel_s;

typedef enum sampleType_s
{
  ADC_SAMPLE_POOLING,     //!< ADC Sample via pooling
  ADC_SAMPLE_DMA,         //!< ADC Sample via DMA
  ADC_SAMPLE_INTERRUPT,   //!< ADC sample via Interrupt
  ADC_SAMPLE_INVALID      //!< ADC Invalid Sample method
}sampleType_s;

#endif

typedef eDEVError_s (*fpIOCTLFunction)(void * vpValue);

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
result_type_e eResultFormat = MLX_FORMAT_INTEGER; //!< Default Result Format
MLXCallBack fpUserCallBack = NULL;                //!< Default User Callback
bool bInterruptBuffered = false;                  //!< Default Interrupt type

/**
 * Result Union
 */
static union result_u
{
  uint16_t hData;
  float    fData;
} result_u;

/**
 * Default Sensor Initialization Values
 */
static adc_config_s sMLX75305 =
    {
        .eChannel = (adcChannel_s)MLX_SENSOR_CHANNEL,
        .wSampleRate = MLX_SENSOR_SAMPLE_RATE,
        .eResolution = MLX_SENSOR_RESOLUTION,
        .bBurstMode = MLX_SENSOR_BURST_MODE,
        .eSampleType = MLX_SENSOR_SAMPLE_TYPE,
    };


uint16_t hBufferArray[MLX_SENSOR_SIZE_OF_BUFFER]; //!< RingBuffer array
static RINGBUFF_T mlxBuffer;                         //!< RingBuffer Control handle

/******************************************************************************
* Function Prototypes
*******************************************************************************/
static float MLX_fConvertData(uint16_t hData);

static eDEVError_s MLX_eChangeResType(void * vpValue);
static eDEVError_s MLX_eChangeSampleMode(void * vpValue);
static eDEVError_s MLX_eChangeCallBack(void * vpValue);
static eDEVError_s MLX_eChangeBurstMode(void * vpValue);
static eDEVError_s MLX_eChangeResolution(void * vpValue);
static eDEVError_s MLX_eChangeSampleRate(void * vpValue);

#define X(a, b) b,
fpIOCTLFunction MLX_pIOCTLFunction[] =  //!< IOCTL array of function mapping
{
    IOCTL_MLX57305
};
#undef X
/******************************************************************************
* Function Definitions
*******************************************************************************/
#if defined(UNITY_TEST)
void UNITY_TESTING_FUNCTIONS(void);
uint16_t ADC_hSample(adc_config_s * data)
{
  (void)data;
  return (rand() % (2000 - 0)) + 0;
}

eDEVError_s ADC_eInit(adc_config_s *pADC)
{
  (void)pADC;
  return DEV_ERROR_SUCCESS;
}

void ADC_vStartSampling(adc_config_s *pADC)
{
  (void)pADC;
}

void ADC_vStopSampling(adc_config_s *pADC)
{
  (void)pADC;
}

void ADC_vDeInit(adc_config_s *pADC)
{
  (void)pADC;
}
void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
* Function : vThisCallBack(uint16_t hwADCData)
*//**
* \b Description:
*
* This is a private function used as callback for interrupt routines of sensor
* MLX57305. This function will receive data from the mculib layer and depending
* on sample type it will perform a different action. If buffered interrupt/DMA
* is used the it will save the data into a ring buffer, if the interrupt is not
* buffered, then it will forward the data to a callback provided by the user.
* If no callback is provided, then nothing will happen.
*
* PRE-CONDITION: Interface Opened and interrupt activated
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* Do not use it !
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void vThisCallBack(uint16_t hwADCData)
{
  switch (sMLX75305.eSampleType) {
    case MLX_SAMPLE_DMA:
    case MLX_SAMPLE_INTERRUPT:

      if (bInterruptBuffered)
        {
          //Store data into Ring Buffer
          if(!RingBuffer_Insert(&mlxBuffer,&hwADCData)) //RingBuffer Full
            {
              //Pop from tail and then insert into head
              uint16_t htrash;
              RingBuffer_Pop(&mlxBuffer, &htrash);
              RingBuffer_Insert(&mlxBuffer,&hwADCData);
            }
        }
      else
        {
          //convert and post to user
          if (fpUserCallBack != NULL)
            {
              result_u.fData = MLX_fConvertData(hwADCData);
              fpUserCallBack(&result_u);
            }
        }
      break;
    default:
      break;
  }

}

/******************************************************************************
* Function : MLX_fConvertData(uint16_t hwADCData)
*//**
* \b Description:
*
* This is a private function used to convert the sensor data to a user choosen
* format. The sensor data is a maximum of 10 bit integer, representing the ADC levels.
* This function can convert this value to LUX or uW/cm^2. The return result will be float.
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: None
*
* @return     float data converted (or kept in integer value, cast the result back to 16 bit)
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
* result_u.fData = MLX_fConvertData(hwADCData); //In the example result_u is a union between uint16_t and float
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static float MLX_fConvertData(uint16_t hData)
{
  if (hData >= (1 << (MLX_MAXIMUM_RESOLUTION - sMLX75305.eResolution)))
    {
      hData = (1 << (MLX_MAXIMUM_RESOLUTION - sMLX75305.eResolution)) - 1;
    }

  switch (eResultFormat)
    {
      case MLX_FORMAT_FLOAT_LUX:
        result_u.fData = (hData * (MLX_MAX_V_LEVEL / (1 << (MLX_MAXIMUM_RESOLUTION - sMLX75305.eResolution))) / MLX_MINIMUM_VOLTAGE_V_per_uWCM2) * MLX_uWCM2_TO_LUX;
        break;
      case MLX_FORMAT_FLOAT_UWCM2:
        result_u.fData = (hData * (MLX_MAX_V_LEVEL / (1 << (MLX_MAXIMUM_RESOLUTION - sMLX75305.eResolution))) / MLX_MINIMUM_VOLTAGE_V_per_uWCM2);
        break;
      default:
        result_u.fData = (float)hData;
        break;
    }
  return result_u.fData;
}

/******************************************************************************
* Function : MLX_eChangeResType(void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to change ADC result type. This function must be used by MLX_ioctl ONLY !
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
* if (!MLX_eChangeResType(vpValue)) PRINTF("Result type changed");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s MLX_eChangeResType(void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  if (*(result_type_e*)vpValue >= MLX_FORMAT_INVALID)
    {
      return DEV_ERROR_MLX_INVALID_RESULT_TYPE;
    }
  eResultFormat = *(result_type_e*)vpValue;
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : MLX_eChangeSampleMode(void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to change ADC sample mode. This function must be used by MLX_ioctl ONLY !
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
* if (!MLX_eChangeSampleMode(vpValue)) PRINTF("Sample mode changed");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s MLX_eChangeSampleMode(void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  if (*(sample_mode_s*)vpValue >= MLX_SAMPLE_INVALID)
    {
      return (eDEVError_s)MCU_ERROR_ADC_INVALID_SAMPLE_TYPE;
    }
  if (*(sample_mode_s*)vpValue == MLX_SAMPLE_INTERRUPT_BUFFERED)
    {
      bInterruptBuffered = true;
      sMLX75305.eSampleType = (sampleType_s)MLX_SAMPLE_INTERRUPT;
    }
  else
    {
      sMLX75305.eSampleType = *(sampleType_s*)vpValue;
      bInterruptBuffered = false;
    }
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : MLX_eChangeCallBack(void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to change ADC interrupt callback. This function must be used by MLX_ioctl ONLY !
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
* if (!MLX_eChangeCallBack(vpValue)) PRINTF("CallBack changed");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s MLX_eChangeCallBack(void * vpValue)
{
  if ((adcCallBack*)vpValue == NULL)
    {
      return (eDEVError_s)MCU_ERROR_INVALID_CALLBACK;
    }
  sMLX75305.fpCallBack = (adcCallBack)vpValue;
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : MLX_eChangeBurstMode(void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to change ADC Burst mode. This function must be used by MLX_ioctl ONLY !
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
* if (!MLX_eChangeBurstMode(vpValue)) PRINTF("Burst Mode changed");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s MLX_eChangeBurstMode(void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  sMLX75305.bBurstMode = *(bool*)vpValue;
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : MLX_eChangeResolution(void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to change ADC Resolution. This function must be used by MLX_ioctl ONLY !
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
* if (!MLX_eChangeResolution(vpValue)) PRINTF("Resoluion changed");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s MLX_eChangeResolution(void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  if (*(adcResolution_s*)vpValue >= ADC_RES_INVALID)
    {
      return (eDEVError_s)MCU_ERROR_ADC_INVALID_RESOLUTION;
    }
  sMLX75305.eResolution = *(adcResolution_s*)vpValue;
  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : MLX_eChangeSampleRate(void * vpValue)
*//**
* \b Description:
*
* This is a private function representing a IOCTL request, in this case the request
* to change ADC Sample Rate. This function must be used by MLX_ioctl ONLY !
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
* if (!MLX_eChangeSampleRate(vpValue)) PRINTF("Sample Rate changed");
~~~~~~~~~~~~~~~
*
* @see MLX_open, MLX_write, MLX_read, MLX_ioctl and MLX_close
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s MLX_eChangeSampleRate(void * vpValue)
{
  if (vpValue == NULL)
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  if (*(adcSampleRates_s*)vpValue >= ADC_SAMPLE_MAX)
    {
      return (eDEVError_s)MCU_ERROR_ADC_INVALID_SAMPLE_RATE;
    }
  sMLX75305.wSampleRate = *(adcSampleRates_s*)vpValue;
  return DEV_ERROR_SUCCESS;
}


eDEVError_s MLX_open(void)
{
  RingBuffer_Init(&mlxBuffer, hBufferArray, sizeof(uint16_t), MLX_SENSOR_SIZE_OF_BUFFER);
  RingBuffer_Flush(&mlxBuffer);

  sMLX75305.eChannel = (adcChannel_s)MLX_SENSOR_CHANNEL;
  sMLX75305.wSampleRate = MLX_SENSOR_SAMPLE_RATE;
  sMLX75305.eResolution = MLX_SENSOR_RESOLUTION;
  sMLX75305.bBurstMode = MLX_SENSOR_BURST_MODE;
  sMLX75305.eSampleType = MLX_SENSOR_SAMPLE_TYPE;
  sMLX75305.fpCallBack = vThisCallBack;

  eDEVError_s eError = (eDEVError_s)ADC_eInit(&sMLX75305); //Init ADC
  ADC_vStartSampling(&sMLX75305);             // Start Sampling

  return eError;
}

uint32_t MLX_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize)
{
  (void)this;
  if ((tBufferSize < sizeof(float)) || (vpBuffer == NULL)) //Buffer must have enough space
    {
      return 0;
    }
  //Check Sample Mode, if Pooling read data and return, if buffered, read buffer
  uint16_t hData;
  uint32_t wResult = 0;

  switch (sMLX75305.eSampleType) {
    case MLX_SAMPLE_POOLING:
      hData = ADC_hSample(&sMLX75305);
      result_u.fData = MLX_fConvertData(hData);
      wResult = sizeof(result_u);
      *(uint32_t*)vpBuffer = (uint32_t)result_u.fData;
      break;
    case MLX_SAMPLE_INTERRUPT_BUFFERED:
      if (!RingBuffer_Pop(&mlxBuffer, &hData))
        {
          return 0; //RingBuffer Empty
        }
      result_u.fData = MLX_fConvertData(hData);
      wResult = sizeof(result_u);
      *(uint32_t*)vpBuffer = (uint32_t)result_u.fData;
      break;
    default:  //Not Pooling or Buffered Interrupt
      break;
  }

  return wResult;
}

uint32_t MLX_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize)
{
  (void)this;
  (void)vpBuffer;
  (void)tBufferSize;
  return 0; //Nothing to write into sensor
}

eDEVError_s MLX_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue)
{
  (void)this;
  if ((wRequest >= IOCTL_MLX_INVALID) || (vpValue == NULL))
    {
      return DEV_ERROR_INVALID_IOCTL;
    }
  eDEVError_s eError;
  ADC_vStopSampling(&sMLX75305);
  ADC_vDeInit(&sMLX75305);

  //Configure
  eError = MLX_pIOCTLFunction[wRequest](vpValue);

  eError = (eDEVError_s)ADC_eInit(&sMLX75305);
  ADC_vStartSampling(&sMLX75305);
  return eError;
}

eDEVError_s MLX_close(struct peripheral_descriptor_s* const this)
{
  (void)this;
  ADC_vStopSampling(&sMLX75305);
  ADC_vDeInit(&sMLX75305);

  return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Unity Testing
*******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(MLX75305DeviceTest);

TEST_SETUP(MLX75305DeviceTest)
{
}

TEST_TEAR_DOWN(MLX75305DeviceTest)
{
}

/*
 * This test will verify the list initialization to all NULLs
 */
TEST(MLX75305DeviceTest, test_Verify_that_data_convertion_is_correct)
{
  //adc_config_s test;
  eResultFormat = MLX_FORMAT_FLOAT_LUX;

  TEST_ASSERT_EQUAL_FLOAT(0, MLX_fConvertData(0));
  TEST_ASSERT_EQUAL_FLOAT(31.4439182, MLX_fConvertData(10));
  TEST_ASSERT_EQUAL_FLOAT(314.439178, MLX_fConvertData(100));
  TEST_ASSERT_EQUAL_FLOAT(3144.39185, MLX_fConvertData(1000));
  TEST_ASSERT_EQUAL_FLOAT(3216.71265, MLX_fConvertData(1023));
  TEST_ASSERT_EQUAL_FLOAT(3216.71265, MLX_fConvertData(2000));

  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;

  TEST_ASSERT_EQUAL_FLOAT(0, MLX_fConvertData(0));
  TEST_ASSERT_EQUAL_FLOAT(4.60379457, MLX_fConvertData(10));
  TEST_ASSERT_EQUAL_FLOAT(46.0379457, MLX_fConvertData(100));
  TEST_ASSERT_EQUAL_FLOAT(460.379457, MLX_fConvertData(1000));
  TEST_ASSERT_EQUAL_FLOAT(470.968192, MLX_fConvertData(1023));
  TEST_ASSERT_EQUAL_FLOAT(470.968192, MLX_fConvertData(2000));

  eResultFormat = MLX_FORMAT_INTEGER;

  TEST_ASSERT_EQUAL_FLOAT(0, MLX_fConvertData(0));
  TEST_ASSERT_EQUAL_FLOAT(10.00, MLX_fConvertData(10));
  TEST_ASSERT_EQUAL_FLOAT(100.00, MLX_fConvertData(100));
  TEST_ASSERT_EQUAL_FLOAT(1000.00, MLX_fConvertData(1000));
  TEST_ASSERT_EQUAL_FLOAT(1023.00, MLX_fConvertData(1023));
  TEST_ASSERT_EQUAL_FLOAT(1023.00, MLX_fConvertData(2000));
}

/*
 * On this test a random number between 0 - 2000 is generated and the result is checked
 * against a delta value. The result must be between 0 and the maximum value for a given
 * conversion
 */
TEST(MLX75305DeviceTest, test_Verify_ChangeResType)
{
  eResultFormat = MLX_FORMAT_INTEGER;
  result_type_e value = MLX_FORMAT_FLOAT_LUX;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeResType((void*)&value));
  TEST_ASSERT_EQUAL(value, eResultFormat);

  value = MLX_FORMAT_INVALID;
  TEST_ASSERT_EQUAL(DEV_ERROR_MLX_INVALID_RESULT_TYPE, MLX_eChangeResType((void*)&value));
  TEST_ASSERT_EQUAL(MLX_FORMAT_FLOAT_LUX, eResultFormat);

  value = MLX_FORMAT_INVALID + 10;
  TEST_ASSERT_EQUAL(DEV_ERROR_MLX_INVALID_RESULT_TYPE, MLX_eChangeResType((void*)&value));
  TEST_ASSERT_EQUAL(MLX_FORMAT_FLOAT_LUX, eResultFormat);

  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, MLX_eChangeResType(NULL));
  TEST_ASSERT_EQUAL(MLX_FORMAT_FLOAT_LUX, eResultFormat);
}

TEST(MLX75305DeviceTest, test_Verify_ChangeSampleMode)
{
  bInterruptBuffered = false;
  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;

  //Test correct change
  sample_mode_s value = MLX_SAMPLE_DMA;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeSampleMode((void*)&value));
  TEST_ASSERT_EQUAL(value, sMLX75305.eSampleType);

  //Test invalid change
  value = MLX_SAMPLE_INVALID;
  TEST_ASSERT_EQUAL(MCU_ERROR_ADC_INVALID_SAMPLE_TYPE, MLX_eChangeSampleMode((void*)&value));
  TEST_ASSERT_EQUAL(MLX_SAMPLE_DMA, sMLX75305.eSampleType);

  //Test very invalid change
  value = MLX_SAMPLE_INVALID + 10;
  TEST_ASSERT_EQUAL(MCU_ERROR_ADC_INVALID_SAMPLE_TYPE, MLX_eChangeSampleMode((void*)&value));
  TEST_ASSERT_EQUAL(MLX_SAMPLE_DMA, sMLX75305.eSampleType);

  //Test another correct change
  value = MLX_SAMPLE_INTERRUPT;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeSampleMode((void*)&value));
  TEST_ASSERT_EQUAL(value, sMLX75305.eSampleType);

  //Test Buffered change result
  value = MLX_SAMPLE_INTERRUPT_BUFFERED;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeSampleMode((void*)&value));
  TEST_ASSERT_EQUAL(MLX_SAMPLE_INTERRUPT, sMLX75305.eSampleType);
  TEST_ASSERT_EQUAL(true, bInterruptBuffered);

  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, MLX_eChangeSampleMode(NULL));
  TEST_ASSERT_EQUAL(MLX_SAMPLE_INTERRUPT, sMLX75305.eSampleType);
}

TEST(MLX75305DeviceTest, test_Verify_ChangeCallBack)
{
  void callback(uint16_t data){(void)data;}
  sMLX75305.fpCallBack = NULL;

  TEST_ASSERT_EQUAL(MCU_ERROR_INVALID_CALLBACK, MLX_eChangeCallBack(NULL));
  TEST_ASSERT_NULL(sMLX75305.fpCallBack);

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeCallBack(callback));
  TEST_ASSERT_EQUAL_PTR(callback, sMLX75305.fpCallBack);
}

TEST(MLX75305DeviceTest, test_Verify_ChangeBurstMode)
{
  sMLX75305.bBurstMode = false;
  bool value = true;

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeBurstMode((void*)&value));
  TEST_ASSERT_EQUAL(true, sMLX75305.bBurstMode);
}

TEST(MLX75305DeviceTest, test_Verify_ChangeResolution)
{
  sMLX75305.eResolution = ADC_RES_10BITS;
  adcResolution_s value = ADC_RES_3BITS;

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeResolution((void*)&value));
  TEST_ASSERT_EQUAL(value, sMLX75305.eResolution);

  value = ADC_RES_INVALID;
  TEST_ASSERT_EQUAL(MCU_ERROR_ADC_INVALID_RESOLUTION, MLX_eChangeResolution((void*)&value));
  TEST_ASSERT_EQUAL(ADC_RES_3BITS, sMLX75305.eResolution);

  value = ADC_RES_INVALID + 10;
  TEST_ASSERT_EQUAL(MCU_ERROR_ADC_INVALID_RESOLUTION, MLX_eChangeResolution((void*)&value));
  TEST_ASSERT_EQUAL(ADC_RES_3BITS, sMLX75305.eResolution);

  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, MLX_eChangeResolution(NULL));
  TEST_ASSERT_EQUAL(ADC_RES_3BITS, sMLX75305.eResolution);
}

TEST(MLX75305DeviceTest, test_Verify_ChangeSampleRate)
{
  sMLX75305.wSampleRate = ADC_SAMPLE_400KHZ;
  adcSampleRates_s value = ADC_SAMPLE_200KHZ;

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_eChangeSampleRate((void*)&value));
  TEST_ASSERT_EQUAL(value, sMLX75305.wSampleRate);

  value = ADC_SAMPLE_MAX;
  TEST_ASSERT_EQUAL(MCU_ERROR_ADC_INVALID_SAMPLE_RATE, MLX_eChangeSampleRate((void*)&value));
  TEST_ASSERT_EQUAL(ADC_SAMPLE_200KHZ, sMLX75305.wSampleRate);

  value = ADC_SAMPLE_MAX + 10;
  TEST_ASSERT_EQUAL(MCU_ERROR_ADC_INVALID_SAMPLE_RATE, MLX_eChangeSampleRate((void*)&value));
  TEST_ASSERT_EQUAL(ADC_SAMPLE_200KHZ, sMLX75305.wSampleRate);

  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, MLX_eChangeSampleRate(NULL));
  TEST_ASSERT_EQUAL(ADC_SAMPLE_200KHZ, sMLX75305.wSampleRate);
}

TEST(MLX75305DeviceTest, test_Verify_MLX_open)
{

  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_open());
  TEST_ASSERT_EQUAL(MLX_SENSOR_CHANNEL, sMLX75305.eChannel);
  TEST_ASSERT_EQUAL(MLX_SENSOR_SAMPLE_RATE, sMLX75305.wSampleRate);
  TEST_ASSERT_EQUAL(MLX_SENSOR_RESOLUTION, sMLX75305.eResolution);
  TEST_ASSERT_EQUAL(MLX_SENSOR_BURST_MODE, sMLX75305.bBurstMode);
  TEST_ASSERT_EQUAL(MLX_SENSOR_SAMPLE_TYPE, sMLX75305.eSampleType);
  TEST_ASSERT_EQUAL_PTR(vThisCallBack, sMLX75305.fpCallBack);

  TEST_ASSERT_EQUAL(true, RingBuffer_IsEmpty(&mlxBuffer));
  TEST_ASSERT_EQUAL(MLX_SENSOR_SIZE_OF_BUFFER, RingBuffer_GetFree(&mlxBuffer));
  TEST_ASSERT_EQUAL(MLX_SENSOR_SIZE_OF_BUFFER, RingBuffer_GetSize(&mlxBuffer));
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&mlxBuffer));
}

TEST(MLX75305DeviceTest, test_Verify_MLX_read)
{
  uint8_t buffer = 0;
  uint16_t buffer2 = 0;
  float buffer3 = 0.0;
  uint32_t buffer4 = 0;

  MLX_open();
  //Buffer does not have enough space
  TEST_ASSERT_EQUAL(0, MLX_read(NULL, &buffer, sizeof(buffer)));

  //Buffer does not have enough space
  TEST_ASSERT_EQUAL(0, MLX_read(NULL, &buffer, sizeof(buffer2)));

  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  eResultFormat = MLX_FORMAT_INTEGER;
  TEST_ASSERT_EQUAL(4, MLX_read(NULL, &buffer4, sizeof(buffer4)));
  TEST_ASSERT_HEX32_WITHIN(512, 512, buffer4);

  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  eResultFormat = MLX_FORMAT_FLOAT_LUX;
  TEST_ASSERT_EQUAL(4, MLX_read(NULL, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_FLOAT_WITHIN(1608.356325, 1608.356325, buffer3);

  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  TEST_ASSERT_EQUAL(4, MLX_read(NULL, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_FLOAT_WITHIN(235.484096, 235.484096, buffer3);

  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, MLX_read(NULL, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  sMLX75305.eSampleType = MLX_SAMPLE_DMA;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, MLX_read(NULL, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  //no data in buffer
  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT_BUFFERED;
  bInterruptBuffered = true;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, MLX_read(NULL, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  //Data in buffer
  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT_BUFFERED;
  bInterruptBuffered = true;
  eResultFormat = MLX_FORMAT_INTEGER;
  buffer4 = 0;
  uint16_t data = 50;
  RingBuffer_Insert(&mlxBuffer, &data);
  TEST_ASSERT_EQUAL(1, RingBuffer_GetCount(&mlxBuffer));
  TEST_ASSERT_EQUAL(4, MLX_read(NULL, &buffer4, sizeof(buffer4)));
  TEST_ASSERT_EQUAL(data, buffer4);

  //NULL buffer
  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, MLX_read(NULL, NULL, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);
}

TEST(MLX75305DeviceTest, test_Verify_MLX_ioctl)
{
  uint32_t buffer4 = 0;

  MLX_open();
  //NULL Buffer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, MLX_ioctl(NULL, IOCTL_MLX_RESULT_TYPE, NULL));
  //Invalid Request
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, MLX_ioctl(NULL, IOCTL_MLX_INVALID, &buffer4));

  //Change of Result Type via IOCTL
  eResultFormat = MLX_FORMAT_INTEGER;
  buffer4 = MLX_FORMAT_FLOAT_LUX;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_ioctl(NULL, IOCTL_MLX_RESULT_TYPE, &buffer4));
  TEST_ASSERT_EQUAL(MLX_FORMAT_FLOAT_LUX, eResultFormat);

  //Change of Burst mode via IOCTL
  sMLX75305.bBurstMode = false;
  buffer4 = true;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_ioctl(NULL, IOCTL_MLX_SET_BURST_MODE, &buffer4));
  TEST_ASSERT_EQUAL(true, sMLX75305.bBurstMode);

  //Change of Resolution via IOCTL
  sMLX75305.eResolution = ADC_RES_10BITS;
  buffer4 = ADC_RES_6BITS;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_ioctl(NULL, IOCTL_MLX_SET_RESOLUTION, &buffer4));
  TEST_ASSERT_EQUAL(ADC_RES_6BITS, sMLX75305.eResolution);

  //Change of Sample Rate via IOCTL
  sMLX75305.wSampleRate = ADC_SAMPLE_400KHZ;
  buffer4 = ADC_SAMPLE_200KHZ;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_ioctl(NULL, IOCTL_MLX_SET_SAMPLE_RATE, &buffer4));
  TEST_ASSERT_EQUAL(ADC_SAMPLE_200KHZ, sMLX75305.wSampleRate);

  //Change of Sample Mode via IOCTL
  bInterruptBuffered = false;
  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  buffer4 = MLX_SAMPLE_INTERRUPT_BUFFERED;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_ioctl(NULL, IOCTL_MLX_SAMPLE_MODE, &buffer4));
  TEST_ASSERT_EQUAL(MLX_SAMPLE_INTERRUPT, sMLX75305.eSampleType);
  TEST_ASSERT_EQUAL(true, bInterruptBuffered);

  //Change of Callback via IOCTL
  void callback(uint16_t data){}
  sMLX75305.fpCallBack = NULL;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_ioctl(NULL, IOCTL_MLX_SET_CALLBACK, callback));
  TEST_ASSERT_EQUAL_PTR(callback, sMLX75305.fpCallBack);
}

TEST(MLX75305DeviceTest, test_Verify_MLX_write)
{
  MLX_open();
  uint32_t buffer = 0;
  //Verify that any condition will return 0
  TEST_ASSERT_EQUAL(0, MLX_write(NULL, NULL, 0)); //NULL pointer
  TEST_ASSERT_EQUAL(0, MLX_write(NULL, NULL, 10)); //NULL pointer and wrong size
  TEST_ASSERT_EQUAL(0, MLX_write(NULL, &buffer, 1)); //valid input
  TEST_ASSERT_EQUAL(0, MLX_write(NULL, &buffer, 10)); //Wrong buffer size
}

TEST(MLX75305DeviceTest, test_Verify_MLX_close)
{
  MLX_open();
  //Verify that any condition will return 0
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, MLX_close(NULL)); //NULL pointer
}

TEST(MLX75305DeviceTest, test_Verify_DEV_open)
{
  TEST_ASSERT_NULL(DEV_open(PERIPHERAL_INVALID));
  peripheral_descriptor_p MLX_SENSOR = DEV_open(PERIPHERAL_MLX75305);
  TEST_ASSERT_NOT_NULL(MLX_SENSOR);
  //Try to reopen
  TEST_ASSERT_NULL(DEV_open(PERIPHERAL_MLX75305));
  DEV_close(MLX_SENSOR);
}

TEST(MLX75305DeviceTest, test_Verify_DEV_close)
{
  //Try to close NULL
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(NULL));
  peripheral_descriptor_p MLX_SENSOR = DEV_open(PERIPHERAL_MLX75305);
  TEST_ASSERT_NOT_NULL(MLX_SENSOR);
  //Try to close
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(MLX_SENSOR));
  //Try to re-close
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_close(MLX_SENSOR));
}

TEST(MLX75305DeviceTest, test_Verify_DEV_write)
{
  //Try to write NULL
  TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
  peripheral_descriptor_p MLX_SENSOR = DEV_open(PERIPHERAL_MLX75305);
  TEST_ASSERT_NOT_NULL(MLX_SENSOR);

  uint32_t buffer = 1234;
  //Try to close
  TEST_ASSERT_EQUAL(0, DEV_write(MLX_SENSOR, &buffer, 1));
  //Try to write NULL
  TEST_ASSERT_EQUAL(0, DEV_write(NULL, NULL, 0));
  //Try to close
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_close(MLX_SENSOR));
}

TEST(MLX75305DeviceTest, test_Verify_DEV_ioctl)
{
  uint32_t buffer4 = 0;

  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_DESCRIPTOR, DEV_ioctl(NULL, 0, NULL));
  peripheral_descriptor_p MLX_SENSOR = DEV_open(PERIPHERAL_MLX75305);
  //NULL Buffer
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_RESULT_TYPE, NULL));
  //Invalid Request
  TEST_ASSERT_EQUAL(DEV_ERROR_INVALID_IOCTL, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_INVALID, &buffer4));

  //Change of Result Type via IOCTL
  eResultFormat = MLX_FORMAT_INTEGER;
  buffer4 = MLX_FORMAT_FLOAT_LUX;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_RESULT_TYPE, &buffer4));
  TEST_ASSERT_EQUAL(MLX_FORMAT_FLOAT_LUX, eResultFormat);

  //Change of Burst mode via IOCTL
  sMLX75305.bBurstMode = false;
  buffer4 = true;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_SET_BURST_MODE, &buffer4));
  TEST_ASSERT_EQUAL(true, sMLX75305.bBurstMode);

  //Change of Resolution via IOCTL
  sMLX75305.eResolution = ADC_RES_10BITS;
  buffer4 = ADC_RES_6BITS;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_SET_RESOLUTION, &buffer4));
  TEST_ASSERT_EQUAL(ADC_RES_6BITS, sMLX75305.eResolution);

  //Change of Sample Rate via IOCTL
  sMLX75305.wSampleRate = ADC_SAMPLE_400KHZ;
  buffer4 = ADC_SAMPLE_200KHZ;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_SET_SAMPLE_RATE, &buffer4));
  TEST_ASSERT_EQUAL(ADC_SAMPLE_200KHZ, sMLX75305.wSampleRate);

  //Change of Sample Mode via IOCTL
  bInterruptBuffered = false;
  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  buffer4 = MLX_SAMPLE_INTERRUPT_BUFFERED;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_SAMPLE_MODE, &buffer4));
  TEST_ASSERT_EQUAL(MLX_SAMPLE_INTERRUPT, sMLX75305.eSampleType);
  TEST_ASSERT_EQUAL(true, bInterruptBuffered);

  //Change of Callback via IOCTL
  void callback(uint16_t data){}
  sMLX75305.fpCallBack = NULL;
  TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, DEV_ioctl(MLX_SENSOR, IOCTL_MLX_SET_CALLBACK, callback));
  TEST_ASSERT_EQUAL_PTR(callback, sMLX75305.fpCallBack);
  DEV_close(MLX_SENSOR);
}

TEST(MLX75305DeviceTest, test_Verify_DEV_read)
{
  uint8_t buffer = 0;
  uint16_t buffer2 = 0;
  float buffer3 = 0.0;
  uint32_t buffer4 = 0;

  TEST_ASSERT_EQUAL(0, DEV_read(NULL, NULL, 0));
  peripheral_descriptor_p MLX_SENSOR = DEV_open(PERIPHERAL_MLX75305);
  //Buffer does not have enough space
  TEST_ASSERT_EQUAL(0, DEV_read(MLX_SENSOR, &buffer, sizeof(buffer)));

  //Buffer does not have enough space
  TEST_ASSERT_EQUAL(0, DEV_read(MLX_SENSOR, &buffer, sizeof(buffer2)));

  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  eResultFormat = MLX_FORMAT_INTEGER;
  TEST_ASSERT_EQUAL(4, DEV_read(MLX_SENSOR, &buffer4, sizeof(buffer4)));
  TEST_ASSERT_HEX32_WITHIN(512, 512, buffer4);

  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  eResultFormat = MLX_FORMAT_FLOAT_LUX;
  TEST_ASSERT_EQUAL(4, DEV_read(MLX_SENSOR, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_FLOAT_WITHIN(1608.356325, 1608.356325, buffer3);

  sMLX75305.eSampleType = MLX_SAMPLE_POOLING;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  TEST_ASSERT_EQUAL(4, DEV_read(MLX_SENSOR, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_FLOAT_WITHIN(235.484096, 235.484096, buffer3);

  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, DEV_read(MLX_SENSOR, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  sMLX75305.eSampleType = MLX_SAMPLE_DMA;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, DEV_read(MLX_SENSOR, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  //no data in buffer
  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT_BUFFERED;
  bInterruptBuffered = true;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, DEV_read(MLX_SENSOR, &buffer3, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  //Data in buffer
  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT_BUFFERED;
  bInterruptBuffered = true;
  eResultFormat = MLX_FORMAT_INTEGER;
  buffer4 = 0;
  uint16_t data = 50;
  RingBuffer_Insert(&mlxBuffer, &data);
  TEST_ASSERT_EQUAL(1, RingBuffer_GetCount(&mlxBuffer));
  TEST_ASSERT_EQUAL(4, DEV_read(MLX_SENSOR, &buffer4, sizeof(buffer4)));
  TEST_ASSERT_EQUAL(data, buffer4);

  //NULL buffer
  sMLX75305.eSampleType = MLX_SAMPLE_INTERRUPT;
  eResultFormat = MLX_FORMAT_FLOAT_UWCM2;
  buffer3 = 0;
  TEST_ASSERT_EQUAL(0, DEV_read(MLX_SENSOR, NULL, sizeof(buffer3)));
  TEST_ASSERT_EQUAL_FLOAT(0, buffer3);

  DEV_close(MLX_SENSOR);
}

#endif
