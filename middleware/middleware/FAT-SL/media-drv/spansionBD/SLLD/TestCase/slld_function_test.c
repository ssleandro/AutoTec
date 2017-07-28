/**************************************************************************
 * Copyright (C)2013 Spansion LLC. All Rights Reserved .
 *
 * This software is owned and published by:
 * Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <lld_test_util.h>

static  TestCaseTable_t BasicTestCaseTable;
/*****************************\
 ****test case implementation******
\*****************************/
extern char *EraseModeName[];
extern char *PrgModeName[];
extern char *ReadModeName[];
static bool SectorPgrmReadRandomVal(uint32_t SectorNo,uint32_t len,BYTE Ersmode,BYTE Pgrmode,BYTE Readmode){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorAddr,SectorSize;
    Testbuf_t *tbuf;
    uint32_t PageSize = DevInfo->PageSize;
    bool ret = false;
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    tsc_printf("Sector%3d Program/Read/Verify with random data,<%s, %s, %s>\n",SectorNo,EraseModeName[Ersmode],PrgModeName[Pgrmode],ReadModeName[Readmode]);
    if(!apiok(slld_EraseFlash(SectorAddr,Ersmode)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;

    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,len)))goto end;
    if(!apiok(slld_ProgramFlash(SectorAddr,tbuf->wbuf,len,PageSize,Pgrmode))) goto end;

    /*read and verify*/
    if(!apiok(slld_ReadFlash(SectorAddr,tbuf->rbuf,len,PageSize,Readmode))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
static bool SectorPgrmReadOp(uint32_t SectorNo,uint32_t len,BYTE Ersmode,BYTE Pgrmode,BYTE Readmode){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorAddr,SectorSize;
    Testbuf_t *tbuf;
    bool ret = false;
    DEVSTATUS status;
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    tsc_printf("Sector%3d Program/Read/Verify with random data,<%s, %s, %s>\n",SectorNo,EraseModeName[Ersmode],PrgModeName[Pgrmode],ReadModeName[Readmode]);
    if(!apiok(slld_SEOp(SectorAddr,&status)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;

    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,len)))goto end;
    if(!apiok(slld_WriteOp(SectorAddr,tbuf->wbuf,len,&status))) goto end;

    /*read and verify*/
    if(!apiok(slld_ReadOp(SectorAddr,tbuf->rbuf,len))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
static bool SectorPgrmReadFixedVal(uint32_t SectorNo,uint32_t len,BYTE value,BYTE Ersmode,BYTE Pgrmode,BYTE Readmode){

    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorAddr,SectorSize;
    Testbuf_t *tbuf;
    uint32_t PageSize = DevInfo->PageSize;
    bool ret = false;
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    tsc_printf("Sector%3d Program/Read/Verify with Fix data 0x%02x,<%s, %s, %s>\n",SectorNo,value,EraseModeName[Ersmode],PrgModeName[Pgrmode],ReadModeName[Readmode]);
    if(!apiok(slld_EraseFlash(SectorAddr,Ersmode)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;

    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInitFixedVal(tbuf,len,value)))goto end;
    if(!apiok(slld_ProgramFlash(SectorAddr,tbuf->wbuf,len,PageSize,Pgrmode))) goto end;

    /*read and verify*/
    if(!apiok(slld_ReadFlash(SectorAddr,tbuf->rbuf,len,PageSize,Readmode))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#ifdef INCL_SPI_BURSTWRAPCMD
static bool SectorPgrmBrustWrapRead(uint32_t SectorNo,uint32_t addrOffset,uint32_t len,uint32_t wrapLen,BYTE Ersmode, BYTE Pgrmode){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorAddr,SectorSize,addr;
    Testbuf_t *tbuf;
    uint32_t PageSize = DevInfo->PageSize;
    bool ret = false;
    uint32_t wrappedLen,ix;
    uint32_t Tbuflen = wrapLen * (len/wrapLen + 1);
    uint8_t *wbuf,*rbuf;
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    if(!ok(addrOffset < wrapLen))return false;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    addr = SectorAddr + addrOffset;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d ,burst read address = 0x%02x\n",SectorNo,SectorAddr,SectorSize,addr));
    tsc_printf("Sector%3d Program/Burst warp Read/Verify with random data,<%s, %s QUAD_IO_RAED> wrap length %d\n",SectorNo,EraseModeName[Ersmode],PrgModeName[Pgrmode],wrapLen);


    if(!apiok(slld_EraseFlash(SectorAddr,Ersmode)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;

    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,Tbuflen)))goto end;
    if(!apiok(slld_ProgramFlash(SectorAddr,tbuf->wbuf,len,PageSize,Pgrmode))) goto end;
    /*enable burst mode*/
    switch(wrapLen){
    case 8:
        if ( !apiok(slld_SetBurstWrapMode(BURST_ENABLE_W8)))return false;
        break;
    case 16:
        if ( !apiok(slld_SetBurstWrapMode(BURST_ENABLE_W16)))return false;
        break;
    case 32:
        if ( !apiok(slld_SetBurstWrapMode(BURST_ENABLE_W32)))return false;
        break;
    case 64:
        if ( !apiok(slld_SetBurstWrapMode(BURST_ENABLE_W64)))return false;
        break;
    default :
        break;
    }
    wrappedLen = addr%wrapLen;
    TRC(DEBUG,("wrapLen = %d wrappedLen =%d \n",wrapLen,wrappedLen));
    if(wrappedLen == 0 )	{
        /*read and verify*/
        if(!apiok(slld_ReadFlash(SectorAddr,tbuf->rbuf,len,wrapLen,QUAD_IO_RAED))) goto end;
        //printBuf(tbuf->rbuf,Tbuflen,0);
        if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,len)))goto end;
    }else{
        /*read and verify*/
        if(!apiok(slld_ReadFlash(addr,tbuf->rbuf,Tbuflen,wrapLen,QUAD_IO_RAED))) goto end;
        //printBuf(tbuf->rbuf,Tbuflen,0);
        wbuf = tbuf->wbuf;
        rbuf = tbuf->rbuf;
        for(ix = 0;ix < len/wrapLen; ix++){
            if( !ok( ifEqual(wbuf,rbuf + wrapLen - wrappedLen,wrappedLen)))goto end;/*verify wrapped,wrapped data offset in rd buffer is (wrapLen - wrappedLen)*/
            if( !ok( ifEqual(wbuf + wrappedLen,rbuf,wrapLen - wrappedLen)))goto end;
            wbuf += wrapLen;
            rbuf += wrapLen;
        }
        /*the last*/
        if(len%wrapLen <= wrappedLen){
            if( !ok( ifEqual(wbuf,rbuf + wrapLen - wrappedLen,len%wrapLen)))goto end;
        }
        else{
            if( !ok( ifEqual(wbuf,rbuf + wrapLen - wrappedLen,wrappedLen)))goto end;
            if( !ok( ifEqual(wbuf + wrappedLen,rbuf,len%wrapLen-wrappedLen)))goto end;
        }
    }
    ret = true;
    end:
    if ( !apiok(slld_SetBurstWrapMode(BURST_DISABLE)))return false;
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#endif
#ifndef REMOVE_Read_IDCmd
/*use CMD AB*/
static bool cmd_func_Read_ID( void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    BYTE IDBuf[1];
    if( !apiok(slld_Read_IDCmd(IDBuf)))return false;
    if(DevInfo->DevId != IDBuf[0]){
        tsc_printf("Device Id 0x%02x is not correct expect to 0x%02x,\n",IDBuf[0],DevInfo->DevId);
        return false;
    }
    tsc_printf("Read device ID = 0x%02x\n",IDBuf[0]);
    return true;
}
#endif

#ifndef REMOVE_RDIDCmd
/*use CMD 9F*/
static bool cmd_func_JEDECId( void *d ,int argc, const char *argv[]){
    BYTE CFIBuf[81];
#ifdef FL_1K  /*FL_1K can't support CFI header*/
    if( !apiok(slld_RDIDCmd(CFIBuf, 2)))return false;
    if(!ok(CFIBuf[0] == 0x01))return false;
    tsc_printf("Read Manufacturer ID = 0x%02x , Device TYPE = 0x%02x\n",CFIBuf[0],CFIBuf[1]);
#else
    if( !apiok(slld_RDIDCmd(CFIBuf, 81)))return false;
    if(!ok(CFIBuf[0] == 0x01))return false;
    if (! ok(CFIBuf[0x10] == 'Q' &&
            CFIBuf[0x11] == 'R' &&
            CFIBuf[0x12] == 'Y'))return false;

    if (! ok(CFIBuf[CFIBuf[0x15]] == 'P' &&
            CFIBuf[CFIBuf[0x15] + 1] == 'R' &&
            CFIBuf[CFIBuf[0x15]+ 2] == 'I')){
        TRC(ERROR,("CFIBuf[15] = %d \n",CFIBuf[15]));
        return false;
    }

#endif
    return true;
}
#endif
#ifdef FS_S
#ifndef REMOVE_RDQIDCmd
/*use 0xAF CMD*/
static bool cmd_func_ReadQId( void *d ,int argc, const char *argv[]){
    BYTE CFIBuf[81];
    bool ret = false;
    BYTE val=0;
    if( !apiok(slld_QuadAllModeCtrl(QUAD_ALL_ENABLE)))goto end;
    QspiEnableQuadAll();
    if(SLLD_OK != slld_RDARCmd(CR2V,&val)) goto end;
    if(!ok((val&0x40) != 0))goto end;
    if( !apiok(slld_RDQIDCmd(CFIBuf, 81)))goto end;
    if(!ok(CFIBuf[0] == 0x01))goto end;
    if (! ok(CFIBuf[0x10] == 'Q' &&
            CFIBuf[0x11] == 'R' &&
            CFIBuf[0x12] == 'Y'))goto end;

    if (! ok(CFIBuf[CFIBuf[0x15]] == 'P' &&
            CFIBuf[CFIBuf[0x15] + 1] == 'R' &&
            CFIBuf[CFIBuf[0x15]+ 2] == 'I')){
        TRC(ERROR,("CFIBuf[15] = %d \n",CFIBuf[15]));
        goto end;
    }
    ret = true;
    end:
    if( !apiok(slld_QuadAllModeCtrl(QUAD_ALL_DISABLE)))return false;
    QspiDisableQuadAll();
    return ret;
}
#endif
#endif
#ifndef REMOVE_READ_IDENTIFICATIONCmd
/*use CMD 90*/
static bool cmd_func_Read_Identification( void *d ,int argc, const char *argv[]){
    BYTE IDBuf[2];
    BYTE tmp;
    if( !apiok(slld_Read_IdentificationCmd(IDBuf, 0x0))) return false;//manufacturer read out first followed by the DeviceID
    if(!ok(IDBuf[0] == 0x01))return false;
    tmp = IDBuf[1];
    if( !apiok(slld_Read_IdentificationCmd(IDBuf, 0x1))) return false; //DeviceID read out first followed by the manufacturer
    if(!ok(IDBuf[1] == 0x01))return false;
    if(!ok(IDBuf[0] == tmp))return false;
    tsc_printf("Read Manufacturer ID = %02x , Device ID = %02x\n",IDBuf[1],IDBuf[0]);
    return true;
}
#endif
#ifdef INCL_SPI_READSFDPCMD
static bool cmd_func_Read_SFDP( void *d ,int argc, const char *argv[]){
    bool ret = false;
    Testbuf_t *tbuf;
    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,256)))goto end;
    if( !apiok(slld_ReadSFDPCmd(0, tbuf->rbuf)))goto end;
    if(!ok(tbuf->rbuf[0] == 'S' &&
            tbuf->rbuf[1] == 'F' &&
            tbuf->rbuf[2] == 'D' &&
            tbuf->rbuf[3] == 'P' ))goto end;
    /*print all data*/
    printBuf(tbuf->rbuf,tbuf->len,0);
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#endif
static bool cmd_func_readSector(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorAddr,SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    uint32_t PageSize = DevInfo->PageSize;
    bool ret = false;
    Testbuf_t *tbuf;

    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    TRC(NOTICE,("SectorNo=%u len=%u \n",SectorNo,len));
    if( !ok(len > 0 && len  <= SectorSize))return false;

    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,len)))goto end;
    /*read*/
    if(!apiok(slld_ReadFlash(SectorAddr,tbuf->rbuf,len,PageSize,NORMAL_READ))) goto end;
    /*print all data*/
    printBuf(tbuf->rbuf,tbuf->len,SectorAddr);
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}

static bool cmd_func_readMemory(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t len;
    uint32_t addr = strtoint(argv[1]);
    uint32_t PageSize = DevInfo->PageSize;
    bool ret = false;
    Testbuf_t *tbuf;
    len = strtoint(argv[2]);
    if( !ok(len  > 0))return false;
    if( !ok( (addr + len) <= DevInfo->Memsize))return false;
    TRC(NOTICE,("Address=0x%08x length =%u \n",addr,len));
    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,len)))goto end;
    /*read*/
    if(!apiok(slld_ReadFlash(addr,tbuf->rbuf,len,PageSize,NORMAL_READ))) goto end;
    /*print all data*/
    printBuf(tbuf->rbuf,tbuf->len,addr);
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#if (defined(FL_1K)|| defined(FL_K))
static bool cmd_func_ReadSecuRegs(void *d ,int argc, const char *argv[]){
    uint32_t SecurityRegAddr;
    uint8_t buf[256]={0};
    uint32_t ix = strtoint(argv[1]);
    if( !ok(ix >= 0 && ix < 4))return false;
    SecurityRegAddr = SECURITY_REG0 + ix * 0x001000;
    tsc_printf("Read Security registerSecurity  RegNo%d SecurityRegAddr = 0x%08x  \n",ix,SecurityRegAddr);
    /*read*/
    if(!apiok(slld_ReadSecurityCmd(SecurityRegAddr,buf,256)))return false;
    /*print all data*/
    printBuf(buf,256,0);
    return true;
}
#endif
bool cmd_func_NormalPgrmRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    }
    return true;
}
bool cmd_func_PgrmReadOp(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if(!ok(SectorSize != Size4K))return false;
        break;
    case Size8K:
        if(!ok(SectorSize != Size8K))return false;
        break;
    default:
        if( !ok(SectorPgrmReadOp(SectorNo,len,SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    }
    return true;
}
static bool cmd_func_NormalPgrmReadFixed(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint8_t  data;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    data = strtoint(argv[3]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadFixedVal(SectorNo,len,data,PARAM_4K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadFixedVal(SectorNo,len,data,PARAM_8K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadFixedVal(SectorNo,len,data,SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))return false;
        break;
    }
    return true;
}

static bool cmd_func_Normal4BPgrmRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_4B_ERASE,PROG_PAGE_4B,NORMAL_4B_READ)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_4B_ERASE,PROG_PAGE_4B,NORMAL_4B_READ)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_4B_ERASE,PROG_PAGE_4B,NORMAL_4B_READ)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmFastRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,FAST_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,FAST_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,FAST_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmFast4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_4B_ERASE,PROG_PAGE_4B,FAST_4B_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_4B_ERASE,PROG_PAGE_4B,FAST_4B_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_4B_ERASE,PROG_PAGE_4B,FAST_4B_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDualRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DUAL_OUT_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DUAL_OUT_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DUAL_OUT_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDual4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DUAL_4BOUT_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DUAL_4BOUT_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DUAL_4BOUT_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmQuadRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE_QUAD,QUAD_OUT_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE_QUAD,QUAD_OUT_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE_QUAD,QUAD_OUT_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmQuad4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE_QUAD_4B,QUAD_4BOUT_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE_QUAD_4B,QUAD_4BOUT_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE_QUAD_4B,QUAD_4BOUT_RAED)))return false;
        break;
    }
    return true;
}

static bool cmd_func_PgrmDualIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DUAL_IO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DUAL_IO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DUAL_IO_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDualIO4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DUAL_4BIO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DUAL_4BIO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DUAL_4BIO_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmQuadIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE_QUAD,QUAD_IO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE_QUAD,QUAD_IO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE_QUAD,QUAD_IO_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmQuadIO4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE_QUAD_4B,QUAD_4BIO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE_QUAD_4B,QUAD_4BIO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE_QUAD_4B,QUAD_4BIO_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDDRFastRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DDR_FAST_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DDR_FAST_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DDR_FAST_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDDRFast4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DDR_FAST_4B_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DDR_FAST_4B_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DDR_FAST_4B_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDDRDualIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DDR_DURAL_IO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DDR_DURAL_IO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DDR_DURAL_IO_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDDRDualIO4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DDR_DURAL_4B_IO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DDR_DURAL_4B_IO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DDR_DURAL_4B_IO_RAED)))return false;
        break;
    }
    return true;
}

static bool cmd_func_PgrmDDRQuadIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DDR_QUAD_IO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DDR_QUAD_IO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DDR_QUAD_IO_RAED)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmDDRQuadIO4BRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,DDR_QUAD_4B_IO_RAED)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,DDR_QUAD_4B_IO_RAED)))return false;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,DDR_QUAD_4B_IO_RAED)))return false;
        break;
    }
    return true;
}

#ifdef INCL_SPI_BURSTWRAPCMD
static bool cmd_func_PgrmBurst8QuadIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    uint32_t addrOffset = strtoint(argv[3]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize - addrOffset ;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    if(!ok(addrOffset < 8))return false;

    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,8,PARAM_4K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,8,PARAM_8K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    default:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,8,SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmBurst16QuadIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    uint32_t addrOffset = strtoint(argv[3]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize - addrOffset ;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    if(!ok(addrOffset < 16))return false;

    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,16,PARAM_4K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,16,PARAM_8K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    default:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,16,SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    }
    return true;
}

static bool cmd_func_PgrmBurst32QuadIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    uint32_t addrOffset = strtoint(argv[3]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize - addrOffset ;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    if(!ok(addrOffset < 32))return false;

    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,32,PARAM_4K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,32,PARAM_8K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    default:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,32,SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    }
    return true;
}
static bool cmd_func_PgrmBurst64QuadIORead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize;
    uint32_t len;
    uint32_t SectorNo = strtoint(argv[1]);
    uint32_t addrOffset = strtoint(argv[3]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    if(strcmp(argv[2],"all") == 0)
        len = SectorSize - addrOffset ;
    else
        len = strtoint(argv[2]);
    if( !ok(len > 0 && len  <= SectorSize))return false;
    if(!ok(addrOffset < 64))return false;

    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,64,PARAM_4K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    case Size8K:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,64,PARAM_8K_SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    default:
        if( !ok(SectorPgrmBrustWrapRead(SectorNo,addrOffset,len,64,SECTOR_ERASE,PROG_PAGE)))return false;
        break;
    }
    return true;
}
#endif
#ifndef REMOVE_P4EOp
static bool cmd_func_4kerase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    if(!apiok(slld_EraseFlash(SectorAddr,PARAM_4K_SECTOR_ERASE)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_P4E_4BOp
static bool cmd_func_4k4Berase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    if(!apiok(slld_EraseFlash(SectorAddr,PARAM_4K_SECTOR_4B_ERASE)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_P8EOp
static bool cmd_func_8kerase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    if(!apiok(slld_EraseFlash(SectorAddr,PARAM_8K_SECTOR_ERASE)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_P8E_4BOp
static bool cmd_func_8k4Berase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    if(!apiok(slld_EraseFlash(SectorAddr,PARAM_8K_SECTOR_4B_ERASE)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_SEOp
static bool cmd_func_Sectorerase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    if(!apiok(slld_EraseFlash(SectorAddr,SECTOR_ERASE)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_BLOCKERASEOp
static bool cmd_func_Blockerase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t BlockSize,BlockAddr;
    uint32_t BlockNo = strtoint(argv[1]);
    if(!ok(BlockNo < DevInfo->BlockTotalNumber))return false;
    BlockSize = DevInfo->pBlockArray[BlockNo].BlockSize;
    BlockAddr = DevInfo->pBlockArray[BlockNo].BlockAddr;
    TRC(NOTICE,("BlockNo = %d,BlockAddr = 0x%08x,BlockSize = %d\n",BlockNo,BlockAddr,BlockSize));
    if(!apiok(slld_EraseFlash(BlockAddr,BLOCK_ERASE)))return false;
    if(!apiok(slld_IfSecErased(BlockAddr,BlockSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_SE_4BOp
static bool cmd_func_Sector4Berase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    if(!apiok(slld_EraseFlash(SectorAddr,SECTOR_4B_ERASE)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    return true;
}
#endif
#ifndef REMOVE_BEOp
static bool cmd_func_Buckerase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo;
    if(!apiok(slld_EraseFlash(0x00,BULK_ERASE)))return false;
    for(SectorNo = 0; SectorNo < DevInfo->SectorTotalNumber;SectorNo++){
        SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
        SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
        if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) {
            TRC(ERROR,("SectorNo = %u SectorSize = %u \n",SectorNo,SectorSize));
            return false;
        }
    }
    return true;
}
static bool cmd_func_Buck1erase(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo;
    if(!apiok(slld_EraseFlash(0x00,BULK_ERASE1)))return false;
    for(SectorNo = 0; SectorNo < DevInfo->SectorTotalNumber;SectorNo++){
        SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
        SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
        if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) {
            TRC(ERROR,("SectorNo = %u SectorSize = %u \n",SectorNo,SectorSize));
            return false;
        }
    }
    return true;
}
#endif
extern void os_DelayMicroSec(DWORD microsec);

#ifndef REMOVE_ERS_SSPCmd
static bool cmd_func_EraseSuspend(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t PageSize = DevInfo->PageSize;
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorSize0;
    uint32_t SectorNo = strtoint(argv[1]);
    uint8_t rdstatus;
    DEVSTATUS devStatus;
    Testbuf_t *tbuf;
    bool ret = false;
    /*sector 0 for program verify during erase suspend time*/
    if(!ok(SectorNo > 0 && SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize0 = DevInfo->pSectorArray[0].SectorSize;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    switch(SectorSize0){
    case Size4K:
        /*sector 0 for program verify during erase suspend time*/
        if(!apiok(slld_EraseFlash(0,PARAM_4K_SECTOR_ERASE)))return false;
        break;
    case Size8K:
        /*sector 0 for program verify during erase suspend time*/
        if(!apiok(slld_EraseFlash(0,PARAM_8K_SECTOR_ERASE)))return false;
        break;
    default:
        /*sector 0 for program verify during erase suspend time*/
        if(!apiok(slld_EraseFlash(0,SECTOR_ERASE)))return false;
        break;
    }
    if(!apiok(slld_IfSecErased(0,SectorSize0))) return false;

    switch(SectorSize){
    case Size4K:
        if( !apiok(slld_WRENCmd()))return false;
#ifndef REMOVE_P4ECmd
        if( !apiok(slld_P4ECmd(SectorAddr)))return false;
#endif
        break;
#ifndef	REMOVE_P8ECmd
    case Size8K:
        if( !apiok(slld_WRENCmd()))return false;
        if( !apiok(slld_P8ECmd(SectorAddr)))return false;
        break;
#endif
    default:
        if( !apiok(slld_WRENCmd()))return false;
        if( !apiok(slld_SECmd(SectorAddr)))return false;
        break;
    }
    os_DelayMicroSec(100);/*delay 100 us*/
    if(!apiok(slld_ERS_SSPCmd()))return false;
    /*wait until erase stopped*/
    if(!apiok(slld_Poll(&devStatus)))return false;
    if(!apiok(slld_RDSR2Cmd(&rdstatus)))return false;
    /*check if the erase has been stopped*/
#ifdef FLS_CMD
    if(!ok(rdstatus&0x02))return false;
#else
    if(!ok(rdstatus&0x80))return false;
#endif
    tsc_printf("Erase has been suspended , SecNo = %d SectorSize = %d\n",SectorNo,SectorSize);
    /*program/read sector 0*/
    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,SectorSize0)))goto end;
    if(!apiok(slld_ProgramFlash(0,tbuf->wbuf,SectorSize0,PageSize,PROG_PAGE))) goto end;
    /*read and verify*/
    if(!apiok(slld_ReadFlash(0,tbuf->rbuf,SectorSize0,PageSize,NORMAL_READ))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;

    /*resume erase */
    if(!apiok(slld_ERS_RESCmd()))goto end;
    tsc_printf("Erase has been resumed\n");
    if(!apiok(slld_Poll(&devStatus)))goto end;
    if(!apiok(slld_RDSRCmd(&rdstatus)))goto end;
    if(!ok((rdstatus&0x02) == 0))goto end;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#endif
#if ( defined( FLS_CMD) || defined( FL1K_CMD))
static bool cmd_func_ProgramSuspend(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t PageSize = DevInfo->PageSize;
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorSize0;
    uint32_t SectorNo = strtoint(argv[1]);
    uint8_t rdstatus;
    DEVSTATUS devStatus;
    Testbuf_t *tbuf;
    bool ret = false;
    /*sector 0 for read verify during program suspend time*/
    if(!ok(SectorNo > 0 && SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize0 = DevInfo->pSectorArray[0].SectorSize;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    switch(SectorSize0){
    case Size4K:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(0,PARAM_4K_SECTOR_ERASE)))return false;
        break;
    case Size8K:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(0,PARAM_8K_SECTOR_ERASE)))return false;
        break;
    default:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(0,SECTOR_ERASE)))return false;
        break;
    }
    /*program sector prepare for reading during program sectorN period .*/
    if(!apiok(slld_IfSecErased(0,SectorSize0))) return false;

    switch(SectorSize){
    case Size4K:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,PARAM_4K_SECTOR_ERASE)))return false;
        break;
    case Size8K:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,PARAM_8K_SECTOR_ERASE)))return false;
        break;
    default:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,SECTOR_ERASE)))return false;
        break;
    }
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;

    /*program/read sector 0*/
    tbuf = malloc(sizeof(Testbuf_t)*2);
    if(!ok(testBufInit(&tbuf[0],SectorSize0)))goto end;
    if(!ok(testBufInit(&tbuf[1],PageSize)))goto end;
    if(!apiok(slld_ProgramFlash(0,tbuf[0].wbuf,SectorSize0,PageSize,PROG_PAGE))) goto end;
    if(!apiok(slld_ReadFlash(0,tbuf[0].rbuf,SectorSize0,PageSize,NORMAL_READ))) goto end;
    if( !ok( ifEqual(tbuf[0].wbuf,tbuf[0].rbuf,tbuf[0].len)))goto end;

    /*start program to sector <SectorNo> */
    if( !apiok(slld_WRENCmd()))goto end;
    if( !apiok(slld_PPCmd(SectorAddr, tbuf[1].wbuf,tbuf[1].len)))goto end;
    os_DelayMicroSec(50);
    /*start program suspend*/
#ifdef FLS_CMD
#ifdef FS_S
    if(!apiok(slld_EPS_Cmd()))goto end;
#else
    if(!apiok(slld_PGSPCmd()))goto end;
#endif
#else /*FL1K*/
    if(!apiok(slld_ERS_SSPCmd()))return false;
#endif
    /*wait until program stopped*/
    if(!apiok(slld_Poll(&devStatus)))goto end;

    if(!apiok(slld_RDSR2Cmd(&rdstatus)))goto end;

    /*check if the erase has been stopped*/
#ifdef FLS_CMD
    if(!ok(rdstatus&0x01))goto end;
#else /*FL1K*/
    if(!ok(rdstatus&0x80))goto end;
#endif
    tsc_printf("Program has been suspended , SecNo = %d SectorSize = %d\n",SectorNo,SectorSize);

    /*read sector 0 and verify*/
    memset(tbuf[0].rbuf,0,tbuf[0].len);
    if(!apiok(slld_ReadFlash(0,tbuf[0].rbuf,SectorSize0,PageSize,NORMAL_READ))) goto end;
    if( !ok( ifEqual(tbuf[0].wbuf,tbuf[0].rbuf,tbuf[0].len)))goto end;

    /*resume program */
#ifdef FLS_CMD
    if(!apiok(slld_PGRSCmd()))goto end;
#else
    if(!apiok(slld_ERS_RESCmd()))goto end;
#endif
    tsc_printf("Program has been resumed\n");
    if(!apiok(slld_Poll(&devStatus)))goto end;
    if(!apiok(slld_RDSRCmd(&rdstatus)))goto end;
    if(!ok((rdstatus&0x01) == 0))goto end;
    /*verify if it is program success ,read length of a page size of data*/
    if(!apiok(slld_ReadFlash(SectorAddr,tbuf[1].rbuf,tbuf[1].len,PageSize,NORMAL_READ))) goto end;
    if( !ok( ifEqual(tbuf[1].wbuf,tbuf[1].rbuf,tbuf[1].len)))goto end;
    ret = true;
    end:
    testBuffree(&tbuf[0]);
    testBuffree(&tbuf[1]);
    free(tbuf);
    return ret;
}
#endif
static bool cmd_func_readRegs(void *d ,int argc, const char *argv[]){
    uint8_t status_val,cfg_val,rdstatus;
    uint8_t vals[4];
#ifndef REMOVE_RDSRCmd
    if(!apiok(slld_RDSRCmd(&status_val)))return false;
    tsc_printf("Status Register1            0x%02x\n",status_val);
#endif
#ifndef REMOVE_RCRCmd

    if(!apiok(slld_RCRCmd(&cfg_val)))return false;
    tsc_printf("Configure register          0x%02x\n",cfg_val);
#endif

#if ( defined( FLS_CMD) || defined( FL1K_CMD))
    if(!apiok(slld_RDSR2Cmd(&rdstatus)))return false;
    tsc_printf("Status Register2            0x%02x\n",rdstatus);
#endif

#ifdef FL1K_CMD
#ifndef REMOVE_RDSR3Cmd
    if(!apiok(slld_RDSR3Cmd(&rdstatus)))return false;
    tsc_printf("Status Register3            0x%02x\n",rdstatus);
#endif
#endif
#ifndef REMOVE_BRRDCmd
    if(!apiok(slld_BRRDCmd(&rdstatus)))return false;
    tsc_printf("Bank register is            0x%02x\n",rdstatus);
#endif

#ifndef REMOVE_ABRDCmd
    if(!apiok(slld_ABRDCmd((unsigned long *)vals)))return false;
    tsc_printf("AutoBoot register[0]        0x%02x \n",vals[0]);
    tsc_printf("AutoBoot register[1]        0x%02x \n",vals[1]);
    tsc_printf("AutoBoot register[2]        0x%02x \n",vals[2]);
    tsc_printf("AutoBoot register[3]        0x%02x \n",vals[3]);
#endif
#ifdef FLS_CMD
#ifndef REMOVE_PPB_RDCmd
    if(!apiok(slld_PPB_RDCmd(0,&rdstatus)))return false;
    tsc_printf("Sector0 PPB Access register 0x%02x\n",rdstatus);
#endif
#endif
    return true;
}
#ifndef REMOVE_MULTIOMODE_FUNCTIONS
#ifndef REMOVE_WRROp
static bool cmd_func_WrCfgReg(void *d ,int argc, const char *argv[]){
    uint8_t cfg_val,status_val;
    DEVSTATUS devStatus;
    cfg_val = strtoint(argv[1]);
    if(!apiok(slld_RDSRCmd(&status_val)))return false;
    if(!apiok(slld_WRROp(&status_val,&cfg_val,NULL,&devStatus))) return false;
    if(!apiok(slld_RCRCmd(&cfg_val)))return false;
    tsc_printf("Configure register          0x%02x\n",cfg_val);
    return true;
}
#endif
#endif

#ifndef REMOVE_RCRCmd
static bool cmd_func_WrStatus1Reg(void *d ,int argc, const char *argv[]){
    uint8_t cfg_val,status_val;
    DEVSTATUS devStatus;
    status_val = strtoint(argv[1]);
    if(!apiok(slld_RCRCmd(&cfg_val)))return false;

    if(!apiok(slld_WRROp(&status_val,&cfg_val,NULL,&devStatus))) return false;
    if(!apiok(slld_RDSRCmd(&status_val)))return false;
    tsc_printf("Status Register1            0x%02x\n",status_val);
    return true;
}
#endif
#if (defined FLS_CMD)
static bool cmd_func_WrStatus2Reg(void *d ,int argc, const char *argv[]){
    uint8_t cfg_val,status_val,status2_val;
    DEVSTATUS devStatus;
    status2_val = strtoint(argv[1]);
    if(!apiok(slld_RDSRCmd(&status_val)))return false;
    if(!apiok(slld_RCRCmd(&cfg_val)))return false;

    if(!apiok(slld_WRROp(&status_val,&cfg_val,&status2_val,&devStatus))) return false;
    if(!apiok(slld_RDSR2Cmd(&status2_val)))return false;
    tsc_printf("Status Register2            0x%02x\n",status2_val);
    return true;
}
#endif
#ifndef REMOVE_SRSTCmd
static bool cmd_func_ChipReset(void *d, int argc, const char *argv[]){
    if(!apiok(slld_SRSTCmd()))return false;
    return true;
}
#endif
extern void QspiPowerReleaeCmdupdate(unsigned int mode);
static bool cmd_func_PWdownRelease( void *d, int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t len = 2;
    uint32_t SectorNo = 0;
    Testbuf_t *tbuf;
    bool ret = false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr =DevInfo->pSectorArray[SectorNo].SectorAddr;
    /*program test*/
    switch(SectorSize){
    case Size4K:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,PARAM_4K_SECTOR_ERASE)))return false;
        break;
    case Size8K:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,PARAM_8K_SECTOR_ERASE)))return false;
        break;
    default:
        /*sector 0 for read verify during program suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,SECTOR_ERASE)))return false;
        break;
    }
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;
    /*enter deep power down*/
    QspiPowerReleaeCmdupdate(1);
    if(!apiok(slld_DPCmd()))return false;
    tsc_printf("Enter Deep power down mode\n");
    os_DelayMicroSec(100);/*delay 100us*/

    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(&tbuf[0],len)))goto end;
    /*don't poll status register ,maybe don't return*/
    if( !apiok(slld_PPCmd(SectorAddr, tbuf->wbuf,tbuf->len)))goto end;
    os_DelayMicroSec(50000); /*wait 50ms */
    if(!apiok(slld_ReadFlash(SectorAddr,tbuf->rbuf,tbuf->len,256,NORMAL_READ))) goto end;
    if(ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len))goto end;
    /*exit deep power down*/
    if(!apiok(slld_RESCmd())) goto end;
    os_DelayMicroSec(100);/*delay 100us*/
    tsc_printf("Exit Deep power down mode\n");
    /*program should works now */
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_4K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))goto end;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,PARAM_8K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))goto end;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,len,SECTOR_ERASE,PROG_PAGE,NORMAL_READ)))goto end;
        break;
    }
    ret = true;
    end:
    QspiPowerReleaeCmdupdate(0);
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}

#ifdef  INCL_SPI_READMODERESETCMD
static bool cmd_func_ContinueReset( void *d, int argc, const char *argv[]){
    if(!apiok(slld_ReadModeResetCmd())) return false;
    return true;
}
#endif

#if (defined(FL_1K)|| defined(FL_K))
static bool cmd_func_PgrmReadSecuRegs( void *d, int argc, const char *argv[]){
    uint32_t SecurityRegAddr;
    Testbuf_t *tbuf;
    bool ret = false;
    DEVSTATUS devStatus;
    uint32_t ix = strtoint(argv[1]);
    if( !ok(ix > 0 && ix < 4))return false;
    SecurityRegAddr = SECURITY_REG0 + ix * 0x001000;
    tsc_printf("SecurityRegNo%d SecurityRegAddr = 0x%08x Program/Read/Verify with random data\n",ix,SecurityRegAddr);
    if( !apiok(slld_SecurityRegErase(SecurityRegAddr)))return false;
    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,256)))goto end;
    if(!apiok(slld_ProgramSecurityOp(SecurityRegAddr,tbuf->wbuf,tbuf->len,&devStatus))) goto end;
    /*read and verify*/
    if(!apiok(slld_ReadSecurityCmd(SecurityRegAddr,tbuf->rbuf,tbuf->len))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#if 0
static bool cmd_func_PgrmDefaultSecuReg0( void *d, int argc, const char *argv[]){
    uint32_t SecurityRegAddr;
    uint8_t buf[256] ={0};
    Testbuf_t *tbuf;
    bool ret = false;
    DEVSTATUS devStatus;
    uint32_t ix = 0;
    buf[0] = 0x53;
    buf[1] = 0x46;
    buf[2] = 0x44;
    buf[3] = 0x50;
    buf[0x0c] = 0x80;
    buf[0x84] = 0xff;
    buf[0x85] = 0xff;
    buf[0x86] = 0xff;
    buf[0x84] = 0x00;
    if( !ok(ix >= 0 && ix < 4))return false;
    SecurityRegAddr = SECURITY_REG0 + ix * 0x001000;
    tsc_printf("SecurityRegNo%d SecurityRegAddr = 0x%08x Program/Read/Verify with random data\n",ix,SecurityRegAddr);
    if( !apiok(slld_SecurityRegErase(SecurityRegAddr)))return false;
    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,256)))goto end;
    memcpy(tbuf->wbuf,buf,256);
    if(!apiok(slld_ProgramSecurityOp(SecurityRegAddr,tbuf->wbuf,tbuf->len,&devStatus))) goto end;
    /*read and verify*/
    if(!apiok(slld_ReadSecurityCmd(SecurityRegAddr,tbuf->rbuf,tbuf->len))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}
#endif
#endif
#ifdef FLS_CMD
static bool cmd_func_dataLearningReg(void *d ,int argc, const char *argv[]){
    BYTE val_wr = 0xAA;
    BYTE val_rd = 0;
    if(!apiok(slld_WVDLRCmd(&val_wr,1)))return false;
    if(!apiok(slld_DLPRDCmd(&val_rd,1)))return false;
    if(!ok(val_wr == val_rd))return false;
    return true;
}

#endif

#ifdef FS_S
#ifndef REMOVE_RDARCmd
static bool cmd_func_readAnyReg(void *d ,int argc, const char *argv[]){
    uint32_t reg_addr= strtoint(argv[1]);
    uint8_t val;
    if(!apiok(slld_RDARCmd(reg_addr,&val)))return false;
    tsc_printf("Read Register with address 0x%x : 0x%x\n",reg_addr,val);
    return true;
}
static bool cmd_func_readAnyRegs(void *d ,int argc, const char *argv[]){
    uint8_t val;
    if(!apiok(slld_RDARCmd(SR1NV,&val)))return false;
    tsc_printf("Read Register SR1NV  0x%06x : 0x%02x\n",SR1NV,val);

    if(!apiok(slld_RDARCmd(CR1NV,&val)))return false;
    tsc_printf("Read Register CR1NV  0x%06x : 0x%02x\n",CR1NV,val);

    if(!apiok(slld_RDARCmd(CR2NV,&val)))return false;
    tsc_printf("Read Register CR2NV  0x%06x : 0x%02x\n",CR2NV,val);

    if(!apiok(slld_RDARCmd(CR3NV,&val)))return false;
    tsc_printf("Read Register CR3NV  0x%06x : 0x%02x\n",CR3NV,val);

    if(!apiok(slld_RDARCmd(CR4NV,&val)))return false;
    tsc_printf("Read Register CR4NV  0x%06x : 0x%02x\n",CR4NV,val);

    if(!apiok(slld_RDARCmd(SR1V,&val)))return false;
    tsc_printf("Read Register SR1V  0x%06x : 0x%02x\n",SR1V,val);

    if(!apiok(slld_RDARCmd(SR2V,&val)))return false;
    tsc_printf("Read Register SR2V  0x%06x : 0x%02x\n",SR2V,val);

    if(!apiok(slld_RDARCmd(CR1V,&val)))return false;
    tsc_printf("Read Register CR1V  0x%06x : 0x%02x\n",CR1V,val);

    if(!apiok(slld_RDARCmd(CR2V,&val)))return false;
    tsc_printf("Read Register CR2V  0x%06x : 0x%02x\n",CR2V,val);

    if(!apiok(slld_RDARCmd(CR3V,&val)))return false;
    tsc_printf("Read Register CR3V  0x%06x : 0x%02x\n",CR3V,val);

    if(!apiok(slld_RDARCmd(CR4V,&val)))return false;
    tsc_printf("Read Register CR4V  0x%06x : 0x%02x\n",CR4V,val);

    return true;
}
static bool cmd_func_writeAnyReg(void *d ,int argc, const char *argv[]){
    uint32_t reg_addr= strtoint(argv[1]);
    uint8_t val = strtoint(argv[2]);
    DEVSTATUS devStatus;
    if(!apiok(slld_WRAR_Op(reg_addr,&val,&devStatus)))return false;
    tsc_printf("Write Register with address 0x%x : 0x%x\n",reg_addr,val);
    val= 0;
    if(!apiok(slld_RDARCmd(reg_addr,&val)))return false;
    tsc_printf("Read Register with address 0x%x : 0x%x\n",reg_addr,val);
    return true;
}

#endif

static bool cmd_func_QuadAllModeCtrl(void *d ,int argc, const char *argv[])
{
    uint32_t mode= strtoint(argv[1]);
    if(QUAD_ALL_ENABLE == mode){
        if(!apiok(slld_QuadAllModeCtrl(QUAD_ALL_ENABLE)))return false;
        QspiEnableQuadAll();
    }
    else{
        if(!apiok(slld_QuadAllModeCtrl(QUAD_ALL_DISABLE)))return false;
        QspiDisableQuadAll();
    }
    return true;
}
#ifndef REMOVE_RSTC
static bool cmd_func_softwareRst(void *d ,int argc, const char *argv[])
{
    uint8_t val_old,val_new;
    DEVSTATUS devStatus;
    if(!apiok(slld_RDARCmd(CR4V,&val_old)))return false;
    val_new = val_old | 0xe0; //Output Impedance ,default is b000
    if(!apiok(slld_WRAR_Op(CR4V,&val_new,&devStatus)))return false;

    if(!apiok(slld_RSTENCmd()))return false;
    if(!apiok(slld_RSTCmd()))return false;
    os_DelayMicroSec(100000);/*delay 100 ms*/
    if(!apiok(slld_RDARCmd(CR4V,&val_new)))return false;
    if(!ok(val_new == val_old))return false;
    return true;
}
#endif

static bool cmd_func_EvaluateEraseStatus(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t PageSize = DevInfo->PageSize;
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    Testbuf_t *tbuf;
    bool ret = false;
    /*sector 0 for program verify during erase suspend time*/
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    TRC(NOTICE,("SecNo = %d,SectorAddr = 0x%08x,SectorSize = %d\n",SectorNo,SectorAddr,SectorSize));
    switch(SectorSize){
    case Size4K:
        /*sector 0 for program verify during erase suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,PARAM_4K_SECTOR_ERASE)))return false;
        break;
    case Size8K:
        /*sector 0 for program verify during erase suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,PARAM_8K_SECTOR_ERASE)))return false;
        break;
    default:
        /*sector 0 for program verify during erase suspend time*/
        if(!apiok(slld_EraseFlash(SectorAddr,SECTOR_ERASE)))return false;
        break;
    }
    if(!apiok(slld_EvaluateEraseStatus(SectorAddr)))return false;
    if(!apiok(slld_IfSecErased(SectorAddr,SectorSize))) return false;

    /*program/read sector */
    tbuf = malloc(sizeof(Testbuf_t));
    if(!ok(testBufInit(tbuf,SectorSize)))goto end;
    if(!apiok(slld_ProgramFlash(SectorAddr,tbuf->wbuf,SectorSize,PageSize,PROG_PAGE))) goto end;
    /*read and verify*/
    if(!apiok(slld_ReadFlash(SectorAddr,tbuf->rbuf,SectorSize,PageSize,NORMAL_READ))) goto end;
    if( !ok( ifEqual(tbuf->wbuf,tbuf->rbuf,tbuf->len)))goto end;

    /*erase sector ,delay a short while and reset chip.*/
    switch(SectorSize){
    case Size4K:
        if( !apiok(slld_WRENCmd()))goto end;
        if( !apiok(slld_P4ECmd(SectorAddr)))goto end;
        break;
#ifndef REMOVE_P8ECmd
    case Size8K:
        if( !apiok(slld_WRENCmd()))goto end;
        if( !apiok(slld_P8ECmd(SectorAddr)))goto end;
        break;
#endif
    default:
        if( !apiok(slld_WRENCmd()))goto end;
        if( !apiok(slld_SECmd(SectorAddr)))goto end;
        break;
    }
    os_DelayMicroSec(100);/*delay 100 us*/

    if(!apiok(slld_RSTENCmd()))goto end;
    if(!apiok(slld_RSTCmd()))goto end;
    os_DelayMicroSec(100000);/*delay 100 ms*/
    if(!apiok(slld_ESSCmd(SectorAddr)))goto end;
    /*evaluate erase status result should be not ok*/
    if(!ok(slld_EvaluateEraseStatus(SectorAddr) == SLLD_ERROR))goto end;
    ret = true;
    end:
    testBuffree(tbuf);
    free(tbuf);
    return ret;
}

#ifndef REMOVE_4BAM_Cmd

static bool cmd_func_Enter4BAM(void *d ,int argc, const char *argv[]){
    DEVSTATUS devStatus;
    uint8_t val;
    if(!apiok( slld_4BAM_Cmd())) return false;
    QspiEnable4ByteAddress();
    if(!apiok(slld_RDARCmd(CR2V,&val))) return false;
    if(!ok(val & 0x80)) return false;
    /*revert to 3 byte*/
    val &= ~0x80;
    /*write register using 4byte address mode*/
    if(!apiok(slld_WRAR_Op(CR2V,&val,&devStatus)))return false;
    QspiDisable4ByteAddress();
    if(!apiok(slld_RDARCmd(CR2V,&val))) return false;
    if(!ok((val & 0x80) == 0)) return false;
    return true;
}
#endif

static bool cmd_func_DYBTest(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorSize,SectorAddr;
    uint32_t SectorNo = strtoint(argv[1]);
    uint8_t val;
    DEVSTATUS devStatus;
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorSize = DevInfo->pSectorArray[SectorNo].SectorSize;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    /*DYB protected*/
    val = 0;
    if(!apiok( slld_DYBWR_Op(SectorAddr,&val,&devStatus))) return false;

    if(!apiok( slld_DYBRD_Cmd(SectorAddr,&val))) return false;
    if(!ok(val == 0))return false;

    /*program should not works now */
    switch(SectorSize){
    case Size4K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,SectorSize,PARAM_4K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ ) == false)) goto end;
        break;
    case Size8K:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,SectorSize,PARAM_8K_SECTOR_ERASE,PROG_PAGE,NORMAL_READ) == false)) goto end;
        break;
    default:
        if( !ok(SectorPgrmReadRandomVal(SectorNo,SectorSize,SECTOR_ERASE,PROG_PAGE,NORMAL_READ) == false)) goto end;
        break;
    }
    end:
    /*clear the SRV1 caused by above operation.*/
    if(!apiok( slld_ClearStatusRegisterCmd())) return false;
    val = 0xff;
    if(!apiok( slld_DYBWR_Op(SectorAddr,&val,&devStatus))) return false;
    if(!apiok( slld_DYBRD_Cmd(SectorAddr,&val))) return false;
    if(!ok(val == 0xff))return false;
    return true;
}

static bool cmd_func_PPBRead(void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t SectorAddr;
    uint8_t val;
    uint32_t SectorNo = strtoint(argv[1]);
    val = 0x0;
    if(!ok(SectorNo < DevInfo->SectorTotalNumber))return false;
    SectorAddr = DevInfo->pSectorArray[SectorNo].SectorAddr;
    if(!apiok( slld_PPBRD_Cmd(SectorAddr,&val))) return false;
    tsc_printf("Sector %d PPB val = 0x%x\n",SectorNo,val);
    if(!ok(val == 0xff))return false;
    return true;
}

#endif // FS_S


static bool cmd_func_showstat( void *d, int argc, const char *argv[]){
    TestCaseTable_t *t =&BasicTestCaseTable;
    int ix =0;
    tsc_printf("basic function statistics :\n");
    for( ix= 0;ix < t->len;ix++){
        if(t->elems[ix].onOff== true){
            tsc_printf("[%-30s]:",t->elems[ix].name);
            tsc_printf("pass : %-5u ",t->elems[ix].pass);
            tsc_printf("error: %-5u \n",t->elems[ix].error);
            //		timerStruct start;            /*test start time ticks*/ TODO
        }

    }
    return true ;
}
static bool cmd_func_cleanstat( void *d, int argc, const char *argv[]){
    TestCaseTable_t *t =&BasicTestCaseTable;
    int ix =0;

    for( ix= 0;ix < t->len;ix++){
        t->elems[ix].pass = 0;
        t->elems[ix].error = 0;
        //t->elems[ix].start = 0; /*test start time ticks*/ TODO
    }
    tsc_printf("basic function statistics cleaned:\n");
    return true ;
}


/*****************************\
 ****   Command support   ******
 \*****************************/
static const Command_t funcTest_commands[] ={
#ifndef REMOVE_Read_IDCmd
        {"func_readRES",1,cmd_func_Read_ID,NULL,NULL,"Read Electronic Signature use CMD AB"},
#endif

#ifndef REMOVE_RDIDCmd
        {"func_JEDECId",1,cmd_func_JEDECId,NULL,NULL,"Read JEDEC ID using CMD 9F"},
#endif

#ifndef REMOVE_READ_IDENTIFICATIONCmd
        {"func_readREMS",1,cmd_func_Read_Identification,NULL,NULL,"Read ID(Electronic Manufacturer and device Signature) use CMD 90"},
#endif
#ifdef INCL_SPI_READSFDPCMD
        {"func_Read_SFDP",1,cmd_func_Read_SFDP,NULL,NULL,"Read Serial Flash Discoverable Parameter"},
#endif
#ifndef REMOVE_P4EOp
        {"func_4kerase",2,cmd_func_4kerase,NULL,"SectorNo","4KB erase sector"},
#endif
#ifndef REMOVE_BLOCKERASEOp
        {"func_Blockerase",2,cmd_func_Blockerase,NULL,"BlockNo"," Block erase sector"},
#endif
#ifndef REMOVE_P4E_4BOp
        {"func_4k4Berase",2,cmd_func_4k4Berase,NULL,"SectorNo","4KB erase sector using 4Byte address"},
#endif
#ifndef REMOVE_P8EOp
        {"func_8kerase",2,cmd_func_8kerase,NULL,"SectorNo","8KB erase sector "},
#endif
#ifndef REMOVE_P8E_4BOp
        {"func_8k4Berase",2,cmd_func_8k4Berase,NULL,"SectorNo","8KB erase sector using 4Byte address"},
#endif
#ifndef REMOVE_SEOp
        {"func_Sectorerase",2,cmd_func_Sectorerase,NULL,"SectorNo","sector erase "},
#endif
#ifndef REMOVE_SE_4BOp
        {"func_Sector4Berase",2,cmd_func_Sector4Berase,NULL,"SectorNo","sector erase using 4Byte address"},
#endif
#ifndef REMOVE_BEOp
        {"func_Buckerase",1,cmd_func_Buckerase,NULL,NULL,"whole chip erase "},
        {"func_Buck1erase",1,cmd_func_Buck1erase,NULL,NULL,"whole chip erase using alternate command"},
#endif
        {"func_NormalPgrmRead",3,cmd_func_NormalPgrmRead,NULL,    "SectorNo length|all","    program/read/verify length of random data in sector,all=\"whole sector\""},
        {"func_PgrmReadOp",3,cmd_func_PgrmReadOp,NULL,    "SectorNo length|all","    program/read/verify using slld_ReadOp slld_WriteOp and slld_SEOp,all=\"whole sector\""},
#ifndef REMOVE_SE_4BOp
        {"func_Normal4BPgrmRead",3,cmd_func_Normal4BPgrmRead,NULL,"SectorNo length|all","    program/read/verify length of random value data in sector using 4B address,all=\"whole sector\""},
#endif
#ifndef REMOVE_Fast_ReadCmd
        {"func_PgrmFastRead",3,cmd_func_PgrmFastRead,NULL,"SectorNo length|all","    program/fast read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_Fast_Read_4BCmd
        {"func_PgrmFast4BRead",3,cmd_func_PgrmFast4BRead,NULL,"SectorNo length|all","    program/Fast 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DUALOUTPUT_READCmd
        {"func_PgrmDualRead",3,cmd_func_PgrmDualRead,NULL,"SectorNo length|all","    program/Dual read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DUALOUTPUT_READ_4BCmd
        {"func_PgrmDual4BRead",3,cmd_func_PgrmDual4BRead,NULL,"SectorNo length|all","    program/Dual 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_QUADOUTPUT_READCmd
        {"func_PgrmQuadRead",3,cmd_func_PgrmQuadRead,NULL,"SectorNo length|all","    program/Quad read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_QUADOUTPUT_READ_4BCmd
        {"func_PgrmQuad4BRead",3,cmd_func_PgrmQuad4BRead,NULL,"SectorNo length|all","    program/Quad 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DUALOUTPUT_HP_READCmd
        {"func_PgrmDualIORead",3,cmd_func_PgrmDualIORead,NULL,"SectorNo length|all","    program/Dual IO read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DUALOUTPUT_HP_READ_4BCmd
        {"func_PgrmDualIO4BRead",3,cmd_func_PgrmDualIO4BRead,NULL,"SectorNo length|all","    program/Dual IO 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_QUADOUTPUT_HP_READCmd
        {"func_PgrmQuadIORead",3,cmd_func_PgrmQuadIORead,NULL,"SectorNo length|all","    program/Quad IO read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_QUADOUTPUT_HP_READ_4BCmd
        {"func_PgrmQuadIO4BRead",3,cmd_func_PgrmQuadIO4BRead,NULL,"SectorNo length|all","    program/Quad IO 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DDR_Fast_ReadCmd
        {"func_PgrmDDRFastRead",3,cmd_func_PgrmDDRFastRead,NULL,"SectorNo length|all","    program/DDR fast read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DDR_Fast_4BReadCmd
        {"func_PgrmDDRFast4BRead",3,cmd_func_PgrmDDRFast4BRead,NULL,"SectorNo length|all","    program/DDR fast 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif

#ifndef REMOVE_DDR_DUALOUTPUT_HP_READCmd
        {"func_PgrmDDRDualIORead",3,cmd_func_PgrmDDRDualIORead,NULL,"SectorNo length|all","    program/DDR Dual IO read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DDR_DUALOUTPUT_HP_READ_4BCmd
        {"func_PgrmDDRDualIO4BRead",3,cmd_func_PgrmDDRDualIO4BRead,NULL,"SectorNo length|all","    program/DDR Dual IO 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DDR_QUADOUTPUT_HP_READCmd
        {"func_PgrmDDRQuadIORead",3,cmd_func_PgrmDDRQuadIORead,NULL,"SectorNo length|all","    program/DDR Quad IO read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifndef REMOVE_DDR_QUADOUTPUT_HP_READ_4BCmd
        {"func_PgrmDDRQuadIO4BRead",3,cmd_func_PgrmDDRQuadIO4BRead,NULL,"SectorNo length|all","    program/DDR Quad IO 4B address read/verify length of random value data in sector,all=\"whole sector\""},
#endif
#ifdef INCL_SPI_BURSTWRAPCMD
        {"func_PgrmBurst8QuadIORead",4,cmd_func_PgrmBurst8QuadIORead,NULL,"SectorNo length|all addrOffset","program/burst wrap Quad IO read/verify ,addrOffset<8  ,all=\"whole sector\""},
        {"func_PgrmBurst16QuadIORead",4,cmd_func_PgrmBurst16QuadIORead,NULL,"SectorNo length|all addrOffset","program/burst wrap Quad IO read/verify ,addrOffset<16  ,all=\"whole sector\""},
        {"func_PgrmBurst32QuadIORead",4,cmd_func_PgrmBurst32QuadIORead,NULL,"SectorNo length|all addrOffset","program/burst wrap Quad IO read/verify ,addrOffset<32  ,all=\"whole sector\""},
        {"func_PgrmBurst64QuadIORead",4,cmd_func_PgrmBurst64QuadIORead,NULL,"SectorNo length|all addrOffset","program/burst wrap Quad IO read/verify ,addrOffset<64  ,all=\"whole sector\""},
#endif
        {"func_NormalPgrmReadFixed",4,cmd_func_NormalPgrmReadFixed,NULL,"SectorNo length|all data","program/read/verify length of fixed value data in sector,all=\"whole sector\""},

#ifndef REMOVE_ERS_SSPCmd
        {"func_EraseSuspend",2,cmd_func_EraseSuspend,NULL,"SectorNo","suspend/resume erase specified sector , SectorNo > 0 ,sector 0 used for program/read during suspend period"},
#endif
#if ( defined( FLS_CMD) || defined( FL1K_CMD))
        {"func_ProgramSuspend",2,cmd_func_ProgramSuspend,NULL,"SectorNo","suspend/resume program specified sector , SectorNo > 0 ,sector 0 used for read during suspend period"},
#endif
#if (defined(FL_1K)|| defined(FL_K))
        {"func_PgrmReadSecuRegs",2,cmd_func_PgrmReadSecuRegs,NULL,"SecurityRegNo 1-3","Security registers1~3 program/read/verify with 256 byte random value data "},
        {"func_ReadSecurityRegs",2,cmd_func_ReadSecuRegs,NULL,"SecurityRegNo 0-3","read specified Security registers and display"},
        //{"func_PgrmDefaultSecuReg0",1,cmd_func_PgrmDefaultSecuReg0,NULL,NULL,"restore the security register 0 because of incautious erase "},
#endif
        {"func_readRegs",1,cmd_func_readRegs,NULL,NULL,"read registers value"},
#ifdef INCL_SPI_READMODERESETCMD
        {"func_ContinueReset",1,cmd_func_ContinueReset,NULL,NULL,"Continue read mode reset"},
#endif
#ifndef REMOVE_RESCmd
        {"func_PWdownRelease",1,cmd_func_PWdownRelease,NULL,NULL,"deep power down /release test"},
#endif
#ifndef REMOVE_RCRCmd
        {"func_WrCfgReg",2,cmd_func_WrCfgReg,NULL,"value","write configure register"},
        {"func_WrStatus1Reg",2,cmd_func_WrStatus1Reg,NULL,"value","write Status1 register"},
#endif
#if (defined FLS_CMD)
        {"func_WrStatus2Reg",2,cmd_func_WrStatus2Reg,NULL,"value","write Status2 register"},
#endif
#ifdef FS_S

#ifndef REMOVE_RDQIDCmd
        {"func_ReadQId",1,cmd_func_ReadQId,NULL,NULL,"Read JEDEC ID using CMD AF in Quad All Mode."},
#endif

#ifndef REMOVE_RDARCmd
        {"func_readAnyReg",2,cmd_func_readAnyReg,NULL,"reg_addr","Read any device register-non-volatile and volatile"},
        {"func_writeAnyReg",3,cmd_func_writeAnyReg,NULL,"reg_addr value","Write any device register-non-volatile and volatile"},
        {"func_readAnyRegs",1,cmd_func_readAnyRegs,NULL,NULL,"Read any device registers"},

#endif
        {"func_QuadAllModeCtrl",2,cmd_func_QuadAllModeCtrl,NULL,"mode","Enable or Disable Quad All mode.0:QUAD_ALL_ENABLE 1:QUAD_ALL_DISABLE"},
#ifndef REMOVE_ESSCmd
        {"func_EvaluateEraseStatus",2,cmd_func_EvaluateEraseStatus,NULL,"SectorNo","Evaluate Erase Status on given sector"},
#endif
#ifndef REMOVE_4BAM_Cmd
        {"func_Enter4BAM",1,cmd_func_Enter4BAM,NULL,NULL,"test Enter 4 Byte address mode"},
#endif
        {"func_DYBTest",2,cmd_func_DYBTest,NULL,"SectorNo","DYB test sector protection"},
        {"func_PPBRead",2,cmd_func_PPBRead,NULL,NULL,"PPB sector protection"},
#ifndef REMOVE_RSTC
        {"func_softwareRst",1,cmd_func_softwareRst,NULL,NULL,"software reset chip,RSTEN and RST"},
#endif
#endif //#ifdef FS_S

        {"func_readSector",3,cmd_func_readSector,NULL,"SectorNo length|all ","read sector content from flash and display"},
        {"func_readMemory",3,cmd_func_readMemory,NULL,"address length ","read length of flash memory content from given address and display"},
#ifdef FLS_CMD
        {"func_dataLearningReg",1,cmd_func_dataLearningReg,NULL,NULL,"Data Learning Pattern Register Read and Write"},
#endif
#ifndef REMOVE_SRSTCmd
        {"func_ChipReset",1,cmd_func_ChipReset,NULL,NULL,"software reset chip"},
#endif
        {"func_showstat",1,cmd_func_showstat,NULL,NULL,"show function test statistics"},
        {"func_cleanstat",1,cmd_func_cleanstat,NULL,NULL,"clean function test statistics"},
};
CommandTable_t funcTest_commandsTable = {
        funcTest_commands,
        ElmNum(funcTest_commands)
};
/*****************************\
 ****   test case support   ******
\*****************************/

static TestCase_t basic_test_cases[] ={
#ifndef REMOVE_Read_IDCmd
        {true,"func_readRES",cmd_func_Read_ID,"Read Electronic Signature use CMD AB",0,NULL,0,0},
#endif

#ifndef REMOVE_RDIDCmd
        {true,"func_JEDECId",cmd_func_JEDECId,"Read JEDEC ID using CMD 9F",0,NULL,0,0},
#endif

#ifndef REMOVE_READ_IDENTIFICATIONCmd
        {true,"func_readREMS",cmd_func_Read_Identification,"Read ID(Electronic Manufacturer and device Signature) use CMD 90",0,NULL,0,0},
#endif
#ifdef INCL_SPI_READSFDPCMD
        {true,"func_Read_SFDP",cmd_func_Read_SFDP,"Read Serial Flash Discoverable Parameter",0,NULL,0,0},
#endif
#ifndef REMOVE_P4EOp
        {true,"func_4kerase",cmd_func_4kerase,"4KB erase sector",2,"default 0",0,0},
#endif
#ifndef REMOVE_BLOCKERASEOp
        {true,"func_Blockerase",cmd_func_Blockerase," Block erase sector",2,"default 0",0,0},
#endif
#ifndef REMOVE_P4E_4BOp
        {true,"func_4k4Berase",cmd_func_4k4Berase,"4KB erase sector using 4Byte address",2,"default 0",0,0},
#endif
#ifndef REMOVE_P8EOp
        {true,"func_8kerase",cmd_func_8kerase,"8KB erase sector ",2,"default 1"},
#endif
#ifndef REMOVE_P8E_4BOp
        {true,"func_8k4Berase",cmd_func_8k4Berase,"8KB erase sector using 4Byte address",2,"default 1",0,0},
#endif
#ifndef REMOVE_SEOp
        {true,"func_Sectorerase",cmd_func_Sectorerase,"sector erase ",2,"default 33",0,0},
#endif
#ifndef REMOVE_SE_4BOp
        {true,"func_Sector4Berase",cmd_func_Sector4Berase,"sector erase using 4Byte address",2,"default 33",0,0},
#endif

        {true,"func_NormalPgrmReadRdm",cmd_func_NormalPgrmRead,"    program/read/verify length of random data in sector",3,"default 0 all",0,0},
        {true,"func_PgrmReadOp",cmd_func_PgrmReadOp,"    program/read/verify using slld_ReadOp slld_WriteOp and slld_SEOp",3,"default 32 all",0,0},
#ifndef REMOVE_SE_4BOp
        {true,"func_Normal4BPgrmReadRdm",cmd_func_Normal4BPgrmRead,"    program/read/verify length of random value data in sector using 4B address",3,"default 1 all",0,0},
#endif
#ifndef REMOVE_Fast_ReadCmd
        {true,"func_PgrmFastReadRdm",cmd_func_PgrmFastRead,"    program/fast read/verify length of random value data in sector",3,"default 2 all",0,0},
#endif
#ifndef REMOVE_Fast_Read_4BCmd
        {true,"func_PgrmFast4BRead",cmd_func_PgrmFast4BRead,"    program/Fast 4B address read/verify length of random value data in sector",3,"default 3 all",0,0},
#endif
#ifndef REMOVE_DUALOUTPUT_READCmd
        {true,"func_PgrmDualRead",cmd_func_PgrmDualRead,"    program/Dual read/verify length of random value data in sector",3,"default 4 all",0,0},
#endif
#ifndef REMOVE_DUALOUTPUT_READ_4BCmd
        {true,"func_PgrmDual4BRead",cmd_func_PgrmDual4BRead,"    program/Dual 4B address read/verify length of random value data in sector",3,"default 5 all",0,0},
#endif
#ifndef REMOVE_QUADOUTPUT_READCmd
        {true,"func_PgrmQuadRead",cmd_func_PgrmQuadRead,"    program/Quad read/verify length of random value data in sector",3,"default 6 all",0,0},
#endif
#ifndef REMOVE_QUADOUTPUT_READ_4BCmd
        {true,"func_PgrmQuad4BRead",cmd_func_PgrmQuad4BRead,"    program/Quad 4B address read/verify length of random value data in sector",3,"default 7 all",0,0},
#endif
#ifndef REMOVE_DUALOUTPUT_HP_READCmd
        {true,"func_PgrmDualIORead",cmd_func_PgrmDualIORead,"    program/Dual IO read/verify length of random value data in sector",3,"default 8 all",0,0},
#endif
#ifndef REMOVE_DUALOUTPUT_HP_READ_4BCmd
        {true,"func_PgrmDualIO4BRead",cmd_func_PgrmDualIO4BRead,"    program/Dual IO 4B address read/verify length of random value data in sector",3,"default 9 all",0,0},
#endif
#ifndef REMOVE_QUADOUTPUT_HP_READCmd
        {true,"func_PgrmQuadIORead",cmd_func_PgrmQuadIORead,"    program/Quad IO read/verify length of random value data in sector",3,"default 10 all",0,0},
#endif
#ifndef REMOVE_QUADOUTPUT_HP_READ_4BCmd
        {true,"func_PgrmQuadIO4BRead",cmd_func_PgrmQuadIO4BRead,"    program/Quad IO 4B address read/verify length of random value data in sector",3,"default 11 all",0,0},
#endif
#ifndef REMOVE_DDR_Fast_ReadCmd
        {true,"func_PgrmDDRFastRead",cmd_func_PgrmDDRFastRead,"    program/DDR fast read/verify length of random value data in sector",3,"default 12 all",0,0},
#endif
#ifndef REMOVE_DDR_Fast_4BReadCmd
        {true,"func_PgrmDDRFast4BRead",cmd_func_PgrmDDRFast4BRead,"    program/DDR fast 4B address read/verify length of random value data in sector",3,"default 13 all",0,0},
#endif

#ifndef REMOVE_DDR_DUALOUTPUT_HP_READCmd
        {true,"func_PgrmDDRDualIORead",cmd_func_PgrmDDRDualIORead,"    program/DDR Dual IO read/verify length of random value data in sector",3,"default 14 all",0,0},
#endif
#ifndef REMOVE_DDR_DUALOUTPUT_HP_READ_4BCmd
        {true,"func_PgrmDDRDualIO4BRead",cmd_func_PgrmDDRDualIO4BRead,"    program/DDR Dual IO 4B address read/verify length of random value data in sector",3,"default 15 all",0,0},
#endif
#ifndef REMOVE_DDR_QUADOUTPUT_HP_READCmd
        {true,"func_PgrmDDRQuadIORead",cmd_func_PgrmDDRQuadIORead,"    program/DDR Quad IO read/verify length of random value data in sector",3,"default 32 all",0,0},
#endif
#ifndef REMOVE_DDR_QUADOUTPUT_HP_READ_4BCmd
        {true,"func_PgrmDDRQuadIO4BRead",cmd_func_PgrmDDRQuadIO4BRead,"    program/DDR Quad IO 4B address read/verify length of random value data in sector,all=\"whole sector\"",3,"default 33 all",0,0},
#endif

#ifdef INCL_SPI_BURSTWRAPCMD
        {true,"func_PgrmBurst8QuadIORead",cmd_func_PgrmBurst8QuadIORead,"program/burst wrap8 Quad IO read/verify",4,"default 34 all 3",0,0},
        {true,"func_PgrmBurst16QuadIORead",cmd_func_PgrmBurst16QuadIORead,"program/burst wrap16 Quad IO read/verify ",4,"default 35 all 4",0,0},
        {true,"func_PgrmBurst32QuadIORead",cmd_func_PgrmBurst32QuadIORead,"program/burst wrap32 Quad IO read/verify",4,"default 36 all 16",0,0},
        {true,"func_PgrmBurst64QuadIORead",cmd_func_PgrmBurst64QuadIORead,"program/burst wrap64 Quad IO read/verify",4,"default 37 all 32",0,0},
#endif

        {true,"func_NormalPgrmReadFixed",cmd_func_NormalPgrmReadFixed,"program/read/verify length of fixed value data in sector",4,"default 38 all 0xa5",0,0},

#ifndef REMOVE_ERS_SSPCmd
        {true,"func_EraseSuspend",cmd_func_EraseSuspend,"suspend/resume erase specified sector ",2,"default 2 ",0,0},
#endif
#if ( defined( FLS_CMD) || defined( FL1K_CMD))
        {true,"func_ProgramSuspend",cmd_func_ProgramSuspend,"suspend/resume program specified sector",2,"default 32 ",0,0},
#endif

#if (defined(FL_1K)|| defined(FL_K))
        {true,"func_PgrmReadSecuRegs",cmd_func_PgrmReadSecuRegs,"Security registers program/read/verify with 256 byte random value data ",2,"default 1 ",0,0},
        {true,"func_ReadSecurityRegs",cmd_func_ReadSecuRegs,"read specified Security registers and display",2,"default 1 ",0,0},
#endif
        {true,"func_readRegs",cmd_func_readRegs,"read registers value",0,NULL,0,0},
#ifndef REMOVE_RESCmd
        {true,"func_PWdownRelease",cmd_func_PWdownRelease,"deep power down /release test",0,NULL,0,0},
#endif

        {true,"func_readSector",cmd_func_readSector,"read sector content from flash and display",3,"default 0 32 ",0,0},
        {true,"func_readMemory",cmd_func_readMemory,"read length of flash memory content from given address and display",3,"default 0x100 48 ",0,0},


#ifdef FS_S

#ifndef REMOVE_RDQIDCmd
        {true,"func_ReadQId",cmd_func_ReadQId,"Read JEDEC ID using CMD AF in Quad All Mode.",0,NULL,0,0},
#endif

#ifndef REMOVE_ESSCmd
        {true,"func_EvaluateEraseStatus",cmd_func_EvaluateEraseStatus,"Evaluate Erase Status on given sector",2,"default 1",0,0},
#endif

#ifndef REMOVE_RSTC
        {true,"func_softwareRst",cmd_func_softwareRst,"software reset chip,RSTEN and RST",0,NULL,0,0},
#endif

#ifndef REMOVE_4BAM_Cmd
        {true,"func_Enter4BAM",cmd_func_Enter4BAM,"test Enter 4 Byte address mode",0,NULL,0,0},
#endif

        {true,"func_DYBTest",cmd_func_DYBTest,"DYB test sector protection",2,"default 1",0,0},
#endif //ifdef FS_S

#ifdef FLS_CMD
        {true,"func_dataLearningReg",cmd_func_dataLearningReg,"Data Learning Pattern Register Read and Write",0,NULL,0,0},
#endif
#ifndef REMOVE_SRSTCmd
        {true,"func_ChipReset",cmd_func_ChipReset,"software reset chip",0,NULL,0,0},
#endif
#ifndef REMOVE_BEOp
        {true,"func_Buckerase",cmd_func_Buckerase,"whole chip erase ",0,NULL,0,0},
        //{true,"func_Buck1erase",cmd_func_Buck1erase,"whole chip erase using alternate command",0,NULL,0,0},
#endif

};
static  TestCaseTable_t BasicTestCaseTable ={
        basic_test_cases,
        ElmNum(basic_test_cases)
};

bool cmd_proc_func_test(void * d ,int argc, const char *argv[]){
    Procedure_t *procedure = d;
    TestCaseTable_t *t = &BasicTestCaseTable;
    int ix, argcs = 0;
    const char *argvs[MAX_INPUG_ARGS];
    char *buf = NULL;
    bool ret=true;

    for(ix = 0;ix < t->len; ix++) {

        if(true == t->elems[ix].onOff){
            buf = NULL;
            lld_printf("  [%u]. %s %s \n",ix+1,"description:",t->elems[ix].des);
            if(t->elems[ix].num_args && t->elems[ix].args){
                const int len= strlen(t->elems[ix].args);
                buf  = strcpy(malloc( len + 1),t->elems[ix].args);
                argcs = fmt_args(buf,argvs);
                if(argcs !=t->elems[ix].num_args){
                    lld_printf("  Fail ! \n");
                    ret =false;
                    lld_printf("default args not equal to the args number required\n");
                    free(buf);
                    continue;
                }
                PARAM_PRINT(argcs,argvs);
            }
            procedure->total++;
            if(t->elems[ix].call(&t->elems[ix],argcs,argvs)){
                procedure->pass++;
                t->elems[ix].pass++;
                lld_printf("  PASS ! \n");
            }
            else{
                procedure->error++;
                t->elems[ix].error++;
                lld_printf("  Fail ! \n");
                ret =false;
            }
            lld_printf("\n");
            if(buf)free(buf);
        }
    }
    /*printf the result*/
    cmd_func_showstat(NULL,0,NULL);
    return ret;
}

