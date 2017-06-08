/*********************************************************************
 *                SEGGER Microcontroller GmbH & Co. KG                *
 *        Solutions for real time microcontroller applications        *
 **********************************************************************
 *                                                                    *
 *        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
 *                                                                    *
 *        Internet: www.segger.com    Support:  support@segger.com    *
 *                                                                    *
 **********************************************************************

 ** emWin V5.30 - Graphical user interface for embedded applications **
 All  Intellectual Property rights  in the Software belongs to  SEGGER.
 emWin is protected by  international copyright laws.  Knowledge of the
 source code may not be used to write a similar product.  This file may
 only be used in accordance with the following terms:

 The software has been licensed to  NXP Semiconductors USA, Inc.  whose
 registered  office  is  situated  at 411 E. Plumeria Drive, San  Jose,
 CA 95134, USA  solely for  the  purposes  of  creating  libraries  for
 NXPs M0, M3/M4 and  ARM7/9 processor-based  devices,  sublicensed  and
 distributed under the terms and conditions of the NXP End User License
 Agreement.
 Full source code is available at: www.segger.com

 We appreciate your understanding and fairness.
 ----------------------------------------------------------------------
 Licensing information

 Licensor:                 SEGGER Microcontroller Systems LLC
 Licensed to:              NXP Semiconductors
 Licensed SEGGER software: emWin
 License number:           GUI-00186
 License model:            emWin License Agreement, dated August 20th 2011
 Licensed product:         -
 Licensed platform:        NXP's ARM 7/9, Cortex-M0,M3,M4
 Licensed number of seats: -
 ----------------------------------------------------------------------
 File        : GUIDRV_NoOpt_1_8.h
 Purpose     : Interface definition for non optimized drawing functions
 ---------------------------END-OF-HEADER------------------------------
 */

#ifndef GUIDRV_NOOPT_1_8_H
#define GUIDRV_NOOPT_1_8_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "GUI_Private.h"

#ifndef GUIDRV_NOOPT_1_8_H
#define GUIDRV_NOOPT_1_8_H

	void GUIDRV__NoOpt_XorPixel (GUI_DEVICE * pDevice, int x, int y);
	void GUIDRV__NoOpt_DrawHLine (GUI_DEVICE * pDevice, int x0, int y, int x1);
	void GUIDRV__NoOpt_DrawVLine (GUI_DEVICE * pDevice, int x, int y0, int y1);
	void GUIDRV__NoOpt_FillRect (GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1);
	void GUIDRV__NoOpt_DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0, int xSize, int ySize, int BitsPerPixel, int BytesPerLine, const U8 * pData, int Diff, const LCD_PIXELINDEX * pTrans);

#endif

#ifdef __cplusplus
}
#endif
#endif /* GUIDRV_NOOPT_1_8_H */

/*************************** End of file ****************************/
