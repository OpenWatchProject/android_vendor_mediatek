/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "lcm_drv.h"
#ifndef BUILD_LK
#include <linux/string.h>
#endif

#define REGFLAG_DELAY 0xFE
#define REGFLAG_END_OF_TABLE 0xFF

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) lcm_util.set_reset_pin(v)

#define UDELAY(n) lcm_util.udelay(n)
#define MDELAY(n) lcm_util.mdelay(n)

#define dsi_set_cmdq_V2(cmd, count, para_list, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, para_list, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define dsi_write_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define dsi_write_regs(addr, para, nums) lcm_util.dsi_write_regs(addr, para, nums)
#define dsi_dcs_read_lcm_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[32];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xF0, 1, {0xC3}},
	{0xF0, 1, {0x96}},
	{0xED, 1, {0xC3}},
	{0xE4, 2, {0x40, 0x0F}},
	{0xE7, 1, {0x83}},
	{0xC3, 4, {0x33, 0x02, 0x25, 0x04}},
	{0xE5, 14, {0xB2, 0xF5, 0xBD, 0x24, 0x22, 0x25, 0x10, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22}},
	{0xEC, 7, {0x00, 0x55, 0x00, 0x00, 0x00, 0x49, 0x22}},
	{0x36, 1, {0x0C}},
	{0xB2, 1, {0x00}},
	{0x3A, 1, {0x07}},
	{0xC5, 1, {0x6B}},
	{0xE0, 14, {0x88, 0x0B, 0x10, 0x08, 0x07, 0x03, 0x2C, 0x33, 0x43, 0x08, 0x16, 0x16, 0x2A, 0x2E}},
	{0xE1, 14, {0x88, 0x0B, 0x10, 0x08, 0x06, 0x02, 0x2B, 0x32, 0x42, 0x09, 0x16, 0x15, 0x2A, 0x2E}},
	{0xA4, 2, {0xC0, 0x7B}},
	{0xD9, 1, {0x02}},
	{0xB6, 2, {0xC7, 0x31}},
	{0xB3, 1, {0x01}},
	{0xC1, 4, {0x77, 0x07, 0xC2, 0x07}},
	{0xA5, 9, {0x00, 0x00, 0x00, 0x00, 0x20, 0x16, 0x2A, 0x8A, 0x02}},
	{0xBA, 7, {0x0A, 0x5A, 0x23, 0x10, 0x25, 0x02, 0x00}},
	{0xBB, 8, {0x00, 0x27, 0x00, 0x29, 0x82, 0x87, 0x18, 0x00}},
	{0xA6, 9, {0x00, 0x00, 0x00, 0x00, 0x20, 0x16, 0x2A, 0x8A, 0x02}},
	{0xBC, 8, {0x00, 0x27, 0x00, 0x29, 0x82, 0x87, 0x18, 0x00}},
	{0xBD, 11, {0xA1, 0xB2, 0x2B, 0x1A, 0x56, 0x43, 0x34, 0x65, 0xFF, 0xFF, 0x0F}},
	{0xF0, 1, {0x3C}},
	{0xF0, 1, {0x69}},
	{0x21, 0, {0x00}},
	{0x11 , 0 , {0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x29 , 0 , {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 0, {0x00}},
	{REGFLAG_DELAY, 5, {}},

	// Display On
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display Off
	{0x28, 0, {0x00}},

	// Sleep In
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 5, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	for (i = 0; i < count; i++) {
		struct LCM_setting_table *t = &table[i];

		switch (t->cmd) {
		case REGFLAG_DELAY:
			MDELAY(t->count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V2(t->cmd, t->count, t->para_list, force_update);
			break;
		}
	}
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;
	params->width = 400;
	params->height = 400;
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.mode = BURST_VDO_MODE;
	params->dsi.LANE_NUM = LCM_ONE_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	params->dsi.packet_size = 256;
	params->dsi.intermediat_buffer_num = 0;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count = 320 * 3;
	params->dsi.vertical_sync_active = 3;
	params->dsi.vertical_backporch = 4;
	params->dsi.vertical_frontporch = 4;
	params->dsi.vertical_active_line = 400;
	params->dsi.horizontal_sync_active = 40;
	params->dsi.horizontal_backporch = 40;
	params->dsi.horizontal_frontporch = 40;
	params->dsi.horizontal_active_pixel = 400;
	params->dsi.PLL_CLOCK = 150;
	params->dsi.ssc_disable = 1;
}

static void lcm_init(void)
{
	SET_RESET_PIN(0);
	MDELAY(120);
	SET_RESET_PIN(1);
	MDELAY(120);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

static unsigned int lcm_compare_id(void)
{
	return 1;
}

LCM_DRIVER st7797_400x400_dsi_vdo_lcm_drv = {
	.name = "st7797_400x400_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
};
