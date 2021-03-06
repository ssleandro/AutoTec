/*
 * FreeRTOS+FAT SL V1.0.1 (C) 2014 HCC Embedded
 *
 * The FreeRTOS+FAT SL license terms are different to the FreeRTOS license 
 * terms.
 * 
 * FreeRTOS+FAT SL uses a dual license model that allows the software to be used 
 * under a standard GPL open source license, or a commercial license.  The 
 * standard GPL license (unlike the modified GPL license under which FreeRTOS 
 * itself is distributed) requires that all software statically linked with 
 * FreeRTOS+FAT SL is also distributed under the same GPL V2 license terms.  
 * Details of both license options follow:
 * 
 * - Open source licensing -
 * FreeRTOS+FAT SL is a free download and may be used, modified, evaluated and
 * distributed without charge provided the user adheres to version two of the 
 * GNU General Public License (GPL) and does not remove the copyright notice or 
 * this text.  The GPL V2 text is available on the gnu.org web site, and on the
 * following URL: http://www.FreeRTOS.org/gpl-2.0.txt.
 * 
 * - Commercial licensing -
 * Businesses and individuals who for commercial or other reasons cannot comply
 * with the terms of the GPL V2 license must obtain a commercial license before 
 * incorporating FreeRTOS+FAT SL into proprietary software for distribution in 
 * any form.  Commercial licenses can be purchased from 
 * http://shop.freertos.org/fat_sl and do not require any source files to be 
 * changed.
 *
 * FreeRTOS+FAT SL is distributed in the hope that it will be useful.  You
 * cannot use FreeRTOS+FAT SL unless you agree that you use the software 'as
 * is'.  FreeRTOS+FAT SL is provided WITHOUT ANY WARRANTY; without even the
 * implied warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. Real Time Engineers Ltd. and HCC Embedded disclaims all
 * conditions and terms, be they implied, expressed, or statutory.
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/FreeRTOS-Plus
 *
 */

#ifndef _PSP_STRING_H_
#define _PSP_STRING_H_

#include <stddef.h>
#include <string.h>

#include "../../version/ver_psp_string.h"
#if VER_PSP_STRING_MAJOR != 1 || VER_PSP_STRING_MINOR != 4
#error Incompatible PSP_STRING version number!
#endif

#define psp_memcpy( d, s, l )    memcpy( ( d ), ( s ), (size_t)( l ) )
#define psp_memmove( d, s, l )   memmove( ( d ), ( s ), (size_t)( l ) )
#define psp_memset( d, c, l )    memset( ( d ), ( c ), (size_t)( l ) )
#define psp_memcmp( s1, s2, l )  memcmp( ( s1 ), ( s2 ), (size_t)( l ) )
#define psp_strnlen( s, l )      strnlen( ( s ), ( size_t )( l ) )
#define psp_strncat( d, s, l )   strncat( ( d ), ( s ), (size_t)( l ) )
#define psp_strncpy( d, s, l )   strncpy( ( d ), ( s ), (size_t)( l ) )
#define psp_strncmp( s1, s2, l ) strncmp( ( s1 ), ( s2 ), (size_t)( l ) )

#endif /* ifndef _PSP_STRING_H_ */

