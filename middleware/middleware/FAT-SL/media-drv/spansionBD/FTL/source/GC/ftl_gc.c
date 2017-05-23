/* file: ftl_gc.c */
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

#ifdef __KERNEL__
  #include <linux/span/FTL/ftl_def.h>
  #include <linux/span/FTL/ftl_calc.h>
  #include <linux/span/FTL/ftl_common.h>
  #include <linux/span/FTL/ftl_if_in.h>
#else
  #include "ftl_def.h"
  #include "ftl_calc.h"
  #include "ftl_common.h"
  #include "ftl_if_in.h"
#endif // #ifdef __KERNEL__

#define DEBUG_GC_ANNOUNCE   (0)
#define DEBUG_GC_BLOCKS     (0)
#define DEBUG_BLOCK_SELECT  (0)
#define DEBUG_PRE_ERASED    (0)
#define DEBUG_CHECK_WL      (0)

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
extern BAD_BLOCK_INFO    badBlockInfo;
#endif
//-------------------------------------------------------------------------------------
FTL_STATUS ClearGC_Info(void)
{
    GC_Info.devID = EMPTY_BYTE;
    GC_Info.endMerge.logicalPageNum = EMPTY_DWORD;
    GC_Info.endMerge.DevID = EMPTY_BYTE;
    GC_Info.endMerge.logicalEBNum = EMPTY_WORD;
    GC_Info.endMerge.phyPageNum = EMPTY_DWORD;
    GC_Info.startMerge.logicalPageNum = EMPTY_DWORD;
    GC_Info.startMerge.DevID = EMPTY_BYTE;
    GC_Info.startMerge.logicalEBNum = EMPTY_WORD;
    GC_Info.startMerge.phyPageNum = EMPTY_DWORD;
    GC_Info.logicalEBlock = EMPTY_WORD;
    return FTL_ERR_PASS;
}

//-------------------------------------------------------------------------------------
FTL_STATUS ClearMergeGC_Info(FTL_DEV DevID, UINT16 logicalEBNum, UINT32 logicalPageNum)
{   
    if((GC_Info.endMerge.logicalPageNum == logicalPageNum) 
       && (GC_Info.endMerge.DevID == DevID)
       && (GC_Info.endMerge.logicalEBNum == logicalEBNum))
    {
       GC_Info.endMerge.logicalPageNum = EMPTY_DWORD;
       GC_Info.endMerge.DevID = EMPTY_BYTE;
       GC_Info.endMerge.logicalEBNum = EMPTY_WORD;
       GC_Info.endMerge.phyPageNum = EMPTY_DWORD;    
    }   
    if((GC_Info.startMerge.logicalPageNum == logicalPageNum) 
       && (GC_Info.startMerge.DevID == DevID)
       && (GC_Info.startMerge.logicalEBNum == logicalEBNum))
    {
       GC_Info.startMerge.logicalPageNum = EMPTY_DWORD;
       GC_Info.startMerge.DevID = EMPTY_BYTE;
       GC_Info.startMerge.logicalEBNum = EMPTY_WORD;
       GC_Info.startMerge.phyPageNum = EMPTY_DWORD;    
    }   
    return FTL_ERR_PASS;
}

//----------------------------
FTL_STATUS ClearGCPageBitMap(void)  /*0*/
{
    UINT16 count = 0;                        /*2*/
                         /*total stack bytes - 2*/
    for(count = 0; count< NUM_PAGES_PER_EBLOCK; count++)
    {
       GCMoveArray[count] = FTL_FALSE;

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       badBlockPhyPageAddr[count] = EMPTY_DWORD;
       #endif

    }
    GCMoveArrayNotEmpty = FTL_FALSE;
    return FTL_ERR_PASS;
}

//---------------------------
FTL_STATUS SetPageMoved(UINT16 pageAddress, UINT32 phyPageAddr)    /*2*/
{
                   /*total stack bytes - 2*/

    if(pageAddress > NUM_PAGES_PER_EBLOCK)
    {
       return FTL_ERR_GC_SET_MOVED;
    }
    GCMoveArrayNotEmpty = FTL_TRUE;
    GCMoveArray[pageAddress] = FTL_TRUE;

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    badBlockPhyPageAddr[pageAddress] = phyPageAddr;
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    return FTL_ERR_PASS;
}

//--------------------------------------------------------------------------
FTL_STATUS IsPageMoved(UINT32 pageAddr, UINT8_PTR isMoved)  /*4, 4*/
{
                   /*total stack bytes - 12 */

    if(pageAddr > NUM_PAGES_PER_EBLOCK)
    {
       return FTL_ERR_GC_IS_MOVED;
    }
    if( GCMoveArray[pageAddr] == FTL_FALSE)
    {
       (*isMoved) = FTL_FALSE;
    }
    else
    {
       (*isMoved) = FTL_TRUE;
    }
    return FTL_ERR_PASS;
}

//--------------------------------------------
FTL_STATUS FTL_SwapDataReserveEBlock(FTL_DEV devID, UINT16 logicalPageNum, 
    UINT16_PTR ptrPhyReservedBlock, UINT16_PTR ptrLogicalReservedBlock, UINT8 WLflag, UINT8 badBlockFlagIn) 
{
    UINT32 tempEraseCount = 0;                         /*2*/
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
    UINT8 eraseStatus = FTL_TRUE;                      /*1*/
    #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

    UINT16 phyReservedEBlock = EMPTY_WORD;             /*2*/
    UINT16 pickedEB = EMPTY_WORD;                      /*2*/
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16  freePageIndex = EMPTY_WORD;
    UINT8 badEBlockFlag = FTL_FALSE;                   /*1*/
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 chainEBNum = EMPTY_WORD;
    UINT16 phyChainEBNum = EMPTY_WORD;
    #endif
    #endif
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    CHAIN_LOG_ENTRY chainLogEntry;                     /*16*/
    FLASH_PAGE_INFO flashPageInfo = {0, 0, {0, 0}};    /*11*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT8 count = 0;                                   /*1*/

    if((status = TABLE_GetReservedEB(devID, &pickedEB, WLflag)) != FTL_ERR_PASS)
    {
       return FTL_ERR_ECHAIN_SETUP_SANITY1;
    }

    phyReservedEBlock = GetPhysicalEBlockAddr(devID, pickedEB);

    #if DEBUG_BLOCK_SELECT
    DBG_Printf("Swap: pickedEB = 0x%X, ", pickedEB, 0);
    DBG_Printf("phyReservedEBlock = 0x%X\n", phyReservedEBlock, 0);
    #endif  // #if DEBUG_BLOCK_SELECT

    tempEraseCount = GetEraseCount(devID, pickedEB);
    if(logicalPageNum != EMPTY_WORD)
    {

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(badBlockFlagIn == FTL_FALSE)
       {
          StoreSourceBadBlockInfo(devID, logicalPageNum, FTL_ERR_DATA_RESERVE);
          StoreTargetBadBlockInfo(devID, pickedEB, FTL_ERR_DATA_RESERVE);
       }
       #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       SetPhysicalEBlockAddr(devID, pickedEB, GetPhysicalEBlockAddr(devID, logicalPageNum));
       SetEraseCount(devID, pickedEB, GetEraseCount(devID, logicalPageNum));
       SetPhysicalEBlockAddr(devID, logicalPageNum, phyReservedEBlock);
       SetEraseCount(devID, logicalPageNum, tempEraseCount);
       MarkEBlockMappingTableEntryDirty(devID, pickedEB);
       MarkEBlockMappingTableEntryDirty(devID, logicalPageNum);
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
       tempEraseCount = GetTrueEraseCount(devID, pickedEB);
       status = SetSaveStaticWL(devID, pickedEB, tempEraseCount);
       if (status != FTL_ERR_PASS)
       {
          return status;
       }
       tempEraseCount = GetTrueEraseCount(devID, logicalPageNum);
       status = SetSaveStaticWL(devID, logicalPageNum, tempEraseCount);
       if (status != FTL_ERR_PASS)
       {
          return status;
       }
       #endif
       #endif

       #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       badEBlockFlag = GetBadEBlockStatus(devID, logicalPageNum);             
       SetBadEBlockStatus(devID, logicalPageNum, GetBadEBlockStatus(devID, pickedEB));
       SetBadEBlockStatus(devID, pickedEB, badEBlockFlag);
       if(badBlockFlagIn == FTL_TRUE)
       {
          #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
          chainEBNum = GetChainLogicalEBNum(devID, logicalPageNum);
          if(chainEBNum != EMPTY_WORD) 
          {
              // its chained, the to informaiton of current logical EB, is valid, both logical and physical, the logical of the from EB is also valid, the physical of the from EB has to be updated.
              phyChainEBNum = GetChainPhyEBNum(devID, chainEBNum);
              SetChainPhyEBNum(devID, chainEBNum, phyReservedEBlock);
              MarkEBlockMappingTableEntryDirty(devID, chainEBNum);
          }
          #endif
          *ptrLogicalReservedBlock = pickedEB;
          *ptrPhyReservedBlock = phyReservedEBlock;
          return status;
       }
       #endif

    }
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    else
    {
       if(((NUMBER_OF_SYSTEM_EBLOCKS - ((TABLE_GetReservedEBlockNum(devID) + 1) + gcSaveCount)) - TABLE_GetUsedSysEBCount(devID)) < NUM_CHAIN_EBLOCKS)
       {
          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(badBlockFlagIn == FTL_FALSE)
          {
             //check free page index
             freePageIndex = GetFreePageIndex(devID, *ptrLogicalReservedBlock);
             if(freePageIndex < NUM_PAGES_PER_EBLOCK)
             {
                 StoreSourceBadBlockInfo(devID, *ptrLogicalReservedBlock, FTL_ERR_CHAIN_NOT_FULL_EB);
                 StoreTargetBadBlockInfo(devID, pickedEB, FTL_ERR_CHAIN_NOT_FULL_EB);
             }
             else
             {
                 StoreSourceBadBlockInfo(devID, pickedEB, FTL_ERR_CHAIN_FULL_EB);
                 StoreTargetBadBlockInfo(devID, *ptrLogicalReservedBlock, FTL_ERR_CHAIN_FULL_EB);
             }
          }
          #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

          // Empty Blocks are available for chaining
          // Erase "To" EBlock

          #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
          eraseStatus = GetEBErased(devID, pickedEB);

          #if DEBUG_PRE_ERASED
          if(FTL_TRUE == eraseStatus)
          {
             DBG_Printf("FTL_SwapDataReserveEBlock: EBlock 0x%X is already erased\n", pickedEB, 0);
          }
          #endif  // #if DEBUG_PRE_ERASED
          if(FTL_FALSE == eraseStatus)
          #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

          {
             status = FTL_EraseOp(devID, pickedEB);
             if(FTL_ERR_PASS != status)
             {
                if(FTL_ERR_FAIL == status)
                {
                   return status;
                }
                #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                if(badBlockFlagIn == FTL_FALSE)
                {
                   StoreSourceBadBlockInfo(devID, pickedEB, FTL_ERR_CHAIN_FULL_EB);
                   StoreTargetBadBlockInfo(devID, *ptrLogicalReservedBlock, FTL_ERR_CHAIN_FULL_EB);
                }
                #endif   // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                return FTL_ERR_GC_ERASE1;
             }
          }
          // Clear the bit map and ppa tables of the chained-to EBlock
          SetDirtyCount(devID, pickedEB, 0);
          TABLE_ClearFreeBitMap(devID, pickedEB);
          TABLE_ClearPPATable(devID, pickedEB);
          MarkPPAMappingTableEntryDirty(devID, pickedEB, 0);
          MarkEBlockMappingTableEntryDirty(devID, pickedEB);
          SetChainLink(devID, *ptrLogicalReservedBlock, pickedEB, 
             *ptrPhyReservedBlock, phyReservedEBlock);
          /* CREATE A CHAIN LOG ENTRY */
          if((status = GetNextLogEntryLocation(devID, &flashPageInfo)) != FTL_ERR_PASS)
          {
             return status;
          }
          chainLogEntry.logicalFrom = *ptrLogicalReservedBlock;
          chainLogEntry.logicalTo = pickedEB;
          chainLogEntry.phyFrom = *ptrPhyReservedBlock;
          chainLogEntry.phyTo = phyReservedEBlock;
          for(count = 0;count < CHAIN_LOG_ENTRY_RESERVED; count++)
          {
             chainLogEntry.reserved[count] = EMPTY_BYTE;
          }
          chainLogEntry.type = CHAIN_LOG_TYPE;
          if((status = FTL_WriteLogInfo(&flashPageInfo, (UINT8_PTR)&chainLogEntry)) != FTL_ERR_PASS)
          {
             return status;
          }
            
          #if DEBUG_GC_BLOCKS
          DBG_Printf("Chain Log: logFrom = 0x%X, ", chainLogEntry.logicalFrom, 0);
          DBG_Printf("logTo = 0x%X, ", chainLogEntry.logicalTo, 0);
          DBG_Printf("phyFrom = 0x%X, ", chainLogEntry.phyFrom, 0);
          DBG_Printf("phyTo = 0x%X\n", chainLogEntry.phyTo, 0);
          #endif  // #if DEBUG_GC_BLOCKS

       }
       else
       {
          #if DEBUG_BLOCK_SELECT
          DBG_Printf("  Not enough Reserved EBlocks to setup a chain; Must GC instead\n", 0, 0);
          #endif  // #if DEBUG_BLOCK_SELECT

          if((status = TABLE_InsertReservedEB(devID, pickedEB)) != FTL_ERR_PASS)
          {
             return status;
          }

          return FTL_ERR_ECHAIN_GC_NEEDED;
       }
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    *ptrLogicalReservedBlock = pickedEB;
    *ptrPhyReservedBlock = phyReservedEBlock;
    return status;
}

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
//---------------------------------------
FTL_STATUS CopyPagesForDataGC(FTL_DEV devID, UINT16 logicalEBNum, UINT16 reserveEBNum, UINT16_PTR FreedUpPages, UINT8_PTR pageBitMap)
{
    FLASH_STATUS flashStatus = FLASH_PASS;             /*4*/
    UINT16 phyFromEBlock = EMPTY_WORD;                 /*2*/
    UINT16 phyToEBlock = EMPTY_WORD;                   /*2*/
    UINT16 logicalPageOffset = EMPTY_WORD;             /*2*/
    FREE_BIT_MAP_TYPE bitMap = 0;                      /*1*/
    UINT16 phyPageOffset = EMPTY_WORD;                 /*2*/
    UINT8  isMoved = 0;                                /*1*/
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 logChainToEB = EMPTY_WORD;                  /*2*/
    UINT16 phyChainToEB = EMPTY_WORD;                  /*2*/
    UINT16 chainFreePageIndex = EMPTY_WORD;            /*2*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    phyFromEBlock = GetPhysicalEBlockAddr(devID, reserveEBNum);
    phyToEBlock = GetPhysicalEBlockAddr(devID, logicalEBNum);

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    logChainToEB = GetChainLogicalEBNum(devID, logicalEBNum);
    if(logChainToEB != EMPTY_WORD)
    {
       phyChainToEB = GetChainPhyEBNum(devID, logicalEBNum);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    pageInfo.devID = devID;
    pageInfo.vPage.pageOffset = 0;
    pageInfo.byteCount = VIRTUAL_PAGE_SIZE;

    // Loop through bit map of "From" EBlock
    for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
    {
       isMoved = FTL_FALSE;
       if (FTL_TRUE == GCMoveArrayNotEmpty)
       {
          IsPageMoved(logicalPageOffset, &isMoved);
       }
       if (FTL_TRUE == isMoved)
       {
          // This page is being rewritten - do not copy
          // Clear both the Bit Map in the Block Info and PPA Tables
          UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, EMPTY_INVALID, BLOCK_INFO_EMPTY_PAGE);

          #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
          if(logChainToEB != EMPTY_WORD)
          {    /*clear the chain EB as well*/
             UpdatePageTableInfo(devID, logChainToEB, logicalPageOffset, 
                EMPTY_INVALID, BLOCK_INFO_EMPTY_PAGE);
          }
          #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

       }
       else
       {
          // convert logical page to physical page
          phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
          if ((EMPTY_INVALID != phyPageOffset) && (CHAIN_INVALID != phyPageOffset))
          {
             // Page must be valid - Copy Page
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyFromEBlock, phyPageOffset);
             flashStatus = FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if (FLASH_PASS != flashStatus)
             {
                return FTL_ERR_GC_PAGE_LOAD1;
             }
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyToEBlock, phyPageOffset);
             flashStatus = FLASH_RamPageWriteDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if (FLASH_PASS != flashStatus)
             {
                return FTL_ERR_GC_PAGE_WR1;
             }
             // Update bit map in GC log entry
             SetBitMapField(&pageBitMap[0], logicalPageOffset, 1, GC_MOVED_PAGE);
          }
       }
    }

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    // clear the chain info
    if(logChainToEB != EMPTY_WORD)
    {  
       ClearChainLink(devID, logicalEBNum, logChainToEB);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Update Block_Info table
    // Erase Count was written by FTL_SwapDataReserveEBlock()
    SetGCOrFreePageNum(devID, logicalEBNum, GCNum[devID]++);
    SetDirtyCount(devID, logicalEBNum, 0);
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);

    // Mark all stale pages as free
    for (phyPageOffset = 0; phyPageOffset < NUM_PAGES_PER_EBLOCK; phyPageOffset++)
    {
       bitMap = GetEBlockMapFreeBitIndex(devID, logicalEBNum, phyPageOffset);
       if (BLOCK_INFO_STALE_PAGE == bitMap)
       {
          *FreedUpPages = *FreedUpPages + 1;
          // Note: PPA table should not include this page
          SetEBlockMapFreeBitIndex(devID, logicalEBNum, phyPageOffset, BLOCK_INFO_EMPTY_PAGE);
       }       
    }

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    /*let do it again for the chained pages*/
    for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
    {
       isMoved = FTL_FALSE;
       if (FTL_TRUE == GCMoveArrayNotEmpty)
       {
          IsPageMoved(logicalPageOffset, &isMoved);
       }
       if (FTL_TRUE == isMoved)
       {  
          // already done this no, need to do it again
       }
       else
       {
          // convert logical page to physical page
          phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
          if (CHAIN_INVALID == phyPageOffset)
          {
             // Page is in chained EB, get it from there
             phyPageOffset = GetPPASlot(devID, logChainToEB, logicalPageOffset);
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyChainToEB, phyPageOffset);
             *FreedUpPages = *FreedUpPages - 1;
             flashStatus = FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if (FLASH_PASS != flashStatus)
             {
                return FTL_ERR_GC_PAGE_LOAD2;
             }
             /*get a index to write to*/
             chainFreePageIndex = GetFreePageIndex(devID, logicalEBNum);
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyToEBlock, chainFreePageIndex);
             flashStatus = FLASH_RamPageWriteDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if (FLASH_PASS != flashStatus)
             {
                return FTL_ERR_GC_PAGE_WR2;
             }
             UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
                chainFreePageIndex, BLOCK_INFO_VALID_PAGE);  
             /*clear the chained EB status*/
             UpdatePageTableInfo(devID, logChainToEB, logicalPageOffset,
                EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
             // Update bit map in GC log entry
             SetBitMapField(&pageBitMap[0], logicalPageOffset, 1, GC_MOVED_PAGE);
          }
       }
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Update dirty bit in Block_Info table
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    return FTL_ERR_PASS;
}

#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
//---------------------------------------
FTL_STATUS CopyPagesForDataGC(FTL_DEV devID, UINT16 logicalEBNum, UINT16 reserveEBNum, UINT16_PTR FreedUpPages, UINT8_PTR pageBitMap)
{
    UINT16 phyFromEBlock = EMPTY_WORD;                 /*2*/
    UINT16 phyToEBlock = EMPTY_WORD;                   /*2*/
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 phyPageOffset = 0;                          /*2*/
    UINT16 freePageIndex = 0;                          /*2*/
    UINT8  isMoved = 0;                                /*1*/
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 logChainEBNum = EMPTY_WORD;                 /*2*/
    UINT16 phyChainEBNum = EMPTY_WORD;                 /*2*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    phyFromEBlock = GetPhysicalEBlockAddr(devID, reserveEBNum);
    phyToEBlock = GetPhysicalEBlockAddr(devID, logicalEBNum);

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    logChainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
    if(logChainEBNum != EMPTY_WORD)
    {
       phyChainEBNum = GetChainPhyEBNum(devID, logicalEBNum);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    TABLE_ClearFreeBitMap(devID, logicalEBNum);

    pageInfo.devID = devID;
    pageInfo.vPage.pageOffset = 0;
    pageInfo.byteCount = NUMBER_OF_BYTES_PER_PAGE;

    // Loop through bit map of "From" EBlock
    for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
    {
       isMoved = FTL_FALSE;
       if(GCMoveArrayNotEmpty == FTL_TRUE)
       {
          IsPageMoved(pageOffset, &isMoved);
       }
       if(isMoved == FTL_TRUE)
       {
          *FreedUpPages = *FreedUpPages + 1;
          SetPPASlot(devID, logicalEBNum, pageOffset, EMPTY_INVALID);
       }
       else
       {
          // convert logical page to physical page
          phyPageOffset = GetPPASlot(devID, logicalEBNum, pageOffset);
          if(phyPageOffset == EMPTY_INVALID)
          {
             *FreedUpPages = *FreedUpPages + 1;
          }
          else
          {
             // Page must be valid - Copy Page
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyFromEBlock, phyPageOffset);

             #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
             if(phyPageOffset == CHAIN_INVALID)
             {
                phyPageOffset = GetPPASlot(devID, logChainEBNum, pageOffset);
                pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyChainEBNum, phyPageOffset);
             }
             #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

             if(FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devID][0]) != FLASH_PASS)
             {
                return FTL_ERR_GC_PAGE_LOAD1;
             }
             freePageIndex = GetFreePageIndex(devID, logicalEBNum);
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyToEBlock, freePageIndex);
             flashStatus = FLASH_RamPageWriteDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if(flashStatus != FLASH_PASS)
             {
                if(flashStatus == FLASH_PARAM)
                {
                   return FTL_ERR_FAIL;
                }
                return FTL_ERR_GC_PAGE_WR1;
             }
             UpdatePageTableInfo(devID, logicalEBNum, pageOffset, freePageIndex, BLOCK_INFO_VALID_PAGE);
             // Update bit map in GC log entry
             SetBitMapField(&pageBitMap[0], pageOffset, 1, GC_MOVED_PAGE);
          }
       }
    }

    // Update Block_Info table
    // Erase Count was written by FTL_SwapDataReserveEBlock()
    SetGCOrFreePageNum(devID, logicalEBNum, GCNum[devID]++);
    SetDirtyCount(devID, logicalEBNum, 0);

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if(logChainEBNum != EMPTY_WORD)
    {
       ClearChainLink(devID, logicalEBNum, logChainEBNum);
       SetDirtyCount(devID, logChainEBNum, 0);
       MarkEBlockMappingTableEntryDirty(devID, logChainEBNum);
       MarkAllPagesStatus(devID, logChainEBNum, BLOCK_INFO_STALE_PAGE);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Update dirty bit in Block_Info table
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

//---------------------------------------
FTL_STATUS DataGC(FTL_DEV devID, UINT16 logicalEBNum,
    UINT16_PTR FreedUpPages, UINT16_PTR freePageIndex, UINT8 WLflag ) 
{
    GC_LOG_ENTRY gcLog;
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    UINT16 phyFromEBlock = EMPTY_WORD;                 /*2*/
    UINT16 phyToEBlock = EMPTY_WORD;                   /*2*/
    UINT16 count = 0;                                  /*2*/
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    UINT8  useGcInfo = 0;                              /*1*/
    UINT16 logToEBlock = EMPTY_WORD;                   /*2*/
    UINT8 pageBitMap[GC_MOVE_BITMAP];
    UINT8 typeCount = 0;                               /*1*/
    UINT8 checkFlag = FTL_FALSE;

    #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
    UINT16 logSwapEBlock = EMPTY_WORD;                 /*2*/
    #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 chainEBNum = EMPTY_WORD;                    /*2*/
    CHAIN_INFO chainInfo = {0, 0, 0, 0, 0};            /*10*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    *FreedUpPages = 0;
    *freePageIndex = 0;
    useGcInfo = FTL_FALSE;
    if ((EMPTY_BYTE == devID) || (EMPTY_WORD == logicalEBNum))
    {
       useGcInfo = FTL_TRUE;
       devID = GC_Info.devID;
       logicalEBNum = GC_Info.logicalEBlock;
    }
    if((status = FTL_CheckForGCLogSpace(devID)) != FTL_ERR_PASS)
    {
          return status;
    }

    #if ((DEBUG_FTL_API_ANNOUNCE == 1) || (DEBUG_GC_ANNOUNCE == 1))
    DBG_Printf("FTL_ForcedGC: useGcInfo = %d, ", useGcInfo, 0);
    DBG_Printf("devID = %d, ", devID, 0);
    DBG_Printf("logicalEBNum = 0x%X\n", logicalEBNum, 0);
    #endif  // #if (DEBUG_FTL_API_ANNOUNCE == 1 || DEBUG_GC_ANNOUNCE == 1)

    #if DEBUG_CHECK_TABLES
    status = DBG_CheckPPAandBitMap(devID, logicalEBNum);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    #endif  // #if DEBUG_CHECK_TABLES

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    chainInfo.isChained = FTL_FALSE;
    chainInfo.devID = EMPTY_BYTE;
    chainInfo.logChainToEB = EMPTY_WORD;
    chainInfo.phyChainToEB = EMPTY_WORD;
    chainInfo.phyPageAddr = EMPTY_WORD;
    chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
    if(chainEBNum != EMPTY_WORD) 
    {
       chainInfo.isChained = FTL_TRUE;
       chainInfo.logChainToEB = chainEBNum;
       chainInfo.phyChainToEB = GetChainPhyEBNum(devID, logicalEBNum);
        
       #if (DEBUG_FTL_API_ANNOUNCE == 1 || DEBUG_GC_ANNOUNCE == 1)
       DBG_Printf("  logChainToEB = 0x%X, ", chainEBNum, 0);
       DBG_Printf("phyChainToEB = 0x%X\n", chainInfo.phyChainToEB, 0);
       #endif  // #if (DEBUG_FTL_API_ANNOUNCE == 1 || DEBUG_GC_ANNOUNCE == 1)

    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    phyFromEBlock = GetPhysicalEBlockAddr(devID, logicalEBNum);
    // Identify EB to copy to, not chaining, regular GC, so swap will not erase, nor create a log, no need to worry about bad block
    if((status = FTL_SwapDataReserveEBlock(devID, logicalEBNum, &phyToEBlock, &logToEBlock, WLflag, FTL_FALSE)) != FTL_ERR_PASS)
    {
       return status;
    }
    
    #if DEBUG_GC_BLOCKS
    DBG_Printf("Forced_GC: phyFromEBlock = 0x%X, ", phyFromEBlock, 0); 
    DBG_Printf("phyToEBlock = 0x%X", phyToEBlock, 0);

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if(chainEBNum != EMPTY_WORD) 
    {
       DBG_Printf(",  phyChainEB = 0x%X", chainInfo.phyChainToEB, 0);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    
    DBG_Printf("\n", 0, 0);
    #endif  // #if DEBUG_GC_BLOCKS

     // initialize page bitmap
    for (count = 0; count < GC_MOVE_BITMAP; count++)
    {
       pageBitMap[count] = 0;
    }
    
    // Create and initialize GC log entry
    gcLog.partA.GCNum = GCNum[devID];
    gcLog.partA.type = GC_TYPE_A;
    for (count = 0; count < sizeof(gcLog.partA.reserved); count++)
    {
       gcLog.partA.reserved[count] = EMPTY_BYTE;
    }
    gcLog.partA.holdForMerge = GCMoveArrayNotEmpty;
    gcLog.partA.logicalEBAddr = logicalEBNum;
    gcLog.partA.reservedEBAddr = logToEBlock;
    for (typeCount = 0; typeCount < NUM_GC_TYPE_B; typeCount++)
    {
       gcLog.partB[typeCount].type = GC_TYPE_B;
       for (count = 0; count < sizeof(gcLog.partB[typeCount].pageMovedBitMap); count++)
       {
          gcLog.partB[typeCount].pageMovedBitMap[count] = 0;
       }
       for (count = 0; count < sizeof(gcLog.partB[typeCount].reserved); count++)
       {
          gcLog.partB[typeCount].reserved[count] = EMPTY_BYTE;
       }
       gcLog.partB[typeCount].checkWord = EMPTY_WORD;
    }

    // Write GC log entry to flash
    status = GetNextLogEntryLocation(devID, &pageInfo);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    status = FTL_WriteLogInfo(&pageInfo, (UINT8_PTR)&gcLog.partA);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }    

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    eraseStatus = GetEBErased(devID, logicalEBNum);

    #if DEBUG_PRE_ERASED
    if(FTL_TRUE == eraseStatus)
    {
       DBG_Printf("FTL_InternalForcedGC: EBlock 0x%X is already erased\n", logicalEBNum, 0);
    }
    #endif  // #if DEBUG_PRE_ERASED

    if(FTL_FALSE == eraseStatus)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    {
       status = FTL_EraseOp(devID, logicalEBNum);
       if (FTL_ERR_PASS != status)
       {
          if (FTL_ERR_FAIL == status)
          {
             return status;
          }
          return FTL_ERR_GC_ERASE2;
       }
    }

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, logicalEBNum, FTL_FALSE); /*since the EB will be written to now*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);

    status = CopyPagesForDataGC(devID, logicalEBNum, logToEBlock, FreedUpPages, &pageBitMap[0]);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }

    // copy page bitmap to GC_TYPE_B
    for (typeCount = 0; typeCount < NUM_GC_TYPE_B; typeCount++)
    {
       MEM_Memcpy(&gcLog.partB[typeCount].pageMovedBitMap[0], \
                  &pageBitMap[typeCount * NUM_ENTRIES_GC_TYPE_B], \
                  NUM_ENTRIES_GC_TYPE_B);
    }

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    // Write GC log entry to flash
    for (typeCount = 0; typeCount < NUM_GC_TYPE_B; typeCount++)
    {
       status = GetNextLogEntryLocation(devID, &pageInfo);
       if (FTL_ERR_PASS != status)
       {
          return status;
       }
       status = FTL_WriteLogInfo(&pageInfo, (UINT8_PTR)&gcLog.partB[typeCount]);
       if (FTL_ERR_PASS != status)
       {
          return status;
       }
    }

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    // Pack GC log type A entry to flash
    if((status = InitPackedLogs()) != FTL_ERR_PASS)
    {
       return status;
    }

    // Pack GC log type B entry to flash
    for (typeCount = 0; typeCount < NUM_GC_TYPE_B; typeCount++)
    {
       status = CopyPackedLogs(typeCount, (UINT8_PTR)&gcLog.partB[typeCount]);
       if (FTL_ERR_PASS != status)
       {
          return status;
       }
    }
    // Write GC log entry to flash
    status = GetNextLogEntryLocation(devID, &pageInfo);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    status = WritePackedLogs(&pageInfo);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    #if DEBUG_CHECK_TABLES
    status = DBG_CheckPPAandBitMap(devID, logicalEBNum);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    #endif  // #if DEBUG_CHECK_TABLES
    
    if(FTL_DeleteFlag == FTL_FALSE)
    {
       if ((FTL_TRUE == GCMoveArrayNotEmpty) || (EMPTY_WORD != gcSave[0].phyEbNum))
       {
          // Save the EB if it is to be merged or if a previous EB has been saved
          status = FTL_AddToGCSave(devID, phyFromEBlock);
          if (status != FTL_ERR_PASS)
          {
             return status;
          }

          #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
          if(chainEBNum != EMPTY_WORD) 
          {
             // this is a chained EB, lets check teh phyPageAddr
             // Save the EB if it is to be merged or if a previous EB has been saved
             status = FTL_AddToGCSave(devID, chainInfo.phyChainToEB);
             if (status != FTL_ERR_PASS)
             {
                return status;
             }
          }
          #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
          checkFlag = FTL_TRUE;
       }
    }

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    if(FTL_FALSE == gProtectForSuperSysEBFlag)
    {
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
    if((status = FindAndSwapUnusedEB(devID, logToEBlock, &logSwapEBlock)) != FTL_ERR_PASS)
    {
       return status;
    }
    if (EMPTY_WORD != logSwapEBlock)
    {
       status = CreateSwapEBLog(devID, logSwapEBlock, logToEBlock);
       if (FTL_ERR_PASS != status)
       {
          return status;
       }
    }
    
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if(chainInfo.isChained == FTL_TRUE)
    { 
       if((status = FindAndSwapUnusedEB(devID, chainInfo.logChainToEB, &logSwapEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
       if (EMPTY_WORD != logSwapEBlock)
       {
          status = CreateSwapEBLog(devID, logSwapEBlock, chainInfo.logChainToEB);
          if (FTL_ERR_PASS != status)
          {
             return status;
          }
       }
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    }
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    if(FTL_FALSE == checkFlag)
    {
       if((status = TABLE_InsertReservedEB(devID, logToEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
       #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
       if(chainEBNum != EMPTY_WORD) 
       {
          if((status = TABLE_InsertReservedEB(devID, chainInfo.logChainToEB)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       #endif
    }
    return FTL_ERR_PASS;
}

#if(FTL_UNLINK_GC == FTL_TRUE)
//---------------------------------------
FTL_STATUS CopyPagesForUnlinkGC(FTL_DEV devID, UINT16 logFromEBNum, UINT16 logToEBNum, UINT8_PTR pageBitMap)
{
    UINT16 phyFromEBNum = EMPTY_WORD;                  /*2*/
    UINT16 phyToEBNum = EMPTY_WORD;                    /*2*/
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 phyPageOffset = 0;                          /*2*/
    UINT16 freePageIndex = 0;                          /*2*/
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    FREE_BIT_MAP_TYPE bitMap = 0;                      /*1*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    phyFromEBNum = GetPhysicalEBlockAddr(devID, logFromEBNum);
    phyToEBNum = GetPhysicalEBlockAddr(devID, logToEBNum);

    pageInfo.devID = devID;
    pageInfo.byteCount = NUMBER_OF_BYTES_PER_PAGE;
    pageInfo.vPage.pageOffset = 0;

    // Loop through bit map of "From" EBlock
    for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
    {
       bitMap = GetBitMapField(&pageBitMap[0], pageOffset, 1);
       if(bitMap == GC_MOVED_PAGE)
       {
          // Page must be valid - Copy Page
          phyPageOffset = GetPPASlot(devID, logFromEBNum, pageOffset);
          pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyFromEBNum, phyPageOffset);
          if(FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devID][0]) != FLASH_PASS)
          {
             return FTL_ERR_FLASH_LOAD_02;
          }
          /*get a index to write to*/
          freePageIndex = GetFreePageIndex(devID, logToEBNum);
          pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyToEBNum, freePageIndex);
          flashStatus = FLASH_RamPageWriteDataBlock(&pageInfo, &pseudoRPB[devID][0]);
          if(flashStatus != FLASH_PASS)
          {
             if(flashStatus == FLASH_PARAM)
             {
                return FTL_ERR_FAIL;
             }
             return FTL_ERR_FLASH_COMMIT_06;
          }
          /* update chained-to EBlock status */
          UpdatePageTableInfo(devID, logToEBNum, pageOffset, freePageIndex, BLOCK_INFO_VALID_PAGE);
          /* clear chained-from EBlock status */
          UpdatePageTableInfo(devID, logFromEBNum, pageOffset, EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
       }
    }
    return FTL_ERR_PASS;
}

//---------------------------------------
FTL_STATUS UnlinkGC(FTL_DEV devID, UINT16 fromLogicalEBlock)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 fromPhysicalEBlock = EMPTY_WORD;            /*2*/
    UINT16 toLogicalEBlock = EMPTY_WORD;               /*2*/
    UINT16 toPhysicalEBlock = EMPTY_WORD;              /*2*/
    UINT16 validPages = 0;                             /*2*/
    UINT16 freePages = 0;                              /*2*/
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 phyPageOffset = 0;                          /*2*/
    UINT16 count = 0;                                  /*2*/
    UINT16 typeCount = 0;                              /*2*/
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    UNLINK_LOG_ENTRY unlinkLog;
    UINT8 pageBitMap[GC_MOVE_BITMAP];
    UINT8 checkFlag = FTL_FALSE;                       /*1*/

    #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
    UINT16 logSwapEBlock = EMPTY_WORD;                 /*2*/
    #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

    if((status = FTL_CheckForGCLogSpace(devID)) != FTL_ERR_PASS)
    {
       return status;
    }

    fromPhysicalEBlock = GetPhysicalEBlockAddr(devID, fromLogicalEBlock);
    #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
    toLogicalEBlock = GetChainLogicalEBNum(devID, fromLogicalEBlock);
    toPhysicalEBlock = GetChainPhyEBNum(devID, fromLogicalEBlock);
    #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    StoreSourceBadBlockInfo(devID, toLogicalEBlock, FTL_ERR_DATA_RESERVE);
    StoreTargetBadBlockInfo(devID, fromLogicalEBlock, FTL_ERR_DATA_RESERVE);
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    for(count = 0; count < GC_MOVE_BITMAP; count++)
    {
       pageBitMap[count] = 0;
    }
    unlinkLog.partA.checkWord = EMPTY_WORD;
    unlinkLog.partA.fromLogicalEBAddr = fromLogicalEBlock;
    unlinkLog.partA.toLogicalEBAddr = toLogicalEBlock;
    for(count = 0; count < sizeof(unlinkLog.partA.reserved); count++)
    {
       unlinkLog.partA.reserved[count] = EMPTY_BYTE;
    }
    for(typeCount = 0; typeCount < NUM_UNLINK_TYPE_B; typeCount++)
    {
       unlinkLog.partB[typeCount].checkWord = EMPTY_WORD;
       unlinkLog.partB[typeCount].type = UNLINK_LOG_TYPE_B;
       for(count = 0; count < sizeof(unlinkLog.partB[typeCount].pageMovedBitMap); count++)
       {
          unlinkLog.partB[typeCount].pageMovedBitMap[count] = 0;
       }
       for(count = 0; count < sizeof(unlinkLog.partB[typeCount].reserved); count++)
       {
          unlinkLog.partB[typeCount].reserved[count] = EMPTY_BYTE;
       }
    }

    validPages = GetNumValidPages(devID, fromLogicalEBlock);
    freePages = GetNumFreePages(devID, toLogicalEBlock);

    if(validPages == 0)
    {
       if((status = UnlinkChain(devID, fromLogicalEBlock, toLogicalEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }

       unlinkLog.partA.type = UNLINK_LOG_TYPE_A1;
       if((status = GetNextLogEntryLocation(devID, &pageInfo)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = FTL_WriteLogInfo(&pageInfo, (UINT8_PTR)&unlinkLog.partA)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    else if(freePages >= validPages)
    {
       unlinkLog.partA.type = UNLINK_LOG_TYPE_A2;
       for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
       {
          phyPageOffset = GetPPASlot(devID, fromLogicalEBlock, pageOffset);
          if((phyPageOffset != EMPTY_INVALID) && (phyPageOffset != CHAIN_INVALID))
          {
             SetBitMapField(&pageBitMap[0], pageOffset, 1, GC_MOVED_PAGE);
          }
       }
       for(count = 0; count < NUM_UNLINK_TYPE_B; count++)
       {
          MEM_Memcpy(&unlinkLog.partB[count].pageMovedBitMap[0], &pageBitMap[count*NUM_ENTRIES_UNLINK_TYPE_B], NUM_ENTRIES_UNLINK_TYPE_B);
       }

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       if((status = GetNextLogEntryLocation(devID, &pageInfo)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = FTL_WriteLogInfo(&pageInfo, (UINT8_PTR)&unlinkLog.partA)) != FTL_ERR_PASS)
       {
          return status;
       }
       for(count = 0; count < NUM_UNLINK_TYPE_B; count++)
       {
          if((status = GetNextLogEntryLocation(devID, &pageInfo)) != FTL_ERR_PASS)
          {
             return status;
          }
          if((status = FTL_WriteLogInfo(&pageInfo, (UINT8_PTR)&unlinkLog.partB[count])) != FTL_ERR_PASS)
          {
             return status;
          }
       }

       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       if((status = InitPackedLogs()) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = CopyPackedLogs(0, (UINT8_PTR)&unlinkLog.partA)) != FTL_ERR_PASS)
       {
          return status;
       }
       for(count = 0; count < NUM_UNLINK_TYPE_B; count++)
       {
          if((status = CopyPackedLogs((count + 1), (UINT8_PTR)&unlinkLog.partB[count])) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       if((status = GetNextLogEntryLocation(devID, &pageInfo)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = WritePackedLogs(&pageInfo)) != FTL_ERR_PASS)
       {
          return status;
       }
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       if((status = CopyPagesForUnlinkGC(devID, fromLogicalEBlock, toLogicalEBlock, &pageBitMap[0])) != FTL_ERR_PASS)
       {
          return status;
       }

       if((status = UnlinkChain(devID, fromLogicalEBlock, toLogicalEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
    }

    if(FTL_DeleteFlag == FTL_FALSE)
    {
       // Save the EB if a previous EB has been saved
       if((status = FTL_AddToGCSave(devID, fromPhysicalEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
       checkFlag = FTL_TRUE;
    }

    #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
    if((status = FindAndSwapUnusedEB(devID, toLogicalEBlock, &logSwapEBlock)) != FTL_ERR_PASS)
    {
       return status;
    }
    if(logSwapEBlock != EMPTY_WORD)
    {
       if((status = CreateSwapEBLog(devID, logSwapEBlock, toLogicalEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

    if(FTL_FALSE == checkFlag)
    {
       if((status = TABLE_InsertReservedEB(devID, toLogicalEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
    }

    return FTL_ERR_PASS;
}
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

//---------------------------------------
FTL_STATUS FTL_InternalForcedGC(FTL_DEV devID, UINT16 logicalEBNum,
    UINT16_PTR FreedUpPages, UINT16_PTR freePageIndex, UINT8 WLflag ) 
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    #if(FTL_UNLINK_GC == FTL_TRUE)
    FTL_DEV inDevID = 0;                               /*1*/
    UINT16 inLogicalEBlock = EMPTY_WORD;               /*2*/
    UINT16 chainEBNum = EMPTY_WORD;                    /*2*/
    UINT16 validPages = 0;                             /*2*/
    UINT16 freePages = 0;                              /*2*/

    if(devID == EMPTY_BYTE)
    {
       inDevID = GC_Info.devID;
    }
    else
    {
       inDevID = devID;
    }

    if(logicalEBNum == EMPTY_WORD)
    {
       inLogicalEBlock = GC_Info.logicalEBlock;
    }
    else
    {
       inLogicalEBlock = logicalEBNum;
    }

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    if(FTL_FALSE == gProtectForSuperSysEBFlag)
    {
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
    chainEBNum = GetChainLogicalEBNum(inDevID, inLogicalEBlock);
    #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
    if((WLflag == FTL_FALSE) && (GCMoveArrayNotEmpty == FTL_FALSE) && (chainEBNum != EMPTY_WORD))
    {
       validPages = GetNumValidPages(inDevID, inLogicalEBlock);
       freePages = GetNumFreePages(inDevID, chainEBNum);
       if((validPages == 0) || (freePages >= validPages))
       {
          if((status = UnlinkGC(inDevID, inLogicalEBlock)) != FTL_ERR_PASS)
          {
             return status;
          }
          return FTL_ERR_PASS;
       }
    }
    
    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    }
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    #endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

    if((status = DataGC(devID, logicalEBNum, FreedUpPages, freePageIndex, WLflag)) != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS FTL_ForcedGC(FTL_DEV devID, UINT16 logicalEBNum,
    UINT16_PTR FreedUpPages, UINT16_PTR freePageIndex ) 
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;
    #endif
    #endif

    #if (DEBUG_ENABLE_LOGGING == FTL_TRUE)
    if((status = DEBUG_InsertLog((UINT32)logicalEBNum, EMPTY_DWORD, DEBUG_LOG_FORCED_GC)) != FTL_ERR_PASS)
    {
       return(status);
    }    
    #endif

    if((status = FTL_CheckPointer(FreedUpPages)) != FTL_ERR_PASS)
    {
       return status;
    }
    if((status = FTL_CheckPointer(freePageIndex)) != FTL_ERR_PASS)
    {
       return status;
    }
    if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
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
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    if(FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum , CACHE_WRITE_TYPE)))
    {
       return status;
    }
    #endif

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    status = InternalForcedGCWithBBManagement(devID, logicalEBNum, FreedUpPages, freePageIndex, FTL_FALSE);

    #else
    status = FTL_InternalForcedGC(devID, logicalEBNum, FreedUpPages, freePageIndex, FTL_FALSE);
    #endif

    if(status != FTL_ERR_PASS)
    {
       FTL_ClearMTLockBit();
       return status;
    }

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    if(GetTransLogEBFailedBadBlockInfo() == FTL_TRUE)
    {
       if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
       {
          FTL_ClearMTLockBit();
          return status;
       }
    }
    ClearTransLogEBBadBlockInfo();
    #endif

    FTL_ClearMTLockBit();
    FTL_UpdatedFlag = UPDATED_DONE;
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS Flush_GC(FTL_DEV devID)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 logicalEBNum = 0;                           /*2*/
    UINT16 physicalEBNum = EMPTY_WORD;                 /*2*/
    UINT16 logGCEBNum[MIN_FLUSH_GC_EBLOCKS];
    UINT32 latestIncNumber = 0;                        /*4*/
    FLASH_PAGE_INFO flashPageInfo = {0, 0, {0, 0}};    /*11*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    SYS_EBLOCK_INFO_PTR sysTempPtr = NULL;             /*4*/
    UINT16 bitCount = 0;                               /*2*/
    INT16 eBlockCount = 0;                             /*2*/
    UINT32 key = 0;                                    /*4*/
    KEY_TABLE_ENTRY flushEBNum[NUM_FLUSH_LOG_EBLOCKS]; /*16*/
    UINT16 flushEBCount = 0;                           /*2*/

    FLASH_STATUS flashStatus = FLASH_PASS;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT8 present = EMPTY_BYTE;
    UINT16 index = EMPTY_WORD;
    UINT8 dependency = EMPTY_BYTE;
    CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo = { 0, 0 };
    FLASH_PAGE_INFO flushStructPageInfo = {0, 0, {0, 0}};
    FLASH_PAGE_INFO flushRAMTablePageInfo = {0, 0, {0, 0}};
    UINT8 tempCache[FLUSH_RAM_TABLE_SIZE];
    UINT8 count = 0;
    SYS_EBLOCK_FLUSH_INFO sysEBlockFlushInfo;
    UINT16 flushEB = 0;
    UINT8 flushTypeCnt = 0;
    UINT16 phyEBlockAddrTmp = 0;
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT16_PTR flushInfoEntry = NULL;                       /*4*/
    UINT8 tempArray[SECTOR_SIZE];
    UINT16 count2 = 0;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    #endif /* #if (CACHE_RAM_BD_MODULE == FTL_TRUE) */
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;                          /*2*/
    UINT8 checkBBMark = FTL_FALSE;                   /*1*/
    UINT16 storeTemp[MIN_FLUSH_GC_EBLOCKS];          /*2*/
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    latestIncNumber = GetFlushEBCounter(devID);
    for(eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
    {
       flushEBNum[eBlockCount].logicalEBNum = EMPTY_WORD;
       flushEBNum[eBlockCount].phyAddr = EMPTY_WORD;
       flushEBNum[eBlockCount].key = EMPTY_DWORD;
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       flushEBNum[eBlockCount].cacheNum = EMPTY_BYTE;
       #endif
    }
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)    
    for(eBlockCount = 0; eBlockCount < MIN_FLUSH_GC_EBLOCKS; eBlockCount++)
    {
       storeTemp[eBlockCount] = EMPTY_WORD;
    }
    #endif
    for(eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetFlushLogEntry(devID, eBlockCount, &logicalEBNum, &physicalEBNum, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key != EMPTY_DWORD)
       {
          flushEBNum[flushEBCount].logicalEBNum = logicalEBNum;
          flushEBNum[flushEBCount].phyAddr = physicalEBNum;
          flushEBNum[flushEBCount].key = key;
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          TABLE_GetFlushLogCacheEntry(devID, physicalEBNum, &flushEBNum[flushEBCount].cacheNum);
          #endif
          /* Makes all flush entries of target EB occupied, in case that the tail of flush entries in the EB are skipped.
          This is the case that available entries are smaller than the required entries. */
          SetGCOrFreePageNum(devID, logicalEBNum, MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK);
          flushEBCount++;
       }
    }
    if((status = TABLE_FlushEBClear(devID)) != FTL_ERR_PASS)
    {
       return status;
    }
    for(eBlockCount = 0; eBlockCount < MIN_FLUSH_GC_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetReservedEB(devID, &logicalEBNum, FTL_FALSE)) != FTL_ERR_PASS)
       {
          return status;
       }
       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if((FTL_FALSE == SuperEBInfo[devID].checkLost) && (FTL_FALSE == SuperEBInfo[devID].checkSuperPF) && (FTL_FALSE == SuperEBInfo[devID].checkSysPF))
       {
          if((status = FTL_CreateSuperSysEBLog(devID, SYS_EBLOCK_INFO_CHANGED)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       storeTemp[eBlockCount] = logicalEBNum;
       #endif
       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       eraseStatus = GetEBErased(devID, logicalEBNum);

       #if DEBUG_PRE_ERASED
       if(FTL_TRUE == eraseStatus)
       {
          DBG_Printf("Flush_GC: EBlock 0x%X is already erased\n", logicalEBNum, 0);
       }
       #endif  // #if DEBUG_PRE_ERASED
       if(FTL_FALSE == eraseStatus)
       #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

       {
          if((status = FTL_EraseOp(devID, logicalEBNum)) != FTL_ERR_PASS)
          {
             #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             if(status == FTL_ERR_FAIL)
             {
                return status;
             }
             SetBadEBlockStatus(devID, logicalEBNum, FTL_TRUE);
             flashPageInfo.devID = devID;
             physicalEBNum = GetPhysicalEBlockAddr(devID, logicalEBNum);
             flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(physicalEBNum, 0);
             flashPageInfo.vPage.pageOffset = 0;
             flashPageInfo.byteCount = 0;
             if(FLASH_MarkDefectEBlock(&flashPageInfo) != FLASH_PASS)
             {
                // do nothing, just try to mark bad, even if it fails we move on.
             }
             for(eBlockCount = 0; eBlockCount < MIN_FLUSH_GC_EBLOCKS; eBlockCount++)
             {
               if(EMPTY_WORD != storeTemp[eBlockCount])
               {
                  TABLE_InsertReservedEB(devID,storeTemp[eBlockCount]);
               }
             }
             if((status = TABLE_FlushEBClear(devID)) != FTL_ERR_PASS)
             {
                return status;
             }
             for(eBlockCount = 0; eBlockCount < flushEBCount; eBlockCount++)
             {
                status = TABLE_FlushEBInsert(devID, flushEBNum[eBlockCount].logicalEBNum, 
                      flushEBNum[eBlockCount].phyAddr, flushEBNum[eBlockCount].key);
                if(status != FTL_ERR_PASS)
                {
                   return status;
                }
                SetFlushLogEBCounter(devID, flushEBNum[eBlockCount].key);
             }
             return FTL_ERR_FLUSH_FLUSH_GC_FAIL;

             #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             return status;
             #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

          }
       }
       latestIncNumber++;
       SetFlushLogEBCounter(devID, latestIncNumber);
       physicalEBNum = GetPhysicalEBlockAddr(devID, logicalEBNum);
       status = TABLE_FlushEBInsert(devID, logicalEBNum, physicalEBNum, latestIncNumber);
       if(status != FTL_ERR_PASS)
       {
          return status;
       }
       SetGCOrFreePageNum(devID, logicalEBNum, 1);
       MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
       logGCEBNum[eBlockCount] = logicalEBNum;
    }
    /* make dirty */
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    for(bitCount = 0; bitCount < BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE; bitCount++)
    {
       MarkEBlockMappingTableSectorDirty(devID, bitCount);
    }
    for(bitCount = 0; bitCount < BITS_PPA_DIRTY_BITMAP_DEV_TABLE; bitCount++)
    {
       MarkPPAMappingTableSectorDirty(devID, bitCount);
    }
    #else
    
    for (flushTypeCnt = 0; flushTypeCnt < 2; flushTypeCnt++)
    {
        for (bitCount = 0; bitCount < MAX_EBLOCK_MAP_INDEX; bitCount++)
        {
            if ((((bitCount * FLUSH_RAM_TABLE_SIZE) % ((UINT16)(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))) == 0))
            {
                logicalEBNum = (UINT16)((bitCount * FLUSH_RAM_TABLE_SIZE) / (UINT16)(EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
            }
            else{
                logicalEBNum = (UINT16)((bitCount * FLUSH_RAM_TABLE_SIZE) / (UINT16)((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1);
            }

            // check dirty index.
            CACHE_IsPresentEB(devID, logicalEBNum, &present);
            if (CACHE_EBM_PPA_PRESENT == present)
            {
                if (FTL_ERR_PASS != (status = (CACHE_GetIndex(devID, logicalEBNum, &index, &dependency))))
                {
                    return status;
                }
                if (FTL_TRUE == CACHE_IsDirtyIndex(devID, index))
                {
                    continue; // skip
                }
            }


            #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
            for (count = 0; count < sizeof(sysEBlockFlushInfo.reserved); count++)
            {
                sysEBlockFlushInfo.reserved[count] = EMPTY_BYTE;
            }

            #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
            sysEBlockFlushInfo.tableCheckWord = EMPTY_WORD;
            #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

            sysEBlockFlushInfo.eBlockNumLoc = EMPTY_WORD;
            sysEBlockFlushInfo.entryIndexLoc = EMPTY_WORD;
            sysEBlockFlushInfo.endPoint = EMPTY_BYTE;
            sysEBlockFlushInfo.logIncNum = EMPTY_DWORD;

            sysEBlockFlushInfo.tableOffset = bitCount;


            if (0 == flushTypeCnt)
            {
                // PPA
                sysEBlockFlushInfo.tableOffset = (UINT16)(PPA_CACHE_TABLE_OFFSET * sysEBlockFlushInfo.tableOffset);
                sysEBlockFlushInfo.type = PPA_MAP_TABLE_FLUSH;
                for (count = 0; count < PPA_CACHE_TABLE_OFFSET; count++)
                {
                    if (FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, (UINT16)((bitCount * PPA_CACHE_TABLE_OFFSET) + count), &eBlockPPAMapInfo, CACHE_PPAMAP)))
                    {
                        return status;
                    }

                    if ((CACHE_EMPTY_ENTRY_INDEX == eBlockPPAMapInfo.entryIndex) && (CACHE_EMPTY_FLASH_LOG_ARRAY == eBlockPPAMapInfo.flashLogEBArrayCount))
                    {
                        continue;
                    }

                    for (eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
                    {
                        if (flushEBNum[eBlockCount].cacheNum == eBlockPPAMapInfo.flashLogEBArrayCount)
                        {
                            phyEBlockAddrTmp = flushEBNum[eBlockCount].phyAddr;
                            break;
                        }

                    }

                    if ((status = GetFlushLoc(devID, phyEBlockAddrTmp, eBlockPPAMapInfo.entryIndex, &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
                    {
                        return status;
                    }

                    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                    if ((FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, tempCache)) != FLASH_PASS)
                    {
                        return FTL_ERR_FLASH_READ_15;
                    }
                    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                    flashStatus = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, tempArray);
                    MEM_Memcpy((UINT8_PTR)&(tempCache[0]), &tempArray[FLUSH_INFO_SIZE], FLUSH_RAM_TABLE_SIZE);
                    if (flashStatus != FLASH_PASS)
                    {
                        return FTL_ERR_FLASH_READ_06;
                    }
                    #endif

                    if ((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
                        &flushRAMTablePageInfo, &flushEB)) != FTL_ERR_PASS)
                    {
                        return status;
                    }

                    // set position
                    eBlockPPAMapInfo.entryIndex = (UINT16)GetGCNum(devID, flushEB);
                    TABLE_GetFlushLogCacheEntry(devID, GetPhysicalEBlockAddr(devID, flushEB), &eBlockPPAMapInfo.flashLogEBArrayCount);

                    if (FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_PPAMAP)))
                    {
                        return status;
                    }

                    // Inc free page
                    IncGCOrFreePageNum(devID, flushEB);

                    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                    if (FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, tempCache) != FLASH_PASS)
                    {
                       return FTL_ERR_FLASH_WRITE_05;
                    }

                    // Write END SIGNATURE, @Time T2
                    if ((status = FTL_WriteFlushInfo(&flushStructPageInfo, &sysEBlockFlushInfo)) != FTL_ERR_PASS)
                    {
                        return status;
                    }
                    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                    MEM_Memcpy((UINT8_PTR)&tempArray[0], (UINT8_PTR)&sysEBlockFlushInfo, FLUSH_INFO_SIZE);
                    MEM_Memcpy((UINT8_PTR)&tempArray[FLUSH_INFO_SIZE], &tempCache[0], FLUSH_RAM_TABLE_SIZE);
                    for (count2 = FLUSH_INFO_SIZE + FLUSH_RAM_TABLE_SIZE; count2 < SECTOR_SIZE; count2++)
                    {
                       tempArray[count2] = EMPTY_BYTE;
                    }

                    flushInfoEntry = (UINT16_PTR)&tempArray[0];
                    flushInfoEntry[FLUSH_INFO_TABLE_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_TABLE_START], (FLUSH_RAM_TABLE_SIZE / 2));
                    flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);

                    flashStatus = FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, &tempArray[0]);
                    #endif

                    sysEBlockFlushInfo.tableOffset++;
                }
            }


            if (1 == flushTypeCnt)
            {
                // EB
                sysEBlockFlushInfo.type = EBLOCK_MAP_TABLE_FLUSH;
                if (FTL_ERR_PASS != (status = CACHE_GetEBlockAndPPAMap(devID, bitCount, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
                {
                    return status;
                }

                if ((CACHE_EMPTY_ENTRY_INDEX == eBlockPPAMapInfo.entryIndex) && (CACHE_EMPTY_FLASH_LOG_ARRAY == eBlockPPAMapInfo.flashLogEBArrayCount))
                {
                    continue;
                }

                for (eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
                {
                    if (flushEBNum[eBlockCount].cacheNum == eBlockPPAMapInfo.flashLogEBArrayCount)
                    {
                        phyEBlockAddrTmp = flushEBNum[eBlockCount].phyAddr;
                        break;
                    }                    
                }

                if ((status = GetFlushLoc(devID, phyEBlockAddrTmp, eBlockPPAMapInfo.entryIndex, &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
                {
                    return status;
                }

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                if ((FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, tempCache)) != FLASH_PASS)
                {
                    return FTL_ERR_FLASH_READ_15;
                }
                #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

                flashStatus = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, tempArray);
                MEM_Memcpy((UINT8_PTR)&(tempCache[0]), &tempArray[FLUSH_INFO_SIZE], FLUSH_RAM_TABLE_SIZE);
                if (flashStatus != FLASH_PASS)
                {
                   return FTL_ERR_FLASH_READ_06;
                }
                #endif

                if ((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
                    &flushRAMTablePageInfo, &flushEB)) != FTL_ERR_PASS)
                {
                    return status; // go Flush GC.
                }

                // set position
                eBlockPPAMapInfo.entryIndex = (UINT16)GetGCNum(devID, flushEB);
                TABLE_GetFlushLogCacheEntry(devID, GetPhysicalEBlockAddr(devID, flushEB), &eBlockPPAMapInfo.flashLogEBArrayCount);

                if (FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
                {
                    return status;
                }

                // Inc free page
                IncGCOrFreePageNum(devID, flushEB);

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                if (FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, tempCache) != FLASH_PASS)
                {
                    return FTL_ERR_FLASH_WRITE_05;
                }

                // Write END SIGNATURE, @Time T2
                if ((status = FTL_WriteFlushInfo(&flushStructPageInfo, &sysEBlockFlushInfo)) != FTL_ERR_PASS)
                {
                    return status;
                }
                #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                MEM_Memcpy((UINT8_PTR)&tempArray[0], (UINT8_PTR)&sysEBlockFlushInfo, FLUSH_INFO_SIZE);
                MEM_Memcpy((UINT8_PTR)&tempArray[FLUSH_INFO_SIZE], &tempCache[0], FLUSH_RAM_TABLE_SIZE);
                for (count2 = FLUSH_INFO_SIZE + FLUSH_RAM_TABLE_SIZE; count2 < SECTOR_SIZE; count2++)
                {
                   tempArray[count2] = EMPTY_BYTE;
                }

                flushInfoEntry = (UINT16_PTR)&tempArray[0];
                flushInfoEntry[FLUSH_INFO_TABLE_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_TABLE_START], (FLUSH_RAM_TABLE_SIZE / 2));
                flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);

                flashStatus = FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, &tempArray[0]);
                #endif

            }
        }
    }
    #endif
    /* flush */
    if((status = TABLE_FlushDevice(devID, FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
    {
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(status == FTL_ERR_FLUSH_FLUSH_FAIL)
       {
          if((status = TABLE_FlushEBClear(devID)) != FTL_ERR_PASS)
          {
             return status;
          }
          for(eBlockCount = 0; eBlockCount < flushEBCount; eBlockCount++)
          {
             status = TABLE_FlushEBInsert(devID, flushEBNum[eBlockCount].logicalEBNum, 
                      flushEBNum[eBlockCount].phyAddr, flushEBNum[eBlockCount].key);
             if(status != FTL_ERR_PASS)
             {
                return status;
             }
             SetFlushLogEBCounter(devID, flushEBNum[eBlockCount].key);
          }
          for(eBlockCount = 0; eBlockCount < MIN_FLUSH_GC_EBLOCKS; eBlockCount++)
          {
             if(EMPTY_WORD != storeTemp[eBlockCount])
             {
                 TABLE_InsertReservedEB(devID,storeTemp[eBlockCount]);
             }
          }
          return FTL_ERR_FLUSH_FLUSH_GC_FAIL;
       }
       else
       #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       {
          return status;
       }
    }

    flashPageInfo.devID = devID;
    flashPageInfo.vPage.pageOffset = 0;
    flashPageInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
    sysEBlockInfo.type = SYS_EBLOCK_INFO_FLUSH;
    sysEBlockInfo.checkVersion = EMPTY_WORD;
    sysEBlockInfo.oldSysBlock = EMPTY_WORD;
    sysEBlockInfo.fullFlushSig = EMPTY_WORD;
    for (bitCount = 0; bitCount < sizeof(sysEBlockInfo.reserved); bitCount++)
    {
       sysEBlockInfo.reserved[bitCount] = EMPTY_BYTE;
    }
    latestIncNumber = GetFlushEBCounter(devID);
    for(eBlockCount = (MIN_FLUSH_GC_EBLOCKS-1); eBlockCount >= 0; eBlockCount--)
    {
       logicalEBNum = logGCEBNum[eBlockCount];
       physicalEBNum = GetPhysicalEBlockAddr(devID, logicalEBNum);
       /* Erase GC eblk */
       flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(physicalEBNum, 0);
       /* Write system eblk info to GC eblk */
       sysEBlockInfo.incNumber = latestIncNumber--;
       sysEBlockInfo.phyAddrThisEBlock = physicalEBNum;
       #if (MIN_FLUSH_GC_EBLOCKS > 1)
       if(eBlockCount < (MIN_FLUSH_GC_EBLOCKS-1))
       {
          sysEBlockInfo.fullFlushSig = FULL_FLUSH_SIGNATURE;
       }
       #endif

       #if(FTL_CHECK_VERSION == FTL_TRUE)
       if(eBlockCount == 0)
       {
          sysEBlockInfo.checkVersion = CalcCheckWord((UINT16_PTR)FTL_FLASH_IMAGE_VERSION, NUM_WORDS_OF_VERSION);
       }
       #endif  // #if(FTL_CHECK_VERSION == FTL_TRUE)

       // moved the sys block writing function here, from above for PFT.
       if((status = FTL_WriteSysEBlockInfo(&flashPageInfo, &sysEBlockInfo)) != FTL_ERR_PASS)
       {
          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(status == FTL_ERR_FAIL)
          {
             return status;
          }
          SetBadEBlockStatus(devID, logicalEBNum, FTL_TRUE);
          if(FLASH_MarkDefectEBlock(&flashPageInfo) != FLASH_PASS)
          {
             // do nothing, just try to mark bad, even if it fails we move on.
          }
          if((status = TABLE_FlushEBClear(devID)) != FTL_ERR_PASS)
          {
             return status;
          }
          for(eBlockCount = 0; eBlockCount < flushEBCount; eBlockCount++)
          {
             status = TABLE_FlushEBInsert(devID, flushEBNum[eBlockCount].logicalEBNum, 
                      flushEBNum[eBlockCount].phyAddr, flushEBNum[eBlockCount].key);
             if(status != FTL_ERR_PASS)
             {
                return status;
             }
             SetFlushLogEBCounter(devID, flushEBNum[eBlockCount].key);
          }
          for(eBlockCount = 0; eBlockCount < MIN_FLUSH_GC_EBLOCKS; eBlockCount++)
          {
             if(EMPTY_WORD != storeTemp[eBlockCount])
             {
                 TABLE_InsertReservedEB(devID,storeTemp[eBlockCount]);
             }
          }
          return FTL_ERR_FLUSH_FLUSH_GC_FAIL;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return status;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       SetEBErased(devID, logicalEBNum, FTL_FALSE);
       #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    }
    /* Mark old flush log eblks */
    // It does not matter if the ECC is turned off for these records.
    // This operation is messing up the Check Word, so the Sys Info will be invalid anyway
    sysEBlockInfo.oldSysBlock = OLD_SYS_BLOCK_SIGNATURE;
    for(eBlockCount = 0; eBlockCount < flushEBCount; eBlockCount++)
    {
       logicalEBNum = flushEBNum[eBlockCount].logicalEBNum;
       #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
       if((status = TABLE_InsertReservedEB(devID, logicalEBNum)) != FTL_ERR_PASS)
       {
          return status;
       }
       #endif
       flashPageInfo.devID = devID;
       physicalEBNum = flushEBNum[eBlockCount].phyAddr;
       flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(physicalEBNum, 0);
       flashPageInfo.vPage.pageOffset = (UINT16)((UINT32)(&sysTempPtr->oldSysBlock)); 
       flashPageInfo.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
//     flashPageInfo.byteCount = sizeof(sysEBlockInfo.oldSysBlock);
       flashStatus = FLASH_RamPageWriteMetaData(&flashPageInfo, (UINT8_PTR)&sysEBlockInfo.oldSysBlock);
       if(flashStatus != FLASH_PASS)
       {
          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(flashStatus == FLASH_PARAM)
          {
             return FTL_ERR_FLASH_WRITE_13;
          }
          SetBadEBlockStatus(devID, logicalEBNum, FTL_TRUE);
          // just try to mark bad, even if it fails we move on.
          FLASH_MarkDefectEBlock(&flashPageInfo);

          checkBBMark = FTL_TRUE;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return FTL_ERR_FLASH_WRITE_13;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if((status = TABLE_InsertReservedEB(devID, logicalEBNum)) != FTL_ERR_PASS)
       {
          return status;
       }
       #endif
    }

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    // Erase the log entries, only if its not empty...
    while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
    {
       status = FTL_EraseAllTransLogBlocksOp(devID);
       if(FTL_ERR_MARKBB_COMMIT == status)
       {
          checkBBMark = FTL_TRUE;
          status = FTL_ERR_PASS;
       }
       if(status != FTL_ERR_LOG_NEW_EBLOCK_FAIL)
       {
          break;
       }
       checkBBMark = FTL_TRUE;
       sanityCounter++;
    }
    if(status != FTL_ERR_PASS)
    {
       return status;
    }
    if(FTL_TRUE == checkBBMark)
    {
       return FTL_ERR_FLUSH_FLUSH_GC_FAIL;
    }    
    #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    // Erase the log entries, only if its not empty...
    if((status = FTL_EraseAllTransLogBlocksOp(devID)) != FTL_ERR_PASS)
    {
       return status;
    }
    #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    return FTL_ERR_PASS;
}

//--------------------------------------------------------------
FTL_STATUS FTL_SetGCThreshold(UINT8 type, UINT32 threshold)  /*  1,  4*/
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }
    switch(type)
    {
       case 0:
          GC_THRESHOLD = (UINT16)threshold;
          break;
       case 1:
          Delete_GC_Threshold = (UINT8)threshold;
          break;
       case 2:
          GC_THRESHOLD = (UINT16)threshold;
          Delete_GC_Threshold = (UINT8)threshold;
          break;
       default:
          status = FTL_ERR_GC_SET_THRESHOLD;
          break;
    }
    FTL_ClearMTLockBit();
    return status;
}

// This is the entry point to the static wear leveling functionality.
// this needs to be called only when there is no pending GC.
// A new parameter, pickHottest, is added to FTL_InternalForcedGC.  FTL_InternalForcedGC()
//   passes this parameter to FTL_SwapDataReserveEB() which passes it to pickEBCandidate(). 
//   All callers of FTL_InternalForcedGC, except FTL_StaticWearLevelData(),
//   will set this parameter to FTL_FALSE.

//---------------------------------------
#if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
FTL_STATUS FTL_StaticWearLevelData(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FTL_DEV devID = EMPTY_BYTE;                        /*1*/
    UINT32 eraseRange = 0;                             /*2*/
    UINT16 coldestEB = 0;                              /*2*/
    UINT16 freedUpPages = 0;                           /*2*/
    UINT16 freePageIndex = 0;                          /*2*/
    #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
    UINT16 chainEB = 0;
    #endif  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)

    for (devID = 0; devID < NUMBER_OF_DEVICES; devID++)
    {
       eraseRange = FindEraseCountRange(devID, &coldestEB);
       #if (DEBUG_CHECK_WL == FTL_TRUE)
       if(EMTPY_WORD == eraseRange)
       {
          return FTL_ERR_FAIL;
       }
       #endif
       if (eraseRange > FTL_DATA_WEAR_LEVEL_RANGE)
       {
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, coldestEB, CACHE_WRITE_TYPE)))
          {
             return status;
          }
          #endif
          #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
          chainEB = GetChainLogicalEBNum(devID, coldestEB);
          if (EMPTY_WORD == chainEB)
          {
              if(coldestEB >= NUMBER_OF_DATA_EBLOCKS)
              {
                 // Coldest EB is already in Reserved-Available pool
                 continue;
              }
          }
          else
          {
              if(coldestEB >= NUMBER_OF_DATA_EBLOCKS)
              {
                 // Coldest EB is already in Reserved-Available pool
                 coldestEB = chainEB;
                 // Coldest EB is changed to chainEB in DataEB, so CACHE_LoadEB needs to be invoked again.
                 #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                 if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, coldestEB, CACHE_WRITE_TYPE)))
                 {
                    return status;
                 }
                 #endif
              }
          }
          #else  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
          if (coldestEB >= NUMBER_OF_DATA_EBLOCKS)
          {
             // Coldest EB is already in Reserved-Available pool
             continue;
          }          
          #endif  // #else  // #if (FTL_TRUE == FTL_EBLOCK_CHAINING)
//        StaticWLInfo.staticWLCallCounter++;

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          status = InternalForcedGCWithBBManagement(devID, coldestEB, &freedUpPages, &freePageIndex, FTL_TRUE); 

          #else
          status = FTL_InternalForcedGC(devID, coldestEB, &freedUpPages, &freePageIndex, FTL_TRUE); 
          #endif

          if(status != FTL_ERR_PASS)
          {
             return status;
          }

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(GetTransLogEBFailedBadBlockInfo() == FTL_TRUE)
          {
             if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
             {
                return status;
             }
          }
          ClearTransLogEBBadBlockInfo();
          #endif

       }
    }
    return FTL_ERR_PASS;
}
#endif  // #if (FTL_STATIC_WEAR_LEVELING != FTL_TRUE)

//--------------------------------
UINT32 FindEraseCountRange(FTL_DEV devID, UINT16_PTR coldestEB)
{
    UINT16 logicalBlockNum = 0;                        /*2*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 eraseCount = 0;                             /*2*/
    #endif
    UINT32 lowestCount = EMPTY_DWORD;                  /*2*/
    UINT32 highestCount = 0;                           /*2*/

    // If the group of coldest EBs includes one of the Reserved-Available EBs, 
    //   we want to select that one.  Therefore, we search that group first
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    GetSaveStaticWL(devID, &logicalBlockNum, &highestCount, CACHE_WL_HIGH);
    GetSaveStaticWL(devID, coldestEB, &lowestCount, CACHE_WL_LOW);
    #else
    for (logicalBlockNum = NUMBER_OF_DATA_EBLOCKS; logicalBlockNum < NUM_EBLOCKS_PER_DEVICE; logicalBlockNum++)
    {
       #if (DEBUG_CHECK_WL == FTL_FALSE)
       if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID, logicalBlockNum))
       {
          continue;
       }
       #endif
       eraseCount = GetTrueEraseCount(devID, logicalBlockNum);
       if (lowestCount > eraseCount)
       {
          *coldestEB = logicalBlockNum;
          lowestCount = eraseCount;
       }
       if (highestCount < eraseCount)
       {
          highestCount = eraseCount;
       }       
    }
    for (logicalBlockNum = 0; logicalBlockNum < NUMBER_OF_DATA_EBLOCKS; logicalBlockNum++)
    {
       eraseCount = GetTrueEraseCount(devID, logicalBlockNum);
       if (lowestCount > eraseCount)
       {
          *coldestEB = logicalBlockNum;
          lowestCount = eraseCount;
       }
       if (highestCount < eraseCount)
       {
          highestCount = eraseCount;
       }
    }
    #endif

    #if (DEBUG_CHECK_WL == FTL_TRUE)
    if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID, *coldestEB))
    {
       DBG_Printf("Warning FindEraseCountRange: Get System EB 0x%X\n", *coldestEB, 0);
       return EMTPY_WORD;
    }
    #endif

    return highestCount - lowestCount;
}

//-----------------------------
// This function replaces the function of the same name
//   If pickHottest is FTL_TRUE, this function will return the Hottest candidate instead of the Coldest
UINT16 pickEBCandidate(EMPTY_LIST_PTR emptyList, UINT16 totalEmpty, UINT8 pickHottest)
{
   UINT16 count = 0;                                   /*2*/
   UINT32 selectedScore = EMPTY_DWORD;                 /*4*/
   UINT16 selectedIndex = 0;                           /*2*/

    // If pickHottest = FTL_FALSE, the entry with the lowest weight is picked
    // If pickHottest = FTL_TRUE, the entry with the highest weight is picked
    if (FTL_TRUE == pickHottest)
    {
       selectedScore = 0;
    }
    for (count = 0; count < totalEmpty; count++)
    {
       if ((FTL_FALSE == pickHottest) && (emptyList[count].isErased == FTL_TRUE))
       {
          return emptyList[count].logEBNum;
       }
       if(((FTL_TRUE == pickHottest) && (selectedScore < emptyList[count].eraseScore)) ||
          ((FTL_FALSE == pickHottest) && (selectedScore > emptyList[count].eraseScore)))
       {
          selectedScore = emptyList[count].eraseScore;
          selectedIndex = count;
       }
    }
    return emptyList[selectedIndex].logEBNum;
}

FTL_STATUS FTL_CheckForGCLogSpace(FTL_DEV devID)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 latestIncNumber = EMPTY_DWORD;              /*4*/
    UINT16 logLogicalEBNum = EMPTY_WORD;               /*2*/
    UINT16 logPhyEBAddr = EMPTY_WORD;                  /*2*/
    UINT16 entryIndex = 0;                             /*2*/
    UINT16 numLogEntries = 0;                          /*2*/

    latestIncNumber = GetTransLogEBCounter(devID);
    status = TABLE_TransLogEBGetLatest(devID, &logLogicalEBNum, &logPhyEBAddr, latestIncNumber);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }

    // Get free index
    entryIndex = GetFreePageIndex(devID, logLogicalEBNum);

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    // + 1 to account for the type A
    numLogEntries = NUM_GC_TYPE_B + 1;

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    numLogEntries = 2;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    #if(FTL_ENABLE_UNUSED_EB_SWAP  == FTL_TRUE)
    /* for EB swap entry */
    numLogEntries++;
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    numLogEntries++;
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #endif  // #if(FTL_ENABLE_UNUSED_EB_SWAP  == FTL_TRUE)

    if ((entryIndex + numLogEntries) > NUM_LOG_ENTRIES_PER_EBLOCK)
    {
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(GetTransLogEBArrayCount(devID) < (NUM_TRANSACTION_LOG_EBLOCKS - LOG_EBLOCK_RETRIES))

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(GetTransLogEBArrayCount(devID) < NUM_TRANSACTION_LOG_EBLOCKS)
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       {
          if((status = CreateNextTransLogEBlock(devID, logLogicalEBNum)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       else
       {
          status = TABLE_Flush(FLUSH_NORMAL_MODE);
          if (FTL_ERR_PASS != status)
          {
             return status;
          }
       }
    }
    return status;
}


#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
//---------------------------
UINT32 GetBBPageMoved(UINT16 pageOffset)
{
    return badBlockPhyPageAddr[pageOffset];
}

FTL_STATUS InternalForcedGCWithBBManagement(FTL_DEV devID, UINT16 logicalEBNum,
    UINT16_PTR FreedUpPages, UINT16_PTR freePageIndex, UINT8 WLflag ) 
{
    FTL_STATUS GC_result = FTL_ERR_PASS;
    FTL_STATUS badBlockStatus = FTL_ERR_PASS;
    UINT16 GCCount = 0;
    do
    {
       GC_result = FTL_InternalForcedGC(devID, logicalEBNum, FreedUpPages, freePageIndex, WLflag);
       if(GC_result != FTL_ERR_PASS)
       {
          if(GCCount >= MAX_BAD_BLOCK_SANITY_TRIES)
          {
             return (GC_result); //exit the transfer
          }
          else
          {
             GCCount++;
             badBlockStatus = TranslateBadBlockError(GC_result);
             if(badBlockStatus == FTL_ERR_LOG_WR)
             {
                 // log write failure
                 badBlockStatus = BB_ManageBadBlockErrorForGCLog(); 
             }
             else if(badBlockStatus == FTL_ERR_BAD_BLOCK_SOURCE)
             {
                // UnlinkGC failure
                badBlockStatus = BB_ManageBadBlockErrorForSource();
             }else if(badBlockStatus == FTL_ERR_BAD_BLOCK_TARGET)
             {
                // data write failure
                 badBlockStatus = BB_ManageBadBlockErrorForTarget(); // all flash changes will be in the reserve pool
             }
             else if(badBlockStatus != FTL_ERR_PASS)
             {
                 return badBlockStatus;
             }
          }
       }
       ClearBadBlockInfo();

    }while(GC_result != FTL_ERR_PASS);             

    return GC_result;
}

/*---------------------------------------
This function will pick an new block, upate the chaining and EB mapping info, erase it if needed. copy hte pages and return
before calling this function, the eblock mapping table has to be restored before calling this function.
//---------------------------------------*/
FTL_STATUS BadBlockCopyPages(FTL_DEV devID, UINT16 logicalEBNum ) 
{

    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    UINT16 phyFromEBlock = EMPTY_WORD;                 /*2*/
    UINT16 phyToEBlock = EMPTY_WORD;                   /*2*/
    UINT16 phyPageOffset = EMPTY_WORD;
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    UINT16 logToEBlock = EMPTY_WORD;                   /*2*/
    UINT16 logicalPageOffset = EMPTY_WORD;
    UINT16 pageCount = 0;                              /*2*/
    UINT8 isMoved = FTL_FALSE;                         /*1*/
    UINT16 sanityCounter = 0;

    pageInfo.devID = devID;
    pageInfo.vPage.pageOffset = 0;
    pageInfo.byteCount = NUMBER_OF_BYTES_PER_PAGE;

    phyFromEBlock = GetPhysicalEBlockAddr(devID, logicalEBNum);

    do
    {
       sanityCounter++;
       if(sanityCounter > MAX_BAD_BLOCK_SANITY_TRIES)
       {
            return status;
       }

       pageCount = 0;
       status = FTL_ERR_PASS;

       // Identify EB to copy to, not chaining, regular GC, so swap will not erase, nor create a log, no need to worry about bad block
       if((status = FTL_SwapDataReserveEBlock(devID, logicalEBNum, &phyToEBlock, &logToEBlock, FTL_FALSE, FTL_TRUE)) != FTL_ERR_PASS)
       {
          return status;
       }

       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       eraseStatus = GetEBErased(devID, logicalEBNum);

       if(FTL_FALSE == eraseStatus)
       #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       {
          status = FTL_EraseOp(devID, logicalEBNum);
          if(FTL_ERR_PASS != status)
          {
             if(FTL_ERR_FAIL == status)
             {
                return FTL_ERR_GC_ERASE3;
             }
             SetBadEBlockStatus(devID, logicalEBNum, FTL_TRUE);
             if(FLASH_MarkDefectEBlock(&pageInfo) != FLASH_PASS)
             {
                 // do nothing, just try to mark bad, even if it fails we move on.
             }
             continue;
          }
       }

       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       SetEBErased(devID, logicalEBNum, FTL_FALSE); /*since the EB will be written to now*/
       #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

       MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);

// -------------------------------------------------------------------------------------------------
// COPY PAGES
       for (logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
       {

          phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
          if ((EMPTY_INVALID != phyPageOffset) && (CHAIN_INVALID != phyPageOffset))
          {
             // Page must be valid - Copy Page
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyFromEBlock, phyPageOffset);
             flashStatus = FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if (FLASH_PASS != flashStatus)
             {
                return FTL_ERR_GC_BB_LOAD1;
             }
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyToEBlock, phyPageOffset);
             flashStatus = FLASH_RamPageWriteDataBlock(&pageInfo, &pseudoRPB[devID][0]);
             if (FLASH_PASS != flashStatus)
             {
                status = FTL_ERR_GC_BB_PAGE_WR1;
                if(FLASH_PARAM == flashStatus)
                {
                   return status;
                }
                SetBadEBlockStatus(devID, logicalEBNum, FTL_TRUE);
                if(FLASH_MarkDefectEBlock(&pageInfo) != FLASH_PASS)
                {
                   // do nothing, just try to mark bad, even if it fails we move on.
                }
                break;
             }
             pageCount++;
         }

       }
       if (FTL_ERR_PASS != status)
       {
          continue;
       }
       if((GC_Info.devID != EMPTY_BYTE) && (GCMoveArrayNotEmpty == FTL_TRUE))
       {
          for(logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
          {
             IsPageMoved(logicalPageOffset, &isMoved);
             if(isMoved == FTL_TRUE)
             {
                pageInfo.vPage.vPageAddr = GetBBPageMoved(logicalPageOffset);
                if(pageInfo.vPage.vPageAddr != EMPTY_DWORD)
                {
                   flashStatus = FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devID][0]);
                   if (FLASH_PASS != flashStatus)
                   {
                      return FTL_ERR_GC_BB_LOAD2;
                   }
                   phyPageOffset = GetFreePageIndex(devID, logicalEBNum);  
                   pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyToEBlock, phyPageOffset);
                   flashStatus = FLASH_RamPageWriteDataBlock(&pageInfo, &pseudoRPB[devID][0]);
                   if (FLASH_PASS != flashStatus)
                   {
                      status =  FTL_ERR_GC_BB_PAGE_WR2;
                      if(FLASH_PARAM == flashStatus)
                      {
                         return status;
                      }
                      SetBadEBlockStatus(devID, logicalEBNum, FTL_TRUE);
                      if(FLASH_MarkDefectEBlock(&pageInfo) != FLASH_PASS)
                      {
                          // do nothing, just try to mark bad, even if it fails we move on.
                      }
                      break;
                   }
                   pageCount++;
                }
             }
          }
       }
    } while(status != FTL_ERR_PASS);

    if((GC_Info.devID != EMPTY_BYTE) && (GCMoveArrayNotEmpty == FTL_TRUE))
    {
       for(logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
       {
          IsPageMoved(logicalPageOffset, &isMoved);
          if(isMoved == FTL_TRUE)
          {
             pageInfo.vPage.vPageAddr = GetBBPageMoved(logicalPageOffset);
             if(pageInfo.vPage.vPageAddr != EMPTY_DWORD)
             {
                phyPageOffset = GetFreePageIndex(devID, logicalEBNum);  
                UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, phyPageOffset,  BLOCK_INFO_VALID_PAGE);
             }
          }
       }
    }    
    #if DEBUG_CHECK_TABLES
    if(pageCount > NUM_PAGES_PER_EBLOCK)
    {
       return FTL_ERR_GC_BB_PAGE_SANITY;
    }
    status = DBG_CheckPPAandBitMap(devID, logicalEBNum);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    #endif  // #if DEBUG_CHECK_TABLES    
    return FTL_ERR_PASS;
}



//----------------------------------------

FTL_STATUS BadBlockEraseFailure(FTL_DEV devID, UINT16 eBlockNum)
{
    FTL_STATUS status = FTL_ERR_FAIL;
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0}};        /*11*/
    UINT16 sanityCounter = 0;
    UINT16 phyToEBlock = EMPTY_WORD;
    UINT16 logToEBlock = EMPTY_WORD;

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    while((status != FTL_ERR_PASS) && (sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES))
    {
        sanityCounter++;
        SetBadEBlockStatus(devID, eBlockNum, FTL_TRUE);
        flashPage.devID = devID;
        flashPage.byteCount = 0;
        flashPage.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(GetPhysicalEBlockAddr(devID, eBlockNum), 0);
        flashPage.vPage.pageOffset = 0;
        if(FLASH_MarkDefectEBlock(&flashPage) != FLASH_PASS)
        {
           // do nothing, just try to mark bad, even if it fails we move on;             
        }
        if((status = FTL_SwapDataReserveEBlock(devID, eBlockNum, &phyToEBlock, &logToEBlock, FTL_FALSE, FTL_TRUE)) != FTL_ERR_PASS)
        {
             return status;
        }
        #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
        eraseStatus = GetEBErased(devID, eBlockNum);

        #if DEBUG_PRE_ERASED
        if(FTL_TRUE == eraseStatus)
        {
           DBG_Printf("BadBlockEraseFailure: EBlock 0x%X is already erased\n", eBlockNum, 0);
        }
        #endif  // #if DEBUG_PRE_ERASED
        if(FTL_FALSE == eraseStatus)
        #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
        {
           status = FTL_EraseOp(devID, eBlockNum);
           if (FTL_ERR_PASS != status)
           {
              if(FTL_ERR_FAIL == status)
              {
                 return status;
              }
               continue; 
           }
        }
    }
    return status;
}

FTL_STATUS BB_ManageBadBlockErrorForSource(void)
{
/*        // the strategy for dealing with data area writes is to 
        1) mark the block bad, 
        2) copy the orginal data using the saved EB mapping info to a new block in the reseve pool, 
        3) swap the bad the good block
        4) overwrite the old EB mapping table with the saved one
        5) update the phy eb info to point to the new block
        5) flush
        6) restart the transfer by building the transfer map again
        */
    FTL_STATUS status = FTL_ERR_PASS;
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0}};        /*11*/
    UINT16 badPhyEB = EMPTY_WORD;
    UINT16 BBLogicalEB = EMPTY_WORD;
    UINT8 chainFlag = FTL_FALSE;

    if(badBlockInfo.operation == FTL_ERR_CHAIN_NOT_FULL_EB)
    {
         BB_FindBBInChainedEBs(&BBLogicalEB, &chainFlag, &badPhyEB);
    }
    RestoreSourceBadBlockInfo();
    RestoreTargetBadBlockInfo();
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum, FTL_FALSE); /*since the EB will be written to now*/
    if(badBlockInfo.targetLogicalEBNum != EMPTY_WORD)
    {
        SetEBErased(badBlockInfo.devID, badBlockInfo.targetLogicalEBNum, FTL_FALSE); /*since the EB will be written to now*/
    }
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    // set the erase bit for both the blocks
    if(badBlockInfo.operation == FTL_ERR_CHAIN_NOT_FULL_EB)
    {
        SetBadEBlockStatus(badBlockInfo.devID, BBLogicalEB, FTL_TRUE);
        flashPage.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(GetPhysicalEBlockAddr(badBlockInfo.devID, BBLogicalEB), 0);
    }
    else
    {
        SetBadEBlockStatus(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum, FTL_TRUE);
        flashPage.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(GetPhysicalEBlockAddr(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum), 0);
    }
    status = BadBlockCopyPages(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum);
    if(status != FTL_ERR_PASS)
    {
        return status;
    }
    if((badBlockInfo.devID != EMPTY_BYTE) && (badBlockInfo.sourceLogicalEBNum != EMPTY_WORD))
    {
       flashPage.devID = badBlockInfo.devID;
       flashPage.byteCount = 0;
       flashPage.vPage.pageOffset = 0;
       if(FLASH_MarkDefectEBlock(&flashPage) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on;             
       }
    }
    if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS) // to save the bad block mark
    {
       FTL_ClearMTLockBit();
       return status;
    }
    ClearBadBlockInfo();
    return status;
}

FTL_STATUS BB_ManageBadBlockErrorForChainErase(void)
{
    FTL_STATUS status = FTL_ERR_PASS;
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0}};        /*11*/

    RestoreTargetBadBlockInfo();
    if((badBlockInfo.devID != EMPTY_BYTE) && (badBlockInfo.sourceLogicalEBNum != EMPTY_WORD))
    {
       RestoreSourceBadBlockInfo();
       SetBadEBlockStatus(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum, FTL_TRUE);
       flashPage.devID = badBlockInfo.devID;
       flashPage.byteCount = 0;
       flashPage.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(GetPhysicalEBlockAddr(badBlockInfo.devID, badBlockInfo.sourceLogicalEBNum), 0);
       flashPage.vPage.pageOffset = 0;
       if(FLASH_MarkDefectEBlock(&flashPage) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on;             
       }
    }

    if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS) // to save the bad block mark
    {
       FTL_ClearMTLockBit();
       return status;
    }
    ClearBadBlockInfo();

    // 1 Mark Block Bad
    // 5 simple flush 
    return status;
}



FTL_STATUS BB_FindBBInChainedEBs(UINT16_PTR BBlogicalEB, UINT8_PTR chainFlag, UINT16_PTR badPhyEB)
{
    FTL_STATUS status = FTL_ERR_PASS;
    UINT32 physicalPage = EMPTY_DWORD;
    UINT16 phyEBNum = EMPTY_WORD;
    UINT16 physicalEBNum = EMPTY_WORD;
    UINT32 currentLBA = EMPTY_DWORD;
    UINT32 logicalPageAddr = EMPTY_DWORD;
    UINT16 logicalEBNum = EMPTY_WORD;
    UINT16 chainEBNum = EMPTY_WORD;
    FTL_DEV devID = EMPTY_BYTE;
    UINT16 index = EMPTY_WORD;

    index = FTL_GetCurrentIndex();
    if(index == 0)
    {
        return FTL_ERR_GC_BB_INDEX;
    }
    else
    {
        index--;
    }

    *BBlogicalEB = EMPTY_WORD;
    currentLBA = GetTMStartLBA(index);
    devID = GetTMDevID(index);
    physicalPage = GetTMPhyPage(index);


    if((currentLBA != EMPTY_DWORD) && (devID != EMPTY_BYTE))
    {
       status = GetLogicalEBNum(physicalPage, &physicalEBNum);
       if(status != FTL_ERR_PASS)
       {
           return status;
       }
       status = GetPageNum(currentLBA, &logicalPageAddr);
       if(status != FTL_ERR_PASS)
       {
           return status;
       }
       status = GetLogicalEBNum(logicalPageAddr, &logicalEBNum);
       if(status != FTL_ERR_PASS)
       {
           return status;
       }
       phyEBNum = GetPhysicalEBlockAddr(devID, logicalEBNum);

       if(physicalEBNum == phyEBNum)
       {
           *badPhyEB = phyEBNum;
           *BBlogicalEB = logicalEBNum;
           *chainFlag = FTL_FALSE;
       }
       else
       {
           #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
           chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
           if(chainEBNum != EMPTY_WORD) 
           { 
               phyEBNum = GetPhysicalEBlockAddr(devID, chainEBNum);
               if(physicalEBNum == phyEBNum)
               {
                  *badPhyEB = phyEBNum;
                  *BBlogicalEB = chainEBNum;
                  *chainFlag = FTL_TRUE;
               }
               else
               {
                  *badPhyEB = EMPTY_WORD;
                  *BBlogicalEB = EMPTY_WORD;
                  *chainFlag = FTL_FALSE;
               }
           }
           else
           #endif
           {
                *badPhyEB = EMPTY_WORD;
                *BBlogicalEB = EMPTY_WORD;
                *chainFlag = FTL_FALSE;
           }
       }
    }
    else
    {
        return FTL_ERR_GC_BB_CHAIN;
    }
    return status;
}

FTL_STATUS BB_ManageBadBlockErrorForTarget(void)
{
    FTL_STATUS status = FTL_ERR_PASS;
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0}};        /*11*/

    RestoreSourceBadBlockInfo();
    if((badBlockInfo.devID != EMPTY_BYTE) && (badBlockInfo.targetLogicalEBNum != EMPTY_WORD))
    {
       RestoreTargetBadBlockInfo();
       SetBadEBlockStatus(badBlockInfo.devID, badBlockInfo.targetLogicalEBNum, FTL_TRUE);
       flashPage.devID = badBlockInfo.devID;
       flashPage.byteCount = 0;
       flashPage.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(GetPhysicalEBlockAddr(badBlockInfo.devID, badBlockInfo.targetLogicalEBNum), 0);
       flashPage.vPage.pageOffset = 0;
       if(FLASH_MarkDefectEBlock(&flashPage) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on;             
       }
    }

    if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS) // to save the bad block mark
    {
       FTL_ClearMTLockBit();
       return status;
    }
    ClearBadBlockInfo();

    // 1 Mark Block Bad
    // 5 simple flush 
    return status;
}

FTL_STATUS BB_ManageBadBlockErrorForGCLog(void)
{
    FTL_STATUS status = FTL_ERR_PASS;
//    nothing to do here, managed by the System area bad block system 
    // 1 Mark Block Bad
    // 5 simple flush
    // TABLE_Flush() should not be called.
    /*
    if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS) // to save the bad block mark
    {
       FTL_ClearMTLockBit();
       return status;
    }
    */
    return status;
}

#endif
