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
