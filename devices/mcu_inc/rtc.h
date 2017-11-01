/****************************************************************************
 * Title                 :   rtc
 * Filename              :   rtc.h
 * Author                :   Henrique Reis
 * Origin Date           :   30/10/2017
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
 *    Date    Version       Author          Description
 * 30/10/2017  1.0.0     Henrique Reis      rtc.h created.
 *
 *****************************************************************************/
#ifndef DEVICES_MCU_INC_RTC_H_
#define DEVICES_MCU_INC_RTC_H_

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
/**
 * @brief RTC time type option
 */
typedef enum {
	RTC_SECOND,		/*!< Second */
	RTC_MINUTE,		/*!< Month */
	RTC_HOUR,			/*!< Hour */
	RTC_DAYOFMONTH,	/*!< Day of month */
	RTC_DAYOFWEEK,		/*!< Day of week */
	RTC_DAYOFYEAR,		/*!< Day of year */
	RTC_MONTH,			/*!< Month */
	RTC_YEAR,			/*!< Year */
	RTC_INVALID
} eRTCTimeIndex;

typedef struct {
	uint32_t time[RTC_INVALID];
} sRTCTime;

typedef void (*rtcCallBack) (sRTCTime sRTCTimeStruct); //!< RTC callback

typedef struct rtc_config_s
{
	rtcCallBack fpRTCCallBack;    //!< RTC Callback function
} rtc_config_s;

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Public Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

eMCUError_s RTC_eInit (const rtc_config_s* pRTC);
eMCUError_s RTC_eDeInit (void);
void RTC_vGetFullTime (const sRTCTime* pTime);
void RTC_vSetFullTime (const sRTCTime* pTime);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DEVICES_MCU_INC_RTC_H_ */
