/****************************************************************************
 * Title                 :   unity_test_diagnostic_runner Source File
 * Filename              :   unity_test_diagnostic_runner.c
 * Author                :   Joao Paulo Martins
 * Origin Date           :   08 de abr de 2016
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
 *    Date    Version       Author                     Description
 *  08/04/16   1.0.0  Joao Paulo Martins   unity_test_diagnostic_runner.c created.
 *
 *****************************************************************************/
/** @file  unity_test_diagnostic_runner.c
 *  @brief
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
TEST_GROUP_RUNNER(DiagnosticModuleTest)
{
	RUN_TEST_CASE(DiagnosticModuleTest, test_Verify_InitDeviceLayer);
	RUN_TEST_CASE(DiagnosticModuleTest, test_Verify_eInitDiagPublisher);
	RUN_TEST_CASE(DiagnosticModuleTest, test_Verify_wLOG_PackBrokerTransaction);
	RUN_TEST_CASE(DiagnosticModuleTest, test_Verify_DetectThread);
}

TEST_GROUP_RUNNER(DiagLoggerTest)
{
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_onAssert);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_onSubs);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_onUnSubs);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_loggerInit);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_loggerWritter);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_loggerReader);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_loggerHeaderReader);
	RUN_TEST_CASE(DiagLoggerTest, test_Verify_Printf);
}
#endif
