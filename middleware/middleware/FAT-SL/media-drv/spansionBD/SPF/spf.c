/****************************************************************************
 * Title                 :   FLASHER Include File
 * Filename              :   FLASHER.c
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
 *  12 de abr de 2016   1.0.0   thiago.palmieri FLASHER include file Created.
 *
 *****************************************************************************/
/** @file fpifi.c
 *  @brief This file provides Access to External Flash for programming, reading
 *  and erasing.
 *
 *  After device initialization, the External Flash will be available for reading
 *  via pointer addresses as usual.
 *
 *  In order to program some value on the memory, three steps must be taken.
 *  1. Set the starting programming address via IOCTL command
 *  2. Erase the Interval to be programmed
 *  3. Finally, program the address via WRITE command
 *
 *  Reading can be made in one or two steps.
 *  1. Reading via pointer to address, as usual
 *  or
 *  1a.Set the Reading Address via IOCTL command
 *  2a.Use READ command to read a number of bytes
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "spf.h"

#include "ring_buffer.h"
#include "mculib.h"
#include <strings.h>

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
/**
 * Default Sensor Initialization Values
 */
static spifi_config_s sFLASHER =
	{
		.eSPIFI = FLS_DEFAULT_SPIFI_INTERFACE,
		.wSPIFIBaseAddress = FLS_STARTING_MEMORY_ADDRESS,
	};

static uint8_t SPF_Inited = 0;

SPF_STATUS SPF_Init (void)
{
	SPF_STATUS eError = SPF_OK;
	if (SPF_Inited == 0)
	{
		sFLASHER.eSPIFI = FLS_DEFAULT_SPIFI_INTERFACE;
		sFLASHER.wSPIFIBaseAddress = FLS_STARTING_MEMORY_ADDRESS;
		sFLASHER.vpPrivateData = NULL;
		SPF_Inited = 1;
		if (SPF_eInit(&sFLASHER) != MCU_ERROR_SUCCESS)
		{
			SPF_Inited = 0;
			eError = SPF_ERROR;
		}
	}
	return eError;
}

SPF_STATUS SPF_Read (uint32_t sys_addr, uint8_t *target, uint32_t len_in_bytes)
{
	uint32_t wResult = SPF_ERROR;
	uint32_t wReadAddress = FLS_STARTING_MEMORY_ADDRESS + sys_addr;
	if (target != NULL)
	{
		if (SPF_eRead(&sFLASHER, wReadAddress, (uint32_t*)target, len_in_bytes) == MCU_ERROR_SUCCESS)
		{
			wResult = SPF_OK;
		}
	}
	return wResult;
}

SPF_STATUS SPF_Write (uint32_t sys_addr, uint8_t *data_buf, uint32_t len_in_bytes, DEVSTATUS *dev_status_ptr)
{
	uint32_t wResult = SPF_ERROR;
	uint32_t wReadAddress = FLS_STARTING_MEMORY_ADDRESS + sys_addr;

	if ((data_buf != NULL) && (len_in_bytes > 0))
	{
		if (SPF_eProgram(&sFLASHER, wReadAddress, (uint32_t*)data_buf, len_in_bytes) == MCU_ERROR_SUCCESS)
		{
			wResult = SPF_OK;
		}
	}
	return wResult;
}

SPF_STATUS SPF_Erase (uint32_t sys_addr, DEVSTATUS *dev_status_ptr)
{
	uint32_t wResult = SPF_ERROR;
	uint32_t wReadAddress = FLS_STARTING_MEMORY_ADDRESS + sys_addr;

	if (SPF_eErase(&sFLASHER, true, wReadAddress, wReadAddress + 0x1000) == MCU_ERROR_SUCCESS)
	{
		wResult = SPF_OK;
	}

	return wResult;
}
