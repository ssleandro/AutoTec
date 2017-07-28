/* file: ftl_calc.h */
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

#ifndef FTL_CALC_H
#define FTL_CALC_H

#include "ftl_def.h"

UINT8 GetBitMapField (UINT8_PTR table, UINT16 fieldNum, UINT8 fieldSize);
void SetBitMapField (UINT8_PTR table, UINT16 fieldNum, UINT8 fieldSize, UINT8 value);
FTL_STATUS IncPageAddr (ADDRESS_STRUCT_PTR pageAddr);
FTL_STATUS GetPageNum (UINT32 LBA, UINT32_PTR pageAddress);
FTL_STATUS GetDevNum (UINT32 LBA, UINT8_PTR devNum);
FTL_STATUS GetSectorNum (UINT32 LBA, UINT8_PTR secNum);
FTL_STATUS GetLogicalEBNum (UINT32 pageAddress, UINT16_PTR logicalEBNum);
FTL_STATUS GetLogicalPageOffset (UINT32 pageAddress, UINT16_PTR pageOffset);
FTL_STATUS GetPhysicalPageOffset (UINT32 pageAddress, UINT16_PTR pageOffset);
FTL_STATUS GetPageSpread (UINT32 LBA, UINT32 NB, ADDRESS_STRUCT_PTR startPage, UINT32_PTR totalPages, ADDRESS_STRUCT_PTR endPage );
UINT32 CalcPhyPageAddrFromLogIndex (UINT16 phyEBNum, UINT16 logIndex);
UINT32 CalcPhyPageAddrFromPageOffset (UINT16 phyEBNum, UINT16 pageOffset);
UINT16 GetIndexFromPhyPage (UINT32 freePageAddr);
FTL_STATUS SetPhyPageIndex (FTL_DEV deviceNum, UINT16 logicalPageAddr, UINT16 physicalPage);
UINT16 CalcNumLogEntries (UINT32 numPages);
UINT16 CalcCheckWord (UINT16_PTR dataPtr, UINT16 nDataWords);
UINT8 VerifyCheckWord (UINT16_PTR dataPtr, UINT16 nDataWords, UINT16 checkWord);
//UINT16 pickEBCandidate(EMPTY_LIST_PTR emptyList, UINT16 totalEmpty, UINT16_PTR index);
UINT16 pickEBCandidate (EMPTY_LIST_PTR emptyList, UINT16 totalEmpty, UINT8 pickHottest);
UINT32 CalculateEBEraseWeight (FTL_DEV devID, UINT16 logEBNum);
UINT32 CALC_MixPageAddress (UINT32 pageAddr);
/* 
 * Spansion CRC algorithm generates standard CRC32(4byte) per 2048byte data array.
 */

#if (SPANSION_CRC32 == FTL_TRUE)
typedef enum
{
	CRC_ERROR_NONE = 0, /* no error             */
	CRC_ERROR /* data error           */
}crc_error;

/*
 * Build auxiliary table for parallel byte-at-a-time CRC-32.
 */
#define CRC32_POLY 0x04c11db7     /* AUTODIN II, Ethernet, & FDDI */
#define PAYLOAD_SIZE_BYTES_CRC VIRTUAL_PAGE_SIZE

extern void CalcInitCRC(void);

void CalcCalculateCRC(UINT8_PTR data, UINT32_PTR crc32);
FTL_STATUS CalcCompareCRC(UINT32 origCRC, UINT32 newCRC);

#endif /*SPANSION_CRC32*/

#endif  // #ifndef FTL_CALC_H
