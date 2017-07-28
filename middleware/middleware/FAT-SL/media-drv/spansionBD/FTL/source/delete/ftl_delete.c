/* file: ftl_delete.c */
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

/* Delete */
#ifdef __KERNEL__
#include <linux/span/FTL/ftl_common.h>
#include <linux/span/FTL/ftl_if_in.h>
#include <linux/span/FTL/ftl_if_ex.h>
#else
#include "ftl_common.h"
#include "ftl_if_in.h"
#include "ftl_if_ex.h"
#endif // #ifdef __KERNEL__

//-------------------------------------------
FTL_STATUS FTL_DeviceObjectsDelete (UINT32 LBA, UINT32 NB, UINT32_PTR sectorsDeleted )
{
#if (FTL_ENABLE_DELETE_API == FTL_TRUE)
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT32 totalPages = 0; /*4*/
	UINT32 currentPageCount = 0; /*4*/
	UINT32 currentLogicalPageNum = 0; /*4*/
	ADDRESS_STRUCT endPage =
	{	0,0,0}; /*6*/
	ADDRESS_STRUCT startPage =
	{	0,0,0}; /*6*/
	ADDRESS_STRUCT currentPage =
	{	0,0,0}; /*6*/
	UINT16 currentLogicalEBNum = 0; /*2*/
	UINT16 logicalPageOffset = 0; /*2*/
	UINT8 isFullPage = FTL_FALSE; /*1*/
	UINT16 tempPPA = EMPTY_WORD; /*1*/
	UINT8 numSectors = 0; /*1*/

#if(FTL_DELETE_WITH_GC == FTL_TRUE)
	UINT16 logicalEBNum[NUM_DEVICES]; /*8*/
	UINT16 logicalGCEBNum = EMPTY_WORD; /*2*/
	UINT16 freedUpPages = EMPTY_WORD; /*2*/
	UINT16 freePageIndex = EMPTY_WORD; /*2*/
#endif  // #if(FTL_DELETE_WITH_GC == FTL_TRUE)

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	UINT16 chainEBNum = 0; /*2*/
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

#if(FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
	UINT8 devID = 0; /*1*/
	UINT16 sanityCounter = 0;
#endif
#endif  // #if(FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

#if (DEBUG_FTL_API_ANNOUNCE == 1)
	DBG_Printf("FTL_DeviceObjectsDelete: LBA = %d, ", LBA, 0);
	DBG_Printf("NB = %d, \n", NB, 0);

#endif  // #if (DEBUG_FTL_API_ANNOUNCE == 1)
#if (DEBUG_ENABLE_LOGGING == FTL_TRUE)
	if((status = DEBUG_InsertLog(LBA, NB, DEBUG_LOG_DELETE)) != FTL_ERR_PASS)
	{
		return(status);
	}
#endif

	if((status = FTL_CheckRange(LBA, NB)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckPointer(sectorsDeleted)) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
	{
		return status;
	}

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
	for(devID = 0; devID < NUMBER_OF_DEVICES; devID++)
	{
		while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
		{
			if((status = FTL_CheckForSuperSysEBLogSpace(devID, SYS_EBLOCK_INFO_CHANGED)) != FTL_ERR_PASS)
			{
				return status;
			}
			if((status = FTL_CreateSuperSysEBLog(devID, SYS_EBLOCK_INFO_CHANGED)) == FTL_ERR_PASS)
			{
				break;
			}
			if(status != FTL_ERR_SUPER_WRITE_02)
			{
				return status;
			}
			sanityCounter++;
		}
		if(sanityCounter >= MAX_BAD_BLOCK_SANITY_TRIES)
		{
			return status;
		}
	}
#endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

	if(NB > 0)
	{
		if((status = GetPageSpread(LBA, NB, &startPage, &totalPages, &endPage)) != FTL_ERR_PASS)
		{
			FTL_ClearMTLockBit();
			return status;
		}
		(*sectorsDeleted) = 0;
		currentPage = startPage;

		for(currentPageCount = 0; currentPageCount < totalPages; currentPageCount++)
		{
			currentLogicalPageNum = currentPage.logicalPageNum;
			if((status = GetLogicalEBNum(currentLogicalPageNum, &currentLogicalEBNum)) != FTL_ERR_PASS)
			{
				FTL_ClearMTLockBit();
				return status;
			}
			if((status = GetLogicalPageOffset(currentLogicalPageNum, &logicalPageOffset)) != FTL_ERR_PASS)
			{
				FTL_ClearMTLockBit();
				return status;
			}

			/* start page */
			if(currentPageCount == 0)
			{
				/* single page transfer */
				if(currentPageCount == (totalPages-1))
				{
					if((currentPage.pageOffset == 0) && (endPage.pageOffset == 0))
					{
						isFullPage = FTL_TRUE;
					}
					else
					{
						/*Don't delete the index because this is NOT a full page*/
						isFullPage = FTL_FALSE;
					}
					numSectors = (UINT8)NB;
				}
				/* multiple pages transfer */
				else
				{
					if(currentPage.pageOffset == 0)
					{
						isFullPage = FTL_TRUE;
					}
					else
					{
						/*Don't delete the index because this is NOT a full page*/
						isFullPage = FTL_FALSE;
					}
					numSectors = NUMBER_OF_SECTORS_PER_PAGE - currentPage.pageOffset;
				}

				if(isFullPage == FTL_FALSE)
				{
					if(HitDeleteInfo(currentPage.devID, currentLogicalPageNum) == FTL_ERR_PASS)
					{
						if((status = UpdateDeleteInfo(currentPage.pageOffset, numSectors)) != FTL_ERR_PASS)
						{
							return status;
						}
						if(GetDeleteInfoNumSectors() == NUMBER_OF_SECTORS_PER_PAGE)
						{
							isFullPage = FTL_TRUE;
						}
					}
				}

				if(isFullPage == FTL_TRUE)
				{
					if((status = ClearDeleteInfo()) != FTL_ERR_PASS)
					{
						return status;
					}
				}
				else
				{
					if(currentPageCount == (totalPages-1))
					{
						if(HitDeleteInfo(currentPage.devID, currentLogicalPageNum) != FTL_ERR_PASS)
						{
							if((status = ClearDeleteInfo()) != FTL_ERR_PASS)
							{
								return status;
							}
							status = InitDeleteInfo(currentPage.devID, currentLogicalPageNum,
							currentPage.pageOffset, numSectors);
							if(status != FTL_ERR_PASS)
							{
								return status;
							}
						}
					}
					else
					{
						if((status = ClearDeleteInfo()) != FTL_ERR_PASS)
						{
							return status;
						}
					}
				}
			}
			/* end page */
			else if(currentPageCount == (totalPages-1))
			{
				if(endPage.pageOffset == 0)
				{
					isFullPage = FTL_TRUE;
				}
				else
				{
					/*Don't delete the index because this is NOT a full page*/
					isFullPage = FTL_FALSE;

					status = InitDeleteInfo(currentPage.devID, currentLogicalPageNum,
					currentPage.pageOffset, endPage.pageOffset);
					if(status != FTL_ERR_PASS)
					{
						return status;
					}
				}
			}
			/* middle page */
			else
			{
				isFullPage = FTL_TRUE;
			}

			if(isFullPage == FTL_TRUE)
			{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	if (FTL_ERR_PASS != (status = CACHE_LoadEB(0, currentLogicalEBNum, CACHE_WRITE_TYPE)))
	{
		return status;
	}
#endif
	tempPPA = GetPPASlot(currentPage.devID, currentLogicalEBNum, logicalPageOffset);
	if(tempPPA != EMPTY_INVALID)
	{

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	if(tempPPA == CHAIN_INVALID)
	{
		chainEBNum = GetChainLogicalEBNum(currentPage.devID, currentLogicalEBNum);
		if(chainEBNum == EMPTY_WORD)
		{
			FTL_ClearMTLockBit();
			return FTL_ERR_ECHAIN_GC_DEL;
		}
		else
		{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	if (FTL_ERR_PASS != (status = CACHE_LoadEB(currentPage.devID, chainEBNum, CACHE_WRITE_TYPE)))
	{
		return status;
	}
#endif
	SetPPASlot(currentPage.devID, currentLogicalEBNum, logicalPageOffset, EMPTY_INVALID);
	MarkPPAMappingTableEntryDirty(currentPage.devID, currentLogicalEBNum, logicalPageOffset);
	UpdatePageTableInfo(currentPage.devID, chainEBNum,
	logicalPageOffset, EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
}
}
else
{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	if (FTL_ERR_PASS != (status = CACHE_LoadEB(currentPage.devID, currentLogicalEBNum, CACHE_WRITE_TYPE)))
	{
		return status;
	}
#endif
	UpdatePageTableInfo(currentPage.devID, currentLogicalEBNum,
	logicalPageOffset, EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
}

#else  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	UpdatePageTableInfo(currentPage.devID, currentLogicalEBNum,
	logicalPageOffset, EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
#endif  // #else  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

	(*sectorsDeleted) += NUM_SECTORS_PER_PAGE;
}

#if(FTL_RPB_CACHE == FTL_TRUE)
	if(currentLogicalPageNum == GetRPBCacheLogicalPageAddr(currentPage.devID))
	{
		ClearRPBCache(currentPage.devID);
	}
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)
}
if((status = IncPageAddr(&currentPage)) != FTL_ERR_PASS)
{
	FTL_ClearMTLockBit();
	return status;
}
}

#if(FTL_DELETE_WITH_GC == FTL_TRUE)
	/*erase eblks*/
	for(currentPageCount = 0; currentPageCount < NUM_DEVICES; currentPageCount++)
	{
		logicalEBNum[currentPageCount] = EMPTY_WORD;
	}
	currentPage = startPage;
	for(currentPageCount = 0; currentPageCount < totalPages; currentPageCount++)
	{
		GetLogicalEBNum(currentPage.logicalPageNum, &currentLogicalEBNum);
		if(currentLogicalEBNum == logicalEBNum[currentPage.devID])
		{
			/*ForcedGC have already been done on currentLogicalEBNum*/
		}
		else
		{
			logicalEBNum[currentPage.devID] = currentLogicalEBNum;
			logicalGCEBNum = EMPTY_WORD;

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	if (FTL_ERR_PASS != (status = CACHE_LoadEB(currentPage.devID, currentLogicalEBNum, CACHE_WRITE_TYPE)))
	{
		return status;
	}
#endif

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	chainEBNum = GetChainLogicalEBNum(currentPage.devID, currentLogicalEBNum);
	if(chainEBNum != EMPTY_WORD)
	{
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	if (FTL_ERR_PASS != (status = CACHE_LoadEB(currentPage.devID, chainEBNum, CACHE_WRITE_TYPE)))
	{
		return status;
	}
#endif
	if(GetNumFreePages(currentPage.devID, chainEBNum) < Delete_GC_Threshold)
	{
		logicalGCEBNum = currentLogicalEBNum;
	}
}
else
{
	if(GetNumFreePages(currentPage.devID, currentLogicalEBNum) < Delete_GC_Threshold)
	{
		logicalGCEBNum = currentLogicalEBNum;
	}
}

#else  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	if(GetNumFreePages(currentPage.devID, currentLogicalEBNum) < Delete_GC_Threshold)
	{
		logicalGCEBNum = currentLogicalEBNum;
	}
#endif  // #else  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

	if(logicalGCEBNum != EMPTY_WORD)
	{
		FTL_DeleteFlag = FTL_TRUE;
		freedUpPages = EMPTY_WORD;
		freePageIndex = EMPTY_WORD;
		status = ClearGC_Info();
		if(FTL_ERR_PASS != status)
		{
			FTL_ClearMTLockBit();
			return status;
		}

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	if(FTL_ERR_PASS != (status = CACHE_LoadEB(currentPage.devID, logicalGCEBNum , CACHE_WRITE_TYPE)))
	{
		return status;
	}
#endif

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
	status = InternalForcedGCWithBBManagement(currentPage.devID, logicalGCEBNum, &freedUpPages, &freePageIndex, FTL_FALSE);
                if(status != FTL_ERR_PASS)
                {
                   FTL_ClearMTLockBit();
                   return status;
                }
                if(GetTransLogEBFailedBadBlockInfo() == FTL_TRUE)
                {
                   if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
                   {
                      FTL_ClearMTLockBit();
                      return status;
                   }
                }
                ClearTransLogEBBadBlockInfo();

                #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                status = FTL_InternalForcedGC(currentPage.devID, logicalGCEBNum, &freedUpPages, &freePageIndex, FTL_FALSE);
                if(status != FTL_ERR_PASS)
                {
                   FTL_ClearMTLockBit();
                   return status;
                }
                #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

                FTL_DeleteFlag = FTL_FALSE;
             }
          }
          if((status = IncPageAddr(&currentPage)) != FTL_ERR_PASS)
          {
             FTL_ClearMTLockBit();
             return status;
          }
       }
       #endif  // #if(FTL_DELETE_WITH_GC == FTL_TRUE)

    }
    FTL_ClearMTLockBit();
    #endif  // #if (FTL_ENABLE_DELETE_API == FTL_TRUE)

    FTL_UpdatedFlag = UPDATED_DONE;
    return FTL_ERR_PASS;
}
