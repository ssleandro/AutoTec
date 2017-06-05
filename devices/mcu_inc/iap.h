/****************************************************************************
 * Title                 :	IN APPLICATION PROGRAMMIN API
 * Filename              :	iap.h
 * Author                :	Joao Paulo Martins
 * Origin Date           :   05/05/2016
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
 *    Date      Version   		Author         		Description
 * 05/05/2016	1.0.0		Joao Paulo Martins   File created.
 *
 *****************************************************************************/
/** @file iap.h
 *  @brief This API refers to the feature of writing and erasing sectors and
 *  pages of the on-chip flash of a microcontroller, also known as " IN
 *  APPLICATION PROGRAMMING "
 *
 */

#ifndef ABS_INC_IAP_H_
#define ABS_INC_IAP_H_

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
 * Function : IAP_eInit(void)
 *//**
 * \b Description:
 * This function initializes and prepare the flash memory for erase and write operations.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: IAP commands enabled
 *
 * @return Status of the operation
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * IAP_eInit();
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_eInit (void);

/******************************************************************************
 * Function : IAP_ePrepareForReadWrite(uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank);
 *//**
 * \b Description:
 * This command must be executed before erasing and writing the flash memory, in order to avoid
 * the sectors to be protected, condition of the sectors after the operations.
 *
 * PRE-CONDITION: IAP commands initialized
 *
 * POST-CONDITION: Flash sectors ready to be erased or written.
 *
 * @return CMD_SUCCESS, BUSY or INVALID_SECTOR
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define START_SECTOR_NUM 	2
 * #define END_SECTOR_NUM 		3
 * #define FLASH_BANK 				0
 *
 *	eMCUError_s status;
 *	status = IAP_ePrepareForReadWrite(START_SECTOR_NUM, END_SECTOR_NUM, FLASH_BANK);
 * if (status == MCU_ERROR_SUCESS)
 * 	printf("Sectors ready to be erased/written");
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_ePrepareForReadWrite (uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank);

/******************************************************************************
 * Function : IAP_eCopyRamToFlash(uint32_t wDstAdd, uint32_t *wpSrcAdd, uint32_t wByteswrt);
 *//**
 * \b Description:
 *	This command is used to program the flash memory, copying data stored in RAM memory.
 *	After the operation, the affected sectors became protected against new write operations.
 *
 * PRE-CONDITION: Destination address should be a 512 boundary and the number of bytes equal to 512 or 1024 or 4096.
 * 							 Should be prepared first calling the IAP_ePrepareForReadWrite function.
 *
 * POST-CONDITION: Flash sectors with new data.
 *
 * @return Status of the operation following eMCUError codes.
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define SECTOR3 3
 * #define SECTOR3_ADDR 0x1000000
 * #define PAGESIZE 512
 * eMCUError_s status1, status2;
 *
 * uint8_t *ram_data;
 * ram_data = malloc(PAGESIZE * sizeof(uint8_t));
 * memset(ram_data, 0x00, PAGE_SIZE);
 *
 * IAP_eInit();
 * status1 = IAP_ePrepareForReadWrite(SECTOR3, SECTOR3, 0);
 *
 * if (status1 == MCU_ERROR_SUCCESS)
 * {
 * 	status2 = IAP_eCopyRamToFlash(SECTOR3_ADDR, ram_data, PAGE_SIZE);
 * 	if (status2 == MCU_ERROR_SUCCESS)
 * 		printf("SUCCESS");
 * }
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_eCopyRamToFlash (uint32_t wDstAdd, uint32_t *wpSrcAdd, uint32_t wByteswrt);

/******************************************************************************
 * Function : IAP_eEraseSector(uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank);
 *//**
 * \b Description:
 *	This command is used to erase a sector or multiple sectors of on-chip flash.
 *	To erase a single sector use the same number for start and end sector numbers.
 *
 * PRE-CONDITION: Destination address should be a 512 boundary and the number of bytes equal to 512 or 1024 or 4096.
 * 							 Should be prepared first calling the IAP_ePrepareForReadWrite function.
 *
 * POST-CONDITION: Blank sectors.
 *
 * @return Status of the operation following eMCUError codes.
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define SECTOR3 3
 * #define SECTOR4 4
 * eMCUError_s status1, status2;
 *
 * IAP_eInit();
 * status1 = IAP_ePrepareForReadWrite(SECTOR3, SECTOR4, 0);
 *
 * if (status1 == MCU_ERROR_SUCCESS)
 * {
 * 	status2 = IAP_eEraseSector(SECTOR3, SECTOR4, 0);
 *
 * 	if (status2 == MCU_ERROR_SUCCESS)
 * 		printf("SUCCESS");
 * }
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_eEraseSector (uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank);

/******************************************************************************
 * Function : eMCUError_s IAP_eBlankCheckSector(uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank);
 *//**
 * \b Description:
 *	This command is used to check if a sector or multiple sectors are blank.
 *
 * PRE-CONDITION: IAP commands initialized and valid sector numbers;
 *
 * POST-CONDITION: Blank sectors.
 *
 * @return Status of the operation, according to eMCUError codes.
 * 				(MCU_ERROR_SUCCESS or MCU_ERROR_IAP_SECTOR_NOT_BLANK)
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define SECTOR3 3
 * #define SECTOR4 4
 * eMCUError_s status1;
 *
 * IAP_eInit();
 *
 * status1 = IAP_eBlankCheckSector(SECTOR3, SECTOR4, 0);
 * if (status1 == MCU_ERROR_SUCCESS)
 * {
 * 		printf("SUCCESS");
 * }
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
/*TODO check return value of non-blank sectors */
eMCUError_s IAP_eBlankCheckSector (uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank);

/******************************************************************************
 * Function : IAP_wReadPID()
 *//**
 * \b Description:
 *	Read part PID number, an unique 32-bit bit number that identifies the component.
 *
 * PRE-CONDITION: IAP commands initialized
 *
 * POST-CONDITION: none
 *
 * @return Part PID number (32 bit)
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wPID;
 * IAP_eInit();
 *
 * wPID = IAP_wReadPID();
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t IAP_wReadPID ();

/******************************************************************************
 * Function : IAP_wReadBootCode();
 *//**
 * \b Description:
 *	Read part boot code number, a 32-bit value that identifies the version of the on-chip boot code.
 *
 * PRE-CONDITION: IAP commands initialized
 *
 * POST-CONDITION: none
 *
 * @return Boot code number (32 bit)
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint32_t wBcode;
 * IAP_eInit();
 *
 * wBcode = IAP_wReadBootCode();
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
uint32_t IAP_wReadBootCode ();

/******************************************************************************
 * Function : IAP_eCompare(uint32_t dstAdd, uint32_t srcAdd, uint32_t bytescmp);
 *//**
 * \b Description:
 *	This command is used to compare two regions of the flash or RAM memory, checking for
 *	equal contents on both areas.
 *
 * PRE-CONDITION: The source and destination address should be a word boundary.
 * 							 The number of bytes should be multiple of 4.
 *
 * POST-CONDITION: none
 *
 * @return MCU_ERROR_SUCCESS in case of equal areas.
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define SECTOR3 3
 * #define SECTOR4 4
 * #define PAGESIZE 512
 * eMCUError_s status;
 *
 * IAP_eInit();
 *
 * status = IAP_eCompare(SECTOR3, SECTOR4, PAGESIZE);
 * if (status == MCU_ERROR_SUCCESS)
 * {
 * 		printf("Both memory areas are equal");
 * }
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_eCompare (uint32_t dstAdd, uint32_t srcAdd, uint32_t bytescmp);

/******************************************************************************
 * Function : IAP_eReinvokeISP();
 *//**
 * \b Description:
 *	This command is used to invoke the ROM-based boot loader in ISP mode.
 *
 * PRE-CONDITION: none
 *
 * POST-CONDITION: The part will restart and enter the ISP boot loader function.
 *
 * @return none
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * IAP_eReinvokeISP();
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_eReinvokeISP ();
uint32_t IAP_wReadUID ();

/******************************************************************************
 * Function : eMCUError_s IAP_eErasePage(uint32_t strPage, uint32_t endPage);
 *//**
 * \b Description:
 *	This command allows to erase one or more pages (512 bytes) from one sector
 *	of the on-chip flash;
 *
 * PRE-CONDITION: IAP commands initialized and valid address numbers;
 *
 * POST-CONDITION: Blank pages of the selected memory space.
 *
 * @return Status of the operation, according to eMCUError codes.
 * 				(MCU_ERROR_SUCCESS or error codes)
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * #define PAGE1 0x10000000
 * #define PAGE2 0x10000200 //512 b
 * #define PAGE3 0x10000400 //512 b
 *
 * eMCUError_s status1;
 *
 * IAP_eInit();
 *
 * status1 = IAP_eErasePage(PAGE1, PAGE1);
 * if (status1 == MCU_ERROR_SUCCESS)
 * {
 * 		printf("One page erased");
 * }
 *
 * status1 = IAP_eErasePage(PAGE2, PAGE3);
 * if (status1 == MCU_ERROR_SUCCESS)
 * {
 * 		printf("Two pages erased");
 * }
 *
 *
 ~~~~~~~~~~~~~~~
 *
 * @see IAP_ePrepareForReadWrite, IAP_eCopyRamToFlash, IAP_eEraseSector
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
eMCUError_s IAP_eErasePage (uint32_t strPage, uint32_t endPage);

eMCUError_s IAP_eSetBootFlashBank (uint8_t bankNum);
uint32_t IAP_wGetMemoryBank (uint32_t wAbsoluteAddrm);
uint32_t IAP_wGetSectorNum (uint32_t wAbsoluteAddrm);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ABS_INC_IAP_H_ */
