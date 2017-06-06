/****************************************************************************
 * Title                 :   simulator_config
 * Filename              :   simulator_config.h
 * Author                :   Henrique Reis
 * Origin Date           :   19 de abr de 2017
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
 *  19/04/17   1.0.0     Henrique Reis         simulator_config.h created.
 *
 *****************************************************************************/
#ifndef SIMULATOR_CONFIG_SIMULATOR_CONFIG_H_
#define SIMULATOR_CONFIG_SIMULATOR_CONFIG_H_

/******************************************************************************
 * Includes
 *******************************************************************************/

/******************************************************************************
 * Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Configuration Constants
 *******************************************************************************/
//Main loop wait
#define SIMULATOR_QUEUE_WAIT   0           //!< No timeout - wait forever
#define SIMULATOR_DEFAULT_MSGSIZE  3       //!< Default output format - 16 bytes

/******************************************************************************
 * Macros
 *******************************************************************************/

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SIMULATOR_CONFIG_SIMULATOR_CONFIG_H_ */
