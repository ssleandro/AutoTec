// file: ftl_calc.c
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
#include <linux/span/FTL/ftl_if_ex.h>
#include <linux/span/FTL/ftl_def.h>
#include <linux/span/FTL/ftl_common.h>
#else
#include "ftl_if_ex.h"
#include "ftl_def.h"
#include "ftl_common.h"
#endif // #ifdef __KERNEL__

UINT32 CALC_MixPageAddress (UINT32 pageAddr)
{
	UINT32 tempAddr = EMPTY_DWORD;
	UINT32 tempAddr1 = EMPTY_DWORD;

	tempAddr = (NUMBER_OF_PAGES_PER_EBLOCK * (pageAddr / NUMBER_OF_PAGES_PER_EBLOCK));
	tempAddr1 = NUMBER_OF_PAGES_PER_DEVICE - tempAddr;
	tempAddr1 += (pageAddr - tempAddr);
	return tempAddr1;
}
//------------------------------
UINT8 GetBitMapField (UINT8* table, UINT16 fieldNum, UINT8 fieldSize)
{
	UINT16 shift = 0; /*2*/
	UINT16 index = 0; /*2*/
	UINT8 mask = 0; /*1*/

	switch (fieldSize)
	{
		case 1:
		index = fieldNum >> 3;
		shift = 7 - (fieldNum & 0x7);
		mask = 0x1;
			break;
		case 2:
		index = fieldNum >> 2;
		shift = 6 - ((fieldNum & 0x3) << 1);
		mask = 0x3;
			break;
		case 3:
		index = fieldNum / 3;
		shift = 5 - ((fieldNum % 3) * 3);
		mask = 0x7;
			break;
		case 4:
		index = fieldNum >> 1;
		shift = 4 - ((fieldNum & 0x1) << 2);
		mask = 0xF;
			break;
		default:  // 5, 6, 7, or 8
		index = fieldNum;
		shift = 8 - fieldSize;
		mask = ((1 << fieldSize) - 1) << shift;
			break;
	}
	return mask & (table[index] >> shift);
}

//---------------------
void SetBitMapField (UINT8* table, UINT16 fieldNum, UINT8 fieldSize, UINT8 value)
{
	UINT16 shift = 0; /*2*/
	UINT16 index = 0; /*2*/
	UINT8 mask = 0; /*1*/

	switch (fieldSize)
	{
		case 1:
		index = fieldNum >> 3;
		shift = 7 - (fieldNum & 0x7);
		mask = 0x1 << shift;
			break;
		case 2:
		index = fieldNum >> 2;
		shift = 6 - ((fieldNum & 0x3) << 1);
		mask = 0x3 << shift;
			break;
		case 3:
		index = fieldNum / 3;
		shift = 5 - ((fieldNum % 3) * 3);
		mask = 0x7 << shift;
			break;
		case 4:
		index = fieldNum >> 1;
		shift = 4 - ((fieldNum & 0x1) << 2);
		mask = 0xF << shift;
			break;
		default:  // 5, 6, 7, or 8
		index = fieldNum;
		shift = 8 - fieldSize;
		mask = ((1 << fieldSize) - 1) << shift;
			break;
	}
	table[index] = (table[index] & ~mask) | ((value << shift) & mask);
}

//-------------------
FTL_STATUS IncPageAddr (ADDRESS_STRUCT_PTR pageAddr)
{
	if (pageAddr->devID == (NUM_DEVICES - 1))
	{
		pageAddr->devID = 0;
		(pageAddr->logicalPageNum)++;
	}
	else
	{
		(pageAddr->devID)++;
	}
	pageAddr->pageOffset = 0;
	return FTL_ERR_PASS;
}

//-----------------------
FTL_STATUS GetPageNum (UINT32 LBA, UINT32_PTR pageAddress)
{
	*pageAddress = (LBA & PAGE_ADDRESS_BIT_MAP) >> PAGE_ADDRESS_SHIFT;
	return FTL_ERR_PASS;
}

//------------------------------------------------------------------------
FTL_STATUS GetDevNum (UINT32 LBA, UINT8_PTR devNum)
{
	*devNum = (UINT8)(LBA & DEVICE_BIT_MAP) >> DEVICE_BIT_SHIFT;
	return FTL_ERR_PASS;
}

//-------------------------------------------------------------------------
FTL_STATUS GetSectorNum (UINT32 LBA, UINT8_PTR secNum)
{
	*secNum = (UINT8)(LBA & SECTOR_BIT_MAP) >> SECTOR_BIT_SHIFT;
	return FTL_ERR_PASS;
}

//-----------------------------------------------------------------------
FTL_STATUS GetLogicalEBNum (UINT32 pageAddress, UINT16_PTR logicalEBNum)
{
	*logicalEBNum = (UINT16)(pageAddress/NUM_PAGES_PER_EBLOCK);
	return FTL_ERR_PASS;
}

//---------------------------------------------------------------------------
FTL_STATUS GetLogicalPageOffset (UINT32 pageAddress, UINT16_PTR pageOffset)
{
	(*pageOffset) = (UINT16)(pageAddress % NUM_PAGES_PER_EBLOCK);
	return FTL_ERR_PASS;
}

// Note: in some technologies the Logical offset is different than the physical
//---------------------------------------------------------------------------
FTL_STATUS GetPhysicalPageOffset (UINT32 pageAddress, UINT16_PTR pageOffset)
{
	(*pageOffset) = (UINT16)(pageAddress % NUM_PAGES_PER_EBLOCK);
	return FTL_ERR_PASS;
}

//-----------------------------------------------------------------------------
FTL_STATUS SetPhyPageIndex (FTL_DEV deviceNum, UINT16 logicalPageAddr, UINT16 physicalPage)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT16 pageOffset = 0; /*2*/
	UINT16 logicalPageOffset = 0; /*2*/
	UINT16 logicalEBNum = 0; /*2*/

	if ((status = GetPhysicalPageOffset(physicalPage, &pageOffset)) != FTL_ERR_PASS)
	{
		return status;
	}
	if ((status = GetLogicalPageOffset(logicalPageAddr, &logicalPageOffset)) != FTL_ERR_PASS)
	{
		return status;
	}
	if ((status = GetLogicalEBNum(logicalPageAddr, &logicalEBNum)) != FTL_ERR_PASS)
	{
		return status;
	}
	SetPPASlot(deviceNum, logicalEBNum, logicalPageOffset, pageOffset);
	MarkPPAMappingTableEntryDirty(deviceNum, logicalEBNum, logicalPageOffset);
	return FTL_ERR_PASS;
}

//-----------------------------------------------------------------------------
FTL_STATUS GetPageSpread (UINT32 LBA, UINT32 NB, ADDRESS_STRUCT_PTR startPage,
UINT32_PTR totalPages, ADDRESS_STRUCT_PTR endPage )
{
	UINT32 aTemp = 0; /*4*/
	UINT32 bTemp = 0; /*4*/
	UINT32 cTemp = 0; /*4*/
	FTL_STATUS status = FTL_ERR_PASS; /*4*/

	if((status = GetDevNum(LBA, (&(startPage->devID)))) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = GetPageNum(LBA, (&(startPage->logicalPageNum)))) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = GetSectorNum(LBA, (&(startPage->pageOffset)))) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = GetDevNum(LBA + NB, (&(endPage->devID)))) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = GetPageNum(LBA + NB, (&(endPage->logicalPageNum)))) != FTL_ERR_PASS)
	{
		return status;
	}
	if((status = GetSectorNum(LBA + NB, (&(endPage->pageOffset)))) != FTL_ERR_PASS)
	{
		return status;
	}
	*totalPages = 0;
	if( startPage->pageOffset > 0)
	{
		(*totalPages)++;
		aTemp = (NUM_SECTORS_PER_PAGE - startPage->pageOffset);
	}
	if( endPage->pageOffset > 0)
	{
		(*totalPages)++;
		bTemp = endPage->pageOffset;
	}
	if((endPage->devID == startPage->devID) &&
	(endPage->logicalPageNum == startPage->logicalPageNum) &&
	(bTemp != 0) && (aTemp != 0) )
	{
		(*totalPages)--;
		return FTL_ERR_PASS;
	}
	cTemp = NB - aTemp - bTemp;
	(*totalPages) += cTemp / NUM_SECTORS_PER_PAGE;

#ifdef FTL_LEVEL_ONE_DEFINED
	if(cTemp % NUM_SECTORS_PER_PAGE) /*cTemp must be whole pages in size, if not we have error*/
	{
		return FTL_ERR_CALC_2;
	}
#endif  // #ifdef FTL_LEVEL_ONE_DEFINED

	return FTL_ERR_PASS;
}

//----------------------
UINT32 CalcPhyPageAddrFromLogIndex (UINT16 phyEBNum, UINT16 logIndex)
{
	if (logIndex == EMPTY_WORD)
	{
		return EMPTY_DWORD;
	}
	return ((phyEBNum * NUM_PAGES_PER_EBLOCK) + (logIndex / (VIRTUAL_PAGE_SIZE / LOG_ENTRY_DELTA)));
}

//-------------------------
UINT32 CalcPhyPageAddrFromPageOffset (UINT16 phyEBNum, UINT16 pageOffset)
{
	if (pageOffset == EMPTY_INVALID)
	{
		return EMPTY_DWORD;
	}
	return ((phyEBNum * NUM_PAGES_PER_EBLOCK) + pageOffset);
}

//-----------------------------
UINT32 CalcFlushRamTablePages (UINT16 phyEBNum, UINT16 index)
{
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
	if (index == 0)
	{
		return EMPTY_WORD;
	}
#endif

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	return (UINT32)((phyEBNum * NUM_PAGES_PER_EBLOCK)+((EBLOCK_SIZE - (index * FLUSH_RAM_TABLE_SIZE))/VIRTUAL_PAGE_SIZE));
#else
	return (UINT32)((phyEBNum * NUM_PAGES_PER_EBLOCK) + ((EBLOCK_SIZE - (index * SECTOR_SIZE)) / VIRTUAL_PAGE_SIZE));
#endif
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	return (UINT32)((phyEBNum * NUM_PAGES_PER_EBLOCK)+((index * SECTOR_SIZE)/VIRTUAL_PAGE_SIZE));
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
}

//------------------------------
UINT16 CalcFlushRamTableOffset (UINT16 index)
{
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
	if (index == 0)
	{
		return EMPTY_WORD;
	}
#endif

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
	return ((EBLOCK_SIZE - (index * FLUSH_RAM_TABLE_SIZE)) % VIRTUAL_PAGE_SIZE);
#else
	return ((EBLOCK_SIZE - (index * SECTOR_SIZE)) % VIRTUAL_PAGE_SIZE);
#endif

#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
	return ((index * SECTOR_SIZE) % VIRTUAL_PAGE_SIZE);
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
}

//---------------------------
UINT16 GetIndexFromPhyPage (UINT32 freePageAddr)
{
	return (UINT16)(freePageAddr % NUM_PAGES_PER_EBLOCK);
}

//------------------------------
UINT16 CalcNumLogEntries (UINT32 numPages)
{
	UINT32 temp0 = 0; /*4*/
	UINT32 temp = 0; /*4*/

	if (numPages > (NUM_ENTRIES_TYPE_A + (NUM_ENTRIES_TYPE_B * TEMP_B_ENTRIES)))
	{
		return EMPTY_WORD;
	}
	if (numPages <= NUM_ENTRIES_TYPE_A)
	{
		return MIN_LOG_ENTRIES_NEEDED;
	}
	temp0 = numPages - NUM_ENTRIES_TYPE_A;
	temp = temp0 / NUM_ENTRIES_TYPE_B;
	if (temp0 % NUM_ENTRIES_TYPE_B)
	{
		temp++;
	}
	return (UINT16)(temp + MIN_LOG_ENTRIES_NEEDED); /*to account for TYPE_A and account for TYPE_C*/
}

//------------------------
UINT16 CalcCheckWord (UINT16_PTR dataPtr, UINT16 nDataWords)
{
	UINT16 count = 0; /*2*/
	UINT16 sum = 0; /*2*/

	sum = 0;
	for (count = 0; count < nDataWords; count++)
	{
		sum += *dataPtr;
		dataPtr++;
	}
	return (sum ^ EMPTY_WORD) + 1;
}

//------------------------
UINT8 VerifyCheckWord (UINT16_PTR dataPtr, UINT16 nDataWords, UINT16 checkWord)
{
	UINT16 localWord = 0; /*2*/

	localWord = CalcCheckWord(dataPtr, nDataWords);
	if (localWord != checkWord)
	{
		return FTL_TRUE;  // Fail
	}
	return FTL_FALSE;   // Pass
}

//-------------------------
UINT32 CalculateEBEraseWeight (FTL_DEV devID, UINT16 logEBNum)
{
	UINT32 eraseCount = 0; /*4*/
	UINT32 useCount = 0; /*4*/

	eraseCount = GetTrueEraseCount(devID, logEBNum);
	useCount = GetNumUsedPages(devID, logEBNum);
	return (eraseCount * NUM_PAGES_PER_EBLOCK) + useCount;
}

#if (SPANSION_CRC32 == FTL_TRUE)
void CalcInitCRC()
{
	int i, j;
	unsigned int c;

	for (i = 0; i < 256; ++i)
	{
		for (c = i << 24, j = 8; j > 0; --j)
		c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
		crc32_table[i] = c;
	}
}

void CalcCalculateCRC(UINT8_PTR data, UINT32_PTR crc32)
{
	unsigned char *p;
	unsigned int crc;
	int len = PAYLOAD_SIZE_BYTES_CRC;

	if (!crc32_table[1]) /* if not already done, */
	CalcInitCRC(); /* build table */

	crc = 0xffffffff; /* preload shift register, per CRC-32 spec */
	for (p = data; len > 0; ++p, --len)
	crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p];

	crc = ~crc; /* transmit complement, per CRC-32 spec */
	*crc32 = crc;

}

FTL_STATUS CalcCompareCRC(UINT32 origCRC, UINT32 newCRC)
{
	if(origCRC == newCRC)
	{
		return CRC_ERROR_NONE;
	}
	else
	{
		return CRC_ERROR;
	}
}
#endif
