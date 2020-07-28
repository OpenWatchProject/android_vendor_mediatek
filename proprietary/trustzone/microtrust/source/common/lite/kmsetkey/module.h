/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifndef _module_H_
#define _module_H_

#define KMSETKEY_HARDWARE_MODULE_NAME "Hardware kmsetkey HAL"
#define KMSETKEY_HARDWARE_MODULE_ID "kmsetkey"
#define KMSETKEY_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)


#ifdef __cplusplus
extern "C" {
#endif
int nv_kmsetkey_open(const struct hw_module_t* module, const char* id,
		struct hw_device_t** device);

int nv_kmsetkey_close(hw_device_t *hw);

#ifdef __cplusplus
}
#endif

#endif
