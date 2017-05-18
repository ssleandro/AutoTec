/* slld.c - Source Code for Spansion SPI Flash's Low Level Driver */

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
#ifndef SLLD_UTIL_H_
#define SLLD_UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "slld.h"

enum BootTypes{
    BT_UNIFORM_NONE     = 0,
    BT_SMALL_TOP_BOTTOM = 1,
    BT_SMALL_BOTTOM     = 2,
    BT_SMALL_TOP        = 3,
    BT_UNIFORM_BOTTOM   = 4,
    BT_UNIFORM_TOP      = 5,
    BT_UNIFORM_WP_ALL   = 6,
    BT_UNIFORM_WP_TOP_BOTTOM = 7,
    BT_LIMIT            = 8
};
enum EraseMode{
    PARAM_4K_SECTOR_ERASE = 0,
    PARAM_4K_SECTOR_4B_ERASE,
    PARAM_8K_SECTOR_ERASE,
    PARAM_8K_SECTOR_4B_ERASE,
    SECTOR_ERASE,
    SECTOR_4B_ERASE,
    BULK_ERASE, /*Chip erase for FL1K*/
    BULK_ERASE1,/*Chip erase for FL1K*/
    BLOCK_ERASE,
};
enum PgrgMode{
    PROG_PAGE = 0,
    PROG_PAGE_4B,
    PROG_PAGE_QUAD,
    PROG_PAGE_QUAD_4B,
};
enum ReadMode{
    NORMAL_READ = 0,
    NORMAL_4B_READ,
    FAST_RAED,
    FAST_4B_RAED,
    DUAL_OUT_RAED,
    DUAL_4BOUT_RAED,
    QUAD_OUT_RAED,
    QUAD_4BOUT_RAED,
    DUAL_IO_RAED,
    DUAL_4BIO_RAED,
    QUAD_IO_RAED,
    QUAD_4BIO_RAED,
    DDR_FAST_RAED,
    DDR_FAST_4B_RAED,
    DDR_DURAL_IO_RAED,
    DDR_DURAL_4B_IO_RAED,
    DDR_QUAD_IO_RAED,
    DDR_QUAD_4B_IO_RAED
};
enum Sector_size{
    Size4K = 4096,
    Size8K = 8192
};
enum QuadModeCtrl{
    QUAD_ENABLE = 0,
    QUAD_DISABLE
};
enum QuadModeAllCtrl{
    QUAD_ALL_ENABLE = 0,
    QUAD_ALL_DISABLE
};
#ifdef FS_S
enum BurstWrapModeCtrl{
    BURST_ENABLE_W8  = 0x10,
    BURST_ENABLE_W16 = 0x11,
    BURST_ENABLE_W32 = 0x12,
    BURST_ENABLE_W64 = 0x13,
    BURST_DISABLE    = 0x00
};
#else
enum BurstWrapModeCtrl{
    BURST_ENABLE_W8  = 0x00,
    BURST_ENABLE_W16 = 0x20,
    BURST_ENABLE_W32 = 0x40,
    BURST_ENABLE_W64 = 0x60,
    BURST_DISABLE    = 0x10
};
#endif
enum SecurityRegAddr{
    SECURITY_REG0  = 0x000000,
    SECURITY_REG1 =  0x001000,
    SECURITY_REG2 =  0x002000,
    SECURITY_REG3 =  0x003000,
};
typedef struct{
    DWORD type;
    DWORD regions;
} BTRC;

typedef struct{
    char name[80];
    unsigned int Value;
}Name_t;

#define CFI_SPROGTO         0x1F
#define CFI_BPROGTO         0x20
#define CFI_SECERASETO      0x21
#define CFI_CHIPERASETO     0x22
#define CFI_SPROGTOMAX      0x23
#define CFI_BPROGTOMAX      0x24
#define CFI_SECERASETOMAX   0x25
#define CFI_CHIPERASETOMAX  0x26

#define CFI_DEVSIZE         0x27
#define CFI_MULTIBYTEWRITE  0x2A
#define CFI_REGIONCOUNT     0x2C

#define CFI_PRIMARY         0x40
#define CFI_PRIVERMAJOR     0x43
#define CFI_PRIVERMINOR     0x44

#define CFI_ERASESUSPEND    0x46
#define CFI_SECTORPROTECT   0x47

#define CFI_ADVSECPROTECT   0x49

#define CFI_SIMULOP         0x4A
#define CFI_BURSTMODE       0x4B
#define CFI_PAGEMODE        0x4C
#define CFI_BOOTFLAG        0x4F
#define CFI_PROGSUSPEND     0x50

#define CFI_UNLOCKBYPASS    0x51

#define CFI_SECSISIZE       0x52

#define CFI_ERASESUSPTO     0x55
#define CFI_PROGRAMSUSPTO   0x56
#define CFI_BANKCOUNT       0x57

typedef struct{
    unsigned int SectorSize;
    unsigned int SectorAddr;
}SectorInfo;

typedef struct{
    unsigned int BlockSize;
    unsigned int BlockAddr;
    unsigned int BlockSectorNum;
}BlockInfo;

typedef struct{
    unsigned int BankSize;
    unsigned int BankAddr;
}BankInfo;

typedef struct{
    unsigned int SecotrNumber;
    unsigned int SectorSize;
    unsigned int RegionAddr;
}RegionInfo;

typedef struct{
    /*Device ID*/
    unsigned int DevId;
    /*Boot Type */
    unsigned int BootType;
    /*total flash memory size*/
    unsigned int Memsize;
    /*page information*/
    unsigned int PageSize;
    /*sector information*/
    unsigned int SectorTotalNumber;
    unsigned int MaxSectorSize;
    unsigned int IsBottom; /*where small sector size located*/
    SectorInfo *pSectorArray;

    /*block information*/
    unsigned int BlockTotalNumber;
    BlockInfo *pBlockArray;
    /*bank information*/
    unsigned int BankTotalNumber;
    BankInfo *pBankArray;
    /*Region information*/
    unsigned int RegionTotalNumber;
    RegionInfo *pRegionArray;
    /*if EHP support */
    unsigned int SupportEHP;
}DeviceInfo;
SLLD_STATUS slld_ProbeDevice();
DeviceInfo* slld_GetGeoInfo();
SLLD_STATUS slld_EraseFlash(ADDRESS addr,BYTE mode);
SLLD_STATUS slld_ProgramFlash(ADDRESS addr, FLASHDATA *buf ,BYTECOUNT len, unsigned int LimitSize, BYTE mode);
SLLD_STATUS slld_ReadFlash(ADDRESS addr,FLASHDATA *buf ,BYTECOUNT len,unsigned int LimitSize,BYTE mode);
SLLD_STATUS slld_IfSecErased (ADDRESS SecAddr , BYTECOUNT SecSize);
SLLD_STATUS slld_QuadModeCtrl(BYTE mode);
SLLD_STATUS slld_QuadAllModeCtrl(BYTE mode);
SLLD_STATUS slld_SetLatencyCode(BYTE value);
#ifdef INCL_SPI_BURSTWRAPCMD
SLLD_STATUS slld_SetBurstWrapMode(BYTE mode);
#endif
#if (defined(FL_1K)|| defined(FL_K))
SLLD_STATUS slld_SecurityRegErase(ADDRESS addr);
#endif
#ifndef REMOVE_ESSCmd
SLLD_STATUS slld_EvaluateEraseStatus(ADDRESS addr);
#endif
#endif /* SLLD_UTIL_H_ */
