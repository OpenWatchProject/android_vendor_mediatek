/*
 * Copyright (C) 2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define IMSG_TAG "ut_kmsetkey"

#include <errno.h>
#include <hardware/hardware.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <imsg_log.h>
#include <log/log.h>

#include "ut_km_def.h"
#include "ut_km_ioctl.h"
#include "ut_kmsetkey.h"
#include "ut_km_tac.h"

typedef struct {
    unsigned int all_len;
    unsigned int peakb_len;
    unsigned int inputpkb_len;
    unsigned int ekkb_pub_len;
    unsigned int reserved[4];
} kmsetkey_t;

static unsigned char input_ekkb_pub[] = {
	0x54, 0xCB, 0xCD, 0xB3, 0xFF, 0xCD, 0xCC, 0xDD, 0xA3, 0xFF, 0x5D, 0x30, 0x95, 0x03, 0x2D, 0x2A,
	0x2A, 0x12, 0xE6, 0x90, 0x0B, 0x3F, 0xF7, 0x85, 0xE5, 0xB3, 0xDD, 0x8E, 0x4B, 0x27, 0x9D, 0x58,
	0xC8, 0x24, 0x7A, 0xB0, 0x83, 0x8B, 0xB1, 0xD4, 0xA4, 0x92, 0x17, 0x0E, 0xF2, 0xCF, 0x19, 0x9A,
	0xAB, 0xCE, 0xFB, 0x68, 0xCB, 0x86, 0x94, 0x6E, 0x16, 0x8E, 0x3D, 0xCC, 0xF8, 0x0C, 0xA6, 0x30,
	0x1C, 0x47, 0xA6, 0xB6, 0x50, 0x2F, 0x68, 0x94, 0x23, 0x0C, 0x62, 0xAF, 0xE1, 0x44, 0xA4, 0x27,
	0xD8, 0x79, 0x05, 0x68, 0x51, 0x89, 0x04, 0x49, 0x61, 0x93, 0x7A, 0xEF, 0xB5, 0xB9, 0x17, 0x72,
	0x28, 0x87, 0xBA, 0x94, 0x4A, 0xB8, 0xF1, 0x46, 0xCF, 0xE7, 0x53, 0x0A, 0x02, 0x5A, 0xEE, 0x59,
	0x47, 0xBE, 0xC2, 0x41, 0x98, 0xD9, 0x5B, 0x17, 0xAF, 0x10, 0x0B, 0xE0, 0x92, 0xBA, 0x65, 0x30,
	0x63, 0x76, 0x94, 0x2A, 0x26, 0x7D, 0x3F, 0x94, 0x2E, 0x9F, 0x06, 0xB8, 0xD3, 0xB0, 0x76, 0xE9,
	0xBD, 0xBA, 0x07, 0x6E, 0xE1, 0x3D, 0x1F, 0xC6, 0xDB, 0x7F, 0x34, 0xC1, 0xB4, 0xED, 0x8B, 0x00,
	0x36, 0xAE, 0x1E, 0xBB, 0x65, 0x81, 0x38, 0x94, 0x77, 0xE2, 0x4E, 0x5C, 0xC1, 0x9F, 0x93, 0x2D,
	0x29, 0xA3, 0x30, 0x29, 0xF7, 0xEC, 0xFC, 0xCC, 0x87, 0x3F, 0xFA, 0x09, 0xAD, 0x1E, 0xE5, 0xAF,
	0x4E, 0xCF, 0x0E, 0x44, 0x8C, 0xE3, 0xBF, 0x8D, 0x5B, 0xEB, 0xD6, 0xA0, 0xEA, 0xC6, 0xBF, 0xB1,
	0x56, 0xD5, 0xC9, 0xE6, 0xB8, 0xE1, 0xB9, 0x94, 0x85, 0xAD, 0x38, 0x38, 0xDD, 0xE2, 0x57, 0xCC,
	0xFE, 0xED, 0xF0, 0x2A, 0x10, 0xB6, 0x8E, 0x3C, 0xA2, 0x4D, 0x97, 0x60, 0x3E, 0xEC, 0x92, 0xE2,
	0xC1, 0x72, 0xB6, 0x38, 0xE2, 0xC0, 0xA8, 0xCA, 0xD6, 0xEB, 0x0C, 0x35, 0xE9, 0x3E, 0x8D, 0x91,
};



static unsigned char inputpkb[] = {
	0x05, 0x14, 0x24, 0x14, 0x2F, 0xE1, 0xFC, 0x61, 0xB1, 0x0B, 0x97, 0xAF, 0x5C, 0x66, 0xB0, 0xF6,
	0x15, 0x26, 0xF6, 0x1C, 0x96, 0xC8, 0xBA, 0x96, 0x77, 0xD5, 0x4E, 0xFB, 0xA4, 0x91, 0xFD, 0xFB,
	0x16, 0x33, 0xED, 0x6E, 0xCC, 0x41, 0x0F, 0xCF, 0xC2, 0x94, 0xC2, 0x64, 0x1C, 0xFA, 0x12, 0x66,
	0x04, 0xE3, 0x4C, 0xF0, 0xB4, 0x5F, 0x15, 0x4B, 0xDB, 0xF4, 0x29, 0x1F, 0x98, 0xD3, 0xF5, 0x4E,
	0xA8, 0xD2, 0xA1, 0x0E, 0x8B, 0x59, 0xBF, 0x17, 0xDE, 0xB7, 0xA7, 0x50, 0x02, 0x2F, 0x14, 0x9C,
	0x7A, 0xE5, 0x24, 0x6D, 0x0E, 0x9F, 0xDE, 0x45, 0x4D, 0x6A, 0x75, 0x06, 0xB3, 0xDA, 0x88, 0x86,
	0x8D, 0xA6, 0x11, 0x43, 0xA8, 0x17, 0xA9, 0x6F, 0x70, 0x27, 0x01, 0xDA, 0xFA, 0xAF, 0xF6, 0xA8,
	0x47, 0xEC, 0xEF, 0x29, 0x66, 0x4E, 0xA8, 0x7C, 0x99, 0xFA, 0x40, 0xB8, 0xD4, 0x8A, 0x2C, 0xB1,
};

static int ut_kmsetkey_open(const hw_module_t* module, const char* name, hw_device_t** device);

static kmsetkey_error_t ut_ree_import_attest_keybox(const unsigned char* peakb,
    const unsigned int peakb_len)
{
    IMSG_INFO("kmsetkey ca start");
    int32_t retVal;
    uint8_t* p_data_buff = 0;
    uint32_t in_params_size = 0;
    ut_message_t kmstr_msg;
    kmsetkey_t input_msg;

	if (peakb == NULL) {
		IMSG_ERROR("null pointer in import attest key");
		return KMSK_ERROR_UNEXPECTED_NULL_POINTER;
	}

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&input_msg, 0, sizeof(kmsetkey_t));
    input_msg.peakb_len = peakb_len;
    input_msg.inputpkb_len = sizeof(inputpkb);
    input_msg.ekkb_pub_len = sizeof(input_ekkb_pub);
    input_msg.all_len = input_msg.peakb_len + input_msg.inputpkb_len
                        + input_msg.ekkb_pub_len + sizeof(kmsetkey_t);

    if (input_msg.inputpkb_len != 128 ||
        input_msg.ekkb_pub_len != 256) {
        IMSG_ERROR("invalid inputlen in import attest key");
        return KMSK_ERROR_UNKNOWN_ERROR;
    }

    /* msg header + kmsetkey_t + peakb + inputPkb + Ekkb_pub */
    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    p_data_buff = keymaster_buffer + sizeof(ut_message_t);
    kmstr_msg.command.commandId = CMD_ID_TEE_REE_IMPORT_KEYBOX;
    kmstr_msg.import_attest_keybox.attest_keybox_len = input_msg.all_len;
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));

    memcpy(p_data_buff, &input_msg, sizeof(kmsetkey_t));
    p_data_buff += sizeof(kmsetkey_t);

    memcpy(p_data_buff, peakb, input_msg.peakb_len);
    p_data_buff += input_msg.peakb_len;
    memcpy(p_data_buff, inputpkb, input_msg.inputpkb_len);
    p_data_buff += input_msg.inputpkb_len;
    memcpy(p_data_buff, input_ekkb_pub, input_msg.ekkb_pub_len);

    p_data_buff = keymaster_buffer + sizeof(ut_message_t);

    retVal = ut_km_import_google_key(p_data_buff, input_msg.all_len);

    if (retVal != 0) {
        IMSG_ERROR("TEE invoke command err %d", retVal);
        return KMSK_ERROR_INVOKE_COMMAND_FAILED;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    if (kmstr_msg.response.error != KMSK_ERROR_OK) {
        IMSG_ERROR("import attestkeybox failed %d", kmstr_msg.response.error);
        return KMSK_ERROR_RESPONSE_ERROR;
    }

    //retVal = kmstr_msg.response.error;
    IMSG_INFO("kmsetkey ca end");
    return KMSK_ERROR_OK;

}

static kmsetkey_error_t ut_ree_check_attest_keybox(const unsigned char* peakb,
    const unsigned int peakb_len)
{
    (void) peakb;
    (void) peakb_len;
    ALOGI("kmsetkey ca check keybox start\n");
    int32_t retVal;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    retVal = ut_km_check_google_key();
    if (retVal != 0) {
        IMSG_ERROR("check keybox failed %d", retVal);
        return KMSK_ERROR_INVOKE_COMMAND_FAILED;
    }

    ALOGI("kmsetkey ca end");
    return KMSK_ERROR_OK;

}

static int ut_kmsetkey_close(hw_device_t* dev) {
    IMSG_ENTER();
    TEE_Free_KM_Buffer();
    free(dev);

    return 0;
}

static struct hw_module_methods_t kmsetkey_module_methods = {
	.open = ut_kmsetkey_open,
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
		.author = "Microtrust",
		.methods = &kmsetkey_module_methods,
		.dso = 0,
		.reserved = {},
	},
};

static int ut_kmsetkey_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    IMSG_ENTER();

    if (name == NULL)
        return -EINVAL;

    // Make sure we initialize only if module provided is known
    if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
        (module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
        (module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
        (0 != memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
                    sizeof(KMSETKEY_HARDWARE_MODULE_NAME)-1)) )
	  {
        IMSG_ERROR("invalide kmsetkey module information");
        return -EINVAL;
    }

    kmsetkey_device_t* dev = NULL;
    dev = (kmsetkey_device_t*)malloc(sizeof(kmsetkey_device_t));

    dev->common.tag = HARDWARE_MODULE_TAG;
    dev->common.version = 1;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = ut_kmsetkey_close;

    dev->attest_key_install = ut_ree_import_attest_keybox;
    dev->attest_key_check = ut_ree_check_attest_keybox;

    *device = (hw_device_t*)dev;
    return 0;
}
