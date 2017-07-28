/*******************************************************************************
 * Title                 :   LCD Controller Driver Library
 * Filename              :   lcd.c
 * Author                :   Joao Paulo Martins
 * Origin Date           :   11/02/2016
 * Version               :   1.0.1
 * Compiler              :   GCC 5.2 2015q4
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
 *******************************************************************************/
/*************** SOURCE REVISION LOG *****************************************
 *
 *    Date        Version       Author               Description
 *  11/02/2016     1.0.0     Joao Paulo Martins    File creation
 *  25/02/2016     1.0.1     Joao Paulo Martins    First working version of the driver
 *  03/03/2016     1.0.2     Joao Paulo Martins	  Framebuffer addr in the init struct
 *******************************************************************************/
/** @file 
 *  @brief 
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "board.h"
#include "mculib.h"
#include <lcd.h>

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Module Preprocessor Macros
 *******************************************************************************/

/******************************************************************************
 * Module Typedefs
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static lcd_cursor_size_e eLCD_CursorSize = LCD_CURSIZE_64x64;

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/

/******************************************************************************
 * Function : LCD_vSetFrameBuffer(void* buffer);
 *//**
 * \b Description:
 *
 * This is a private function that informs the LCD peripheral the base address of
 * the framebuffer.
 *
 * PRE-CONDITION:  Initialized LCD controller peripheral.
 * POST-CONDITION:
 *
 * @return     void
 *
 * @see LCD_eInit
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static void pvt_LCD_vSetFrameBuffer (void* buffer)
{
	LPC_LCD->UPBASE = (uint32_t)buffer;
}

/******************************************************************************
 * Function : pvt_LCD_vLowLevelInit(lcd_config_s *LCD_ConfigStruct)
 *//**
 * \b Description:
 *
 * This is a private function that sets the LCD peripheral registers following
 * the initialization struct. It configures the operation of the peripheral according
 * to parameters of LCD hardware.
 *
 * PRE-CONDITION:  A valid lcd_config_s structure and an external RAM memory to be
 * 				  used as framebuffer for the LCD screen.
 * POST-CONDITION: All control and operation registers of LCD peripheral correctly
 * 				  configured.
 *
 * @return     void
 *
 * \b Example
 ~~~~~~~~~~~~~~~{.c}
 *   lcd_config_s sMyLCD;
 *   myLCD.screen_width        = 480;
 *   myLCD.screen_height       = 272;
 *   myLCD.lcd_pixel_e         = LCD_PIX_BPP24;
 *   myLCD.lcd_color_format_e  = LCD_COLOR_RGB;
 *
 *  //Setup LCD and SDRAM pin muxing - this is done in board-specific file
 *  BRD_LCDConfig();
 *
 *  //Reset LCD controller
 *  BRD_vWait_ms(5);
 *  LPC_RGU->RESET_CTRL[0] = (1UL << 16);
 *  BRD_vWait_ms(5);
 *
 *  //Init LCD registers
 *  pvt_LCD_vLowLevelInit(sLCDConfig);
 *
 ~~~~~~~~~~~~~~~
 *
 * @see LCD_eInit, BRD_LCDConfig
 *
 * <br><b> - HISTORY OF CHANGES - </b>
 *
 * <table align="left" style="width:800px">
 * <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
 * <tr><td> 17/02/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
 * </table><br><br>
 * <hr>
 *
 *******************************************************************************/
static void pvt_LCD_vLowLevelInit (lcd_config_s *LCD_ConfigStruct)
{
	uint32_t i, regValue, *pPal;
	uint32_t pcd;

	/*
	 * This routine does not need to check if lcd_config_s is valid because this
	 * was done in LCD_vInit
	 * */

	/* disable the display */
	LPC_LCD->CTRL &= ~CLCDC_LCDCTRL_ENABLE;

	/* Setting LCD_TIMH register */
	regValue = (((((LCD_ConfigStruct->screen_width / 16) - 1) & 0x3F) << 2)
		| (((LCD_ConfigStruct->HSync_pulse_width - 1) & 0xFF) << 8)
		| (((LCD_ConfigStruct->horizontal_porch.front - 1) & 0xFF) << 16)
		| (((LCD_ConfigStruct->horizontal_porch.back - 1) & 0xFF) << 24));

	LPC_LCD->TIMH = regValue;

	/* Setting LCD_TIMV register */
	regValue = ((((LCD_ConfigStruct->screen_height - 1) & 0x3FF) << 0)
		| (((LCD_ConfigStruct->VSync_pulse_width - 1) & 0x03F) << 10)
		| (((LCD_ConfigStruct->vertical_porch.front - 1) & 0x0FF) << 16)
		| (((LCD_ConfigStruct->vertical_porch.back - 1) & 0x0FF) << 24));

	LPC_LCD->TIMV = regValue;

	/* Generate the clock and signal polarity control word */
	regValue = 0;
	regValue = (((LCD_ConfigStruct->ac_bias_frequency - 1) & 0x1F) << 6);
	regValue |= (LCD_ConfigStruct->OE_pol & 1) << 14;
	regValue |= (LCD_ConfigStruct->panel_clk_edge & 1) << 13;
	regValue |= (LCD_ConfigStruct->HSync_pol & 1) << 12;
	regValue |= (LCD_ConfigStruct->VSync_pol & 1) << 11;

	/* Compute clocks per line based on panel type */

	switch (LCD_ConfigStruct->lcd_panel_type)
	{
		case LCD_MONO_4_TYP:
		regValue |= ((((LCD_ConfigStruct->screen_width / 4) - 1) & 0x3FF) << 16);
			break;

		case LCD_MONO_8_TYP:
		regValue |= ((((LCD_ConfigStruct->screen_width / 8) - 1) & 0x3FF) << 16);
			break;

		case LCD_CSTN_TYP:
		regValue |= (((((LCD_ConfigStruct->screen_width * 3) / 8) - 1) & 0x3FF) << 16);
			break;

		case LCD_TFT_TYP:
		default:
		regValue |= /* 1<<26 | */(((LCD_ConfigStruct->screen_width - 1) & 0x3FF) << 16);
	}

	/* Panel clock divisor - fixed but should be calculated from LCDDCLK*/
	pcd = 5; //LCD_ConfigStruct->pcd;
	pcd &= 0x3FF;
	regValue |= ((pcd >> 5) << 27) | ((pcd) & 0x1F);

	LPC_LCD->POL = regValue;

	/* configure line end control */
	if (LCD_ConfigStruct->line_end_delay)
	{
		LPC_LCD->LE = (LCD_ConfigStruct->line_end_delay - 1) | 1 << 16;
	}
	else
	{
		LPC_LCD->LE = 0;
	}

	/* disable interrupts */
	LPC_LCD->INTMSK = 0;

	/* set bits per pixel */
	regValue = LCD_ConfigStruct->bits_per_pixel << 1;

	/* set color format BGR or RGB */
	regValue |= LCD_ConfigStruct->color_format << 8;
	regValue |= LCD_ConfigStruct->lcd_panel_type << 4;

	if (LCD_ConfigStruct->dual_panel == 1)
	{
		regValue |= 1 << 7;
	}
	LPC_LCD->CTRL = regValue;
	/* clear palette */
	pPal = (uint32_t*)(&(LPC_LCD->PAL));

	for (i = 0; i < 128; i++)
	{
		*pPal = 0;
		pPal++;
	}
}

/* LCD Power On ***************************************************************/
void LCD_vPower (lcd_onoff_e OnOff)
{
	volatile int i;

	if (OnOff) // 1 = Turn on
	{
		LPC_LCD->CTRL |= CLCDC_LCDCTRL_PWR;
		for (i = 0; i < 1000000; i++)
			;
		LPC_LCD->CTRL |= CLCDC_LCDCTRL_ENABLE;
	}
	else // 0 = Turn off
	{
		LPC_LCD->CTRL &= ~CLCDC_LCDCTRL_PWR;
		for (i = 0; i < 1000000; i++)
			;
		LPC_LCD->CTRL &= ~CLCDC_LCDCTRL_ENABLE;
	}
}

/* High Level LCD Initialization  *****************************************/
eMCUError_s LCD_eInit (lcd_config_s *sLCDConfig)
{

	/* Check if the lcd_config_s structure is valid by checking the framebuffer
	 * address. This should be at internal/external SRAM space
	 */
	uint32_t *vPointer = (uint32_t*)sLCDConfig->framebuffer_addr;

	if ((vPointer == NULL) || (
		!((vPointer >= (uint32_t*)0x28000000) && (vPointer < (uint32_t*)0x40000000)) &&
			!((vPointer >= (uint32_t*)0x60000000) && (vPointer < (uint32_t*)0x80000000))))
	{
		/* Is NULL or is outside all RAM areas, exit function */
		goto invalid_init;
	}

	/* Setup LCD and SDRAM pin muxing - this is done in board-specific file */
	BRD_LCDConfig();

	/* Reset LCD controller */
	BRD_vWait_ms(5);
	LPC_RGU->RESET_CTRL[0] = (1UL << 16);
	BRD_vWait_ms(5);

	/* Init LCD registers */
	pvt_LCD_vLowLevelInit(sLCDConfig);

	/* Set framebuffer */
	pvt_LCD_vSetFrameBuffer(sLCDConfig->framebuffer_addr);

	/* Power On */
	LCD_vPower(LCD_POWER_ON);
	BRD_vWait_ms(100);

	return MCU_ERROR_SUCCESS;

	invalid_init:
	return MCU_ERROR_LCD_INVALID_CONFIG;
}

void LCD_vCursorConfig (lcd_cursor_size_e cursor_size, bool sync)
{
	eLCD_CursorSize = cursor_size;
	LPC_LCD->CRSR_CFG = ((sync ? 1 : 0) << 1) | cursor_size;
}

void LCD_vCursorWriteImage (uint8_t cursor_num, void* Image)
{
	int i, j;
	volatile uint32_t *fifoptr, *crsr_ptr = (uint32_t *)Image;

	/* Check if Cursor Size was configured as 32x32 or 64x64*/
	if (eLCD_CursorSize == LCD_CURSIZE_32x32)
	{
		i = cursor_num * 64;
		j = i + 64;
	}
	else
	{
		i = 0;
		j = 256;
	}

	fifoptr = (volatile uint32_t*)&(LPC_LCD->CRSR_IMG[0]);

	/* Copy Cursor Image content to FIFO */
	for (; i < j; i++)
	{

		*fifoptr = *crsr_ptr;
		crsr_ptr++;
		fifoptr++;
	}
}

void* LCD_vpCursorGetImageBufferAddress (uint8_t cursor_num)
{
	return (void*)&(LPC_LCD->CRSR_IMG[cursor_num * 64]);
}

void LCD_vCursorEnable (uint8_t cursor_num, uint8_t OnOff)
{
	if (OnOff)
	{
		LPC_LCD->CRSR_CTRL = (cursor_num << 4) | 1;
	}
	else
	{
		LPC_LCD->CRSR_CTRL = (cursor_num << 4);
	}
}

void LCD_vLoadPalette (void* palette)
{
	lcd_palette_entry_s pal_entry, *ptr_pal_entry;
	uint8_t i, *pal_ptr;

	/* This function supports loading of the color palette from
	 the C file generated by the bmp2c utility. It expects the
	 palette to be passed as an array of 32-bit BGR entries having
	 the following format:
	 2:0 - Not used
	 7:3 - Blue
	 10:8 - Not used
	 15:11 - Green
	 18:16 - Not used
	 23:19 - Red
	 31:24 - Not used
	 arg = pointer to input palette table address */
	ptr_pal_entry = &pal_entry;
	pal_ptr = (uint8_t *)palette;

	/* 256 entry in the palette table */
	for (i = 0; i < 256 / 2; i++)
	{
		pal_entry.Bl = (*pal_ptr++) >> 3; /* blue first */
		pal_entry.Gl = (*pal_ptr++) >> 3; /* get green */
		pal_entry.Rl = (*pal_ptr++) >> 3; /* get red */
		pal_ptr++; /* skip over the unused byte */
		/* do the most significant halfword of the palette */
		pal_entry.Bu = (*pal_ptr++) >> 3; /* blue first */
		pal_entry.Gu = (*pal_ptr++) >> 3; /* get green */
		pal_entry.Ru = (*pal_ptr++) >> 3; /* get red */
		pal_ptr++; /* skip over the unused byte */

		LPC_LCD->PAL[i] = *(volatile uint32_t *)ptr_pal_entry;
	}
}

void LCD_vCursorLoadPalette0 (uint32_t palette_color)
{
	/* 7:0 - Red
	 15:8 - Green
	 23:16 - Blue
	 31:24 - Not used*/
	LPC_LCD->CRSR_PAL0 = (uint32_t)palette_color;
}

void LCD_vCursorLoadPalette1 (uint32_t palette_color)
{
	/* 7:0 - Red
	 15:8 - Green
	 23:16 - Blue
	 31:24 - Not used*/
	LPC_LCD->CRSR_PAL1 = (uint32_t)palette_color;
}

void LCD_vSetInterrupt (lcd_intsrc_e Int)
{
	LPC_LCD->INTMSK |= Int;
}

void LCD_vClrInterrupt (lcd_intsrc_e Int)
{
	LPC_LCD->INTCLR |= Int;
}

lcd_intsrc_e LCD_eGetInterrupt (void)
{
	return (lcd_intsrc_e)LPC_LCD->INTRAW;
}

void LCD_vCursorSetInterrupt (void)
{
	LPC_LCD->CRSR_INTMSK |= 1;
}

void LCD_vCursorClrInterrupt (void)
{
	LPC_LCD->CRSR_INTCLR |= 1;
}

void LCD_vCursorSetPos (uint16_t x, uint16_t y)
{
	LPC_LCD->CRSR_XY = (x & 0x3FF) | ((y & 0x3FF) << 16);
}

void LCD_vCursorSetClip (uint16_t x, uint16_t y)
{
	LPC_LCD->CRSR_CLIP = (x & 0x3F) | ((y & 0x3F) << 8);
}

