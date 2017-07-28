/*
 * @brief Common FLASH IAP support functions
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "chip.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
static unsigned int iapCommands[6]; // @ 0x10080000;
static unsigned int iapResults[4]; // @ 0x10080018;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

uint8_t Chip_IAP_init (void)
{
#if 0
	uint32_t command[5], result[4];
	command[0] = IAP_INIT_CMD;
	SystemCoreClockUpdate();
	iap_entry(command, result);
	return result[0];
#endif

	iapCommands[0] = IAP_INIT_CMD;
	SystemCoreClockUpdate();
	iap_entry(iapCommands, iapResults);
	return iapResults[0];
}

/* Prepare sector for write operation */
uint8_t Chip_IAP_PreSectorForReadWrite (uint32_t strSector, uint32_t endSector, uint8_t flashBank)
{
#if 0
	uint32_t command[5], result[4];
	command[0] = IAP_PREWRRITE_CMD;
	command[1] = strSector;
	command[2] = endSector;
	command[3] = flashBank;
	iap_entry(command, result);
	return result[0];
#endif

	iapCommands[0] = IAP_PREWRRITE_CMD;
	iapCommands[1] = strSector;
	iapCommands[2] = endSector;
	iapCommands[3] = flashBank;
	iap_entry(iapCommands, iapResults);
	return iapResults[0];
}

/* Copy RAM to flash */
uint8_t Chip_IAP_CopyRamToFlash (uint32_t dstAdd, uint32_t *srcAdd, uint32_t byteswrt)
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_WRISECTOR_CMD;
	command[1] = dstAdd;
	command[2] = (uint32_t) srcAdd;
	command[3] = byteswrt;
	command[4] = SystemCoreClock / 1000;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_WRISECTOR_CMD;
	iapCommands[1] = dstAdd;
	iapCommands[2] = (uint32_t)srcAdd;
	iapCommands[3] = byteswrt;
	iapCommands[4] = SystemCoreClock / 1000;
	iap_entry(iapCommands, iapResults);

	return iapResults[0];
}

/* Erase sector */
uint8_t Chip_IAP_EraseSector (uint32_t strSector, uint32_t endSector, uint8_t flashBank)
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_ERSSECTOR_CMD;
	command[1] = strSector;
	command[2] = endSector;
	command[3] = SystemCoreClock / 1000;
	command[4] = flashBank;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_ERSSECTOR_CMD;
	iapCommands[1] = strSector;
	iapCommands[2] = endSector;
	iapCommands[3] = SystemCoreClock / 1000;
	iapCommands[4] = flashBank;
	iap_entry(iapCommands, iapResults);

	return iapResults[0];

}

/* Blank check sector */
uint8_t Chip_IAP_BlankCheckSector (uint32_t strSector, uint32_t endSector, uint8_t flashBank)
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_BLANK_CHECK_SECTOR_CMD;
	command[1] = strSector;
	command[2] = endSector;
	command[3] = flashBank;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_BLANK_CHECK_SECTOR_CMD;
	iapCommands[1] = strSector;
	iapCommands[2] = endSector;
	iapCommands[3] = flashBank;
	iap_entry(iapCommands, iapResults);

	return iapResults[0];
}

/* Read part identification number */
uint32_t Chip_IAP_ReadPID ()
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_REPID_CMD;
	iap_entry(command, result);

	return result[1];
#endif

	iapCommands[0] = IAP_REPID_CMD;
	iap_entry(iapCommands, iapResults);

	return iapResults[1];
}

/* Read boot code version number */
uint32_t Chip_IAP_ReadBootCode ()
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_READ_BOOT_CODE_CMD;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_READ_BOOT_CODE_CMD;
	iap_entry(iapCommands, iapResults);

	return iapResults[1];
}

/* IAP compare */
uint8_t Chip_IAP_Compare (uint32_t dstAdd, uint32_t srcAdd, uint32_t bytescmp)
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_COMPARE_CMD;
	command[1] = dstAdd;
	command[2] = srcAdd;
	command[3] = bytescmp;
	iap_entry(command, result);
#endif

	iapCommands[0] = IAP_COMPARE_CMD;
	iapCommands[1] = dstAdd;
	iapCommands[2] = srcAdd;
	iapCommands[3] = bytescmp;
	iap_entry(iapCommands, iapResults);
	return iapResults[0];
}

/* Reinvoke ISP */
uint8_t Chip_IAP_ReinvokeISP ()
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_REINVOKE_ISP_CMD;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_REINVOKE_ISP_CMD;
	iap_entry(iapCommands, iapResults);

	return iapResults[0];
}

/* Read the unique ID */
uint32_t Chip_IAP_ReadUID ()
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_READ_UID_CMD;
	iap_entry(command, result);

	return result[1];
#endif

	iapCommands[0] = IAP_READ_UID_CMD;
	iap_entry(iapCommands, iapResults);

	return iapResults[1];
}

/* Erase page */
uint8_t Chip_IAP_ErasePage (uint32_t strPage, uint32_t endPage)
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_ERASE_PAGE_CMD;
	command[1] = strPage;
	command[2] = endPage;
	command[3] = SystemCoreClock / 1000;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_ERASE_PAGE_CMD;
	iapCommands[1] = strPage;
	iapCommands[2] = endPage;
	iapCommands[3] = SystemCoreClock / 1000;
	iap_entry(iapCommands, iapResults);
	return iapResults[0];
}

/* Set active boot flash bank */
uint8_t Chip_IAP_SetBootFlashBank (uint8_t bankNum)
{
#if 0
	uint32_t command[5], result[4];

	command[0] = IAP_SET_BOOT_FLASH;
	command[1] = bankNum;
	command[2] = SystemCoreClock / 1000;
	iap_entry(command, result);

	return result[0];
#endif

	iapCommands[0] = IAP_SET_BOOT_FLASH;
	iapCommands[1] = bankNum;
	iapCommands[2] = SystemCoreClock / 1000;
	iap_entry(iapCommands, iapResults);

	return iapResults[0];
}
