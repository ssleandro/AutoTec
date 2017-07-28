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
#ifndef LLD_TEST_UTIL_H_
#define LLD_TEST_UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "slld_util.h"
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int int16_t;
typedef unsigned short int uint16_t;

typedef signed  int int32_t;
typedef unsigned  int uint32_t;

typedef unsigned char bool;
enum bool_s {
    false,
    true
};
#define ElmNum(a)   (sizeof(a)/sizeof(*a))
#define MAX_COMMAND_LEN 100
#define MAX_COMMAND_NUM 200
#define MAX_INPUG_ARGS    15
#define MAX_SCRATCH_BUF_SIZE 64*1024
/*****************************\
 ****   Command support   ******
\*****************************/

typedef bool (*CmdCallback)( void*, int, const char**);
typedef bool (*ProcedureCallback)( void*, int, const char**);
typedef bool (*TestCaseCallback)(  void*, int, const char**);

typedef struct {
    const char *name;
    unsigned int num_args; /*number of the args*/
    CmdCallback call;
    void *data;
    const char *arg_txt;
    const char *des;
}Command_t;

typedef struct  {
    const Command_t *elems;
    unsigned int len;
}CommandTable_t;

typedef struct  {
    const CommandTable_t *elems;
}Commands_t;


typedef struct {
    bool        onOff;            /* TEST ON/OFF false or true*/
    ProcedureCallback call;       /*call back*/
    void *data;                   /*data may be used */
    const char  *des;             /* procedure descriptor */
    uint32_t    total;            /* total test count number of each procedure */
    uint32_t    pass;             /* test pass count number */
    uint32_t    error;            /* test error count number */
}Procedure_t;

typedef struct{
    Procedure_t *elems;
    unsigned int len;
}ProcedureTable_t;
typedef struct {
    bool        onOff;            /* TEST ON/OFF false or true*/
    const char *name;
    TestCaseCallback call;        /*call back*/
    const char  *des;             /* Testcase descriptor */
    uint32_t    num_args;         /*default args number*/
    char *      args;             /*default args*/
    uint32_t    pass;             /* test pass count number */
    uint32_t    error;            /* test error count number */
    //    timerStruct start;            /*test start time ticks*/ TODO
    //    timerStruct end;              /*test end time ticks*/
}TestCase_t;

typedef struct {
    TestCase_t *elems;
    unsigned int len;
}TestCaseTable_t;

typedef struct  {
    char input[MAX_COMMAND_LEN+1];
    char *ptr;
    char history[20];
}CmdInput_t;

/*buffer for test*/
typedef struct  {
    uint8_t *wbuf; /*buffer for writing*/
    uint8_t *rbuf; /* buffer for reading*/
    uint32_t len;     /* buffer size*/
}Testbuf_t;

/*log level*/
enum {
    TRC_ERROR,        /* Error message */
    TRC_WARNING,      /* Warning message */
    TRC_NOTICE,       /* Important Normal message*/
    TRC_INFO,         /* Information */
    TRC_DEBUG,        /* Debug messages*/
};


int fmt_args(char *str,const char *argv[]);
bool ok_impl( bool, const char*, ...);
const char *filename( const char*);
void lld_printf( const char* format, ...);
int getinputs(CmdInput_t * cmd);
unsigned int strtoint(const char* strPtr);
int execute_command(CmdInput_t *cmd,const CommandTable_t *t[]);

bool slld_test_init();
bool slld_test_exit();
bool testBufInit(Testbuf_t *t ,uint32_t len);
bool testBufInitFixedVal(Testbuf_t *t ,uint32_t len,uint8_t Value);
void testBuffree(Testbuf_t *t);
bool ifEqual( uint8_t * buf1,  uint8_t* buf2, uint32_t size);
void printBuf(uint8_t *buf ,uint32_t len,uint32_t offset);
bool scratchpad_buffer_init(void);
bool scratchpad_buffer_fill(uint32_t size,uint8_t data, char * str);
bool scratchpad_buffer_replace(uint8_t *sbuf,uint32_t size);
bool scratchpad_buffer_show(uint32_t size);
bool cmd_proc_func_test(void * d ,int argc, const char *argv[]);
bool cmd_proc_stress_test(void * d ,int argc, const char *argv[]);

/*some basic function may be used by other files*/
bool cmd_func_NormalPgrmRead(void *d ,int argc, const char *argv[]);

#define TRC(level,args)         GEN_TRC(debug_level,level,args,"")
extern unsigned char debug_level;
#define GEN_TRC(debug_level,level,args,ex_fmt,...)   do { \
        if( (debug_level) < TRC_##level) break; \
        printf( "%s:", #level); \
        printf( "%s/%u:", filename( __FILE__), __LINE__); \
        printf( ex_fmt " ", ##__VA_ARGS__); \
        printf args; \
} while(0)

#define SOURCE_PTR      "at %s:%u"
#define SOURCE_ARG      filename( __FILE__), __LINE__// // c99 can't support "__func__"

extern int pxres;
extern int pxerr;

#define ok(c)                   ok_impl( (c) != 0, \
        SOURCE_PTR " NOT FULFILLED: %s", SOURCE_ARG, #c)

#define apiok(r)         (pxres = (r), ok_impl( 0 == pxres, \
        SOURCE_PTR " %s: %d", SOURCE_ARG, #r, pxres))

#define pxok(r)         (pxres = (r), pxerr = errno, ok_impl( 0 <= pxres, \
        SOURCE_PTR " %s: %s", SOURCE_ARG, #r, strerror( pxerr)))



#define tsc_printf(fmt,...)  do{\
        if( (debug_level) < TRC_ERROR)break;\
        lld_printf("%s"fmt,"       ", ##__VA_ARGS__);\
}while(0)

#define PARAM_PRINT(argc,argv) do{\
        int ix;\
        if( (debug_level) < TRC_WARNING)break;\
        tsc_printf("param : argc %u [",argc);\
        for(ix=0;ix< argc;ix++){\
            lld_printf(" %s ",argv[ix]);\
        }\
        lld_printf(" ]\n");\
}while(0)

#endif /* LLD_TEST_UTIL_H_ */
