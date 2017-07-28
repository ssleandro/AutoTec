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
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <lld_test_util.h>
#include <slld_hal.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "sppc.h"
#endif

void * g_scratchpad_bufAddr = NULL;
uint32_t g_scratchpad_buf_enalbe = false;

int fmt_args(char *str,const char *argv[]);
void lld_printf( const char* format, ...);

extern const CommandTable_t *allCommands[];
extern bool cmd_help( void *d, int argc, const char *argv[]);
int execute_command(CmdInput_t *cmd,const CommandTable_t *t[]){
    const int len= strlen(cmd->input);
    char *buf  = strcpy(malloc( len + 1),cmd->input);
    unsigned int argc = 0;
    bool res = false;
    bool missMatch_args = false;
    const char *argv[MAX_INPUG_ARGS];
    unsigned int ix=0,iy=0,ik;
    argc = fmt_args(buf,argv);
    if( argc < 1) {
        res = true;
        goto end;
    }
    else{
        while(t[ix]){
            for(iy=0;iy<t[ix]->len;iy++)
                if( strcmp( argv[0], t[ix]->elems[iy].name) == 0){
                    if(t[ix]->elems[iy].num_args == argc) {
                        PARAM_PRINT(argc,argv);
                        t[ix]->elems[iy].call(t[ix]->elems[iy].data, argc, argv);
                        res = true;
                        goto end;
                    }
                    else{

                        missMatch_args= true;
                        goto InvalidCmd;
                    }
                }
            ix++;
        }
    }
    InvalidCmd:
    lld_printf( "Invalid command=%s  ", argv[0]);
    for( ik = 1; ik < argc; ik++)
        lld_printf(  " argv[%u]=%s ", ik, argv[ik]);
    lld_printf("\n");
    if(missMatch_args == true){
        lld_printf("   %-26s",t[ix]->elems[iy].name);
        if(t[ix]->elems[iy].arg_txt)lld_printf(" <%s> ",t[ix]->elems[iy].arg_txt);
        if(t[ix]->elems[iy].des)lld_printf(" %-s \n",t[ix]->elems[iy].des);

    }
    else{
        cmd_help((void *)allCommands,0,NULL);
    }


    end:
    free(buf);
    return res;

}

unsigned int strtoint(const char* str){
    char ch;
    unsigned int val = 0,r=10;
    while ('0' == *str)str++;
    if('x' == *str || 'X' == *str){
        str++;
        r =16;
    }
    while((ch = *str++)){
        if ('0' <= ch && ch <= '9')
            val = val * r + ch - '0';
        else if ('a' <= ch && ch <= 'f')
            val = val * 16 + ch - 'a' + 10;
        else if ('A' <= ch && ch <= 'F')
            val = val * 16 + ch - 'A' + 10;
    }
    return val;

}
const char *filename( const char *name) {
    const char *res = strrchr( name,'\\');
    return res ? res+1 : name;
}
bool ok_impl( bool c, const char *fmt, ...) {
    if( !c) {
        va_list va;

        char buff[300];
        va_start( va, fmt);
        vsnprintf( buff, sizeof( buff), fmt, va);
        printf(buff);
        printf("\n");
        va_end( va);
    }
    return c;
}
int getinputs(CmdInput_t * cmd){
    lld_printf("LLD>");
    gets((char *)&cmd->input);
    if (!*cmd->input) return(false);
    return true ;
}

int fmt_args(char *str,const char *argv[]){
    const int len= strlen(str);
    char *ptr = str, *const end = str + len;
    int argc = 0;
    for(ptr = str;ptr < end; ptr++){
        while( isspace((int)*ptr) &&  *ptr && ptr < end ) ptr++;
        if( !( *ptr && ptr < end)) break;
        argv[argc++] = ptr;

        while( !isspace((int)*ptr) && *ptr && ptr < end ) ptr++;
        if( !( ptr <= end)) break;
        if( ! (argc < MAX_INPUG_ARGS)){
            lld_printf("commands args can't exceed than %u\n",MAX_INPUG_ARGS);
            return 0;
        }
        *ptr = '\0';
    }
    return argc;
}
void lld_printf( const char* format, ...)
{
    va_list vl;
    va_start(vl,format);
    vprintf(format, vl);
    fflush(stdout);
    va_end(vl);
}
void printBuf(uint8_t *buf ,uint32_t len,uint32_t offset){
    uint32_t ix = 0;
    for(ix =0 ;ix <len ;ix++){
        if(ix%16)
            lld_printf("%02x ",buf[ix]);
        else{
            lld_printf("\n");
            lld_printf("0x%08x %02x ",ix + offset,buf[ix]);
        }
    }
    lld_printf("\n");
    return;
}
bool ifEqual( uint8_t * buf1,  uint8_t* buf2, uint32_t size){
    uint32_t ix;
    for(ix=0; ix< size; ix++, buf1++, buf2++)
        if ( *buf1 !=  *buf2){
            TRC(ERROR,("ix = %u *buf1 = 0x%02x  *buf2= 0x%02x size=%u \n",ix,*buf1,*buf2,size));
            return false;
        }
    return true;
}

bool testBufInit(Testbuf_t *t ,uint32_t len){
    uint32_t ix;
    uint8_t *p;
    if(g_scratchpad_buf_enalbe == true){
        if(!ok((t->wbuf = (uint8_t *)g_scratchpad_bufAddr) != NULL))return false;
        if(!ok((t->rbuf = malloc(len)) != NULL)){
            return false;
        }
        t->len = len;
        memset(t->rbuf,0,len);
    }
    else{
        if(!ok((t->wbuf = malloc(len)) != NULL))return false;
        if(!ok((t->rbuf = malloc(len)) != NULL)){
            free(t->wbuf);
            return false;
        }
        t->len = len;
        p = t->wbuf;
        for(ix=0;ix<len;ix++)
            *p++ = (uint8_t)rand();
        memset(t->rbuf,0,len);
    }

    return true;
}
void testBuffree(Testbuf_t *t){

    if(t->wbuf != (uint8_t *)g_scratchpad_bufAddr){
        if(t->wbuf)free(t->wbuf);
    }
    if(t->rbuf)free(t->rbuf);
    t->rbuf = NULL;
    t->wbuf = NULL;
    t->len =0;
    return ;
}

bool scratchpad_buffer_init(void){
    if(!ok(g_scratchpad_bufAddr == NULL)) return false;
    if(!ok((g_scratchpad_bufAddr = malloc(MAX_SCRATCH_BUF_SIZE)) != NULL))return false;
    memset(g_scratchpad_bufAddr,0,MAX_SCRATCH_BUF_SIZE);
    g_scratchpad_buf_enalbe = false;
    return true;
}
bool scratchpad_buffer_fill(uint32_t size,uint8_t data, char * str){
    uint32_t ix;
    uint8_t *buf = (uint8_t *) g_scratchpad_bufAddr;
    if(!ok(buf != NULL))return false;
    for(ix=0; ix< size; ix++, buf++)
        if ( *str == '+')
            *buf = data++;
        else if( *str == '-')
            *buf = data--;
        else if(*str == '=')
            *buf = data;
        else return false;
    return true;
}
bool scratchpad_buffer_replace(uint8_t *sbuf,uint32_t size){
    uint8_t *buf = (uint8_t *) g_scratchpad_bufAddr;
    if(!ok(buf != NULL && buf != NULL))return false;
    if(!ok(size < MAX_SCRATCH_BUF_SIZE))return false;
    memcpy(buf,(void *)sbuf,size);
    return true;
}

bool scratchpad_buffer_show(uint32_t size){
    uint8_t *buf = (uint8_t *) g_scratchpad_bufAddr;
    if(!ok(buf != NULL))return false;
    if(!ok(size < MAX_SCRATCH_BUF_SIZE))return false;
    printBuf(buf,size,0);
    return true;
}
bool testBufInitFixedVal(Testbuf_t *t ,uint32_t len,uint8_t Value){
    uint32_t ix;
    uint8_t *p;
    if(!ok((t->wbuf = malloc(len)) != NULL))return false;
    if(!ok((t->rbuf = malloc(len)) != NULL)){
        free(t->wbuf);
        return false;
    }
    t->len = len;
    p = t->wbuf;
    for(ix=0;ix<len;ix++)
        *p++ = Value;
    memset(t->rbuf,0,len);
    return true;
}
unsigned long base_addr_g;
unsigned long window_size_g;

#ifndef WIN32
int pci_mmap(unsigned long *addr){
    unsigned short *pciMem;
    int fd, len,val;
    pca9554_config_t pcacfg={0};
    if ( !pxok (fd = open( "/dev/sppc0", O_RDWR)))return false;
    if ( !pxok (ioctl( fd, SPPC_IOCTL_GETSIZE, &len)))return false;
    if (len == 0){
        printf( "Error: disabled memory bank!\n"); exit( -1);
    }
    printf("memroy size = 0x%x\n",len);
    pciMem = mmap( NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (pciMem ==  MAP_FAILED) { perror( "mmap() failed"); exit( -1); }
    printf( "PCI mapped to %p\n", pciMem);
    *addr = (unsigned long)pciMem;

#ifdef PCI4_CONTROLLER
    val = 1;
    /* enable CPLD */
    if ( !pxok (ioctl( fd, SPPC_IOCTL_ENABLECPLD, &val)))return false;
    if ( !pxok (ioctl( fd, SPPC_IOCTL_READPCA9554, &pcacfg)))return false;
    //printf("pcacfg[0]= 0x%x pcacfg[1]= 0x%x pcacfg[2]= 0x%x pcacfg[3]= 0x%x \n",pcacfg[0],pcacfg[1],pcacfg[2],pcacfg[3]);
    pcacfg[1] = 0x37;pcacfg[2] = 0xff;pcacfg[3] = 0xc0;
    if ( !pxok (ioctl( fd, SPPC_IOCTL_WRITEPCA9554, &pcacfg)))return false;
    REG_INTERFACE_CONTROL = 0x05;  //  enable FS&SM interface
    CS_INTERFACE = 0x00;          //   set CS0 to SM0
    //printf("interface_ctrl= 0x%x CS_INTERFACE=0x%x CS_MAP=0x%x\n",REG_INTERFACE_CONTROL,CS_INTERFACE,CS_MAP);
    FS_CONTROL = 0x06;		// reset SPI controller
    FS_CONTROL = 0x3f;		// set x2 clk, CSn0
    val = 0;
    /* disable CPLD */
    if ( !pxok (ioctl( fd, SPPC_IOCTL_ENABLECPLD, &val)))return false;
    usleep(100000);		// wait 1ms
#endif
    return true;
}
#else
extern int HalAdaptationInit(unsigned long *base_addr, unsigned long *window_size, int BankCS);
extern void AmdPCIHalExit (void);
#endif

bool slld_test_init(){

    DeviceInfo *DevInfo;
    uint32_t i;
    lld_printf("Initialize Spansion PCI\n");
#ifdef WIN32
    if ( !apiok( HalAdaptationInit(&base_addr_g,&window_size_g,0) ))return false;
#else /*linux*/
    if ( !ok(pci_mmap(&base_addr_g)))return false;
#endif
    lld_printf("SLLD version = %s\n", SLLD_VERSION);
#ifdef BOARD226
    /*default table ,will be replaced later*/
#ifdef FL_1K
    if ( !apiok(slld_QspiRAMWr(FLS_1K,0,256))) return false;
    slld_QspiRegWr(SP00226_REG_ADDR_SPI_DDR_SMPL, 0x02);
#elif defined(FS_S)
    if ( !apiok(slld_QspiRAMWr(FSS,0,256))) return false;
    slld_QspiRegWr(SP00226_REG_ADDR_SPI_DDR_SMPL, 0xC3);/*due to FPGA modification*/
#elif defined(FL_S)
    if ( !apiok(slld_QspiRAMWr(FLS_EHP,0,256))) return false;
    slld_QspiRegWr(SP00226_REG_ADDR_SPI_DDR_SMPL, 0x02);
#endif

#endif /*end #ifdef BOARD226*/

    if( !apiok(slld_ProbeDevice()))return false;
    DevInfo = slld_GetGeoInfo();
    lld_printf("Memory size = %u MB\n",DevInfo->Memsize/(1024*1024));
    lld_printf("Boot Type = %u \n",DevInfo->BootType);
    lld_printf("Total Bank number = %u\n",DevInfo->BankTotalNumber);
    lld_printf("Total Block number = %u\n",DevInfo->BlockTotalNumber);
    lld_printf("Total Sector number = %u\n",DevInfo->SectorTotalNumber);
    lld_printf("Total Region number = %u \n",DevInfo->RegionTotalNumber);
    for(i =0; i < DevInfo->RegionTotalNumber;i++){
        lld_printf("Region[%d].SecotrNumber = %u \n",i,DevInfo->pRegionArray[i].SecotrNumber);
        lld_printf("Region[%d].SectorSize =   %u KB \n",i,DevInfo->pRegionArray[i].SectorSize/1024);
    }
    lld_printf("Max SectorSize = %u KB \n",DevInfo->MaxSectorSize/1024);
    lld_printf("Page size = %u \n",DevInfo->PageSize);
    if(DevInfo->RegionTotalNumber > 1)
        lld_printf("Small Sector locate in %s\n",DevInfo->IsBottom ? "Bottom":"Top");
#ifdef FL_S
    lld_printf("%s EHP(Enhanced high performance)\n",DevInfo->SupportEHP ? "Support":"Don't Support");
#endif

#ifdef BOARD226
#ifdef FL_S
    if(DevInfo->SupportEHP){
        if ( !apiok(slld_QspiRAMWr(FLS_EHP,0,256))) return false;
    }
    else
        if ( !apiok(slld_QspiRAMWr(FLS_HP,0,256))) return false;
#endif /*end FL_S*/
#endif /*end #ifdef BOARD226*/

    /*enable quad mode,fast dual also can works under this mode*/
    if ( !apiok(slld_QuadModeCtrl(QUAD_ENABLE)))return false;
#if (defined (FL_1K) || defined (FL_S))
    /*Test case only support LC=00*/
    if ( !apiok(slld_SetLatencyCode(0x00)))return false;
#endif
#ifdef INCL_SPI_BURSTWRAPCMD
    if ( !apiok(slld_SetBurstWrapMode(BURST_DISABLE)))return false;
#endif
#ifdef  INCL_SPI_READMODERESETCMD
    //if ( !apiok(slld_ReadModeResetCmd()))return false;  //FIXME FLS seems can't pass
#endif
    lld_printf("\n");
    return true ;
}
bool slld_test_exit(){
    DeviceInfo *DevInfo;
    DevInfo = slld_GetGeoInfo();
    if(!DevInfo->pBlockArray) free(DevInfo->pBlockArray);
    if(!DevInfo->pSectorArray) free(DevInfo->pSectorArray);
    if(!DevInfo->pBankArray) free(DevInfo->pBankArray);
    if(!DevInfo->pRegionArray) free(DevInfo->pRegionArray);
#ifdef WIN32
    AmdPCIHalExit();
#endif
    return true ;
}
