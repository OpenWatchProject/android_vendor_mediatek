/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
/**
 * Low level touch API stubs for generic platform
 */


 struct tpd_data{
	uint32_t position_x;
	uint32_t position_y;
	uint32_t finger_states;//
 };

 struct tpd_tui_obj{
	 int (*vendor_tpd_enable)(int enable);//let the hardware know that data should be written to fifo
	 int (*vendor_tpd_read_data)(struct tpd_data *data);//open data rerport to HAL
	 int (*vendor_tpd_clear_intr)(void);
	 int (*vendor_tpd_rst)(void);
 };

extern struct tpd_tui_obj tpd_tui_common_obj;
extern int register_mtk_tpd_tui_api(struct tpd_tui_obj *obj);
