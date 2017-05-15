/****************************************************************************
* Title                 :   UART HAL
* Filename              :   uart.h
* Author                :   Joao Paulo Martins
* Origin Date           :   04/02/2016
* Version               :   1.0.5
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
*    Date    Version      Author               Description
*  04/02/16   1.0.0  Joao Paulo Martins   First design of UART API.
*  10/02/16   1.0.1  Joao Paulo Martins   Addition of the board config files
*  19/02/16   1.0.2  Joao Paulo Martins   Reception by interrupt feature
*  04/03/16   1.0.3  Joao Paulo Martins   Fixing minor details
*  09/03/16   1.0.4  Joao Paulo Martins   Added DEINIT function
*  31/03/16		1.0.5	 Joao Paulo Martins		Include RTOS header to use SysTick
*
*****************************************************************************/
/** @file uart.h
 *  @brief This module provides and standard API to handle UART peripherals.
 *
 *  The features of this API are:
 *    - UART initialization and basic configuration (word lenght, parity, stop bits);
 *    - Baudrate auto configuration;
 *    - GetChar / PutChar methods;
 *    - Send/Receive streams methods;
 *    - Interrupt/callback for data reception;
 */
#ifndef UART_H_
#define UART_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
#include "mcuerror.h"


/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define UART_RX_PERMANENT 0    //!< Interruption on any reception

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/
/**
  * @brief  Callback definition for UART receive interrupts.
  * @param  bBuffer: pointer to the top of a buffer thst store the
  *                  received data;
  * @param  bLen: Amount of data received at the last UART RX interruption;
  */
typedef void (* uartCallback)(uint8_t *bBuffer, uint32_t bLen);

/**
  * @brief UART config parameter: word lenght (8 or 7 bits)
  */
typedef enum uart_wordlen_e
{
  UART_WORD_8B = 0,
  UART_WORD_7B,
} uart_wordlen_e;

/**
  * @brief UART config parameter: stop bits (1 or 2 bits)
  */
typedef enum uart_stopbits_e
{
  UART_STOPBITS1 = 0,
  UART_STOPBITS2,
} uart_stopbits_e;

/**
  * @brief UART config parameter: parity (none, even or odd)
  */
typedef enum uart_parity_e
{
  UART_PARITY_NONE = 0,
  UART_PARITY_ODD,
  UART_PARITY_EVEN,
} uart_parity_e;

/*
 * @brief UART standard baudrate values
 * */
typedef enum uart_baudrate_e
{
  UART_B9600    = 9600,
  UART_B19200   = 19200,
  UART_B38400   = 38400,
  UART_B57600   = 57600,
  UART_B115200  = 115200,
  UART_B230400  = 230400,
  UART_B460800  = 460800,
  UART_B1000000 = 1000000,
  UART_BINVALID ,
}uart_baudrate_e;

/**
  * @brief UART State definition
  */
typedef enum uart_state_e
{
  UART_RESET             = 0x00,    /*!< Peripheral is not yet Initialized                  */
  UART_READY             = 0x01,    /*!< Peripheral Initialized and ready for use           */
  UART_PERMANENT_RX_IT   = 0x02,    /*!< Interrupt on any receive data arrival              */
  UART_FIXED_RX_IT       = 0x42,    /*!< Receive N bytes by interruption                    */
  UART_BUSY_TX           = 0x12,    /*!< Data Transmission process is ongoing               */
  UART_BUSY_RX           = 0x22,    /*!< Data Reception process is ongoing                  */
  UART_BUSY_TX_RX        = 0x32,    /*!< Data Transmission and Reception process is ongoing */
  UART_TIMEOUT           = 0x03,    /*!< Timeout state                                      */
  UART_ERROR             = 0x04,    /*!< Error on any basic operation                       */
  UART_BUF_OVERFLOW      = 0x05,    /*!< RX Buffer overflow                                 */
}uart_state_e;

/*
*   @brief  UART peripherals nomenclature
*/
typedef enum uart_channel_e
{
  UART0 = 0,         /*!< UART 0 peripheral generic nomenclature   */
  UART1,             /*!< UART 1 peripheral generic nomenclature   */
  UART2,             /*!< UART 2 peripheral generic nomenclature   */
  UART3,             /*!< UART 3 peripheral generic nomenclature   */
  UART_INVALID       /*!< UART 5 peripheral generic nomenclature   */
} uart_channel_e;

/*
*   @brief  UART peripheral basic info needed to init
*/
typedef struct uart_config_s
{
  uart_channel_e  eChannel;             //!< UART instance (UART1, UART2, etc..)
  uart_baudrate_e eBaudrate;            //!< Baudrate as defined number
  uart_wordlen_e  eWordLenght;          //!< Lenght of the data field
  uart_stopbits_e eStopbits;            //!< Stop bits
  uart_parity_e   eParity;              //!< Parity
  uart_state_e    eStatus;              //!< Status of the peripheral
  uartCallback    fpCallBack;           //!< Callback to handle rx interrupt
  uint8_t         *bInBuffer;           //!< Pointer to incoming data buffer
  int             iRxTferSize;          //!< Size of the reception transfer
  void            *vpPrivate;           //!< Private data
} uart_config_s;


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
* Function : UART_eInit(uart_config_s *pUART)
*//**
* \b Description:
* This function is part of the MCULIB UART API. It configures an UART peripheral
* based on the uart_config_s handle. The caller must set some parameters of the
* handle before call the init function. If not set, it will assume the default
* configuration of 8-N-1 9600 bps baud.
*
* PRE-CONDITION: valid uart_config_s handle with valid UART instance
*
* POST-CONDITION: UART peripheral configured and enabled
*
* @return Status of the UART initialization process
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* uart_config_s uart_handle;
*
* // Generic configuration
* uart_handle.channel     = UART1;          // enumerator that list UART devices
* uart_handle.baudrate    = UART_B9600;     // integer value of baudrate
* uart_handle.wordlenght  = UART_WORD_8B;   // UART frame as 8-bit lenght
* uart_handle.stopbits    = UART_STOPBITS1; // UART frame with 1 stop bit
* uart_handle.parity      = PARITY_EVEN;    // UART frame with even parity
*
* // Performs the initialization
* UART_bInit( &uart_handle );
*
~~~~~~~~~~~~~~~
*
* @see
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
eMCUError_s UART_eInit(uart_config_s *pUART);

void UART_vDeInit(uart_config_s *pUART);

/******************************************************************************
* Function: UART_vReconfSettings(uart_config_s *pUART);
*//**
* \b Description:
* This function performs the configuration of UART peripherals. The parameters are set
* on an valid uart_config_s structure. The only required parameter is which UART will be
* used (uart_config_s.e_Channel). The default configuration of the UART is 8-N-1 9600 bps baud.
*
* PRE-CONDITION: A valid uart_config_s handle
*
* POST-CONDITION: UART peripheral enabled with desired word lenght, parity, stop bits and baudrate.
*
* @return nothing
*
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* // Using a already-created handle, set the structure parameters
* uart_handle.baudrate    = UART_B9600;           // integer value of baudrate
* uart_handle.wordlenght  = UART_WORD_8B;   // UART frame as 8-bit lenght
* uart_handle.stopbits    = UART_STOPBITS1; // UART frame with 1 stop bit
* uart_handle.parity      = PARITY_EVEN;    // UART frame with even parity
*
* // calls the configuration routine
* UART_vReconfSettings( &uart_handle );
*
~~~~~~~~~~~~~~~
*
* @see
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
void UART_vReconfSettings(uart_config_s *pUART);

/******************************************************************************
* Function : UART_eSendData(uart_config_s *pUART, uint8_t *vData, int iLenght)
*//**
* \b Description:
*
* This method sends a stream of bytes via UART device, and waits until all the bytes
* have been sent (blocking).
*
* PRE-CONDITION: valid uart_config_s handle
*
* POST-CONDITION:
*
* @return Status of the operation, based on eMCUError_s error codes.
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* extern uart_config_s uart_handle; //valid handle
* uint8_t vector[4] = {0, 1, 2, 3};
*
* // Send data to serial port
* UART_SendData(&uarthandle, (uint8_t*) vector, 4);
~~~~~~~~~~~~~~~
*
* @see
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
uint32_t UART_eSendData(uart_config_s *pUART, uint8_t *vData, int32_t iLenght);

/******************************************************************************
* Function : UART_eRecvData(uart_config_s *pUART, uint8_t *vData, int iLenght)
*//**
* \b Description:
*
* This method waits a stream of bytes to be received at UART device, in blocking mode.
*
*
* PRE-CONDITION: valid uart_config_s handle
*
* POST-CONDITION:
*
* @return Number of bytes received or zero when an error occurs.
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* extern uart_config_s uart_handle; //valid and initialized handle
* uint32_t wReceivedBytes;
*
* uint8_t vector[100];
*
* // Receives 10 bytes of the serial port
* wReceivedBytes = UART_RecvData(&uarthandle, (uint8_t*) vector, 10);
~~~~~~~~~~~~~~~
*
* @see
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
uint32_t UART_eRecvData(uart_config_s *pUART, uint8_t *vData, int32_t wLenght);

/******************************************************************************
* Function : UART_eRecvData_IT(uart_config_s *pUART, int iIRQPrio, int iArg);
*//**
* \b Description:
*
* Receive an amount of data in non blocking mode using interruption.
* The uart_config_s handle must have a pointer to a callback function (which
* will be called when the interruption occurs) and a pointer to a receive data buffer.
*
* The second argument is the interrupt priority number.
*
* There are two modes of operation, the first is a continuous-enabled interrupt mode, which
* executes the callback when any packet of data arrives at UART. The second is a fixed-lenght
* reception, which will execute the callback only when a well-defined amout of bytes arrive at
* the UART. The mode of operation is defined in the wArg argument. When set as UART_RX_PERMANENT
* it will activate the first mode; when set with any number N > 0, it will activate the second
* mode and will trigger the callback only when N bytes have been read.
*
* PRE-CONDITION:
* A valid uart_config_s with a pointer to a uartCallback function implementation and a buffer pointer
* to store the received data.
*
* POST-CONDITION:
* An UART receive interrupt enabled and waiting for new data.
*
* @return An error code of the success of operation.
*
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* void usercallback(uint8_t *data, uint8_t len)
* {
*   int i;
*
*   // Print all received data
*   for (i = 0; i < len; i++)
*   {
*     printf("%d", *data);
*     data++;
*   }
* }
*
* int main(void)
* {
*   uart_config_s uartHandle;
*   uint8_t       buffer[100];
*
*   uartHandle.eChannel = UART1;
*   uartHandle.fpCallBack = usercallback;
*   uartHandle.bInBuffer = buffer;
*
*   UART_eInit(&uartHandle);
*
*   // Trigger a 10 byte reception at UART1
*   UART_eRecvData_IT(&uartHandle, 1, 10);
*
*   for (;;)
* }
*
*
~~~~~~~~~~~~~~~
*
* @see
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
eMCUError_s UART_eRecvData_IT(uart_config_s *pUART, int32_t iIRQPrio, int32_t iArg);
eMCUError_s UART_eRecvDataConfig(uart_config_s *pUART, int32_t iIRQPrio, int32_t iArg);

/******************************************************************************
* Function : UART_ePutChar (uart_config_s *pUART, char *vData);
*//**
* \b Description:
* This function tries to send a byte to a UART device in non-blocking mode.
*
* PRE-CONDITION:
* A valid uart_config_s handle.
*
* POST-CONDITION:
*
* @return An error code the notifies if the byte (char) was sent.
*
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
* // Waits until the UART efectivelly sends out the byte
* while ( UART_ePutChar(&uartHandle, (uint8_t) 0x05) != MCU_ERROR_SUCCESS)
* {
*   printf("Transmission buffer is full \n");
* }
*
* printf("Byte 0x05 was sent successfully");
*
~~~~~~~~~~~~~~~
*
* @see
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
eMCUError_s UART_ePutChar (uart_config_s *pUART, char *vData);

/******************************************************************************
* Function : UART_eGetChar (uart_config_s *pUART, char *vData);
*//**
* \b Description:
* This function tries to read a char from an UART device, in non-blocking mode.
*
* PRE-CONDITION:
* A valid uart_config_s handle.
*
* POST-CONDITION:
*
* @return An error code the notifies if the char was read.
*
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
* // Waits until some character arrive at UART
* while ( UART_eGetChar(&uartHandle, myBufferPointer) != MCU_ERROR_SUCCESS)
* {
*   printf("No data received\n");
* }
*
* printf("Received %c", myBufferPointer[0]);
*
~~~~~~~~~~~~~~~
*
* @see
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
eMCUError_s UART_eGetChar (uart_config_s *pUART, char *vData);

void UART_vInitPeripheralList(void);
void UART_vAddPeripheralList(uart_config_s *sElement, uint8_t bPos);
void UART_vRemovePeripheralList(uint8_t bPos);

/******************************************************************************
* Function : Enables the system-level UART interrupt
*//**
* \b Description:
* This function activates the interrupt line for the requested UART peripheral.
*
* PRE-CONDITION:
* A valid uart_config_s handle.
*
* POST-CONDITION:
*
* @return
*
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
*
~~~~~~~~~~~~~~~
*
* @see
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
eMCUError_s UART_eEnableIRQ(uart_config_s *pUART);

/******************************************************************************
* Function : Disables the system-level UART interrupt
*//**
* \b Description:
* This function de-activates the interrupt line for the requested UART peripheral.
*
* PRE-CONDITION:
* A valid uart_config_s handle.
*
* POST-CONDITION:
*
* @return
*
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
*
~~~~~~~~~~~~~~~
*
* @see
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
eMCUError_s UART_eDisableIRQ(uart_config_s *pUART);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /*UART_H_*/

/*** End of File **************************************************************/
