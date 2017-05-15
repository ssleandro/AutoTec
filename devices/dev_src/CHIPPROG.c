/****************************************************************************
* Title                 :		Chip Programmer Device
* Filename              :		CHIPPROG.c
* Author                :		Joao Paulo Martins
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
/** @file CHIPPROG.c
 *  @brief This file provides on-chip flash programming utilities
 *
 *  This module is a device API to interact with on-chip flash features. Using
 *  the In-Application Programming (IAP) functions (based on IAP MCULIB module),
 *  is possible to program the on-chip flash with user-defined data, check for
 *  blank sectors and read the memory.
 *
 *  Specific macros and constant configurations are set at the CHIPPROG_config.h file.
 *
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "CHIPPROG.h"
#include <stddef.h>
#include <stdbool.h>
#ifndef UNITY_TEST
#include "mculib.h"
#else
#include "unity_fixture.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#endif

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
/******************************************************************************
* Module Typedefs
*******************************************************************************/
#if defined (UNITY_TEST)
void UNITY_TEST_VARIABLES(void);

#define ONCHIP_NUM_SECTORS 15
#define ONCHIP_SECTOR_A			0
#define ONCHIP_SECTOR_B			1
#define INVALID_VALUE			0xFFFFFFFF

#define LPC4357_SECTORS \
	X(0,  SECTOR_A0_START, 	SECTOR_A0_END, 	SECTOR_B0_START, 	SECTOR_B0_END)		\
	X(1,  SECTOR_A1_START, 	SECTOR_A1_END, 	SECTOR_B1_START, 	SECTOR_B1_END)		\
	X(2,  SECTOR_A2_START, 	SECTOR_A2_END, 	SECTOR_B2_START, 	SECTOR_B2_END)		\
	X(3,  SECTOR_A3_START, 	SECTOR_A3_END, 	SECTOR_B3_START, 	SECTOR_B3_END)		\
	X(4,  SECTOR_A4_START, 	SECTOR_A4_END, 	SECTOR_B4_START, 	SECTOR_B4_END)		\
	X(5,  SECTOR_A5_START, 	SECTOR_A5_END, 	SECTOR_B5_START, 	SECTOR_B5_END)		\
	X(6,  SECTOR_A6_START, 	SECTOR_A6_END, 	SECTOR_B6_START, 	SECTOR_B6_END)		\
	X(7,  SECTOR_A7_START, 	SECTOR_A7_END, 	SECTOR_B7_START, 	SECTOR_B7_END)		\
	X(8,  SECTOR_A8_START, 	SECTOR_A8_END, 	SECTOR_B8_START, 	SECTOR_B8_END)		\
	X(9,  SECTOR_A9_START, 	SECTOR_A9_END, 	SECTOR_B9_START, 	SECTOR_B9_END)		\
	X(10, SECTOR_A10_START, SECTOR_A10_END, SECTOR_B10_START, SECTOR_B10_END)		\
	X(11, SECTOR_A11_START, SECTOR_A11_END, SECTOR_B11_START, SECTOR_B11_END)		\
	X(12, SECTOR_A12_START, SECTOR_A12_END, SECTOR_B12_START, SECTOR_B12_END)		\
	X(13, SECTOR_A13_START, SECTOR_A13_END, SECTOR_B13_START, SECTOR_B13_END)		\
	X(14, SECTOR_A14_START, SECTOR_A14_END, SECTOR_B14_START, SECTOR_B14_END)		\

#define X(IND, AS, AE, BS, BE) [IND] = AS,
const uint32_t wSectorA_StartAddr[] = {
		LPC4357_SECTORS
};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = BS,
const uint32_t wSectorB_StartAddr[] = {
		LPC4357_SECTORS
};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = AE,
const uint32_t wSectorA_EndAddr[] = {
		LPC4357_SECTORS
};
#undef X

#define X(IND, AS, AE, BS, BE) [IND] = BE,
const uint32_t wSectorB_EndAddr[] = {
		LPC4357_SECTORS
};
#undef X

typedef enum eMCUError_s
{
  MCU_ERROR_SUCCESS                    = 0,   //!< SUCCESS
  MCU_ERROR_IAP_CMD_SUCCESS             	,	  //!< IAP command is executed successfully */
  MCU_ERROR_IAP_INVALID_COMMAND         	,   //!< IAP invalid command */
  MCU_ERROR_IAP_SRC_ADDR_ERROR          	,	  //!< IAP sector source address is not on word boundary */
  MCU_ERROR_IAP_DST_ADDR_ERROR          	,	  //!< IAP sector destination address is not on a correct boundary */
  MCU_ERROR_IAP_SRC_ADDR_NOT_MAPPED     	,	  //!< IAP source address is not mapped in the memory map */
  MCU_ERROR_IAP_DST_ADDR_NOT_MAPPED     	,	  //!< IAP destination address is not mapped in the memory map */
  MCU_ERROR_IAP_COUNT_ERROR             	,	  //!< IAP byte count is not multiple of 4 or is not a permitted value */
  MCU_ERROR_IAP_INVALID_SECTOR          	,	  //!< IAP sector number is invalid or end sector number is greater than start sector number */
  MCU_ERROR_IAP_SECTOR_NOT_BLANK        	,	  //!< IAP sector is not blank */
  MCU_ERROR_IAP_SECTOR_NOT_PREPARED     	,	  //!< IAP command to prepare sector for write operation was not executed */
  MCU_ERROR_IAP_COMPARE_ERROR           	,	  //!< IAP source and destination data not equal */
  MCU_ERROR_IAP_BUSY                    	,	  //!< IAP flash programming hardware interface is busy */
  MCU_ERROR_IAP_PARAM_ERROR             	,	  //!< IAP insufficient number of parameters or invalid parameter */
  MCU_ERROR_IAP_ADDR_ERROR              	,	  //!< IAP address is not on word boundary */
  MCU_ERROR_IAP_ADDR_NOT_MAPPED         	,	  //!< IAP address is not mapped in the memory map */
  MCU_ERROR_IAP_CMD_LOCKED              	,	  //!< IAP command is locked */
  MCU_ERROR_IAP_INVALID_CODE            	,	  //!< IAP unlock code is invalid */
  MCU_ERROR_IAP_INVALID_BAUD_RATE       	,	  //!< IAP invalid baud rate setting */
  MCU_ERROR_IAP_INVALID_STOP_BIT        	,	  //!< IAP invalid stop bit setting */
  MCU_ERROR_IAP_CRP_ENABLED             	,	  //!< IAP code read protection enabled */

}eMCUError_s;

void END_OF_UNITY_TEST_VARIABLES(void);
#endif

typedef eDEVError_s (*fpIOCTLFunction)(uint32_t wRequest, void * vpValue); //!< IOCTL function pointer

/* @var sCHP_Manager_s
 * @brief Typedef of struct created to define a type for a control variable of the module,
 * it contains the necessary information to perform the on-chip flash memory transctions.
 * */
typedef struct sCHP_Manager_s
{
	uint32_t *wpStartAddr;	//!< Start address of a read/write operation;
	uint32_t *wpEndAddr;		//!< End address of a read/write operation;
	uint32_t wStartSector;	//!< Start sector of a read/write operation;
	uint32_t wEndSector;		//!< End sector of a read/write operation;
	uint8_t  bMemoryBank;		//!< Bank of the flash area where the read/write will occur;
	uint32_t *wpSourceAddr;	//!< Source address when copying from one area to another
	uint32_t *wpDestAddr;		//!< Destination address when copying from one area to another
	chp_opmode_e eOpMode;		//!< Enum parameter to describe the operation (IDLE, WRITE, READ)
	uint8_t	 bStatus;				//!< Byte value to get the error codes from MCULIB IAP operations
} sCHP_Manager_s;

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
/* @var sCHP_Manager
 * @param This variable holds the necessary information to perform read and write
 * operations. It's not accessible from outside the module, only using the IOCTL
 * device calls. After a successful operation, this var is reset to be used on the
 * next operation */
static sCHP_Manager_s sCHP_Manager =
{
		.wpStartAddr = NULL,
		.wpEndAddr = NULL,
		.wStartSector = CHP_INVALID_SECTOR,
		.wEndSector = CHP_INVALID_SECTOR,
		.bMemoryBank = CHP_INVALID_BANK,
		.wpSourceAddr = NULL,
		.wpEndAddr = NULL,
		.eOpMode = CHP_OPMODE_IDLE,
		.bStatus = (uint8_t) MCU_ERROR_SUCCESS,
};

/******************************************************************************
* Function Prototypes
*******************************************************************************/
static eDEVError_s CHP_eSetStartAddr(uint32_t wRequest, void * vpValue);
static eDEVError_s CHP_eSetEndAddr(uint32_t wRequest, void * vpValue);
static eDEVError_s CHP_eEraseMemory(uint32_t wRequest, void * vpValue);
static eDEVError_s CHP_eSetDestAddr(uint32_t wRequest, void * vpValue);

#define X(a, b) b,
fpIOCTLFunction CHP_pIOCTLFunction[] =  //!< IOCTL array of function mapping
{
    IOCTL_CHIPPROG
};
#undef X
/******************************************************************************
* Function Definitions
*******************************************************************************/
#if defined(UNITY_TEST)
	void UNITY_TESTING_FUNCTIONS(void);

	eMCUError_s IAP_eInit(void) {return MCU_ERROR_SUCCESS;}
	eMCUError_s	IAP_ePrepareForReadWrite(uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank)
	{return MCU_ERROR_SUCCESS;}
	eMCUError_s IAP_eCopyRamToFlash(uint32_t wDstAdd, uint32_t *wpSrcAdd, uint32_t wByteswrt)
	{return MCU_ERROR_SUCCESS;}
	eMCUError_s IAP_eEraseSector(uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank)
	{return MCU_ERROR_SUCCESS;}
	eMCUError_s IAP_eBlankCheckSector(uint32_t wStrSector, uint32_t wEndSector, uint8_t bFlashBank)
	{return MCU_ERROR_SUCCESS;}
	uint32_t 		IAP_wReadPID(){return 0;}
	uint32_t 		IAP_wReadBootCode(){return 0;}
	eMCUError_s IAP_eCompare(uint32_t dstAdd, uint32_t srcAdd, uint32_t bytescmp)
	{return MCU_ERROR_SUCCESS;}
	eMCUError_s IAP_eReinvokeISP(){return MCU_ERROR_SUCCESS;}
	uint32_t 		IAP_wReadUID(){return 0;}
	eMCUError_s IAP_eErasePage(uint32_t strPage, uint32_t endPage){return MCU_ERROR_SUCCESS;}
	eMCUError_s IAP_eSetBootFlashBank(uint8_t bankNum){return MCU_ERROR_SUCCESS;}

	uint32_t IAP_wGetMemoryBank(uint32_t wAbsoluteAddrm)
	{
		/* Checks the bank of the requested adresses */
		if ((wAbsoluteAddrm >= SECTOR_A0_START) && (wAbsoluteAddrm <= SECTOR_A14_END))
		{
			return ONCHIP_SECTOR_A;
		}
		else if ((wAbsoluteAddrm >= SECTOR_B0_START) && (wAbsoluteAddrm <= SECTOR_B14_END))
		{
			return ONCHIP_SECTOR_B;
		}
		else
		{
			/* Address is not on the on-chip flash space */
			return INVALID_VALUE;
		}
	}

	uint32_t IAP_wGetSectorNum(uint32_t wAbsoluteAddrm)
	{
		uint32_t bSector = INVALID_VALUE;
		uint8_t bFlashBank = IAP_wGetMemoryBank(wAbsoluteAddrm);

		/* Checks if the address is valid */
		if (bFlashBank == INVALID_VALUE)
			return INVALID_VALUE;

		/* Valid address, find the corresponding sector number */
		if (bFlashBank == ONCHIP_SECTOR_A)
		{
			for (bSector = 0; bSector < ONCHIP_NUM_SECTORS; bSector++)
			{
				/* Check the index of the sector */
				if ((wAbsoluteAddrm >= wSectorA_StartAddr[bSector]) && \
						(wAbsoluteAddrm <= wSectorA_EndAddr[bSector]))
				{
					break;
				}
			}
		}

		if (bFlashBank == ONCHIP_SECTOR_B)
		{
			for (bSector = 0; bSector < ONCHIP_NUM_SECTORS; bSector++)
			{
				/* Check the index of the sector */
				if ((wAbsoluteAddrm >= wSectorB_StartAddr[bSector]) && \
						(wAbsoluteAddrm <= wSectorB_EndAddr[bSector]))
				{
					break;
				}
			}
		}

		return bSector;
	}

	void END_OF_UNITY_TESTING_FUNCTIONS(void);
#endif

/******************************************************************************
* Function : CHP_eParamValidation(chp_opmode_e bOperation)
*//**
* \b Description:
* This function performs a validation of the sCHP_Manager adresses fields to check if it
* has the necessary and correct info before a read/write operation.
*
* PRE-CONDITION: none
*
* POST-CONDITION: none
*
* @return eDEVError_s code as an error returning code
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* uint32_t wMyData[256];
* sCHP_Manager.eOpMode = CHP_OPMODE_WRITE;
* sCHP_Manager.wpDestAddr = (uint32_t*) FLASH_A_SECTOR1;
* sCHP_Manager.wpStartSector = SECTOR1;
* sCHP_Manager.bMemoryBank = FLASH_BANK_A;
*
* if (CHP_eParamValidation(CHP_OPMODE_WRITE) != DEV_ERROR_SUCCESS)
* 	printf("Error in operation parameters");
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static inline eDEVError_s CHP_eParamValidation(chp_opmode_e bOperation)
{
	if (bOperation == CHP_OPMODE_WRITE)
	{
		if (sCHP_Manager.wpDestAddr == NULL)
		{
			return DEV_ERROR_CHIPPROG_INVALID_ADDR;
		}

		if (sCHP_Manager.wStartSector == CHP_INVALID_SECTOR)
		{
			return DEV_ERROR_CHIPPROG_INVALID_ADDR;
		}

		if (sCHP_Manager.bMemoryBank == CHP_INVALID_BANK)
		{
			return DEV_ERROR_CHIPPROG_INVALID_PARAM;
		}
	}

	else if (bOperation == CHP_OPMODE_ERASE)
	{
		if ((sCHP_Manager.wpStartAddr == NULL)||(sCHP_Manager.wpEndAddr == NULL))
		{
			return DEV_ERROR_CHIPPROG_INVALID_ADDR;
		}

		if ((sCHP_Manager.wStartSector == CHP_INVALID_SECTOR)||(sCHP_Manager.wEndSector == CHP_INVALID_SECTOR))
		{
			return DEV_ERROR_CHIPPROG_INVALID_ADDR;
		}

		if (sCHP_Manager.bMemoryBank == CHP_INVALID_BANK)
		{
			return DEV_ERROR_CHIPPROG_INVALID_PARAM;
		}
	}
	else
	{
		return DEV_ERROR_CHIPPROG_INVALID_PARAM;
	}

	return DEV_ERROR_SUCCESS;
}


/******************************************************************************
* Function : CHP_vResetAddrManager(void)
*//**
* \b Description: This function reset the sCHP_Manager to default values.
*
* PRE-CONDITION: none
*
* POST-CONDITION: sCHP_Manager with default start values
*
* @return none
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* CHP_vResetAddrManager();
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static void CHP_vResetAddrManager(void)
{
	sCHP_Manager.wpStartAddr = NULL;
	sCHP_Manager.wpEndAddr = NULL;
	sCHP_Manager.wStartSector = CHP_INVALID_SECTOR;
	sCHP_Manager.wEndSector = CHP_INVALID_SECTOR;
	sCHP_Manager.bMemoryBank = CHP_INVALID_BANK;
	sCHP_Manager.wpSourceAddr = NULL;
	sCHP_Manager.wpEndAddr = NULL;
	sCHP_Manager.eOpMode = CHP_OPMODE_IDLE;
	sCHP_Manager.bStatus = (uint8_t) MCU_ERROR_SUCCESS;
}

/******************************************************************************
* Function : CHP_eSetStartAddr(uint32_t wRequest, void * vpValue)
*//**
* \b Description: Set the start address of the on-chip flash for an erase operation.
* This function takes an address number as input argument an sets the parameters of
* the sCHP_Manager variable using this number. The function uses the input address to
* obtain the sector number and the flash bank, using the IAP module of the MCULIB layer.
*
* PRE-CONDITION: none
* ATTENTION: This function must not be used directly, use the DEV_ioctl interface.
*
* POST-CONDITION: sCHP_Manager with the following fields updated: wStartSector,
* bMemoryBank, wpStartAddr.
*
* @return eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
uint32_t wStartAddrNumber = 0x12345678;
if (CHP_eSetStartAddr(IOCTL_CHP_START_ADDR, wStartAddrNumber) != DEV_ERROR_SUCCESS)
	printf("invalid address");
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s CHP_eSetStartAddr(uint32_t wRequest, void * vpValue)
{
	/* Check sector number automatically checks if address is valid */
	uint32_t wNum = IAP_wGetSectorNum((uint32_t) vpValue);

	if (wNum == CHP_INVALID_SECTOR)
	{
		return DEV_ERROR_CHIPPROG_INVALID_ADDR;
	}

	sCHP_Manager.wStartSector = wNum;
	sCHP_Manager.bMemoryBank = IAP_wGetMemoryBank((uint32_t) vpValue);
	sCHP_Manager.wpStartAddr = (uint32_t*) vpValue;
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : CHP_eSetStartAddr(uint32_t wRequest, void * vpValue)
*//**
* \b Description: Set the end address of the on-chip flash for an erase operation.
* This function takes an address number as input argument an sets the parameters of
* the sCHP_Manager variable using this number. The function uses the input address to
* obtain the sector number and the flash bank, using the IAP module of the MCULIB layer.
*
* PRE-CONDITION: none
* ATTENTION: This function must not be used directly, use the DEV_ioctl interface.
*
* POST-CONDITION: sCHP_Manager with the following fields updated: wEndSector,
* bMemoryBank, wpEndAddr.
*
* @return eDEVError_s
*
* \b Example
~~~~~~~~~~~~~~~{.c}
uint32_t wEndAddrNumber = 0x12345678;
if (CHP_eSetEndAddr(IOCTL_CHP_END_ADDR, wEndAddrNumber) != DEV_ERROR_SUCCESS)
	printf("Invalid address");
~~~~~~~~~~~~~~~
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s CHP_eSetEndAddr(uint32_t wRequest, void * vpValue)
{
	/* Check sector number automatically checks if address is valid */
	uint32_t wNum = IAP_wGetSectorNum((uint32_t) vpValue);

	if (wNum == CHP_INVALID_SECTOR)
	{
		return DEV_ERROR_CHIPPROG_INVALID_ADDR;
	}

	sCHP_Manager.wEndSector = wNum;
	sCHP_Manager.bMemoryBank = IAP_wGetMemoryBank((uint32_t) vpValue);
	sCHP_Manager.wpEndAddr = (uint32_t*) vpValue;
	return DEV_ERROR_SUCCESS;
}

/******************************************************************************
* Function : CHP_eEraseMemory(uint32_t wRequest, void * vpValue)
*//**
* \b Description: This function triggers an erase operation at the on-chip flash,
* based on the parameters in sCHP_Manager. The affected area will follow the starting
* and ending sector numbers.
*
* PRE-CONDITION: The sCHP_Manager must have the valid start and end address of the affected
* area. This is achieved using IOCTL calls before calling the erase operation IOCTL.
* ATTENTION: This function must not be used directly, use the DEV_ioctl interface.
*
* POST-CONDITION: The requested area will be erased (filled with 0xFF).
*
* @return eDEVError_s error status code
*
* \b Example
~~~~~~~~~~~~~~~{.c}
uint32_t wStart = FLASH_A_SEC_2_START;
uint32_t wEnd = FLASH_A_SEC_10_END;

// Set the start addres / sector
CHP_eSetStartAddr(IOCTL_CHP_START_ADDR, wStart);

// Set the end address
CHP_eSetEndAddr(IOCTL_CHP_END_ADDR, wEnd);

// Triggers the erase operation
if (CHP_eEraseMemory(IOCTL_CHP_ERASE_MEM, NULL) != DEV_ERROR_SUCCESS)
 	 printf("Error while trying to erase flash memory!");

~~~~~~~~~~~~~~~
*
* @see CHP_eSetStartAddr, CHP_eSetEndAddr
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s CHP_eEraseMemory(uint32_t wRequest, void * vpValue)
{
	//(uint32_t)wRequest;
	//(void*)vpValue;

	eDEVError_s eValidation = CHP_eParamValidation(CHP_OPMODE_ERASE);
	if (eValidation)
	{
		return eValidation;
	}

	/* Prepare sectors */
  sCHP_Manager.bStatus = (uint8_t) IAP_ePrepareForReadWrite( sCHP_Manager.wStartSector , \
																														 sCHP_Manager.wEndSector, sCHP_Manager.bMemoryBank);
  if (sCHP_Manager.bStatus)
  	goto exit_point;

  /* Erase sectors */
  sCHP_Manager.bStatus = (uint8_t) IAP_eEraseSector( sCHP_Manager.wStartSector , \
																														 sCHP_Manager.wEndSector, sCHP_Manager.bMemoryBank);
exit_point:
  return (eDEVError_s) sCHP_Manager.bStatus;
}

/******************************************************************************
* Function : CHP_eSetDestAddr(uint32_t wRequest, void * vpValue)
*//**
* \b Description: This function sets the starting address for a writing burst operation
* on the on-chip flash memory.
*
* PRE-CONDITION: none
* ATTENTION: This function must not be used directly, use the DEV_ioctl interface.
*
* POST-CONDITION: sCHP_Manager with the following updated fields: wStartSector, bMemoryBank,
* wpDestAddr
*
* @return eDEVError_s error code
*
* \b Example
~~~~~~~~~~~~~~~{.c}
uint32_t wTargetAddress = 0x12345678;
if (CHP_eSetDestAddr(IOCTL_CHP_DEST_ADDR, wTargetAddress) != DEV_ERROR_SUCCESS)
	printf("Invalid address");
~~~~~~~~~~~~~~~
*
* @see CHP_write
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 17/03/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
static eDEVError_s CHP_eSetDestAddr(uint32_t wRequest, void * vpValue)
{
	/* Check sector number automatically checks if address is valid */
	uint32_t wNum = IAP_wGetSectorNum((uint32_t) vpValue);

	if (wNum == CHP_INVALID_SECTOR)
	{
		return DEV_ERROR_CHIPPROG_INVALID_ADDR;
	}

	sCHP_Manager.wStartSector = wNum;
	sCHP_Manager.bMemoryBank = IAP_wGetMemoryBank((uint32_t) vpValue);
	sCHP_Manager.wpDestAddr = (uint32_t*) vpValue;
	return DEV_ERROR_SUCCESS;
}

/***************** Standard device interface functions ************************/

eDEVError_s CHP_open(void)
{
  /* Prepare the IAP commands */
	CHP_vResetAddrManager();
	return (eDEVError_s) IAP_eInit();
}

uint32_t CHP_read(struct peripheral_descriptor_s* const this,
        void * const vpBuffer,
        const uint32_t tBufferSize)
{
	if (vpBuffer == NULL)
	{
		return 0;
	}

  /* Check if the requested area is blank or not */
  uint32_t wInputParam = (uint32_t) vpBuffer;
  sCHP_Manager.wStartSector = IAP_wGetSectorNum(wInputParam);
  sCHP_Manager.wEndSector = IAP_wGetSectorNum(wInputParam + (tBufferSize-1));
  sCHP_Manager.bMemoryBank = IAP_wGetMemoryBank(wInputParam);

  /* Perform the checking */
  eMCUError_s eResult = IAP_eBlankCheckSector(sCHP_Manager.wStartSector, \
																							sCHP_Manager.wEndSector, sCHP_Manager.bMemoryBank);
  sCHP_Manager.bStatus = (uint8_t) eResult;

  /* Perform the checking */
  if (eResult == MCU_ERROR_SUCCESS)
  {
  	CHP_vResetAddrManager();
  	return tBufferSize; // TRUE - sector(s) are empty
  }
  else
  {
  	return 0; // FALSE - not empty or problem
  }
}

uint32_t CHP_write(struct peripheral_descriptor_s* const this,
      const void * vpBuffer,
      const uint32_t tBufferSize)
{
  if (vpBuffer == NULL)
  {
  	sCHP_Manager.bStatus = (uint8_t) MCU_ERROR_IAP_PARAM_ERROR;
  	return 0;
  }

  /* test page sizes */
  if ((tBufferSize != 0x200) && (tBufferSize != 0x400) && (tBufferSize != 0x1000))
  {
  	sCHP_Manager.bStatus = (uint8_t) MCU_ERROR_IAP_COUNT_ERROR;
  	return 0;
  }

  /* Check mapped address */
  if (CHP_eParamValidation(CHP_OPMODE_WRITE))
  {
  	sCHP_Manager.bStatus = (uint8_t) MCU_ERROR_IAP_DST_ADDR_NOT_MAPPED;
  	return 0;
  }

  /* Set the END sector */
  sCHP_Manager.wEndSector = IAP_wGetSectorNum( (uint32_t)sCHP_Manager.wpDestAddr + (tBufferSize-1));

  /* Prepare the respective sectors for writing */
  sCHP_Manager.bStatus = (uint8_t) IAP_ePrepareForReadWrite( sCHP_Manager.wStartSector , \
																														 sCHP_Manager.wEndSector, sCHP_Manager.bMemoryBank);
  if (sCHP_Manager.bStatus != MCU_ERROR_SUCCESS)
  {
  	return 0;
  }

  /* Write flash memory */
  sCHP_Manager.bStatus = (uint8_t) IAP_eCopyRamToFlash((uint32_t) sCHP_Manager.wpDestAddr, \
																											 (uint32_t*) vpBuffer, tBufferSize);
  if (sCHP_Manager.bStatus != MCU_ERROR_SUCCESS)
  {
  	return 0;
  }

  /* Successful operation */
  CHP_vResetAddrManager();
  return tBufferSize;
}

eDEVError_s CHP_ioctl(struct peripheral_descriptor_s* const this,
      uint32_t wRequest,
      void * vpValue)
{
  (void)this;

  /* Test if param and request are valid */
  if ((wRequest >= IOCTL_CHP_INVALID) || (vpValue == NULL))
  {
    return DEV_ERROR_INVALID_IOCTL;
  }

  /* Call the request related function */
  return CHP_pIOCTLFunction[wRequest](wRequest, vpValue);
}

eDEVError_s CHP_close(struct peripheral_descriptor_s* const this)
{
  (void)this;
  CHP_vResetAddrManager();
  return DEV_ERROR_SUCCESS;
}


/******************************************************************************
* Unity Testing
*******************************************************************************/

#if defined(UNITY_TEST)

TEST_GROUP(CHIPPROGDeviceTest);

TEST_SETUP(CHIPPROGDeviceTest)
{
	CHP_vResetAddrManager();
}

TEST_TEAR_DOWN(CHIPPROGDeviceTest)
{
	CHP_vResetAddrManager();
}

TEST(CHIPPROGDeviceTest, test_Verify_IAP_wGetMemoryBank)
{
	// Test using pre-defined sector areas
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_A, IAP_wGetMemoryBank(SECTOR_A0_START));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_A, IAP_wGetMemoryBank(SECTOR_A0_END));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_A, IAP_wGetMemoryBank(SECTOR_A1_START));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_A, IAP_wGetMemoryBank(SECTOR_A1_END));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_B, IAP_wGetMemoryBank(SECTOR_B0_START));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_B, IAP_wGetMemoryBank(SECTOR_B0_END));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_B, IAP_wGetMemoryBank(SECTOR_B1_START));
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_B, IAP_wGetMemoryBank(SECTOR_B1_END));

	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetMemoryBank(SECTOR_A0_START-1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetMemoryBank(SECTOR_A14_END+1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetMemoryBank(SECTOR_B0_START-1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetMemoryBank(SECTOR_B14_END+1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetMemoryBank(0));
}

TEST(CHIPPROGDeviceTest, test_Verify_IAP_wGetSectorNum)
{
	TEST_ASSERT_EQUAL(0, IAP_wGetSectorNum(SECTOR_A0_START));
	TEST_ASSERT_EQUAL(0, IAP_wGetSectorNum(SECTOR_A0_END));
	TEST_ASSERT_EQUAL(1, IAP_wGetSectorNum(SECTOR_A1_START));
	TEST_ASSERT_EQUAL(1, IAP_wGetSectorNum(SECTOR_A1_END));
	TEST_ASSERT_EQUAL(2, IAP_wGetSectorNum(SECTOR_A2_START));
	TEST_ASSERT_EQUAL(2, IAP_wGetSectorNum(SECTOR_A2_END));
	TEST_ASSERT_EQUAL(3, IAP_wGetSectorNum(SECTOR_A3_START));
	TEST_ASSERT_EQUAL(3, IAP_wGetSectorNum(SECTOR_A3_END));
	TEST_ASSERT_EQUAL(4, IAP_wGetSectorNum(SECTOR_A4_START));
	TEST_ASSERT_EQUAL(4, IAP_wGetSectorNum(SECTOR_A4_END));
	TEST_ASSERT_EQUAL(5, IAP_wGetSectorNum(SECTOR_A5_START));
	TEST_ASSERT_EQUAL(5, IAP_wGetSectorNum(SECTOR_A5_END));
	TEST_ASSERT_EQUAL(6, IAP_wGetSectorNum(SECTOR_A6_START));
	TEST_ASSERT_EQUAL(6, IAP_wGetSectorNum(SECTOR_A6_END));
	TEST_ASSERT_EQUAL(7, IAP_wGetSectorNum(SECTOR_A7_START));
	TEST_ASSERT_EQUAL(7, IAP_wGetSectorNum(SECTOR_A7_END));
	TEST_ASSERT_EQUAL(8, IAP_wGetSectorNum(SECTOR_A8_START));
	TEST_ASSERT_EQUAL(8, IAP_wGetSectorNum(SECTOR_A8_END));
	TEST_ASSERT_EQUAL(9, IAP_wGetSectorNum(SECTOR_A9_START));
	TEST_ASSERT_EQUAL(9, IAP_wGetSectorNum(SECTOR_A9_END));
	TEST_ASSERT_EQUAL(10, IAP_wGetSectorNum(SECTOR_A10_START));
	TEST_ASSERT_EQUAL(10, IAP_wGetSectorNum(SECTOR_A10_END));
	TEST_ASSERT_EQUAL(11, IAP_wGetSectorNum(SECTOR_A11_START));
	TEST_ASSERT_EQUAL(11, IAP_wGetSectorNum(SECTOR_A11_END));
	TEST_ASSERT_EQUAL(12, IAP_wGetSectorNum(SECTOR_A12_START));
	TEST_ASSERT_EQUAL(12, IAP_wGetSectorNum(SECTOR_A12_END));
	TEST_ASSERT_EQUAL(13, IAP_wGetSectorNum(SECTOR_A13_START));
	TEST_ASSERT_EQUAL(13, IAP_wGetSectorNum(SECTOR_A13_END));
	TEST_ASSERT_EQUAL(14, IAP_wGetSectorNum(SECTOR_A14_START));
	TEST_ASSERT_EQUAL(14, IAP_wGetSectorNum(SECTOR_A14_END));

	TEST_ASSERT_EQUAL(0, IAP_wGetSectorNum(SECTOR_B0_START));
	TEST_ASSERT_EQUAL(0, IAP_wGetSectorNum(SECTOR_B0_END));
	TEST_ASSERT_EQUAL(1, IAP_wGetSectorNum(SECTOR_B1_START));
	TEST_ASSERT_EQUAL(1, IAP_wGetSectorNum(SECTOR_B1_END));
	TEST_ASSERT_EQUAL(2, IAP_wGetSectorNum(SECTOR_B2_START));
	TEST_ASSERT_EQUAL(2, IAP_wGetSectorNum(SECTOR_B2_END));
	TEST_ASSERT_EQUAL(3, IAP_wGetSectorNum(SECTOR_B3_START));
	TEST_ASSERT_EQUAL(3, IAP_wGetSectorNum(SECTOR_B3_END));
	TEST_ASSERT_EQUAL(4, IAP_wGetSectorNum(SECTOR_B4_START));
	TEST_ASSERT_EQUAL(4, IAP_wGetSectorNum(SECTOR_B4_END));
	TEST_ASSERT_EQUAL(5, IAP_wGetSectorNum(SECTOR_B5_START));
	TEST_ASSERT_EQUAL(5, IAP_wGetSectorNum(SECTOR_B5_END));
	TEST_ASSERT_EQUAL(6, IAP_wGetSectorNum(SECTOR_B6_START));
	TEST_ASSERT_EQUAL(6, IAP_wGetSectorNum(SECTOR_B6_END));
	TEST_ASSERT_EQUAL(7, IAP_wGetSectorNum(SECTOR_B7_START));
	TEST_ASSERT_EQUAL(7, IAP_wGetSectorNum(SECTOR_B7_END));
	TEST_ASSERT_EQUAL(8, IAP_wGetSectorNum(SECTOR_B8_START));
	TEST_ASSERT_EQUAL(8, IAP_wGetSectorNum(SECTOR_B8_END));
	TEST_ASSERT_EQUAL(9, IAP_wGetSectorNum(SECTOR_B9_START));
	TEST_ASSERT_EQUAL(9, IAP_wGetSectorNum(SECTOR_B9_END));
	TEST_ASSERT_EQUAL(10, IAP_wGetSectorNum(SECTOR_B10_START));
	TEST_ASSERT_EQUAL(10, IAP_wGetSectorNum(SECTOR_B10_END));
	TEST_ASSERT_EQUAL(11, IAP_wGetSectorNum(SECTOR_B11_START));
	TEST_ASSERT_EQUAL(11, IAP_wGetSectorNum(SECTOR_B11_END));
	TEST_ASSERT_EQUAL(12, IAP_wGetSectorNum(SECTOR_B12_START));
	TEST_ASSERT_EQUAL(12, IAP_wGetSectorNum(SECTOR_B12_END));
	TEST_ASSERT_EQUAL(13, IAP_wGetSectorNum(SECTOR_B13_START));
	TEST_ASSERT_EQUAL(13, IAP_wGetSectorNum(SECTOR_B13_END));
	TEST_ASSERT_EQUAL(14, IAP_wGetSectorNum(SECTOR_B14_START));
	TEST_ASSERT_EQUAL(14, IAP_wGetSectorNum(SECTOR_B14_END));

	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetSectorNum(SECTOR_A0_START-1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetSectorNum(SECTOR_A14_END+1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetSectorNum(SECTOR_B0_START-1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetSectorNum(SECTOR_B14_END+1));
	TEST_ASSERT_EQUAL(INVALID_VALUE, IAP_wGetSectorNum(0));
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_eParamValidation)
{
	sCHP_Manager.wpDestAddr = NULL;
	sCHP_Manager.wStartSector = 0;
	sCHP_Manager.bMemoryBank = 0;
	TEST_ASSERT_EQUAL(DEV_ERROR_CHIPPROG_INVALID_ADDR, \
										CHP_eParamValidation(CHP_OPMODE_WRITE));

	sCHP_Manager.wpDestAddr = (uint32_t*) 0x1a000000;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, \
										CHP_eParamValidation(CHP_OPMODE_WRITE));

	TEST_ASSERT_EQUAL(DEV_ERROR_CHIPPROG_INVALID_PARAM, \
										CHP_eParamValidation(CHP_OPMODE_IDLE));

	CHP_vResetAddrManager();
	TEST_ASSERT_EQUAL(DEV_ERROR_CHIPPROG_INVALID_ADDR, \
										CHP_eParamValidation(CHP_OPMODE_ERASE));

	sCHP_Manager.wpStartAddr = (uint32_t*) 0x1a000000;
	sCHP_Manager.wpEndAddr = (uint32_t*) 0x1a003fff;
	sCHP_Manager.wStartSector = 0;
	sCHP_Manager.wEndSector = 0;
	sCHP_Manager.bMemoryBank = 1;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, \
										CHP_eParamValidation(CHP_OPMODE_ERASE));
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_eSetStartAddr)
{
	uint32_t *wAddr = (uint32_t*) (SECTOR_A0_START-1);
	TEST_ASSERT_EQUAL(DEV_ERROR_CHIPPROG_INVALID_ADDR, CHP_eSetStartAddr(0, (void*) wAddr));

	wAddr = (uint32_t*) SECTOR_A7_START;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CHP_eSetStartAddr(0, (void*) wAddr));
	TEST_ASSERT_EQUAL(7, sCHP_Manager.wStartSector);
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_A, sCHP_Manager.bMemoryBank);
	TEST_ASSERT_EQUAL_PTR(wAddr, sCHP_Manager.wpStartAddr);
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_eSetEndAddr)
{
	uint32_t *wAddr = (uint32_t*) (SECTOR_A14_END+1);
	TEST_ASSERT_EQUAL(DEV_ERROR_CHIPPROG_INVALID_ADDR, CHP_eSetEndAddr(0, (void*) wAddr));

	wAddr = (uint32_t*) (SECTOR_A13_START + 0x200);
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CHP_eSetEndAddr(0, (void*) wAddr));
	TEST_ASSERT_EQUAL(13, sCHP_Manager.wEndSector);
	TEST_ASSERT_EQUAL(ONCHIP_SECTOR_A, sCHP_Manager.bMemoryBank);
	TEST_ASSERT_EQUAL_PTR(wAddr, sCHP_Manager.wpEndAddr);
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_eSetDestAddr)
{
	uint32_t *wAddr = (uint32_t*) (SECTOR_B14_END+1);
	TEST_ASSERT_EQUAL(DEV_ERROR_CHIPPROG_INVALID_ADDR, CHP_eSetDestAddr(0, (void*) wAddr));

	wAddr = (uint32_t*) SECTOR_B12_START;
	TEST_ASSERT_EQUAL(DEV_ERROR_SUCCESS, CHP_eSetDestAddr(0, (void*) wAddr));
	TEST_ASSERT_EQUAL_PTR(wAddr, sCHP_Manager.wpDestAddr);
	TEST_ASSERT_EQUAL_PTR(NULL, sCHP_Manager.wpSourceAddr);
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_eEraseMemory)
{
	TEST_ASSERT_NOT_EQUAL(DEV_ERROR_SUCCESS, CHP_eEraseMemory(0, NULL));

	/* Set only start addr */
	CHP_eSetStartAddr(0, (void*) SECTOR_A10_START);
	TEST_ASSERT_NOT_EQUAL(DEV_ERROR_SUCCESS, CHP_eEraseMemory(0, NULL));

	/* Set start and end addr */
	CHP_eSetEndAddr(0, (void*) SECTOR_A11_END);
	TEST_ASSERT_EQUAL(MCU_ERROR_SUCCESS, CHP_eEraseMemory(0, NULL));
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_read)
{
	/* No parameters previously prepared */
	TEST_ASSERT_EQUAL(0, CHP_read(NULL, NULL, (4*0x2000)));

	/* Set correct address */
	TEST_ASSERT_EQUAL((4*0x2000), CHP_read(NULL, (void*) SECTOR_A0_START, (4*0x2000)));
}

TEST(CHIPPROGDeviceTest, test_Verify_CHP_write)
{
	/* No parameters previously prepared */
	TEST_ASSERT_EQUAL(0, CHP_write(NULL, NULL, (4*0x2000)));

	/* Wrong data size */
	TEST_ASSERT_EQUAL(0, CHP_write(NULL, (void*) SECTOR_B0_START, (4*0x2000)));

	/* Destination address not set */
	TEST_ASSERT_EQUAL(0, CHP_write(NULL, (void*) SECTOR_B0_START, 0x400));

	/* Set dest address */
	CHP_eSetDestAddr(0, (void*) SECTOR_A0_START);
	TEST_ASSERT_EQUAL(0x400, CHP_write(NULL, (void*) SECTOR_B0_START, 0x400));
}

#endif
