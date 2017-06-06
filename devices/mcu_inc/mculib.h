/****************************************************************************
 * Title                 :   MCUlib Include File
 * Filename              :   mculib.h
 * Author                :   Thiago Palmieri
 * Origin Date           :   11/02/2016
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
 *  11/02/16   1.0.0   Thiago Palmieri MCUlib include file Created.
 *
 *****************************************************************************/
/** @file mculib.h
 *  @brief This file provides all necessary includes for the MCUlib library
 *
 *  This file must be the only one included in upper layers
 */

#ifndef ABS_INC_MCULIB_H_
#define ABS_INC_MCULIB_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

#include "mcuerror.h"
#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "iap.h"
#include "pwm.h"
#include "usbcdc.h"
#include "lcd.h"
#include "mcu.h"
#include "uart.h"
#include "wdt.h"
#include <inttypes.h>
#include <spifi.h>
#include <stdbool.h>

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
/*
 * The MCULIBVERSION is represented in byte way.
 * So, instead of "1.0.0" we have 0x100, version 1.1.4, would be 0x114 and so on
 * This kind of implementation limits the number of minor revisions to 16, so in
 * other words, CODE IT RIGHT !!!!
 * */

#define MCULIBVERSION 0x100 //!< MCUlib version
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

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

#endif /* ABS_INC_MCULIB_H_ */
/*** End of File **************************************************************/
