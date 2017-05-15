/****************************************************************************
* Title                 :   deviceerror Include File
* Filename              :   deviceerror.h
* Author                :   thiago.palmieri
* Origin Date           :   15 de mar de 2016
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
*  15 de mar de 2016   1.0.0   thiago.palmieri deviceerror include file Created.
*
*****************************************************************************/
/** @file deviceerror.h
 *  @brief This file provides Error codes for device layer
 *
 */
#ifndef INC_DEVICEERROR_H_
#define INC_DEVICEERROR_H_

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
/**
 * This Typedef holds all Device Error codes
 */
typedef enum eDEVError_s
{
  DEV_ERROR_SUCCESS                    = 0,   //!< SUCCESS
  DEV_ERROR_INVALID_DESCRIPTOR         = 500, //!< Invalid Descriptor used
  DEV_ERROR_INVALID_IOCTL                   , //!< Invalid IOCTL command
  DEV_ERROR_INVALID_PERIPHERAL              , //!< Invalid Peripheral
  DEV_ERROR_MLX_INVALID_RESULT_TYPE         , //!< Invalid MLX Result Type
  DEV_ERROR_TERMDEV_PERIPHERAL_INIT_ERR     , //!< Peripheral Initialization Error
  DEV_ERROR_TERMDEV_SENDING_ERROR           , //!< TX Error
  DEV_ERROR_TERMDEV_SETTING_ERROR           , //!< Configuration Error
  DEV_ERROR_M2GGPSCOMM_SETTING_ERROR        , //!< Configuration Error
  DEV_ERROR_WDT_ALREADY_STARTED             , //!< WDT already started
  DEV_ERROR_CHIPPROG_INVALID_ADDR           ,
  DEV_ERROR_CHIPPROG_INVALID_PARAM          ,
  DEV_ERROR_LAST_ERROR                      , //!< LAST ERROR
}eDEVError_s;

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

#endif /* INC_DEVICEERROR_H_ */

/*** End of File **************************************************************/
