/*******************************************************************************
* Title                 :   GPIO
* Filename              :   gpio.c
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
*******************************************************************************/
/*************** SOURCE REVISION LOG *****************************************
*
*    Date    Version   Author         Description 
*  03/02/16   1.0.0   Thiago Palmieri GPIO HAL Created.
*
*******************************************************************************/
/** @file gpio.c
 *  @brief This is the source file for GPIO functions
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdlib.h>
#include "gpio.h"
#include "board.h"
#ifdef USE_SYSVIEW
#include "SEGGER.h"
#include "SEGGER_SYSVIEW.h"
#endif

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define MAXIMUM_IRQ 7
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define GPIO_MAP_PORT(x) (sGPIOMap[x].bGPIOPort)) //!< GPIO PORT map macro
#define GPIO_MAP_PIN(x) (sGPIOMap[x].bGPIOPin))   //!< GPIO PIN map macro
/******************************************************************************
* Module Typedefs
*******************************************************************************/
/**
 * This private Struct holding the GPIO mapped port/pin
 */
typedef struct gpio_config_private_s
{
    uint8_t bGPort;              //!< GPIO Port (Private Variable)
    uint8_t bGPin;               //!< GPIO Pin (Private Variable)
    uint8_t bPinIntIndex;        //!< GPIO PIN Interrupt Index
    LPC43XX_IRQn_Type eGPIOIRQ;  //!< GPIO IRQ
    bool bGPIOStarted;           //!< GPIO already started (Private Variable)
}gpio_config_private_s;


/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static uint8_t g_bFreeInterruptIndex = 0; //!< First Free Interrupt Index

static gpio_config_s * psGPIOInterruptTable[MAXIMUM_IRQ + 1] = {NULL}; //!< Initialized Interrupt struct holder
/******************************************************************************
* Function Prototypes
*******************************************************************************/
extern void GPIO0_IRQHandler(void);
extern void GPIO1_IRQHandler(void);
extern void GPIO2_IRQHandler(void);
extern void GPIO3_IRQHandler(void);
extern void GPIO4_IRQHandler(void);
extern void GPIO5_IRQHandler(void);
extern void GPIO6_IRQHandler(void);
extern void GPIO7_IRQHandler(void);
/******************************************************************************
* Function Definitions
*******************************************************************************/
void GPIO0_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
  /* Run Callback */
  if (psGPIOInterruptTable[0] != NULL) psGPIOInterruptTable[0]->fpCallBack();

#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}

void GPIO1_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(1));
  /* Run Callback */
  if (psGPIOInterruptTable[1] != NULL) psGPIOInterruptTable[1]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
void GPIO2_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(2));
  /* Run Callback */
  if (psGPIOInterruptTable[2] != NULL) psGPIOInterruptTable[2]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
void GPIO3_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(3));
  /* Run Callback */
  if (psGPIOInterruptTable[3] != NULL) psGPIOInterruptTable[3]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
void GPIO4_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(4));
  /* Run Callback */
  if (psGPIOInterruptTable[4] != NULL) psGPIOInterruptTable[4]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
void GPIO5_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(5));
  /* Run Callback */
  if (psGPIOInterruptTable[5] != NULL) psGPIOInterruptTable[5]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
void GPIO6_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(6));
  /* Run Callback */
  if (psGPIOInterruptTable[6] != NULL) psGPIOInterruptTable[6]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
void GPIO7_IRQHandler(void)
{
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordEnterISR();
#endif
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(7));
  /* Run Callback */
  if (psGPIOInterruptTable[7] != NULL) psGPIOInterruptTable[7]->fpCallBack();
#ifdef USE_SYSVIEW
  SEGGER_SYSVIEW_RecordExitISR();
#endif
}
/******************************************************************************
* Function : GPIO_bSearchMapping(uint8_t bMainPort, uint8_t bMainPin)
*//**
* \b Description:
*
* This private function is used to perform a Chip port/pin mapping to correct GPIO port/pin.
* This function must receive the Chip Port and Pin as input and will provide a
* position on the sGPIOMap array. This array holds the mapping for all GPIO pins
* on the chip.
*
* PRE-CONDITION: valid PORT and PIN
*
* POST-CONDITION: none
*
* @return     Array position that holds the correct mapping
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sLedRed.bMPort = (1 << 5); // CHIP PORT 5
*   sLedRed.bMPin  = (1 << 1); // CHIP PIN 1
*   sLedRed.eDirection  = GPIO_OUTPUT; // SET AS OUTPUT
*   sLedRed.bDefaultOutputHigh  = true; // Default output is High
*
*   uint8_t bArrayPos = GPIO_bSearchMapping(sLedRed.bPort, sLedRed.bPin);
*   uint8_t bGPIOPortIs = sGPIOMap[bArrayPos].bGPIOPort;
*   uint8_t bGPIOPinIs = sGPIOMap[bArrayPos].bGPIOPin;
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
static uint8_t GPIO_bSearchMapping(const uint8_t bMainPort, const uint8_t bMainPin)
{
  uint8_t bIndexPort, bIndexPin;

  //Iterate through all ports
  for (bIndexPort = 0; bIndexPort < bGPIOMapSize; bIndexPort ++)
    {
      // Port found, now go find pin
      if (sGPIOMap[bIndexPort].bMainPort == bMainPort)
        {
          //Iterate through pins on port
          for (bIndexPin = bIndexPort; bIndexPin < bGPIOMapSize; bIndexPin++, bIndexPort++)
            {
              if (sGPIOMap[bIndexPin].bMainPort != bMainPort) //If mainPort changed, then combination PORT/PIN does not exist
                {
                  return bGPIOMapSize + 1; //Return invalid position
                }
              if (sGPIOMap[bIndexPin].bMainPin == bMainPin) // Found pair Port/Pin, return position
                {
                  return bIndexPin;
                }
            }
        }
    }

  return bGPIOMapSize + 1; //Return invalid position
}

/******************************************************************************
* Function : GPIO_bTest(const gpio_config_s *pGPIO)
*//**
* \b Description:
*
* This private function is used to verify a pin state. It is used internally for
* the GPIO_vToggle function only.
*
* PRE-CONDITION: Initialized GPIO
*
* POST-CONDITION: none
*
* @return     TRUE or FALSE
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   gpio_config_s sLedRed;
*   sButton.bMPort = (1 << 5); // CHIP PORT 5
*   sButton.bMPin  = (1 << 1); // CHIP PIN 1
*   sButton.eDirection  = GPIO_INPUT; // SET AS INPUT
*   sButton.bDefaultOutputHigh  = true; // Default output is High
*
*   GPIO_eInit(&sButton);
*   if (GPIO_bTest(&sButton)) PRINTF("Button is not pressed");
*   else PRINTF("Button is pressed");
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
static bool GPIO_bTest(const gpio_config_s *pGPIO)
{
  gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;
  return (bool) Chip_GPIO_GetPinState(LPC_GPIO_PORT, pPrivateGPIO->bGPort, pPrivateGPIO->bGPin);
}

/******************************************************************************
* Function : GPIO_eSearchInterrupt(gpio_config_s *pGPIO)
*//**
* \b Description:
*
* This private function must be called to assign the correct Interrupt for a given GPIO
*
* PRE-CONDITION: GPIO Data
*
* POST-CONDITION: Interrupt assigned
*
* @return     eMCUerror
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
* @see GPIO_eInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 01/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eMCUError_s GPIO_eSearchInterrupt(gpio_config_s *pGPIO)
{
  if (g_bFreeInterruptIndex > MAXIMUM_IRQ)
    {
      return MCU_ERROR_GPIO_INVALID_INTERRUPTION;
    }

  LPC43XX_IRQn_Type eTable[] =
      {
          PIN_INT0_IRQn,
          PIN_INT1_IRQn,
          PIN_INT2_IRQn,
          PIN_INT3_IRQn,
          PIN_INT4_IRQn,
          PIN_INT5_IRQn,
          PIN_INT6_IRQn,
          PIN_INT7_IRQn
  };

  gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;

  pPrivateGPIO->eGPIOIRQ = eTable[g_bFreeInterruptIndex];
  pPrivateGPIO->bPinIntIndex = g_bFreeInterruptIndex++;
  return MCU_ERROR_SUCCESS;

}

/******************************************************************************
* Function : GPIO_eCheckFault(gpio_config_s * pGPIO)
*//**
* \b Description:
*
* This private function must be called to verify GPIO input data
*
* PRE-CONDITION: GPIO Data
*
* POST-CONDITION: return error
*
* @return     eMCUError_s error type
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
* @see GPIO_eInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 01/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eMCUError_s GPIO_eCheckFault(const gpio_config_s *pGPIO)
{
  if (pGPIO->vpPrivateData != NULL) // Check for double initialization
    {
      return MCU_ERROR_GPIO_INVALID_INITIALIZATION;
    }

  if (
      //If Both INPUT and OUTPUT
      ((pGPIO->eDirection & GPIO_OUTPUT) && (pGPIO->eDirection & GPIO_INPUT)) ||
      // or no direction defined
      (!(pGPIO->eDirection & GPIO_OUTPUT) && !(pGPIO->eDirection & GPIO_INPUT)) ||
      // or invalid value entered
      (pGPIO->eDirection >= GPIO_DIRECTION_INVALID)
      )
    {
      return MCU_ERROR_GPIO_INVALID_DIRECTION;
    }

  if (pGPIO->ePull >= GPIO_PULL_INVALID)
    {
      return MCU_ERROR_GPIO_INVALID_INITIALIZATION;
    }
  return MCU_ERROR_SUCCESS;
}

/******************************************************************************
* Function : GPIO_vEnableInterrupts(gpio_config_s *pGPIO)
*//**
* \b Description:
*
* This private function must be called to initialize the interrupt routines
*
* PRE-CONDITION: GPIO Data
*
* POST-CONDITION: Interrupt enabled
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*  //Not available
~~~~~~~~~~~~~~~
*
* @see GPIO_eInit, GPIO_vSet, GPIO_vClear, GPIO_vToggle, GPIO_bRead
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 01/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static void GPIO_vEnableInterrupts(gpio_config_s *pGPIO)
{
  gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;
  Chip_SCU_GPIOIntPinSel(pPrivateGPIO->bPinIntIndex, pPrivateGPIO->bGPort, pPrivateGPIO->bGPin);
  Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));

  switch (pGPIO->eInterrupt) {
    case GPIO_INTERRUPT_ON_RISING:
      Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      break;
    case GPIO_INTERRUPT_ON_FALLING:
      Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      break;
    case GPIO_INTERRUPT_ON_BOTH:
      Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      break;
    case GPIO_INTERRUPT_ON_LOW:
      Chip_PININT_SetPinModeLevel(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      break;
    case GPIO_INTERRUPT_ON_HIGH:
      Chip_PININT_SetPinModeLevel(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(pPrivateGPIO->bPinIntIndex));
      break;
    default:
      break;
  }

  NVIC_ClearPendingIRQ(pPrivateGPIO->eGPIOIRQ);
  NVIC_SetPriority(pPrivateGPIO->eGPIOIRQ, 5);
  NVIC_EnableIRQ(pPrivateGPIO->eGPIOIRQ);

  psGPIOInterruptTable[pPrivateGPIO->bPinIntIndex] = pGPIO;
}

eMCUError_s GPIO_eInit(gpio_config_s *pGPIO)
{
  eMCUError_s eErrorCode = GPIO_eCheckFault(pGPIO);

  if (eErrorCode)
    {
      return eErrorCode;
    }

  // This part of the code will search the table for the correct main port/pin mapping
  // and return its position on the array
  uint8_t bPosition = GPIO_bSearchMapping(pGPIO->bMPort, pGPIO->bMPin);

  if (bPosition > bGPIOMapSize) //Check if GPIO pin was already Initialized
    {
      return MCU_ERROR_GPIO_INVALID_PORT_OR_PIN;
    }

  //Create private struct, populate it and then reference it on the public struct
  gpio_config_private_s *psPrivate = malloc(sizeof(gpio_config_private_s));
  *psPrivate = (gpio_config_private_s)
      {
    .bGPort = sGPIOMap[bPosition].bGPIOPort,
    .bGPin = sGPIOMap[bPosition].bGPIOPin,
    .bGPIOStarted = true
      };
  pGPIO->vpPrivateData = psPrivate;

  if ((pGPIO->eInterrupt != GPIO_INTERRUPT_DISABLED) && (pGPIO->eDirection == GPIO_INPUT))
    {
      eErrorCode = GPIO_eSearchInterrupt(pGPIO);
    }

  if (eErrorCode)
    {
      return eErrorCode;
    }

  BRD_GPIOConfig(bPosition, pGPIO->ePull);  //PinMux

  Chip_GPIO_Init(LPC_GPIO_PORT);

  if (pGPIO->eDirection & GPIO_OUTPUT) // SET GPIO DIR OUTPUT
    {
      Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, psPrivate->bGPort, psPrivate->bGPin);
      // SET DEFAULT STATE
      Chip_GPIO_SetPinState(LPC_GPIO_PORT, psPrivate->bGPort, psPrivate->bGPin, pGPIO->bDefaultOutputHigh);
    }
  else // SET GPIO DIR INPUT
    {
      Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, psPrivate->bGPort, psPrivate->bGPin);
      if (pGPIO->eInterrupt != GPIO_INTERRUPT_DISABLED)
        {
          GPIO_vEnableInterrupts(pGPIO);
        }
    }

  return MCU_ERROR_SUCCESS;
}

void GPIO_vSet(const gpio_config_s *pGPIO)
{
  if ((pGPIO->vpPrivateData != NULL) && (!(pGPIO->eDirection & GPIO_INPUT)))
    {
      gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;
      Chip_GPIO_SetPinState(LPC_GPIO_PORT, pPrivateGPIO->bGPort, pPrivateGPIO->bGPin, true);
    }
}

void GPIO_vClear(const gpio_config_s *pGPIO)
{
  if ((pGPIO->vpPrivateData != NULL) && (!(pGPIO->eDirection & GPIO_INPUT)))
    {
      gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;
      Chip_GPIO_SetPinState(LPC_GPIO_PORT, pPrivateGPIO->bGPort, pPrivateGPIO->bGPin, false);
    }
}

void GPIO_vToggle(const gpio_config_s *pGPIO)
{
  if ((pGPIO->vpPrivateData != NULL) && (!(pGPIO->eDirection & GPIO_INPUT)))
    {
      gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;
      Chip_GPIO_SetPinState(LPC_GPIO_PORT, pPrivateGPIO->bGPort, pPrivateGPIO->bGPin, !GPIO_bTest(pGPIO));
    }
}

bool GPIO_bRead(const gpio_config_s *pGPIO)
{
  if (pGPIO->vpPrivateData != NULL)
    {
      gpio_config_private_s *pPrivateGPIO = pGPIO->vpPrivateData;
      return Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, pPrivateGPIO->bGPort, pPrivateGPIO->bGPin);
    }
  return false;
}


/*************** END OF FUNCTIONS ***************************************************************************/
