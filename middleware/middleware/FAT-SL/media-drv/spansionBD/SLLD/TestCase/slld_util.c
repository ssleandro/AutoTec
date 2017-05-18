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
#include "slld_util.h"
// boot type - region count table
BTRC BootTable[] ={
        {BT_UNIFORM_NONE,     1},
        {BT_SMALL_TOP_BOTTOM, 3},
        {BT_SMALL_BOTTOM,     2},
        {BT_SMALL_TOP,        2},
        {BT_UNIFORM_BOTTOM,   1},
        {BT_UNIFORM_TOP,      1},
        {BT_UNIFORM_WP_ALL,   1},
        {BT_UNIFORM_WP_TOP_BOTTOM,2},
        {BT_LIMIT,            0}
};
Name_t DeviceIDs[] ={
        {"S25FL_204K",0x12},
        {"S25FL_208K",0x13},
        {"S25FL_116K",0x14},/*16Mbit 2MB*/
        {"S25FL_132K",0x15},/*32Mbit 4MB*/
        {"S25FL_164K",0x16},/*64Mbit 8MB*/
        {"S25FL_128S",0x17},
        {"S25FL_256S",0x18},
        {"",0}
};
Name_t DeviceIdMSB[] ={
        {"128",0x20},
        {"256",0x02},
        {"",0}
};
Name_t DeviceIdLSB[] ={
        {"128",0x18},
        {"256",0x19},
        {"",0}
};
Name_t DeviceIDFamilys[] ={
        {"S25FL-",0x80},
        {"S25FS-",0x81},
        {"",0}
};
Name_t FLS_Model_number0[] ={
        {"EHPLC, SO/WSON footprint                           " ,'0'},
        {"EHPLC, 5 x 5 ball BGA footprint                    " ,'2'},
        {"EHPLC, 4 x 6 ball BGA footprint                    " ,'3'},
        {"EHPLC, SO footprint with RESET#                    " ,'G'},
        {"EHPLC, SO footprint with RESET# and VIO            " ,'R'},
        {"EHPLC, 5 x 5 ball BGA footprint with RESET# and VIO" ,'A'},
        {"EHPLC, 4 x 6 ball BGA footprint with RESET# and VIO" ,'B'},
        {"EHPLC, 5 x 5 ball BGA footprint with RESET#         ",'C'},
        {"EHPLC, 4 x 6 ball BGA footprint with RESET#         ",'D'},
        {"HPLC, SO/WSON footprint                             ",'9'},
        {"HPLC, 5 x 5 ball BGA footprint                      ",'4'},
        {"HPLC, 4 x 6 ball BGA footprint                      ",'8'},
        {"HPLC, SO footprint with RESET#                      ",'H'},
        {"HPLC, SO footprint with RESET# and VIO              ",'Q'},
        {"HPLC, 5 x 5 ball BGA footprint with RESET# and VIO  ",'7'},
        {"HPLC, 4 x 6 ball BGA footprint with RESET# and VIO  ",'6'},
        {"HPLC, 5 x 5 ball BGA footprint with RESET#          ",'E'},
        {"HPLC, 4 x 6 ball BGA footprint with RESET#          ",'F'},
        {"",0}
};
Name_t FLS_Model_number1[] ={
        {"Uniform 64-kB sectors",'0'},
        {"Uniform 256-kB sectors",'1'},
        {"",0}
};

Name_t FL127S_Model_number0[] ={
        {"SO/USON footprint                            ",'1'},
        {"5 x 5 ball BGA footprint with RESET#         ",'C'},
        {"4 x 6 ball BGA footprint with RESET#         ",'D'},
        {"",0}
};
Name_t FL127S_Model_number1[] ={
        {"Uniform 64-kB sectors",'0'},
        {"",0}
};

Name_t FSS_Model_number0[] ={
        {"SOIC16 / WSON footprint, 64-kB Physical Sector ",'0'},
        {"SOIC8 / USON, 64-kB Physical Sector            ",'1'},
        {"5x5 ball BGA footprint, 64-kB Physical Sector  ",'2'},
        {"4x6 ball BGA footprint, 64-kB Physical Sector  ",'3'},
        {"VSOP8, 64-kB Physical Sector                   ",'5'},
        {"",0}
};
Name_t FSS_Model_number1[] ={
        {"Uniform 64-kB sectors",'0'},
        {"",0}
};
char *EraseModeName[]={
        "PARAM_4K_SECTOR_ERASE",
        "PARAM_4K_SECTOR_4B_ERASE",
        "PARAM_8K_SECTOR_ERASE",
        "PARAM_8K_SECTOR_4B_ERASE",
        "SECTOR_ERASE",
        "SECTOR_4B_ERASE",
        "BULK_ERASE",
        "BULK_ERASE1",
        "BLOCK_ERASE",
};
char *PrgModeName[]={
        "PROG_PAGE",
        "PROG_PAGE_4B",
        "PROG_PAGE_QUAD",
        "PROG_PAGE_QUAD_4B",

};
char *ReadModeName[]={
        "NORMAL_READ",
        "NORMAL_4B_READ",
        "FAST_RAED",
        "FAST_4B_RAED",
        "DUAL_OUT_RAED",
        "DUAL_4BOUT_RAED",
        "QUAD_OUT_RAED",
        "QUAD_4BOUT_RAED",
        "DUAL_IO_RAED",
        "DUAL_4BIO_RAED",
        "QUAD_IO_RAED",
        "QUAD_4BIO_RAED",
        "DDR_FAST_RAED",
        "DDR_FAST_4B_RAED",
        "DDR_DURAL_IO_RAED",
        "DDR_DURAL_4B_IO_RAED",
        "DDR_QUAD_IO_RAED",
        "DDR_QUAD_4B_IO_RAED",
};
char * get_nameByVal(Name_t * t,unsigned int Val){
    while(strcmp(t->name, "")){
        if(t->Value == Val)return t->name;
        t++;
    }
    return "";
}

/******************************************************************************
 *
 * Function Description:  Function to probe device
 *
 * Parameters: None
 *
 * RETURNS:
      Function call status:  SLLD_OK  -  slld Device Found and information is correct
                             SLLD_ERROR  - slld DEvice Not Found or information is not correct
 ********************************************************************************/
DeviceInfo g_DevInfo;
extern BYTECOUNT g_PAGE_SIZE;
SLLD_STATUS slld_ProbeDevice(){
    unsigned int i,SectorMax = 0;
    BYTE IsBottom = TRUE;
    DeviceInfo* Dev = slld_GetGeoInfo();

#ifdef FL_1K
    BYTE IDBuf[2];
    BYTE SFDP[256];
    if( SLLD_OK != (slld_Read_IdentificationCmd(IDBuf, 0x0))) return printf("slld_Read_IdentificationCmd error!\n"),SLLD_ERROR;//manufacturer read out first followed by the DeviceID
    if(IDBuf[0] != 0x01)return printf("IDBuf[0] != 0x01 !\n"),SLLD_ERROR;
    Dev->DevId = IDBuf[1];
    printf("%s Device is Found ,Device ID = %2.2X\n",get_nameByVal(DeviceIDs,IDBuf[1]),IDBuf[1]);//%s %s Found Manufacturer ID = %2.2X\n", ID[0]
    if( SLLD_OK != slld_ReadSFDPCmd(0,SFDP)) return printf("Serial Flash Discoverable Parameter SFDP error!\n"),SLLD_ERROR;
    Dev->Memsize = 0x200000<<(SFDP[SFDP[0x0c] + 7]);/*0x200000 = 2MB 16Mbit*/
    Dev->PageSize = 256;
    SectorMax = 4096;
    Dev->MaxSectorSize = SectorMax;
    Dev->BlockTotalNumber = Dev->Memsize/65536;/*block size = 64k*/
    Dev->SectorTotalNumber = Dev->Memsize/Dev->MaxSectorSize;/*sector size = 4k*/

    /*for 1k family sector size always is  4k , block size always is 64k*/
    if( !(Dev->pSectorArray  =  malloc(sizeof(SectorInfo)*Dev->SectorTotalNumber)))return printf("Dev->pSectorArray fail \n"),SLLD_ERROR;
    if( !(Dev->pBlockArray  =  malloc(sizeof(BlockInfo)*Dev->BlockTotalNumber)))return printf("Dev->pBlockArray fail \n"),SLLD_ERROR;

    for(i = 0; i < Dev->BlockTotalNumber; i++){
        Dev->pBlockArray[i].BlockSize = 65536;
        Dev->pBlockArray[i].BlockSectorNum = Dev->pBlockArray[i].BlockSize/Dev->MaxSectorSize;
        Dev->pBlockArray[i].BlockAddr = i * Dev->pBlockArray[i].BlockSize;
    }
    for(i = 0; i < Dev->SectorTotalNumber; i++)
    {
        Dev->pSectorArray[i].SectorSize = Dev->MaxSectorSize;
        Dev->pSectorArray[i].SectorAddr = i * Dev->pSectorArray[i].SectorSize;
    }
    Dev->IsBottom = IsBottom;
    Dev->BankTotalNumber = 1;
    Dev->RegionTotalNumber = 1;
    if( !(Dev->pRegionArray  =  malloc(sizeof(RegionInfo)*Dev->RegionTotalNumber)))return printf("Dev->pRegionArray fail \n"),SLLD_ERROR;
    Dev->pRegionArray[0].SecotrNumber = Dev->SectorTotalNumber;
    Dev->pRegionArray[0].SectorSize = SectorMax;
    Dev->pRegionArray[0].RegionAddr = 0;
#else
    BYTE CFI[81];
#ifndef REMOVE_Read_IDCmd
    BYTE IDBuf[1];
#endif
#ifndef REMOVE_RCRCmd
    FLASHDATA CfgReg;
#endif
    unsigned int j,tmp,offset = 0;
#if (defined FL_127S || defined FS_S)
    BYTE rdstatus;
#endif
    if( SLLD_OK != (slld_RDIDCmd(CFI, 81)))return printf("slld_RDIDCmd error!\n"),SLLD_ERROR;
    if(CFI[0] != 0x01)return printf("CFI[0] != 0x01 !\n"),SLLD_ERROR;
    printf("CFG content:\n");
    for(i =0 ;i <0x40 ;i++){
        if(i%16)
            printf("%02x ",CFI[i]);
        else{
            printf("\n");
            printf("0x%08x %02x ",i,CFI[i]);
        }
    }
    printf("\n");
#ifndef REMOVE_Read_IDCmd
    if( SLLD_OK != (slld_Read_IDCmd(IDBuf)))return printf("slld_Read_IDCmd error!\n"),SLLD_ERROR;
    Dev->DevId = IDBuf[0];
    printf("Device %s is Found ,Device ID = %2.2X\n",get_nameByVal(DeviceIDs,IDBuf[0]),IDBuf[0]);
#else
    printf("Device %s%s is Found \n",get_nameByVal(DeviceIDFamilys,CFI[5]),get_nameByVal(DeviceIdLSB,CFI[2]));

#endif
    printf("Order Information :\n");
#ifdef FL_S
#ifdef FL_127S
    printf("%c ,%s\n",CFI[6],get_nameByVal(FL127S_Model_number0,CFI[6]));
    printf("%c ,%s\n",CFI[7],get_nameByVal(FL127S_Model_number1,CFI[7]));
#endif
    switch(CFI[6]){
    case '0':
    case '2':
    case '3':
    case 'G':
    case 'R':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
#ifndef FL_127S
        Dev->SupportEHP = 1;
#endif
        break;
    default:
        Dev->SupportEHP = 0;
        break;
    }
#endif
#ifdef FS_S
    printf("%c ,%s\n",CFI[6],get_nameByVal(FSS_Model_number0,CFI[6]));
    printf("%c ,%s\n",CFI[7],get_nameByVal(FSS_Model_number1,CFI[7]));
#endif

    /* Get page size */
    if (CFI[CFI_PAGEMODE]){
        if (CFI[CFI_PAGEMODE] == 1)
            Dev->PageSize = 4;
        else if (CFI[CFI_PAGEMODE] == 2)
            Dev->PageSize = 8;
        else if (CFI[CFI_PAGEMODE] == 3)
            Dev->PageSize = 256;
        else if (CFI[CFI_PAGEMODE] == 4)
            Dev->PageSize = 512;
    }
    /*for FL127S page size can be changed by OPT SR[6]bit*/
#ifdef FL_127S
    if(SLLD_OK != slld_RDSR2Cmd(&rdstatus))return SLLD_ERROR;
    if(rdstatus & 0x40){
        Dev->PageSize = 512;
        g_PAGE_SIZE = 512;
    }
    else
    {
        Dev->PageSize = 256;
        g_PAGE_SIZE = 256;
    }

#endif
    /*for FS_S page size can be changed by OPT CR3V[4]bit*/
#ifdef FS_S
    if(SLLD_OK != slld_RDARCmd(CR3V,&rdstatus))return SLLD_ERROR;
    if(rdstatus & 0x10){
        Dev->PageSize = 512;
        g_PAGE_SIZE = 512;
    }
    else
    {
        Dev->PageSize = 256;
        g_PAGE_SIZE = 256;
    }

#endif
    Dev->BootType = CFI[CFI_BOOTFLAG];
    //if(CFI[CFI_REGIONCOUNT] != BootTable[Dev->BootType].regions)return  printf("Dev->BootType %d \n",Dev->BootType) ,SLLD_ERROR;
    /* Memory size */
    Dev->Memsize = (1 << CFI[CFI_DEVSIZE]);
    /*****************************\
     ****   Set Bank Info   ********
    \*****************************/
    Dev->BankTotalNumber = Dev->Memsize/0x1000000;//16MB one bank
    if( !(Dev->pBankArray  =  malloc(sizeof(BankInfo)*Dev->BankTotalNumber)))return printf("Dev->pBankArray fail \n"),SLLD_ERROR;
    for (i = 0; i < Dev->BankTotalNumber; i++){
        Dev->pBankArray[i].BankAddr = 0x1000000*i;
        Dev->pBankArray[i].BankSize = 0x1000000;
    }
    /*****************************\
     ****    Set Region Info  ******
    \*****************************/
    Dev->RegionTotalNumber = CFI[CFI_REGIONCOUNT];
    if(Dev->RegionTotalNumber == 0)return printf("RegionTotalNumber should big than 0!\n"),SLLD_ERROR;

    if( !(Dev->pRegionArray  =  malloc(sizeof(RegionInfo)*Dev->RegionTotalNumber)))return printf("Dev->pRegionArray fail \n"),SLLD_ERROR;
    Dev->SectorTotalNumber =0;
    offset = 0;

    /*judge if the small sector in the Bottom*/
#ifndef REMOVE_RCRCmd
    slld_RCRCmd(&CfgReg);
    IsBottom = (CfgReg & 0x4) ? FALSE : TRUE;
#else
    IsBottom = TRUE;
#endif
    if(!IsBottom){//top reverse configuration for following calculation
        for (i=0; i<4; i++){
            tmp = CFI[0x2D+i];
            CFI[0x2D+i] = CFI[0x31+i];
            CFI[0x31+i] = tmp;
        }
    }
    for (i = 0; i < Dev->RegionTotalNumber; i++){
#ifdef FL_127S
        if(i == 0)
            Dev->pRegionArray[i].SecotrNumber =16;
        else
            Dev->pRegionArray[i].SecotrNumber = ((CFI[0x2E + (i * 4)] << 8) + (CFI[0x2D + (i * 4)])) + 1;
#else

        Dev->pRegionArray[i].SecotrNumber = ((CFI[0x2E + (i * 4)] << 8) + (CFI[0x2D + (i * 4)])) + 1;
#endif
        Dev->pRegionArray[i].SectorSize = ((CFI[0x30 + (i * 4)] << 16) + (CFI[0x2F + (i * 4)] << 8));
        Dev->pRegionArray[i].RegionAddr = offset;
        offset += Dev->pRegionArray[i].SecotrNumber * Dev->pRegionArray[i].SectorSize;
        Dev->SectorTotalNumber += Dev->pRegionArray[i].SecotrNumber;
        if(Dev->pRegionArray[i].SectorSize > SectorMax) SectorMax = Dev->pRegionArray[i].SectorSize;
    }

    /*****************************\
     ****    Set Sector Info  ******
    \*****************************/
    Dev->MaxSectorSize = SectorMax;
    if( !(Dev->pSectorArray  =  malloc(sizeof(SectorInfo)*Dev->SectorTotalNumber)))return printf("Dev->pSectorArray fail \n"),SLLD_ERROR;
    offset = 0;
    for (i = 0; i < Dev->RegionTotalNumber; i++){
        for(j =0 ;j < Dev->pRegionArray[i].SecotrNumber;j++)
        {
            Dev->pSectorArray[offset].SectorAddr = Dev->pRegionArray[i].RegionAddr + j * Dev->pRegionArray[i].SectorSize;
            Dev->pSectorArray[offset].SectorSize = Dev->pRegionArray[i].SectorSize;
            offset++;
        }
    }

    /*****************************\
     ****    Set Block Info  ******
    \*****************************/
    //if( !(Dev->pBlockArray  =  malloc(sizeof(BlockInfo)*Dev->BlockTotalNumber)))return printf("Dev->pBlockArray fail \n"),SLLD_ERROR;
    Dev->BlockTotalNumber = 1;

#endif
    return SLLD_OK;
}
/******************************************************************************
 *
 * Function Description:  Function to get device geometry information
 *
 * Parameters: None
 *
 * RETURNS:
  DeviceInfo *
 ********************************************************************************/
DeviceInfo* slld_GetGeoInfo(){
    return &g_DevInfo;
}

/******************************************************************************
 *
 * Function Description:  Check sector is erased or not
 *
 * Parameters: SecAddr   sector address
 *             SecSize   sector size
 *
 * RETURNS: TRUE OR FALSE
 ********************************************************************************/
SLLD_STATUS slld_IfSecErased (ADDRESS SecAddr , BYTECOUNT SecSize)
{
    unsigned int ix = 0;
    DEVSTATUS ret = SLLD_ERROR;
    FLASHDATA * rdBuf ;
    rdBuf = malloc(SecSize);
    if(SLLD_OK != slld_ReadFlash(SecAddr,rdBuf,SecSize,256,NORMAL_READ))goto end;
    for(ix=0; ix< SecSize; ix++)
        if ( rdBuf[ix] !=  0xFF){
            printf("0x%08x:SecotrAddr[%d] = 0x%02x is NOT equal to 0xFF\n",(unsigned int)SecAddr,ix,rdBuf[ix]);
            goto end;
        }
    ret = SLLD_OK;
    end:
    free(rdBuf);
    return ret;
}
/******************************************************************************
 *
 * Function Description:  Function to erase device
 *
 * Parameters:  addr   address of Sector
 *              mode   EraseMode
 * RETURNS:SLLD_ERROR or SLLD_OK
  DeviceInfo *
 ********************************************************************************/
SLLD_STATUS slld_EraseFlash(ADDRESS addr,BYTE mode)
{
    DEVSTATUS status;
    /*printf("Erase mode %s\n",EraseModeName[mode]);*/
    switch(mode){
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_P4EOp
    case PARAM_4K_SECTOR_ERASE:
        if(SLLD_OK != slld_P4EOp(addr,&status))
        {
            printf("slld_P4EOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#ifndef REMOVE_P4E_4BOp
    case PARAM_4K_SECTOR_4B_ERASE:
        if(SLLD_OK != slld_P4E_4BOp(addr,&status))
        {
            printf("slld_P4E_4BOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#endif
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_P8EOp
    case PARAM_8K_SECTOR_ERASE:
        if(SLLD_OK != slld_P8EOp(addr,&status))
        {
            printf("slld_P8EOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#endif
#ifndef REMOVE_P8E_4BOp
    case PARAM_8K_SECTOR_4B_ERASE:
        if(SLLD_OK != slld_P8E_4BOp(addr,&status)){
            printf("slld_P8E_4BOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#ifndef REMOVE_SEOp
    case SECTOR_ERASE:
        if(SLLD_OK != slld_SEOp(addr,&status))
        {
            printf("slld_SEOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#ifndef REMOVE_SE_4BOp
    case SECTOR_4B_ERASE:
        if(SLLD_OK != slld_SE_4BOp(addr,&status)){
            printf("slld_SE_4BOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#ifndef REMOVE_BEOp
    case BULK_ERASE:
        if(SLLD_OK != slld_BEOp(&status)){
            printf("slld_BEOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
    case BULK_ERASE1:
        if(SLLD_OK != slld_BE1Op(&status))
        {
            printf("slld_BE1Op error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
#ifndef REMOVE_BLOCKERASEOp
    case BLOCK_ERASE:
        if(SLLD_OK != slld_BlockEraseOp(addr,&status))
        {
            printf("slld_BlockEraseOp error status = %d\n",status);
            return SLLD_ERROR;
        }
        break;
#endif
    default:
        return printf("mode %s can't supported \n",EraseModeName[mode]),SLLD_ERROR;
        break;
    }
    return SLLD_OK;
}
/******************************************************************************
 *
 * Function Description:  Function to program device
 *
 * Parameters:  addr   address of Sector
 *              buf    point to data buffer
 *              len    length of the buffer
 *              LimitSize size of the write in one time command,e.g. pageSize
 *              mode   PgrgMode
 * RETURNS:SLLD_ERROR or SLLD_OK
  DeviceInfo *
 ********************************************************************************/
SLLD_STATUS slld_ProgramFlash(ADDRESS addr, FLASHDATA *buf ,BYTECOUNT len, unsigned int LimitSize, BYTE mode){
    DEVSTATUS status;
    FLASHDATA *data_buf = buf;
    unsigned int ix = len/LimitSize + 1;
    unsigned int left = len%LimitSize;
    unsigned int size = (ix == 1)?left:LimitSize;
    /*printf("Program mode %s\n",PrgModeName[mode]);*/
    while(ix--){
        switch(mode){
#ifndef REMOVE_PPOp
        case PROG_PAGE:
            if( SLLD_OK != (slld_PPOp(addr,data_buf,size,&status))){
                printf("slld_PPOp status = %d\n",status);
                return SLLD_ERROR;
            }
            break;
#endif
#ifndef REMOVE_PP_4BOp
        case PROG_PAGE_4B:
            if( SLLD_OK != (slld_PP_4BOp(addr,data_buf,size,&status))){
                printf("slld_PP_4BOp status = %d\n",status);
                return SLLD_ERROR;
            }
            break;
#endif
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_QUAD_PAGE_PROGRAMCmd
        case PROG_PAGE_QUAD:
            if( SLLD_OK !=(slld_QPPOp(addr,data_buf,size,&status))){
                printf(" slld_QPPOp status = %d\n",status);
                return SLLD_ERROR;
            }
            break;
#endif
#endif
#ifndef REMOVE_QUAD_PAGE_PROGRAM_4BOp
        case PROG_PAGE_QUAD_4B:

            if( SLLD_OK != (slld_QPP_4BOp(addr,data_buf,size,&status))){
                printf("slld_QPP_4BOp status = %d\n",status);
                return SLLD_ERROR;
            }
            break;
#endif
        default:
            //printf("mode %s can't supported ,trying to use %s mode\n",PrgModeName[mode],PrgModeName[PROG_PAGE]);
            if( SLLD_OK != (slld_PPOp(addr,data_buf,size,&status))){
                printf("slld_PPOp status = %d\n",status);
                return SLLD_ERROR;
            }
            break;
        }
        addr += size;
        data_buf += size;
        if(ix == 1) size = left; /*left*/
    }/*end while()*/
    return SLLD_OK;
}
/******************************************************************************
 *
 * Function Description:  Function to read from device
 *
 * Parameters:  addr   address of Sector
 *              buf    point to data buffer to store read data
 *              len    length of the buffer to be read
 *              LimitSize size of the read data in one read command
 *              mode   PgrgMode
 * RETURNS:SLLD_ERROR or SLLD_OK
  DeviceInfo *
 ********************************************************************************/
SLLD_STATUS slld_ReadFlash(ADDRESS addr,FLASHDATA *buf ,BYTECOUNT len,unsigned int LimitSize,BYTE mode){
    FLASHDATA *data_buf = buf;
    unsigned int ix = len/LimitSize + 1;
    unsigned int left = len%LimitSize;
    unsigned int size = (ix == 1)?left:LimitSize;
    BYTE       modebit;
    /*printf("Read mode %s\n",ReadModeName[mode]);*/
    while(ix--){
        switch (mode) {
#ifndef REMOVE_ReadCmd
        case  NORMAL_READ:
            if( SLLD_OK != (slld_ReadCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_Read_4BCmd
        case  NORMAL_4B_READ:
            if( SLLD_OK != (slld_Read_4BCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_Fast_ReadCmd
        case  FAST_RAED:
            if( SLLD_OK != (slld_Fast_ReadCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_Fast_Read_4BCmd
        case  FAST_4B_RAED:
            if( SLLD_OK != (slld_Fast_Read_4BCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_DUALOUTPUT_READCmd
        case  DUAL_OUT_RAED:

            if( SLLD_OK != (slld_DualIOReadCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifndef REMOVE_DUALOUTPUT_READ_4BCmd
        case  DUAL_4BOUT_RAED:
            if( SLLD_OK != (slld_DualIORead_4BCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_QUADOUTPUT_READCmd
        case  QUAD_OUT_RAED:
            if( SLLD_OK !=(slld_QuadIOReadCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifndef REMOVE_QUADOUTPUT_READ_4BCmd
        case  QUAD_4BOUT_RAED:
            if( SLLD_OK != (slld_QuadIORead_4BCmd(addr,data_buf,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_DUALOUTPUT_HP_READCmd
        case  DUAL_IO_RAED:
            modebit = 0x0;
            if(SLLD_OK != (slld_DualIOHPReadCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifndef REMOVE_DUALOUTPUT_HP_READ_4BCmd
        case  DUAL_4BIO_RAED:
            modebit = 0x0;
            if(SLLD_OK != (slld_DualIOHPRead_4BCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_QUADOUTPUT_HP_READCmd
        case  QUAD_IO_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_QuadIOHPReadCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifndef REMOVE_QUADOUTPUT_HP_READ_4BCmd
        case  QUAD_4BIO_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_QuadIOHPRead_4BCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#ifdef FLS_CMD
#ifndef REMOVE_DDR_Fast_ReadCmd
        case  DDR_FAST_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_DDR_Fast_ReadCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifdef FLS_CMD
#ifndef REMOVE_DDR_Fast_4BReadCmd
        case  DDR_FAST_4B_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_DDR_Fast_4BReadCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifdef FLS_CMD
#ifndef REMOVE_DDR_DUALOUTPUT_HP_READCmd
        case  DDR_DURAL_IO_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_DDR_DualIOHPReadCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifdef FLS_CMD
#ifndef REMOVE_DDR_DUALOUTPUT_HP_READ_4BCmd
        case  DDR_DURAL_4B_IO_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_DDR_DualIOHPRead_4BCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifdef FLS_CMD
#ifndef REMOVE_DDR_QUADOUTPUT_HP_READCmd
        case  DDR_QUAD_IO_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_DDR_QuadIOHPReadCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
#ifdef FLS_CMD
#ifndef REMOVE_DDR_QUADOUTPUT_HP_READ_4BCmd
        case  DDR_QUAD_4B_IO_RAED:
            modebit = 0x0;
            if( SLLD_OK != (slld_DDR_QuadIOHPRead_4BCmd(addr,data_buf,modebit,size)))return SLLD_ERROR;
            break;
#endif
#endif
        default:
            return printf("mode %s can't supported \n",ReadModeName[mode]),SLLD_ERROR;
            break;
        }
        addr += size;
        data_buf += size;
        if(ix == 1) size = left; /*left*/
    }
    return SLLD_OK;
}
/******************************************************************************
 *
 * Function Description:  Function to set QUAD mode bit
 *
 * Parameters: mode          QUAD_ENABLE or QUAD_DISABLE
 *
 * RETURNS:
      Function call status:  SLLD_OK
                             SLLD_ERROR
 ********************************************************************************/

SLLD_STATUS slld_QuadModeCtrl(BYTE mode){

#ifndef REMOVE_RCRCmd
    DEVSTATUS devStatus;
    BYTE cfg_val,status_val = 0;
    if(SLLD_OK != slld_RDSRCmd(&status_val)) return SLLD_ERROR;
    if(mode == QUAD_ENABLE){
        if ( SLLD_OK != slld_RCRCmd(&cfg_val))return SLLD_ERROR;
        cfg_val |= 0x2;
        if ( SLLD_OK != slld_WRROp(&status_val,&cfg_val,NULL,&devStatus)) return SLLD_ERROR;
    }
    else{
        if ( SLLD_OK != slld_RCRCmd(&cfg_val))return SLLD_ERROR;
        cfg_val &= ~0x2;
        if ( SLLD_OK != slld_WRROp(&status_val,&cfg_val,NULL,&devStatus)) return SLLD_ERROR;
    }
#endif
#ifdef FL_1K
    DEVSTATUS devStatus;
    BYTE status_val[3] = {0};
    if(SLLD_OK != slld_RDSRCmd(&status_val[0])) return SLLD_ERROR;
    if(SLLD_OK != slld_RDSR2Cmd(&status_val[1])) return SLLD_ERROR;
    if(mode == QUAD_ENABLE){
        status_val[1] |= 0x2;
        if ( SLLD_OK != slld_WRSRegOp(status_val,VOLATILE_WR,2,&devStatus)) return SLLD_ERROR;
    }
    else{
        status_val[1] &= ~0x2;
        if ( SLLD_OK != slld_WRSRegOp(status_val,VOLATILE_WR,2,&devStatus)) return SLLD_ERROR;
    }
#endif
    return SLLD_OK;
}


/******************************************************************************
 *
 * Function Description:  Function to set LC (latency code)bit
 *
 * Parameters: value      latency code value
 *
 * RETURNS:
      Function call status:  SLLD_OK
                             SLLD_ERROR
 ********************************************************************************/

SLLD_STATUS slld_SetLatencyCode(BYTE value){
#ifndef REMOVE_RCRCmd
    DEVSTATUS devStatus;
    BYTE cfg_val,status_val = 0;
    if(SLLD_OK != slld_RDSRCmd(&status_val)) return SLLD_ERROR;
    if ( SLLD_OK != slld_RCRCmd(&cfg_val))return SLLD_ERROR;
    cfg_val &= ~0xc0;
    cfg_val |= (value&0x3)<<6;
    if ( SLLD_OK != slld_WRROp(&status_val,&cfg_val,NULL,&devStatus)) return SLLD_ERROR;
#endif
#ifdef FL_1K
    DEVSTATUS devStatus;
    BYTE status_val[3] = {0};
    if(SLLD_OK != slld_RDSRCmd(&status_val[0])) return SLLD_ERROR;
    if(SLLD_OK != slld_RDSR2Cmd(&status_val[1])) return SLLD_ERROR;
    if(SLLD_OK != slld_RDSR3Cmd(&status_val[2])) return SLLD_ERROR;
    status_val[3] &= 0x0f;
    status_val[3] |= value&0xf;
    if ( SLLD_OK != slld_WRSRegOp(status_val,VOLATILE_WR,3,&devStatus)) return SLLD_ERROR;
#endif
    return SLLD_OK;
}
/******************************************************************************
 *
 * Function Description:  Function to set burst wrap bit
 *
 * Parameters: mode      BURST_ENABLE_W8,
                         BURST_ENABLE_W16,
                         BURST_ENABLE_W32,
                         BURST_ENABLE_W64,
                         BURST_DISABLE
 *
 * RETURNS:
      Function call status:  SLLD_OK
                             SLLD_ERROR
 ********************************************************************************/
#ifdef INCL_SPI_BURSTWRAPCMD
SLLD_STATUS slld_SetBurstWrapMode(BYTE mode){
    if(!(mode == BURST_ENABLE_W8 ||
            mode == BURST_ENABLE_W16 ||
            mode == BURST_ENABLE_W32 ||
            mode == BURST_ENABLE_W64 ||
            mode == BURST_DISABLE))return SLLD_ERROR;

    if(SLLD_OK != slld_SetBurstWrapCmd(&mode)) return SLLD_ERROR;
    return SLLD_OK;
}
#endif

#if (defined(FL_1K)|| defined(FL_K))
/******************************************************************************
 *
 * Function Description:  Function to erase Security register
 *
 * Parameters:  addr   address of Security register
 *                     SECURITY_REG0
 *                     SECURITY_REG1
 *                     SECURITY_REG2
 *                     SECURITY_REG3
 * RETURNS:SLLD_ERROR or SLLD_OK
  DeviceInfo *
 ********************************************************************************/
SLLD_STATUS slld_SecurityRegErase(ADDRESS addr)
{
    DEVSTATUS devStatus;

    if( !((addr >= SECURITY_REG1 && addr <= SECURITY_REG1 + 256) ||
            (addr >= SECURITY_REG2 && addr <= SECURITY_REG2 + 256) ||
            (addr >= SECURITY_REG3 && addr <= SECURITY_REG3 + 256)))return SLLD_ERROR;
    if(SLLD_OK != slld_EraseSecurityOp(addr,&devStatus)) return SLLD_ERROR;
    return SLLD_OK;
}
#endif
#ifdef FS_S
#ifndef REMOVE_RDARCmd
/******************************************************************************
 *
 * Function Description:  Function to set QUAD all mode bit
 *
 * Parameters: mode          QUAD_ALL_ENABLE or QUAD_ALL_DISABLE
 *
 * RETURNS:
      Function call status:  SLLD_OK
                             SLLD_ERROR
 ********************************************************************************/

SLLD_STATUS slld_QuadAllModeCtrl(BYTE mode){

    DEVSTATUS devStatus;
    BYTE val = 0;
    if(SLLD_OK != slld_RDARCmd(CR2V,&val)) return SLLD_ERROR;
    if(mode == QUAD_ALL_ENABLE){
        val |= 0x40;
        if ( SLLD_OK != slld_WRAR_Op(CR2V,&val,&devStatus)) return SLLD_ERROR;
    }
    else{
        val &= ~0x40;
        if ( SLLD_OK != slld_WRAR_Op(CR2V,&val,&devStatus)) return SLLD_ERROR;
    }

    return SLLD_OK;
}
#endif

#ifndef REMOVE_ESSCmd
/******************************************************************************
 *
 * Function Description:  Function to Evaluate Erase Status
 *
 * Parameters: addr   address of sector
 *
 * RETURNS:
      Function call status:  SLLD_OK
                             SLLD_ERROR
 ********************************************************************************/
SLLD_STATUS slld_EvaluateEraseStatus(ADDRESS addr){
    DEVSTATUS devStatus;
    BYTE val = 0;
    if(SLLD_OK != slld_ESSCmd(addr)) return SLLD_ERROR;
    if(SLLD_OK != slld_Poll(&devStatus)) return SLLD_ERROR;
    if(SLLD_OK != slld_RDARCmd(SR2V,&val)) return SLLD_ERROR;
    if((val & 0x04) == 0) return SLLD_ERROR;
    return SLLD_OK;
}
#endif

#endif //FS_S
