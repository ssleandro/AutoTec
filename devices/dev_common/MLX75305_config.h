/****************************************************************************
 * Title                 :   MLX75305_config Include File
 * Filename              :   MLX75305_config.h
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
 *  14/03/2016   1.0.0   thiago.palmieri MLX75305_config include file Created.
 *
 *****************************************************************************/
/** @file MLX75305_config.h
 *  @brief This file provides default configurations for the sensor MLX75305.
 *
 */
#ifndef INC_MLX75305_CONFIG_H_
#define INC_MLX75305_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define MLX_MAX_V_LEVEL (3.3)         //!< Voltage Reference

#define MLX_GAIN_AAA 10               //!< Sensor model GAIN
#define MLX_GAIN_AAB 10               //!< Sensor model GAIN
#define MLX_GAIN_ABA 1                //!< Sensor model GAIN
#define MLX_GAIN_ABB 1                //!< Sensor model GAIN

#define MLX_GAIN MLX_GAIN_ABA         //!< Used Sensor Model
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define MLX_MAXIMUM_IRRADIANCE_uWCM2 (50 * MAX_V_LEVEL)     //!< Maximum Irradiance in uW/cm^2
#define MLX_MINIMUM_VOLTAGE_V_per_uWCM2 (MLX_GAIN * 0.007)  //!< Minimum Detection voltage in mV/(uW/cm^2)
#define MLX_uWCM2_TO_LUX (6.83)                             //!< Constant converter multiplier from uW/cm^2 to LUX

#define MLX_MAXIMUM_SAMPLE_RATE 400000                            //!< 400Khz Maximum sample rate
#define MLX_MAXIMUM_RESOLUTION  10                                //!< 10Bits Maximum Resolution

//Sensor default definitions
#define MLX_SENSOR_CHANNEL      ADC1                              //!< ADC0
#if defined (UNITY_TEST)
#define MLX_SENSOR_RESOLUTION   0                    //!< Resolution
#else
#define MLX_SENSOR_RESOLUTION   ADC_RES_10BITS                    //!< Resolution
#endif
#define MLX_SENSOR_BURST_MODE   false                             //!< Burst mode
#define MLX_SENSOR_SAMPLE_RATE  ADC_SAMPLE_100KHZ                 //!< Sample frequency in interrupt mode
#define MLX_SENSOR_SAMPLE_TYPE  ADC_SAMPLE_POOLING                //!< Sample is pooling

//RingBuffer
#define MLX_SENSOR_SIZE_OF_BUFFER 5
/******************************************************************************
 * Macros
 *******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
//CAN, PORT, TD, RD, MODE
#define IOCTL_MLX57305 \
  X(IOCTL_MLX_RESULT_TYPE, MLX_eChangeResType) \
  X(IOCTL_MLX_SAMPLE_MODE, MLX_eChangeSampleMode) \
  X(IOCTL_MLX_SET_CALLBACK, MLX_eChangeCallBack) \
  X(IOCTL_MLX_SET_BURST_MODE, MLX_eChangeBurstMode) \
  X(IOCTL_MLX_SET_RESOLUTION, MLX_eChangeResolution) \
  X(IOCTL_MLX_SET_SAMPLE_RATE, MLX_eChangeSampleRate) \
  X(IOCTL_MLX_INVALID, NULL) \

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for MLX75305 sensor
 */
#define X(a, b) a,
typedef enum IOCTL_MLX_REQUEST_e
{
	IOCTL_MLX57305
} IOCTL_MLX_REQUEST_e;
#undef X

typedef void (*MLXCallBack) (void *);    //!< Sensor Callback

/**
 * This Typedef holds all MLX75305 result data formats
 */
typedef enum result_type_e
{
	MLX_FORMAT_INTEGER = 0,    //!< Result in Integer format (level based on resolution)
	MLX_FORMAT_FLOAT_LUX = 1,    //!< Result in LUX
	MLX_FORMAT_FLOAT_UWCM2 = 2,    //!< Result in uW/cm^2
	MLX_FORMAT_INVALID              //!< Result Invalid
} result_type_e;

/**
 * This Typedef holds all Sample modes for MLX75305
 */
typedef enum sample_mode_s
{
	MLX_SAMPLE_POOLING = 0,      //!< Sensor Sample via pooling
	MLX_SAMPLE_DMA = 1,      //!< Sensor Sample via DMA
	MLX_SAMPLE_INTERRUPT = 2,      //!< Sensor sample via Interrupt
	MLX_SAMPLE_INTERRUPT_BUFFERED = 3,      //!< Sensor Sample via Interrupt Buffer
	MLX_SAMPLE_INVALID,      //!< Sensor Invalid Sample method
} sample_mode_s;
/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_MLX75305_CONFIG_H_ */

/*** End of File **************************************************************/
