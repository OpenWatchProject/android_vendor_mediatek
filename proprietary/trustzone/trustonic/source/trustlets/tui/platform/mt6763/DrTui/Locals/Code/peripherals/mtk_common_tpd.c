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
#define TAG "[MTK_touch]"

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "mt_typedefs.h"
#include "i2c.h"

#include "log.h"
#include "mtk_common_tpd.h"

struct tpd_tui_obj tpd_tui_common_obj = {0};

int register_mtk_tpd_tui_api(struct tpd_tui_obj *obj)
{
	int err = 0;

	if(obj != NULL){
		tpd_tui_common_obj.vendor_tpd_enable = obj->vendor_tpd_enable;
		tpd_tui_common_obj.vendor_tpd_read_data = obj->vendor_tpd_read_data;
		tpd_tui_common_obj.vendor_tpd_clear_intr = obj->vendor_tpd_clear_intr;
		tpd_tui_common_obj.vendor_tpd_rst = obj->vendor_tpd_rst;
	}else{
		err = -1;
		return err;
	}

	return err;
}

