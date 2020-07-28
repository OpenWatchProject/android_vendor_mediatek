#include <stdbool.h>
#include <hardware/hardware.h>
#include <hardware/gatekeeper.h>
#include "module.h"

/* -------------------------------------------------------------------------
 * Module definitions needed for integrtion with Android HAL framework.
 * -------------------------------------------------------------------------*/

static struct hw_module_methods_t soft_gatekeeper_module_methods = {
	.open = soft_gatekeeper_open,
};

__attribute__((visibility("default")))
struct gatekeeper_module HAL_MODULE_INFO_SYM = {
	.common =
	{
		.tag = HARDWARE_MODULE_TAG,
		.module_api_version = GATEKEEPER_MODULE_API_VERSION_0_1,
		.hal_api_version = HARDWARE_HAL_API_VERSION,
		.id = GATEKEEPER_HARDWARE_MODULE_ID,
		.name = GATEKEEPER_SOFTWARE_MODULE_NAME,
		.author = "MediaTek",
		.methods = &soft_gatekeeper_module_methods,
		.dso = 0,
		.reserved = {},
	},
};

