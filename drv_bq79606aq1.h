/*!
	@file   drv_bq79606aq1.h
	@brief  <brief description here>
	@t.odo	-
	---------------------------------------------------------------------------

	MIT License
	Copyright (c) 2022 Io. D (devcoons)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/

#ifndef DRV_BQ79606AQ1_H_
#define DRV_BQ79606AQ1_H_

/******************************************************************************
* Includes
******************************************************************************/

#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "definitions.h"

#if __has_include("drv_uart.h") && __has_include("drv_gpio.h")
	#include "drv_uart.h"
	#include "drv_gpio.h"
	#define DRV_BQ79606AQ1_ENABLED
#endif


#ifdef DRV_BQ79606AQ1_ENABLED
/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

#if !defined(ENUM_I_STATUS)
#define ENUM_I_STATUS
typedef enum
{
	I_OK 			= 0x00,
	I_INVALID 		= 0x01,
	I_EXISTS 		= 0x02,
	I_NOTEXISTS 	= 0x03,
	I_FAILED 		= 0x04,
	I_EXPIRED 		= 0x05,
	I_UNKNOWN 		= 0x06,
	I_INPROGRESS 	= 0x07,
	I_IDLE			= 0x08,
	I_FULL			= 0x09,
	I_EMPTY			= 0x0A,
	I_YES			= 0x0B,
	I_NO			= 0x0C,
	I_SKIP			= 0x0D,
	I_DEBUG_01 		= 0xE0,
	I_DEBUG_02 		= 0xE1,
	I_DEBUG_03 		= 0xE2,
	I_DEBUG_04 		= 0xE3,
	I_DEBUG_05 		= 0xE4,
	I_DEBUG_06 		= 0xE5,
	I_DEBUG_07 		= 0xE6,
	I_DEBUG_08 		= 0xE7,
	I_DEBUG_09 		= 0xE8,
	I_DEBUG_10 		= 0xE9,
	I_DEBUG_11 		= 0xEA,
	I_DEBUG_12 		= 0xEB,
	I_DEBUG_13 		= 0xEC,
	I_DEBUG_14 		= 0xED,
	I_DEBUG_15 		= 0xEE,
	I_DEBUG_16 		= 0xEF,
	I_MEMUNALIGNED 	= 0xFD,
	I_NOTIMPLEMENTED= 0xFE,
	I_ERROR 		= 0xFF
	}i_status;
#endif

typedef enum
{
	bq79_rqtp_sdr 	= 0x00,
	bq79_rqtp_sdw 	= 0x01,
	bq79_rqtp_sr 	= 0x02,
	bq79_rqtp_sw 	= 0x03,
	bq79_rqtp_br 	= 0x04,
	bq79_rqtp_bw 	= 0x05,
	bq79_rqtp_bwr 	= 0x06,
}bq79_rqtp_e;

typedef enum
{
	bq79_dtsz_1 	= 0x00,
	bq79_dtsz_2 	= 0x01,
	bq79_dtsz_3 	= 0x02,
	bq79_dtsz_4 	= 0x03,
	bq79_dtsz_5 	= 0x04,
	bq79_dtsz_6 	= 0x05,
	bq79_dtsz_7 	= 0x06,
	bq79_dtsz_8 	= 0x06,
}bq79_rqds_e;

typedef enum
{
	bq79_rg_DEVADD_OTP_0000 		= 0x0000,
	bq79_rg_CONFIG_0001 			= 0x0001,
	bq79_rg_COMM_CTRL_0020 			= 0x0020,
	bq79_rg_DAISY_CHAIN_CTRL_0021 	= 0x0021,
	bq79_rg_COMM_TO_0023 			= 0x0023,
	bq79_rg_AUX_ADC_CONF_0026 		= 0x0026,

	bq79_rg_GPIO_ADC_CONF_0028		= 0x0028,

	bq79_rg_GPIO1_CONF_002F 		= 0x002F,
	bq79_rg_GPIO2_CONF_0030 		= 0x0030,
	bq79_rg_GPIO3_CONF_0030 		= 0x0031,
	bq79_rg_GPIO4_CONF_0030 		= 0x0032,
	bq79_rg_GPIO5_CONF_0030 		= 0x0033,
	bq79_rg_GPIO6_CONF_0030 		= 0x0034,

	bq79_rg_DEVADD_USR_0104 		= 0x0104,
	bq79_rg_CONTROL1_0105 			= 0x0105,
	bq79_rg_CONTROL2_0106 			= 0x0106,
	bq79_rg_OTP_PROG_CTRL_0107 		= 0x0107,
	bq79_rg_GPIO_OUT_0108 			= 0x0108,
	bq79_rg_CELL_ADC_CTRL_0109 		= 0x0109,
	bq79_rg_AUX_ADC_CTRL1_010A		= 0x010A,
	bq79_rg_AUX_ADC_CTRL2_010B		= 0x010B,
	bq79_rg_AUX_ADC_CTRL3_010C		= 0x010C,

	bq79_rg_CB_SW_EN_0115			= 0x0115,

	bq79_rg_SPI_CFG_0154			= 0x0154,
	bq79_rg_SPI_TX_0155 			= 0x0155,
	bq79_rg_SPI_EXE_0156			= 0x0156,

	bq79_rg_DEV_STAT_0204			= 0x0204,

	bq79_rg_VCELL1_HF_0207 			= 0x0215,// = 0x0207,   //= 0x0215,//
	bq79_rg_VCELL1_LF_0208 			= 0x0216,//= 0x0208,    //= 0x0216,//
	bq79_rg_VCELL2_HF_0209 			= 0x0217,//= 0x0209,    //= 0x0217,//
	bq79_rg_VCELL2_LF_020A 			= 0x0218,//= 0x020A,    //= 0x0218,//
	bq79_rg_VCELL3_HF_020B 			= 0x0219,//= 0x020B,    //= 0x0219,//
	bq79_rg_VCELL3_LF_020C 			= 0x0220,//= 0x020C,    //= 0x0220,//
	bq79_rg_VCELL4_HF_020D 			= 0x0221,//= 0x020D,    //= 0x0221,//
	bq79_rg_VCELL4_LF_020E 			= 0x0222,//= 0x020E,    //= 0x0222,//
	bq79_rg_VCELL5_HF_020F 			= 0x0223,//= 0x020F,    //= 0x0223,//
	bq79_rg_VCELL5_LF_0210 			= 0x0224,//= 0x0210,    //= 0x0224,//
	bq79_rg_VCELL6_HF_0211 			= 0x0225,//= 0x0211,    //= 0x0225,//
	bq79_rg_VCELL6_LF_0212 			= 0x0226,//= 0x0212,    //= 0x0226,//

	bq79_rg_AUX_GPIO1H_022D 		= 0x022D,
	bq79_rg_AUX_GPIO1L_022E 		= 0x022E,
	bq79_rg_AUX_GPIO2H_022F 		= 0x022F,
	bq79_rg_AUX_GPIO2L_0230 		= 0x0230,
	bq79_rg_AUX_GPIO3H_0231 		= 0x0231,
	bq79_rg_AUX_GPIO3L_0232 		= 0x0232,
	bq79_rg_AUX_GPIO4H_0233 		= 0x0233,
	bq79_rg_AUX_GPIO4L_0234 		= 0x0234,
	bq79_rg_AUX_GPIO5H_0235 		= 0x0235,
	bq79_rg_AUX_GPIO5L_0236 		= 0x0236,
	bq79_rg_AUX_GPIO6H_0237 		= 0x0237,
	bq79_rg_AUX_GPIO6L_0238 		= 0x0238,

	bq79_rg_DIE_TEMPH_023B 			= 0x023B,
	bq79_rg_DIE_TEMPL_023C 			= 0x023C,

	bq79_rg_SPI_RX_0260             = 0x0260,

}bq79_regs_e;

typedef struct
{
	uint8_t dev;
	bq79_regs_e reg;
	uint8_t data[128];
	uint8_t data_sz;
	uint16_t crc;
}bq79606_resp_t;

typedef struct
{
	bq79_rqtp_e rqtp;
	bq79_regs_e reg;
	uint8_t dev;
	uint8_t data[8];
	bq79_rqds_e dtsz;
}bq79606_cmd_t;

typedef struct
{
	uint8_t id;
	uint8_t direction : 2;
	uint8_t is_base : 1;
	uint8_t is_tos : 1;
	uint8_t lut;
	int16_t ntemp;
	int16_t ctmp[6];
	int16_t cvlt[6];
}bq79606_node_t;

typedef struct
{
	uint8_t devs_sz;
	uint8_t db_dev_sz;
	uint8_t d0_devs_sz;
	uint8_t d1_devs_sz;
	uint8_t sz_mismatch;
	uint8_t is_ring;
	uint8_t _w_resp;
	uint8_t* _out;
	uint32_t _out_sz;
	uint8_t* _in;
	uint32_t _in_sz;
	uint8_t _cdir;
}bq79606_hinfo_t;

typedef struct
{
	uart_t* uart;
	uint32_t baudrate;
	gpio_t* wkup;
	gpio_t* nfault;
	uint8_t exp_devs_sz;
}bq79606_config_t;

typedef struct
{
	uint8_t idx;
	uint8_t matrix_tb[6][4];
}bq79_mx_t;

typedef struct
{
	bq79606_config_t config;
	bq79606_hinfo_t hinfo;
	bq79606_node_t nodes[64];
	bq79_mx_t bal_table[64];

}bq79606_t;

/******************************************************************************
* Declaration | Public Functions
******************************************************************************/

i_status bq79606aq1_initialize(bq79606_t*);
i_status bq79606aq1_wakeup(bq79606_t*);
i_status bq79606aq1_wakeup_lite(bq79606_t* h);
i_status bq79606aq1_shutdown(bq79606_t*);
i_status bq79606aq1_sftreset(bq79606_t*);
i_status bq79606aq1_cnr(bq79606_t* h);
i_status bq79606aq1_gcvlt(bq79606_t* h);

i_status bq79606aq1_evltr(bq79606_t* h);
i_status bq79606aq1_gdtmp(bq79606_t* h);
i_status bq79606aq1_gctmp(bq79606_t* h);

i_status bq79606_cmd(bq79606_t* b, bq79606_cmd_t *c);
i_status bq79606_cmdt(bq79606_t* b, bq79606_cmd_t *c, int trials);


i_status bq79606aq1_scb(bq79606_t* b,uint8_t slave);
i_status bq79606aq1_stBal(bq79606_t* b,uint8_t* s);

void test_bq(bq79606_t* b);
/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
#endif
