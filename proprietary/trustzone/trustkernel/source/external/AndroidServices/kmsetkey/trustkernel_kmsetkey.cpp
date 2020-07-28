/*
 * Copyright (C) 2017 TrustKernel Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to TrustKernel Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from TrustKernel Incorporated.
 */

#include <errno.h>
#include <hardware/hardware.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <android/log.h>
#include <tee_client_api.h>

#include "kmsetkey.h"

#define LOG_TAG "kmsetkey"

#undef ALOGE
#define ALOGE(fmt, ...) \
    __android_log_print(ANDROID_LOG_ERROR, \
        LOG_TAG, "%s:%d: " fmt, basename(__FILE__), __LINE__, ##__VA_ARGS__)

#undef ALOGD
#define ALOGD(fmt, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, \
        LOG_TAG, "%s:%d: " fmt, basename(__FILE__), __LINE__, ##__VA_ARGS__)

#undef ALOGI
#define ALOGI(fmt, ...) \
    __android_log_print(ANDROID_LOG_INFO, \
        LOG_TAG, "%s:%d: " fmt, basename(__FILE__), __LINE__, ##__VA_ARGS__)

#undef ALOGW
#define ALOGW(fmt, ...) \
    __android_log_print(ANDROID_LOG_WARN, \
        LOG_TAG, "%s:%d: " fmt, basename(__FILE__), __LINE__, ##__VA_ARGS__)

extern struct kmsetkey_module HAL_MODULE_INFO_SYM;
#define CMD_KPH_KMSETKEY_ID 18

static const TEEC_UUID kmsetkey_uuid = { 0xb46325e6, 0x5c90, 0x8252, { 0x2e, 0xad, 0xa8, 0xe3, 0x2e, 0x51, 0x80, 0xd6 } };

TEEC_Context context;
TEEC_Session session;

bool is_open_tee = false;

static int trustkernel_kmsetkey_open(const hw_module_t *module, const char *name, hw_device_t **device);

TEEC_Result TEEOpenSession()
{
    TEEC_Result ret = TEEC_SUCCESS;
#define RETRY_TEEC_FUNC(name, ...) do { \
    for (;;) { \
        ret = TEEC_##name(__VA_ARGS__); \
        if (ret != TEEC_SUCCESS) { \
            ALOGE(" " #name " failed with 0x%08x\n", ret); \
            sleep(1); \
        } else { \
            break; \
        } \
    } \
} while(0)

    RETRY_TEEC_FUNC(InitializeContext, NULL, &context);
    if (ret != TEEC_SUCCESS) {
        ALOGE("TEEC_InitializeContext failed with return value:%x\n", ret);
        goto cleanup_1;
    }

    RETRY_TEEC_FUNC(OpenSession, &context, &session,
                    &kmsetkey_uuid, TEEC_LOGIN_PUBLIC,
                    NULL, NULL, NULL);
    if (ret != TEEC_SUCCESS) {
        ALOGE("TEEC_OpenSession failed with return value:%x\n", ret);
        goto cleanup_2;
    }

    ALOGI("Open session successfully\n");
    is_open_tee = true;
    return TEEC_SUCCESS;

cleanup_2:
    TEEC_FinalizeContext(&context);
cleanup_1:
    return ret;
#undef RETRY_TEEC_FUNC
}

void TEECloseSession()
{
    ALOGI("Close session successfully\n");

    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
    is_open_tee = false;
}

static kmsetkey_error_t trustkernel_ree_import_attest_keybox(const unsigned char *peakb, const unsigned int peakb_len)
{
    TEEC_Result ret = 0;
    TEEC_Operation op;

    kmsetkey_error_t err = KMSK_ERROR_UNKNOWN_ERROR;

    uint8_t *buf = NULL;
    uint32_t ro = 0;

    if (peakb == NULL) {
        ALOGE("Unexpected NULL peakb.");
        return KMSK_ERROR_UNEXPECTED_NULL_POINTER;
    }

    if ((buf = (uint8_t *) malloc(peakb_len)) == NULL) {
        ALOGE("Allocate %u bytes peakb failed\n", peakb_len);
        return KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
    }

    if (!is_open_tee) {
        if ((ret = TEEOpenSession())) {
            ALOGE("Open session failed with 0x%x\n", ret);
            goto free;
        }
    }

    memset(&op, 0, sizeof(op));

    op.paramTypes = TEEC_PARAM_TYPES(
            TEEC_MEMREF_TEMP_INPUT,
            TEEC_NONE,
            TEEC_NONE,
            TEEC_NONE);

    memcpy(buf, peakb, peakb_len);

    op.params[0].tmpref.buffer = buf;
    op.params[0].tmpref.size = peakb_len;

    op.started = 1;

    if ((ret = TEEC_InvokeCommand(&session, CMD_KPH_KMSETKEY_ID, &op, &ro)) != TEEC_SUCCESS) {
        ALOGE("Invokecommand(CMD_KMSEKEY_ID) failed with ret 0x%0x errOrigin: %u\n", ret, ro);
        err = KMSK_ERROR_INVOKE_COMMAND_FAILED;
        goto free;
     }

    err = KMSK_ERROR_OK;

free:
    free(buf);
    return err;
}

static int trustkernel_kmsetkey_close(hw_device_t* dev)
{
    TEECloseSession();
    free(dev);
    return 0;
}

static int trustkernel_kmsetkey_open(const hw_module_t* module, const char* name, hw_device_t** device)
{
    TEEC_Result ret = TEEC_SUCCESS;
    kmsetkey_device_t *dev = NULL;

    if (name == NULL)
        return -EINVAL;

    // Make sure we initialize only if module provided is known
    if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
        (module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
        (module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
        (0 != memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
                    sizeof(KMSETKEY_HARDWARE_MODULE_NAME)-1)) ) {
        ALOGI("Invalid kmsetkey module information");
        return -EINVAL;
    }

    if ((dev = (kmsetkey_device_t*) malloc(sizeof(kmsetkey_device_t))) == NULL) {
        ALOGE("Failed to allocate %zu byte kmsetkey_device_t\n", sizeof(kmsetkey_device_t));
        return -ENOMEM;
    }

    if ((ret = (int) TEEOpenSession())) {
        ALOGE("kmsetkey open device node failed with 0x%x errno=%d", ret, errno);
        free(dev);
        return ret;
    }

    dev->common.tag = HARDWARE_MODULE_TAG;
    dev->common.version = 1;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = trustkernel_kmsetkey_close;

    dev->attest_key_install = trustkernel_ree_import_attest_keybox;

    *device = (hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t kmsetkey_module_methods = {
    .open = trustkernel_kmsetkey_open,
};

__attribute__((visibility("default")))
struct kmsetkey_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = KMSETKEY_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = KMSETKEY_HARDWARE_MODULE_ID,
        .name = KMSETKEY_HARDWARE_MODULE_NAME,
        .author = "TrustKernel",
        .methods = &kmsetkey_module_methods,
        .dso = 0,
        .reserved = {},
    },
};
