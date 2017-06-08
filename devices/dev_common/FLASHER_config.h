/****************************************************************************
 * Title                 :   FLASHER_config Include File
 * Filename              :   FLASHER_config.h
 * Author                :   thiago.palmieri
 * Origin Date           :   12 de abr de 2016
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
 *  12 de abr de 2016   1.0.0   thiago.palmieri FLASHER_config include file Created.
 *
 *****************************************************************************/
/** @file FLASHER_config.h
 *  @brief This file provides Initial configuration to be used by the FLASHER interface.
 *
 */
#ifndef COMMON_FLASHER_CONFIG_H_
#define COMMON_FLASHER_CONFIG_H_

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
#define FLS_DEFAULT_SPIFI_INTERFACE SPIFI0      //!< Default SPIFI Interface
#define FLS_STARTING_MEMORY_ADDRESS 0x14000000  //!< Starting SPIFI Address
/******************************************************************************
 * Macros
 *******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
//CAN, PORT, TD, RD, MODE
#define IOCTL_FLASHER \
  X(IOCTL_FLS_SET_PROGRAM_ADDRESS, FLS_eIOCTLSetAddress) \
  X(IOCTL_FLS_ERASE_BLOCK, FLS_eIOCTLErase) \
  X(IOCTL_FLS_ERASE_ADDRESS, FLS_eIOCTLErase) \
  X(IOCTL_FLS_GET_MEM_SIZE, FLS_eIOCTLgets) \
  X(IOCTL_FLS_GET_BLOCK_SIZE, FLS_eIOCTLgets) \
  X(IOCTL_FLS_GET_PAGE_SIZE, FLS_eIOCTLgets) \
  X(IOCTL_FLS_GET_MAX_CLOCK, FLS_eIOCTLgets) \
  X(IOCTL_FLS_INVALID, NULL) \

/******************************************************************************
 * Typedefs
 *******************************************************************************/
/**
 * This Typedef holds all possible IOCTL Requests for CBA Communicator
 */
#define X(a, b) a,
typedef enum IOCTL_FLS_REQUEST_e
{
	IOCTL_FLASHER
} IOCTL_FLS_REQUEST_e;
#undef X
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

#endif /* COMMON_FLASHER_CONFIG_H_ */

/*** End of File **************************************************************/
