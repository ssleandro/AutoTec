// file: SystemDef.h
/**************************************************************************
* Copyright (C)2012 Spansion Inc. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion Inc, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software constitutes driver source code for use in programming Spansion's 
* Flash memory components. This software is licensed by Spansion to be adapted only 
* for use in systems utilizing Spansion's Flash memories. Spansion is not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Spansion is providing this source code "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the source code herein.  
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE, 
* REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED 
* USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY, 
* FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  SPANSION WILL 
* HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR 
* OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE, 
* INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL, 
* SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS, 
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Copyright notice must be included with 
* this software, whether used in part or whole, at all times.  
*/

#ifndef SYSTEM_DEF_H
#define SYSTEM_DEF_H

#ifndef BOOL
#define BOOL unsigned char               
#endif 

#ifndef SINT8
#define SINT8 signed char    
#endif 

#ifndef SINT16
#define SINT16 signed short 
#endif 

#ifndef SINT32
#define SINT32 signed long
#endif 
        
#ifndef UINT8
#define UINT8 unsigned char
#endif 

#ifndef UINT16
#define UINT16 unsigned short
#endif 

#ifndef UINT32
#define UINT32 unsigned long
#endif 

#ifndef UINT64
#define UINT64 unsigned long long               
#endif 

#ifndef INT8
#define INT8 char 
#endif 

#ifndef INT16
#define INT16 short  
#endif 

#ifndef INT32
#define INT32 long  
#endif 

#ifndef INT64
#define INT64 long long  
#endif 

#ifndef OK
#define OK    (0)
#endif

#ifndef ERR
#define ERR   (1)
#endif

#ifndef NULL
#define NULL  (0)
#endif // #ifndef NULL

#ifndef FALSE
#define FALSE (0)
#endif // #ifndef FALSE

#ifndef TRUE
#define TRUE  (1)
#endif // #ifndef TRUE


#ifndef PASS
#define PASS  (0) 
#endif 

#ifndef FAIL
#define FAIL (1)
#endif 

#ifndef PARAM
#define PARAM  (2)
#endif 
 
#define BYTES_PER_BLOCK 512
  
#endif
