// file: ftl_ops.c
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
  #include <linux/span/FTL/ftl_lowlevel.h>
  #include <linux/span/FTL/ftl_common.h>
#else
  #include "ftl_lowlevel.h"
  #include "ftl_common.h"
#endif // #ifdef __KERNEL__

#define DEBUG_LOG_ENTRIES     (0)
#define DEBUG_ERASE_OP        (0)
#define DEBUG_EARLY_BAD_ENTRY (1)
#define DEBUG_REDUNDANT_MESG  (1)
#define DEBUG_CHECK_GET_SUPER (0)

//------------------------------------------------------------------------------
/* This function will do the transfer map, and also the page address cache, when implimented */
/* isPreloaded and phyPage are needed for when the transfer map and address cache are implimented*/
FTL_STATUS FTL_BackgroundTask(void) /**/
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/

    if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }

    FTL_ClearMTLockBit();
    return FTL_ERR_PASS;
}

#if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
//-----------------------------
FTL_STATUS FTL_WriteLogInfo(FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr)
{
    UINT16_PTR logEntry = (UINT16_PTR) logPtr;         /*4*/
    UINT8 redundantLog[LOG_ENTRY_SIZE*2];              /*32*/

    logEntry[LOG_ENTRY_CHECK_WORD] = CalcCheckWord(&logEntry[LOG_ENTRY_DATA_START], LOG_ENTRY_DATA_WORDS);
    MEM_Memcpy(&redundantLog[0], logPtr, LOG_ENTRY_SIZE);
    MEM_Memcpy(&redundantLog[LOG_ENTRY_SIZE], logPtr, LOG_ENTRY_SIZE);
    flashPagePtr->byteCount += LOG_ENTRY_SIZE; 
    if(FLASH_RamPageWriteMetaData(flashPagePtr, &redundantLog[0]) != FLASH_PASS)
    {
       return FTL_ERR_LOG_WR;
    }
    flashPagePtr->byteCount -= LOG_ENTRY_SIZE;
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS FTL_WriteFlushInfo(FLASH_PAGE_INFO_PTR flashPagePtr, SYS_EBLOCK_FLUSH_INFO_PTR flushInfoPtr)
{
    UINT16_PTR flushInfoEntry = (UINT16_PTR) flushInfoPtr; /*4*/
    UINT8 redundantInfo[FLUSH_INFO_SIZE*2];                /*32*/

    flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);
    MEM_Memcpy(&redundantInfo[0], (UINT8_PTR)flushInfoEntry, FLUSH_INFO_SIZE);
    MEM_Memcpy(&redundantInfo[FLUSH_INFO_SIZE], (UINT8_PTR)flushInfoEntry, FLUSH_INFO_SIZE);
    flashPagePtr->byteCount += FLUSH_INFO_SIZE; 
    if(FLASH_RamPageWriteMetaData(flashPagePtr, &redundantInfo[0]) != FLASH_PASS)
    {
       return FTL_ERR_FLASH_WRITE_08;
    }
    flashPagePtr->byteCount -= FLUSH_INFO_SIZE;
    return FTL_ERR_PASS;
}

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
//-----------------------------
FTL_STATUS FTL_WriteSuperInfo(FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr)
{
    UINT16_PTR logEntry = (UINT16_PTR) logPtr;         /*4*/
    UINT8 redundantLog[LOG_ENTRY_SIZE*2];              /*32*/

    logEntry[LOG_ENTRY_CHECK_WORD] = CalcCheckWord(&logEntry[LOG_ENTRY_DATA_START], LOG_ENTRY_DATA_WORDS);
    MEM_Memcpy(&redundantLog[0], logPtr, LOG_ENTRY_SIZE);
    MEM_Memcpy(&redundantLog[LOG_ENTRY_SIZE], logPtr, LOG_ENTRY_SIZE);
    flashPagePtr->byteCount += LOG_ENTRY_SIZE; 
    if(FLASH_RamPageWriteMetaData(flashPagePtr, &redundantLog[0]) != FLASH_PASS)
    {
       return FTL_ERR_SUPER_WRITE_01;
    }
    flashPagePtr->byteCount -= LOG_ENTRY_SIZE;
    return FTL_ERR_PASS;
}
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

//---------------------------------------
FTL_STATUS FTL_WriteSysEBlockInfo(FLASH_PAGE_INFO_PTR flashPagePtr, SYS_EBLOCK_INFO_PTR sysInfoPtr)
{
    UINT16_PTR localPtr = (UINT16_PTR) sysInfoPtr;     /*4*/
    UINT8 redundantInfo[SYS_INFO_SIZE*2];              /*32*/

    // Write sector once
    localPtr[SYS_INFO_CHECK_WORD] = CalcCheckWord(&localPtr[SYS_INFO_DATA_START], SYS_INFO_DATA_WORDS);
    MEM_Memcpy(&redundantInfo[0], (UINT8_PTR)localPtr, SYS_INFO_SIZE);
    MEM_Memcpy(&redundantInfo[SYS_INFO_SIZE], (UINT8_PTR)localPtr, SYS_INFO_SIZE);
    flashPagePtr->byteCount += SYS_INFO_SIZE; 
    if((FLASH_RamPageWriteMetaData(flashPagePtr, &redundantInfo[0])) != FLASH_PASS)
    {
       return FTL_ERR_FLASH_WRITE_12;
    }
    flashPagePtr->byteCount -= SYS_INFO_SIZE;
    return FTL_ERR_PASS;
}

#else  // #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
//----------------------------------
FTL_STATUS FTL_WriteLogInfo(FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16_PTR logEntry = (UINT16_PTR) logPtr;         /*4*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT32 byteCount = 0;                              /*4*/
    UINT32 count = 0;                                  /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;
    UINT8 storeArray[SECTOR_SIZE];                     /*1*/
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    logEntry[LOG_ENTRY_CHECK_WORD] = CalcCheckWord(&logEntry[LOG_ENTRY_DATA_START], LOG_ENTRY_DATA_WORDS);

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    if(FLASH_RamPageWriteMetaData(flashPagePtr, (UINT8_PTR)logEntry) != FLASH_PASS)
    {
       return FTL_ERR_LOG_WR;
    }

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    MEM_Memcpy(storeArray, logPtr, (UINT16)flashPagePtr->byteCount);
    for(count = flashPagePtr->byteCount; count < SECTOR_SIZE; count++)
    {
       storeArray[count] = EMPTY_BYTE;
    }
    byteCount = flashPagePtr->byteCount;
    flashPagePtr->byteCount = SECTOR_SIZE;
    flashStatus = FLASH_RamPageWriteDataBlock(flashPagePtr, storeArray);
    if(flashStatus != FLASH_PASS)
    {
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(flashStatus == FLASH_PARAM)
       {
          return FTL_ERR_FAIL;
       }
       if((status = TransLogEBFailure(flashPagePtr, storeArray)) != FTL_ERR_PASS)
       {
          return status;
       }

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return FTL_ERR_LOG_WR;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    flashPagePtr->byteCount = byteCount;

    if(writeLogFlag == FTL_FALSE)
    {
       writeLogFlag = FTL_TRUE;
    }
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS FTL_WriteFlushInfo(FLASH_PAGE_INFO_PTR flashPagePtr, SYS_EBLOCK_FLUSH_INFO_PTR flushInfoPtr)
{
    UINT16_PTR flushInfoEntry = (UINT16_PTR) flushInfoPtr; /*4*/

    flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);
    if(FLASH_RamPageWriteMetaData(flashPagePtr, (UINT8_PTR)flushInfoEntry) != FLASH_PASS)
    {
       return FTL_ERR_FLASH_WRITE_08;
    }
    return FTL_ERR_PASS;
}

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#if (FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
//----------------------------------
FTL_STATUS InitPackedSuperInfo(void)
{
    UINT32 count = 0;                                  /*4*/

    for(count = 0; count < SECTOR_SIZE; count++)
    {
       packedSuperInfo[count] = EMPTY_BYTE;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS CopyPackedSuperInfo(UINT16 offset, UINT8_PTR logPtr)
{
    UINT16_PTR logEntry = (UINT16_PTR)logPtr;          /*4*/

    logEntry[LOG_ENTRY_CHECK_WORD] = CalcCheckWord(&logEntry[LOG_ENTRY_DATA_START], LOG_ENTRY_DATA_WORDS);
    MEM_Memcpy(&packedSuperInfo[(offset * LOG_ENTRY_SIZE)], logPtr, LOG_ENTRY_SIZE);
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS WritePackedSuperInfo(FLASH_PAGE_INFO_PTR flashPagePtr)
{
    UINT32 byteCount = 0;                              /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    byteCount = flashPagePtr->byteCount;
    flashPagePtr->byteCount = SECTOR_SIZE;
    if((flashStatus = FLASH_RamPageWriteDataBlock(flashPagePtr, &packedSuperInfo[0])) != FLASH_PASS)
    {
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(flashStatus == FLASH_PARAM)
       {
          return FTL_ERR_FAIL;
       }
       if(FLASH_MarkDefectEBlock(flashPagePtr) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on.
       }
       return FTL_ERR_SUPER_WRITE_02;
       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return FTL_ERR_SUPER_WRITE_02;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    flashPagePtr->byteCount = byteCount;
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

//-----------------------------
FTL_STATUS FTL_WriteSuperInfo(FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr)
{
    UINT16_PTR logEntry = (UINT16_PTR) logPtr;         /*4*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT32 byteCount = 0;                              /*4*/
    UINT32 count = 0;                                  /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    logEntry[LOG_ENTRY_CHECK_WORD] = CalcCheckWord(&logEntry[LOG_ENTRY_DATA_START], LOG_ENTRY_DATA_WORDS);

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    if(FLASH_RamPageWriteMetaData(flashPagePtr, (UINT8_PTR)logEntry) != FLASH_PASS)
    {
       return FTL_ERR_SUPER_WRITE_05;
    }

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    MEM_Memcpy(&pseudoRPB[flashPagePtr->devID][0], logPtr, (UINT16)flashPagePtr->byteCount);
    for(count = flashPagePtr->byteCount; count < SECTOR_SIZE; count++)
    {
       pseudoRPB[flashPagePtr->devID][count] = EMPTY_BYTE;
    }
    byteCount = flashPagePtr->byteCount;
    flashPagePtr->byteCount = SECTOR_SIZE;
    flashStatus = FLASH_RamPageWriteDataBlock(flashPagePtr, &pseudoRPB[flashPagePtr->devID][0]);
    if(flashStatus != FLASH_PASS)
    {
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(flashStatus == FLASH_PARAM)
       {
          return FTL_ERR_FAIL;
       }
       if(FLASH_MarkDefectEBlock(flashPagePtr) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on.
       }
       return FTL_ERR_SUPER_WRITE_03;
       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return FTL_ERR_SUPER_WRITE_03;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    flashPagePtr->byteCount = byteCount;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    return FTL_ERR_PASS;
}
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)


//---------------------------------------
FTL_STATUS FTL_WriteSysEBlockInfo(FLASH_PAGE_INFO_PTR flashPagePtr, SYS_EBLOCK_INFO_PTR sysInfoPtr)
{
    UINT16_PTR localPtr = (UINT16_PTR) sysInfoPtr;     /*4*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT32 byteCount = 0;                              /*4*/
    UINT32 count = 0;                                  /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    // Write sector once
    localPtr[SYS_INFO_CHECK_WORD] = CalcCheckWord(&localPtr[SYS_INFO_DATA_START], SYS_INFO_DATA_WORDS);

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    if((FLASH_RamPageWriteMetaData(flashPagePtr, (UINT8_PTR)localPtr)) != FLASH_PASS)
    {
       return FTL_ERR_FLASH_WRITE_12;
    }

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    MEM_Memcpy(&pseudoRPB[flashPagePtr->devID][0], (UINT8_PTR)sysInfoPtr, (UINT16)flashPagePtr->byteCount);
    for(count = flashPagePtr->byteCount; count < SECTOR_SIZE; count++)
    {
       pseudoRPB[flashPagePtr->devID][count] = EMPTY_BYTE;
    }
    byteCount = flashPagePtr->byteCount;
    flashPagePtr->byteCount = SECTOR_SIZE;
    flashStatus = FLASH_RamPageWriteDataBlock(flashPagePtr, &pseudoRPB[flashPagePtr->devID][0]);
    if(flashStatus != FLASH_PASS)
    {
       if(flashStatus == FLASH_PARAM)
       {
          return FTL_ERR_FAIL;
       }
       return FTL_ERR_FLASH_WRITE_12;
    }
    flashPagePtr->byteCount = byteCount;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    return FTL_ERR_PASS;
}
#endif  // #else  // #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)

//---------------------------------
FTL_STATUS FTL_EraseAllTransLogBlocksOp(FTL_DEV devID)
{
    FTL_STATUS status     = FTL_ERR_PASS;              /*4*/
    UINT8  tempCount      = 0x0;                       /*1*/
    UINT16 logicalAddr    = 0x0;                       /*2*/
    UINT16 phyEBAddr      = 0x0;                       /*2*/
    UINT16 eBlockNum      = 0x0;                       /*2*/
    UINT32 latestIncNumber= 0x0;                       /*4*/
    UINT16 nextLogicalEBAddr = 0;                      /*2*/
    UINT16 nextPhyEBAddr = 0;                          /*2*/
    FLASH_PAGE_INFO flashPageInfo = {0, 0, {0, 0}};    /*11*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    SYS_EBLOCK_INFO_PTR tempSysPtr = NULL;             /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT8 checkBBMark = FTL_FALSE;                   /*1*/
    #endif

    if((status = FTL_FindEmptyTransLogEBlock(devID, &nextLogicalEBAddr, &nextPhyEBAddr)) != FTL_ERR_PASS)
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

    #if (DEBUG_FTL_API_ANNOUNCE == 1)
    DBG_Printf("FTL_EraseAllTransLogBlocksOp: Eb picked=0x%x \n", nextLogicalEBAddr, 0);
    #endif  

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    eraseStatus = GetEBErased(devID, nextLogicalEBAddr);

    #if DEBUG_PRE_ERASED
    if(FTL_TRUE == eraseStatus)
    {
       DBG_Printf("FTL_EraseAllTransLogBlocksOp: EBlock 0x%X is already erased\n", nextLogicalEBAddr, 0);
    }
    #endif  // #if DEBUG_PRE_ERASED
    if(FTL_FALSE == eraseStatus)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    {

       if((status = FTL_EraseOp(devID, nextLogicalEBAddr)) != FTL_ERR_PASS)
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(status == FTL_ERR_FAIL)
          {
             return status;
          }
          SetBadEBlockStatus(devID, nextLogicalEBAddr, FTL_TRUE);
          flashPageInfo.devID = devID;
          flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(nextPhyEBAddr, 0);
          flashPageInfo.vPage.pageOffset = 0;
          flashPageInfo.byteCount = 0;
          if(FLASH_MarkDefectEBlock(&flashPageInfo) != FLASH_PASS)
          {
             // do nothing, just try to mark bad, even if it fails we move on.
          }
          return FTL_ERR_LOG_NEW_EBLOCK_FAIL;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return status;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
    }
    flashPageInfo.devID            = devID;
    flashPageInfo.vPage.pageOffset = 0;
    flashPageInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
    flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(nextPhyEBAddr, 0);
    //construct the system info for the Log Eblock with new key
    sysEBlockInfo.phyAddrThisEBlock = nextPhyEBAddr;
    sysEBlockInfo.incNumber      = (GetTransLogEBCounter(devID) + 1);
    sysEBlockInfo.type           = SYS_EBLOCK_INFO_LOG;
    sysEBlockInfo.checkVersion   = EMPTY_WORD;
    sysEBlockInfo.oldSysBlock    = EMPTY_WORD;
    sysEBlockInfo.fullFlushSig   = EMPTY_WORD;
    for(tempCount = 0; tempCount < sizeof(sysEBlockInfo.reserved); tempCount++)
    {
       sysEBlockInfo.reserved[tempCount] = EMPTY_BYTE;
    }
    if((status = FTL_WriteSysEBlockInfo(&flashPageInfo, &sysEBlockInfo)) != FTL_ERR_PASS)
    {

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(status == FTL_ERR_FAIL)
       {
          return status;
       }
       SetBadEBlockStatus(devID, nextLogicalEBAddr, FTL_TRUE);
       if(FLASH_MarkDefectEBlock(&flashPageInfo) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on.
       }
       return FTL_ERR_LOG_NEW_EBLOCK_FAIL;

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return status;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, nextLogicalEBAddr, FTL_FALSE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    // Initialize EBlock Info entries
    SetDirtyCount(devID, nextLogicalEBAddr, 0);
    SetGCOrFreePageNum(devID, nextLogicalEBAddr, 1);
    MarkEBlockMappingTableEntryDirty(devID, nextLogicalEBAddr);
    SetTransLogEBCounter(devID, sysEBlockInfo.incNumber);
    // This is required to overwrite the old entry.
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {
       if((status = TABLE_GetTransLogEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
       {
          return status; // trying to excess outside table.
       }
       if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
       {
          break; // no more entries in table
       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
       if((status = TABLE_InsertReservedEB(devID, logicalAddr)) != FTL_ERR_PASS)
       {
             return status;
       }
       #endif
       //write  OLD_SYS_BLOCK_SIGNATURE
       sysEBlockInfo.oldSysBlock    = OLD_SYS_BLOCK_SIGNATURE;
       flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, 0);
       flashPageInfo.vPage.pageOffset = (UINT16)((UINT32)&(tempSysPtr->oldSysBlock));
       flashPageInfo.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
//     flashPageInfo.byteCount = sizeof(sysEBlockInfo.oldSysBlock);
       flashStatus = FLASH_RamPageWriteMetaData(&flashPageInfo, (UINT8_PTR)&sysEBlockInfo.oldSysBlock);
       if(flashStatus != FLASH_PASS)
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(flashStatus == FLASH_PARAM)
          {
             return FTL_ERR_FLASH_WRITE_09;
          }
          SetBadEBlockStatus(devID, logicalAddr, FTL_TRUE);
          // just try to mark bad, even if it fails we move on.
          FLASH_MarkDefectEBlock(&flashPageInfo);

          checkBBMark = FTL_TRUE;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return FTL_ERR_FLASH_WRITE_09;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if((status = TABLE_InsertReservedEB(devID, logicalAddr)) != FTL_ERR_PASS)
       {
             return status;
       }
       #endif
    }
    if((status = TABLE_TransEBClear(devID)) != FTL_ERR_PASS)
    {
       return status;
    }
    if((status = TABLE_TransLogEBInsert(devID, nextLogicalEBAddr, nextPhyEBAddr, sysEBlockInfo.incNumber)) != FTL_ERR_PASS)
    {
       return status;
    }

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    writeLogFlag = FTL_FALSE;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    if(FTL_TRUE == checkBBMark)
    {
       return FTL_ERR_MARKBB_COMMIT;
    }
    #endif

    return FTL_ERR_PASS;
}


#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
//------------------------------
FTL_STATUS GetNextSuperSysEBEntryLocation(FTL_DEV devID, FLASH_PAGE_INFO_PTR pageInfoPtr, UINT16_PTR entryIndexPtr)  /*  1,  4*/
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 latestIncNumber = EMPTY_DWORD;              /*4*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    UINT16 logicalBlockNum = EMPTY_WORD;               /*2*/
    UINT16 entryIndex = EMPTY_WORD;                    /*2*/
    UINT16 byteOffset = 0;                             /*2*/

    // Initialize variables
    pageInfoPtr->devID = devID;
    pageInfoPtr->byteCount = LOG_ENTRY_SIZE;
    latestIncNumber = GetSuperSysEBCounter(devID);
    status = TABLE_SuperSysEBGetLatest(devID, &logicalBlockNum, &phyEBlockAddr, latestIncNumber);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    // Get offset to free entry
    entryIndex = GetFreePageIndex(devID, logicalBlockNum);
    if (entryIndex < NUM_LOG_ENTRIES_PER_EBLOCK)
    {
       // Latest EBlock has room for more
       byteOffset = entryIndex * LOG_ENTRY_DELTA;
       pageInfoPtr->vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, entryIndex);
       pageInfoPtr->vPage.pageOffset = byteOffset % VIRTUAL_PAGE_SIZE;
       SetGCOrFreePageNum(devID, logicalBlockNum, entryIndex + 1);
       *entryIndexPtr = entryIndex;
       MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);

       //#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       //SetTransLogEBNumBadBlockInfo(logicalBlockNum);
       //#endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    else
    {
       return FTL_ERR_SUPER_CANNOT_FIND_NEXT_ENTRY;
    }

    return FTL_ERR_PASS;
}

//---------------------------------
FTL_STATUS CreateNextSuperSystemEBlockOp(FTL_DEV devID)
{
    FTL_STATUS status     = FTL_ERR_PASS;              /*4*/
    UINT8  tempCount      = 0x0;                       /*1*/
    UINT16 logicalAddr    = 0x0;                       /*2*/
    UINT16 phyEBAddr      = 0x0;                       /*2*/
    UINT16 eBlockNum      = 0x0;                       /*2*/
    UINT32 latestIncNumber= 0x0;                       /*4*/
    UINT16 nextLogicalEBAddr = 0;                      /*2*/
    UINT16 nextPhyEBAddr = 0;                          /*2*/
    FLASH_PAGE_INFO flashPageInfo = {0, 0, {0, 0}};    /*11*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    SYS_EBLOCK_INFO_PTR tempSysPtr = NULL;             /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;             /*1*/

    UINT16 findDataEBNum; 
    UINT16 findSysEBNum;

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;                          /*2*/
    UINT16 findTries = 0;                              /*2*/
    UINT8  checkBBMark = FTL_FALSE;                       /*1*/
    #endif  // #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    do{
       findTries++;
       if(findTries > 2)
       {
          return status;
       }
    #endif

       if((status = FTL_FindEmptySuperSysEBlock(devID, &nextLogicalEBAddr, &nextPhyEBAddr)) != FTL_ERR_PASS)
       {
          #if DEBUG_CHECK_GET_SUPER
          DBG_Printf("Fail: FTL_FindEmptySuperSysEBlock: Reserved Area\n", 0, 0);
          #endif
          FTL_FindAllAreaSuperSysEBlock(devID, &findDataEBNum, &findSysEBNum);
          if(1 <= findSysEBNum)
          {
             gProtectForSuperSysEBFlag = FTL_TRUE;
             #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             sanityCounter = 0;
             while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
             {
                if((status = Flush_GC(devID)) != FTL_ERR_FLUSH_FLUSH_GC_FAIL)
                {
                   break;
                }
                sanityCounter++;
             }
             if(status != FTL_ERR_PASS)
             {
                return status;
             }
             #else
             if((status = Flush_GC(devID)) != FTL_ERR_PASS)
             {
                return status;
             }
             #endif
             gProtectForSuperSysEBFlag = FTL_FALSE;
             if((status = FTL_FindEmptySuperSysEBlock(devID, &nextLogicalEBAddr, &nextPhyEBAddr)) != FTL_ERR_PASS)
             {
                #if DEBUG_CHECK_GET_SUPER
                DBG_Printf("Fail: FTL_FindEmptySuperSysEBlock: System Area\n", 0, 0);
                #endif
                #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                continue;
                #else
                return status;
                #endif
             }
          }else if(1 <= findDataEBNum ){

             if((status = DataGCForSuperSysEB()) != FTL_ERR_PASS)
             {
                return status;
             }
             if((status = FTL_FindEmptySuperSysEBlock(devID, &nextLogicalEBAddr, &nextPhyEBAddr)) != FTL_ERR_PASS)
             {
                #if DEBUG_CHECK_GET_SUPER
                DBG_Printf("Fail: FTL_FindEmptySuperSysEBlock: Data Area\n", 0, 0);
                #endif
                #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                continue;
                #else
                return status;
                #endif
             }
          }
       }
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    } while (status != FTL_ERR_PASS);
    #endif

    #if (DEBUG_FTL_API_ANNOUNCE == 1)
    DBG_Printf("CreateNextSuperSystemEBlockOp: Eb picked=0x%x \n", nextLogicalEBAddr, 0);
    #endif


    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    eraseStatus = GetEBErased(devID, nextLogicalEBAddr);

    #if DEBUG_PRE_ERASED
    if(FTL_TRUE == eraseStatus)
    {
       DBG_Printf("CreateNextSuperSystemEBlockOp: EBlock 0x%X is already erased\n", nextLogicalEBAddr, 0);
    }
    #endif  // #if DEBUG_PRE_ERASED
    if(FTL_FALSE == eraseStatus)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    {

       if((status = FTL_EraseOp(devID, nextLogicalEBAddr)) != FTL_ERR_PASS)
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(status == FTL_ERR_FAIL)
          {
             return status;
          }          
          SetBadEBlockStatus(devID, nextLogicalEBAddr, FTL_TRUE);
          flashPageInfo.devID = devID;
          flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(nextPhyEBAddr, 0);
          flashPageInfo.vPage.pageOffset = 0;
          flashPageInfo.byteCount = 0;
          if(FLASH_MarkDefectEBlock(&flashPageInfo) != FLASH_PASS)
          {
             // do nothing, just try to mark bad, even if it fails we move on.
          }
          return FTL_ERR_SUPER_LOG_NEW_EBLOCK_FAIL_01;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return status;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
    }
    flashPageInfo.devID            = devID;
    flashPageInfo.vPage.pageOffset = 0;
    flashPageInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
    flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(nextPhyEBAddr, 0);
    //construct the system info for the Log Eblock with new key
    sysEBlockInfo.phyAddrThisEBlock = nextPhyEBAddr;
    sysEBlockInfo.incNumber      = (GetSuperSysEBCounter(devID) + 1);
    sysEBlockInfo.type           = SYS_EBLOCK_INFO_SUPER;
    sysEBlockInfo.checkVersion   = EMPTY_WORD;
    sysEBlockInfo.oldSysBlock    = EMPTY_WORD;
    sysEBlockInfo.fullFlushSig   = EMPTY_WORD;
    for(tempCount = 0; tempCount < sizeof(sysEBlockInfo.reserved); tempCount++)
    {
       sysEBlockInfo.reserved[tempCount] = EMPTY_BYTE;
    }
    if((status = FTL_WriteSysEBlockInfo(&flashPageInfo, &sysEBlockInfo)) != FTL_ERR_PASS)
    {

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(status == FTL_ERR_FAIL)
       {
          return status;
       }       
       SetBadEBlockStatus(devID, nextLogicalEBAddr, FTL_TRUE);
       if(FLASH_MarkDefectEBlock(&flashPageInfo) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on.
       }
       return FTL_ERR_SUPER_LOG_NEW_EBLOCK_FAIL_02;

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return status;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, nextLogicalEBAddr, FTL_FALSE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    // Initialize EBlock Info entries
    SetDirtyCount(devID, nextLogicalEBAddr, 0);
    SetGCOrFreePageNum(devID, nextLogicalEBAddr, 1);
    MarkEBlockMappingTableEntryDirty(devID, nextLogicalEBAddr);
    SetSuperSysEBCounter(devID, sysEBlockInfo.incNumber);
    // This is required to overwrite the old entry.
    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {
       if((status = TABLE_GetSuperSysEBEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
       {
          return status; // trying to excess outside table.
       }
       if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
       {
          break; // no more entries in table
       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
       if((status = TABLE_InsertReservedEB(devID, logicalAddr)) != FTL_ERR_PASS)
       {
             return status;
       }
       #endif
       //write  OLD_SYS_BLOCK_SIGNATURE
       sysEBlockInfo.oldSysBlock    = OLD_SYS_BLOCK_SIGNATURE;
       flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, 0);
       flashPageInfo.vPage.pageOffset = (UINT16)((UINT32)&(tempSysPtr->oldSysBlock));
       flashPageInfo.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
       flashStatus = FLASH_RamPageWriteMetaData(&flashPageInfo, (UINT8_PTR)&sysEBlockInfo.oldSysBlock);
       if(flashStatus != FLASH_PASS)
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(FLASH_PARAM == flashStatus)
          {
             return FTL_ERR_SUPER_FLASH_WRITE_01;
          }
          SetBadEBlockStatus(devID, logicalAddr, FTL_TRUE);
          // just try to mark bad, even if it fails we move on.
          FLASH_MarkDefectEBlock(&flashPageInfo);
          checkBBMark = FTL_TRUE;
          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return FTL_ERR_SUPER_FLASH_WRITE_01;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(FTL_FALSE == checkBBMark)
       {
          if((status = TABLE_InsertReservedEB(devID, logicalAddr)) != FTL_ERR_PASS)
          {
             return status;
          }
          checkBBMark = FTL_FALSE;
       }
       #endif
    }
    if((status = TABLE_SuperSysEBClear(devID)) != FTL_ERR_PASS)
    {
       return status;
    }
    if((status = TABLE_SuperSysEBInsert(devID, nextLogicalEBAddr, nextPhyEBAddr, sysEBlockInfo.incNumber)) != FTL_ERR_PASS)
    {
       return status;
    }

    return FTL_ERR_PASS;
}
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)


FTL_STATUS FTL_RemoveOldTransLogBlocks(FTL_DEV devID, UINT32 logEBCounter)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 eBlockCount = 0;                            /*2*/
    UINT16 blockNum = EMPTY_WORD;                      /*2*/
    UINT16 logicalEBAddr = EMPTY_WORD;                 /*2*/
    UINT16 physicalEBAddr = EMPTY_WORD;                /*2*/
    UINT32 key = EMPTY_DWORD;                          /*4*/

    for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetTransLogEntry(devID, eBlockCount, &logicalEBAddr, &physicalEBAddr, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key == EMPTY_DWORD)
       {
          break;
       }
       if(key <= logEBCounter)
       {
          blockNum = eBlockCount;
       }
    }
    if(blockNum != EMPTY_WORD)
    {
       if((status = TABLE_TransLogEBRemove(devID, blockNum)) != FTL_ERR_PASS)
       {
          return status;
       }
    }

    return FTL_ERR_PASS;
}

///----------------------------------
FTL_STATUS FTL_EraseOp(FTL_DEV devID, UINT16 logicalEBNum)
{
    FTL_STATUS status = FTL_ERR_PASS;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
    UINT32 eraseCount = 0;
    UINT16 lowLogicalEBNum = 0;
    UINT32 lowestCount = 0;
    #endif
    #endif

    status = FTL_EraseOpNoDirty(devID, logicalEBNum);
    if(status != FTL_ERR_PASS)
    {
       return status;
    }
    IncEraseCount(devID, logicalEBNum);

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
    eraseCount = GetTrueEraseCount(devID, logicalEBNum);
    status = SetSaveStaticWL(devID, logicalEBNum, eraseCount);
    if (status != FTL_ERR_PASS)
    {
       return status;
    }
    status = GetSaveStaticWL(devID, &lowLogicalEBNum, &lowestCount, CACHE_WL_LOW);
    if (status != FTL_ERR_PASS)
    {
       return status;
    }
    if (lowLogicalEBNum == logicalEBNum)
    {
       status = ClearSaveStaticWL(devID, logicalEBNum, eraseCount, CACHE_WL_LOW);
       if (status != FTL_ERR_PASS)
       {
          return status;
       }
    }
    #endif
    #endif
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, logicalEBNum, FTL_TRUE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    return FTL_ERR_PASS;
}

FTL_STATUS FTL_EraseOpNoDirty(FTL_DEV devID, UINT16 logicalEBNum)
{
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    UINT16 phyEBNum = 0;                               /*2*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    phyEBNum = GetPhysicalEBlockAddr(devID, logicalEBNum);
    pageInfo.devID = devID;
    pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyEBNum, 0);
    pageInfo.vPage.pageOffset = 0;
    pageInfo.byteCount = 0;

    #if DEBUG_ERASE_OP
    DBG_Printf("FTL_EraseOp: Erase Logical EBlock 0x%X, ", logicalEBNum, 0);
    DBG_Printf("physical EBlock 0x%X\n", phyEBNum, 0);
    #endif  // #if DEBUG_ERASE_OP

    flashStatus = FLASH_Erase(&pageInfo);
    if(flashStatus != FLASH_PASS)
    {
       if(flashStatus == FLASH_PARAM)
       {
          return FTL_ERR_FAIL;
       }
       return FTL_ERR_FLASH_ERASE_01;
    }
    return FTL_ERR_PASS;
}
//---------------------------------
UINT32 GetTotalDirtyBitCnt(FTL_DEV devID)
{
    UINT16 secCnt = 0;                                 /*2*/
    UINT32 dirtyBitTotalCnt = 0;                       /*4*/

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    for(secCnt = 0; secCnt < NUM_EBLOCK_MAP_INDEX; secCnt++)
    {
       if(FTL_TRUE == CACHE_IsDirtyIndex(devID, secCnt))
       {
          dirtyBitTotalCnt++; // EB
          dirtyBitTotalCnt = dirtyBitTotalCnt + PPA_CACHE_TABLE_OFFSET; // PPA 
       }
    }
    #else
    for(secCnt = 0; secCnt < BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE; secCnt++)
    {
       if(IsEBlockMappingTableSectorDirty(devID, secCnt))
       {
          dirtyBitTotalCnt++;
       }
    }
    for(secCnt = 0; secCnt < BITS_PPA_DIRTY_BITMAP_DEV_TABLE; secCnt++)
    {
       if(IsPPAMappingTableSectorDirty(devID, secCnt))
       {
          dirtyBitTotalCnt++;
       }
    }
    #endif
    return dirtyBitTotalCnt;
}

//----------------------------------
FTL_STATUS CheckFlushSpace(FTL_DEV devID)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 phyEBlockAddr    = 0x0;                     /*2*/
    UINT16 freeIndex        = 0x0;                     /*2*/
    UINT16 logicalEBlockNum = 0x0;                     /*2*/
    UINT32 latestIncNumber  = 0x0;                     /*4*/
    UINT32 dirtyBitTotalCnt = 0x0;                     /*4*/

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;                          /*2*/
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    latestIncNumber = GetFlushEBCounter(devID);
    if((status = TABLE_FlushEBGetLatest(devID, &logicalEBlockNum, &phyEBlockAddr, latestIncNumber)) != FTL_ERR_PASS)
    {
       return status;
    }
    freeIndex = GetFreePageIndex(devID, logicalEBlockNum);
    MarkEBlockMappingTableEntryDirty(devID, logicalEBlockNum);
    // Get the total dirty bits
    dirtyBitTotalCnt = GetTotalDirtyBitCnt(devID);
    /* (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK-1) is used instead of (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK). */
    #if(CACHE_RAM_BD_MODULE == FTL_FALSE)
    if(dirtyBitTotalCnt >= (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK-1))
    #else
    if(dirtyBitTotalCnt >= (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK - (1 + PPA_CACHE_TABLE_OFFSET)))
    #endif
    {
       return FTL_ERR_FLUSH_GC_NEEDED;
    }
    if((freeIndex + dirtyBitTotalCnt) >= MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK)
    {
       if(TABLE_GetReservedEBlockNum(devID) <= MIN_SYSTEM_RESERVE_EBLOCKS)
       {
          return FTL_ERR_FLUSH_GC_NEEDED;
       }
       if(GetFlushLogEBArrayCount(devID) >= NUM_FLUSH_LOG_EBLOCKS)
       {
          return FTL_ERR_FLUSH_GC_NEEDED;
       }
       SetGCOrFreePageNum(devID, logicalEBlockNum, MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK);

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
       {
          if((status = TABLE_GetReservedEB(devID, &logicalEBlockNum, FTL_FALSE)) != FTL_ERR_PASS)
          {
             return status;
          }
          if((status = CreateNextFlushEntryLocation(devID, logicalEBlockNum)) != FTL_ERR_FLUSH_NEXT_EBLOCK_FAIL)
          {
             break;
          }
          sanityCounter++;
       }
       if(status != FTL_ERR_PASS)
       {
          return status;
       }

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if((status = TABLE_GetReservedEB(devID, &logicalEBlockNum, FTL_FALSE)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = CreateNextFlushEntryLocation(devID, logicalEBlockNum)) != FTL_ERR_PASS)
       {
          return status;
       }
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    return FTL_ERR_PASS;
}

FTL_STATUS GetLogEntryLocation(FTL_DEV devID, LOG_ENTRY_LOC_PTR nextLoc)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 physicalEBNum = EMPTY_WORD;                 /*2*/
    UINT32 key = 0;                                    /*4*/
    UINT16 count = 0;                                  /*2*/
    UINT16 logEBAddr = EMPTY_WORD;                     /*2*/
    UINT16 phyEBAddr = EMPTY_WORD;                     /*2*/
    UINT32 latestIncNumber = 0;                        /*4*/
    UINT16 eBlockNum = EMPTY_WORD;                     /*2*/
    UINT16 entryIndex = EMPTY_WORD;                    /*2*/

    key = GetTransLogEBCounter(devID);
    if((status = TABLE_TransLogEBGetLatest(devID, &logicalEBNum, &physicalEBNum, key)) != FTL_ERR_PASS)
    {
       return status;
    }
    for(count = 0; count < NUM_TRANSACTION_LOG_EBLOCKS; count++)
    {
       if((status = TABLE_GetTransLogEntry(devID, count, &logEBAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((logEBAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
       {
          break;
       }
       if((logEBAddr == logicalEBNum) && (phyEBAddr == physicalEBNum) && (latestIncNumber == key))
       {
          eBlockNum = count;
          break;
       }
    }
    if(eBlockNum != EMPTY_WORD)
    {
       entryIndex = GetFreePageIndex(devID, logicalEBNum);
       nextLoc->eBlockNum = eBlockNum;
       nextLoc->entryIndex = entryIndex;
    }
    else
    {
       return FTL_ERR_LOG_NO_FOUND_EBLOCK;
    }
    return FTL_ERR_PASS;
}

//-----------------------------------
FTL_STATUS TABLE_FlushDevice(FTL_DEV devID, UINT8 flushMode)
{
    FTL_STATUS status = FTL_ERR_PASS;                       /*4*/
    UINT8  numRamTables       = 0x2;                        /*1*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT8  bitMapData         = 0x0;                        /*1*/
    #endif
    UINT8  flushTypeCnt       = 0x0;                        /*1*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 dataByteCnt        = 0x0;                        /*4*/
    #endif
    UINT16 logicalBlockNum    = 0x0;                        /*2*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 totalRamTableBytes = 0x0;                        /*4*/
    UINT16 bitMapCounter      = 0x0;                        /*2*/
    #endif
    UINT32 dirtyBitTotalCnt   = 0x0;                        /*4*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT32 dirtyBitCounter    = 0x0;                        /*4*/
    UINT16 tempCount = 0;                                   /*2*/
    UINT8_PTR dirtyBitMapPtr     = NULL;                    /*4*/
    UINT8_PTR ramMappingTablePtr = NULL;                    /*4*/
    #endif
    FLASH_PAGE_INFO flushStructPageInfo = {0, 0, {0, 0}};   /*11*/
    FLASH_PAGE_INFO flushRAMTablePageInfo = {0, 0, {0, 0}}; /*11*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    SYS_EBLOCK_FLUSH_INFO sysEBlockFlushInfo;               /*16*/
    UINT8_PTR srcPtr = NULL;                                /*4*/
    UINT32 tempSize = 0;                                    /*4*/
    UINT16 maxTableOffset = 0;                              /*2*/
    LOG_ENTRY_LOC nextLoc = {0, 0};                         /*4*/
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    CACHE_INFO_EBMCACHE ebmCacheInfo = { 0, 0, 0, 0 };
    CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo = { 0, 0 };
    CACHE_INFO_RAMMAP ramMapInfo = { 0, 0, 0, 0 };
    UINT16 indexCount = 0;
    UINT16 blockNum = 0;
    UINT16 totalBlock = 0;
    UINT16 logicalEBNumTmp = 0;
    UINT16 phyEBlockAddrTmp = 0;
    UINT32 key = 0;
    UINT16 saveIndex[NUM_FLUSH_LOG_EBLOCKS];
    UINT8 skipFlag = FTL_FALSE;
//    UINT16 freeIndex = 0;
    UINT16 latestLogicalEBNum = 0;
    UINT16 currentLogicalEBNum = 0;
    UINT32 keyTmp = 0;
    #endif

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT16_PTR flushInfoEntry = NULL;                       /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;
    #endif
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    #if (DEBUG_FTL_API_ANNOUNCE == 1)
    DBG_Printf("TABLE_FlushDevice: \n", 0, 0);
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    for(tempCount = 0; tempCount < sizeof(sysEBlockFlushInfo.reserved); tempCount++)
    {
       sysEBlockFlushInfo.reserved[tempCount] = EMPTY_BYTE;
    }

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    sysEBlockFlushInfo.tableCheckWord = EMPTY_WORD;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    // For Flush_GC case
    key = GetFlushEBCounter(devID);
    if ((status = TABLE_FlushEBGetLatest(devID, &latestLogicalEBNum, &phyEBlockAddrTmp, key)) != FTL_ERR_PASS)
    {
       return status;
    }
    if ((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
       &flushRAMTablePageInfo, &currentLogicalEBNum)) != FTL_ERR_PASS)
    {
       return status;
    }
    if (latestLogicalEBNum != currentLogicalEBNum)
    {
       // set dirty mark
       skipFlag = FTL_FALSE;
       for (blockNum = 0; blockNum < NUM_FLUSH_LOG_EBLOCKS; blockNum++)
       {
          TABLE_GetFlushLogEntry(devID, blockNum, &logicalEBNumTmp, &phyEBlockAddrTmp, &keyTmp);
          if (EMPTY_WORD == logicalEBNumTmp)
          {
             break;
          }
          if (logicalEBNumTmp == currentLogicalEBNum)
          {
             key = keyTmp;
             skipFlag = FTL_TRUE;
          }
          if ((FTL_TRUE == skipFlag) && (keyTmp >= key))
          {
             MarkEBlockMappingTableEntryDirty(devID, logicalEBNumTmp);
          }
       }
    }

    dirtyBitTotalCnt = gCounterDirty; // Save dirty count
    if(gCounterDirty == 0)
    {
       return FTL_ERR_PASS;
    }

    totalBlock = 0;
    for (blockNum = 0; blockNum < NUM_FLUSH_LOG_EBLOCKS; blockNum++)
    {
       TABLE_GetFlushLogEntry(devID, blockNum, &logicalEBNumTmp, &phyEBlockAddrTmp, &key);
       if (EMPTY_WORD == logicalEBNumTmp)
       {
           continue;
       }
       if (FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNumTmp, &ramMapInfo)))
       {
           return status;
       }
       if (FTL_TRUE == ramMapInfo.presentEBM)
       {
           skipFlag = FTL_FALSE;
           for (indexCount = 0; indexCount < totalBlock; indexCount++)
           {
               if (saveIndex[indexCount] == ramMapInfo.ebmCacheIndex)
               {
                   skipFlag = FTL_TRUE;
                   break;
               }
           }
           if (FTL_TRUE == skipFlag)
           {
               continue;
           }
           saveIndex[totalBlock] = ramMapInfo.ebmCacheIndex; // Save flush eb
           totalBlock++;
       }
    }


    for(flushTypeCnt = 0; flushTypeCnt < numRamTables; flushTypeCnt++)
    {
       gCounterDirty = (UINT16)dirtyBitTotalCnt;
       for(indexCount = 0;indexCount < NUM_EBLOCK_MAP_INDEX; indexCount++)
       {
          if (flushTypeCnt == 1)
          {
             skipFlag = FTL_FALSE;
             for (blockNum = 0; blockNum < totalBlock; blockNum++)
             {
                if (saveIndex[blockNum] == indexCount)
                {
                   skipFlag = FTL_TRUE;
                   break;
                }
             }
             if (FTL_TRUE == skipFlag)
             {
                continue;
             }
          }
          if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, indexCount, &ebmCacheInfo)))
          {
             return status;
          }

          if(CACHE_DIRTY == ebmCacheInfo.cacheStatus)
          {

             if((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo, 
               &flushRAMTablePageInfo, &logicalBlockNum)) != FTL_ERR_PASS)
             {
                return status; // go Flush GC.
             }

             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, logicalBlockNum, FTL_FALSE);
             #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

             // update position
             eBlockPPAMapInfo.entryIndex = (UINT16)GetGCNum(devID, logicalBlockNum);
             TABLE_GetFlushLogCacheEntry(devID, GetPhysicalEBlockAddr(devID, logicalBlockNum), &eBlockPPAMapInfo.flashLogEBArrayCount);

             if(flushTypeCnt == 1)
             {
                // Flush EBM table
                 if (FTL_ERR_PASS != (status = CACHE_CacheToFlash(devID, indexCount, eBlockPPAMapInfo, CACHE_EBLOCKMAP, flushMode)))
                {
                   return status;
                }

             }else{

                // Flush PPA table
                if (FTL_ERR_PASS != (status = CACHE_CacheToFlash(devID, indexCount, eBlockPPAMapInfo, CACHE_PPAMAP, flushMode)))
                {
                   return status;
                }
             }
          }
       }
    }


    for (blockNum = 0; blockNum < totalBlock; blockNum++)
    {

       if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, saveIndex[blockNum], &ebmCacheInfo)))
       {
          return status;
       }

       if (CACHE_DIRTY == ebmCacheInfo.cacheStatus)
       {

          // Flush EBM table
          if ((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
                &flushRAMTablePageInfo, &logicalBlockNum)) != FTL_ERR_PASS)
          {
             return status; // go Flush GC.
          }

          #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
          SetEBErased(devID, logicalBlockNum, FTL_FALSE);
          #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

          // update position
          eBlockPPAMapInfo.entryIndex = (UINT16)GetGCNum(devID, logicalBlockNum);
          TABLE_GetFlushLogCacheEntry(devID, GetPhysicalEBlockAddr(devID, logicalBlockNum), &eBlockPPAMapInfo.flashLogEBArrayCount);

          if (FTL_ERR_PASS != (status = CACHE_CacheToFlash(devID, saveIndex[blockNum], eBlockPPAMapInfo, CACHE_EBLOCKMAP, flushMode)))
          {
             return status;
          }
       }
    }

    // clear dirty
    if(FTL_ERR_PASS != (status = CACHE_CleanAllDirtyIndex(devID)))
    {
       return status;
    }    

    // For Flush_GC case
    key = GetFlushEBCounter(devID);
    if ((status = TABLE_FlushEBGetLatest(devID, &latestLogicalEBNum, &phyEBlockAddrTmp, key)) != FTL_ERR_PASS)
    {
       return status;
    }
    if ((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo,
        &flushRAMTablePageInfo, &currentLogicalEBNum)) != FTL_ERR_PASS)
    {
       return status;
    }
    if (latestLogicalEBNum != currentLogicalEBNum)
    {
       SetGCOrFreePageNum(devID, currentLogicalEBNum, MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK);
    }
    #else
    sysEBlockFlushInfo.logIncNum = EMPTY_DWORD;
    dirtyBitTotalCnt = GetTotalDirtyBitCnt(devID); // needed to findout the end_point
    if(dirtyBitTotalCnt == 0)
    {
       return FTL_ERR_PASS;
    }
    for(flushTypeCnt = 0; flushTypeCnt < numRamTables; flushTypeCnt++)
    {
       dataByteCnt = 0x0;
       if(flushTypeCnt == 1)
       {
          ramMappingTablePtr = (UINT8_PTR) (&EBlockMappingTable[devID][0]);
          dirtyBitMapPtr     = (UINT8_PTR)(&EBlockMappingTableDirtyBitMap[devID][0]);
          totalRamTableBytes = sizeof(EBlockMappingTable[devID]);
          maxTableOffset     = (BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE - 1);
          sysEBlockFlushInfo.type       = EBLOCK_MAP_TABLE_FLUSH;
       }
       else if(flushTypeCnt == 0)
       {
          ramMappingTablePtr = (UINT8_PTR)(&PPAMappingTable[devID]);
          dirtyBitMapPtr     = (UINT8_PTR)(&PPAMappingTableDirtyBitMap[devID][0]);
          totalRamTableBytes = sizeof(PPAMappingTable[devID]);
          maxTableOffset     = (BITS_PPA_DIRTY_BITMAP_DEV_TABLE - 1);
          sysEBlockFlushInfo.type       = PPA_MAP_TABLE_FLUSH;
       }
       for(dataByteCnt = 0, bitMapCounter = 0; dataByteCnt < totalRamTableBytes;
          dataByteCnt += FLUSH_RAM_TABLE_SIZE, bitMapCounter++)
       {
          // If we already wrote all the dirty entries, just return.
          if(dirtyBitTotalCnt == 0)
          {
             return FTL_ERR_PASS;
          }
          bitMapData = GetBitMapField(dirtyBitMapPtr, bitMapCounter, 1);
          if(bitMapData == DIRTY_BIT)
          {
             dirtyBitCounter++;
             if((status = GetNextFlushEntryLocation(devID, &flushStructPageInfo, 
                &flushRAMTablePageInfo, &logicalBlockNum)) != FTL_ERR_PASS)
             {
                return status; // Do GC.
             }

             #if (DEBUG_FTL_API_ANNOUNCE == 1)
             DBG_Printf("TABLE_FlushDevice: type=%d, ", sysEBlockFlushInfo.type, 0);
             DBG_Printf("index=%d \n", bitMapCounter, 0);
             #endif  

             // Inc free page
             IncGCOrFreePageNum(devID, logicalBlockNum);
             // Mark dirty for the flush entry
             MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);                

             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, logicalBlockNum, FTL_FALSE);
             #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

             // clean the dirty bit...
             SetBitMapField(dirtyBitMapPtr, bitMapCounter, 1, CLEAN_BIT);
             //dirtyBitTotalCnt = GetTotalDirtyBitCnt(devID); // can be implemented more efficiently...
             dirtyBitTotalCnt--;
             // type, TableOffset, endPoint, erase started, reserve  @Time T1
             // sysEBlockFlushInfo.type & sysEBlockFlushInfo.tableOffset already set above
             // Mark end point if its last entry
             if((dirtyBitTotalCnt == 0) && (flushTypeCnt == 1))
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
             if(flushTypeCnt == 1)
             {
                // flush adjusted free page
                if((status = AdjustFlushEBlockFreePage(devID, logicalBlockNum, bitMapCounter)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
             // Flush EBlockMappingTable data - 512 bits/ 200 bytes
             sysEBlockFlushInfo.tableOffset = (UINT16)(dataByteCnt/FLUSH_RAM_TABLE_SIZE);
             tempSize = totalRamTableBytes - dataByteCnt;

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             if((sysEBlockFlushInfo.tableOffset == maxTableOffset) && (tempSize < SECTOR_SIZE))
             {
                srcPtr = (UINT8_PTR)(ramMappingTablePtr + (sysEBlockFlushInfo.tableOffset * SECTOR_SIZE));
                MEM_Memcpy(&pseudoRPB[devID][0], srcPtr, (UINT16)tempSize);
                for(tempCount = (UINT16)tempSize; tempCount < SECTOR_SIZE; tempCount++)
                {
                   pseudoRPB[devID][tempCount] = EMPTY_BYTE;
                }
                if(FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, &pseudoRPB[devID][0]) != FLASH_PASS)
                {
                   return FTL_ERR_FLASH_WRITE_03;
                }
             }
             else
             {
                if(FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, (UINT8_PTR)(ramMappingTablePtr 
                   + (sysEBlockFlushInfo.tableOffset * SECTOR_SIZE))) != FLASH_PASS)
                {
                   return FTL_ERR_FLASH_WRITE_05;
                }
             }

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             MEM_Memcpy(&pseudoRPB[devID][0], (UINT8_PTR)&sysEBlockFlushInfo, FLUSH_INFO_SIZE);
             srcPtr = (UINT8_PTR)(ramMappingTablePtr + (sysEBlockFlushInfo.tableOffset * FLUSH_RAM_TABLE_SIZE));
             if((sysEBlockFlushInfo.tableOffset == maxTableOffset) && (tempSize < FLUSH_RAM_TABLE_SIZE))
             {
                MEM_Memcpy(&pseudoRPB[devID][FLUSH_INFO_SIZE], srcPtr, (UINT16)tempSize);
                for(tempCount = (UINT16)(tempSize + FLUSH_INFO_SIZE); tempCount < FLUSH_RAM_TABLE_SIZE; tempCount++)
                {
                   pseudoRPB[devID][tempCount] = EMPTY_BYTE;
                }
             }
             else
             {
                MEM_Memcpy(&pseudoRPB[devID][FLUSH_INFO_SIZE], srcPtr, FLUSH_RAM_TABLE_SIZE);
             }
             flushInfoEntry = (UINT16_PTR)&pseudoRPB[devID][0];
             flushInfoEntry[FLUSH_INFO_TABLE_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_TABLE_START], (FLUSH_RAM_TABLE_SIZE/2));
             flushInfoEntry[FLUSH_INFO_CHECK_WORD] = CalcCheckWord(&flushInfoEntry[FLUSH_INFO_DATA_START], FLUSH_INFO_DATA_WORDS);
             flashStatus = FLASH_RamPageWriteDataBlock(&flushRAMTablePageInfo, &pseudoRPB[devID][0]);
             if(flashStatus != FLASH_PASS)
             {

                #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                if(flashStatus == FLASH_PARAM)
                {
                   return FTL_ERR_FLASH_WRITE_10;
                }
                SetBadEBlockStatus(devID, logicalBlockNum, FTL_TRUE);
                if(FLASH_MarkDefectEBlock(&flushRAMTablePageInfo) != FLASH_PASS)
                {
                   // do nothing, just try to mark bad, even if it fails we move on.
                }
                if(flushTypeCnt == 1)
                {
                   // restore free page if adjusted
                   if((status = RestoreFlushEBlockFreePage(devID, logicalBlockNum)) != FTL_ERR_PASS) 
                   {
                      return status;
                   }
                }
                return FTL_ERR_FLUSH_FLUSH_FAIL;

                #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                return FTL_ERR_FLASH_WRITE_10;
                #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

             }
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flushTypeCnt == 1)
             {
                // restore free page if adjusted
                if((status = RestoreFlushEBlockFreePage(devID, logicalBlockNum)) != FTL_ERR_PASS) 
                {
                   return status;
                }
             }

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             // Write END SIGNATURE, @Time T2
             if((status = FTL_WriteFlushInfo(&flushStructPageInfo, &sysEBlockFlushInfo)) != FTL_ERR_PASS)
             {
                return status;
             }
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

          }
       }
    }

    #endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

    return FTL_ERR_PASS;
}

//---------------------------------------
FTL_STATUS UpdateEBOrderingTable(FTL_DEV devID, UINT16 startEB, UINT16_PTR formatCount)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    UINT16 eBlockCount = 0;                            /*2*/
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0} };       /*11*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    UINT32 key = EMPTY_DWORD;                          /*4*/
    UINT16 countEB = EMPTY_WORD;                       /*2*/
    UINT16 phyEBlockAddrOld = EMPTY_WORD;              /*2*/
    SYS_EBLOCK_INFO_PTR sysTempPtr = NULL;             /*4*/
    UINT32 prevKey = EMPTY_DWORD;                      /*4*/
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 phyEBlockAddrTmp = EMPTY_WORD;              /*2*/
    UINT16 logicalEBNumTmp = EMPTY_WORD;               /*2*/
    UINT8 swap = FTL_FALSE;                            /*1*/
    UINT16 logEBlockAddrOld = EMPTY_WORD;              /*2*/
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 saveLogicalAddr = EMPTY_WORD;              /*2*/
    #endif
    UINT16 tempFormatCount = 0;                       /*2*/

    flashPage.devID = devID;
    for (eBlockCount = startEB; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
    {
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       if (FTL_START_EBLOCK == startEB)
       {
          if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, eBlockCount, CACHE_INIT_TYPE)))
          {
             return status;
          }
       }
       #endif

       phyEBlockAddr = GetPhysicalEBlockAddr(devID, eBlockCount);
       flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
       flashPage.vPage.pageOffset = 0;

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       flashPage.byteCount = sizeof(sysEBlockInfo);
       flash_status = FLASH_RamPageReadMetaData(&flashPage, (UINT8_PTR)(&sysEBlockInfo));

       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       flashPage.byteCount = SECTOR_SIZE;
       flash_status = FLASH_RamPageReadDataBlock(&flashPage, &pseudoRPB[devID][0]);
       MEM_Memcpy((UINT8_PTR)(&sysEBlockInfo), &pseudoRPB[devID][0], SYS_INFO_SIZE);
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       if(flash_status != FLASH_PASS)
       {

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if(flash_status == FLASH_ECC_FAIL)
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, eBlockCount, FTL_FALSE);
             #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             
             if(SYSTEM_START_EBLOCK > eBlockCount)
             {
                continue;
             }

             (*formatCount)++;

             #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             if(FLASH_CheckDefectEBlock(&flashPage) != FLASH_PASS)
             {
                continue;
             }
             #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

             #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
             if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
             {
                continue;
             }
             #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

             if (FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,eBlockCount))
             {
                continue;
             }

             if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
             {
                return status;
             }
             continue;
          }
          #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

          return FTL_ERR_FLASH_READ_08;
       }
       if((FTL_FALSE == VerifyCheckWord((UINT16_PTR)&sysEBlockInfo.type, SYS_INFO_DATA_WORDS, sysEBlockInfo.checkWord)) &&
              (sysEBlockInfo.oldSysBlock != OLD_SYS_BLOCK_SIGNATURE) && (sysEBlockInfo.phyAddrThisEBlock == phyEBlockAddr))
       {
          if(sysEBlockInfo.type == SYS_EBLOCK_INFO_FLUSH)
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, eBlockCount, FTL_FALSE);
             #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

             // Insert the entry, if fails check if this has higher incNum and insert that instead
             if((status = TABLE_FlushEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) == FTL_ERR_FLUSH_NO_ENTRIES)
             {
                swap = FTL_FALSE;
                for(countEB = 0; countEB < NUM_FLUSH_LOG_EBLOCKS; countEB++)
                {
                   if((status = TABLE_GetFlushLogEntry(devID, countEB, &logicalEBNumTmp, &phyEBlockAddrTmp, &key)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   if(key < sysEBlockInfo.incNumber)
                   {
                      swap = FTL_TRUE;
                   }
                }
                if(swap == FTL_TRUE)
                {
                   // This will clear the smallest entry
                   if((status = TABLE_FlushEBGetNext(devID, &logEBlockAddrOld, &phyEBlockAddrOld, NULL)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
                   if((status = TABLE_InsertReservedEB(devID, logEBlockAddrOld)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #else
                   saveLogicalAddr = logEBlockAddrOld;
                   #endif
                   // Space is now avaiable, Insert the higher entry
                   if((status = TABLE_FlushEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddrOld, 0);
                }
                else
                {
                   #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
                   if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #else
                   saveLogicalAddr = eBlockCount;
                   #endif
                   flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
                }
                // Mark old flush log eblock
                sysEBlockInfo.oldSysBlock = OLD_SYS_BLOCK_SIGNATURE;
                flashPage.devID = devID;
                flashPage.vPage.pageOffset = (UINT16)((UINT32)(&sysTempPtr->oldSysBlock));
                flashPage.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
//              flashPage.byteCount = sizeof(sysEBlockInfo.oldSysBlock);
                flash_status = FLASH_RamPageWriteMetaData(&flashPage, (UINT8_PTR)&sysEBlockInfo.oldSysBlock);
                if(flash_status != FLASH_PASS)
                {

                   #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   if(flash_status == FLASH_PARAM)
                   {
                      return FTL_ERR_FLASH_WRITE_07;
                   }
                   SetBadEBlockStatus(devID, saveLogicalAddr, FTL_TRUE);
                   // just try to mark bad, even if it fails we move on.
                   FLASH_MarkDefectEBlock(&flashPage);

                   #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   return FTL_ERR_FLASH_WRITE_07;
                   #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

                }
                #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                if((status = TABLE_InsertReservedEB(devID, saveLogicalAddr)) != FTL_ERR_PASS)
                {
                   return status;
                }
                #endif
             }
             else if(status != FTL_ERR_PASS)
             {
                return status;
             }
             if(sysEBlockInfo.incNumber > GetFlushEBCounter(devID))
             {
                SetFlushLogEBCounter(devID, sysEBlockInfo.incNumber);
             }
             if(SYSTEM_START_EBLOCK > eBlockCount)
             {
                tempFormatCount++;
             }
             continue;
          }
          if(sysEBlockInfo.type == SYS_EBLOCK_INFO_LOG)
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, eBlockCount, FTL_FALSE);
             #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

             if((status = TABLE_TransLogEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) == FTL_ERR_LOG_INSERT)
             {
                swap = FTL_FALSE;
                for(countEB = 0; countEB < NUM_TRANSACTION_LOG_EBLOCKS; countEB++)
                {
                   if((status = TABLE_GetTransLogEntry(devID, countEB, &logicalEBNumTmp, &phyEBlockAddrTmp, &key)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   if(key < sysEBlockInfo.incNumber)
                   {
                      swap = FTL_TRUE;
                   }
                }
                if(swap == FTL_TRUE)
                {
                   // This will clear the smallest entry
                   if((status = TABLE_TransLogEBGetNext(devID, &logEBlockAddrOld, &phyEBlockAddrOld, NULL)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
                   if((status = TABLE_InsertReservedEB(devID, logEBlockAddrOld)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #else
                   saveLogicalAddr = logEBlockAddrOld;
                   #endif
                   // Space is now avaiable, Insert the higher entry
                   if((status = TABLE_TransLogEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddrOld, 0);
                }
                else
                {

                   #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
                   if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #else
                   saveLogicalAddr = eBlockCount;
                   #endif
                   flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
                }
                // Mark old flush log eblock
                sysEBlockInfo.oldSysBlock = OLD_SYS_BLOCK_SIGNATURE;
                flashPage.devID = devID;
                flashPage.vPage.pageOffset = (UINT16)((UINT32)(&sysTempPtr->oldSysBlock));
                flashPage.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
//              flashPage.byteCount = sizeof(sysEBlockInfo.oldSysBlock);
                flash_status = FLASH_RamPageWriteMetaData(&flashPage, (UINT8_PTR)&sysEBlockInfo.oldSysBlock);
                if(flash_status != FLASH_PASS)
                {

                   #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   if(flash_status == FLASH_PARAM)
                   {
                      return FTL_ERR_FLASH_WRITE_06;
                   }
                   SetBadEBlockStatus(devID, saveLogicalAddr, FTL_TRUE);
                   // just try to mark bad, even if it fails we move on.
                   FLASH_MarkDefectEBlock(&flashPage);

                   #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   return FTL_ERR_FLASH_WRITE_06;
                   #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

                }
                #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                if((status = TABLE_InsertReservedEB(devID, saveLogicalAddr)) != FTL_ERR_PASS)
                {
                   return status;
                }
                #endif
                
             }
             else if(status != FTL_ERR_PASS)
             {
                return status;
             }
             if(sysEBlockInfo.incNumber > GetTransLogEBCounter(devID))
             {
                SetTransLogEBCounter(devID, sysEBlockInfo.incNumber);
             }
             if(SYSTEM_START_EBLOCK > eBlockCount)
             {
                tempFormatCount++;
             }
             continue;
          }
          if(SYSTEM_START_EBLOCK > eBlockCount)
          {
             continue;
          }
          (*formatCount)++;

          #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
          if(FTL_ERR_PASS == TABLE_CheckUsedSuperEB(devID, eBlockCount))
          {
             continue;
          }
          #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

          if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       else
       {

          if(SYSTEM_START_EBLOCK > eBlockCount)
          {
             continue;
          }

          (*formatCount)++;

          #if (ENABLE_EB_ERASED_BIT == FTL_TRUE) // For Filed Update, correct format compatibility between current version(1.2.11) and new version(1.2.12).
          if((sysEBlockInfo.oldSysBlock == OLD_SYS_BLOCK_SIGNATURE) && (FTL_TRUE == GetEBErased(devID,eBlockCount)) && (SYSTEM_START_EBLOCK == startEB))
          {
             DBG_Printf("[warning] Inconsistency between oldSysBlock and eraseStatus is detected. Set EraseStatus as FALSE to erase the EB anyway. \n", 0, 0);
             SetEBErased(devID, eBlockCount, FTL_FALSE);
          }
          #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(FLASH_CheckDefectEBlock(&flashPage) != FLASH_PASS)
          {
             continue;
          }
          #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

          #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
          if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
          {
             continue;
          }
          #endif

          #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
          if(FTL_ERR_PASS == TABLE_CheckUsedSuperEB(devID, eBlockCount))
          {
             continue;
          }
          #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

          if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
    }
    /* check sanity */
    for(eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetFlushLogEntry(devID, eBlockCount, &logicalEBNum, &phyEBlockAddr, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key == EMPTY_DWORD)
       {
          break;
       }
       if(eBlockCount > 0)
       {
          if(key > (prevKey + 1))
          {
             if((status = TABLE_FlushEBRemove(devID, eBlockCount)) != FTL_ERR_PASS)
             {
                return status;
             }
             SetFlushLogEBCounter(devID, prevKey);
          }
       }
       prevKey = key;
    }
    (*formatCount) = (*formatCount) - tempFormatCount;

    return FTL_ERR_PASS;
}

//-----------------------------------
FTL_STATUS TABLE_Flush(UINT8 flushMode)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT8 devID = 0;                                   /*1*/
    UINT8 flushGCFlag = FTL_FALSE;                     /*1*/

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT8 flushFail = FTL_FALSE;                       /*1*/
    UINT16 sanityCounter = 0;                          /*2*/
    UINT8 checkBBMark = FTL_FALSE;                   /*1*/
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    for(devID = 0; devID < NUM_DEVICES; devID++)
    {
       if(((status = CheckFlushSpace(devID)) == FTL_ERR_FLUSH_GC_NEEDED) || (flushMode == FLUSH_GC_MODE))
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
          if((status = Flush_GC(devID)) != FTL_ERR_PASS)
          {
             return status;
          }
          #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)

          flushGCFlag = FTL_TRUE;
       }
       else if(status != FTL_ERR_PASS)
       {             
          return status;
       }
       // This does not check for avaiable free space...
       if(flushGCFlag == FTL_FALSE) //already done a full flush, dont flush again.
       {
          if((status = TABLE_FlushDevice(devID, flushMode)) != FTL_ERR_PASS)
          {

             #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             if((status == FTL_ERR_FLUSH_FLUSH_FAIL) || (status == FTL_ERR_FLUSH_NEXT_ENTRY))
             {
                flushFail = FTL_TRUE;
             }
             else
             #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

             {
                return status;
             }
          }
          if(flushMode == FLUSH_NORMAL_MODE)
          {
             // Erase the log entries, only if its not empty...
             #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             if(flushFail == FTL_FALSE)
             {
                while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
                {
                   status = FTL_EraseAllTransLogBlocksOp(devID);
                   if(status == FTL_ERR_MARKBB_COMMIT)
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
                   flushFail = FTL_TRUE;
                }
             }

             #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             if((status = FTL_EraseAllTransLogBlocksOp(devID)) != FTL_ERR_PASS)
             {
                return status;
             }
             #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

          }
       }

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if((flushGCFlag == FTL_TRUE) || (flushFail == FTL_TRUE))
       {
          sanityCounter = 0;
          while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
          {
             if((status = Flush_GC(devID)) != FTL_ERR_FLUSH_FLUSH_GC_FAIL)
             {
                break;
             }
             sanityCounter++;
          }
          if(status != FTL_ERR_PASS)
          {
             return status;
          }
       }
       #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    writeLogFlag = FTL_FALSE;
    #endif

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    FTL_ClearGCSave(CLEAR_GC_SAVE_RUNTIME_MODE);
    #endif // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    return FTL_ERR_PASS;
}

//--------------------------------------
FTL_STATUS TABLE_LoadFlushTable()
{
    FTL_STATUS status = FTL_ERR_PASS;                       /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;                 /*4*/
    UINT8 eBlockCnt      = 0x0;                             /*1*/
    UINT8 devID          = 0x0;                             /*1*/
    UINT8 ramTablesUpdated  = FTL_FALSE;                    /*1*/
    UINT8 logEblockFound = FTL_FALSE;                       /*1*/
    UINT16 logicalAddr   = 0x0;                             /*2*/
    UINT16 phyAddr       = 0x0;                             /*2*/
    UINT32 key           = 0x0;                             /*4*/
    UINT16 freePageIndex = 0x0;                             /*2*/
    UINT16 formatCount   = 0x0;                             /*2*/
    UINT16 validFreePageIndex = 1;                          /*2*/
    UINT32 transLogEBCounter = 0;                           /*4*/
    UINT32 logIndexFound = EMPTY_DWORD;                     /*4*/
    SYS_EBLOCK_INFO sysEBlockInfo;                          /*16*/
    SYS_EBLOCK_FLUSH_INFO sysEBlockFlushInfo;               /*16*/
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0}};             /*11*/
    FLASH_PAGE_INFO flushStructPageInfo = {0, 0, {0, 0}};   /*11*/
    FLASH_PAGE_INFO flushRAMTablePageInfo = {0, 0, {0, 0}}; /*11*/
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT8_PTR EBlockMappingTablePtr = NULL;                 /*4*/
    UINT8_PTR PPAMappingTablePtr    = NULL;                 /*4*/
    #endif
    LOG_ENTRY_LOC shutdownLoc = {EMPTY_WORD, EMPTY_WORD};   /*4*/
    UINT16 eBlockCount = 0x0;                                /*2*/

    KEY_TABLE_ENTRY tempArray[NUM_DEVICES][NUM_TRANSACTION_LOG_EBLOCKS];
    UINT16 tempCount = 0x0;                                  /*1*/
    UINT16 tempBlockCount = 0x0;                            /*2*/
    UINT8 tempFound = FTL_FALSE;                            /*1*/
    UINT32 tempEBCounter = 0x0;                             /*4*/

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    SYS_EBLOCK_INFO_PTR tempSysPtr = NULL;                  /*4*/
    UINT16 phyEBAddr      = 0x0;                            /*2*/
    UINT16 eBlockNum      = 0x0;                            /*2*/
    UINT32 latestIncNumber= 0x0;                            /*4*/
    UINT8  checkFlag      = FTL_FALSE;                      /*1*/
    KEY_TABLE_ENTRY tempSuperArray[NUM_DEVICES][NUM_SUPER_SYS_EBLOCKS];
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT8 flushEblockFailed = FTL_FALSE;                    /*1*/
    UINT16 checkLoopPhyEB = EMPTY_WORD;                     /*1*/
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;                               /*2*/
    UINT8 checkBBMark = FTL_FALSE;                        /*1*/
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT16 eccErrSector = EMPTY_WORD;
    UINT16 flashReadCount = 0x0;
    #endif

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT16 bitMapCounter = 0;
    #endif
    #endif

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    KEY_TABLE_ENTRY tempFlushArray[NUM_DEVICES][NUM_FLUSH_LOG_EBLOCKS];
    CACHE_INFO_EBLOCK_PPAMAP eBlockPPAMapInfo = { 0, 0 };
    UINT16 logicalEBNum = 0x0;
    #if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
    UINT16 index[NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES];
    #else
    UINT16_PTR index;
    #endif
    UINT16 indexCount = 0;
    UINT16 indexMaxCount = 0;
    UINT8 eBlockCnt2 = 0;
    UINT8 pfFlushFlag = FTL_FALSE;
    #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
    #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
    UINT16 ebCount = 0;
    UINT32 sector = 0;
    UINT32 sector2 = 0;
    UINT16 offset = 0;
    #endif
    UINT32 maxValue = 0;
    UINT32 eraseCount = 0;
    #endif
    #endif
   

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
    index = (UINT16_PTR)MEM_Malloc(sizeof(UINT16) * (NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES));
    if (NULL == index)
    {
        DBG_Printf("index array Malloc Error\n", 0, 0);
        return FTL_ERR_FAIL;
    }
    #endif
    #endif

    for(devID = 0; devID < NUM_DEVICES; devID++)
    {

       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       checkFlag = FTL_FALSE;
       #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       checkLoopPhyEB = EMPTY_WORD;
       do {
          if (EMPTY_WORD != checkLoopPhyEB) // take back a EBlock/PPAMappingTable.
          {
             #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
             for (eBlockCount = 0; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
             {
                TABLE_ClearMappingTable(devID, eBlockCount, eBlockCount, ERASE_STATUS_GET_DWORD_MASK | 1);
                TABLE_ClearPPATable(devID, eBlockCount);
             }
             for (bitMapCounter = 0; bitMapCounter < EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE; bitMapCounter++)
             {
                EBlockMappingTableDirtyBitMap[devID][bitMapCounter] = CLEAN_BIT;
             }
             #else
             // don't need this. clear array after routine
             #endif
             flushEblockFailed = FTL_FALSE;
          }
       #endif

       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

       for(eBlockCnt = 0; eBlockCnt < NUM_FLUSH_LOG_EBLOCKS; eBlockCnt++)
       {
          tempFlushArray[devID][eBlockCnt].logicalEBNum = EMPTY_WORD;
          tempFlushArray[devID][eBlockCnt].phyAddr = EMPTY_WORD;
          tempFlushArray[devID][eBlockCnt].key = EMPTY_DWORD;
          tempFlushArray[devID][eBlockCnt].cacheNum = EMPTY_BYTE;
       }

       indexCount = 0;
       if((status = CACHE_ClearAll()) != FTL_ERR_PASS) // clear cache table
       {
          return status;
       }

       #else
       EBlockMappingTablePtr = (UINT8_PTR) (&EBlockMappingTable[devID][0]);
       PPAMappingTablePtr    = (UINT8_PTR)(&PPAMappingTable[devID][0][0]);
       #endif

       for(eBlockCnt = 0; eBlockCnt < NUM_FLUSH_LOG_EBLOCKS; eBlockCnt++)
       {
          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          if((status = TABLE_FlushEBGetNext(devID, &logicalAddr, &phyAddr, &key)) == FTL_ERR_FLUSH_NO_EBLOCKS)
          {
             break;
          }
          #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if ((status = TABLE_GetFlushLogEntry(devID, eBlockCnt, &logicalAddr, &phyAddr, &key)) != FTL_ERR_PASS)
          {
             return status;
          }
          if (EMPTY_WORD == logicalAddr && EMPTY_WORD == phyAddr && EMPTY_DWORD == key)
          {
             status = FTL_ERR_FLUSH_NO_EBLOCKS;
             break;
          }
          #endif

          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          tempFlushArray[devID][eBlockCnt].logicalEBNum = logicalAddr;
          tempFlushArray[devID][eBlockCnt].phyAddr = phyAddr;
          tempFlushArray[devID][eBlockCnt].key = key;
          #endif

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          for (freePageIndex = 1, validFreePageIndex = 0; freePageIndex < MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK; freePageIndex++)
          #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          for (freePageIndex = 0, validFreePageIndex = 0; freePageIndex < MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK; freePageIndex++)
          #endif
          {
             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             if ((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) == 0)
             {
                eccErrSector = EMPTY_WORD;
             #endif
             if((status = GetFlushLoc(devID, phyAddr, freePageIndex, &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
             {
                return status;
             }

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             flash_status = FLASH_RamPageReadMetaData(&flushStructPageInfo, (UINT8_PTR)&sysEBlockFlushInfo);

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             flushRAMTablePageInfo.byteCount = VIRTUAL_PAGE_SIZE; // Set 2048 byte size
             flash_status = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, &pseudoRPB[devID][0]);
             MEM_Memcpy((UINT8_PTR)&sysEBlockFlushInfo, &pseudoRPB[devID][0], FLUSH_INFO_SIZE);
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flash_status != FLASH_PASS)
             {

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                // power failure check...
                if(flash_status == FLASH_ECC_FAIL)
                {
                   // power failure occurred, invoke Flush_GC
                   flushEblockFailed = FTL_TRUE;

                   for(flashReadCount = 0; flashReadCount < NUMBER_OF_SECTORS_PER_PAGE; flashReadCount++)
                   {
                      if((status = GetFlushLoc(devID, phyAddr, (freePageIndex + flashReadCount), &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      
                      flash_status = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, &pseudoRPB[devID][(flashReadCount * SECTOR_SIZE)]);
                      if(flash_status != FLASH_PASS)
                      {
                         if(flash_status == FLASH_ECC_FAIL)
                         {
                            if(EMPTY_WORD == eccErrSector)
                            {
                               eccErrSector = flashReadCount;
                            }
                            //DBG_Printf("ECC error Sector = %d\n", eccErrSector, 0);
                         }else{
                            return FTL_ERR_FLASH_READ_07;
                         }
                      }
                   }
                }
                else
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

                {
                   return FTL_ERR_FLASH_READ_07;
                }
             }
             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             }else{
                MEM_Memcpy((UINT8_PTR)&pseudoRPB[devID][0], (UINT8_PTR)&pseudoRPB[devID][((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) * SECTOR_SIZE)], SECTOR_SIZE);
                MEM_Memcpy((UINT8_PTR)&sysEBlockFlushInfo, &pseudoRPB[devID][0], FLUSH_INFO_SIZE);
             } // if((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) == 0)

             if(EMPTY_WORD != eccErrSector && ((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) == eccErrSector))
             {
                // ECC error
                DBG_Printf("ECC error break Sector = %d\n", freePageIndex % NUMBER_OF_SECTORS_PER_PAGE, 0); // debug                
                break;
             }
             // Skip Sys info
             if (0 == freePageIndex)
             {
                continue;
             }
             #endif
             // power failure check...
             if (FTL_TRUE == VerifyCheckWord((UINT16_PTR) &sysEBlockFlushInfo.type, 
                FLUSH_INFO_DATA_WORDS, sysEBlockFlushInfo.checkWord))
             {
                // No more entries..
                break;
             }

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             if((status = VerifyRamTable((UINT16_PTR)&pseudoRPB[devID][0])) != FTL_ERR_PASS)
             {
                // power failure occurred, invoke Flush_GC
                flushEblockFailed = FTL_TRUE;
                break;
             }


             if(phyAddr != checkLoopPhyEB)
             {
                if(sysEBlockFlushInfo.type == EBLOCK_MAP_TABLE_FLUSH)
                {

                   #if (DEBUG_FTL_API_ANNOUNCE == 1)
                   DBG_Printf("TABLE_LoadFlushTable EBM: offset=%d \n", sysEBlockFlushInfo.tableOffset, 0);
                   #endif  

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   eBlockPPAMapInfo.entryIndex = freePageIndex;
                   eBlockPPAMapInfo.flashLogEBArrayCount = eBlockCnt;
                   index[indexCount + (devID * NUMBER_OF_DEVICES)] = sysEBlockFlushInfo.tableOffset;
                   indexCount++;
                   if(indexCount >= NUM_EBLOCK_MAP_INDEX)
                   {
                      indexMaxCount = indexCount;
                      indexCount = 0;
                   }
                   if(FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
                   {
                      return status;
                   }

                   #else
                   if((status = LoadRamTable(&flushRAMTablePageInfo, EBlockMappingTablePtr, 
                      sysEBlockFlushInfo.tableOffset, sizeof(EBlockMappingTable[devID]))) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #endif
                }
                else if(sysEBlockFlushInfo.type == PPA_MAP_TABLE_FLUSH)
                {

                   #if (DEBUG_FTL_API_ANNOUNCE == 1)
                   DBG_Printf("TABLE_LoadFlushTable PPA: offset=%d \n", sysEBlockFlushInfo.tableOffset, 0);
                   #endif  

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   eBlockPPAMapInfo.entryIndex = freePageIndex;
                   eBlockPPAMapInfo.flashLogEBArrayCount = eBlockCnt;
                   if(FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_PPAMAP)))
                   {
                      return status;
                   }
                   #else
                   if((status = LoadRamTable(&flushRAMTablePageInfo, PPAMappingTablePtr, 
                      sysEBlockFlushInfo.tableOffset, sizeof(PPAMappingTable[devID]))) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   #endif
                }
             }
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

             if(sysEBlockFlushInfo.endPoint == END_POINT_SIGNATURE)
             {
                validFreePageIndex = freePageIndex;
                logIndexFound = sysEBlockFlushInfo.logIncNum;
                shutdownLoc.eBlockNum = sysEBlockFlushInfo.eBlockNumLoc;
                shutdownLoc.entryIndex = sysEBlockFlushInfo.entryIndexLoc;
             }
          }            
          if((validFreePageIndex == 0) && (freePageIndex == MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK))
          {
             flashPage.devID = devID;
             flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyAddr, 0);
             flashPage.vPage.pageOffset = 0;


             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             flashPage.byteCount = sizeof(sysEBlockInfo);
             flash_status = FLASH_RamPageReadMetaData(&flashPage, (UINT8_PTR)(&sysEBlockInfo));

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             flashPage.byteCount = SECTOR_SIZE;
             flash_status = FLASH_RamPageReadDataBlock(&flashPage, &pseudoRPB[devID][0]);
             MEM_Memcpy((UINT8_PTR)&sysEBlockInfo, &pseudoRPB[devID][0], SYS_INFO_SIZE);
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flash_status != FLASH_PASS)
             {
                return FTL_ERR_FLUSH_READ;
             }
             if(sysEBlockInfo.fullFlushSig == FULL_FLUSH_SIGNATURE)
             {
                validFreePageIndex = freePageIndex;
                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                // 2nd loop skips
                continue;
                #endif
             }
          }
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          if((freePageIndex - 1) != validFreePageIndex)
          {
             pfFlushFlag = FTL_TRUE;
          }
          #endif
          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if((freePageIndex - 1) == validFreePageIndex)
          {
             // 2nd loop skips
             continue;
          }
          if (((freePageIndex - 1) != validFreePageIndex) && (EMPTY_WORD == checkLoopPhyEB))
          {
             // go to 2nd loop
             checkLoopPhyEB = phyAddr;
             //DBG_Printf("Goto 2nd loop: 0x%x\n", phyAddr, 0);
             break;
          }
          #endif

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          for(freePageIndex = 1; freePageIndex <= validFreePageIndex; freePageIndex++)
          #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          for(freePageIndex = 0; freePageIndex <= validFreePageIndex; freePageIndex++)
          #endif
          {
             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             if ((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) == 0)
             {
             #endif
             if((status = GetFlushLoc(devID, phyAddr, freePageIndex, &flushStructPageInfo, &flushRAMTablePageInfo)) != FTL_ERR_PASS)
             {
                return status;
             }

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             flash_status = FLASH_RamPageReadMetaData(&flushStructPageInfo, (UINT8_PTR)&sysEBlockFlushInfo);

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             flushRAMTablePageInfo.byteCount = VIRTUAL_PAGE_SIZE; // Set 2048 byte size
             flash_status = FLASH_RamPageReadDataBlock(&flushRAMTablePageInfo, &pseudoRPB[devID][0]);
             MEM_Memcpy((UINT8_PTR)&sysEBlockFlushInfo, &pseudoRPB[devID][0], FLUSH_INFO_SIZE);
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flash_status != FLASH_PASS)
             {
                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                if(flash_status == FLASH_ECC_FAIL)
                {
                  // power failure occurred, invoke Flush_GC
                   flushEblockFailed = FTL_TRUE;

                }else{
                   return FTL_ERR_FLASH_READ_06;
                }
                #else
                return FTL_ERR_FLASH_READ_06;
                #endif
             }

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             }
             else{
                 MEM_Memcpy((UINT8_PTR)&pseudoRPB[devID][0], (UINT8_PTR)&pseudoRPB[devID][((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) * SECTOR_SIZE)], SECTOR_SIZE);
                 MEM_Memcpy((UINT8_PTR)&sysEBlockFlushInfo, &pseudoRPB[devID][0], FLUSH_INFO_SIZE);
             } // if((freePageIndex % NUMBER_OF_SECTORS_PER_PAGE) == 0)

             // Skip Sys info
             if (0 == freePageIndex)
             {
                continue;
             }
             #endif

             // power failure check...
             if (FTL_TRUE == VerifyCheckWord((UINT16_PTR) &sysEBlockFlushInfo.type, FLUSH_INFO_DATA_WORDS, sysEBlockFlushInfo.checkWord))
             {
                // No more entries..
                break;
             }

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             if((status = VerifyRamTable((UINT16_PTR)&pseudoRPB[devID][0])) != FTL_ERR_PASS)
             {
                // power failure occurred, invoke Flush_GC
                flushEblockFailed = FTL_TRUE;
                break;
             }
             #endif

             if(sysEBlockFlushInfo.type == EBLOCK_MAP_TABLE_FLUSH)
             {

                #if (DEBUG_FTL_API_ANNOUNCE == 1)
                DBG_Printf("TABLE_LoadFlushTable EBM: offset=%d \n", sysEBlockFlushInfo.tableOffset, 0);
                #endif  

                #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                eBlockPPAMapInfo.entryIndex = freePageIndex;
                eBlockPPAMapInfo.flashLogEBArrayCount = eBlockCnt;
                index[indexCount + (devID * NUMBER_OF_DEVICES)] = sysEBlockFlushInfo.tableOffset;
                indexCount++;
                if(indexCount >= NUM_EBLOCK_MAP_INDEX)
                {
                   indexMaxCount = indexCount;
                   indexCount = 0;
                }
                if(FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_EBLOCKMAP)))
                {
                   return status;
                }

                #else
                if((status = LoadRamTable(&flushRAMTablePageInfo, EBlockMappingTablePtr, 
                   sysEBlockFlushInfo.tableOffset, sizeof(EBlockMappingTable[devID]))) != FTL_ERR_PASS)
                {
                   return status;
                }
                #endif
             }
             else if(sysEBlockFlushInfo.type == PPA_MAP_TABLE_FLUSH)
             {

                #if (DEBUG_FTL_API_ANNOUNCE == 1)
                DBG_Printf("TABLE_LoadFlushTable PPA: offset=%d \n", sysEBlockFlushInfo.tableOffset, 0);
                #endif  

                #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                eBlockPPAMapInfo.entryIndex = freePageIndex;
                eBlockPPAMapInfo.flashLogEBArrayCount = eBlockCnt;
                if(FTL_ERR_PASS != (status = CACHE_SetEBlockAndPPAMap(devID, sysEBlockFlushInfo.tableOffset, &eBlockPPAMapInfo, CACHE_PPAMAP)))
                {
                   return status;
                }
                #else
                if((status = LoadRamTable(&flushRAMTablePageInfo, PPAMappingTablePtr, 
                   sysEBlockFlushInfo.tableOffset, sizeof(PPAMappingTable[devID]))) != FTL_ERR_PASS)
                {
                   return status;
                }
                #endif
             }
          }
       }
       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       }while ((eBlockCnt < NUM_FLUSH_LOG_EBLOCKS) && (FTL_ERR_FLUSH_NO_EBLOCKS != status));

       // Clear FlushLogEBArray
       if ((status = TABLE_FlushEBClear(devID)) != FTL_ERR_PASS)
       {
          return status;
       }
       #endif

       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

       // Set Flush Array
       if(0 == indexMaxCount)
       {
          indexMaxCount = indexCount;   
       }

       for (eBlockCnt = 0; eBlockCnt < NUM_FLUSH_LOG_EBLOCKS; eBlockCnt++)
       {
          logicalAddr = tempFlushArray[devID][eBlockCnt].logicalEBNum;
          phyAddr = tempFlushArray[devID][eBlockCnt].phyAddr;
          key = tempFlushArray[devID][eBlockCnt].key;
          if (EMPTY_WORD == logicalAddr)
          {
             continue;
          }
          if ((status = TABLE_FlushEBInsert(devID, logicalAddr, phyAddr, key)) != FTL_ERR_PASS)
          {
             return status;
          }
       }

       // Set Reserved and System EB
       for(logicalEBNum = NUM_DATA_EBLOCKS; logicalEBNum < NUMBER_OF_ERASE_BLOCKS; logicalEBNum++)
       {
          if(FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum, CACHE_INIT_TYPE)))
          {
             return status;
          }
       }

       TABLE_FlushEBClear(devID);
       #endif

       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if(((shutdownLoc.eBlockNum == EMPTY_WORD) && (shutdownLoc.entryIndex == EMPTY_WORD)) || (FTL_TRUE == SuperEBInfo[devID].checkLost)) // For Power Failure Case.
       #else  // #if(FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if((shutdownLoc.eBlockNum == EMPTY_WORD) && (shutdownLoc.entryIndex == EMPTY_WORD)) // For Power Failure Case.
       #endif
       {
          for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
          {
             tempArray[devID][eBlockCount].logicalEBNum = EMPTY_WORD;
             tempArray[devID][eBlockCount].phyAddr = EMPTY_WORD;
             tempArray[devID][eBlockCount].key = EMPTY_DWORD;
          }
          for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
          {   
             if((status = TABLE_GetTransLogEntry(devID, eBlockCount, &logicalAddr, &phyAddr, &key)) != FTL_ERR_PASS)
             {
                break; // trying to excess outside table.
             }
             if((logicalAddr == EMPTY_WORD) && (phyAddr == EMPTY_WORD) && (key == EMPTY_DWORD))
             {
                break; // no more entries in table
             }
             tempArray[devID][eBlockCount].logicalEBNum = logicalAddr;
             tempArray[devID][eBlockCount].phyAddr = phyAddr;
             tempArray[devID][eBlockCount].key = key;
          }
          tempCount = GetTransLogEBArrayCount(devID);
          tempEBCounter = GetTransLogEBCounter(devID);
       }
       
       // clear and update the flush Tables entries
       if((status = TABLE_InitEBOrderingTable(devID)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = UpdateEBOrderingTable(devID, SYSTEM_START_EBLOCK, (UINT16_PTR)&formatCount)) != FTL_ERR_PASS)
       {
          return status;
       }

       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       for (eBlockCnt = 0; eBlockCnt < NUM_FLUSH_LOG_EBLOCKS; eBlockCnt++)
       {
          if ((status = TABLE_FlushEBGetNext(devID, &logicalAddr, &phyAddr, &key)) == FTL_ERR_FLUSH_NO_EBLOCKS)
          {
             break;
          }
          for (eBlockCnt2 = 0; eBlockCnt2 < NUM_FLUSH_LOG_EBLOCKS; eBlockCnt2++)
          {
             if (phyAddr == tempFlushArray[devID][eBlockCnt2].phyAddr)
             {
                tempFlushArray[devID][eBlockCnt2].logicalEBNum = logicalAddr;
             }
          }
       }


       for (eBlockCnt = 0; eBlockCnt < NUM_FLUSH_LOG_EBLOCKS; eBlockCnt++)
       {
          logicalAddr = tempFlushArray[devID][eBlockCnt].logicalEBNum;
          phyAddr = tempFlushArray[devID][eBlockCnt].phyAddr;
          key = tempFlushArray[devID][eBlockCnt].key;
          if (EMPTY_WORD == logicalAddr)
          {
             continue;
          }
          if ((status = TABLE_FlushEBInsert(devID, logicalAddr, phyAddr, key)) != FTL_ERR_PASS)
          {
             return status;
          }
       }

       #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
       for (logicalEBNum = 0; logicalEBNum < NUMBER_OF_ERASE_BLOCKS; logicalEBNum++)
       {
          if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum, CACHE_INIT_TYPE)))
          {
             return status;
          }
          eraseCount = GetTrueEraseCount(devID, logicalEBNum);

          if (ERASE_STATUS_CLEAR_DWORD_MASK == eraseCount)
          {
             continue;
          }

          status = SetSaveStaticWL(devID, logicalEBNum, eraseCount);
          if (status != FTL_ERR_PASS)
          {
             return status;
          }

          if (GetGCNum(devID, logicalEBNum) > maxValue && logicalEBNum < NUM_DATA_EBLOCKS)
          {
             maxValue = GetGCNum(devID, logicalEBNum);
          }
       }
       // Set to greater than the largest value used so far
       GCNum[devID] = maxValue + 1;

       #endif // #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
       for (indexCount = 0; indexCount < indexMaxCount; indexCount++)
       {
          if ((((index[indexCount + (devID * NUM_EBLOCK_MAP_INDEX)] * FLUSH_RAM_TABLE_SIZE) % ((EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))) == 0))
          {
             logicalEBNum = (UINT16)((index[indexCount + (devID * NUM_EBLOCK_MAP_INDEX)] * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD));
          }
          else{
             logicalEBNum = (UINT16)((index[indexCount + (devID * NUM_EBLOCK_MAP_INDEX)] * FLUSH_RAM_TABLE_SIZE) / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1;
          }
          if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, logicalEBNum, CACHE_INIT_TYPE)))
          {
             return status;
          }
       }
       #endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
       {
          tempSuperArray[devID][eBlockNum].logicalEBNum = EMPTY_WORD;
          tempSuperArray[devID][eBlockNum].phyAddr = EMPTY_WORD;
          tempSuperArray[devID][eBlockNum].key = EMPTY_DWORD;
       }
       for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
       {
          if((status = TABLE_GetSuperSysEBEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
          {
             break; // trying to excess outside table.
          }
          if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
          {
             break; // no more entries in table
          }
          tempSuperArray[devID][eBlockNum].logicalEBNum = logicalAddr;
          tempSuperArray[devID][eBlockNum].phyAddr = phyEBAddr;
          tempSuperArray[devID][eBlockNum].key = latestIncNumber;
       }
       if((status = TABLE_SuperSysEBClear(devID)) != FTL_ERR_PASS)
       {
          return status;
       }
       for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
       {
          phyEBAddr = tempSuperArray[devID][eBlockNum].phyAddr;
          latestIncNumber = tempSuperArray[devID][eBlockNum].key;
          logicalAddr = GetLogicalEBlockAddr(devID,phyEBAddr);
          if((status = TABLE_SuperSysEBInsert(devID, logicalAddr, phyEBAddr, latestIncNumber)) != FTL_ERR_PASS)
          {
             if((status = TABLE_SuperSysEBClear(devID)) != FTL_ERR_PASS)
             {
                // skip
             }
             if((status = TABLE_SuperSysEBInsert(devID, EMPTY_WORD, EMPTY_WORD, EMPTY_DWORD)) != FTL_ERR_PASS)
             {
                //skip
             }
             SuperEBInfo[devID].checkSuperPF = FTL_TRUE;
          }
       }
       TABLE_ClearReservedEB(devID);
       for(eBlockCount = SYSTEM_START_EBLOCK; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
       {
          #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
          if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, eBlockCount, FTL_FALSE);
             #endif // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             continue;
          }
          #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

          if (FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,eBlockCount))
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, eBlockCount, FTL_FALSE);
             #endif // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             continue;
          }

          if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       #endif
       
       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if(((shutdownLoc.eBlockNum == EMPTY_WORD) && (shutdownLoc.entryIndex == EMPTY_WORD)) || (FTL_TRUE == SuperEBInfo[devID].checkLost)) // For Power Failure case.
       #else  // #if(FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if((shutdownLoc.eBlockNum == EMPTY_WORD) && (shutdownLoc.entryIndex == EMPTY_WORD)) // For Power Failure case.
       #endif
       {
          tempFound = FTL_FALSE;
          for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++) // sanity check
          {
             if((status = TABLE_GetTransLogEntry(devID, eBlockCount, &logicalAddr, &phyAddr, &key)) != FTL_ERR_PASS)
             {
                break; // trying to excess outside table.
             }
             if((logicalAddr == EMPTY_WORD) && (phyAddr == EMPTY_WORD) && (key == EMPTY_DWORD))
             {
                break; // no more entries in table
             }
             tempFound = FTL_FALSE;

             for(tempBlockCount = 0; tempBlockCount < tempCount; tempBlockCount++)
             {
                if(tempArray[devID][tempBlockCount].phyAddr == phyAddr)
                {
                   tempFound = FTL_TRUE;
                }
             }
             if(FTL_FALSE == tempFound)
             {
                break;
             }
          }

          if(tempCount != GetTransLogEBArrayCount(devID))  // Check an array count
          {
             tempFound = FTL_FALSE;
          }
          if(FTL_FALSE == tempFound) // Restructuring TransEB 
          {
             if((status = TABLE_TransEBClear(devID)) != FTL_ERR_PASS)
             {
                return status;
             }
             for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
             {
                phyAddr = tempArray[devID][eBlockCount].phyAddr;
                if(EMPTY_WORD == phyAddr)
                {
                   continue;
                }
                logicalAddr = GetLogicalEBlockAddr(devID,phyAddr);
                if(EMPTY_WORD == logicalAddr)
                {
                   continue;
                }
                key = tempArray[devID][eBlockCount].key;
                if((status = TABLE_TransLogEBInsert(devID, logicalAddr, phyAddr, key)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
             SetTransLogEBCounter(devID, tempEBCounter);

             TABLE_ClearReservedEB(devID);
             for(eBlockCount = SYSTEM_START_EBLOCK; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
             {
                #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
                SetEBErased(devID, eBlockCount, FTL_FALSE);
                #endif // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

                #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
                if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
                {
                   continue;
                }
                #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

                if (FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,eBlockCount))
                {
                   continue;
                }

                if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
          }
       }
       
       if(logIndexFound != EMPTY_DWORD)
       {

          transLogEBCounter = GetTransLogEBCounter(devID);
          if(logIndexFound == transLogEBCounter)
          {   // get rid of teh log entries, the current flush is already the latest one, dont need this 

             #if (DEBUG_FTL_API_ANNOUNCE == 1)
             DBG_Printf("TABLE_LoadFlushTable: logIndexFound=%d \n", logIndexFound, 0);
             #endif             

             if((shutdownLoc.eBlockNum == EMPTY_WORD) && (shutdownLoc.entryIndex == EMPTY_WORD))
             {

                #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
                for(eBlockCount = SYSTEM_START_EBLOCK; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
                {
                   SetEBErased(devID, eBlockCount, FTL_FALSE);
                }
                #endif // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)


                #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
                SuperEBInfo[devID].checkSysPF = FTL_TRUE;

                // for Power Failure Caes
                if(FTL_TRUE == SuperEBInfo[devID].checkLost || FTL_TRUE == SuperEBInfo[devID].checkSuperPF || FTL_TRUE == SuperEBInfo[devID].checkSysPF)
                {
                   for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
                   {
                      if((status = TABLE_GetSuperSysEBEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
                      {
                         break;// trying to excess outside table.
                      }
                      if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
                      {
                         break; // no more entries in table
                      }
                      //write  OLD_SYS_BLOCK_SIGNATURE
                      sysEBlockInfo.oldSysBlock    = OLD_SYS_BLOCK_SIGNATURE;
                      flashPage.devID            = devID;
                      flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, 0);
                      flashPage.vPage.pageOffset = (UINT16)((UINT32)&(tempSysPtr->oldSysBlock));
                      flashPage.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
                      if((flash_status = FLASH_RamPageWriteMetaData(&flashPage, (UINT8_PTR)&sysEBlockInfo.oldSysBlock)) != FLASH_PASS)
                      {
                         #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                         if(FLASH_PARAM == flash_status)
                         {
                            return FTL_ERR_SUPER_FLASH_WRITE_02;
                         }
                         SetBadEBlockStatus(devID, logicalAddr, FTL_TRUE);
                         
                         if(FLASH_MarkDefectEBlock(&flashPage) != FLASH_PASS)
                         {
                            // do nothing, just try to mark bad, even if it fails we move on.
                         }
                         #else
                         return FTL_ERR_SUPER_FLASH_WRITE_02;
                         #endif
                      }else{
                         if((status = TABLE_InsertReservedEB(devID, eBlockNum)) != FTL_ERR_PASS)
                         {
                            return status;
                         }
                      }
                   }
                   if(FTL_FALSE == checkFlag)
                   {
                      TABLE_SuperSysEBClear(devID);
                      TABLE_SuperSysEBInsert(devID, EMPTY_WORD, EMPTY_WORD, EMPTY_DWORD); // for next Super System EB
                      checkFlag = FTL_TRUE;
                   }
                }
                #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)


                #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
                {
                   status = FTL_EraseAllTransLogBlocksOp(devID);
                   if(status == FTL_ERR_MARKBB_COMMIT)
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

                #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                if((status = FTL_EraseAllTransLogBlocksOp(devID)) != FTL_ERR_PASS)
                {
                   return status;
                }
                #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

             }
          }
          else if(logIndexFound < transLogEBCounter)
          {
             if((shutdownLoc.eBlockNum == EMPTY_WORD) && (shutdownLoc.entryIndex == EMPTY_WORD))
             {
                if((status = FTL_RemoveOldTransLogBlocks(devID, logIndexFound)) != FTL_ERR_PASS)
                {
                   return status;
                }

                TABLE_ClearReservedEB(devID);
                for(eBlockCount = SYSTEM_START_EBLOCK; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
                {
                   #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
                   SetEBErased(devID, eBlockCount, FTL_FALSE);
                   #endif // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

                   #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
                   if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
                   {
                      continue;
                   }
                   #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

                   if (FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,eBlockCount))
                   {
                      continue;
                   }

                   if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                }
             }
          }
          else
          {
             DBG_Printf("TABLE_LoadFlushTable: logIndexFound=%d, ", logIndexFound, 0);
             DBG_Printf("transLogEBCounter=%d\n", transLogEBCounter, 0);
             return FTL_ERR_LOG_EB_COUNTER;
          }
       }        
       // Go and read Log entries, (if any)
       if((status = GetTransLogsSetRAMTables(devID, &shutdownLoc, &ramTablesUpdated, &logEblockFound)) != FTL_ERR_PASS)
       {
          return status;
       }

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       if((ramTablesUpdated == FTL_TRUE) || (logEblockFound == FTL_TRUE) || (pfFlushFlag == FTL_TRUE))
       #else
       if((ramTablesUpdated == FTL_TRUE) || (logEblockFound == FTL_TRUE))
       #endif
       #else  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
       if ((ramTablesUpdated == FTL_TRUE) || (logEblockFound == FTL_TRUE) || (flushEblockFailed == FTL_TRUE) || (pfFlushFlag == FTL_TRUE) || (FTL_TRUE == SuperEBInfo[devID].checkLost))
       #else
       if ((ramTablesUpdated == FTL_TRUE) || (logEblockFound == FTL_TRUE) || (flushEblockFailed == FTL_TRUE) || (pfFlushFlag == FTL_TRUE))       
       #endif
       #else
       if((ramTablesUpdated == FTL_TRUE) || (logEblockFound == FTL_TRUE) || (flushEblockFailed == FTL_TRUE))
       #endif
       #endif  // #else  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       {

          for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
          {
             tempArray[devID][eBlockCount].logicalEBNum = EMPTY_WORD;
             tempArray[devID][eBlockCount].phyAddr = EMPTY_WORD;
             tempArray[devID][eBlockCount].key = EMPTY_DWORD;
          }
          for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
          {
             if((status = TABLE_GetTransLogEntry(devID, eBlockCount, &logicalAddr, &phyAddr, &key)) != FTL_ERR_PASS)
             {
                break; // trying to excess outside table.
             }
             if((logicalAddr == EMPTY_WORD) && (phyAddr == EMPTY_WORD) && (key == EMPTY_DWORD))
             {
                break; // no more entries in table
             }

             logicalAddr = GetLogicalEBlockAddr(devID,phyAddr);
             tempArray[devID][eBlockCount].logicalEBNum = logicalAddr;
             tempArray[devID][eBlockCount].phyAddr = phyAddr;
             tempArray[devID][eBlockCount].key = key;
          }
          if((status = TABLE_TransEBClear(devID)) != FTL_ERR_PASS)
          {
             return status;
          }
          for(eBlockCount = 0; eBlockCount < NUM_TRANSACTION_LOG_EBLOCKS; eBlockCount++)
          {
             logicalAddr = tempArray[devID][eBlockCount].logicalEBNum;
             phyAddr = tempArray[devID][eBlockCount].phyAddr;
             key = tempArray[devID][eBlockCount].key;
             if((logicalAddr == EMPTY_WORD) && (phyAddr == EMPTY_WORD) && (key == EMPTY_DWORD))
             {
                 break;
             }
             if((status = TABLE_TransLogEBInsert(devID, logicalAddr, phyAddr, key)) != FTL_ERR_PASS)
             {
                 return status;
             }
          }


          TABLE_ClearReservedEB(devID);
          for(eBlockCount = SYSTEM_START_EBLOCK; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, eBlockCount, FTL_FALSE);
             #endif // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

             #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
             if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
             {
                continue;
             }
             #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
             
             if (FTL_ERR_PASS == TABLE_CheckUsedSysEB(devID,eBlockCount))
             {
                continue;
             }
                          
             if((status = TABLE_InsertReservedEB(devID, eBlockCount)) != FTL_ERR_PASS)
             {
                return status;
             }
          }

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if ((status = TABLE_Flush(FLUSH_GC_MODE)) != FTL_ERR_PASS)
          {
             return status;
          }
          if((status = GetSpareInfoSetPPATable()) != FTL_ERR_PASS)
          {
             return status;
          }
          #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

          #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
          SuperEBInfo[devID].checkSysPF = FTL_TRUE;

           // for Power Failure Caes
          if(FTL_TRUE == SuperEBInfo[devID].checkLost || FTL_TRUE == SuperEBInfo[devID].checkSuperPF || FTL_TRUE == SuperEBInfo[devID].checkSysPF)
          {
             for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
             {
                if((status = TABLE_GetSuperSysEBEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
                {
                   break;// trying to excess outside table.
                }
                if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
                {
                   break; // no more entries in table
                }
                //write  OLD_SYS_BLOCK_SIGNATURE
                sysEBlockInfo.oldSysBlock    = OLD_SYS_BLOCK_SIGNATURE;
                flashPage.devID            = devID;
                flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, 0);
                flashPage.vPage.pageOffset = (UINT16)((UINT32)&(tempSysPtr->oldSysBlock));
                flashPage.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
                if((flash_status = FLASH_RamPageWriteMetaData(&flashPage, (UINT8_PTR)&sysEBlockInfo.oldSysBlock)) != FLASH_PASS)
                {
                   #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   if(FLASH_PARAM == flash_status)
                   {
                      return FTL_ERR_SUPER_FLASH_WRITE_03;
                   }
                   SetBadEBlockStatus(devID, logicalAddr, FTL_TRUE);
                   
                   if(FLASH_MarkDefectEBlock(&flashPage) != FLASH_PASS)
                   {
                      // do nothing, just try to mark bad, even if it fails we move on.
                   }
                   #else
                   return FTL_ERR_SUPER_FLASH_WRITE_03;
                   #endif
                }else{
                   if((status = TABLE_InsertReservedEB(devID, logicalAddr)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                }
             }
             if(FTL_FALSE == checkFlag)
             {
                TABLE_SuperSysEBClear(devID);
                TABLE_SuperSysEBInsert(devID, EMPTY_WORD, EMPTY_WORD, EMPTY_DWORD); // for next Super System EB
             }
          }
          #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

          // Flush after load to mark a stable point.
          if((status = TABLE_Flush(FLUSH_GC_MODE)) != FTL_ERR_PASS)
          {
             return status;
          }

          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          eBlockCount = gTargetPftEBForNand;
          if(EMPTY_WORD != eBlockCount)
          {
             if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, eBlockCount, CACHE_INIT_TYPE)))
             {
                return status;
             }
             if(EMPTY_WORD != GetChainLogicalEBNum(devID, eBlockCount))
             {
                if(eBlockCount >= NUM_DATA_EBLOCKS)
                {
                   eBlockCount = GetChainLogicalEBNum(devID, eBlockCount);
                }
             }
             if(eBlockCount < NUM_DATA_EBLOCKS)
             {
                if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, eBlockCount, CACHE_INIT_TYPE)))
                {
                   return status;
                }

                #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                status = InternalForcedGCWithBBManagement(devID, eBlockCount, &validFreePageIndex, &freePageIndex, FTL_TRUE);

                #else
                status = FTL_InternalForcedGC(devID, eBlockCount, &validFreePageIndex, &freePageIndex, FTL_TRUE);
                #endif

                if(status != FTL_ERR_PASS)
                {
                   return status;
                }
             }

          }
          gTargetPftEBForNand = EMPTY_WORD;
          if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
          {
             return status;
          }
          #endif
          #endif

          shutdownLoc.eBlockNum = EMPTY_WORD;
          shutdownLoc.entryIndex = EMPTY_WORD;
       }
       if((status = ResetIndexValue(devID, &shutdownLoc)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(formatCount >= NUM_SYSTEM_EBLOCKS)
       {
          return FTL_ERR_FLUSH_TOO_MANY_BLOCKS;
       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(FTL_TRUE == checkBBMark)
       {
          // Flush BB Mark
          if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       #endif
    }

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if (CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
    if (index)
    {
       MEM_Free(index);
    }
    #endif
    #endif

    return FTL_ERR_PASS;
}

//-------------------------------
FTL_STATUS GetTransLogsSetRAMTables(FTL_DEV devID, LOG_ENTRY_LOC_PTR startLoc, UINT8_PTR ramTablesUpdated, 
    UINT8_PTR logEblockFound)
{
    FTL_STATUS status        = FTL_ERR_PASS;           /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    UINT16 entryIndex        = 0x0;                    /*2*/
    UINT16 byteOffset        = 0x0;                    /*2*/
    UINT16 logicalAddr       = 0x0;                    /*2*/
    UINT16 phyEBAddr         = 0x0;                    /*2*/
    UINT16 eBlockNum         = 0x0;                    /*2*/
    UINT16 logEntry[LOG_ENTRY_SIZE/2];                 /*16*/
    UINT16 logType           = EMPTY_WORD;             /*2*/
    UINT16 prevLogType       = EMPTY_WORD;             /*2*/
    UINT32 latestIncNumber   = 0x0;                    /*4*/
    UINT8  skipFlag          = FTL_FALSE;              /*1*/
    UINT16 savedEntryIndex   = 0x0;                    /*2*/
    UINT16 savedEBlockNum    = 0x0;                    /*2*/
    UINT8  GCLogTypeB        = EMPTY_BYTE;             /*1*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    UINT16 logEBNum          = 0x0;                    /*2*/
    UINT16 phyEBOffset       = 0x0;                    /*2*/
    UINT8  entryBCnt         = 0x0;                    /*1*/
    UINT8  pageLocEntryCnt   = 0x0;                    /*1*/
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    #if DEBUG_LOG_ENTRIES
    UINT8  pageCnt           = 0;                      /*1*/
    #endif  // #if DEBUG_LOG_ENTRIES

    #if DEBUG_EARLY_BAD_ENTRY
    UINT16 badEntry           = FTL_FALSE;             /*2*/
    #endif  // #if DEBUG_EARLY_BAD_ENTRY
     
    FLASH_PAGE_INFO   flashPageInfo = {0, 0, {0, 0}};  /*11*/
    SYS_EBLOCK_INFO   sysEBlockInfo;                   /*16*/
    GC_LOG_ENTRY      gcLog;
    TRANS_LOG_ENTRY_A_PTR getLogType;                  /*16*/
     
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    CHAIN_LOG_ENTRY   chainLog;                        /*16*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
    EBSWAP_LOG_ENTRY  ebSwapLog;                       /*16*/
    #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

    #if(FTL_UNLINK_GC == FTL_TRUE)
    UINT8 foundUnlinkLog = FTL_FALSE;                  /*1*/
    UINT8 unlinkLogTypeB = 0;                          /*1*/
    UINT16 count = 0;                                  /*2*/
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 freePageIndex = 0;                          /*2*/
    FREE_BIT_MAP_TYPE bitMap = 0;                      /*1*/
    UINT8 pageBitMap[GC_MOVE_BITMAP];
    UNLINK_LOG_ENTRY unlinkLog;
    #endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT8  transLogFlag = FTL_FALSE;                   /*1*/
    SPARE_LOG_ENTRY spareLog;                          /*16*/
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    *logEblockFound = FTL_FALSE;
    for(eBlockNum = 0; eBlockNum < NUM_TRANSACTION_LOG_EBLOCKS; eBlockNum++)
    {
       if((status = TABLE_GetTransLogEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
       {
          return status; // trying to excess outside table.
       }
       if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
       {
          break; // no more entries in table
       }
       if((startLoc->eBlockNum != EMPTY_WORD) && (startLoc->entryIndex != EMPTY_WORD))
       {
          if(eBlockNum < startLoc->eBlockNum)
          {
             continue;
          }
       }

       flashPageInfo.devID = devID;
       flashPageInfo.vPage.pageOffset = 0;
       flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, 0);

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       flashPageInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
       flash_status = FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)(&sysEBlockInfo));

       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       flashPageInfo.byteCount = SECTOR_SIZE;
       flash_status = FLASH_RamPageReadDataBlock(&flashPageInfo, &pseudoRPB[devID][0]);
       MEM_Memcpy((UINT8_PTR)(&sysEBlockInfo), &pseudoRPB[devID][0], SYS_INFO_SIZE);
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       if(flash_status != FLASH_PASS)
       {
          return FTL_ERR_FLASH_READ_05;
       }
       // Only if this valid Trans block
       if((FTL_FALSE == VerifyCheckWord((UINT16_PTR)&sysEBlockInfo.type, 
          SYS_INFO_DATA_WORDS, sysEBlockInfo.checkWord)) &&
          (sysEBlockInfo.oldSysBlock != OLD_SYS_BLOCK_SIGNATURE) && 
          (sysEBlockInfo.type == SYS_EBLOCK_INFO_LOG))
       {
             
          #if DEBUG_EARLY_BAD_ENTRY
          badEntry = FTL_FALSE;
          #endif  // #if DEBUG_EARLY_BAD_ENTRY
             
          // go through all the entries in the block
          for(entryIndex = 1; entryIndex < NUM_LOG_ENTRIES_PER_EBLOCK; entryIndex++)
          {
             if((startLoc->eBlockNum != EMPTY_WORD) && (startLoc->entryIndex != EMPTY_WORD))
             {
                if((eBlockNum == startLoc->eBlockNum) && (entryIndex < startLoc->entryIndex))
                {
                   continue;
                }
             }
             byteOffset = entryIndex * LOG_ENTRY_DELTA;
             flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, entryIndex);
             flashPageInfo.vPage.pageOffset = byteOffset % VIRTUAL_PAGE_SIZE;

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             flashPageInfo.byteCount = sizeof(logEntry);
             flash_status = FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)&logEntry[0]);

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             flashPageInfo.byteCount = SECTOR_SIZE;
             flash_status = FLASH_RamPageReadDataBlock(&flashPageInfo, &pseudoRPB[devID][0]);
             MEM_Memcpy( (UINT8_PTR)&logEntry[0], &pseudoRPB[devID][0],LOG_ENTRY_SIZE);
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flash_status != FLASH_PASS)
             {

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                // power failure check...
                if(flash_status == FLASH_ECC_FAIL)
                {
                   // power failure occurred, invoke Flush_GC
                   *logEblockFound = FTL_TRUE;
                   break;
                }
                else
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

                {
                   return FTL_ERR_FLASH_READ_04;
                }
             }
             if(*logEblockFound == FTL_FALSE)
             {
                if(FLASH_CheckEmpty((UINT8_PTR)&logEntry[0], LOG_ENTRY_SIZE) != FLASH_PASS)
                {
                   *logEblockFound = FTL_TRUE;
                }
             }

             #if DEBUG_EARLY_BAD_ENTRY
             if (FTL_TRUE == badEntry)
             {
                if (EMPTY_WORD != logEntry[LOG_ENTRY_DATA_START])
                {
                   DBG_Printf("GetTransLogsSetRAMTables: Error: Bad Log Entry\n", 0, 0); 
                   DBG_Printf("   detected before end of Log, entryIndex = %d\n", entryIndex - 1, 0);
                }
                break;
             }
             #endif  // #if DEBUG_EARLY_BAD_ENTRY
                 
             if (VerifyCheckWord(&logEntry[LOG_ENTRY_DATA_START], 
                LOG_ENTRY_DATA_WORDS, logEntry[LOG_ENTRY_CHECK_WORD]))
             {

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
                // Primary Copy is bad; Check Redundant Copy
                flashPageInfo.vPage.pageOffset += LOG_ENTRY_SIZE;
                if((FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)&logEntry[0])) != FLASH_PASS)
                {
                   return FTL_ERR_FLASH_READ_03;
                }
                flashPageInfo.vPage.pageOffset -= LOG_ENTRY_SIZE;
                if (VerifyCheckWord(&logEntry[LOG_ENTRY_DATA_START], 
                   LOG_ENTRY_DATA_WORDS, logEntry[LOG_ENTRY_CHECK_WORD]))
                #endif  // #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                {
                   // Log Entry is bad

                   #if DEBUG_EARLY_BAD_ENTRY     
                   badEntry = FTL_TRUE;
                   // Get one more entry
                   continue;

                   #else  // #if DEBUG_EARLY_BAD_ENTRY
                   // Don't check any more entries
                   break;
                   #endif  // #else  // #if DEBUG_EARLY_BAD_ENTRY

                }

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)

                #if DEBUG_REDUNDANT_MESG
                else
                {
                   DBG_Printf("GetTransLogsSetRAMTables: Use Redundant Log Entry: %d\n", entryIndex, 0);
                }
                #endif  // #if DEBUG_REDUNDANT_MESG

                #endif  // #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             }
             getLogType = (TRANS_LOG_ENTRY_A_PTR)logEntry;
             logType = getLogType->type; // get LogType
             if((UINT8)logType == EMPTY_BYTE)
             {
                break;
             }

             #if(FTL_UNLINK_GC == FTL_TRUE)
             if(foundUnlinkLog == FTL_TRUE)
             {
                if((status = UpdateRAMTablesUsingUnlinkLogs(devID, &unlinkLog)) != FTL_ERR_PASS)
                {
                   return status;
                }
                *ramTablesUpdated = FTL_TRUE;
                foundUnlinkLog = FTL_FALSE;
             }
             #endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

             switch(logType)
             {
                // ........Trans Logs...........
                case TRANS_LOG_TYPE_A:
                {
                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                   TranslogBEntries = 0;
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_TYPE_A_SEQUENCE;
                   }
                   prevLogType = TRANS_LOG_TYPE_A;
                   MEM_Memcpy((UINT8_PTR)&TransLogEntry.entryA, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, (UINT16)((TransLogEntry.entryA.LBA) / NUM_SECTORS_PER_EBLOCK), CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   #endif

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Transfer Log A skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("seqNum = %d, ", TransLogEntry.entryA.seqNum, 0);
                   DBG_Printf("LBA = 0x%X, ", TransLogEntry.entryA.LBA, 0);
                   DBG_Printf("checkWord = 0x%X\n", TransLogEntry.entryA.checkWord, 0);
                   for (pageCnt = 0; pageCnt < NUM_ENTRIES_TYPE_A; pageCnt++)
                   {
                      DBG_Printf("  pageLoc[%d]: ", pageCnt, 0);
                      DBG_Printf("logEBNum = 0x%X ," ,TransLogEntry.entryA.pageLoc[pageCnt].logEBNum, 0);
                      DBG_Printf("phyEBOffset = 0x%X\n" ,TransLogEntry.entryA.pageLoc[pageCnt].phyEBOffset, 0);
                   }
                   #endif  // #if DEBUG_LOG_ENTRIES

                   break;

                   #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                   return FTL_ERR_LOG_TYPE_A_ENTRY;
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                }
                case TRANS_LOG_TYPE_B:
                {
                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                   if(prevLogType != TRANS_LOG_TYPE_A && prevLogType != TRANS_LOG_TYPE_B) // correct seq. check.
                   {
                      return FTL_ERR_LOG_TYPE_B_SEQUENCE;
                   }
                   prevLogType = TRANS_LOG_TYPE_B;
                   MEM_Memcpy((UINT8_PTR)&TransLogEntry.entryB[TranslogBEntries], (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Transfer Log B skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("seqNum = %d, ", TransLogEntry.entryB[TranslogBEntries].seqNum, 0);
                   DBG_Printf("checkWord = 0x%X\n", TransLogEntry.entryB[TranslogBEntries].checkWord, 0);
                   for (pageCnt = 0; pageCnt < NUM_ENTRIES_TYPE_B; pageCnt++)
                   {
                      DBG_Printf("  pageLoc[%d]: ", pageCnt, 0);
                      DBG_Printf("logEBNum = 0x%X ," ,TransLogEntry.entryB[TranslogBEntries].pageLoc[pageCnt].logEBNum, 0);
                      DBG_Printf("phyEBOffset = 0x%X\n" ,TransLogEntry.entryB[TranslogBEntries].pageLoc[pageCnt].phyEBOffset, 0);
                   }
                   #endif  // #if DEBUG_LOG_ENTRIES

                   TranslogBEntries++;
                   break;

                   #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                   return FTL_ERR_LOG_TYPE_B_ENTRY;
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                }
                case TRANS_LOG_TYPE_C:
                {
                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                   if((prevLogType == TRANS_LOG_TYPE_A) || (prevLogType == TRANS_LOG_TYPE_B)) // correct seq. check.
                   {
                      prevLogType = EMPTY_WORD;
                   }
                   else
                   {
                      return FTL_ERR_LOG_TYPE_C_SEQUENCE;
                   }
                   MEM_Memcpy((UINT8_PTR)&TransLogEntry.entryC, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Transfer Log C skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("seqNum = %d, ", TransLogEntry.entryC.seqNum, 0);
                   DBG_Printf("GCNum = 0x%X, ",TransLogEntry.entryC.GCNum, 0);
                   DBG_Printf("checkWord = 0x%X\n", TransLogEntry.entryC.checkWord, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   if (FTL_TRUE == skipFlag)
                   {
                      skipFlag = FTL_FALSE;
                      // Back up to the saved point
                      entryIndex = savedEntryIndex;
                      eBlockNum = savedEBlockNum;
                      prevLogType = GC_TYPE_A;
                      // Don't process this yet
                      break;
                   }
                   if((status = UpdateRAMTablesUsingTransLogs(devID)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   *ramTablesUpdated = FTL_TRUE;
                   break;

                   #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                   return FTL_ERR_LOG_TYPE_C_ENTRY;
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                }
                // .........GC Logs...........
                case GC_TYPE_A:
                {
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_GC_A_SEQUENCE;
                   }
                   prevLogType = GC_TYPE_A;
                   if (FTL_FALSE == skipFlag)
                   {
                      MEM_Memcpy((UINT8_PTR)&gcLog.partA, (UINT8_PTR)&logEntry[0], sizeof(logEntry));
                   }

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, gcLog.partA.logicalEBAddr, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   #endif

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("GC Log A skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("GCNum = 0x%X, ", gcLog.partA.GCNum, 0);
                   DBG_Printf("holdForMerge = %d,\n", gcLog.partA.holdForMerge, 0);
                   DBG_Printf("  logicalEB = 0x%X, ", gcLog.partA.logicalEBAddr, 0);
                   DBG_Printf("resEB = 0x%X, ", gcLog.partA.reservedEBAddr, 0);
                   DBG_Printf("checkWord = 0x%X\n", gcLog.partA.checkWord, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
                   SetEBErased(devID, gcLog.partA.reservedEBAddr, FTL_FALSE);
                   #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

                   if (FTL_TRUE == gcLog.partA.holdForMerge)
                   {
                      if (FTL_FALSE == skipFlag)
                      {
                         #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                         if (FTL_FALSE == transLogFlag)
                         #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                         {
                            // Stop processing until a TRANS_TYPE_C is found,
                            //   then starting processing again with the next entry
                            skipFlag = FTL_TRUE;
                            savedEntryIndex = entryIndex;
                            savedEBlockNum = eBlockNum;
                         }
                      }
                   }
                   break;
                }
                case GC_TYPE_B:
                {
                   // correct seq. check.
                   if(prevLogType == GC_TYPE_A)
                   {
                      GCLogTypeB = 0;
                   }

                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                   else if(prevLogType == GC_TYPE_B)
                   {
                      GCLogTypeB++;
                   }
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                   else
                   {
                      return FTL_ERR_LOG_GC_B_SEQUENCE;
                   }

                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                   #if (NUM_GC_TYPE_B > 1)
                   if(GCLogTypeB < (NUM_GC_TYPE_B - 1))
                   {
                      prevLogType = GC_TYPE_B;
                   }
                   else
                   {
                      prevLogType = EMPTY_WORD;
                   }
                   #else
                   prevLogType = EMPTY_WORD;
                   #endif

                   MEM_Memcpy((UINT8_PTR)&gcLog.partB[GCLogTypeB], (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("GC Log B skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("checkWord = 0x%X, pageMovedBitMap:\n", gcLog.partB[GCLogTypeB].checkWord, 0);
                   for (pageCnt = 0; pageCnt < NUM_ENTRIES_GC_TYPE_B; pageCnt++)
                   {
                      DBG_Printf(" %02X", gcLog.partB[GCLogTypeB].pageMovedBitMap[pageCnt], 0);
                   }
                   DBG_Printf("\n", 0, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   if(GCLogTypeB == (NUM_GC_TYPE_B - 1))
                   {
                      if (FTL_FALSE == skipFlag)
                      {
                         if((status = UpdateRAMTablesUsingGCLogs(devID, &gcLog)) != FTL_ERR_PASS)
                         {
                            return status;
                         } 
                         *ramTablesUpdated = FTL_TRUE;
                      }
                   }
                   break;

                   #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                   prevLogType = GC_TYPE_B;

                   if((status = ReadPackedGCLogs(&pseudoRPB[devID][0], &gcLog)) == FTL_ERR_PASS)
                   {
                      GCLogTypeB = 0;
                      prevLogType = EMPTY_WORD;
                      if (FTL_FALSE == skipFlag)
                      {
                         transLogFlag = FTL_FALSE;
                         if((status = UpdateRAMTablesUsingGCLogs(devID, &gcLog)) != FTL_ERR_PASS)
                         {
                            return status;
                         } 
                         *ramTablesUpdated = FTL_TRUE;
                      }
                   }
                   else
                   {
                      return status;
                   }
                   break;
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                }
                    
                #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
                // ........Chain Logs..........
                case CHAIN_LOG_TYPE:
                {
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_CHAIN_SEQUENCE;
                   }
                   MEM_Memcpy((UINT8_PTR)&chainLog, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, chainLog.logicalFrom, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }

                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, chainLog.logicalTo, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   #endif

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Chain Log skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("logFrom = 0x%X, ", chainLog.logicalFrom, 0);
                   DBG_Printf("logTo = 0x%X,\n", chainLog.logicalTo, 0);
                   DBG_Printf("  phyFrom = 0x%X, ", chainLog.phyFrom, 0);
                   DBG_Printf("phyTo = 0x%X\n", chainLog.phyTo, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES
 
                   if (FTL_FALSE == skipFlag)
                   {
                      if((status = UpdateRAMTablesUsingChainLogs(devID, &chainLog)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      *ramTablesUpdated = FTL_TRUE;
                   }
                   break;
                }
                #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

                #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
                // ........EBSwap Logs..........
                case EBSWAP_LOG_TYPE:
                {
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_EBSWAP_SEQUENCE;
                   }
                   MEM_Memcpy((UINT8_PTR)&ebSwapLog, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("EBSwap Log skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("logicalDataEB = 0x%X, ", ebSwapLog.logicalDataEB, 0);
                   DBG_Printf("logicalReservedEB = 0x%X,\n", ebSwapLog.logicalReservedEB, 0);
                   DBG_Printf("  checkWord = 0x%X\n", ebSwapLog.checkWord, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   if (FTL_FALSE == skipFlag)
                   {
                      if((status = UpdateRAMTablesUsingEBSwapLogs(devID, &ebSwapLog)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      *ramTablesUpdated = FTL_TRUE;
                   }
                   break;
                }
                #endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)

                #if(FTL_UNLINK_GC == FTL_TRUE)
                // ........UnlinkGC Logs..........
                case UNLINK_LOG_TYPE_A1:
                {
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_UNLINK_A1_SEQUENCE;
                   }
                   MEM_Memcpy((UINT8_PTR)&unlinkLog.partA, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, unlinkLog.partA.fromLogicalEBAddr, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, unlinkLog.partA.toLogicalEBAddr, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   #endif

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Unlink GC Log A1 skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("fromLogicalEB = 0x%X, ", unlinkLog.partA.fromLogicalEBAddr, 0);
                   DBG_Printf("toLogicalEB = 0x%X, ", unlinkLog.partA.toLogicalEBAddr, 0);
                   DBG_Printf("checkWord = 0x%X\n", unlinkLog.partA.checkWord, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   if(FTL_FALSE == skipFlag)
                   {
                      if((status = UpdateRAMTablesUsingUnlinkLogs(devID, &unlinkLog)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      *ramTablesUpdated = FTL_TRUE;
                   }
                   break;
                }
                case UNLINK_LOG_TYPE_A2:
                {
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_UNLINK_A2_SEQUENCE;
                   }
                   prevLogType = UNLINK_LOG_TYPE_A2;
                   MEM_Memcpy((UINT8_PTR)&unlinkLog.partA, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, unlinkLog.partA.fromLogicalEBAddr, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   if (FTL_ERR_PASS != (status = CACHE_LoadEB(devID, unlinkLog.partA.toLogicalEBAddr, CACHE_INIT_TYPE)))
                   {
                      return status;
                   }
                   #endif

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Unlink GC Log A2 skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("fromLogicalEB = 0x%X, ", unlinkLog.partA.fromLogicalEBAddr, 0);
                   DBG_Printf("toLogicalEB = 0x%X, ", unlinkLog.partA.toLogicalEBAddr, 0);
                   DBG_Printf("checkWord = 0x%X\n", unlinkLog.partA.checkWord, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                   if((status = ReadPackedUnlinkLogs(&pseudoRPB[devID][0], &unlinkLog)) == FTL_ERR_PASS)
                   {
                      unlinkLogTypeB = 0;
                      prevLogType = EMPTY_WORD;
                      if (FTL_FALSE == skipFlag)
                      {
                         foundUnlinkLog = FTL_TRUE;
                      }
                   }
                   else
                   {
                      return status;
                   }
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

                   break;
                }
                case UNLINK_LOG_TYPE_B:
                {
                   #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                   // correct seq. check.
                   if(prevLogType == UNLINK_LOG_TYPE_A2)
                   {
                      unlinkLogTypeB = 0;
                   }
                   else if(prevLogType == UNLINK_LOG_TYPE_B)
                   {
                      unlinkLogTypeB++;
                   }
                   else
                   {
                      return FTL_ERR_LOG_UNLINK_B_SEQUENCE;
                   }

                   #if (NUM_UNLINK_TYPE_B > 1)
                   if(unlinkLogTypeB < (NUM_UNLINK_TYPE_B - 1))
                   {
                      prevLogType = UNLINK_LOG_TYPE_B;
                   }
                   else
                   {
                      prevLogType = EMPTY_WORD;
                   }
                   #else
                   prevLogType = EMPTY_WORD;
                   #endif

                   MEM_Memcpy((UINT8_PTR)&unlinkLog.partB[unlinkLogTypeB], (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("Unlink Log B skipFlag = %d, ", skipFlag, 0);
                   DBG_Printf("checkWord = 0x%X, pageMovedBitMap:\n", unlinkLog.partB[unlinkLogTypeB].checkWord, 0);
                   for (pageCnt = 0; pageCnt < NUM_ENTRIES_UNLINK_TYPE_B; pageCnt++)
                   {
                      DBG_Printf(" %02X", unlinkLog.partB[unlinkLogTypeB].pageMovedBitMap[pageCnt], 0);
                   }
                   DBG_Printf("\n", 0, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   if(unlinkLogTypeB == (NUM_UNLINK_TYPE_B - 1))
                   {
                      if(FTL_FALSE == skipFlag)
                      {
                         foundUnlinkLog = FTL_TRUE;
                      }
                   }
                   break;

                   #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                   return FTL_ERR_LOG_UNLINK_B_ENTRY;
                   #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                }
                #endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                // ........Spare Logs...........
                case SPARE_LOG_TYPE:
                {
                   if(prevLogType != EMPTY_WORD) // correct seq. check.
                   {
                      return FTL_ERR_LOG_TYPE_SPARE_SEQ;
                   }
                   MEM_Memcpy((UINT8_PTR)&spareLog, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

                   if (FTL_TRUE == skipFlag)
                   {
                      skipFlag = FTL_FALSE;
                      transLogFlag = FTL_TRUE;
                      // Back up to the saved point
                      entryIndex = savedEntryIndex;
                      eBlockNum = savedEBlockNum;
                      prevLogType = GC_TYPE_A;
                      // Don't process this yet
                      break;
                   }
                   *ramTablesUpdated = FTL_TRUE;
                   break;
                }
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

                default:
                {
                   break;
                }
             }  // switch
          } // entryIndex loop
       } // valid trans block check
    } // eBlockNum loop

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    if((prevLogType == TRANS_LOG_TYPE_A) || (prevLogType == TRANS_LOG_TYPE_B))
    {
       // Previous write operation did not complete
       // Mark all pages listed as stale so they will not be re-used
       // Trans Entry A 
       for(pageLocEntryCnt = 0; pageLocEntryCnt < NUM_ENTRIES_TYPE_A; pageLocEntryCnt++)
       {
          logEBNum = TransLogEntry.entryA.pageLoc[pageLocEntryCnt].logEBNum; 
          phyEBOffset = TransLogEntry.entryA.pageLoc[pageLocEntryCnt].phyEBOffset;
          if(logEBNum == EMPTY_WORD)
          {
             break;  // No more Type A entries
          }

          #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
          if (CHAIN_FLAG == (CHAIN_FLAG & phyEBOffset))
          {
             // Page is in chained-to EBlock
             logEBNum = GetChainLogicalEBNum(devID, logEBNum);
             phyEBOffset = phyEBOffset & ~CHAIN_FLAG;
          }
          #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

          // If chain-to EBlock was never assigned, do not update table
          if (FTL_ERR_PASS != TABLE_CheckUsedSysEB(devID,logEBNum))
          {
             #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
             SetEBErased(devID, logEBNum, FTL_FALSE);
             #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

             // if page was valid before the transaction, it is still valid when the transaction is canceled
             if (BLOCK_INFO_VALID_PAGE != GetEBlockMapFreeBitIndex(devID, logEBNum, phyEBOffset))
             {
                UpdatePageTableInfo(devID, logEBNum, EMPTY_INVALID, phyEBOffset, BLOCK_INFO_STALE_PAGE);
             }
          }
       }
       // Trans Entry B
       for(entryBCnt = 0; entryBCnt < TranslogBEntries; entryBCnt++)
       {
          for(pageLocEntryCnt = 0; pageLocEntryCnt < NUM_ENTRIES_TYPE_B; pageLocEntryCnt++)
          {
             logEBNum = TransLogEntry.entryB[entryBCnt].pageLoc[pageLocEntryCnt].logEBNum; 
             phyEBOffset = TransLogEntry.entryB[entryBCnt].pageLoc[pageLocEntryCnt].phyEBOffset;
             if(logEBNum == EMPTY_WORD)
             {
                break; // No more Type B entries
             }

             #if( FTL_EBLOCK_CHAINING == FTL_TRUE)
             if (CHAIN_FLAG == (CHAIN_FLAG & phyEBOffset))
             {
                // Page is in chained-to EBlock
                logEBNum = GetChainLogicalEBNum(devID, logEBNum);
                phyEBOffset = phyEBOffset & ~CHAIN_FLAG;
             }
             #endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

             // If chain-to EBlock was never assigned, do not update table
             if (FTL_ERR_PASS != TABLE_CheckUsedSysEB(devID,logEBNum))
             {
                #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
                SetEBErased(devID, logEBNum, FTL_FALSE);
                #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

                // if page was valid before the transaction, it is still valid when the transaction is canceled
                if (BLOCK_INFO_VALID_PAGE != GetEBlockMapFreeBitIndex(devID, logEBNum, phyEBOffset))
                {
                   UpdatePageTableInfo(devID, logEBNum, EMPTY_INVALID, phyEBOffset, BLOCK_INFO_STALE_PAGE);
                }
             }
          }
       }
    }
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    #if(FTL_UNLINK_GC == FTL_TRUE)
    if(foundUnlinkLog == FTL_TRUE)
    {
       for(count = 0; count < NUM_UNLINK_TYPE_B; count++)
       {
          MEM_Memcpy(&pageBitMap[count*NUM_ENTRIES_UNLINK_TYPE_B], &unlinkLog.partB[count].pageMovedBitMap[0], NUM_ENTRIES_UNLINK_TYPE_B);
       }

       for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
       {
          bitMap = GetBitMapField(&pageBitMap[0], pageOffset, 1);
          if(bitMap == GC_MOVED_PAGE)
          {
             freePageIndex = GetFreePageIndex(devID, unlinkLog.partA.toLogicalEBAddr);
             UpdatePageTableInfo(devID, unlinkLog.partA.toLogicalEBAddr, EMPTY_INVALID, freePageIndex, BLOCK_INFO_STALE_PAGE);
          }
       }
       foundUnlinkLog = FTL_FALSE;
    }
    #endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

    return FTL_ERR_PASS;
}

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
//------------------------------
FTL_STATUS ProcessPageLoc(FTL_DEV devID, LOG_PHY_PAGE_LOCATION_PTR pageLocPtr, 
    UINT32 pageAddress)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 logicalEBNum = 0;                           /*2*/
    UINT16 logicalPageOffset = 0;                      /*2*/
    UINT16 phyPageOffset = 0;                          /*2*/

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 chainLogEBNum = EMPTY_WORD;                 /*2*/
    #endif  //  #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    
    logicalEBNum = pageLocPtr->logEBNum;
    
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    chainLogEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
    #endif  //  #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    if((status = GetLogicalPageOffset(pageAddress, &logicalPageOffset)) != FTL_ERR_PASS)
    {
        return status;
    }
    // ...... Process Old Page .......
    // If this transfer follows another transfer, then the old page was a 
    //   merged-page that needs to be marked stale.
    // If this transfer follows a GC, then the GC was part of the same Write Op.
    //   The GC Log marked merged-from pages as erased.  Other PageLoc elements
    //   in this Transfer Log may have re-marked the old page to valid.     
    phyPageOffset = GetPPASlot(devID, logicalEBNum, logicalPageOffset);
    if(phyPageOffset != EMPTY_INVALID) 
    {
       // Old page exists
       
       #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
       if (EMPTY_WORD != chainLogEBNum)
       {
          // Block has been chained
          if (pageLocPtr->phyEBOffset & CHAIN_FLAG)
          {
             // New page is in chain-to block
             if (phyPageOffset != CHAIN_INVALID)
             {
                // Old page is in chained-from EBlock
                // Marked old page as chained
                UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset,
                   CHAIN_INVALID, BLOCK_INFO_STALE_PAGE);
             }
             else
             {
                // Old page is in chained-to EBlock
                // Mark old page as stale
                UpdatePageTableInfo(devID, chainLogEBNum, logicalPageOffset, 
                   EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
             }
          }
          else
          {
             // New page is in chain-from block, therefore
             // Old page must also be in chained-from EBlock
             // Mark old page as stale
             UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
                EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
          }
       }
       else
       #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

       {
          // Block has not been chained or Chaining is turned off
          // Mark old page as stale.
          UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
             EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
       }
    }

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    else
    {
       // Old page does not exist
       if (pageLocPtr->phyEBOffset & CHAIN_FLAG)
       {
          // New page is in chain-to block
          // Mark old location as chained
          UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset,
             CHAIN_INVALID, BLOCK_INFO_STALE_PAGE);
       }
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // .... Process New Page .....
    phyPageOffset = (UINT16) (pageLocPtr->phyEBOffset & PPA_MASK);
    
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if (EMPTY_WORD != chainLogEBNum)
    {
       // Block has been chained
       if (pageLocPtr->phyEBOffset & CHAIN_FLAG)
       {
          // New page is in chain-to block      
          UpdatePageTableInfo(devID, chainLogEBNum, logicalPageOffset,
             phyPageOffset, BLOCK_INFO_VALID_PAGE);

          #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
          SetEBErased(devID, chainLogEBNum, FTL_FALSE);
          #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

       }
       else
       {
          // New page is in chain-from block
          UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
             phyPageOffset, BLOCK_INFO_VALID_PAGE);

          #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
          SetEBErased(devID, logicalEBNum, FTL_FALSE);
          #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

       }
    }
    else
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    {
       // Block has not been chained or chaining is turned off
       UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
          phyPageOffset, BLOCK_INFO_VALID_PAGE);

       #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
       SetEBErased(devID, logicalEBNum, FTL_FALSE);
       #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS UpdateRAMTablesUsingTransLogs(FTL_DEV devID)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT8  entryBCnt         = 0x0;                    /*1*/
    UINT8  pageLocEntryCnt   = 0x0;                    /*1*/
    UINT32 pageAddress       = 0x0;                    /*4*/

    LastTransLogLba = TransLogEntry.entryA.LBA;
    LastTransLogNpages = 0;
    if((status = GetPageNum(TransLogEntry.entryA.LBA, &pageAddress)) != FTL_ERR_PASS)
    {
       return status;
    }
    // Trans Entry A update...
    for(pageLocEntryCnt = 0; pageLocEntryCnt < NUM_ENTRIES_TYPE_A; pageLocEntryCnt++)
    {
       if(TransLogEntry.entryA.pageLoc[pageLocEntryCnt].logEBNum == EMPTY_WORD)
       {
          break;  // No more Type A entries
       }
       status = ProcessPageLoc(devID, &TransLogEntry.entryA.pageLoc[pageLocEntryCnt], 
          pageAddress);
       if (status != FTL_ERR_PASS)
       {
          return status;
       }
       LastTransLogNpages++;
       pageAddress++;
    }
    // Trans Entry B update...
    for(entryBCnt = 0; entryBCnt < TranslogBEntries; entryBCnt++)
    {
       for(pageLocEntryCnt = 0; pageLocEntryCnt < NUM_ENTRIES_TYPE_B; pageLocEntryCnt++)
       {
          if(TransLogEntry.entryB[entryBCnt].pageLoc[pageLocEntryCnt].logEBNum == EMPTY_WORD)
          {
             break; // because writting EntryB is optional..
          }
          status = ProcessPageLoc(devID, &TransLogEntry.entryB[entryBCnt].pageLoc[pageLocEntryCnt], 
             pageAddress);
          if (status != FTL_ERR_PASS)
          {
             return status;
          }
          LastTransLogNpages++;
          pageAddress++;
       }
    }
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
//-----------------------------
FTL_STATUS ProcessGCPageBitMap(FTL_DEV devID, UINT16 logicalEBNum, UINT8_PTR pageBitMap)
{
    UINT8 bitMapData          = 0x0;                   /*1*/
    UINT16 logicalPageOffset  = EMPTY_WORD;            /*2*/
    UINT16 phyPageOffset      = EMPTY_WORD;            /*2*/
    FREE_BIT_MAP_TYPE bitMap  = 0;                     /*1*/

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 chainEBNum         = EMPTY_WORD;            /*2*/
    UINT16 chainFreePageIndex = EMPTY_WORD;            /*2*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Mark erased pages
    for(logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
    {
       bitMapData = GetBitMapField(&pageBitMap[0], logicalPageOffset, 1);
       if(bitMapData != GC_MOVED_PAGE)
       {
          //  Mark the page empty using the logical address
          //  The bit map includes the pages copied from either EBlock
          UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
             EMPTY_INVALID, BLOCK_INFO_EMPTY_PAGE);
          // Don't clear the corresponding page in the chained-to EBlock
       }
    }

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    // clear the chain info
    if(chainEBNum != EMPTY_WORD)
    {  
       ClearChainLink(devID, logicalEBNum, chainEBNum);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Mark all stale pages as free
    for (phyPageOffset = 0; phyPageOffset < NUM_PAGES_PER_EBLOCK; phyPageOffset++)
    {
       bitMap = GetEBlockMapFreeBitIndex(devID, logicalEBNum, phyPageOffset);
       if (BLOCK_INFO_STALE_PAGE == bitMap)
       {
          // Note: PPA table should not include this page
          SetEBlockMapFreeBitIndex(devID, logicalEBNum, phyPageOffset, BLOCK_INFO_EMPTY_PAGE);
       }       
    }

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    if (chainEBNum != EMPTY_WORD)
    {
       // Validate pages copied from the chained-to EBlock
       for(logicalPageOffset = 0; logicalPageOffset < NUM_PAGES_PER_EBLOCK; logicalPageOffset++)
       {
          bitMapData = GetBitMapField(&pageBitMap[0], logicalPageOffset, 1);
          if(bitMapData == GC_MOVED_PAGE)
          {
             // Page was copied
             if (EMPTY_INVALID != GetPPASlot(devID, chainEBNum, logicalPageOffset))
             {
                // Page came from chained-to EBlock
                // Find first free page
                chainFreePageIndex = GetFreePageIndex(devID, logicalEBNum);
                UpdatePageTableInfo(devID, logicalEBNum, logicalPageOffset, 
                   chainFreePageIndex, BLOCK_INFO_VALID_PAGE);
                // Mark old page stale
                UpdatePageTableInfo(devID, chainEBNum, logicalPageOffset,
                   EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
             }
          }
       }
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Update dirty bit in Block_Info table
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    return FTL_ERR_PASS;
}

#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
//-----------------------------
FTL_STATUS ProcessGCPageBitMap(FTL_DEV devID, UINT16 logicalEBNum, UINT8_PTR pageBitMap)
{
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 freePageIndex = 0;                          /*2*/
    FREE_BIT_MAP_TYPE bitMap = 0;                      /*1*/

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 chainEBNum = EMPTY_WORD;                    /*2*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    TABLE_ClearFreeBitMap(devID, logicalEBNum);

    for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
    {
       bitMap = GetBitMapField(&pageBitMap[0], pageOffset, 1);
       if(bitMap == GC_MOVED_PAGE)
       {
          freePageIndex = GetFreePageIndex(devID, logicalEBNum);
          UpdatePageTableInfo(devID, logicalEBNum, pageOffset, freePageIndex, BLOCK_INFO_VALID_PAGE);
       }
       else
       {
          SetPPASlot(devID, logicalEBNum, pageOffset, EMPTY_INVALID);
       }
    }

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
    if(chainEBNum != EMPTY_WORD)
    {
       ClearChainLink(devID, logicalEBNum, chainEBNum);
       SetDirtyCount(devID, chainEBNum, 0);
       MarkEBlockMappingTableEntryDirty(devID, chainEBNum);
       MarkAllPagesStatus(devID, chainEBNum, BLOCK_INFO_STALE_PAGE);
    }
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    // Update dirty bit in Block_Info table
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

//-----------------------------
FTL_STATUS UpdateRAMTablesUsingGCLogs(FTL_DEV devID, GC_LOG_ENTRY_PTR ptrGCLog)
{
    FTL_STATUS status         = FTL_ERR_PASS;          /*4*/
    UINT16 logicalEBNum       = EMPTY_WORD;            /*2*/
    UINT16 reservedEBNum      = EMPTY_WORD;            /*2*/
    UINT16 phyFromEBlock      = EMPTY_WORD;            /*2*/
    UINT16 phyToEBlock        = EMPTY_WORD;            /*2*/
    UINT32 oldToEraseCount    = EMPTY_DWORD;           /*2*/
    UINT32 oldFromEraseCount  = EMPTY_DWORD;           /*2*/
    UINT8 count               = EMPTY_BYTE;            /*1*/
    UINT8 pageBitMap[GC_MOVE_BITMAP];

    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    UINT16 chainEBNum         = EMPTY_WORD;            /*2*/
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT8 badEBlockFlag = FTL_FALSE;             
    #endif
    logicalEBNum = ptrGCLog->partA.logicalEBAddr;
    reservedEBNum = ptrGCLog->partA.reservedEBAddr;
    phyFromEBlock = GetPhysicalEBlockAddr(devID, logicalEBNum);
    phyToEBlock = GetPhysicalEBlockAddr(devID, reservedEBNum);
    
    #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
    chainEBNum = GetChainLogicalEBNum(devID, logicalEBNum);
    #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

    for (count = 0; count < NUM_GC_TYPE_B; count++)
    {
       MEM_Memcpy(&pageBitMap[count * NUM_ENTRIES_GC_TYPE_B], \
                  &ptrGCLog->partB[count].pageMovedBitMap[0], \
                  NUM_ENTRIES_GC_TYPE_B);
    }
    
    // Swap EBlocks
    oldToEraseCount = GetEraseCount(devID, reservedEBNum);
    oldFromEraseCount = GetEraseCount(devID, logicalEBNum);
    SetPhysicalEBlockAddr(devID, reservedEBNum, phyFromEBlock);
    SetPhysicalEBlockAddr(devID, logicalEBNum, phyToEBlock);
    SetEraseCount(devID, reservedEBNum, oldFromEraseCount);
    SetEraseCount(devID, logicalEBNum, oldToEraseCount);
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    #if (FTL_STATIC_WEAR_LEVELING == FTL_TRUE)
    oldFromEraseCount = GetTrueEraseCount(devID, reservedEBNum);
    status = SetSaveStaticWL(devID, reservedEBNum, oldFromEraseCount);
    if (status != FTL_ERR_PASS)
    {
       return status;
    }
    oldToEraseCount = GetTrueEraseCount(devID, logicalEBNum);
    status = SetSaveStaticWL(devID, logicalEBNum, oldToEraseCount);
    if (status != FTL_ERR_PASS)
    {
       return status;
    }
    #endif
    #endif
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    badEBlockFlag = GetBadEBlockStatus(devID, reservedEBNum);             
    SetBadEBlockStatus(devID, reservedEBNum, GetBadEBlockStatus(devID, logicalEBNum));
    SetBadEBlockStatus(devID, logicalEBNum, badEBlockFlag);
    #endif    
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    if (GetEBErased(devID, logicalEBNum) == FTL_FALSE)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    {
       IncEraseCount(devID, logicalEBNum);
    }

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, logicalEBNum, FTL_FALSE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    MarkEBlockMappingTableEntryDirty(devID, reservedEBNum);

    if((status = ProcessGCPageBitMap(devID, logicalEBNum, pageBitMap)) != FTL_ERR_PASS)
    {
       return status;
    }

    // Change stale pages to empty. 
    // Note: Chained pages are stale in the chained-from EBlock
    SetGCOrFreePageNum(devID, logicalEBNum, ptrGCLog->partA.GCNum);    
    SetDirtyCount(devID, logicalEBNum, 0);
    MarkEBlockMappingTableEntryDirty(devID, logicalEBNum);
    return FTL_ERR_PASS;
}

#if(FTL_EBLOCK_CHAINING == FTL_TRUE)
//-----------------------------
FTL_STATUS UpdateRAMTablesUsingChainLogs(FTL_DEV devID, CHAIN_LOG_ENTRY_PTR chainLogPtr)
{
    UINT16 logicalToEBNum = EMPTY_WORD;                /*2*/
    UINT16 logicalFromEBNum = EMPTY_WORD;              /*2*/
    
    logicalToEBNum = chainLogPtr->logicalTo;
    logicalFromEBNum = chainLogPtr->logicalFrom;

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    if(GetEBErased(devID, logicalToEBNum) == FTL_FALSE)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    {
        IncEraseCount(devID, logicalToEBNum);
    }
        // Clear the bit map and ppa tables of the chained-to EBlock
    SetDirtyCount(devID, logicalToEBNum, 0);
    TABLE_ClearFreeBitMap(devID, logicalToEBNum);
    TABLE_ClearPPATable(devID, logicalToEBNum);
    MarkPPAMappingTableEntryDirty(devID, logicalToEBNum, 0);

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, logicalToEBNum, FTL_FALSE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    SetChainLink(devID, logicalFromEBNum, logicalToEBNum, 
       chainLogPtr->phyFrom, chainLogPtr->phyTo);
    MarkEBlockMappingTableEntryDirty(devID, logicalFromEBNum);
    MarkEBlockMappingTableEntryDirty(devID, logicalToEBNum);
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

#if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)
//-----------------------------
FTL_STATUS UpdateRAMTablesUsingEBSwapLogs(FTL_DEV devID, EBSWAP_LOG_ENTRY_PTR EBSwapLogPtr)
{
    SwapUnusedEBlock(devID, EBSwapLogPtr->logicalDataEB, EBSwapLogPtr->logicalReservedEB);
    return FTL_ERR_PASS;
}

//--------------------------------------
FTL_STATUS CreateSwapEBLog(FTL_DEV devID, UINT16 logicalDataEB, UINT16 logicalReservedEB)
{
    EBSWAP_LOG_ENTRY ebSwapLog;                        /*16*/
    FLASH_PAGE_INFO flashPageInfo = {0, 0, {0, 0}};    /*11*/
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT8 count = 0;                                   /*1*/

    if((status = GetNextLogEntryLocation(devID, &flashPageInfo)) != FTL_ERR_PASS)
    {
       return status;
    }
    for (count = 0; count < EBSWAP_LOG_ENTRY_RESERVED; count++)
    {
       ebSwapLog.reserved[count] = EMPTY_BYTE;
    }
    ebSwapLog.type = EBSWAP_LOG_TYPE;
    ebSwapLog.logicalDataEB = logicalDataEB;
    ebSwapLog.logicalReservedEB = logicalReservedEB;
    if((status = FTL_WriteLogInfo(&flashPageInfo, (UINT8_PTR)&ebSwapLog)) != FTL_ERR_PASS)
    {
        return status;
    }
    return FTL_ERR_PASS;
}
#endif  // #if (FTL_ENABLE_UNUSED_EB_SWAP == FTL_TRUE)


#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

FTL_STATUS FTL_FindSuperSysEB(FTL_DEV devID)
{

    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    UINT16 eBlockCount = 0;                            /*2*/
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0} };       /*11*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    UINT32 key = EMPTY_DWORD;                          /*4*/
    UINT16 countEB = EMPTY_WORD;                       /*2*/
    UINT16 phyEBlockAddrOld = EMPTY_WORD;              /*2*/
    SYS_EBLOCK_INFO_PTR sysTempPtr = NULL;             /*4*/
    UINT32 prevKey = EMPTY_DWORD;                      /*4*/
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 phyEBlockAddrTmp = EMPTY_WORD;              /*2*/
    UINT16 logicalEBNumTmp = EMPTY_WORD;               /*2*/
    UINT8 swap = FTL_FALSE;                            /*1*/
    UINT16 logEBlockAddrOld = EMPTY_WORD;              /*2*/

    flashPage.devID = devID;
    for(eBlockCount = SUPER_SYS_START_EBLOCKS; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
    {
       phyEBlockAddr = GetPhysicalEBlockAddr(devID, eBlockCount);
       flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
       flashPage.vPage.pageOffset = 0;

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       flashPage.byteCount = sizeof(sysEBlockInfo);
       flash_status = FLASH_RamPageReadMetaData(&flashPage, (UINT8_PTR)(&sysEBlockInfo));

       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       flashPage.byteCount = SECTOR_SIZE;
       flash_status = FLASH_RamPageReadDataBlock(&flashPage, &pseudoRPB[devID][0]);
       MEM_Memcpy((UINT8_PTR)(&sysEBlockInfo), &pseudoRPB[devID][0], SYS_INFO_SIZE);
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       if(flash_status != FLASH_PASS)
       {
          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if(flash_status == FLASH_ECC_FAIL)
          {
             continue;
          }
          #endif

          return FTL_ERR_SUPER_READ_01;
       }
       if((FTL_FALSE == VerifyCheckWord((UINT16_PTR)&sysEBlockInfo.type, SYS_INFO_DATA_WORDS, sysEBlockInfo.checkWord)) &&
              (sysEBlockInfo.oldSysBlock != OLD_SYS_BLOCK_SIGNATURE) && (sysEBlockInfo.phyAddrThisEBlock == phyEBlockAddr))
       {
          if(sysEBlockInfo.type == SYS_EBLOCK_INFO_SUPER)
          {
             // Insert the entry, if fails check if this has higher incNum and insert that instead
             if((status = TABLE_SuperSysEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) == FTL_ERR_FLUSH_NO_ENTRIES)
             {
                swap = FTL_FALSE;
                for(countEB = 0; countEB < NUM_SUPER_SYS_EBLOCKS; countEB++)
                {
                   if((status = TABLE_GetSuperSysEBEntry(devID, countEB, &logicalEBNumTmp, &phyEBlockAddrTmp, &key)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   if(key < sysEBlockInfo.incNumber)
                   {
                      swap = FTL_TRUE;
                   }
                }
                if(swap == FTL_TRUE)
                {
                   // This will clear the smallest entry
                   if((status = TABLE_SuperSysEBGetNext(devID, &logEBlockAddrOld, &phyEBlockAddrOld, NULL)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   // Space is now avaiable, Insert the higher entry
                   if((status = TABLE_SuperSysEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) != FTL_ERR_PASS)
                   {
                      return status;
                   }
                   flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddrOld, 0);
                }
                else
                {
                   flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
                }
                // Mark old flush log eblock
                sysEBlockInfo.oldSysBlock = OLD_SYS_BLOCK_SIGNATURE;
                flashPage.devID = devID;
                flashPage.vPage.pageOffset = (UINT16)((UINT32)(&sysTempPtr->oldSysBlock));
                flashPage.byteCount = OLD_SYS_BLOCK_SIGNATURE_SIZE;
//              flashPage.byteCount = sizeof(sysEBlockInfo.oldSysBlock);
                if((FLASH_RamPageWriteMetaData(&flashPage, (UINT8_PTR)&sysEBlockInfo.oldSysBlock)) != FLASH_PASS)
                {

                   #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   SetBadEBlockStatus(devID, logEBlockAddrOld, FTL_TRUE);
                   // just try to mark bad, even if it fails we move on.
                   FLASH_MarkDefectEBlock(&flashPage);

                   #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
                   return FTL_ERR_SUPER_WRITE_04;
                   #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

                }
             }
             else if(status != FTL_ERR_PASS)
             {
                return status;
             }
             if(sysEBlockInfo.incNumber > GetSuperSysEBCounter(devID))
             {
                SetSuperSysEBCounter(devID, sysEBlockInfo.incNumber);
             }
             continue;
          }
       }
    }
    /* check sanity */
    for(eBlockCount = 0; eBlockCount < NUM_SUPER_SYS_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetSuperSysEBEntry(devID, eBlockCount, &logicalEBNum, &phyEBlockAddr, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key == EMPTY_DWORD)
       {
          break;
       }
       if(eBlockCount > 0)
       {
          if(key > (prevKey + 1))
          {
             if((status = TABLE_SuperSysEBRemove(devID, eBlockCount)) != FTL_ERR_PASS)
             {
                return status;
             }
             SetSuperSysEBCounter(devID, prevKey);
          }
       }
       prevKey = key;
    }
    return FTL_ERR_PASS;
}


//-------------------------------
FTL_STATUS GetSuperSysInfoLogs(FTL_DEV devID, UINT16_PTR storePhySysEB, UINT8_PTR checkSuperPF)
{
    FTL_STATUS status        = FTL_ERR_PASS;           /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    UINT16 entryIndex        = 0x0;                    /*2*/
    UINT16 byteOffset        = 0x0;                    /*2*/
    UINT16 logicalAddr       = 0x0;                    /*2*/
    UINT16 phyEBAddr         = 0x0;                    /*2*/
    UINT16 eBlockNum         = 0x0;                    /*2*/
    UINT16 logEntry[LOG_ENTRY_SIZE/2];                 /*16*/
    UINT16 logType           = EMPTY_WORD;             /*2*/
    UINT32 latestIncNumber   = 0x0;                    /*4*/
    UINT16 left              = 0x0;                    /*2*/
    UINT16 right             = 0x0;                    /*2*/
    UINT16 mid               = 0x0;                    /*2*/
    UINT16 tempEntryIndex   = 0x0;                     /*2*/

    #if DEBUG_EARLY_BAD_ENTRY
    UINT16 badEntry           = FTL_FALSE;             /*2*/
    #endif  // #if DEBUG_EARLY_BAD_ENTRY
     
    FLASH_PAGE_INFO   flashPageInfo = {0, 0, {0, 0}};  /*11*/
    SYS_EBLOCK_INFO   sysEBlockInfo;                   /*16*/
    SUPER_SYS_INFO    superSysInfo;                    /*16*/

    UINT16 storeLog[MAX_NUM_SYS_EBLOCKS];
    UINT16 count = 0;                                  /*2*/
    UINT16 tempCount = 0;                              /*2*/ 
    UINT8  endflag = FTL_FALSE;                        /*1*/
    UINT8 checkFlag =FTL_FALSE;
    SUPER_SYS_INFO_PTR getLogType;                     /*16*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT16 packedCount = 0;                            /*2*/
    UINT8 firstFlag = FTL_FALSE;                       /*1*/
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

    for(count = 0; count < MAX_NUM_SYS_EBLOCKS; count++)
    {
       storeLog[count] = EMPTY_WORD;
    }
    count = 0;

    for(eBlockNum = 0; eBlockNum < NUM_SUPER_SYS_EBLOCKS; eBlockNum++)
    {
       (*checkSuperPF) = FTL_FALSE;
       endflag = FTL_FALSE;
       if((status = TABLE_GetSuperSysEBEntry(devID, eBlockNum, &logicalAddr, &phyEBAddr, &latestIncNumber)) != FTL_ERR_PASS)
       {
          return status; // trying to excess outside table.
       }
       if((logicalAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (latestIncNumber == EMPTY_DWORD))
       {
          break; // no more entries in table
       }

       flashPageInfo.devID = devID;
       flashPageInfo.vPage.pageOffset = 0;
       flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, 0);

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       flashPageInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
       flash_status = FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)(&sysEBlockInfo));

       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       flashPageInfo.byteCount = SECTOR_SIZE;
       flash_status = FLASH_RamPageReadDataBlock(&flashPageInfo, &pseudoRPB[devID][0]);
       MEM_Memcpy((UINT8_PTR)(&sysEBlockInfo), &pseudoRPB[devID][0], SYS_INFO_SIZE);
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       if(flash_status != FLASH_PASS)
       {
          return FTL_ERR_FLASH_READ_05;
       }
       // Only if this valid Trans block
       if((FTL_FALSE == VerifyCheckWord((UINT16_PTR)&sysEBlockInfo.type, 
          SYS_INFO_DATA_WORDS, sysEBlockInfo.checkWord)) &&
          (sysEBlockInfo.oldSysBlock != OLD_SYS_BLOCK_SIGNATURE) && 
          (sysEBlockInfo.type == SYS_EBLOCK_INFO_SUPER))
       {
             
          #if DEBUG_EARLY_BAD_ENTRY
          badEntry = FTL_FALSE;
          #endif  // #if DEBUG_EARLY_BAD_ENTRY

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          (*checkSuperPF) = FTL_TRUE;
          #endif

          // Binary Search
          right = NUM_LOG_ENTRIES_PER_EBLOCK;
          left = 1;
          tempEntryIndex = 1;
          while(left <= right) {
             checkFlag = FTL_FALSE;
             tempEntryIndex = (left + right) / 2; /* calc of middle key */
             mid = tempEntryIndex;
             byteOffset = tempEntryIndex * LOG_ENTRY_DELTA;
             flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, tempEntryIndex);
             flashPageInfo.vPage.pageOffset = byteOffset % VIRTUAL_PAGE_SIZE;

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             flashPageInfo.byteCount = sizeof(logEntry);
             flash_status = FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)&logEntry[0]);

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             flashPageInfo.byteCount = SECTOR_SIZE;
             flash_status = FLASH_RamPageReadDataBlock(&flashPageInfo, &pseudoRPB[devID][0]);
             MEM_Memcpy( (UINT8_PTR)&logEntry[0], &pseudoRPB[devID][0],LOG_ENTRY_SIZE);
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flash_status != FLASH_PASS)
             {
                 #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                 return FTL_ERR_SUPER_READ_04;
                 #endif
             }

             if(FLASH_CheckEmpty((UINT8_PTR)&logEntry[0], LOG_ENTRY_SIZE) != FLASH_PASS)
             {
                left = mid + 1; /* adjustment of left(start) key */
             }else{
                right = mid - 1; /* adjustment of right(end) key */
             }

          }
          
          if(0 == tempEntryIndex)
          {
             return FTL_ERR_FAIL;
          }

          MEM_Memcpy((UINT8_PTR)&superSysInfo, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

          if(SYS_EBLOCK_INFO_CHANGED == superSysInfo.type)
          {
             entryIndex = tempEntryIndex;
          }else{
             do {
                if(1 == tempEntryIndex)
                {
                   break;
                }
                tempEntryIndex--;
                byteOffset = tempEntryIndex * LOG_ENTRY_DELTA;
                flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, tempEntryIndex);
                flashPageInfo.vPage.pageOffset = byteOffset % VIRTUAL_PAGE_SIZE;

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                flashPageInfo.byteCount = sizeof(logEntry);
                flash_status = FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)&logEntry[0]);

                #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                flashPageInfo.byteCount = SECTOR_SIZE;
                flash_status = FLASH_RamPageReadDataBlock(&flashPageInfo, &pseudoRPB[devID][0]);
                MEM_Memcpy( (UINT8_PTR)&logEntry[0], &pseudoRPB[devID][0],LOG_ENTRY_SIZE);
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                MEM_Memcpy((UINT8_PTR)&superSysInfo, (UINT8_PTR)&logEntry[0], sizeof(logEntry));

             }while(SYS_EBLOCK_INFO_CHANGED != superSysInfo.type);

             entryIndex = tempEntryIndex;
          }

          // go through all the entries in the block
          for(entryIndex = tempEntryIndex; entryIndex < NUM_LOG_ENTRIES_PER_EBLOCK; entryIndex++)
          {

             byteOffset = entryIndex * LOG_ENTRY_DELTA;
             flashPageInfo.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBAddr, entryIndex);
             flashPageInfo.vPage.pageOffset = byteOffset % VIRTUAL_PAGE_SIZE;

             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
             flashPageInfo.byteCount = sizeof(logEntry);
             flash_status = FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)&logEntry[0]);

             #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             flashPageInfo.byteCount = SECTOR_SIZE;
             flash_status = FLASH_RamPageReadDataBlock(&flashPageInfo, &pseudoRPB[devID][0]);
             MEM_Memcpy( (UINT8_PTR)&logEntry[0], &pseudoRPB[devID][0],LOG_ENTRY_SIZE);
             firstFlag = FTL_TRUE;

             #if DEBUG_EARLY_BAD_ENTRY
             badEntry = FTL_FALSE;
             #endif  // #if DEBUG_EARLY_BAD_ENTRY
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             if(flash_status != FLASH_PASS)
             {

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
                // power failure check...
                if(flash_status == FLASH_ECC_FAIL)
                {
                   (*checkSuperPF) = FTL_FALSE;
                   break;
                }
                else
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

                {
                   return FTL_ERR_SUPER_READ_05;
                }
             }


             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             for(packedCount = 0;packedCount < SECTOR_SIZE; packedCount += LOG_ENTRY_SIZE)
             {
                if(FTL_FALSE == firstFlag)
                {
                   MEM_Memcpy( (UINT8_PTR)&logEntry[0], &pseudoRPB[devID][packedCount],LOG_ENTRY_SIZE);
                }else{
                   firstFlag = FTL_FALSE;
                }
             #endif


             #if DEBUG_EARLY_BAD_ENTRY
             if (FTL_TRUE == badEntry)
             {
                if (EMPTY_WORD != logEntry[LOG_ENTRY_DATA_START])
                {
                   DBG_Printf("GetSuperSysInfoLogs: Error: Bad Log Entry\n", 0, 0); 
                   DBG_Printf("   detected before end of Log, entryIndex = %d\n", entryIndex - 1, 0);
                }
                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                else{
                   (*checkSuperPF) = FTL_FALSE;  
                }
                #endif
                break;
             }
             #endif  // #if DEBUG_EARLY_BAD_ENTRY

             if (VerifyCheckWord(&logEntry[LOG_ENTRY_DATA_START], 
                LOG_ENTRY_DATA_WORDS, logEntry[LOG_ENTRY_CHECK_WORD]))
             {
                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
                // Primary Copy is bad; Check Redundant Copy
                flashPageInfo.vPage.pageOffset += LOG_ENTRY_SIZE;
                if((FLASH_RamPageReadMetaData(&flashPageInfo, (UINT8_PTR)&logEntry[0])) != FLASH_PASS)
                {
                   return FTL_ERR_SUPER_READ_02;
                }
                flashPageInfo.vPage.pageOffset -= LOG_ENTRY_SIZE;
                if (VerifyCheckWord(&logEntry[LOG_ENTRY_DATA_START], 
                   LOG_ENTRY_DATA_WORDS, logEntry[LOG_ENTRY_CHECK_WORD]))
                #endif  // #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

                {
                   // Log Entry is bad

                   #if DEBUG_EARLY_BAD_ENTRY     
                   badEntry = FTL_TRUE;
                   // Get one more entry
                   continue;

                   #else  // #if DEBUG_EARLY_BAD_ENTRY
                   // Don't check any more entries
                   break;
                   #endif  // #else  // #if DEBUG_EARLY_BAD_ENTRY

                }

                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)

                #if DEBUG_REDUNDANT_MESG
                else
                {
                   DBG_Printf("GetSuperSysInfoLogs: Use Redundant Log Entry: %d\n", entryIndex, 0);
                }
                #endif  // #if DEBUG_REDUNDANT_MESG

                #endif  // #if(FTL_REDUNDANT_LOG_ENTRIES == FTL_TRUE)
                #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

             }
             getLogType = (SUPER_SYS_INFO_PTR)logEntry;
             logType = getLogType->type; // get LogType
             if((UINT8)logType == EMPTY_BYTE)
             {
                #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
                (*checkSuperPF) = FTL_FALSE;
                #endif
                break;
             }

             switch(logType)
             {
                // ........SuperSysInfo...........
                case SYS_EBLOCK_INFO_SYSEB:
                {
                   endflag = FTL_FALSE;

                   MEM_Memcpy((UINT8_PTR)&superSysInfo, (UINT8_PTR)&logEntry[0], sizeof(logEntry));
                   if(entryIndex == superSysInfo.EntryNumThisIndex)
                   {
                      for(tempCount = 0; tempCount < NUM_SYS_EB_ENTRY; tempCount++)
                      {
                         if(EMPTY_WORD != superSysInfo.PhyEBNum[tempCount])
                         {
                            storeLog[count] = superSysInfo.PhyEBNum[tempCount];
                            count++;
                         }
                      }
                   }
                   if(count > MAX_NUM_SYS_EBLOCKS)
                   {
                      return FTL_ERR_FAIL;
                   }
                   if(0 == superSysInfo.decNumber)
                   {
                      SuperEBInfo[devID].storeFreePage[eBlockNum] = entryIndex;
                      endflag = FTL_TRUE;
                   }

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("SYS_EBLOCK_INFO_SYSEB\n", 0, 0);
                   for(tempCount = 0; tempCount < NUM_SYS_EB_ENTRY; tempCount++)
                   {
                      DBG_Printf("superSysInfo.PhyEBNum[%d] = ", tempCount, 0);
                      DBG_Printf("0x%x\n", superSysInfo.PhyEBNum[tempCount], 0);
                   }
                   DBG_Printf("superSysInfo.decNumber = %d\n", superSysInfo.decNumber, 0);
                   DBG_Printf("superSysInfo.EntryNumThisIndex = %d\n", superSysInfo.EntryNumThisIndex, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   break;

                }
                case SYS_EBLOCK_INFO_CHANGED:
                {
                   MEM_Memcpy((UINT8_PTR)&superSysInfo, (UINT8_PTR)&logEntry[0], sizeof(logEntry));
                   if(entryIndex == superSysInfo.EntryNumThisIndex)
                   {
                      for(tempCount = 0; tempCount < NUM_SYS_EB_ENTRY; tempCount++)
                      {
                         if(EMPTY_WORD != superSysInfo.PhyEBNum[tempCount])
                         {
                            break;
                         }
                      }
                      if(NUM_SYS_EB_ENTRY == tempCount)
                      {
                         for (tempCount = 0; tempCount < MAX_NUM_SYS_EBLOCKS; tempCount++)
                         {
                            storeLog[tempCount] = EMPTY_WORD;
                         }
                         count = 0;
                         endflag = FTL_FALSE;
                      }
                   }

                   #if DEBUG_LOG_ENTRIES
                   DBG_Printf("SYS_EBLOCK_INFO_CHANGED\n", 0, 0);
                   for(tempCount = 0; tempCount < NUM_SYS_EB_ENTRY; tempCount++)
                   {
                      DBG_Printf("superSysInfo.PhyEBNum[%d] = ", tempCount, 0);
                      DBG_Printf("0x%x\n", superSysInfo.PhyEBNum[tempCount], 0);
                   }
                   DBG_Printf("superSysInfo.decNumber = %d\n", superSysInfo.decNumber, 0);
                   DBG_Printf("superSysInfo.EntryNumThisIndex = %d\n", superSysInfo.EntryNumThisIndex, 0);
                   #endif  // #if DEBUG_LOG_ENTRIES

                   break;

                }
                    
                default:
                {
                   break;
                }
             }  // switch
             #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
             }
             #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          } // entryIndex loop
       } // valid trans block check
    } // eBlockNum loop

    if(FTL_FALSE == endflag)
    {
       (*checkSuperPF) = FTL_TRUE;
       return FTL_ERR_FAIL;
    }

    for(tempCount = 0; tempCount < MAX_NUM_SYS_EBLOCKS; tempCount++)
    {
       storePhySysEB[tempCount] = storeLog[tempCount];
    }

    return FTL_ERR_PASS;
}

FTL_STATUS SetSysEBRamTable(FTL_DEV devID, UINT16_PTR storeSysEB, UINT16_PTR formatCount)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    UINT16 eBlockCount = 0;                            /*2*/
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0} };       /*11*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    UINT32 key = EMPTY_DWORD;                          /*4*/
    UINT32 prevKey = EMPTY_DWORD;                      /*4*/
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/

    flashPage.devID = devID;
    for(eBlockCount = 0; eBlockCount < MAX_NUM_SYS_EBLOCKS; eBlockCount++)
    {
       if(EMPTY_WORD == storeSysEB[eBlockCount])
       {
          break;
       }
       phyEBlockAddr = storeSysEB[eBlockCount];
       flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
       flashPage.vPage.pageOffset = 0;

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       flashPage.byteCount = sizeof(sysEBlockInfo);
       flash_status = FLASH_RamPageReadMetaData(&flashPage, (UINT8_PTR)(&sysEBlockInfo));

       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       flashPage.byteCount = SECTOR_SIZE;
       flash_status = FLASH_RamPageReadDataBlock(&flashPage, &pseudoRPB[devID][0]);
       MEM_Memcpy((UINT8_PTR)(&sysEBlockInfo), &pseudoRPB[devID][0], SYS_INFO_SIZE);
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       if(flash_status != FLASH_PASS)
       {

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          if(flash_status == FLASH_ECC_FAIL)
          {
             continue;
          }
          #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

          return FTL_ERR_SUPER_READ_03;
       }
       if((FTL_FALSE == VerifyCheckWord((UINT16_PTR)&sysEBlockInfo.type, SYS_INFO_DATA_WORDS, sysEBlockInfo.checkWord)) &&
              (sysEBlockInfo.oldSysBlock != OLD_SYS_BLOCK_SIGNATURE) && (sysEBlockInfo.phyAddrThisEBlock == phyEBlockAddr))
       {
          if(sysEBlockInfo.type == SYS_EBLOCK_INFO_FLUSH)
          {
             // Insert the entry, if fails check if this has higher incNum and insert that instead
             if((status = TABLE_FlushEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) == FTL_ERR_FLUSH_NO_ENTRIES)
             {
                return FTL_ERR_SUPER_FLUSH_NO_ROOM;
             }
             else if(status != FTL_ERR_PASS)
             {
                return status;
             }
             if(sysEBlockInfo.incNumber > GetFlushEBCounter(devID))
             {
                SetFlushLogEBCounter(devID, sysEBlockInfo.incNumber);
             }
             (*formatCount)++;
             continue;
          }
          if(sysEBlockInfo.type == SYS_EBLOCK_INFO_LOG)
          {
             if((status = TABLE_TransLogEBInsert(devID, eBlockCount, phyEBlockAddr, sysEBlockInfo.incNumber)) == FTL_ERR_LOG_INSERT)
             {
                return FTL_ERR_SUPER_LOG_NO_ROOM_01;
             }
             else if(status != FTL_ERR_PASS)
             {
                return status;
             }
             if(sysEBlockInfo.incNumber > GetTransLogEBCounter(devID))
             {
                SetTransLogEBCounter(devID, sysEBlockInfo.incNumber);
             }
             (*formatCount)++;
             continue;
          }
       }
    }
    /* check sanity */
    for(eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetFlushLogEntry(devID, eBlockCount, &logicalEBNum, &phyEBlockAddr, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key == EMPTY_DWORD)
       {
          break;
       }
       if(eBlockCount > 0)
       {
          if(key > (prevKey + 1))
          {
             if((status = TABLE_FlushEBRemove(devID, eBlockCount)) != FTL_ERR_PASS)
             {
                return status;
             }
             SetFlushLogEBCounter(devID, prevKey);
          }
       }
       prevKey = key;
    }
    return FTL_ERR_PASS;
}

FTL_STATUS FTL_CheckForSuperSysEBLogSpace(FTL_DEV devID, UINT8 mode)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 latestIncNumber = EMPTY_DWORD;              /*4*/
    UINT16 logLogicalEBNum = EMPTY_WORD;               /*2*/
    UINT16 logPhyEBAddr = EMPTY_WORD;                  /*2*/
    UINT16 entryIndex = 0;                             /*2*/
    UINT16 numLogEntries = 0;                          /*2*/
    UINT16 total = 0;                                  /*2*/
    UINT16 odd = 0;                                    /*2*/
    UINT16 ebPhyAddr[MAX_NUM_SYS_EBLOCKS];
    UINT16 ebCount = 0;                                /*2*/

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;                          /*2*/
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    if(SYS_EBLOCK_INFO_CHANGED == mode)
    {
       if(FTL_TRUE == SuperEBInfo[devID].checkChanged)
       {
          return FTL_ERR_PASS; // If changed, skip operation.
       }
    }else if (SYS_EBLOCK_INFO_SYSEB == mode){
       if(FTL_FALSE == SuperEBInfo[devID].checkChanged)
       {
          return FTL_ERR_PASS; // If not changed, skip operation.
       }
    }else{
       DBG_Printf("FTL_CheckForSuperSysEBLogSpace: The mode is a paramter error\n", 0, 0);
       return FTL_ERR_SUPER_PARAM_03;
    }


    latestIncNumber = GetSuperSysEBCounter(devID);
    status = TABLE_SuperSysEBGetLatest(devID, &logLogicalEBNum, &logPhyEBAddr, latestIncNumber);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }

    // Get free index
    entryIndex = GetFreePageIndex(devID, logLogicalEBNum);

    if(SYS_EBLOCK_INFO_CHANGED == mode)
    {
       numLogEntries = 1;
    }else if(SYS_EBLOCK_INFO_SYSEB == mode){

       // Get Physical Address of SystemEB/
       TABLE_GetPhySysEB(devID, &ebCount, ebPhyAddr);
       total = ebCount / NUM_SYS_EB_ENTRY;
       odd = ebCount % NUM_SYS_EB_ENTRY;
       if(0 < odd)
       {
          total++;
       }
       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       numLogEntries = total + 1; // add " + 1" for next SYS_EBLOCK_INFO_CHANGED
       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       numLogEntries = ((total * LOG_ENTRY_SIZE) / LOG_ENTRY_DELTA);
       odd  = ((total * LOG_ENTRY_SIZE) % LOG_ENTRY_DELTA);
       if(0 < odd)
       {
          numLogEntries++;
       }
       numLogEntries = numLogEntries + 1;  // add " + 1" for next SYS_EBLOCK_INFO_CHANGED
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    }

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    if ((entryIndex + numLogEntries) > NUM_LOG_ENTRIES_PER_EBLOCK || GetBadEBlockStatus(devID, logLogicalEBNum))
    #else
    if ((entryIndex + numLogEntries) > NUM_LOG_ENTRIES_PER_EBLOCK)
    #endif
    {
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
       {
          if((status = CreateNextSuperSystemEBlockOp(devID)) == FTL_ERR_PASS)
          {
             break;
          }
          if((FTL_ERR_SUPER_LOG_NEW_EBLOCK_FAIL_01 != status) && (FTL_ERR_SUPER_LOG_NEW_EBLOCK_FAIL_02 != status))
          {
             return status;
          }
          sanityCounter++;
       }
       if(sanityCounter >= MAX_BAD_BLOCK_SANITY_TRIES)
       {
          return status;
       }
       #else
       if((status = CreateNextSuperSystemEBlockOp(devID)) != FTL_ERR_PASS)
       {
          return status;
       }       
       #endif
    }
    return status;
}


//--------------------------------------
FTL_STATUS FTL_CreateSuperSysEBLog(FTL_DEV devID, UINT8 mode)
{
    SUPER_SYS_INFO ebSuperLog;                        /*16*/
    FLASH_PAGE_INFO flashPageInfo = {0, 0, {0, 0}};    /*11*/
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 count = 0;                                  /*2*/
    UINT16 entryCount = 0;                             /*2*/
    UINT16 ebCount = 0;                                /*2*/
    UINT16 ebPhyAddr[MAX_NUM_SYS_EBLOCKS];
    UINT16 total = 0;                                  /*2*/
    UINT16 odd = 0;                                    /*2*/  
    UINT16 index = 0;                                  /*2*/
    UINT16 entryIndex = 0;                             /*2*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    FTL_STATUS ram_status = FTL_ERR_PASS;               /*4*/
    UINT32 latestIncNumber = EMPTY_DWORD;              /*4*/
    UINT16 logLogicalEBNum = EMPTY_WORD;               /*2*/
    UINT16 logPhyEBAddr = EMPTY_WORD;                  /*2*/
    #endif

    if(SYS_EBLOCK_INFO_CHANGED == mode)
    {
       if(FTL_TRUE == SuperEBInfo[devID].checkChanged)
       {
          return FTL_ERR_PASS; // If changed, skip operation.
       }
    }else if (SYS_EBLOCK_INFO_SYSEB == mode){
       if(FTL_FALSE == SuperEBInfo[devID].checkChanged)
       {
          return FTL_ERR_PASS; // If not changed, skip operation.
       }
    }else{
       DBG_Printf("CreateSuperSysEBLog: The mode is a paramter error\n", 0, 0);
       return FTL_ERR_SUPER_PARAM_01;
    }

    if(SYS_EBLOCK_INFO_CHANGED == mode)
    {
       total = 1;
    }else if(SYS_EBLOCK_INFO_SYSEB == mode){
       // Get Physical Address of SystemEB/
       TABLE_GetPhySysEB(devID, &ebCount, ebPhyAddr);
       total = ebCount / NUM_SYS_EB_ENTRY;
       odd = ebCount % NUM_SYS_EB_ENTRY;
       if(1 <= odd)
       {
          total++;
       }
       index = 0;
    }

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    InitPackedSuperInfo();
    if((status = GetNextSuperSysEBEntryLocation(devID, &flashPageInfo, &entryIndex)) != FTL_ERR_PASS)
    {
       return status;
    }
    ebSuperLog.EntryNumThisIndex = entryIndex;
    #endif

    for(count = 0; count < total; count++)
    {
       if(SYS_EBLOCK_INFO_CHANGED == mode)
       {
          ebSuperLog.type = SYS_EBLOCK_INFO_CHANGED;
          ebSuperLog.decNumber = 0;
          for (entryCount = 0; entryCount < NUM_SYS_EB_ENTRY; entryCount++)
          {
             ebSuperLog.PhyEBNum[entryCount] = EMPTY_WORD;
          }

       }else if(SYS_EBLOCK_INFO_SYSEB == mode){
          ebSuperLog.type = SYS_EBLOCK_INFO_SYSEB;
          ebSuperLog.decNumber = (UINT8)((total - count) - 1);
          for (entryCount = 0; entryCount < NUM_SYS_EB_ENTRY; entryCount++)
          {
             if(index < ebCount) 
             {
                ebSuperLog.PhyEBNum[entryCount] = ebPhyAddr[index];
                index++;
             }else{
                ebSuperLog.PhyEBNum[entryCount] = EMPTY_WORD;
             }
          }
       }
       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       if((status = GetNextSuperSysEBEntryLocation(devID, &flashPageInfo, &entryIndex)) != FTL_ERR_PASS)
       {
          return status;
       }

       ebSuperLog.EntryNumThisIndex = entryIndex;
       if((status = FTL_WriteSuperInfo(&flashPageInfo, (UINT8_PTR)&ebSuperLog)) != FTL_ERR_PASS)
       {
          return status;
       }
       #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       CopyPackedSuperInfo(count, (UINT8_PTR)&ebSuperLog);
       #endif
    }

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    if((status = WritePackedSuperInfo(&flashPageInfo)) != FTL_ERR_PASS)
    {
       if(status == FTL_ERR_FAIL)
       {
          return FTL_ERR_SUPER_PARAM_02;
       }
       latestIncNumber = GetSuperSysEBCounter(devID);
       if((ram_status = TABLE_SuperSysEBGetLatest(devID, &logLogicalEBNum, &logPhyEBAddr, latestIncNumber)) != FTL_ERR_PASS)
       {
          return ram_status;
       }
       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       SetBadEBlockStatus(devID, logLogicalEBNum, FTL_TRUE);
       #endif
       return status;
    }
    #endif

    SuperEBInfo[devID].checkChanged = FTL_TRUE;

    return FTL_ERR_PASS;
}
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)


#if(FTL_UNLINK_GC == FTL_TRUE)
//-----------------------------
FTL_STATUS ProcessUnlinkPageBitMap(FTL_DEV devID, UINT16 logFromEBNum, UINT16 logToEBNum, UINT8_PTR pageBitMap)
{
    UINT16 pageOffset = 0;                             /*2*/
    UINT16 freePageIndex = 0;                          /*2*/
    FREE_BIT_MAP_TYPE bitMap = 0;                      /*1*/

    for(pageOffset = 0; pageOffset < NUM_PAGES_PER_EBLOCK; pageOffset++)
    {
       bitMap = GetBitMapField(&pageBitMap[0], pageOffset, 1);
       if(bitMap == GC_MOVED_PAGE)
       {
          freePageIndex = GetFreePageIndex(devID, logToEBNum);
          UpdatePageTableInfo(devID, logToEBNum, pageOffset, freePageIndex, BLOCK_INFO_VALID_PAGE);
          UpdatePageTableInfo(devID, logFromEBNum, pageOffset, EMPTY_INVALID, BLOCK_INFO_STALE_PAGE);
       }
    }
    return FTL_ERR_PASS;
}

//-----------------------------
FTL_STATUS UpdateRAMTablesUsingUnlinkLogs(FTL_DEV devID, UNLINK_LOG_ENTRY_PTR ptrUnlinkLog)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 fromLogicalEBlock = EMPTY_WORD;             /*2*/
    UINT16 toLogicalEBlock = EMPTY_WORD;               /*2*/
    UINT16 count = 0;                                  /*2*/
    UINT8 pageBitMap[GC_MOVE_BITMAP];

    fromLogicalEBlock = ptrUnlinkLog->partA.fromLogicalEBAddr;
    toLogicalEBlock = ptrUnlinkLog->partA.toLogicalEBAddr;

    if(ptrUnlinkLog->partA.type == UNLINK_LOG_TYPE_A2)
    {
       for(count = 0; count < NUM_UNLINK_TYPE_B; count++)
       {
          MEM_Memcpy(&pageBitMap[count*NUM_ENTRIES_UNLINK_TYPE_B], &ptrUnlinkLog->partB[count].pageMovedBitMap[0], NUM_ENTRIES_UNLINK_TYPE_B);
       }

       if((status = ProcessUnlinkPageBitMap(devID, fromLogicalEBlock, toLogicalEBlock, &pageBitMap[0])) != FTL_ERR_PASS)
       {
          return status;
       }
    }

    if((status = UnlinkChain(devID, fromLogicalEBlock, toLogicalEBlock)) != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
//----------------------------------
FTL_STATUS InitPackedLogs(void)
{
    UINT32 count = 0;                                  /*4*/

    for(count = 0; count < SECTOR_SIZE; count++)
    {
       packedLog[count] = EMPTY_BYTE;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS CopyPackedLogs(UINT16 offset, UINT8_PTR logPtr)
{
    UINT16_PTR logEntry = (UINT16_PTR)logPtr;          /*4*/

    logEntry[LOG_ENTRY_CHECK_WORD] = CalcCheckWord(&logEntry[LOG_ENTRY_DATA_START], LOG_ENTRY_DATA_WORDS);
    MEM_Memcpy(&packedLog[(offset * LOG_ENTRY_SIZE)], logPtr, LOG_ENTRY_SIZE);
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS WritePackedLogs(FLASH_PAGE_INFO_PTR flashPagePtr)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 byteCount = 0;                              /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    byteCount = flashPagePtr->byteCount;
    flashPagePtr->byteCount = SECTOR_SIZE;
    flashStatus = FLASH_RamPageWriteDataBlock(flashPagePtr, &packedLog[0]);
    if(flashStatus != FLASH_PASS)
    {

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(FLASH_PARAM == flashStatus)
       {
          return FTL_ERR_FLASH_WRITE_16;
       }
       if((status = TransLogEBFailure(flashPagePtr, &packedLog[0])) != FTL_ERR_PASS)
       {
          return status;
       }

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return FTL_ERR_FLASH_WRITE_16;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    flashPagePtr->byteCount = byteCount;
    if(writeLogFlag == FTL_FALSE)
    {
       writeLogFlag = FTL_TRUE;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS ReadPackedGCLogs(UINT8_PTR logPtr, GC_LOG_ENTRY_PTR ptrGCLog)
{
    UINT16_PTR logEntry = NULL;                        /*4*/
    UINT16 count = 0;                                  /*2*/

    /*Copy the type B log entries*/
    for(count = 0; count < NUM_GC_TYPE_B; count++)
    {
       logEntry = (UINT16_PTR)&logPtr[(count * LOG_ENTRY_SIZE)];
       if (VerifyCheckWord(&logEntry[LOG_ENTRY_DATA_START], 
          LOG_ENTRY_DATA_WORDS, logEntry[LOG_ENTRY_CHECK_WORD]))
       {
          return FTL_ERR_LOG_CHECK_GC_B;
       }
       MEM_Memcpy((UINT8_PTR)&(ptrGCLog->partB[count]), (UINT8_PTR)&logEntry[0], LOG_ENTRY_SIZE);
    }
    return FTL_ERR_PASS;
}

#if(FTL_UNLINK_GC == FTL_TRUE)
//----------------------------------
FTL_STATUS ReadPackedUnlinkLogs(UINT8_PTR logPtr, UNLINK_LOG_ENTRY_PTR ptrUnlinkLog)
{
    UINT16_PTR logEntry = NULL;                        /*4*/
    UINT16 count = 0;                                  /*2*/

    /*Copy the type B log entries*/
    for(count = 0; count < NUM_UNLINK_TYPE_B; count++)
    {
       logEntry = (UINT16_PTR)&logPtr[((count + 1) * LOG_ENTRY_SIZE)];
       if (VerifyCheckWord(&logEntry[LOG_ENTRY_DATA_START], 
          LOG_ENTRY_DATA_WORDS, logEntry[LOG_ENTRY_CHECK_WORD]))
       {
          return FTL_ERR_LOG_CHECK_UNLINK_B;
       }
       MEM_Memcpy((UINT8_PTR)&(ptrUnlinkLog->partB[count]), (UINT8_PTR)&logEntry[0], LOG_ENTRY_SIZE);
    }
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

//-----------------------------
FTL_STATUS VerifyRamTable(UINT16_PTR tablePtr)
{
    if(VerifyCheckWord(&tablePtr[FLUSH_INFO_TABLE_START], (FLUSH_RAM_TABLE_SIZE/2), tablePtr[FLUSH_INFO_TABLE_CHECK_WORD]) == FTL_TRUE)
    {
       return FTL_ERR_FLUSH_VERIFY_TABLE;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS FTL_WriteSpareInfo(FLASH_PAGE_INFO_PTR flashPagePtr, SPARE_INFO_PTR spareInfoPtr)
{
    UINT16_PTR spareInfo = (UINT16_PTR)spareInfoPtr; /*4*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    spareInfo[SPARE_INFO_CHECK_WORD] = CalcCheckWord(&spareInfo[SPARE_INFO_DATA_START], SPARE_INFO_DATA_WORDS);
    flashStatus = FLASH_RamPageWriteSpareData(flashPagePtr, (UINT8_PTR)spareInfo);
    if(flashStatus != FLASH_PASS)
    {
       if(flashStatus == FLASH_PARAM)
       {
          return FTL_ERR_FAIL;
       }
       return FTL_ERR_FLASH_WRITE_18;
    }
    return FTL_ERR_PASS;
}

//----------------------------------
FTL_STATUS GetSpareInfoSetPPATable(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    FTL_DEV devCount = 0;                              /*1*/
    UINT8 validFlag = FTL_FALSE;                       /*2*/
    UINT16 eBlockCount = 0;                            /*2*/
    UINT16 logicalPageOffset = 0;                      /*2*/
    UINT16 phyPageOffset = 0;                          /*2*/
    UINT16 oldPageOffset = 0;                          /*2*/
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 physicalEBNum = EMPTY_WORD;                 /*2*/
    UINT16 chainFromEBNum = EMPTY_WORD;                /*2*/
    UINT16 spareLogicalEBNum = EMPTY_WORD;             /*2*/
    FLASH_PAGE_INFO pageInfo = {0, 0, {0, 0}};         /*11*/
    SPARE_INFO spareInfo;                              /*16*/
    UINT16 spareBuf[SPARE_INFO_SIZE/2];                /*16*/
    #if (SPANSION_CRC32 == FTL_TRUE)
    UINT32 crc32 = 0;                                  /*4*/
    #endif
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT16 oldFreePageIndex = 0;
    UINT16 newFreePageIndex = 0;
    UINT16 chainToEBNum = EMPTY_WORD;
    UINT16 tempEBlockCount = EMPTY_WORD;
    UINT8 loop = 0;
    UINT8 checkChain = FTL_FALSE;
    UINT8 checkPf = FTL_FALSE;
    gTargetPftEBForNand = EMPTY_WORD;
    #endif
    
    pageInfo.byteCount = SPARE_INFO_SIZE;
    pageInfo.vPage.pageOffset = VIRTUAL_PAGE_SIZE;
    for(devCount = 0; devCount < NUM_DEVICES; devCount++)
    {
       pageInfo.devID = devCount;

       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       // clear dirty
       if(FTL_ERR_PASS != (status = CACHE_CleanAllDirtyIndex(devCount)))
       {
          return status;
       }
       for(loop = 0; loop < 2; loop++)
       {
          if(0 == loop)
          {
             // Mark invalid dirty  
             gCheckPfForNand = FTL_TRUE;
             CACHE_ClearAllPfEBForNAND(devCount);
          }else if(1 == loop){
             // Mark valid dirty
             gCheckPfForNand = FTL_FALSE;
          }
       #endif
       
       for(eBlockCount = 0; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
       {
          validFlag = FTL_FALSE;

          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

          if(loop == 1) // 2nd loop
          {
             checkPf = CACHE_IsPfEBForNAND(devCount, eBlockCount);
             if(FTL_FALSE == checkPf)
             {
                continue;
             }
          }

          if(NUM_DATA_EBLOCKS <= eBlockCount)
          {
             chainFromEBNum = GetChainLogicalEBNum(devCount, eBlockCount);
             if(chainFromEBNum != EMPTY_WORD)
             {
                continue;
             }
          }

          checkChain = FTL_FALSE;
          
          do { // for chain
          
          validFlag = FTL_FALSE;
          chainFromEBNum = EMPTY_WORD;

          if(FTL_TRUE == checkChain){
             eBlockCount = chainToEBNum;
          }

          if (FTL_ERR_PASS != (status = CACHE_LoadEB(devCount, eBlockCount, CACHE_INIT_TYPE)))
          {
             return status;
          }

          if(NUM_DATA_EBLOCKS > eBlockCount)
          {
             chainToEBNum = GetChainLogicalEBNum(devCount, eBlockCount);
          }else{
             chainToEBNum = EMPTY_WORD;
          }
          if(FTL_FALSE == checkChain)
          {
              checkPf = FTL_FALSE;
          }
          if(FTL_FALSE == checkChain)
          {
             if(chainToEBNum != EMPTY_WORD)
             {
                checkChain = FTL_TRUE;
                tempEBlockCount = eBlockCount;
             }else{
                tempEBlockCount = EMPTY_WORD;
             }
          }else if(FTL_TRUE == checkChain){
             newFreePageIndex = GetFreePageIndex(devCount, tempEBlockCount);
             if(newFreePageIndex != oldFreePageIndex)
             {
                checkPf = FTL_TRUE;
             }
             checkChain = FTL_FALSE;
          }

          oldFreePageIndex = GetFreePageIndex(devCount, eBlockCount);

          #endif
          TABLE_ClearFreeBitMap(devCount, eBlockCount);
          TABLE_ClearPPATable(devCount, eBlockCount);

          logicalEBNum = eBlockCount;
          physicalEBNum = GetPhysicalEBlockAddr(devCount, eBlockCount);


          if(FTL_ERR_PASS == TABLE_CheckUsedSysEB(devCount, eBlockCount))
          {
             continue;
          }

          #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
          if(eBlockCount >= NUM_DATA_EBLOCKS)
          {
             chainFromEBNum = GetChainLogicalEBNum(devCount, eBlockCount);
             if(chainFromEBNum != EMPTY_WORD)
             {
                logicalEBNum = chainFromEBNum;
             }
             else
             {
                #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)
                SetEBErased(devCount, eBlockCount, FTL_FALSE);
                #endif  // #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)
                
                #if (CACHE_RAM_BD_MODULE == FTL_TRUE)               
                checkPf = FTL_TRUE;
                #endif

                MarkAllPagesStatus(devCount, eBlockCount, BLOCK_INFO_STALE_PAGE);
                continue;
             }
          }
          #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

          for(phyPageOffset = 0; phyPageOffset < NUM_PAGES_PER_EBLOCK; phyPageOffset++)
          {
             pageInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(physicalEBNum, phyPageOffset);
             pageInfo.vPage.pageOffset = VIRTUAL_PAGE_SIZE;
             pageInfo.byteCount = SPARE_INFO_SIZE;
             if((flash_status = FLASH_RamPageReadSpareData(&pageInfo, (UINT8_PTR)&spareBuf[0])) != FLASH_PASS)
             {
                // power failure check...
                if(flash_status == FLASH_ECC_FAIL)
                {
                   if(validFlag == FTL_FALSE)
                   {
                      validFlag = FTL_TRUE;

                      #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)
                      SetEBErased(devCount, eBlockCount, FTL_FALSE);
                      #endif  // #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)

                   }
                   #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                   checkPf = FTL_TRUE;
                   if(EMPTY_WORD != gTargetPftEBForNand)
                   {
                      return FTL_ERR_CACHE_GET_SPARE_INFO_1;
                   }                   
                   gTargetPftEBForNand = eBlockCount;
                   #endif
                   
                   UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, phyPageOffset, BLOCK_INFO_STALE_PAGE);
                }
                else
                {
                   return FTL_ERR_FLASH_READ_26;
                }
             }
             else
             {
                if(FLASH_CheckEmpty((UINT8_PTR)&spareBuf[0], SPARE_INFO_SIZE) == FLASH_PASS)
                {
                   /* read main area to check empty */
                   pageInfo.byteCount = VIRTUAL_PAGE_SIZE;
                   pageInfo.vPage.pageOffset = 0;
                   if((flash_status = FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devCount][0])) != FLASH_PASS)
                   {
                      // power failure check...
                      if(flash_status == FLASH_ECC_FAIL)
                      {
                         if(validFlag == FTL_FALSE)
                         {
                            validFlag = FTL_TRUE;

                            #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)
                            SetEBErased(devCount, eBlockCount, FTL_FALSE);
                            #endif  // #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)

                         }
                         #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                         checkPf = FTL_TRUE;
                         if(EMPTY_WORD != gTargetPftEBForNand)
                         {
                            return FTL_ERR_CACHE_GET_SPARE_INFO_2;
                         }                      
                         gTargetPftEBForNand = eBlockCount;
                         
                         #endif
                         
                         UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, phyPageOffset, BLOCK_INFO_STALE_PAGE);
                         continue;
                      }
                      else
                      {
                         return FTL_ERR_FLASH_READ_27;
                      }
                   }
                   if(FLASH_CheckEmpty((UINT8_PTR)&pseudoRPB[devCount][0], VIRTUAL_PAGE_SIZE) == FLASH_PASS)
                   {
                      break;
                   }
                   else
                   {
                      if(validFlag == FTL_FALSE)
                      {
                         validFlag = FTL_TRUE;

                         #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)
                         SetEBErased(devCount, eBlockCount, FTL_FALSE);
                         #endif  // #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)

                      }
                      #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                      checkPf = FTL_TRUE;
                      if(EMPTY_WORD != gTargetPftEBForNand)
                      {
                         return FTL_ERR_CACHE_GET_SPARE_INFO_3;
                      }                      
                      gTargetPftEBForNand = eBlockCount;
                      #endif
                      UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, phyPageOffset, BLOCK_INFO_STALE_PAGE);
                   }
                }
                else
                {
                   if(validFlag == FTL_FALSE)
                   {
                      validFlag = FTL_TRUE;

                      #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)
                      SetEBErased(devCount, eBlockCount, FTL_FALSE);
                      #endif  // #if(ENABLE_EB_ERASED_BIT == FTL_TRUE)

                   }
                   if(VerifyCheckWord(&spareBuf[SPARE_INFO_DATA_START], SPARE_INFO_DATA_WORDS, spareBuf[SPARE_INFO_CHECK_WORD]))
                   {
                      UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, phyPageOffset, BLOCK_INFO_STALE_PAGE);
                         #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                         checkPf = FTL_TRUE;
                         if(EMPTY_WORD != gTargetPftEBForNand)
                         {
                            return FTL_ERR_CACHE_GET_SPARE_INFO_4;
                         }                                               
                         gTargetPftEBForNand = eBlockCount;
                         #endif
                   }
                   else
                   {
                      /* read main area to check power failure */
                      pageInfo.byteCount = VIRTUAL_PAGE_SIZE;
                      pageInfo.vPage.pageOffset = 0;
                      if((flash_status = FLASH_RamPageReadDataBlock(&pageInfo, &pseudoRPB[devCount][0])) != FLASH_PASS)
                      {
                         // power failure check...
                         if(flash_status == FLASH_ECC_FAIL)
                         {
                            UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, phyPageOffset, BLOCK_INFO_STALE_PAGE);
                            #if (CACHE_RAM_BD_MODULE == FTL_TRUE)                     
                            checkPf = FTL_TRUE;
                            if(EMPTY_WORD != gTargetPftEBForNand)
                            {
                               return FTL_ERR_CACHE_GET_SPARE_INFO_5;
                            }
                            gTargetPftEBForNand = eBlockCount;
                            #endif
                         }
                         else
                         {
                            return FTL_ERR_FLASH_READ_09;
                         }
                      }
                      else
                      {
                         MEM_Memcpy((UINT8_PTR)&spareInfo, (UINT8_PTR)&spareBuf[0], sizeof(spareBuf));
                         #if (SPANSION_CRC32 == FTL_TRUE)
                         CalcCalculateCRC(&pseudoRPB[devCount][0], &crc32);
                         if(CRC_ERROR == CalcCompareCRC(crc32, spareInfo.crc32))
                         {
                             UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, phyPageOffset, BLOCK_INFO_STALE_PAGE);
                             #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                             checkPf = FTL_TRUE;
                             if(EMPTY_WORD != gTargetPftEBForNand)
                             {
                                return FTL_ERR_CACHE_GET_SPARE_INFO_6;
                             }                                                                                
                             gTargetPftEBForNand = eBlockCount;                             
                             #endif
                             continue;
                         }
                         #endif
                         if((status = GetLogicalEBNum(spareInfo.logicalPageAddr, &spareLogicalEBNum)) != FTL_ERR_PASS)
                         {
                            return status;
                         }
                         if(spareLogicalEBNum == logicalEBNum)
                         {
                            logicalPageOffset = GetIndexFromPhyPage(spareInfo.logicalPageAddr);
                            oldPageOffset = GetPPASlot(devCount, eBlockCount, logicalPageOffset);

                            UpdatePageTableInfo(devCount, eBlockCount, logicalPageOffset, phyPageOffset, BLOCK_INFO_VALID_PAGE);
                            if(oldPageOffset != EMPTY_INVALID)
                            {
                               UpdatePageTableInfo(devCount, eBlockCount, EMPTY_INVALID, oldPageOffset, BLOCK_INFO_STALE_PAGE);
                            }

                            #if(FTL_EBLOCK_CHAINING == FTL_TRUE)
                            if(eBlockCount >= NUM_DATA_EBLOCKS)
                            {
                               if(chainFromEBNum != EMPTY_WORD)
                               {
                                  UpdatePageTableInfo(devCount, chainFromEBNum, logicalPageOffset, CHAIN_INVALID, BLOCK_INFO_STALE_PAGE);
                               }
                               else
                               {
                                  return FTL_ERR_TABLE_SPARE_CHAIN;
                               }
                            }
                            #endif  // #if(FTL_EBLOCK_CHAINING == FTL_TRUE)

                         }
                         else
                         {
                            if(eBlockCount < NUM_DATA_EBLOCKS)
                            {
                               MarkAllPagesStatus(devCount, eBlockCount, BLOCK_INFO_STALE_PAGE);
                               #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
                               checkPf = FTL_TRUE;
                               #endif
                               break;
                            }
                            else
                            {
                               return FTL_ERR_TABLE_SPARE_EB;
                            }
                         }
                      }
                   }
                }
             }
          }
          #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
          }while(FTL_TRUE == checkChain);

          newFreePageIndex = GetFreePageIndex(devCount, eBlockCount);
          if((newFreePageIndex != oldFreePageIndex) || (FTL_TRUE == checkPf))
          {
             if(EMPTY_WORD != chainFromEBNum)
             { 

                if((status = CACHE_SetPfEBForNAND(devCount, chainFromEBNum, FTL_TRUE)) != FTL_ERR_PASS)
                {
                   return status;
                }
                if((status = CACHE_SetPfEBForNAND(devCount, eBlockCount, FTL_TRUE)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }else{
                if((status = CACHE_SetPfEBForNAND(devCount, eBlockCount, FTL_TRUE)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
          }
          if(EMPTY_WORD != tempEBlockCount)
          {
             eBlockCount = tempEBlockCount;
          }
          #endif
          
       }

       #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
       } //for(loop = 0; loop < 2; loop++)
       gCheckPfForNand = FTL_FALSE;
       for(eBlockCount = NUM_DATA_EBLOCKS; eBlockCount < NUM_EBLOCKS_PER_DEVICE; eBlockCount++)
       {
          MarkEBlockMappingTableEntryDirty(devCount, eBlockCount);
       }
       CACHE_ClearAllPfEBForNAND(devCount);
       #endif
    }

    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    gCheckPfForNand = FTL_FALSE;
    #endif

    return FTL_ERR_PASS;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)

//-----------------------------
FTL_STATUS LoadRamTable(FLASH_PAGE_INFO_PTR flashPage, UINT8_PTR ramTablePtr, UINT16 tableOffset, UINT32 devTableSize)
{
    UINT32 dataBytes = 0;                              /*4*/
    UINT8_PTR destPtr = NULL;                          /*4*/

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    dataBytes = devTableSize - (tableOffset * SECTOR_SIZE);
    // Aligned case
    if(dataBytes >= SECTOR_SIZE)
    {
       destPtr = (UINT8_PTR)((UINT8_PTR)ramTablePtr + (tableOffset * SECTOR_SIZE));
       if((FLASH_RamPageReadDataBlock(flashPage, destPtr)) != FLASH_PASS)
       {
          return FTL_ERR_FLASH_READ_15;
       }
    }
    else    // Not Aligned case
    {
       if((FLASH_RamPageReadDataBlock(flashPage, &pseudoRPB[flashPage->devID][0])) != FLASH_PASS)
       {
          return FTL_ERR_FLASH_READ_16;
       }
       destPtr = (UINT8_PTR)((UINT8_PTR)ramTablePtr + (tableOffset * SECTOR_SIZE));
       MEM_Memcpy(destPtr, &pseudoRPB[flashPage->devID][0], (UINT16)dataBytes);
    }

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    /* ram table is already read to &pseudoRPB[flashPage->devID][0] */
    dataBytes = devTableSize - (tableOffset * FLUSH_RAM_TABLE_SIZE);
    destPtr = (UINT8_PTR)((UINT8_PTR)ramTablePtr + (tableOffset * FLUSH_RAM_TABLE_SIZE));
    // Aligned case
    if(dataBytes >= FLUSH_RAM_TABLE_SIZE)
    {
       MEM_Memcpy(destPtr, &pseudoRPB[flashPage->devID][FLUSH_INFO_SIZE], FLUSH_RAM_TABLE_SIZE);
    }
    else    // Not Aligned case
    {
       MEM_Memcpy(destPtr, &pseudoRPB[flashPage->devID][FLUSH_INFO_SIZE], (UINT16)dataBytes);
    }
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    return FTL_ERR_PASS;
}

//-----------------------
FTL_STATUS GetFlushLoc(FTL_DEV devID, UINT16 phyEBlockAddr, UINT16 freePageIndex, FLASH_PAGE_INFO_PTR flushInfoPtr, FLASH_PAGE_INFO_PTR flushRamTablePtr)
{
    flushRamTablePtr->devID            = devID;
    flushRamTablePtr->vPage.vPageAddr  = CalcFlushRamTablePages(phyEBlockAddr, freePageIndex);
    flushRamTablePtr->vPage.pageOffset = CalcFlushRamTableOffset(freePageIndex);
    flushRamTablePtr->byteCount        = SECTOR_SIZE;

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    flushRamTablePtr->byteCount        = FLUSH_RAM_TABLE_SIZE; 
    #endif
    flushInfoPtr->devID                = devID;
    flushInfoPtr->vPage.vPageAddr      = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, freePageIndex);
    flushInfoPtr->vPage.pageOffset     = (freePageIndex & ((VIRTUAL_PAGE_SIZE/BYTES_PER_CL)-1)) * BYTES_PER_CL;
    flushInfoPtr->byteCount            = sizeof(SYS_EBLOCK_FLUSH_INFO); // read/write word at a time.

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    flushInfoPtr->devID                = flushRamTablePtr->devID;
    flushInfoPtr->vPage.vPageAddr      = flushRamTablePtr->vPage.vPageAddr;
    flushInfoPtr->vPage.pageOffset     = flushRamTablePtr->vPage.pageOffset;
    flushInfoPtr->byteCount            = sizeof(SYS_EBLOCK_FLUSH_INFO); // read/write word at a time.
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    return FTL_ERR_PASS;
}

//---------------------------
FTL_STATUS GetNextFlushEntryLocation(FTL_DEV devID, FLASH_PAGE_INFO_PTR flushInfoPtr, FLASH_PAGE_INFO_PTR flushRamTablePtr, UINT16_PTR logicalBlockNumPtr)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    UINT16 entryIndex = EMPTY_WORD;                    /*2*/
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT32 key = EMPTY_DWORD;                          /*4*/
    UINT16 eBlockCount = 0;                            /*2*/

    for(eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetFlushLogEntry(devID, eBlockCount, &logicalEBNum, &phyEBlockAddr, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key != EMPTY_DWORD)
       {
          entryIndex = GetFreePageIndex(devID, logicalEBNum);
          if(entryIndex < MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK)
          {
             break;
          }
       }
    }
    if(entryIndex >= MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK)
    {
       return FTL_ERR_FLUSH_NEXT_ENTRY;
    }
    *logicalBlockNumPtr = logicalEBNum;
    if((status = GetFlushLoc(devID, phyEBlockAddr, entryIndex, flushInfoPtr, flushRamTablePtr)) != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}

//--------------------------
FTL_STATUS CreateNextFlushEntryLocation(FTL_DEV devID, UINT16 logicalBlockNum)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 phyEBlockAddr  = 0x0;                       /*2*/
    UINT16 tempCount      = 0x0;                       /*2*/
    UINT32 latestIncNumber= 0x0;                       /*4*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    FLASH_PAGE_INFO flushInfo = {0, 0, {0, 0}};        /*11*/

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    if((FTL_FALSE == SuperEBInfo[devID].checkLost) && (FTL_FALSE == SuperEBInfo[devID].checkSuperPF) && (FTL_FALSE == SuperEBInfo[devID].checkSysPF))
    {
       if((status = FTL_CreateSuperSysEBLog(devID, SYS_EBLOCK_INFO_CHANGED)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    eraseStatus = GetEBErased(devID, logicalBlockNum);

    #if DEBUG_PRE_ERASED
    if(FTL_TRUE == eraseStatus)
    {
       DBG_Printf("CreateNextFlushEntryLocation: EBlock 0x%X is already erased\n", logicalBlockNum, 0);
    }
    #endif  // #if DEBUG_PRE_ERASED
    if(FTL_FALSE == eraseStatus)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    {
       // Erase it before writting to make sure its empty...
       if((status = FTL_EraseOp(devID, logicalBlockNum)) != FTL_ERR_PASS)
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(status == FTL_ERR_FAIL)
          {
             return status;
          }
          SetBadEBlockStatus(devID, logicalBlockNum, FTL_TRUE);
          flushInfo.devID = devID;
          phyEBlockAddr = GetPhysicalEBlockAddr(devID, logicalBlockNum);
          flushInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyEBlockAddr, 0);
          flushInfo.vPage.pageOffset = 0;
          flushInfo.byteCount = 0;
          if(FLASH_MarkDefectEBlock(&flushInfo) != FLASH_PASS)
          {
             // do nothing, just try to mark bad, even if it fails we move on.
          }
          return FTL_ERR_FLUSH_NEXT_EBLOCK_FAIL;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return status;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       }
    }
    flushInfo.devID = devID;
    flushInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
    flushInfo.vPage.pageOffset = 0;
    phyEBlockAddr = GetPhysicalEBlockAddr(devID, logicalBlockNum);
    flushInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyEBlockAddr, 0);
    latestIncNumber = GetFlushEBCounter(devID) + 1;
    sysEBlockInfo.incNumber = latestIncNumber;
    sysEBlockInfo.phyAddrThisEBlock = phyEBlockAddr;
    sysEBlockInfo.type = SYS_EBLOCK_INFO_FLUSH;
    sysEBlockInfo.checkVersion = EMPTY_WORD;
    sysEBlockInfo.oldSysBlock = EMPTY_WORD;
    sysEBlockInfo.fullFlushSig = EMPTY_WORD;
    for(tempCount = 0; tempCount < NUM_SYS_RESERVED_BYTES; tempCount++)
    {
       sysEBlockInfo.reserved[tempCount] = EMPTY_BYTE;
    }
    if((status = FTL_WriteSysEBlockInfo(&flushInfo, &sysEBlockInfo)) != FTL_ERR_PASS)
    {

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(status == FTL_ERR_FAIL)
       {
          return status;
       }
       SetBadEBlockStatus(devID, logicalBlockNum, FTL_TRUE);
       if(FLASH_MarkDefectEBlock(&flushInfo) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on.
       }
       return FTL_ERR_FLUSH_NEXT_EBLOCK_FAIL;

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return status;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, logicalBlockNum, FTL_FALSE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetFlushLogEBCounter(devID, latestIncNumber);
    SetGCOrFreePageNum(devID, logicalBlockNum, 1);    
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
    /*what if this is full*/
    if((status = TABLE_FlushEBInsert(devID, logicalBlockNum, phyEBlockAddr, latestIncNumber)) != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}

//------------------------------
FTL_STATUS GetNextLogEntryLocation(FTL_DEV devID, FLASH_PAGE_INFO_PTR pageInfoPtr)  /*  1,  4*/
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 latestIncNumber = EMPTY_DWORD;              /*4*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    UINT16 logicalBlockNum = EMPTY_WORD;               /*2*/
    UINT16 entryIndex = EMPTY_WORD;                    /*2*/
    UINT16 byteOffset = 0;                             /*2*/

    // Initialize variables
    pageInfoPtr->devID = devID;
    pageInfoPtr->byteCount = LOG_ENTRY_SIZE;
    latestIncNumber = GetTransLogEBCounter(devID);
    status = TABLE_TransLogEBGetLatest(devID, &logicalBlockNum, &phyEBlockAddr, latestIncNumber);
    if (FTL_ERR_PASS != status)
    {
       return status;
    }
    // Get offset to free entry
    entryIndex = GetFreePageIndex(devID, logicalBlockNum);
    if (entryIndex < NUM_LOG_ENTRIES_PER_EBLOCK)
    {
       // Latest EBlock has room for more
       byteOffset = entryIndex * LOG_ENTRY_DELTA;
       pageInfoPtr->vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, entryIndex);
       pageInfoPtr->vPage.pageOffset = byteOffset % VIRTUAL_PAGE_SIZE;
       SetGCOrFreePageNum(devID, logicalBlockNum, entryIndex + 1);
       MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       SetTransLogEBNumBadBlockInfo(logicalBlockNum);
       #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    else
    {
       return FTL_ERR_CANNOT_FIND_NEXT_ENTRY;
    }

    return FTL_ERR_PASS;
}

//--------------------------
FTL_STATUS CreateNextLogEntryLocation(FTL_DEV devID, UINT16 logicalBlockNum)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 phyEBlockAddr  = 0x0;                       /*2*/
    UINT16 tempCount      = 0x0;                       /*2*/
    UINT32 latestIncNumber= 0x0;                       /*4*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/
    FLASH_PAGE_INFO flushInfo = {0, 0, {0, 0}};        /*11*/

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    UINT8 eraseStatus = FTL_FALSE;                     /*1*/
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    if((status = FTL_CreateSuperSysEBLog(devID, SYS_EBLOCK_INFO_CHANGED)) != FTL_ERR_PASS)
    {
       return status;
    }
    #endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    eraseStatus = GetEBErased(devID, logicalBlockNum);

    #if DEBUG_PRE_ERASED
    if(FTL_TRUE == eraseStatus)
    {
       DBG_Printf("CreateNextLogEntryLocation: EBlock 0x%X is already erased\n", logicalBlockNum, 0);
    }
    #endif  // #if DEBUG_PRE_ERASED
    if(FTL_FALSE == eraseStatus)
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    {

       // Erase it before writting to make sure its empty...
       if((status = FTL_EraseOp(devID, logicalBlockNum)) != FTL_ERR_PASS)
       {

          #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          if(status == FTL_ERR_FAIL)
          {
             return status;
          }
          SetBadEBlockStatus(devID, logicalBlockNum, FTL_TRUE);
          flushInfo.devID = devID;
          phyEBlockAddr = GetPhysicalEBlockAddr(devID, logicalBlockNum);
          flushInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyEBlockAddr, 0);
          flushInfo.vPage.pageOffset = 0;
          flushInfo.byteCount = 0;
          if(FLASH_MarkDefectEBlock(&flushInfo) != FLASH_PASS)
          {
             // do nothing, just try to mark bad, even if it fails we move on.
          }
          return FTL_ERR_LOG_NEXT_EBLOCK_FAIL;

          #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          return status;
          #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       }
    }
    flushInfo.devID = devID;
    flushInfo.byteCount = sizeof(SYS_EBLOCK_INFO);
    flushInfo.vPage.pageOffset = 0;
    phyEBlockAddr = GetPhysicalEBlockAddr(devID, logicalBlockNum);
    flushInfo.vPage.vPageAddr = CalcPhyPageAddrFromPageOffset(phyEBlockAddr, 0);
    latestIncNumber = GetTransLogEBCounter(devID) + 1;
    sysEBlockInfo.phyAddrThisEBlock = phyEBlockAddr;
    sysEBlockInfo.incNumber = latestIncNumber;
    sysEBlockInfo.type = SYS_EBLOCK_INFO_LOG;
    sysEBlockInfo.checkVersion = EMPTY_WORD;
    sysEBlockInfo.oldSysBlock = EMPTY_WORD;
    sysEBlockInfo.fullFlushSig = EMPTY_WORD;
    for(tempCount = 0; tempCount < NUM_SYS_RESERVED_BYTES; tempCount++)
    {
       sysEBlockInfo.reserved[tempCount] = EMPTY_BYTE;
    }
    if((status = FTL_WriteSysEBlockInfo(&flushInfo, &sysEBlockInfo)) != FTL_ERR_PASS)
    {

       #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       if(status == FTL_ERR_FAIL)
       {
          return status;
       }
       SetBadEBlockStatus(devID, logicalBlockNum, FTL_TRUE);
       if(FLASH_MarkDefectEBlock(&flushInfo) != FLASH_PASS)
       {
          // do nothing, just try to mark bad, even if it fails we move on.
       }
       return FTL_ERR_LOG_NEXT_EBLOCK_FAIL;

       #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
       return status;
       #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    }
    #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetEBErased(devID, logicalBlockNum, FTL_FALSE);
    #endif  // #if (ENABLE_EB_ERASED_BIT == FTL_TRUE)
    SetTransLogEBCounter(devID, latestIncNumber);
    SetDirtyCount(devID, logicalBlockNum, 0);
    SetGCOrFreePageNum(devID, logicalBlockNum, 1);    
    MarkEBlockMappingTableEntryDirty(devID, logicalBlockNum);
    /*what if this is full*/
    if((status = TABLE_TransLogEBInsert(devID, logicalBlockNum, phyEBlockAddr, latestIncNumber)) != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}

//--------------------------
FTL_STATUS CreateNextTransLogEBlock(FTL_DEV devID, UINT16 logicalBlockNum)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 nextLogicalEBlock = EMPTY_WORD;             /*2*/
    UINT16 nextPhysicaEBlock = EMPTY_WORD;             /*2*/

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;                          /*2*/
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    SetGCOrFreePageNum(devID, logicalBlockNum, NUM_LOG_ENTRIES_PER_EBLOCK);

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
    {
       if((status = FTL_FindEmptyTransLogEBlock(devID, &nextLogicalEBlock, &nextPhysicaEBlock)) != FTL_ERR_PASS)
       {
          return status;
       }
       if((status = CreateNextLogEntryLocation(devID, nextLogicalEBlock)) != FTL_ERR_LOG_NEXT_EBLOCK_FAIL)
       {
          break;
       }
       sanityCounter++;
    }
    if(status != FTL_ERR_PASS)
    {
       return status;
    }

    #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    if((status = FTL_FindEmptyTransLogEBlock(devID, &nextLogicalEBlock, &nextPhysicaEBlock)) != FTL_ERR_PASS)
    {
       return status;
    }
    if((status = CreateNextLogEntryLocation(devID, nextLogicalEBlock)) != FTL_ERR_PASS)
    {
       return status;
    }
    #endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    return FTL_ERR_PASS;
}

//-----------------------------------------
FTL_STATUS TABLE_InitGcNum(void)
{
    UINT32 maxValue = 0;                               /*4*/
    UINT16 logicalBlockNum = EMPTY_WORD;               /*2*/
    FTL_DEV devID = 0;                                 /*1*/

    for (devID = 0; devID < NUMBER_OF_DEVICES; devID++)
    {
       maxValue = 0;
       for (logicalBlockNum = 0; logicalBlockNum < NUM_DATA_EBLOCKS; logicalBlockNum++)
       {
          if (GetGCNum(devID, logicalBlockNum) > maxValue)
          {
             maxValue = GetGCNum(devID, logicalBlockNum);
          }
       }
       // Set to greater than the largest value used so far
       GCNum[devID] = maxValue + 1;
    }
    return FTL_ERR_PASS;
}

//------------------------------
FTL_STATUS GetPhyPageAddr(ADDRESS_STRUCT_PTR currentPage, UINT16 phyEBNum, 
    UINT16 logEBNum, UINT32_PTR phyPage) 
{   
    UINT16 pageIndex = 0;                              /*2*/
    UINT16 temp2 = 0;                                  /*2*/

    pageIndex = GetIndexFromPhyPage(currentPage->logicalPageNum);
    temp2 = GetPPASlot(currentPage->devID,  logEBNum, pageIndex);
    *phyPage = CalcPhyPageAddrFromPageOffset(phyEBNum, temp2);
    if(temp2 == CHAIN_INVALID)
    {
       *phyPage  = PAGE_CHAINED;
    }       
    return FTL_ERR_PASS;
}

#if( FTL_EBLOCK_CHAINING == FTL_TRUE)
//--------------------------
UINT16 GetLongestChain(FTL_DEV devID)
{
    UINT16 logicalEBNum = EMPTY_WORD;                  /*2*/
    UINT16 chainEB = 0;                                /*2*/
    UINT16 numPages = 0;                               /*2*/
    UINT16 numPagesTemp = 0;                           /*2*/
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
          numPagesTemp =  NUM_PAGES_PER_EBLOCK - GetNumFreePages(devID, logicalEBNum);
          if(numPagesTemp > numPages)
          {
             numPages = numPagesTemp;
             highEB = chainEB;
          }               
       }
    }
    return highEB;
}
#endif  // #if( FTL_EBLOCK_CHAINING == FTL_TRUE)

//------------------------
FTL_STATUS FTL_CheckMount_SetMTLockBit(void)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    FTL_STATUS status = FTL_ERR_PASS;

    if(mountStatus == 0)
    {
       return FTL_ERR_NOT_MOUNTED;
    }
    if((status = FTL_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//-----------------------
FTL_STATUS FTL_CheckUnmount_SetMTLockBit(void)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    FTL_STATUS status = FTL_ERR_PASS;
 
    if(mountStatus == 1)
    {
       return FTL_ERR_MOUNTED;
    }
    if((status = FTL_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//------------------------
FTL_STATUS FTL_SetMTLockBit(void)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    if(lockStatus == 0)
    {
       lockStatus = 1;
    }
    else
    {
       return FTL_ERR_LOCKED;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//-------------------------
FTL_STATUS FTL_ClearMTLockBit(void)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    if(lockStatus == 1)
    {
       lockStatus = 0;
    }
    else
    {
       return FTL_ERR_UNLOCKED;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//------------------------
void FTL_SetMountBit(void)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    mountStatus = 1;
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)
}

//---------------------------
void FTL_ClearMountBit(void)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    mountStatus = 0;
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)
}

//----------------------------
FTL_STATUS FTL_CheckDevID(UINT8 DevID)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    if(DevID >= NUM_DEVICES)
    {
       return FTL_ERR_ARG_DEVID;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//------------------------------
FTL_STATUS FTL_CheckRange(UINT32 lba, UINT32 nb)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    if((lba >= MAX_NUMBER_LBA) ||           // Check for start too high
       (nb > MAX_NUMBER_LBA) ||            // Check for too many
       ((lba + nb) < lba) ||                // Check for addition overflow
       ((lba + nb) < nb) ||                 // Check for addition overflow
       ((lba + nb - 1) >= MAX_NUMBER_LBA))  // Check for end too high
    {
       return FTL_ERR_OUT_OF_RANGE;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//------------------------------
FTL_STATUS FTL_CheckPointer(void *Ptr)
{
    #if(FTL_CHECK_ERRORS == FTL_TRUE)
    if(Ptr == NULL)
    {
       return FTL_ERR_ARG_PTR;
    }
    #endif  // #if (FTL_CHECK_ERRORS == FTL_TRUE)

    return FTL_ERR_PASS;
}

//--------------------------------
FTL_STATUS ResetIndexValue(FTL_DEV devID, LOG_ENTRY_LOC_PTR startLoc)
{
     UINT32  latestIncNumber = EMPTY_DWORD;            /*4*/
     UINT16  logicalBlockNum = EMPTY_WORD;             /*2*/
     UINT16  phyEBlockAddr = EMPTY_WORD;               /*2*/
     FTL_STATUS status = FTL_ERR_PASS;                 /*4*/
     UINT32 key = 0;                                   /*4*/
     UINT16 count = 0;                                 /*2*/
     UINT16 logEBAddr = EMPTY_WORD;                    /*2*/
     UINT16 phyEBAddr = EMPTY_WORD;                    /*2*/
     UINT16 eBlockNum = EMPTY_WORD;                    /*2*/

     latestIncNumber = GetTransLogEBCounter(devID);
     status = TABLE_TransLogEBGetLatest(devID, &logicalBlockNum, &phyEBlockAddr, latestIncNumber);
     if (FTL_ERR_PASS != status)
     {
        return status;
     }
     if((startLoc->eBlockNum == EMPTY_WORD) && (startLoc->entryIndex == EMPTY_WORD))
     {
        SetGCOrFreePageNum(devID, logicalBlockNum, 1);
     }
     else
     {
       for(count = 0; count < NUM_TRANSACTION_LOG_EBLOCKS; count++)
       {
          if((status = TABLE_GetTransLogEntry(devID, count, &logEBAddr, &phyEBAddr, &key)) != FTL_ERR_PASS)
          {
             return status;
          }
          if((logEBAddr == EMPTY_WORD) && (phyEBAddr == EMPTY_WORD) && (key == EMPTY_DWORD))
          {
             break;
          }
          if((logEBAddr == logicalBlockNum) && (phyEBAddr == phyEBlockAddr) && (key == latestIncNumber))
          {
             eBlockNum = count;
             break;
          }
       }
       if(eBlockNum == startLoc->eBlockNum)
       {
          SetGCOrFreePageNum(devID, logicalBlockNum, startLoc->entryIndex);
       }
       else
       {
          return FTL_ERR_FLUSH_SHUTDOWN;
       }
     }
     return FTL_ERR_PASS;
}

#if(FTL_CHECK_VERSION == FTL_TRUE)
FTL_STATUS FTL_CheckVersion(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    FLASH_STATUS flash_status = FLASH_PASS;            /*4*/
    UINT8 devID = 0;                                   /*1*/
    UINT16 eBlockCount = 0;                            /*2*/
    UINT16 checkVersion = EMPTY_WORD;                  /*2*/
    UINT16 logEBlockAddr = EMPTY_WORD;                 /*2*/
    UINT16 phyEBlockAddr = EMPTY_WORD;                 /*2*/
    UINT32 key = EMPTY_DWORD;                          /*4*/
    FLASH_PAGE_INFO flashPage = {0, 0, {0, 0} };       /*11*/
    SYS_EBLOCK_INFO sysEBlockInfo;                     /*16*/

    checkVersion = CalcCheckWord((UINT16_PTR)FTL_FLASH_IMAGE_VERSION, NUM_WORDS_OF_VERSION);
    for(eBlockCount = 0; eBlockCount < NUM_FLUSH_LOG_EBLOCKS; eBlockCount++)
    {
       if((status = TABLE_GetFlushLogEntry(devID, eBlockCount, &logEBlockAddr, &phyEBlockAddr, &key)) != FTL_ERR_PASS)
       {
          return status;
       }
       if(key != EMPTY_DWORD)
       {
          flashPage.devID = devID;
          flashPage.vPage.vPageAddr = CalcPhyPageAddrFromLogIndex(phyEBlockAddr, 0);
          flashPage.vPage.pageOffset = 0;

          #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
          flashPage.byteCount = sizeof(sysEBlockInfo);
          flash_status = FLASH_RamPageReadMetaData(&flashPage, (UINT8_PTR)(&sysEBlockInfo));

          #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
          flashPage.byteCount = SECTOR_SIZE;
          flash_status = FLASH_RamPageReadDataBlock(&flashPage, &pseudoRPB[devID][0]);
          MEM_Memcpy((UINT8_PTR)(&sysEBlockInfo), &pseudoRPB[devID][0], SYS_INFO_SIZE);
          #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

          if(flash_status != FLASH_PASS)
          {
             return FTL_ERR_FLASH_READ_17;
          }

          if(sysEBlockInfo.checkVersion != EMPTY_WORD)
          {
             if(sysEBlockInfo.checkVersion == checkVersion)
             {
                return FTL_ERR_PASS;
             }
             else
             {
                return FTL_ERR_VERSION_MISMATCH;
             }
          }
       }
    }

    return FTL_ERR_VERSION_UNKNOWN;
}
#endif // #if(FTL_CHECK_VERSION == FTL_TRUE)

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
FTL_STATUS TransLogEBFailure(FLASH_PAGE_INFO_PTR flashPagePtr, UINT8_PTR logPtr)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 logicalBlockNum = EMPTY_WORD;               /*2*/
    UINT32 byteCount = 0;                              /*4*/
    UINT16 sanityCounter = 0;                          /*2*/
    FLASH_STATUS flashStatus = FLASH_PASS;

    byteCount = flashPagePtr->byteCount;
    SetTransLogEBFailedBadBlockInfo();
    logicalBlockNum = GetTransLogEBNumBadBlockInfo();
    SetBadEBlockStatus(flashPagePtr->devID, logicalBlockNum, FTL_TRUE);
    if(FLASH_MarkDefectEBlock(flashPagePtr) != FLASH_PASS)
    {
       // do nothing, just try to mark bad, even if it fails we move on.
    }
    while(sanityCounter < MAX_BAD_BLOCK_SANITY_TRIES)
    {
       if(GetTransLogEBArrayCount(flashPagePtr->devID) < NUM_TRANSACTION_LOG_EBLOCKS)
       {
          if((status = CreateNextTransLogEBlock(flashPagePtr->devID, logicalBlockNum)) != FTL_ERR_PASS)
          {
             return status;
          }
          /*rewrite the log*/
          if((status = GetNextLogEntryLocation(flashPagePtr->devID, flashPagePtr)) != FTL_ERR_PASS)
          {
             return status;
          }
          flashPagePtr->byteCount = byteCount;
          flashStatus = FLASH_RamPageWriteDataBlock(flashPagePtr, logPtr);
          if(flashStatus == FLASH_PASS)
          {
             status = FTL_ERR_PASS;
             break;
          }
          else
          {
             if(flashStatus == FLASH_PARAM)
             {
                return FTL_ERR_FAIL;
             }
             logicalBlockNum = GetTransLogEBNumBadBlockInfo();
             SetBadEBlockStatus(flashPagePtr->devID, logicalBlockNum, FTL_TRUE);
             if(FLASH_MarkDefectEBlock(flashPagePtr) != FLASH_PASS)
             {
                // do nothing, just try to mark bad, even if it fails we move on.
             }
             sanityCounter++;
          }
       }
       else
       {
          return FTL_ERR_LOG_RECOVERY_EBLOCK;
       }
    }
    if(status != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
