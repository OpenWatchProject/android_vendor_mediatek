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

#define FRAME_HEIGHT 400
#define FRAME_WIDTH 400

#define REGFLAG_DELAY 0xDE
#define REGFLAG_END_OF_TABLE 0xDF

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
	{0xFE, 1, {0x0A}},
	{0x29, 1, {0x10}},
	{0xFE, 1, {0x05}},
	{0x05, 1, {0x15}},
	{0xFE, 1, {0x00}},
	{0x35, 1, {0x00}},
	{0x11, 0, {0x00}},
	{REGFLAG_DELAY, 5, {}},
	{0x29, 0, {0x00}},
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

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	// Set Brightness
	{0x51, 1, {0xFF}},
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
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.mode = SYNC_EVENT_VDO_MODE;
	params->dsi.LANE_NUM = LCM_ONE_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.packet_size = 256;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.compatibility_for_nvk = 0;
	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 12;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch = 20;
	params->dsi.horizontal_frontporch = 20;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.PLL_CLOCK = 150;
	params->dsi.hw_rotation = 1;
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

static void lcm_set_backlight(unsigned int level)
{
	if (level > 255)
		level = 255;

	lcm_backlight_level_setting[0].para_list[0] = level;
	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_set_backlight_cmdq(void *handle, unsigned int level)
{
	lcm_set_backlight(level);
}

static unsigned int lcm_compare_id(void)
{
	unsigned char buffer[3];
	unsigned int array[5];

	SET_RESET_PIN(0);
	MDELAY(120);
	SET_RESET_PIN(1);
	MDELAY(120);

	array[0] = 0x00043902;
	array[1] = 0x010980ff;
	array[2] = 0x80001500;
	array[3] = 0x00033902;
	array[4] = 0x000980ff;
	dsi_set_cmdq(array, 5, 1);
	MDELAY(10);
	array[0] = 0x00033700;
	dsi_set_cmdq(array, 1, 1);

	array[0] = 0x02001500;
	dsi_set_cmdq(array, 1, 1);

	dsi_dcs_read_lcm_reg_v2(0x04, buffer, 3);

	if (buffer[0] == 0x01 && buffer[1] == 0xFF)
		return 1;
	else
		return 0;
}

LCM_DRIVER rm67162_400x400_dsi_cmd_oled_hehui_lcm_drv = {
	.name = "rm67162_400x400_dsi_cmd_oled_hehui",
	.set_backlight = lcm_set_backlight,
	.set_backlight_cmdq = lcm_set_backlight_cmdq,
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
};
