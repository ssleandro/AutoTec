/****************************************************************************
 * Title                 :   DS2411R_config
 * Filename              :   DS2411R_config.h
 * Author                :   Henrique Reis
 * Origin Date           :   03/11/2017
 * Version               :   1.0.0
 * Compiler              :   GCC 5.4 2016q2 / ICCARM 7.40.3.8938
 * Target                :   LPC43XX M4
 * Notes                 :   Qualicode Machine Technologies
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
 *    Date    Version       Author          Description
 * 03/11/2017  1.0.0     Henrique Reis      DS2411R_config.h created.
 *
 *****************************************************************************/
#ifndef DEVICES_DEV_COMMON_DS2411R_CONFIG_H_
#define DEVICES_DEV_COMMON_DS2411R_CONFIG_H_

/******************************************************************************
* Includes
*******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#endif

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define DS2411R_ID_IO_GPIO_PORT			0x0C
#define DS2411R_ID_IO_GPIO_PIN			0x04

#define DS2411R_CMD_READ_ROM				0x33
#define DS2411R_CMD_SEARCH_ROM			0xF0
#define DS2411R_CMD_OVERDRIVE_SKIP_ROM	0x3C

#define DS2411R_ID_BYTES_NUMBER			8

#define DS2411R_RESET_LOW_TIME_US		480
#define DS2411R_PRES_DETECT_TIME_US		70
#define DS2411R_WRITE_0_LOW_TIME_US		60
#define DS2411R_WRITE_1_LOW_TIME_US		10
#define DS2411R_RECOVERY_TIME_US			10
#define DS2411R_TIME_SLOT_DURATION_US	65
#define DS2411R_READ_LOW_TIME_US			480
#define DS2411R_READ_SAMPLE_TIME_US		4

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Public Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

#endif /* DEVICES_DEV_COMMON_DS2411R_CONFIG_H_ */
