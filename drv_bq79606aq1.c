/*!
	@file   drv_bq79606aq1.c
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

/******************************************************************************
* Includes
******************************************************************************/

#include "drv_bq79606aq1.h"

#ifdef DRV_BQ79606AQ1_ENABLED
/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

static bq79606_t* bq79_interfaces[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static uint32_t bq79_interfaces_cnt = 0;

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

static void uart_callback(UART_HandleTypeDef *huart, uint8_t* buffer, uint32_t size);

static i_status bq79606aq1_gen_cfr(uint8_t*, uint32_t*, bq79_rqtp_e,
									bq79_rqds_e, uint8_t, uint16_t, uint8_t*);

static i_status bq79606aq1_act_ccd(uart_t *h, uint32_t brate);
static i_status bq79606aq1_act_s2a(uart_t * h);
static i_status bq79606aq1_act_cr(uart_t * h);
static i_status bq79606aq1_act_bdwup(gpio_t* wkup);
static i_status bq79606aq1_cmd_cct(bq79606_t* h);
static i_status bq79606aq1_cmd_shw(bq79606_t* h);
static i_status bq79606aq1_cmd_edc(bq79606_t* h, uint8_t dir);
static i_status bq79606aq1_cmd_sda(bq79606_t* h, int s_addr ,int e_addr);
static i_status bq79606aq1_cmd_gda(bq79606_t* h, uint8_t addr);
static i_status bq79606aq1_cmd_sdawm(bq79606_t* h, uint8_t dir);
static i_status bq79606aq1_cmd_ccr(bq79606_t* h);
static i_status bq79606aq1_cmd_esdcc(bq79606_t* h);
static i_status bq79606aq1_cmd_spo(bq79606_t* h, uint8_t dir);
static i_status bq79606aq1_cmd_cfgbase(bq79606_t* h, uint8_t addr);
static i_status bq79606aq1_cmd_cfgstack(bq79606_t* h, uint8_t addr);
static i_status bq79606aq1_cmd_cfgtstack(bq79606_t* h, uint8_t addr);
static i_status bq79606aq1_cmd_dir(bq79606_t* h, uint8_t dir);
static i_status bq79606aq1_cmd_chdir(bq79606_t* h, uint8_t dir);
static i_status bq79606aq1_cmd_dirall(bq79606_t* h, uint8_t dir);
static i_status bq79606aq1_cmd_wk(bq79606_t* h, uint8_t dir);
/******************************************************************************
* Definition  | Static Functions
******************************************************************************/



void test_bq(bq79606_t* h)
{
	bq79606aq1_shutdown(h);
		vTaskDelay(100);
		if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
		vTaskDelay(92);
		if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
		vTaskDelay(2);
		if(bq79606aq1_act_s2a(h->config.uart)!= I_OK) return I_ERROR;
		for(int k=0;k<5000;k++)	__NOP();

		bq79606aq1_cmd_edc(h, 0);
		bq79606aq1_cmd_edc(h, 2);
		for(int k=0;k<5000;k++)	__NOP();
		bq79606aq1_cmd_esdcc(h);
		for(int k=0;k<15000;k++)__NOP();
		bq79606aq1_cmd_dir(h,2);
		for(int k=0;k<5000;k++)	__NOP();
		bq79606aq1_cmd_dirall(h,2);
		for(int k=0;k<5000;k++)	__NOP();
		if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
			vTaskDelay(92);
			if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
			vTaskDelay(2);
			if(bq79606aq1_act_s2a(h->config.uart)!= I_OK) return I_ERROR;
			for(int k=0;k<5000;k++)	__NOP();
		bq79606_cmd_t read_dchain =
		{
				.data = {0x00,0,0,0,0,0,0,0},
				.reg = bq79_rg_DAISY_CHAIN_CTRL_0021,
				.rqtp = bq79_rqtp_sdr,
				.dtsz = bq79_dtsz_1
		};

		bq79606_cmd(h, &read_dchain);
		vTaskDelay(10);

		bq79606aq1_cmd_chdir(h,2);
		vTaskDelay(10);


		bq79606_cmd(h, &read_dchain);


		if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
		vTaskDelay(92);
		if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
		vTaskDelay(2);
		if(bq79606aq1_act_s2a(h->config.uart)!= I_OK) return I_ERROR;
		for(int k=0;k<5000;k++)	__NOP();
}








/////////////////////////////////////



static void uart_callback(UART_HandleTypeDef *huart, uint8_t* buffer, uint32_t size)
{
	static bq79606_t* _b = NULL;
	for(register uint32_t i=0;i<bq79_interfaces_cnt;i++)
		if(bq79_interfaces[i]->config.uart->huart == huart)
			_b = bq79_interfaces[i];

	if(_b == NULL)
		return;

	if(size < 3)
	{
		_b->hinfo._w_resp = 0xFF;
		return;
	}

	memmove(_b->hinfo._in,buffer,size);
	_b->hinfo._in_sz = size;
	_b->hinfo._w_resp = 0;
}
/******************************************************************************/
static i_status bq79606aq1_gen_cfr(uint8_t* out,uint32_t* out_size,
									bq79_rqtp_e req_type, bq79_rqds_e data_size,
									uint8_t dev_addr, uint16_t reg_addr, uint8_t* data)
{
	uint8_t daddr = req_type <= 3 ? 1 : 0;
	memset(out,0,*out_size);
	out[0] = 0x80 | (req_type << 4) | data_size;
	out[1] = dev_addr;
	out[daddr+1] = (reg_addr & 0xFF00) >> 8;
	out[daddr+2] = (reg_addr & 0x00FF) >> 0;
	memmove(&out[daddr+3],data,data_size+1);
	crc16_ibm_to_array_msb(&out[daddr+data_size+4], 0xffff, out, daddr+data_size+4);
	*out_size = daddr+data_size+6;
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_act_ccd(uart_t *h, uint32_t brate)
{
	uart_send_lowpulse(h,
			floor((18*10000000)/brate));
	uint32_t ctick = HAL_GetTick();
	while(h->send_custom_low != 0)
	{
		if(iTIMEOUT(HAL_GetTick(),ctick,2) == 1)
			return I_ERROR;
		__NOP();
	}
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_act_s2a(uart_t * h)
{
	uart_send_lowpulse(h, 300);
	uint32_t ctick = HAL_GetTick();
	while(h->send_custom_low != 0)
	{
		if(iTIMEOUT(HAL_GetTick(),ctick,2) == 1)
			return I_ERROR;
		__NOP();
	}
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_act_cr(uart_t * h)
{
	uint32_t ctick = HAL_GetTick();
	uart_send_lowpulse(h, 500);
	while(h->send_custom_low != 0)
	{
		if(iTIMEOUT(HAL_GetTick(),ctick,2) == 1)
			return I_ERROR;
		__NOP();
	}
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_act_bdwup(gpio_t* wkup)
{
	gpio_set_state(wkup, GPIO_PIN_SET);
	gpio_send_lowpulse(wkup, 270);
	uint32_t ctick = HAL_GetTick();
	while(wkup->_send_custom_low != 0)
	{
		if(iTIMEOUT(HAL_GetTick(),ctick,2) == 1)
			return I_ERROR;
		__NOP();
	}
	gpio_set_state(wkup, GPIO_PIN_SET);
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_cmd_brcfg(bq79606_t* h)
{
	bq79606_cmd_t c =
	{
			.data = {0x0},
			.dev = 0,
			.reg = bq79_rg_CONFIG_0001,
			.rqtp = bq79_rqtp_br,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_cfgbase(bq79606_t* h, uint8_t addr)
{
	bq79606_cmd_t c =
	{
			.data = {0},
			.dev = addr,
			.reg = bq79_rg_CONFIG_0001,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_cfgstack(bq79606_t* h, uint8_t addr)
{
	bq79606_cmd_t c =
	{
			.data = {0x2,0,0,0,0,0,0,0},
			.dev = addr,
			.reg = bq79_rg_CONFIG_0001,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_cfgtstack(bq79606_t* h, uint8_t addr)
{
	bq79606_cmd_t c =
	{
			.data = {0x3,0,0,0,0,0,0,0},
			.dev = addr,
			.reg = bq79_rg_CONFIG_0001,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_cct(bq79606_t* h)
{
	bq79606_cmd_t c =
	{
			.data = {0x3A,0,0,0,0,0,0,0}, //0x3A 2 sec -0x3b 10 sec - 39 0.1 sec
			.dev = 0,
			.reg = bq79_rg_COMM_TO_0023,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_spo(bq79606_t* h, uint8_t dir)
{
	bq79606_cmd_t c =
	{
			.data = {dir == 1 ? 0x20 : 0xA0,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_COMM_TO_0023,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_shtdw(bq79606_t* h, uint8_t dev,uint8_t dir)
{
	bq79606_cmd_t c =
	{
			.data = {dir == 2 ? 0x88 : 0x08, 0, 0, 0, 0, 0, 0, 0},
			.dev = dev,
			.reg = bq79_rg_CONTROL1_0105,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	i_status r = bq79606_cmd(h,&c);
	vTaskDelay(1);
	return r;
}
/******************************************************************************/
static i_status bq79606aq1_cmd_edc(bq79606_t* h, uint8_t dir)
{
	bq79606_cmd_t c =
	{
			.data = {dir == 0 ? 0 : (dir == 1 ? 0x0C : (dir == 2 ? 0x30 : 0x3C)),0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_DAISY_CHAIN_CTRL_0021,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_esdcc(bq79606_t* h)
{
	bq79606_cmd_t c =
	{
			.data = {0x40,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL2_0106,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_dsdcc(bq79606_t* h)
{
	bq79606_cmd_t c =
	{
			.data = {0x00,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL2_0106,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_ccr(bq79606_t* h) // Clear configuration register
{
	bq79606_cmd_t c =
	{
			.data = {0,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONFIG_0001,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_sdawm(bq79606_t* h, uint8_t dir) // Clear configuration register
{
	bq79606_cmd_t c =
	{
			.data = {dir == 1 ? 0x01 : 0x81,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL1_0105,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_dir(bq79606_t* h, uint8_t dir) // Clear configuration register
{
	bq79606_cmd_t c =
	{
			.data = {dir == 1 ? 0x00 : 0x80,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL1_0105,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_dirall(bq79606_t* h, uint8_t dir) // Clear configuration register
{
	bq79606_cmd_t c =
	{
			.data = {dir == 1 ? 0x00 : 0x80,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL1_0105,
			.rqtp = bq79_rqtp_bwr,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_diralln(bq79606_t* h, uint8_t dir) // Clear configuration register
{
	bq79606_cmd_t c =
	{
			.data = {dir == 1 ? 0x00 : 0x80,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL1_0105,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_sda(bq79606_t* h, int s_addr ,int e_addr) // Enum addresses
{
	bq79606_cmd_t c =
	{
			.data = {0,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_DEVADD_USR_0104,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	bq79606_cmd(h,&c);

	if(s_addr <= e_addr)
	{
		for(int i = s_addr;i<=e_addr;i++)
		{
			c.data[0] = i;
			bq79606_cmd(h,&c);
		}
	}
	else
	{
		for(int i = s_addr;i>=e_addr;i--)
		{
			c.data[0] = i;
			bq79606_cmd(h,&c);
		}
	}
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_cmd_gda(bq79606_t* h, uint8_t addr)
{
	bq79606_cmd_t c =
	{
			.data = {0,0,0,0,0,0,0,0},
			.dev = addr,
			.reg = bq79_rg_DEVADD_USR_0104,
			.rqtp = bq79_rqtp_sdr,
			.dtsz = bq79_dtsz_1
	};
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
static i_status bq79606aq1_cmd_chdir(bq79606_t* h, uint8_t dir)
{
	dir = (dir != 1 && dir !=2) ? 1 : dir;

	if(h->hinfo._cdir == dir)
		return I_OK;

	h->hinfo._cdir = dir;

	bq79606aq1_cmd_edc(h, 0);
	for(int k=0;k<3000;k++)	__NOP();
	bq79606aq1_cmd_edc(h, dir);
	for(int k=0;k<3000;k++)	__NOP();
	bq79606aq1_cmd_esdcc(h);
	for(int k=0;k<3000;k++)__NOP();
	bq79606aq1_cmd_dir(h,dir);
	for(int k=0;k<3000;k++)	__NOP();
	bq79606aq1_cmd_dirall(h,dir);
	for(int k=0;k<9000;k++)	__NOP();
	if(bq79606aq1_act_ccd(h->config.uart,h->config.baudrate)!= I_OK)	return I_ERROR;
	for(int k=0;k<7000;k++)	__NOP();
	if(bq79606aq1_cmd_cct(h)!= I_OK)	return I_ERROR;
	for(int k=0;k<9000;k++)	__NOP();
	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_cmd_wk(bq79606_t* h, uint8_t dir)
{
	dir = (dir != 1 && dir !=2) ? 1 : dir;

	bq79606aq1_cmd_edc(h, 0);
	for(int k=0;k<3000;k++)	__NOP();
	bq79606aq1_cmd_edc(h, dir);
	for(int k=0;k<5000;k++)	__NOP();
	bq79606aq1_cmd_esdcc(h);
	for(int k=0;k<15000;k++)__NOP();
	bq79606aq1_cmd_dir(h,dir);
	for(int k=0;k<5000;k++)	__NOP();
	bq79606aq1_cmd_dirall(h,dir);
	for(int k=0;k<5000;k++)	__NOP();
	return I_OK;
}

static i_status bq79606aq1_get_node(bq79606_t* h,uint8_t id, bq79606_node_t** n)
{
	if(id == 0)
	{
		*n = &h->nodes[0];
		return I_OK;
	}

	for(int i=1;i<=(h->hinfo.d0_devs_sz+h->hinfo.d1_devs_sz);i++)
	{
		if(h->nodes[i].id == id && h->nodes[i].direction!=0)
		{
			*n = &h->nodes[i];
			return I_OK;
		}
	}
	return I_ERROR;
}

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

i_status bq79606aq1_initialize(bq79606_t* h)
{
	memset(h->nodes,0,64*sizeof(bq79606_node_t));
	h->hinfo.is_ring = 0;

	for(register uint32_t i=0;i<bq79_interfaces_cnt;i++)
		if(bq79_interfaces[i] == h)
			return I_OK;

	uart_callback_add(h->config.uart, uart_callback);

	bq79_interfaces[bq79_interfaces_cnt] = h;
	bq79_interfaces_cnt++;

	return I_OK;
}
/******************************************************************************/
static i_status bq79606aq1_cmd_enumnd(bq79606_t* h,int dir, uint8_t saddr, uint8_t eaddr, uint8_t* ndevs)
{
	if(bq79606aq1_cmd_ccr(h)!= I_OK)	return I_ERROR;

	bq79606aq1_cmd_sdawm(h,dir);

	if(bq79606aq1_cmd_sda(h,saddr,eaddr)!= I_OK)	return I_ERROR;

	*ndevs = 0;

	if(saddr<=eaddr)
	{
		for(int i=saddr;i<=eaddr;i++)
		{
			if(bq79606aq1_cmd_gda(h,i) != I_OK)
				break;
			*ndevs = *ndevs + 1;
		}
	}
	else
	{
		for(int i=saddr;i>=eaddr;i--)
		{
			if(bq79606aq1_cmd_gda(h,i) != I_OK)
				break;
			*ndevs = *ndevs + 1;
		}
	}
	return I_OK;
}
/******************************************************************************/
i_status bq79606aq1_wakeup(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	bq79606aq1_shutdown(h);
	vTaskDelay(100);
	memset(h->nodes,0,64*sizeof(bq79606_node_t));
	if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
	vTaskDelay(92);
	if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;
	vTaskDelay(2);
	if(bq79606aq1_act_s2a(h->config.uart)!= I_OK) return I_ERROR;
	for(int k=0;k<5000;k++)	__NOP();
	if(bq79606aq1_act_ccd(h->config.uart, h->config.baudrate)!= I_OK)	return I_ERROR;
	for(int k=0;k<15000;k++)	__NOP();
	if(bq79606aq1_cmd_cct(h)!= I_OK)	return I_ERROR;
	vTaskDelay(1);

	// Enumerate normdir devices(nodes)
	h->hinfo.d0_devs_sz = 0;
	h->hinfo.d1_devs_sz = 0;
	bq79606aq1_cmd_enumnd(h,1,1,63,&h->hinfo.d0_devs_sz);
	for(int i=1;i<=h->hinfo.d0_devs_sz;i++)
	{
		h->nodes[i].id = i;
		h->nodes[i].direction = 1;
	}
	// Change dir and check if lastdev is reachable
	bq79606aq1_cmd_chdir(h,2);
	//if not (or no normdir devs exist) then wkup revdir and enum
	uint8_t is_broken = h->hinfo.d0_devs_sz != h->config.exp_devs_sz ? 1 : 0;
			//: (bq79606aq1_cmd_gda(h,h->hinfo.d0_devs_sz) == I_OK ? 0 : 1);
	if(is_broken == 1)
	{
		 h->hinfo.is_ring = 0;
		bq79606_cmd_t c =
		{
				.data = {0xA0,0,0,0,0,0,0,0},
				.dev = 0,
				.reg = bq79_rg_CONTROL1_0105,
				.rqtp = bq79_rqtp_bwr,
				.dtsz = bq79_dtsz_1
		};
		bq79606_cmd(h,&c);
		vTaskDelay(92);
		bq79606aq1_cmd_chdir(h,2);
		vTaskDelay(5);

		bq79606aq1_cmd_enumnd(h,2,h->hinfo.d0_devs_sz+1,63,&h->hinfo.d1_devs_sz);
		if(h->hinfo.d1_devs_sz != 0)
		{
			bq79606aq1_cmd_enumnd(h,2,h->hinfo.d1_devs_sz+h->hinfo.d0_devs_sz,h->hinfo.d0_devs_sz+1,&h->hinfo.d1_devs_sz);
			for(int i=h->hinfo.d0_devs_sz+1;i<(h->hinfo.d0_devs_sz+1+h->hinfo.d1_devs_sz);i++)
			{
				h->nodes[i].id = i;
				h->nodes[i].direction = 2;
			}
		 	bq79606aq1_cmd_brcfg(h);
		}
		bq79606aq1_cmd_chdir(h,1);
	}
	else
	{
		 h->hinfo.is_ring = 1;
		bq79606aq1_cmd_chdir(h,1);
		bq79606aq1_cmd_cfgtstack(h,h->hinfo.d0_devs_sz);
	 	for(int i=1;i<h->hinfo.d0_devs_sz;i++)
	 		bq79606aq1_cmd_cfgstack(h,i);
	 	bq79606aq1_cmd_cfgbase(h,0);
	 	bq79606aq1_cmd_brcfg(h);
	}
	return  h->hinfo.is_ring == 1 ? I_OK : I_ERROR;
}

i_status bq79606aq1_wakeup_lite(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;


	if(bq79606aq1_act_bdwup(h->config.wkup)!= I_OK) return I_ERROR;

	vTaskDelay(2);
	if(bq79606aq1_act_s2a(h->config.uart)!= I_OK) return I_ERROR;
	for(int k=0;k<5000;k++)	__NOP();
	if(bq79606aq1_act_ccd(h->config.uart, h->config.baudrate)!= I_OK)	return I_ERROR;
	for(int k=0;k<15000;k++)	__NOP();
	if(bq79606aq1_cmd_cct(h)!= I_OK)	return I_ERROR;
	vTaskDelay(1);


	return I_OK;
}

/******************************************************************************/
i_status bq79606aq1_shutdown(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	uint8_t nod = 0;
	bq79606aq1_cmd_chdir(h,1);
	bq79606aq1_cmd_enumnd(h,1,1,63,&nod);
	for(int i=nod;i>=1;i--)
		bq79606aq1_cmd_shtdw(h, i, 1);
	bq79606aq1_cmd_chdir(h,2);
	bq79606aq1_cmd_enumnd(h,2,1,63,&nod);
	for(int i=nod;i>=1;i--)
		bq79606aq1_cmd_shtdw(h, i, 2);

	bq79606aq1_cmd_shtdw(h, 0, 1);
	return I_OK;
}
/******************************************************************************/
i_status bq79606aq1_sftreset(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	bq79606_cmd_t c =
	{
			.data = {0x02,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL1_0105,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606aq1_cmd_edc(h, 0);
	bq79606aq1_cmd_edc(h, 2);
	bq79606aq1_cmd_esdcc(h);
	bq79606aq1_cmd_dir(h,2);
	bq79606aq1_cmd_dirall(h,2);

	bq79606_cmd(h,&c);
	for(int k=0;k<5000;k++)	__NOP();
	bq79606aq1_cmd_edc(h, 1);
	bq79606aq1_cmd_esdcc(h);
	bq79606aq1_cmd_dir(h,1);
	bq79606aq1_cmd_dirall(h,1);

	bq79606_cmd(h,&c);
	for(int k=0;k<5000;k++)	__NOP();

	c.rqtp = bq79_rqtp_sdr;
	return bq79606_cmd(h,&c);
}
/******************************************************************************/
i_status bq79606aq1_cnr(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	bq79606aq1_cmd_chdir(h,2);
	for(int i=0;i<64;i++)
	{
		if(h->nodes[i].direction == 2 || i==0)
		{
			h->nodes[i].lut = h->nodes[i].lut!=255 ? h->nodes[i].lut+1 : 255;
			if(bq79606aq1_cmd_gda(h,h->nodes[i].id) == I_OK)
				h->nodes[i].lut = 0;
		}
	}
	bq79606aq1_cmd_chdir(h,1);
	for(int i=0;i<64;i++)
	{
		if(h->nodes[i].direction == 1 || i==0)
		{
			h->nodes[i].lut =  h->nodes[i].lut!=255 ? h->nodes[i].lut+1 : 255;
			if(bq79606aq1_cmd_gda(h,h->nodes[i].id) == I_OK)
				h->nodes[i].lut = 0;
		}
	}

	uint32_t lut = h->nodes[0].lut;

	for(int i=1;i<64;i++)
		if(h->nodes[i].direction == 1 || h->nodes[i].direction == 2)
			lut+=h->nodes[i].lut;

	if(lut>3 || (h->hinfo.d0_devs_sz+h->hinfo.d1_devs_sz+1)!= h->config.exp_devs_sz)
	{

		uint8_t d0 =0;
		uint8_t d1 =0;
		if ((h->hinfo.d0_devs_sz+h->hinfo.d1_devs_sz+1)!= h->config.exp_devs_sz)
		{
			bq79606aq1_wakeup_lite(h);
			vTaskDelay(5);
			bq79606aq1_cmd_enumnd(h,1,1, h->config.exp_devs_sz+1,&d0);
		}
		bq79606aq1_cmd_chdir(h,1);

		bq79606aq1_cmd_enumnd(h,1,1, h->config.exp_devs_sz+1,&d0);
		for(int i=1;i<=d0;i++)
		{
			h->nodes[i].id = i;
			h->nodes[i].direction = 1;
			h->nodes[i].lut = 0;
		}

		bq79606aq1_cmd_chdir(h,2);

		uint8_t is_broken = d0 == 0 ? 1
				: (bq79606aq1_cmd_gda(h,d0) == I_OK ? 0 : 1);
		if(is_broken == 1)
		{
			bq79606aq1_cmd_enumnd(h,2,h->hinfo.d0_devs_sz+1, h->config.exp_devs_sz+2,&d1);
			if(d1 != 0)
			{
				bq79606aq1_cmd_enumnd(h,2,d1+d0,d0+1,&d1);
				for(int i=d0+1;i<(d0+1+d1);i++)
				{
					h->nodes[i].id = i;
					h->nodes[i].direction = 2;
					h->nodes[i].lut = 0;
				}
			}
		}
		bq79606aq1_cmd_chdir(h,1);

		h->hinfo.d0_devs_sz = d0;
		h->hinfo.d1_devs_sz = d1;
	}
	else
	{
		if(h->hinfo.d1_devs_sz !=0 )
		{
			uint8_t mx_rid = 0;
			for(int i=0;i<64;i++)
			{
				if(h->nodes[i].direction == 2 &&  mx_rid < h->nodes[i].id)
					mx_rid = h->nodes[i].id;
			}
			if(mx_rid != 0)
			{
				if(bq79606aq1_cmd_gda(h,mx_rid) == I_OK)
				{
					h->hinfo.d0_devs_sz = 0;
					h->hinfo.d1_devs_sz = 0;
					bq79606aq1_cmd_enumnd(h,1,1,63,&h->hinfo.d0_devs_sz);
					for(int i=1;i<=h->hinfo.d0_devs_sz;i++)
					{
						h->nodes[i].id = i;
						h->nodes[i].direction = 1;
					}
					bq79606aq1_cmd_chdir(h,2);
					uint8_t is_broken = h->hinfo.d0_devs_sz == 0 ? 1
							: (bq79606aq1_cmd_gda(h,h->hinfo.d0_devs_sz) == I_OK ? 0 : 1);
					if(is_broken == 1)
					{
						bq79606aq1_cmd_enumnd(h,2,h->hinfo.d0_devs_sz+1,63,&h->hinfo.d1_devs_sz);
						if(h->hinfo.d1_devs_sz != 0)
						{
							bq79606aq1_cmd_enumnd(h,2,h->hinfo.d1_devs_sz+h->hinfo.d0_devs_sz,h->hinfo.d0_devs_sz+1,&h->hinfo.d1_devs_sz);
							for(int i=h->hinfo.d0_devs_sz+1;i<(h->hinfo.d0_devs_sz+1+h->hinfo.d0_devs_sz);i++)
							{
								h->nodes[i].id = i;
								h->nodes[i].direction = 2;
							}
						}
					}
					bq79606aq1_cmd_chdir(h,1);
				}
			}
		}
	}

	h->hinfo.db_dev_sz = h->nodes[0].lut > 3 ? 0 : 1;

	h->hinfo.sz_mismatch = (h->hinfo.db_dev_sz+h->hinfo.d0_devs_sz+h->hinfo.d1_devs_sz) != h->config.exp_devs_sz;

	h->hinfo.is_ring = h->hinfo.sz_mismatch == 1 ? 0 : ((h->hinfo.db_dev_sz+h->hinfo.d0_devs_sz == h->config.exp_devs_sz) ? 1 : 0 );
	return I_OK;
}
/******************************************************************************/

i_status bq79606aq1_evltr(bq79606_t* h)
{
	bq79606_cmd_t c0 =
	{
			.data = {0x53,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL2_0106,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};


	bq79606_cmd_t c1 =
	{
			.data = {0x3f,0xF0,0x03,0x02,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CELL_ADC_CTRL_0109,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_4
	};

	bq79606_cmd_t c2 =
	{
			.data = {0x20,0x20,0x20,0x20,0x20,0x20,0,0},
			.dev = 0,
			.reg = bq79_rg_GPIO1_CONF_002F,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_6
	};

	bq79606_cmd_t c3 =
	{
			.data = {0x3F,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_GPIO_ADC_CONF_0028,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c4 =
	{
			.data = {0x0C,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_AUX_ADC_CONF_0026,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	if(h->hinfo.d1_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,2);
		bq79606_cmd(h,&c0);

		bq79606_cmd(h,&c2);
		bq79606_cmd(h,&c3);
		bq79606_cmd(h,&c4);

		bq79606_cmd(h,&c1);
//		bq79606_cmd(h,&c0);
	}

	if(h->hinfo.d0_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,1);
		bq79606_cmd(h,&c0);

		bq79606_cmd(h,&c2);
		bq79606_cmd(h,&c3);
		bq79606_cmd(h,&c4);

		bq79606_cmd(h,&c1);
	}
	vTaskDelay(2);
	return I_OK;
}

/******************************************************************************/
i_status bq79606aq1_gdtmp(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	bq79606_node_t *nd = NULL;

	bq79606_cmd_t c =
	{
			.data = {0x01,0,0,0,0,0,0,0},
			.reg = bq79_rg_DIE_TEMPH_023B,
			.rqtp = bq79_rqtp_sdr,
			.dtsz = bq79_dtsz_1
	};

	//get 0 dev vgts;
	c.dev = 0;
	if(bq79606_cmdt(h,&c,2) == I_OK)
	{
		uint32_t rem_sz = h->hinfo._in_sz;
		while(rem_sz !=0)
		{
			uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
			p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			if(p_sz > (h->hinfo._in_sz-6))	break;
			if(p_id != 0)				break;
			if(p_rg != 0x023B)			break;

			h->nodes[0].ntemp = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
			h->nodes[0].ntemp = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			nd->lut = 0;
			rem_sz=rem_sz-2;
		}
	}
	else
	{
		h->nodes[0].lut = h->nodes[0].lut + 1;
	}

	// get rev side vtgs
	if(h->hinfo.d1_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,2);
		for(int k=0;k<5000;k++)	__NOP();
		for(int i=0;i<64;i++)
		{
			if(h->nodes[i].direction == 2)
			{
				c.dev = h->nodes[i].id;
				if(bq79606_cmdt(h,&c,2) == I_OK)
				{
					uint32_t rem_sz = h->hinfo._in_sz;
					while(rem_sz !=0)
					{
						uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
						p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						if(p_sz > (h->hinfo._in_sz-6))	break;
						if(p_id > 63)				break;
						if(p_rg != 0x023B)			break;
						h->nodes[i].ntemp = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
						h->nodes[i].ntemp = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						nd->lut = 0;
						rem_sz=rem_sz-2;
					}
				}
				else
				{
					h->nodes[i].lut = h->nodes[i].lut + 1;
				}
			}
		}
	}

	// get norm side vtgs
	if(h->hinfo.d0_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,1);
		for(int k=0;k<5000;k++)	__NOP();
		for(int i=0;i<64;i++)
		{
			if(h->nodes[i].direction == 1)
			{
				c.dev = h->nodes[i].id;
				if(bq79606_cmdt(h,&c,2) == I_OK)
				{
					uint32_t rem_sz = h->hinfo._in_sz;
					while(rem_sz !=0)
					{
						uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
						p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						if(p_sz > (h->hinfo._in_sz-6))	break;
						if(p_id > 63)				break;
						if(p_rg != 0x023B)			break;
						h->nodes[i].ntemp = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
						h->nodes[i].ntemp = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
						nd->lut = 0;
						rem_sz=rem_sz-2;
					}
				}
				else
				{
					h->nodes[i].lut = h->nodes[i].lut + 1;
				}
			}
		}
	}
	return I_OK;
}

/******************************************************************************/

i_status bq79606aq1_gctmp(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	bq79606_node_t *nd = NULL;
	uint16_t tmpcrc = 0;
	bq79606_cmd_t c =
	{
			.data = {0x0B,0,0,0,0,0,0,0},
			.reg = bq79_rg_AUX_GPIO1H_022D,
			.rqtp = bq79_rqtp_sdr,
			.dtsz = bq79_dtsz_1
	};

	//get 0 dev vgts;
	c.dev = 0;
	if(bq79606_cmdt(h,&c,2) == I_OK)
	{
		tmpcrc = crc16_ibm(0xFFFF,h->hinfo._in, 16);
		if(((tmpcrc&0x00FF) == h->hinfo._in[17])&&(((tmpcrc>>8)&0x00FF) == h->hinfo._in[16]))
		{
			uint32_t rem_sz = h->hinfo._in_sz;
			while(rem_sz !=0)
			{
				uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
				uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
				uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
				p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
				if(p_sz > (h->hinfo._in_sz-6))				break;
				if(p_id > 63)				break;
				if(p_rg != bq79_rg_AUX_GPIO1H_022D)			break;
				for(int cv = 0; cv<((p_sz+1)/2);cv++)
				{
					h->nodes[0].ctmp[cv] = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
					h->nodes[0].ctmp[cv] |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
				}
				nd->lut = 0;
				rem_sz=rem_sz-2;
			}
		}
	}
	else
	{
		h->nodes[0].lut = h->nodes[0].lut + 1;
	}

	// get rev side vtgs
	if(h->hinfo.d1_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,2);
		for(int k=0;k<5000;k++)	__NOP();
		for(int i=0;i<64;i++)
		{
			if(h->nodes[i].direction == 2)
			{
				c.dev = h->nodes[i].id;
				if(bq79606_cmdt(h,&c,2) == I_OK)
				{
					tmpcrc = crc16_ibm(0xFFFF,h->hinfo._in, 16);
					if(((tmpcrc&0x00FF) == h->hinfo._in[17])&&(((tmpcrc>>8)&0x00FF) == h->hinfo._in[16]))
					{
						uint32_t rem_sz = h->hinfo._in_sz;
						while(rem_sz !=0)
						{
							uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
							p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							if(p_sz > (h->hinfo._in_sz-6))	break;
							if(p_id > 63)					break;
							if(p_rg != bq79_rg_AUX_GPIO1H_022D)				break;
							for(int cv = 0; cv<((p_sz+1)/2);cv++)
							{
								h->nodes[i].ctmp[cv] = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
								h->nodes[i].ctmp[cv] |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							}
							nd->lut = 0;
							rem_sz=rem_sz-2;
						}
					}
				}
				else
				{
					h->nodes[i].lut = h->nodes[i].lut + 1;
				}
			}
		}
	}

	// get norm side vtgs
	if(h->hinfo.d0_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,1);
		for(int k=0;k<5000;k++)	__NOP();
		for(int i=0;i<64;i++)
		{
			if(h->nodes[i].direction == 1)
			{
				c.dev = h->nodes[i].id;
				if(bq79606_cmdt(h,&c,2) == I_OK)
				{
					tmpcrc = crc16_ibm(0xFFFF,h->hinfo._in, 16);
					if(((tmpcrc&0x00FF) == h->hinfo._in[17])&&(((tmpcrc>>8)&0x00FF) == h->hinfo._in[16]))
					{
						uint32_t rem_sz = h->hinfo._in_sz;
						while(rem_sz !=0)
						{
							uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
							p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							if(p_sz > (h->hinfo._in_sz-6))	break;
							if(p_id > 63)					break;
							if(p_rg != bq79_rg_AUX_GPIO1H_022D)			break;
							for(int cv = 0; cv<((p_sz+1)/2);cv++)
							{
								h->nodes[i].ctmp[cv] = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
								h->nodes[i].ctmp[cv] |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							}
							nd->lut = 0;
							rem_sz=rem_sz-2;
						}
					}

				}
				else
				{
					h->nodes[i].lut = h->nodes[i].lut + 1;
				}
			}
		}
	}
	return I_OK;
}

i_status bq79606aq1_gcvlt(bq79606_t* h)
{
	if(h == NULL)
		return I_ERROR;

	bq79606_node_t *nd = NULL;

	bq79606_cmd_t c =
	{
			.data = {0x0B,0,0,0,0,0,0,0},
			.reg = bq79_rg_VCELL1_HF_0207,
			.rqtp = bq79_rqtp_sdr,
			.dtsz = bq79_dtsz_1
	};

	//get 0 dev vgts;
	c.dev = 0;
	uint16_t tmpcrc = 0;
	if(bq79606_cmdt(h,&c,2) == I_OK)
	{
		uint32_t rem_sz = h->hinfo._in_sz;
		tmpcrc = crc16_ibm(0xFFFF,h->hinfo._in, 16);
		if(((tmpcrc&0x00FF) == h->hinfo._in[17])&&(((tmpcrc>>8)&0x00FF) == h->hinfo._in[16]))
		{
			while(rem_sz !=0)
			{
			uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
			p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			if(p_sz > (h->hinfo._in_sz-6))				break;
			if(p_id > 63)				break;
			if(p_rg != bq79_rg_VCELL1_HF_0207)			break;
			for(int cv = 0; cv<((p_sz+1)/2);cv++)
			{
				h->nodes[0].cvlt[cv] = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
				h->nodes[0].cvlt[cv] |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
			}
			nd->lut = 0;
			rem_sz=rem_sz-2;
			}
		}
	}
	else
	{
		h->nodes[0].lut = h->nodes[0].lut + 1;
	}

	// get rev side vtgs
	if(h->hinfo.d1_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,2);
		for(int k=0;k<5000;k++)	__NOP();
		for(int i=0;i<64;i++)
		{
			if(h->nodes[i].direction == 2)
			{
				c.dev = h->nodes[i].id;
				if(bq79606_cmdt(h,&c,2) == I_OK)
				{
					tmpcrc = crc16_ibm(0xFFFF,h->hinfo._in, 16);
					if(((tmpcrc&0x00FF) == h->hinfo._in[17])&&(((tmpcrc>>8)&0x00FF) == h->hinfo._in[16]))
					{
						uint32_t rem_sz = h->hinfo._in_sz;
						while(rem_sz !=0)
						{
							uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
							p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							if(p_sz > (h->hinfo._in_sz-6))	break;
							if(p_id > 63)					break;
							if(p_rg != bq79_rg_VCELL1_HF_0207)				break;
							for(int cv = 0; cv<((p_sz+1)/2);cv++)
							{
								h->nodes[i].cvlt[cv] = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
								h->nodes[i].cvlt[cv] |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							}
							nd->lut = 0;
							rem_sz=rem_sz-2;
						}
					}
				}
				else
				{
					h->nodes[i].lut = h->nodes[i].lut + 1;
				}
			}
		}
	}

	// get norm side vtgs
	if(h->hinfo.d0_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,1);
		for(int k=0;k<5000;k++)	__NOP();
		for(int i=0;i<64;i++)
		{
			if(h->nodes[i].direction == 1)
			{
				c.dev = h->nodes[i].id;
				if(bq79606_cmdt(h,&c,2) == I_OK)
				{
					tmpcrc = crc16_ibm(0xFFFF,h->hinfo._in, 16);
					if(((tmpcrc&0x00FF) == h->hinfo._in[17])&&(((tmpcrc>>8)&0x00FF) == h->hinfo._in[16]))
					{
						uint32_t rem_sz = h->hinfo._in_sz;
						while(rem_sz !=0)
						{
							uint8_t p_sz = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint8_t p_id = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							uint16_t p_rg = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
							p_rg |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							if(p_sz > (h->hinfo._in_sz-6))	break;
							if(p_id > 63)					break;
							if(p_rg != bq79_rg_VCELL1_HF_0207)			break;
							for(int cv = 0; cv<((p_sz+1)/2);cv++)
							{
								h->nodes[i].cvlt[cv] = h->hinfo._in[h->hinfo._in_sz - (rem_sz--)] << 8;
								h->nodes[i].cvlt[cv] |= h->hinfo._in[h->hinfo._in_sz - (rem_sz--)];
							}
							nd->lut = 0;
							rem_sz=rem_sz-2;
						}
					}
				}
				else
				{
					h->nodes[i].lut = h->nodes[i].lut + 1;
				}
			}
		}
	}
	return I_OK;
}

/******************************************************************************/
i_status bq79606_cmd(bq79606_t* b, bq79606_cmd_t *c)
{
	if(b == NULL)
		return I_ERROR;
	if(c == NULL)
		return I_ERROR;

	for(int k=0;k<100;k++)
		__NOP();
	b->hinfo._out_sz = 256;
	bq79606aq1_gen_cfr(b->hinfo._out, &b->hinfo._out_sz, c->rqtp,
			c->dtsz, c->dev, c->reg, c->data);

	if(c->rqtp == bq79_rqtp_sdw || c->rqtp == bq79_rqtp_sw || c->rqtp == bq79_rqtp_bw || c->rqtp == bq79_rqtp_bwr)
	{
		i_status r = uart_send(b->config.uart, b->hinfo._out, b->hinfo._out_sz);
		for(int k=0;k<2000;k++)
			__NOP();
		return r;
	}
	else
	{
		b->hinfo._w_resp = 0x80;
		uint32_t ctick = HAL_GetTick();
		uart_send(b->config.uart, b->hinfo._out, b->hinfo._out_sz);
		while((b->hinfo._w_resp & 0xF0) == 0x80)
		{
			if(iTIMEOUT(HAL_GetTick(),ctick,2) == 1)
				return I_INVALID;
			for(int i=0;i<1000;i++)
				__NOP();
		}
		return b->hinfo._w_resp == 0 ? I_OK : I_ERROR;
	}
}

i_status bq79606_cmdt(bq79606_t* b, bq79606_cmd_t *c, int trials)
{
	i_status r = I_ERROR;
	for(int i=0;i<trials;i++)
	{
		r = bq79606_cmd(b,c);
		if(r == I_OK)
			return I_OK;
		for(int i=0;i<150000;i++)
						__NOP();
	}

	return I_ERROR;
}

/*fede tbc*/

i_status bq79606aq1_configadc(bq79606_t* h)
{

	bq79606_cmd_t c0 =
	{
			.data = {0x3f,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CELL_ADC_CTRL_0109,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c1 =
	{ // Decimation ratio to 256, Sampling 1MHz low pass to 40Hz
			.data = {0x62,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x24,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c2 = // 1ms conversion rate
	{
			.data = {0x09,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x25,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c3 =
	{
			.data = {0x00,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x27,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c4 =
	{
			.data = {0x10,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x106,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
//
	bq79606_cmd_t c5 =
	{
			.data = {0x20,0x20,0x20,0x20,0x20,0x20,0,0},
			.dev = 0,
			.reg = 0x2F,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_6
	};

	bq79606_cmd_t c6 =
	{
			.data = {0xFD,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x10A,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c7 =
	{
			.data = {0x03,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x10B,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c8 =
	{
			.data = {0x0F,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x10C,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t c9 =
	{
			.data = {0x13,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = 0x106,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};

	if(h->hinfo.d1_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,2);

		bq79606_cmd(h,&c0);
		bq79606_cmd(h,&c1);
		bq79606_cmd(h,&c2);
		bq79606_cmd(h,&c3);
		bq79606_cmd(h,&c4);
		bq79606_cmd(h,&c5);
		bq79606_cmd(h,&c6);
		bq79606_cmd(h,&c7);
		bq79606_cmd(h,&c8);
		bq79606_cmd(h,&c9);

	}

	if(h->hinfo.d0_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,1);
		bq79606_cmd(h,&c0);
		bq79606_cmd(h,&c1);
		bq79606_cmd(h,&c2);
		bq79606_cmd(h,&c3);
		bq79606_cmd(h,&c4);
		bq79606_cmd(h,&c5);
		bq79606_cmd(h,&c6);
		bq79606_cmd(h,&c7);
		bq79606_cmd(h,&c8);
		bq79606_cmd(h,&c9);
	}
	vTaskDelay(2);
	return I_OK;
}

i_status bq79606aq1_resetADC(bq79606_t* h)
{

	bq79606_cmd_t c0 =
	{
			.data = {0x13,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CONTROL2_0106,
			.rqtp = bq79_rqtp_bw,
			.dtsz = bq79_dtsz_1
	};
	if(h->hinfo.d1_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,2);

		bq79606_cmd(h,&c0);

	}

	if(h->hinfo.d0_devs_sz != 0)
	{
		bq79606aq1_cmd_chdir(h,1);
		bq79606_cmd(h,&c0);
	}

//	bq79606_cmd(h,&c0);
//
//	bq79606aq1_cmd_chdir(h,1);
//	for(int i=0;i<64;i++)
//	{
//		if(h->nodes[i].direction == 1)
//		{
//			c0.dev = h->nodes[i].id;
//			bq79606_cmd(h,&c0);
//		}
//	}
//
//	bq79606aq1_cmd_chdir(h,2);
//	for(int i=0;i<64;i++)
//	{
//		if(h->nodes[i].direction == 2)
//		{
//			c0.dev = h->nodes[i].id;
//			bq79606_cmd(h,&c0);
//		}
//	}

	vTaskDelay(2);
	return I_OK;
}


i_status bq79606aq1_scb(bq79606_t* b,uint8_t slave) //send config balancing
{


	bq79606_cmd_t bal =
	{
			.data = {0,0,0,0,0,0,0,0},
			.dev = slave,
			.reg = bq79_rg_CB_SW_EN_0115,
			.rqtp = bq79_rqtp_sdr,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd(b, &bal);

	return I_OK;
}


i_status bq79606aq1_balOff(bq79606_t* h) //send stop
{
	bq79606_cmd_t c0 =
	{
			.data = {0,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CB_SW_EN_0115,
			.rqtp = bq79_rqtp_sdw, // change to broadcast
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd(h,&c0);

	bq79606aq1_cmd_chdir(h,1);
	for(int i=0;i<64;i++)
	{
		if(h->nodes[i].direction == 1)
		{
			c0.dev = h->nodes[i].id;
			bq79606_cmd(h,&c0);
		}
	}

	bq79606aq1_cmd_chdir(h,2);
	for(int i=0;i<64;i++)
	{
		if(h->nodes[i].direction == 2)
		{
			c0.dev = h->nodes[i].id;
			bq79606_cmd(h,&c0);
		}
	}

	return I_OK;
}


//TOBE REMOVE
#include "lib_data.h"
uint8_t val;

uint8_t test;

i_status bq79606aq1_sndBal(bq79606_t* b,uint8_t even_odd,uint8_t count)
{
	bq79606_cmd_t bal =
	{
			.data = {0,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_CB_SW_EN_0115,
			.rqtp = bq79_rqtp_sdw, // change to broadcast
			.dtsz = bq79_dtsz_1
	};

	bq79606aq1_balOff(b);

	bq79606_node_t *c_node = NULL;


	for(uint8_t i=0; i < b->config.exp_devs_sz; i++)
	{
		bal.data[0] = 0;
		if(even_odd == 1) //dispari
		{
			bal.data[0] |= b->bal_table[i].matrix_tb[0][b->bal_table[i].idx]<<0;
			bal.data[0] |= b->bal_table[i].matrix_tb[2][b->bal_table[i].idx]<<2;
			bal.data[0] |= b->bal_table[i].matrix_tb[4][b->bal_table[i].idx]<<4;

		}
		else if(even_odd == 2) //pari
		{
			bal.data[0] |= b->bal_table[i].matrix_tb[1][b->bal_table[i].idx]<<1;
			bal.data[0] |= b->bal_table[i].matrix_tb[3][b->bal_table[i].idx]<<3;
			bal.data[0] |= b->bal_table[i].matrix_tb[5][b->bal_table[i].idx]<<5;
		}
		if((count>0) && (count%2 == 0))
		{
			b->bal_table[i].idx++;
		}

		bal.data[0] |=1<<7; // SW_EN
		data.sl_bal[i]= bal.data[0];

		bq79606aq1_get_node(b,i,&c_node);

		bal.dev = c_node->id;
		bq79606aq1_cmd_chdir(b,c_node->direction);
		bq79606_cmd(b, &bal);
	}

	return I_OK;
}


/*
 *function used to read the id by bq to eeprom
 *
 * in development
 */

i_status bq_spiWrite(bq79606_t* b,uint8_t* msg,uint8_t lung);
i_status bq_spiRead(bq79606_t* b ,uint8_t* msg,uint8_t lung);


i_status bq_readId(bq79606_t* b)
{
	uint8_t tx_msg[8];
	uint8_t rx_msg[8];

	bq79606_cmd_t gpio_1_wp =
	{
			.data = {0x14,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_GPIO1_CONF_002F, /* wp HIGH*/
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t gpio_2_hold =
	{
			.data = {0x14,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_GPIO2_CONF_0030, /* Hold HIGH */
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t gpio_1_2_H =
	{
			.data = {0x03,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_GPIO_OUT_0108, /* Hold HIGH */
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd(b,&gpio_1_wp); // as output
	bq79606_cmd(b,&gpio_2_hold);// as output
	bq79606_cmd(b, &gpio_1_2_H);// HIGH
	//OK


//
//	//1)write en  0x06
//	//2) write    0x02 + add start +byte
//	//3) write disable 0x01
//
//	//1)
//	tx_msg[0]= 0x06;
//	bq_spiWrite(b,tx_msg,1);
//
//	//2)
//	tx_msg[0]= 0x02;// WRITE 02
//	tx_msg[1]= 0x00;//add 00
//	tx_msg[2]= 0xAA;//number
//	bq_spiWrite(b,tx_msg,3);
//

	tx_msg[0]= 0x03;//cmd  READ 03
	tx_msg[1]= 0x00;//add 00

	bq_spiRead(b,tx_msg,3);


}

i_status bq_spiWrite(bq79606_t* b ,uint8_t* msg,uint8_t lung)
{

	bq79606_cmd_t spi_config =
			{
					.data = {0x08,0,0,0,0,0,0,0},
					.dev = 0,
					.reg = bq79_rg_SPI_CFG_0154,
					.rqtp = bq79_rqtp_sdw,
					.dtsz = bq79_dtsz_1
			};


	for (int i = 0; i< lung; i++)
	{


		bq79606_cmd_t msg_s =
		{
				.data[0] = msg[i],
				.dev = 0,
				.reg = bq79_rg_SPI_TX_0155,
				.rqtp = bq79_rqtp_sdw,
				.dtsz = bq79_dtsz_1
		};

		bq79606_cmd_t exe =
		{
				.data = {0x01,0,0,0,0,0,0,0},
				.dev = 0,
				.reg = bq79_rg_SPI_EXE_0156,
				.rqtp = bq79_rqtp_sdw,
				.dtsz = bq79_dtsz_1
		};

		bq79606_cmd(b,&msg_s);
		bq79606_cmd(b,&spi_config);
		bq79606_cmd(b,&exe);

	}

	spi_config.data[0] = 0x18; // metto alto il cs
	bq79606_cmd(b,&spi_config);

	return I_OK;

}

i_status bq_spiRead(bq79606_t* b,uint8_t* msg,uint8_t lung)
{

	bq79606_cmd_t spi_config =
	{
			.data = {0x08,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_SPI_CFG_0154,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t msg_cmd =
	{
			.data[0] =msg[0],
			.dev = 0,
			.reg = bq79_rg_SPI_TX_0155,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t msg_add =
	{
			.data[0] = msg[1],
			.dev = 0,
			.reg = bq79_rg_SPI_TX_0155,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};

	bq79606_cmd_t exe =
	{
			.data = {0x01,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_SPI_EXE_0156,
			.rqtp = bq79_rqtp_sdw,
			.dtsz = bq79_dtsz_1
	};


	bq79606_cmd_t msg_r =
	{
			.data ={0,0,0,0,0,0,0,0},
			.dev = 0,
			.reg = bq79_rg_SPI_RX_0260,
			.rqtp = bq79_rqtp_sdr,
			.dtsz = bq79_dtsz_1
	};


	bq79606_cmd(b,&spi_config);
	bq79606_cmd(b, &msg_cmd);
	bq79606_cmd(b, &exe);

	bq79606_cmd(b,&spi_config);
	bq79606_cmd(b, &msg_add);
	bq79606_cmd(b, &exe);

	bq79606_cmd(b,&spi_config);
	bq79606_cmd(b,&exe);

	spi_config.data[0] = 0x18; // metto alto il cs
	bq79606_cmd(b,&spi_config);

	bq79606_cmd(b, &msg_r);


	return I_OK;
}
/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
