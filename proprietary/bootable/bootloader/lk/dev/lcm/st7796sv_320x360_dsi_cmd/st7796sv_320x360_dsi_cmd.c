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

#define FRAME_WIDTH 320
#define FRAME_HEIGHT 360

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
	{0x36, 1, {0x48}},
	{0x3A, 1, {0x77}},
	{0xB4, 1, {0x00}},
	{0xB1, 2, {0xA0, 0x10}},
	{0xB5, 4, {0x1F, 0x50, 0x00, 0x20}},
	{0xB6, 3, {0x8A, 0x07, 0x2C}},
	{0xB9, 2, {0x02, 0xE0}},
	{0xC0, 2, {0x80, 0x77}},
	{0xC1, 1, {0x1C}},
	{0xC2, 1, {0xA7}},
	{0xC5, 1, {0x14}},
	{0xE8, 8, {0x40, 0x82, 0x00, 0x00, 0x29, 0x19, 0xA5, 0x33}},
	{0xE0, 14, {0x70, 0x06, 0x0B, 0x0B, 0x0A, 0x04, 0x39, 0x33, 0x51, 0x1B, 0x18, 0x19, 0x2B, 0x2E}},
	{0xE1, 14, {0x70, 0x0D, 0x11, 0x08, 0x08, 0x15, 0x36, 0x54, 0x4F, 0x38, 0x16, 0x15, 0x2D, 0x2F}},
	{0xF0, 1, {0x3C}},
	{0xF0, 1, {0x69}},
	{0x21, 0, {}},
	{0x35, 1, {0x00}},
	{0x11, 0, {}},
	{REGFLAG_DELAY, 5, {}},
	{0x29, 0, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 0, {}},
	{REGFLAG_DELAY, 5, {}},

	// Display On
	{0x29, 0, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display Off
	{0x28, 0, {}},

	// Sleep In
	{0x10, 0, {}},
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

	//params->dsi.mode = BURST_VDO_MODE;
	//params->dsi.packet_size = 256;
	//params->dsi.vertical_sync_active = 3;
	//params->dsi.vertical_backporch = 4;
	//params->dsi.vertical_frontporch = 4;
	//params->dsi.horizontal_sync_active = 40;
	//params->dsi.horizontal_backporch = 40;
	//params->dsi.horizontal_frontporch = 40;
	//params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.mode = CMD_MODE;
	params->dsi.LANE_NUM = LCM_ONE_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	params->dsi.intermediat_buffer_num = 0;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count = 360 * 3;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.compatibility_for_nvk = 0;
	params->dsi.PLL_CLOCK = 221;
	params->dsi.ssc_range = 0;
	params->dsi.ssc_disable = 1;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
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

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}

static unsigned int lcm_compare_id(void)
{
	unsigned int array[1];
	unsigned short id;
	unsigned char buffer[4];

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(100);

	array[0] = 0x00043700;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);

	dsi_dcs_read_lcm_reg_v2(0xD3, buffer, 4);
	id = (buffer[1] << 8) | (buffer[2]);

	return (0x7796 == id) ? 1 : 0;
}

LCM_DRIVER st7796sv_320x360_dsi_cmd_lcm_drv = {
	.name = "st7796sv_320x360_dsi_cmd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.update = lcm_update,
};
