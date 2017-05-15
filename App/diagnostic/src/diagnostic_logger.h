/****************************************************************************
* Title                 :   diagnostic_logger Include File
* Filename              :   diagnostic_logger.h
* Author                :   thiago.palmieri
* Origin Date           :   26 de abr de 2016
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
*  26 de abr de 2016   1.0.0   thiago.palmieri diagnostic_logger include file Created.
*
*****************************************************************************/
/** @file diagnostic_logger.h
 *  @brief This file provides Buffer logging functionalities.
 *
 *  This module is able to log the following events:
 *
 *  - HardFault
 *  - Asserts
 *  - Publishing
 *  - Un/Subscription
 *  - User Inputs (via DBG_LOG messages)
 *  - Watchdog
 *  - Malloc Errors
 *  - Stack Overflows
 *
 *  This file requires that the buffer size and line size to be defined on the configuration file.
 *
 *  The Buffer size defines an area of RAM that will hold all debug information
 *  into a FIFO circular Buffer.
 *
 *  The Buffer line size defines the maximum amount of data to be logged per event, both
 *  defines must be a power of two. See the example below;
 *
 *  Each event has a specific formatting, and it should be taken into consideration
 *  before reading it. All bytes not used by the event will default to 0x00.
 *
 *  \b DIG_BUFFER_SIZE           (1 << 12) <- Buffer Size for error logging \n
 *  \b DIG_BUFFER_MAX_LINE_SIZE  128       <- Line size for error logging \n
 *
 *
 *
~~~~~~~~~~~~~~~
 *  HARDFAULT:
 *  The Hardfault event will save the registers status at the time of the fault, the logged
 *  message have the following format:
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  REGISTERS (4 Bytes each) on the following order:
 *  R0, R1, R2, R3, R12, LR, PC, PSR, _CFSR, _HFSR, _DFSR, _AFSR, _MMAR, _BFAR
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  ASSERTS:
 *  The Assert event will log a text message containing the file and line of assertion failure:
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  STRING       - with the following format ("File: [%s] - Line: [%u]", p, line)
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  PUBLISHING:
 *  This event will log message publishing through broker interface.
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  CONTRACT HEADER:
 *  - ORIGIN     - 1 Byte (in enum format, see common_app.h)
 *  - TOPIC      - 1 Byte (in enum format, see common_app.h)
 *  - VERSION    - 1 Byte
 *  - TYPE       - 1 Byte (in enum format, see interface_broker.h)
 *  MESSAGE HEADER:
 *  - KEY        - 2 Bytes
 *  - SIZE       - 2 Bytes
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  SUBSCRIPTION/UNSUBSCRIPTION:
 *  Those event will log module un/subscription from broker interface, Both will log the same information:
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  DESTINE      - 1 Byte (Module performing the un/subscription, in enum format see common_app.h)
 *  TOPIC        - 1 Byte (Topic being un/subscribed, in enum format see common_app.h)
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  MALLOC:
 *  This event will log a malloc fault from the OS. This event will require futher analysis from
 *  the log to identify what exactly caused the fault.
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  STRING       - with the following format ("MALLOC FAILED !")
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  STACK OVERFLOW:
 *  This event will log a stack overflow fault from the OS, the thread name will be provided
 *  in text format:
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  STRING       - with the following format ("STACK OVERFLOW - %s", pcTaskName)
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  WATCHDOG:
 *  This event will log Watchdog events.
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  MODULE       - 1 Byte (Module in which the watchdog check failed, in enum format see common_app.h)
 *  THREAD       - 1 Byte (Thread that failed. Position in wdt Flag Array)
~~~~~~~~~~~~~~~
 *
~~~~~~~~~~~~~~~
 *  USER INPUTS:
 *  This event will log user inputs (via printf). See debug_tool.h for further information
 *
 *  TIMESTAMP    - 4 Bytes (counter since chip initialization)
 *  LEVEL & TYPE - 1 Byte (Debug level in enum format and type of event, also in enum format, see debug_tool.h)
 *  PAYLOAD SIZE - 1 Byte (Size of payload)
 *  STRING       - up to (DIG_BUFFER_MAX_LINE_SIZE - 6) Bytes formated by the user.
~~~~~~~~~~~~~~~
 */
#ifndef DIAGNOSTIC_SRC_DIAGNOSTIC_LOGGER_H_
#define DIAGNOSTIC_SRC_DIAGNOSTIC_LOGGER_H_

/******************************************************************************
* Includes
*******************************************************************************/

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
#if defined(UNITY_TEST)
typedef void * TaskHandle_t;
typedef void * osThreadId;
#endif

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
* Function : DIG_prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
*//**
* \b Description:
*
* This function is called in case of HardFault.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //This function must be started by the IC
~~~~~~~~~~~~~~~
*
* @see HardFault_Handler
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 26/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void DIG_prvGetRegistersFromStack( uint32_t *pulFaultStackAddress );

#ifndef UNITY_TEST
/******************************************************************************
* Function : DIG_vLoggerInit(osThreadId Thread)
*//**
* \b Description:
*
* This function is used to initialize Debugger logger. It must be executed as soon as possible
* otherwise loggin functions will be disabled.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* DIG_vLoggerInit(ThisThread);
~~~~~~~~~~~~~~~
*
* @see DIG_vLoggerInit
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 26/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
uint8_t DIG_vLoggerInit(osThreadId Thread);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DIAGNOSTIC_SRC_DIAGNOSTIC_LOGGER_H_ */

/*** End of File **************************************************************/
