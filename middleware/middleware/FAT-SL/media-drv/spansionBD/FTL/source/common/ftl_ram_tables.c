// file: ftl_ram_tables.c
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

#define FTL_RAM_TABLES_C
#ifdef __KERNEL__
  #include <linux/span/FTL/ftl_common.h>
#else
  #include "ftl_common.h"
#endif // #ifdef __KERNEL__

#define DEBUG_PRINT_ERRORS         (1)
#define DEBUG_BLOCK_ERASED         (0)
#define ANNOUNCE_UPDATE_PAGE_TABLE (0)
#define ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE  (0)
#define DEBUG_RESERVEDEB           (0)
#define DEBUG_SUPER_SYS            (0)

/*Global RAM declerations*/
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
#if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
UINT8 EBlockMappingCache[NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES][FLUSH_RAM_TABLE_SIZE];
UINT8 PPAMappingCache[NUM_PPA_MAP_INDEX * NUMBER_OF_DEVICES][FLUSH_RAM_TABLE_SIZE];
UINT16 EBMCacheIndex[NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES];
#else
UINT32 gSaveTotalRamAllowed = 0;
UINT8 gCheckFirstMalloc = FTL_FALSE;
UINT8_PTR_PTR EBlockMappingCache;
UINT8_PTR_PTR PPAMappingCache;
UINT16_PTR EBMCacheIndex;

UINT16 numBlockMapIndex = 0;
UINT16 numPpaMapIndex = 0;
UINT16 cacheIndexChangeArea = 0;
UINT16 thesholdDirtyCount = 0;
UINT16 ebmCacheIndexSize = 0;
UINT16 eblockMappingCacheSize = 0;
UINT16 ppaMappingCacheSize = 0;
#endif // #if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
UINT16 EBlockMapIndex[MAX_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES];
UINT16 PPAMapIndex[MAX_PPA_MAP_INDEX * NUMBER_OF_DEVICES];
UINT32 RamMapIndex[NUMBER_OF_ERASE_BLOCKS * NUMBER_OF_DEVICES];
#else
EBLOCK_MAPPING_ENTRY   EBlockMappingTable[NUM_DEVICES][NUM_EBLOCKS_PER_DEVICE];
PPA_MAPPING_ENTRY      PPAMappingTable[NUM_DEVICES][NUM_EBLOCKS_PER_DEVICE][NUM_PAGES_PER_EBLOCK];
UINT8                  PPAMappingTableDirtyBitMap[NUM_DEVICES][PPA_DIRTY_BITMAP_DEV_TABLE_SIZE];
UINT8                  EBlockMappingTableDirtyBitMap[NUM_DEVICES][EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE];
#endif
KEY_TABLE_ENTRY        FlushLogEBArray[NUM_DEVICES][NUM_FLUSH_LOG_EBLOCKS];
KEY_TABLE_ENTRY        TransLogEBArray[NUM_DEVICES][NUM_TRANSACTION_LOG_EBLOCKS];
UINT16                 FlushLogEBArrayCount[NUM_DEVICES];
UINT16                 TransLogEBArrayCount[NUM_DEVICES];
UINT32                 TransLogEBCounter[NUM_DEVICES];
UINT32                 FlushLogEBCounter[NUM_DEVICES];
#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
KEY_TABLE_ENTRY        SuperSysEBArray[NUM_DEVICES][NUM_SUPER_SYS_EBLOCKS];
UINT16                 SuperSysEBArrayCount[NUM_DEVICES];
UINT32                 SuperSysEBCounter[NUM_DEVICES];
SUPER_EB_INFO          SuperEBInfo[NUM_DEVICES];
UINT8                  gProtectForSuperSysEBFlag;
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
UINT8                  packedSuperInfo[SECTOR_SIZE];
#endif
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
UINT32                 GCNum[NUM_DEVICES];
GC_INFO                GC_Info;
UINT16                 GC_THRESHOLD;
TRANSFER_MAP_STRUCT    transferMap[NUM_TRANSFER_MAP_ENTRIES];
UINT16                 TransferMapIndexEnd;
UINT16                 TransferMapIndexStart;
FTL_DEV                previousDevice;
UINT8                  GCMoveArray[NUM_PAGES_PER_EBLOCK];    
UINT8                  GCMoveArrayNotEmpty;

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
TRANS_LOG_ENTRY        TransLogEntry;
UINT16                 TranslogBEntries;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

UINT8                  pseudoRPB[NUM_DEVICES][NUMBER_OF_BYTES_PER_PAGE];
UINT32                 LastTransLogLba;
UINT8                  LastTransLogNpages;
UINT8                  Delete_GC_Threshold;
UINT8                  FTL_initFlag = INIT_NOT_DONE;
UINT8                  FTL_UpdatedFlag = UPDATED_NOT_DONE;
DEL_INFO               Del_Info;
UINT16                 ReservedEBlock[NUM_DEVICES][NUMBER_OF_SYSTEM_EBLOCKS];
UINT16                 ReservedEBlockNum[NUM_DEVICES]; /* changed by Nobu Feb 18, 2015 : UINT8 -> UINT16 for ML16G2 */
UINT32                 FlushEBlockAdjustedFreePage[NUM_DEVICES];
TRANSFER_EB            transferEB[MAX_TRANSFER_EBLOCKS];
#if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
STATIC_WL_INFO         StaticWLInfo;
#endif
#if(FTL_RPB_CACHE == FTL_TRUE)
RPB_CACHE              RPBCache[NUM_DEVICES];
RPB_CACHE_READ_GROUP   RPBCacheReadGroup;
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

#if(FTL_CHECK_ERRORS == FTL_TRUE)
UINT8                  mountStatus = 0;
UINT8                  lockStatus = 0;
#endif  // #if(FTL_CHECK_ERRORS == FTL_TRUE)

GC_SAVE                gcSave[MAX_BLOCKS_TO_SAVE];
UINT16                 gcSaveCount;

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
UINT8                  packedLog[SECTOR_SIZE];
UINT8                  writeLogFlag = FTL_FALSE;
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
BAD_BLOCK_INFO    badBlockInfo;
UINT32            badBlockPhyPageAddr[NUM_PAGES_PER_EBLOCK];
#endif

/*It indicates UnlinkGC is invoked by Delete Operation, so that UnlinkGC does not need to save the EB.*/
UINT8                  FTL_DeleteFlag = FTL_FALSE;

#if (SPANSION_CRC32 == FTL_TRUE)
UINT32 crc32_table[256];
#endif


#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
UINT8 gCounterLRU;
UINT16 gCounterDirty;
UINT16 gDataAreaCounterDirty;
UINT8 gCacheNum;
UINT16 gCrossedLEB[NUM_CROSS_LEB];
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
UINT8 gCheckPfForNand = FTL_FALSE;
UINT16 gTargetPftEBForNand = EMPTY_WORD;
#endif

// test structure
#ifdef DEBUG_TEST_ARRAY
UINT32 EBlockMappingTableTest[NUMBER_OF_ERASE_BLOCKS][EBLOCK_MAPPING_ENTRY_SIZE/4];
UINT32 PPAMappingTableTest[NUMBER_OF_ERASE_BLOCKS][(NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)/4];
#endif

SAVE_STATIC_WL SaveStaticWL[NUMBER_OF_DEVICES];
SAVE_CAHIN_VAILD_USED_PAGE SaveValidUsedPage[(NUM_CHAIN_EBLOCKS * NUMBER_OF_DEVICES)];

#ifdef DEBUG_PROTOTYPE
UINT8 gTestFlag; 
UINT8 arrayOffsetTemp;
UINT16 glogicalEBtemp;
#endif

#ifdef DEBUG_CROSS_BOUNDARY_ROUTE
UINT32 gCacheHit = 0;
UINT32 gCacheMiss = 0;
UINT32 gNoUpAndDownCase = 0;
UINT32 gNoDownCase = 0;
UINT32 gNoUpCase = 0;
UINT32 gDistanceUpAndDownCase = 0;
#endif
#endif

#if(FTL_CHECK_BAD_BLOCK_LIMIT == FTL_TRUE)
UINT16 gBBCount[NUMBER_OF_DEVICES];
UINT16 gBBDevLimit[NUMBER_OF_DEVICES];
#endif

/*end global RAM declerations*/
//------------------------------
void Init_PseudoRPB(void)
{
   FTL_DEV devCount = 0;
   UINT16 byteCount = 0;
   for(devCount = 0; devCount < NUM_DEVICES; devCount++)
   {
      for(byteCount = 0; byteCount < NUMBER_OF_BYTES_PER_PAGE; byteCount++)
      {
           pseudoRPB[devCount][byteCount] = 0xFF;
      }
   }
}

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
void StoreSourceBadBlockInfo(FTL_DEV devID, UINT16 logicalEB, UINT16 currentOperation)
{
    UINT16 counter = 0;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif
    badBlockInfo.devID = devID;
    badBlockInfo.operation = currentOperation;
    badBlockInfo.sourceLogicalEBNum = logicalEB;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalEB, &EBMramStructPtr, &PPAramStructPtr);
    badBlockInfo.sourceEBMap = (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr));
    #else
    badBlockInfo.sourceEBMap = EBlockMappingTable[devID][logicalEB];
    #endif
    for(counter = 0; counter < NUM_PAGES_PER_EBLOCK; counter++)
    {
        #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
        badBlockInfo.sourcePPA[counter] = (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * counter)));
        #else
        badBlockInfo.sourcePPA[counter] = PPAMappingTable[devID][logicalEB][counter];
        #endif
    }
}

void StoreTargetBadBlockInfo(FTL_DEV devID, UINT16 logicalEB, UINT16 currentOperation)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif
    UINT16 counter = 0;
    badBlockInfo.devID = devID;
    badBlockInfo.operation = currentOperation;
    badBlockInfo.targetLogicalEBNum = logicalEB;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalEB, &EBMramStructPtr, &PPAramStructPtr);
    badBlockInfo.targetEBMap = (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr));
    #else
    badBlockInfo.targetEBMap = EBlockMappingTable[devID][logicalEB];
    #endif
    for(counter = 0; counter < NUM_PAGES_PER_EBLOCK; counter++)
    {
        #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
        badBlockInfo.targetPPA[counter] = (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * counter)));
        #else
        badBlockInfo.targetPPA[counter] = PPAMappingTable[devID][logicalEB][counter];
        #endif
    }
}

void ClearSourceBadBlockInfo(void)
{
    UINT16 counter = 0;                 /*2*/
    badBlockInfo.sourceEBMap.dirtyCount = EMPTY_WORD;
    badBlockInfo.sourceEBMap.freePage_GCNum = EMPTY_DWORD;
    badBlockInfo.sourceEBMap.phyEBAddr = EMPTY_WORD;
    badBlockInfo.sourceEBMap.eraseCount = EMPTY_WORD;
    badBlockInfo.sourceEBMap.chainToFrom = EMPTY_DWORD;
    badBlockInfo.sourceEBMap.isBadBlock = EMPTY_BYTE;
    for(counter = 0; counter < EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE; counter++)
    {
        badBlockInfo.sourceEBMap.freeBitMap[counter] = 0; /*all pages are free*/
    }
    for(counter = 0; counter < NUM_PAGES_PER_EBLOCK; counter++)
    {
        badBlockInfo.sourcePPA[counter] = 0;
    }
}

void ClearTargetBadBlockInfo(void)
{
    UINT16 counter = 0;
    badBlockInfo.targetEBMap.dirtyCount = EMPTY_WORD;
    badBlockInfo.targetEBMap.freePage_GCNum = EMPTY_DWORD;
    badBlockInfo.targetEBMap.phyEBAddr = EMPTY_WORD;
    badBlockInfo.targetEBMap.eraseCount = EMPTY_WORD;
    badBlockInfo.targetEBMap.chainToFrom = EMPTY_DWORD;
    badBlockInfo.targetEBMap.isBadBlock = EMPTY_BYTE;
    for(counter = 0; counter < EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE; counter++)
    {
        badBlockInfo.targetEBMap.freeBitMap[counter] = 0; /*all pages are free*/
    }
    for(counter = 0; counter < NUM_PAGES_PER_EBLOCK; counter++)
    {
        badBlockInfo.targetPPA[counter] = 0;
    }

}
void ClearBadBlockInfo(void)
{
    if(badBlockInfo.devID != EMPTY_BYTE)
    {
       badBlockInfo.devID = EMPTY_BYTE;
       badBlockInfo.sourceLogicalEBNum = EMPTY_WORD;
       badBlockInfo.targetLogicalEBNum = EMPTY_WORD;
       badBlockInfo.operation = EMPTY_WORD;
       ClearSourceBadBlockInfo();
       ClearTargetBadBlockInfo();
    }
}

void RestoreSourceBadBlockInfo(void)
{
    UINT16 counter = 0;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif

    if((badBlockInfo.devID != EMPTY_BYTE) && (badBlockInfo.sourceLogicalEBNum != EMPTY_WORD))
    {
        #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
        CACHE_GetRAMOffsetEB(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
        (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)) =  badBlockInfo.sourceEBMap;
        #else
        EBlockMappingTable[badBlockInfo.devID][badBlockInfo.sourceLogicalEBNum] = badBlockInfo.sourceEBMap;
        #endif
        for(counter = 0; counter < NUM_PAGES_PER_EBLOCK; counter++)
        {
           #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
           (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE *counter))) = badBlockInfo.sourcePPA[counter];
           #else
           PPAMappingTable[badBlockInfo.devID][badBlockInfo.sourceLogicalEBNum][counter] = badBlockInfo.sourcePPA[counter];
           #endif
        }
    }
}

void RestoreTargetBadBlockInfo(void)
{
    UINT16 counter = 0;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif

    if((badBlockInfo.devID != EMPTY_BYTE) && (badBlockInfo.targetLogicalEBNum != EMPTY_WORD))
    {
        #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
        CACHE_GetRAMOffsetEB(badBlockInfo.devID, badBlockInfo.targetLogicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
        (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)) =  badBlockInfo.targetEBMap;
        #else
        EBlockMappingTable[badBlockInfo.devID][badBlockInfo.targetLogicalEBNum] = badBlockInfo.targetEBMap;
        #endif
        for(counter = 0; counter < NUM_PAGES_PER_EBLOCK; counter++)
        {
           #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
           (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * counter))) = badBlockInfo.targetPPA[counter];
           #else
           PPAMappingTable[badBlockInfo.devID][badBlockInfo.targetLogicalEBNum][counter] = badBlockInfo.targetPPA[counter];
           #endif
        }
    }
}

void ClearTransLogEBBadBlockInfo(void)
{
    badBlockInfo.TransLogEBFailed = FTL_FALSE;
    badBlockInfo.TransLogEBNum = EMPTY_WORD;
    badBlockInfo.reserved = EMPTY_WORD;
}

void SetTransLogEBFailedBadBlockInfo(void)
{
    badBlockInfo.TransLogEBFailed = FTL_TRUE;
}

UINT8 GetTransLogEBFailedBadBlockInfo(void)
{
    return badBlockInfo.TransLogEBFailed;
}

void SetTransLogEBNumBadBlockInfo(UINT16 logicalEBNum)
{
    badBlockInfo.TransLogEBNum = logicalEBNum;
}

UINT16 GetTransLogEBNumBadBlockInfo(void)
{
    return badBlockInfo.TransLogEBNum;
}


FTL_STATUS isBadBlockError(FTL_STATUS status)
{
    FTL_STATUS result = FTL_ERR_PASS;

    result = TranslateBadBlockError(status);
    if((result == FTL_ERR_LOG_WR) || (result == FTL_ERR_BAD_BLOCK_SOURCE) )
    {
        return FTL_TRUE;
    }
    return FTL_FALSE;
}

FTL_STATUS TranslateBadBlockError(FTL_STATUS status)
{
    /* three types of errors can be returned, data area writes, data reserve pool writes, and system area logging
    // the strategy for dealing with data area writes is to 
        1) mark the block bad, 
        2) copy the orginal data using the saved EB mapping info to a new block in the reseve pool, 
        3) swap the bad the good block
        4) overwrite the old EB mapping table with the saved one
        5) update the phy eb info to point to the new block
        5) flush
        6) restart the transfer by building the transfer map again
    // the strategy for dealing with reserve pool writes, is to 
        1) mark the block bad, 
        2) do a system flush
    // the strategy for dealing the logging area write is 
        1)     */
    switch (status)
    {
       case FTL_ERR_LOG_WR:
           return FTL_ERR_LOG_WR;      

       case FTL_ERR_FLASH_WRITE_04:
       case FTL_ERR_FLASH_WRITE_02:
       case FTL_ERR_FLASH_WRITE_17:
       case FTL_ERR_FLASH_COMMIT_02:
       case FTL_ERR_FLASH_COMMIT_06:
       case FTL_ERR_FLASH_COMMIT_07:
       case FTL_ERR_GC_ERASE1:
       case FTL_ERR_FLASH_WRITE_18:
           return FTL_ERR_BAD_BLOCK_SOURCE;

       case FTL_ERR_FLASH_BUSY_09:
       case FTL_ERR_FLASH_STATUS_03:
       case FTL_ERR_FLASH_ERASE_01:
       case FTL_ERR_GC_ERASE2:
       case FTL_ERR_DATA_GC_FLASH_TIMEOUT2:
       case FTL_ERR_GC_PAGE_WR2:
       case FTL_ERR_DATA_GC_FLASH_TIMEOUT1:
       case FTL_ERR_GC_PAGE_WR1:
           return FTL_ERR_BAD_BLOCK_TARGET;
       
       default:
           return status;
    }
}



#endif

//-----------------------------
void FTL_ClearGCSave (UINT8 clearMode) 
{
    UINT8 index = 0;                                   /*1*/

    for (index = 0; index < MAX_BLOCKS_TO_SAVE; index++)
    {
       if(CLEAR_GC_SAVE_RUNTIME_MODE == clearMode)
       {
          if ((EMPTY_BYTE != gcSave[index].devId) && (EMPTY_WORD != gcSave[index].phyEbNum))
          {
             #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
             if (EMPTY_WORD != GetLogicalEBlockAddr(gcSave[index].devId, gcSave[index].phyEbNum))
             {
                TABLE_InsertReservedEB(gcSave[index].devId, GetLogicalEBlockAddr(gcSave[index].devId, gcSave[index].phyEbNum));
             }
             #else
             TABLE_InsertReservedEB(gcSave[index].devId, GetLogicalEBlockAddr(gcSave[index].devId,gcSave[index].phyEbNum));
             #endif
          }
       }
       gcSave[index].devId = EMPTY_BYTE;
       gcSave[index].phyEbNum = EMPTY_WORD;
    }
    gcSaveCount = 0;
}

//---------------------------------
FTL_STATUS FTL_AddToGCSave (FTL_DEV devId, UINT16 phyEbNum)
{
    UINT8 index = 0;                                   /*1*/

    for (index = 0; index < MAX_BLOCKS_TO_SAVE; index++)
    {
       if (gcSave[index].devId == EMPTY_BYTE && gcSave[index].phyEbNum == EMPTY_WORD)
       {
          gcSave[index].devId = devId;
          gcSave[index].phyEbNum = phyEbNum;
          gcSaveCount++;
          return FTL_ERR_PASS;
       }
    }
    return FTL_ERR_NO_ROOM_IN_GCSAVE;
}

/*erase counter functions*/
#if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
//---------------------------------
UINT8 GetEBErased(FTL_DEV devID, UINT16 logEBNum)
{  
    UINT32 temp = 0;                                   /*2*/

    temp = GetEraseCount(devID, logEBNum);
    temp = ((temp & ERASE_STATUS_GET_DWORD_MASK) >> ERASE_STATUS_DWORD_SHIFT);
    return (UINT8)temp;
}

//----------------------------------
void SetEBErased(FTL_DEV devID, UINT16 logEBNum, UINT8 eraseStatus)
{
   UINT32 temp = 0;
   UINT32 temp2 = 0;

   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
   UINT32 EBMramStructPtr = 0;
   UINT32 PPAramStructPtr = 0;
   #endif

   #if DEBUG_BLOCK_ERASED
   DBG_Printf("SetEBErased: logEBNum = 0x%X, ", logEBNum, 0);
   DBG_Printf("eraseStatus = %d\n", eraseStatus, 0);
   #endif  // #if DEBUG_BLOCK_ERASED

   temp = GetEraseCount(devID, logEBNum);
   temp = (temp & ERASE_STATUS_CLEAR_DWORD_MASK);
   temp2 = ((eraseStatus << ERASE_STATUS_DWORD_SHIFT) & ERASE_STATUS_GET_DWORD_MASK);
   temp = temp | temp2;
   SetEraseCount(devID, logEBNum, temp);
}
#endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

/*Access functions*/
//--------------------------
UINT32 GetTransLogEBCounter(FTL_DEV devID)
{
    return TransLogEBCounter[devID];    
}

//------------------------------
UINT16 GetTransLogEBArrayCount(FTL_DEV devID)
{
   return TransLogEBArrayCount[devID];
}

//----------------------
void SetTransLogEBCounter(FTL_DEV devID, UINT32 counter)
{
    TransLogEBCounter[devID] = counter;    
}

//-------------------------
UINT32 GetFlushEBCounter(FTL_DEV devID)
{
    return FlushLogEBCounter[devID];    
}

//---------------------------
void SetFlushLogEBCounter(FTL_DEV devID, UINT32 counter)
{
    FlushLogEBCounter[devID] = counter;    
}

//------------------------------
UINT16 GetFlushLogEBArrayCount(FTL_DEV devID)
{
   return FlushLogEBArrayCount[devID];
}

/********* Block Info ****************/

//-----------------------------
UINT16 GetPhysicalEBlockAddr(FTL_DEV devID, UINT16 logicalBlockNum)
{

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    return (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).phyEBAddr;
    #else
    return (EBlockMappingTable[devID][logicalBlockNum].phyEBAddr);   
    #endif
}


UINT16 GetLogicalEBlockAddr(FTL_DEV devID, UINT16 physicalBlockNum)
{
    UINT16 logicalBlockNum = EMPTY_WORD;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    for(logicalBlockNum = NUM_DATA_EBLOCKS;logicalBlockNum < NUM_EBLOCKS_PER_DEVICE; logicalBlockNum++)
    #else
    for(logicalBlockNum = 0;logicalBlockNum < NUM_EBLOCKS_PER_DEVICE; logicalBlockNum++)
    #endif
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
       if(physicalBlockNum == (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).phyEBAddr)
       #else
       if(physicalBlockNum == EBlockMappingTable[devID][logicalBlockNum].phyEBAddr)
       #endif
       {
          return logicalBlockNum;
       }
    }
    // fail
    return EMPTY_WORD;    
}

//------------------------------
void SetPhysicalEBlockAddr(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 phyBlockNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetPhysicalEBlockAddr: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("phyBlockNum = 0x%X\n", phyBlockNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).phyEBAddr = phyBlockNum;
    #else
    EBlockMappingTable[devID][logicalBlockNum].phyEBAddr = phyBlockNum;  
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//-------------------------------
UINT32 GetEraseCount(FTL_DEV devID, UINT16 logicalBlockNum)
{
    UINT32 eraseCount = 0;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    eraseCount = ((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).eraseCount & ERASE_STATUS_CLEAR_WORD_MASK);
    eraseCount |= (UINT32)(((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount & ERASE_COUNT_GET_WORD_DIRTY_MASK) << ERASE_COUNT_DWORD_DIRTY_SHIFT);
    if(ERASE_STATUS_GET_WORD_MASK == ((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).eraseCount & ERASE_STATUS_GET_WORD_MASK))
    {
       eraseCount |= ERASE_STATUS_GET_DWORD_MASK;
    }
    return eraseCount;
    #else
    eraseCount = (EBlockMappingTable[devID][logicalBlockNum].eraseCount & ERASE_STATUS_CLEAR_WORD_MASK);
    eraseCount |= (UINT32)((EBlockMappingTable[devID][logicalBlockNum].dirtyCount & ERASE_COUNT_GET_WORD_DIRTY_MASK) << ERASE_COUNT_DWORD_DIRTY_SHIFT);
    if(ERASE_STATUS_GET_WORD_MASK == (EBlockMappingTable[devID][logicalBlockNum].eraseCount & ERASE_STATUS_GET_WORD_MASK))
    {
       eraseCount |= ERASE_STATUS_GET_DWORD_MASK;
    }
    return eraseCount;
    #endif    
}

//------------------------------
UINT32 GetTrueEraseCount(FTL_DEV devID, UINT16 logicalBlockNum)
{
    UINT32 temp = 0;                                   /*2*/

    temp = GetEraseCount(devID, logicalBlockNum);
    temp = temp & ERASE_STATUS_CLEAR_DWORD_MASK;
    return temp;
}

//------------------------------
void SetEraseCount(FTL_DEV devID, UINT16 logicalBlockNum, UINT32 eraseCount)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetEraseCount: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("eraseCount = 0x%X\n", eraseCount, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).eraseCount = (UINT16)(eraseCount & ERASE_STATUS_CLEAR_WORD_MASK);
    if(ERASE_STATUS_GET_DWORD_MASK == (eraseCount & ERASE_STATUS_GET_DWORD_MASK))
    {
       (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).eraseCount |= ERASE_STATUS_GET_WORD_MASK;
    }
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount &= ERASE_COUNT_CLEAR_WORD_DIRTY_MASK;
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount |= ((eraseCount >> ERASE_COUNT_DWORD_DIRTY_SHIFT) & ERASE_COUNT_GET_WORD_DIRTY_MASK);
    #else
    EBlockMappingTable[devID][logicalBlockNum].eraseCount = (UINT16)(eraseCount & ERASE_STATUS_CLEAR_WORD_MASK);
    if(ERASE_STATUS_GET_DWORD_MASK == (eraseCount & ERASE_STATUS_GET_DWORD_MASK))
    {
       EBlockMappingTable[devID][logicalBlockNum].eraseCount |= ERASE_STATUS_GET_WORD_MASK;
    }
    EBlockMappingTable[devID][logicalBlockNum].dirtyCount &= ERASE_COUNT_CLEAR_WORD_DIRTY_MASK;
    EBlockMappingTable[devID][logicalBlockNum].dirtyCount |= ((eraseCount >> ERASE_COUNT_DWORD_DIRTY_SHIFT) & ERASE_COUNT_GET_WORD_DIRTY_MASK);
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//-------------------------------
void IncEraseCount(FTL_DEV devID, UINT16 logicalBlockNum)
{
    UINT32 eraseCount = 0;                             /*2*/

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("IncEraseCount: logicalBlockNum = 0x%X\n", logicalBlockNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    eraseCount = GetTrueEraseCount(devID, logicalBlockNum);
    if(eraseCount < ERASE_COUNT_LIMIT)
    {
       eraseCount = GetEraseCount(devID, logicalBlockNum) + 1;
       SetEraseCount(devID, logicalBlockNum, eraseCount);
    }
    else
    {
        DBG_Printf("[Warning] Erase Count is over ERASE_COUNT_LIMIT", 0, 0);
    }
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//------------------------------
UINT16 GetDirtyCount(FTL_DEV devID, UINT16 logicalBlockNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    return (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount;
    #else
    return (EBlockMappingTable[devID][logicalBlockNum].dirtyCount);    
    #endif
}

#if( FTL_EBLOCK_CHAINING == FTL_TRUE)
//------------------------------
UINT16 GetChainLogicalEBNum(FTL_DEV devID, UINT16 logicalBlockNum)
{
    UINT32 temp = 0;                                   /*4*/
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    temp = (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom;
    #else
    temp = (EBlockMappingTable[devID][logicalBlockNum].chainToFrom);    
    #endif
    return (UINT16)(temp >> NUM_BITS_EB_CHAIN);
}

//--------------------------------
UINT16 GetChainPhyEBNum(FTL_DEV devID, UINT16 logicalBlockNum)
{
    UINT32 temp = 0;                                   /*4*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    temp = (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom;
    #else
    temp = (EBlockMappingTable[devID][logicalBlockNum].chainToFrom);    
    #endif
    return (UINT16)(temp & EB_PHYSICAL_CHAIN_MASK);
}

//--------------------------------
void SetChainLogicalEBNum(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 logEBNum)
{
    UINT32 temp = 0;                                   /*4*/
    UINT32 temp2 = (UINT32)logEBNum;                   /*4*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetChainLogicalEBNum: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("logEBNum = 0x%X\n", logEBNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    temp = (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom;
    #else
    temp = (EBlockMappingTable[devID][logicalBlockNum].chainToFrom);    
    #endif
    temp = temp & EB_PHYSICAL_CHAIN_MASK;
    temp = temp | (temp2 << NUM_BITS_EB_CHAIN);
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom = temp;
    #else
    EBlockMappingTable[devID][logicalBlockNum].chainToFrom = temp;   
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//----------------------------------
void SetChainPhyEBNum(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 PhyEBNum)
{
    UINT32 temp = 0;                                   /*4*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetChainPhyEBNum:  logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("PhyEBNum = 0x%X\n", PhyEBNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    temp = (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom;
    #else
    temp = (EBlockMappingTable[devID][logicalBlockNum].chainToFrom);    
    #endif
    temp = temp & EB_LOGICAL_CHAIN_MASK;
    temp = temp | ((UINT32)PhyEBNum);
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom = temp;
    #else
    EBlockMappingTable[devID][logicalBlockNum].chainToFrom = temp;    
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//-------------------------------
void ClearChainLink(FTL_DEV devID, UINT16 logicalBlockNumFrom, UINT16 logicalBlockNumTo)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT8 count = 0;
    #endif
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("ClearChainLink: logicalBlockNumFrom = 0x%X, ", logicalBlockNumFrom, 0);
    DBG_Printf("logicalBlockNumTo = 0x%X, ", logicalBlockNumTo, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_GetRAMOffsetEB(devID, logicalBlockNumFrom, &EBMramStructPtr, &PPAramStructPtr);
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom = EMPTY_DWORD;
    CACHE_GetRAMOffsetEB(devID, logicalBlockNumTo, &EBMramStructPtr, &PPAramStructPtr);
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom = EMPTY_DWORD;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    for (count = 0; count < NUM_CHAIN_EBLOCKS; count++)
    {
        if (logicalBlockNumFrom == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum || logicalBlockNumTo == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
        {
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = EMPTY_WORD;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount = EMPTY_WORD;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount = EMPTY_WORD;
            break;
        }
    }
    #endif
    #else
    EBlockMappingTable[devID][logicalBlockNumFrom].chainToFrom = EMPTY_DWORD;    
    EBlockMappingTable[devID][logicalBlockNumTo].chainToFrom = EMPTY_DWORD; 
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNumFrom);
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNumTo);
}

//---------------------------
void SetChainLink(FTL_DEV devID, UINT16 logEBNumFrom, UINT16 logEBNumTo, 
    UINT16 phyEBNumFrom, UINT16 phyEBNumTo)
{
    SetChainLogicalEBNum(devID, logEBNumFrom, logEBNumTo);
    SetChainPhyEBNum(devID, logEBNumFrom, phyEBNumTo);
    SetChainLogicalEBNum(devID, logEBNumTo, logEBNumFrom);
    SetChainPhyEBNum(devID, logEBNumTo, phyEBNumFrom);
    MarkEBlockMappingTableEntryDirty(devID, logEBNumFrom);
    MarkEBlockMappingTableEntryDirty(devID, logEBNumTo);
}

#if(FTL_UNLINK_GC == FTL_TRUE)
//-----------------------------
FTL_STATUS UnlinkChain(FTL_DEV devID, UINT16 logicalBlockNumFrom, UINT16 logicalBlockNumTo)
{
    UINT16 physicalBlockNumFrom = EMPTY_WORD;          /*2*/
    UINT16 physicalBlockNumTo = EMPTY_WORD;            /*2*/
    UINT32 eraseCountFrom = 0;                         /*2*/
    UINT32 eraseCountTo = 0;                           /*2*/
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 phyPageOffset = 0;                          /*2*/
    FREE_BIT_MAP_TYPE bitMap = 0;                      /*1*/
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT8 badEBlockFlag = FTL_FALSE;             
    #endif
    for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
    {
       phyPageOffset = GetPPASlot(devID, logicalBlockNumTo, pageOffset);
       SetPPASlot(devID, logicalBlockNumFrom, pageOffset, phyPageOffset);
       bitMap = GetEBlockMapFreeBitIndex(devID, logicalBlockNumTo, pageOffset);
       SetEBlockMapFreeBitIndex(devID, logicalBlockNumFrom, pageOffset, bitMap);
    }

    // change physical addr
    physicalBlockNumFrom = GetPhysicalEBlockAddr(devID, logicalBlockNumFrom);
    physicalBlockNumTo = GetPhysicalEBlockAddr(devID, logicalBlockNumTo);
    SetPhysicalEBlockAddr(devID, logicalBlockNumFrom, physicalBlockNumTo);
    SetPhysicalEBlockAddr(devID, logicalBlockNumTo, physicalBlockNumFrom);

    // change erase count
    eraseCountFrom = GetEraseCount(devID, logicalBlockNumFrom);
    eraseCountTo = GetEraseCount(devID, logicalBlockNumTo);
    SetEraseCount(devID, logicalBlockNumFrom, eraseCountTo);
    SetEraseCount(devID, logicalBlockNumTo, eraseCountFrom);
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
    eraseCountTo = GetTrueEraseCount(devID, logicalBlockNumFrom);
    SetSaveStaticWL(devID, logicalBlockNumFrom, eraseCountTo);
    eraseCountFrom = GetTrueEraseCount(devID, logicalBlockNumTo);
    SetSaveStaticWL(devID, logicalBlockNumTo, eraseCountFrom);
    #endif
    #endif
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    badEBlockFlag = GetBadEBlockStatus(devID, logicalBlockNumFrom);             
    SetBadEBlockStatus(devID, logicalBlockNumFrom, GetBadEBlockStatus(devID, logicalBlockNumTo));
    SetBadEBlockStatus(devID, logicalBlockNumTo, badEBlockFlag);
    #endif    
    SetGCOrFreePageNum(devID, logicalBlockNumFrom, GCNum[devID]++);
    SetDirtyCount(devID, logicalBlockNumFrom, GetDirtyCount(devID, logicalBlockNumTo));

    ClearChainLink(devID, logicalBlockNumFrom, logicalBlockNumTo);
    MarkAllPagesStatus(devID, logicalBlockNumTo, BLOCK_INFO_STALE_PAGE);

    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNumFrom);
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNumTo);
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

//---------------------------
UINT16 GetChainWithLowestVaildPages(FTL_DEV devID)
{
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 chainEB = 0;                                /*2*/
    UINT16 numValidPages = 0;                          /*2*/
    UINT16 numValidLowest = EMPTY_WORD;                /*2*/
    UINT16 highEB = EMPTY_WORD;                        /*2*/

    for (logicalEBNum = NUM_DATA_EBLOCKS; logicalEBNum < NUM_EBLOCKS_PER_DEVICE; logicalEBNum++)
    {
       if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,logicalEBNum))
       {
          continue;
       }
       chainEB = GetChainLogicalEBNum(devID, logicalEBNum);
       if(chainEB != EMPTY_WORD)
       { 
          numValidPages =  GetNumValidPages(devID, logicalEBNum);
          numValidPages +=  GetNumValidPages(devID, chainEB);
          if(numValidPages  > numValidLowest)
          {
             numValidLowest = numValidPages;
             highEB = chainEB;
          }               
       }
    }
    return highEB;
}

//------------------------
UINT16 GetChainWithLowestVaildUsedRatio(FTL_DEV devID)
{
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 chainEB = 0;                                /*2*/
    UINT32 numUsed = 0;                                /*4*/
    UINT16 numUsedTemp = 0;                            /*2*/
    UINT16 numValidTemp = 0;                           /*2*/
    UINT32 numValid = 0;                               /*4*/
    UINT32 currentInverseRatio = 0;                    /*4*/
    UINT32 highestInverseRatio = 0; /*using inverse because interger division will produce 0*/
    UINT16 highEB = EMPTY_WORD;                        /*2*/
    UINT8 eblockCount = 0;                             /*1*/
    TRANSFER_EB transferEB = {EMPTY_BYTE, EMPTY_WORD}; /*3*/
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE) 
    UINT8 count = 0;
    #endif

    for (logicalEBNum = NUM_DATA_EBLOCKS; logicalEBNum < NUM_EBLOCKS_PER_DEVICE; logicalEBNum++)
    {
       if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,logicalEBNum))
       {
          continue;
       }
       chainEB = GetChainLogicalEBNum(devID, logicalEBNum);
       if(chainEB != EMPTY_WORD)
       { 
          GetNumValidUsedPages(devID, logicalEBNum, &numUsedTemp, &numValidTemp);             
          numValid =  (UINT32)numValidTemp;
          numUsed = (UINT32)numUsedTemp;
          if(numUsed == 0)
          {
             continue;
          }
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE) 
          for(count = 0; count < NUM_CHAIN_EBLOCKS; count++)
          {
              if (chainEB == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
             {
                  numValid += (UINT32)SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount;
                  numUsed += (UINT32)SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount;
                break;
             }
          }
          #else
          GetNumValidUsedPages(devID, chainEB, &numUsedTemp, &numValidTemp);             
          numValid +=  (UINT32)numValidTemp;
          numUsed += (UINT32)numUsedTemp;
          #endif
          if(numValid != 0)
          {
             currentInverseRatio =  (numUsed << 15)/numValid;
          }
          else
          {
             currentInverseRatio = EMPTY_DWORD;
          }
          if(highestInverseRatio  < currentInverseRatio)
          {
             highestInverseRatio = currentInverseRatio;
             highEB = chainEB;
          }               
          else if(highestInverseRatio == currentInverseRatio)
          {
             for(eblockCount = 0; eblockCount < MAX_TRANSFER_EBLOCKS; eblockCount++)
             {
                GetTransferEB(eblockCount, &transferEB);
                if((devID == transferEB.devID) && (highEB == transferEB.logicalEBNum))
                {
                   highEB = chainEB;
                }
             }
          }
       }
    }
    return highEB;
}
#endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

//--------------------------------
UINT16 GetNumValidPages(FTL_DEV devID, UINT16 logicalEBNum)
{
    UINT16 temp = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    FREE_BIT_MAP_TYPE bitValue = 0;                    /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(temp = 0; temp < NUM_PAGES_PER_EBLOCK; temp++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #else
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalEBNum].freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #endif
       if(bitValue == BLOCK_INFO_VALID_PAGE)
       {
          count++;
       }
    }
    return count;
}

//-------------------------------
UINT16 GetNumInvalidPages(FTL_DEV devID, UINT16 logicalEBNum)
{
    UINT16 temp = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    FREE_BIT_MAP_TYPE bitValue = 0;                    /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(temp = 0; temp < NUM_PAGES_PER_EBLOCK; temp++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #else
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalEBNum].freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #endif
       if(bitValue == BLOCK_INFO_STALE_PAGE)
       {
          count++;
       }
    }
    return count;
}

//-------------------------------
UINT16 GetNumUsedPages(FTL_DEV devID, UINT16 logicalEBNum)
{
    UINT16 temp = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    FREE_BIT_MAP_TYPE bitValue = 0;                    /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(temp = 0; temp < NUM_PAGES_PER_EBLOCK; temp++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #else
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalEBNum].freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #endif
       if(bitValue != BLOCK_INFO_EMPTY_PAGE)
       {
          count++;
       }
    }
    return count;
}

//---------------------------------------
void GetNumValidUsedPages(FTL_DEV devID, UINT16 logicalEBNum, UINT16_PTR used, UINT16_PTR valid)
{
    UINT16 temp = 0;                                   /*2*/
    UINT16 usedCount = 0;                              /*2*/
    UINT16 validCount = 0;                             /*2*/
    FREE_BIT_MAP_TYPE bitValue = 0;                    /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(temp = 0; temp < NUM_PAGES_PER_EBLOCK; temp++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #else
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalEBNum].freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #endif
       if(bitValue != BLOCK_INFO_EMPTY_PAGE)
       {
          usedCount++;
       }
       if(bitValue == BLOCK_INFO_VALID_PAGE)
       {
          validCount++;
       }        
    }
    *used = usedCount;
    *valid = validCount;
}

//-----------------------------------
UINT16 GetNumFreePages(FTL_DEV devID, UINT16 logicalEBNum)
{
    UINT16 temp = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    FREE_BIT_MAP_TYPE bitValue = 0;                    /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(temp = 0; temp < NUM_PAGES_PER_EBLOCK; temp++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #else
       bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalEBNum].freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
       #endif
       if(bitValue == BLOCK_INFO_EMPTY_PAGE)
       {
          count++;
       }
    }
    return count;
}

//----------------------------------
void SetDirtyCount(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 dirtyCount)
{

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetDirtyCount: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("dirtyCount = 0x%X\n", dirtyCount, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount = dirtyCount;
    #else
    EBlockMappingTable[devID][logicalBlockNum].dirtyCount = dirtyCount;    
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//-----------------------------------
UINT16 GetTotalFreePages(FTL_DEV devID, UINT16 logicalEBNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    return (NUM_PAGES_PER_EBLOCK - (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount);
    #else
    return (NUM_PAGES_PER_EBLOCK - EBlockMappingTable[devID][logicalEBNum].dirtyCount);
    #endif
}

//-------------------------------------
UINT16 GetFreePageIndex(FTL_DEV devID, UINT16 logicalBlockNum)
{
    UINT16 temp = 0;                                   /*2*/
    FREE_BIT_MAP_TYPE bitValue = 0;                    /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);

    if((logicalBlockNum < NUM_DATA_EBLOCKS) || (EMPTY_DWORD != (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom))    
    #else
    if((logicalBlockNum < NUM_DATA_EBLOCKS) || (EMPTY_DWORD != EBlockMappingTable[devID][logicalBlockNum].chainToFrom))
    #endif
    {
       for(temp = 0; temp < NUM_PAGES_PER_EBLOCK; temp++)
       {
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
          #else
          bitValue = (FREE_BIT_MAP_TYPE) GetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0])), 
          temp, FTL_BITS_PER_PAGE);
          #endif
          if(bitValue == BLOCK_INFO_EMPTY_PAGE)
          {
             return temp;
          }
       }
    }
    else
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       return (UINT16)((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum);
       #else
       return (UINT16)(EBlockMappingTable[devID][logicalBlockNum].freePage_GCNum);
       #endif
    }
    return temp;
}

//-------------------------------------
void SetUsedPageIndex(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 usedPageIdx)
{

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 count = 0;
    UINT16 freeCount = EMPTY_WORD;
    #endif

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetUsedPageIndex:  logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("usedPageIdx = 0x%X\n", usedPageIdx, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    /*this one will need to scan the bit map and set it as well*/
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    SetBitMapField((UINT8_PTR)(&((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0])),
        usedPageIdx, FTL_BITS_PER_PAGE, BLOCK_INFO_VALID_PAGE);
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if ((NUM_DATA_EBLOCKS > logicalBlockNum) && (EMPTY_WORD != GetChainLogicalEBNum(devID, logicalBlockNum)))
    {
       for (count = 0; count < (NUM_CHAIN_EBLOCKS); count++)
       {
          if (logicalBlockNum == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
          {
             break;
          }
          if (EMPTY_WORD == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
          {
              if (EMPTY_WORD == freeCount)
              {
                  freeCount = count;
              }
          }
       }
       if (count == NUM_CHAIN_EBLOCKS)
       {
          count = freeCount;
          SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = logicalBlockNum;
       }
       GetNumValidUsedPages(devID, logicalBlockNum, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount);
    }
    #endif // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #else
    SetBitMapField((UINT8_PTR)(&(EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0])), 
       usedPageIdx, FTL_BITS_PER_PAGE, BLOCK_INFO_VALID_PAGE);    
    #endif  // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//--------------------------------------
void SetOldPageIndex(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 oldPageIdx)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 count = 0;
    UINT16 freeCount = EMPTY_WORD;
    #endif

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetOldPageIndex: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("oldPageIdx = 0x%X\n", oldPageIdx, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    /*this one will need to scan the bit map and set it as well*/
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    SetBitMapField(&(*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0], oldPageIdx, FTL_BITS_PER_PAGE, BLOCK_INFO_STALE_PAGE);    
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount++;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if ((NUM_DATA_EBLOCKS > logicalBlockNum) && (EMPTY_WORD != GetChainLogicalEBNum(devID, logicalBlockNum)))
    {
        for (count = 0; count < (NUM_CHAIN_EBLOCKS); count++)
        {
            if (logicalBlockNum == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
            {
                break;
            }
            if (EMPTY_WORD == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
            {
                if (EMPTY_WORD == freeCount)
                {
                    freeCount = count;
                }
            }
        }
        if (count == NUM_CHAIN_EBLOCKS)
        {
            count = freeCount;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = logicalBlockNum;
        }
        GetNumValidUsedPages(devID, logicalBlockNum, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount);
    }
    #endif // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #else    
    SetBitMapField(&EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0], oldPageIdx, FTL_BITS_PER_PAGE, BLOCK_INFO_STALE_PAGE);    
    EBlockMappingTable[devID][logicalBlockNum].dirtyCount++;
    #endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//-------------------------------------
void UpdatePageTableInfo(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 logicalPageIndex, 
    UINT16 phyPageIdx, UINT8 bitStatus)
{
    UINT16 tempPPA = EMPTY_WORD;                       /*2*/
    
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 count = 0;
    UINT16 freeCount = EMPTY_WORD;
    #endif

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_PAGE_TABLE
    DBG_Printf("UpdatePageTableInfo: logBlock=0x%X, ", logicalBlockNum, 0);
    DBG_Printf("logPageIdx=0x%X, ", logicalPageIndex, 0);
    DBG_Printf("phyPageIdx=0x%X, ", phyPageIdx, 0);
    DBG_Printf("bitSt=%d\n", bitStatus, 0);
    #endif  // #if ANNOUNCE_UPDATE_PAGE_TABLE
    
    if(logicalPageIndex == EMPTY_INVALID) // special case to just change the bit map.
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       SetBitMapField(&(*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0], 
          phyPageIdx, FTL_BITS_PER_PAGE, bitStatus);             
       #else
       SetBitMapField(&EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0], 
          phyPageIdx, FTL_BITS_PER_PAGE, bitStatus);             
       #endif
    }
    else
    {
       if((phyPageIdx == EMPTY_INVALID) || phyPageIdx == CHAIN_INVALID)
       {
          // get the current physical page for the PPA slot
          // because it will get cleared later
          tempPPA = GetPPASlot(devID, logicalBlockNum, logicalPageIndex);       
          if((tempPPA != EMPTY_INVALID) && (tempPPA != CHAIN_INVALID))
          {
             #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
             SetBitMapField(&(*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0], 
                tempPPA, FTL_BITS_PER_PAGE, bitStatus);    
             #else
             SetBitMapField(&EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0], 
                tempPPA, FTL_BITS_PER_PAGE, bitStatus);    
             #endif
          }
       }
       else
       {
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          SetBitMapField(&(*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0], 
             phyPageIdx, FTL_BITS_PER_PAGE, bitStatus);   
          #else
          SetBitMapField(&EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0], 
             phyPageIdx, FTL_BITS_PER_PAGE, bitStatus);    
          #endif
       }
       SetPPASlot(devID, logicalBlockNum, logicalPageIndex, phyPageIdx);
       if(bitStatus == BLOCK_INFO_STALE_PAGE) 
       {
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount++;
          #else
          EBlockMappingTable[devID][logicalBlockNum].dirtyCount++;
          #endif
       }
       MarkPPAMappingTableEntryDirty(devID, logicalBlockNum, logicalPageIndex);
    }
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if ((NUM_DATA_EBLOCKS > logicalBlockNum) && (EMPTY_WORD != GetChainLogicalEBNum(devID, logicalBlockNum)))
    {
       for (count = 0; count < (NUM_CHAIN_EBLOCKS); count++)
       {
          if (logicalBlockNum == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
          {
             break;
          }
          if (EMPTY_WORD == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
          {
             if (EMPTY_WORD == freeCount)
             {
                freeCount = count;
             }
          }
       }
       if (count == NUM_CHAIN_EBLOCKS)
       {
          count = freeCount;
          SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = logicalBlockNum;
       }
       GetNumValidUsedPages(devID, logicalBlockNum, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount);
    }
    #endif // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//----------------------------------
void DecGCOrFreePageNum(FTL_DEV devID, UINT16 logicalEBNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("DecGCOrFreePageNum: logicalEBNum = 0x%X\n", logicalEBNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum--;
    #else
    EBlockMappingTable[devID][logicalEBNum].freePage_GCNum--;
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
}

//------------------------------------
void IncGCOrFreePageNum(FTL_DEV devID, UINT16 logicalEBNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("IncGCOrFreePageNum: logicalEBNum = 0x%X\n", logicalEBNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum++;
    #else
    EBlockMappingTable[devID][logicalEBNum].freePage_GCNum++;
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
}

//-------------------------------------
UINT32 GetGCNum(FTL_DEV devID, UINT16 logicalBlockNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    return ((*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum);
    #else
    return (EBlockMappingTable[devID][logicalBlockNum].freePage_GCNum);    
    #endif
}

//-------------------------------------
void SetGCOrFreePageNum(FTL_DEV devID, UINT16 logicalBlockNum, UINT32 GCNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetGCOrFreePageNum: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("GCNum = 0x%X\n", GCNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum = GCNum;
    #else
    EBlockMappingTable[devID][logicalBlockNum].freePage_GCNum = GCNum;   
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
}

//-------------------------------------
FREE_BIT_MAP_TYPE GetEBlockMapFreeBitIndex(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 phyPageOffset)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    return (FREE_BIT_MAP_TYPE) GetBitMapField(&(*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0], 
       phyPageOffset, FTL_BITS_PER_PAGE);
    #else
    return (FREE_BIT_MAP_TYPE) GetBitMapField(&EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0], 
       phyPageOffset, FTL_BITS_PER_PAGE);
    #endif
}

//--------------------------------------
void SetEBlockMapFreeBitIndex(FTL_DEV devID, UINT16 logicalBlockNum, UINT16 phyPageOffset, FREE_BIT_MAP_TYPE value)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 count = 0;
    UINT16 freeCount = EMPTY_WORD;
    #endif

    CACHE_GetRAMOffsetEB(devID, logicalBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("SetEBlockMapFreeBitIndex: logicalBlockNum = 0x%X, ", logicalBlockNum, 0);
    DBG_Printf("phyPageOffset = 0x%X, ", phyPageOffset, 0);
    DBG_Printf("value = %d\n", value, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    SetBitMapField(&(*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[0], phyPageOffset, FTL_BITS_PER_PAGE, value);
    #else
    SetBitMapField(&EBlockMappingTable[devID][logicalBlockNum].freeBitMap[0], phyPageOffset, FTL_BITS_PER_PAGE, value);
    #endif
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if ((NUM_DATA_EBLOCKS > logicalBlockNum) && (EMPTY_WORD != GetChainLogicalEBNum(devID, logicalBlockNum)))
    {
        for (count = 0; count < (NUM_CHAIN_EBLOCKS); count++)
        {
            if (logicalBlockNum == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
            {
                break;
            }
            if (EMPTY_WORD == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
            {
                if (EMPTY_WORD == freeCount)
                {
                    freeCount = count;
                }
            }
        }
        if (count == NUM_CHAIN_EBLOCKS)
        {
            count = freeCount;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = logicalBlockNum;
        }
        GetNumValidUsedPages(devID, logicalBlockNum, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount, &SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount);
    }
    #endif // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #endif  // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
}

//-----------------------------------------
void ClearEBlockMapFreeBitIndex(FTL_DEV devID, UINT16 logicalEBNum)
{
    UINT8  count = 0;                                  /*1*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("ClearEBlockMapFreeBitIndex: logicalEBNum = 0x%X\n", logicalEBNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    for (count = 0; count < EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE; count++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[count] = 0;
       #else
       EBlockMappingTable[devID][logicalEBNum].freeBitMap[count] = 0;
       #endif
    }
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if ((NUM_DATA_EBLOCKS > logicalEBNum) && (EMPTY_WORD != GetChainLogicalEBNum(devID, logicalEBNum)))
    {
        for (count = 0; count < (NUM_CHAIN_EBLOCKS); count++)
        {
            if (logicalEBNum == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
            {
                break;
            }
        }
        if (count != NUM_CHAIN_EBLOCKS)
        {
            // clear
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = EMPTY_WORD;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount = EMPTY_WORD;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount = EMPTY_WORD;
        }
    }
    #endif // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #endif  // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

}

//------------------------------------------
void MarkEBlockMappingTableSectorDirty(FTL_DEV devID, UINT16 sector)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
     // don't need
    #else
    SetBitMapField(&EBlockMappingTableDirtyBitMap[devID][0], sector, 1, DIRTY_BIT);
    #endif
}

//------------------------------------------
void MarkEBlockMappingTableEntryDirty(FTL_DEV devID, UINT16 logicalEBNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 sector = 0;                                 /*4*/
    UINT32 sector1 = 0;                                /*4*/
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)   
    if(FTL_FALSE == gCheckPfForNand){
    #endif
    CACHE_MarkEBDirty(devID, logicalEBNum);
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)    
    }
    #endif
    #else
    sector = ((((UINT32) logicalEBNum) * sizeof(EBLOCK_MAPPING_ENTRY)) )
       / FLUSH_RAM_TABLE_SIZE;
    MarkEBlockMappingTableSectorDirty(devID, (UINT16) sector);

    sector1 = ((((UINT32) logicalEBNum + 1) * sizeof(EBLOCK_MAPPING_ENTRY)) - 1)
       / FLUSH_RAM_TABLE_SIZE;
    if(sector != sector1)
    {
       MarkEBlockMappingTableSectorDirty(devID, (UINT16)sector1);
    }
    #endif
}

//-------------------------------------------
void MarkEBlockMappingTableSectorClean(FTL_DEV devID, UINT16 sector)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // Don't need it
    #else
    SetBitMapField(&EBlockMappingTableDirtyBitMap[devID][0], sector, 1, CLEAN_BIT);
    #endif
}

//-------------------------------------------
UINT8 IsEBlockMappingTableSectorDirty(FTL_DEV devID, UINT16 sector)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // Don't need it
    #else
    if(GetBitMapField(&EBlockMappingTableDirtyBitMap[devID][0], sector, 1) == DIRTY_BIT)
    #endif
    {
       return FTL_TRUE;
    }
    return FTL_FALSE;
}

/************ PPA Table ************/
//-------------------------------------
UINT16 GetPPASlot(FTL_DEV devID, UINT16 logicalEBNum, UINT16 logicalPageOffset)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    return (UINT16)((*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * logicalPageOffset))) & EMPTY_INVALID);
    #else
    return (UINT16)(PPAMappingTable[devID][logicalEBNum][logicalPageOffset] & EMPTY_INVALID);
    #endif
}

//-------------------------------------
void SetPPASlot(FTL_DEV devID, UINT16 logicalEBNum, UINT16 logicalPageOffset, UINT16 value)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * logicalPageOffset))) = (PPA_MAPPING_ENTRY)(value & EMPTY_INVALID);
    #else
    PPAMappingTable[devID][logicalEBNum][logicalPageOffset] = (PPA_MAPPING_ENTRY)(value & EMPTY_INVALID);
    #endif
    MarkPPAMappingTableEntryDirty(devID, logicalEBNum, logicalPageOffset);
}

//-------------------------------------
void MarkPPAMappingTableSectorDirty(FTL_DEV devID, UINT16 sector)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // don't need
    #else
    SetBitMapField(&PPAMappingTableDirtyBitMap[devID][0], sector, 1, DIRTY_BIT);
    #endif
}

//--------------------------------------
void MarkPPAMappingTableEntryDirty(FTL_DEV devID, UINT16 logicalEBNum, UINT16 logicalPageOffset)
{
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 sector = 0;                                 /*4*/
    #endif

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 sector1 = 0;                                /*4*/
    #endif
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    if(FTL_FALSE == gCheckPfForNand){ 
    #endif
    CACHE_MarkEBDirty(devID, logicalEBNum);
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    }
    #endif
    #else
    sector = (((((UINT32) logicalEBNum) * NUM_PAGES_PER_EBLOCK) + ((UINT32) logicalPageOffset)) * 
       sizeof(PPA_MAPPING_ENTRY)) / SECTOR_SIZE;
    MarkPPAMappingTableSectorDirty(devID, (UINT16) sector);

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    sector1 = (((((UINT32) logicalEBNum) * NUM_PAGES_PER_EBLOCK)) * 
       sizeof(PPA_MAPPING_ENTRY)) / FLUSH_RAM_TABLE_SIZE;
    if(sector != sector1)
    {
       MarkPPAMappingTableSectorDirty(devID, (UINT16) sector1);
    }
    sector1 = (((((UINT32) logicalEBNum) * NUM_PAGES_PER_EBLOCK) + (NUM_PAGES_PER_EBLOCK - 1)) * 
       sizeof(PPA_MAPPING_ENTRY)) / FLUSH_RAM_TABLE_SIZE;
    if(sector != sector1)
    {
       MarkPPAMappingTableSectorDirty(devID, (UINT16) sector1);
    }
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    #endif  // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
}

//----------------------------------------
void MarkPPAMappingTableSectorClean(FTL_DEV devID, UINT16 sector)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // Don't need it
    #else
    SetBitMapField(&PPAMappingTableDirtyBitMap[devID][0], sector, 1, CLEAN_BIT);
    #endif
}

//----------------------------------------
UINT8 IsPPAMappingTableSectorDirty(FTL_DEV devID, UINT16 sector)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // Don't need it
    #else
    if(GetBitMapField(&PPAMappingTableDirtyBitMap[devID][0], sector, 1) == DIRTY_BIT)
    #endif
    {
        return FTL_TRUE;
    }
    return FTL_FALSE;
}

/********* TRANSFER MAP ACCESS FUNCTIONS ************/

//--------------------------------------------------------------------
UINT8 GetTMDevID(UINT16 index)
{
    return transferMap[index].devID;
}

//--------------------------------------------------------------------
void SetTMDevID(UINT16 index, FTL_DEV devId)
{
    transferMap[index].devID = devId;
}

//--------------------------------------------------------------------
UINT8 GetTMNumSectors(UINT16 index)
{
    return transferMap[index].numSectors;
}

//--------------------------------------------------------------------
void SetTMNumSectors(UINT16 index, UINT8 sec)
{
    transferMap[index].numSectors = sec;
}

//--------------------------------------------------------------------
UINT32 GetTMStartLBA(UINT16 index)
{
    return transferMap[index].startLBA;
}

//--------------------------------------------------------------------
void SetTMStartLBA(UINT16 index, UINT32 lba)
{
    transferMap[index].startLBA = lba;
}

//--------------------------------------------------------------------
TRANS_MAP_ENTRY_PTR GetTMPointer(UINT16 index)
{
    return &transferMap[index];
}

//--------------------------------------------------------------------
UINT32 GetTMPhyPage(UINT16 index)
{
    return transferMap[index].phyPageAddr;
}

//--------------------------------------------------------------------
void SetTMPhyPage(UINT16 index, UINT32 phyPage)
{
    transferMap[index].phyPageAddr = phyPage;
}

//--------------------------------------------------------------------
UINT32 GetTMMergePage(UINT16 index)
{
    return transferMap[index].mergePageForWrite;
}

//--------------------------------------------------------------------
void SetTMMergePage(UINT16 index, UINT32 mergePage)
{
    transferMap[index].mergePageForWrite = mergePage;
}

//--------------------------------------------------------------------
UINT8 GetTMStartSector(UINT16 index)
{
    return transferMap[index].startSector;
}

//--------------------------------------------------------------------
void SetTMStartSector(UINT16 index, UINT8 sector)
{
    transferMap[index].startSector = sector;
}

void SetTMLogInfo(UINT16 index, UINT16 value)
{
    transferMap[index].logEBlockEntryIndex = value;
}

UINT16 GetTMLogInfo(UINT16 index)
{
    return transferMap[index].logEBlockEntryIndex;
}

/*******************************************************/
//-------------------------
void TABLE_ClearFreeBitMap(FTL_DEV devID, UINT16 eBlockNum)
{
    UINT16 bitMapCounter = EMPTY_WORD;                 /*2*/
    
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 count = 0;
    #endif

    CACHE_GetRAMOffsetEB(devID, eBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("TABLE_ClearFreeBitMap: eBlockNum = 0x%X\n", eBlockNum, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    for(bitMapCounter = 0; bitMapCounter < EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE; bitMapCounter++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freeBitMap[bitMapCounter] = 0; /*all pages are free*/
       #else
       EBlockMappingTable[devID][eBlockNum].freeBitMap[bitMapCounter] = 0; /*all pages are free*/
       #endif
    }
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if ((NUM_DATA_EBLOCKS > eBlockNum) && (EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockNum)))
    {
        for (count = 0; count < (NUM_CHAIN_EBLOCKS); count++)
        {
            if (eBlockNum == SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum)
            {
                break;
            }
        }
        if (count != NUM_CHAIN_EBLOCKS)
        {
            // clear
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].LogEBNum = EMPTY_WORD;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].UsedPageCount = EMPTY_WORD;
            SaveValidUsedPage[count + (devID * NUM_CHAIN_EBLOCKS)].ValidPageCount = EMPTY_WORD;
        }
    }
    #endif // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #endif  // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
}

//---------------------------
void TABLE_ClearMappingTable(FTL_DEV devID, UINT16 logicalEBNum, UINT16 phyEBAddr, UINT32 eraseCount)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount = 0;
    #else
    EBlockMappingTable[devID][logicalEBNum].dirtyCount = 0;
    #endif
    if(logicalEBNum >= SYSTEM_START_EBLOCK)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum = 1;
       #else
       EBlockMappingTable[devID][logicalEBNum].freePage_GCNum = 1; /*1 to account for the system info, only two eblocks will have it after a format*/
       #endif
    }    
    else
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).freePage_GCNum = 0;
       #else
       EBlockMappingTable[devID][logicalEBNum].freePage_GCNum = 0;          
       #endif
    }

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).phyEBAddr = phyEBAddr;
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).eraseCount = (UINT16)(eraseCount & ERASE_STATUS_CLEAR_WORD_MASK);
    if(ERASE_STATUS_GET_DWORD_MASK == (eraseCount & ERASE_STATUS_GET_DWORD_MASK))
    {
       (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).eraseCount |= ERASE_STATUS_GET_WORD_MASK;
    }
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount &= ERASE_COUNT_CLEAR_WORD_DIRTY_MASK;
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).dirtyCount |= ((eraseCount >> ERASE_COUNT_DWORD_DIRTY_SHIFT) & ERASE_COUNT_GET_WORD_DIRTY_MASK);
    #else
    EBlockMappingTable[devID][logicalEBNum].phyEBAddr = phyEBAddr;
    EBlockMappingTable[devID][logicalEBNum].eraseCount = (UINT16)(eraseCount & ERASE_STATUS_CLEAR_WORD_MASK);
    if(ERASE_STATUS_GET_DWORD_MASK == (eraseCount & ERASE_STATUS_GET_DWORD_MASK))
    {
       EBlockMappingTable[devID][logicalEBNum].eraseCount |= ERASE_STATUS_GET_WORD_MASK;
    }
    EBlockMappingTable[devID][logicalEBNum].dirtyCount &= ERASE_COUNT_CLEAR_WORD_DIRTY_MASK;
    EBlockMappingTable[devID][logicalEBNum].dirtyCount |= ((eraseCount >> ERASE_COUNT_DWORD_DIRTY_SHIFT) & ERASE_COUNT_GET_WORD_DIRTY_MASK);
    #endif

    TABLE_ClearFreeBitMap(devID, logicalEBNum);
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).chainToFrom = EMPTY_DWORD;
    #else
    EBlockMappingTable[devID][logicalEBNum].chainToFrom = EMPTY_DWORD;
    #endif
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    #endif
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    SetBadEBlockStatus(devID, logicalEBNum, FTL_FALSE);
    #endif
}

#if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
UINT8 GetBadEBlockStatus(FTL_DEV devID, UINT16 logicalEBNum)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    return (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).isBadBlock;
    #else
    return EBlockMappingTable[devID][logicalEBNum].isBadBlock;
    #endif
}

void SetBadEBlockStatus(FTL_DEV devID, UINT16 logicalEBNum, UINT8 badBlockStatus)
{
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr);
    (*(EBLOCK_MAPPING_ENTRY_PTR)(EBMramStructPtr)).isBadBlock = badBlockStatus;
    #else
    EBlockMappingTable[devID][logicalEBNum].isBadBlock = badBlockStatus;
    #endif
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    if (FTL_TRUE == badBlockStatus)
    {
       MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    }
    #else
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    #endif
}
#endif

//------------------------------
FTL_STATUS TABLE_InitMappingTable(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FTL_DEV devID = 0;                                 /*1*/
    UINT16 eBlockNum = 0;                              /*2*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT16 bitMapCounter = 0;                          /*2*/
    #endif

    #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE
    DBG_Printf("TABLE_InitMappingTable:\n", 0, 0);
    #endif  // #if ANNOUNCE_UPDATE_EBLOCK_MAPPING_TABLE

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    for(eBlockNum = NUM_DATA_EBLOCKS; eBlockNum < NUM_EBLOCKS_PER_DEVICE; eBlockNum++)
    {
       if(FTL_ERR_PASS != (status = CACHE_LoadEB(devID, eBlockNum, CACHE_INIT_TYPE)))
       {
          return status;
       }
    }
    #else
    for(eBlockNum = 0; eBlockNum < NUM_EBLOCKS_PER_DEVICE; eBlockNum++)
    {
       for(devID = 0; devID < NUM_DEVICES; devID++)
       {
          TABLE_ClearMappingTable(devID, eBlockNum, eBlockNum, ERASE_STATUS_GET_DWORD_MASK | 1);
       }
    }
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    for(devID = 0; devID < NUM_DEVICES; devID++)
    { 
       for(bitMapCounter = 0; bitMapCounter < EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE; bitMapCounter++)
       {          
          EBlockMappingTableDirtyBitMap[devID][bitMapCounter] = CLEAN_BIT;
       }
    }
    #endif
    return status;
}

//----------------------------
void TABLE_ClearPPATable(FTL_DEV devID, UINT16 eBlockNum)
{
    UINT16 pageCounter = 0;                            /*2*/
    
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;

    CACHE_GetRAMOffsetEB(devID, eBlockNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(pageCounter = 0; pageCounter < NUM_PAGES_PER_EBLOCK; pageCounter++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * pageCounter))) = EMPTY_INVALID;
       #else
       PPAMappingTable[devID][eBlockNum][pageCounter] = EMPTY_INVALID;
       #endif
    }
}

//-----------------------
FTL_STATUS TABLE_InitPPAMappingTable(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    FTL_DEV devID = 0;                                 /*1*/
    UINT16 eBlockNum = 0;                              /*2*/
    UINT16 pageCounter = 0;                            /*2*/
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // Don't need it

    #else
    for(eBlockNum = 0; eBlockNum < NUM_EBLOCKS_PER_DEVICE; eBlockNum++)
    {
       for(devID = 0; devID < NUM_DEVICES; devID++)
       {       
          TABLE_ClearPPATable(devID, eBlockNum);
       }
    }
    for(devID = 0; devID < NUM_DEVICES; devID++)
    {       
       for(pageCounter = 0; pageCounter < PPA_DIRTY_BITMAP_DEV_TABLE_SIZE; pageCounter++)
       {
          PPAMappingTableDirtyBitMap[devID][pageCounter] = CLEAN_BIT;
       }
    }
    #endif
    return status;
}

//---------------------------
FTL_STATUS TABLE_InitEBOrderingTable(FTL_DEV devID)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 eBlockNum = 0;                              /*2*/

    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {
       TransLogEBArray[devID][eBlockNum].key = EMPTY_DWORD;          
       TransLogEBArray[devID][eBlockNum].phyAddr = EMPTY_WORD;          
       TransLogEBArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
    } 
    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {
       FlushLogEBArray[devID][eBlockNum].key = EMPTY_DWORD;
       FlushLogEBArray[devID][eBlockNum].phyAddr = EMPTY_WORD;
       FlushLogEBArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       FlushLogEBArray[devID][eBlockNum].cacheNum = EMPTY_BYTE;
       #endif
    }
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    gCacheNum = 0;
    #endif
    SetTransLogEBCounter(devID, 0);
    SetFlushLogEBCounter(devID,0);
    FlushLogEBArrayCount[devID] = 0;
    TransLogEBArrayCount[devID] = 0;
    for(eBlockNum = 0; eBlockNum < NUMBER_OF_SYSTEM_EBLOCKS; eBlockNum++)
    {
       ReservedEBlock[devID][eBlockNum] = EMPTY_WORD;
    }
    ReservedEBlockNum[devID] = 0;
    FlushEBlockAdjustedFreePage[devID] = EMPTY_DWORD;
    return status;  
}  

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
//----------------------------------------
FTL_STATUS FTL_ClearA(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT8 count = 0;                                   /*1*/

    /*Fixed portion*/   
    TransLogEntry.entryA.LBA = EMPTY_DWORD;
    TransLogEntry.entryA.seqNum = 0;
    TransLogEntry.entryA.checkWord = EMPTY_WORD;
    TransLogEntry.entryA.type = TRANS_LOG_TYPE_A;    
    for(count = 0; count < NUM_ENTRIES_TYPE_A; count++)
    {
       TransLogEntry.entryA.pageLoc[count].phyEBOffset = EMPTY_WORD;
       TransLogEntry.entryA.pageLoc[count].logEBNum = EMPTY_WORD;    
    }
    return status;
}

//-------------------------------------------
FTL_STATUS FTL_ClearB(UINT16 count)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 count1 = 0;                                 /*1*/

    /*Fixed portion*/
    TransLogEntry.entryB[count].checkWord = EMPTY_WORD;
    TransLogEntry.entryB[count].type = TRANS_LOG_TYPE_B;
    TransLogEntry.entryB[count].seqNum = (UINT8)(1 + count);
    for(count1 = 0; count1 < NUM_ENTRIES_TYPE_B; count1++)
    {
       TransLogEntry.entryB[count].pageLoc[count1].phyEBOffset = EMPTY_WORD;
       TransLogEntry.entryB[count].pageLoc[count1].logEBNum = EMPTY_WORD;          
    }
    return status;
}

//-------------------------------------------------
FTL_STATUS FTL_ClearC(UINT16 seqNum)
{   
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 count = 0;                                  /*1*/

    TransLogEntry.entryC.checkWord = EMPTY_WORD;
    TransLogEntry.entryC.seqNum = (UINT8)(1 + seqNum);
    TransLogEntry.entryC.type = TRANS_LOG_TYPE_C;  
    TransLogEntry.entryC.GCNum = EMPTY_DWORD;
    for (count = 0; count < sizeof(TransLogEntry.entryC.reserved); count++)
    {
       TransLogEntry.entryC.reserved[count] = EMPTY_BYTE;
    }
    return status;
}

//---------------------------------------
FTL_STATUS TABLE_InitTransLogEntry(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT8 count = 0;                                   /*1*/

    if((status = FTL_ClearA()) != FTL_ERR_PASS)
    {
       return status;
    }
    for(count = 0; count < TEMP_B_ENTRIES; count++)
    {
       if((status = FTL_ClearB(count)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    if((status = FTL_ClearC(count+1)) != FTL_ERR_PASS)
    {
       return status;
    }

    return status;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

//-----------------------------
FTL_STATUS TABLE_FlushEBInsert(FTL_DEV devID, UINT16 logicalAddr, UINT16 phyEBAddr, UINT32 key)
{      
    UINT16 eBlockNum = 0;                              /*2*/

    if(FlushLogEBArrayCount[devID] >= NUM_FLUSH_LOG_EBLOCKS)
    {
       return FTL_ERR_FLUSH_NO_ENTRIES;
    }
    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {  
       if(FlushLogEBArray[devID][eBlockNum].key == EMPTY_DWORD)
       {
          /*just put it here*/
          FlushLogEBArray[devID][eBlockNum].key = key;
          FlushLogEBArray[devID][eBlockNum].phyAddr = phyEBAddr;
          FlushLogEBArray[devID][eBlockNum].logicalEBNum = logicalAddr;
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          FlushLogEBArray[devID][eBlockNum].cacheNum = gCacheNum;
          gCacheNum++;
          #endif
          FlushLogEBArrayCount[devID]++;
          TABLE_SortFlushTable(devID);
          return FTL_ERR_PASS;
       }
    }  
    return FTL_ERR_FLUSH_NO_ROOM;
}

//-----------------------------
FTL_STATUS TABLE_FlushEBRemove(FTL_DEV devID, UINT16 blockNum)
{
    UINT16 blockCount = 0;                             /*2*/
    UINT16 FlushEBCount = 0;                           /*2*/

    if(blockNum >= FlushLogEBArrayCount[devID])
    {
        return FTL_ERR_FLUSH_NO_EBLOCK;
    }
    FlushEBCount = FlushLogEBArrayCount[devID];
    for(blockCount = blockNum; blockCount < FlushEBCount; blockCount++)
    {
       FlushLogEBArray[devID][blockCount].key = EMPTY_DWORD;
       FlushLogEBArray[devID][blockCount].phyAddr = EMPTY_WORD;
       FlushLogEBArray[devID][blockCount].logicalEBNum = EMPTY_WORD;
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       FlushLogEBArray[devID][blockCount].cacheNum = EMPTY_BYTE;
       gCacheNum--;
       #endif
       FlushLogEBArrayCount[devID]--;
    }
    TABLE_SortFlushTable(devID);
    return FTL_ERR_PASS;
}

//-----------------------------
FTL_STATUS TABLE_FlushEBGetNext(FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT16_PTR phyEBAddrPtr, UINT32_PTR keyPtr)
{
    UINT16 eBlockNum = 0;                              /*2*/
    UINT32 smallestKey = EMPTY_DWORD;                  /*4*/
    UINT16 smallestKeyEB = EMPTY_WORD;                 /*2*/

    if((FlushLogEBArrayCount[devID] > NUM_FLUSH_LOG_EBLOCKS) || (FlushLogEBArrayCount[devID] == 0))
    {
       return FTL_ERR_FLUSH_NO_EBLOCKS;
    }
    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {
       if(FlushLogEBArray[devID][eBlockNum].key < smallestKey)
       {
          smallestKeyEB = eBlockNum;
          smallestKey = FlushLogEBArray[devID][eBlockNum].key;
       }
    }
    if(keyPtr != NULL)
    {
       *keyPtr = FlushLogEBArray[devID][smallestKeyEB].key;
    }
    if(phyEBAddrPtr != NULL)
    {
       *phyEBAddrPtr = FlushLogEBArray[devID][smallestKeyEB].phyAddr;
    }
    if(logicalAddrPtr != NULL)
    {
       *logicalAddrPtr = FlushLogEBArray[devID][smallestKeyEB].logicalEBNum;
    }
    FlushLogEBArray[devID][smallestKeyEB].key = EMPTY_DWORD;
    FlushLogEBArray[devID][smallestKeyEB].phyAddr = EMPTY_WORD;
    FlushLogEBArray[devID][smallestKeyEB].logicalEBNum = EMPTY_WORD;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    FlushLogEBArray[devID][smallestKeyEB].cacheNum = EMPTY_BYTE;
    gCacheNum--;
    #endif
    FlushLogEBArrayCount[devID]--;    
    TABLE_SortFlushTable(devID);
    return FTL_ERR_PASS;
}

//-----------------------
FTL_STATUS TABLE_FlushEBClear(FTL_DEV devID)
{
    UINT16 eBlockNum = 0;                              /*2*/

    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {
       FlushLogEBArray[devID][eBlockNum].key = EMPTY_DWORD;
       FlushLogEBArray[devID][eBlockNum].phyAddr = EMPTY_WORD;
       FlushLogEBArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       FlushLogEBArray[devID][eBlockNum].cacheNum = EMPTY_BYTE;
       #endif
    }
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    gCacheNum = 0;
    #endif
    FlushLogEBArrayCount[devID] = 0;
    return FTL_ERR_PASS;
}

//---------------------------------
void TABLE_SortFlushTable(FTL_DEV devID)  /*  1*/
{
    #if(NUM_FLUSH_LOG_EBLOCKS > 1)
    UINT32 temp32 = 0;                                 /*4*/
    UINT16 temp16 = 0;                                 /*2*/
    UINT16 loop = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    UINT16 last = 0;                                   /*2*/
    UINT16 end = 0;                                    /*2*/
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT8 temp8 = 0;                                   /*1*/
    #endif

    last = NUM_FLUSH_LOG_EBLOCKS - 1;
    for (loop = 0; loop < (NUM_FLUSH_LOG_EBLOCKS - 1); loop++)
    {
       end = last;
       last = 0;
       for (count = 0; count < end; count++)
       {
          if (FlushLogEBArray[devID][count].key > FlushLogEBArray[devID][count+1].key)
          {
             temp32 = FlushLogEBArray[devID][count].key;
             FlushLogEBArray[devID][count].key = FlushLogEBArray[devID][count+1].key;
             FlushLogEBArray[devID][count+1].key = temp32;
             temp16 = FlushLogEBArray[devID][count].phyAddr;
             FlushLogEBArray[devID][count].phyAddr = FlushLogEBArray[devID][count+1].phyAddr;
             FlushLogEBArray[devID][count+1].phyAddr = temp16;
             temp16 = FlushLogEBArray[devID][count].logicalEBNum;
             FlushLogEBArray[devID][count].logicalEBNum = FlushLogEBArray[devID][count+1].logicalEBNum;
             FlushLogEBArray[devID][count+1].logicalEBNum = temp16;
             #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
             temp8 = FlushLogEBArray[devID][count].cacheNum;
             FlushLogEBArray[devID][count].cacheNum = FlushLogEBArray[devID][count+1].cacheNum;
             FlushLogEBArray[devID][count+1].cacheNum = temp8;
             #endif
             last = count;
          }
       }
       if (0 == last)
       {
          break;
       }
    }
    #endif  // #if(NUM_FLUSH_LOG_EBLOCKS > 1)
}

//--------------------------------------
FTL_STATUS TABLE_FlushEBGetLatest(FTL_DEV devID, UINT16_PTR flushEBlockPtr,
    UINT16_PTR phyEBAddrPtr, UINT32 key)
{
    UINT16 block = 0;                                  /*2*/

    *flushEBlockPtr = EMPTY_WORD;
    for (block = 0; block < NUM_FLUSH_LOG_EBLOCKS; block++)
    {
       if (FlushLogEBArray[devID][block].key == key)
       {
          *flushEBlockPtr = FlushLogEBArray[devID][block].logicalEBNum;
          *phyEBAddrPtr = FlushLogEBArray[devID][block].phyAddr;
          break;
       }
    }
    if (EMPTY_WORD == *flushEBlockPtr)
    {
       // No Flush EBlocks at all - Sanity Error
       return FTL_ERR_FLUSH_GET_LATEST;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS TABLE_GetFlushLogEntry(FTL_DEV devID, UINT16 blockNum, UINT16_PTR logicalEBNumPtr, 
    UINT16_PTR phyAddrPtr, UINT32_PTR keyPtr)
{
    if(logicalEBNumPtr != NULL)
    {
       *logicalEBNumPtr = FlushLogEBArray[devID][blockNum].logicalEBNum;
    }
    if(phyAddrPtr != NULL)
    {
       *phyAddrPtr = FlushLogEBArray[devID][blockNum].phyAddr;
    }
    if(keyPtr != NULL)
    {
       *keyPtr = FlushLogEBArray[devID][blockNum].key;
    }
    return FTL_ERR_PASS;
}

//-------------------------------------
FTL_STATUS TABLE_SetFlushLogEntry(FTL_DEV devID, UINT16 blockNum, UINT16 logicalEBNum, UINT16 phyAddr, UINT32 key)
{
    if(blockNum >= NUM_FLUSH_LOG_EBLOCKS)
    {
       return FTL_ERR_FLUSH_LOG_ENTRY;
    }
    FlushLogEBArray[devID][blockNum].logicalEBNum = logicalEBNum;
    FlushLogEBArray[devID][blockNum].phyAddr      = phyAddr;
    FlushLogEBArray[devID][blockNum].key          = key;
    return FTL_ERR_PASS;
}


#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
#if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
FTL_STATUS CACHE_DynamicAllocation(UINT32 totalRamAllowed)
{
    UINT16 i = 0;

    UINT32 tempUsedSize = 0;
    UINT32 tempCount = 0;
    UINT32 totalBdRamSize = 0;


    if (FTL_FALSE == gCheckFirstMalloc)
    {
        // first time
       gCheckFirstMalloc = FTL_TRUE;
    }else{
       if (gSaveTotalRamAllowed == totalRamAllowed)
       {
          // skip
          return FTL_ERR_PASS;
       }
       // free
       if (EBlockMappingCache[0])
       {
          MEM_Free(EBlockMappingCache[0]);
          if (EBlockMappingCache)
          {
             MEM_Free(EBlockMappingCache);
          }
       }
       if (PPAMappingCache[0])
       {
          MEM_Free(PPAMappingCache[0]);
          if (PPAMappingCache)
          {
             MEM_Free(PPAMappingCache);
          }
       }
       if (EBMCacheIndex)
       {
          MEM_Free(EBMCacheIndex);
       }
    }
    gSaveTotalRamAllowed = totalRamAllowed;

    if (0x0 != totalRamAllowed && EMPTY_DWORD != totalRamAllowed)
    {
       tempUsedSize = (totalRamAllowed - TEMP_BD_RAM_SIZE);
       tempCount = (tempUsedSize / ((FLUSH_RAM_TABLE_SIZE * (1/*EBlock*/ + PPA_CACHE_TABLE_OFFSET)) + 2/*ebm entry one size*/));

       // Create EBlockMappingCache and PPAMappingCache
       if(((MAX_PPA_MAP_INDEX * FLUSH_RAM_TABLE_SIZE * NUMBER_OF_DEVICES) + (MAX_PPA_MAP_INDEX * FLUSH_RAM_TABLE_SIZE * NUMBER_OF_DEVICES) + (NUMBER_OF_ERASE_BLOCKS * NUMBER_OF_DEVICES * 2/*ebm entry one size*/)) < tempUsedSize)
       {
          numBlockMapIndex = MAX_EBLOCK_MAP_INDEX;
          numPpaMapIndex    = (numBlockMapIndex * PPA_CACHE_TABLE_OFFSET);
       }else{
          numBlockMapIndex = (UINT16)tempCount;
          numPpaMapIndex = (UINT16)(tempCount * PPA_CACHE_TABLE_OFFSET);
       }

       // Set possible swap area of EBMCacheIndex
       if (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) % FLUSH_RAM_TABLE_SIZE) == 0)
       {
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          if (numBlockMapIndex > ((((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE)) + SAVE_DATA_EB_INDEX))
          {
             cacheIndexChangeArea = (numBlockMapIndex - ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE));
          }
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if (numBlockMapIndex > (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE+ CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1 + SAVE_DATA_EB_INDEX))
          {
             cacheIndexChangeArea = (numBlockMapIndex - (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1));
          }
#endif
       }else{
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          if (numBlockMapIndex > (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1 + SAVE_DATA_EB_INDEX))
          {
             cacheIndexChangeArea = numBlockMapIndex - (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1);
          }
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if (numBlockMapIndex > (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE+ CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 2 + SAVE_DATA_EB_INDEX))
          {
             cacheIndexChangeArea = (numBlockMapIndex - (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 2));
          }
#endif
        }
    }else if (0x0 == totalRamAllowed){ // Minimum Setting
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
       {
          numBlockMapIndex = ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + SAVE_DATA_EB_INDEX);
          cacheIndexChangeArea = (numBlockMapIndex - (NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE));
       }else{
          numBlockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1) + SAVE_DATA_EB_INDEX);
          cacheIndexChangeArea = (numBlockMapIndex - ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1));
       }
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       if ((NUMBER_OF_ERASE_BLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
       {
          if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
          {
             numBlockMapIndex = ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + SAVE_DATA_EB_INDEX);
          }else{
             numBlockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1) + SAVE_DATA_EB_INDEX);
          }
       }else{

          if (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) % FLUSH_RAM_TABLE_SIZE) == 0)
          {
             numBlockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE)) + 1 + SAVE_DATA_EB_INDEX);
          }else{
             numBlockMapIndex = (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 2) + SAVE_DATA_EB_INDEX);
          }
        }
        cacheIndexChangeArea = 1;
#endif
        numPpaMapIndex = (numBlockMapIndex * PPA_CACHE_TABLE_OFFSET);
    }else if (EMPTY_DWORD == totalRamAllowed){ // Maximum Setting
       numBlockMapIndex = (MAX_EBLOCK_MAP_INDEX);
       numPpaMapIndex    = (MAX_PPA_MAP_INDEX);
       if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
       {
          cacheIndexChangeArea = (numBlockMapIndex - (NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE));
       }else{
          cacheIndexChangeArea = (numBlockMapIndex - ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1));
       }
    }

    #if (CROSS_CASE_ON == FTL_FALSE)
    if(cacheIndexChangeArea < 2)
    {
       thesholdDirtyCount = 1;
    }else{
       thesholdDirtyCount = (cacheIndexChangeArea - 1);
    }
    #else
    if(cacheIndexChangeArea < 3)
    {
       thesholdDirtyCount =  1;
    }else{
       thesholdDirtyCount = (cacheIndexChangeArea - 2);
    }
    #endif
    if (thesholdDirtyCount == 0)
    {
       DBG_Printf("THESHOLD_DIRTY_COUNT is setting 0\n", 0, 0);
       return FTL_ERR_CACHE_ALLOCTION_ARRAY;
    }

    // Check ram size
    ebmCacheIndexSize = (2/*UINT16 size*/ * numBlockMapIndex * NUMBER_OF_DEVICES);
    eblockMappingCacheSize = (1/*UINT8 size*/ * numBlockMapIndex * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE);
    ppaMappingCacheSize = (1/*UINT8 size*/ *  numPpaMapIndex * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE);

    totalBdRamSize = (TEMP_BD_RAM_SIZE + ebmCacheIndexSize + eblockMappingCacheSize + ppaMappingCacheSize);

    if (0x0 != totalRamAllowed && EMPTY_DWORD != totalRamAllowed)
    {
       if(totalRamAllowed < totalBdRamSize || cacheIndexChangeArea == 0)
       {
          DBG_Printf("totalRamAllowed is less than minimum TOTAL_BD_RAM_SIZE\n", 0, 0);
          return FTL_ERR_CACHE_ALLOCTION_ARRAY;
       }
    }

    // EBlockMappingCache
    EBlockMappingCache = (UINT8_PTR_PTR)MEM_Malloc(sizeof(UINT8_PTR) * (numBlockMapIndex * NUMBER_OF_DEVICES));
    if (NULL == EBlockMappingCache)
    {
       DBG_Printf("EBlockMappingCache Malloc Error\n", 0, 0);
       return FTL_ERR_CACHE_ALLOCTION_ARRAY;
    }
    EBlockMappingCache[0] = (UINT8_PTR)MEM_Malloc(sizeof(UINT8) * FLUSH_RAM_TABLE_SIZE * (numBlockMapIndex * NUMBER_OF_DEVICES));
    if (NULL == EBlockMappingCache[0])
    {
       DBG_Printf("EBlockMappingCache Malloc Error\n", 0, 0);
       return FTL_ERR_CACHE_ALLOCTION_ARRAY;
    }
    for (i = 1; i < (numBlockMapIndex * NUMBER_OF_DEVICES); i++)
    {
       EBlockMappingCache[i] = EBlockMappingCache[0] + (i * FLUSH_RAM_TABLE_SIZE);
    }

    // PPAMappingCache
    PPAMappingCache = (UINT8_PTR_PTR)MEM_Malloc(sizeof(UINT8_PTR) * (numPpaMapIndex * NUMBER_OF_DEVICES));
    if (NULL == PPAMappingCache)
    {
       DBG_Printf("PPAMappingCache Malloc Error\n", 0, 0);
       return FTL_ERR_CACHE_ALLOCTION_ARRAY;
    }
    PPAMappingCache[0] = (UINT8_PTR)MEM_Malloc(sizeof(UINT8) * FLUSH_RAM_TABLE_SIZE * (numPpaMapIndex * NUMBER_OF_DEVICES));
    if (NULL == PPAMappingCache[0])
    {
       DBG_Printf("PPAMappingCache Malloc Error\n", 0, 0);
       return FTL_ERR_CACHE_ALLOCTION_ARRAY;
    }
    for (i = 1; i < (numPpaMapIndex * NUMBER_OF_DEVICES); i++)
    {
       PPAMappingCache[i] = PPAMappingCache[0] + (i * FLUSH_RAM_TABLE_SIZE);
    }

    // EBMCacheIndex
    EBMCacheIndex = (UINT16_PTR)MEM_Malloc(sizeof(UINT16) * (numBlockMapIndex * NUMBER_OF_DEVICES));
    if (NULL == EBMCacheIndex)
    {
       DBG_Printf("EBMCacheIndex Malloc Error\n", 0, 0);
       return FTL_ERR_CACHE_ALLOCTION_ARRAY;
    }

    return FTL_ERR_PASS;
}
#endif // #if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)

FTL_STATUS TABLE_GetFlushLogCacheEntry(FTL_DEV devID, UINT16 phyAddr, UINT8_PTR cacheNum_ptr)
{
    UINT16 count = 0;
    
    for (count = 0; count < NUM_FLUSH_LOG_EBLOCKS; count++)
    {
       if (phyAddr == FlushLogEBArray[devID][count].phyAddr)
       {
          *cacheNum_ptr = FlushLogEBArray[devID][count].cacheNum;
          return FTL_ERR_PASS;
       }
    }
    return FTL_ERR_FAIL;
}
FTL_STATUS TABLE_GetFlushLogPhyEntry(FTL_DEV devID, UINT8 cacheNum, UINT16_PTR phyAddr_ptr)
{
    UINT16 count = 0;

    for (count = 0; count < NUM_FLUSH_LOG_EBLOCKS; count++)
    {
       if (cacheNum == FlushLogEBArray[devID][count].cacheNum)
       {
          *phyAddr_ptr = FlushLogEBArray[devID][count].phyAddr;
          return FTL_ERR_PASS;
       }
    }
    return FTL_ERR_FAIL;
}
FTL_STATUS TABLE_GetFlushLogCountEntry(FTL_DEV devID, UINT8 cacheNum, UINT16_PTR blockNum_ptr)
{
    UINT16 count = 0;

    for (count = 0; count < NUM_FLUSH_LOG_EBLOCKS; count++)
    {
       if (cacheNum == FlushLogEBArray[devID][count].cacheNum)
       {
          *blockNum_ptr = count;
          return FTL_ERR_PASS;
       }
    }
    return FTL_ERR_FAIL;
}

#if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
FTL_STATUS ClearSaveStaticWL(FTL_DEV devID, UINT16 logicalEBNum, UINT32 eraseCount, UINT8 hlFlag)
{
    if (CACHE_WL_HIGH == hlFlag)
    {
       SaveStaticWL[devID].HighestLogEBNum = logicalEBNum;
       SaveStaticWL[devID].HighestCount = eraseCount;
    }
    else if (CACHE_WL_LOW == hlFlag){
       SaveStaticWL[devID].LowestLogEBNum = logicalEBNum;
       SaveStaticWL[devID].LowestCount = eraseCount;
    }
    return FTL_ERR_PASS;
}
FTL_STATUS SetSaveStaticWL(FTL_DEV devID, UINT16 logicalEBNum, UINT32 eraseCount)
{
    if (SaveStaticWL[devID].HighestCount < eraseCount)
    {
       SaveStaticWL[devID].HighestLogEBNum = logicalEBNum;
       SaveStaticWL[devID].HighestCount = eraseCount;
    }
    if (SaveStaticWL[devID].LowestCount > eraseCount)
    {
       SaveStaticWL[devID].LowestLogEBNum = logicalEBNum;
       SaveStaticWL[devID].LowestCount = eraseCount;
    }
    return FTL_ERR_PASS;
}

FTL_STATUS GetSaveStaticWL(FTL_DEV devID, UINT16_PTR logicalEBNum, UINT32_PTR eraseCount, UINT8 hlFlag)
{
    if (CACHE_WL_HIGH == hlFlag)
    {
        *logicalEBNum = SaveStaticWL[devID].HighestLogEBNum;
        *eraseCount = SaveStaticWL[devID].HighestCount;
    }else if (CACHE_WL_LOW == hlFlag){
        *logicalEBNum = SaveStaticWL[devID].LowestLogEBNum;
        *eraseCount = SaveStaticWL[devID].LowestCount;
    }

    return FTL_ERR_PASS;
}

#endif // #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)

#endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

//-------------------------------------
FTL_STATUS TABLE_TransLogEBInsert(FTL_DEV devID, UINT16 logicalAddr, UINT16 phyEBAddr, UINT32 key)
{      
    UINT16 eBlockNum = 0;                              /*2*/

    if(TransLogEBArrayCount[devID] >= NUM_TRANSACTION_LOG_EBLOCKS)
    {
       return FTL_ERR_LOG_INSERT;
    }
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {  
       if(TransLogEBArray[devID][eBlockNum].key == EMPTY_DWORD)
       {
          /*just put it here*/
          TransLogEBArray[devID][eBlockNum].key = key;
          TransLogEBArray[devID][eBlockNum].phyAddr = phyEBAddr;
          TransLogEBArray[devID][eBlockNum].logicalEBNum = logicalAddr;          
          TransLogEBArrayCount[devID]++;
          TABLE_SortTransTable(devID);
          return FTL_ERR_PASS;
       }
    }  
    return FTL_ERR_LOG_NO_ROOM;
}

//--------------------------------
FTL_STATUS TABLE_TransLogEBGetNext(FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT16_PTR phyEBAddrPtr, UINT32_PTR keyPtr)
{
    UINT16 eBlockNum = 0;                              /*2*/
    UINT32 smallestKey = EMPTY_DWORD;                  /*4*/
    UINT16 smallestKeyEB = EMPTY_WORD;                 /*2*/

    if((TransLogEBArrayCount[devID] > NUM_TRANSACTION_LOG_EBLOCKS) || (TransLogEBArrayCount[devID] == 0))
    {
        return FTL_ERR_LOG_NO_EBLOCKS;
    }
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {
       if(TransLogEBArray[devID][eBlockNum].key < smallestKey)
       {
          smallestKeyEB = eBlockNum;
          smallestKey = TransLogEBArray[devID][eBlockNum].key;
       }
    }
    if(keyPtr != NULL)
    {
       *keyPtr = TransLogEBArray[devID][smallestKeyEB].key;
    }
    if(phyEBAddrPtr != NULL)
    {
       *phyEBAddrPtr = TransLogEBArray[devID][smallestKeyEB].phyAddr;
    }
    if(logicalAddrPtr != NULL)
    {
       *logicalAddrPtr = TransLogEBArray[devID][smallestKeyEB].logicalEBNum;
    }

    #if(NUM_TRANSACTION_LOG_EBLOCKS > 1)
    for(eBlockNum = smallestKeyEB; eBlockNum < (NUM_TRANSACTION_LOG_EBLOCKS-1); eBlockNum++)
    {
       TransLogEBArray[devID][eBlockNum].key = TransLogEBArray[devID][eBlockNum+1].key;
       TransLogEBArray[devID][eBlockNum].phyAddr = TransLogEBArray[devID][eBlockNum+1].phyAddr;
       TransLogEBArray[devID][eBlockNum].logicalEBNum = TransLogEBArray[devID][eBlockNum+1].logicalEBNum;
    }

    #else  // #if(NUM_TRANSACTION_LOG_EBLOCKS > 1)
    eBlockNum = 0;
    #endif  // #else  // #if(NUM_TRANSACTION_LOG_EBLOCKS > 1)

    TransLogEBArray[devID][eBlockNum].key = EMPTY_DWORD;
    TransLogEBArray[devID][eBlockNum].phyAddr = EMPTY_WORD;
    TransLogEBArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
    TransLogEBArrayCount[devID]--;
    TABLE_SortTransTable(devID);
    return FTL_ERR_PASS;
}

//------------------------------------------
FTL_STATUS TABLE_TransLogEBGetLatest(FTL_DEV devID, UINT16_PTR logEBlockPtr,
    UINT16_PTR phyEBAddrPtr, UINT32 key) 
{
    UINT16 block = 0;                                  /*2*/

    *logEBlockPtr = EMPTY_WORD;
    for(block = 0; block < NUM_TRANSACTION_LOG_EBLOCKS; block++)
    {
       if(TransLogEBArray[devID][block].key == key)
       {
          *logEBlockPtr = TransLogEBArray[devID][block].logicalEBNum;
          *phyEBAddrPtr = TransLogEBArray[devID][block].phyAddr;
          break;
       }
    }
    if(EMPTY_WORD == *logEBlockPtr)
    {
       return FTL_ERR_LOG_GET_LATEST;
    }
    return FTL_ERR_PASS;
}

//-----------------------------------
FTL_STATUS TABLE_GetTransLogEntry(FTL_DEV devID, UINT16 blockNum, UINT16_PTR logEBlockPtr, UINT16_PTR phyAddrPtr, UINT32_PTR keyPtr)
{
    if((TransLogEBArrayCount[devID] > NUM_TRANSACTION_LOG_EBLOCKS) || (TransLogEBArrayCount[devID] == 0))
    {
       return FTL_ERR_LOG_GET_ENTRY;
    }
    if(logEBlockPtr != NULL)
    {
       *logEBlockPtr = TransLogEBArray[devID][blockNum].logicalEBNum;
    }
    if(phyAddrPtr != NULL)
    {
       *phyAddrPtr   = TransLogEBArray[devID][blockNum].phyAddr;
    }
    if(keyPtr != NULL)
    {
       *keyPtr       = TransLogEBArray[devID][blockNum].key;
    }
    return FTL_ERR_PASS;
}

//-----------------------
FTL_STATUS TABLE_TransEBClear(FTL_DEV devID)
{
    UINT16 eBlockNum = 0;                              /*2*/

    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {
       TransLogEBArray[devID][eBlockNum].key = EMPTY_DWORD;
       TransLogEBArray[devID][eBlockNum].phyAddr = EMPTY_WORD;
       TransLogEBArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
    }
    TransLogEBArrayCount[devID] = 0;
    return FTL_ERR_PASS;
}

//-------------------------------------
FTL_STATUS TABLE_TransLogEBRemove(FTL_DEV devID, UINT16 blockNum)
{
    UINT16 blockCount = 0;                             /*2*/

    if(blockNum >= TransLogEBArrayCount[devID])
    {
        return FTL_ERR_LOG_EB_REMOVE;
    }

    for(blockCount = 0; blockCount <= blockNum; blockCount++)
    {
       TransLogEBArray[devID][blockCount].key = EMPTY_DWORD;
       TransLogEBArray[devID][blockCount].phyAddr = EMPTY_WORD;
       TransLogEBArray[devID][blockCount].logicalEBNum = EMPTY_WORD;
       TransLogEBArrayCount[devID]--;
    }
    TABLE_SortTransTable(devID);
    return FTL_ERR_PASS;
}

//---------------------------------
void TABLE_SortTransTable(FTL_DEV devID)  /*  1*/
{
    #if(NUM_TRANSACTION_LOG_EBLOCKS > 1)
    UINT32 temp32 = 0;                                 /*4*/
    UINT16 temp16 = 0;                                 /*2*/
    UINT16 loop = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    UINT16 last = 0;                                   /*2*/
    UINT16 end = 0;                                    /*2*/

    last = NUM_TRANSACTION_LOG_EBLOCKS - 1;
    for (loop = 0; loop < (NUM_TRANSACTION_LOG_EBLOCKS - 1); loop++)
    {
       end = last;
       last = 0;
       for (count = 0; count < end; count++)
       {
          if (TransLogEBArray[devID][count].key > TransLogEBArray[devID][count+1].key)
          {
             temp32 = TransLogEBArray[devID][count].key;
             TransLogEBArray[devID][count].key = TransLogEBArray[devID][count+1].key;
             TransLogEBArray[devID][count+1].key = temp32;
             temp16 = TransLogEBArray[devID][count].phyAddr;
             TransLogEBArray[devID][count].phyAddr = TransLogEBArray[devID][count+1].phyAddr;
             TransLogEBArray[devID][count+1].phyAddr = temp16;
             temp16 = TransLogEBArray[devID][count].logicalEBNum;
             TransLogEBArray[devID][count].logicalEBNum = TransLogEBArray[devID][count+1].logicalEBNum;
             TransLogEBArray[devID][count+1].logicalEBNum = temp16;
             last = count;
          }
       }
       if (0 == last)
       {
          break;
       }
    }
    #endif  // #if(NUM_TRANSACTION_LOG_EBLOCKS > 1)
}

//---------------------------------
FTL_STATUS FTL_FindEmptyTransLogEBlock(FTL_DEV devID, UINT16_PTR logicalEBNumPtr, UINT16_PTR physicalEBNumPtr)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    if((status = TABLE_GetReservedEB(devID, logicalEBNumPtr, FTL_FALSE)) != FTL_ERR_PASS)
    {
       return status;
    }
    *physicalEBNumPtr = GetPhysicalEBlockAddr(devID, *logicalEBNumPtr);
    return FTL_ERR_PASS;
}

//-----------------------------------------------
FTL_STATUS TABLE_SetTransLogEntry(FTL_DEV devID, UINT16 blockNum, UINT16 logEBlock, UINT16 phyAddr, UINT32 key)
{
    if((TransLogEBArrayCount[devID] > NUM_TRANSACTION_LOG_EBLOCKS) || (TransLogEBArrayCount[devID] == 0))
    {
       return FTL_ERR_LOG_SET_ENTRY;
    }
    if(blockNum >= NUM_TRANSACTION_LOG_EBLOCKS)
    {
       return FTL_ERR_LOG_BLK_OUT_OF_RANGE;
    }
    TransLogEBArray[devID][blockNum].logicalEBNum = logEBlock;
    TransLogEBArray[devID][blockNum].phyAddr = phyAddr;
    TransLogEBArray[devID][blockNum].key = key;
    return FTL_ERR_PASS;
}



#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
//---------------------------------
FTL_STATUS FTL_FindEmptySuperSysEBlock(FTL_DEV devID, UINT16_PTR logicalEBNumPtr, UINT16_PTR physicalEBNumPtr)
{

    UINT16 logicalEBNum = 0;                           /*2*/
    UINT16 eblockCount = 0;                            /*2*/
    UINT32 eraseCount = 0;                             /*2*/
    UINT16 ReservedEBNum = 0;                          /*2*/
    UINT32 lowestErase = EMPTY_DWORD;                   /*2*/
    UINT16 lowestIndex = EMPTY_WORD;                   /*2*/

    ReservedEBNum = (UINT16)(ReservedEBlockNum[devID] & EMPTY_WORD);
    if(ReservedEBNum == 0)
    {
       return FTL_ERR_SUPER_NO_SYS_EBLOCK;
    }

    for(eblockCount = 0; eblockCount < ReservedEBNum; eblockCount++)
    {
       logicalEBNum = ReservedEBlock[devID][eblockCount];
       eraseCount = GetTrueEraseCount(devID, logicalEBNum);
       if((eraseCount < lowestErase) && (SUPER_SYS_START_EBLOCKS <= GetPhysicalEBlockAddr(devID,ReservedEBlock[devID][eblockCount])))
       {
          lowestErase = eraseCount;
          lowestIndex = eblockCount;
       }
    }

    if((EMPTY_DWORD == lowestErase) || (EMPTY_WORD == lowestIndex))
    {
       #if DEBUG_SUPER_SYS
       DBG_Printf("Fail : FTL_FindEmptySuperSysEBlock\n", 0, 0);
       #endif
       return FTL_ERR_SUPER_EB_NO_ENTRY;
    }

    *logicalEBNumPtr = ReservedEBlock[devID][lowestIndex];
    *physicalEBNumPtr = GetPhysicalEBlockAddr(devID,ReservedEBlock[devID][lowestIndex]);
    for(eblockCount = lowestIndex; eblockCount < (ReservedEBNum-1); eblockCount++)
    {
       ReservedEBlock[devID][eblockCount] = ReservedEBlock[devID][eblockCount+1];
    }
    ReservedEBlock[devID][ReservedEBNum-1] = EMPTY_WORD;
    ReservedEBlockNum[devID]--;
    return FTL_ERR_PASS;
}


FTL_STATUS FTL_FindAllAreaSuperSysEBlock(FTL_DEV devID, UINT16_PTR findDataEBNumPtr, UINT16_PTR findSystemEBNumPtr)
{

    UINT16 sysCount = 0;
    UINT16 dataCount = 0;
    UINT16 eBlockNum = 0;
    #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    UINT16 chainEB = 0;
    #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    FTL_STATUS status = FTL_ERR_PASS;
    #endif
    

    // find System Area
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {  
       if(EMPTY_WORD == TransLogEBArray[devID][eBlockNum].phyAddr)
       {
          continue;
       }
       if(SUPER_SYS_START_EBLOCKS <= TransLogEBArray[devID][eBlockNum].phyAddr)
       {
          sysCount++;
       }
    }
    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {
       if(EMPTY_WORD == FlushLogEBArray[devID][eBlockNum].phyAddr)
       {
          continue;
       }
       if(SUPER_SYS_START_EBLOCKS <= FlushLogEBArray[devID][eBlockNum].phyAddr)
       {
          sysCount++;
       }
    }
    // find Data Area
    for(eBlockNum = 0; eBlockNum < NUMBER_OF_DATA_EBLOCKS; eBlockNum++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       if(FTL_ERR_PASS != (status = CACHE_LoadEB(devID, eBlockNum , CACHE_WRITE_TYPE)))
       {
          return status;
       }
       #endif
       if(SUPER_SYS_START_EBLOCKS <= GetPhysicalEBlockAddr(devID,eBlockNum))
       {
          dataCount++;
       }
       #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
       chainEB = GetChainLogicalEBNum(devID, eBlockNum);
       if (EMPTY_WORD != chainEB)
       {
          if(SUPER_SYS_START_EBLOCKS <= GetPhysicalEBlockAddr(devID,chainEB))
          {
             dataCount++;
          }
       }
       #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    }
    (*findDataEBNumPtr) = dataCount;
    (*findSystemEBNumPtr) = sysCount;

    return FTL_ERR_PASS;
}

FTL_STATUS DataGCForSuperSysEB(void)
{

    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FTL_DEV devID = EMPTY_BYTE;                        /*1*/
    UINT16 freedUpPages = 0;                           /*2*/
    UINT16 freePageIndex = 0;                          /*2*/

    UINT32 lowestErase = EMPTY_DWORD;                  /*2*/
    UINT16 lowestEB = EMPTY_WORD;                      /*2*/
    UINT32 eraseCount = 0;                             /*2*/
    UINT16 logicalEBNum = 0;                           /*2*/

    #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    UINT16 chainEB = 0;
    #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)

    for (devID = 0; devID < NUMBER_OF_DEVICES; devID++)
    {

       for(logicalEBNum = 0; logicalEBNum < NUM_DATA_EBLOCKS; logicalEBNum++)
       {
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          if(FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum , CACHE_WRITE_TYPE)))
          {
             return status;
          }
          #endif
          eraseCount = GetTrueEraseCount(devID, logicalEBNum);
          if((eraseCount < lowestErase) && (SUPER_SYS_START_EBLOCKS <= GetPhysicalEBlockAddr(devID,logicalEBNum)))
          {
             lowestErase = eraseCount;
             lowestEB = logicalEBNum;
          }
          #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
          chainEB = GetChainLogicalEBNum(devID, logicalEBNum);
          if (EMPTY_WORD != chainEB)
          {
             eraseCount = GetTrueEraseCount(devID, logicalEBNum);
             if((eraseCount < lowestErase) && (SUPER_SYS_START_EBLOCKS <= GetPhysicalEBlockAddr(devID,chainEB)))
             {
                lowestErase = eraseCount;
                lowestEB = chainEB;
             }
          }          
          #endif
       }

       if((EMPTY_WORD == lowestErase) || (EMPTY_WORD == lowestEB))
       {
          return FTL_ERR_FAIL;
       }

       gProtectForSuperSysEBFlag = FTL_TRUE;

       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       if(FTL_ERR_PASS != (status = CACHE_LoadEB(devID, lowestEB , CACHE_WRITE_TYPE)))
       {
          return status;
       }
       #endif

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       status = InternalForcedGCWithBBManagement(devID, lowestEB, &freedUpPages, &freePageIndex, FTL_TRUE); 

       #else
       status = FTL_InternalForcedGC(devID, lowestEB, &freedUpPages, &freePageIndex, FTL_TRUE); 
       #endif

       if(status != FTL_ERR_PASS)
       {
          return status;
       }

       // For power failure
       if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
       {
          return status;
       }
       
       gProtectForSuperSysEBFlag = FTL_FALSE;
       
       FTL_ClearGCSave(CLEAR_GC_SAVE_RUNTIME_MODE);
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       ClearTransLogEBBadBlockInfo();
       #endif
    }
    return FTL_ERR_PASS;
}


FTL_STATUS ClearSuperEBInfo(void)
{
    FTL_DEV devID = EMPTY_BYTE;                        /*1*/
    UINT8 eBlockNum = 0;                                  /*1*/

    for (devID = 0; devID < NUMBER_OF_DEVICES; devID++)
    {
       SuperEBInfo[devID].checkChanged = FTL_FALSE;
       SuperEBInfo[devID].checkLost = FTL_FALSE;
       SuperEBInfo[devID].checkSuperPF = FTL_FALSE;
       SuperEBInfo[devID].checkSysPF = FTL_FALSE;
       for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
       {
          SuperEBInfo[devID].storeFreePage[eBlockNum] = 0;
       }
    }
    return FTL_ERR_PASS;
}

//----------------------
void SetSuperSysEBCounter(FTL_DEV devID, UINT32 counter)
{
    SuperSysEBCounter[devID] = counter;    
}

//--------------------------
UINT32 GetSuperSysEBCounter(FTL_DEV devID)
{
    return SuperSysEBCounter[devID];    
}

//----------------------
FTL_STATUS TABLE_GetSuperSysEBEntry(FTL_DEV devID, UINT16 blockNum, UINT16_PTR logEBlockPtr, UINT16_PTR phyAddrPtr, UINT32_PTR keyPtr)
{
    if((SuperSysEBArrayCount[devID] > NUM_SUPER_SYS_EBLOCKS) || (SuperSysEBArrayCount[devID] == 0))
    {
       return FTL_ERR_SUPER_LOG_GET_ENTRY;
    }
    if(logEBlockPtr != NULL)
    {
       *logEBlockPtr = SuperSysEBArray[devID][blockNum].logicalEBNum;
    }
    if(phyAddrPtr != NULL)
    {
       *phyAddrPtr   = SuperSysEBArray[devID][blockNum].phyAddr;
    }
    if(keyPtr != NULL)
    {
       *keyPtr       = SuperSysEBArray[devID][blockNum].key;
    }
    return FTL_ERR_PASS;
}


//-----------------------
FTL_STATUS TABLE_SuperSysEBClear(FTL_DEV devID)
{
    UINT16 eBlockNum = 0;                              /*2*/

    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {
       SuperSysEBArray[devID][eBlockNum].key = EMPTY_DWORD;
       SuperSysEBArray[devID][eBlockNum].phyAddr = EMPTY_WORD;
       SuperSysEBArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
    }
    SuperSysEBArrayCount[devID] = 0;
    return FTL_ERR_PASS;
}


//-------------------------------------
FTL_STATUS TABLE_SuperSysEBInsert(FTL_DEV devID, UINT16 logicalAddr, UINT16 phyEBAddr, UINT32 key)
{      
    UINT16 eBlockNum = 0;                              /*2*/

    if(SuperSysEBArrayCount[devID] >= NUM_SUPER_SYS_EBLOCKS)
    {
       return FTL_ERR_SUPER_LOG_INSERT;
    }
    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {  
       if(SuperSysEBArray[devID][eBlockNum].key == EMPTY_DWORD)
       {
          /*just put it here*/
          SuperSysEBArray[devID][eBlockNum].key = key;
          SuperSysEBArray[devID][eBlockNum].phyAddr = phyEBAddr;
          SuperSysEBArray[devID][eBlockNum].logicalEBNum = logicalAddr;          
          SuperSysEBArrayCount[devID]++;
          TABLE_SortSuperTable(devID);
          return FTL_ERR_PASS;
       }
    }  
    return FTL_ERR_SUPER_LOG_NO_ROOM_02;
}

//------------------------------------------
FTL_STATUS TABLE_SuperSysEBGetLatest(FTL_DEV devID, UINT16_PTR logEBlockPtr,
    UINT16_PTR phyEBAddrPtr, UINT32 key) 
{
    UINT16 block = 0;                                  /*2*/

    *logEBlockPtr = EMPTY_WORD;
    for(block = 0; block < NUM_SUPER_SYS_EBLOCKS; block++)
    {
       if(SuperSysEBArray[devID][block].key == key)
       {
          *logEBlockPtr = SuperSysEBArray[devID][block].logicalEBNum;
          *phyEBAddrPtr = SuperSysEBArray[devID][block].phyAddr;
          break;
       }
    }
    if(EMPTY_WORD == *logEBlockPtr)
    {
       return FTL_ERR_SUPER_LOG_GET_LATEST;
    }
    return FTL_ERR_PASS;
}



//-----------------------------
FTL_STATUS TABLE_SuperSysEBGetNext(FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT16_PTR phyEBAddrPtr, UINT32_PTR keyPtr)
{
    UINT16 eBlockNum = 0;                              /*2*/
    UINT32 smallestKey = EMPTY_DWORD;                  /*4*/
    UINT16 smallestKeyEB = EMPTY_WORD;                 /*2*/

    if((SuperSysEBArrayCount[devID] > NUM_SUPER_SYS_EBLOCKS) || (SuperSysEBArrayCount[devID] == 0))
    {
       return FTL_ERR_SUPER_NO_EBLOCKS;
    }
    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {
       if(SuperSysEBArray[devID][eBlockNum].key < smallestKey)
       {
          smallestKeyEB = eBlockNum;
          smallestKey = SuperSysEBArray[devID][eBlockNum].key;
       }
    }
    if(keyPtr != NULL)
    {
       *keyPtr = SuperSysEBArray[devID][smallestKeyEB].key;
    }
    if(phyEBAddrPtr != NULL)
    {
       *phyEBAddrPtr = SuperSysEBArray[devID][smallestKeyEB].phyAddr;
    }
    if(logicalAddrPtr != NULL)
    {
       *logicalAddrPtr = SuperSysEBArray[devID][smallestKeyEB].logicalEBNum;
    }
    SuperSysEBArray[devID][smallestKeyEB].key = EMPTY_DWORD;
    SuperSysEBArray[devID][smallestKeyEB].phyAddr = EMPTY_WORD;
    SuperSysEBArray[devID][smallestKeyEB].logicalEBNum = EMPTY_WORD;
    SuperSysEBArrayCount[devID]--;    
    TABLE_SortSuperTable(devID);
    return FTL_ERR_PASS;
}

//---------------------------------
void TABLE_SortSuperTable(FTL_DEV devID)  /*  1*/
{
    #if(NUM_SUPER_SYS_EBLOCKS > 1)
    UINT32 temp32 = 0;                                 /*4*/
    UINT16 temp16 = 0;                                 /*2*/
    UINT16 loop = 0;                                   /*2*/
    UINT16 count = 0;                                  /*2*/
    UINT16 last = 0;                                   /*2*/
    UINT16 end = 0;                                    /*2*/

    last = NUM_SUPER_SYS_EBLOCKS - 1;
    for (loop = 0; loop < (NUM_SUPER_SYS_EBLOCKS - 1); loop++)
    {
       end = last;
       last = 0;
       for (count = 0; count < end; count++)
       {
          if (SuperSysEBArray[devID][count].key > SuperSysEBArray[devID][count+1].key)
          {
             temp32 = SuperSysEBArray[devID][count].key;
             SuperSysEBArray[devID][count].key = SuperSysEBArray[devID][count+1].key;
             SuperSysEBArray[devID][count+1].key = temp32;
             temp16 = SuperSysEBArray[devID][count].phyAddr;
             SuperSysEBArray[devID][count].phyAddr = SuperSysEBArray[devID][count+1].phyAddr;
             SuperSysEBArray[devID][count+1].phyAddr = temp16;
             temp16 = SuperSysEBArray[devID][count].logicalEBNum;
             SuperSysEBArray[devID][count].logicalEBNum = SuperSysEBArray[devID][count+1].logicalEBNum;
             SuperSysEBArray[devID][count+1].logicalEBNum = temp16;
             last = count;
          }
       }
       if (0 == last)
       {
          break;
       }
    }
    #endif  // #if(NUM_SUPER_SYS_EBLOCKS > 1)
}


//-----------------------------
FTL_STATUS TABLE_SuperSysEBRemove(FTL_DEV devID, UINT16 blockNum)
{
    UINT16 blockCount = 0;                             /*2*/
    UINT16 SuperSysEBCount = 0;                        /*2*/

    if(blockNum >= SuperSysEBArrayCount[devID])
    {
        return FTL_ERR_SUPER_NO_EBLOCK;
    }
    SuperSysEBCount = SuperSysEBArrayCount[devID];
    for(blockCount = blockNum; blockCount < SuperSysEBCount; blockCount++)
    {
       SuperSysEBArray[devID][blockCount].key = EMPTY_DWORD;
       SuperSysEBArray[devID][blockCount].phyAddr = EMPTY_WORD;
       SuperSysEBArray[devID][blockCount].logicalEBNum = EMPTY_WORD;
       SuperSysEBArrayCount[devID]--;
    }
    TABLE_SortSuperTable(devID);
    return FTL_ERR_PASS;
}

FTL_STATUS TABLE_GetPhySysEB(FTL_DEV devID, UINT16_PTR countPtr, UINT16_PTR phyEBAddrPtr)
{

    UINT16 eBlockNum = 0;                              /*2*/

    *countPtr = 0;

    // Get TransLogEBArray
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {
        if(EMPTY_WORD == TransLogEBArray[devID][eBlockNum].logicalEBNum)
        {
           continue;
        }
        phyEBAddrPtr[*countPtr] = TransLogEBArray[devID][eBlockNum].phyAddr;
        (*countPtr)++;
    }
    // Get FlushLogEBArray
    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {
       if(EMPTY_WORD == FlushLogEBArray[devID][eBlockNum].logicalEBNum)
       {
          continue;
       }
       phyEBAddrPtr[*countPtr] = FlushLogEBArray[devID][eBlockNum].phyAddr;
       (*countPtr)++;
    }
    for(eBlockNum = (*countPtr); eBlockNum < MAX_NUM_SYS_EBLOCKS; eBlockNum++)
    {
       phyEBAddrPtr[eBlockNum] = EMPTY_WORD;
    }
    return FTL_ERR_PASS;
}

FTL_STATUS TABLE_CheckUsedSuperEB(FTL_DEV devID, UINT16 logicalAddr)
{
    UINT16 eBlockNum = 0;                              /*2*/

    if(logicalAddr < SYSTEM_START_EBLOCK)
    {
       return FTL_ERR_OUT_OF_RANGE;
    }
    if(logicalAddr == EMPTY_WORD)
    {
       return FTL_ERR_OUT_OF_RANGE;
    }
    // Check SuperSysEBArray
    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {
       if(logicalAddr == SuperSysEBArray[devID][eBlockNum].logicalEBNum)
       {
          return FTL_ERR_PASS;
       }
    }    
    return FTL_ERR_FAIL;
}
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

//-----------------------------------------------
FTL_STATUS TABLE_ClearReservedEB(FTL_DEV devID)
{
    UINT16 numEBlocks = 0;                              /*2*/ /* changed by Nobu - Feb 17th, 2015 : UINT8 -> UINT16 */

    for(numEBlocks = 0; numEBlocks < NUMBER_OF_SYSTEM_EBLOCKS; numEBlocks++)
    {
       ReservedEBlock[devID][numEBlocks] = EMPTY_WORD;
    }
    ReservedEBlockNum[devID] = 0;

    return FTL_ERR_PASS;
}

//-----------------------------------------------
FTL_STATUS TABLE_InsertReservedEB(FTL_DEV devID, UINT16 logicalAddr)
{
    UINT16 numEBlocks = 0;                              /*2*/  /* changed by Nobu Feb 18, 2015 : UINT8 -> UINT16 for ML16G2 */
    UINT16 findCount = 0;                               /*2*/  /* changed by Nobu Feb 18, 2015 : UINT8 -> UINT16 for ML16G2 */

    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    if(GetBadEBlockStatus(devID, logicalAddr) == FTL_TRUE)
    {
       return FTL_ERR_PASS;
    }    
    #endif
    if(logicalAddr < SYSTEM_START_EBLOCK)
    {
       return FTL_ERR_PASS;
    }
    numEBlocks = ReservedEBlockNum[devID];
    if(numEBlocks >= NUMBER_OF_SYSTEM_EBLOCKS)
    {
       return FTL_ERR_SYSEBLK_NO_BLK;
    }
    #if DEBUG_RESERVEDEB
    DBG_Printf("TABLE_InsertReservedEB: logical EBlock 0x%X\n", logicalAddr, 0);
    #endif
    for(findCount = 0; findCount < numEBlocks; findCount++)
    {
       if(logicalAddr == ReservedEBlock[devID][findCount])
       {
          return FTL_ERR_PASS;
       }
    }
    ReservedEBlock[devID][numEBlocks] = logicalAddr;
    ReservedEBlockNum[devID]++;
    return FTL_ERR_PASS;
}

//-----------------------------------------------
FTL_STATUS TABLE_GetReservedEB(FTL_DEV devID, UINT16_PTR logicalAddrPtr, UINT8 WLflag)
{
    UINT16 logicalEBNum = 0;                           /*2*/
    UINT16 eblockCount = 0;                            /*2*/
    UINT32 eraseCount = 0;                             /*2*/
    UINT16 ReservedEBNum = 0;                          /*2*/

    UINT16 count = 0;                                   /*2*/
    UINT32 selectedScore = EMPTY_DWORD;                 /*4*/
    UINT16 selectedIndex = 0;                           /*2*/


    ReservedEBNum = (UINT16)(ReservedEBlockNum[devID] & EMPTY_WORD);
    if(ReservedEBNum == 0)
    {
       return FTL_ERR_FLUSH_NO_SYS_EBLOCK;
    }

    // If WLflag = FTL_FALSE, the entry with the lowest weight is picked
    // If WLflag = FTL_TRUE, the entry with the highest weight is picked
    if (FTL_TRUE == WLflag)
    {
       selectedScore = 0;
    }
    for(eblockCount = 0; eblockCount < ReservedEBNum; eblockCount++)
    {
       logicalEBNum = ReservedEBlock[devID][eblockCount];
       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if(FTL_TRUE == gProtectForSuperSysEBFlag)
       {
          if((SUPER_SYS_START_EBLOCKS <= GetPhysicalEBlockAddr(devID,logicalEBNum)) && (NUM_SUPER_SYS_EBLOCKS > count))
          {
             count++;
             continue;
          }
       }
       #endif
       
       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       if ((FTL_FALSE == WLflag) && (GetEBErased(devID, logicalEBNum) == FTL_TRUE))
       {
          selectedIndex = eblockCount;
          break;
       }
       #endif
       eraseCount = GetTrueEraseCount(devID, logicalEBNum);
       if(((FTL_TRUE == WLflag) && (selectedScore < eraseCount)) ||
          ((FTL_FALSE == WLflag) && (selectedScore > eraseCount)))
       {
          selectedScore = eraseCount;
          selectedIndex = eblockCount;
       }

    }
    *logicalAddrPtr = ReservedEBlock[devID][selectedIndex];
    for(eblockCount = selectedIndex; eblockCount < (ReservedEBNum-1); eblockCount++)
    {
       ReservedEBlock[devID][eblockCount] = ReservedEBlock[devID][eblockCount+1];
    }
    #if DEBUG_RESERVEDEB
    DBG_Printf("TABLE_GetReservedEB: logical EBlock 0x%X\n", *logicalAddr, 0);
    #endif
    ReservedEBlock[devID][ReservedEBNum-1] = EMPTY_WORD;
    ReservedEBlockNum[devID]--;
    return FTL_ERR_PASS;
}


//-----------------------------------------------
UINT16 TABLE_GetReservedEBlockNum(FTL_DEV devID) /* changed by Nobu Feb 18, 2015 : UINT8 -> UINT16 for ML16G2 */
{
    return ReservedEBlockNum[devID];
}

UINT16 TABLE_GetUsedSysEBCount(FTL_DEV devID)
{
    UINT16 sum = 0;

    sum = FlushLogEBArrayCount[devID];
    sum += TransLogEBArrayCount[devID];

    return sum;
}

FTL_STATUS TABLE_CheckUsedSysEB(FTL_DEV devID, UINT16 logicalAddr)
{
    UINT16 eBlockNum = 0;                              /*2*/

    if(logicalAddr < SYSTEM_START_EBLOCK)
    {
       return FTL_ERR_OUT_OF_RANGE;
    }
    if(logicalAddr == EMPTY_WORD)
    {
       return FTL_ERR_OUT_OF_RANGE;
    }
    // Check TransLogEBArray
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {  
       if(logicalAddr == TransLogEBArray[devID][eBlockNum].logicalEBNum)
       {
          return FTL_ERR_PASS;
       }
    }
    // Check FlushLogEBArray
    for(eBlockNum = 0; eBlockNum < NUM_FLUSH_LOG_EBLOCKS; eBlockNum++)
    {
       if(logicalAddr == FlushLogEBArray[devID][eBlockNum].logicalEBNum)
       {
          return FTL_ERR_PASS;
       }
    }
    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    // Check SuperSysEBArray
    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {
       if(logicalAddr == SuperSysEBArray[devID][eBlockNum].logicalEBNum)
       {
          return FTL_ERR_PASS;
       }
    }    
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    return FTL_ERR_FAIL;
}

//------------------------------------------
#if (CACHE_RAM_BD_MODULE == FTL_FALSE)
FTL_STATUS AdjustFlushEBlockFreePage(FTL_DEV devID, UINT16 logicalFlushEBlockNum, UINT16 dirtyBitMapCount)
{
    UINT16 sectionNum = 0x0;                           /*2*/
    UINT16 count = 0x0;                                /*2*/
    UINT16 incCount = 0x0;                             /*2*/

    sectionNum = ((logicalFlushEBlockNum * EBLOCK_MAPPING_ENTRY_SIZE)/ FLUSH_RAM_TABLE_SIZE);
    if(sectionNum == (BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE - 1))
    {
       return FTL_ERR_PASS;
    }
    else if(sectionNum > (BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE - 1))
    {
       return FTL_ERR_FLUSH_OUT_OF_RANGE;
    }
    else
    {
       // the flush eblock is NOT in last dirty bit map section.
       if(dirtyBitMapCount == sectionNum)
       {
          // Adjust free page for remaining pages
          // Should be restored after flush
          FlushEBlockAdjustedFreePage[devID] = GetGCNum(devID, logicalFlushEBlockNum);
          incCount = BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE - sectionNum;
          for(count = 1; count < incCount; count++)
          {
             if(GetBitMapField(&EBlockMappingTableDirtyBitMap[devID][0], (dirtyBitMapCount+count), 1) == DIRTY_BIT)
             {
                IncGCOrFreePageNum(devID, logicalFlushEBlockNum);
             }
          }
       }
    }

    return FTL_ERR_PASS;
}
#endif
//------------------------------------------
FTL_STATUS RestoreFlushEBlockFreePage(FTL_DEV devID, UINT16 logicalFlushEBlockNum)
{
    if(FlushEBlockAdjustedFreePage[devID] != EMPTY_DWORD)
    {
       SetGCOrFreePageNum(devID, logicalFlushEBlockNum, FlushEBlockAdjustedFreePage[devID]);
       FlushEBlockAdjustedFreePage[devID] = EMPTY_DWORD;
    }
    return FTL_ERR_PASS;
}

//----------------------------------------------
FTL_STATUS DBG_CheckMappingTables(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 count=0;                                    /*2*/
    FTL_DEV devCount = 0;                              /*1*/
    UINT16 eb = 0;                                     /*2*/
    UINT16 eblist[NUM_EBLOCKS_PER_DEVICE];

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 logChainEB = 0;                             /*2*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    for(devCount = 0; devCount < NUM_DEVICES; devCount++)
    {
       for(count=0; count < NUM_EBLOCKS_PER_DEVICE; count++)
       {
          eblist[count] = EMPTY_WORD;
       }
       for(count=0; count < NUM_EBLOCKS_PER_DEVICE; count++)
       {
          eb = GetPhysicalEBlockAddr(devCount, count);
          if(eblist[eb] == EMPTY_WORD)
          {
             eblist[eb] = count;
          }
          else
          {

             #if DEBUG_PRINT_ERRORS
             DBG_Printf("DBG_CheckMappingTables: Error: More than one Table entry points to phyEB 0x%X\n", eb, 0);
             #endif  // #if DEBUG_PRINT_ERRORS

             status = FTL_ERR_DEBUG_DUPLICATE_BLOCK;
          }
       }
       for(count=0; count < NUM_EBLOCKS_PER_DEVICE; count++)
       {
          if(eblist[count] == EMPTY_WORD)
          {

             #if DEBUG_PRINT_ERRORS
             DBG_Printf("DBG_CheckMappingTables: Error: No table entry points to phyEB 0x%X\n", eb, 0);
             #endif  // #if DEBUG_PRINT_ERRORS

             status = FTL_ERR_DEBUG_MISSING_BLOCK;
          }
       }

       #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
       for(count=0; count < NUM_EBLOCKS_PER_DEVICE; count++)
       {
          logChainEB = GetChainLogicalEBNum(devCount, count);
          if (EMPTY_WORD != logChainEB)
          {
             if (count != GetChainLogicalEBNum(devCount, logChainEB))
             {

                #if DEBUG_PRINT_ERRORS
                DBG_Printf("DBG_CheckMappingTables: Error: Logical Chain Broke from 0x%X\n", count, 0);
                DBG_Printf("  GetChainLogicalEBNum(%d, ", devCount, 0);
                DBG_Printf("0x%X) = ", logChainEB, 0);
                DBG_Printf("0x%X\n", GetChainLogicalEBNum(devCount, logChainEB), 0);
                #endif  // #if DEBUG_PRINT_ERRORS

                status = FTL_ERR_DEBUG_LOG_CHAIN_BROKE;
             }
             if (GetPhysicalEBlockAddr(devCount, logChainEB) != GetChainPhyEBNum(devCount, count))
             {

                #if DEBUG_PRINT_ERRORS
                DBG_Printf("DBG_CheckMappingTables: Error: Physical Chain Broke from 0x%X\n", count, 0);
                DBG_Printf("  GetChainPhyEBNum(%d, ", devCount, 0);
                DBG_Printf("0x%X) = ", count, 0);
                DBG_Printf("0x%X\n", GetChainPhyEBNum(devCount, count), 0);
                DBG_Printf("  GetPhysicalEBlockAddr(%d, ", devCount, 0);
                DBG_Printf("0x%X) = ", logChainEB, 0);
                DBG_Printf("0x%X\n", GetPhysicalEBlockAddr(devCount, logChainEB), 0);
                #endif  // #if DEBUG_PRINT_ERRORS

                status = FTL_ERR_DEBUG_PHY_CHAIN_BROKE;
             }
          }
       }
       #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    }
    return status;
}

//--------------------------------
FTL_STATUS DBG_CheckPPAandBitMap(FTL_DEV devID, UINT16 logicalEBNum)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 logicalPageOffset = EMPTY_WORD;             /*2*/
    UINT16 phyPageOffset = EMPTY_WORD;                 /*2*/
    UINT16 phyPageOffset2 = EMPTY_WORD;                /*2*/
    FREE_BIT_MAP_TYPE bitMap = EMPTY_BYTE;             /*1*/
    UINT8 found = 0;                                   /*1*/

    #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    UINT16 chainEBNum = EMPTY_WORD;                    /*2*/
    #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)

    for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
    {
       phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
       if ((EMPTY_INVALID != phyPageOffset) && (CHAIN_INVALID != phyPageOffset))
       {
          bitMap = GetEBlockMapFreeBitIndex(devID, logicalEBNum, phyPageOffset);
          if (BLOCK_INFO_VALID_PAGE != bitMap)
          {
          
             #if DEBUG_PRINT_ERRORS
             DBG_Printf("DBG_CheckPPAandBitMap: Error: PPA points to non-valid page: devID=%d\n", devID, 0);
             DBG_Printf("  logEBNum=0x%X, ", logicalEBNum, 0);
             DBG_Printf("logPageOffset=0x%X, ", logicalPageOffset, 0);
             DBG_Printf("phyPageOffset=0x%X, ", phyPageOffset, 0);
             DBG_Printf("bitMap=%d\n", bitMap, 0);
             #endif  // #if DEBUG_PRINT_ERRORS
             
             status = FTL_ERR_TABLE_PPA_BITMAP_01;
          }
       }
    }
    for (phyPageOffset = 0; phyPageOffset < NUM_PAGES_PER_EBLOCK; phyPageOffset++)
    {
       bitMap = GetEBlockMapFreeBitIndex(devID, logicalEBNum, phyPageOffset);
       if (BLOCK_INFO_VALID_PAGE == bitMap)
       {
          found = 0; 
          for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
          {
             phyPageOffset2 = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
             if (phyPageOffset == phyPageOffset2)
             {
                found++;
             }
          }
          if (found == 0)
          {
             #if DEBUG_PRINT_ERRORS
             DBG_Printf("DBG_CheckPPAandBitMap: Error: No PPA entries for devID=%d\n", devID, 0);
             DBG_Printf("  logEBNum=0x%X, ", logicalEBNum, 0);
             DBG_Printf("phyPageOffset=0x%X\n", phyPageOffset, 0);
             #endif  // #if DEBUG_PRINT_ERRORS

             status = FTL_ERR_TABLE_PPA_BITMAP_03;
          }
          if(found > 1)
          {
           
             #if DEBUG_PRINT_ERRORS
             DBG_Printf("DBG_CheckPPAandBitMap: Error: multiple PPA entries for devID=%d\n", devID, 0);
             DBG_Printf("  logEBNum=0x%X, ", logicalEBNum, 0);
             DBG_Printf("phyPageOffset=0x%X\n", phyPageOffset, 0);
             #endif  // #if DEBUG_PRINT_ERRORS
             
             status = FTL_ERR_TABLE_PPA_BITMAP_02;
          }
       }
    }

    #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    if (logicalEBNum < NUM_DATA_EBLOCKS)
    {
       // Data EBlock
       chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
       if (EMPTY_WORD == chainEBNum)
       {
          // logicalEBNum is not chained - There should not be any CHAIN_INVALID PPA Entries
          for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
          {
             phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
             if (CHAIN_INVALID == phyPageOffset)
             {

                #if DEBUG_PRINT_ERRORS
                DBG_Printf("DBG_CheckPPAandBitMap: Error: non-Chained EBlock contains a chained page\n", 0, 0);
                DBG_Printf("  for devID=%d, ", devID, 0);
                DBG_Printf("logical EB=0x%X, ", logicalEBNum, 0);
                DBG_Printf("logicalPageOffset=0x%X\n", logicalPageOffset, 0);
                #endif  // #if DEBUG_PRINT_ERRORS

                status = FTL_ERR_TABLE_PPA_BITMAP_06;
             }
          }
       }
       else
       {
          // logicalEBNum is chained - all chained pages should be valid in the chained-to EBlock
          for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
          {
             phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
             if (CHAIN_INVALID == phyPageOffset)
             {
                // Page is chained
                phyPageOffset = GetPPASlot(devID, chainEBNum, logicalPageOffset);
                if (EMPTY_INVALID == phyPageOffset)
                {

                   #if DEBUG_PRINT_ERRORS
                   DBG_Printf("DBG_CheckPPAandBitMap: Error: Chained page is invalid in chained-to EBlock\n", 0, 0);
                   DBG_Printf("  for devID=%d, ", devID, 0);
                   DBG_Printf("chained-from EB=0x%X, ", logicalEBNum, 0);
                   DBG_Printf("chained-to EB=0x%X, ", chainEBNum, 0);
                   DBG_Printf("logicalPageOffset=0x%X\n", logicalPageOffset, 0);
                   #endif  // #if DEBUG_PRINT_ERRORS

                   status = FTL_ERR_TABLE_PPA_BITMAP_04;
                }
             }
          }
       }
    }
    else
    {
       if(FTL_ERR_FAIL == TABLE_CheckUsedSysEB(devID,logicalEBNum))
       {
          // Data Reserved EBlock
          chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
          if (EMPTY_WORD != chainEBNum)
          {
             // logicalEBNum is chained
             for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
             {
                phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
                if (EMPTY_INVALID != phyPageOffset)
                {
                   // Page is valid
                   phyPageOffset = GetPPASlot(devID, chainEBNum, logicalPageOffset);
                   if (CHAIN_INVALID != phyPageOffset)
                   {

                      #if DEBUG_PRINT_ERRORS
                      DBG_Printf("DBG_CheckPPAandBitMap: Error: Valid page is not chained in chained-from EBlock\n", 0, 0);
                      DBG_Printf("  for devID=%d, ", devID, 0);
                      DBG_Printf("chained-to EB=0x%X, ", logicalEBNum, 0);
                      DBG_Printf("chained-from EB=0x%X, ", chainEBNum, 0);
                      DBG_Printf("logicalPageOffset=0x%X\n", logicalPageOffset, 0);
                      #endif  // #if DEBUG_PRINT_ERRORS

                      status = FTL_ERR_TABLE_PPA_BITMAP_05;
                   }
                }
             }
          }
       }
    }
    #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)

    return status;
}

FTL_STATUS ClearDeleteInfo(void)
{
    UINT8 sectorCount = 0;                             /*1*/

    Del_Info.devID = EMPTY_BYTE;
    Del_Info.logicalPageAddr = EMPTY_DWORD;
    for(sectorCount = 0; sectorCount < NUMBER_OF_SECTORS_PER_PAGE; sectorCount++)
    {
       Del_Info.sector[sectorCount] = FTL_FALSE;
    }
    return FTL_ERR_PASS;
}

FTL_STATUS InitDeleteInfo(UINT8 devID, UINT32 logicalPageAddr, UINT8 startSector, UINT8 numSectors)
{
    UINT8 sectorCount = 0;                             /*1*/

    if((Del_Info.devID != EMPTY_BYTE) || (Del_Info.logicalPageAddr != EMPTY_DWORD))
    {
       return FTL_ERR_DEL_INFO_EMPTY;
    }

    Del_Info.devID = devID;
    Del_Info.logicalPageAddr = logicalPageAddr;
    for(sectorCount = startSector; sectorCount < (startSector + numSectors); sectorCount++)
    {
       Del_Info.sector[sectorCount] = FTL_TRUE;
    }
    return FTL_ERR_PASS;
}

FTL_STATUS HitDeleteInfo(UINT8 devID, UINT32 logicalPageAddr)
{
    if((devID == Del_Info.devID) && (logicalPageAddr == Del_Info.logicalPageAddr))
    {
       return FTL_ERR_PASS;
    }

    return FTL_ERR_DEL_INFO_MISS;
}

FTL_STATUS UpdateDeleteInfo(UINT8 startSector, UINT8 numSectors)
{
    UINT8 sectorCount = 0;                             /*1*/

    for(sectorCount = startSector; sectorCount < (startSector + numSectors); sectorCount++)
    {
       Del_Info.sector[sectorCount] = FTL_TRUE;
    }
    return FTL_ERR_PASS;
}

UINT8 GetDeleteInfoNumSectors(void)
{
    UINT8 sectorCount = 0;                             /*1*/
    UINT8 numSectors = 0;                              /*1*/

    for(sectorCount = 0; sectorCount < NUMBER_OF_SECTORS_PER_PAGE; sectorCount++)
    {
       if(Del_Info.sector[sectorCount] == FTL_TRUE)
       {
          numSectors++;
       }
    }

    return numSectors;
}

void ClearTransferEB(void)
{
    UINT8 eblockCount = 0;                             /*1*/

    for(eblockCount = 0; eblockCount < MAX_TRANSFER_EBLOCKS; eblockCount++)
    {
       transferEB[eblockCount].devID = EMPTY_BYTE;
       transferEB[eblockCount].logicalEBNum = EMPTY_WORD;
    }
}

void SetTransferEB(ADDRESS_STRUCT_PTR startPage, ADDRESS_STRUCT_PTR endPage)
{
    UINT16 logStartEB = EMPTY_WORD;                    /*2*/
    UINT16 logEndEB = EMPTY_WORD;                      /*2*/

    GetLogicalEBNum(startPage->logicalPageNum, &logStartEB);
    transferEB[0].devID = startPage->devID;
    transferEB[0].logicalEBNum = logStartEB;
    GetLogicalEBNum(endPage->logicalPageNum, &logEndEB);
    if((endPage->devID != startPage->devID) || (logEndEB != logStartEB))
    {
       transferEB[1].devID = endPage->devID;
       transferEB[1].logicalEBNum = logEndEB;
    }
}

void GetTransferEB(UINT8 eblockCount, TRANSFER_EB_PTR transferEBPtr)
{
    transferEBPtr->devID = transferEB[eblockCount].devID;
    transferEBPtr->logicalEBNum = transferEB[eblockCount].logicalEBNum;
}

#if(FTL_RPB_CACHE == FTL_TRUE)
//---------------------------------
UINT8 GetRPBCacheStatus(UINT8 devID)
{
    return RPBCache[devID].cache.status;
}

//-------------------------------------
void SetRPBCacheStatus(UINT8 devID, UINT8 status)
{
    RPBCache[devID].cache.status = status;
}

//------------------------------------
UINT32 GetRPBCacheLogicalPageAddr(UINT8 devID)
{
    return RPBCache[devID].cache.logicalPageAddr;
}

//----------------------------------
void SetRPBCacheLogicalPageAddr(UINT8 devID, UINT32 logicalPageAddr)
{
    RPBCache[devID].cache.logicalPageAddr = logicalPageAddr;
}

//----------------------------------
void ClearRPBCache(UINT8 devID)
{
    SetRPBCacheStatus(devID, CACHE_EMPTY);
    SetRPBCacheLogicalPageAddr(devID, EMPTY_DWORD);
}

//----------------------------------
UINT8_PTR GetRPBCache(UINT8 devID)
{
    return RPBCache[devID].RPB;
}
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

#if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
//------------------------------------
FTL_STATUS FindAndSwapUnusedEB(FTL_DEV devID, UINT16 logFrom, UINT16_PTR logTo)
{
    UINT8 flag = FTL_FALSE;                            /*1*/
    UINT8 count = 0;                                   /*1*/
    UINT16 temp = EMPTY_WORD;                          /*2*/
    UINT16 logicalEBNum = 0;                           /*2*/
    UINT32 score = EMPTY_DWORD;                        /*2*/
    UINT32 chosenScore = EMPTY_DWORD;                  /*2*/
    UINT16 chosenEBNum = EMPTY_WORD;                   /*2*/
    TRANSFER_EB transferEB = {EMPTY_BYTE, EMPTY_WORD}; /*3*/
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/


    // logFrom should be in the reserve pool and not chained
    if(logFrom < NUM_DATA_EBLOCKS)
    {
       return FTL_ERR_PRE_ERASED_PARAMETER;
    }
    if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,logFrom))
    {
       return FTL_ERR_PRE_ERASED_PARAMETER;
    }

    #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    if (EMPTY_WORD != GetChainLogicalEBNum(devID, logFrom))
    {
       return FTL_ERR_PRE_ERASED_CHAINED;
    }
    #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)

    // Initialize chosenScore to logFrom
    // Don't botter with EBlocks greater or equal to this
    chosenScore = GetEraseCount(devID, logFrom);

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    // Invert Pre-Erased bit so that Pre-Erased EBlocks will have higher priority
    chosenScore ^= ERASE_STATUS_GET_DWORD_MASK;
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    // Scan all Data EBlocks
    for (logicalEBNum = 0; logicalEBNum < NUM_DATA_EBLOCKS; logicalEBNum++)
    { 
       #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
       if (EMPTY_WORD != GetChainLogicalEBNum(devID, logicalEBNum))
       {
          // EBlock is chained
          continue;
       }
       #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
       #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(GetBadEBlockStatus(devID, logicalEBNum) == FTL_TRUE)
       {
           // block is bad, ignore it.
           continue;
       }
       #endif
       if (GetNumValidPages(devID, logicalEBNum) > 0)
       { 
          // EBlock is not empty
          continue;
       }
       if(GC_Info.devID != EMPTY_BYTE)
       {
          if((logicalEBNum == GC_Info.logicalEBlock) || 
             (logicalEBNum == GC_Info.startMerge.logicalEBNum ) || 
             (logicalEBNum == GC_Info.endMerge.logicalEBNum ))
          {
             // EBlock must be held for merging
             continue;
          }
       }
       flag = FTL_FALSE;
       temp = GetPhysicalEBlockAddr(devID, logicalEBNum);
       for (count = 0; count < MAX_BLOCKS_TO_SAVE; count++)
       {
          if ((gcSave[count].phyEbNum == temp) && (gcSave[count].devId == devID))
          {
             // EBlock is saved for PFT recovery
             flag = FTL_TRUE;
             break;
          } 
       }
       if (FTL_TRUE == flag)
       {
          continue;
       }
       flag = FTL_FALSE;
       for (count = 0; count < MAX_TRANSFER_EBLOCKS; count++)
       {
          GetTransferEB(count, &transferEB);
          if ((transferEB.devID == devID) && (transferEB.logicalEBNum == logicalEBNum))
          {
             // EBlock is saved for PFT recovery
             flag = FTL_TRUE;
          }
       }
       if (FTL_TRUE == flag)
       {
          continue;
       }
       score = GetEraseCount(devID, logicalEBNum);

       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       // Invert Pre-Erased bit so that Pre-Erased EBlocks will have higher priority
       score ^= ERASE_STATUS_GET_DWORD_MASK;
       #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

       // Select EBlock with lowest score
       if (score < chosenScore)
       { 
          chosenScore = score;
          chosenEBNum = logicalEBNum;
       }
    }
    if (chosenEBNum != EMPTY_WORD)
    {
       #if DEBUG_BLOCK_ERASED
       DBG_Printf("FindAndSwapUnusedEB: Swap logEB 0x%X ", logFrom, 0);
       DBG_Printf("phyEB 0x%X with\n", GetPhysicalEBlockAddr(devID, logFrom), 0);
       DBG_Printf("  logEB 0x%X ", chosenEBNum, 0);
       DBG_Printf("phyEB 0x%X ", GetPhysicalEBlockAddr(devID, chosenEBNum), 0);
       DBG_Printf("score = 0x%X\n", chosenScore, 0);
       #endif  // #if DEBUG_BLOCK_ERASED

       SwapUnusedEBlock(devID, chosenEBNum, logFrom);
       if((status = TABLE_InsertReservedEB(devID, logFrom)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    *logTo = chosenEBNum;
    return FTL_ERR_PASS;
}

//-----------------------------------
void SwapUnusedEBlock(FTL_DEV devID, UINT16 logicalDataEB, UINT16 logicalReservedEB)
{
    UINT32 tempData = EMPTY_WORD;                      /*2*/
    UINT32 tempRes = EMPTY_WORD;                       /*2*/
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT8 badEBlockFlag = FTL_FALSE;             
    #endif
//    UINT16 count = 0;
//    FREE_BIT_MAP_TYPE bitMapData = 0;
//    FREE_BIT_MAP_TYPE bitMapRes = 0;

          /*swap the physical address*/
    tempRes = (UINT16)GetPhysicalEBlockAddr(devID, logicalReservedEB);
    tempData = (UINT16)GetPhysicalEBlockAddr(devID, logicalDataEB);
    SetPhysicalEBlockAddr(devID, logicalReservedEB, (UINT16)tempData);
    SetPhysicalEBlockAddr(devID, logicalDataEB, (UINT16)tempRes);
          /*swap the erase count*/
    tempData = GetEraseCount(devID, logicalDataEB);
    tempRes = GetEraseCount(devID, logicalReservedEB);
    SetEraseCount(devID, logicalReservedEB, tempData);
    SetEraseCount(devID, logicalDataEB, tempRes);
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    badEBlockFlag = GetBadEBlockStatus(devID, logicalDataEB);             
    SetBadEBlockStatus(devID, logicalDataEB, GetBadEBlockStatus(devID, logicalReservedEB));
    SetBadEBlockStatus(devID, logicalReservedEB, badEBlockFlag);
    #endif        
    // Clear the Dirty Count of both EBlocks;  The loop will increment it as necessary
    SetDirtyCount(devID, logicalReservedEB, 0);
    SetDirtyCount(devID, logicalDataEB, 0);

/*    // Mark all VALID pages from the other EBlock as STALE in the current EBlock,
    //   otherwise, copy the bitMap from the other.
    // Clear the PPA tables for both EBlocks
    for(count = 0; count < NUM_PAGES_PER_EBLOCK; count++)
    {
       bitMapData = GetEBlockMapFreeBitIndex(devID, logicalDataEB, count);
       bitMapRes = GetEBlockMapFreeBitIndex(devID, logicalReservedEB, count);
       if (BLOCK_INFO_VALID_PAGE == bitMapData)
       {
          bitMapData = BLOCK_INFO_STALE_PAGE;          
       }
       if (BLOCK_INFO_VALID_PAGE == bitMapRes)
       {
          bitMapRes = BLOCK_INFO_STALE_PAGE;          
       }
       SetEBlockMapFreeBitIndex(devID, logicalDataEB, count, bitMapRes);
       SetEBlockMapFreeBitIndex(devID, logicalReservedEB, count, bitMapData);
       SetPPASlot(devID, logicalDataEB, count, EMPTY_INVALID);
       SetPPASlot(devID, logicalReservedEB, count, EMPTY_INVALID);
       if(bitMapData == BLOCK_INFO_STALE_PAGE) 
       {
          EBlockMappingTable[devID][logicalReservedEB].dirtyCount++;
       }
       if(bitMapRes == BLOCK_INFO_STALE_PAGE) 
       {
          EBlockMappingTable[devID][logicalDataEB].dirtyCount++;
       }
    }*/

    // Mark the new logicalDataEB as not-erased and STALE because there may be sone
    //   valid pages left over from the last GC
    // If the new logicalReservedEB was pre-erased, mark all of its pages EMPTY,
    //   otherwise, mark all pages STALE because they
    MarkAllPagesStatus(devID, logicalDataEB, BLOCK_INFO_STALE_PAGE);

    #if (FTL_TRUE == ENABLE_EB_ERASED_BIT)
    SetEBErased(devID, logicalDataEB, FTL_FALSE);
    if (FTL_TRUE == GetEBErased(devID, logicalReservedEB))
       {
       MarkAllPagesStatus(devID, logicalReservedEB, BLOCK_INFO_EMPTY_PAGE);
       }
    else
    #endif  // #if (FTL_TRUE == ENABLE_EB_ERASED_BIT)

    {
       MarkAllPagesStatus(devID, logicalReservedEB, BLOCK_INFO_STALE_PAGE);
    }
}
#endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

//-----------------------------------
/*This function should only be called on a EB thats fully erased, meaning, all the pages are marked free*/
void MarkAllPagesStatus(FTL_DEV devID, UINT16 logEBNum, UINT8 bitStatus)
{
    UINT16 count = 0;                                  /*2*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT32 EBMramStructPtr = 0;
    UINT32 PPAramStructPtr = 0;
    CACHE_GetRAMOffsetEB(devID, logEBNum, &EBMramStructPtr, &PPAramStructPtr);
    #endif

    for(count = 0; count < NUM_PAGES_PER_EBLOCK; count++)
    {
       UpdatePageTableInfo(devID, logEBNum, EMPTY_INVALID, count, bitStatus);
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       (*(PPA_MAPPING_ENTRY*)(PPAramStructPtr + (PPA_MAPPING_ENTRY_SIZE * count))) = EMPTY_INVALID;
       #else
       PPAMappingTable[devID][logEBNum][count] = EMPTY_INVALID;            
       #endif
         /*have to clear the PPA as well*/
    }
    MarkPPAMappingTableEntryDirty(devID, logEBNum, 0);
}

#if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
//-----------------------------
// This function replaces the function of the same name
//   If pickHottest is FTL_TRUE, this function will return the Hottest candidate instead of the Coldest
FTL_STATUS InitStaticWLInfo(void)
{
    StaticWLInfo.threshold = FTL_DATA_WEAR_LEVEL_RANGE;
    StaticWLInfo.count = 0;
    StaticWLInfo.staticWLCallCounter = 0;
    return FTL_ERR_PASS;
}

UINT32 GetStaticWLThreshold(void)
{
   return StaticWLInfo.threshold;
}

void IncStaticWLCount(void)
{
   StaticWLInfo.count++;
}

void CheckStaticWLCount(void)
{
    if(StaticWLInfo.count > STATIC_WL_THRESHOLD_COUNT)
    {
        StaticWLInfo.threshold += 10;
        StaticWLInfo.count = 0;
    }
}
#endif
