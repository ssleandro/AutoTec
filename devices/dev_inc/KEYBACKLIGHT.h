/****************************************************************************
 * Title                 :   KEYBACKLIGHT Include File
 * Filename              :   KEYBACKLIGHT.h
 * Author                :   thiago.palmieri
 * Origin Date           :   17 de mar de 2016
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
 *  17 de mar de 2016   1.0.0   thiago.palmieri KEYBACKLIGHT include file Created.
 *
 *****************************************************************************/
/** @file KEYBACKLIGHT.h
 *  @brief This file provides Keypad Backlight Control
 *
 */
#ifndef INC_KEYBACKLIGHT_H_
#define INC_KEYBACKLIGHT_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "devicelib.h"
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
 * Function : KBL_open(void)
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to open the KEYPAD Backlight interface.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: Sensor initialized
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p KEYBACK = DEV_open(DEVICE_KEYBACKLIGHT);
 *   if (KEYBACK != NULL) PRINTF("KEYBACK ready !");
 ~~~~~~~~~~~~~~~
 *
 * @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s KBL_open (void);

/******************************************************************************
 * Function : KBL_read(struct peripheral_descriptor_s* const this,
 *        void * const vpBuffer,
 *        const uint32_t tBufferSize)
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to read the KEYPAD Backlight interface. Not used.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     uint32_t number of data read (always ZERO)
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p KEYBACK = DEV_open(DEVICE_KEYBACKLIGHT);
 *   if (KEYBACK != NULL) PRINTF("KEYBACK ready !");
 *   uint16_t hBuffer[10];
 *   uint32_t bytes_read = DEV_read(KEYBACK, &hBuffer, 10);
 *   if (!bytes_read)
 *     PRINTF("This is a backlight, what are you trying to read?");
 ~~~~~~~~~~~~~~~
 *
 * @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t KBL_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize);

/******************************************************************************
 * Function : KBL_write(peripheral_descriptor_p const this,
 *    const void * vpBuffer,
 *    const uint32_t tBufferSize);
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to write data to KEYPAD backlight. Not used.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: None
 *
 * @return     uint32_t number of data written (always ZERO)
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p KEYBACK = DEV_open(DEVICE_KEYBACKLIGHT);
 *   if (KEYBACK != NULL) PRINTF("KEYBACK ready !");
 *   uint16_t hBuffer = 60000;
 *   uint32_t bytes_write = DEV_write(KEYBACK, &hBuffer, 1);
 *   if (!bytes_write)
 *     PRINTF("This is a backlight, what are you trying to write??");
 ~~~~~~~~~~~~~~~
 *
 * @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t KBL_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize);

/******************************************************************************
 * Function : KBL_ioctl(peripheral_descriptor_p const this,
 *    uint32_t wRequest,
 *    void * vpValue);
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to control a device (IO ConTroL). The device descriptor,
 * a control request and control value must be passed into the function. Each device will have
 * its own set of control requests, therefore the user must verify on each device which control
 * is valid.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: Device controlled
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p KEYBACK = DEV_open(DEVICE_KEYBACKLIGHT);
 *   if (KEYBACK != NULL) PRINTF("KEYBACK ready !");
 *   uint8_t duty = 50;
 *   eDEVError_s eError = DEV_ioctl(KEYBACK, IOCTL_KBL_SET_DUTY, (void*)&duty);
 *   if (!eError) PRINTF("Duty changed"):
 ~~~~~~~~~~~~~~~
 *
 * @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s KBL_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue);

/******************************************************************************
 * Function : KBL_close(peripheral_descriptor_p const this)
 *//**
 * \b Description:
 *
 * ATTENTION: This interface must not be used directly, use the DEV_ interface.
 *
 * This is a public function used to close the KEYPAD Backlight interface. The device descriptor
 * must be passed. After this function is used, the Backlight will be locked.
 *
 * PRE-CONDITION: Interface Opened
 *
 * POST-CONDITION: Device Closed
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p KEYBACK = DEV_open(DEVICE_KEYBACKLIGHT);
 *   if (KEYBACK != NULL) PRINTF("KEYBACK ready !");
 *   DEV_close(KEYBACK);
 ~~~~~~~~~~~~~~~
 *
 * @see KBL_open, KBL_write, KBL_read, KBL_ioctl and KBL_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s KBL_close (struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_KEYBACKLIGHT_H_ */

/*** End of File **************************************************************/
