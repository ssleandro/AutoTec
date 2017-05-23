/****************************************************************************
* Title                 :   MCU ACCESS FILE
* Filename              :   mcu.c
* Author                :   Thiago Palmieri
* Origin Date           :   18/02/2016
* Version               :   1.0.0
* Compiler              :   GCC 5.2 2015q4
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
*  18/02/16   1.0.0   Thiago Palmieri MCU ACCESS Created.
*
*****************************************************************************/
/** @file mcu.c
 *  @brief This module provides a MCU data access for the upper layers
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/

#include "mcu.h"
#include "board.h"
#include "chip.h"

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

void MCU_vSysInit(void)
{
  BRD_vSystemCoreClockUpdate();
}

void MCU_vSDRAMInit(void)
{
    BRD_ExtMemoryConfig();    
}

uint32_t MCU_wReadBootCode(void)
{
  Chip_IAP_init();
  return Chip_IAP_ReadBootCode();
}

uint32_t MCU_wReadPID(void)
{
  Chip_IAP_init();
  return Chip_IAP_ReadPID();
}

void MCU_vFFSInit (void)
{
	fs_init();
}
