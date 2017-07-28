// file: Flash.h
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

#ifndef FLASH_H
#define FLASH_H

#include "SystemDef.h"

//======= defines =======
typedef struct _vPage
{
	UINT32 vPageAddr;
	UINT16 pageOffset;
} VIRTUAL_PAGE, *VIRTUAL_PAGE_PTR;

typedef struct _Flash_Page_Info
{
	UINT8 devID;
	UINT32 byteCount;
	VIRTUAL_PAGE vPage;
} FLASH_PAGE_INFO, *FLASH_PAGE_INFO_PTR;

typedef unsigned long FLASH_STATUS;

#define FLASH_BASE            (0x14000000L)
#define FLASH_PASS            (FLASH_BASE + 0)
#define FLASH_FAIL            (FLASH_BASE + 1)
#define FLASH_UNKNOWN         (FLASH_BASE + 2)
#define FLASH_PROGRAM_FAIL    (FLASH_BASE + 3)
#define FLASH_ERASE_FAIL      (FLASH_BASE + 4)
#define FLASH_LOCKED          (FLASH_BASE + 5)
#define FLASH_PARAM           (FLASH_BASE + 6)
#define FLASH_TIMEOUT_FAIL    (FLASH_BASE + 7)
#define FLASH_ECC_FAIL        (FLASH_BASE + 8)
#define FLASH_LOAD_FAIL       (FLASH_BASE + 9)

//======= Flash Functions ==========

//----- FLASH_DeviceBase --------
// This function returns base address of device.
UINT32 FLASH_DeviceBase (UINT8 devID);

//----- FLASH_Init ---------
// Called when FTL is initializing
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS FLASH_Init (void);

//----- FLASH_Shutdown --------
// Called when FTL is shutting down
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS FLASH_Shutdown (void);

//------- FLASH_PageLoad ------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Load a page from the Flash Media into the Flash RAM page buffer
//   to be read later.
// Do not wait for load to finish.
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_PageLoad (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_RamPageReadDataBlock ----------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Read data from the Flash RAM page buffer to the System RAM using DMA
// Do not wait for the data to be transfered.
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL, FLASH_LOAD_FAIL, or FLASH_PARAM
FLASH_STATUS FLASH_RamPageReadDataBlock (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_RamPageReadMetaData ----------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Read data from the Flash RAM page buffer to the System RAM using PIO
// Return after data has been transfered.
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL, FLASH_LOAD_FAIL, or FLASH_PARAM
FLASH_STATUS FLASH_RamPageReadMetaData (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_RamPageReadSpareData ----------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Read data from the Flash RAM page buffer to the System RAM using PIO
// Return after data has been transfered.
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL, FLASH_LOAD_FAIL, or FLASH_PARAM
FLASH_STATUS FLASH_RamPageReadSpareData (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_RamPageWriteCMD ----------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Write data to the Flash RAM page buffer from the System RAM using DMA
// Do not wait for the data to be transfered.
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_RamPageWriteCMD (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------ FLASH_RamPageWriteFFPad ---------
// Block until ONFI bus is idle and Flash Device is ready.
// Write data to the Flash RAM page buffer using PIO and pad the rest 
//   of the page with 0xFF.
// Block until data is actually transfered.
// THIS FUNCTION ASSUMES THAT THE PAGE HAS NOT BEEN PREVIOUSLY WRITTEN TO.
// NOTE: pageOffset and byteCount must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_RamPageWriteFFPad (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_PageCommit ----------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Commit a page in the Flash RAM page buffer to be written to the Flash Media
// Do not wait for device to be ready.
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: device selection (0 to (NUMBER_OF_DEVICES - 1))
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_PageCommit (UINT8 devID);

//------- FLASH_RamPageWriteDataBlock -------
// Block until ONFI bus is idle and Flash Device is ready.
// Write data to the Flash RAM page buffer from the System RAM using DMA.
// Do not wait for the data to be transfered.
// THIS FUNCTION ASSUMES THAT THE PAGE HAS BEEN LOADED WITH
//    FLASH_PageLoadForMove().
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_RamPageWriteDataBlock (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_RamPageWriteMetaData -------
// Block until ONFI bus is idle and Flash Device is ready.
// Write data to the Flash RAM page buffer from the System RAM using PIO.
// Block until data is actually transfered.
// THIS FUNCTION ASSUMES THAT THE PAGE HAS BEEN LOADED WITH
//    FLASH_PageLoadForMove()
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_RamPageWriteMetaData (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_RamPageWriteSpareData -------
// Block until ONFI bus is idle and Flash Device is ready.
// Write data to the Flash RAM page buffer from the System RAM using PIO.
// Block until data is actually transfered.
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Param 2: pointer to the System RAM
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_RamPageWriteSpareData (FLASH_PAGE_INFO *pageInfo, UINT8 *dataBuf);

//------- FLASH_Erase ----------
// Block until the ONFI bus is idle and the Flash Device is ready
// Erase an EBlock inside the Flash Device
// Do not wait for device to come ready
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_Erase (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_PageLoadForMove -------
// Block until the ONFI bus is idle and the Flash Device is ready.
// Load a page from the Flash Media into the Flash RAM page buffer.
// Do not wait for load to finish.
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL, FLASH_LOAD_FAIL, or FLASH_PARAM
FLASH_STATUS FLASH_PageLoadForMove (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_PageCommitForMove ------
// Assume the device is already ready.
// Commit a page in the Flash RAM page buffer to be written to the Media
//   to a different address.
// Do not wait for the device to become ready.
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_PageCommitForMove (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_SuspendErase ----------
// Block until the ONFI bus is idle
// Suspend erase an EBlock inside the Flash Device
// Do not wait for device to come ready
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_SuspendErase (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_ResumeErase ----------
// Block until the ONFI bus is idle
// Resume suspended erase an EBlock inside the Flash Device
// Do not wait for device to come ready
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_ResumeErase (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_CheckReady ----------
// This functions returns ready/busy status of Flash Device
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL or FLASH_PARAM
FLASH_STATUS FLASH_CheckReady (FLASH_PAGE_INFO *pageInfo);

//------ FLASH_WaitForFlashXferComplete ----
// This function will wait until the previous command has finished
// Returns: FLASH_PASS or FLASH_TIMEOUT_FAIL
FLASH_STATUS FLASH_WaitForFlashXferComplete (void);

//------ FLASH_WaitForFlashNotBusy -----
// This function will wait for the selected device to come ready
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: device selection (0 to (NUMBER_OF_DEVICES - 1))
// Returns: FLASH_PASS or FLASH_TIMEOUT_FAIL
FLASH_STATUS FLASH_WaitForFlashNotBusy (UINT8 devID);

//------ FLASH_ReadStatus -----------
// Block until the device is ready
// Read status of the Flash and return the status
// NOTE: pageOffset of pageInfo and byteCount of pageInfo must be divisable by two
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_PROGRAM_FAIL, FLASH_ERASE_FAIL, 
//          FLASH_FAIL, or FLASH_PARAM
FLASH_STATUS FLASH_ReadStatus (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_CheckEmpty ----
// This function will check whether data is empty
// Param 1: pointer to the data in system RAM (32bit aligned)
// Param 2: size of the data in system RAM
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS FLASH_CheckEmpty (UINT8 *dataBuf, UINT32 size);

//------- FLASH_CheckDefectEBlock ----
// This function will check whether the eblock is a defect eblock
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_FAIL
FLASH_STATUS FLASH_CheckDefectEBlock (FLASH_PAGE_INFO *pageInfo);

//------- FLASH_MarkDefectEBlock ----
// This function will mark the eblock as a defect eblock
// Param 1: pointer to page info structure
// Returns: FLASH_PASS, FLASH_MARK_DEFECT_FAIL
FLASH_STATUS FLASH_MarkDefectEBlock (FLASH_PAGE_INFO *pageInfo);

//------ FLASH_GetReadyDeviceID -----
// This function will set the specified variable to a bit
//   mask of the devices that have caused a ready interrupt
//   since the last time this function has been called.
// If the system does not support interrupts, then the bit
//   mask will be set to the last device that was sent a command.
// Param 1: pointer to variable to be set
// Returns: FLASH_PASS
FLASH_STATUS FLASH_GetReadyDeviceID (UINT8 *bitmask);

//------ FLASH_GetXferDoneDeviceID ----
// This function will set the specified variable to the 
//   ID of the device that has caused a transfer done interrupt.
// If the system does not support interrupts, then the variable
//   will be set to the last device that was sent a command.
// Param 1: pointer to variable to be set
// Returns: FLASH_PASS
FLASH_STATUS FLASH_GetXferDoneDeviceID (UINT8 *devID);

//------ FLASH_GetSectorReady ------
// Prepare to transfer data to a sector (start ECC calculation)
// Param 1: pointer to the data in system RAM
// Param 2: pointer to the ECC in system RAM
// Param 3: FTL_FLASH_TO_RAM or FTL_RAM_TO_FLASH
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS FLASH_GetSectorReady (UINT8 *sysRAM, UINT8 *sysECCRAM, UINT8 dataDirection);

//------- FLASH_IndicateSectorDone ----
// Finish transfering data for a sector (end ECC calculation)
// Param 1: pointer to the data in system RAM
// Param 2: pointer to the ECC in system RAM
// Param 3: FTL_FLASH_TO_RAM or FTL_RAM_TO_FLASH
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS FLASH_IndicateSectorDone (UINT8 *sysRAM, UINT8 *sysECCRAM, UINT8 dataDirection);

//====== Debug Functions ======

//------ DBG_PrintStringMsg -----
// Print a message to the user through the selected Port
// The current implimentation uses the serial port
// Param 1: pointer to string to print
// Param 2: port number if needed
// Returns: FLASH_PASS
FLASH_STATUS DBG_PrintStringMsg (char* MessagePointer, UINT32 port);

//------ DBG_Printf --------
// Format and print a message to the user through the selected Port.
// The numeric parameter is formatted using the string simular to
//    the printf() function.
// Param 1: pointer to format string
// Param 2: data
// Param 3: port number if needed
// Returns: FTL_ERR_PASS
FLASH_STATUS DBG_Printf (char* FormatPointer, UINT32 data, UINT32 port);

//========= Interrupt Functions =========

//------ INT_Disable -----
// Disable the selected Interrupt
// The current implimentation disables both FIQ and INT Interrupts
// Param 1: Type of interrupt to disable
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS INT_Disable (UINT32 InterruptType);

//------ INT_Enable ------
// Enable the selected Interrupt
// The current implimentation enables both FIQ and INT Interrupts
// Param 1: Type of interrupt to enable
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS INT_Enable (UINT32 InterruptType);

//------ INT_Generate ------
// Generate the selected Interrupt
// Param 1: device to associate the interrupt with
// Param 2: Type of interrupt to generate
// Returns: FLASH_PASS
FLASH_STATUS INT_Generate (UINT8 devId, UINT32 InterruptType);

//======= Timer Functions ===========

//------ TIME_Get -----
// Get the tick count from the specified timer
// The current implimentation uses the standard C function clock()
// Param 1: selected timer
// Param 2: pointer to variable to receive the count 
// Returns: FLASH_PASS or FLASH_FAIL
FLASH_STATUS TIME_Get (UINT32 TimerId, UINT32* TimerValue);

//------ TIME_TimeMark -----
// Signal the start or end of operations
// Param 1: operation
// Param 2: TM_START or TM_STOP
// Returns: FLASH_PASS
FLASH_STATUS TIME_TimeMark (UINT8 op, UINT8 flag);

#endif
