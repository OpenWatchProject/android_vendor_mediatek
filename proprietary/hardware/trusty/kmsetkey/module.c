/*
 * Copyright (c) 2016, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include <stdbool.h>
#include <hardware/hardware.h>
#include "kmsetkey.h"
#include "module.h"

/* -------------------------------------------------------------------------
   Module definitions needed for integrtion with Android HAL framework.
   -------------------------------------------------------------------------*/

static struct hw_module_methods_t kmsetkey_module_methods = {
	.open = nv_kmsetkey_open,
};

__attribute__((visibility("default")))
struct kmsetkey_module HAL_MODULE_INFO_SYM = {
	.common =
	{
		.tag = HARDWARE_MODULE_TAG,
		.module_api_version = KMSETKEY_MODULE_API_VERSION_0_1,
		.hal_api_version = HARDWARE_HAL_API_VERSION,
		.id = KMSETKEY_HARDWARE_MODULE_ID,
		.name = KMSETKEY_HARDWARE_MODULE_NAME,
		.author = "MediaTek",
		.methods = &kmsetkey_module_methods,
		.dso = 0,
		.reserved = {},
	},
};
