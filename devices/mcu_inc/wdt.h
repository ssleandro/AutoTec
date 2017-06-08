/****************************************************************************
 * Title                 :   wdt Include File
 * Filename              :   wdt.h
 * Author                :   thiago.palmieri
 * Origin Date           :   6 de mai de 2016
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
 *  6 de mai de 2016   1.0.0   thiago.palmieri wdt include file Created.
 *
 *****************************************************************************/
/** @file wdt.h
 *  @brief This file provides provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of Watchdo functions
 *  TODO: Fix text
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
#ifndef DEVICES_MCU_INC_WDT_H_
#define DEVICES_MCU_INC_WDT_H_

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
typedef void (*wdtCallBack) (void); //!< Watchdog callback

/**
 * This Struct holds Watchdog configuration
 */
typedef struct wdt_config_s
{
	uint32_t wTimeout;            //!< WDT Timeout in milliseconds
	wdtCallBack pfWDTCallBack;    //!< WDT Callback function
} wdt_config_s;

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

/******************************************************************************
 * Function : WDT_eInit(wdt_config_s * pWDT)
 *//**
 * \b Description:
 *
 * This is a public function used to initialize the Watchdog.
 * A wdt_config_s struct pointer must be passed to the function. The struct
 * must be initialized with proper information (timeout larger than 0).
 * If a callback pointer is passed into the sctructure, then the watchdog
 * will fire the callback when the timer expires, otherwise it will reset
 * the chip.
 *
 * PRE-CONDITION: Valid timeout value.
 *
 * POST-CONDITION: WDT initialized and ready.
 *
 * @return     eMCUerror
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   wdt_config_s sWDT;
 *   sWDT.wTimeout = 500; //500 milliseconds
 *
 *   if (MCU_ERROR_SUCCESS == WDT_eInit(&sWDT))
 *     PRINTF("WDT Initiated, but not started");
 ~~~~~~~~~~~~~~~
 *
 * @see WDT_eInit, WDT_eDeInit, WDT_eFeed, WDT_eStart, WDT_bFetchResetCause, WDT_vForceReset
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s WDT_eInit (wdt_config_s * pWDT);

/******************************************************************************
 * Function : WDT_eDeInit(const wdt_config_s * pWDT)
 *//**
 * \b Description:
 *
 * This is a public function used to disable the Watchdog.
 * A previously initialized wdt_config_s struct pointer must be passed to the function.
 *
 * PRE-CONDITION: WDT enabled.
 *
 * POST-CONDITION: WDT disabled (if possible)
 *
 * @return     eMCUerror
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   wdt_config_s sWDT;
 *   sWDT.wTimeout = 500; //500 milliseconds
 *
 *   if (MCU_ERROR_SUCCESS == WDT_eInit(&sWDT))
 *     PRINTF("WDT Initiated, but not started");
 ~~~~~~~~~~~~~~~
 *
 * @see WDT_eInit, WDT_eDeInit, WDT_eFeed, WDT_eStart, WDT_bFetchResetCause, WDT_vForceReset
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s WDT_eDeInit (const wdt_config_s * pWDT);

/******************************************************************************
 * Function : WDT_eFeed(const wdt_config_s * pWDT)
 *//**
 * \b Description:
 *
 * This is a public function used to feed the Watchdog.
 * A previously initialized wdt_config_s struct pointer must be passed to the function.
 * If not fed properly, the watchdog will expire, causing an interrupt or reset.
 *
 * PRE-CONDITION: WDT enabled and started.
 *
 * POST-CONDITION: WDT restarted
 *
 * @return     eMCUerror
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   wdt_config_s sWDT;
 *   sWDT.wTimeout = 500; //500 milliseconds
 *
 *   if (MCU_ERROR_SUCCESS == WDT_eInit(&sWDT))
 *     PRINTF("WDT Initiated, but not started");
 *   WDT_eStart(&sWDT);
 *   WDT_eFeed(&sWDT);
 ~~~~~~~~~~~~~~~
 *
 * @see WDT_eInit, WDT_eDeInit, WDT_eFeed, WDT_eStart, WDT_bFetchResetCause, WDT_vForceReset
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s WDT_eFeed (const wdt_config_s * pWDT);

/******************************************************************************
 * Function : WDT_eStart(const wdt_config_s * pWDT)
 *//**
 * \b Description:
 *
 * This is a public function used to Start the Watchdog.
 * A previously initialized wdt_config_s struct pointer must be passed to the function.
 *
 * PRE-CONDITION: WDT enabled
 *
 * POST-CONDITION: WDT restarted
 *
 * @return     eMCUerror
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   wdt_config_s sWDT;
 *   sWDT.wTimeout = 500; //500 milliseconds
 *
 *   if (MCU_ERROR_SUCCESS == WDT_eInit(&sWDT))
 *     PRINTF("WDT Initiated, but not started");
 *   WDT_eStart(&sWDT);
 *   WDT_eFeed(&sWDT);
 ~~~~~~~~~~~~~~~
 *
 * @see WDT_eInit, WDT_eDeInit, WDT_eFeed, WDT_eStart, WDT_bFetchResetCause, WDT_vForceReset
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s WDT_eStart (const wdt_config_s * pWDT);

/******************************************************************************
 * Function : uint8_t WDT_bFetchResetCause(void)
 *//**
 * \b Description:
 *
 * This is a public function used to identify if previous restart was due to watchdog expiration.
 * The function will return 0 if chip died of natural causes, it will return 1 if
 * died of dog bite.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: none
 *
 * @return     1 if reset by watchdog, 0 otherwise
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   uint8_t cause = WDT_bFetchResetCause();
 *   if (cause) PRINTF("Watchdog caused previous reset");
 ~~~~~~~~~~~~~~~
 *
 * @see WDT_eInit, WDT_eDeInit, WDT_eFeed, WDT_eStart, WDT_bFetchResetCause, WDT_vForceReset
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 06/06/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint8_t WDT_bFetchResetCause (void);

/******************************************************************************
 * Function : WDT_vForceReset(void)
 *//**
 * \b Description:
 *
 * This is a public function used to force a chip reset with cause reason equal to watchdog.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: none
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   WDT_vForceReset();
 ~~~~~~~~~~~~~~~
 *
 * @see WDT_eInit, WDT_eDeInit, WDT_eFeed, WDT_eStart, WDT_bFetchResetCause, WDT_vForceReset
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 09/06/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
void WDT_vForceReset (void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DEVICES_MCU_INC_WDT_H_ */

/*** End of File **************************************************************/
