/****************************************************************************
 * Title                 :   debug_tool Include File
 * Filename              :   debug_tool.c
 * Author                :   thiago.palmieri
 * Origin Date           :   24 de mar de 2016
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
 *  24 de mar de 2016   1.0.0   thiago.palmieri debug_tool include file Created.
 *
 *****************************************************************************/
/** @file debug_tool.c
 *  @brief This file provides Debug Functions.
 *
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "debug_tool.h"
#include "M2G_app.h"
#include <stdarg.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/
#define MAXIMUM_PRINTF_ARRAY_SIZE 256
/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/
#if defined (UNITY_TEST)
uint32_t osCheckFreeStack(void * test)
{
	(void)test;
	return 0;
}
#endif
/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void TOL_vItoa (char **buf, unsigned int d, int base);
static int TOL_wFormatstring (char *buf, const char *fmt, va_list va);
static int TOL_wFormatlength (const char *fmt, va_list va);

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

/******************************************************************************
 * Function : TOL_vItoa(char **buf, unsigned int d, int base)
 *//**
 * \b Description:
 *
 * This private function is used to Convert integer to ascii
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * uint8_t val = 64;
 * TOL_vItoa(&buf, val, 10);
 ~~~~~~~~~~~~~~~
 *
 * @see TOL_wPrintf, TOL_vItoa, TOL_wFormatstring, TOL_wFormatlength, TOL_Fwrite
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static void TOL_vItoa (char **buf, unsigned int d, int base)
{
	int div = 1;
	while (d / div >= base)
		div *= base;

	while (div != 0)
	{
		int num = d / div;
		d = d % div;
		div /= base;
		if (num > 9)
			*((*buf)++) = (num - 10) + 'A';
		else
			*((*buf)++) = num + '0';
	}
}

/******************************************************************************
 * Function : TOL_wFormatstring(char *buf, const char *fmt, va_list va)
 *//**
 * \b Description:
 *
 * This private function Writes arguments va to buffer buf according to format fmt
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     Length of string
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //to be used only by printf function
 ~~~~~~~~~~~~~~~
 *
 * @see TOL_wPrintf, TOL_vItoa, TOL_wFormatstring, TOL_wFormatlength, TOL_Fwrite
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static int TOL_wFormatstring (char *buf, const char *fmt, va_list va)
{
	char *start_buf = buf;
	while (*fmt)
	{
		/* Character needs formating? */
		if (*fmt == '%')
		{
			switch (*(++fmt))
			{
				case 'c':
				*buf++ = va_arg(va, int);
					break;
				case 'd':
				case 'i':
				{
					signed int val = va_arg(va, signed int);
					if (val < 0)
					{
						val *= -1;
						*buf++ = '-';
					}
					TOL_vItoa(&buf, val, 10);
				}
					break;
				case 's':
				{
					char * arg = va_arg(va, char *);
					while (*arg)
					{
						*buf++ = *arg++;
					}
				}
					break;
				case 'u':
				TOL_vItoa(&buf, va_arg(va, unsigned int), 10);
					break;
				case 'x':
				case 'X':
				TOL_vItoa(&buf, va_arg(va, int), 16);
					break;
				case '%':
				*buf++ = '%';
					break;
			}
			fmt++;
		}
		/* Else just copy */
		else
		{
			*buf++ = *fmt++;
		}
	}
	*buf = 0;

	return (int)(buf - start_buf);
}

/******************************************************************************
 * Function : TOL_wFormatlength(const char *fmt, va_list va)
 *//**
 * \b Description:
 *
 * This private function Calculates maximum length of the resulting string from the
 * format string and va_list va
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     Maximum length
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //to be used only by printf function
 ~~~~~~~~~~~~~~~
 *
 * @see TOL_wPrintf, TOL_vItoa, TOL_wFormatstring, TOL_wFormatlength, TOL_Fwrite
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static int TOL_wFormatlength (const char *fmt, va_list va)
{
	int length = 0;
	while (*fmt)
	{
		if (*fmt == '%')
		{
			++fmt;
			switch (*fmt)
			{
				case 'c':
				//va_arg(va, int);
				++length;
					break;
				case 'd':
				case 'i':
				case 'u':
				/* 32 bits integer is max 11 characters with minus sign */
				length += 11;
				//va_arg(va, int);
					break;
				case 's':
				{
					char * str = va_arg(va, char *);
					while (*str++)
						++length;
				}
					break;
				case 'x':
				case 'X':
				/* 32 bits integer as hex is max 8 characters */
				length += 8;
				//va_arg(va, unsigned int);
					break;
				default:
				++length;
					break;
			}
		}
		else
		{
			++length;
		}
		++fmt;
	}
	return length;
}

/******************************************************************************
 * Function : TOL_Fwrite(uint8_t bLevel, const uint8_t * buf, uint8_t size)
 *//**
 * \b Description:
 *
 * This private function Copies, starting from the memory location buf, count elements
 * (each of size size) into the stream previously defined in code.
 *
 * PRE-CONDITION: None
 *
 * POST-CONDITION: None
 *
 * @return     Number of elements written
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 * //to be used only by printf function
 ~~~~~~~~~~~~~~~
 *
 * @see TOL_wPrintf, TOL_vItoa, TOL_wFormatstring, TOL_wFormatlength, TOL_Fwrite
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 28/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static uint32_t TOL_Fwrite (uint8_t bLevel, const uint8_t * buf, uint8_t size)
{
	return DIG_wLoggerWritter(GET_TIMESTAMP(), (bLevel | DEBUG_USER), (uint8_t*)buf, size);
}

uint32_t TOL_wPrintf (uint8_t bLevel, const char *fmt, ...)
{
	uint32_t length = 0;
	va_list va;
	va_start(va, fmt);
	length = TOL_wFormatlength(fmt, va);
	va_end(va);
	{
		char buf[MAXIMUM_PRINTF_ARRAY_SIZE];
		va_start(va, fmt);
		length = TOL_wFormatstring(buf, fmt, va);
		length = TOL_Fwrite(bLevel, (uint8_t*)buf, length);
		va_end(va);
	}
	return length;
}

void TOL_vCheckStack (const char * bName)
{
#if (DBG_LOG_LEVEL == LEVEL_DEBUG)
	DBG_LOG_DEBUG("%s Stack Free: %d", bName, osCheckFreeStack(NULL));
#endif
}

/******************************************************************************
 * Unity Testing
 *******************************************************************************/
