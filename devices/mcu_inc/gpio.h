/****************************************************************************
* Title                 :   GPIO
* Filename              :   gpio.h
* Author                :   Thiago Palmieri
* Origin Date           :   03/02/2016
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
*  03/02/16   1.0.0   Thiago Palmieri GPIO HAL Created.
*
*****************************************************************************/
/** @file gpio.h
 *  @brief This module provides an adaptation layer between the MCU HAL
 *  and the DEVICES layer
 *
 *  This is the header file for the definition of GPIO functions
 *
 *  A GPIO pin must be initalized before it can be used. In order to initialize it
 *  a PIN/PORt pair must be passed, the PULL to be used, its direction and if it
 *  is initialized as High or Low.
 *
 *  The SET, CLEAR and TOGGLE functions can be used if the GPIO is set as output, and
 *  the READ in case of INPUT.
 *
 *  ATTENTION: If the Structure has a pointer to private data, it is recommended to
 *  ALLWAYS INITIALIZE IT TO NULL.
 */
#ifndef GPIO_H_
#define GPIO_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdbool.h>
#include <inttypes.h>
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
 * This is a GPIO Interrupt callback typedef
 */
typedef void (*vGpioCallback)(void);

/**
 * This enumeration is a list of possible GPIO interrupt types
 */
typedef enum eInterruptType
{
  GPIO_INTERRUPT_DISABLED,    //!< GPIO Interrupt disabled
  GPIO_INTERRUPT_ON_RISING,   //!< GPIO Interrupt on Rising Edge
  GPIO_INTERRUPT_ON_FALLING,  //!< GPIO Interrupt on Falling Edge
  GPIO_INTERRUPT_ON_BOTH,     //!< GPIO Interrupt on both edges
  GPIO_INTERRUPT_ON_LOW,      //!< GPIO Interrupt on LOW
  GPIO_INTERRUPT_ON_HIGH,     //!< GPIO Interrupt on HIGH
  GPIO_INTERRUPT_INVALID      //!< GPIO Interrupt Invalid
} eInterruptType;

/**
 * This enumeration is a list of possible GPIO interrupt types
 */
typedef enum ePinDirection
{
  GPIO_OUTPUT   = (0x1 << 3),       //!< GPIO direction is OUTPUT
  GPIO_INPUT    = (0x2 << 3),       //!< GPIO direction is INPUT
  GPIO_DIRECTION_INVALID            //!< GPIO Invalid Direction
} ePinDirection;

/**
 * This enumeration is a list of possible GPIO interrupt types
 */
typedef enum ePinPull
{
  GPIO_PULLUP   = (0x0 << 3),       //!< GPIO Pull type is PULL DOWN
  GPIO_PULLDOWN = (0x3 << 3),       //!< GPIO Pull type is PULL UP
  GPIO_PULL_INVALID                 //!< GPIO Invalid PULL
} ePinPull;

/**
 * This Struct holds GPIO configuration
 */
typedef struct gpio_config_s
{
  uint8_t bMPort;              //!< MAIN Port
  uint8_t bMPin;               //!< MAIN Pin
  ePinDirection eDirection;    //!< GPIO Direction
  ePinPull ePull;              //!< GPIO Pull Type
  bool bDefaultOutputHigh;     //!< GPIO default output value when pin is OUTPUT
  eInterruptType eInterrupt;   //!< GPIO type of Interrupt when pin is INPUT
  vGpioCallback fpCallBack;    //!< GPIO Interrupt callback
  void * vpPrivateData;        //!< Pointer to private data
} gpio_config_s;
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
* Function : GPIO_bInit(gpio_config_s *pgpio)
*//**
* \b Description:
*
* This function is used to initialize the GPIO interface. The function receives a
* GPIO configuration structure. If the given GPIO cannot be initialized or
* if it is already initialized, the function will return an error.
*
* PRE-CONDITION: valid PORT, PIN and DIRECTION
*
* POST-CONDITION: GPIO pin initialized.
*
* @return     Status (ERROR = 0 or SUCCESS != ERROR)
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sLedRed.bMPort = (1 << 5); // PORT 5
*   sLedRed.bMPin  = (1 << 1); // PIN 1
*   sLedRed.eDirection  = GPIO_OUTPUT; // SET AS OUTPUT
*   sLedRed.bDefaultOutputHigh  = true; // Default output is High
*
*   GPIO_bInit(&sLedRed);
~~~~~~~~~~~~~~~
*
* @see GPIO_bInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
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
eMCUError_s GPIO_eInit(gpio_config_s *pgpio);

/******************************************************************************
* Function : GPIO_vSet(gpio_config_s *pgpio)
*//**
* \b Description:
*
* This function is used to set the GPIO pin to HIGH. The function receives a
* GPIO configuration structure. If the structure is not initialized or
* the pin is set as INPUT, the function will not perform the change.
*
* PRE-CONDITION: GPIO OUTPUT pin structure Initialized
*
* POST-CONDITION: GPIO output set to HIGH.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sLedRed.bMPort = (1 << 5); // PORT 5
*   sLedRed.bMPin  = (1 << 1); // PIN 1
*   sLedRed.eDirection  = GPIO_OUTPUT; // SET AS OUTPUT
*   sLedRed.bDefaultOutputHigh  = false; // Default output is LOW
*
*   GPIO_bInit(&sLedRed);  // Initialize Red LED GPIO
*   GPIO_vSet(&sLedRed);   // Set output to HIGH
~~~~~~~~~~~~~~~
*
* @see GPIO_bInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
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
void GPIO_vSet(const gpio_config_s *pgpio);

/******************************************************************************
* Function : GPIO_vClear(gpio_config_s *pgpio)
*//**
* \b Description:
*
* This function is used to set the GPIO pin to LOW. The function receives a
* GPIO configuration structure. If the structure is not initialized or
* the pin is set as INPUT, the function will not perform the change.
*
* PRE-CONDITION: GPIO OUTPUT pin structure Initialized
*
* POST-CONDITION: GPIO output set to LOW.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sLedRed.bMPort = (1 << 5); // PORT 5
*   sLedRed.bMPin  = (1 << 1); // PIN 1
*   sLedRed.eDirection  = GPIO_OUTPUT; // SET AS OUTPUT
*   sLedRed.bDefaultOutputHigh  = false; // Default output is LOW
*
*   GPIO_bInit(&sLedRed);  // Initialize Red LED GPIO
*   GPIO_vSet(&sLedRed);   // Set output to HIGH
*   GPIO_vClear(&sLedRed);   // Set output to LOW
~~~~~~~~~~~~~~~
*
* @see GPIO_bInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
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
void GPIO_vClear(const gpio_config_s *pgpio);

/******************************************************************************
* Function : GPIO_vToggle(gpio_config_s *pgpio)
*//**
* \b Description:
*
* This function is used to toggle the GPIO pin. The function receives a
* GPIO configuration structure. If the structure is not initialized or
* the pin is set as INPUT, the function will not perform the change.
*
* PRE-CONDITION: GPIO OUTPUT pin structure Initialized
*
* POST-CONDITION: GPIO output toggled.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sLedRed.bMPort = (1 << 5); // PORT 5
*   sLedRed.bMPin  = (1 << 1); // PIN 1
*   sLedRed.eDirection  = GPIO_OUTPUT; // SET AS OUTPUT
*   sLedRed.bDefaultOutputHigh  = false; // Default output is LOW
*
*   GPIO_bInit(&sLedRed);    // Initialize Red LED GPIO
*   GPIO_vSet(&sLedRed);     // Set output to HIGH
*   GPIO_vClear(&sLedRed);   // Set output to LOW
*   GPIO_vToggle(&sLedRed);  // Toggled output (was LOW, now it changed to HIGH)
~~~~~~~~~~~~~~~
*
* @see GPIO_bInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
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
void GPIO_vToggle(const gpio_config_s *pgpio);

/******************************************************************************
* Function : GPIO_bRead(gpio_config_s *pgpio)
*//**
* \b Description:
*
* This function is used to read the GPIO pin. The function receives a
* GPIO configuration structure. If the structure is not initialized the
* result will be false, but without any meaning.
*
* PRE-CONDITION: GPIO INPUT pin structure Initialized
*
* POST-CONDITION: GPIO input read.
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sLedRed.bMPort = (1 << 5); // PORT 5
*   sLedRed.bMPin  = (1 << 1); // PIN 1
*   sLedRed.eDirection  = GPIO_OUTPUT; // SET AS OUTPUT
*   sLedRed.bDefaultOutputHigh  = false; // Default output is LOW
*
*   GPIO_bInit(&sLedRed);    // Initialize Red LED GPIO
*   if (GPIO_bRead) PRINTF("PIN is HIGH");
*   else PRINTF("PIN is LOW");
~~~~~~~~~~~~~~~
*
* @see GPIO_bInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
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
bool GPIO_bRead(const gpio_config_s *pgpio);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /*GPIO_H_*/

/*** End of File **************************************************************/
