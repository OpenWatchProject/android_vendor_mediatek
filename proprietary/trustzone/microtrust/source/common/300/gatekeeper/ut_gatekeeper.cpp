/*
 * Copyright (C) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

/*
 * Copyright (C) 2010 MediaTek Inc.
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/gatekeeper.h>
#include <hardware/hardware.h>

#include "tee_client_api.h"
#include "ut_gk_config.h"
#include "ut_gk_log.h"
#include "ut_gk_struct.h"

#define CMD_ID_ENROLL 0x01
#define CMD_ID_VARIFY 0x02
#define CMD_ID_DELETE_USER 0x03
#define CMD_ID_DELETE_ALL_USERS 0x04
#define GATEKEEPER_BUFF_SIZE (4096)
#define GK_COMMAND_MAGIC 'Q'

gatekeeper_device_t* dev = NULL;

static bool ut_gk_transfer_buff(void* buff)
{
    if (NULL == buff) {
        return false;
    }
    GK_LOG_INF_F("~~~~ ut_gk_transfer_buff begin ~~~~");

    const char* hostname = "bta_loader";
    TEEC_UUID uuid_ta = {
        0xc1882f2d, 0x885e, 0x4e13, {0xa8, 0xc8, 0xe2, 0x62, 0x24, 0x61, 0xb2, 0xfa}};

    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation operation;
    TEEC_SharedMemory sharedmem;
    TEEC_Result result;
    uint32_t returnOrigin = 0;

    memset(&context, 0, sizeof(context));
    memset(&session, 0, sizeof(session));
    result = TEEC_InitializeContext(hostname, &context);
    if (TEEC_SUCCESS != result) {
        IMSG_ERROR("Failed to initialize context ,err: %x", result);
        goto release_1;
    }
    result = TEEC_OpenSession(&context, &session, &uuid_ta, TEEC_LOGIN_PUBLIC, NULL, NULL,
                              &returnOrigin);
    if (TEEC_SUCCESS != result) {
        TEEC_FinalizeContext(&context);
        IMSG_ERROR("Failed to open session,err: %x", result);
        goto release_2;
    }

    sharedmem.buffer = buff;
    sharedmem.size = GATEKEEPER_BUFF_SIZE;
    sharedmem.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
    result = TEEC_RegisterSharedMemory(&context, &sharedmem);
    if (TEEC_SUCCESS != result) {
        IMSG_ERROR("Failed to register shared memory,err: %x", result);
        goto release_3;
    }
    memset(&operation, 0x00, sizeof(operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.started = 1;
    operation.params[0].memref.parent = &sharedmem;
    operation.params[0].memref.size = sharedmem.size;
    result = TEEC_InvokeCommand(&session, GK_COMMAND_MAGIC, &operation, NULL);
    if (TEEC_SUCCESS != result) {
        IMSG_ERROR("Failed to invoke command,err: %x", result);
        goto release_4;
    }

release_4:
    TEEC_ReleaseSharedMemory(&sharedmem);
release_3:
    TEEC_CloseSession(&session);
release_2:
    TEEC_FinalizeContext(&context);
release_1:

    GK_LOG_INF_F("~~~~ ut_gk_transfer_buff end ~~~~");
    if (TEEC_SUCCESS != result) {
        return false;
    } else {
        return true;
    }
}

static int ut_gk_enroll(const struct gatekeeper_device* dev, uint32_t uid,
                        const uint8_t* current_password_handle,
                        uint32_t current_password_handle_length, const uint8_t* current_password,
                        uint32_t current_password_length, const uint8_t* desired_password,
                        uint32_t desired_password_length, uint8_t** enrolled_password_handle,
                        uint32_t* enrolled_password_handle_length)
{
    GK_LOG_INF_F("~~~~ Enter ree enroll ~~~~");
    int ret = -1;
    uint8_t* position = NULL;
    uint8_t* gatekeeper_transfer_buffer = NULL;
    ut_gk_struct_t* ut_gk_struct = NULL;
    ut_gk_enroll_out_t* ut_gk_enroll_out = NULL;

    (void)dev;

    if (desired_password_length >= 100) {
        IMSG_ERROR("Desired_password_length %d, max 99", desired_password_length);
        goto out;
    }
    /* clang-format off */
    if (NULL == enrolled_password_handle || NULL == enrolled_password_handle_length ||
        NULL == desired_password || 0 == desired_password_length) {
        IMSG_ERROR("Input param error, ptr is NULL or len is 0."
                   "enrolled_handle_buffer %p || enrolled_handle_length_buffer %p || "
                   "desired_password_buffer %p || desired_password_length %d",
                   enrolled_password_handle, enrolled_password_handle_length,
                   desired_password, desired_password_length);
        goto out;
    }
    if (NULL == current_password_handle || 0 == current_password_handle_length ||
        NULL == current_password || 0 == current_password_length) {
        current_password_handle = NULL;
        current_password_handle_length = 0;
        current_password = NULL;
        current_password_length = 0;
    } else if (current_password_handle_length != sizeof(password_handle_t) ||
               current_password_length >= 100) {
        IMSG_ERROR("Input param error, current_handle_length %d(expect %d) || "
                   "current_password_length %d(expect <100)",
                   current_password_handle_length, sizeof(password_handle_t),
                   current_password_length);
        goto out;
    }
/* clang-format on */

#ifdef DBG_GK
    IMSG_DEBUG("==== [ree] enroll variable [in] begin ====");
    IMSG_DEBUG("uid is %d", uid);
    IMSG_DEBUG("current_password_handle_length is %d", current_password_handle_length);
    IMSG_DEBUG("current_password_length is %d", current_password_length);
    for (uint32_t i = 0; i < current_password_length; i++)
        IMSG_DEBUG("current_password[%d]: %d", i, current_password[i]);
    IMSG_DEBUG("desired_password_length is %d", desired_password_length);
    for (uint32_t i = 0; i < desired_password_length; i++)
        IMSG_DEBUG("desired_password[%d]: %d", i, desired_password[i]);
    IMSG_DEBUG("==== [ree] enroll variable [in] end ====");
#endif
    gatekeeper_transfer_buffer = (uint8_t*)calloc(1, GATEKEEPER_BUFF_SIZE);
    if (NULL == gatekeeper_transfer_buffer) {
        IMSG_ERROR("Malloc gatekeeper_transfer_buffer failed, size %d", GATEKEEPER_BUFF_SIZE);
        goto out;
    }

    ut_gk_struct = (ut_gk_struct_t*)gatekeeper_transfer_buffer;
    ut_gk_struct->cmd = CMD_ID_ENROLL;
    ut_gk_struct->uid = uid;
    ut_gk_struct->ut_gk_enroll.current_password_handle_length = current_password_handle_length;
    ut_gk_struct->ut_gk_enroll.current_password_length = current_password_length;
    ut_gk_struct->ut_gk_enroll.desired_password_length = desired_password_length;

    position = gatekeeper_transfer_buffer + sizeof(ut_gk_struct_t);
    memcpy(position, current_password_handle, current_password_handle_length);
    position += current_password_handle_length;
    memcpy(position, current_password, current_password_length);
    position += current_password_length;
    memcpy(position, desired_password, desired_password_length);

    if (!ut_gk_transfer_buff(gatekeeper_transfer_buffer))
        goto out;

    ut_gk_enroll_out = (ut_gk_enroll_out_t*)gatekeeper_transfer_buffer;
    if (ERROR_RETRY == ut_gk_enroll_out->error) {
        GK_LOG_INF_F("~~~~ From ree enroll, failed, retry timeout %lu!! ~~~~",
                     ut_gk_enroll_out->retry_timeout);
        ret = ut_gk_enroll_out->retry_timeout;
        goto out;
    } else if (ERROR_NONE != ut_gk_enroll_out->error) {
        if (ut_gk_enroll_out->error == ERROR_PASSWORD_WRONG)
            GK_LOG_INF_F("~~~~ From ree enroll, failed, password wrong ~~~~");
        else if (ut_gk_enroll_out->error == ERROR_INVALID)
            GK_LOG_INF_F("~~~~ From ree enroll, failed, parameter wrong ~~~~");
        else if (ut_gk_enroll_out->error == ERROR_CMD_WRONG)
            GK_LOG_INF_F("~~~~ From ree enroll, failed, cmd wrong, cmd %#x ~~~~",
                         ut_gk_struct->cmd);
        else
            GK_LOG_INF_F("~~~~ From ree enroll, failed, inner error ~~~~");
        goto out;
    }

    *enrolled_password_handle_length = ut_gk_enroll_out->enrolled_password_handle_length;
    *enrolled_password_handle = (uint8_t*)calloc(1, *enrolled_password_handle_length);
    if (NULL == *enrolled_password_handle) {
        IMSG_ERROR("Malloc *enrolled_password_handle failed, size %d",
                   *enrolled_password_handle_length);
        goto out;
    }
    memcpy(*enrolled_password_handle, gatekeeper_transfer_buffer + sizeof(ut_gk_enroll_out_t),
           *enrolled_password_handle_length);

#ifdef DBG_GK
    /* clang-format off */
    IMSG_DEBUG("==== [ree] enroll variable [out] begin ====");
    IMSG_DEBUG("ut_gk_enroll_out->error no error");
    IMSG_DEBUG("sizeof(struct password_handle_t) is %d",
               sizeof(struct password_handle_t));
    IMSG_DEBUG("version is %u",
               ((struct password_handle_t*) (*enrolled_password_handle))->version);
    IMSG_DEBUG("user_id is %llu",
               ((struct password_handle_t*) (*enrolled_password_handle))->user_id);
    IMSG_DEBUG("flags is %llu",
               ((struct password_handle_t*) (*enrolled_password_handle))->flags);
    IMSG_DEBUG("salt is %llu",
               ((struct password_handle_t*) (*enrolled_password_handle))->salt);
    IMSG_DEBUG("handle_length is %lu", *enrolled_password_handle_length);
    IMSG_DEBUG("hardware_backed is %u",
               ((struct password_handle_t*) (*enrolled_password_handle))->hardware_backed);
    IMSG_DEBUG("==== [ree] enroll variable [out] end ====");
/* clang-format on */
#endif
    GK_LOG_INF_F("~~~~ From ree enroll, success ~~~~");
    ret = 0;

out:
    if (gatekeeper_transfer_buffer != NULL)
        free(gatekeeper_transfer_buffer);
    return ret;
}

static int ut_gk_verify(const struct gatekeeper_device* dev, uint32_t uid, uint64_t challenge,
                        const uint8_t* enrolled_password_handle,
                        uint32_t enrolled_password_handle_length, const uint8_t* provided_password,
                        uint32_t provided_password_length, uint8_t** auth_token,
                        uint32_t* auth_token_length, bool* request_reenroll)
{
    GK_LOG_INF_F("~~~~ Enter ree verify ~~~~");
    int ret = -1;
    uint8_t* position = NULL;
    uint8_t* gatekeeper_transfer_buffer = NULL;
    ut_gk_struct_t* ut_gk_struct = NULL;
    ut_gk_verify_out_t* ut_gk_verify_out = NULL;
    (void)dev;

    /* clang-format off */
    if (NULL == enrolled_password_handle || 0 == enrolled_password_handle_length ||
        NULL == provided_password || 0 == provided_password_length || NULL == auth_token ||
        NULL == auth_token_length || NULL == request_reenroll ||
        enrolled_password_handle_length != sizeof(password_handle_t)) {
        IMSG_ERROR("Input param error, ptr is NULL or len is 0. "
                   "handle_buffer %p || handle_length %d || "
                   "password_buffer %p || password_length %d || "
                   "auth_token_buffer %p || auth_token_length_buffer %p || "
                   "request_reenroll_buffer %p",
                   enrolled_password_handle, enrolled_password_handle_length,
                   provided_password, provided_password_length,
                   auth_token, auth_token_length,
                   request_reenroll);
        goto out;
    }
    /* clang-format on */
    if (provided_password_length >= 100) {
        /* We should increase fail times in tee, so can't return directly. */
        IMSG_ERROR("Provided_password_length %d, max 99. Will verify fail",
                   provided_password_length);
        provided_password_length = 17;
    }

#ifdef DBG_GK
    /* clang-format off */
    IMSG_DEBUG("==== [ree] verify variable [in] begin ====");
    IMSG_DEBUG("uid is %lu", uid);
    IMSG_DEBUG("challenge is %llu", challenge);
    IMSG_DEBUG("enrolled_password_handle_length is %lu", enrolled_password_handle_length);
    IMSG_DEBUG("provided_password_length is %lu", provided_password_length);
    for (uint32_t i = 0; i < provided_password_length; i++)
        IMSG_DEBUG("provided_password[%d]: %d", i, provided_password[i]);
    IMSG_DEBUG("version is %u",
               ((struct password_handle_t*) (enrolled_password_handle))->version);
    IMSG_DEBUG("user_id is %llu",
               ((struct password_handle_t*) (enrolled_password_handle))->user_id);
    IMSG_DEBUG("flags is %llu",
               ((struct password_handle_t*) (enrolled_password_handle))->flags);
    IMSG_DEBUG("salt is %llu",
               ((struct password_handle_t*) (enrolled_password_handle))->salt);
    IMSG_DEBUG("hardware_backed is %u",
               ((struct password_handle_t*) (enrolled_password_handle))->hardware_backed);
    IMSG_DEBUG("==== [ree] verify variable [in] end ====");
/* clang-format on */
#endif
    gatekeeper_transfer_buffer = (uint8_t*)calloc(1, GATEKEEPER_BUFF_SIZE);
    if (gatekeeper_transfer_buffer == NULL) {
        IMSG_ERROR("Malloc gatekeeper_transfer_buffer failed, size %d", GATEKEEPER_BUFF_SIZE);
        goto out;
    }

    ut_gk_struct = (ut_gk_struct_t*)gatekeeper_transfer_buffer;
    ut_gk_struct->cmd = CMD_ID_VARIFY;
    ut_gk_struct->uid = uid;
    ut_gk_struct->ut_gk_verify.challenge = challenge;
    ut_gk_struct->ut_gk_verify.enrolled_password_handle_length = enrolled_password_handle_length;
    ut_gk_struct->ut_gk_verify.provided_password_length = provided_password_length;

    position = gatekeeper_transfer_buffer + sizeof(ut_gk_struct_t);
    memcpy(position, enrolled_password_handle, enrolled_password_handle_length);
    position += enrolled_password_handle_length;
    memcpy(position, provided_password, provided_password_length);

    if (!ut_gk_transfer_buff(gatekeeper_transfer_buffer))
        goto out;

    ut_gk_verify_out = (ut_gk_verify_out_t*)gatekeeper_transfer_buffer;
    if (ERROR_RETRY == ut_gk_verify_out->error) {
        GK_LOG_INF_F("~~~~ From ree verify, failed, retry timeout %lu ~~~~",
                     ut_gk_verify_out->retry_timeout);
        ret = ut_gk_verify_out->retry_timeout;
        goto out;
    } else if (ERROR_NONE != ut_gk_verify_out->error) {
        if (ut_gk_verify_out->error == ERROR_PASSWORD_WRONG)
            GK_LOG_INF_F("~~~~ From ree verify, failed, password wrong ~~~~");
        else if (ut_gk_verify_out->error == ERROR_INVALID)
            GK_LOG_INF_F("~~~~ From ree verify, failed, parameter wrong ~~~~");
        else if (ut_gk_verify_out->error == ERROR_CMD_WRONG)
            GK_LOG_INF_F("~~~~ From ree verify, failed, cmd wrong, cmd %#x ~~~~",
                         ut_gk_struct->cmd);
        else
            GK_LOG_INF_F("~~~~ From ree verify, failed, inner error ~~~~");
        goto out;
    }

    *request_reenroll = ut_gk_verify_out->request_reenroll;
    *auth_token_length = ut_gk_verify_out->auth_token_length;
    *auth_token = (uint8_t*)calloc(1, *auth_token_length);
    if (NULL == *auth_token) {
        IMSG_ERROR("Malloc *auth_token failed, size %d", *auth_token_length);
        goto out;
    }
    memcpy(*auth_token, gatekeeper_transfer_buffer + sizeof(ut_gk_verify_out_t),
           *auth_token_length);

#ifdef DBG_GK
    /* clang-format off */
    IMSG_DEBUG("==== [ree] verify variable [out] begin ====");
    IMSG_DEBUG("ut_gk_verify_out->error no error");
    IMSG_DEBUG("request_reenroll is %u", *request_reenroll);
    IMSG_DEBUG("auth_token_length is %lu", *auth_token_length);
    IMSG_DEBUG("version is %u", ((hw_auth_token_t*) (*auth_token))->version);
    IMSG_DEBUG("challenge is %llu", ((hw_auth_token_t*) (*auth_token))->challenge);
    IMSG_DEBUG("user_id is %llu", ((hw_auth_token_t*) (*auth_token))->user_id);
    IMSG_DEBUG("authenticator_id is %llu",
               ((hw_auth_token_t*) (*auth_token))->authenticator_id);
    IMSG_DEBUG("authenticator_type is %lu",
               ((hw_auth_token_t*) (*auth_token))->authenticator_type);
    IMSG_DEBUG("timestamp is %llu", ((hw_auth_token_t*) (*auth_token))->timestamp);
    IMSG_DEBUG("==== [ree] verify variable [out] end ====");
/* clang-format on */
#endif
    GK_LOG_INF_F("~~~~ From ree verify, success ~~~~");
    ret = 0;

out:
    if (gatekeeper_transfer_buffer != NULL)
        free(gatekeeper_transfer_buffer);
    return ret;
}

int ut_gk_delete_user(const struct gatekeeper_device* dev, uint32_t uid)
{
    GK_LOG_INF_F("~~~~ Enter ree delete_user ~~~~");
    int ret = -1;
    uint8_t* gatekeeper_transfer_buffer = NULL;
    ut_gk_struct_t* ut_gk_struct = NULL;
    ut_gk_del_user_out_t* ut_gk_del_user_out = NULL;
    (void)dev;

    gatekeeper_transfer_buffer = (uint8_t*)calloc(1, GATEKEEPER_BUFF_SIZE);
    if (NULL == gatekeeper_transfer_buffer) {
        IMSG_ERROR("Malloc gatekeeper_transfer_buffer failed, size %d", GATEKEEPER_BUFF_SIZE);
        goto out;
    }

    ut_gk_struct = (ut_gk_struct_t*)gatekeeper_transfer_buffer;
    ut_gk_struct->cmd = CMD_ID_DELETE_USER;
    ut_gk_struct->uid = uid;

    if (!ut_gk_transfer_buff(gatekeeper_transfer_buffer))
        goto out;

    ut_gk_del_user_out = (ut_gk_del_user_out_t*)gatekeeper_transfer_buffer;
    if (ut_gk_del_user_out->error != ERROR_NONE) {
        if (ut_gk_del_user_out->error == ERROR_UID_NOTFOUND)
            GK_LOG_INF_F("~~~~ From ree delete_user, failed, no such uid %lu ~~~~", uid);
        else if (ut_gk_del_user_out->error == ERROR_CMD_WRONG)
            GK_LOG_INF_F("~~~~ From ree delete_user, failed, cmd wrong, cmd %#x ~~~~",
                         ut_gk_struct->cmd);
        else
            GK_LOG_INF_F("~~~~ From ree delete_user, failed, inner error ~~~~");
        goto out;
    }
    GK_LOG_INF_F("~~~~ From ree delete_user, success ~~~~");
    ret = 0;

out:
    if (gatekeeper_transfer_buffer != NULL)
        free(gatekeeper_transfer_buffer);
    return ret;
}

int ut_gk_delete_all_users(const struct gatekeeper_device* dev)
{
    GK_LOG_INF_F("~~~~ Enter ree delete_all_users ~~~~");
    int ret = -1;
    uint8_t* gatekeeper_transfer_buffer = NULL;
    ut_gk_struct_t* ut_gk_struct = NULL;
    ut_gk_del_user_out_t* ut_gk_del_user_out = NULL;
    (void)dev;

    gatekeeper_transfer_buffer = (uint8_t*)calloc(1, GATEKEEPER_BUFF_SIZE);
    if (NULL == gatekeeper_transfer_buffer) {
        IMSG_ERROR("Malloc gatekeeper_transfer_buffer failed, size %d", GATEKEEPER_BUFF_SIZE);
        goto out;
    }

    ut_gk_struct = (ut_gk_struct_t*)gatekeeper_transfer_buffer;
    ut_gk_struct->cmd = CMD_ID_DELETE_ALL_USERS;

    if (!ut_gk_transfer_buff(gatekeeper_transfer_buffer))
        goto out;

    ut_gk_del_user_out = (ut_gk_del_user_out_t*)gatekeeper_transfer_buffer;
    if (ut_gk_del_user_out->error != ERROR_NONE) {
        if (ut_gk_del_user_out->error == ERROR_CMD_WRONG)
            GK_LOG_INF_F("~~~~ From ree delte_all_users, failed, cmd wrong, cmd %#x ~~~~",
                         ut_gk_struct->cmd);
        else
            GK_LOG_INF_F("~~~~ From ree delete_all_users, failed, inner error ~~~~");
        goto out;
    }
    GK_LOG_INF_F("~~~~ From ree delete_all_users, success ~~~~");
    ret = 0;

out:
    if (gatekeeper_transfer_buffer != NULL)
        free(gatekeeper_transfer_buffer);
    return ret;
}

static int ut_gk_close(hw_device_t* dev)
{
    IMSG_ENTER();
    free(dev);
    return 0;
}

static int ut_gk_open(const hw_module_t* module, const char* name, hw_device_t** device)
{
    IMSG_ENTER();

    if (strcmp(name, HARDWARE_GATEKEEPER) != 0) {
        IMSG_ERROR("Name not match, expect %s, actual %s", HARDWARE_GATEKEEPER, name);
        return -EINVAL;
    }

    dev = (gatekeeper_device_t*)calloc(1, sizeof(gatekeeper_device_t));
    if (NULL == dev) {
        IMSG_ERROR("Malloc dev failed, size %d", sizeof(gatekeeper_device_t));
        return -EFAULT;
    }

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 1;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = ut_gk_close;
    dev->enroll = ut_gk_enroll;
    dev->verify = ut_gk_verify;
#ifndef GATEKEEPER_DATA_SAVE_IN_RAM
    dev->delete_user = ut_gk_delete_user;
    dev->delete_all_users = ut_gk_delete_all_users;
#else
    dev->delete_user = nullptr;
    dev->delete_all_users = nullptr;
#endif

    *device = &dev->common;
    return 0;
}

static struct hw_module_methods_t gatekeeper_module_methods = {
    .open = ut_gk_open,
};

struct gatekeeper_module HAL_MODULE_INFO_SYM __attribute__((visibility("default"))) = {
    .common =
        {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = GATEKEEPER_MODULE_API_VERSION_0_1,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = GATEKEEPER_HARDWARE_MODULE_ID,
            .name = "Gatekeeper UT HAL",
            .author = "The Microtrust Gatekeeper Source Project",
            .methods = &gatekeeper_module_methods,
            .dso = 0,
            .reserved = {},
        },
};
