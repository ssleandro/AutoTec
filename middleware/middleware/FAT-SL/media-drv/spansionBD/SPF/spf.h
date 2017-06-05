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
#ifndef SPF_H_
#define SPF_H_

#include <inttypes.h>

#define FLS_DEFAULT_SPIFI_INTERFACE SPIFI0      //!< Default SPIFI Interface
#define FLS_STARTING_MEMORY_ADDRESS 0x14000000  //!< Starting SPIFI Address

// SLLD Returned values
typedef enum
{
	SPF_OK = 0x0,
	SPF_E_DEVICE_SOFTWARE_PROTECTED,
	SPF_E_HAL_ERROR = 0x200,
	SPF_ERROR = 0xFFFF
} SPF_STATUS;

typedef enum
{
	dev_status_unknown = 0,
	dev_not_busy,
	dev_program_error,
	dev_erase_error,
	dev_suspend,
	dev_busy
} DEVSTATUS;

extern SPF_STATUS SPF_Init (void);
extern SPF_STATUS SPF_Read (uint32_t sys_addr, uint8_t *target, uint32_t len_in_bytes);
extern SPF_STATUS SPF_Write (uint32_t sys_addr, uint8_t *data_buf, uint32_t len_in_bytes, DEVSTATUS *dev_status_ptr);
extern SPF_STATUS SPF_Erase (uint32_t sys_addr, DEVSTATUS *dev_status_ptr);

#endif
