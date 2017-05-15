/****************************************************************************
* Title                 :
* Filename              :
* Author                :
* Origin Date           :   10/05/2016
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
*    Date    		Version   		Author         		Description
*  10/05/2016		 1.0.0		Joao Paulo Martins	File creation
*
*****************************************************************************/
/** @file CHIPPROG.h
 *  @brief This file provides on-chip flash programming utilities
 *
 */

#ifndef DEVICES_DEV_COMMON_CHIPPROG_H_
#define DEVICES_DEV_COMMON_CHIPPROG_H_

/******************************************************************************
* Includes
*******************************************************************************/
#ifndef UNITY_TEST
#include "mculib.h"
#else
#endif

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
#define CHP_INVALID_SECTOR 	0xFFFFFFFF
#define CHP_INVALID_ADDRESS 0xFFFFFFFF
#define CHP_INVALID_BANK		0xFF

/******************************************************************************
* Configuration Constants
*******************************************************************************/
//#define CHP_OPMODE_IDLE		0
//#define CHP_OPMODE_ERASE	1
//#define CHP_OPMODE_WRITE	2

/******************************************************************************
* Macros
*******************************************************************************/
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
#define IOCTL_CHIPPROG \
  X(IOCTL_CHP_START_ADDR, CHP_eSetStartAddr) 	\
  X(IOCTL_CHP_END_ADDR, CHP_eSetEndAddr) 			\
	X(IOCTL_CHP_ERASE_MEM, CHP_eEraseMemory)		\
	X(IOCTL_CHP_DEST_ADDR, CHP_eSetDestAddr)		\
	X(IOCTL_CHP_INVALID, NULL)									\

/**
 * This Typedef holds all possible IOCTL Requests for Chip Programming Module
 */
#define X(a, b) a,
typedef enum IOCTL_CHP_REQUEST_e
{
  IOCTL_CHIPPROG
}IOCTL_CHP_REQUEST_e;
#undef X

/******************************************************************************
* Typedefs
*******************************************************************************/
/******************************************************************************
* Variables
*******************************************************************************/
/******************************************************************************
* Function Prototypes
*******************************************************************************/

#endif /* DEVICES_DEV_COMMON_CHIPPROG_H_ */
