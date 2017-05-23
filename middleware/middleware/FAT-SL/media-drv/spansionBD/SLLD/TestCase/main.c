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

int pxres;
int pxerr;
unsigned char debug_level = TRC_WARNING;
static ProcedureTable_t ProcedureTable;
const CommandTable_t *allCommands[MAX_COMMAND_NUM];

bool cmd_help( void *d, int argc, const char *argv[]);
static bool cmd_exit( void *d, int argc, const char *argv[]);
static bool cmd_set_debug_level(void * d ,int argc, const char *argv[]);
static bool cmd_proc_showstat( void *d, int argc, const char *argv[]);
static bool cmd_proc_cleanstat( void *d, int argc, const char *argv[]);
static bool cmd_runall(void * d ,int argc, const char *argv[]);

/*****************************\
 ****   GDB Command support   ******
\*****************************/
void GdbBreak(void)
{
    return;
}
void GdbCmd(int argc, char *arg0){
    CmdInput_t cmdline={{0}};
    sprintf(cmdline.input,"%s",arg0);
    execute_command(&cmdline,allCommands);
    return;
}

bool cmd_help( void *d, int argc, const char *argv[]) {
    int ix=0,iy=0;
    const CommandTable_t **cmd_t =d;
    lld_printf(" LLD Test Command list:  \n");
    while((cmd_t[ix])){
        lld_printf("---\n");
        for(iy= 0; iy < cmd_t[ix]->len;iy++){
            lld_printf("   %-26s",cmd_t[ix]->elems[iy].name);
            if(cmd_t[ix]->elems[iy].arg_txt)lld_printf(" <%s> ",cmd_t[ix]->elems[iy].arg_txt);
            if(cmd_t[ix]->elems[iy].des)lld_printf(" %-s \n",cmd_t[ix]->elems[iy].des);
        }
        ix++;
    }
    return true;
}

static bool cmd_exit( void *d, int argc, const char *argv[]) {
    ok(slld_test_exit());
    lld_printf("bye\n");
    exit(0);
    return true;
}
static bool cmd_set_debug_level(void * d ,int argc, const char *argv[]){
    debug_level = strtoint(argv[1]);
    return true;
}
extern uint32_t g_scratchpad_buf_enalbe ;

static bool cmd_fill_scratchpad_buffer(void * d ,int argc, const char *argv[]){
    unsigned int  size = strtoint(argv[1]);
    unsigned char data = strtoint(argv[2]);
    char *str = (char*)argv[3];
    unsigned char  *buf =  (unsigned char *)strtoint(argv[4]);
    if(!ok(size < MAX_SCRATCH_BUF_SIZE)) return false;
    if(buf == NULL){
        if(!ok(*str=='+' || *str=='-' || *str=='='))return false;
        if(!ok(scratchpad_buffer_fill(size,data,str)))return false;
    }else
    {
        if(!ok(scratchpad_buffer_replace(buf,size)))return false;
    }
    lld_printf("success fill_scratchpad_buffer\n");
    return true;
}

static bool cmd_enable_scratchpad_buffer(void * d ,int argc, const char *argv[]){
    if(!ok(strtoint(argv[1]) == 0 || strtoint(argv[1]) == 1))return false;
    if(strtoint(argv[1]) == 0)
        g_scratchpad_buf_enalbe = false;
    else
        g_scratchpad_buf_enalbe = true;
    lld_printf("scratchpad buffer is %s ,buffer size = %dKB\n",(g_scratchpad_buf_enalbe == true)?"enabled":"disabled",MAX_SCRATCH_BUF_SIZE/1024);
    if(!ok(scratchpad_buffer_show(32))) return false;
    return true;
}

static bool cmd_show_scratchpad_buffer(void * d ,int argc, const char *argv[]){
    unsigned int  size = strtoint(argv[1]);
    lld_printf("\n");
    lld_printf("scratchpad buffer is %s ,buffer size = %dKB\n",(g_scratchpad_buf_enalbe == true)?"enabled":"disabled",MAX_SCRATCH_BUF_SIZE/1024);
    if(!ok(scratchpad_buffer_show(size))) return false;
    return true;
}
/*procedure statistic print*/
bool cmd_proc_showstat( void *d, int argc, const char *argv[]){
    ProcedureTable_t *t =d;
    int ix =0;
    for( ix= 0;ix < t->len;ix++){
        if(t->elems[ix].onOff== true){
            lld_printf("%-10s statistics show: \n",t->elems[ix].des);
            lld_printf("total : %-25u",t->elems[ix].total);
            lld_printf("pass :  %-25u",t->elems[ix].pass);
            lld_printf("error : %-25u \n",t->elems[ix].error);
        }
    }
    return true ;
}
bool cmd_proc_cleanstat( void *d, int argc, const char *argv[]){
    ProcedureTable_t *t =d;
    int ix =0;
    for( ix= 0;ix < t->len;ix++){
        t->elems[ix].total = 0;
        t->elems[ix].pass = 0;
        t->elems[ix].error = 0;
    }
    return true ;
}

static bool cmd_runall(void * d ,int argc, const char *argv[]){

    ProcedureTable_t *t = d;
    int ix;
    bool ret=false;
    for(ix = 0; ix < t->len;ix++){
        if(true == t->elems[ix].onOff){
            lld_printf("%u.%s start! \n\n",ix+1,t->elems[ix].des);
            if( t->elems[ix].call(&t->elems[ix],argc,argv)){
                lld_printf("%s end! \n",t->elems[ix].des);
                ret =true;
            }
            else{
                lld_printf("%s end! \n",t->elems[ix].des);
            }
        }
        cmd_proc_showstat(t,0,NULL);
        lld_printf("\n");
    }
    return ret;
}
/*****************************\
 ****  test procedure support**
\*****************************/
static  Procedure_t Procedure[] ={
        {true,cmd_proc_func_test,NULL,"[BASE FUNCTION TEST] ",0,0,0},
        {true,cmd_proc_stress_test,NULL,"[STRESS TEST] ",0,0,0}
};
static ProcedureTable_t ProcedureTable ={
        Procedure,
        ElmNum(Procedure)
};

/*****************************\
 ****   Command support   ******
\*****************************/
static const Command_t basic_commands[] ={
        {"help",1,cmd_help,(void *)allCommands,NULL,"get help information"},
        {"exit",1,cmd_exit,(void *)allCommands,NULL,"end program"},
        {"set_debug_level",2,cmd_set_debug_level,(void *)allCommands,"level","0: ERROR 1:WARNING 2:NOTICE 3:INFO 4:DEBUG"},
        {"enable_scratchpad_buffer",2,cmd_enable_scratchpad_buffer,NULL,"switch","0 :disable 1: enable "},
        {"fill_scratchpad_buffer",5,cmd_fill_scratchpad_buffer,NULL,"size data str buf","str:\"+ - = \" ,if buf is 0 then copy size of data from buf to scratchped"},
        {"show_scratchpad_buffer",2,cmd_show_scratchpad_buffer,NULL,NULL,"size: the size of the scratchpad buffer to show"},
        {"proc_showstat",1,cmd_proc_showstat,&ProcedureTable,NULL,"show procedure statistics"},
        {"proc_cleanstat",1,cmd_proc_cleanstat,&ProcedureTable,NULL,"clean procedure statistics"},
        {"proc_func_test",1,cmd_proc_func_test,&Procedure[0],NULL,"Test basic functions"},
        {"proc_stress_test",1,cmd_proc_stress_test,&Procedure[1],NULL,"Stress test"},
        {"runall",1,cmd_runall,&ProcedureTable,NULL,"Test all procedure"},
};
static  CommandTable_t basic_commandsTable = {
        basic_commands,

        ElmNum(basic_commands)
};
extern CommandTable_t funcTest_commandsTable;
extern CommandTable_t StressTest_commandsTable;
const CommandTable_t *allCommands[] ={
        &basic_commandsTable,
        &funcTest_commandsTable,
        &StressTest_commandsTable,
        NULL
};


int main(int argc, char *argv[]) {
    CmdInput_t cmdline;
    setbuf(stdout,NULL);
    if(!ok(slld_test_init()))return 1;
    if(!ok(scratchpad_buffer_init()))return 1;
    cmd_help((void *)allCommands,0,NULL);
    GdbBreak();
    while(true){
        if(getinputs(&cmdline)) {
            execute_command(&cmdline,allCommands);
        }else{
        }
    }
    return EXIT_SUCCESS;
}

