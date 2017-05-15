/****************************************************************************
* Title                 :   unity_test_broker_runner Include File
* Filename              :   unity_test_broker_runner.c
* Author                :   thiago.palmieri
* Origin Date           :   9 de mar de 2016
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
*  9 de mar de 2016   1.0.0   thiago.palmieri unity_test_broker_runner include file Created.
*
*****************************************************************************/
/** @file unity_test_broker_runner.c
 *  @brief This file provides <add description>
 *
 */
#if defined(UNITY_TEST)
/******************************************************************************
* Includes
*******************************************************************************/
#include "unity_fixture.h"
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
TEST_GROUP_RUNNER(MLX75305DeviceTest)
{
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_that_data_convertion_is_correct);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_ChangeResType);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_ChangeSampleMode);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_ChangeCallBack);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_ChangeBurstMode);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_ChangeResolution);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_ChangeSampleRate);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_MLX_open);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_MLX_read);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_MLX_ioctl);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_MLX_write);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_MLX_close);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_DEV_open);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_DEV_close);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_DEV_write);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_DEV_ioctl);
  RUN_TEST_CASE(MLX75305DeviceTest, test_Verify_DEV_read);
}

#endif
