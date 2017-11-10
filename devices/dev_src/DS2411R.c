/****************************************************************************
 * Title                 :   DS2411R
 * Filename              :   DS2411R.c
 * Author                :   Henrique Reis
 * Origin Date           :   03/11/2017
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
 * 03/11/2017  1.0.0     Henrique Reis      DS2411R.c created.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "DS2411R.h"
#include "DS2411R_config.h"
#include <string.h>
#ifndef UNITY_TEST
#include "mculib.h"
#include "board.h"
#endif

/******************************************************************************
 * Module Preprocessor Constants
 *******************************************************************************/

/******************************************************************************
 * Variables from others modules
 *******************************************************************************/

/******************************************************************************
 * Typedefs
 *******************************************************************************/

/******************************************************************************
 * Public Variables
 *******************************************************************************/

/******************************************************************************
 * Module Variable Definitions
 *******************************************************************************/
static uint8_t abM2GSerialNumber [DS2411R_ID_BYTES_NUMBER];
static gpio_config_s sDS2411R;
static bool bValidIDNumber = false;

const uint8_t abCRC8Lookup [256] =
{
    0,   94,  188,  226,   97,   63,  221,  131,
  194,  156,  126,   32,  163,  253,   31,   65,
  157,  195,   33,  127,  252,  162,   64,   30,
   95,    1,  227,  189,   62,   96,  130,  220,
   35,  125,  159,  193,   66,   28,  254,  160,
  225,  191,   93,    3,  128,  222,   60,   98,
  190,  224,    2,   92,  223,  129,   99,   61,
  124,   34,  192,  158,   29,   67,  161,  255,
   70,   24,  250,  164,   39,  121,  155,  197,
  132,  218,   56,  102,  229,  187,   89,    7,
  219,  133,  103,   57,  186,  228,    6,   88,
   25,   71,  165,  251,  120,   38,  196,  154,
  101,   59,  217,  135,    4,   90,  184,  230,
  167,  249,   27,   69,  198,  152,  122,   36,
  248,  166,   68,   26,  153,  199,   37,  123,
   58,  100,  134,  216,   91,    5,  231,  185,
  140,  210,   48,  110,  237,  179,   81,   15,
   78,   16,  242,  172,   47,  113,  147,  205,
   17,   79,  173,  243,  112,   46,  204,  146,
  211,  141,  111,   49,  178,  236,   14,   80,
  175,  241,   19,   77,  206,  144,  114,   44,
  109,   51,  209,  143,   12,   82,  176,  238,
   50,  108,  142,  208,   83,   13,  239,  177,
  240,  174,   76,   18,  145,  207,   45,  115,
  202,  148,  118,   40,  171,  245,   23,   73,
    8,   86,  180,  234,  105,   55,  213,  139,
   87,    9,  235,  181,   54,  104,  138,  212,
  149,  203,   41,  119,  244,  170,   72,   22,
  233,  183,   85,   11,  136,  214,   52,  106,
   43,  117,  151,  201,   74,   20,  246,  168,
  116,   42,  200,  150,   21,   75,  169,  247,
  182,  232,   10,   84,  215,   137, 107,  53
};

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/******************************************************************************
 * Function Definitions
 *******************************************************************************/
eDEVError_s IDN_open (void)
{
	sDS2411R.vpPrivateData = NULL;
	sDS2411R.bDefaultOutputHigh = true;
	sDS2411R.eDirection = GPIO_INPUT;
	sDS2411R.ePull = GPIO_PULL_INACT;
	sDS2411R.eInterrupt = GPIO_INTERRUPT_DISABLED;
	sDS2411R.fpCallBack = NULL;
	sDS2411R.bMPort = DS2411R_ID_IO_GPIO_PORT;
	sDS2411R.bMPin = DS2411R_ID_IO_GPIO_PIN;

	// Initialize time pulse external interrupt
	eDEVError_s eError = (eDEVError_s) GPIO_eInit(&sDS2411R);
	return eError;
}

uint8_t IDN_OneWire_VerifyCRC(void)
{
	uint8_t bI, bAux, bCRCCalc = 0;

	for (bI = DS2411R_ID_BYTES_NUMBER; bI > 0; bI--)
	{
		bAux = abM2GSerialNumber[bI - 1];
		bAux = bAux ^ (bCRCCalc);

		bCRCCalc = abCRC8Lookup [bAux];
	}

	return bCRCCalc;
}

void IDN_OneWire_ReadByte(uint8_t* pbData)
{
	*pbData = 0;

#if defined (AUTEQ_VERSION)
	for (uint8_t bI = 0; bI < DS2411R_ID_BYTES_NUMBER; bI++) {
		*pbData >>= 1;

		GPIO_vClear(&sDS2411R);
		BRD_vWait_us(DS2411R_READ_SAMPLE_TIME_US);

		GPIO_vSet(&sDS2411R);
		BRD_vWait_us(DS2411R_READ_SAMPLE_TIME_US);

		if (GPIO_bRead(&sDS2411R))
		{
			*pbData |= 0x80;
		}

		BRD_vWait_us(DS2411R_TIME_SLOT_DURATION_US - (2*DS2411R_READ_SAMPLE_TIME_US));
	}
#else
	for (uint8_t bI = 0; bI < DS2411R_ID_BYTES_NUMBER; bI++) {
		*pbData >>= 1;

		GPIO_vSetDir(&sDS2411R, true);
		GPIO_vClear(&sDS2411R);
		BRD_vWait_us(DS2411R_READ_SAMPLE_TIME_US);

		GPIO_vSet(&sDS2411R);
		BRD_vWait_us(DS2411R_READ_SAMPLE_TIME_US);

		GPIO_vSetDir(&sDS2411R, false);

		if (GPIO_bRead(&sDS2411R))
		{
			*pbData |= 0x80;
		}

		BRD_vWait_us(DS2411R_TIME_SLOT_DURATION_US - (2*DS2411R_READ_SAMPLE_TIME_US));
		GPIO_vSet(&sDS2411R);
	}
#endif
}

void IDN_OneWire_WriteByte(uint8_t bData)
{
#if defined (AUTEQ_VERSION)
	for (uint8_t bI = 0; bI < DS2411R_ID_BYTES_NUMBER; bI++) {
		if (bData & 0x01)
		{
			GPIO_vClear(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_1_LOW_TIME_US);

			GPIO_vSet(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_0_LOW_TIME_US);
		} else
		{
			GPIO_vClear(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_0_LOW_TIME_US);

			GPIO_vSet(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_1_LOW_TIME_US);
		}
		bData >>= 1;
	}
#else
	GPIO_vSetDir(&sDS2411R, true);

	for (uint8_t bI = 0; bI < DS2411R_ID_BYTES_NUMBER; bI++) {
		if (bData & 0x01)
		{
			GPIO_vClear(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_1_LOW_TIME_US);

			GPIO_vSet(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_0_LOW_TIME_US);
		} else
		{
			GPIO_vClear(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_0_LOW_TIME_US);

			GPIO_vSet(&sDS2411R);
			BRD_vWait_us(DS2411R_WRITE_1_LOW_TIME_US);
		}
		bData >>= 1;
	}
#endif
}

bool IDN_OneWire_Reset(void)
{
	bool bPresencePulse = false;

#if defined (AUTEQ_VERSION)
	GPIO_vClear(&sDS2411R);
	BRD_vWait_us(DS2411R_RESET_LOW_TIME_US);

	GPIO_vSet(&sDS2411R);
	BRD_vWait_us(DS2411R_PRES_DETECT_TIME_US);

	bPresencePulse = GPIO_bRead(&sDS2411R);

	BRD_vWait_us(DS2411R_RESET_LOW_TIME_US - DS2411R_PRES_DETECT_TIME_US);
#else
	GPIO_vSetDir(&sDS2411R, true);

	GPIO_vClear(&sDS2411R);
	BRD_vWait_us(DS2411R_RESET_LOW_TIME_US);

	GPIO_vSet(&sDS2411R);
	GPIO_vSetDir(&sDS2411R, false);

	while(GPIO_bRead(&sDS2411R));

	while(!GPIO_bRead(&sDS2411R));

	bPresencePulse = true;

	GPIO_vSetDir(&sDS2411R, true);
#endif

	return bPresencePulse;
}

uint32_t IDN_OneWire_Read_ID_Number(void)
{
	uint8_t bReadBytes = 0;

	if (IDN_OneWire_Reset())
	{
		BRD_vWait_ms(5);

		IDN_OneWire_WriteByte(DS2411R_CMD_READ_ROM);

		BRD_vWait_ms(5);

		for (uint8_t bI = sizeof(abM2GSerialNumber); bI > 0; bI--) {
			IDN_OneWire_ReadByte(&abM2GSerialNumber[bI - 1]);
		}

		if (IDN_OneWire_VerifyCRC() == 0)
		{
			bReadBytes = DS2411R_ID_BYTES_NUMBER;
		}
	}
	return bReadBytes;
}

uint32_t IDN_read (struct peripheral_descriptor_s* const this,
	void * const vpBuffer,
	const uint32_t tBufferSize)
{
	if (tBufferSize < 8)
		return 0;

	uint8_t bReadBytes = 0;
	(void)this;

	bReadBytes = IDN_OneWire_Read_ID_Number();
	bValidIDNumber = (bReadBytes == 8);

	if(bValidIDNumber)
		memcpy(vpBuffer, abM2GSerialNumber, tBufferSize);

	return (uint32_t) bReadBytes;
}

uint32_t IDN_write (struct peripheral_descriptor_s* const this,
	const void * vpBuffer,
	const uint32_t tBufferSize)
{
	(void)this;
	(void)tBufferSize;

	return 0;
}

eDEVError_s IDN_ioctl (struct peripheral_descriptor_s* const this,
	uint32_t wRequest,
	void * vpValue)
{
	(void)this;
	(void)wRequest;
	(void)vpValue;
	return DEV_ERROR_SUCCESS;
}

eDEVError_s IDN_close (struct peripheral_descriptor_s* const this)
{
	return DEV_ERROR_SUCCESS;
}

