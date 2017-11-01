/****************************************************************************
 * Title                 :   devicelib Include File
 * Filename              :   devicelib.h
 * Author                :   Thiago Palmieri
 * Origin Date           :   29/02/2016
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
 *  29/02/16   1.0.0   Thiago Palmieri Devicelib include file Created.
 *
 *****************************************************************************/
/** @file devicelib.h
 *  @brief This file provides all necessary includes for the DEVICE library
 *
 *  This file must be the only one included in upper layers.
 *
 *  In order to add a new device, the DEVICE_MAPPING MACRO must be populated
 *  accordingly and its header file must be included in devicelib.c
 */

#ifndef INC_DEVICELIB_H_
#define INC_DEVICELIB_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <FLASHER_config.h>
#include "deviceerror.h"
#include <stdint.h>
#include <TERMDEV_config.h>

#include "LCDBACKLIGHT_config.h"
#include "KEYBACKLIGHT_config.h"
#include "MLX75305_config.h"
#include "CBACOMM_config.h"
#include "WATCHDOG_config.h"
#include "M2GISOCOMM_config.h"
#include "M2GSENSORCOMM_config.h"
#include "M2GGPSCOMM_config.h"
#include "CBT09427_config.h"
#include "EXTBUZZER_config.h"
#include "CHIPPROG_config.h"
#include "MCURTC_config.h"

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
/*
 * The DEVLIBVERSION is represented in byte way.
 * So, instead of "1.0.0" we have 0x100, version 1.1.4, would be 0x114 and so on
 * This kind of implementation limits the number of minor revisions to 16, so in
 * other words, CODE IT RIGHT !!!!
 * */

#define DEVLIBVERSION 0x100 //!< DEVlib version
/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/
/**
 * This private MACRO holds all the mapping between chip Port/Pin to CAN Port/Pin
 */
//CAN, PORT, TD, RD, MODE
#define DEVICE_MAPPING \
		  X(PERIPHERAL_MCURTC, RTC_open, RTC_read, RTC_write, RTC_ioctl, RTC_close) \
        X(PERIPHERAL_CBT09427, CBT_open, CBT_read, CBT_write, CBT_ioctl, CBT_close) \
        X(PERIPHERAL_EXTBUZZER, EXB_open, EXB_read, EXB_write, EXB_ioctl, EXB_close) \
        X(PERIPHERAL_M2GGPSCOMM, M2GGPS_open, M2GGPS_read, M2GGPS_write, M2GGPS_ioctl, M2GGPS_close) \
        X(PERIPHERAL_M2GSENSORCOMM, M2GSEN_open, M2GSEN_read, M2GSEN_write, M2GSEN_ioctl, M2GSEN_close) \
        X(PERIPHERAL_M2GISOCOMM, M2GISO_open, M2GISO_read, M2GISO_write, M2GISO_ioctl, M2GISO_close) \
        X(PERIPHERAL_TERMDEV,  TDV_open, TDV_read, TDV_write, TDV_ioctl, TDV_close) \
        X(PERIPHERAL_FLASHER,  FLS_open, FLS_read, FLS_write, FLS_ioctl, FLS_close) \
        X(PERIPHERAL_CHIPPROG, CHP_open, CHP_read, CHP_write, CHP_ioctl, CHP_close) \
        X(PERIPHERAL_WDT,  WDT_open, WDT_read, WDT_write, WDT_ioctl, WDT_close) \
        X(PERIPHERAL_INVALID, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0) \

//    X(PERIPHERAL_LCDBACKLIGHT, LBK_open, LBK_read, LBK_write, LBK_ioctl, LBK_close)
//    X(PERIPHERAL_KEYBACKLIGHT, KBL_open, KBL_read, KBL_write, KBL_ioctl, KBL_close)
//    X(PERIPHERAL_CBACOMM, CCM_open, CCM_read, CCM_write, CCM_ioctl, CCM_close)
//    X(PERIPHERAL_MLX75305, MLX_open, MLX_read, MLX_write, MLX_ioctl, MLX_close)
/******************************************************************************
 * Typedefs
 *******************************************************************************/

/**
 * This Typedef holds the Peripheral descriptor
 */
typedef struct peripheral_descriptor_s
{
	eDEVError_s (*open) (void);
	uint32_t (*read) (struct peripheral_descriptor_s* const this,
		void * const vpBuffer,
		const uint32_t tBufferSize);  //!< Pointer to Read Function
	uint32_t (*write) (struct peripheral_descriptor_s* const this,
		const void * vpBuffer,
		const uint32_t tBufferSize);  //!< Pointer to Write Function
	eDEVError_s (*ioctl) (struct peripheral_descriptor_s* const this,
		uint32_t wRequest,
		void * vpValue);            //!< Pointer to ioctl Function
	eDEVError_s (*close) (struct peripheral_descriptor_s* const this);
	uint8_t bOpen;
} peripheral_descriptor_s;

typedef peripheral_descriptor_s* peripheral_descriptor_p; //!< Peripheral descriptor pointer

//typedef eDEVError_s (*fpIOCTLFunction)(void * vpValue);

/******************************************************************************
 * Variables
 *******************************************************************************/
/**
 * This enum holds all peripheral IDs
 */
#define X(a, open, read, write, ioctl, exit) a,
typedef enum peripheralID_e
{
	DEVICE_MAPPING
} peripheralID_e;
#undef X

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Function : DEV_open(peripheralID_e eID)
 *//**
 * \b Description:
 *
 * This is a public function used to open a device interface. The device to open is passed
 * as a function parameter, and the response will be a device descriptor.
 * Without the descriptor, no device can be operated.
 *
 * PRE-CONDITION: Valid peripheral ID
 *
 * POST-CONDITION: Device opened
 *
 * @return     peripheral_descriptor_p
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p MLX75305;
 *   MLX75305 = DEV_open(DEVICE_MLX75305);
 *   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
 ~~~~~~~~~~~~~~~
 *
 * @see DEV_open, DEV_write, DEV_read, DEV_ioctl and DEV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
peripheral_descriptor_p DEV_open (peripheralID_e eID);

/******************************************************************************
 * Function : DEV_read(peripheral_descriptor_p const this,
 *      void * const vpBuffer,
 *      const uint32_t tBufferSize)
 *//**
 * \b Description:
 *
 * This is a public function used to read a device data. The device descriptor, a buffer and
 * buffer size must be passed. The function will return the data readed (if any) on the buffer
 * and the number of bytes read on the return value.
 *
 * PRE-CONDITION: Valid Descriptor
 *
 * POST-CONDITION: Data read
 *
 * @return     uint32_t number of data read
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p MLX75305;
 *   MLX75305 = DEV_open(DEVICE_MLX75305);
 *   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
 *   uint16_t hBuffer[10];
 *   uint32_t bytes_read = DEV_read(MLX75305, &hBuffer, 10);
 *   if (bytes_read)
 *     PRINTF("%d Bytes Read", bytes_read);
 ~~~~~~~~~~~~~~~
 *
 * @see DEV_open, DEV_write, DEV_read, DEV_ioctl and DEV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t DEV_read (peripheral_descriptor_p const this,
	void * const vpBuffer,
	const uint32_t tBufferSize);

/******************************************************************************
 * Function : DEV_write(peripheral_descriptor_p const this,
 *    const void * vpBuffer,
 *    const uint32_t tBufferSize);
 *//**
 * \b Description:
 *
 * This is a public function used to write data to a device. The device descriptor, a buffer and
 * data size must be passed. The function will return the data written (if any) from the buffer.
 *
 * PRE-CONDITION: Valid Descriptor
 *
 * POST-CONDITION: Data written
 *
 * @return     uint32_t number of data written
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p MLX75305;
 *   MLX75305 = DEV_open(DEVICE_MLX75305);
 *   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
 *   uint16_t hBuffer = 60000;
 *   uint32_t bytes_write = DEV_write(MLX75305, &hBuffer, 1);
 *   if (bytes_write)
 *     PRINTF("%d Bytes Written", bytes_write);
 ~~~~~~~~~~~~~~~
 *
 * @see DEV_open, DEV_write, DEV_read, DEV_ioctl and DEV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t DEV_write (peripheral_descriptor_p const this,
	const void * vpBuffer,
	const uint32_t tBufferSize);

/******************************************************************************
 * Function : DEV_ioctl(peripheral_descriptor_p const this,
 *    uint32_t wRequest,
 *    void * vpValue);
 *//**
 * \b Description:
 *
 * This is a public function used to control a device (IO ConTroL). The device descriptor,
 * a control request and control value must be passed into the function. Each device will have
 * its own set of control requests, therefore the user must verify on each device which control
 * is valid.
 *
 * PRE-CONDITION: Valid Descriptor, control request and value
 *
 * POST-CONDITION: Device controlled
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p MLX75305;
 *   MLX75305 = DEV_open(DEVICE_MLX75305);
 *   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
 *   eDEVError_s eError = DEV_ioctl(MLX75305, IOCTL_MLX_SET_RESOLUTION, (void*)ADC_RES_10BITS);
 *   if (!eError) PRINTF("Resolution Changed"):
 ~~~~~~~~~~~~~~~
 *
 * @see DEV_open, DEV_write, DEV_read, DEV_ioctl and DEV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s DEV_ioctl (peripheral_descriptor_p const this,
	uint32_t wRequest,
	void * vpValue);

/******************************************************************************
 * Function : DEV_close(peripheral_descriptor_p const this)
 *//**
 * \b Description:
 *
 * This is a public function used to close a device. The device descriptor, must be passed.
 *
 * PRE-CONDITION: Valid Descriptor
 *
 * POST-CONDITION: Device Closed
 *
 * @return     eDEVError_s
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   peripheral_descriptor_p MLX75305;
 *   MLX75305 = DEV_open(DEVICE_MLX75305);
 *   if (MLX75305 != NULL) PRINTF("MLX75305 ready !");
 *   uint16_t hBuffer = 60000;
 *   uint32_t bytes_write = DEV_write(MLX75305, &hBuffer, 1);
 *   if (bytes_write)
 *     PRINTF("%d Bytes Written", bytes_write);
 *   DEV_close(MLX75305);
 ~~~~~~~~~~~~~~~
 *
 * @see DEV_open, DEV_write, DEV_read, DEV_ioctl and DEV_close
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 15/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eDEVError_s DEV_close (peripheral_descriptor_p const this);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INC_DEVICELIB_H_ */
/*** End of File **************************************************************/
