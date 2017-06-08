/**************************************************************************
 * Copyright (C)2014 Spansion Inc. All Rights Reserved. 
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

#ifdef __KERNEL__
#include <linux/span/FTL/ftl_lowlevel.h>
#include <linux/span/FTL/ftl_common.h>
#include <linux/span/FTL/ftl_if_in.h>
#include <linux/span/FTL/ftl_debug.h>
#include <linux/span/FTL/ftl_def.h>
#else
#include "ftl_lowlevel.h"
#include "ftl_common.h"
#include "ftl_if_in.h"
#include "ftl_debug.h"
#include "ftl_def.h"
#endif // #ifdef __KERNEL__

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)

#define DEBUG_CACHE_GetRAMOffsetEB     (1)
#define DEBUG_CACHE_RemoveIndex        (1)

FTL_STATUS CACHE_LoadEB (FTL_DEV devID, UINT16 logicalEBNum, UINT8 typeAPI)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT8 present = EMPTY_BYTE;
	UINT16 index = EMPTY_WORD;
	UINT8 dependency = EMPTY_BYTE;
	UINT8 cycle = 0;
	UINT32 sector = 0;
	UINT32 totalSector = 0;
	UINT32 EBMramStructPtr = 0;
	UINT32 PPAramStructPtr = 0;
	UINT8 presentFlag = FTL_FALSE;

	if(CACHE_INIT_TYPE != typeAPI)
	{
		if(FTL_TRUE == CACHE_IsThesholdDirtyIndex())
		{
			if((status = TABLE_Flush(FLUSH_SHUTDOWN_MODE)) != FTL_ERR_PASS)
			{
				return status;
			}
		}
	}

	if(FTL_FALSE == CACHE_IsPresentEB(devID,logicalEBNum, &present))
	{
		return status;
	}

	if(CACHE_EBM_PPA_PRESENT == present)
	{
		presentFlag = FTL_TRUE;
		// Cache Hit
#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
		gCacheHit++;
#endif
	}
	else if(CACHE_EBM_PRESENT == present)
	{
		// Expect not to be here.
	}
	else if(CACHE_NO_PRESENT == present)
	{
		// Cache Miss
#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
		gCacheMiss++;
#endif
		if((CACHE_INIT_TYPE == typeAPI) && (NUM_DATA_EBLOCKS <= logicalEBNum))
		{
			// Insert Reserved Area and System Area
			if(FTL_TRUE == CACHE_IsCrossedEB(devID,logicalEBNum))
			{
				CACHE_GetSector(logicalEBNum, &sector);
				CACHE_GetSector((NUM_EBLOCKS_PER_DEVICE - 1), &totalSector);

				sector = totalSector - sector;
				index = (UINT16)(((NUM_EBLOCK_MAP_INDEX - 1) + (devID * NUM_EBLOCK_MAP_INDEX)) - sector);

				dependency = CACHE_DEPEND_UP_DOWN;

			}
			else
			{
				CACHE_GetSector(logicalEBNum, &sector);
				CACHE_GetSector((NUM_EBLOCKS_PER_DEVICE - 1), &totalSector);

				sector = totalSector - sector;
				index = (UINT16)(((NUM_EBLOCK_MAP_INDEX - 1) + (devID * NUM_EBLOCK_MAP_INDEX)) - sector);

				dependency = CACHE_NO_DEPEND;
			}
		}
		else
		{
			for(cycle = 0; cycle < 2; cycle++)
			{
				if(FTL_ERR_PASS != (status = CACHE_AllocateIndex(devID, logicalEBNum, &index, &dependency, typeAPI)))
				{
					if ((FTL_ERR_CACHE_FLUSH_NEED == status) && (0==cycle))
					{
						if((status = TABLE_Flush(FLUSH_SHUTDOWN_MODE)) != FTL_ERR_PASS)
						{
							return status;
						}
						continue;
					}
					return status;
				}
				break;
			}
		}
	}

	if(EMPTY_BYTE != dependency)
	{
		if(FTL_ERR_PASS != (status = CACHE_InsertEB(devID, logicalEBNum, index, dependency, present, typeAPI)))
		{
			return status;
		}
	}
	else
	{
		/* Initialize table rather than insert table from flash. */
		if(FTL_ERR_PASS != (status = CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr)))
		{
			return status;
		}
#ifdef DEBUG_PROTOTYPE
		if (EMPTY_DWORD == *(UINT32_PTR)EBMramStructPtr)
#else
		if(EMPTY_WORD == (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).phyEBAddr)
#endif
		{
			TABLE_ClearMappingTable(devID, logicalEBNum, logicalEBNum, ERASE_STATUS_GET_DWORD_MASK | 1);
			TABLE_ClearPPATable(devID, logicalEBNum);
		}
	}

	if (CACHE_NO_DEPEND != dependency && CACHE_WRITE_TYPE == typeAPI && presentFlag == FTL_FALSE)
	{
		if((status = TABLE_Flush(FLUSH_SHUTDOWN_MODE)) != FTL_ERR_PASS)
		{
			return status;
		}
	}

	if(CACHE_INIT_TYPE != typeAPI)
	{
		//  Get index
		if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}
		index = ramMapInfo.ebmCacheIndex;
		if(FTL_ERR_PASS != (status = CACHE_UpdateLRUIndex(devID, index, typeAPI)))
		{
			return status;
		}
	}

	return status;
}

FTL_STATUS CACHE_GetRAMOffsetEB (FTL_DEV devID, UINT16 logicalEBNum, UINT32_PTR EBMramStructPtr, UINT32_PTR PPAramStructPtr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT32 tempOffset = 0;
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
	UINT8 tempCrossOffset = 0;
#else
	UINT16 tempCrossOffset = 0;
#endif

	static UINT16 stLogicalEBNum = EMPTY_WORD;
	static UINT32 stEBMramStruct = EMPTY_DWORD;
	static UINT32 stPPAramStruct = EMPTY_DWORD;

	if (stLogicalEBNum == logicalEBNum)
	{
		*EBMramStructPtr = stEBMramStruct;
		*PPAramStructPtr = stPPAramStruct;

#if DEBUG_CACHE_GetRAMOffsetEB
		if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}

		if (FTL_FALSE == ramMapInfo.presentEBM)
		{
			return FTL_ERR_FAIL;
		}

		// EBlock
		if (stEBMramStruct != (UINT32)(&EBlockMappingCache[ramMapInfo.ebmCacheIndex][ramMapInfo.indexOffset * 2]))
		{
			DBG_Printf("Error: EBMCache Pointer is modified. \n",0,0);
			return FTL_ERR_FAIL;
		}

		// PPA
		ramMapInfo.ebmCacheIndex = (UINT16)(ramMapInfo.ebmCacheIndex * PPA_CACHE_TABLE_OFFSET);

		if (0 != (ramMapInfo.indexOffset % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2)))
		{
			// cross bandary case
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
			tempCrossOffset = (UINT8)(ramMapInfo.indexOffset % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2));
			ramMapInfo.indexOffset = (UINT8)(ramMapInfo.indexOffset - tempCrossOffset); // adjust offset for cross bandary
#else
			tempCrossOffset = (UINT16)(ramMapInfo.indexOffset % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2));
			ramMapInfo.indexOffset = (UINT16)(ramMapInfo.indexOffset - tempCrossOffset); // adjust offset for cross bandary
#endif
		}

		tempOffset = (UINT32)((ramMapInfo.indexOffset / ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2)) * ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) / 2));
		if ((tempOffset * 2) >= FLUSH_RAM_TABLE_SIZE)
		{
			ramMapInfo.ebmCacheIndex = (UINT16)(ramMapInfo.ebmCacheIndex + (UINT16)((tempOffset * 2) / FLUSH_RAM_TABLE_SIZE)); // next index
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
			ramMapInfo.indexOffset = (UINT8)(((tempOffset * 2) % FLUSH_RAM_TABLE_SIZE) / 2);
#else
			ramMapInfo.indexOffset = (UINT16)(((tempOffset * 2) % FLUSH_RAM_TABLE_SIZE) / 2);
#endif
		}
		else
		{
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
			ramMapInfo.indexOffset = (UINT8)tempOffset;
#else
			ramMapInfo.indexOffset = (UINT16)tempOffset;
#endif
		}

		if (stPPAramStruct != (UINT32)(&PPAMappingCache[ramMapInfo.ebmCacheIndex][(ramMapInfo.indexOffset * 2)]))
		{
			DBG_Printf("Error: PPACache Pointer is modified. \n",0,0);
			return FTL_ERR_FAIL;
		}
#endif /* DEBUG_CACHE_GetRAMOffsetEB */
	}
	else
	{
		if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			DBG_Printf("[ERROR] CACHE_GetRamMap : logicalEBNum = 0x%x \n", logicalEBNum, 0);
			return status;
		}

		if (FTL_FALSE == ramMapInfo.presentEBM)
		{
			DBG_Printf("[ERROR] ramMapInfo.presentEBM is FTL_FALSE : logicalEBNum = 0x%x \n", logicalEBNum, 0);
			return FTL_ERR_GET_RAM_OFFSET_EB;
		}

		stLogicalEBNum = logicalEBNum;

		// EBlock
		*EBMramStructPtr = (UINT32)(&EBlockMappingCache[ramMapInfo.ebmCacheIndex][ramMapInfo.indexOffset * 2]);// byte address
		stEBMramStruct = *EBMramStructPtr;

		// PPA
		ramMapInfo.ebmCacheIndex = (UINT16)(ramMapInfo.ebmCacheIndex * PPA_CACHE_TABLE_OFFSET);

		if (0 != (ramMapInfo.indexOffset % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2)))
		{
			// cross bandary case
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
			tempCrossOffset = (UINT8)(ramMapInfo.indexOffset % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2));
			ramMapInfo.indexOffset = (UINT8)(ramMapInfo.indexOffset - tempCrossOffset); // adjust offset for cross bandary
#else
			tempCrossOffset = (UINT16)(ramMapInfo.indexOffset % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2));
			ramMapInfo.indexOffset = (UINT16)(ramMapInfo.indexOffset - tempCrossOffset); // adjust offset for cross bandary
#endif
		}

		tempOffset = (UINT32)((ramMapInfo.indexOffset / ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / 2)) * ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) / 2));
		if ((tempOffset * 2) >= FLUSH_RAM_TABLE_SIZE)
		{
			ramMapInfo.ebmCacheIndex = (UINT16)(ramMapInfo.ebmCacheIndex + (UINT16)((tempOffset * 2) / FLUSH_RAM_TABLE_SIZE)); // next index
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
			ramMapInfo.indexOffset = (UINT8)(((tempOffset * 2) % FLUSH_RAM_TABLE_SIZE) / 2);
#else
			ramMapInfo.indexOffset = (UINT16)(((tempOffset * 2) % FLUSH_RAM_TABLE_SIZE) / 2);
#endif
		}
		else
		{
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
			ramMapInfo.indexOffset = (UINT8)tempOffset;
#else
			ramMapInfo.indexOffset = (UINT16)tempOffset;
#endif
		}

		*PPAramStructPtr = (UINT32)(&PPAMappingCache[ramMapInfo.ebmCacheIndex][(ramMapInfo.indexOffset * 2)]); // byte address
		stPPAramStruct = *PPAramStructPtr;
	}
	return status;
}

FTL_STATUS CACHE_ClearAll(void)
{
	FTL_STATUS status = FTL_ERR_PASS;
	FTL_DEV devID = 0;
	UINT16 index = 0;
	UINT16 logicalEBNum = 0;
	UINT32 i = 0;

	for (i = 0; i < NUM_CROSS_LEB; i++)
	gCrossedLEB[i] = EMPTY_WORD;

	for(devID = 0; devID < NUMBER_OF_DEVICES; devID++)
	{
		for(index = 0; index < MAX_EBLOCK_MAP_INDEX; index++)
		{
			if(FTL_ERR_PASS != (status = CACHE_ClearEBlockandPPAMap(devID,index,CACHE_EBLOCKMAP)))
			{
				return status;
			}
		}

		for(index = 0; index < MAX_PPA_MAP_INDEX; index++)
		{
			if(FTL_ERR_PASS != (status = CACHE_ClearEBlockandPPAMap(devID,index,CACHE_PPAMAP)))
			{
				return status;
			}
		}

		for(logicalEBNum = 0; logicalEBNum < NUMBER_OF_ERASE_BLOCKS; logicalEBNum++)
		{
			RamMapIndex[logicalEBNum + (devID * NUMBER_OF_ERASE_BLOCKS)] = CACHE_INIT_RAM_MAP_INDEX;
		}

		for(index = 0; index < NUM_EBLOCK_MAP_INDEX; index++)
		{
			if(FTL_ERR_PASS != (status = CACHE_ClearEBMCache(devID,index)))
			{
				return status;
			}
		}

		for(index = 0; index < NUM_EBLOCK_MAP_INDEX; index++)
		{
			for(i = 0; i < FLUSH_RAM_TABLE_SIZE; i++)
			{
				EBlockMappingCache[index + (devID * NUMBER_OF_DEVICES)][i] = EMPTY_BYTE;
			}
		}

		for(index = 0; index < NUM_PPA_MAP_INDEX; index++)
		{
			for(i = 0; i < FLUSH_RAM_TABLE_SIZE; i++)
			{
				PPAMappingCache[index + (devID * NUMBER_OF_DEVICES)][i] = EMPTY_BYTE;
			}
		}

		for (index = 0; index < NUM_CHAIN_EBLOCKS; index++)
		{
			SaveValidUsedPage[index + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = EMPTY_WORD;
			SaveValidUsedPage[index + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount = EMPTY_WORD;
			SaveValidUsedPage[index + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount = EMPTY_WORD;
		}

		if(FTL_ERR_PASS != (status = ClearSaveStaticWL(devID, EMPTY_WORD, 0, CACHE_WL_HIGH)))
		{
			return status;
		}
		if(FTL_ERR_PASS != (status = ClearSaveStaticWL(devID, EMPTY_WORD, EMPTY_DWORD, CACHE_WL_LOW)))
		{
			return status;
		}
	}

	if(FTL_ERR_PASS != (status = CACHE_CleargLRUCounter()))
	{
		return status;
	}

	gCounterDirty = 0;
	gDataAreaCounterDirty = 0;

	return status;
}

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
FTL_STATUS CACHE_SetPfEBForNAND(FTL_DEV devID, UINT16 logicalEBNum, UINT8 flag)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 temp = 0;
	UINT16 temp2 = 0;
	UINT32 sector = 0;

	if((status = CACHE_GetSector(logicalEBNum, &sector)) != FTL_ERR_PASS)
	{
		return status;
	}

	temp = EBlockMapIndex[sector + (devID * MAX_EBLOCK_MAP_INDEX)];
	temp = (UINT16)(temp & 0x7FFF);  // FTL_TRUE case
	if(FTL_FALSE == flag)
	{
		temp2 = 0x8000;
		temp = temp | temp2; // FTL_FALSE case
	}
	EBlockMapIndex[sector + (devID * MAX_EBLOCK_MAP_INDEX)] = temp;

	return status;
}

UINT8 CACHE_IsPfEBForNAND(FTL_DEV devID, UINT16 logicalEBNum)
{
	UINT16 temp = 0;
	UINT32 sector = 0;

	CACHE_GetSector(logicalEBNum, &sector);

	temp = EBlockMapIndex[sector + (devID * MAX_EBLOCK_MAP_INDEX)];
	temp = (UINT16)((temp & 0x8000) >> 15);

	if(0x1 == temp)
	{
		return FTL_FALSE;
	}
	else
	{
		return FTL_TRUE;
	}
}

FTL_STATUS CACHE_ClearAllPfEBForNAND(FTL_DEV devID)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 temp = 0;
	UINT32 sector = 0;

	for(sector = 0; sector < MAX_EBLOCK_MAP_INDEX; sector++)
	{
		temp = EBlockMapIndex[sector + (devID * MAX_EBLOCK_MAP_INDEX)];
		temp = (UINT16)(temp | 0x8000);
		EBlockMapIndex[sector + (devID * MAX_EBLOCK_MAP_INDEX)] = temp;
	}
	return status;
}
#endif // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

// Index Basis Interface
FTL_STATUS CACHE_AllocateIndex(FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT8 typeAPI)
{
	FTL_STATUS status = FTL_ERR_PASS;

	if(FTL_TRUE == CACHE_IsCrossedEB(devID,logicalEBNum))
	{
		if(FTL_ERR_PASS != (status = CACHE_FindIndexForCross(devID, logicalEBNum, index_ptr, dependency_ptr, typeAPI)))
		{
			return status;
		}
	}
	else
	{
		if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, index_ptr, EMPTY_WORD, EMPTY_WORD, FTL_TRUE)))
		{
			return status;
		}
		*dependency_ptr = CACHE_NO_DEPEND;
	}
	return status;
}

FTL_STATUS CACHE_FindIndexForAlign(FTL_DEV devID, UINT16_PTR index_ptr, UINT16 skipIndex, UINT16 skipIndex2, UINT16 removed)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT16 index = 0;
	UINT8 miniLRU = EMPTY_BYTE;
	UINT8 tempLRU = EMPTY_BYTE;
	UINT16 candidateIndex = EMPTY_WORD;
	UINT8 checkFree = FTL_FALSE;

	// find candidate cache index
	for(index = 0; index < CACHE_INDEX_CHANGE_AREA; index++)
	{
		if ((index == skipIndex) || (index == skipIndex2))
		{
			// EMPTY_WORD does not enter here.
			continue;
		}

		// check no dirty
		if(FTL_TRUE == CACHE_IsDirtyIndex(devID, index))
		{
			continue;
		}

		// check free 
		if(FTL_TRUE == CACHE_IsFreeIndex(devID, index))
		{
			if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
			{
				return status;
			}

			candidateIndex = index;
			checkFree = FTL_TRUE;
			break;
		}

		// if free area is present, don't enter. 
		if(FTL_FALSE == checkFree)
		{
			if(FTL_TRUE == (status =CACHE_IsCleanIndex(devID, index)))
			{
				if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
				{
					return status;
				}
				// Temporary Implementation. LRU Logic needs to be updated for final version.
				tempLRU = ebmCacheInfo.rLRUCount;
				tempLRU = (UINT8)(ebmCacheInfo.wLRUCount + tempLRU);
				tempLRU = (UINT8)(tempLRU / 2);// average

				if(miniLRU >= tempLRU)
				{
					miniLRU = tempLRU;
					candidateIndex = index;
				}
			}
		}
	}

	if (removed == FTL_TRUE)
	{
		// no free case
		if(FTL_FALSE == checkFree)
		{
			if(FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, candidateIndex)))
			{
				return status;
			}
		}
	}

	if (candidateIndex == EMPTY_WORD)
	{
		return FTL_ERR_CACHE_FIND_INDEX_FOR_ALIGN;
	}

	*index_ptr = candidateIndex;

	return FTL_ERR_PASS;
}

FTL_STATUS CACHE_FindIndexForCross(FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT8 typeAPI)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT8 checkPresent = EMPTY_BYTE;
	UINT16 indexUp = 0;
	UINT16 indexDown = 0;
	UINT8 dependency = 0;

	// check up side
	if (FTL_ERR_PASS != (status = CACHE_GetIndex(devID, (UINT16)(logicalEBNum - 1), &indexUp, &dependency)))
	{
		if (FTL_ERR_CACHE_NOT_FOUND != status)
		{
			return status;
		}
	}

	if (logicalEBNum == NUMBER_OF_DATA_EBLOCKS - 1)
	{
		if (EMPTY_WORD != indexUp)
		{
			checkPresent = CACHE_DEPEND_UP_DOWN;
		}
		else
		{
			checkPresent = CACHE_DEPEND_DOWN;
		}

		// check edge
		if (CACHE_INDEX_CHANGE_AREA == indexUp)
		{
			DBG_Printf("CACHE_FindIndexForCross: Edeg case should not be here. \n",0,0);
			return FTL_ERR_CACHE_INDEX_FIND_CANDIDATE_FOR_CROSS_BOUNDARY_1;
		}

		// check down side
		if (FTL_ERR_PASS != (status = CACHE_GetIndex(devID, (UINT16)(logicalEBNum + 1), &indexDown, &dependency)))
		{
			DBG_Printf("CACHE_FindIndexForCross: Edge case should be always on cache. \n",0,0);
			return FTL_ERR_CACHE_INDEX_FIND_CANDIDATE_FOR_CROSS_BOUNDARY_2;
		}

		// check edge
		if (CACHE_INDEX_CHANGE_AREA != indexDown)
		{
			DBG_Printf("CACHE_FindIndexForCross: Edge case should not be moved. \n",0,0);
			return FTL_ERR_CACHE_INDEX_FIND_CANDIDATE_FOR_CROSS_BOUNDARY_3;
		}
	}
	else
	{
		if (EMPTY_WORD != indexUp)
		{
			checkPresent = CACHE_DEPEND_UP;
		}

		// check edge
		if (CACHE_INDEX_CHANGE_AREA == indexUp)
		{
			if (FTL_TRUE == CACHE_IsDirtyIndex(devID, indexUp))
			{
				return FTL_ERR_CACHE_FLUSH_NEED; // go to flush
			}
		}

		// check down side
		if (FTL_ERR_PASS != (status = CACHE_GetIndex(devID, (UINT16)(logicalEBNum + 1), &indexDown, &dependency)))
		{
			if (FTL_ERR_CACHE_NOT_FOUND != status)
			{
				return status;
			}
		}

		// check edge
		if (0 == indexDown)
		{
			if (FTL_TRUE == CACHE_IsDirtyIndex(devID, indexDown))
			{
				return FTL_ERR_CACHE_FLUSH_NEED; // go to flush
			}
		}

		if (EMPTY_WORD != indexDown)
		{
			if (CACHE_DEPEND_UP == checkPresent)
			{
				checkPresent = CACHE_DEPEND_UP_DOWN; // both side is present.
			}
			else
			{
				checkPresent = CACHE_DEPEND_DOWN;
			}
		}
	}

	if(EMPTY_BYTE == checkPresent) // both side is not present.
	{
#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
		gNoUpAndDownCase++;
#endif
		if(FTL_ERR_PASS != (status = CACHE_FindIndexNeitherPresented(devID, index_ptr, dependency_ptr)))
		{
			return status;
		}
	}
	else if(CACHE_DEPEND_UP == checkPresent)
	{ // up side is present only. 
#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
		gNoDownCase++;
#endif
		if(FTL_ERR_PASS != (status = CACHE_FindIndexUpsidePresented(devID, index_ptr, dependency_ptr, indexUp)))
		{
			return status;
		}
	}
	else if(CACHE_DEPEND_DOWN == checkPresent)
	{ // down side is present only.
#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
		gNoUpCase++;
#endif
		if(FTL_ERR_PASS != (status = CACHE_FindIndexDownsidePresented(devID, index_ptr, dependency_ptr, indexDown)))
		{
			return status;
		}
	}
	else if(CACHE_DEPEND_UP_DOWN == checkPresent)
	{ // up and down side is present but distance them.
#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
		gDistanceUpAndDownCase++;
#endif
		if (FTL_ERR_PASS != (status = CACHE_FindIndexBothPresented(devID, index_ptr, dependency_ptr, indexUp, indexDown, logicalEBNum, typeAPI)))
		{
			return status;
		}
	}

	return FTL_ERR_PASS;
}

FTL_STATUS CACHE_RemoveIndex (FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT16 logicalEBNum = 0;
	UINT16 ebCount = 0;

#ifdef DEBUG_DATA_CLEAR
	DEBUG_CACHE_SetEBMCacheFree(devID, index);
#endif

	if(FTL_ERR_PASS != (status = CACHE_GetEB(devID,index, &logicalEBNum)))
	{
		return status;
	}

	if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
	{
		return status;
	}

	if (CACHE_DEPEND_DOWN == ebmCacheInfo.dependency)
	{
		logicalEBNum -= 1;
	}

	// Remove associated RamMapIndex
	for (ebCount = 0; ebCount < EBM_ENTRY_COUNT; ebCount++)
	{
		if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum + ebCount, &ramMapInfo)))
		{
			return status;
		}

		if (FTL_TRUE == ramMapInfo.presentEBM)
		{
			if ((0 == ebCount) && (CACHE_DEPEND_DOWN == ebmCacheInfo.dependency))
			{
				if (index - 1 == ramMapInfo.ebmCacheIndex)
				{
					if (FTL_ERR_PASS != (status = CACHE_ClearRamMap(devID, logicalEBNum + ebCount)))
					{
						return status;
					}
				}
			}
			else
			{
				if (index == ramMapInfo.ebmCacheIndex)
				{
					if (FTL_ERR_PASS != (status = CACHE_ClearRamMap(devID, logicalEBNum + ebCount)))
					{
						return status;
					}
				}
			}
		}
	}

#if DEBUG_CACHE_RemoveIndex
	if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum + ebCount, &ramMapInfo)))
	{
		return status;
	}
	if (index == ramMapInfo.ebmCacheIndex)
	DBG_Printf("Inconsistency between rammapindex and ebmcacheindex3. \n", 0, 0);
#endif

	// set free
	if(FTL_ERR_PASS != (status = CACHE_ClearEBMCache(devID, index)))
	{
		return status;
	}

	if(CACHE_DEPEND_UP == ebmCacheInfo.dependency)
	{
		if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, (UINT16)(index + 1))))
		{
			return status;
		}
	}
	else if(CACHE_DEPEND_DOWN == ebmCacheInfo.dependency)
	{
		if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, (UINT16)(index - 1))))
		{
			return status;
		}
	}

	return status;
}

FTL_STATUS CACHE_MoveIndex(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	if (fromIndex == toIndex)
	{
		return status;
	}

	// copy ebmCacheInfo
	if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, toIndex, &ebmCacheInfo)))
	{
		return status;
	}

	if (CACHE_FREE != ebmCacheInfo.cacheStatus)
	{
		if (FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, toIndex)))
		{
			return status;
		}
	}

	// copy data
	if (FTL_ERR_PASS != (status = CACHE_CacheToCache(devID, fromIndex, toIndex)))
	{
		return status;
	}

	if (FTL_ERR_PASS != (status = CACHE_MoveEBMCache(devID, fromIndex, toIndex)))
	{
		return status;
	}

	// change index of RamMapEntry
	if (FTL_ERR_PASS != (status = CACHE_MoveRamMap(devID, fromIndex, toIndex)))
	{
		return status;
	}

	return status;
}

FTL_STATUS CACHE_NoDpendIndex(FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	ebmCacheInfo.cacheStatus = EMPTY_BYTE;
	ebmCacheInfo.dependency = CACHE_NO_DEPEND; // change no depend
	ebmCacheInfo.rLRUCount = EMPTY_BYTE;
	ebmCacheInfo.wLRUCount = EMPTY_BYTE;
	if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, index, &ebmCacheInfo)))
	{
		return status;
	}

	return status;
}

FTL_STATUS CACHE_IsThesholdDirtyIndex(void)
{
	if(gDataAreaCounterDirty >=THESHOLD_DIRTY_COUNT)
	{
		return FTL_TRUE;
	}
	return FTL_FALSE;
}

#ifdef DEBUG_PROTOTYPE
FTL_STATUS TABLE_Flush(UINT8 flushMode)
{
	FTL_STATUS status = FTL_ERR_PASS;
	FTL_DEV devID = 0;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo =
	{	0, 0};
	UINT16 indexCount = 0;
	UINT16 logicalEBNum = 0;
	UINT32 sector = 0;

	for(devID = 0; devID < NUMBER_OF_DEVICES; devID++)
	{
		for(indexCount = 0;indexCount < NUM_EBLOCK_MAP_INDEX; indexCount++)
		{
			if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, indexCount, &ebmCacheInfo)))
			{
				return status;
			}

			if(CACHE_DIRTY == ebmCacheInfo.cacheStatus)
			{

				if(FTL_ERR_PASS != (status = CACHE_GetEB(devID, indexCount, &logicalEBNum)))
				{
					return status;
				}

				sector = ((((UINT32) logicalEBNum) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;

				// EBM

				// Get EBlock location
				if(FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)sector, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
				{
					return status;
				}

				// Flush
				if (FTL_ERR_PASS != (status = CACHE_CacheToFlash(devID, indexCount, eBlockPPAMapInfo, CACHE_EBLOCKMAP, flushMode)))
				{
					return status;
				}

				// PPA

				// Get EBlock location
				if (FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)(sector * PPA_CACHE_TABLE_OFFSET), &eBlockPPAMapInfo, CACHE_PPAMAP)))
				{
					return status;
				}

				// Flush
				if (FTL_ERR_PASS != (status = CACHE_CacheToFlash(devID, indexCount, eBlockPPAMapInfo, CACHE_PPAMAP, flushMode)))
				{
					return status;
				}
			}
		}
	}

	// clear dirty
	if(FTL_ERR_PASS != (status = CACHE_CleanAllDirtyIndex(devID)))
	{
		return status;
	}

	return FTL_ERR_PASS;
}
#endif

FTL_STATUS CACHE_UpdateLRUIndex(FTL_DEV devID, UINT16 index, UINT8 typeAPI)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT16 indexCount = 0;
	UINT8 counterLRU = 0;

	if(FTL_ERR_PASS != (status = CACHE_GetgLRUCounter(&counterLRU)))
	{
		return status;
	}

	counterLRU++;
	if(counterLRU > LIMIT_LRU)
	{
		// Divide Op
		counterLRU = (UINT8)(counterLRU >> DIVIDE_LRU);

		if(FTL_ERR_PASS != (status = CACHE_SetgLRUCounter(counterLRU)))
		{
			return status;
		}

		for(indexCount = 0;indexCount < CACHE_INDEX_CHANGE_AREA; indexCount++)
		{
			if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, indexCount, &ebmCacheInfo)))
			{
				return status;
			}

			if(index == indexCount)
			{
				if(CACHE_WRITE_TYPE == typeAPI)
				{
					ebmCacheInfo.rLRUCount = (UINT8)(ebmCacheInfo.rLRUCount >> DIVIDE_LRU);
					ebmCacheInfo.wLRUCount = counterLRU;
				}
				else if(CACHE_READ_TYPE == typeAPI)
				{
					ebmCacheInfo.rLRUCount = counterLRU;
					ebmCacheInfo.wLRUCount = (UINT8)(ebmCacheInfo.wLRUCount >> DIVIDE_LRU);
				}
			}
			else
			{
				ebmCacheInfo.rLRUCount = (UINT8)(ebmCacheInfo.rLRUCount >> DIVIDE_LRU);
				ebmCacheInfo.wLRUCount = (UINT8)(ebmCacheInfo.wLRUCount >> DIVIDE_LRU);

			}

			// no update
			ebmCacheInfo.cacheStatus = EMPTY_BYTE;
			ebmCacheInfo.dependency = EMPTY_BYTE;

			if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, indexCount, &ebmCacheInfo)))
			{
				return status;
			}
		}
	}
	else
	{
		if(FTL_ERR_PASS != (status = CACHE_SetgLRUCounter(counterLRU)))
		{
			return status;
		}

		if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
		{
			return status;
		}

		if(CACHE_WRITE_TYPE == typeAPI)
		{
			ebmCacheInfo.wLRUCount = counterLRU;
			ebmCacheInfo.rLRUCount = EMPTY_BYTE;
		}
		else if(CACHE_READ_TYPE == typeAPI)
		{
			ebmCacheInfo.rLRUCount = counterLRU;
			ebmCacheInfo.wLRUCount = EMPTY_BYTE;
		}

		// no update
		ebmCacheInfo.cacheStatus = EMPTY_BYTE;
		ebmCacheInfo.dependency = EMPTY_BYTE;

		if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, index, &ebmCacheInfo)))
		{
			return status;
		}
	}

	return status;
}

FTL_STATUS CACHE_CleanAllDirtyIndex(FTL_DEV devID)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT16 index = 0;

	for (devID = 0; devID < NUMBER_OF_DEVICES; devID++)
	{
		for (index = 0; index < NUM_EBLOCK_MAP_INDEX; index++)
		{
			if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
			{
				return status;
			}

			if (CACHE_DIRTY == ebmCacheInfo.cacheStatus)
			{
				// clear dirty bit
				ebmCacheInfo.cacheStatus = CACHE_CLEAN;
				ebmCacheInfo.dependency = EMPTY_BYTE;
				ebmCacheInfo.rLRUCount = EMPTY_BYTE;
				ebmCacheInfo.wLRUCount = EMPTY_BYTE;
				if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, index, &ebmCacheInfo)))
				{
					return status;
				}
			}
		}
	}
	gCounterDirty = 0;
	gDataAreaCounterDirty = 0;

	return status;
}

// EB Basis Interface
FTL_STATUS CACHE_InsertEB(FTL_DEV devID, UINT16 logicalEBNum, UINT16 index, UINT8 dependency, UINT8 present, UINT8 typeAPI)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT32 sector = 0; /*4*/
	UINT32 sector2 = 0;
	UINT32 sector6 = 0;

	UINT16 ebCount = 0;
	UINT32 offsetTable;
	UINT8 lastRamMap = FTL_FALSE;
	UINT16 logicalEBNumFirst = 0;
	UINT16 indexTarget = 0;
	UINT16 crossedLEB = EMPTY_WORD;

	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo =
	{	0, 0};

	UINT16 i = 0;

	if(CACHE_EBM_PPA_PRESENT == present)
	{
		return FTL_ERR_CACHE_EB_INSERT_1;
	}

	if(CACHE_EBM_PRESENT == present)
	{
		// Expect not to be here.
	}
	else if(CACHE_NO_PRESENT == present)
	{
		if(CACHE_NO_DEPEND == dependency || CACHE_DEPEND_UP == dependency)
		{
			sector = ((((UINT32) logicalEBNum) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
		}
		if(CACHE_DEPEND_DOWN == dependency)
		{  // [NOTE] I'll check at brushup.
			sector = ((((UINT32) logicalEBNum) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
			sector2 = ((((UINT32) logicalEBNum + 1) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) - 1) / FLUSH_RAM_TABLE_SIZE;
		}
		if(CACHE_DEPEND_UP_DOWN == dependency)
		{
			sector = ((((UINT32) logicalEBNum) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
			sector2 = ((((UINT32) logicalEBNum + 1) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) - 1) / FLUSH_RAM_TABLE_SIZE;
		}

		if(CACHE_DEPEND_DOWN != dependency)
		{
			// Set associated RamMapIndex
			offsetTable = logicalEBNum*(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE;
			if (offsetTable >= (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))
			{
				// Set the first LEB in Table
				logicalEBNumFirst = logicalEBNum - (UINT16)((offsetTable + 1) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
			}
			else
			{
				logicalEBNumFirst = logicalEBNum;
			}
			lastRamMap = FTL_FALSE;
			for (ebCount = 0; ebCount < EBM_ENTRY_COUNT; ebCount++)
			{
				if (ebCount)
				{
					sector6 = (((UINT32)logicalEBNumFirst + 1 + ebCount) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) - 1) / FLUSH_RAM_TABLE_SIZE;
					if ((sector + 1 == sector6) && (CACHE_NO_DEPEND == dependency))
					{
						break;
					}
					if ((sector + 1 == sector6) && (CACHE_NO_DEPEND != dependency))
					{
						lastRamMap = FTL_TRUE;
					}
				}

				if ((logicalEBNumFirst + ebCount) >= NUMBER_OF_ERASE_BLOCKS)
				{
					continue;
				}

				ramMapInfo.ebmCacheIndex = index;
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
				ramMapInfo.indexOffset = (UINT8)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * (logicalEBNumFirst + ebCount)) % FLUSH_RAM_TABLE_SIZE) / 2); // word address
#else
				ramMapInfo.indexOffset = (UINT16)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * (logicalEBNumFirst + ebCount)) % FLUSH_RAM_TABLE_SIZE) / 2); // word address
#endif
				ramMapInfo.presentEBM = FTL_TRUE;
				ramMapInfo.presentPPA = FTL_TRUE;
				if (FTL_ERR_PASS != (status = CACHE_SetRamMap(devID, logicalEBNumFirst + ebCount, &ramMapInfo)))
				{
					return status;
				}
				if (FTL_TRUE == lastRamMap)
				{
					break;
				}
			}

			if(CACHE_DEPEND_UP_DOWN == dependency)
			{
				ebmCacheInfo.cacheStatus = CACHE_CLEAN;
				ebmCacheInfo.dependency = CACHE_DEPEND_UP;
			}
			else
			{
				ebmCacheInfo.cacheStatus = CACHE_CLEAN;
				ebmCacheInfo.dependency = dependency;
			}
			ebmCacheInfo.rLRUCount = 0x0;
			ebmCacheInfo.wLRUCount = 0x0;
			if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, index, &ebmCacheInfo)))
			{
				return status;
			}

			// EB 
			if(FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)sector, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
			{
				return status;
			}

			if ((CACHE_EMPTY_ENTRY_INDEX != eBlockPPAMapInfo.entryIndex) && (CACHE_EMPTY_FLASH_LOG_ARRAY != eBlockPPAMapInfo.flashLogEBArrayCount))
			{
				// Load
				if(FTL_ERR_PASS != (status = CACHE_FlashToCache(devID, eBlockPPAMapInfo, index, CACHE_EBLOCKMAP)))
				{
					return status;
				}
			}
			else
			{
				// can't find EBM table in flash
				// Initialize associated EBM cache.
				offsetTable = logicalEBNum*(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE;
				if (offsetTable >= (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))
				{
					// Set the first LEB in Table
					logicalEBNumFirst = logicalEBNum - (UINT16)((offsetTable + 1) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
				}
				else
				{
					logicalEBNumFirst = logicalEBNum;
				}

				indexTarget = index;
				for (i = 0; i < FLUSH_RAM_TABLE_SIZE; i++)
				{
					EBlockMappingCache[indexTarget + (devID * NUM_EBLOCK_MAP_INDEX)][i] = EMPTY_BYTE;
				}

				lastRamMap = FTL_FALSE;
				for (ebCount = 0; ebCount < EBM_ENTRY_COUNT; ebCount++)
				{

					if ((logicalEBNumFirst + ebCount) >= NUMBER_OF_ERASE_BLOCKS)
					{
						continue;
					}

					if (ebCount)
					{
						sector6 = (((UINT32)logicalEBNumFirst + 1 + ebCount) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) - 1) / FLUSH_RAM_TABLE_SIZE;
						if ((sector + 1 == sector6) && (CACHE_NO_DEPEND == dependency))
						{
							break;
						}
						if ((sector + 1 == sector6) && (CACHE_NO_DEPEND != dependency))
						{
							lastRamMap = FTL_TRUE;
						}
					}

					TABLE_ClearMappingTable(devID, logicalEBNumFirst + ebCount, logicalEBNumFirst + ebCount, ERASE_STATUS_GET_DWORD_MASK | 1);

					if (FTL_TRUE == lastRamMap)
					{
						break;
					}
				}
			}

			// PPA 
			if(FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)(sector * PPA_CACHE_TABLE_OFFSET), &eBlockPPAMapInfo, CACHE_PPAMAP)))
			{
				return status;
			}

			if ((CACHE_EMPTY_ENTRY_INDEX != eBlockPPAMapInfo.entryIndex) && (CACHE_EMPTY_FLASH_LOG_ARRAY != eBlockPPAMapInfo.flashLogEBArrayCount))
			{
				// Load
				if(FTL_ERR_PASS != (status = CACHE_FlashToCache(devID, eBlockPPAMapInfo, index, CACHE_PPAMAP)))
				{
					return status;
				}
			}
			else
			{
				// can't find PPA table in flash
				// Initialize associated PPA cache.
				offsetTable = logicalEBNum*(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE;
				if (offsetTable >= (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))
				{
					// Set the first LEB in Table
					logicalEBNumFirst = logicalEBNum - (UINT16)((offsetTable + 1) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
				}
				else
				{
					logicalEBNumFirst = logicalEBNum;
				}
				lastRamMap = FTL_FALSE;
				for (ebCount = 0; ebCount < EBM_ENTRY_COUNT; ebCount++) /* EBM_ENTRY_COUNT, Expect to be OK for PPA */
				{
					if ((logicalEBNumFirst + ebCount) >= NUMBER_OF_ERASE_BLOCKS)
					{
						continue;
					}

					if (ebCount)
					{
						sector6 = (((UINT32)logicalEBNumFirst + 1 + ebCount) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) - 1) / FLUSH_RAM_TABLE_SIZE;
						if ((sector + 1 == sector6) && (CACHE_NO_DEPEND == dependency))
						{
							break;
						}
						if ((sector + 1 == sector6) && (CACHE_NO_DEPEND != dependency))
						{
							lastRamMap = FTL_TRUE;
						}
					}

					TABLE_ClearPPATable(devID, logicalEBNumFirst + ebCount);

					if (FTL_TRUE == lastRamMap)
					{
						break;
					}
				}
			}
		}

		if(CACHE_DEPEND_UP_DOWN == dependency || CACHE_DEPEND_DOWN == dependency)
		{
			for (ebCount = 0; ebCount < EBM_ENTRY_COUNT+1/* Just in Case */; ebCount++)
			{

				if ((logicalEBNum + ebCount) >= NUMBER_OF_ERASE_BLOCKS)
				{
					continue;
				}

				if (!ebCount)
				{
					if (CACHE_DEPEND_DOWN == dependency)
					{
						ramMapInfo.ebmCacheIndex = (UINT16)(index - 1);
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
						ramMapInfo.indexOffset = (UINT8)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * (logicalEBNum + ebCount)) % FLUSH_RAM_TABLE_SIZE) / 2); // word address
#else
						ramMapInfo.indexOffset = (UINT16)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * (logicalEBNum + ebCount)) % FLUSH_RAM_TABLE_SIZE) / 2); // word address
#endif
						ramMapInfo.presentEBM = FTL_TRUE;
						ramMapInfo.presentPPA = FTL_TRUE;
						if (FTL_ERR_PASS != (status = CACHE_SetRamMap(devID, (logicalEBNum + ebCount), &ramMapInfo)))
						{
							return status;
						}
						ebmCacheInfo.cacheStatus = EMPTY_BYTE;
						ebmCacheInfo.dependency = CACHE_DEPEND_UP;
						ebmCacheInfo.rLRUCount = EMPTY_BYTE;
						ebmCacheInfo.wLRUCount = EMPTY_BYTE;
						if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, (UINT16)(index - 1), &ebmCacheInfo)))
						{
							return status;
						}
					}
				}
				else
				{
					sector6 = (((UINT32)logicalEBNum + 1 + ebCount) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) - 1) / FLUSH_RAM_TABLE_SIZE;
					if (sector + 2 == sector6)
					{
						break;
					}

					if (CACHE_DEPEND_DOWN == dependency)
					{
						ramMapInfo.ebmCacheIndex = index;
					}
					else if (CACHE_DEPEND_UP_DOWN == dependency)
					{
						ramMapInfo.ebmCacheIndex = (UINT16)(index + 1);
					}
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
					ramMapInfo.indexOffset = (UINT8)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * (logicalEBNum + ebCount)) % FLUSH_RAM_TABLE_SIZE) / 2); // word address
#else
					ramMapInfo.indexOffset = (UINT16)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * (logicalEBNum + ebCount)) % FLUSH_RAM_TABLE_SIZE) / 2); // word address
#endif
					ramMapInfo.presentEBM = FTL_TRUE;
					ramMapInfo.presentPPA = FTL_TRUE;
					if (FTL_ERR_PASS != (status = CACHE_SetRamMap(devID, logicalEBNum + ebCount, &ramMapInfo)))
					{
						return status;
					}
				}
			}

			ebmCacheInfo.cacheStatus = CACHE_CLEAN;
			ebmCacheInfo.dependency = CACHE_DEPEND_DOWN;
			ebmCacheInfo.rLRUCount = 0x0;
			ebmCacheInfo.wLRUCount = 0x0;
			if(CACHE_DEPEND_DOWN == dependency)
			{
				if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, index, &ebmCacheInfo)))
				{
					return status;
				}
			}
			else
			{
				if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, (UINT16)(index+1), &ebmCacheInfo)))
				{
					return status;
				}
			}

			// EB 
			if(FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)sector2, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
			{
				return status;
			}

			if ((CACHE_EMPTY_ENTRY_INDEX != eBlockPPAMapInfo.entryIndex) && (CACHE_EMPTY_FLASH_LOG_ARRAY != eBlockPPAMapInfo.flashLogEBArrayCount))
			{
				if(CACHE_DEPEND_DOWN == dependency)
				{
					// Load
					if(FTL_ERR_PASS != (status = CACHE_FlashToCache(devID, eBlockPPAMapInfo, index, CACHE_EBLOCKMAP)))
					{
						return status;
					}
				}
				else
				{
					// Load
					if(FTL_ERR_PASS != (status = CACHE_FlashToCache(devID, eBlockPPAMapInfo, (UINT16)(index+1), CACHE_EBLOCKMAP)))
					{
						return status;
					}
				}
			}
			else
			{
				// can't find EBM table in flash
				if (CACHE_DEPEND_DOWN == dependency)
				{
					indexTarget = index;
				}
				else if (CACHE_DEPEND_UP_DOWN == dependency)
				{
					indexTarget = (UINT16)(index + 1);
				}

				for (i = 0; i < FLUSH_RAM_TABLE_SIZE; i++)
				{
					EBlockMappingCache[indexTarget + (devID * NUMBER_OF_DEVICES)][i] = EMPTY_BYTE;
				}

				for (ebCount = 0; ebCount < EBM_ENTRY_COUNT + 1/* Just in Case */; ebCount++)
				{
					if ((logicalEBNum + ebCount) >= NUMBER_OF_ERASE_BLOCKS)
					{
						continue;
					}

					if (!ebCount)
					{
						if (CACHE_DEPEND_DOWN == dependency)
						{
							TABLE_ClearMappingTable(devID, logicalEBNum + ebCount, logicalEBNum + ebCount, ERASE_STATUS_GET_DWORD_MASK | 1);
						}
					}
					else
					{
						sector6 = (((UINT32)logicalEBNum + 1 + ebCount) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) - 1) / FLUSH_RAM_TABLE_SIZE;
						if (sector + 2 == sector6)
						{
							break;
						}

						TABLE_ClearMappingTable(devID, logicalEBNum + ebCount, logicalEBNum + ebCount, ERASE_STATUS_GET_DWORD_MASK | 1);
					}
				}
			}

			// PPA
			if(FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)(sector2 * PPA_CACHE_TABLE_OFFSET), &eBlockPPAMapInfo, CACHE_PPAMAP)))
			{
				return status;
			}

			if ((CACHE_EMPTY_ENTRY_INDEX != eBlockPPAMapInfo.entryIndex) && (CACHE_EMPTY_FLASH_LOG_ARRAY != eBlockPPAMapInfo.flashLogEBArrayCount))
			{

				if(CACHE_DEPEND_DOWN == dependency)
				{
					// Load
					if(FTL_ERR_PASS != (status = CACHE_FlashToCache(devID, eBlockPPAMapInfo, index, CACHE_PPAMAP)))
					{
						return status;
					}
				}
				else
				{
					// Load
					if(FTL_ERR_PASS != (status = CACHE_FlashToCache(devID, eBlockPPAMapInfo, (UINT16)(index+1), CACHE_PPAMAP)))
					{
						return status;
					}
				}
			}
			else
			{

				// can't find PPA table in flash
				for (ebCount = 0; ebCount < EBM_ENTRY_COUNT + 1/* Just in Case */; ebCount++)
				{
					if ((logicalEBNum + ebCount) >= NUMBER_OF_ERASE_BLOCKS)
					{
						continue;
					}

					if (!ebCount)
					{
						if (CACHE_DEPEND_DOWN == dependency)
						{
							TABLE_ClearPPATable(devID, logicalEBNum + ebCount);
						}
					}
					else
					{
						sector6 = (((UINT32)logicalEBNum + 1 + ebCount) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) - 1) / FLUSH_RAM_TABLE_SIZE;
						if (sector + 2 == sector6)
						{
							break;
						}

						TABLE_ClearPPATable(devID, logicalEBNum + ebCount);
					}
				}
			}
		}
	}
	else
	{

		return FTL_ERR_CACHE_EB_INSERT_2;
	}

	// clear other cross boundary
	if(CACHE_NO_DEPEND != dependency)
	{
		/* Kick out data of original crossedLEB. */
		for (i = 0; i < NUM_CROSS_LEB; i++)
		{
			if ((gCrossedLEB[i] != logicalEBNum) && (gCrossedLEB[i] != EMPTY_WORD))
			{
				if (crossedLEB == EMPTY_WORD)
				{
					crossedLEB = gCrossedLEB[i];
				}
				else
				{
					DBG_Printf("Should be one candidate to be removed. \n", 0, 0);
					return FTL_ERR_CACHE_CROSS_LEB_EXCEED_ONE_1;
				}
			}
		}
		if (EMPTY_WORD != crossedLEB)
		{
			if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, crossedLEB, &ramMapInfo)))
			{
				return status;
			}
			if (FTL_ERR_PASS != (status = CACHE_ClearRamMap(devID, crossedLEB)))
			{
				return status;
			}

			if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, ramMapInfo.ebmCacheIndex)))
			{
				return status;
			}

			if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, (UINT16)(ramMapInfo.ebmCacheIndex + 1))))
			{
				return status;
			}
		}

		if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}

		if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
		{
			return status;
		}

		// set target cross boundary
		if (CACHE_WRITE_TYPE == typeAPI)
		{
			if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
			{
				ebmCacheInfo.cacheStatus = CACHE_DIRTY;
				gCounterDirty++;
			}
		}
		else
		{
			if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
			{
				ebmCacheInfo.cacheStatus = CACHE_CLEAN;
			}
		}
		ebmCacheInfo.dependency = CACHE_DEPEND_UP;
		ebmCacheInfo.rLRUCount = EMPTY_BYTE;
		ebmCacheInfo.wLRUCount = EMPTY_BYTE;
		if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
		{
			return status;
		}

		if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, (UINT16)(ramMapInfo.ebmCacheIndex + 1), &ebmCacheInfo)))
		{
			return status;
		}
		if (CACHE_WRITE_TYPE == typeAPI)
		{
			if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
			{
				ebmCacheInfo.cacheStatus = CACHE_DIRTY;
				gCounterDirty++;
			}
		}
		else
		{
			if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
			{
				ebmCacheInfo.cacheStatus = CACHE_CLEAN;
			}
		}
		ebmCacheInfo.dependency = CACHE_DEPEND_DOWN;
		ebmCacheInfo.rLRUCount = EMPTY_BYTE;
		ebmCacheInfo.wLRUCount = EMPTY_BYTE;
		if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, (UINT16)(ramMapInfo.ebmCacheIndex + 1), &ebmCacheInfo)))
		{
			return status;
		}
	}

	return FTL_ERR_PASS;
}

// Index EB Translation Interface
FTL_STATUS CACHE_GetIndex (FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
	{
		return status;
	}

	if(FTL_TRUE == ramMapInfo.presentEBM)
	{
		if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
		{
			return status;
		}
		*index_ptr = ramMapInfo.ebmCacheIndex;
		*dependency_ptr = ebmCacheInfo.dependency;
	}
	else
	{
		// not found
		*index_ptr = EMPTY_WORD;
		*dependency_ptr = EMPTY_BYTE;
		return FTL_ERR_CACHE_NOT_FOUND;
	}

	return status;
}

FTL_STATUS CACHE_GetEB (FTL_DEV devID, UINT16 index, UINT16_PTR logicalEBNum_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT16 logicalEBNum = 0;

	for(logicalEBNum = 0;logicalEBNum < NUMBER_OF_ERASE_BLOCKS;logicalEBNum++)
	{
		if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}
		if(FTL_TRUE == ramMapInfo.presentEBM)
		{
			if(index == ramMapInfo.ebmCacheIndex)
			{
				*logicalEBNum_ptr = logicalEBNum;
				return status;
			}
		}
	}

	// not found
	return FTL_ERR_CACHE_NOT_FOUND;
}

FTL_STATUS CACHE_GetSector(UINT16 logicalEBNum, UINT32_PTR sector_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;

	*sector_ptr = ((((UINT32)logicalEBNum) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))) / FLUSH_RAM_TABLE_SIZE;

	return status;
}

UINT8_PTR CACHE_GetEBMCachePtr(FTL_DEV devID, UINT16 index)
{
	return &(EBlockMappingCache[index + (devID * NUM_EBLOCK_MAP_INDEX)][0]);
}

UINT8_PTR CACHE_GetPPACachePtr(FTL_DEV devID, UINT16 index)
{
	return &(PPAMappingCache[index + (devID * NUM_EBLOCK_MAP_INDEX)][0]);
}

// Query Interface of Index State
FTL_STATUS CACHE_IsFreeIndex (FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID,index,&ebmCacheInfo)))
	{
		return status;
	}

	if(ebmCacheInfo.cacheStatus == CACHE_FREE)
	{
		return FTL_TRUE;
	}

	return FTL_FALSE;
}

FTL_STATUS CACHE_IsCleanIndex (FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID,index,&ebmCacheInfo)))
	{
		return status;
	}

	if(ebmCacheInfo.cacheStatus == CACHE_CLEAN)
	{
		return FTL_TRUE;
	}

	return FTL_FALSE;
}

FTL_STATUS CACHE_IsDirtyIndex (FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID,index,&ebmCacheInfo)))
	{
		return status;
	}

	if(ebmCacheInfo.cacheStatus == CACHE_DIRTY)
	{
		return FTL_TRUE;
	}

	return FTL_FALSE;

}

// Query Interface of LEB State
FTL_STATUS CACHE_IsPresentEB (FTL_DEV devID, UINT16 logicalEBNum, UINT8_PTR present_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};

	if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
	{
		return status;
	}

	if(FTL_TRUE == ramMapInfo.presentEBM && FTL_TRUE == ramMapInfo.presentPPA)
	{
		*present_ptr = CACHE_EBM_PPA_PRESENT;
	}
	else if(FTL_TRUE == ramMapInfo.presentEBM && FTL_FALSE == ramMapInfo.presentPPA)
	{
		*present_ptr = CACHE_EBM_PRESENT;
	}
	else if(FTL_FALSE == ramMapInfo.presentEBM && FTL_FALSE == ramMapInfo.presentPPA)
	{
		*present_ptr = CACHE_NO_PRESENT;
	}
	else
	{
		return FTL_FALSE;
	}

	return FTL_TRUE;
}

FTL_STATUS CACHE_IsCrossedEB (FTL_DEV devID, UINT16 logicalEBNum)
{
	UINT32 sector = 0; /*4*/
	UINT32 sector1 = 0; /*4*/

	sector = ((((UINT32) logicalEBNum) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) )
	/ FLUSH_RAM_TABLE_SIZE;

	sector1 = ((((UINT32) logicalEBNum + 1) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) - 1)
	/ FLUSH_RAM_TABLE_SIZE;
	if(sector != sector1)
	{
		return FTL_TRUE;
	}
	return FTL_FALSE;
}

// Structure Access Interface of EBlockMapIndex and PPAMapIndex
FTL_STATUS CACHE_SetEBlockAndPPAMap (FTL_DEV devID, UINT16 index, CACHE_INFO_EBLOCK_PPAMAP_PTR eBlockPPAMapInfo_ptr, UINT8 type)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 temp = 0x0;
	UINT16 temp2 = 0x0;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if(EBLOCK_SIZE > 0x1000)
	temp2 = (UINT16)(eBlockPPAMapInfo_ptr->flashLogEBArrayCount & 0x000F);
	temp = (UINT16)(temp |(temp2 << 12));
	temp = (UINT16)(temp |(eBlockPPAMapInfo_ptr->entryIndex & 0x0FFF));
#else
	temp2 = (UINT16)(eBlockPPAMapInfo_ptr->flashLogEBArrayCount & 0x00FF);
	temp = (UINT16)(temp | (temp2 << 8));
	temp = (UINT16)(temp |(eBlockPPAMapInfo_ptr->entryIndex & 0x00FF));
#endif
#else // NAND
	temp2 = (UINT16)(eBlockPPAMapInfo_ptr->flashLogEBArrayCount & 0x001F);
	temp = (UINT16)(temp |(temp2 << 10));
	temp = (UINT16)(temp |(eBlockPPAMapInfo_ptr->entryIndex & 0x03FF));
#endif
	if(type == CACHE_EBLOCKMAP)
	{
		EBlockMapIndex[index + (devID * MAX_EBLOCK_MAP_INDEX)] = temp;
	}
	else if(type == CACHE_PPAMAP)
	{
		PPAMapIndex[index + (devID * MAX_PPA_MAP_INDEX)] = temp;
	}
	else
	{
		return FTL_ERR_CACHE_SET_EBLOCKMAP;
	}
	return status;
}

FTL_STATUS CACHE_GetEBlockAndPPAMap (FTL_DEV devID, UINT16 index, CACHE_INFO_EBLOCK_PPAMAP_PTR eBlockPPAMapInfo_ptr, UINT8 type)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 temp = 0x0;

	if(type == CACHE_EBLOCKMAP)
	{
#ifdef DEBUG_PROTOTYPE
		gTestFlag = FTL_TRUE;
		if((((index * FLUSH_RAM_TABLE_SIZE) % (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) == 0))
		{
			glogicalEBtemp = (UINT16)((index * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
		}
		else
		{
			glogicalEBtemp = (UINT16)(((index * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1);
		}
#endif
		temp = EBlockMapIndex[index + (devID * MAX_EBLOCK_MAP_INDEX)];
	}
	else if(type == CACHE_PPAMAP)
	{
#ifdef DEBUG_PROTOTYPE
		gTestFlag = FTL_FALSE;

		if (((((index / PPA_CACHE_TABLE_OFFSET) * FLUSH_RAM_TABLE_SIZE) % (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) == 0))
		{
			glogicalEBtemp = (UINT16)(((index / PPA_CACHE_TABLE_OFFSET) * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
		}
		else
		{
			glogicalEBtemp = (UINT16)((((index / PPA_CACHE_TABLE_OFFSET) * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1);
		}
#endif
		temp = PPAMapIndex[index + (devID * NUM_PPA_MAP_INDEX)];
	}
	else
	{
		return FTL_ERR_CACHE_GET_EBLOCKMAP;
	}
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if(EBLOCK_SIZE > 0x1000)
	eBlockPPAMapInfo_ptr->flashLogEBArrayCount = (UINT8)((temp & 0xF000) >> 12);
	eBlockPPAMapInfo_ptr->entryIndex = (UINT16)(temp & 0x0FFF);
#else
	eBlockPPAMapInfo_ptr->flashLogEBArrayCount = (UINT8)((temp & 0xFF00) >> 8);
	eBlockPPAMapInfo_ptr->entryIndex = (UINT16)(temp & 0x00FF);
#endif
#else // NAND
	eBlockPPAMapInfo_ptr->flashLogEBArrayCount = (UINT8)((temp & 0x7C00) >> 10);
	eBlockPPAMapInfo_ptr->entryIndex = (UINT16)(temp & 0x03FF);
#endif
	return status;
}

FTL_STATUS CACHE_ClearEBlockandPPAMap (FTL_DEV devID, UINT16 index, UINT8 type)
{
	FTL_STATUS status = FTL_ERR_PASS;
	if(type == CACHE_EBLOCKMAP)
	{
		EBlockMapIndex[index + (devID * MAX_EBLOCK_MAP_INDEX)] = EMPTY_WORD;
	}
	else if(type == CACHE_PPAMAP)
	{
		PPAMapIndex[index + (devID * NUM_PPA_MAP_INDEX)] = EMPTY_WORD;
	}
	else
	{
		return FTL_ERR_CACHE_CLEAR_EBLOCKMAP;
	}
	return status;
}

// Structure Access Interface of RamMapIndex
FTL_STATUS CACHE_SetRamMap (FTL_DEV devID, UINT16 logicalEBNum, CACHE_INFO_RAMMAP_PTR ramMapInfo_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT32 temp = 0x0;
	UINT32 temp2 = 0x0;
	UINT32 i = 0;

	if (FTL_TRUE == CACHE_IsCrossedEB(devID, logicalEBNum))
	{
		if(NUMBER_OF_DATA_EBLOCKS > logicalEBNum)
		{
			for (i = 0; i < NUM_CROSS_LEB; i++)
			{
				if (gCrossedLEB[i] == EMPTY_WORD)
				{
					gCrossedLEB[i] = logicalEBNum;
					break;
				}
			}
			if (i == NUM_CROSS_LEB)
			{
				DBG_Printf("Set: Inconsistency between gCrossedLEB and LEB \n",0,0);
				return FTL_ERR_CACHE_CROSS_LEB_NO_SPACE;
			}
		}
	}

	temp = RamMapIndex[logicalEBNum + (devID * NUMBER_OF_ERASE_BLOCKS)];

	if(EMPTY_BYTE != ramMapInfo_ptr->presentEBM)
	{
		temp2 = (UINT32)ramMapInfo_ptr->presentEBM & 0x01;
		temp = 0x7FFFFFFF & temp;
		temp = temp |(temp2 << 31);
	}

	if(EMPTY_BYTE != ramMapInfo_ptr->presentPPA)
	{
		temp2 = (UINT32)ramMapInfo_ptr->presentPPA & 0x01;
		temp = 0xBFFFFFFF & temp;
		temp = temp |(temp2 << 30);
	}

	if(EMPTY_WORD != ramMapInfo_ptr->ebmCacheIndex)
	{
		temp2 = (UINT32)ramMapInfo_ptr->ebmCacheIndex & 0x3FFF;
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
		temp = 0xFFC000FF & temp;
		temp = temp |(temp2 << 8);
#else
		temp = 0xC000FFFF & temp;
		temp = temp | (temp2 << 16);
#endif
	}
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
	if(EMPTY_BYTE != ramMapInfo_ptr->indexOffset)
#else
	if(EMPTY_WORD != ramMapInfo_ptr->indexOffset)
#endif
	{
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
		temp2 = (UINT32)ramMapInfo_ptr->indexOffset & 0xFF;
		temp = 0xFFFFFF00 & temp;
#else
		temp2 = (UINT32)ramMapInfo_ptr->indexOffset & 0xFFFF;
		temp = 0xFFFF0000 & temp;
#endif
		temp = temp | temp2;
	}

	RamMapIndex[logicalEBNum + (devID * NUMBER_OF_ERASE_BLOCKS)] = temp;

	return status;
}

FTL_STATUS CACHE_GetRamMap (FTL_DEV devID, UINT16 logicalEBNum, CACHE_INFO_RAMMAP_PTR ramMapInfo_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT32 temp = 0x0;

	temp = RamMapIndex[logicalEBNum + (devID * NUMBER_OF_ERASE_BLOCKS)];

	ramMapInfo_ptr->presentEBM = (UINT8)((temp >> 31) & 0x01);
	ramMapInfo_ptr->presentPPA = (UINT8)((temp >> 30) & 0x01);
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
	ramMapInfo_ptr->ebmCacheIndex = (UINT16)((temp >> 8) & 0x3FFF);
	ramMapInfo_ptr->indexOffset = (UINT8)(temp & 0xFF);
#else
	ramMapInfo_ptr->ebmCacheIndex = (UINT16)((temp >> 16) & 0x3FFF);
	ramMapInfo_ptr->indexOffset = (UINT16)(temp & 0xFFFF);
#endif

	return status;
}

FTL_STATUS CACHE_ClearRamMap (FTL_DEV devID,UINT16 logicalEBNum)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT32 i = 0;

	if (FTL_TRUE == CACHE_IsCrossedEB(devID, logicalEBNum))
	{
		if(NUMBER_OF_DATA_EBLOCKS > logicalEBNum)
		{
			for (i = 0; i < NUM_CROSS_LEB; i++)
			{
				if (gCrossedLEB[i] == logicalEBNum)
				{
					gCrossedLEB[i] = EMPTY_WORD;
					break;
				}
			}
			if (i == NUM_CROSS_LEB)
			{
				DBG_Printf("Clear: Inconsistency between gCrossedLEB and LEB \n",0,0);
				return FTL_ERR_CACHE_CROSS_LEB_NOT_FOUND;
			}
		}
	}

	RamMapIndex[logicalEBNum + (devID * NUMBER_OF_ERASE_BLOCKS)] = CACHE_INIT_RAM_MAP_INDEX;
	return status;
}

FTL_STATUS CACHE_MoveRamMap(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT16 fromLogicalEBNum = EMPTY_WORD;
	UINT32 fromSector = EMPTY_DWORD;
	UINT32 fromSector1 = EMPTY_DWORD;
	UINT16 logicalEBNum = 0;

	if (FTL_ERR_PASS != (status = CACHE_GetEB(devID, fromIndex, &fromLogicalEBNum)))
	{
		return status;
	}

	if (FTL_ERR_PASS != (status = CACHE_GetSector(fromLogicalEBNum, &fromSector)))
	{
		return status;
	}

	// check if previous sector is cross or not
	if (FTL_TRUE == CACHE_IsCrossedEB(devID, (UINT16)(fromLogicalEBNum - 1)))
	{
		if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, (UINT16)(fromLogicalEBNum - 1), &ramMapInfo)))
		{
			return status;
		}
		if (ramMapInfo.ebmCacheIndex != CACHE_EMPTY_EBM_CACHE_INDEX)
		{
			if (FTL_ERR_PASS != (status = CACHE_ClearRamMap(devID, (UINT16)(fromLogicalEBNum - 1))))
			{
				return status;
			}
		}
	}

	for (logicalEBNum = fromLogicalEBNum; logicalEBNum < NUMBER_OF_ERASE_BLOCKS; logicalEBNum++)
	{

		if (FTL_ERR_PASS != (status = CACHE_GetSector(logicalEBNum, &fromSector1)))
		{
			return status;
		}
		if (fromSector != fromSector1)
		{
			break;
		}
		else
		{
			if (FTL_TRUE == CACHE_IsCrossedEB(devID, logicalEBNum))
			{
				if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
				{
					return status;
				}
				if (ramMapInfo.ebmCacheIndex != CACHE_EMPTY_EBM_CACHE_INDEX)
				{
					if (FTL_ERR_PASS != (status = CACHE_ClearRamMap(devID, logicalEBNum)))
					{
						return status;
					}
				}
				break;
			}
		}

		if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}

		ramMapInfo.ebmCacheIndex = toIndex;
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
		ramMapInfo.indexOffset = EMPTY_BYTE;
#else
		ramMapInfo.indexOffset = EMPTY_WORD;
#endif
		ramMapInfo.presentEBM = EMPTY_BYTE;
		ramMapInfo.presentPPA = EMPTY_BYTE;
		if (FTL_ERR_PASS != (status = CACHE_SetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}
	}

	return status;
}

// Structure Access Interface of EBMCacheIndex
FTL_STATUS CACHE_SetEBMCache (FTL_DEV devID, UINT16 index, CACHE_INFO_EBMCACHE_PTR ebmCacheInfo_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 temp = 0x0;
	UINT16 temp2 = 0x0;

	if (index >= NUM_EBLOCK_MAP_INDEX)
	return FTL_ERR_CACHE_INDEX_OUT_OF_SCOPE_1;

	temp = EBMCacheIndex[index + (devID * NUM_EBLOCK_MAP_INDEX)];

	if(EMPTY_BYTE != ebmCacheInfo_ptr->cacheStatus)
	{
		temp2 = (UINT16)(ebmCacheInfo_ptr->cacheStatus & 0x03);
		temp = (UINT16)(0x3FFF & temp);
		temp = (UINT16)(temp |(temp2 << 14));
	}

	if (index > CACHE_INDEX_CHANGE_AREA)
	{
		ebmCacheInfo_ptr->dependency = CACHE_NO_DEPEND;
	}
	if ((index == CACHE_INDEX_CHANGE_AREA) && (ebmCacheInfo_ptr->dependency == CACHE_DEPEND_UP))
	{
		ebmCacheInfo_ptr->dependency = CACHE_NO_DEPEND;
	}
	if (EMPTY_BYTE != ebmCacheInfo_ptr->dependency)
	{
		temp2 = (UINT16)(ebmCacheInfo_ptr->dependency & 0x03);
		temp = (UINT16)(0xCFFF & temp);
		temp = (UINT16)(temp | (temp2 << 12));
	}

	if(EMPTY_BYTE != ebmCacheInfo_ptr->wLRUCount)
	{
		temp2 = (UINT16)(ebmCacheInfo_ptr->wLRUCount & 0x3F);
		temp = (UINT16)(0xF03F & temp);
		temp = (UINT16)(temp |(temp2 << 6));
	}

	if(EMPTY_BYTE != ebmCacheInfo_ptr->rLRUCount)
	{
		temp2 = (UINT16)(ebmCacheInfo_ptr->rLRUCount & 0x3F);
		temp = (UINT16)(0xFFC0 & temp);
		temp = (UINT16)(temp | temp2);
	}

	EBMCacheIndex[index + (devID * NUM_EBLOCK_MAP_INDEX)] = temp;
	return status;
}

FTL_STATUS CACHE_GetEBMCache (FTL_DEV devID, UINT16 index, CACHE_INFO_EBMCACHE_PTR ebmCacheInfo_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 temp = 0x0;

	if (index >= NUM_EBLOCK_MAP_INDEX)
	return FTL_ERR_CACHE_INDEX_OUT_OF_SCOPE_2;

	temp = EBMCacheIndex[index + (devID * NUM_EBLOCK_MAP_INDEX)];

	ebmCacheInfo_ptr->cacheStatus = (UINT8)((temp >> 14) & 0x03);
	ebmCacheInfo_ptr->dependency = (UINT8)((temp >> 12) & 0x03);
	ebmCacheInfo_ptr->wLRUCount = (UINT16)((temp >> 6) & 0x3F);
	ebmCacheInfo_ptr->rLRUCount = (UINT8)(temp & 0xFF);

	return status;
}

FTL_STATUS CACHE_ClearEBMCache (FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;

	if (index >= NUM_EBLOCK_MAP_INDEX)
	return FTL_ERR_CACHE_INDEX_OUT_OF_SCOPE_3;

	EBMCacheIndex[index + (devID * NUM_EBLOCK_MAP_INDEX)] = CACHE_INIT_EBM_CACHE_INDEX;
	return status;
}

FTL_STATUS CACHE_MoveEBMCache(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};

	if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, fromIndex, &ebmCacheInfo)))
	{
		return status;
	}

	// check partner
	if (CACHE_DEPEND_UP == ebmCacheInfo.dependency)
	{

		if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, (UINT16)(fromIndex + 1))))
		{
			return status;
		}
	}
	else if (CACHE_DEPEND_DOWN == ebmCacheInfo.dependency)
	{

		if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, (UINT16)(fromIndex - 1))))
		{
			return status;
		}
	}

	ebmCacheInfo.dependency = CACHE_NO_DEPEND; // change no depend
	if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, toIndex, &ebmCacheInfo)))
	{
		return status;
	}

	// clear from
	if (FTL_ERR_PASS != (status = CACHE_ClearEBMCache(devID, fromIndex)))
	{
		return status;
	}

#ifdef DEBUG_DATA_CLEAR
	DEBUG_CACHE_SetEBMCacheFree(devID, fromIndex);
#endif

	return status;
}

// Structure Access Interface of gLRUCounter
FTL_STATUS CACHE_SetgLRUCounter(UINT8 counterLRU)
{
	FTL_STATUS status = FTL_ERR_PASS;
	gCounterLRU = counterLRU;
	return status;
}

FTL_STATUS CACHE_GetgLRUCounter(UINT8_PTR counterLRU_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	*counterLRU_ptr = gCounterLRU;
	return status;
}

FTL_STATUS CACHE_CleargLRUCounter(void)
{
	FTL_STATUS status = FTL_ERR_PASS;
	gCounterLRU = 0;
	return status;
}

FTL_STATUS CACHE_MarkEBDirty(FTL_DEV devID, UINT16 logicalEBNum)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};

	if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
	{
		return status;
	}

	if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
	{
		return status;
	}
	if(CACHE_DIRTY != ebmCacheInfo.cacheStatus)
	{
		ebmCacheInfo.cacheStatus = CACHE_DIRTY;
		ebmCacheInfo.dependency = EMPTY_BYTE;
		ebmCacheInfo.rLRUCount = EMPTY_BYTE;
		ebmCacheInfo.wLRUCount = EMPTY_BYTE;
		if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
		{
			return status;
		}
		gCounterDirty++;
		if(NUM_DATA_EBLOCKS > logicalEBNum)
		{
			gDataAreaCounterDirty++;
		}
	}
	if(FTL_TRUE == CACHE_IsCrossedEB(devID, logicalEBNum))
	{
		if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, (UINT16)(logicalEBNum+1), &ramMapInfo)))
		{
			return status;
		}

		if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
		{
			return status;
		}
		if(CACHE_DIRTY == ebmCacheInfo.cacheStatus)
		{
			return status; // skip
		}

		ebmCacheInfo.cacheStatus = CACHE_DIRTY;
		ebmCacheInfo.dependency = EMPTY_BYTE;
		ebmCacheInfo.rLRUCount = EMPTY_BYTE;
		ebmCacheInfo.wLRUCount = EMPTY_BYTE;
		if(FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
		{
			return status;
		}
		gCounterDirty++;
		if (NUM_DATA_EBLOCKS > logicalEBNum)
		{
			gDataAreaCounterDirty++;
		}
	}
	return status;
}

FTL_STATUS CACHE_FindIndexNeitherPresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT8 miniLRU1 = EMPTY_BYTE;
	UINT8 miniLRU2 = EMPTY_BYTE;
	UINT8 tempLRU = EMPTY_BYTE;
	UINT16 index = 0;
	UINT8 checkFree = FTL_FALSE;
	UINT16 candidateIndex1 = EMPTY_WORD;
	UINT16 candidateIndex2 = EMPTY_WORD;

	// find Free
	for(index = 0; index < CACHE_INDEX_CHANGE_AREA; index++)
	{
		if(FTL_TRUE == CACHE_IsFreeIndex(devID, index))
		{
			if(FTL_TRUE == checkFree)
			{
				*dependency_ptr = CACHE_DEPEND_UP_DOWN;
				break;
			}
			else
			{
				*index_ptr = index;
				checkFree = FTL_TRUE;
			}
		}
		else
		{
			// not contiguous free area
			checkFree = FTL_FALSE;
			*index_ptr = EMPTY_BYTE;// reset
		}

		if(FTL_TRUE == CACHE_IsDirtyIndex(devID, index))
		{
			continue;
		}

		if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
		{
			return status;
		}
		tempLRU = ebmCacheInfo.rLRUCount;
		tempLRU = (UINT8)(ebmCacheInfo.wLRUCount + tempLRU);
		tempLRU = (UINT8)(tempLRU / 2); // average

		if(miniLRU1 >= tempLRU || candidateIndex2 == EMPTY_WORD)
		{
			if(miniLRU1 >= tempLRU)
			{
				miniLRU1 = tempLRU;
				candidateIndex2 = candidateIndex1; // second
				candidateIndex1 = index;// first
			}
			else
			{
				if(miniLRU2 >= tempLRU)
				{
					miniLRU2 = tempLRU;
					candidateIndex2 = index; // second
				}
			}
		}
	}

	// no free case. create contiguous no dirty area.
	if(CACHE_DEPEND_UP_DOWN != *dependency_ptr)
	{
		// check edge
		if((CACHE_INDEX_CHANGE_AREA-1) == candidateIndex1)
		{
			if((UINT16)(candidateIndex1 - 1) != candidateIndex2)
			{
				if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(candidateIndex1 - 1)))
				{
					if(FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, (UINT16)(candidateIndex1 - 1), candidateIndex2)))
					{
						return status;
					}
				}
			}
			else
			{
				if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(candidateIndex1 - 1)))
				{
					if(FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, (UINT16)(candidateIndex1 - 1))))
					{
						return status;
					}
				}
			}

			if (FTL_TRUE != CACHE_IsFreeIndex(devID, candidateIndex1))
			{
				if (FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, candidateIndex1)))
				{
					return status;
				}
			}

			*index_ptr = (UINT16)(candidateIndex1 - 1);
		}
		else
		{
			if((UINT16)(candidateIndex1 + 1) != candidateIndex2)
			{
				if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(candidateIndex1 + 1)))
				{
					if(FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, (UINT16)(candidateIndex1 + 1), candidateIndex2)))
					{
						return status;
					}
				}
			}
			else
			{
				if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(candidateIndex1 + 1)))
				{
					if(FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, (UINT16)(candidateIndex1 + 1))))
					{
						return status;
					}
				}
			}

			if (FTL_TRUE != CACHE_IsFreeIndex(devID, candidateIndex1))
			{
				if (FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, candidateIndex1)))
				{
					return status;
				}
			}

			*index_ptr = candidateIndex1;
		}

		*dependency_ptr = CACHE_DEPEND_UP_DOWN;
	}

	return status;
}

FTL_STATUS CACHE_FindIndexUpsidePresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT16 indexUp)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT8 miniLRU1 = EMPTY_BYTE;
	UINT16 candidateIndex1 = EMPTY_WORD;

	// check edge
	if((CACHE_INDEX_CHANGE_AREA-1) == indexUp)
	{
		miniLRU1 = EMPTY_BYTE;
		candidateIndex1 = EMPTY_WORD;

		if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, &candidateIndex1, indexUp, EMPTY_WORD, FTL_FALSE /* No removed */)))
		{
			return status;
		}

		// move to no edge 
		if(FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, indexUp, candidateIndex1)))
		{
			return status;
		}
		// set
		indexUp = candidateIndex1;
	}

	if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, (UINT16)(indexUp + 1), &ebmCacheInfo)))
	{
		return status;
	}
	if(CACHE_FREE != ebmCacheInfo.cacheStatus)
	{

		candidateIndex1 = EMPTY_WORD;
		miniLRU1 = EMPTY_BYTE;

		if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, &candidateIndex1, indexUp, EMPTY_WORD, FTL_FALSE /* No removed */)))
		{
			return status;
		}

		// swap partner
		if((UINT16)(indexUp + 1) == candidateIndex1)
		{
			if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(indexUp + 1)))
			{
				if(FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, candidateIndex1)))
				{
					return status;
				}
			}
		}
		else
		{
			if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(indexUp + 1)))
			{
				if(FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, (UINT16)(indexUp + 1), candidateIndex1)))
				{
					return status;
				}
			}
		}
	}

	// set
	*index_ptr = (UINT16)(indexUp + 1);//candidateIndex1;
	*dependency_ptr = CACHE_DEPEND_DOWN;

	return status;
}

FTL_STATUS CACHE_FindIndexDownsidePresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT16 indexDown)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT8 miniLRU1 = EMPTY_BYTE;
	UINT16 candidateIndex1 = EMPTY_WORD;

	// check edge
	if(0 == indexDown)
	{
		miniLRU1 = EMPTY_BYTE;
		candidateIndex1 = EMPTY_WORD;

		if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, &candidateIndex1, indexDown, EMPTY_WORD, FTL_FALSE /* No removed */)))
		{
			return status;
		}

		// move
		if(FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, indexDown, candidateIndex1)))
		{
			return status;
		}

		// set
		indexDown = candidateIndex1;
	}

	if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, (UINT16)(indexDown - 1), &ebmCacheInfo)))
	{
		return status;
	}
	if(CACHE_FREE != ebmCacheInfo.cacheStatus)
	{

		miniLRU1 = EMPTY_BYTE;
		candidateIndex1 = EMPTY_WORD;

		if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, &candidateIndex1, indexDown, EMPTY_WORD, FTL_FALSE /* No removed */)))
		{
			return status;
		}

		// swap
		if((UINT16)(indexDown - 1) == candidateIndex1)
		{
			if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(indexDown - 1)))
			{
				if(FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, candidateIndex1)))
				{
					return status;
				}
			}
		}
		else
		{
			if(FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(indexDown - 1)))
			{
				if(FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, (UINT16)(indexDown - 1), candidateIndex1)))
				{
					return status;
				}
			}
		}
	}

	// set
	*index_ptr = (UINT16)(indexDown - 1);
	*dependency_ptr = CACHE_DEPEND_UP;

	return status;
}

FTL_STATUS CACHE_FindIndexBothPresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT16 indexUp, UINT16 indexDown, UINT16 logicalEBNum, UINT8 typeAPI)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT8 miniLRU1 = EMPTY_BYTE;
	UINT16 candidateIndex1 = EMPTY_WORD;
	UINT16 crossedLEB = EMPTY_WORD;
	UINT32 i = 0;

	// check edge
	if (0 == indexDown)
	{
		miniLRU1 = EMPTY_BYTE;
		candidateIndex1 = EMPTY_WORD;

		if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, &candidateIndex1, indexUp, indexDown, FTL_FALSE /* No removed */)))
		{
			return status;
		}

		// move
		if (FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, indexDown, candidateIndex1)))
		{
			return status;
		}

		// set
		indexDown = candidateIndex1;
	}

	if ((UINT16)(indexUp + 1) != indexDown)
	{
		// swap partner
		if (FTL_TRUE != CACHE_IsFreeIndex(devID, (UINT16)(indexDown - 1)))
		{
			miniLRU1 = EMPTY_BYTE;
			candidateIndex1 = EMPTY_WORD;

			if (FTL_ERR_PASS != (status = CACHE_FindIndexForAlign(devID, &candidateIndex1, indexUp, indexDown, FTL_FALSE /* No removed */)))
			{
				return status;
			}

			if ((UINT16)(indexDown - 1) == candidateIndex1)
			{
				if (FTL_ERR_PASS != (status = CACHE_RemoveIndex(devID, (UINT16)(indexDown - 1))))
				{
					return status;
				}
			}
			else
			{
				if (FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, (UINT16)(indexDown - 1), candidateIndex1)))
				{
					return status;
				}
			}
		}

		if (FTL_ERR_PASS != (status = CACHE_MoveIndex(devID, indexUp, (UINT16)(indexDown - 1))))
		{
			return status;
		}
	} /* if ((indexUp + 1) != indexDown) */

	/* Kick out data of original crossedLEB. */
	for (i = 0; i < NUM_CROSS_LEB; i++)
	{
		if ((gCrossedLEB[i] != logicalEBNum) && (gCrossedLEB[i] != EMPTY_WORD))
		{
			if (crossedLEB == EMPTY_WORD)
			{
				crossedLEB = gCrossedLEB[i];
			}
			else
			{
				DBG_Printf("Should be one candidate to be removed. \n", 0, 0);
				return FTL_ERR_CACHE_CROSS_LEB_EXCEED_ONE_2;
			}
		}
	}
	if (EMPTY_WORD != crossedLEB)
	{
		if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, crossedLEB, &ramMapInfo)))
		{
			return status;
		}
		if (FTL_ERR_PASS != (status = CACHE_ClearRamMap(devID, crossedLEB)))
		{
			return status;
		}

		if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, ramMapInfo.ebmCacheIndex)))
		{
			return status;
		}

		if (FTL_ERR_PASS != (status = CACHE_NoDpendIndex(devID, (UINT16)(ramMapInfo.ebmCacheIndex + 1))))
		{
			return status;
		}
	}

	/* RamMapIndex set here, since insert operation is not required. */
	ramMapInfo.ebmCacheIndex = indexDown - 1;
#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
	ramMapInfo.indexOffset = (UINT8)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * logicalEBNum) % FLUSH_RAM_TABLE_SIZE) / 2);
#else
	ramMapInfo.indexOffset = (UINT16)((((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) * logicalEBNum) % FLUSH_RAM_TABLE_SIZE) / 2);
#endif
	ramMapInfo.presentEBM = FTL_TRUE;
	ramMapInfo.presentPPA = FTL_TRUE;
	if (FTL_ERR_PASS != (status = CACHE_SetRamMap(devID, logicalEBNum, &ramMapInfo)))
	{
		return status;
	}

	// set up depend
	if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, (UINT16)(indexDown - 1), &ebmCacheInfo)))
	{
		return status;
	}
	if (CACHE_WRITE_TYPE == typeAPI)
	{
		if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
		{
			gCounterDirty++;
			ebmCacheInfo.cacheStatus = CACHE_DIRTY;
		}
	}
	else
	{
		if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
		{
			ebmCacheInfo.cacheStatus = CACHE_CLEAN;
		}
	}
	ebmCacheInfo.dependency = CACHE_DEPEND_UP;
	ebmCacheInfo.rLRUCount = EMPTY_BYTE;
	ebmCacheInfo.wLRUCount = EMPTY_BYTE;
	if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, (UINT16)(indexDown - 1), &ebmCacheInfo)))
	{
		return status;
	}

	// set down depend
	if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, indexDown, &ebmCacheInfo)))
	{
		return status;
	}
	if (CACHE_WRITE_TYPE == typeAPI)
	{
		if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
		{
			gCounterDirty++;
			ebmCacheInfo.cacheStatus = CACHE_DIRTY;
		}
	}
	else
	{
		if (CACHE_DIRTY != ebmCacheInfo.cacheStatus)
		{
			ebmCacheInfo.cacheStatus = CACHE_CLEAN;
		}
	}
	ebmCacheInfo.dependency = CACHE_DEPEND_DOWN;
	ebmCacheInfo.rLRUCount = EMPTY_BYTE;
	ebmCacheInfo.wLRUCount = EMPTY_BYTE;
	if (FTL_ERR_PASS != (status = CACHE_SetEBMCache(devID, indexDown, &ebmCacheInfo)))
	{
		return status;
	}

	// set (No insert required.)
	*index_ptr = EMPTY_WORD;
	*dependency_ptr = EMPTY_BYTE;

	return status;
}

// Load Flush to Cache
FTL_STATUS CACHE_FlashToCache(FTL_DEV devID, CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo, UINT16 toIndex, UINT8 type)
{
	FTL_STATUS status = FTL_ERR_PASS;
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	FLASH_STATUS flash_status = FLASH_PASS;
#endif
#ifdef DEBUG_PROTOTYPE
	UINT32 cal = 0;
	UINT16 count = 0;
	UINT16 ppaNum = 0;
	UINT32 i = 0;
	UINT32 sector = 0;
	UINT32 sector2 = 0;
	UINT16 checklog = 0;
	UINT16 checklog_save = 0;
	UINT32 temp = 0;
#if (CACHE_EB_PAD == FTL_TRUE)
	UINT8 temp2 = 0;
#endif

	if(FTL_TRUE == gTestFlag)
	{
		// cal for test
		cal = (UINT32)(eBlockPPAMapInfo.entryIndex * FLUSH_RAM_TABLE_SIZE);
		cal = (UINT32)(cal + (eBlockPPAMapInfo.flashLogEBArrayCount * (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK * FLUSH_RAM_TABLE_SIZE)));

		// copy
		for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
		{
#if (CACHE_EB_PAD == FTL_TRUE)

			if(temp2 >= EBLOCK_MAPPING_ENTRY_SIZE)
			{
				EBlockMappingCache[toIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count] = EMPTY_BYTE;
			}
			else
			{
				EBlockMappingCache[toIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count] = EBlockMappingTable[(devID * NUMBER_OF_ERASE_BLOCKS*EBLOCK_MAPPING_ENTRY_SIZE) + (glogicalEBtemp * EBLOCK_MAPPING_ENTRY_SIZE) + count];
				temp2++;
			}
#else
			EBlockMappingCache[toIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count] = EBlockMappingTable[(devID * NUMBER_OF_ERASE_BLOCKS*(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + cal + count];
#endif
		}
	}

	// PPA 
	if(FTL_FALSE == gTestFlag)
	{
		eBlockPPAMapInfo.flashLogEBArrayCount = (UINT8)(eBlockPPAMapInfo.flashLogEBArrayCount - arrayOffsetTemp);
		cal = (UINT32)((eBlockPPAMapInfo.entryIndex * FLUSH_RAM_TABLE_SIZE));
		cal = (UINT32)(cal + (eBlockPPAMapInfo.flashLogEBArrayCount * (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK * FLUSH_RAM_TABLE_SIZE)));

		// check       

		sector = ((((UINT32) glogicalEBtemp) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
		if((((sector * FLUSH_RAM_TABLE_SIZE) % (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) == 0))
		{
			checklog = (UINT16)((sector * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
		}
		else
		{
			checklog = (UINT16)(((sector * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1);
		}
		checklog_save = (UINT16)checklog;

		i = 0;

		while(1)
		{
			sector2 = ((((UINT32) checklog) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
			if(sector2 == sector)
			{
				i++; // logicalEB count 
			}
			else
			{
				break;
			}
			checklog++;
		}
		temp = i * NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE;
		i = 0;
		for(ppaNum = 0; ppaNum < PPA_CACHE_TABLE_OFFSET; ppaNum++)
		{
			for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
			{
				if(i >= temp)
				{
					PPAMappingCache[(toIndex * PPA_CACHE_TABLE_OFFSET) + ppaNum + (devID * NUM_EBLOCK_MAP_INDEX)][count] = EMPTY_BYTE;
				}
				else
				{
					PPAMappingCache[(toIndex * PPA_CACHE_TABLE_OFFSET) + ppaNum + (devID * NUM_EBLOCK_MAP_INDEX)][count] = PPAMappingTable[((checklog_save * NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)+(ppaNum * FLUSH_RAM_TABLE_SIZE)) + count];
					i++;
				}
			}
		}
	}
#else
	UINT16 logicalEBNum = 0;
	UINT16 phyAddr = 0;

	FLASH_PAGE_INFO flushStructPageInfo =
	{	0, 0,
		{	0, 0}};
	FLASH_PAGE_INFO flushRAMTablePageInfo =
	{	0, 0,
		{	0, 0}};
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT32 key = 0;
	UINT16 count = 0;
	UINT16 saveToIndex = 0;
	UINT16 blockNum = 0;
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	UINT8 tempArray[SECTOR_SIZE];
#endif

	TABLE_GetFlushLogCountEntry(devID, eBlockPPAMapInfo.flashLogEBArrayCount, &blockNum);
	if((status = TABLE_GetFlushLogEntry(devID, blockNum, &logicalEBNum, &phyAddr, &key)) != FTL_ERR_PASS)
	{
		return status;
	}

	if((status = GetFlushLoc(devID, phyAddr, eBlockPPAMapInfo.entryIndex, &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
	{
		return status;
	}

	if(CACHE_EBLOCKMAP == type)
	{
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
		//if((FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, &(EBlockMappingCache[toIndex + (devID * NUM_EBLOCK_MAP_INDEX)][0]))) != FLASH_PASS)
		if((FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, (UINT8_PTR)CACHE_GetEBMCachePtr(devID, toIndex))) != FLASH_PASS)
		{
			return FTL_ERR_CACHE_FLASH_READ_01;
		}
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
		flash_status = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, tempArray);
		MEM_Memcpy((UINT8_PTR)CACHE_GetEBMCachePtr(devID, toIndex), &tempArray[FLUSH_INFO_SIZE], FLUSH_RAM_TABLE_SIZE);
		if (flash_status != FLASH_PASS)
		{
			return FTL_ERR_CACHE_FLASH_READ_02;
		}
#endif
	}
	else if(CACHE_PPAMAP == type)
	{
		saveToIndex = toIndex;
		toIndex = (UINT16)(PPA_CACHE_TABLE_OFFSET * toIndex);

		for(count = 0; count < PPA_CACHE_TABLE_OFFSET; count++)
		{
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
			if((FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, CACHE_GetPPACachePtr(devID, toIndex))) != FLASH_PASS)
			{
				return FTL_ERR_CACHE_FLASH_READ_03;
			}
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
			flash_status = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, tempArray);
			MEM_Memcpy((UINT8_PTR)CACHE_GetPPACachePtr(devID, toIndex), &tempArray[FLUSH_INFO_SIZE], FLUSH_RAM_TABLE_SIZE);
			if (flash_status != FLASH_PASS)
			{
				return FTL_ERR_CACHE_FLASH_READ_04;
			}
#endif

			// end
			if(PPA_CACHE_TABLE_OFFSET == (count + 1))
			{
				break;
			}

			// For next PPA table
			if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, saveToIndex, &ebmCacheInfo)))
			{
				return status;
			}

			eBlockPPAMapInfo.entryIndex++; // next table
			if(MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK == eBlockPPAMapInfo.entryIndex)// [Note] I'll need to check them at brushup.
			{
				eBlockPPAMapInfo.entryIndex = 1;
				eBlockPPAMapInfo.flashLogEBArrayCount++;
			}

			TABLE_GetFlushLogCountEntry(devID, eBlockPPAMapInfo.flashLogEBArrayCount, &blockNum);

			if ((status = TABLE_GetFlushLogEntry(devID, blockNum, &logicalEBNum, &phyAddr, &key)) != FTL_ERR_PASS)
			{
				return status;
			}

			if((status = GetFlushLoc(devID, phyAddr, eBlockPPAMapInfo.entryIndex, &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
			{
				return status;
			}

			toIndex++;

		}
	}
#endif

	return status;
}

// Load Cache to Flush
FTL_STATUS CACHE_CacheToFlash(FTL_DEV devID, UINT16 fromIndex, CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo, UINT8 type, UINT8 flushMode)
{
	FTL_STATUS status = FTL_ERR_PASS;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	FLASH_STATUS flashStatus = FLASH_PASS;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
#ifdef DEBUG_PROTOTYPE
	UINT32 cal = 0;
	UINT16 count = 0;
	UINT16 ppaNum = 0;
	UINT16 checklog = 0;
	UINT16 checklog_save = 0;
	UINT32 sector = 0;
	UINT32 sector2 = 0;
	UINT32 i = 0;
	UINT32 temp = 0;

	// cal for test
	if(FTL_TRUE == gTestFlag)
	{
		cal = (UINT32)(eBlockPPAMapInfo.entryIndex * FLUSH_RAM_TABLE_SIZE);
		cal = (UINT32)(cal + (eBlockPPAMapInfo.flashLogEBArrayCount * (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK * FLUSH_RAM_TABLE_SIZE)));

#if (CACHE_EB_PAD == FTL_TRUE)
		for(count = 0; count < EBLOCK_MAPPING_ENTRY_SIZE; count++)
		{
			EBlockMappingTable[(devID * NUMBER_OF_ERASE_BLOCKS*EBLOCK_MAPPING_ENTRY_SIZE) + (glogicalEBtemp * EBLOCK_MAPPING_ENTRY_SIZE) + count] = EBlockMappingCache[fromIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count];
		}
#else
		for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
		{
			EBlockMappingTable[(devID * NUMBER_OF_ERASE_BLOCKS*(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + cal + count] = EBlockMappingCache[fromIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count];
		}
#endif

	}

	// PPA Todo
	if(FTL_FALSE == gTestFlag)
	{
		eBlockPPAMapInfo.flashLogEBArrayCount = (UINT8)(eBlockPPAMapInfo.flashLogEBArrayCount - arrayOffsetTemp);
		cal = (UINT32)(eBlockPPAMapInfo.entryIndex * FLUSH_RAM_TABLE_SIZE);
		cal = (UINT32)(cal + (eBlockPPAMapInfo.flashLogEBArrayCount * (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK * FLUSH_RAM_TABLE_SIZE)));

		sector = ((((UINT32) glogicalEBtemp) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
		if((((sector * FLUSH_RAM_TABLE_SIZE) % (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) == 0))
		{
			checklog = (UINT16)((sector * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
		}
		else
		{
			checklog = (UINT16)(((sector * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1);
		}
		checklog_save = (UINT16)checklog;

		i = 0;
		while(1)
		{
			sector2 = ((((UINT32) checklog) * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) ) / FLUSH_RAM_TABLE_SIZE;
			if(sector2 == sector)
			{
				i++; // logicalEB count 
			}
			else
			{
				break;
			}
			checklog++;
		}
		temp = i * NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE;
		i = 0;
		for(ppaNum = 0; ppaNum < PPA_CACHE_TABLE_OFFSET; ppaNum++)
		{
			for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
			{
				PPAMappingTable[((checklog_save * NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)+(ppaNum * FLUSH_RAM_TABLE_SIZE)) + count] = PPAMappingCache[(fromIndex * PPA_CACHE_TABLE_OFFSET) + ppaNum + (devID * NUM_EBLOCK_MAP_INDEX)][count];
				i++;
				if(i >= temp)
				{
					break;
				}
			}
			if(i >= temp)
			{
				break;
			}
		}
	}

#else

	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT16 count = 0;
	UINT32 sector = 0;
	LOG_ENTRY_LOC nextLoc =
	{	0, 0}; /*4*/
	FLASH_PAGE_INFO flushStructPageInfo =
	{	0, 0,
		{	0, 0}}; /*11*/
	FLASH_PAGE_INFO flushRAMTablePageInfo =
	{	0, 0,
		{	0, 0}}; /*11*/
	UINT16 logicalBlockNum = 0;
	UINT16 flushEB = 0;
	SYS_EBLOCK_FLUSH_INFO sysEBlockFlushInfo; /*16*/
	UINT16 saveFromIndex = 0;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	UINT16_PTR flushInfoEntry = NULL; /*4*/
	UINT8 tempArray[SECTOR_SIZE];
	UINT16 count2 = 0;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
	for(count = 0; count < sizeof(sysEBlockFlushInfo.reserved); count++)
	{
		sysEBlockFlushInfo.reserved[count] = EMPTY_BYTE;
	}

#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	sysEBlockFlushInfo.tableCheckWord = EMPTY_WORD;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

	sysEBlockFlushInfo.logIncNum = EMPTY_DWORD;

	if((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
							&flushRAMTablePageInfo, &flushEB)) != FTL_ERR_PASS)
	{
		return status; // go Flush GC.
	}

	// Inc free page
	IncGCOrFreePageNum(devID, flushEB);

	gCounterDirty--;

	if((gCounterDirty == 0) && (type == CACHE_EBLOCKMAP))
	{
		if(flushMode == FLUSH_SHUTDOWN_MODE)
		{
			if((status = GetLogEntryLocation(devID, &nextLoc)) != FTL_ERR_PASS)
			{
				return status;
			}
			sysEBlockFlushInfo.eBlockNumLoc = nextLoc.eBlockNum;
			sysEBlockFlushInfo.entryIndexLoc = nextLoc.entryIndex;
		}
		else
		{
			sysEBlockFlushInfo.eBlockNumLoc = EMPTY_WORD;
			sysEBlockFlushInfo.entryIndexLoc = EMPTY_WORD;
		}
		sysEBlockFlushInfo.endPoint = END_POINT_SIGNATURE;
		sysEBlockFlushInfo.logIncNum = GetTransLogEBCounter(devID);
	}
	else
	{
		sysEBlockFlushInfo.eBlockNumLoc = EMPTY_WORD;
		sysEBlockFlushInfo.entryIndexLoc = EMPTY_WORD;
		sysEBlockFlushInfo.endPoint = EMPTY_BYTE;
	}

	// EBM
	if(FTL_ERR_PASS != (status = CACHE_GetEB(devID, fromIndex, &logicalBlockNum)))
	{
		return status;
	}
	if(FTL_ERR_PASS != (status = CACHE_GetSector(logicalBlockNum, &sector)))
	{
		return status;
	}
	sysEBlockFlushInfo.tableOffset = (UINT16)sector;

	if(CACHE_EBLOCKMAP == type)
	{
		sysEBlockFlushInfo.type = EBLOCK_MAP_TABLE_FLUSH;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
		if(FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, CACHE_GetEBMCachePtr(devID, fromIndex)) != FLASH_PASS)
		{
			return FTL_ERR_CACHE_FLASH_WRITE_01;
		}

		// Write END SIGNATURE, @Time T2
		if((status = FTL_WriteFlushInfo(&flushStructPageInfo, &sysEBlockFlushInfo)) != FTL_ERR_PASS)
		{
			return status;
		}
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

		MEM_Memcpy((UINT8_PTR)&tempArray[0], (UINT8_PTR)&sysEBlockFlushInfo, FLUSH_INFO_SIZE);
		MEM_Memcpy((UINT8_PTR)&tempArray[FLUSH_INFO_SIZE], CACHE_GetEBMCachePtr(devID, fromIndex), FLUSH_RAM_TABLE_SIZE);
		for (count2 = FLUSH_INFO_SIZE + FLUSH_RAM_TABLE_SIZE; count2 < SECTOR_SIZE; count2++)
		{
			tempArray[count2] = EMPTY_BYTE;
		}

		flushInfoEntry = (UINT16_PTR)&tempArray[0];
		flushInfoEntry[FLUSH_INFO_TABLE_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_TABLE_START], (FLUSH_RAM_TABLE_SIZE / 2));
		flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);

		flashStatus = FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, &tempArray[0]);
		if(flashStatus != FLASH_PASS)
		{
#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
			if(flashStatus == FLASH_PARAM)
			{
				return FTL_ERR_CACHE_FLASH_WRITE_02;
			}
			SetBadEBlockStatus(devID, flushEB, FTL_TRUE);
			if (FLASH_MarkDefectEBlock(&flushRAMTablePageInfo) != FLASH_PASS)
			{
				// do nothing, just try to mark bad, even if it fails we move on.
			}
			return FTL_ERR_FLUSH_FLUSH_FAIL;
#else
			return FTL_ERR_CACHE_FLASH_WRITE_02;
#endif
		}
#endif
		// set
		if(FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
		{
			return status;
		}

	}
	else if(CACHE_PPAMAP == type)
	{
		saveFromIndex = fromIndex;
		fromIndex = (UINT16)(PPA_CACHE_TABLE_OFFSET * fromIndex);
		sysEBlockFlushInfo.tableOffset = (UINT16)(PPA_CACHE_TABLE_OFFSET * sysEBlockFlushInfo.tableOffset);
		sysEBlockFlushInfo.type = PPA_MAP_TABLE_FLUSH;
		for(count = 0; count < PPA_CACHE_TABLE_OFFSET; count++)
		{
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)         
			if(FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, (UINT8_PTR)CACHE_GetPPACachePtr(devID, fromIndex)) != FLASH_PASS)
			{
				return FTL_ERR_CACHE_FLASH_WRITE_03;
			}

			// Write END SIGNATURE, @Time T2
			if((status = FTL_WriteFlushInfo(&flushStructPageInfo, &sysEBlockFlushInfo)) != FTL_ERR_PASS)
			{
				return status;
			}
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

			MEM_Memcpy((UINT8_PTR)&tempArray[0], (UINT8_PTR)&sysEBlockFlushInfo, FLUSH_INFO_SIZE);
			MEM_Memcpy((UINT8_PTR)&tempArray[FLUSH_INFO_SIZE], (UINT8_PTR)CACHE_GetPPACachePtr(devID, fromIndex), FLUSH_RAM_TABLE_SIZE);
			for (count2 = FLUSH_INFO_SIZE + FLUSH_RAM_TABLE_SIZE; count2 < SECTOR_SIZE; count2++)
			{
				tempArray[count2] = EMPTY_BYTE;
			}

			flushInfoEntry = (UINT16_PTR)&tempArray[0];
			flushInfoEntry[FLUSH_INFO_TABLE_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_TABLE_START], (FLUSH_RAM_TABLE_SIZE / 2));
			flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);

			flashStatus = FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, &tempArray[0]);

			if(flashStatus != FLASH_PASS)
			{
#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
				if(flashStatus == FLASH_PARAM)
				{
					return FTL_ERR_CACHE_FLASH_WRITE_04;
				}
				SetBadEBlockStatus(devID, flushEB, FTL_TRUE);
				if (FLASH_MarkDefectEBlock(&flushRAMTablePageInfo) != FLASH_PASS)
				{
					// do nothing, just try to mark bad, even if it fails we move on.
				}
				return FTL_ERR_FLUSH_FLUSH_FAIL;
#else
				return FTL_ERR_CACHE_FLASH_WRITE_04;
#endif
			}
#endif

			// set position
			if(FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_PPAMAP)))
			{
				return status;
			}

			// check end
			if(PPA_CACHE_TABLE_OFFSET == (UINT16)(count + 1))
			{
				break;
			}

			// For next PPA table
			if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, saveFromIndex, &ebmCacheInfo)))
			{
				return status;
			}

			if((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
									&flushRAMTablePageInfo, &flushEB)) != FTL_ERR_PASS)
			{
				return status; // Do GC.
			}
			// update position
			eBlockPPAMapInfo.entryIndex = (UINT16)GetGCNum(devID, flushEB);

			TABLE_GetFlushLogCacheEntry(devID, GetPhysicalEBlockAddr(devID, flushEB), &eBlockPPAMapInfo.flashLogEBArrayCount);

			// Inc free page
			IncGCOrFreePageNum(devID, flushEB);

			sysEBlockFlushInfo.tableOffset++;
			fromIndex++;
		}
	}
#endif

	return status;
}

// Load Cache to Cache
FTL_STATUS CACHE_CacheToCache(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 count = 0;
	UINT16 ppaNum = 0;

	// copy EBM
	for(count = 0;count < FLUSH_RAM_TABLE_SIZE; count++)
	{
		EBlockMappingCache[toIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count] = EBlockMappingCache[fromIndex + (devID * NUM_EBLOCK_MAP_INDEX)][count];
	}
	// copy PPA
	for(ppaNum = 0; ppaNum < PPA_CACHE_TABLE_OFFSET; ppaNum++)
	{
		for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
		{
			PPAMappingCache[(toIndex * PPA_CACHE_TABLE_OFFSET) + ppaNum + (devID * NUM_EBLOCK_MAP_INDEX)][count] = PPAMappingCache[(fromIndex * PPA_CACHE_TABLE_OFFSET) + ppaNum + (devID * NUM_EBLOCK_MAP_INDEX)][count];
		}
	}
	return status;
}

#if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
FTL_STATUS CACHE_GetUsedRamSize(unsigned long* miniRamSize, unsigned long* maxRamSize)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 numEblockMapIndex = 0;
	UINT16 cacheIndexChangeArea = 0;
	UINT16 numPpaMapIndex = 0;
	UINT32 ebmCacheIndexSize = 0;
	UINT32 eblockMappingCacheSize = 0;
	UINT32 ppaMappingCacheSize = 0;

	// Get minimum
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
	if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
	{
		numEblockMapIndex = ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + SAVE_DATA_EB_INDEX);
	}
	else
	{
		numEblockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1) + SAVE_DATA_EB_INDEX);
	}
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	if ((NUMBER_OF_ERASE_BLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
	{
		if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
		{
			numEblockMapIndex = ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + SAVE_DATA_EB_INDEX);
		}
		else
		{
			numEblockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1) + SAVE_DATA_EB_INDEX);
		}
	}
	else
	{

		if (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) % FLUSH_RAM_TABLE_SIZE) == 0)
		{
			numEblockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE)) + 1 + SAVE_DATA_EB_INDEX);
		}
		else
		{
			numEblockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 2) + SAVE_DATA_EB_INDEX);
		}
	}
	cacheIndexChangeArea = 1;
#endif // #if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
	numPpaMapIndex = (numEblockMapIndex * PPA_CACHE_TABLE_OFFSET);

	// Calc ram size
	ebmCacheIndexSize = (2/*UINT16 size*/* numEblockMapIndex * NUMBER_OF_DEVICES);
	eblockMappingCacheSize = (1/*UINT8 size*/* numEblockMapIndex * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE);
	ppaMappingCacheSize = (1/*UINT8 size*/* numPpaMapIndex * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE);

	*miniRamSize = (TEMP_BD_RAM_SIZE + ebmCacheIndexSize + eblockMappingCacheSize + ppaMappingCacheSize);

	// Get maximum
	numEblockMapIndex = (MAX_EBLOCK_MAP_INDEX);
	numPpaMapIndex = (MAX_PPA_MAP_INDEX);

	// Calc ram size
	ebmCacheIndexSize = (2/*UINT16 size*/* numEblockMapIndex * NUMBER_OF_DEVICES);
	eblockMappingCacheSize = (1/*UINT8 size*/* numEblockMapIndex * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE);
	ppaMappingCacheSize = (1/*UINT8 size*/* numPpaMapIndex * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE);

	*maxRamSize = (TEMP_BD_RAM_SIZE + ebmCacheIndexSize + eblockMappingCacheSize + ppaMappingCacheSize);

	return status;
}
#endif

#ifdef DEBUG_PROTOTYPE

// Debug Only Blank Function
// The following A/I will be invoked, when the requested data does not exist on Flash.
// In Cache Module Unit Test, all data exist on Flash as pre-condition, then always on Flash.
void TABLE_ClearPPATable(FTL_DEV devID, UINT16 eBlockNum)
{
#if 0
	UINT32 EBMramStructPtr = 0;
	UINT32 PPAramStructPtr = 0;
	UINT32 testData2_ptr[(NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) / 4];

	CACHE_GetRAMOffsetEB(devID, eBlockNum, &EBMramStructPtr, &PPAramStructPtr);

	// PPA
	for (i = 0; i < ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) / 4); i++)
	{
		testData2_ptr[i] = 0;
	}
	testData_ptr[0] = 0x01234567;
	testData_ptr[1] = 0x89ABCDEF;

	memcpy((UINT32_PTR)PPAramStructPtr, testData2_ptr, sizeof(UINT8)* (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE));
#endif
	DBG_Printf("TABLE_ClearPPATable: Should not enter here. \n", 0, 0);
	return;
}

void TABLE_ClearMappingTable(FTL_DEV devID, UINT16 logicalEBNum, UINT16 phyEBAddr, UINT16 eraseCount)
{
#if 0
	UINT32 EBMramStructPtr = 0;
	UINT32 PPAramStructPtr = 0;
	UINT32 testData_ptr[EBLOCK_MAPPING_ENTRY_SIZE / 4];

	CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);

	// EBM
	for (i = 0; i < (EBLOCK_MAPPING_ENTRY_SIZE / 4); i++)
	{
		testData_ptr[i] = 0;
	}
	testData_ptr[0] = 0x01234567;
	testData_ptr[1] = 0x89ABCDEF;

	memcpy((UINT32_PTR)EBMramStructPtr, testData_ptr, sizeof(UINT8)* EBLOCK_MAPPING_ENTRY_SIZE);
#endif
	DBG_Printf("TABLE_ClearMappingTable: Should not enter here.\n", 0, 0);
	return;
}

#endif 

#endif /* (CACHE_RAM_BD_MODULE == FTL_TRUE) */
