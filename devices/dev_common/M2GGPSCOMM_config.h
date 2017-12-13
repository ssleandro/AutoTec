/****************************************************************************
 * Title                 :   M2GGPSCOMM_config
 * Filename              :   M2GGPSCOMM_config.h
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
 *  xx/xx/xx   1.0.0    Henrique Reis    M2GGPSCOMM_config.h created.
 *
 *****************************************************************************/
/** @file   M2GGPSCOMM_config.h
 *  @brief
 *
 */
#ifndef COMMON_M2GGPSCOMM_CONFIG_H_
#define COMMON_M2GGPSCOMM_CONFIG_H_

#ifndef UNITY_TEST
#include "mculib.h"
#else
#endif

/*
 * Configuration Constants
 * Initial configuration for the device
 * For M2GGPSCOMM UART is the only one way to communication
 */
#define M2GGPSCOMM_UART_CHANNEL         UART2                   /*!< UART channel to be used by the device */
#define M2GGPSCOMM_UART_BAUD            UART_B9600              /*!< Standard baudrate to be used in UART */

#define M2GGPSCOMM_RX_FIFO_BUF_SIZE		 16
#define M2GGPSCOMM_BUF_SIZE             512                     /*!< Fixed receive buffer size in bytes */

/*
 * Macros
 */
/* This private MACRO holds all the mapping between IOCTL commands and functions */
#define IOCTL_M2GGPSCOMM \
    X(IOCTL_M2GGPSCOMM_CHANGE_INT_STATUS, M2GGPS_eChangeIntStatus) \
    X(IOCTL_M2GGPSCOMM_CHANGE_BAUD_RATE, M2GGPS_eChangeBaudRate) \
    X(IOCTL_M2GGPSCOMM_INVALID, NULL) \

/* This typedef holds all possible IOCTL requests for M2GGPSCOMM */
#define X(a, b) a,
typedef enum IOCTL_M2GGPSCOMM_REQUEST_e
{
	IOCTL_M2GGPSCOMM
} IOCTL_M2GGPSCOMM_REQUEST_e;
#undef X

#endif /* COMMON_M2GGPSCOMM_CONFIG_H_ */
