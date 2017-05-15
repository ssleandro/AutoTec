/****************************************************************************
* Title                 :   diagnostic_logger Include File
* Filename              :   diagnostic_logger.c
* Author                :   thiago.palmieri
* Origin Date           :   26 de abr de 2016
* Version               :   1.0.0
* Compiler              :   GCC 5.2 2015q4 / ICCARM 7.50.2.10312
* Target                :   LPC43XX M4
* Notes                 :   None
*
* THIS SOFTWARE IS PROVIDED BY AUTEQ TELEMATICA "AS IS" AND ANY EXPRESSED
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL AUTEQ TELEMATICA OR ITS CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
/*************** INTERFACE CHANGE LIST **************************************
*
*    Date    Version   Author         Description
*  26 de abr de 2016   1.0.0   thiago.palmieri diagnostic_logger include file Created.
*
*****************************************************************************/
/** @file diagnostic_logger.c
 *  @brief This file provides Buffer logging functionalities, see diagnostic_logger.h
 *  for further information.
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "M2G_app.h"
#include "diagnostic_logger.h"
#include "../../diagnostic/config/diagnostic_config.h"
#if defined(UNITY_TEST)
#include "unity_fixture.h"
#include <stdint.h>
#endif
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/
#if defined(UNITY_TEST)
uint32_t osSignalSet(uint32_t * p, uint32_t d)
{
  (void)d;
  (void)p;
  return 0;
}

extern void osEnterCritical(void);
extern void osExitCritical(void);

#endif

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
CREATE_RINGBUFFER(uint8_t, logger, DIG_BUFFER_SIZE);
static bool bBufferInitialized = false;
static osThreadId callerThread = NULL;
/******************************************************************************
* Function Prototypes
*******************************************************************************/
uint32_t DIG_wLoggerWritter(uint32_t wTimestamp, uint8_t bLevelAndType, uint8_t* pbData, uint8_t bDataSize);
/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : vApplicationMallocFailedHook( void )
*//**
* \b Description:
*
* This function is executed by the OS in case of Malloc Failure.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //This function must be started by the OS
~~~~~~~~~~~~~~~
*
* @see vApplicationMallocFailedHook, vApplicationStackOverflowHook, vApplicationTickHook
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 30/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void vApplicationMallocFailedHook( void )
{
  char bDebug[DIG_BUFFER_MAX_LINE_SIZE];
  memset(&bDebug[0], 0x00, DIG_BUFFER_MAX_LINE_SIZE);
  snprintf(bDebug, sizeof(bDebug), "MALLOC FAILED !");

  DIG_wLoggerWritter(GET_TIMESTAMP(), (LEVEL_ERROR | DEBUG_QUEUE), (uint8_t*)bDebug , strlen(bDebug));

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Error(bDebug);
#endif

  while(1);
}

/******************************************************************************
* Function : vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
*//**
* \b Description:
*
* This function is executed by the OS in case of Stack overflow.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //This function must be started by the OS
~~~~~~~~~~~~~~~
*
* @see vApplicationMallocFailedHook, vApplicationStackOverflowHook, vApplicationTickHook
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 30/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
  (void)xTask;
  char bDebug[DIG_BUFFER_MAX_LINE_SIZE];
  memset(&bDebug[0], 0x00, DIG_BUFFER_MAX_LINE_SIZE);
  snprintf(bDebug, sizeof(bDebug), "STACK OVERFLOW - %s", pcTaskName);

  DIG_wLoggerWritter(GET_TIMESTAMP(), (LEVEL_ERROR | DEBUG_QUEUE), (uint8_t*)bDebug , strlen(bDebug));

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Error(bDebug);
#endif

  while(1);
}

/******************************************************************************
* Function : DIG_prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
*//**
* \b Description:
*
* This function is called in case of HardFault.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //This function must be started by the IC
~~~~~~~~~~~~~~~
*
* @see HardFault_Handler
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 26/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void DIG_prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
  volatile uint32_t awThisData[14];
  static uint8_t infinite = 1;
  enum registers_e
  {
    r0  = 0,
    r1,
    r2,
    r3,
    r12,
    lr,
    pc,
    psr,
    _CFSR,
    _HFSR,
    _DFSR,
    _AFSR,
    _MMAR,
    _BFAR
  };

  awThisData[r0] = pulFaultStackAddress[ 0 ]; //r0
  awThisData[r1] = pulFaultStackAddress[ 1 ]; //r1
  awThisData[r2] = pulFaultStackAddress[ 2 ]; //r2
  awThisData[r3] = pulFaultStackAddress[ 3 ]; //r3

  awThisData[r12] = pulFaultStackAddress[ 4 ];  //r12
  awThisData[lr] = pulFaultStackAddress[ 5 ];   //lr
  awThisData[pc] = pulFaultStackAddress[ 6 ];   //pc
  awThisData[psr] = pulFaultStackAddress[ 7 ];  //psr

  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  awThisData[_CFSR] = (*((volatile uint32_t *)(0xE000ED28))) ;  //_CFSR

  // Hard Fault Status Register
  awThisData[_HFSR] = (*((volatile uint32_t *)(0xE000ED2C))) ;  //_HFSR

  // Debug Fault Status Register
  awThisData[_DFSR] = (*((volatile uint32_t *)(0xE000ED30))) ;  //_DFSR

  // Auxiliary Fault Status Register
  awThisData[_AFSR] = (*((volatile uint32_t *)(0xE000ED3C))) ;  //_AFSR

  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  awThisData[_MMAR] = (*((volatile uint32_t *)(0xE000ED34))) ;  //_MMAR
  // Bus Fault Address Register
  awThisData[_BFAR] = (*((volatile uint32_t *)(0xE000ED38))) ;  //_BFAR

  //Log Error
  DIG_wLoggerWritter(GET_TIMESTAMP(), (LEVEL_CRITICAL | DEBUG_HARDFAULT), (uint8_t*)awThisData , (sizeof(awThisData) / sizeof(uint8_t)));

  /* When the following line is hit, the variables contain the register values. */
  while(infinite){};//Restart?
}

/******************************************************************************
* Function : HardFault_Handler (void)
*//**
* \b Description:
*
* This is the Hard_fault handler.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //This is the Hard_fault function
~~~~~~~~~~~~~~~
*
* @see main
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 14/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
#ifndef UNITY_TEST
void HardFault_Handler (void)
{
  /* The fault handler implementation calls a function called
  DIG_prvGetRegistersFromStack(). */
  ASM volatile
  (
      " tst lr, #4                                                \n"
      " ite eq                                                    \n"
      " mrseq r0, msp                                             \n"
      " mrsne r0, psp                                             \n"
      " ldr r1, [r0, #24]                                         \n"
#if defined(__IAR_SYSTEMS_ICC__)
      " bl DIG_prvGetRegistersFromStack                           \n"
#elif defined (__GNUC__)
      " ldr r2, handler2_address_const                            \n"
      " bx r2                                                     \n"
      " handler2_address_const: .word DIG_prvGetRegistersFromStack\n"
#endif

  );
}
#endif

/******************************************************************************
* Function : DIG_onAssert__(unsigned int timer, char const *file, unsigned line)
*//**
* \b Description:
*
* This function is called every time an assertion fails.
*
* PRE-CONDITION: None
*
* POST-CONDITION: None
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //This is the assert function
~~~~~~~~~~~~~~~
*
* @see main
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 14/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
void DIG_onAssert__(unsigned int timer, uint32_t debugLevel, char const *file, unsigned line)
{
  char* p = (strrchr(file, '/') + 1);
  char bDebug[DIG_BUFFER_MAX_LINE_SIZE];
  memset(&bDebug[0], 0x00, DIG_BUFFER_MAX_LINE_SIZE);
  snprintf(bDebug, sizeof(bDebug), "File: [%s] - Line: [%u]", p, line);
  DIG_wLoggerWritter(timer, (debugLevel | DEBUG_ASSERT), (uint8_t*)bDebug , strlen(bDebug));

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SEGGER_SYSVIEW_Error(bDebug);
#endif
  if (debugLevel >= DBG_ASSERT_STOP_LEVEL) //If level larger or equal than defined, then stop, else continue
    {
      while(1);
    }
}

void DIG_onSubs__(unsigned int timer, uint8_t eDestine, uint8_t eTopic)
{
  uint8_t abData[2];
  abData[0] = eDestine;
  abData[1] = eTopic;
  DIG_wLoggerWritter(timer, (LEVEL_INFO | DEBUG_SUBS), (uint8_t*)abData, (sizeof(abData) / sizeof(abData[0])));
}

void DIG_onUnSubs__(unsigned int timer, uint8_t eDestine, uint8_t eTopic)
{
  uint8_t abData[2];
  abData[0] = eDestine;
  abData[1] = eTopic;
  DIG_wLoggerWritter(timer, (LEVEL_INFO | DEBUG_UNSUBS), (uint8_t*)&abData[0], (sizeof(abData) / sizeof(abData[0])));
}

void DIG_onWDT__(unsigned int timer, uint8_t eModule, uint8_t bFlagPos)
{
  uint8_t abData[2];
  abData[0] = eModule;
  abData[1] = bFlagPos;
  DIG_wLoggerWritter(timer, (LEVEL_ERROR | DEBUG_WATCHDOG), (uint8_t*)&abData[0], (sizeof(abData) / sizeof(abData[0])));
}

uint8_t DIG_vLoggerInit(osThreadId Thread)
{
  //TODO: Init Buffer and prepare HEADER (Firmware Version and board ID)
  uint8_t bResult = 0;
  if (!bBufferInitialized)
    {
      INITIALIZE_RINGBUFFER(uint8_t, logger, DIG_BUFFER_SIZE);
      bBufferInitialized = true;
      callerThread = Thread;
      bResult = 1;
    }
  return bResult;
}

uint32_t DIG_wLoggerWritter(uint32_t wTimestamp, uint8_t bLevelAndType, uint8_t* pbData, uint8_t bDataSize)
{
  osEnterCritical();
  uint32_t wResult = 0;
  if ((bLevelAndType <= DBG_LOG_LEVEL) && bBufferInitialized)
    {
      uint8_t bMaxSize = DIG_BUFFER_MAX_LINE_SIZE - sizeof(wTimestamp) - sizeof(bLevelAndType) - sizeof(bDataSize);
      uint8_t bTemp[DIG_BUFFER_MAX_LINE_SIZE];
      memset(&bTemp[0], 0x00, DIG_BUFFER_MAX_LINE_SIZE);
      memcpy(&bTemp[0], &wTimestamp, sizeof(wTimestamp));     //Timestamp (4 Bytes)
      memcpy(&bTemp[sizeof(wTimestamp)], &bLevelAndType, sizeof(bLevelAndType));    //Level and Type (1 Byte)
      memcpy(&bTemp[sizeof(wTimestamp) + sizeof(bLevelAndType)], &bDataSize, sizeof(bDataSize));    //DataSize (1 Byte)
      memcpy(&bTemp[sizeof(wTimestamp) + sizeof(bLevelAndType) + sizeof(bDataSize)], pbData, (bDataSize > bMaxSize ? bMaxSize : bDataSize));
      //insert into buffer
      if (!RingBuffer_InsertMult(&BUFFER(logger), &bTemp[0], DIG_BUFFER_MAX_LINE_SIZE))
        {
          uint8_t bGarbage[DIG_BUFFER_MAX_LINE_SIZE];
          RingBuffer_PopMult(&BUFFER(logger), &bGarbage[0], DIG_BUFFER_MAX_LINE_SIZE);
          RingBuffer_InsertMult(&BUFFER(logger), &bTemp[0], DIG_BUFFER_MAX_LINE_SIZE);
        }
      wResult = DIG_BUFFER_MAX_LINE_SIZE;
      if (callerThread != NULL)
        { //if watchdog or hardfault print
          if (((bLevelAndType & 0x0F) == DEBUG_HARDFAULT) || ((bLevelAndType & 0x0F) == DEBUG_WATCHDOG))
            {
              DEV_write(pTDVHandle, &bTemp, (sizeof(wTimestamp) + sizeof(bLevelAndType) + sizeof(bDataSize) + bDataSize));
            }
          else  //else signal receiver thread
            {
              osSignalSet(callerThread, 0xDE);
            }
        }
    }
  osExitCritical();
  return wResult;
}

void DIG_vLoggerHeaderReader(uint8_t* pbData, uint32_t* wDataSize)
{
  (void)pbData;
  osEnterCritical();
  //TODO: Read Header
  *wDataSize = DIG_BUFFER_MAX_LINE_SIZE;
  osExitCritical();
}

void DIG_vLoggerReader(uint8_t* pbData, uint32_t* wDataSize)
{
  osEnterCritical();
  *wDataSize = RingBuffer_PopMult(&BUFFER(logger), pbData, DIG_BUFFER_MAX_LINE_SIZE);
  osExitCritical();
}
/******************************************************************************
* Unity Testing
*******************************************************************************/
#if defined(UNITY_TEST)

TEST_GROUP(DiagLoggerTest);

TEST_SETUP(DiagLoggerTest)
{
  //This is run before EACH TEST
}

TEST_TEAR_DOWN(DiagLoggerTest)
{
  //This is run after EACH TEST
}


TEST(DiagLoggerTest, test_Verify_onAssert)
{
  bBufferInitialized = false;
  //Test not initialized
  uint8_t bLevel = LEVEL_INFO;
  uint32_t wFakeTimestamp = 123456789;
  uint8_t abFile[] = "/PATH/TO/FILE.c";
  uint32_t wLine = 156;

  DIG_onAssert__(wFakeTimestamp, bLevel, (char const *)abFile, wLine);
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));

  //Initialize and re-test
  DIG_vLoggerInit(NULL);
  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  DIG_onAssert__(wFakeTimestamp, bLevel, (char const *)abFile, wLine);
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, RingBuffer_GetCount(&BUFFER(logger)));

  //Verify saved buffer
  uint8_t abResult[DIG_BUFFER_MAX_LINE_SIZE];
  uint32_t wSize;
  DIG_vLoggerReader(abResult, &wSize);

  //Verify result
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, wSize);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 0) & 0xFF), abResult[0]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 8) & 0xFF), abResult[1]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 16) & 0xFF), abResult[2]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 24) & 0xFF), abResult[3]);
  TEST_ASSERT_EQUAL((LEVEL_INFO | DEBUG_ASSERT), abResult[4]);

  uint8_t* pbBuffer = &abResult[6];
  char* p = (strrchr((const char*)abFile, '/') + 1);
  char bDebug[DIG_BUFFER_MAX_LINE_SIZE];
  memset(&bDebug[0], 0x00, DIG_BUFFER_MAX_LINE_SIZE);
  snprintf(bDebug, sizeof(bDebug), "File: [%s] - Line: [%u]", p, wLine);

  STRCMP_EQUAL(pbBuffer, bDebug);

  //Test if buffer was removed
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));
  //Restore
  bBufferInitialized = false;
}

TEST(DiagLoggerTest, test_Verify_onSubs)
{
  bBufferInitialized = false;
  //Test not initialized
  uint8_t bDestine = MODULE_ACTUATOR;
  uint8_t bTopic = TOPIC_COMM;
  uint32_t wFakeTimestamp = 123456789;

  DIG_onSubs__(wFakeTimestamp, bDestine, bTopic);
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));

  //Initialize and re-test
  DIG_vLoggerInit(NULL);
  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  DIG_onSubs__(wFakeTimestamp, bDestine, bTopic);
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, RingBuffer_GetCount(&BUFFER(logger)));

  //Verify saved buffer
  uint8_t abResult[DIG_BUFFER_MAX_LINE_SIZE];
  uint32_t wSize;
  DIG_vLoggerReader(abResult, &wSize);

  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, wSize);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 0) & 0xFF), abResult[0]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 8) & 0xFF), abResult[1]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 16) & 0xFF), abResult[2]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 24) & 0xFF), abResult[3]);
  TEST_ASSERT_EQUAL((LEVEL_INFO | DEBUG_SUBS), abResult[4]);

  TEST_ASSERT_EQUAL(2, abResult[5]);        //size of data
  TEST_ASSERT_EQUAL(bDestine, abResult[6]);
  TEST_ASSERT_EQUAL(bTopic, abResult[7]);
  //Test if buffer was removed
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));
  //Restore
  bBufferInitialized = false;

}

TEST(DiagLoggerTest, test_Verify_onUnSubs)
{
  bBufferInitialized = false;
  //Test not initialized
  uint8_t bDestine = MODULE_ACTUATOR;
  uint8_t bTopic = TOPIC_COMM;
  uint32_t wFakeTimestamp = 123456789;

  DIG_onUnSubs__(wFakeTimestamp, bDestine, bTopic);
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));

  //Initialize and re-test
  DIG_vLoggerInit(NULL);
  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  DIG_onUnSubs__(wFakeTimestamp, bDestine, bTopic);
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, RingBuffer_GetCount(&BUFFER(logger)));

  //Verify saved buffer
  uint8_t abResult[DIG_BUFFER_MAX_LINE_SIZE];
  uint32_t wSize;
  DIG_vLoggerReader(abResult, &wSize);

  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, wSize);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 0) & 0xFF), abResult[0]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 8) & 0xFF), abResult[1]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 16) & 0xFF), abResult[2]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 24) & 0xFF), abResult[3]);
  TEST_ASSERT_EQUAL((LEVEL_INFO | DEBUG_UNSUBS), abResult[4]);

  TEST_ASSERT_EQUAL(bDestine, abResult[6]);
  TEST_ASSERT_EQUAL(bTopic, abResult[7]);
  //Test if buffer was removed
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));
  //Restore
  bBufferInitialized = false;

}

TEST(DiagLoggerTest, test_Verify_loggerInit)
{
  DIG_vLoggerInit(NULL);
  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));
  TEST_ASSERT_EQUAL(DIG_BUFFER_SIZE, RingBuffer_GetFree(&BUFFER(logger)));

}

TEST(DiagLoggerTest, test_Verify_loggerWritter)
{
  bBufferInitialized = false;
  //Test not initialized
  uint8_t bLevel = LEVEL_CRITICAL;
  uint8_t bType = DEBUG_USER;
  uint32_t wFakeTimestamp = 123456789;
  uint8_t abBuffer[] = "Testing Logger";
  uint32_t wBufferSize = sizeof(abBuffer) / sizeof(abBuffer[0]);

  TEST_ASSERT_EQUAL(0, DIG_wLoggerWritter(wFakeTimestamp, (bLevel | bType), &abBuffer[0], wBufferSize));
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));

  //Initialize and re-test
  DIG_vLoggerInit(NULL);
  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, DIG_wLoggerWritter(wFakeTimestamp, (bLevel | bType), &abBuffer[0], wBufferSize));
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, RingBuffer_GetCount(&BUFFER(logger)));

  //Verify saved buffer
  uint8_t abResult[DIG_BUFFER_MAX_LINE_SIZE];
  RingBuffer_PopMult(&BUFFER(logger), &abResult[0], DIG_BUFFER_MAX_LINE_SIZE);

  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 0) & 0xFF), abResult[0]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 8) & 0xFF), abResult[1]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 16) & 0xFF), abResult[2]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 24) & 0xFF), abResult[3]);
  TEST_ASSERT_EQUAL((bLevel | bType), abResult[4]); //level and type
  TEST_ASSERT_EQUAL(sizeof(abBuffer), abResult[5]); //size of data
  uint8_t * pbResult = &abResult[6];
  uint8_t * pbBuffer = &abBuffer[0];
  STRCMP_EQUAL(pbBuffer, pbResult);
  //Test if buffer was removed
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));

  //TODO: Test Log levels

  //Restore
  bBufferInitialized = false;
}

TEST(DiagLoggerTest, test_Verify_loggerReader)
{
  bBufferInitialized = false;
  //Test not initialized
  uint8_t bLevel = LEVEL_CRITICAL;
  uint8_t bType = DEBUG_USER;
  uint32_t wFakeTimestamp = 123456789;
  uint8_t abBuffer[] = "Testing Logger";
  uint32_t wBufferSize = sizeof(abBuffer) / sizeof(abBuffer[0]);

  TEST_ASSERT_EQUAL(0, DIG_wLoggerWritter(wFakeTimestamp, (bLevel | bType), &abBuffer[0], wBufferSize));
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));

  //Initialize and re-test
  DIG_vLoggerInit(NULL);
  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, DIG_wLoggerWritter(wFakeTimestamp, (bLevel | bType), &abBuffer[0], wBufferSize));
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, RingBuffer_GetCount(&BUFFER(logger)));

  //Verify saved buffer
  uint8_t abResult[DIG_BUFFER_MAX_LINE_SIZE];
  uint32_t wSize;
  DIG_vLoggerReader(abResult, &wSize);

  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, wSize);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 0) & 0xFF), abResult[0]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 8) & 0xFF), abResult[1]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 16) & 0xFF), abResult[2]);
  TEST_ASSERT_EQUAL(((wFakeTimestamp >> 24) & 0xFF), abResult[3]);
  TEST_ASSERT_EQUAL((bLevel | bType), abResult[4]);
  uint8_t * pbResult = &abResult[6];
  uint8_t * pbBuffer = &abBuffer[0];
  STRCMP_EQUAL(pbBuffer, pbResult);
  //Test if buffer was removed
  TEST_ASSERT_EQUAL(0, RingBuffer_GetCount(&BUFFER(logger)));
  //Restore
  bBufferInitialized = false;
}

TEST(DiagLoggerTest, test_Verify_loggerHeaderReader)
{
  //TODO: Implement reader
}

TEST(DiagLoggerTest, test_Verify_Printf)
{
  uint32_t wNumber = 1234;

  DIG_vLoggerInit(NULL);
  DBG_LOG_INFO("Testing %d", wNumber);

  TEST_ASSERT_EQUAL(true, bBufferInitialized);
  TEST_ASSERT_EQUAL(DIG_BUFFER_MAX_LINE_SIZE, RingBuffer_GetCount(&BUFFER(logger)));

  //Verify saved buffer
  uint8_t abResult[DIG_BUFFER_MAX_LINE_SIZE];
  uint8_t abTest[DIG_BUFFER_MAX_LINE_SIZE];
  uint32_t wSize;
  DIG_vLoggerReader(abResult, &wSize);
  TEST_ASSERT_EQUAL((DEBUG_USER | LEVEL_INFO), abResult[4]);
  snprintf((char*)abTest, DIG_BUFFER_MAX_LINE_SIZE, "Testing %d", wNumber);

  uint8_t * pbResult = &abResult[6];
  uint8_t * pbBuffer = &abTest[0];
  STRCMP_EQUAL(pbBuffer, pbResult);
}
#endif
