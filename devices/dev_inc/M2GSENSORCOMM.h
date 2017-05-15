/****************************************************************************
* Title                 :   M2GSENSORCOMM
* Filename              :   M2GSENSORCOMM.h
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
*    Date    Version       Author          Description
*  XX/XX/XX   1.0.0    Henrique Reis    M2GSENSORCOMM.h created.
*
*****************************************************************************/
/** @file   M2GSENSORCOMM.h
*  @brief
*
*/
#ifndef INC_M2GSENSORCOMM_H_
#define INC_M2GSENSORCOMM_H_

/* Includes */
#include <inttypes.h>
#include "devicelib.h"

/* Preprocessor Constants */

/* Configuration Constants */

/* Typedefs */
typedef enum M2GSENSORCOMM_interface_e
{
    M2GSENSORCOMM_CAN	        =	0x01,	/*!< M2GSENSORCOMM messages over CAN */
    M2GSENSORCOMM_INVALID       =   0x02,	/*!< M2GSENSORCOMM invalid interface */
} M2GSENSORCOMM_interface_e;

typedef enum M2GSENSORCOMM_pchannel_e
{
    M2GSENSORCOMM_CHANNEL0,			/*!< Individual channel 0 for selected peripheral */
    M2GSENSORCOMM_CHANNEL1,	       		/*!< Individual channel 1 for selected peripheral */
    M2GSENSORCOMM_INVALID_CHANNEL,              /*!< Invalid channel */
} M2GSENSORCOMM_pchannel_e;

typedef void (*M2GSENSORCOMM_Callback)(uint8_t *bpInputBuffer, uint32_t dSize);	/*!< M2GSENSORCOMM receive callback typedef */

typedef struct M2GSENSORCOMM_Handle_s
{
    uint32_t					dReservedInterfaces;	/*!< Interfaces reserved for use */
    M2GSENSORCOMM_interface_e	eActiveInterface;		/*!< Active input/output peripheral (For this device only CAN is available) */
    uint32_t					bDeviceStatus;			/*!< Device healtb status */
    M2GSENSORCOMM_Callback		fpCallback;				/*!< Callback function to receive messages */
} M2GSENSORCOMM_Handle_s;

/* Function Prototypes */
eDEVError_s M2GSEN_open(void);
uint32_t M2GSEN_read(struct peripheral_descriptor_s* const this, void * const vpBuffer, const uint32_t tBufferSize);
uint32_t M2GSEN_write(struct peripheral_descriptor_s* const this, const void * vpBuffer, const uint32_t tBufferSize);
eDEVError_s M2GSEN_ioctl(struct peripheral_descriptor_s* const this, uint32_t wRequest, void * vpValue);
eDEVError_s M2GSEN_close(struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
extern "C"{
#endif
    
#endif /* INC_M2GSENSORCOMM_H_ */
