/****************************************************************************
 * Title                 :   M2GGPSCOMM
 * Filename              :   M2GGPSCOMM.h
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
 *  XX/XX/XX   1.0.0    Henrique Reis    M2GGPSCOMM.h created.
 *
 *****************************************************************************/
/** @file   M2GGPSCOMM.h
 *  @brief
 *
 */
#ifndef INC_M2GGPSCOMM_H_
#define INC_M2GGPSCOMM_H_

/* Includes */
#include <inttypes.h>
#include "devicelib.h"

/* Preprocessor constants */

/* Configuration constants */

/* Typedefs */
typedef void (*M2GGPSCOMM_Callback) (uint8_t *bpInputBuffer, uint32_t dSize); /*!< M2GGPSCOMM receive callback typedef */

/* Function prototypes */
eDEVError_s M2GGPS_open (void);
uint32_t M2GGPS_read (struct peripheral_descriptor_s* const this, void * const vpBuffer, const uint32_t tBufferSize);
uint32_t M2GGPS_write (struct peripheral_descriptor_s* const this, const void * vpBuffer, const uint32_t tBufferSize);
eDEVError_s M2GGPS_ioctl (struct peripheral_descriptor_s* const this, uint32_t wRequest, void * vpValue);
eDEVError_s M2GGPS_close (struct peripheral_descriptor_s* const this);

#ifdef __cplusplus
extern "C"
{
#endif

#endif /* INC_M2GGPSCOMM_H_ */
