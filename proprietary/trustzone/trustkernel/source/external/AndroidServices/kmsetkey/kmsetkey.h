/*
 * Copyright (C) 2017 TrustKernel Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to TrustKernel Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from TrustKernel Incorporated.
 */

#ifndef _TRUSTKERNEL_KMSET_KEY_H_
#define _TRUSTKERNEL_KMSET_KEY_H_

#include <stdbool.h>
#include <stdint.h>
#include <hardware/hardware.h>

#define KMSETKEY_HARDWARE_MODULE_NAME   "hwkmsetkeyHAL"
#define KMSETKEY_HARDWARE_MODULE_ID     "kmsetkey"

#define KMSETKEY_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)

typedef enum {
    KMSK_ERROR_OK = 0,
    KMSK_ERROR_UNEXPECTED_NULL_POINTER = -1,
    KMSK_ERROR_MEMORY_ALLOCATION_FAILED = -2,
    KMSK_ERROR_RESPONSE_ERROR = -3,
    KMSK_ERROR_INVOKE_COMMAND_FAILED = -4,

    KMSK_ERROR_UNKNOWN_ERROR = -1000,
} kmsetkey_error_t;

struct kmsetkey_module {
	hw_module_t common;
};

struct kmsetkey_device {
	hw_device_t common;
	kmsetkey_error_t (*attest_key_install)(const uint8_t *peakb, const uint32_t peakb_len);
};

typedef struct kmsetkey_device kmsetkey_device_t;

#endif
