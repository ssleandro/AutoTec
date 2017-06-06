/****************************************************************************
 * Title                 :   IN APPLICATION PROGRAMMING API
 * Filename              :   iap.c
 * Author                :   Joao Paulo Martins
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
 *    Date    Version        Author         		Description
 *  05/05/16   1.0.0    Joao Paulo Martins	File created
 *
 *****************************************************************************/
/** @file api.c
 *  @brief This module provides the functionality of write and erase sectors
 *  and pages of the on-chip flash memory.
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/

#include "iap.h"
#include "board.h"
#include "chip.h"

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
#define ONCHIP_NUM_SECTORS 15
#define ONCHIP_SECTOR_A			0
#define ONCHIP_SECTOR_B			1
#define ONCHIP_SECTOR_INVALID		2
#define INVALID_VALUE			0xFFFFFFFF

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
/* ************************ IAP ERROR CODES ******************************* */
/**
 * This private MACRO holds all the mapping between particular MCU IAP error codes
 * and the eMCUError error codes
 */
#define IAP_ERROR_CODES_MAPPING \
  	X( 0,  MCU_ERROR_SUCCESS ) 				\
		X( 1,  MCU_ERROR_IAP_INVALID_COMMAND ) 		\
		X( 2,  MCU_ERROR_IAP_SRC_ADDR_ERROR ) 			\
		X( 3,  MCU_ERROR_IAP_DST_ADDR_ERROR ) 			\
		X( 4,  MCU_ERROR_IAP_SRC_ADDR_NOT_MAPPED ) \
		X( 5,  MCU_ERROR_IAP_DST_ADDR_NOT_MAPPED ) \
		X( 6,  MCU_ERROR_IAP_COUNT_ERROR ) 				\
		X( 7,  MCU_ERROR_IAP_INVALID_SECTOR ) 			\
		X( 8,  MCU_ERROR_IAP_SECTOR_NOT_BLANK ) 		\
		X( 9,  MCU_ERROR_IAP_SECTOR_NOT_PREPARED ) \
		X( 10, MCU_ERROR_IAP_COMPARE_ERROR ) 			\
		X( 11, MCU_ERROR_IAP_BUSY ) 							\
		X( 12, MCU_ERROR_IAP_PARAM_ERROR ) 				\
		X( 13, MCU_ERROR_IAP_ADDR_ERROR ) 				\
		X( 14, MCU_ERROR_IAP_ADDR_NOT_MAPPED ) 		\
		X( 15, MCU_ERROR_IAP_CMD_LOCKED )				 	\
		X( 16, MCU_ERROR_IAP_INVALID_CODE ) 			\
		X( 17, MCU_ERROR_IAP_INVALID_BAUD_RATE ) 	\
		X( 18, MCU_ERROR_IAP_INVALID_STOP_BIT ) 	\
		X( 19, MCU_ERROR_IAP_CRP_ENABLED )

#define LPC4357_SECTORS \
	X(0,  SECTOR_A0_START, 	SECTOR_A0_END, 	SECTOR_B0_START, 	SECTOR_B0_END)		\
	X(1,  SECTOR_A1_START, 	SECTOR_A1_END, 	SECTOR_B1_START, 	SECTOR_B1_END)		\
	X(2,  SECTOR_A2_START, 	SECTOR_A2_END, 	SECTOR_B2_START, 	SECTOR_B2_END)		\
	X(3,  SECTOR_A3_START, 	SECTOR_A3_END, 	SECTOR_B3_START, 	SECTOR_B3_END)		\
	X(4,  SECTOR_A4_START, 	SECTOR_A4_END, 	SECTOR_B4_START, 	SECTOR_B4_END)		\
	X(5,  SECTOR_A5_START, 	SECTOR_A5_END, 	SECTOR_B5_START, 	SECTOR_B5_END)		\
	X(6,  SECTOR_A6_START, 	SECTOR_A6_END, 	SECTOR_B6_START, 	SECTOR_B6_END)		\
	X(7,  SECTOR_A7_START, 	SECTOR_A7_END, 	SECTOR_B7_START, 	SECTOR_B7_END)		\
	X(8,  SECTOR_A8_START, 	SECTOR_A8_END, 	SECTOR_B8_START, 	SECTOR_B8_END)		\
	X(9,  SECTOR_A9_START, 	SECTOR_A9_END, 	SECTOR_B9_START, 	SECTOR_B9_END)		\
	X(10, SECTOR_A10_START, SECTOR_A10_END, SECTOR_B10_START, SECTOR_B10_END)		\
	X(11, SECTOR_A11_START, SECTOR_A11_END, SECTOR_B11_START, SECTOR_B11_END)		\
	X(12, SECTOR_A12_START, SECTOR_A12_END, SECTOR_B12_START, SECTOR_B12_END)		\
	X(13, SECTOR_A13_START, SECTOR_A13_END, SECTOR_B13_START, SECTOR_B13_END)		\
	X(14, SECTOR_A14_START, SECTOR_A14_END, SECTOR_B14_START, SECTOR_B14_END)		\

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
/* ************************* IAP COMMANDS ****************************/
//#define X(IAP_CODE, MCU_CODE) [IAP_CODE] = MCU_CODE,
#define X(IAP_CODE, MCU_CODE) MCU_CODE,
const eMCUError_s eaIAP_ReturnCode[] = {
IAP_ERROR_CODES_MAPPING
};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = AS,
const uint32_t wSectorA_StartAddr[] = {
LPC4357_SECTORS
	};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = BS,
const uint32_t wSectorB_StartAddr[] = {
LPC4357_SECTORS
	};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = AE,
const uint32_t wSectorA_EndAddr[] = {
LPC4357_SECTORS
	};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = BE,
const uint32_t wSectorB_EndAddr[] = {
LPC4357_SECTORS
	};
#undef X

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

/* Prepares the IAP mechanism */
eMCUError_s IAP_eInit (void)
{
	uint8_t bIAPCode = Chip_IAP_init();
	return eaIAP_ReturnCode[bIAPCode];
}

/* Prepare sector for write/erase operations - this must be executed before these ops */
eMCUError_s IAP_ePrepareForReadWrite (uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank)
{
	uint8_t bIAPCode = Chip_IAP_PreSectorForReadWrite(wStrSector, wEndSector, bFlashBank);
	return eaIAP_ReturnCode[bIAPCode];
}

/* Copy RAM to flash - effectively write memory */
eMCUError_s IAP_eCopyRamToFlash (uint32_t wDstAdd, uint32_t *wpSrcAdd, uint32_t wByteswrt)
{
	uint8_t bIAPCode = Chip_IAP_CopyRamToFlash(wDstAdd, wpSrcAdd, wByteswrt);
	return eaIAP_ReturnCode[bIAPCode];
}

/* Erase sector */
eMCUError_s IAP_eEraseSector (uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank)
{
	uint8_t bIAPCode = Chip_IAP_EraseSector(wStrSector, wEndSector, bFlashBank);
	return eaIAP_ReturnCode[bIAPCode];
}

/* Blank check sector */
eMCUError_s IAP_eBlankCheckSector (uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank)
{
	uint8_t bIAPCode = Chip_IAP_BlankCheckSector(wStrSector, wEndSector, bFlashBank);
	return eaIAP_ReturnCode[bIAPCode];
}

/* Read part identification number */
uint32_t IAP_wReadPID ()
{
	return Chip_IAP_ReadPID();
}

/* Read boot code version number */
uint32_t IAP_wReadBootCode ()
{
	return Chip_IAP_ReadBootCode();
}

/* IAP compare memory areas, RAM or flash */
eMCUError_s IAP_eCompare (uint32_t dstAdd, uint32_t srcAdd, uint32_t bytescmp)
{
	uint8_t bIAPCode = Chip_IAP_Compare(dstAdd, srcAdd, bytescmp);
	return eaIAP_ReturnCode[bIAPCode];
}

/* Re invoke ISP */
eMCUError_s IAP_eReinvokeISP ()
{
	uint8_t bIAPCode = Chip_IAP_ReinvokeISP();
	return eaIAP_ReturnCode[bIAPCode];
}

/* Read the unique ID */
uint32_t IAP_wReadUID ()
{
	return Chip_IAP_ReadUID();
}

/* Erase page */
eMCUError_s IAP_eErasePage (uint32_t strPage, uint32_t endPage)
{
	uint8_t bIAPCode = Chip_IAP_ErasePage(strPage, endPage);
	return eaIAP_ReturnCode[bIAPCode];
}

/* Set active boot flash bank */
eMCUError_s IAP_eSetBootFlashBank (uint8_t bankNum)
{
	uint8_t bIAPCode = Chip_IAP_SetBootFlashBank(bankNum);
	return eaIAP_ReturnCode[bIAPCode];
}

/* TODO: evaluate the return of INVALID_VALUE when addr is not valid */

uint32_t IAP_wGetMemoryBank (uint32_t wAbsoluteAddrm)
{
	/* Checks the bank of the requested address */
	if ((wAbsoluteAddrm >= SECTOR_A0_START) && (wAbsoluteAddrm <= SECTOR_A14_END))
	{
		return ONCHIP_SECTOR_A;
	}
	else if ((wAbsoluteAddrm >= SECTOR_B0_START) && (wAbsoluteAddrm <= SECTOR_B14_END))
	{
		return ONCHIP_SECTOR_B;
	}
	else
	{
		/* Address is not on the on-chip flash space */
		return ONCHIP_SECTOR_INVALID;
	}
}

uint32_t IAP_wGetSectorNum (uint32_t wAbsoluteAddrm)
{
	uint32_t bSector = INVALID_VALUE;
	uint8_t bFlashBank = IAP_wGetMemoryBank(wAbsoluteAddrm);

	/* Checks if the address is valid */
	if (bFlashBank == ONCHIP_SECTOR_INVALID)
		return INVALID_VALUE;

	/* Valid address, find the corresponding sector number */
	if (bFlashBank == ONCHIP_SECTOR_A)
	{
		for (bSector = 0; bSector < ONCHIP_NUM_SECTORS; bSector++)
		{
			/* Check the index of the sector */
			if ((wAbsoluteAddrm >= wSectorA_StartAddr[bSector]) &&
				(wAbsoluteAddrm <= wSectorA_EndAddr[bSector]))
			{
				break;
			}
		}
	}

	if (bFlashBank == ONCHIP_SECTOR_B)
	{
		for (bSector = 0; bSector < ONCHIP_NUM_SECTORS; bSector++)
		{
			/* Check the index of the sector */
			if ((wAbsoluteAddrm >= wSectorB_StartAddr[bSector]) &&
				(wAbsoluteAddrm <= wSectorB_EndAddr[bSector]))
			{
				break;
			}
		}
	}

	return bSector;
}

