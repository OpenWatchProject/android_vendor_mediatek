/*
 * Copyright (C) 2016 MediaTek Inc.
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
