// file: Memory.h
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

#ifndef MEMORY_H
#define MEMORY_H

#include "SystemDef.h"

//-------- MEM_Memcpy -------
// Copy a block of memory from src to dest.
// Param 1: pointer to destination
// Param 2: pointer to source
// Param 3: size of a block of memory
void MEM_Memcpy (UINT8 *dest, UINT8 *src, UINT32 size);

#if (CACHE_RAM_BD_MODULE == FTL_TRUE && CACHE_DYNAMIC_ALLOCATION == FTL_TRUE)
void *MEM_Malloc (UINT32 size);
void MEM_Free (void *pointer);
#endif

#endif
