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

#include "../../api/api_mdriver_span.h"
#include "../../api/api_mdriver.h"
#include "../../psp/include/psp_string.h"

#include "../../media-drv/spansionBD/FTL/include/Flash.h"
#include "../../media-drv/spansionBD/FTL/source/include/ftl_if_ex.h"


/* The F_DRIVER structure that is filled with the Spansion disk versions of the read
sector, write sector, etc. functions. */
static F_DRIVER  t_driver;

/*
 Name: span_format
*/

int span_format( void )
{
  FTL_INIT_STRUCT initStruct;
  FTL_STATUS status;
  initStruct.format = FTL_FORCE_FORMAT;       
  initStruct.os_type = FTL_RTOS_INT;         // Use Blocking Code
  initStruct.table_storage = FTL_TABLE_LOCATION;
  initStruct.allocate = FTL_ALLOCATE;        // Allocate tables
  
  if((status = FTL_InitAll(&initStruct)) != FTL_ERR_PASS)
  {
//    DBG_Printf("ERROR: ioctl_span_format: FTL_InitAll() \n", 0, 0);
    return MDRIVER_SPAN_ERR_FORMAT;
  }
  
  return MDRIVER_SPAN_NO_ERROR;
}

/*
 Name: span_init
*/

int span_init(void)
{
  FTL_INIT_STRUCT initStruct;
  FTL_STATUS status;

  initStruct.format = FTL_DONT_FORMAT;       // Format as needed
  initStruct.os_type = FTL_RTOS_INT;         // Use Blocking Code
  initStruct.table_storage = FTL_TABLE_LOCATION;
  initStruct.allocate = FTL_ALLOCATE;        // Allocate tables

  if((status = FTL_InitAll(&initStruct)) != FTL_ERR_PASS)
  {
    if(status == FTL_ERR_NOT_FORMATTED)
    {
      if((status = FTL_Format()) !=  FTL_ERR_PASS)
      {
//       DBG_Printf("ERROR: ioctl_span_init: FTL_Format() \n", 0, 0);
       return MDRIVER_SPAN_ERR_SECTOR;
      }
      
      if((status = FTL_InitAll(&initStruct)) != FTL_ERR_PASS)
      {
//       DBG_Printf("ERROR: ioctl_span_init: FTL_InitAll() 2\n", 0, 0);
       return MDRIVER_SPAN_ERR_SECTOR;
      }
	  }
		else
    {
//     DBG_Printf("ERROR: ioctl_span_init: FTL_InitAll() \n", 0, 0);
     return MDRIVER_SPAN_NO_ERROR;
    }

  }
  return MDRIVER_SPAN_NO_ERROR;
}

/*
 Name: span_terminate
*/

static int span_terminate( F_DRIVER * driver )
{
  FTL_STATUS status;
  if((status = FTL_Shutdown()) != FTL_ERR_PASS)
   {
//    DBG_Printf("ERROR: ioctl_span_terminate: FTL_Shutdown() \n", 0, 0);
    return MDRIVER_SPAN_ERR_SECTOR;
   }
  return MDRIVER_SPAN_NO_ERROR;
}

/*
 Name: getphy_span
*/

static int getphy_span( F_DRIVER * driver, F_PHY * phy )
{
  FTL_STATUS status;
  FTL_CAPACITY cap;
  if((status = FTL_GetCapacity(&cap)) != FTL_ERR_PASS)
   {
    DBG_Printf("ERROR: ioctl_getphy_span: FTL_GetCapacity() \n", 0, 0);
    return MDRIVER_SPAN_ERR_SECTOR;
   }
  
  phy->number_of_sectors = cap.numBlocks;
  phy->bytes_per_sector = (cap.totalSize/cap.numBlocks);
	return MDRIVER_SPAN_NO_ERROR;
}


/*
 Name: readsector_span
*/

static int readsector_span( F_DRIVER * driver, void * data, unsigned long sector )
{
  FTL_STATUS status;
  unsigned long done;
  if((status = FTL_DeviceObjectsRead((unsigned char *)data, sector, 1, &done)) != FTL_ERR_PASS)
  {
//   DBG_Printf("ERROR: readsector_span: sect_no=0x%X, ", sector, 0);
   return MDRIVER_SPAN_ERR_SECTOR;
  }

  return MDRIVER_SPAN_NO_ERROR;
}

/*
 Name: writesector_span
*/

static int writesector_span( F_DRIVER * driver, void * data, unsigned long sector )
{
  FTL_STATUS status;
  unsigned long done;
  if((status = FTL_DeviceObjectsWrite((unsigned char *)data, sector, 1, &done)) != FTL_ERR_PASS)
  {
//   DBG_Printf("ERROR: writesector_span: sect_no=0x%X, ", sector, 0);
   return MDRIVER_SPAN_ERR_SECTOR;
  }
  
  return MDRIVER_SPAN_NO_ERROR;
}

/****************************************************************************
 *
 * initfunc_span
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameter
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/
F_DRIVER * initfunc_span ( unsigned long driver_param )
{
  ( void ) driver_param;
  
  (void)psp_memset( &t_driver, 0, sizeof( F_DRIVER ) );
  
  t_driver.readsector = readsector_span;
  t_driver.writesector = writesector_span;
  t_driver.getphy = getphy_span;
  t_driver.release = span_terminate;
  
  return &t_driver;
} /* initfunc_span */

