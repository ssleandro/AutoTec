/****************************************************************************
 * Title                 :   isobus_tools
 * Filename              :   isobus_tools.h
 * Author                :   Henrique Reis
 * Origin Date           :   06/06/2017
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
 * 06/06/2017  1.0.0     Henrique Reis      isobus_tools.h created.
 *
 *****************************************************************************/
#ifndef APP_ISOBUS_SRC_ISOBUS_TOOLS_H_
#define APP_ISOBUS_SRC_ISOBUS_TOOLS_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include "isobus_core.h"
#include "iso11783.h"

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
* Public Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
void ISO_vUpdateConfigurationDataMask (void);
void ISO_vUpdateInstallationDataMask (void);
void ISO_vUpdatePlanterDataMask (void);
void ISO_vUpdateTestModeDataMask (event_e);
void ISO_vUpdateTrimmingDataMask (void);
void ISO_vUpdateSystemDataMask (void);
void ISO_vChangeActiveMask(eIsobusMask eNewMask);

void ISO_vIsobusUpdateVTStatus (ISOBUSMsg*);

#endif /* APP_ISOBUS_SRC_ISOBUS_TOOLS_H_ */
