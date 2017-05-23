// file: ftl_cache.c
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

/* Cache and Tanslation Table */
#ifdef __KERNEL__
  #include <linux/span/FTL/ftl_def.h>
  #include <linux/span/FTL/ftl_calc.h>
  #include <linux/span/FTL/ftl_common.h>
#else
  #include "ftl_def.h"
  #include "ftl_calc.h"
  #include "ftl_common.h"
#endif // #ifdef __KERNEL__

//------------------------
FTL_STATUS FTL_FlushTableCache(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                     /*4*/

    #if((FTL_RPB_CACHE == FTL_TRUE) || ((FTL_SUPER_SYS_EBLOCK == FTL_TRUE) && (FTL_DEFECT_MANAGEMENT == FTL_TRUE)))
    UINT8 devID = 0;                                      /*1*/
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;
    #endif
    #endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

    #if (DEBUG_FTL_API_ANNOUNCE == 1)
    DBG_Printf("FTL_FlushTableCache:\n", 0, 0);
    #endif  // #if (DEBUG_FTL_API_ANNOUNCE == 1)

    if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    for(devID = 0; devID < NUMBER_OF_DEVICES; devID++)
    {
       sanityCounter = 0;
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

    #if(FTL_RPB_CACHE == FTL_TRUE)
    for(devID = 0; devID < NUM_DEVICES; devID++)
    {
       if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    #endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

    if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
    {
       FTL_ClearMTLockBit();
       return status;
    }
    FTL_ClearMTLockBit();
    FTL_UpdatedFlag = UPDATED_DONE;
    return FTL_ERR_PASS;
}

//------------------------------------------------
FTL_STATUS FTL_FlushDataCache(void)
{
    FTL_STATUS status = FTL_ERR_PASS;                     /*4*/

    #if((FTL_RPB_CACHE == FTL_TRUE) || ((FTL_SUPER_SYS_EBLOCK == FTL_TRUE) && (FTL_DEFECT_MANAGEMENT == FTL_TRUE)))
    UINT8 devID = 0;                                      /*1*/
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    UINT16 sanityCounter = 0;
    #endif
    #endif  // #if((FTL_RPB_CACHE == FTL_TRUE) || (FTL_SUPER_SYS_EBLOCK == FTL_TRUE))

    if((status = FTL_CheckMount_SetMTLockBit()) != FTL_ERR_PASS)
    {
       return status;
    }

    #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    for(devID = 0; devID < NUMBER_OF_DEVICES; devID++)
    {
       sanityCounter = 0;
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
    
    #if(FTL_RPB_CACHE == FTL_TRUE)
    for(devID = 0; devID < NUM_DEVICES; devID++)
    {
       if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    #endif  // #if(FTL_RPB_CACHE == FTL_TRUE)
    
    FTL_ClearMTLockBit();
    FTL_UpdatedFlag = UPDATED_DONE;
    return FTL_ERR_PASS;
}

#if(FTL_RPB_CACHE == FTL_TRUE)
//------------------------------
FTL_STATUS InitRPBCache(void)
{
    FTL_DEV devCount = 0;                              /*1*/

    for(devCount = 0; devCount < NUM_DEVICES; devCount++)
    {
       ClearRPBCache(devCount);
    }
    RPBCacheReadGroup.LBA = 0;
    RPBCacheReadGroup.NB = 0;
    RPBCacheReadGroup.byteBuffer = 0;
    return FTL_ERR_PASS;
}

//------------------------------
FTL_STATUS UpdateRPBCache(UINT8 devID, UINT32 logicalPageAddr, UINT32 startSector, UINT32 numSectors, UINT8_PTR byteBuffer)
{
    UINT8 *destBuf = NULL;                                /*4*/

    if(GetRPBCacheStatus(devID) == CACHE_EMPTY)
    {
       return FTL_ERR_RPB_CACHE_EMPTY_01;
    }
    if(GetRPBCacheLogicalPageAddr(devID) != logicalPageAddr)
    {
       return FTL_ERR_RPB_CACHE_MISS_01;
    }
    destBuf = GetRPBCache(devID) + (startSector * SECTOR_SIZE);
    MEM_Memcpy(destBuf, byteBuffer, (numSectors * SECTOR_SIZE));
    SetRPBCacheLogicalPageAddr(devID, logicalPageAddr);
    SetRPBCacheStatus(devID, CACHE_DIRTY);
    return FTL_ERR_PASS;
}

//------------------------
FTL_STATUS ReadRPBCache(UINT8 devID, UINT32 logicalPageAddr, UINT32 startSector, UINT32 numSectors, UINT8_PTR byteBuffer)
{
    UINT8 *srcBuf = NULL;                                 /*4*/

    if(GetRPBCacheStatus(devID) == CACHE_EMPTY)
    {
       return FTL_ERR_RPB_CACHE_EMPTY_02;
    }
    if(GetRPBCacheLogicalPageAddr(devID) != logicalPageAddr)
    {
       return FTL_ERR_RPB_CACHE_MISS_02;
    }
    srcBuf = GetRPBCache(devID) + (startSector * SECTOR_SIZE);
    MEM_Memcpy(byteBuffer, srcBuf, (numSectors * SECTOR_SIZE));
    return FTL_ERR_PASS;
}

//----------------------------
FTL_STATUS FlushRPBCache(UINT8 devID)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 LBA = 0;                                    /*4*/
    UINT32 entries = 0;                                /*4*/
    UINT8 *byteBuffer = NULL;                          /*4*/

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    FTL_STATUS badBlockStatus = FTL_ERR_PASS;          /*4*/
    UINT16 writeCount = 0;                             /*2*/
    #endif

    if(GetRPBCacheStatus(devID) != CACHE_DIRTY)
    {
       return FTL_ERR_PASS;
    }
    LBA = ((GetRPBCacheLogicalPageAddr(devID) * NUM_DEVICES) + devID) * NUMBER_OF_SECTORS_PER_PAGE;

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    do
    {
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

       do
       {
          status = FTL_BuildTransferMapForWriteBlocking(LBA, NUMBER_OF_SECTORS_PER_PAGE, &entries);
          if(status != FTL_ERR_PASS)
          {
       
             #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             badBlockStatus = TranslateBadBlockError(status);
             #endif
       
             if(status == FTL_ERR_DATA_GC_NEEDED)
             {
                UINT32 GC_status = FTL_ERR_PASS;
                UINT16 temp = EMPTY_WORD;
                UINT16 temp2 = EMPTY_WORD;

               #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
               GC_status = InternalForcedGCWithBBManagement(EMPTY_BYTE, EMPTY_WORD, &temp, &temp2, FTL_FALSE);

               #else
               GC_status = FTL_InternalForcedGC(EMPTY_BYTE, EMPTY_WORD, &temp, &temp2, FTL_FALSE);
               #endif

               if(GC_status != FTL_ERR_PASS)
               {
                  return GC_status;
               }
             }
             #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
             else if(badBlockStatus == FTL_ERR_BAD_BLOCK_SOURCE)
             {
                 if(writeCount >= MAX_BAD_BLOCK_SANITY_TRIES)
                 {
                     return status; 
                 }
                 writeCount++;
                 badBlockStatus = BB_ManageBadBlockErrorForChainErase(); // space check erase failure. so all flash changes will be in teh reserve pool
                 if(badBlockStatus !=FTL_ERR_PASS)
                 {
                     return status;
                 }
             }
             else if(badBlockStatus == FTL_ERR_LOG_WR)
             {
                 // should not come into here.
                 return FTL_ERR_FAIL;
                 /*
                 if(writeCount >= MAX_BAD_BLOCK_SANITY_TRIES)
                 {
                     return status; 
                 }
                 writeCount++;
                 badBlockStatus = BB_ManageBadBlockErrorForGCLog(); // space check erase failure. so all flash changes will be in teh reserve pool
                 if(badBlockStatus !=FTL_ERR_PASS)
                 {
                     return status;
                 }
                 // change error code
                 status = FTL_ERR_FAIL;
                 */
             }
             #endif
             else
             {
                return status;
             }
          }
       }while(status != FTL_ERR_PASS);
       
       /* check sanity */
       if(entries != 1)
       {
          return FTL_ERR_RPB_CACHE_NUM_01;
       }
       byteBuffer = GetRPBCache(devID);
       if((status = FTL_TransferPageForWrite(&byteBuffer, &entries)) != FTL_ERR_PASS)
       {
       
          #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
          badBlockStatus = TranslateBadBlockError(status);
          if(badBlockStatus == FTL_ERR_LOG_WR)
          {
              if(writeCount >= MAX_BAD_BLOCK_SANITY_TRIES)
              {
                  return status; 
              }
              writeCount++;
              badBlockStatus = BB_ManageBadBlockErrorForGCLog(); // space check erase failure. so all flash changes will be in teh reserve pool
              if(badBlockStatus !=FTL_ERR_PASS)
              {
                  return status;
              }
          }
          else if(badBlockStatus == FTL_ERR_BAD_BLOCK_SOURCE)
          {
              if(writeCount >= MAX_BAD_BLOCK_SANITY_TRIES)
              {
                  return status; 
              }
              writeCount++;
              badBlockStatus = BB_ManageBadBlockErrorForSource(); // space check erase failure. so all flash changes will be in teh reserve pool
              if(badBlockStatus !=FTL_ERR_PASS)
              {
                  return status;
              }
          }
          else
          #endif
          {
             return status;
          }
       }

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
        ClearBadBlockInfo();
    } while(status != FTL_ERR_PASS);
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    if(GetTransLogEBFailedBadBlockInfo() == FTL_TRUE)
    {
       if((status = TABLE_Flush(FLUSH_NORMAL_MODE)) != FTL_ERR_PASS)
       {
          return status;
       }
    }
    ClearTransLogEBBadBlockInfo();
    #endif  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

    SetRPBCacheStatus(devID, CACHE_CLEAN);
    return FTL_ERR_PASS;
}

//-------------------------
FTL_STATUS ReadFlash(UINT8 devID, UINT32 logicalPageAddr, UINT32 startSector, UINT32 NB, UINT8_PTR byteBuffer)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 LBA = 0;                                    /*4*/
    UINT32 entries = 0;                                /*4*/

    LBA = (((logicalPageAddr * NUM_DEVICES) + devID) * NUMBER_OF_SECTORS_PER_PAGE) + startSector;
    status = FTL_BuildTransferMapForReadBlocking(LBA, NB, &entries);
    if (status != FTL_ERR_PASS)
    {
       return status;
    }
    /* check sanity */
    if(entries != 1)
    {
       return FTL_ERR_RPB_CACHE_NUM_02;
    }
    if((status = FTL_TransferPageForRead(&byteBuffer, &entries)) != FTL_ERR_PASS)
    {
       return status;
    }
    return FTL_ERR_PASS;
}

//-------------------------
FTL_STATUS FillRPBCache(UINT8 devID, UINT32 logicalPageAddr)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT8_PTR byteBuffer = NULL;                       /*4*/

    byteBuffer = GetRPBCache(devID);
    if((status = ReadFlash(devID, logicalPageAddr, 0, NUMBER_OF_SECTORS_PER_PAGE, byteBuffer)) != FTL_ERR_PASS)
    {
       return status;
    }
    SetRPBCacheLogicalPageAddr(devID, logicalPageAddr);
    SetRPBCacheStatus(devID, CACHE_CLEAN);
    return FTL_ERR_PASS;
}

//-----------------------------
FTL_STATUS RPBCacheForWrite(UINT8_PTR *byteBuffer, UINT32_PTR LBA, UINT32_PTR NB, UINT32_PTR bytesDone)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 totalPages = 0;                             /*4*/
    ADDRESS_STRUCT endPage = {0,0,0};                  /*6*/
    ADDRESS_STRUCT startPage = {0,0,0};                /*6*/
    UINT8 devID = 0;                                   /*1*/
    UINT32 logicalPageAddr = EMPTY_WORD;               /*2*/
    UINT8 startSector = 0;                             /*1*/
    UINT32 numSectors = 0;                             /*4*/
    CACHE_INFO cache = {0, 0};                         /*5*/
    #if(FTL_RPB_CACHE_API_PFT_SAFE == FTL_FALSE)
    UINT8_PTR dataBuf = NULL;                          /*4*/
    #endif  // #if(FTL_RPB_CACHE_API_PFT_SAFE == FTL_FALSE)

    numSectors = *NB;
    if(numSectors > 0)
    {
       if((status = GetPageSpread(*LBA, numSectors, &startPage, &totalPages, &endPage)) != FTL_ERR_PASS)
       {
          return status;
       }

       if(totalPages == 1)
       { /* single-page transaction */
          devID = startPage.devID;
          logicalPageAddr = startPage.logicalPageNum;
          startSector = startPage.pageOffset;
          cache.status = GetRPBCacheStatus(devID);
          cache.logicalPageAddr = GetRPBCacheLogicalPageAddr(devID);
          if(cache.status == CACHE_EMPTY)
          {
             if((status = FillRPBCache(devID, logicalPageAddr)) != FTL_ERR_PASS)
             {
                return status;
             }
             if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
             {
                return status;
             }
          }
          else if(cache.status == CACHE_CLEAN)
          {
             if(logicalPageAddr != cache.logicalPageAddr)
             {
                if((status = FillRPBCache(devID, logicalPageAddr)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
             if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
             {
                return status;
             }
          }
          else if(cache.status == CACHE_DIRTY)
          {
             if(logicalPageAddr == cache.logicalPageAddr)
             {
                if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
             else
             {
                if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
                {
                   return status;
                }
                if((status = FillRPBCache(devID, logicalPageAddr)) != FTL_ERR_PASS)
                {
                   return status;
                }
                if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
          }
          *LBA += numSectors;
          *NB -= numSectors;
          if(*NB != 0)
          {
             return FTL_ERR_RPB_CACHE_NUM_03;
          }
          *byteBuffer += (numSectors * SECTOR_SIZE);
          *bytesDone += (numSectors * SECTOR_SIZE);
       }
       else
       { /* multiple-pages transaction */
          #if(FTL_RPB_CACHE_API_PFT_SAFE == FTL_TRUE)
          for(devID = 0; devID < NUM_DEVICES; devID++)
          {
             if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
             {
                return status;
             }
             ClearRPBCache(devID);
          }

          #else  // #if(FTL_RPB_CACHE_API_PFT_SAFE == FTL_TRUE)
          /*start page*/
          devID = startPage.devID;
          logicalPageAddr = startPage.logicalPageNum;
          startSector = startPage.pageOffset;
          numSectors = NUMBER_OF_SECTORS_PER_PAGE - startSector;
          cache.status = GetRPBCacheStatus(devID);
          cache.logicalPageAddr = GetRPBCacheLogicalPageAddr(devID);
          if(cache.status == CACHE_EMPTY)
          {

          }
          else if(cache.status == CACHE_CLEAN)
          {
             if(logicalPageAddr == cache.logicalPageAddr)
             {
                if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
                {
                   return status;
                }
                if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
                {
                   return status;
                }
                *LBA += numSectors;
                *NB -= numSectors;
                *byteBuffer += (numSectors * SECTOR_SIZE);
                *bytesDone += (numSectors * SECTOR_SIZE);
             }
          }
          else if(cache.status == CACHE_DIRTY)
          {
             if(logicalPageAddr == cache.logicalPageAddr)
             {
                if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
                {
                   return status;
                }
                if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
                {
                   return status;
                }
                *LBA += numSectors;
                *NB -= numSectors;
                *byteBuffer += (numSectors * SECTOR_SIZE);
                *bytesDone += (numSectors * SECTOR_SIZE);
             }
             else
             {
                if((status = FlushRPBCache(devID)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
          }

          /*end page*/
          if(endPage.pageOffset == 0)
          {
             if(endPage.devID == 0)
             {
                devID = (NUM_DEVICES - 1);
                logicalPageAddr = endPage.logicalPageNum - 1;
             }
             else
             {
                devID = endPage.devID - 1;
                logicalPageAddr = endPage.logicalPageNum;
             }
             startSector = 0;
             numSectors = NUM_SECTORS_PER_PAGE;
          }
          else
          {
             devID = endPage.devID;
             logicalPageAddr = endPage.logicalPageNum;
             startSector = 0;
             numSectors = endPage.pageOffset;
          }
          cache.status = GetRPBCacheStatus(devID);
          cache.logicalPageAddr = GetRPBCacheLogicalPageAddr(devID);
          dataBuf = (*byteBuffer) + ((*NB - numSectors) * SECTOR_SIZE);
          if(cache.status == CACHE_EMPTY)
          {
             if((status = FillRPBCache(devID, logicalPageAddr)) != FTL_ERR_PASS)
             {
                return status;
             }
             if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, dataBuf)) != FTL_ERR_PASS)
             {
                return status;
             }
             *NB -= numSectors;
             *bytesDone += (numSectors * SECTOR_SIZE);
          }
          else if(cache.status == CACHE_CLEAN)
          {
             if(logicalPageAddr != cache.logicalPageAddr)
             {
                if((status = FillRPBCache(devID, logicalPageAddr)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
             if((status = UpdateRPBCache(devID, logicalPageAddr, startSector, numSectors, dataBuf)) != FTL_ERR_PASS)
             {
                return status;
             }
             *NB -= numSectors;
             *bytesDone += (numSectors * SECTOR_SIZE);
          }
          else if(cache.status == CACHE_DIRTY)
          {
             return FTL_ERR_RPB_CACHE_DIRTY;
          }
          #endif  // #else  // #if(FTL_RPB_CACHE_API_PFT_SAFE == FTL_TRUE)
       }
    }
    return FTL_ERR_PASS;
}

//------------------------
FTL_STATUS RPBCacheForRead(UINT8_PTR *byteBuffer, UINT32_PTR LBA, UINT32_PTR NB, UINT32_PTR bytesDone)
{
    FTL_STATUS status = FTL_ERR_PASS;                  /*4*/
    UINT32 totalPages = 0;                             /*4*/
    ADDRESS_STRUCT endPage = {0,0,0};                  /*6*/
    ADDRESS_STRUCT startPage = {0,0,0};                /*6*/
    ADDRESS_STRUCT currentPage = {0,0,0};              /*6*/
    UINT8 devID = 0;                                   /*1*/
    UINT32 logicalPageAddr = EMPTY_WORD;               /*2*/
    UINT8 startSector = 0;                             /*1*/
    UINT32 numSectors = 0;                             /*4*/
    UINT32 currentPageCount = 0;                       /*4*/
    UINT32 totalSectors = 0;                           /*4*/
    UINT8_PTR dataBuf = NULL;                          /*4*/
    CACHE_INFO cache = {0, 0};                         /*5*/

    RPBCacheReadGroup.LBA = 0;
    RPBCacheReadGroup.NB = 0;
    RPBCacheReadGroup.byteBuffer = 0;
    numSectors = *NB;
    if(numSectors > 0)
    {
       if((status = GetPageSpread(*LBA, numSectors, &startPage, &totalPages, &endPage)) != FTL_ERR_PASS)
       {
          return status;
       }

       if(totalPages == 1)
       { /* single-page transaction */
          devID = startPage.devID;
          logicalPageAddr = startPage.logicalPageNum;
          startSector = startPage.pageOffset;
          cache.status = GetRPBCacheStatus(devID);
          cache.logicalPageAddr = GetRPBCacheLogicalPageAddr(devID);
          if(cache.status != CACHE_EMPTY)
          {
             if(logicalPageAddr == cache.logicalPageAddr)
             {
                if((status = ReadRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
                {
                   return status;
                }
                *LBA += numSectors;
                *NB -= numSectors;
                if(*NB != 0)
                {
                   return FTL_ERR_RPB_CACHE_NUM_04;
                }
                *byteBuffer += (numSectors * SECTOR_SIZE);
                *bytesDone += (numSectors * SECTOR_SIZE);
             }
          }
       }
       else
       { /* multiple-pages transaction */
          cache.status = GetRPBCacheStatus(devID);
          cache.logicalPageAddr = GetRPBCacheLogicalPageAddr(devID);
          if(cache.status != CACHE_EMPTY)
          {
             currentPage = startPage;
             for(currentPageCount = 0; currentPageCount < totalPages; currentPageCount++)
             {
                devID = currentPage.devID;
                logicalPageAddr = currentPage.logicalPageNum;
                startSector = currentPage.pageOffset;
                if(currentPageCount == 0)
                { /*start page*/
                   numSectors = NUM_SECTORS_PER_PAGE - startSector;
                   if(logicalPageAddr == cache.logicalPageAddr)
                   {
                      if((status = ReadRPBCache(devID, logicalPageAddr, startSector, numSectors, *byteBuffer)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      *LBA += numSectors;
                      *NB -= numSectors;
                      *byteBuffer += (numSectors * SECTOR_SIZE);
                      *bytesDone += (numSectors * SECTOR_SIZE);
                      break;
                   }
                }
                else if(currentPageCount < (totalPages - 1))
                { /*middle pages*/
                   numSectors = NUM_SECTORS_PER_PAGE;
                   if(logicalPageAddr == cache.logicalPageAddr)
                   {
                      dataBuf = (*byteBuffer) + (totalSectors * SECTOR_SIZE);
                      if((status = ReadRPBCache(devID, logicalPageAddr, startSector, numSectors, dataBuf)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      RPBCacheReadGroup.LBA = (*LBA) + (totalSectors + numSectors);
                      RPBCacheReadGroup.NB = (*NB) - (totalSectors + numSectors);
                      RPBCacheReadGroup.byteBuffer = (*byteBuffer) + ((totalSectors + numSectors) * SECTOR_SIZE);
                      *NB = totalSectors;
                      *bytesDone += (numSectors * SECTOR_SIZE);
                      break;
                   }
                }
                else
                { /*end page*/
                   if(endPage.pageOffset == 0)
                   {
                      numSectors = NUM_SECTORS_PER_PAGE;
                   }
                   else
                   {
                      numSectors = endPage.pageOffset;
                   }
                   if(logicalPageAddr == cache.logicalPageAddr)
                   {
                      dataBuf = (*byteBuffer) + ((*NB - numSectors) * SECTOR_SIZE);
                      if((status = ReadRPBCache(devID, logicalPageAddr, startSector, numSectors, dataBuf)) != FTL_ERR_PASS)
                      {
                         return status;
                      }
                      *NB -= numSectors;
                      *bytesDone += (numSectors * SECTOR_SIZE);
                      break;
                   }
                }
                totalSectors += numSectors;
                if((status = IncPageAddr(&currentPage)) != FTL_ERR_PASS)
                {
                   return status;
                }
             }
          }
       }
    }
    return FTL_ERR_PASS;
}
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)
