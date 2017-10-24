/****************************************************************************
 * Title                 :   M2GSENSORCOMM_config
 * Filename              :   M2GSENSORCOMM_config.h
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
 *    Date    Version        Author          Description
 *  xx/xx/xx   1.0.0    Henrique Reis    M2GSENSORCOMM_config.h created.
 *
 *****************************************************************************/
/** @file   M2GSENSORCOMM_config.h
 *  @brief
 *
 */
#ifndef COMMON_M2GSENSORCOMM_CONFIG_H_
#define COMMON_M2GSENSORCOMM_CONFIG_H_

/* Includes */
#ifndef UNITY_TEST
#include "mculib.h"
#else
#endif

/* Configuration Constants */
/* Initial configurations for the devices */
// For M2GSENSORCOMM CAN is the only one way to communication
#define M2GSENSORCOMM_IFACE_CAN             0x01			   /*!< M2GSENSORCOMM messages over CAN interface */

#define M2GSENSORCOMM_RESERVED_INTERFACES 	M2GSENSORCOMM_IFACE_CAN	   /*!< Init the CAN peripheral a default */
#define M2GSENSORCOMM_ACTIVE_INTERFACE      M2GSENSORCOMM_IFACE_CAN	   /*!< Activate CAN peripheral a default */

#define M2GSENSORCOMM_CAN_CHANNEL	        CAN0			   /*!< CAN channel to be used by the device */
#define M2GSENSORCOMM_CAN_BITRATE	        CAN_BITRATE_250		   /*!< Standard baudrate to be used in CAN */

#define M2GSENSORCOMM_BUF_SIZE	            250     		   /*!< Fixed receive buffer size in bytes */

/* Macros */
/**
 * This private MACRO holds all the mapping between IOCTL commands and functions
 */
#define IOCTL_M2GSENSORCOMM \
    X(IOCTL_M2GSENSORCOMM_SET_ACTIVE, M2GSEN_eSetActive) \
    X(IOCTL_M2GSENSORCOMM_DISABLE, M2GSEN_eDisable) \
    X(IOCTL_M2GSENSORCOMM_ADD_CAN_ID, M2GSEN_eCANAddID) \
    X(IOCTL_M2GSENSORCOMM_ADD_ALL_CAN_ID, M2GSEN_eCANAddAllID) \
    X(IOCTL_M2GSENSORCOMM_CHANGE_SEND_ID, M2GSEN_eCANChangeSendID) \
	X(IOCTL_M2GSENSORCOMM_GET_STATUS, M2GSEN_eCANGetStatus) \
	X(IOCTL_M2GSENSORCOMM_CLEAR_STATUS, M2GSEN_eCANClearStatus) \
    X(IOCTL_M2GSENSORCOMM_INVALID, NULL) \
                
/**
 * This Typedef holds all possible IOCTL Requests for termdev
 */
#define X(a, b) a,
typedef enum IOCTL_M2GSENSORCOMM_REQUEST_e
{
	IOCTL_M2GSENSORCOMM
} IOCTL_M2GSENSORCOMM_REQUEST_e;
#undef X

#endif /* COMMON_M2GSENSORCOMM_CONFIG_H_ */
