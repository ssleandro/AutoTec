/****************************************************************************
 * Title                 :   isobus_app
 * Filename              :   isobus_app.h
 * Author                :   Henrique Reis
 * Origin Date           :   29/05/2017
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
 * 29/05/2017  1.0.0     Henrique Reis      isobus_app.h created.
 *
 *****************************************************************************/
#ifndef APP_COMMON_APP_COMMON_ISOBUS_APP_H_
#define APP_COMMON_APP_COMMON_ISOBUS_APP_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

typedef enum
{
	DATA_MASK_CONFIGURATION = 0x5000,
	DATA_MASK_INSTALLATION,
	DATA_MASK_PLANTER,
	DATA_MASK_TEST_MODE,
	DATA_MASK_TRIMMING,
	DATA_MASK_SYSTEM,
	ALARM_MASK_CONFIRM_INSTALLATION = 0x50F0,
	DATA_MASK_INVALID
} eIsobusMask;

typedef enum
{
	STATUS_INSTALL_WAITING = 0x000E,
	STATUS_INSTALL_INSTALLING = 0x010E,
	STATUS_INSTALL_INSTALLED = 0x000A,
	STATUS_INSTALL_INSTALL_ERROR = 0x000C,
	STATUS_INSTALL_NONE = 0x0007,
	STATUS_INSTALL_INVALID
} eInstallationStatus;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#endif /* APP_COMMON_APP_COMMON_ISOBUS_APP_H_ */
