/****************************************************************************
* Title                 :   MCU ACCESS FILE
* Filename              :   mcu.h
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
/** @file mcu.h
 *  @brief This module provides a MCU data access for the upper layers
 *
 */


#ifndef ABS_INC_MCU_H_
#define ABS_INC_MCU_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
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

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

  /******************************************************************************
  * Function : MCU_vSysInit(void)
  *//**
  * \b Description:
  *
  * This is a public function used to initialize the Hardware layer.
  * This function must be the first thing called after Main.
  *
  * PRE-CONDITION: Must be called inside Main.
  *
  * POST-CONDITION: Hardware layer initialized
  *
  * @return     void
  *
  * \b Example
  ~~~~~~~~~~~~~~~{.c}
  * int main(void)
  * {
  *   MCU_vSysInit(); //Initialize system
  *
  * }
  ~~~~~~~~~~~~~~~
  *
  * @see MCU_vSysInit
  *
  * <br><b> - HISTORY OF CHANGES - </b>
  *
  * <table align="left" style="width:800px">
  * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
  * <tr><td> 18/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
  * </table><br><br>
  * <hr>
  *
  *******************************************************************************/
void MCU_vSysInit(void);

void MCU_vSDRAMInit(void);

uint32_t MCU_wReadBootCode(void);

uint32_t MCU_wReadPID(void);

void MCU_vFFSInit(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ABS_INC_MCU_H_ */
