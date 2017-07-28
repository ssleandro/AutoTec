/****************************************************************************
 * Title                 :   debug_tool Include File
 * Filename              :   debug_tool.h
 * Author                :   thiago.palmieri
 * Origin Date           :   24 de mar de 2016
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
 *  24 de mar de 2016   1.0.0   thiago.palmieri debug_tool include file Created.
 *
 *****************************************************************************/
/** @file debug_tool.h
 *  @brief This file provides Debug Functions.
 *
 */
#ifndef COMMON_DEBUG_TOOL_H_
#define COMMON_DEBUG_TOOL_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef UNITY_TEST
#include "SEGGER_SYSVIEW_Conf.h"
#include <auteq_os.h>
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#ifndef WEAK
#if defined(__IAR_SYSTEMS_ICC__)
#define WEAK __weak
#elif defined (__GNUC__)
#define WEAK __attribute__((weak))
#endif
#endif
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/* Debug Level */
#define LEVEL_TRACE     (6 << 4)
#define LEVEL_DEBUG     (5 << 4)
#define LEVEL_CRITICAL  (4 << 4)
#define LEVEL_ERROR     (3 << 4)
#define LEVEL_WARNING   (2 << 4)
#define LEVEL_INFO      (1 << 4)
#define LEVEL_FORCE     (0 << 4)
/******************************************************************************
 * Macros
 *******************************************************************************/
#define DBG_LOG_LEVEL LEVEL_DEBUG      //!< Debug Level activated

#if DBG_LOG_LEVEL >= LEVEL_TRACE
#  define DBG_LOG_TRACE( format, ... ) TOL_wPrintf( LEVEL_TRACE, format, ## __VA_ARGS__ )
#else
#  define DBG_LOG_TRACE( format, ... )
#endif

#if DBG_LOG_LEVEL >= LEVEL_DEBUG
#  define DBG_LOG_DEBUG( format, ... ) TOL_wPrintf( LEVEL_DEBUG, format, ## __VA_ARGS__ )
#else
#  define DBG_LOG_DEBUG( format, ... )
#endif

#if DBG_LOG_LEVEL >= LEVEL_CRITICAL
#  define DBG_LOG_CRITICAL( format, ... ) TOL_wPrintf( LEVEL_CRITICAL, format, ## __VA_ARGS__ )
#else
#  define DBG_LOG_CRITICAL( format, ... )
#endif

#if DBG_LOG_LEVEL >= LEVEL_ERROR
#  define DBG_LOG_ERROR( format, ... ) TOL_wPrintf( LEVEL_ERROR, format, ## __VA_ARGS__ )
#else
#  define DBG_LOG_ERROR( format, ... )
#endif

#if DBG_LOG_LEVEL >= LEVEL_WARNING
#  define DBG_LOG_WARNING( format, ... ) TOL_wPrintf( LEVEL_WARNING, format, ## __VA_ARGS__ )
#else
#  define DBG_LOG_WARNING( format, ... )
#endif

#if DBG_LOG_LEVEL >= LEVEL_INFO
#  define DBG_LOG_INFO( format, ... ) TOL_wPrintf( LEVEL_INFO, format, ## __VA_ARGS__ )
#else
#  define DBG_LOG_INFO( format, ... )
#endif

#define DBG_LOG_FORCE( force, format, ... ) {if(force) {TOL_wPrintf( format, ## __VA_ARGS__  );}}

#ifdef NASSERT           /* NASSERT defined--ASSERT disabled */

#define ASSERT(ignore_)  ((void)0)
#define ASSERT_LEVEL(ignore_, ignore_)  ((void)0)

#else

#define ASSERT(test_) \
    ((test_) ? (void)0 : DIG_onAssert__(GET_TIMESTAMP(), LEVEL_FORCE, __FILE__, __LINE__))

#define ASSERT_LEVEL(test_, debugLevel) \
    ((test_) ? (void)0 : DIG_onAssert__(GET_TIMESTAMP(), debugLevel, __FILE__, __LINE__))

#endif
/******************************************************************************
 * Typedefs
 *******************************************************************************/

/**
 * This Typedef holds all logger types
 */
typedef enum debugType_e
{
	DEBUG_USER = 0, //!< DEBUG_USER
	DEBUG_ASSERT = 1, //!< DEBUG_ASSERT
	DEBUG_QUEUE = 2, //!< DEBUG_QUEUE
	DEBUG_WATCHDOG = 3, //!< DEBUG_WATCHDOG
	DEBUG_HARDFAULT = 4, //!< DEBUG_HARDFAULT
	DEBUG_SUBS = 5, //!< DEBUG_SUBS
	DEBUG_UNSUBS = 6, //!< DEBUG_UNSUBS
	DEBUG_PUBS = 7	 //!< DEBUG_PUBS

} debugType_e;
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
	 * Function : DIG_wLoggerWritter(uint32_t wTimestamp, uint8_t bLevelAndType, uint8_t* pbData, uint32_t wDataSize);
	 *//**
	 * \b Description:
	 *
	 * This function Loads data from the given locations and writes them to the buffer.
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     Number of bytes written
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * char test[] = "Test";
	 * TOL_wPrintf(LEVEL_CRITICAL, "This is a %s", test);
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, DIG_onAssert__, DIG_onSubs__, DIG_onUnSubs__, DIG_wLoggerWritter, DIG_onWDT__
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 29/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	WEAK extern uint32_t DIG_wLoggerWritter (uint32_t wTimestamp, uint8_t bLevelAndType, uint8_t* pbData,
		uint8_t bDataSize);

	/******************************************************************************
	 * Function : TOL_wPrintf(uint8_t bLevel, const char *fmt, ...)
	 *//**
	 * \b Description:
	 *
	 * This function Loads data from the given locations and writes them to the stream.
	 * The bLevel input variable is the DEBUG level to be used on the message.
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     Number of bytes written
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * char test[] = "Test";
	 * TOL_wPrintf(LEVEL_CRITICAL, "This is a %s", test);
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, DIG_onAssert__, DIG_onSubs__, DIG_onUnSubs__, DIG_wLoggerWritter, DIG_onWDT__
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	uint32_t TOL_wPrintf (uint8_t bLevel, const char *fmt, ...);

	/******************************************************************************
	 * Function : DIG_onAssert__(unsigned int timer, uint32_t debugLevel, char const *file, unsigned line)
	 *//**
	 * \b Description:
	 *
	 * This function is invoked in case of assertion failure
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     void
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * ASSERT_LEVEL(A == B, LEVEL_INFO);
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, DIG_onAssert__, DIG_onSubs__, DIG_onUnSubs__, DIG_wLoggerWritter, DIG_onWDT__
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	WEAK extern void DIG_onAssert__ (unsigned int timer, uint32_t debugLevel, char const *file, unsigned line);

	/******************************************************************************
	 * Function : DIG_onSubs__(unsigned int timer, uint8_t eDestine, uint8_t eTopic)
	 *//**
	 * \b Description:
	 *
	 * This function is invoked during subscription
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     void
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * DIG_onSubs__(GET_TIMESTAMP(), MODULE_SENSOR, TOPIC_COMM);
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, DIG_onAssert__, DIG_onSubs__, DIG_onUnSubs__, DIG_wLoggerWritter, DIG_onWDT__
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	WEAK extern void DIG_onSubs__ (unsigned int timer, uint8_t eDestine, uint8_t eTopic);

	/******************************************************************************
	 * Function : DIG_onSubs__(unsigned int timer, uint8_t eDestine, uint8_t eTopic)
	 *//**
	 * \b Description:
	 *
	 * This function is invoked during un-subscription
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     void
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * DIG_onUnSubs__(GET_TIMESTAMP(), MODULE_SENSOR, TOPIC_COMM);
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, DIG_onAssert__, DIG_onSubs__, DIG_onUnSubs__, DIG_wLoggerWritter, DIG_onWDT__
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	WEAK extern void DIG_onUnSubs__ (unsigned int timer, uint8_t eDestine, uint8_t eTopic);

	/******************************************************************************
	 * Function : DIG_onWDT__(unsigned int timer, uint8_t eModule, uint8_t bFlagPos)
	 *//**
	 * \b Description:
	 *
	 * This function is invoked in case of Watchdog error
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     void
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * DIG_onWDT__(GET_TIMESTAMP(), MODULE_SENSOR, 1); //Thread position 1 failed
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, DIG_onAssert__, DIG_onSubs__, DIG_onUnSubs__, DIG_wLoggerWritter, DIG_onWDT__
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	WEAK extern void DIG_onWDT__ (unsigned int timer, uint8_t eModule, uint8_t bFlagPos);

	/******************************************************************************
	 * Function : TOL_vCheckStack(const char * bName)
	 *//**
	 * \b Description:
	 *
	 * This function is used to check for current Thread Free stack size. The result is printed
	 * via a Debug message (DBG_LOG_DEBUG)
	 *
	 * PRE-CONDITION: None
	 *
	 * POST-CONDITION: None
	 *
	 * @return     void
	 *
	 * \b Example
	 ~~~~~~~~~~~~~~~{.c}
	 * TOL_vCheckStack("My name");
	 ~~~~~~~~~~~~~~~
	 *
	 * @see TOL_wPrintf, TOL_vCheckStack
	 *
	 * <br><b> - HISTORY OF CHANGES - </b>
	 *
	 * <table align="left" style="width:800px">
	 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
	 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
	 * </table><br><br>
	 * <hr>
	 *
	 *******************************************************************************/
	void TOL_vCheckStack (const char * bName);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMMON_DEBUG_TOOL_H_ */

/*** End of File **************************************************************/
