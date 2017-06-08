/* file: ftl_read.c */
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

/* Read */
#ifdef __KERNEL__
#include <linux/span/FTL/ftl_def.h>
#include <linux/span/FTL/ftl_calc.h>
#include <linux/span/FTL/ftl_common.h>
#include <linux/span/FTL/ftl_if_in.h>
#include <linux/span/FTL/ftl_if_ex.h>
#else
#include "ftl_def.h"
#include "ftl_calc.h"
#include "ftl_common.h"
#include "ftl_if_in.h"
#include "ftl_if_ex.h"
#endif // #ifdef __KERNEL__

//--------------------------------------------------------------
FTL_STATUS FTL_DeviceObjectsRead ( UINT8_PTR buffer, UINT32 LBA, UINT32 NB, UINT32_PTR bytesDone ) /*4,4,4,4*/
{
	UINT32 numEntries = 0; /*4*/
	UINT32 result = FTL_ERR_PASS; /*4*/
	UINT16 currentEntry = 0; /*2*/
	UINT32 remainingEntries = 0; /*4*/
	UINT32 currentLBA_range = 0; /*4*/
	UINT32 currentNBs = 0; /*4*/
	UINT32 NBs_left = 0; /*4*/
	UINT32 previousBufferValue = 0; /*4*/
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	UINT32 eblockBoundary = 0;
#endif
#if (DEBUG_ENABLE_LOGGING == FTL_TRUE)
	if((result = DEBUG_InsertLog(LBA, NB, DEBUG_LOG_READ)) != FTL_ERR_PASS)
	{
		return(result);
	}
#endif
	if((result = FTL_CheckRange(LBA, NB)) != FTL_ERR_PASS)
	{
		return(result);
	}
	if((result = FTL_CheckPointer(buffer)) != FTL_ERR_PASS)
	{
		return(result);
	}
	if((result = FTL_CheckPointer(bytesDone)) != FTL_ERR_PASS)
	{
		return(result);
	}
	if((result = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
	{
		return(result);
	}

	if((result = ClearDeleteInfo()) != FTL_ERR_PASS)
	{
		return(result);
	}

	*bytesDone = 0;

#if(FTL_RPB_CACHE == FTL_TRUE)
	result = RPBCacheForRead(&buffer, &LBA, &NB, bytesDone);
	if(FTL_ERR_PASS != result)
	{
		FTL_ClearMTLockBit();
		return(result);
	}
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

	for(currentLBA_range = LBA, NBs_left = NB; NBs_left > 0;currentLBA_range += currentNBs, NBs_left -= currentNBs)
	{
		if(NBs_left > NUM_SECTORS_PER_EBLOCK)
		{
			currentNBs = NUM_SECTORS_PER_EBLOCK;
		}
		else
		{
			currentNBs = NBs_left;
		}

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
		eblockBoundary = NUM_SECTORS_PER_EBLOCK - (currentLBA_range % NUM_SECTORS_PER_EBLOCK);
		if (currentNBs > eblockBoundary)
		{
			currentNBs = eblockBoundary;
		}
#endif

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
		if (FTL_ERR_PASS != (result = CACHE_LoadEB((UINT8)((currentLBA_range)& DEVICE_BIT_MAP) >> DEVICE_BIT_SHIFT, (UINT16)(currentLBA_range / NUM_SECTORS_PER_EBLOCK), CACHE_READ_TYPE)))
		{
			return result;
		}
#endif

		result = FTL_BuildTransferMapForReadBlocking(currentLBA_range, currentNBs, &numEntries);
		if (FTL_ERR_PASS != result)
		{
			FTL_ClearMTLockBit();
			return(result); //exit the transfer
		}
		//Transfer Map is now built data is now in from HOST in the location pointed at by the bufffer pointer
		for(currentEntry = 0;currentEntry < numEntries; currentEntry++)
		{
			previousBufferValue = (UINT32)(buffer);
			if((result = FTL_TransferPageForRead(&buffer, &remainingEntries)) != FTL_ERR_PASS)
			{
				FTL_ClearMTLockBit();
				return(result); //exit the transfer
			}
			*bytesDone += ((UINT32)(buffer)) - previousBufferValue;
		} //end of inside for
	} //end of outside for

#if(FTL_RPB_CACHE == FTL_TRUE)
	for(currentLBA_range = RPBCacheReadGroup.LBA, NBs_left = RPBCacheReadGroup.NB; NBs_left > 0;currentLBA_range += currentNBs, NBs_left -= currentNBs)
	{
		if(NBs_left > NUM_SECTORS_PER_EBLOCK)
		{
			currentNBs = NUM_SECTORS_PER_EBLOCK;
		}
		else
		{
			currentNBs = NBs_left;
		}
		result = FTL_BuildTransferMapForReadBlocking(currentLBA_range, currentNBs, &numEntries);
		if (FTL_ERR_PASS != result)
		{
			FTL_ClearMTLockBit();
			return(result); //exit the transfer
		}
		//Transfer Map is now built data is now in from HOST in the location pointed at by the bufffer pointer
		for(currentEntry = 0;currentEntry < numEntries; currentEntry++)
		{
			previousBufferValue = (UINT32)(RPBCacheReadGroup.byteBuffer);
			if((result = FTL_TransferPageForRead(&RPBCacheReadGroup.byteBuffer, &remainingEntries)) != FTL_ERR_PASS)
			{
				FTL_ClearMTLockBit();
				return(result); //exit the transfer
			}
			*bytesDone += ((UINT32)(RPBCacheReadGroup.byteBuffer)) - previousBufferValue;
		} //end of inside for
	} //end of outside for
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

	FTL_ClearMTLockBit();
	return (result); //If we made it to this location then we have transfered all the pages without error
}

//--------------------------
FTL_STATUS FTL_BuildTransferMapForReadBlocking ( UINT32 LBA, UINT32 NB,
UINT32_PTR resultingEntries )
{
	UINT8 pageLoopCount = NUM_SECTORS_PER_PAGE; /*1*/
	UINT16 logicalEBNum = 0; /*2*/
	UINT32 status = FTL_ERR_PASS; /*2*/
	UINT16 phyEBNum = 0; /*2*/
	UINT32 totalPages = 0; /*4*/
	UINT32 currentPageCount = 0; /*4*/
	UINT32 phyPageAddr = EMPTY_DWORD; /*4*/
	UINT32 mergePage = EMPTY_DWORD; /*4*/
	UINT32 currentLBA = 0; /*4*/
	ADDRESS_STRUCT endPage =
	{	0,0,0}; /*6*/
	ADDRESS_STRUCT startPage =
	{	0,0,0}; /*6*/
	ADDRESS_STRUCT currentPage =
	{	0,0,0}; /*6*/

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	UINT16 chainEBNum = EMPTY_WORD; /*2*/
	CHAIN_INFO chainInfo =
	{	0, 0, 0, 0, 0}; /*10*/
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

#if (DEBUG_FTL_API_ANNOUNCE == 1)
	DBG_Printf("FTL_BuildTransferMapForReadBlocking: LBA = 0x%X, ", LBA, 0);
	DBG_Printf("NB = %d\n", NB, 0);
#endif  // #if (DEBUG_FTL_API_ANNOUNCE == 1)

	if((status = TRANS_ClearTransMap()) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = GetPageSpread(LBA, NB, &startPage, &totalPages, &endPage )) != FTL_ERR_PASS)
	{
		return status;
	}
	currentPage = startPage;
	currentLBA = LBA;
	for(currentPageCount = 0; currentPageCount < totalPages; currentPageCount++)
	{
#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
		chainInfo.isChained = FTL_FALSE;
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

		if((status = GetLogicalEBNum(currentPage.logicalPageNum, &logicalEBNum)) != FTL_ERR_PASS)
		{
			return status;
		}
		if(currentPageCount == (totalPages-1)) /*This will work even when there is a sub page to write, because totalPages-1 will equal 0*/
		{
			if(currentPageCount == 0)
			{
				currentPage.pageOffset = startPage.pageOffset;
			}
			else
			{
				currentPage.pageOffset = 0;
			}
			if(endPage.pageOffset == 0)
			{
				pageLoopCount = NUM_SECTORS_PER_PAGE;
			}
			else
			{
				pageLoopCount = endPage.pageOffset;
			}
		}
		/*If GC_Info has the data already, make sure phyPage GetPhyPage is not called*/
		phyEBNum = GetPhysicalEBlockAddr(currentPage.devID, logicalEBNum);
		if((status = GetPhyPageAddr(&currentPage, phyEBNum, logicalEBNum,
						&phyPageAddr)) != FTL_ERR_PASS)
		{
			return status;
		}

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
		chainEBNum = GetChainLogicalEBNum(currentPage.devID, logicalEBNum);
		if(chainEBNum != EMPTY_WORD)
		{
			chainInfo.isChained = FTL_TRUE;
			chainInfo.logChainToEB = chainEBNum;
			chainInfo.phyChainToEB = GetChainPhyEBNum(currentPage.devID, logicalEBNum);
			// this is a chained EB, lets check teh phyPageAddr
			if(phyPageAddr == PAGE_CHAINED)
			{
				if((status = GetPhyPageAddr(&currentPage, chainInfo.phyChainToEB, chainInfo.logChainToEB,
								&(chainInfo.phyPageAddr))) != FTL_ERR_PASS)
				{
					return status;
				}
				phyPageAddr = chainInfo.phyPageAddr;
			}
		}
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

		/*need this regardless*/
		if(phyPageAddr == EMPTY_DWORD)
		{
			/*we have a problem, page not found*/
		}
		if((status = UpdateTransferMap(currentLBA, &currentPage, &endPage, &startPage,
						totalPages, phyPageAddr, mergePage, FTL_FALSE, FTL_FALSE)) != FTL_ERR_PASS)
		{
			return status;
		}
		currentLBA = currentLBA + (pageLoopCount - currentPage.pageOffset);
		if((status = IncPageAddr( &currentPage )) != FTL_ERR_PASS)
		{
			return status;
		}
	} //end of for
	*resultingEntries = currentPageCount;
	return FTL_ERR_PASS;
}

//--------------------------------------------------------------------------
FTL_STATUS FTL_TransferPageForRead (UINT8_PTR *byteBuffer, UINT32_PTR remainingEntries)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT16 currentTransfer = 0; /*2*/
	UINT16 startIndex = 0; /*2*/
	UINT16 endIndex = 0; /*2*/
	FTL_DEV deviceNum = 0; /*1*/
	UINT8 dstStartSector = 0; /*1*/
	UINT8 numSectors = 0; /*1*/
	UINT32 physicalPage = EMPTY_DWORD; /*4*/
	UINT32 count = 0; /*4*/
	FLASH_PAGE_INFO flashPageInfo =
	{	0, 0,
		{	0, 0,}}; /*11*/

	if((status = FTL_GetNextTransferMapEntry(&currentTransfer, &startIndex, &endIndex)) != FTL_ERR_PASS )
	{
		return status;
	}
	dstStartSector = GetTMStartSector(currentTransfer);
	numSectors = GetTMNumSectors(currentTransfer);
	deviceNum = GetTMDevID(currentTransfer);
	physicalPage = GetTMPhyPage(currentTransfer);

#if (DEBUG_FTL_API_ANNOUNCE == 1)
	DBG_Printf("FTL_TransferPageForRead: deviceNum=%d, ", deviceNum, 0);
	DBG_Printf("physicalPage=0x%x, ", physicalPage, 0);
	DBG_Printf("dstStartSector=%d, ", dstStartSector, 0);
	DBG_Printf("numSectors=%d\n", numSectors, 0);
#endif  // #if DEBUG_GC_ANNOUNCE

	/*Check to see if the page is aligned*/
	flashPageInfo.byteCount = numSectors * SECTOR_SIZE;
	flashPageInfo.devID = deviceNum;
	flashPageInfo.vPage.pageOffset = dstStartSector * SECTOR_SIZE;
	flashPageInfo.vPage.vPageAddr = physicalPage;
	if (physicalPage == EMPTY_DWORD)
	{
		/*just FF fill*/
		for (count = SECTOR_SIZE * numSectors; count; count--)
		{
#if(AUTOFILL_0xFF_UNMAPPED_SECTORS == FTL_TRUE)
	*((*byteBuffer)++) = EMPTY_BYTE;
#else
	*((*byteBuffer)++) = 0;
#endif
}
}
else
{
dstStartSector = 0;
if((FLASH_RamPageReadDataBlock(&flashPageInfo, &((*byteBuffer)[SECTOR_SIZE * dstStartSector]))) != FLASH_PASS)
{
	return FTL_ERR_FLASH_READ_02;
}
(*byteBuffer) += (numSectors * SECTOR_SIZE);
}
*remainingEntries = endIndex - startIndex;
return FTL_ERR_PASS;
}
