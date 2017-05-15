/****************************************************************************
* Title                 :   EXTBUZZER_config
* Filename              :   EXTBUZZER_config.h
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
*  30/09/16   1.0.0    Henrique Reis    EXTBUZZER_config.h created.
*
*****************************************************************************/
/** @file   EXTBUZZER_config.h
*  @brief
*
*/
#ifndef COMMON_EXTBUZZER_CONFIG_H_
#define COMMON_EXTBUZZER_CONFIG_H_

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
#define EXB_MAXIMUM_FREQUENCY 20000

#define EXB_BUZZER_CHANNEL  PWM_CHANNEL0
#define EXB_BUZZER_DUTY     90
#define EXB_BUZZER_FREQ     264

/******************************************************************************
* Macros
*******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
#define IOCTL_EXTBUZZER \
  X(IOCTL_EXB_CHANGE_DUCY_CYCLE, EXB_eChangeDuty) \
  X(IOCTL_EXB_CHANGE_FREQUENCY, EXB_eChangeFrequency) \
  X(IOCTL_EXB_TURN_ON_OFF, EXB_eChangeStatus) \
  X(IOCTL_EXB_READ_FREQ, EXB_eReadConfig) \
  X(IOCTL_EXB_READ_DUTY, EXB_eReadConfig) \
  X(IOCTL_EXB_INVALID, NULL) \

/******************************************************************************
* Typedefs
*******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for MLX75305 sensor
 */
#define X(a, b) a,
typedef enum IOCTL_EXB_REQUEST_e
{
    IOCTL_EXTBUZZER
}IOCTL_EXB_REQUEST_e;
#undef X

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMMON_EXTBUZZER_CONFIG_H_ */
