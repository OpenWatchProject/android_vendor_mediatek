/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KM_WECHAT_H__
#define __UT_KM_WECHAT_H__

#include <stdbool.h>
#include <stdint.h>
#include <hardware/hardware.h>

#define KMWECHAT_HARDWARE_MODULE_NAME "Hardware kmwechat HAL"
#define KMWECHAT_HARDWARE_MODULE_ID "kmwechat"
#define KMWECHAT_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)

typedef struct {
	uint32_t kmversion;
	uint32_t wechatversion;
	uint32_t supportwechat;
	uint32_t reserved;
} wechat_info_t;

typedef enum {
    KMSK_ERROR_OK = 0,
    KMSK_ERROR_UNEXPECTED_NULL_POINTER = -1,
    KMSK_ERROR_MEMORY_ALLOCATION_FAILED = -2,
    KMSK_ERROR_RESPONSE_ERROR = -3,
    KMSK_ERROR_INVOKE_COMMAND_FAILED = -4,
    KMSK_ERROR_RPMB_FAILED = -5,
    KMSK_ERROR_GENERATE_KEY_FAILED = -6,
    KMSK_ERROR_EXPORT_KEY_FAILED = -7,
    KMSK_ERROR_EXPORT_UUID_FAILED = -8,
	KMSK_ERROR_INVALID_INPUT = -9,

    KMSK_ERROR_UNKNOWN_ERROR = -1000,
} kmwechat_error_t;

struct kmwechat_module {

	hw_module_t common;
};

struct kmwechat_device {

	hw_device_t common;
  kmwechat_error_t (*generate_attk_key_pair)(const unsigned char copy_num, unsigned char *device, unsigned int dlen);
  kmwechat_error_t (*verify_attk_key_pair)();
  kmwechat_error_t (*export_attk_public_key)(const unsigned char** key_data, size_t* key_datalen);
  kmwechat_error_t (*get_device_id)(const unsigned char** device_id, size_t* device_idlen);
  kmwechat_error_t (*get_wechat_info)(wechat_info_t *wechat_info);
};

typedef struct kmwechat_device kmwechat_device_t;

#endif //__UT_KM_WECHAT_H__
