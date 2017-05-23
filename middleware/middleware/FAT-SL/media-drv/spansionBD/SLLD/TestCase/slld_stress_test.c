/**************************************************************************
 * Copyright (C)2011 Spansion LLC. All Rights Reserved .
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
static  TestCaseTable_t StressTestCaseTable;
/*****************************\
 ****test case implementation******
\*****************************/
static bool cmd_stress_ChipNormalPgrmRead( void *d ,int argc, const char *argv[]){
    DeviceInfo* DevInfo = slld_GetGeoInfo();
    uint32_t loopno = strtoint(argv[1]);
    uint32_t ix,Sectorno;
    char arg1[30];
    const char *argvs[]={
            "cmd_func_NormalPgrmRead",
            arg1,/*SectorNo*/
            "all"
    };
    for(ix = 0; ix < loopno; ix++)
        for(Sectorno =0; Sectorno < DevInfo->SectorTotalNumber;Sectorno++){
            sprintf(arg1,"%d",Sectorno);
            if( !ok(cmd_func_NormalPgrmRead(NULL,3,argvs)))return false;
        }
    return true;
}

static bool cmd_stress_showstat( void *d, int argc, const char *argv[]){
    TestCaseTable_t *t =&StressTestCaseTable;
    int ix =0;
    tsc_printf("stress test case statistics :\n");
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
static bool cmd_stress_cleanstat( void *d, int argc, const char *argv[]){
    TestCaseTable_t *t =&StressTestCaseTable;
    int ix =0;

    for( ix= 0;ix < t->len;ix++){
        t->elems[ix].pass = 0;
        t->elems[ix].error = 0;
        //t->elems[ix].start = 0; /*test start time ticks*/ TODO
    }
    tsc_printf("stress test case statistics cleaned:\n");
    return true ;
}

/*****************************\
 ****   Command support   ******
\*****************************/
static const Command_t StressTest_commands[] ={
        {"stress_ChipNormalPgrmRead",2,cmd_stress_ChipNormalPgrmRead,NULL,"loopNo","whole chip NormalPgrmRead"},
        {"stress_showstat",1,cmd_stress_showstat,NULL,NULL,"show stress test statistics"},
        {"stress_cleanstat",1,cmd_stress_cleanstat,NULL,NULL,"clean stress test statistics"},
};
CommandTable_t StressTest_commandsTable = {
        StressTest_commands,
        ElmNum(StressTest_commands)
};
/*****************************\
 ****   test case support   ******
\*****************************/

static TestCase_t stress_test_cases[] ={
        {true,"stress_ChipNormalPgrmRead",cmd_stress_ChipNormalPgrmRead,"whole chip NormalPgrmRead",2,"default 1 ",0,0},/*default parameters argc=1 < loopNo > */

};
static  TestCaseTable_t StressTestCaseTable ={
        stress_test_cases,
        ElmNum(stress_test_cases)
};

bool cmd_proc_stress_test(void * d ,int argc, const char *argv[]){
    Procedure_t *procedure = d;
    TestCaseTable_t *t = &StressTestCaseTable;
    int ix, argcs = 0;
    const char *argvs[MAX_INPUG_ARGS];
    char *buf = NULL;
    bool ret=true;
    //void * pDev = nand_get_mydev();
    //geoInfo->total_blocks = 8; // just for debug #define NUMB_OF_BLOCKS_IN_A_PLANE       4//1024
    for(ix = 0;ix < t->len; ix++) {

        if(true == t->elems[ix].onOff){
            buf = NULL;
            if(t->elems[ix].num_args && t->elems[ix].args){
                const int len= strlen(t->elems[ix].args);
                buf  = strcpy(malloc( len + 1),t->elems[ix].args);
                lld_printf("  [%u]. %s %s \n",ix+1,"description:",t->elems[ix].des);
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
    cmd_stress_showstat(NULL,0,NULL);
    return ret;
}

