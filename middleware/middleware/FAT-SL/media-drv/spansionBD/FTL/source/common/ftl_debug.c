/* file: ftl_debug.c */
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

/*This file will contain API implimentation, to be used for FTL debug in the event of the FTL failure*/
/*The user/customer can use these APIs for reporting back more detailed information on the state of the FTL failure*/
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

#if(FTL_ENABLE_DEBUG_CODE == FTL_TRUE)

#if (DEBUG_ENABLE_LOGGING == FTL_TRUE)
#include "stdio.h"
#include "string.h"
#define DEBUG_MAX_LOG_ELEMENTS   1000
#define DEBUG_TEMP_DATA_BUFFER_SIZE (1024*10)
#define DEBUG_MAX_NUM_SECTORS  (DEBUG_TEMP_DATA_BUFFER_SIZE / 512)
#define SANITY_COUNTER_NUMBER 10000000
extern UINT32 erase_debug_counter;
extern UINT32 write_debug_counter_512;
extern UINT32 write_debug_counter_32;

FTL_DEBUG_LOG_ELEMENT FTLAPICallLog[DEBUG_MAX_LOG_ELEMENTS];
UINT8 TempDataBuffer[DEBUG_TEMP_DATA_BUFFER_SIZE];

UINT32 APICallLogIndex = 0;
#endif
#if (DEBUG_COMPARE_TABLES == FTL_TRUE)
UINT8 debugFlushDone = FTL_FALSE;
#endif  // #if (DEBUG_COMPARE_TABLES == FTL_TRUE)

#if (FTL_DEBUG_STRUCT_SIZE == FTL_TRUE)
//--------------------------
FTL_STATUS FTL_DebugStructSize(void)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT32 structSize = 0; /*4*/
	UINT32 expectedStructSize = 0; /*4*/
	UINT32 arraySize = 0; /*4*/
	UINT32 expectedArraySize = 0; /*4*/
	UINT32 pointerVal = 0; /*4*/
	/*EBLOCK_MAPPING_TABLE*/
	DBG_Printf("\t*****Debug Code Enabled*****:\n\tTo turn off, set FTL_ENABLE_DEBUG_CODE to FTL_FALSE in ftl_if_ex.h \n",0, 0);
	structSize = sizeof(EBLOCK_MAPPING_ENTRY);
	expectedStructSize = EXPECTED_EBLOCK_MAPPING_ENTRY_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_EBLOCK_MAPPING_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(EBlockMappingTable);
	expectedArraySize = EXPECTED_EBLOCK_MAPPING_ENTRY_SIZE * NUM_DEVICES * NUM_EBLOCKS_PER_DEVICE;
	if(arraySize != expectedArraySize)
	{
		return FTL_EBLOCK_MAPPING_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)EBlockMappingTable;
	if((pointerVal % 4) != 0)
	{
		return FTL_EBLOCK_MAPPING_ARRAY_MISALIGNED;
	}
	/*END EBLOCK_MAPPING_TABLE*/
	/*PPA*/
	structSize = sizeof(PPA_MAPPING_ENTRY);
	expectedStructSize = EXPECTED_PPA_MAPPING_ENTRY_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_PPA_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(PPAMappingTable);
	expectedArraySize = EXPECTED_PPA_MAPPING_ENTRY_SIZE * NUM_DEVICES * NUM_EBLOCKS_PER_DEVICE * NUM_PAGES_PER_EBLOCK;
	if(arraySize != expectedArraySize)
	{
		return FTL_PPA_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)PPAMappingTable;
	if((pointerVal % 4) != 0)
	{
		return FTL_PPA_ARRAY_MISALIGNED;
	}
	/*END PPA*/
	/*PPA DIRTY BIT MAP*/
	structSize = sizeof(UINT8);
	expectedStructSize = EXPECTED_PPA_DIRTY_BITMAP_ENTRY_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_PPA_DIRTY_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(PPAMappingTableDirtyBitMap);
	expectedArraySize = EXPECTED_PPA_DIRTY_BITMAP_ENTRY_SIZE * NUM_DEVICES * PPA_DIRTY_BITMAP_DEV_TABLE_SIZE;
	if(arraySize != expectedArraySize)
	{
		return FTL_PPA_DIRTY_ARRAY_SIZE_MISMATCH;
	}
	/*END DIRTY PPA*/
	/*EBLOCK MAP DIRTY BIT MAP*/
	structSize = sizeof(UINT8);
	expectedStructSize = EXPECTED_EBLOCK_MAP_DIRTY_BITMAP_ENTRY_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_EBLOCK_MAP_DIRTY_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(EBlockMappingTableDirtyBitMap);
	expectedArraySize = EXPECTED_EBLOCK_MAP_DIRTY_BITMAP_ENTRY_SIZE * NUM_DEVICES * EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE;
	if(arraySize != expectedArraySize)
	{
		return FTL_EBLOCK_MAP_DIRTY_ARRAY_SIZE_MISMATCH;
	}

	/*END EBLOCK DIRTY BIP MAP*/
	/*FLUSH LOG ARRAY*/
	structSize = sizeof(KEY_TABLE_ENTRY);
	expectedStructSize = EXPECTED_FLUSH_LOG_ENTRY_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_FLUSH_LOG_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(FlushLogEBArray);
	expectedArraySize = EXPECTED_FLUSH_LOG_ENTRY_SIZE * NUM_DEVICES * NUM_FLUSH_LOG_EBLOCKS;
	if(arraySize != expectedArraySize)
	{
		return FTL_FLUSH_LOG_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)FlushLogEBArray;
	if((pointerVal % 4) != 0)
	{
		return FTL_FLUSH_LOG_ARRAY_MISALIGNED;
	}
	/*END FLUSH LOG ARRAY*/
	/*TRANS LOG ARRAY*/
	structSize = sizeof(KEY_TABLE_ENTRY);
	expectedStructSize = EXPECTED_TRANS_LOG_ENTRY_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_TRANS_LOG_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(TransLogEBArray);
	expectedArraySize = EXPECTED_TRANS_LOG_ENTRY_SIZE * NUM_DEVICES * NUM_TRANSACTION_LOG_EBLOCKS;
	if(arraySize != expectedArraySize)
	{
		return FTL_TRANS_LOG_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)TransLogEBArray;
	if((pointerVal % 4) != 0)
	{
		return FTL_TRANS_LOG_ARRAY_MISALIGNED;
	}
	/*END TRANS LOG ARRAY*/
	/*FLUSH LOG ARRAY COUNT*/
	structSize = sizeof(UINT16);
	expectedStructSize = EXPECTED_FLUSH_LOG_ARRAY_COUNT_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_FLUSH_LOG_ARRAY_COUNT_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&FlushLogEBArrayCount[0]);
	if((pointerVal % 2) != 0)
	{
		return FTL_FLUSH_LOG_ARRAY_COUNT_MISALIGNED;
	}
	/*END FLUSH LOG ARRAY COUNT*/
	/*TRANS LOG ARRAY COUNT*/
	structSize = sizeof(UINT16);
	expectedStructSize = EXPECTED_TRANS_LOG_ARRAY_COUNT_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_TRANS_LOG_ENTRY_COUNT_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&TransLogEBArrayCount[0]);
	if((pointerVal % 2) != 0)
	{
		return FTL_TRANS_LOG_ARRAY_COUNT_MISALIGNED;
	}
	/*END TRANS LOG ARRAY COUNT*/
	/*TRANs LOG ARRAY COUNT*/
	structSize = sizeof(UINT32);
	expectedStructSize = EXPECTED_TRANS_LOG_ARRAY_COUNT_SIZE_2;
	if(structSize != expectedStructSize)
	{
		return FTL_TRANS_LOG_ENTRY_COUNT_SIZE_MISMATCH_2;
	}
	arraySize = sizeof(TransLogEBCounter);
	expectedArraySize = EXPECTED_TRANS_LOG_ARRAY_COUNT_SIZE_2 * NUM_DEVICES;
	if(arraySize != expectedArraySize)
	{
		return FTL_TRANS_LOG_ARRAY_COUNT_SIZE_MISMATCH_2;
	}
	pointerVal = (UINT32)TransLogEBCounter;
	if((pointerVal % 4) != 0)
	{
		return FTL_TRANS_LOG_ARRAY_COUNT_MISALIGNED_2;
	}
	/*END TRANS LOG ARRAY COUNT*/
	/*FLUSH LOG ARRAY COUNT*/
	structSize = sizeof(UINT32);
	expectedStructSize = EXPECTED_FLUSH_LOG_ARRAY_COUNT_SIZE_2;
	if(structSize != expectedStructSize)
	{
		return FTL_FLUSH_LOG_ENTRY_COUNT_SIZE_MISMATCH_2;
	}
	arraySize = sizeof(FlushLogEBCounter);
	expectedArraySize = EXPECTED_FLUSH_LOG_ARRAY_COUNT_SIZE_2 * NUM_DEVICES;
	if(arraySize != expectedArraySize)
	{
		return FTL_FLUSH_LOG_ARRAY_COUNT_SIZE_MISMATCH_2;
	}
	pointerVal = (UINT32)FlushLogEBCounter;
	if((pointerVal % 4) != 0)
	{
		return FTL_FLUSH_LOG_ARRAY_COUNT_MISALIGNED_2;
	}
	/*END FLUSH LOG ARRAY COUNT*/
	/*GC NUM*/
	structSize = sizeof(UINT32);
	expectedStructSize = EXPECTED_GC_NUM_STRUCT_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_GC_NUM_ENTRY_SIZE_MISMATCH;
	}
	arraySize = sizeof(GCNum);
	expectedArraySize = EXPECTED_GC_NUM_STRUCT_SIZE * NUM_DEVICES;
	if(arraySize != expectedArraySize)
	{
		return FTL_GC_NUM_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)GCNum;
	if((pointerVal % 4) != 0)
	{
		return FTL_GC_NUM_ARRAY_MISALIGNED;
	}
	/*END GC_NUM*/
	/*GC INFO*/
	structSize = sizeof(GC_INFO);
	expectedStructSize = EXPECTED_GC_INFO_STRUCT_SIZE;
	expectedStructSize += 3; // FUDGE FACTOR: adding becasue of compiler padding. extra is ok. does not cause any problems
	if(structSize != expectedStructSize)
	{
		return FTL_GC_INFO_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&GC_Info);
	if((pointerVal % 4) != 0)
	{
		return FTL_GC_INFO_STRUCT_MISALIGNED;
	}
	/*END GC_INFO*/
	/*GC INFO*/
	structSize = sizeof(UINT16);
	expectedStructSize = EXPECTED_GC_THRESHOLD_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_GC_THRESHOLD_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&GC_THRESHOLD);
	if((pointerVal % 2) != 0)
	{
		return FTL_GC_THRESHOLD_MISALIGNED;
	}
	/*END GC_INFO*/
	/*TransferMapIndexEnd*/
	structSize = sizeof(UINT16);
	expectedStructSize = EXPECTED_TransferMapIndexStart_SIZE;
	if(structSize != expectedStructSize)
	{
		return EXPECTED_TransferMapIndexStart_SIZE;
	}
	pointerVal = (UINT32)(&TransferMapIndexStart);
	if((pointerVal % 2) != 0)
	{
		return FTL_TransferMapIndexStart_MISALIGNED;
	}
	/*End TransferMapIndexEnd*/
	/*Transfer Map*/
	structSize = sizeof(TRANSFER_MAP_STRUCT);
	expectedStructSize = EXPECTED_TRANSFER_MAP_STRUCT_STRUCT_SIZE;
	expectedStructSize += 3; // FUDGE FACTOR: adding becasue of compiler padding. extra is ok. does not cause any problems
	if(structSize != expectedStructSize)
	{
		return FTL_TRANSFER_MAP_STRUCT_SIZE_MISMATCH;
	}
	arraySize = sizeof(transferMap);
	expectedArraySize = EXPECTED_TRANSFER_MAP_STRUCT_STRUCT_SIZE * NUM_TRANSFER_MAP_ENTRIES;
	expectedArraySize += (3 * NUM_TRANSFER_MAP_ENTRIES); // FUDGE FACTOR: adding becasue of compiler padding. extra is ok. does not cause any problems
	if(arraySize != expectedArraySize)
	{
		return FTL_TRANSFER_MAP_STRUCT_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)transferMap;
	if((pointerVal % 4) != 0)
	{
		return FTL_TRANSFER_MAP_STRUCT_ARRAY_MISALIGNED;
	}
	/*END Transfer Map*/
	/*GC move array*/
	structSize = sizeof(UINT8);
	expectedStructSize = EXPECTED_GC_MOVE_ARRAY_STRUCT_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_GC_MOVE_ARRAY_STRUCT_SIZE_MISMATCH;
	}
	arraySize = sizeof(GCMoveArray);
	expectedArraySize = EXPECTED_GC_MOVE_ARRAY_STRUCT_SIZE * NUM_PAGES_PER_EBLOCK;
	if(arraySize != expectedArraySize)
	{
		return FTL_GC_MOVE_ARRAY_SIZE_MISMATCH;
	}
	/*GC move array*/
	/*TRANS LOG*/
	structSize = sizeof(TRANS_LOG_ENTRY);
	expectedStructSize = EXPECTED_TLOG_STRUCT_SIZE;
	expectedStructSize += (LOG_ENTRY_SIZE * (TEMP_B_ENTRIES-1)); // FUDGE FACTOR, need to understand this better!!!
	if(structSize != expectedStructSize)
	{
		return FTL_TLOG_STRUCT_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&TransLogEntry);
	if((pointerVal % 4) != 0)
	{
		return FTL_TLOG_STRUCT_MISALIGNED;
	}
	/*END TRANS LOG*/
	/*TRANS LOGB*/
	structSize = sizeof(UINT16);
	expectedStructSize = EXPECTED_TLOGB_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_TLOGB_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&TranslogBEntries);
	if((pointerVal % 2) != 0)
	{
		return FTL_TLOGB_MISALIGNED;
	}
	/*END TRANS LOGB*/
	/*TRANS LOG LBA*/
	structSize = sizeof(UINT32);
	expectedStructSize = EXPECTED_LAST_LBA_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_LAST_LBA_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)(&LastTransLogLba);
	if((pointerVal % 4) != 0)
	{
		return FTL_LAST_LBA_MISALIGNED;
	}
	/*END TRANS LBA*/
	/*pseudoRPB*/
	structSize = sizeof(UINT8);
	expectedStructSize = EXPECTED_pseudoRPB_STRUCT_SIZE;
	if(structSize != expectedStructSize)
	{
		return FTL_pseudoRPB_STRUCT_SIZE_MISMATCH;
	}
	arraySize = sizeof(pseudoRPB);
	expectedArraySize = EXPECTED_pseudoRPB_STRUCT_SIZE * NUM_DEVICES * VIRTUAL_PAGE_SIZE;
	if(arraySize != expectedArraySize)
	{
		return FTL_pseudoRPB_ARRAY_SIZE_MISMATCH;
	}
	pointerVal = (UINT32)pseudoRPB;
	if((pointerVal % 4) != 0)
	{
		return FTL_pseudoRPB_ARRAY_MISALIGNED;
	}
	/*end pseudoRPB*/
	/*COMPILER WARNING, using pointerVal to make the compiler thing the if is variable*/
	/* obsolete error condition - EBLOCK_MAPPING_ENTRY should be sub-multiple of SECTOR_SIZE
	 pointerVal = 0;
	 if(((SECTOR_SIZE % sizeof(EBLOCK_MAPPING_ENTRY))+ pointerVal) != 0)
	 {
	 return FTL_ERR_INIT_MAP_SIZE;
	 }
	 */
	return status;
}
#endif  // #if (FTL_DEBUG_STRUCT_SIZE == FTL_TRUE)

#if (DEBUG_ENABLE_LOGGING == FTL_TRUE)

void DEBUG_ClearLog(void)
{
	UINT32 index = 0;
	for(index = 0; index < DEBUG_MAX_LOG_ELEMENTS; index++)
	{
		FTLAPICallLog[APICallLogIndex].LBA = EMPTY_DWORD;
		FTLAPICallLog[APICallLogIndex].NB = EMPTY_WORD;
		FTLAPICallLog[APICallLogIndex].operation = EMPTY_BYTE;
	}
	APICallLogIndex = 0;

}

FTL_STATUS DEBUG_InsertLog(UINT32 LBA, UINT32 NB, UINT8 operation)
{
	static UINT8 flag = 0;
	FTL_STATUS status = FTL_ERR_PASS;
	if(APICallLogIndex >= DEBUG_MAX_LOG_ELEMENTS)
	{
		APICallLogIndex = 0;
		if(flag == 0)
		{
			flag = 0x80;
		}
		else
		{
			flag = 0;
		}
	}
	FTLAPICallLog[APICallLogIndex].LBA = LBA;
	FTLAPICallLog[APICallLogIndex].NB = NB;
	FTLAPICallLog[APICallLogIndex].operation = operation + flag;
	APICallLogIndex++;
	return status;
}

FTL_STATUS DEBUG_ReplayLog(UINT32 LBA, UINT32 NB, UINT8 operation)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT32 TempBytesDone = 0;
	FTL_INIT_STRUCT initStruct;
	operation = operation & ((UINT8)0x7F);
	if(NB > DEBUG_MAX_NUM_SECTORS)
	{
		return FTL_ERR_FAIL;
	}
	switch(operation)
	{
		case DEBUG_LOG_WRITE:
		DBG_Printf("Write LBA=%d", LBA, 0);
		DBG_Printf(" NB=%d \n", NB, 0);
		if((status = FTL_DeviceObjectsWrite(TempDataBuffer, LBA, NB, &TempBytesDone)) != FTL_ERR_PASS)
		{
			return(status);
		}
		break;
		case DEBUG_LOG_READ:
		DBG_Printf("Read LBA=%d", LBA, 0);
		DBG_Printf(" NB=%d \n", NB, 0);
		if((status = FTL_DeviceObjectsRead(TempDataBuffer, LBA, NB, &TempBytesDone)) != FTL_ERR_PASS)
		{
			return(status);
		}
		break;
		case DEBUG_LOG_DELETE:
		DBG_Printf("Delete LBA=%d", LBA, 0);
		DBG_Printf(" NB=%d \n", NB, 0);
		if((status = FTL_DeviceObjectsDelete(LBA, NB, &TempBytesDone)) != FTL_ERR_PASS)
		{
			return(status);
		}
		break;
		case DEBUG_LOG_TABLE_FLUSH:
		DBG_Printf("Table Flush = %d", 0,0);
//          if((status = FTL_FlushTableCache()) != FTL_ERR_PASS)
		if((status = FTL_FlushDataCache()) != FTL_ERR_PASS)
		{
			return(status);
		}
		break;
		case DEBUG_LOG_INIT:
		initStruct.allocate = 0;
		initStruct.format = FTL_DONT_FORMAT;
		initStruct.mode = 0;
		initStruct.os_type = 0;
		initStruct.table_storage = 0;
#if (CACHE_RAM_BD_MODULE == FTL_TRUE && CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
		initStruct.total_ram_allowed = gTotal_ram_allowed;
#endif
		DBG_Printf("Init format=%d\n", initStruct.format, 0);
		if((status = FTL_InitAll(&initStruct)) != FTL_ERR_PASS)
		{
			return(status);
		}
		break;
		case DEBUG_LOG_FORCED_GC:
		break;
		default:
		break;
	}
	return status;
}

FTL_STATUS FTL_DEBUG_Replay(UINT8_PTR FileName)
{
	FILE *inputFile;
	FTL_STATUS status = FTL_ERR_PASS;
	UINT32 LBA = EMPTY_DWORD, NB = 1;
	UINT32 fileStatus = 0;
	UINT32 sanityCounter = 0;
	UINT32 sanityReadCounter = 0, sanityWriteCounter = 0, sanityDeleteCounter = 0;
	UINT8 command[25] =
	{	0};
	inputFile = fopen(FileName, "r");
	if(inputFile == NULL)
	{
		return FTL_ERR_FAIL;
	}
	while((fileStatus != EOF) && sanityCounter < SANITY_COUNTER_NUMBER)
	{
//      fileStatus = fscanf(inputFile, "%s %i %i", &command[0], &LBA, &NB); 
		fileStatus = fscanf(inputFile, "%s %i", &command[0], &LBA);
		sanityCounter++;
//      if(strcmp("FTL_DeviceObjectsRead", command) == 0)
		if(strcmp("read", command) == 0)
		{
			status = DEBUG_ReplayLog(LBA, NB, DEBUG_LOG_READ);
			sanityReadCounter++;
		}
//      else if(strcmp("FTL_DeviceObjectsWrite", command) == 0)
		else if(strcmp("write", command) == 0)
		{
			status = DEBUG_ReplayLog(LBA, NB, DEBUG_LOG_WRITE);
			sanityWriteCounter++;
		}
//      else if(strcmp("FTL_DeviceObjectsDelete", command) == 0)
		else if(strcmp("delete", command) == 0)
		{
			status = DEBUG_ReplayLog(LBA, NB, DEBUG_LOG_DELETE);
			sanityDeleteCounter++;
		}
		else if(strcmp("tableFlush", command) == 0)
		{
			status = DEBUG_ReplayLog(LBA, NB, DEBUG_LOG_TABLE_FLUSH);
			sanityDeleteCounter++;
		}
		else
		{
			DBG_Printf("*******unknown command[%d]******* \n",sanityCounter, 0);
		}
		if(status != FTL_ERR_PASS)
		{
			fclose(inputFile);
			return status;
		}

	}
	if(sanityCounter == SANITY_COUNTER_NUMBER)
	{
		fclose(inputFile);
		return FTL_ERR_FAIL;
	}
	fclose(inputFile);
	DBG_Printf("erase count=%d\n", erase_debug_counter, 0);
	DBG_Printf("write_debug_counter_512 count=%d\n", write_debug_counter_512, 0);
	DBG_Printf("write_debug_counter_32 count=%d\n", write_debug_counter_32, 0);
	DBG_Printf("total command count=%d\n", sanityCounter, 0);
	DBG_Printf("total read count=%d\n", sanityReadCounter, 0);
	DBG_Printf("total write count=%d\n", sanityWriteCounter, 0);
	DBG_Printf("total delete count=%d\n", sanityDeleteCounter, 0);
	return status;
}

#endif

#if (DEBUG_COMPARE_TABLES == FTL_TRUE)
// -----------------------
FTL_STATUS DEBUG_FlushTablesAll(void)
{
	FTL_DEV devID = EMPTY_BYTE; /*1*/
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	for(devID = 0; devID < NUM_DEVICES; devID++)
	{
		if((status = DEBUG_FlushDevice(devID)) != FTL_ERR_PASS)
		{
			return status;
		}
	}
	return FTL_ERR_PASS;
}

//--------------------------
FTL_STATUS DEBUG_EraseOp(FTL_DEV devID, UINT16 logicalEBNum)
{
	return FTL_EraseOp(devID, logicalEBNum);
}

//--------------------------
FTL_STATUS DEBUG_FlushDevice(FTL_DEV devID)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT8 numRamTables = 2; /*1*/
	UINT8 flushTypeCnt = 0; /*1*/
	UINT16 dataByteCnt = 0; /*2*/
	UINT32 totalRamTableBytes = 0; /*4*/
	UINT8_PTR ramMappingTablePtr = NULL; /*4*/
	UINT16 logicalEBNum = NUM_EBLOCKS_PER_DEVICE - 1; /*2*/
	FLASH_PAGE_INFO flushStruct =
	{	0, 0,
		{	0, 0}}; /*11*/
	UINT32 flushLocation = 0; /*4*/
	UINT32 sizeofPreviousTable = 0; /*4*/
	UINT32 flashdataByteCnt = 0; /*4*/

	flushLocation = GetPhysicalEBlockAddr(devID, logicalEBNum);
	flushLocation = flushLocation * NUM_PAGES_PER_EBLOCK;
	flushStruct.byteCount = SECTOR_SIZE;
	flushStruct.devID = devID;
	if((status = DEBUG_EraseOp(devID, logicalEBNum)) != FTL_ERR_PASS)
	{
		return status;
	}
	for(flushTypeCnt = 0; flushTypeCnt < numRamTables; flushTypeCnt++)
	{
		dataByteCnt = 0x0;
		if(flushTypeCnt == 1)
		{
			ramMappingTablePtr = (UINT8_PTR) (&EBlockMappingTable[devID][0]);
			sizeofPreviousTable = totalRamTableBytes;
			totalRamTableBytes = sizeof(EBlockMappingTable[devID]);
		}
		else if(flushTypeCnt == 0)
		{
			ramMappingTablePtr = (UINT8_PTR)(&PPAMappingTable[devID]);
			totalRamTableBytes = sizeof(PPAMappingTable[devID]);
			sizeofPreviousTable = 0;
		}
		for(dataByteCnt = 0; dataByteCnt < totalRamTableBytes; dataByteCnt += SECTOR_SIZE)
		{
			flashdataByteCnt = dataByteCnt + sizeofPreviousTable;
			flushStruct.vPage.pageOffset = flashdataByteCnt % VIRTUAL_PAGE_SIZE;
			flushStruct.vPage.vPageAddr = (flashdataByteCnt / VIRTUAL_PAGE_SIZE) + flushLocation;
			if(FLASH_RamPageWriteDataBlock(&flushStruct, (UINT8_PTR)(ramMappingTablePtr + dataByteCnt)) != FLASH_PASS)
			{
				return FTL_ERR_FLASH_WRITE_01;
			}
		}
	}
	return FTL_ERR_PASS;
}

//----------------------------
FTL_STATUS DEBUG_CompareTablesAll(void)
{
	FTL_DEV devID = EMPTY_BYTE; /*1*/
	FTL_STATUS status = FTL_ERR_PASS; /*4*/

	for(devID = 0; devID < NUM_DEVICES; devID++)
	{
		if((status = DEBUG_CompareTables(devID)) != FTL_ERR_PASS)
		{
			DEBUG_CompareTables(devID);
			return status;
		}
	}
	return FTL_ERR_PASS;
}

//----------------------------
FTL_STATUS DEBUG_CompareTables(FTL_DEV devID)
{
	FTL_STATUS status = FTL_ERR_PASS; /*4*/
	UINT8 numRamTables = 2; /*1*/
	UINT8 flushTypeCnt = 0; /*1*/
	UINT16 dataByteCnt = 0; /*2*/
	UINT32 totalRamTableBytes = 0; /*4*/
	UINT8_PTR ramMappingTablePtr = NULL; /*4*/
	UINT16 logicalEBNum = NUM_EBLOCKS_PER_DEVICE - 1; /*2*/
	FLASH_PAGE_INFO flushStruct =
	{	0, 0,
		{	0, 0}}; /*11*/
	UINT32 flushLocation = 0; /*4*/
	UINT32 sizeofPreviousTable = 0; /*4*/
	UINT32 flashdataByteCnt = 0; /*4*/
	UINT8 buffer[512]; /*512*/
	UINT8_PTR ramTablePtr = NULL; /*4*/

	flushLocation = GetPhysicalEBlockAddr(devID, logicalEBNum);
	flushLocation = flushLocation * NUM_PAGES_PER_EBLOCK;
	flushStruct.byteCount = SECTOR_SIZE;
	flushStruct.devID = devID;
	ramTablePtr = &buffer[0];
	for(flushTypeCnt = 0; flushTypeCnt < numRamTables; flushTypeCnt++)
	{
		dataByteCnt = 0x0;
		if(flushTypeCnt == 1)
		{
			ramMappingTablePtr = (UINT8_PTR) (&EBlockMappingTable[devID][0]);
			sizeofPreviousTable = totalRamTableBytes;
			totalRamTableBytes = sizeof(EBlockMappingTable[devID]);
		}
		else if(flushTypeCnt == 0)
		{
			ramMappingTablePtr = (UINT8_PTR)(&PPAMappingTable[devID]);
			totalRamTableBytes = sizeof(PPAMappingTable[devID]);
			sizeofPreviousTable = 0;
		}
		for(dataByteCnt = 0; dataByteCnt < totalRamTableBytes; dataByteCnt += SECTOR_SIZE)
		{
			flashdataByteCnt = dataByteCnt + sizeofPreviousTable;
			flushStruct.vPage.pageOffset = flashdataByteCnt % VIRTUAL_PAGE_SIZE;
			flushStruct.vPage.vPageAddr = (flashdataByteCnt / VIRTUAL_PAGE_SIZE) + flushLocation;
			if((FLASH_RamPageReadDataBlock(&flushStruct, ramTablePtr)) != FLASH_PASS)
			{
				return FTL_ERR_FLASH_READ_14;
			}
			if((status = DEBUG_CompareBuffer(ramTablePtr, ramMappingTablePtr + dataByteCnt, SECTOR_SIZE)) != FTL_ERR_PASS)
			{
				return status;
			}
		}
	}
	return FTL_ERR_PASS;
}

//---------------------------
FTL_STATUS DEBUG_CompareBuffer(UINT8_PTR source1, UINT8_PTR source2, UINT32 size)
{
	UINT32 count = 0; /*4*/

	for(count = 0; count < size; count++)
	{
		if(source1[count] != source2[count])
		{
			return FTL_ERR_FAIL;
		}
	}
	return FTL_ERR_PASS;
}
#endif  // #if (DEBUG_COMPARE_TABLES == FTL_TRUE)

#endif  // #if(FTL_ENABLE_DEBUG_CODE == FTL_TRUE)

#if (DEBUG_CACHE_RAM_BD_MODULE == FTL_TRUE)

// Debug Interface
FTL_STATUS DEBUG_CACHE_EBMCacheIndexToRamMapIndex(void)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT16 index = 0;
	UINT16 logicalEBNum;
	FTL_DEV devID = 0;
	UINT32 EBMramStructPtr = 0;
	UINT32 PPAramStructPtr = 0;
	UINT16 i = 0;
	UINT8 found = FTL_FALSE;
	UINT8 foundDependUp = FTL_FALSE;
	UINT8 foundDependDown = FTL_FALSE;

	UINT16 CounterDirty= 0;
	// UINT16 gCounterDirty;

	for(index = 0; index < NUM_EBLOCK_MAP_INDEX; index++)
	{
		if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
		{
			return status;
		}

		if (CACHE_DIRTY == ebmCacheInfo.cacheStatus)
		CounterDirty++;

		if(CACHE_FREE != ebmCacheInfo.cacheStatus)
		{
			found = FTL_FALSE;
			for (logicalEBNum = 0; logicalEBNum < NUMBER_OF_ERASE_BLOCKS; logicalEBNum++)
			{
				if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
				{
					return status;
				}
				if(FTL_FALSE == ramMapInfo.presentEBM)
				{
					continue;
				}

				if(index == ramMapInfo.ebmCacheIndex)
				{
					found = FTL_TRUE;
#ifdef DEBUG_TEST_ARRAY
					// get
					if(FTL_ERR_PASS != (status = CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr)))
					{
						DBG_Printf("get offset error = 0x%x\n",status,0);
					}

					// compare
					for (i = 0; i < (EBLOCK_MAPPING_ENTRY_SIZE / 4); i++)
					{
						if (EBlockMappingTableTest[logicalEBNum][i] != *((UINT32_PTR)EBMramStructPtr+i))
						{
							DBG_Printf("verify error: Test Mem = 0x%08x", EBlockMappingTableTest[logicalEBNum][i], 0);
							DBG_Printf(" , Cache = 0x%08x\n", *((UINT32_PTR)EBMramStructPtr+i), 0);
						}
					}

					// PPA
					for (i = 0; i < ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) / 4); i++)
					{
						if (PPAMappingTableTest[logicalEBNum][i] != *((UINT32_PTR)PPAramStructPtr+i))
						{
							DBG_Printf("PPA verify error: Test Mem = 0x%08x", PPAMappingTableTest[logicalEBNum][i], 0);
							DBG_Printf(" , Cache = 0x%08x\n", *((UINT32_PTR)PPAramStructPtr+i),0);
						}
					}
#endif
				}
			}
			if (found == FTL_FALSE)
			{
				DBG_Printf("No RamMapIndex found. Pointing to 0x%x \n", index, 0);
			}

			if (CACHE_DEPEND_DOWN == ebmCacheInfo.dependency)
			{
				if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, (UINT16)(index - 1), &ebmCacheInfo)))
				{
					return status;
				}

				if ((CACHE_FREE == ebmCacheInfo.cacheStatus) || (CACHE_DEPEND_UP != ebmCacheInfo.dependency))
				{
					DBG_Printf("Inconsistent dependency. index 0x%x", index, 0);
					DBG_Printf(", index-1 0x%x \n", index-1, 0);
				}

				// More than one crossed LEB on RamMapIndex is error.
				if (foundDependDown == FTL_TRUE)
				{
					DBG_Printf("Duplicate index of DependDown \n", 0, 0);
				}
				else
				{
					foundDependDown = FTL_TRUE;
				}
			}
			else if (CACHE_DEPEND_UP == ebmCacheInfo.dependency)
			{
				if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, (UINT16)(index + 1), &ebmCacheInfo)))
				{
					return status;
				}

				if ((CACHE_FREE == ebmCacheInfo.cacheStatus) || (CACHE_DEPEND_DOWN != ebmCacheInfo.dependency))
				{
					DBG_Printf("Inconsistent dependency. index 0x%x", index, 0);
					DBG_Printf(", index+1 0x%x \n",index + 1, 0);
				}

				// More than one crossed LEB on RamMapIndex is error.
				if (foundDependUp == FTL_TRUE)
				{
					DBG_Printf("Duplicate index of DependUp \n", 0, 0);
				}
				else
				{
					foundDependUp = FTL_TRUE;
				}
			}

		}
	}

	if (gCounterDirty!=CounterDirty)
	{
		DBG_Printf("Inconsistent dirty count. gCounterDirty: 0x%x", gCounterDirty, 0);
		DBG_Printf(", CounterDirty: 0x%x \n", CounterDirty, 0);
	}

	return status;
}

FTL_STATUS DEBUG_CACHE_RamMapIndexToEBMCacheIndex(void)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT16 logicalEBNum = 0;
	FTL_DEV devID = 0;
	UINT32 EBMramStructPtr = 0;
	UINT32 PPAramStructPtr = 0;
	UINT32 i = 0;

	for(logicalEBNum = 0; logicalEBNum < NUMBER_OF_DATA_EBLOCKS; logicalEBNum++)
	{
		if(FTL_ERR_PASS != (status = CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo)))
		{
			return status;
		}

		if(FTL_TRUE == ramMapInfo.presentEBM)
		{
			if(EMPTY_WORD == ramMapInfo.ebmCacheIndex)
			{
				DBG_Printf("DEBUG_CACHE_RamMapIndexToEBMCacheIndex Check Present error: logicalEBNum = 0x%x", logicalEBNum, 0);
				return FTL_ERR_DEBUG_CACHE_RAMMAPINDEX_1;
			}
			if(FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, ramMapInfo.ebmCacheIndex, &ebmCacheInfo)))
			{
				return status;
			}

			if(CACHE_FREE == ebmCacheInfo.cacheStatus)
			{
				DBG_Printf("DEBUG_CACHE_RamMapIndexToEBMCacheIndex Check Present error: logicalEBNum = 0x%x", logicalEBNum, 0);
				return FTL_ERR_DEBUG_CACHE_RAMMAPINDEX_2;
			}

			if (FTL_TRUE == CACHE_IsCrossedEB(devID, logicalEBNum))
			{
				if (CACHE_DEPEND_UP != ebmCacheInfo.dependency)
				{
					DBG_Printf("DEBUG_CACHE_RamMapIndexToEBMCacheIndex CrossEB entry: logicalEBNum = 0x%x", logicalEBNum, 0);
					return FTL_ERR_DEBUG_CACHE_RAMMAPINDEX_3;
				}
			}

#ifdef DEBUG_TEST_ARRAY
			// get
			if(FTL_ERR_PASS != (status = CACHE_GetRAMOffsetEB(devID, logicalEBNum, &EBMramStructPtr, &PPAramStructPtr)))
			{
				DBG_Printf("get offset error = 0x%x\n",status, 0);
			}

			// compare
			for(i = 0;i < (EBLOCK_MAPPING_ENTRY_SIZE/4); i++)
			{
				if (EBlockMappingTableTest[logicalEBNum][i] != *((UINT32_PTR)EBMramStructPtr + i))
				{
					DBG_Printf("verify error: Test Mem = 0x%08x", EBlockMappingTableTest[logicalEBNum][i], 0);
					DBG_Printf(", Cache = 0x%08x\n", *((UINT32_PTR)EBMramStructPtr + i), 0);
				}
			}

			// PPA
			for(i = 0;i < ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)/4); i++)
			{
				if (PPAMappingTableTest[logicalEBNum][i] != *((UINT32_PTR)PPAramStructPtr + i))
				{
					DBG_Printf("PPA verify error: Test Mem = 0x%08x", PPAMappingTableTest[logicalEBNum][i], 0);
					DBG_Printf(", Cache = 0x%08x\n", *((UINT32_PTR)PPAramStructPtr + i), 0);
				}
			}
#endif
		}
	}

	return status;
}

#ifdef DEBUG_TEST_ARRAY
FTL_STATUS DEBUG_CACHE_InsertTestMapping(FTL_DEV devID, UINT16 logicalEBNum, UINT32_PTR testData_ptr, UINT32_PTR testData2_ptr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 count = 0;

	// EBlock 
	for(count = 0; count < (EBLOCK_MAPPING_ENTRY_SIZE/4); count++)
	{
		EBlockMappingTableTest[logicalEBNum][count] = testData_ptr[count];
	}

	// PPA
	for(count = 0; count < ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)/4); count++)
	{
		PPAMappingTableTest[logicalEBNum][count] = testData2_ptr[count];
	}

	return status;
}

FTL_STATUS DEBUG_CACHE_ClearTestMapping(FTL_DEV devID, UINT16 logicalEBNum)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 count = 0;

	// EBlock 
	for(count = 0; count < (EBLOCK_MAPPING_ENTRY_SIZE/4); count++)
	{
		EBlockMappingTableTest[logicalEBNum][count] = EMPTY_DWORD;
	}

	for(count = 0; count < ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)/4); count++)
	{
		PPAMappingTableTest[logicalEBNum][count] = EMPTY_DWORD;
	}

	return status;
}

FTL_STATUS DEBUG_CACHE_CompTestMapping(FTL_DEV devID, UINT16 logicalEBNum, UINT32_PTR EBMramStructPtr, UINT32_PTR PPAramStructPtr)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 count = 0;
	UINT32 EBMramStruct = *EBMramStructPtr;
	UINT32 PPAramStruct = *PPAramStructPtr;

	// EBlock 
	for(count = 0; count < (EBLOCK_MAPPING_ENTRY_SIZE/4); count++)
	{
		if(EBlockMappingTableTest[logicalEBNum][count] != *((UINT32_PTR)EBMramStruct + count))
		{
			DBG_Printf("DEBUG_CACHE_CompTestMapping error EBlock: logical = 0x%x\n", logicalEBNum, 0);
			DBG_Printf("EBlockMappingTableTest[logicalEBNum][count] = 0x%x\n",EBlockMappingTableTest[logicalEBNum][count], 0);
			DBG_Printf("EBMramStructPtr[count] = 0x%x\n", *((UINT32_PTR)EBMramStruct+count), 0);
			return FTL_ERR_DEBUG_CACHE_COMP_MAPPING_1;
		}
	}

	// PPA 
	for(count = 0; count < ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)/4); count++)
	{
		if(PPAMappingTableTest[logicalEBNum][count] != *((UINT32_PTR)PPAramStruct + count))
		{
			DBG_Printf("DEBUG_CACHE_CompTestMapping error EBlock: logical = 0x%x\n", logicalEBNum, 0);
			DBG_Printf("PPAMappingTableTest[logicalEBNum][count] = 0x%x\n",PPAMappingTableTest[logicalEBNum][count], 0);
			DBG_Printf("PPAramStructPtr[count] = 0x%x\n", *((UINT32_PTR)PPAramStruct+count), 0);
			return FTL_ERR_DEBUG_CACHE_COMP_MAPPING_2;
		}
	}
	return status;
}
#endif

#ifdef DEBUG_DATA_CLEAR
FTL_STATUS DEBUG_CACHE_SetEBMCacheFree(FTL_DEV devID, UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	UINT16 count = 0;
	UINT16 ppaNum = 0;

	for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
	{
		EBlockMappingCache[index][count] = EMPTY_BYTE;
	}

	for(ppaNum = 0; ppaNum < PPA_CACHE_TABLE_OFFSET; ppaNum++)
	{
		for(count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
		{
			PPAMappingCache[(index * PPA_CACHE_TABLE_OFFSET) + ppaNum][count] = EMPTY_BYTE;
		}
	}

	return status;
}

FTL_STATUS DEBUG_CACHE_CheckEBMCacheFree(void)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	UINT16 index = 0;
	UINT16 count = 0;
	FTL_DEV devID = 0;

	for (index = 0; index < NUM_EBLOCK_MAP_INDEX; index++)
	{
		if (FTL_ERR_PASS != (status = CACHE_GetEBMCache(devID, index, &ebmCacheInfo)))
		{
			return status;
		}
		if (ebmCacheInfo.cacheStatus == CACHE_FREE)
		{
			for (count = 0; count < FLUSH_RAM_TABLE_SIZE; count++)
			{
				if (EMPTY_BYTE != EBlockMappingCache[index][count])
				{
					DBG_Printf("not free index = 0x%x\n", index, 0);
				}
			}
		}
	}
	return status;
}
#endif

FTL_STATUS DEBUG_CACHE_TABLE_DISPLY(FTL_DEV devID, UINT16 tlogicalEB)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT8 checkFlag = FTL_FALSE;
	UINT16 logicalEBNum = 0;
	UINT16 indexNum = 0;

	DBG_Printf("|================================================================================================================|\n", 0, 0);
	DBG_Printf("| logicalEBNum | indexNum | offset | EB present | PPA present | cacheStatus | dependency | wLRUCount | rLRUCount |\n", 0, 0);
	DBG_Printf("|================================================================================================================|\n", 0, 0);
	for(logicalEBNum = 0; logicalEBNum < NUMBER_OF_ERASE_BLOCKS; logicalEBNum++)
	{

		if(EMPTY_WORD != tlogicalEB)
		{
			logicalEBNum = tlogicalEB;
		}

		checkFlag = FTL_FALSE;
		CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo);
		for(indexNum = 0; indexNum < FLUSH_RAM_TABLE_SIZE; indexNum++)
		{
			if(indexNum == ramMapInfo.ebmCacheIndex && FTL_TRUE == ramMapInfo.presentEBM)
			{
				CACHE_GetEBMCache(devID, indexNum, &ebmCacheInfo);
				checkFlag = FTL_TRUE;
				break;
			}
		}
		if(FTL_TRUE == checkFlag)
		{
			DBG_Printf("|         %4x ",logicalEBNum, 0);
			DBG_Printf("|     %4x ", ramMapInfo.ebmCacheIndex, 0);
			DBG_Printf("|   %4x ", ramMapInfo.indexOffset, 0);
			DBG_Printf("|       %4x ", ramMapInfo.presentEBM, 0);
			DBG_Printf("|        %4x ",ramMapInfo.presentPPA, 0);
			DBG_Printf("|        %4x ", ebmCacheInfo.cacheStatus, 0);
			DBG_Printf("|       %4x ", ebmCacheInfo.dependency, 0);
			DBG_Printf("|      %4x ", ebmCacheInfo.rLRUCount, 0);
			DBG_Printf("|      %4x |\n", ebmCacheInfo.wLRUCount, 0);
		}
		else
		{
			DBG_Printf("|         %4x ",logicalEBNum, 0);
			DBG_Printf("|     %4x ", EMPTY_WORD, 0);
			DBG_Printf("|   %4x ", EMPTY_WORD, 0);
			DBG_Printf("|       %4x ", ramMapInfo.presentEBM, 0);
			DBG_Printf("|        %4x ",ramMapInfo.presentPPA, 0);
			DBG_Printf("|        %4x ", EMPTY_BYTE, 0);
			DBG_Printf("|       %4x ", EMPTY_BYTE, 0);
			DBG_Printf("|      %4x ", EMPTY_BYTE, 0);
			DBG_Printf("|      %4x |\n", EMPTY_BYTE, 0);
		}

		if(EMPTY_WORD != tlogicalEB)
		{
			break;
		}

	}

	DBG_Printf("|================================================================================================================|\n", 0, 0);

	return status;
}

typedef struct _cacheInfoDump
{
	UINT16 number;
	UINT16 logicalEBNum;
	UINT8 presentEBM;
	UINT8 presentPPA;
	UINT16 ebmCacheIndex;
	UINT8 indexOffset;
	UINT8 cacheStatus;
	UINT8 dependency;
	UINT8 wLRUCount;
	UINT8 rLRUCount;
}CACHE_INFO_DUMP, *CACHE_INFO_DUMP_PTR;

#define DUMP_COUNT 500
UINT16 dump_count = 0;
UINT8 dump_first = FTL_FALSE;
FTL_STATUS DEBUG_CACHE_SAVE_DUMP(FTL_DEV devID, UINT16 logicalEBNum,UINT16 index)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_DUMP dumpInfo[DUMP_COUNT];
	CACHE_INFO_EBMCACHE ebmCacheInfo =
	{	0, 0, 0, 0};
	CACHE_INFO_RAMMAP ramMapInfo =
	{	0, 0, 0, 0};
	UINT16 i = 0;

	if(FTL_FALSE == dump_first)
	{
		for(i = 0; i < DUMP_COUNT; i++)
		{
			dumpInfo[dump_count].number = EMPTY_WORD;
			dumpInfo[dump_count].logicalEBNum = EMPTY_WORD;
			dumpInfo[dump_count].presentEBM = EMPTY_BYTE;
			dumpInfo[dump_count].presentPPA = EMPTY_BYTE;
			dumpInfo[dump_count].ebmCacheIndex = EMPTY_WORD;
			dumpInfo[dump_count].indexOffset = EMPTY_BYTE;
			dumpInfo[dump_count].cacheStatus = EMPTY_BYTE;
			dumpInfo[dump_count].dependency = EMPTY_BYTE;
			dumpInfo[dump_count].wLRUCount = EMPTY_BYTE;
			dumpInfo[dump_count].rLRUCount = EMPTY_BYTE;
		}
		dump_first = FTL_TRUE;
	}

	CACHE_GetRamMap(devID, logicalEBNum, &ramMapInfo);
	CACHE_GetEBMCache(devID, index, &ebmCacheInfo);

	dumpInfo[dump_count].number = dump_count;
	dumpInfo[dump_count].logicalEBNum = logicalEBNum;
	dumpInfo[dump_count].presentEBM = ramMapInfo.presentEBM;
	dumpInfo[dump_count].presentPPA = ramMapInfo.presentPPA;
	dumpInfo[dump_count].ebmCacheIndex = ramMapInfo.ebmCacheIndex;
	dumpInfo[dump_count].indexOffset = ramMapInfo.indexOffset;
	dumpInfo[dump_count].cacheStatus = ebmCacheInfo.cacheStatus;
	dumpInfo[dump_count].dependency = ebmCacheInfo.dependency;
	dumpInfo[dump_count].wLRUCount = ebmCacheInfo.wLRUCount;
	dumpInfo[dump_count].rLRUCount = ebmCacheInfo.rLRUCount;

	dump_count++;

	if(dump_count > DUMP_COUNT)
	{
		dump_count = 0;
	}
	return status;
}

FTL_STATUS DEBUG_CACHE_OUTPUT_DUMP(void)
{
	FTL_STATUS status = FTL_ERR_PASS;
	CACHE_INFO_DUMP dumpInfo[DUMP_COUNT];
	UINT16 count = 0;

	DBG_Printf("|================================================================================================================|\n", 0, 0);
	DBG_Printf("| logicalEBNum | indexNum | offset | EB present | PPA present | cacheStatus | dependency | wLRUCount | rLRUCount |\n", 0, 0);
	DBG_Printf("|================================================================================================================|\n", 0, 0);
	for(count = 0; count < DUMP_COUNT; count++)
	{

		if(EMPTY_WORD == dumpInfo[count].number)
		{
			break;
		}

		DBG_Printf("|     %4x  ", dumpInfo[dump_count].number, 0);
		DBG_Printf("|         %4x ", dumpInfo[dump_count].logicalEBNum, 0);
		DBG_Printf("|     %4x ", dumpInfo[dump_count].ebmCacheIndex, 0);
		DBG_Printf("|   %4x ", dumpInfo[dump_count].indexOffset, 0);
		DBG_Printf("|       %4x ", dumpInfo[dump_count].presentEBM, 0);
		DBG_Printf("|        %4x ", dumpInfo[dump_count].presentPPA, 0);
		DBG_Printf("|        %4x ", dumpInfo[dump_count].cacheStatus, 0);
		DBG_Printf("|       %4x ", dumpInfo[dump_count].dependency, 0);
		DBG_Printf("|      %4x ", dumpInfo[dump_count].rLRUCount, 0);
		DBG_Printf("|      %4x |\n",dumpInfo[dump_count].wLRUCount, 0);
	}

	DBG_Printf("|================================================================================================================|\n", 0, 0);

	return status;
}
#endif // #if (DEBUG_CACHE_RAM_BD_MODULE == FTL_TRUE)
