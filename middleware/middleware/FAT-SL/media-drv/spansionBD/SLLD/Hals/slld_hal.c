/* slld_hal_example.c - SLLD Hardware Abstraction Layer example Code */

/**************************************************************************
 * Copyright 2011 Spansion LLC. All Rights Reserved.
 *
 * This software is owned and published by:
 * Spansion LLC, 915 DeGuigne Drive, Sunnyvale, CA 94088 ("Spansion").
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
 * Spansion MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED
 * USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY,
 * FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  Spansion WILL
 * HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR
 * OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS,
 * EVEN IF Spansion HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Copyright notice must be included with
 * this software, whether used in part or whole, at all times.
 */

#include <stdio.h>
#include "slld_targetspecific.h"
#include "slld.h"
#include "slld_hal.h"

#include "board.h"
#include "spifi.h"
#include "spifi_18xx_43xx.h"

void spifi_memory_mode(void) {
  #ifdef CORE_M4
	SCnSCB->ACTLR &= ~2; // disable Cortex write buffer to avoid exceptions when switching back to SPIFI for execution
  #endif

	LPC_SPIFI->STAT = 0x10;	// reset memory mode
	while(LPC_SPIFI->STAT & 0x10);	// wait for reset to complete

	LPC_SPIFI->CTRL = // set up new CTRL register with high speed options
		(0x100 << 0) | 	// timeout
		(0x1 << 16) | 	// cs high, this parameter is dependent on the SPI Flash part and is in SPIFI_CLK cycles. A lower cs high performs faster
		(1 << 29) | 		// receive full clock (rfclk) - allows for higher speeds
		(1 << 30); 			// feedback clock (fbclk) - allows for higher speeds

	// put part in high speed mode (skipping opcodes)
	LPC_SPIFI->DATINTM = 0xa5; // 0xAx will cause part to use high performace reads (skips opcode on subsequent accesses)
	LPC_SPIFI->CMD =
		(0xebul << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
		(0x4 << 21) | 		// frame form indicating opcode and 3 address bytes
		(0x2 << 19) | 		// field form indicating serial opcode and dual/quad other fields
		(0x3 << 16); 			// 3 intermediate data bytes
	while(LPC_SPIFI->STAT & 2); // wait for command to complete

	LPC_SPIFI->MEMCMD =
		(0xebul << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
		(0x6 << 21) | 		// frame form indicating no opcode and 3 address bytes
		(0x2 << 19) | 		// field form indicating serial opcode and dual/quad other fields
		(0x3 << 16); 			// 3 intermediate data bytes

  #ifdef CORE_M4
	SCnSCB->ACTLR |= 2; // reenable Cortex write buffer
  #endif
}

void spifi_command_mode(void) {
	LPC_SPIFI->STAT = 0x10;	// reset memory mode
	while(LPC_SPIFI->STAT & 0x10);	// wait for reset to complete

	LPC_SPIFI->ADDR = 0xffffffff;
	LPC_SPIFI->DATINTM = 0xffffffff;
	LPC_SPIFI->CMD = 		// send all ones for a while to hopefully reset SPI Flash
		(0xfful << 24) | 	// opcode 0xff
		(0x5 << 21) | 		// frame form indicating opcode and 4 address bytes
		(0x0 << 19) | 		// field form indicating all serial fields
		(0x4 << 16); 			// 3 intermediate data bytes
	while(LPC_SPIFI->STAT & 2); // wait for command to complete
}

/*


 static SPIFI_ERR_T read(const SPIFI_HANDLE_T *pHandle, BYTE command, uint32_t addr, uint8_t *readBuff, uint32_t bytes)
 {
 LPC_SPIFI_CHIPHW_T *pSpifiCtrlAddr = (LPC_SPIFI_CHIPHW_T *) pHandle->pInfoData->spifiCtrlAddr;
 SPIFI_ERR_T status = SPIFI_ERR_VAL;
 uint32_t cmdOnlyValue;
 uint32_t cmdValue;
 uint32_t dwords;

 uint32_t *readBuff32 = (uint32_t *) readBuff;

 Limit read to controller data limit in bytes
 if (bytes <= pHandle->pInfoData->maxReadSize) {
 Get the number of dwords to read
 dwords = bytes >> 2;
 bytes -= (dwords << 2);

 if (pHandle->pInfoData->opts & SPIFI_CAP_QUAD_READ) {
 cmdOnlyValue = (SPIFI_CMD_OPCODE(command) |
 SPIFI_CMD_DOUT(0) |
 SPIFI_CMD_INTER(3) |
 SPIFI_CMD_FIELDFORM(SPIFI_FIELDFORM_SERIAL_OPCODE) |
 SPIFI_CMD_FRAMEFORM(SPIFI_FRAMEFORM_OP_4ADDRESS));
 }
 else if (pHandle->pInfoData->opts & SPIFI_CAP_DUAL_READ) {
 cmdOnlyValue = (SPIFI_CMD_OPCODE(command) |
 SPIFI_CMD_DOUT(0) |
 SPIFI_CMD_INTER(1) |
 SPIFI_CMD_FIELDFORM(SPIFI_FIELDFORM_SERIAL_OPCODE) |
 SPIFI_CMD_FRAMEFORM(SPIFI_FRAMEFORM_OP_4ADDRESS));
 }
 Default to single lane mode if no other modes enabled
 else {
 cmdOnlyValue = (SPIFI_CMD_OPCODE(command) |
 SPIFI_CMD_DOUT(0) |
 SPIFI_CMD_INTER(1) |
 SPIFI_CMD_FIELDFORM(SPIFI_FIELDFORM_ALL_SERIAL) |
 SPIFI_CMD_FRAMEFORM(SPIFI_FRAMEFORM_OP_4ADDRESS));
 }

 if (dwords) {
 cmdValue = cmdOnlyValue | SPIFI_CMD_DATALEN(dwords << 2);

 Specify the intermediate data byte (turn off).
 spifi_HW_SetIDATA(pSpifiCtrlAddr, 0xFF);

 Set the address and increment for any remaining bytes
 spifi_HW_SetAddr(pSpifiCtrlAddr, addr);
 addr += (dwords << 2);

 spifi_HW_SetCmd(pSpifiCtrlAddr, cmdValue);
 while (dwords) {
 *readBuff32 = spifi_HW_GetData32(pSpifiCtrlAddr);
 ++readBuff32;
 --dwords;
 }
 spifi_HW_WaitCMD(pSpifiCtrlAddr);
 }

 if (bytes) {
 cmdValue = cmdOnlyValue | SPIFI_CMD_DATALEN(bytes);

 Specify the intermediate data byte (turn off).
 spifi_HW_SetIDATA(pSpifiCtrlAddr, 0xFF);

 spifi_HW_SetAddr(pSpifiCtrlAddr, addr);
 spifi_HW_SetCmd(pSpifiCtrlAddr, cmdValue);

 Read data
 while (bytes) {
 *readBuff = (uint8_t) spifi_HW_GetData8(pSpifiCtrlAddr);
 ++readBuff;
 --bytes;
 }
 spifi_HW_WaitCMD(pSpifiCtrlAddr);
 }
 status = SPIFI_ERR_NONE;
 }
 return status;
 }*/

// ***************************************************************************
//  FLASH_INIT - HAL read function
//
//  input : device_num            device number to which operation will be done
//          command               write a single command byte to flash
//          sys_addr              system address to be used
//          data_buffer           Pointer to the data buffer where to store the read data
//          Number_Of_Read_Bytes  number of bytes to be read
//
//  return value : status of the operation - FAIL or SUCCESS
// ***************************************************************************
FLASH_INITIALIZE()
{
	// enable FBCLK and change defaults
	LPC_SPIFI->CTRL = SPIFI_CTRL_TO(1000) |			// default value is 0xFFFF
										SPIFI_CTRL_CSHI(15) |			// this is default value
										SPIFI_CTRL_RFCLK(1) |			// this is default value
										SPIFI_CTRL_FBCLK(1);
}

// ***************************************************************************
//  FLASH_READ - HAL read function
//
//  input : device_num            device number to which operation will be done
//          command               write a single command byte to flash
//          sys_addr              system address to be used
//          data_buffer           Pointer to the data buffer where to store the read data
//          Number_Of_Read_Bytes  number of bytes to be read
//
//  return value : status of the operation - FAIL or SUCCESS
// ***************************************************************************
SLLD_STATUS FLASH_READ(BYTE command, /* write a single command byte to flash */
		ADDRESS sys_addr, /* system address to be used */
		BYTE *data_buffer, /* Pointer to the data buffer containing data to be written */
		int Number_Of_Read_Bytes /* number of bytes to be read */
)
{
    SLLD_STATUS status = SLLD_OK;
    int data_cycle, Number_Of_Dummy_Bytes = 0;


    // Write the address
    if (sys_addr != ADDRESS_NOT_USED)
    {
    	// put part in high speed mode (skipping opcodes)
    	LPC_SPIFI->DATINTM = 0xa5; // 0xAx will cause part to use high performace reads (skips opcode on subsequent accesses)
    	SCnSCB->ACTLR &= ~2; // disable Cortex write buffer to avoid exceptions when switching back to SPIFI for execution
        switch (command)
        {
            case SPI_DUALIO_HPRD_CMD:
            case SPI_DUALIO_HPRD_4B_CMD:
            {
            	LPC_SPIFI->CMD =
            		(command << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
            		(0x4 << 21) | 		// frame form indicating opcode and 3 address bytes
            		(0x2 << 19) | 		// field form indicating serial opcode and dual/quad other fields
            		(0x1 << 16); 			// 1 intermediate data bytes
            	while(LPC_SPIFI->STAT & 2); // wait for command to complete

            	LPC_SPIFI->MEMCMD =
            		(command << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
            		(0x6 << 21) | 		// frame form indicating no opcode and 3 address bytes
            		(0x2 << 19) | 		// field form indicating serial opcode and dual/quad other fields
            		(0x1 << 16); 			// 1 intermediate data bytes
                break;
            }
            case SPI_QUADIO_HPRD_CMD:
            case SPI_QUADIO_HPRD_4B_CMD:
            {
            	LPC_SPIFI->CMD =
            		(command << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
            		(0x4 << 21) | 		// frame form indicating opcode and 3 address bytes
            		(0x2 << 19) | 		// field form indicating serial opcode and dual/quad other fields
            		(0x3 << 16); 			// 3 intermediate data bytes
            	while(LPC_SPIFI->STAT & 2); // wait for command to complete

            	LPC_SPIFI->MEMCMD =
            		(command << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
            		(0x6 << 21) | 		// frame form indicating no opcode and 3 address bytes
            		(0x2 << 19) | 		// field form indicating serial opcode and dual/quad other fields
            		(0x3 << 16); 			// 3 intermediate data bytes
                break;
            }
            default:
            {
            	LPC_SPIFI->CMD =
            		(command << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
            		(0x1 << 21) | 		// frame form indicating opcode and 3 address bytes
            		(0x0 << 19) | 		// field form indicating serial opcode and dual/quad other fields
            		(0x0 << 14); 			// 1 intermediate data bytes
            	while(LPC_SPIFI->STAT & 2); // wait for command to complete
/*
            	LPC_SPIFI->MEMCMD =
            		(command << 24) | 	// opcode 0xeb Quad IO High Performance Read for Spansion
            		(0x1 << 21) | 		// frame form indicating no opcode and 3 address bytes
            		(0x0 << 19) | 		// field form indicating serial opcode and dual/quad other fields
            		(0x0 << 14); 			// 1 intermediate data bytes
                break; */
            }
        }
        SCnSCB->ACTLR |= 2; // reenable Cortex write buffer
    }

    // Read the data
    if (Number_Of_Read_Bytes != 0)
    {
        switch (command)
        {
            case SPI_DUALIO_RD_CMD:
            case SPI_DUALIO_RD_4B_CMD:
            case SPI_DUALIO_HPRD_CMD:
            case SPI_DUALIO_HPRD_4B_CMD:
            {
                // Read the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Read_Bytes; data_cycle++)
                    *(data_buffer + data_cycle) = *(volatile char*)&LPC_SPIFI->DAT16;
                break;
            }
            case SPI_QUADIO_RD_CMD:
            case SPI_QUADIO_RD_4B_CMD:
            case SPI_QUADIO_HPRD_CMD:
            case SPI_QUADIO_HPRD_4B_CMD:
            {
                // Read the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Read_Bytes; data_cycle++)
                    *(data_buffer + data_cycle) = *(volatile char*)&LPC_SPIFI->DAT32;
                break;
            }
            default:
            {
            	unsigned long a = LPC_SPIFI->DAT8;
                // Read the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Read_Bytes; data_cycle++)
                    *(data_buffer + data_cycle) = *(volatile char*)&LPC_SPIFI->DAT8;
                break;
            }
        }
    }

    return(status);
}

/*
 static SPIFI_ERR_T wrt(const SPIFI_HANDLE_T *pHandle, BYTE command, uint32_t addr, const uint8_t *writeBuff, const uint32_t bBytes)
 {
 SPIFI_ERR_T status = SPIFI_ERR_VAL;

 uint16_t stat;
 uint32_t dwords;
 uint32_t cmdOnlyValue;
 volatile uint32_t sendBytes;
 uint32_t *writeBuff32;

 uint32_t bytes = bBytes;

 LPC_SPIFI_CHIPHW_T *pSpifiCtrlAddr = (LPC_SPIFI_CHIPHW_T *) pHandle->pInfoData->spifiCtrlAddr;

 if (bytes <= pHandle->pInfoData->pageSize) {
 status = SPIFI_ERR_NONE;
 status = spifiPrvCheckWriteState(pHandle);
 if (status == SPIFI_ERR_NONE) {

 spifi_HW_GetStat(pSpifiCtrlAddr);           // Check status

 pHandle->pFamFx->devClearStatus(pHandle);        // Clear status

 spifiPrvSetWREN(pSpifiCtrlAddr);   // Write enable

 // Write the address
 if (addr != ADDRESS_NOT_USED)
 {
 spifi_HW_SetAddr(pSpifiCtrlAddr, addr);
 }

 if (pHandle->pInfoData->pDeviceData->caps & SPIFI_CAP_4BYTE_ADDR) {
 cmdOnlyValue =  ( SPIFI_CMD_OPCODE(command) |
 SPIFI_CMD_FIELDFORM(SPIFI_FIELDFORM_ALL_SERIAL) |
 SPIFI_CMD_FRAMEFORM(SPIFI_FRAMEFORM_OP_4ADDRESS));
 }
 else {  Setup for a 3 Byte address erase
 cmdOnlyValue = ( SPIFI_CMD_OPCODE(command) |
 SPIFI_CMD_FIELDFORM(SPIFI_FIELDFORM_ALL_SERIAL) |
 SPIFI_CMD_FRAMEFORM(SPIFI_FRAMEFORM_OP_3ADDRESS));
 }

 // Write the data
 if (bytes != 0)
 {
 Get the number of dwords to write
 dwords = bytes >> 2;

 process by bytes if amount isn't even number of dwords
 if (bytes & 0x3) {

 Only clear status if the device requires it and set write enable
 pHandle->pFamFx->devClearStatus(pHandle);
 spifiPrvSetWREN(pSpifiCtrlAddr);

 // Write the address
 if (addr != ADDRESS_NOT_USED)
 {
 spifi_HW_SetAddr(pSpifiCtrlAddr, addr);
 }

 spifi_HW_SetCmd(pSpifiCtrlAddr, cmdOnlyValue | SPIFI_CMD_DATALEN(bytes));
 Write data
 while (bytes) {
 spifi_HW_SetData8(pSpifiCtrlAddr, *writeBuff);
 ++writeBuff;
 --bytes;
 }
 spifi_HW_WaitCMD(pSpifiCtrlAddr);
 }
 else if (dwords) {
 uint32_t cmdValue = cmdOnlyValue | SPIFI_CMD_DATALEN(dwords << 2);

 writeBuff32 = (uint32_t*) writeBuff;

 Only clear status if the device requires it and set write enable
 pHandle->pFamFx->devClearStatus(pHandle);
 spifiPrvSetWREN(pSpifiCtrlAddr);

 Set address and increment for any remaining
 // Write the address
 if (addr != ADDRESS_NOT_USED)
 {
 spifi_HW_SetAddr(pSpifiCtrlAddr, addr);
 }

 Finally send command and write the data
 spifi_HW_SetCmd(pSpifiCtrlAddr, cmdValue);
 while (dwords) {
 spifi_HW_SetData32(pSpifiCtrlAddr, *writeBuff32);
 ++writeBuff32;
 --dwords;
 }
 spifi_HW_WaitCMD(pSpifiCtrlAddr);
 }
 } else{
 Only clear status if the device requires it and set write enable
 pHandle->pFamFx->devClearStatus(pHandle);
 spifiPrvSetWREN(pSpifiCtrlAddr);

 // Write the address
 if (addr != ADDRESS_NOT_USED)
 {
 spifi_HW_SetAddr(pSpifiCtrlAddr, addr);
 }

 spifi_HW_SetCmd(pSpifiCtrlAddr, cmdOnlyValue | SPIFI_CMD_DATALEN(bytes));

 spifi_HW_WaitCMD(pSpifiCtrlAddr);
 }

 // If blocking is disabled, exit now
 if ((pHandle->pInfoData->opts & SPIFI_OPT_NOBLOCK) == 0) {
 // Device wait for device to become ready
 spifiPrvWaitUnBusy(pHandle);

 // Read status and check error bits
 stat = spifiFamFxGetDeviceStatus(pHandle, 0);
 if ((stat & SPIFI_STAT_ERASEERR) != 0) {
 status = SPIFI_ERR_ERASEERR;
 } else if ((stat & SPIFI_STAT_PROGERR) != 0) {
 status = SPIFI_ERR_PROGERR;
 } else if (((stat & SPIFI_STAT_FULLLOCK) != 0) ||
 ((stat & SPIFI_STAT_PARTLOCK) != 0) ||
 ((stat & SPIFI_STAT_ISWP) != 0)) {
 status = SPIFI_ERR_LOCKED;
 }
 }
 }
 }
 return status;
 }
 */

// ***************************************************************************
//  FLASH_WRITE - HAL write function
//
//  input : device_num               device number to which operation will be done
//          command                  write a single command byte to flash
//          sys_addr                 system address to be used
//          data_buffer              Pointer to the data buffer where to store the written data
//          Number_Of_Written_Bytes  number of bytes to be written
//
//  return value : status of the operation - FAIL or SUCCESS
// ***************************************************************************
SLLD_STATUS FLASH_WRITE(BYTE command, /* write a single command byte to flash */
		ADDRESS sys_addr, /* system address to be used */
		BYTE *data_buffer, /* Pointer to the data buffer containing data to be written */
		int Number_Of_Written_Bytes /* number of bytes to be written */
)
{
	SLLD_STATUS status = SLLD_OK;
	volatile uint32_t sendBytes;

	int data_cycle;

	// Write the address
	if (sys_addr != ADDRESS_NOT_USED)
	{
		spifi_command_mode();
	}

	LPC_SPIFI->DATINTM = 0x0; // next read command will remove high performance mode
	LPC_SPIFI->ADDR = sys_addr;

	// Write the data
	switch (command)
	{
		case SPI_QPP_CMD:
		case SPI_QPP_4B_CMD:
		case SPI_P4E_CMD:
		case SPI_P4E4_CMD:
		{
			LPC_SPIFI->CMD = (command << 24) | // opcode 0x32/0x34 Quad Page Programming for Spansion
					(0x4 << 21) | 	// frame form indicating opcode and 3 address bytes
					(0x1 << 19) | 	// field form indicating quad data field, others serial
					(0x0 << 16) | 	// 0 intermediate data bytes
					(0x1 << 15) | 	// dout indicates that it is a write
					(256);			// datalen
			break;
		}

		default:
		{
			LPC_SPIFI->CMD = (command << 24) | // opcode 0x02 Page Programming for Spansion
					(0x1 << 21) | 	// frame form indicating opcode and 3 address bytes
					(0x0 << 19) |	// field form indicating all serial
					(0x0 << 16) | 	// 0 intermediate data bytes
					(0x1 << 14) | 	// dout indicates that it is a write
					(256); 			// datalen
			break;
		}
	}

	// Write the data using the relevant mode
	for (data_cycle = 0; data_cycle < Number_Of_Written_Bytes; data_cycle++)
		*(volatile int*) &LPC_SPIFI->DAT32 = *(data_buffer + data_cycle);


	return (status);
}

/*****************************************************************************/
