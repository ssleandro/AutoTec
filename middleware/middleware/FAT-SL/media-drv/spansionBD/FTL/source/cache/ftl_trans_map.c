/* file: ftl_trans_map.c */
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
  #include <linux/span/FTL/ftl_calc.h>
  #include <linux/span/FTL/ftl_common.h>
#else
  #include "ftl_def.h"
  #include "ftl_calc.h"
  #include "ftl_common.h"
#endif // #ifdef __KERNEL__

#define FTL_TRANSFER_MAP_DEBUG  (0)
#define DEBUG_TRANSFER_LOG      (0)

//--------------------------------
FTL_STATUS UpdateTransferMap(UINT32 currentLBA, ADDRESS_STRUCT_PTR currentPage, 
    ADDRESS_STRUCT_PTR endPage, ADDRESS_STRUCT_PTR startPage, UINT32 totalPages, 
    UINT32 phyPage, UINT32 mergePage, UINT8 isWrite, UINT8 isChained)
{
    ADDRESS_STRUCT tempPage = {0,0,0};   /*6*/
    FTL_STATUS status = FTL_ERR_PASS;    /*4*/

    #if FTL_TRANSFER_MAP_DEBUG
    DBG_Printf("UpdateTransferMap: devId = %d, ", currentPage->devID, 0);
    DBG_Printf("currentLBA = 0x%X, ", currentLBA, 0);
    DBG_Printf("totalPages = %d, ", totalPages, 0);
    DBG_Printf("phyPage = 0x%X, ", phyPage, 0);
    DBG_Printf("mergePage = 0x%X\n", mergePage, 0);
    #endif  // #if FTL_TRANSFER_MAP_DEBUG

    if(TransferMapIndexEnd > NUM_TRANSFER_MAP_ENTRIES)
    {
       return FTL_ERR_TRANS_MAP_FULL;
    }
    SetTMDevID(TransferMapIndexEnd, currentPage->devID);
    SetTMStartLBA(TransferMapIndexEnd, currentLBA);
    SetTMPhyPage(TransferMapIndexEnd, phyPage);
    SetTMMergePage(TransferMapIndexEnd, mergePage);
    tempPage = *(currentPage);
    if(endPage->pageOffset == 0)
    {
       /*This is the case, where the page spread ends in the next page*/
       if((status = IncPageAddr( &tempPage )) != FTL_ERR_PASS)
       {
          return status;
       }
       if((tempPage.devID != endPage->devID) || (tempPage.logicalPageNum != endPage->logicalPageNum))
       {
          /*not the last page, so reset the tempPage*/
          tempPage = *(currentPage);
       }
    }
    if((currentPage->devID == startPage->devID) && (currentPage->logicalPageNum == startPage->logicalPageNum))
    { 
       /*this is the start page*/
       if((totalPages != 1) || (endPage->pageOffset == 0))
       {   
          /*multi page transfer*/
          SetTMNumSectors(TransferMapIndexEnd, NUM_SECTORS_PER_PAGE - startPage->pageOffset );
       }
       else
       {
          /*single page transfer*/
          SetTMNumSectors(TransferMapIndexEnd, endPage->pageOffset - startPage->pageOffset);
       }
       SetTMStartSector(TransferMapIndexEnd, startPage->pageOffset);

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       if(isWrite == FTL_TRUE)
       {
          if((status = InsertEntryIntoLogEntry(TransferMapIndexEnd, phyPage, currentLBA, 
             currentPage, isChained)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       TransferMapIndexEnd++;
       return FTL_ERR_PASS;
    }
    if((tempPage.devID == endPage->devID) && (tempPage.logicalPageNum == endPage->logicalPageNum))
    { 
       /*This is the end page*/
       if(endPage->pageOffset == 0)
       {
          SetTMNumSectors(TransferMapIndexEnd, NUM_SECTORS_PER_PAGE);
       }
       else
       {
          if(totalPages != 1)
          {   
             /*multi page transfer*/
             SetTMNumSectors(TransferMapIndexEnd, endPage->pageOffset);
          }
          else
          {
             /*single page transfer*/
             SetTMNumSectors(TransferMapIndexEnd, endPage->pageOffset - startPage->pageOffset);
          }
       }
       SetTMStartSector(TransferMapIndexEnd, currentPage->pageOffset);

       #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       if(isWrite == FTL_TRUE)
       {
          if((status = InsertEntryIntoLogEntry(TransferMapIndexEnd, phyPage, currentLBA, 
             currentPage, isChained)) != FTL_ERR_PASS)
          {
             return status;
          }
       }       
       #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

       TransferMapIndexEnd++;
       return FTL_ERR_PASS;
    }
    /*This is a middle page*/
    SetTMNumSectors(TransferMapIndexEnd, NUM_SECTORS_PER_PAGE );
    SetTMStartSector(TransferMapIndexEnd, 0);

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    if(isWrite == FTL_TRUE)
    {
       if((status = InsertEntryIntoLogEntry(TransferMapIndexEnd, phyPage, currentLBA, 
          currentPage, isChained)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    TransferMapIndexEnd++;
    return FTL_ERR_PASS;
}

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
//-----------------------------
FTL_STATUS InsertEntryIntoLogEntry(UINT16 index, UINT32 phyPageAddr, 
    UINT32 currentLBA, ADDRESS_STRUCT_PTR currentPage, UINT8 isChained)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT16 tempValue = 0;                              /*2*/
    UINT16 tempIndex = 0;                              /*2*/
    UINT16 tempIndex2 = 0;                             /*2*/
    UINT16 tempIndex3 = 0;                             /*2*/
    UINT16 chainFlag = 0;                              /*2*/

    if (isChained)
    {
       chainFlag = CHAIN_FLAG;
    }
    if(index == 0)
    {
       status = FTL_ClearA();
       if (status != FTL_ERR_PASS)
       {
          return status;
       }
       status = FTL_ClearC(0);
       if (status != FTL_ERR_PASS)
       {
          return status;
       }
       TranslogBEntries = 0;
       TransLogEntry.entryA.LBA = currentLBA;
       TransLogEntry.entryC.GCNum = GCNum[currentPage->devID];

       #if DEBUG_TRANSFER_LOG
       DBG_Printf("InsertEntryIntoLogEntry: type A: LBA = 0x%X\n", currentLBA, 0);
       #endif  // #if DEBUG_TRANSFER_LOG

       if((status = GetPhysicalPageOffset(phyPageAddr, &tempValue)) != FTL_ERR_PASS)
       {
          return status;
       }
       TransLogEntry.entryA.pageLoc[index].phyEBOffset = tempValue | chainFlag;

       #if DEBUG_TRANSFER_LOG
       DBG_Printf("  pageLoc[%d].", index, 0);
       DBG_Printf("phyEBOffset = 0x%X, ", tempValue | chainFlag, 0);
       #endif  // #if DEBUG_TRANSFER_LOG

       if((status = GetLogicalEBNum(currentPage->logicalPageNum, &tempValue)) != FTL_ERR_PASS)
       {
          return status;
       }
       TransLogEntry.entryA.pageLoc[index].logEBNum = tempValue;            

       #if DEBUG_TRANSFER_LOG
       DBG_Printf("logEBNum = 0x%X\n", tempValue, 0);
       #endif  // #if DEBUG_TRANSFER_LOG

    }
    else
    {
       if(index < NUM_ENTRIES_TYPE_A)
       {
          if((status = GetPhysicalPageOffset(phyPageAddr, &tempValue)) != FTL_ERR_PASS)
          {
             return status;
          }
          TransLogEntry.entryA.pageLoc[index].phyEBOffset = tempValue | chainFlag;

          #if DEBUG_TRANSFER_LOG
          DBG_Printf("InsertEntryIntoLogEntry: type A:\n", 0, 0);
          DBG_Printf("  pageLoc[%d].", index, 0);
          DBG_Printf("phyEBOffset = 0x%X, ", tempValue | chainFlag, 0);
          #endif  // #if DEBUG_TRANSFER_LOG

          if((status = GetLogicalEBNum(currentPage->logicalPageNum, &tempValue)) != FTL_ERR_PASS)
          {
             return status;
          }
          TransLogEntry.entryA.pageLoc[index].logEBNum = tempValue;            

          #if DEBUG_TRANSFER_LOG
          DBG_Printf("logEBNum = 0x%X\n", tempValue, 0);
          #endif  // #if DEBUG_TRANSFER_LOG

       }
       else
       {
          {
             /*TYPE B, fill it here*/              
             tempIndex = index - NUM_ENTRIES_TYPE_A;
             tempIndex2 = tempIndex/NUM_ENTRIES_TYPE_B;
             tempIndex3 = tempIndex % NUM_ENTRIES_TYPE_B;       
             if(tempIndex3 == 0)
             {
                status = FTL_ClearB(TranslogBEntries);
                if (status != FTL_ERR_PASS)
                {
                   return status;
                }
                TranslogBEntries++;
             }           
             if((status = GetPhysicalPageOffset(phyPageAddr, &tempValue)) != FTL_ERR_PASS)
             {
                return status;
             }
             TransLogEntry.entryB[tempIndex2].pageLoc[tempIndex3].phyEBOffset = tempValue | chainFlag;

             #if DEBUG_TRANSFER_LOG
             DBG_Printf("InsertEntryIntoLogEntry: type B[%d]:\n", tempIndex2, 0);
             DBG_Printf("  pageLoc[%d].", tempIndex3, 0);
             DBG_Printf("phyEBOffset = 0x%X, ", tempValue | chainFlag, 0);
             #endif  // #if DEBUG_TRANSFER_LOG

             if((status = GetLogicalEBNum(currentPage->logicalPageNum, &tempValue)) != FTL_ERR_PASS)
             {
                return status;
             }
             TransLogEntry.entryB[tempIndex2].pageLoc[tempIndex3].logEBNum = tempValue;            

             #if DEBUG_TRANSFER_LOG
             DBG_Printf("logEBNum = 0x%X\n", tempValue, 0);
             #endif  // #if DEBUG_TRANSFER_LOG

          }
          TransLogEntry.entryC.seqNum = (UINT8)(TranslogBEntries + 1);
       }
    }
    return status;
}
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

//------------------------------------
FTL_STATUS FTL_GetNextTransferMapEntry(UINT16_PTR nextEntryIndex, 
    UINT16_PTR startIndex, UINT16_PTR endIndex)
{
    if(TransferMapIndexEnd == TransferMapIndexStart)
    {
       return FTL_ERR_TRANS_MAP_EMPTY;
    }
    *nextEntryIndex = TransferMapIndexStart;
    /*Init this global, so ManageWrite() can access it later*/
    previousDevice = GetTMDevID(TransferMapIndexStart);
    /*should not need this again, clear it*/
    TransferMapIndexStart++;
    #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
    if(TransferMapIndexEnd == TransferMapIndexStart)
    {
       TransferMapIndexEnd = TransferMapIndexStart = 0;
    }
    #endif // #if(FTL_DEFECT_MANAGEMENT == FTL_FALSE)
    *startIndex = TransferMapIndexStart;
    *endIndex = TransferMapIndexEnd;
    return FTL_ERR_PASS;
}

UINT16 FTL_GetCurrentIndex(void)
{
    return TransferMapIndexStart;
}
//---------------------------
FTL_STATUS FTL_GetTransferMapEntry(UINT16 entryIndex, TRANS_MAP_ENTRY_PTR transferMapEntry)
{
    transferMapEntry->devID =  GetTMDevID(entryIndex);
    transferMapEntry->numSectors = GetTMNumSectors(entryIndex);
    transferMapEntry->phyPageAddr = GetTMPhyPage(entryIndex);
    transferMapEntry->startLBA = GetTMStartLBA(entryIndex);
    transferMapEntry->startSector = GetTMStartSector(entryIndex);
    transferMapEntry->logEBlockEntryIndex = GetTMLogInfo(entryIndex);
    transferMapEntry->mergePageForWrite = GetTMMergePage(entryIndex);
    return FTL_ERR_PASS;
}

//----------------------
FTL_STATUS TRANS_ClearEntry(UINT16 index)
{
    if(index > NUM_TRANSFER_MAP_ENTRIES)
    {
       return FTL_ERR_TRANS_NO_ENTRIES;
    }
    SetTMDevID(index, EMPTY_BYTE);
    SetTMNumSectors(index, EMPTY_BYTE);
    SetTMStartLBA(index, EMPTY_DWORD);
    SetTMPhyPage(index, EMPTY_DWORD);
    SetTMStartSector(index, EMPTY_BYTE);
    SetTMMergePage(index, EMPTY_DWORD);
    return FTL_ERR_PASS;
}

//--------------------------------
FTL_STATUS TRANS_ClearTransMap(void)
{
    UINT16 index = 0;                                  /*2*/
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    // Modify to defect management because defect management needs transfermap
    if(TransferMapIndexEnd == TransferMapIndexStart)
    {
       TransferMapIndexEnd = TransferMapIndexStart = 0;
       return FTL_ERR_PASS;
    }
    else
    {
       for(index = 0; index < NUM_TRANSFER_MAP_ENTRIES; index++)
       {
          if((status = TRANS_ClearEntry(index)) != FTL_ERR_PASS)
          {
             return status;
          }
       }
       TransferMapIndexEnd = TransferMapIndexStart = 0;
    }
    return FTL_ERR_PASS;
}

//-------------------------------
FTL_STATUS TRANS_InitTransMap(void)
{
    /*set the Transfer Map variables*/
    previousDevice = EMPTY_BYTE;
    TransferMapIndexEnd = TransferMapIndexStart = 0;
    /*end Transfer map stuff*/
    return FTL_ERR_PASS;
}
