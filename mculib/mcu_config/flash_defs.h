/****************************************************************************
* Title                 :   Internal Flash Memory definitions
* Filename              :   flash_defs.h
* Author                :		Joao Paulo Martins
* Origin Date           :		06/05/2016
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
*    Date    			Version   			Author         		Description
*	 06/05/2016		flash_defs.h	Joao Paulo Martins	File creation
*
*****************************************************************************/
/** @file flash_defs.h
 *  @brief This file provides macros for the internal flash memory of the
 *  microcontroller, configuring the address of the sectors.
 *
 */

#ifndef MCULIB_MCU_CONFIG_FLASH_DEFS_H_
#define MCULIB_MCU_CONFIG_FLASH_DEFS_H_

/******************************************************************************
* Includes
*******************************************************************************/
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
/******************************************************************************
* Configuration Constants
*******************************************************************************/
#if defined (TEST_BOARD) || defined (TDA) || defined (M2G_PROTOB) || defined (EA_LPC4357)
	#define LPC4357_FLASH
#endif

//#define CRP1  0x12345678
//#define CRP2  0x87654321
//#define CRP3  0x43218765
//#define NOCRP 0x11223344
//#define CRP 	NOCRP

/******************************************************************************
* Macros
*******************************************************************************/

/* Microcontroller specific flash sectors address*/

#ifdef LPC4357_FLASH
	/* Bank A - 8 kB sectors */
	#define SECTOR_A0_START	 0x1A000000
	#define SECTOR_A1_START  0x1A002000
	#define SECTOR_A2_START  0x1A004000
	#define SECTOR_A3_START  0x1A006000
	#define SECTOR_A4_START  0x1A008000
	#define SECTOR_A5_START  0x1A00A000
	#define SECTOR_A6_START  0x1A00C000
	#define SECTOR_A7_START  0x1A00E000
	#define SECTOR_A0_END	 	 0x1A001FFF
	#define SECTOR_A1_END    0x1A003FFF
	#define SECTOR_A2_END    0x1A005FFF
	#define SECTOR_A3_END    0x1A007FFF
	#define SECTOR_A4_END    0x1A009FFF
	#define SECTOR_A5_END    0x1A00BFFF
	#define SECTOR_A6_END    0x1A00DFFF
	#define SECTOR_A7_END  	 0x1A00FFFF

	/* Bank A - 64 kB sectors */
	#define SECTOR_A8_START	 0x1A010000
	#define SECTOR_A9_START  0x1A020000
	#define SECTOR_A10_START 0x1A030000
	#define SECTOR_A11_START 0x1A040000
	#define SECTOR_A12_START 0x1A050000
	#define SECTOR_A13_START 0x1A060000
	#define SECTOR_A14_START 0x1A070000
	#define SECTOR_A8_END	 	 0x1A01FFFF
	#define SECTOR_A9_END    0x1A02FFFF
	#define SECTOR_A10_END   0x1A03FFFF
	#define SECTOR_A11_END   0x1A04FFFF
	#define SECTOR_A12_END   0x1A05FFFF
	#define SECTOR_A13_END   0x1A06FFFF
	#define SECTOR_A14_END   0x1A07FFFF

  /* Bank B - 8 kB sectors */
	#define SECTOR_B0_START	 0x1B000000
	#define SECTOR_B1_START  0x1B002000
	#define SECTOR_B2_START  0x1B004000
	#define SECTOR_B3_START  0x1B006000
	#define SECTOR_B4_START  0x1B008000
	#define SECTOR_B5_START  0x1B00A000
	#define SECTOR_B6_START  0x1B00C000
	#define SECTOR_B7_START  0x1B00E000
	#define SECTOR_B0_END	 	 0x1B001FFF
	#define SECTOR_B1_END  	 0x1B003FFF
	#define SECTOR_B2_END  	 0x1B005FFF
	#define SECTOR_B3_END  	 0x1B007FFF
	#define SECTOR_B4_END  	 0x1B009FFF
	#define SECTOR_B5_END  	 0x1B00BFFF
	#define SECTOR_B6_END  	 0x1B00DFFF
	#define SECTOR_B7_END  	 0x1B00FFFF

	/* Bank B - 64 kB sectors */
	#define SECTOR_B8_START	 0x1B010000
	#define SECTOR_B9_START  0x1B020000
	#define SECTOR_B10_START 0x1B030000
	#define SECTOR_B11_START 0x1B040000
	#define SECTOR_B12_START 0x1B050000
	#define SECTOR_B13_START 0x1B060000
	#define SECTOR_B14_START 0x1B070000
	#define SECTOR_B8_END	 	 0x1B01FFFF
	#define SECTOR_B9_END  	 0x1B02FFFF
	#define SECTOR_B10_END 	 0x1B03FFFF
	#define SECTOR_B11_END 	 0x1B04FFFF
	#define SECTOR_B12_END 	 0x1B05FFFF
	#define SECTOR_B13_END 	 0x1B06FFFF
	#define SECTOR_B14_END 	 0x1B07FFFF
#endif

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
extern "C"{
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* MCULIB_MCU_CONFIG_FLASH_DEFS_H_ */
