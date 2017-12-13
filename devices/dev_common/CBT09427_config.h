/****************************************************************************
 * Title                 :   CBT09427_config
 * Filename              :   CBT09427_config.h
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
 *    Date    Version        Author                        Description
 *  30/09/16   1.0.0    Henrique Reis    CBT09427_config.h created.
 *
 *****************************************************************************/
/** @file   CBT09427_config.h
 *  @brief
 *
 */
#ifndef COMMON_CBT09427_CONFIG_H_
#define COMMON_CBT09427_CONFIG_H_

/******************************************************************************
 * Includes
 ******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#endif

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
#define CBT_MAXIMUM_FREQUENCY 20000

#define CBT_BUZZER_CHANNEL  PWM_CHANNEL1
#define CBT_BUZZER_DUTY     0
#define CBT_BUZZER_FREQ     440

/******************************************************************************
 * Macros
 *******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
#define IOCTL_CBT09427 \
  X(IOCTL_CBT_CHANGE_DUCY_CYCLE, CBT_eChangeDuty) \
  X(IOCTL_CBT_CHANGE_FREQUENCY, CBT_eChangeFrequency) \
  X(IOCTL_CBT_TURN_ON_OFF, CBT_eChangeStatus) \
  X(IOCTL_CBT_READ_FREQ, CBT_eReadConfig) \
  X(IOCTL_CBT_READ_DUTY, CBT_eReadConfig) \
  X(IOCTL_CBT_INVALID, NULL) \

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for MLX75305 sensor
 */
#define X(a, b) a,
typedef enum IOCTL_CBT_REQUEST_e
{
	IOCTL_CBT09427
} IOCTL_CBT_REQUEST_e;
#undef X

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

#endif /* COMMON_CBT09427_CONFIG_H_ */
