/****************************************************************************
 * Title                 :   LCD Controller Header File
 * Filename              :   lcd.h
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
 *****************************************************************************/
/*************** INTERFACE CHANGE LIST **************************************
 *
 *    Date    Version   Author         Description 
 *  11/02/2016     1.0.0     Joao Paulo Martins    File creation
 *  25/02/2016     1.0.1     Joao Paulo Martins    First working version of the driver
 *  03/03/2016     1.0.2     Joao Paulo Martins	  Framebuffer addr in the init struct
 *****************************************************************************/
/** @file lcd.h
 *  @brief This file provides an interface to configure and enable a LCD display
 *  using the LPC4357 LCD controller.
 *
 *  To enable the LCD, the upper layer must provide a valid initialization
 *  structure, the lcd_config_s struct. This object must describe the LCD
 *  parameters, like the number of pixels (horizontal and vertical), color format,
 *  and LCD panel type. The init structure must also have the framebuffer address
 *  and this must be a valid EXTERNAL RAM address.
 *
 *  The use of an external RAM is mandatory to enable the LCD feature. The initialization
 *  and enabling of this memory must be done at board-specific file. The LCD init must
 *  call this routine prior to configure the LCD peripheral registers.
 *
 *  The LCD also needs a backlight for proper work. This driver does not control the
 *  backlight of the LCD. This must be done separately.
 *
 *  This version of  the driver only works for TFT LCD displays with direct RGB interface.
 *
 *  Once initialized, the LCD will continuously read the framebuffer area and
 *  translate the pixel information to LCD screen.
 *
 */
#ifndef ABS_INC_LCD_H_
#define ABS_INC_LCD_H_

/******************************************************************************
 * Includes
 *******************************************************************************/
#include <inttypes.h>
#include <stdbool.h>
#include "mcuerror.h"
#include "gpio.h"

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

/** @brief LCD power on/off commands */
typedef enum
{
	LCD_POWER_OFF = 0x00,
	LCD_POWER_ON
} lcd_onoff_e;

/** @brief LCD Interrupt Source */
typedef enum
{
	LCD_INTSRC_FUF = (1 << 1), /**< FIFO underflow bit */
	LCD_INTSRC_LNBU = (1 << 2), /**< LCD next base address update bit */
	LCD_INTSRC_VCOMP = (1 << 3), /**< vertical compare bit */
	LCD_INTSRC_BER = (1 << 4) /**< AHB master error interrupt bit */
} lcd_intsrc_e;

/** @brief LCD signal polarity */
typedef enum
{
	LCD_SIGPOL_ACTIVE_HIGH = 0, /**< Active high polarity */
	LCD_SIGPOL_ACTIVE_LOW = 1 /**< Active low polarity  */
} lcd_sigpolarity_e;

/** @brief LCD clock edge polarity */
typedef enum
{
	LCD_CLKPOL_RISING = 0, /**< Rising edge detection */
	LCD_CLKPOL_FALLING = 1 /**< Falling edge detection */
} lcd_clkpol_e;

/** @brief LCD bits per pixel and pixel format */
typedef enum
{
	LCD_PIX_BPP1 = 0,
	LCD_PIX_BPP2,
	LCD_PIX_BPP4,
	LCD_PIX_BPP8,
	LCD_PIX_BPP16,
	LCD_PIX_BPP24,
	LCD_PIX_BPP16_565,
	LCD_PIX_BPP12_444
} lcd_pixel_e;

/** @brief LCD color format */
typedef enum
{
	LCD_COLOR_RGB = 0,
	LCD_COLOR_BGR
} lcd_color_format_e;

/*********************************************************************//**
 * @brief LCD structure definitions
 **********************************************************************/
/** @brief LCD Palette entry format */
typedef struct
{
	uint32_t Rl :5;
	uint32_t Gl :5;
	uint32_t Bl :5;
	uint32_t Il :1;
	uint32_t Ru :5;
	uint32_t Gu :5;
	uint32_t Bu :5;
	uint32_t Iu :1;
} lcd_palette_entry_s;

/** @brief LCD cursor format in 1 byte LBBP */
typedef struct
{
	uint8_t Pixel3 :2;
	uint8_t Pixel2 :2;
	uint8_t Pixel1 :2;
	uint8_t Pixel0 :2;
} lcd_cursor_format_s;

/** @brief LCD cursor size */
typedef enum
{
	LCD_CURSIZE_32x32 = 0,
	LCD_CURSIZE_64x64
} lcd_cursor_size_e;

/** @brief LCD panel type */
typedef enum
{
	LCD_TFT_TYP = 0x02, /**< standard TFT */
	LCD_MONO_4_TYP = 0x01, /**< 4-bit STN mono */
	LCD_MONO_8_TYP = 0x05, /**< 8-bit STN mono */
	LCD_CSTN_TYP = 0x00 /**< color STN */
} lcd_panel_type_s;

/** @brief LCD porch configuration structure */
typedef struct
{
	uint16_t front; /**< Front porch setting in clocks */
	uint16_t back; /**< Back porch setting in clocks */
} lcd_porch_cfg_s;

/** @brief LCD configuration structure */
typedef struct
{
	uint16_t screen_width; /**< Pixels per line */
	uint16_t screen_height; /**< Lines per panel */
	lcd_porch_cfg_s horizontal_porch; /**< Porch setting for horizontal */
	lcd_porch_cfg_s vertical_porch; /**< Porch setting for vertical */
	uint16_t HSync_pulse_width; /**< HSYNC pulse width in clocks */
	uint16_t VSync_pulse_width; /**< VSYNC pulse width in clocks */
	uint8_t ac_bias_frequency; /**< AC bias frequency in clocks */
	lcd_sigpolarity_e HSync_pol; /**< HSYNC polarity */
	lcd_sigpolarity_e VSync_pol; /**< VSYNC polarity */
	lcd_clkpol_e panel_clk_edge; /**< Panel Clock Edge Polarity */
	lcd_sigpolarity_e OE_pol; /**< Output Enable polarity */
	uint32_t line_end_delay; /**< 0 if not use */
	lcd_pixel_e bits_per_pixel; /**< Maximum bits per pixel the display supports */
	lcd_panel_type_s lcd_panel_type; /**< LCD panel type */
	lcd_color_format_e color_format; /**< BGR or RGB */
	bool dual_panel; /**< Dual panel, TRUE = dual panel display */
	uint16_t pcd;
	void *framebuffer_addr; /**< Framebuffer address */
} lcd_config_s;

/******************************************************************************
 * Variables
 *******************************************************************************/

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function : LCD_vPower(lcd_onoff_e OnOff);
 *//**
 * \b Description:
 *
 * This is a public function used to turn of and off and LCD display.
 *
 * PRE-CONDITION: Initialized LCD controller peripheral.
 *
 * POST-CONDITION: LCD display with control (on/off) pin configured.
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
 *   LCD_eInit(&myLCD);
 *
 *   LCD_vPower(LCD_POWER_OFF); // turn off the display
 ~~~~~~~~~~~~~~~
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
void LCD_vPower (lcd_onoff_e OnOff);

/******************************************************************************
 * Function : LCD_eInit(lcd_config_s *sLCDConfig);
 *//**
 * \b Description:
 *
 * This is a public function that initalizes and enable a LCD display.
 *
 * PRE-CONDITION: Valid lcd_config_s with the correct hardware parameters and framebuffer
 * address. An external RAM memory to hold the framebuffer data.
 *
 * POST-CONDITION: LCD display enabled and configured, as well as an external RAM memory to
 * holds the framebuffer data. The backlight control of LCD is not set in this module
 * and must be defined in separated routines. (The LCD needs a backlight for proper work).
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
 *   LCD_eInit(&myLCD);
 *~~~~~~~~~~~~~~~
 *
 * @see
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
eMCUError_s LCD_eInit (lcd_config_s *sLCDConfig);

/*********************************************************************//**
 * @brief     Configure Cursor
 * @param[in] cursor_size specify size of cursor
 *          - LCD_CURSOR_32x32  :cursor size is 32x32 pixels
 *          - LCD_CURSOR_64x64  :cursor size is 64x64 pixels
 * @param[in] sync cursor sync mode
 *          - TRUE  :cursor sync to the frame sync pulse
 *          - FALSE :cursor async mode
 * @return    None
 **********************************************************************/
void LCD_vCursorConfig (lcd_cursor_size_e cursor_size, bool sync);

/*********************************************************************//**
 * @brief     Write Cursor Image into Internal Cursor Image Buffer
 * @param[in] cursor_num specify number of cursor is going to be written
 *        this param must < 4
 * @param[in] Image point to Cursor Image Buffer
 * @return    None
 **********************************************************************/
void LCD_vCursorWriteImage (uint8_t cursor_num, void* Image);

/*********************************************************************//**
 * @brief     Get Internal Cursor Image Buffer Address
 * @param[in] cursor_num specify number of cursor is going to be written
 *        this param must < 4
 * @return    Cursor Image Buffer Address
 **********************************************************************/
void* LCD_vpCursorGetImageBufferAddress (uint8_t cursor_num);

/*********************************************************************//**
 * @brief     Enable Cursor
 * @param[in] LCDx pointer to LCD Controller Reg Struct, should be: LPC_LCD
 * @param[in] cursor_num specify number of cursor is going to be written
 *        this param must < 4
 * @param[in] OnOff Turn on/off LCD
 *          - TRUE  :Enable
 *          - FALSE :Disable
 * @return    None
 **********************************************************************/
void LCD_vCursorEnable (uint8_t cursor_num, uint8_t OnOff);

/*********************************************************************//**
 * @brief     Load LCD Palette
 * @param[in] palette point to palette address
 * @return    None
 **********************************************************************/
void LCD_vLoadPalette (void* palette);

/*********************************************************************//**
 * @brief     Load Cursor Palette
 * @param[in] palette_color cursor palette 0 value
 * @return    None
 **********************************************************************/
void LCD_vCursorLoadPalette0 (uint32_t palette_color);

/*********************************************************************//**
 * @brief     Load Cursor Palette
 * @param[in] palette_color cursor palette 1 value
 * @return    None
 **********************************************************************/
void LCD_vCursorLoadPalette1 (uint32_t palette_color);

/*********************************************************************//**
 * @brief     Set Interrupt
 * @param[in] Int LCD Interrupt Source, should be:
 *          - LCD_INT_FUF :FIFO underflow
 *          - LCD_INT_LNBU  :LCD next base address update bit
 *          - LCD_INT_VCOMP :Vertical compare bit
 *          - LCD_INT_BER :AHB master error interrupt bit
 * @return    None
 **********************************************************************/
void LCD_vSetInterrupt (lcd_intsrc_e Int);

/*********************************************************************//**
 * @brief     Clear Interrupt
 * @param[in] Int LCD Interrupt Source, should be:
 *          - LCD_INT_FUF :FIFO underflow
 *          - LCD_INT_LNBU  :LCD next base address update bit
 *          - LCD_INT_VCOMP :Vertical compare bit
 *          - LCD_INT_BER :AHB master error interrupt bit
 * @return    None
 **********************************************************************/
void LCD_vClrInterrupt (lcd_intsrc_e Int);

/*********************************************************************//**
 * @brief     Get LCD Interrupt Status
 * @return    INTRAW register
 **********************************************************************/
lcd_intsrc_e LCD_eGetInterrupt (void);

/*********************************************************************//**
 * @brief     Enable Cursor Interrupt
 * @return    None
 **********************************************************************/
void LCD_vCursorSetInterrupt (void);

/*********************************************************************//**
 * @brief     Clear Cursor Interrupt
 * @return    None
 **********************************************************************/
void LCD_vCursorClrInterrupt (void);

/*********************************************************************//**
 * @brief     Set Cursor Position
 * @param[in] x horizontal position
 * @param[in] y vertical position
 * @return    None
 **********************************************************************/
void LCD_vCursorSetPos (uint16_t x, uint16_t y);

/*********************************************************************//**
 * @brief     Set Cursor Clipping Position
 * @param[in] x horizontal position, should be in range: 0..63
 * @param[in] y vertical position, should be in range: 0..63
 * @return    None
 **********************************************************************/
void LCD_vCursorSetClip (uint16_t x, uint16_t y);

#endif /* ABS_INC_LCD_H_ */
