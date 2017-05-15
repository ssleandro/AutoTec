/****************************************************************************
* Title                 :		Chip Programmer Device
* Filename              :		CHIPPROG.c
* Author                :		Joao Paulo Martins
* Origin Date           :   10/05/2016
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
*    Date    		Version   		Author         		Description
*  10/05/2016		 1.0.0		Joao Paulo Martins	File creation
*
*****************************************************************************/
/** @file CHIPPROG.c
 *  @brief This file provides on-chip flash programming utilities
 *
 *  This module is a device API to interact with on-chip flash features. Using
 *  the In-Application Programming (IAP) functions (based on IAP MCULIB module),
 *  is possible to program the on-chip flash with user-defined data, check for
 *  blank sectors and read the memory.
 *
 *  Specific macros and constant configurations are set in CHIPPROG_config.h
 *
 *****************************************************************************/

#ifndef DEVICES_DEV_INC_CHIPPROG_H_
#define DEVICES_DEV_INC_CHIPPROG_H_

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

/*@brief This enum describes the operation options of the CHIPMODE device
 * */
typedef enum chp_opmode_e
{
	CHP_OPMODE_IDLE,
	CHP_OPMODE_ERASE,
	CHP_OPMODE_WRITE
}chp_opmode_e;

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
* Function : CHP_open(void);
*//**
* \b Description: This function initializes the CHIPPROG device. It calls the init
* function of the IAP commands.
*
* PRE-CONDITION:
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* POST-CONDITION: IAP commands ready to be effective.
*
* @return Status of the IAP init function under eDEVError_s error code
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p pCHP = DEV_open(PERIPHERAL_CHIPPROG);
*   if (pCHP != NULL) PRINTF("CHIPPROG ready !");
~~~~~~~~~~~~~~~
*
* @see
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
eDEVError_s CHP_open(void);

/******************************************************************************
* Function : CHP_read(struct peripheral_descriptor_s* const this,
				void * const vpBuffer,
				const uint32_t tBufferSize);
*//**
* \b Description: This function is used to check if a selected area on the
* on-chip flash memory is blank. The input parameters are the starting
* address of the area (vpBuffer) and the lenght of the range to be checked
* (tBufferSize). The checking is performed by sectors, so the vpBuffer
* parameter should be a starting sector address and the tBufferSize should be
* inside the last sector to be checked. Only entire sectors will be affected.
*
* PRE-CONDITION: Device opened.
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* POST-CONDITION: none
*
* @return Number of bytes with blank content inside the requested area.
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*		#define SECTOR_SIZE (64*1024) //64 kB sectors
*		#define SECTOR_A_START 0x1A000000
*
*   peripheral_descriptor_p pCHP = DEV_open(PERIPHERAL_CHIPPROG);
*   if (pCHP != NULL) PRINTF("CHIPPROG ready !");
*
*		uint32_t wStartingAddr = SECTOR_1_START;
*		uint32_t wLenght = 5 * SECTOR_SIZE;
*
* 	// Check for blank sectors starting in sector 1 until sector 1 + 10
* 	uint32_t wBytesBlank = CHP_read(pCHP, wStartingAddr, wLenght);
~~~~~~~~~~~~~~~
*
* @see
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
uint32_t CHP_read(struct peripheral_descriptor_s* const this,
				void * const vpBuffer,
				const uint32_t tBufferSize);

/******************************************************************************
* Function : CHP_write(struct peripheral_descriptor_s* const this,
			const void * vpBuffer,
			const uint32_t tBufferSize);
*//**
* \b Description: This function performs the programming of on-chip flash area.
* The destination area must be previously set using IOCTL CHP_eSetDestAddr call.
* The origin of the data must be in RAM memory. The address of the source of data
* is passed in the vpBuffer parameter, the lenght of the data is passed in the
* tBufferSize parameter.
*
* PRE-CONDITION: The CHIPPROG peripheral should be already initiated and the
* data address should be valid. In order to proper write data to flash memory,
* the affected area has to be erased before the writing operation.
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* POST-CONDITION: On-chip flash memory with new data at the requested area.
*
* @return Number of bytes written.
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*		uint32_t wStart = FLASH_A_SEC_2_START;
*		uint32_t wEnd = FLASH_A_SEC_10_END;
*
*   peripheral_descriptor_p pCHP = DEV_open(PERIPHERAL_CHIPPROG);
*   if (pCHP != NULL) PRINTF("CHIPPROG ready !");
*
* 	// Set the start addres / sector
*		DEV_ioctl(pCHP, IOCTL_CHP_START_ADDR, wStart);
*
*		// Set the end address
*		DEV_ioctl(pCHP, IOCTL_CHP_END_ADDR, wEnd);
*
*		// Triggers the erase operation
*		if (DEV_ioctl(pCHP, IOCTL_CHP_ERASE_MEM, NULL) != DEV_ERROR_SUCCESS)
*		{
* 	 	 	 printf("Error while trying to erase flash memory!");
* 	 	 	 return;
*		}
*
*		// Set the destination address
*		if (DEV_ioctl(pCHP, IOCTL_CHP_DEST_ADDR, wStart) != DEV_ERROR_SUCCESS)
*			printf("Invalid address");
*
*		uint8_t bDataSource[512]; // RAM buffer holding the data to be copied
*		uint32_t wBytesWritten = DEV_write(pCHP, &bDataSource[0], 512);
*
~~~~~~~~~~~~~~~
*
* @see CHP_ioctl, CHP_open, CHP_read
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
uint32_t CHP_write(struct peripheral_descriptor_s* const this,
			const void * vpBuffer,
			const uint32_t tBufferSize);

/******************************************************************************
* Function :
*//**
* \b Description: This function calls the
*
** This is a public function used to control a device (IO ConTroL). The device descriptor,
* a control request and control value must be passed into the function. The control requests
* enabled by the CHIPPROG device are: set start address; set end address; erase memory;
* set destination address;
*
* PRE-CONDITION: Interface Opened
*
* POST-CONDITION: Device controlled
*
* @return  eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*		uint32_t wStart = FLASH_A_SEC_2_START;
*		uint32_t wEnd = FLASH_A_SEC_10_END;
*
*   peripheral_descriptor_p pCHP = DEV_open(PERIPHERAL_CHIPPROG);
*   if (pCHP != NULL) PRINTF("CHIPPROG ready !");
*
* 	// Set the start addres / sector
*		DEV_ioctl(pCHP, IOCTL_CHP_START_ADDR, wStart);
*
*		// Set the end address
*		DEV_ioctl(pCHP, IOCTL_CHP_END_ADDR, wEnd);
*
*		// Triggers the erase operation
*		if (DEV_ioctl(pCHP, IOCTL_CHP_ERASE_MEM, NULL) != DEV_ERROR_SUCCESS)
*		{
* 	 	 	 printf("Error while trying to erase flash memory!");
* 	 	 	 return;
*		}
~~~~~~~~~~~~~~~
*
* @see CHP_eSetStartAddr, CHP_eSetEndAddr, CHP_eEraseMemory, CHP_eSetDestAddr
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
eDEVError_s CHP_ioctl(struct peripheral_descriptor_s* const this,
			uint32_t wRequest,
			void * vpValue);

/******************************************************************************
* Function : CHP_close(struct peripheral_descriptor_s* const this);
*//**
* \b Description:
*
** This is a public function used to close the FLASH interface. The device descriptor
* must be passed. After this function is used, the interface will be closed.
*
* ATTENTION: This interface must not be used directly, use the DEV_ interface.
*
* PRE-CONDITION: Interface opened.
*
* POST-CONDITION: Interface disabled.
*
* @return eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*   peripheral_descriptor_p pCHP = DEV_open(PERIPHERAL_CHIPPROG);
*   if (pCHP != NULL) PRINTF("CHIPPROG ready !");
*   DEV_close(pCHP);
~~~~~~~~~~~~~~~
*
* @see CHP_open, CHP_ioctl, CHP_write
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
eDEVError_s CHP_close(struct peripheral_descriptor_s* const this);


#ifdef UNITY_TEST

#define LPC4357_FLASH
#ifdef LPC4357_FLASH
	/* Bank A - 8 kB sectors */
	#define SECTOR_A0_START	 0x1A000000
	#define SECTOR_A1_START  0x1A002000
	#define SECTOR_A2_START  0x1A004000
	#define SECTOR_A3_START  0x1A006000
	#define SECTOR_A4_START  0x1A008000
	#define SECTOR_A5_START  0x1A00A000
	#define SECTOR_A6_START  0x1A00C000
	#define SECTOR_A7_START  0x1A00E000
	#define SECTOR_A0_END	 	 0x1A001FFF
	#define SECTOR_A1_END    0x1A003FFF
	#define SECTOR_A2_END    0x1A005FFF
	#define SECTOR_A3_END    0x1A007FFF
	#define SECTOR_A4_END    0x1A009FFF
	#define SECTOR_A5_END    0x1A00BFFF
	#define SECTOR_A6_END    0x1A00DFFF
	#define SECTOR_A7_END  	 0x1A00FFFF

	/* Bank A - 64 kB sectors */
	#define SECTOR_A8_START	 0x1A010000
	#define SECTOR_A9_START  0x1A020000
	#define SECTOR_A10_START 0x1A030000
	#define SECTOR_A11_START 0x1A040000
	#define SECTOR_A12_START 0x1A050000
	#define SECTOR_A13_START 0x1A060000
	#define SECTOR_A14_START 0x1A070000
	#define SECTOR_A8_END	 	 0x1A01FFFF
	#define SECTOR_A9_END    0x1A02FFFF
	#define SECTOR_A10_END   0x1A03FFFF
	#define SECTOR_A11_END   0x1A04FFFF
	#define SECTOR_A12_END   0x1A05FFFF
	#define SECTOR_A13_END   0x1A06FFFF
	#define SECTOR_A14_END   0x1A07FFFF

  /* Bank B - 8 kB sectors */
	#define SECTOR_B0_START	 0x1B000000
	#define SECTOR_B1_START  0x1B002000
	#define SECTOR_B2_START  0x1B004000
	#define SECTOR_B3_START  0x1B006000
	#define SECTOR_B4_START  0x1B008000
	#define SECTOR_B5_START  0x1B00A000
	#define SECTOR_B6_START  0x1B00C000
	#define SECTOR_B7_START  0x1B00E000
	#define SECTOR_B0_END	 	 0x1B001FFF
	#define SECTOR_B1_END  	 0x1B003FFF
	#define SECTOR_B2_END  	 0x1B005FFF
	#define SECTOR_B3_END  	 0x1B007FFF
	#define SECTOR_B4_END  	 0x1B009FFF
	#define SECTOR_B5_END  	 0x1B00BFFF
	#define SECTOR_B6_END  	 0x1B00DFFF
	#define SECTOR_B7_END  	 0x1B00FFFF

	/* Bank B - 64 kB sectors */
	#define SECTOR_B8_START	 0x1B010000
	#define SECTOR_B9_START  0x1B020000
	#define SECTOR_B10_START 0x1B030000
	#define SECTOR_B11_START 0x1B040000
	#define SECTOR_B12_START 0x1B050000
	#define SECTOR_B13_START 0x1B060000
	#define SECTOR_B14_START 0x1B070000
	#define SECTOR_B8_END	 	 0x1B01FFFF
	#define SECTOR_B9_END  	 0x1B02FFFF
	#define SECTOR_B10_END 	 0x1B03FFFF
	#define SECTOR_B11_END 	 0x1B04FFFF
	#define SECTOR_B12_END 	 0x1B05FFFF
	#define SECTOR_B13_END 	 0x1B06FFFF
	#define SECTOR_B14_END 	 0x1B07FFFF
#endif

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DEVICES_DEV_INC_CHIPPROG_H_ */

/*** End of File **************************************************************/
