// file: ftl_common.h
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

#ifndef FTL_COMMON_H
#define FTL_COMMON_H

#include "ftl_debug.h"
#include "ftl_calc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
	UINT8 GetBadEBlockStatus(FTL_DEV devID, UINT16 logicalEBNum);
	void SetBadEBlockStatus(FTL_DEV devID, UINT16 logicalEBNum, UINT8 badBlockStatus);
#endif
	FTL_STATUS TRANS_InitTransMap (void);
	void TABLE_ClearFreeBitMap (FTL_DEV devID, UINT16 eBlockNum);
	void TABLE_ClearMappingTable (FTL_DEV devID, UINT16 logicalEBNum, UINT16 phyEBAddr, UINT32 eraseCount);
	FTL_STATUS TABLE_InitMappingTable (void);
	void TABLE_ClearPPATable (FTL_DEV devID, UINT16 eBlockNum);
	FTL_STATUS TABLE_InitPPAMappingTable (void);
	FTL_STATUS FTL_WriteSysEBlockInfo (FLASH_PAGE_INFO_PTR flashPagePtr, SYS_EBLOCK_INFO_PTR sysInfoPtr);
	UINT8 GetTMDevID (UINT16 index);
	void SetTMDevID (UINT16 index, FTL_DEV devId);
	UINT8 GetTMNumSectors (UINT16 index);
	void SetTMNumSectors (UINT16 index, UINT8 sec);
	UINT32 GetTMStartLBA (UINT16 index);
	void SetTMStartLBA (UINT16 index, UINT32 lba);
	TRANS_MAP_ENTRY_PTR GetTMPointer (UINT16 index);
	UINT32 GetTMPhyPage (UINT16 index);
	void SetTMPhyPage (UINT16 index, UINT32 phyPage);
	UINT32 GetTMMergePage (UINT16 index);
	void SetTMMergePage (UINT16 index, UINT32 mergePage);
	UINT8 GetTMStartSector (UINT16 index);
	void SetTMStartSector (UINT16 index, UINT8 sector);
	void SetTMLogInfo (UINT16 index, UINT16 value);
	UINT16 GetTMLogInfo (UINT16 index);
	void DecGCOrFreePageNum (FTL_DEV devID, UINT16 logicalEBNum);
	UINT16 GetTotalFreePages (FTL_DEV devID, UINT16 logicalEBNum);
	FTL_STATUS INIT_InitBasic (void);
	UINT16 GetPhysicalEBlockAddr (FTL_DEV devID, UINT16 logicalBlockNum);
	UINT16 GetLogicalEBlockAddr (FTL_DEV devID, UINT16 physicalBlockNum);
	void SetPhysicalEBlockAddr (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 phyBlockNum);
	UINT32 GetEraseCount (FTL_DEV devID, UINT16 logicalBlockNum);
	void SetEraseCount (FTL_DEV devID, UINT16 logicalBlockNum, UINT32 eraseCount);
	void IncEraseCount (FTL_DEV devID, UINT16 logicalBlockNum);
	UINT16 GetDirtyCount (FTL_DEV devID, UINT16 logicalBlockNum);
	void SetDirtyCount (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 dirtyCount);
	UINT16 GetFreePageIndex (FTL_DEV devID, UINT16 logicalBlockNum);
	void SetFreePageIndex (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 freePageIdx);
	UINT32 GetGCNum (FTL_DEV devID, UINT16 logicalBlockNum);

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
	UINT16 GetChainLogicalEBNum (FTL_DEV devID, UINT16 logicalBlockNum);
	UINT16 GetChainPhyEBNum (FTL_DEV devID, UINT16 logicalBlockNum);
	void SetChainLogicalEBNum (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 logEBNum);
	void SetChainPhyEBNum (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 PhyEBNum);
	void SetChainLink (FTL_DEV devID, UINT16 logEBNumFrom, UINT16 logEBNumTo,
	UINT16 phyEBNumFrom, UINT16 phyEBNumTo);
	void ClearChainLink (FTL_DEV devID, UINT16 logicalBlockNumFrom, UINT16 logicalBlockNumTo);
	UINT16 GetLongestChain (FTL_DEV devID);
	UINT16 GetChainWithLowestVaildUsedRatio (FTL_DEV devID);
	UINT16 GetChainWithLowestVaildPages (FTL_DEV devID);
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

	FREE_BIT_MAP_TYPE GetEBlockMapFreeBitIndex (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 phyPageOffset);
	void SetEBlockMapFreeBitIndex (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 phyPageOffset, FREE_BIT_MAP_TYPE value);
	void SetGCOrFreePageNum (FTL_DEV devID, UINT16 logicalBlockNum, UINT32 GCNum);
	void ClearEBlockMapFreeBitIndex (FTL_DEV devID, UINT16 logicalEBNum);
	void MarkEBlockMappingTableSectorDirty (FTL_DEV devID, UINT16 sector);
	void MarkEBlockMappingTableEntryDirty (FTL_DEV devID, UINT16 logicalEBNum);
	void MarkEBlockMappingTableSectorClean (FTL_DEV devID, UINT16 sector);
	UINT8 IsEBlockMappingTableSectorDirty (FTL_DEV devID, UINT16 sector);
	UINT16 GetPPASlot (FTL_DEV devID, UINT16 logicalEBNum, UINT16 logicalPageOffset);
	void SetPPASlot (FTL_DEV devID, UINT16 logicalEBNum, UINT16 logicalPageOffset, UINT16 value);
	void MarkPPAMappingTableSectorDirty (FTL_DEV devID, UINT16 sector);
	void MarkPPAMappingTableEntryDirty (FTL_DEV devID, UINT16 logicalEBNum, UINT16 logicalPageOffset);
	void MarkPPAMappingTableSectorClean (FTL_DEV devID, UINT16 sector);
	UINT8 IsPPAMappingTableSectorDirty (FTL_DEV devID, UINT16 sector);
	FTL_STATUS TABLE_InitEBOrderingTable (FTL_DEV devID);
	FTL_STATUS TABLE_ClearReservedEB (FTL_DEV devID);
	FTL_STATUS TABLE_InsertReservedEB (FTL_DEV devID, UINT16 logicalAddr);
	FTL_STATUS TABLE_GetReservedEB (FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT8 WLflag);
UINT16 TABLE_GetReservedEBlockNum (FTL_DEV devID); /* changed by Nobu Feb 18, 2015 : UINT8 -> UINT16 for ML16G2 */
UINT16 TABLE_GetUsedSysEBCount (FTL_DEV devID);
FTL_STATUS TABLE_CheckUsedSysEB (FTL_DEV devID, UINT16 logicalAddr);
FTL_STATUS TABLE_FlushEBInsert (FTL_DEV devID, UINT16 logicalAddr, UINT16 phyEBAddr, UINT32 key);
FTL_STATUS TABLE_FlushEBRemove (FTL_DEV devID, UINT16 blockNum);
FTL_STATUS TABLE_FlushEBGetNext (FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT16_PTR phyEBAddrPtr, UINT32_PTR keyPtr);
FTL_STATUS TABLE_FlushEBGetLatest (FTL_DEV devID, UINT16_PTR flushEBlockPtr,
UINT16_PTR phyEBAddrPtr, UINT32 keyPtr);
FTL_STATUS TABLE_FlushEBClear (FTL_DEV devID);
#if (CACHE_RAM_BD_MODULE == FTL_FALSE)
FTL_STATUS AdjustFlushEBlockFreePage (FTL_DEV devID, UINT16 logicalFlushEBlockNum, UINT16 dirtyBitMapCount);
#endif
FTL_STATUS RestoreFlushEBlockFreePage (FTL_DEV devID, UINT16 logicalFlushEBlockNum);
FTL_STATUS TABLE_TransLogEBInsert (FTL_DEV devID, UINT16 logicalAddr, UINT16 phyEBAddr, UINT32 key);
FTL_STATUS TABLE_TransLogEBGetNext (FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT16_PTR phyEBAddrPtr, UINT32_PTR keyPtr);
FTL_STATUS TABLE_TransEBClear (FTL_DEV devID);
FTL_STATUS TABLE_TransLogEBRemove (FTL_DEV devID, UINT16 blockNum);
void TABLE_SortTransTable (FTL_DEV devID);
FTL_STATUS TABLE_TransLogEBGetLatest (FTL_DEV devID, UINT16_PTR LogEBlockPtr,
UINT16_PTR phyEBAddrPtr, UINT32 keyPtr);
UINT16 GetTransLogEBArrayCount (FTL_DEV devID);
UINT32 GetTransLogEBCounter (FTL_DEV devID);
void SetTransLogEBCounter (FTL_DEV devID, UINT32 counter);
UINT32 GetFlushEBCounter (FTL_DEV devID);
void SetFlushLogEBCounter (FTL_DEV devID, UINT32 counter);
UINT16 GetFlushLogEBArrayCount (FTL_DEV devID);
FTL_STATUS TABLE_GetTransLogEntry (FTL_DEV devID, UINT16 blockNum, UINT16_PTR logicalEBNumPtr, UINT16_PTR phyAddrPtr, UINT32_PTR keyPtr);
FTL_STATUS TABLE_SetTransLogEntry (FTL_DEV devID, UINT16 blockNum, UINT16 logicalEBNum, UINT16 phyAddr, UINT32 key);
FTL_STATUS TABLE_GetFlushLogEntry (FTL_DEV devID, UINT16 blockNum, UINT16_PTR logicalEBNumPtr, UINT16_PTR phyAddrPtr, UINT32_PTR keyPtr);
FTL_STATUS TABLE_SetFlushLogEntry (FTL_DEV devID, UINT16 blockNum, UINT16 logicalEBNum, UINT16 phyAddr, UINT32 key);
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
#if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
FTL_STATUS CACHE_DynamicAllocation(UINT32 total_ram_allowed);
#endif
FTL_STATUS TABLE_GetFlushLogCacheEntry(FTL_DEV devID, UINT16 phyAddr, UINT8_PTR cacheNum_ptr);
FTL_STATUS TABLE_GetFlushLogPhyEntry(FTL_DEV devID, UINT8 cacheNum, UINT16_PTR phyAddr_ptr);
FTL_STATUS TABLE_GetFlushLogCountEntry(FTL_DEV devID, UINT8 cacheNum, UINT16_PTR blockNum_ptr);
FTL_STATUS ClearSaveStaticWL(FTL_DEV devID, UINT16 logicalEBNum, UINT32 eraseCount, UINT8 hlFlag);
FTL_STATUS SetSaveStaticWL(FTL_DEV devID, UINT16 logicalEBNum, UINT32 eraseCount);
FTL_STATUS GetSaveStaticWL(FTL_DEV devID, UINT16_PTR logicalEBNum, UINT32_PTR eraseCount, UINT8 hlFlag);
#endif
FTL_STATUS FTL_SwapDataReserveEBlock (FTL_DEV devID, UINT16 logicalPageNum, UINT16_PTR ptrPhyReservedBlock, UINT16_PTR ptrLogicalReservedBlock, UINT8 WLflag, UINT8 badBlockFlagIn);
FTL_STATUS TABLE_Flush (UINT8 flushMode);
FTL_STATUS TABLE_LoadFlushTable (void);
FTL_STATUS TRANS_ClearTransMap (void);
FTL_STATUS FTL_CheckForFreePages (ADDRESS_STRUCT_PTR startPage, ADDRESS_STRUCT_PTR endPage, UINT32 totalPages);
FTL_STATUS GetPhyPageAddr (ADDRESS_STRUCT_PTR currentPage, UINT16 phyEBNum,
UINT16 logEBNum, UINT32_PTR phyPage);
FTL_STATUS ClearGC_Info (void);
FTL_STATUS ClearMergeGC_Info (FTL_DEV DevID, UINT16 logicalEBNum, UINT32 logicalPageNum);
FTL_STATUS UpdateTransferMap (UINT32 currentLBA, ADDRESS_STRUCT_PTR currentPage,
	ADDRESS_STRUCT_PTR endPage, ADDRESS_STRUCT_PTR startPage, UINT32 totalPages,
	UINT32 phyPage, UINT32 mergePage, UINT8 isWrite, UINT8 isChained);
FTL_STATUS ClearGCPageBitMap (void);
FTL_STATUS SetPageMoved (UINT16 pageAddress, UINT32 phyPageAddr);
FTL_STATUS IsPageMoved (UINT32 pageAddr, UINT8_PTR isMoved);

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
FTL_STATUS TABLE_InitTransLogEntry (void);
FTL_STATUS InsertEntryIntoLogEntry (UINT16 index, UINT32 phyPageAddr,
UINT32 currentLBA, ADDRESS_STRUCT_PTR currentPage, UINT8 isChained);
FTL_STATUS ProcessPageLoc (FTL_DEV devID, LOG_PHY_PAGE_LOCATION_PTR pageLocPtr, UINT32 pageAddress);
FTL_STATUS UpdateRAMTablesUsingTransLogs (FTL_DEV devID);
FTL_STATUS FTL_ClearA (void);
FTL_STATUS FTL_ClearB (UINT16 count);
FTL_STATUS FTL_ClearC (UINT16 seqNum);
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

void SetOldPageIndex (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 oldPageIdx);
FTL_STATUS FTL_WriteLogInfo (FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr);
FTL_STATUS FTL_WriteFlushInfo (FLASH_PAGE_INFO_PTR flashPagePtr, SYS_EBLOCK_FLUSH_INFO_PTR flushInfoPtr);
void IncGCOrFreePageNum (FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS GetNextFlushEntryLocation (FTL_DEV devID, FLASH_PAGE_INFO_PTR flushInfoPtr,
	FLASH_PAGE_INFO_PTR flushRamTablePtr, UINT16_PTR logicalBlockNumPtr);
FTL_STATUS CreateNextFlushEntryLocation (FTL_DEV devID, UINT16 logicalBlockNum);
FTL_STATUS GetFlushLoc (FTL_DEV devID, UINT16 phyEBlockAddr, UINT16 freePageIndex, FLASH_PAGE_INFO_PTR flushInfoPtr,
	FLASH_PAGE_INFO_PTR flushRamTablePtr);
FTL_STATUS FTL_GetNextTransferMapEntry (UINT16_PTR nextEntryIndex, UINT16_PTR startIndex, UINT16_PTR endIndex);
UINT16 FTL_GetCurrentIndex (void);
void SetUsedPageIndex (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 usedPageIdx);
FTL_STATUS TABLE_InitGcNum (void);
void TABLE_SortFlushTable (FTL_DEV devID);
FTL_STATUS GetNextLogEntryLocation (FTL_DEV devID, FLASH_PAGE_INFO_PTR pageInfoPtr);
FTL_STATUS CreateNextLogEntryLocation (FTL_DEV devID, UINT16 logicalBlockNum);
FTL_STATUS CreateNextTransLogEBlock (FTL_DEV devID, UINT16 logicalBlockNum);
FTL_STATUS UpdateEBOrderingTable (FTL_DEV devID, UINT16 startEB, UINT16_PTR formatCount);
FTL_STATUS CheckFlushSpace (FTL_DEV devID);
FTL_STATUS FTL_EraseOp (FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS FTL_EraseOpNoDirty (FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS FTL_EraseAllTransLogBlocksOp (FTL_DEV devID);
UINT32 CalcFlushRamTablePages (UINT16 phyEBNum, UINT16 index);
UINT16 CalcFlushRamTableOffset (UINT16 index);
FTL_STATUS LoadRamTable (FLASH_PAGE_INFO_PTR flashPage, UINT8_PTR ramTablePtr, UINT16 tableOffset, UINT32 devTableSize);
FTL_STATUS GetTransLogsSetRAMTables (FTL_DEV devID, LOG_ENTRY_LOC_PTR startLoc, UINT8_PTR ramTablesUpdated, UINT8_PTR logEblockFount);
FTL_STATUS FTL_FindEmptyTransLogEBlock (FTL_DEV devID, UINT16_PTR logicalEBNumPtr, UINT16_PTR physicalEBNumPtr);
FTL_STATUS UpdateRAMTablesUsingGCLogs (FTL_DEV devID, GC_LOG_ENTRY_PTR ptrGCLog);

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
FTL_STATUS UpdateRAMTablesUsingChainLogs (FTL_DEV devID, CHAIN_LOG_ENTRY_PTR chainLogPtr);
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

#if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
FTL_STATUS UpdateRAMTablesUsingEBSwapLogs (FTL_DEV devID, EBSWAP_LOG_ENTRY_PTR EBSwapLogPtr);
FTL_STATUS CreateSwapEBLog (FTL_DEV devID, UINT16 logicalDataEB, UINT16 logicalReservedEB);
#endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

#if(FTL_UNLINK_GC == FTL_TRUE)
FTL_STATUS UnlinkChain (FTL_DEV devID, UINT16 logicalBlockNumFrom, UINT16 logicalBlockNumTo);
FTL_STATUS UpdateRAMTablesUsingUnlinkLogs (FTL_DEV devID, UNLINK_LOG_ENTRY_PTR ptrUnlinkLog);
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

UINT16 FTL_GetNumLogEntries (ADDRESS_STRUCT_PTR startPage, UINT32 totalPages);

FTL_STATUS TABLE_FlushDevice (FTL_DEV devID, UINT8 flushMode);
FTL_STATUS TRANS_ClearEntry (UINT16 index);
UINT32 GetTotalDirtyBitCnt (FTL_DEV devID);
UINT16 GetNumFreePages (FTL_DEV devID, UINT16 logicalEBNum);
UINT16 GetNumValidPages (FTL_DEV devID, UINT16 logicalEBNum);
UINT16 GetNumUsedPages (FTL_DEV devID, UINT16 logicalEBNum);
UINT16 GetNumInvalidPages (FTL_DEV devID, UINT16 logicalEBNum);
void GetNumValidUsedPages (FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR used, UINT16_PTR valid);
FTL_STATUS Flush_GC (FTL_DEV devID);
void UpdatePageTableInfo (FTL_DEV devID, UINT16 logicalBlockNum, UINT16 logicalPageIndex,
UINT16 phyPageIdx, UINT8 bitStatus);
FTL_STATUS DBG_CheckPPAandBitMap (FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS DBG_CheckMappingTables (void);

extern GC_SAVE gcSave[MAX_BLOCKS_TO_SAVE];
extern UINT16 gcSaveCount;
void FTL_ClearGCSave (UINT8 clearMode);
FTL_STATUS FTL_AddToGCSave (FTL_DEV devId, UINT16 phyEbNum);
FTL_STATUS FTL_CheckForGCLogSpace (FTL_DEV devID);
/***ERASE COUNT**********/
#if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
UINT8 GetEBErased (FTL_DEV devID, UINT16 logEBNum);
void SetEBErased (FTL_DEV devID, UINT16 logEBNum, UINT8 eraseStatus);
#endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
FTL_STATUS FTL_WriteSuperInfo (FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr);
FTL_STATUS FTL_FindSuperSysEB (FTL_DEV devID);
FTL_STATUS GetSuperSysInfoLogs (FTL_DEV devID, UINT16_PTR storePhySysEB, UINT8_PTR checkSuperPF);
FTL_STATUS SetSysEBRamTable (FTL_DEV devID, UINT16_PTR storeSysEB, UINT16_PTR formatCount);
FTL_STATUS FTL_CheckForSuperSysEBLogSpace (FTL_DEV devID, UINT8 mode);
FTL_STATUS FTL_CreateSuperSysEBLog (FTL_DEV devID, UINT8 mode);

FTL_STATUS GetNextSuperSysEBEntryLocation (FTL_DEV devID, FLASH_PAGE_INFO_PTR pageInfoPtr, UINT16_PTR entryIndexPtr);
FTL_STATUS CreateNextSuperSystemEBlockOp (FTL_DEV devID);

FTL_STATUS FTL_FindEmptySuperSysEBlock (FTL_DEV devID, UINT16_PTR logicalEBNumPtr, UINT16_PTR physicalEBNumPtr);
FTL_STATUS FTL_FindAllAreaSuperSysEBlock (FTL_DEV devID, UINT16_PTR findDataEBNumPtr, UINT16_PTR findSystemEBNumPtr);
FTL_STATUS DataGCForSuperSysEB (void);
FTL_STATUS ClearSuperEBInfo (void);
void SetSuperSysEBCounter (FTL_DEV devID, UINT32 counter);
UINT32 GetSuperSysEBCounter (FTL_DEV devID);
FTL_STATUS TABLE_GetSuperSysEBEntry (FTL_DEV devID, UINT16 blockNum, UINT16_PTR logEBlockPtr, UINT16_PTR phyAddrPtr, UINT32_PTR keyPtr);
FTL_STATUS TABLE_SuperSysEBClear (FTL_DEV devID);
FTL_STATUS TABLE_SuperSysEBInsert (FTL_DEV devID, UINT16 logicalAddr, UINT16 phyEBAddr, UINT32 key);
FTL_STATUS TABLE_SuperSysEBGetLatest (FTL_DEV devID, UINT16_PTR logEBlockPtr, UINT16_PTR phyEBAddrPtr, UINT32 key);
FTL_STATUS TABLE_SuperSysEBGetNext (FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT16_PTR phyEBAddrPtr, UINT32_PTR keyPtr);
void TABLE_SortSuperTable (FTL_DEV devID);
FTL_STATUS TABLE_SuperSysEBRemove (FTL_DEV devID, UINT16 blockNum);
FTL_STATUS TABLE_GetPhySysEB (FTL_DEV devID, UINT16_PTR countPtr, UINT16_PTR phyEBAddrPtr);
FTL_STATUS TABLE_CheckUsedSuperEB (FTL_DEV devID, UINT16 logicalAddr);
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

void SwapUnusedEBlock (FTL_DEV devID, UINT16 logicalDataEB, UINT16 logicalReservedEB);
void MarkAllPagesStatus (FTL_DEV devID, UINT16 logEBNum, UINT8 bitStatus);
FTL_STATUS FindAndSwapUnusedEB (FTL_DEV devID, UINT16 logFrom, UINT16_PTR logTo);
UINT32 GetTrueEraseCount (FTL_DEV devID, UINT16 logicalBlockNum);

#ifndef FTL_RAM_TABLES_C
#if (CACHE_RAM_BD_MODULE == FTL_FALSE)
extern EBLOCK_MAPPING_ENTRY EBlockMappingTable[NUM_DEVICES][NUM_EBLOCKS_PER_DEVICE];
extern PPA_MAPPING_ENTRY PPAMappingTable[NUM_DEVICES][NUM_EBLOCKS_PER_DEVICE][NUM_PAGES_PER_EBLOCK];
extern UINT8 PPAMappingTableDirtyBitMap[NUM_DEVICES][PPA_DIRTY_BITMAP_DEV_TABLE_SIZE];
extern UINT8 EBlockMappingTableDirtyBitMap[NUM_DEVICES][EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE];
#else
#if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
extern UINT32 gSave_Total_ram_allowed;
extern UINT8 gCheckFirst;
extern UINT8_PTR_PTR EBlockMappingCache;
extern UINT8_PTR_PTR PPAMappingCache;
extern UINT16_PTR EBMCacheIndex;

extern UINT16 numBlockMapIndex;
extern UINT16 numPpaMapIndex;
extern UINT16 cacheIndexChangeArea;
extern UINT16 thesholdDirtyCount;
extern UINT16 ebmCacheIndexSize;
extern UINT16 eblockMappingCacheSize;
extern UINT16 ppaMappingCacheSize;
#endif
#endif
extern KEY_TABLE_ENTRY FlushLogEBArray[NUM_DEVICES][NUM_FLUSH_LOG_EBLOCKS];
extern KEY_TABLE_ENTRY TransLogEBArray[NUM_DEVICES][NUM_TRANSACTION_LOG_EBLOCKS];
extern UINT16 FlushLogEBArrayCount[NUM_DEVICES];
extern UINT16 TransLogEBArrayCount[NUM_DEVICES];
extern UINT32 TransLogEBCounter[NUM_DEVICES];
extern UINT32 FlushLogEBCounter[NUM_DEVICES];
extern UINT32 GCNum[NUM_DEVICES];
extern GC_INFO GC_Info;
extern UINT16 GC_THRESHOLD;
extern TRANSFER_MAP_STRUCT transferMap[NUM_TRANSFER_MAP_ENTRIES];
extern UINT16 TransferMapIndexEnd;
extern UINT16 TransferMapIndexStart;
extern FTL_DEV previousDevice;
extern UINT8 GCMoveArray[NUM_PAGES_PER_EBLOCK];
extern UINT8 GCMoveArrayNotEmpty;

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
extern SUPER_EB_INFO SuperEBInfo[NUM_DEVICES];
extern UINT8 gProtectForSuperSysEBFlag;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
extern UINT8 packedSuperInfo[SECTOR_SIZE];
#endif
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
extern TRANS_LOG_ENTRY TransLogEntry;
extern UINT16 TranslogBEntries;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

extern UINT8 pseudoRPB[NUM_DEVICES][NUMBER_OF_BYTES_PER_PAGE];
extern UINT32 LastTransLogLba;
extern UINT8 LastTransLogNpages;
extern UINT8 Delete_GC_Threshold;
extern UINT8 FTL_initFlag;
extern UINT8 FTL_UpdatedFlag;
extern UINT8 FTL_DeleteFlag;

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
extern BAD_BLOCK_INFO badBlockInfo;
extern UINT32 badBlockPhyPageAddr[NUM_PAGES_PER_EBLOCK];
#endif
#if (SPANSION_CRC32 == FTL_TRUE)
extern UINT32 crc32_table[256];
#endif

#endif  // #ifndef FTL_RAM_TABLES_C

void ClearTransferEB (void);
void SetTransferEB (ADDRESS_STRUCT_PTR startPage, ADDRESS_STRUCT_PTR endPage);
void GetTransferEB (UINT8 eblockCount, TRANSFER_EB_PTR transferEBPtr);
FTL_STATUS ClearDeleteInfo (void);
FTL_STATUS InitFTLRAMSTables (void);
FTL_STATUS InitDeleteInfo (UINT8 devID, UINT32 logicalPageAddr, UINT8 startSector, UINT8 numSectors);
FTL_STATUS HitDeleteInfo (UINT8 devID, UINT32 logicalPageAddr);
FTL_STATUS UpdateDeleteInfo (UINT8 startSector, UINT8 numSectors);
UINT8 GetDeleteInfoNumSectors (void);

#if(FTL_RPB_CACHE == FTL_TRUE)
UINT8 GetRPBCacheStatus(UINT8 devID);
void SetRPBCacheStatus(UINT8 devID, UINT8 status);
UINT32 GetRPBCacheLogicalPageAddr(UINT8 devID);
void SetRPBCacheLogicalPageAddr(UINT8 devID, UINT32 logicalPageAddr);
void ClearRPBCache(UINT8 devID);
UINT8_PTR GetRPBCache(UINT8 devID);
FTL_STATUS InitRPBCache(void);
FTL_STATUS UpdateRPBCache(UINT8 devID, UINT32 logicalPageAddr, UINT32 startSector, UINT32 numSectors, UINT8_PTR byteBuffer);
FTL_STATUS ReadRPBCache(UINT8 devID, UINT32 logicalPageAddr, UINT32 startSector, UINT32 numSectors, UINT8_PTR byteBuffer);
FTL_STATUS FlushRPBCache(UINT8 devID);
FTL_STATUS FillRPBCache(UINT8 devID, UINT32 logicalPageAddr);
FTL_STATUS ReadFlash(UINT8 devID, UINT32 logicalPageAddr, UINT32 startSector, UINT32 NB, UINT8_PTR byteBuffer);
FTL_STATUS RPBCacheForWrite(UINT8_PTR *byteBuffer, UINT32_PTR LBA, UINT32_PTR NB, UINT32_PTR bytesDone);
FTL_STATUS RPBCacheForRead(UINT8_PTR *byteBuffer, UINT32_PTR LBA, UINT32_PTR NB, UINT32_PTR bytesDone);
extern RPB_CACHE_READ_GROUP RPBCacheReadGroup;
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

#if(FTL_CHECK_ERRORS == FTL_TRUE)
extern UINT8 mountStatus;
extern UINT8 lockStatus;
#endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
FTL_STATUS InitPackedLogs(void);
FTL_STATUS CopyPackedLogs(UINT16 offset, UINT8_PTR logPtr);
FTL_STATUS WritePackedLogs(FLASH_PAGE_INFO_PTR flashPagePtr);
FTL_STATUS ReadPackedGCLogs(UINT8_PTR logPtr, GC_LOG_ENTRY_PTR ptrGCLog);

#if(FTL_UNLINK_GC == FTL_TRUE)
FTL_STATUS ReadPackedUnlinkLogs(UINT8_PTR logPtr, UNLINK_LOG_ENTRY_PTR ptrUnlinkLog);
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

FTL_STATUS VerifyRamTable(UINT16_PTR tablePtr);
FTL_STATUS FTL_WriteSpareInfo(FLASH_PAGE_INFO_PTR flashPagePtr, SPARE_INFO_PTR spareInfoPtr);
FTL_STATUS GetSpareInfoSetPPATable(void);

extern UINT8 packedLog[SECTOR_SIZE];
extern UINT8 writeLogFlag;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

FTL_STATUS FTL_InternalFormat (void);
FTL_STATUS FTL_InternalForcedGC (FTL_DEV DeviceID, unsigned short logicalEBNum, unsigned short * FreedUpPages,
	unsigned short * freePageIndex, UINT8 flag);

FTL_STATUS FTL_CheckMount_SetMTLockBit (void);
FTL_STATUS FTL_CheckUnmount_SetMTLockBit (void);
FTL_STATUS FTL_SetMTLockBit (void);
FTL_STATUS FTL_ClearMTLockBit (void);
void FTL_SetMountBit (void);
void FTL_ClearMountBit (void);
FTL_STATUS FTL_CheckDevID (UINT8 DevID);
FTL_STATUS FTL_CheckRange (UINT32 LBA, UINT32 NB);
FTL_STATUS FTL_CheckPointer (void *Ptr);
FTL_STATUS ResetIndexValue (FTL_DEV devID, LOG_ENTRY_LOC_PTR startLoc);

// WEAR LEVELLING
UINT32 FindEraseCountRange (FTL_DEV devID, UINT16_PTR coldestEB);
FTL_STATUS FTL_StaticWearLevelData (void);
UINT32 GetStaticWLThreshold (void);
void IncStaticWLCount (void);
void CheckStaticWLCount (void);
extern STATIC_WL_INFO StaticWLInfo;
FTL_STATUS InitStaticWLInfo (void);

#if(FTL_CHECK_VERSION == FTL_TRUE)
FTL_STATUS FTL_CheckVersion (void);
#endif // #if(FTL_CHECK_VERSION == FTL_TRUE)

void Init_PseudoRPB (void);

#if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
FTL_STATUS BadBlockEraseFailure(FTL_DEV devID, UINT16 eBlockNum);
void StoreSourceBadBlockInfo(FTL_DEV devID, UINT16 logicalEB, UINT16 currentOperation);
void StoreTargetBadBlockInfo(FTL_DEV devID, UINT16 logicalEB, UINT16 currentOperation);
FTL_STATUS BadBlockCopyPages(FTL_DEV devID, UINT16 logicalEBNum );
void ClearSourceBadBlockInfo(void);
void ClearTargetBadBlockInfo(void);
void RestoreSourceBadBlockInfo(void);
void RestoreTargetBadBlockInfo(void);
void ClearBadBlockInfo(void);
void ClearTransLogEBBadBlockInfo(void);
void SetTransLogEBFailedBadBlockInfo(void);
UINT8 GetTransLogEBFailedBadBlockInfo(void);
void SetTransLogEBNumBadBlockInfo(UINT16 logicalEBNum);
UINT16 GetTransLogEBNumBadBlockInfo(void);
FTL_STATUS TransLogEBFailure(FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr);

FTL_STATUS isBadBlockError(FTL_STATUS status);
FTL_STATUS TranslateBadBlockError(FTL_STATUS status);
FTL_STATUS BB_FindBBInChainedEBs(UINT16_PTR BBlogicalEB, UINT8_PTR chainFlag, UINT16_PTR badPhyEB);
FTL_STATUS BB_ManageBadBlockErrorForTarget(void);
FTL_STATUS BB_ManageBadBlockErrorForGCLog(void);
FTL_STATUS BB_ManageBadBlockErrorForSource(void);
FTL_STATUS BB_ManageBadBlockErrorForChainErase(void);
FTL_STATUS InternalForcedGCWithBBManagement(FTL_DEV devID, UINT16 logicalEBNum,
		UINT16_PTR FreedUpPages, UINT16_PTR freePageIndex, UINT8 WLflag );
UINT32 GetBBPageMoved(UINT16 pageOffset);
#endif

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
#if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
extern UINT8 EBlockMappingCache[NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES][FLUSH_RAM_TABLE_SIZE];
extern UINT8 PPAMappingCache[NUM_PPA_MAP_INDEX * NUMBER_OF_DEVICES][FLUSH_RAM_TABLE_SIZE];

extern UINT16 EBMCacheIndex[NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES];
#else
extern UINT8_PTR_PTR EBlockMappingCache;
extern UINT8_PTR_PTR PPAMappingCache;

extern UINT16 *EBMCacheIndex;
#endif // #if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
extern UINT16 EBlockMapIndex[MAX_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES];
extern UINT16 PPAMapIndex[MAX_PPA_MAP_INDEX * NUMBER_OF_DEVICES];
extern UINT32 RamMapIndex[NUMBER_OF_ERASE_BLOCKS * NUMBER_OF_DEVICES];

extern UINT8 gCounterLRU;
extern UINT16 gCounterDirty;
extern UINT16 gDataAreaCounterDirty;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
extern UINT8 gCheckPfForNand;
extern UINT16 gTargetPftEBForNand;
#endif

extern SAVE_STATIC_WL SaveStaticWL[NUMBER_OF_DEVICES];
extern SAVE_CAHIN_VAILD_USED_PAGE SaveValidUsedPage[(NUM_CHAIN_EBLOCKS * NUMBER_OF_DEVICES)];

extern UINT16 gCrossedLEB[NUM_CROSS_LEB];

FTL_STATUS GetLogEntryLocation(FTL_DEV devID, LOG_ENTRY_LOC_PTR nextLoc);

// Main Interface
FTL_STATUS CACHE_LoadEB (FTL_DEV devID, UINT16 logicalEBNum, UINT8 typeAPI);
FTL_STATUS CACHE_GetRAMOffsetEB (FTL_DEV devID, UINT16 logicalEBNum, UINT32_PTR EBMramStructPtr, UINT32_PTR PPAramStructPtr);
FTL_STATUS CACHE_MarkEBDirty(FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS CACHE_ClearAll(void);
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
FTL_STATUS CACHE_SetPfEBForNAND(FTL_DEV devID, UINT16 logicalEBNum, UINT8 flag);
UINT8 CACHE_IsPfEBForNAND(FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS CACHE_ClearAllPfEBForNAND(FTL_DEV devID);
#endif // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

// Index Basis Interface
FTL_STATUS CACHE_AllocateIndex(FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT8 typeAPI);
FTL_STATUS CACHE_FindIndexForAlign(FTL_DEV devID, UINT16_PTR index_ptr, UINT16 skipIndex, UINT16 skipIndex2, UINT16 removed);
FTL_STATUS CACHE_FindIndexForCross (FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT8 typeAPI);
FTL_STATUS CACHE_RemoveIndex (FTL_DEV devID, UINT16 index);
FTL_STATUS CACHE_MoveIndex(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex);
FTL_STATUS CACHE_NoDpendIndex(FTL_DEV devID, UINT16 index);
FTL_STATUS CACHE_IsThesholdDirtyIndex(void);
#ifdef DEBUG_PROTOTYPE
FTL_STATUS TABLE_Flush(UINT8 flushMode);
#endif
FTL_STATUS CACHE_UpdateLRUIndex(FTL_DEV devID, UINT16 index, UINT8 typeAPI);
FTL_STATUS CACHE_CleanAllDirtyIndex(FTL_DEV devID);

// EB Basis Interface
FTL_STATUS CACHE_InsertEB (FTL_DEV devID, UINT16 logicalEBNum, UINT16 index, UINT8 dependency, UINT8 present, UINT8 typeAPI);
FTL_STATUS CACHE_RemoveEB (FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS CACHE_GetSector(UINT16 logicalEBNum, UINT32_PTR sector_ptr);
UINT8_PTR CACHE_GetEBMCachePtr(FTL_DEV devID, UINT16 index);
UINT8_PTR CACHE_GetPPACachePtr(FTL_DEV devID, UINT16 index);

// Index EB Translation Interface
FTL_STATUS CACHE_GetIndex (FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr);
FTL_STATUS CACHE_GetEB (FTL_DEV devID, UINT16 index, UINT16_PTR logicalEBNum_ptr);

// Query Interface of Index State
FTL_STATUS CACHE_IsFreeIndex (FTL_DEV devID, UINT16 index);
FTL_STATUS CACHE_IsCleanIndex (FTL_DEV devID, UINT16 index);
FTL_STATUS CACHE_IsDirtyIndex (FTL_DEV devID, UINT16 index);

// Query Interface of LEB State
FTL_STATUS CACHE_IsPresentEB (FTL_DEV devID, UINT16 logicalEBNum, UINT8_PTR present_ptr);
FTL_STATUS CACHE_IsCrossedEB (FTL_DEV devID, UINT16 logicalEBNum);

// Structure Access Interface of EBlockMapIndex and PPAMapIndex
FTL_STATUS CACHE_SetEBlockAndPPAMap (FTL_DEV devID, UINT16 index, CACHE_INFO_EBLOCK_PPAMAP_PTR eBlockPPAMapInfo_ptr, UINT8 type);
FTL_STATUS CACHE_GetEBlockAndPPAMap (FTL_DEV devID, UINT16 index, CACHE_INFO_EBLOCK_PPAMAP_PTR eBlockPPAMapInfo_ptr, UINT8 type);
FTL_STATUS CACHE_ClearEBlockandPPAMap (FTL_DEV devID, UINT16 index, UINT8 type);

// Structure Access Interface of RamMapIndex
FTL_STATUS CACHE_SetRamMap (FTL_DEV devID, UINT16 logicalEBNum, CACHE_INFO_RAMMAP_PTR ramMapInfo_ptr);
FTL_STATUS CACHE_GetRamMap (FTL_DEV devID, UINT16 logicalEBNum, CACHE_INFO_RAMMAP_PTR ramMapInfo_ptr);
FTL_STATUS CACHE_ClearRamMap (FTL_DEV devID,UINT16 logicalEBNum);
FTL_STATUS CACHE_MoveRamMap(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex);

// Structure Access Interface of EBMCacheIndex
FTL_STATUS CACHE_SetEBMCache (FTL_DEV devID, UINT16 index, CACHE_INFO_EBMCACHE_PTR ebmCacheInfo_ptr);
FTL_STATUS CACHE_GetEBMCache (FTL_DEV devID, UINT16 index, CACHE_INFO_EBMCACHE_PTR ebmCacheInfo_ptr);
FTL_STATUS CACHE_ClearEBMCache (FTL_DEV devID, UINT16 index);
FTL_STATUS CACHE_MoveEBMCache(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex);

// Structure Access Interface of gLRUCounter
FTL_STATUS CACHE_SetgLRUCounter(UINT8 counterLRU);
FTL_STATUS CACHE_GetgLRUCounter(UINT8_PTR counterLRU_ptr);
FTL_STATUS CACHE_CleargLRUCounter(void);

// Subroutine of CACHE_FindCandidateIndexForCrossBoundary
FTL_STATUS CACHE_FindIndexNeitherPresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr);
FTL_STATUS CACHE_FindIndexUpsidePresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT16 indexUp);
FTL_STATUS CACHE_FindIndexDownsidePresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT16 indexDown);
FTL_STATUS CACHE_FindIndexBothPresented(FTL_DEV devID, UINT16_PTR index_ptr, UINT8_PTR dependency_ptr, UINT16 indexUp, UINT16 indexDown, UINT16 logicalEBNum, UINT8 typeAPI);

// Low level Interface
FTL_STATUS CACHE_FlashToCache(FTL_DEV devID, CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo, UINT16 toIndex, UINT8 type);
FTL_STATUS CACHE_CacheToFlash(FTL_DEV devID, UINT16 fromIndex, CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo, UINT8 type, UINT8 flushMode);
FTL_STATUS CACHE_CacheToCache(FTL_DEV devID, UINT16 fromIndex, UINT16 toIndex);

// Debug function
FTL_STATUS DEBUG_CACHE_EBMCacheIndexToRamMapIndex(void);
FTL_STATUS DEBUG_CACHE_RamMapIndexToEBMCacheIndex(void);
#ifdef DEBUG_TEST_ARRAY
FTL_STATUS DEBUG_CACHE_InsertTestMapping(FTL_DEV devID, UINT16 logicalEBNum, UINT32_PTR testData_ptr, UINT32_PTR testData2_ptr);
FTL_STATUS DEBUG_CACHE_ClearTestMapping(FTL_DEV devID, UINT16 logicalEBNum);
FTL_STATUS DEBUG_CACHE_CompTestMapping(FTL_DEV devID, UINT16 logicalEBNum, UINT32_PTR EBMramStructPtr, UINT32_PTR PPAramStructPtr);
#endif
#ifdef DEBUG_DATA_CLEAR
FTL_STATUS DEBUG_CACHE_SetEBMCacheFree(FTL_DEV devID, UINT16 index);
FTL_STATUS DEBUG_CACHE_CheckEBMCacheFree(void);
#endif
FTL_STATUS DEBUG_CACHE_TABLE_DISPLY(FTL_DEV devID, UINT16 tlogicalEBNum);

#endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

#if(FTL_CHECK_BAD_BLOCK_LIMIT == FTL_TRUE)
extern UINT16 gBBCount[NUMBER_OF_DEVICES];
extern UINT16 gBBDevLimit[NUMBER_OF_DEVICES];
#endif

#ifdef __cplusplus
}
#endif

#endif  // #ifndef FTL_COMMON_H
