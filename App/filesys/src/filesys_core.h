/****************************************************************************
 * Title                 :   filesys_core Include File
 * Filename              :   filesys_core.h
 * Author                :   Henrique Reis
 * Origin Date           :
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
 *    Date    Version        Author              Description
 *  XX/XX/XX   1.0.0      Henrique Reis    filesys_core.h created.
 *
 *****************************************************************************/
/** @file   filesys_core.h
 *  @brief
 *
 */
#ifndef FILESYS_SRC_FILESYS_CORE_H_
#define FILESYS_SRC_FILESYS_CORE_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <M2G_app.h>
#if defined (UNITY_TEST)
#include "../filesys/inc/interface_filesys.h"
#else
#include "interface_filesys.h"
#endif
/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/
#define FFS_SIGNAL_PUBLISH			0x55

#define FFS_FLAG_ALL 			0x0FFF
#define FFS_FLAG_STATUS 		0x0001
#define FFS_FLAG_CFG 			0x0002
#define FFS_FLAG_SENSOR_CFG	0x0004
#define FFS_FLAG_STATIC_REG	0x0008

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/

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
 * Function Prototypes
 *******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* FILESYS_SRC_FILESYS_CORE_H_ */
