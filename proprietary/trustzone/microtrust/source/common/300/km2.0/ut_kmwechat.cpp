/*
 * Copyright (C) 2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define IMSG_TAG "ut_kmwechat"

#include <errno.h>
#include <hardware/hardware.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <imsg_log.h>

#include "ut_km_def.h"
#include "ut_km_ioctl.h"
#include "ut_kmwechat.h"
#include "ut_km_wechat_tac.h"



static int ut_kmwechat_open(const hw_module_t* module, const char* name, hw_device_t** device);

kmwechat_error_t ut_generate_attk(const unsigned char copy_num,
                  unsigned char *device, unsigned int dlen) {
    uint32_t retVal = 0;
    ut_message_t kmstr_msg;

    if(device == NULL || dlen == 0) {
      IMSG_ERROR("%s: input null pointer", __FUNCTION__);
      return KMSK_ERROR_UNEXPECTED_NULL_POINTER;
    }

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    kmstr_msg.command.commandId = CMD_ID_TEE_GENERATE_ATTK;
    kmstr_msg.generate_attk.copy_num = copy_num;
    kmstr_msg.generate_attk.device_len = dlen;

    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));
    memcpy(keymaster_buffer + sizeof(ut_message_t), device, dlen);

    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        IMSG_ERROR("TEE invoke command err %d", retVal);
        retVal = -1;
        goto end;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    if (kmstr_msg.response.error != KMSK_ERROR_OK) {
        IMSG_ERROR("keymaster generate attk failed %d", kmstr_msg.response.error);
        retVal = -1;
        goto end;
    }

    retVal = KMSK_ERROR_OK;
end:
    return (kmwechat_error_t)retVal;
}

kmwechat_error_t ut_verify_attk(void) {
    uint32_t retVal = 0;
    ut_message_t kmstr_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    memset(&kmstr_msg, 0, sizeof(ut_message_t));
    kmstr_msg.command.commandId = CMD_ID_TEE_VERIFY_ATTK;
    memcpy(keymaster_buffer, &kmstr_msg, sizeof(ut_message_t));

    retVal = TEE_InvokeCommand();

    if (retVal != 0) {
        retVal = -1;
        IMSG_ERROR("TEE invoke command err %d", retVal);
        goto end;
    }

    memcpy(&kmstr_msg, keymaster_buffer, sizeof(ut_message_t));

    if (kmstr_msg.response.error != KMSK_ERROR_OK) {
        retVal = -1;
        IMSG_ERROR("keymaster verify attk failed %d", kmstr_msg.response.error);
        goto end;
    }

    retVal = KMSK_ERROR_OK;

end:
    return (kmwechat_error_t)retVal;
}

kmwechat_error_t ut_export_attk(const unsigned char** key_data, size_t* key_datalen) {
    int ret = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    keymaster_msg.command.commandId = CMD_ID_TEE_EXPORT_ATTK;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {
        IMSG_ERROR("TEE invoke command err %d", ret);
        ret = -1;
        goto end;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KMSK_ERROR_OK) {
        ret = -1;
        IMSG_ERROR("[%s]TEE_InvokeCommand response failed , error=%d", __func__,
              keymaster_msg.response.error);
        goto end;
    }

    *key_datalen = keymaster_msg.export_attk.attklen;
    *key_data = (unsigned char *)malloc(*key_datalen);
    memset((void *)(*key_data), 0, *key_datalen);
    memcpy((unsigned char*)(*key_data), keymaster_buffer + sizeof(ut_message_t), *key_datalen);
    ret = KMSK_ERROR_OK;
end:
    return (kmwechat_error_t)ret;
}

kmwechat_error_t ut_get_device_id(const unsigned char** device_id, size_t* device_idlen) {
    int ret = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    keymaster_msg.command.commandId = CMD_ID_TEE_GET_DEVICE_ID;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {

        IMSG_ERROR("TEE invoke command err %d", ret);
        ret = -1;
        goto end;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KMSK_ERROR_OK) {
        ret = -1;
        IMSG_ERROR("[%s]TEE_InvokeCommand response failed ,  error=%d", __func__,
              keymaster_msg.response.error);
        goto end;
    }

    *device_idlen = keymaster_msg.get_device_id.idlen;
    *device_id = (unsigned char *)malloc(*device_idlen);
    memset((void *)(*device_id), 0, *device_idlen);
    memcpy((unsigned char*)(*device_id), keymaster_buffer + sizeof(ut_message_t), *device_idlen);
    ret = KMSK_ERROR_OK;

end:
    return (kmwechat_error_t)ret;
}

kmwechat_error_t ut_get_wechat_info(wechat_info_t *wechat_info) {
    int ret = 0;
    ut_message_t keymaster_msg;

    if (TEE_Alloc_KM_Buffer_And_Reset() != 0) {
        IMSG_ERROR("%s: malloc km buffer failed", __FUNCTION__);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    keymaster_msg.command.commandId = CMD_ID_TEE_GET_WECHAT_INFO;
    memcpy(keymaster_buffer, &keymaster_msg, sizeof(ut_message_t));

    ret = TEE_InvokeCommand();

    if (ret != 0) {

        IMSG_ERROR("TEE invoke command err %d", ret);
        ret = -1;
        goto end;
    }

    memcpy(&keymaster_msg, keymaster_buffer, sizeof(ut_message_t));

    if (keymaster_msg.response.error != KMSK_ERROR_OK) {
        ret = -1;
        IMSG_ERROR("[%s]TEE_InvokeCommand response failed ,  error=%d", __func__,
              keymaster_msg.response.error);
        goto end;
    }

    memcpy((void *)wechat_info, (void *)&keymaster_msg.wechat_info, sizeof(get_wechat_info_t));
    ret = KMSK_ERROR_OK;

end:
    return (kmwechat_error_t)ret;
}

kmwechat_error_t ut_km_generate_attk(const unsigned char copy_num, unsigned char *device, unsigned int dlen) {
    IMSG_INFO("[%s][%d] start , copy_num = %d", __func__, __LINE__, copy_num);
    kmwechat_error_t ret = KMSK_ERROR_OK;
    ret = wechat_generate_attk(copy_num, device, dlen);
    IMSG_INFO("ut km generate attk end , ret = %d", ret);
    return ret;
}

kmwechat_error_t ut_km_verify_attk() {
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    kmwechat_error_t ret = KMSK_ERROR_OK;
    ret = wechat_verify_attk();
    IMSG_INFO("[%s][%d] end , ret = %d", __func__, __LINE__, ret);

    return ret;
}

kmwechat_error_t ut_km_export_attk(const unsigned char** key_data, size_t* key_datalen) {
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    kmwechat_error_t ret = KMSK_ERROR_OK;
    ret = wechat_export_attk((unsigned char**)key_data, key_datalen);
    IMSG_INFO("[%s][%d] end , ret = %d , *key_datalen = %u", __func__, __LINE__, ret,
          (unsigned int)(*key_datalen));
    return ret;
}

kmwechat_error_t ut_km_get_device_id(const unsigned char** device_id, size_t* device_idlen) {
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    kmwechat_error_t ret = KMSK_ERROR_OK;
    ret = wechat_get_device_id((unsigned char**)device_id, device_idlen);
    IMSG_INFO("[%s][%d] end , ret = %d ,  *device_idlen = %d", __func__, __LINE__, ret,
          (unsigned int)(*device_idlen));
    return ret;
}

kmwechat_error_t ut_km_get_wechat_info(wechat_info_t *wechat_info) {
    IMSG_INFO("[%s][%d] start", __func__, __LINE__);
    kmwechat_error_t ret = KMSK_ERROR_OK;
    ret = wechat_get_wechat_info(wechat_info);
    IMSG_INFO("[%s][%d] end , ret = %d", __func__, __LINE__, ret);
    return ret;
}

static int ut_kmwechat_close(hw_device_t* dev) {
    IMSG_ENTER();

    free(dev);

    return 0;
}

static struct hw_module_methods_t kmwechat_module_methods = {
	.open = ut_kmwechat_open,
};

__attribute__((visibility("default")))
struct kmwechat_module HAL_MODULE_INFO_SYM = {
	.common =
	{
		.tag = HARDWARE_MODULE_TAG,
		.module_api_version = KMWECHAT_MODULE_API_VERSION_0_1,
		.hal_api_version = HARDWARE_HAL_API_VERSION,
		.id = KMWECHAT_HARDWARE_MODULE_ID,
		.name = KMWECHAT_HARDWARE_MODULE_NAME,
		.author = "Microtrust",
		.methods = &kmwechat_module_methods,
		.dso = 0,
		.reserved = {},
	},
};

static int ut_kmwechat_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    IMSG_ENTER();

    if (name == NULL)
        return -EINVAL;

    // Make sure we initialize only if module provided is known
    if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
        (module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
        (module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
        (0 != memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
                    sizeof(KMWECHAT_HARDWARE_MODULE_NAME)-1)) )
	  {
        IMSG_ERROR("invalide kmwechat module information");
        return -EINVAL;
    }

    kmwechat_device_t* dev = NULL;
    dev = (kmwechat_device_t*)malloc(sizeof(kmwechat_device_t));

    dev->common.tag = HARDWARE_MODULE_TAG;
    dev->common.version = 1;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = ut_kmwechat_close;

    dev->generate_attk_key_pair = ut_km_generate_attk;
    dev->verify_attk_key_pair = ut_km_verify_attk;
    dev->export_attk_public_key = ut_km_export_attk;
    dev->get_device_id = ut_km_get_device_id;
    dev->get_wechat_info = ut_km_get_wechat_info;

    *device = (hw_device_t*)dev;
    return 0;
}
