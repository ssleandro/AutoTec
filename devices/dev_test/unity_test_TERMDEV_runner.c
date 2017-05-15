/****************************************************************************
* Title                 :
* Filename              :
* Author                :
* Origin Date           :
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
*
*
*****************************************************************************/
/** @file
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


/******************************************************************************
* Unity Testing
*******************************************************************************/
TEST_GROUP_RUNNER(TERMDEVDeviceTest)
{
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_vRBSafeInsert);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_wReadBufferProcedure);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_CANCallback);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_USBCallback);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_UARTCallback);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_eSetActive);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_eDisable);
	RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_eCANAddID);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_open);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_write);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_read);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_TDV_ioctl);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_DEV_open);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_DEV_ioctl);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_DEV_write);
  RUN_TEST_CASE(TERMDEVDeviceTest, test_Verify_DEV_read);
}

#endif
