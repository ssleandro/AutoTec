// file: ftl_def.h
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

#ifndef FTL_DEF_H
#define FTL_DEF_H

#include "ftl_if_ex.h"
#include "ftl_if_in.h"

#define FTL_RELEASE_NAME "FTL Version:"
#define FTL_RELEASE_VERSION "04.00.05"
#define FTL_FLASH_IMAGE_VERSION "01.02.11"
#define FTL_RELEASE_DATE "2015.10.30"
#define FTL_RELEASE_TIME "09.00 PM PST"
//#define FTL_RELEASE_DATE __DATE__
//#define FTL_RELEASE_TIME __TIME__
#define PRINT_VERSION DBG_PrintStringMsg(FTL_RELEASE_NAME " " FTL_RELEASE_VERSION " " FTL_RELEASE_DATE " " FTL_RELEASE_TIME, 0)

#define FTL_BITS_PER_PAGE            (2)
#define FTL_BITS_PER_BYTE            (8)

#define NUM_WORDS_OF_VERSION         (4)
#define TOTAL_BITS_PER_EB_NEEDED     (NUM_PAGES_PER_EB * FTL_BITS_PER_PAGE)
#define NUMBER_OF_BYTES_NEEDED       (TOTAL_BITS_PER_EB_NEEDED / FTL_BITS_PER_BYTE)
#define EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE   (NUMBER_OF_BYTES_NEEDED)
#define NUM_PAGES_PER_EBLOCK         (EBLOCK_SIZE / VIRTUAL_PAGE_SIZE)
#define DEVICE_SIZE_IN_PAGES         (NUM_EBLOCKS_PER_DEVICE * NUM_PAGES_PER_EBLOCK)
#define DEVICE_SIZE_IN_BYTES         (DEVICE_SIZE_IN_PAGES * VIRTUAL_PAGE_SIZE)
#define NUM_SECTORS_PER_PAGE         (VIRTUAL_PAGE_SIZE / SECTOR_SIZE)
#define SYSTEM_START_EBLOCK          (NUM_DATA_EBLOCKS)
#define TRANSACTION_LOG_START_EBLOCK (SYSTEM_START_EBLOCK + NUM_CHAIN_EBLOCK_REQUEST)
#define FLUSH_LOG_START_EBLOCK       (SYSTEM_START_EBLOCK + NUM_CHAIN_EBLOCK_REQUEST + NUM_TRANSACTION_LOG_EBLOCKS)
#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#define SUPER_LOG_START_EBLOCK       (SYSTEM_START_EBLOCK + NUM_CHAIN_EBLOCK_REQUEST + NUM_TRANSACTION_LOG_EBLOCKS + NUM_FLUSH_LOG_EBLOCKS)
#endif
#define TRANSACTION_LOG_END_EBLOCK   (FLUSH_LOG_START_EBLOCK - 1)
#define FLUSH_LOG_END_EBLOCK         (FLUSH_LOG_START_EBLOCK + NUM_FLUSH_LOG_EBLOCKS - 1)
//#define FLUSH_GC_START_EBLOCK        (NUM_EBLOCKS_PER_DEVICE - NUM_SYSTEM_RESERVE_EBLOCKS) // 2013/05/31 by saito
#define FTL_START_OFFSET             (FTL_START_EBLOCK * EBLOCK_SIZE)
#define NUMBER_OF_PAGES_PER_DEVICE   ((NUMBER_OF_ERASE_BLOCKS - 1) * NUMBER_OF_PAGES_PER_EBLOCK)

#define MAX_TRANSFER_EBLOCKS         (2)
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#define MAX_BLOCKS_TO_SAVE           (6)
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
#if(NUM_COMMON_RESERVE_EBLOCKS < ((6) + (((MAX_BAD_BLOCK_SANITY_TRIES-1)*3))))
  #error "NUM_COMMON_RESERVE_EBLOCKS is less than MAX_BLOCKS_TO_SAVE"
#endif
#define MAX_BLOCKS_TO_SAVE           ((6) + (((MAX_BAD_BLOCK_SANITY_TRIES-1)*3)))
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#define MAX_LOG_ENTRIES              ((EBLOCK_SIZE / LOG_ENTRY_DELTA) - 1)
#define TOTAL_BYTES_PER_PAGE         VIRTUAL_PAGE_SIZE

#define NUM_SECTORS_PER_EBLOCK       (NUM_SECTORS_PER_PAGE * NUM_PAGES_PER_EBLOCK)
#define NUM_DEVICES                  NUMBER_OF_DEVICES

#ifndef UINT32
#define UINT32 unsigned long
#endif  // #ifndef UINT32

#ifndef UINT16
#define UINT16 unsigned short int
#endif  // #ifndef UINT16

#ifndef UINT16_PTR
#define UINT16_PTR unsigned short int*
#endif  // #ifndef UINT16_PTR

#ifndef UINT8
#define UINT8 unsigned char
#endif  // #ifndef UINT8

#ifndef UINT32_PTR
#define UINT32_PTR unsigned long*
#endif  // #ifndef UINT32_PTR

#ifndef UINT8_PTR
#define UINT8_PTR unsigned char*
#endif  // #ifndef UINT8_PTR

#ifndef UINT8_PTR_PTR
#define UINT8_PTR_PTR unsigned char**
#endif  // #ifndef UINT8_PTR_PTR

#define OLD_SYS_BLOCK_SIGNATURE_SIZE (4)
#define OLD_SYS_BLOCK_SIGNATURE      (0xABCD)
#define FLUSH_DONE_SIGNATURE         (0x12)
#define END_POINT_SIGNATURE          (0x55)
#define FULL_FLUSH_SIGNATURE         (0xAA55)
#define EBLOCK_MAP_TABLE_FLUSH       (0x2)
#define PPA_MAP_TABLE_FLUSH          (0x3)

#define NUM_TRANSFER_MAP_ENTRIES     (NUM_PAGES_PER_EBLOCK+1)  /*715 bytes*//*num entries has to be more than 8, because of transaction log entries*/

#ifndef FTL_DEV
#define FTL_DEV              UINT8
#endif  // #ifndef FTL_DEV

#define NUM_PAGES_PER_EB             NUM_PAGES_PER_EBLOCK
#define EMPTY_BYTE                   (0xFF)
#define EMPTY_DWORD                  (0xFFFFFFFF)
#define EMPTY_WORD                   (0xFFFF)
#define SECTOR_BIT_SHIFT             (0)
#define PAGE_ADDRESS_BIT_MAP         (0xFFFFFFFF)

#define SYS_EBLOCK_INFO_LOG          (0x01)
#define SYS_EBLOCK_INFO_FLUSH        (0x02)
#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#define SYS_EBLOCK_INFO_SUPER        (0x03)
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#define INIT_NOT_DONE                (0)
#define INIT_DONE                    (1)
#define INIT_FORMATTED               (2)

#define UPDATED_NOT_DONE             (0)
#define UPDATED_DONE                 (1)

#define DIRTY_BIT                    (1)
#define CLEAN_BIT                    (0)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
  #define LOG_ENTRY_DELTA            (32)
  #if (CACHE_RAM_BD_MODULE == FTL_FALSE)
  #define FLUSH_RAM_TABLE_SIZE       (SECTOR_SIZE)
  #define MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK (EBLOCK_SIZE / (SECTOR_SIZE + LOG_ENTRY_DELTA))
  #endif
  #define SPANSION_CRC32      FTL_FALSE
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
  #define LOG_ENTRY_DELTA            (512)
#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
  #define FLUSH_RAM_TABLE_SIZE       (SECTOR_SIZE - FLUSH_INFO_SIZE - ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) - FLUSH_INFO_SIZE))
#else
  #define FLUSH_RAM_TABLE_SIZE       (SECTOR_SIZE - FLUSH_INFO_SIZE)
#endif
  #define MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK (EBLOCK_SIZE / SECTOR_SIZE)
  #define SPANSION_CRC32      FTL_TRUE
#endif

#define SYS_EBLOCK_INFO_GC           (0x03)
#define BLOCK_INFO_EMPTY_PAGE        (0x00)
#define BLOCK_INFO_VALID_PAGE        (0x01)
#define BLOCK_INFO_STALE_PAGE        (0x02)
#define GC_MOVED_PAGE                (1)
#define GC_NOT_MOVED_PAGE            (0)
#define SYS_INFO_SIZE                (16)
#define LOG_ENTRY_SIZE               (16)
#define FLUSH_INFO_SIZE              (16)
#define NUM_LOG_ENTRIES_PER_EBLOCK   (EBLOCK_SIZE / LOG_ENTRY_DELTA)
#if (CACHE_RAM_BD_MODULE == FTL_FALSE)
#define MAX_FLUSH_ENTRIES            (((EBLOCK_SIZE / (SECTOR_SIZE + LOG_ENTRY_DELTA)) - 1) * NUM_FLUSH_LOG_EBLOCKS)
#endif
#define NUM_BITS_EB_CHAIN                 (16)
#define EB_LOGICAL_CHAIN_MASK             (0xFFFF0000)
#define EB_PHYSICAL_CHAIN_MASK            (0x0000FFFF)
#define PAGE_CHAINED                      (0xFFFFFFFE)
#define ERASE_STATUS_CLEAR_WORD_MASK      (0x7FFF)
#define ERASE_STATUS_CLEAR_DWORD_MASK     (0xFFFDFFFF)
#define ERASE_STATUS_GET_WORD_MASK        (0x8000)
#define ERASE_COUNT_GET_WORD_DIRTY_MASK   (0xC000)
#define ERASE_COUNT_CLEAR_WORD_DIRTY_MASK (0x3FFF)
#define ERASE_COUNT_DWORD_DIRTY_SHIFT     (1)     
#define ERASE_STATUS_GET_DWORD_MASK       (0x20000) 
#define ERASE_STATUS_DWORD_SHIFT          (17)      
#define ERASE_COUNT_LIMIT                 (100000)
#define CHAIN_FLAG                        (0x8000)
#define DEBUG_FTL_API_ANNOUNCE            (0)
#define FLUSH_NORMAL_MODE                 (1)
#define FLUSH_GC_MODE                     (2)
#define FLUSH_SHUTDOWN_MODE               (3)
#define CLEAR_GC_SAVE_INIT_MODE           (0)
#define CLEAR_GC_SAVE_RUNTIME_MODE        (1)

/*OPTIONAL MODULES*/
#define DEBUG_CHECK_TABLES           (0)
/*END OPTIONAL MODULES*/

#if (1 == NUMBER_OF_SECTORS_PER_PAGE)
  #define SECTOR_BIT_MAP    (0x00000000)
  #define DEVICE_BIT_SHIFT  (0)
#else  // 
  #if (2 == NUMBER_OF_SECTORS_PER_PAGE)
    #define SECTOR_BIT_MAP    (0x00000001)
    #define DEVICE_BIT_SHIFT  (1)
  #else
    #if (4 == NUMBER_OF_SECTORS_PER_PAGE)
      #define SECTOR_BIT_MAP    (0x00000003)
      #define DEVICE_BIT_SHIFT  (2)
    #else
      #if (8 == NUMBER_OF_SECTORS_PER_PAGE)
        #define SECTOR_BIT_MAP    (0x00000007)
        #define DEVICE_BIT_SHIFT  (3)
      #else
        #if (16 == NUMBER_OF_SECTORS_PER_PAGE)
          #define SECTORS_BIT_MAP  (0x000000F)
          #define DEVICE_BIT_SHIFT  (4)
        #else
          #error "NUMBER_OF_SECTORS_PER_PAGE not supported"
        #endif
      #endif
    #endif
  #endif
#endif

#if (1 == NUMBER_OF_DEVICES)
  #define DEVICE_BIT_MAP  (0x00000000 << DEVICE_BIT_SHIFT)
  #define PAGE_ADDRESS_SHIFT  (DEVICE_BIT_SHIFT + 0)
#else
  #if (2 == NUMBER_OF_DEVICES)
    #define DEVICE_BIT_MAP  (0x00000001 << DEVICE_BIT_SHIFT)
    #define PAGE_ADDRESS_SHIFT  (DEVICE_BIT_SHIFT + 1)
  #else
    #if (4 == NUMBER_OF_DEVICES)
      #define DEVICE_BIT_MAP  (0x00000003 << DEVICE_BIT_SHIFT)
      #define PAGE_ADDRESS_SHIFT  (DEVICE_BIT_SHIFT + 2)
    #else
      #if (8 == NUMBER_OF_DEVICES)
        #define DEVICE_BIT_MAP  (0x00000007 << DEVICE_BIT_SHIFT)
        #define PAGE_ADDRESS_SHIFT  (DEVICE_BIT_SHIFT + 3)
      #else
        #if (16 == NUMBER_OF_DEVICES)
          #define DEVICE_BIT_MAP  (0x0000000F) << DEVICE_BIT_SHIFT)
          #define PAGE_ADDRESS_SHIFT  (DEVICE_BIT_SHIFT + 4)
        #else
          #error "NUMBER_OF_DEVICES not supported"
        #endif
      #endif
    #endif
  #endif
#endif

typedef struct _addressStruct{
    FTL_DEV devID;
    UINT8 pageOffset;
    UINT32 logicalPageNum;
} ADDRESS_STRUCT;

typedef  ADDRESS_STRUCT* ADDRESS_STRUCT_PTR;

#define SYS_INFO_DATA_WORDS         ((SYS_INFO_SIZE/2)-1)
#define SYS_INFO_CHECK_WORD         (0)
#define SYS_INFO_DATA_START         (1)
#define NUM_SYS_RESERVED_BYTES      (1)

/**********************FLASH STRUCTURES******************/
typedef struct _sysEblockInfo
{
    UINT16 checkWord;
    UINT8 type;
    UINT8 reserved[NUM_SYS_RESERVED_BYTES];
    UINT16 phyAddrThisEBlock;
    UINT16 checkVersion;
    UINT16 oldSysBlock;
    UINT16 fullFlushSig;
    UINT32 incNumber; /*used as a key*/
} SYS_EBLOCK_INFO, *SYS_EBLOCK_INFO_PTR;
/********************************************************/

typedef struct _logPhyPageLocation
{
    UINT16 logEBNum;
    UINT16 phyEBOffset;
} LOG_PHY_PAGE_LOCATION, *LOG_PHY_PAGE_LOCATION_PTR;

#define TRANS_LOG_TYPE_A         (0x01)
#define NUM_ENTRIES_TYPE_A       (2)
#define TRANS_LOG_TYPE_B         (0x02)
#define NUM_ENTRIES_TYPE_B       (3)
#define TRANS_LOG_TYPE_C         (0x03)
#define GC_TYPE_A                (0x04)
#define GC_TYPE_B                (0x05)
#define NUM_ENTRIES_GC_TYPE_B    (12)
#define NUM_GC_TYPE_B            (((NUMBER_OF_PAGES_PER_EBLOCK / 8) / NUM_ENTRIES_GC_TYPE_B) + 1)
#define GC_MOVE_BITMAP           (NUM_ENTRIES_GC_TYPE_B * NUM_GC_TYPE_B)
#define TEMP_B_ENTRIES           (((NUM_TRANSFER_MAP_ENTRIES - NUM_ENTRIES_TYPE_A) / NUM_ENTRIES_TYPE_B) + 1)
#define CHAIN_LOG_TYPE           (10)
#define EBSWAP_LOG_TYPE          (11)
#define UNLINK_LOG_TYPE_A1       (12)
#define UNLINK_LOG_TYPE_A2       (13)
#define UNLINK_LOG_TYPE_B        (14)
#define SPARE_LOG_TYPE           (15)

#define MIN_LOG_ENTRIES_NEEDED   (2)

#define LOG_ENTRY_DATA_WORDS     ((LOG_ENTRY_SIZE/2)-1)
#define LOG_ENTRY_CHECK_WORD     (0)
#define LOG_ENTRY_DATA_START     (1)

typedef struct _transLogEntryA
{
    UINT16 checkWord;             /* first word */
    UINT8 type;                   /* second word */
    UINT8 seqNum;                 /*       1 byte*/
    LOG_PHY_PAGE_LOCATION pageLoc[NUM_ENTRIES_TYPE_A]; /* 4*2 = 8 bytes*/
    UINT32 LBA;                   /*       4 bytes*/
} TRANS_LOG_ENTRY_A, *TRANS_LOG_ENTRY_A_PTR; /*Total =16 bytes*/

typedef struct _transLogEntryB
{
    UINT16 checkWord;              /* first word */
    UINT8 type;                    /* second word */
    UINT8 seqNum;                  /*       1 byte*/
    LOG_PHY_PAGE_LOCATION pageLoc[NUM_ENTRIES_TYPE_B];  /*8*2 = 12 bytes*/
} TRANS_LOG_ENTRY_B, *TRANS_LOG_ENTRY_B_PTR;  /*Total=16 bytes*/

#define TRANS_LOG_ENTRY_C_RESERVED  (8)
typedef struct _transLogEntryC
{
    UINT16 checkWord;                 /* first word */
    UINT8 type;                       /* second word */
    UINT8 seqNum;                     /*       1 byte*/
    UINT8 reserved[TRANS_LOG_ENTRY_C_RESERVED];
    UINT32 GCNum;                     /*       4 bytes*/
} TRANS_LOG_ENTRY_C, *TRANS_LOG_ENTRY_C_PTR; /*Total =16 bytes*/

typedef struct _transLogEntry
{
   TRANS_LOG_ENTRY_A entryA;
   TRANS_LOG_ENTRY_B entryB[TEMP_B_ENTRIES];
   TRANS_LOG_ENTRY_C entryC;
} TRANS_LOG_ENTRY, *TRANS_LOG_ENTRY_PTR;

typedef struct _transLogCache
{
   TRANS_LOG_ENTRY_C entryC;
   UINT8 flag;
} TRANS_LOG_CACHE, *TRANS_LOG_CACHE_PTR;

/********************************************************/

#define GC_LOG_ENTRY_A_RESERVED  (4)
typedef struct _GCLogEntryA
{
    UINT16 checkWord;                 /* first word */
    UINT8 type;                       /* second word */
    UINT8 holdForMerge;
    UINT8 reserved[GC_LOG_ENTRY_A_RESERVED];
    UINT16 logicalEBAddr;
    UINT16 reservedEBAddr;
    UINT32 GCNum;
} GC_LOG_ENTRY_A, *GC_LOG_ENTRY_A_PTR;

#define GC_LOG_ENTRY_B_RESERVED  (1)
typedef struct _GCLogEntryB
{
    UINT16 checkWord;               /* first word */
    UINT8 type;                     /* second word */
    UINT8 pageMovedBitMap[NUM_ENTRIES_GC_TYPE_B];
    UINT8 reserved[GC_LOG_ENTRY_B_RESERVED];
} GC_LOG_ENTRY_B, *GC_LOG_ENTRY_B_PTR;

typedef struct _GCLogEntry
{
    GC_LOG_ENTRY_A  partA;
    GC_LOG_ENTRY_B  partB[NUM_GC_TYPE_B];
} GC_LOG_ENTRY, *GC_LOG_ENTRY_PTR;


#define SPARE_LOG_ENTRY_RESERVED  (11)
typedef struct _SpareLogEntry
{
    UINT16 checkWord;             /* first word */
    UINT8 type;                   /* second word */
    UINT8 reserved[SPARE_LOG_ENTRY_RESERVED];
    UINT16 logicalEBNum;
} SPARE_LOG_ENTRY, *SPARE_LOG_ENTRY_PTR; /*Total =16 bytes*/

#define SPARE_INFO_SIZE             (16)
#define SPARE_INFO_DATA_WORDS       ((SPARE_INFO_SIZE/2)-1)
#define SPARE_INFO_CHECK_WORD       (0)
#define SPARE_INFO_DATA_START       (1)

#if (SPANSION_CRC32 == FTL_TRUE)
#define SPARE_INFO_RESERVED  (4)
#else
#define SPARE_INFO_RESERVED  (8)
#endif

typedef struct _SpareInfo
{
    UINT16 checkWord;             /* first word */
    UINT16 pad;                   /* for alignment */
    UINT32 logicalPageAddr;
    #if (SPANSION_CRC32 == FTL_TRUE)
    UINT32 crc32;
    #endif
    UINT8 reserved[SPARE_INFO_RESERVED];
} SPARE_INFO, *SPARE_INFO_PTR; /*Total =16 bytes*/

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#define NUM_SYS_EB_ENTRY            (5)
#define SYS_EBLOCK_INFO_SYSEB       (0x04)
#define SYS_EBLOCK_INFO_CHANGED     (0x05)
typedef struct _SuperSysInfo
{
    UINT16 checkWord;             /* first word */
    UINT8 type;
    UINT8 decNumber;
    UINT16 PhyEBNum[NUM_SYS_EB_ENTRY];
    UINT16 EntryNumThisIndex;
} SUPER_SYS_INFO, *SUPER_SYS_INFO_PTR; /*Total =16 bytes*/
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
/********************************************************/

#define FLUSH_INFO_DATA_WORDS     ((FLUSH_INFO_SIZE/2)-1)
#define FLUSH_INFO_CHECK_WORD     (0)
#define FLUSH_INFO_DATA_START     (1)
#define FLUSH_INFO_TABLE_CHECK_WORD (5)
#define FLUSH_INFO_TABLE_START      (FLUSH_INFO_SIZE/2)

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#define SYS_EBLOCK_FLUSH_INFO_RESERVED  (2)
#endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

typedef struct _sysEBFlushInfo
{
    UINT16 checkWord;            /* first word */
    UINT8 type;                  /* second word */
    UINT8 endPoint;
    UINT16 eBlockNumLoc;
    UINT16 entryIndexLoc;
    UINT16 tableOffset;

    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    UINT8 reserved[SYS_EBLOCK_FLUSH_INFO_RESERVED];

    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    UINT16 tableCheckWord;
    #endif  // #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)

    UINT32 logIncNum;
} SYS_EBLOCK_FLUSH_INFO, *SYS_EBLOCK_FLUSH_INFO_PTR;

#define CHAIN_LOG_ENTRY_RESERVED  (5)
typedef struct _chainLogEntry
{
    UINT16 checkWord;            /* first word */
    UINT8  type;                 /* second word */
    UINT8  reserved[CHAIN_LOG_ENTRY_RESERVED];
    UINT16 logicalFrom;
    UINT16 phyFrom;
    UINT16 logicalTo;
    UINT16 phyTo;
} CHAIN_LOG_ENTRY, *CHAIN_LOG_ENTRY_PTR;

#define EBSWAP_LOG_ENTRY_RESERVED  (9)
typedef struct _ebSwapLogEntry
{
    UINT16 checkWord;            /* first word */
    UINT8  type;                 /* second word */
    UINT8  reserved[EBSWAP_LOG_ENTRY_RESERVED];
    UINT16 logicalDataEB;
    UINT16 logicalReservedEB;
} EBSWAP_LOG_ENTRY, *EBSWAP_LOG_ENTRY_PTR;

#if(FTL_UNLINK_GC == FTL_TRUE)
#define UNLINK_LOG_ENTRY_A_RESERVED   (9)
#define NUM_ENTRIES_UNLINK_TYPE_B     (NUM_ENTRIES_GC_TYPE_B)
#define NUM_UNLINK_TYPE_B             (NUM_GC_TYPE_B)

typedef struct _UnlinkLogEntryA
{
    UINT16 checkWord;                 /* first word */
    UINT8 type;
    UINT8 reserved[UNLINK_LOG_ENTRY_A_RESERVED];
    UINT16 fromLogicalEBAddr;
    UINT16 toLogicalEBAddr;
} UNLINK_LOG_ENTRY_A, *UNLINK_LOG_ENTRY_A_PTR;

typedef GC_LOG_ENTRY_B UNLINK_LOG_ENTRY_B;

typedef struct _UnlinkLogEntry
{
    UNLINK_LOG_ENTRY_A partA;
    UNLINK_LOG_ENTRY_B partB[NUM_UNLINK_TYPE_B];
} UNLINK_LOG_ENTRY, *UNLINK_LOG_ENTRY_PTR;
#endif  // #if(FTL_UNLINK_GC == FTL_TRUE)

typedef struct _LogEntryLoc
{
    UINT16 eBlockNum;
    UINT16 entryIndex;
} LOG_ENTRY_LOC, *LOG_ENTRY_LOC_PTR;

/**********************RAM STRUCTURES******************/

#if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
#define PPA_MAPPING_ENTRY            UINT8
#define EMPTY_INVALID                (EMPTY_BYTE)
#define CHAIN_INVALID                (0xFE)
#define PPA_MASK                     (EMPTY_BYTE)
#define PPA_MAPPING_ENTRY_SIZE       (1)
#else
#define PPA_MAPPING_ENTRY            UINT16
#define EMPTY_INVALID                (EMPTY_WORD)
#define CHAIN_INVALID                (0xFFFE)
#define PPA_MASK                     (0x7FFF)
#define PPA_MAPPING_ENTRY_SIZE       (2)
#endif

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
  #if(32 < (PPA_MAPPING_ENTRY_SIZE * NUM_PAGES_PER_EBLOCK))
    #define FLUSH_RAM_TABLE_SIZE     (PPA_MAPPING_ENTRY_SIZE * NUM_PAGES_PER_EBLOCK)
  #else
    #define FLUSH_RAM_TABLE_SIZE     (32)
  #endif
  #define MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK ((EBLOCK_SIZE / (FLUSH_RAM_TABLE_SIZE + LOG_ENTRY_DELTA)))
  #define MAX_FLUSH_ENTRIES            (((EBLOCK_SIZE / (FLUSH_RAM_TABLE_SIZE + LOG_ENTRY_DELTA)) - 1) * NUM_FLUSH_LOG_EBLOCKS)
#else
#define MAX_FLUSH_ENTRIES            (((EBLOCK_SIZE / (SECTOR_SIZE + LOG_ENTRY_DELTA)) - 1) * NUM_FLUSH_LOG_EBLOCKS)
#endif
#endif

// n structures must fit into a sector
#if (0 != (SECTOR_SIZE % PPA_MAPPING_ENTRY_SIZE))
    #error "PPA_MAPPING_ENTRY_SIZE is not valid"
#endif  // #if (0 != (SECTOR_SIZE % PPA_MAPPING_ENTRY_SIZE))

typedef UINT8 FREE_BIT_MAP_TYPE;

#if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
    #define NON_ARRAY_SIZE            (15)
#else
    #define NON_ARRAY_SIZE            (14)
#endif

#define TEMP_EBLOCK_MAPPING_STRUCT_SIZE  (NON_ARRAY_SIZE + EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE)
#if((TEMP_EBLOCK_MAPPING_STRUCT_SIZE % 4) == 0)
#define EBLOCK_MAPPING_TABLE_PAD  (0)
#else
#define EBLOCK_MAPPING_TABLE_PAD  (4 - (TEMP_EBLOCK_MAPPING_STRUCT_SIZE % 4))
#endif

// Note: The following constant must be set to the size of the entries in the
//         EBLOCK_MAPPING_ENTRY that are not arrays
//-- Managed Regions
#if(MANAGED_REGIONS == FTL_TRUE)
typedef struct _managedRegions
{
    UINT16 numberEblocksManaged;
    UINT32 baseAddr;
    UINT8 deviceId;
    UINT32 deviceOffset;
} MANAGED_REGIONS_STRUCT, *MANAGED_REGIONS_STRUCT_PTR;

typedef struct _managedRegionsInfo
{
    UINT32 baseAddr;
    UINT8 deviceId;
    UINT32 deviceOffsetBytes;
} MANAGED_REGIONS_INFO_STRUCT, *MANAGED_REGIONS_INFO_STRUCT_PTR;

#endif


typedef struct _eBlockMappingEntry
{
    UINT16 phyEBAddr;
    UINT16 dirtyCount;
    UINT32 freePage_GCNum;  /*flush blocks, freePage, Data Eblocks use it as GC Number*/
    UINT32 chainToFrom;     /*upper 16 bits logical, lower 16 bits physical, the FROM EB has the TO info, the TO EB has the FROM info*/
    UINT16 eraseCount;
    FREE_BIT_MAP_TYPE freeBitMap[EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE];

    #if (EBLOCK_MAPPING_TABLE_PAD != 0)
      UINT8 reserved[EBLOCK_MAPPING_TABLE_PAD];
    #endif
    #if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
      UINT8 isBadBlock;
    #endif
} EBLOCK_MAPPING_ENTRY, *EBLOCK_MAPPING_ENTRY_PTR;

#if (FTL_DEFECT_MANAGEMENT == FTL_TRUE)
   #define FTL_SWAP_RESERVE_ERASE               0x10

    typedef struct _badBlockInfo
    {
        FTL_DEV devID;
        UINT8 TransLogEBFailed;
        UINT16 TransLogEBNum;
        UINT16 reserved;
        UINT16 sourceLogicalEBNum;
        UINT16 targetLogicalEBNum;
        UINT16 operation;
        EBLOCK_MAPPING_ENTRY   sourceEBMap;
        EBLOCK_MAPPING_ENTRY   targetEBMap;
        PPA_MAPPING_ENTRY      sourcePPA[NUM_PAGES_PER_EBLOCK];
        PPA_MAPPING_ENTRY      targetPPA[NUM_PAGES_PER_EBLOCK];
    } BAD_BLOCK_INFO, *BAD_BLOCK_INFO_PTR;


    #define FTL_BAD_BLOCK_WRITING           (0x1)
    #define FTL_ERR_DATA_RESERVE            (0x2)
    #define FTL_ERR_CHAIN_FULL_EB           (0x100)
    #define FTL_ERR_CHAIN_NOT_FULL_EB       (0x101)
    #define FTL_ERR_BAD_BLOCK_SOURCE        (0x3)
    #define FTL_ERR_BAD_BLOCK_TARGET        (0x4)
#endif

#define EBLOCK_MAPPING_ENTRY_SIZE (NON_ARRAY_SIZE + EBLOCK_MAPPING_TABLE_BIT_MAP_BYTE + EBLOCK_MAPPING_TABLE_PAD)


#if (CACHE_RAM_BD_MODULE == FTL_TRUE)

// defined cache
#define CACHE_EBLOCKMAP (0x0)
#define CACHE_PPAMAP    (0x1)

#define CACHE_FREE      (0x0)
#define CACHE_CLEAN     (0x1)
#define CACHE_DIRTY     (0x2)

#define CACHE_NO_DEPEND      (0x0)
#define CACHE_DEPEND_UP      (0x1)
#define CACHE_DEPEND_DOWN    (0x2)
#define CACHE_DEPEND_UP_DOWN (0x3)

#define CACHE_EBM_PPA_PRESENT (0x0)
#define CACHE_EBM_PRESENT     (0x1)
#define CACHE_NO_PRESENT      (0x2)

#define CACHE_WRITE_TYPE           (0x0)
#define CACHE_READ_TYPE            (0x1)
#define CACHE_INIT_TYPE            (0x2)

#if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if(EBLOCK_SIZE > 0x1000)
#define CACHE_EMPTY_ENTRY_INDEX     (0xFFF)
#define CACHE_EMPTY_FLASH_LOG_ARRAY (0xF)
#else
#define CACHE_EMPTY_ENTRY_INDEX     (0xFF)
#define CACHE_EMPTY_FLASH_LOG_ARRAY (0xFF)
#endif
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
#define CACHE_EMPTY_ENTRY_INDEX     (0x3FF)
#define CACHE_EMPTY_FLASH_LOG_ARRAY (0x1F)
#endif

#define CACHE_INIT_RAM_MAP_INDEX    (0x3FFFFFFF)
#define CACHE_INIT_EBM_CACHE_INDEX  (0x3000)
#define CACHE_EMPTY_EBM_CACHE_INDEX (0x3FFF)
#define CACHE_SAVE_WL_LOGICAL_MASK  (0xFFFF0000)
#define CACHE_SAVE_WL_LOGICAL_SHIFT (16)
#define CACHE_WL_HIGH               (0x0)
#define CACHE_WL_LOW                (0x1)

// LRU setting 
#define LIMIT_LRU  (0x3F) // can't change value
#define DIVIDE_LRU (0x1) 

// Two cross LEB allows the middle of algorithm, but outside of main interface.
#define NUM_CROSS_LEB  (2)

#if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if (EBLOCK_MAPPING_ENTRY_SIZE >= 32) // check EBLOCK_MAPPING_ENTRY_SIZE
   #define CACHE_EB_PAD                    (FTL_FALSE)
   #define CACHE_EBLOCK_MAPPING_ENTRY_PAD (0)
#else
   #define CACHE_EB_PAD                    (FTL_TRUE)
   #define CACHE_EBLOCK_MAPPING_ENTRY_PAD (32 - EBLOCK_MAPPING_ENTRY_SIZE)
#endif
#elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
#define CACHE_EB_PAD                    (FTL_FALSE)
#define CACHE_EBLOCK_MAPPING_ENTRY_PAD (0)
#endif

// offset of PPAMappingCache
#if ((FLUSH_RAM_TABLE_SIZE % (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) == 0)
  #define SAVE_DATA_EB_INDEX (1) // minimum Data EB of index
  #define EBM_ENTRY_COUNT (FLUSH_RAM_TABLE_SIZE / (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD))
  #define CROSS_CASE_ON (FTL_FALSE)
#else
  // cross boundary case
  #define SAVE_DATA_EB_INDEX (4) // minimum Data EB
  #define CROSS_CASE_ON (FTL_TRUE)
  #define EBM_ENTRY_COUNT ((FLUSH_RAM_TABLE_SIZE / (EBLOCK_MAPPING_ENTRY_SIZE +CACHE_EBLOCK_MAPPING_ENTRY_PAD)) + 1)
#endif
#if ((FLUSH_RAM_TABLE_SIZE % (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)) == 0)
  #define PPA_ENTRY_COUNT (FLUSH_RAM_TABLE_SIZE / (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE))
#else
  #define PPA_ENTRY_COUNT ((FLUSH_RAM_TABLE_SIZE / (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)) + 1)
#endif
#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
#if ((NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE) >= FLUSH_RAM_TABLE_SIZE)
  #if (((EBM_ENTRY_COUNT * (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)) % FLUSH_RAM_TABLE_SIZE) == 0)
    #define PPA_CACHE_TABLE_OFFSET ((EBM_ENTRY_COUNT * (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)) / FLUSH_RAM_TABLE_SIZE)
  #else
    #define PPA_CACHE_TABLE_OFFSET ((EBM_ENTRY_COUNT * (NUMBER_OF_PAGES_PER_EBLOCK * PPA_MAPPING_ENTRY_SIZE)) / FLUSH_RAM_TABLE_SIZE) + 1
  #endif
#else
  #define PPA_CACHE_TABLE_OFFSET (1)
#endif
#elif (FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
#if ((EBM_ENTRY_COUNT % PPA_ENTRY_COUNT) == 0)
  #define PPA_CACHE_TABLE_OFFSET (EBM_ENTRY_COUNT / PPA_ENTRY_COUNT)
#else
  #error "Configuration error"
#endif
#endif

#if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
  #define OTHER_RAM_TABLE 8192
#elif (FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
  #define OTHER_RAM_TABLE 6144
#else
  #error "FTL_DEVICE_TYPE is not setting" 
#endif

// maximum EBlock/PPAMapIndex
#if (((NUMBER_OF_ERASE_BLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) % FLUSH_RAM_TABLE_SIZE) == 0)
  #define MAX_EBLOCK_MAP_INDEX ((NUMBER_OF_ERASE_BLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE)
#else
  #define MAX_EBLOCK_MAP_INDEX (((NUMBER_OF_ERASE_BLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1)
#endif
#define MAX_PPA_MAP_INDEX (MAX_EBLOCK_MAP_INDEX * PPA_CACHE_TABLE_OFFSET)

#define EBLOCKMAPINDEX_SIZE      (2/*UINT16 size*/ * MAX_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES)
#define PPAMAPINDEX_SIZE         (2/*UINT16 size*/ * MAX_PPA_MAP_INDEX * NUMBER_OF_DEVICES)
#define RAMMAPINDEX_SIZE         (4/*UINT32 size*/ * NUMBER_OF_ERASE_BLOCKS * NUMBER_OF_DEVICES)

#define TEMP_BD_RAM_SIZE (OTHER_RAM_TABLE + EBLOCKMAPINDEX_SIZE + PPAMAPINDEX_SIZE + RAMMAPINDEX_SIZE)

#if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
#if (CACHE_MINIMUM_RAM == FTL_FALSE && CACHE_MAXIMUM_RAM == FTL_FALSE)

#define TEMP_USED_SIZE (CACHE_BD_RAM_SIZE - TEMP_BD_RAM_SIZE)
#define TEMP_COUNT (TEMP_USED_SIZE / ((FLUSH_RAM_TABLE_SIZE * (1/*EBlock*/ + PPA_CACHE_TABLE_OFFSET)) + 2/*ebm entry one size*/))

// Create EBlockMappingCache and PPAMappingCache
#if (((MAX_PPA_MAP_INDEX * FLUSH_RAM_TABLE_SIZE * NUMBER_OF_DEVICES) + (MAX_PPA_MAP_INDEX * FLUSH_RAM_TABLE_SIZE * NUMBER_OF_DEVICES) + (NUMBER_OF_ERASE_BLOCKS * NUMBER_OF_DEVICES * 2/*ebm entry one size*/)) < TEMP_USED_SIZE)
     #define NUM_EBLOCK_MAP_INDEX (MAX_EBLOCK_MAP_INDEX)
     #define NUM_PPA_MAP_INDEX    (NUM_EBLOCK_MAP_INDEX * PPA_CACHE_TABLE_OFFSET)
#else
     #define NUM_EBLOCK_MAP_INDEX (TEMP_COUNT)
     #define NUM_PPA_MAP_INDEX    (TEMP_COUNT * PPA_CACHE_TABLE_OFFSET)
#endif

// Set possible swap area of EBMCacheIndex
#if (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) % FLUSH_RAM_TABLE_SIZE) == 0)
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    #if (NUM_EBLOCK_MAP_INDEX > ((((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE)) + SAVE_DATA_EB_INDEX))
       #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE))
    #endif
    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    #if (NUM_EBLOCK_MAP_INDEX > (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE+ CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1 + SAVE_DATA_EB_INDEX))
       #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1))
    #endif
    #endif
#else
    #if(FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
    #if (NUM_EBLOCK_MAP_INDEX > (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE+ CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1 + SAVE_DATA_EB_INDEX))
       #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 1))
    #endif
    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
    #if (NUM_EBLOCK_MAP_INDEX > (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE+ CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 2 + SAVE_DATA_EB_INDEX))
       #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) / FLUSH_RAM_TABLE_SIZE) + 2))
    #endif
    #endif
#endif
#else
    #if (CACHE_MINIMUM_RAM == FTL_TRUE)
    #if (FTL_DEVICE_TYPE == FTL_DEVICE_NOR)
       #if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
          #define NUM_EBLOCK_MAP_INDEX ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + SAVE_DATA_EB_INDEX)
          #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - (NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE))
       #else
          #define NUM_EBLOCK_MAP_INDEX (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1) + SAVE_DATA_EB_INDEX)
          #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1))
       #endif
    #elif(FTL_DEVICE_TYPE == FTL_DEVICE_NAND)
       #if ((NUMBER_OF_ERASE_BLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)

          #if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
             #define NUM_EBLOCK_MAP_INDEX ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + SAVE_DATA_EB_INDEX)
          #else
             #define NUM_EBLOCK_MAP_INDEX (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1) + SAVE_DATA_EB_INDEX)
          #endif
       #else
          #if (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD)) % FLUSH_RAM_TABLE_SIZE) == 0)
             #define NUM_EBLOCK_MAP_INDEX (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE)) + 1 + SAVE_DATA_EB_INDEX)
          #else
             #define NUM_EBLOCK_MAP_INDEX (((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 2) + SAVE_DATA_EB_INDEX)
          #endif
       #endif
       #define CACHE_INDEX_CHANGE_AREA (1)
    #endif
       #define NUM_PPA_MAP_INDEX (NUM_EBLOCK_MAP_INDEX * PPA_CACHE_TABLE_OFFSET)
    #elif (CACHE_MAXIMUM_RAM == FTL_TRUE)
       #define NUM_EBLOCK_MAP_INDEX (MAX_EBLOCK_MAP_INDEX)
       #define NUM_PPA_MAP_INDEX    (MAX_PPA_MAP_INDEX)
       #if ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) % FLUSH_RAM_TABLE_SIZE) == 0)
          #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - (NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE))
       #else
          #define CACHE_INDEX_CHANGE_AREA (NUM_EBLOCK_MAP_INDEX - ((NUMBER_OF_SYSTEM_EBLOCKS * (EBLOCK_MAPPING_ENTRY_SIZE + CACHE_EBLOCK_MAPPING_ENTRY_PAD) / FLUSH_RAM_TABLE_SIZE) + 1))
       #endif
    #endif
#endif
#else 
#define NUM_EBLOCK_MAP_INDEX (numBlockMapIndex)
#define NUM_PPA_MAP_INDEX (numPpaMapIndex)
#define CACHE_INDEX_CHANGE_AREA (cacheIndexChangeArea)
#define THESHOLD_DIRTY_COUNT (thesholdDirtyCount)
#define EBMCACHEINDEX_SIZE (ebmCacheIndexSize)
#define EBLOCKMAPPINGCAACHE_SIZE (eblockMappingCacheSize)
#define PPAMAPPINGCACHE_SIZE (ppaMappingCacheSize)
#endif // // #if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)

// Theshold dirty count 
#if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)
#if (CROSS_CASE_ON == FTL_FALSE)
    #if(CACHE_INDEX_CHANGE_AREA < 2)
        #define THESHOLD_DIRTY_COUNT (1)
    #else
        #define THESHOLD_DIRTY_COUNT (CACHE_INDEX_CHANGE_AREA - 1)
    #endif
#else
    #if(CACHE_INDEX_CHANGE_AREA < 3)
       #define THESHOLD_DIRTY_COUNT (1)
    #else
       #define THESHOLD_DIRTY_COUNT (CACHE_INDEX_CHANGE_AREA - 2)
    #endif
#endif
#if (THESHOLD_DIRTY_COUNT == 0)
   #error "THESHOLD_DIRTY_COUNT is setting 0"
#endif

// Check ram size
#define EBMCACHEINDEX_SIZE       (2/*UINT16 size*/ * NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES)
#define EBLOCKMAPPINGCAACHE_SIZE (1/*UINT8 size*/ * NUM_EBLOCK_MAP_INDEX * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE)
#define PPAMAPPINGCACHE_SIZE     (1/*UINT8 size*/ *  NUM_PPA_MAP_INDEX * NUMBER_OF_DEVICES * FLUSH_RAM_TABLE_SIZE)

#define TOTAL_BD_RAM_SIZE (TEMP_BD_RAM_SIZE + EBMCACHEINDEX_SIZE + EBLOCKMAPPINGCAACHE_SIZE + PPAMAPPINGCACHE_SIZE)

#if (CACHE_MINIMUM_RAM == FTL_FALSE && CACHE_MAXIMUM_RAM == FTL_FALSE)
    #ifndef CACHE_INDEX_CHANGE_AREA
    #define CACHE_INDEX_CHANGE_AREA (0)
    #endif
    #if (CACHE_BD_RAM_SIZE < TOTAL_BD_RAM_SIZE || CACHE_INDEX_CHANGE_AREA == 0)
       #error "CACHE_BD_RAM_SIZE is less than minimum TOTAL_BD_RAM_SIZE"
    #endif
#endif

#endif // #if (CACHE_DYNAMIC_ALLOCATION == FTL_FALSE)

#if (((MAX_EBLOCK_MAP_INDEX + MAX_PPA_MAP_INDEX) % (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK - 1)) == 0)
#define MIN_FLUSH_GC_EBLOCKS         ((MAX_EBLOCK_MAP_INDEX + MAX_PPA_MAP_INDEX) / (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK - 1))
#else
#define MIN_FLUSH_GC_EBLOCKS         (((MAX_EBLOCK_MAP_INDEX + MAX_PPA_MAP_INDEX) / (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK - 1)) + 1)
#endif
#if(NUM_FLUSH_LOG_EBLOCKS < MIN_FLUSH_GC_EBLOCKS)
  #error "NUM_FLUSH_LOG_EBLOCKS is less than MIN_FLUSH_GC_EBLOCKS"
#endif

#if(NUM_FLUSH_LOG_EBLOCKS > CACHE_EMPTY_FLASH_LOG_ARRAY)
  #error "NUM_FLUSH_LOG_EBLOCKS is more than CACHE_EMPTY_FLASH_LOG_ARRAY"
#endif

#if(NUMBER_OF_SYSTEM_EBLOCKS >= EMPTY_BYTE)
  #error "NUMBER_OF_SYSTEM_EBLOCKS is more than unsinged 8bit"
#endif

#else
#define NUM_BYTES_IN_EBLOCK_MAPPING_TABLE  (EBLOCK_MAPPING_ENTRY_SIZE * NUM_DEVICES * NUM_EBLOCKS_PER_DEVICE)
#define EBLOCK_MAPPING_DEV_TABLE           (EBLOCK_MAPPING_ENTRY_SIZE * NUM_EBLOCKS_PER_DEVICE)
#define TEMP_BITS_EBLOCK_TEMP              (EBLOCK_MAPPING_DEV_TABLE % FLUSH_RAM_TABLE_SIZE)

#if (TEMP_BITS_EBLOCK_TEMP == 0)
#define BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE (EBLOCK_MAPPING_DEV_TABLE / FLUSH_RAM_TABLE_SIZE)
#else
#define BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE ((EBLOCK_MAPPING_DEV_TABLE / FLUSH_RAM_TABLE_SIZE) + 1)
#endif

#if ((BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE % FTL_BITS_PER_BYTE) == 0)
#define EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE (BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE / FTL_BITS_PER_BYTE)
#else
#define EBLOCK_DIRTY_BITMAP_DEV_TABLE_SIZE ((BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE / FTL_BITS_PER_BYTE) + 1)
#endif

#define NUM_BYTES_IN_PPA_TABLE  (PPA_MAPPING_ENTRY_SIZE * NUM_DEVICES * NUM_EBLOCKS_PER_DEVICE * NUM_PAGES_PER_EBLOCK)
#define PPA_TABLE_DEV_TABLE     (PPA_MAPPING_ENTRY_SIZE * NUM_EBLOCKS_PER_DEVICE * NUM_PAGES_PER_EBLOCK)
#define TEMP_PPA_DIRTY_BIT_TEMP (PPA_TABLE_DEV_TABLE % FLUSH_RAM_TABLE_SIZE)

#if (TEMP_PPA_DIRTY_BIT_TEMP == 0)
#define BITS_PPA_DIRTY_BITMAP_DEV_TABLE  (PPA_TABLE_DEV_TABLE / FLUSH_RAM_TABLE_SIZE)
#else
#define BITS_PPA_DIRTY_BITMAP_DEV_TABLE  ((PPA_TABLE_DEV_TABLE / FLUSH_RAM_TABLE_SIZE) + 1)
#endif

#if ((BITS_PPA_DIRTY_BITMAP_DEV_TABLE % FTL_BITS_PER_BYTE) == 0)
#define PPA_DIRTY_BITMAP_DEV_TABLE_SIZE  (BITS_PPA_DIRTY_BITMAP_DEV_TABLE / FTL_BITS_PER_BYTE)
#else
#define PPA_DIRTY_BITMAP_DEV_TABLE_SIZE  ((BITS_PPA_DIRTY_BITMAP_DEV_TABLE / FTL_BITS_PER_BYTE) + 1)
#endif

#define TOTAL_BITS_DIRTY_BITMAP      (BITS_EBLOCK_DIRTY_BITMAP_DEV_TABLE + BITS_PPA_DIRTY_BITMAP_DEV_TABLE)
#if ((TOTAL_BITS_DIRTY_BITMAP % MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK) == 0)
#define MIN_FLUSH_GC_EBLOCKS         (TOTAL_BITS_DIRTY_BITMAP / (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK - 1))
#else
#define MIN_FLUSH_GC_EBLOCKS         ((TOTAL_BITS_DIRTY_BITMAP / (MAX_FLUSH_ENTRIES_PER_LOG_EBLOCK - 1)) + 1)
#endif
#if(NUM_FLUSH_LOG_EBLOCKS < MIN_FLUSH_GC_EBLOCKS)
  #error "NUM_FLUSH_LOG_EBLOCKS is less than MIN_FLUSH_GC_EBLOCKS"
#endif

#endif

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
#define FLUSH_EBLOCK_RETRIES         (MAX_BAD_BLOCK_SANITY_TRIES)
#define MIN_SYSTEM_RESERVE_EBLOCKS   (FLUSH_EBLOCK_RETRIES * MIN_FLUSH_GC_EBLOCKS)
#else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
#define MIN_SYSTEM_RESERVE_EBLOCKS   (MIN_FLUSH_GC_EBLOCKS)
#endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

#if(NUM_SYSTEM_RESERVE_EBLOCK_REQUEST < MIN_SYSTEM_RESERVE_EBLOCKS)
  #error "NUM_SYSTEM_RESERVE_EBLOCK_REQUEST is less than MIN_SYSTEM_RESERVE_EBLOCKS"
#endif

#if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
#define LOG_EBLOCK_RETRIES           (MAX_BAD_BLOCK_SANITY_TRIES)
#define MIN_TRANSACTION_LOG_EBLOCKS  (1 + LOG_EBLOCK_RETRIES)
#else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)
#define MIN_TRANSACTION_LOG_EBLOCKS  (1)
#endif  // #else  // #if(FTL_DEFECT_MANAGEMENT == FTL_TRUE)

#if(NUM_TRANSACTION_LOG_EBLOCKS < MIN_TRANSACTION_LOG_EBLOCKS)
  #error "NUM_TRANSACTION_LOG_EBLOCKS is less than MIN_TRANSACTION_LOG_EBLOCKS"
#endif

#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
#define MIN_SUPER_SYS_EBLOCKS (1)
#define MIN_SYSTEM_EBLOCKS           (MIN_TRANSACTION_LOG_EBLOCKS + MIN_FLUSH_GC_EBLOCKS + MIN_SYSTEM_RESERVE_EBLOCKS + MIN_SUPER_SYS_EBLOCKS)
#else
#define MIN_SYSTEM_EBLOCKS           (MIN_TRANSACTION_LOG_EBLOCKS + MIN_FLUSH_GC_EBLOCKS + MIN_SYSTEM_RESERVE_EBLOCKS)
#endif

#define MIN_RESERVE_EBLOCKS          (MIN_SYSTEM_EBLOCKS + NUM_CHAIN_EBLOCK_REQUEST)
#if(NUMBER_OF_SYSTEM_EBLOCKS < MIN_RESERVE_EBLOCKS)
  #error "NUMBER_OF_SYSTEM_EBLOCKS is less than MIN_RESERVE_EBLOCKS"
#endif

/**********************************/
typedef FTL_INIT_STRUCT* FTL_INIT_STRUCT_PTR;
/**********************************/

typedef struct _keyTableEntry
{
    UINT16 phyAddr;
    UINT16 logicalEBNum;
    UINT32 key;
    #if (CACHE_RAM_BD_MODULE == FTL_TRUE)
    UINT8 cacheNum;
    #endif
} KEY_TABLE_ENTRY;   /*Total 4 bytes*/

typedef struct _gcMerge{
    FTL_DEV DevID;
    UINT16 logicalEBNum;
    UINT32 logicalPageNum;
    UINT32 phyPageNum;
} GC_MERGE_STRUCT;

typedef struct _gcMergeTemp{
    UINT8  pageOffset;
    UINT8  numSectors;
} GC_MERGE_TEMP_STRUCT;

typedef struct _GCInfo{
    FTL_DEV devID; /*1*/
    UINT16 logicalEBlock; /*2*/
    GC_MERGE_STRUCT startMerge; /*1, 2, 4, 4*/
    GC_MERGE_STRUCT endMerge; /*1, 2, 4, 4*/
} GC_INFO;

typedef struct _GCSave{
    FTL_DEV devId;
    UINT16 phyEbNum;
} GC_SAVE;

typedef struct _moveEntryT
{
    UINT16 phyFromEBlock;
    UINT16 logicalEBNum;
    UINT16 tableOffset;
    UINT8  type;
    UINT8  entryNum;
} FLUSH_MOVE_ENTRY;  /*  8*/

typedef struct _chainInfo
{
    UINT8 isChained;
    FTL_DEV devID;
    UINT16 logChainToEB;
    UINT16 phyChainToEB;
    UINT32 phyPageAddr;
} CHAIN_INFO;  /*  5 */

typedef struct _emptyList
{
    UINT16 logEBNum;
    UINT8 isErased;
    UINT32 eraseScore;
} EMPTY_LIST, *EMPTY_LIST_PTR;  /*  5 */

typedef struct _transferEB{
    FTL_DEV devID;
    UINT16 logicalEBNum;
} TRANSFER_EB, *TRANSFER_EB_PTR;


#if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)
typedef struct _superEBinfo
{
    UINT8 checkChanged;
    UINT8 checkLost;
    UINT8 checkSuperPF;
    UINT8 checkSysPF;
    UINT32 storeFreePage[NUM_SUPER_SYS_EBLOCKS];
} SUPER_EB_INFO;
#endif  // #if (FTL_SUPER_SYS_EBLOCK == FTL_TRUE)

/*FFFFFFFFFLLLLLLLLLLLAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSHHHHHHHHHHHHHHHHHHH*/
#define FLASH_STATUS UINT32
#define BYTES_PER_CL  LOG_ENTRY_DELTA

typedef struct _DeleteInfo
{
   UINT8 devID;
   UINT32 logicalPageAddr;
   UINT8 sector[NUMBER_OF_SECTORS_PER_PAGE];
} DEL_INFO;
typedef struct _staticWLInfo
{
    UINT32 threshold;
    UINT32 count;
    UINT32 staticWLCallCounter;
} STATIC_WL_INFO;

#if(FTL_RPB_CACHE == FTL_TRUE)
typedef struct _cacheInfo
{
    UINT8 status;
    UINT32 logicalPageAddr;
} CACHE_INFO;

typedef struct _RPBCache
{
    CACHE_INFO cache;
    UINT8 RPB[VIRTUAL_PAGE_SIZE];
} RPB_CACHE;

typedef struct _RPBCacheReadGroup
{
    UINT32 LBA;
    UINT32 NB;
    UINT8_PTR byteBuffer;
} RPB_CACHE_READ_GROUP;

enum
{
    CACHE_EMPTY = EMPTY_BYTE,
    CACHE_CLEAN = 0x01,
    CACHE_DIRTY = 0x02
};
#endif  // #if(FTL_RPB_CACHE == FTL_TRUE)

#if (CACHE_RAM_BD_MODULE == FTL_TRUE)

// For EBlock/PPAMapIndex
typedef struct _cacheInfoEblockPpaMap
{
    UINT16 entryIndex;
    UINT8 flashLogEBArrayCount;
} CACHE_INFO_EBLOCK_PPAMAP, *CACHE_INFO_EBLOCK_PPAMAP_PTR;

// For RamMapIndex
typedef struct _cacheInfoRamMap
{
    UINT8 presentEBM;
    UINT8 presentPPA;
    UINT16 ebmCacheIndex;
    #if (NUMBER_OF_PAGES_PER_EBLOCK < 256)
    UINT8 indexOffset;
    #else
    UINT16 indexOffset;
    #endif
} CACHE_INFO_RAMMAP, *CACHE_INFO_RAMMAP_PTR;

// For EBMCacheIndex
typedef struct _cacheInfoEbmCache
{
    UINT8 cacheStatus;
    UINT8 dependency;
    UINT8 wLRUCount;
    UINT8 rLRUCount;
} CACHE_INFO_EBMCACHE, *CACHE_INFO_EBMCACHE_PTR;


typedef struct _saveStaticWL
{
    UINT16 HighestLogEBNum;
    UINT16 LowestLogEBNum;
    UINT32 HighestCount;
    UINT32 LowestCount;
} SAVE_STATIC_WL;

#define SAVE_CAHIN_VALID_USED_SIZE (6)

typedef struct _saveChainValidUsedPage
{
    UINT16 LogEBNum;
    UINT16 ValidPageCount;
    UINT16 UsedPageCount;
} SAVE_CAHIN_VAILD_USED_PAGE;

#endif // #if (CACHE_RAM_BD_MODULE == FTL_TRUE)

#endif  // #ifndef FTL_DEF_H
