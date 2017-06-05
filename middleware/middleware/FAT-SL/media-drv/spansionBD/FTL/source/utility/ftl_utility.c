// file: ftl_utility.c
/**************************************************************************
 * Copyright (C)2012 Spansion Inc. All Rights Reserved. 
 *
 * This software is owned and published by: 
 * Spansion Inc, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software constitutes driver source code for use in programming Spansion's 
 * Flash memory components. This software is licensed by Spansion to be adapted only 
 * for use in systems utilizing Spansion's Flash memories. Spansion is not be 
 * responsible for misuse or illegal use of this software for devices not 
 * supported herein.  Spansion is providing this source code "AS IS" and will 
 * not be responsible for issues arising from incorrect user implementation 
 * of the source code herein.  
 *
 * SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE, 
 * REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED 
 * USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY, 
 * FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  SPANSION WILL 
 * HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR 
 * OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE, 
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS, 
 * EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 *
 * This software may be replicated in part or whole for the licensed use, 
 * with the restriction that this Copyright notice must be included with 
 * this software, whether used in part or whole, at all times.  
 */

/* external APIs suppied to the outside world */

#ifdef __KERNEL__
#include <linux/span/FTL/ftl_def.h>
#include <linux/span/FTL/ftl_common.h>
#else
#include "ftl_def.h"
#include "ftl_common.h"
#endif // #ifdef __KERNEL__

//----------------------------
FTL_STATUS FTL_GetCapacity (FTL_CAPACITY * capacity)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/

	if ((status = FTL_CheckPointer(capacity)) != FTL_ERR_PASS)
	{
		return status;
	}
	if ((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
	{
		return status;
	}
	capacity->totalSize = NUM_DATA_EBLOCKS * NUMBER_OF_PAGES_PER_EBLOCK *
	NUMBER_OF_SECTORS_PER_PAGE * NUM_DEVICES * SECTOR_SIZE; //In Bytes
	capacity->numDBlocks = NUM_DATA_EBLOCKS * NUMBER_OF_PAGES_PER_EBLOCK *
	NUMBER_OF_SECTORS_PER_PAGE * NUM_DEVICES;
	capacity->numEBlocks = NUM_EBLOCKS_PER_DEVICE * NUM_DEVICES;
	capacity->eBlockSizeBytes = EBLOCK_SIZE;
	capacity->eBlockSizePages = NUM_PAGES_PER_EBLOCK;
	capacity->pageSizeDBlocks = NUM_SECTORS_PER_PAGE;
	capacity->pageSizeBytes = NUM_SECTORS_PER_PAGE * SECTOR_SIZE;
	capacity->numBlocks = capacity->numDBlocks;
	capacity->blockSize = capacity->eBlockSizeBytes; //NUM_EBLOCKS_PER_DEVICE * NUM_DEVICES;   // is equivalent to numEBlocks OBSOLETE
	capacity->pageSize = capacity->pageSizeBytes; //NUM_SECTORS_PER_PAGE * SECTOR_SIZE;     // is equivalent to pageSizeBytes OBSOLETE
	capacity->busWidth = 16;                           // hardcoded for now
	capacity->numDevices = NUM_DEVICES;
	FTL_ClearMTLockBit();
	return FTL_ERR_PASS;
}

//--------------------------------
FTL_STATUS FTL_GetUtilizationInfo (UINT32 DeviceID, UINT32_PTR Free, UINT32_PTR Used, UINT32_PTR Dirty )
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT16 numValidTemp = EMPTY_WORD; /*2*/
	UINT16 numInvalidTemp = EMPTY_WORD; /*2*/
	UINT16 numFreeTemp = EMPTY_WORD; /*2*/
	UINT16 numUsedTemp = EMPTY_WORD; /*2*/
	UINT16 logicalEBNum = 0; /*2*/

	if((status = FTL_CheckDevID((UINT8)DeviceID)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(Free)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(Used)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(Dirty)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
	{
		return status;
	}
	for (logicalEBNum = 0; logicalEBNum < NUM_DATA_EBLOCKS; logicalEBNum++)
	{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
		if (FTL_ERR_PASS != (status = CACHE_LoadEB((FTL_DEV)DeviceID, logicalEBNum, CACHE_READ_TYPE)))
		{
			return status;
		}
#endif
		*Used = 0;
		*Dirty = 0;
		*Free = 0;
		GetNumValidUsedPages((FTL_DEV)DeviceID, logicalEBNum, &numUsedTemp, &numValidTemp);
		numInvalidTemp = numUsedTemp - numValidTemp;
		numFreeTemp = NUM_PAGES_PER_EBLOCK - numUsedTemp;
		*Used += numValidTemp;
		*Dirty += numInvalidTemp;
		*Free += numFreeTemp;
	}
	FTL_ClearMTLockBit();
	return FTL_ERR_PASS;
}

//-------------------------------
FTL_STATUS FTL_GetEraseCycles (UINT32 DeviceID, UINT32_PTR high_block_number, UINT32_PTR high_erase_count,
UINT32_PTR low_block_number, UINT32_PTR low_erase_count )
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT32 eraseCount = EMPTY_DWORD; /*4*/
	UINT16 tempCount = EMPTY_WORD; /*2*/
	UINT32 highestEraseCount = EMPTY_DWORD; /*4*/
	UINT16 highestEraseEBNum = EMPTY_WORD; /*2*/
	UINT32 lowestEraseCount = 0; /*4*/
	UINT16 lowestEraseEBNum = 0; /*2*/

	if((status = FTL_CheckDevID((UINT8)DeviceID)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(high_block_number)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(high_erase_count)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(low_block_number)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(low_erase_count)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
	{
		return status;
	}
	for (tempCount = 0; tempCount < NUM_DATA_EBLOCKS; tempCount++)
	{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
		if (FTL_ERR_PASS != (status = CACHE_LoadEB((FTL_DEV)DeviceID, tempCount, CACHE_READ_TYPE)))
		{
			return status;
		}
#endif

		eraseCount = GetTrueEraseCount((FTL_DEV)DeviceID, tempCount);
		if(eraseCount < highestEraseCount)
		{
			highestEraseCount = eraseCount;
			highestEraseEBNum = tempCount;
		}
		if(eraseCount > lowestEraseCount)
		{
			lowestEraseCount = eraseCount;
			lowestEraseEBNum = tempCount;
		}
	}
	*high_block_number = highestEraseEBNum;
	*high_erase_count = highestEraseCount;
	*low_block_number = lowestEraseEBNum;
	*low_erase_count = lowestEraseCount;
	FTL_ClearMTLockBit();
	return FTL_ERR_PASS;
}

//--------------------------------
FTL_STATUS TST_GetDevConfig (FTL_DEV_CONFIG* devConfig)
{
	devConfig->dataEBlkStart = 0;
	devConfig->dataEBlkEnd = NUM_DATA_EBLOCKS - 1;
	devConfig->reserveEBlkStart = NUM_DATA_EBLOCKS;
	devConfig->reserveEBlkEnd = NUMBER_OF_ERASE_BLOCKS - 1;
	devConfig->sysEBlkStart = NUM_DATA_EBLOCKS;
	devConfig->sysEBlkEnd = NUMBER_OF_ERASE_BLOCKS - 1;
	devConfig->numDevices = NUM_DEVICES;
	devConfig->sectorsPerPage = NUM_SECTORS_PER_PAGE;
	devConfig->dataPagesPerEBlock = NUM_PAGES_PER_EBLOCK;
	devConfig->sysPagesPerEBlock = 0;

#if (FTL_EBLOCK_CHAINING == FTL_TRUE)
	devConfig->eblockChainingEnabled = 1;

#else  // #if (FTL_EBLOCK_CHAINING == FTL_TRUE)
	devConfig->eblockChainingEnabled = 0;
#endif  // #else  // #if (FTL_EBLOCK_CHAINING == FTL_TRUE)

	return FTL_ERR_PASS;
}

//-------------------------------
FTL_STATUS TST_GetPhysicalEBEraseCount (FTL_DEV devID, UINT16 phyEBNum, UINT32_PTR eraseCount)
{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
#endif
	UINT16 logicalEBNum = EMPTY_WORD; /*2*/

	*eraseCount = EMPTY_WORD;
	for (logicalEBNum = 0; logicalEBNum < NUM_EBLOCKS_PER_DEVICE; logicalEBNum++)
	{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
		if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum, CACHE_READ_TYPE)))
		{
			return status;
		}
#endif
		if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID, logicalEBNum))
		{
			continue;
		}
		if (GetPhysicalEBlockAddr(devID, logicalEBNum) == phyEBNum)
		{
			*eraseCount = GetTrueEraseCount(devID, logicalEBNum);
			break;
		}
	}
	return FTL_ERR_PASS;
}

//------------------------------------
FTL_STATUS TST_GetLogicalEBEraseCount (FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR physicalEBNum, UINT32_PTR eraseCount)
{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum, CACHE_READ_TYPE)))
	{
		return status;
	}
#endif
	*physicalEBNum = GetPhysicalEBlockAddr(devID, logicalEBNum);
	*eraseCount = GetTrueEraseCount(devID, logicalEBNum);
	return FTL_ERR_PASS;;
}

//-------------------------------
FTL_STATUS TST_GetAverageEraseCount (FTL_DEV devID, UINT16 Start_Logical_eBlk,
UINT16 End_Logical_eBlk, UINT16_PTR avgEraseCount)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT32 totalEraseCount = 0; /*4*/
	UINT32 totalEraseBlocks = 0; /*4*/
	UINT16 logicalEBNum = EMPTY_WORD; /*2*/
	UINT16 physicalEBNum = EMPTY_WORD; /*2*/
	UINT32 eraseCount = 0; /*2*/
	UINT16 start = 0; /*2*/
	UINT16 end = 0; /*2*/

	totalEraseCount = 0;
	totalEraseBlocks = 0;
	start = Start_Logical_eBlk;
	end = End_Logical_eBlk;
	if (EMPTY_WORD == start)
	{
		start = 0;
	}
	if (EMPTY_WORD == end)
	{
		end = NUMBER_OF_ERASE_BLOCKS;
	}
	for (logicalEBNum = start; logicalEBNum < end; logicalEBNum++)
	{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
		if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum, CACHE_READ_TYPE)))
		{
			return status;
		}
#endif
		status = TST_GetLogicalEBEraseCount(devID, logicalEBNum, &physicalEBNum, &eraseCount);
		if (status != FTL_ERR_PASS)
		{
			return status;
		}
		totalEraseCount += (UINT32) eraseCount;
		totalEraseBlocks++;
	}
	*avgEraseCount = (UINT16) (totalEraseCount / totalEraseBlocks);
	return FTL_ERR_PASS;
}

//--------------------------------------------
FTL_STATUS TST_CheckEraseCount (FTL_DEV devID, UINT16 Start_Logical_eBlk, UINT16 End_Logical_eBlk,
	UINT16 expected_value)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT16 blkCount = 0; /*2*/
	UINT16 phyEBNum = EMPTY_WORD; /*2*/
	UINT32 eraseCount = EMPTY_DWORD; /*2*/

	if (End_Logical_eBlk == EMPTY_WORD)
	{
		End_Logical_eBlk = Start_Logical_eBlk;
	}
	for (blkCount = Start_Logical_eBlk; blkCount <= End_Logical_eBlk; blkCount++)
	{
		if ((status = TST_GetLogicalEBEraseCount(devID, blkCount, &phyEBNum, &eraseCount)) != FTL_ERR_PASS)
		{
			return status;
		}
		if (eraseCount != expected_value)
		{
			DBG_Printf("ERROR- TST_CheckEraseCount: logBlk = 0x%X, ", blkCount, 0);
			DBG_Printf("phyBlk = 0x%X, ", phyEBNum, 0);
			DBG_Printf("expectedEC = %d, ", expected_value, 0);
			DBG_Printf("actualEC = %d\n", eraseCount, 0);
			return FTL_ERR_TST_ERASE_CNT_ERR;
		}
	}
	return FTL_ERR_PASS;
}

//---------------------------------------
FTL_STATUS TST_LBAToPBA (UINT32 LBA, PBA_STRUCT_PTR pbaStruct, BOOL verifyFlash)
{
	FLASH_PAGE_INFO pageInfo = { 0, 0, { 0, 0 } }; /*11*/
	UINT16 phyPageIndex = EMPTY_WORD; /*2*/
	UINT16 dataByte = 0; /*2*/
	UINT16 logicalEBNum = EMPTY_WORD; /*2*/
	UINT16 phyEBlockAddr = EMPTY_WORD; /*2*/
	UINT16 pageOffset = EMPTY_WORD; /*2*/
	UINT32 logicalPageAddr = EMPTY_DWORD; /*4*/

	pbaStruct->devID = EMPTY_BYTE;
	pbaStruct->PBA = EMPTY_DWORD;
	pbaStruct->PPA = EMPTY_DWORD;
	GetDevNum(LBA, &pbaStruct->devID);
	GetPageNum(LBA, (UINT32_PTR)&logicalPageAddr);
	GetSectorNum(LBA, (UINT8_PTR)&pageOffset);
	GetLogicalEBNum(logicalPageAddr, (UINT16_PTR)&logicalEBNum);
	phyPageIndex = GetPPASlot(pbaStruct->devID, logicalEBNum, (UINT16)logicalPageAddr);
	if (phyPageIndex == EMPTY_INVALID)
	{
		return FTL_ERR_TST_PAGE_NOT_FOUND;
	}
	phyEBlockAddr = GetPhysicalEBlockAddr(pbaStruct->devID, logicalEBNum);
	pbaStruct->PPA = CalcPhyPageAddrFromPageOffset(phyEBlockAddr, phyPageIndex);
	pbaStruct->PBA = pbaStruct->PPA + pageOffset;
	// Verify the LBA 
	if (verifyFlash == FTL_TRUE)
	{
		pageInfo.devID = pbaStruct->devID;
		pageInfo.byteCount = SECTOR_SIZE;
		pageInfo.vPage.vPageAddr = pbaStruct->PPA;
		pageInfo.vPage.pageOffset = pageOffset * SECTOR_SIZE;
		if ((FLASH_RamPageReadDataBlock(&pageInfo,
			(UINT8_PTR)&pseudoRPB[pageInfo.devID][pageInfo.vPage.pageOffset])) != FLASH_PASS)
		{
			return FTL_ERR_FLASH_READ_01;
		}
		for (dataByte = 0; dataByte < SECTOR_SIZE; dataByte++)
		{
			if (pseudoRPB[pageInfo.devID][pageInfo.vPage.pageOffset + dataByte] != EMPTY_BYTE)
			{
				return FTL_ERR_PASS;
			}
		}
		return FTL_ERR_TST_LBA_MISMATCH;
	}
	return FTL_ERR_PASS;
}

//--------------- TST_ReadDefectList -------------------------------
FTL_STATUS TST_ReadDefectList (FTL_DEFECT* buffer)
{
	// No defects in NOR Flash
	buffer[0].devID = EMPTY_WORD;
	buffer[0].phyEBNum = EMPTY_WORD;
	return FTL_ERR_PASS;
}

FTL_STATUS TST_WriteDefectList (FTL_DEFECT* buffer, UINT16 replace)
{
	FTL_DEFECT defect = { 0, 0 }; /*4*/

	// No defects in NOR Flash
	defect.devID = buffer[0].devID;
	defect.phyEBNum = buffer[0].phyEBNum | replace;
	return FTL_ERR_PASS;
}

//------ TST_ResetFlash -----
/*This function will reset the flash device immediately without waiting for any pending operations to complete.*/
/*This function will not wait for the device to become ready.It will return immediately.*/
FLASH_STATUS TST_ResetFlash (UINT32 devId)
{
	// Stop any Transfer in Progress in HW specific fashion
	return FLASH_PASS;
}
