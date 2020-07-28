/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "GatekeeperHAL"

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <type_traits>
#include <cstring>
#include <unistd.h>
#include <cutils/log.h>
#include <hardware/hw_auth_token.h>

#include "trustkernel_gatekeeper.h"
#include "trustkernel_gatekeeper_ipc.h"

namespace gatekeeper {

TrustKernelGateKeeperDevice::TrustKernelGateKeeperDevice(const hw_module_t *module) {
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
    static_assert(std::is_standard_layout<TrustKernelGateKeeperDevice>::value,
                  "TrustKernelGateKeeperDevice must be standard layout");
    static_assert(offsetof(TrustKernelGateKeeperDevice, device_) == 0,
                  "device_ must be the first member of TrustKernelGateKeeperDevice");
    static_assert(offsetof(TrustKernelGateKeeperDevice, device_.common) == 0,
                  "common must be the first member of gatekeeper_device");
#else
    assert(reinterpret_cast<gatekeeper_device_t *>(this) == &device_);
    assert(reinterpret_cast<hw_device_t *>(this) == &(device_.common));
#endif
    ALOGI("Init device\n");

    memset(&device_, 0, sizeof(device_));
    device_.common.tag = HARDWARE_DEVICE_TAG;
    device_.common.version = 1;
    device_.common.module = const_cast<hw_module_t *>(module);
    device_.common.close = close_device;

    device_.enroll = enroll;
    device_.verify = verify;
    device_.delete_user = nullptr;
    device_.delete_all_users = nullptr;

    error_ = OpenTEESession();
}

TEEC_Result TrustKernelGateKeeperDevice::OpenTEESession() {
// retry teec_#name, and set error code into error_
#define RETRY_TEEC_FUNC(name, ...) do { \
    for (;;) { \
        error_ = TEEC_##name(__VA_ARGS__); \
        if (error_ != TEEC_SUCCESS) { \
            ALOGE(" " #name " failed with 0x%08x\n", error_); \
            sleep(1); \
        } else { \
            break; \
        } \
    } \
} while(0)

    RETRY_TEEC_FUNC(InitializeContext, NULL, &context);
    if (error_ != TEEC_SUCCESS) {
        ALOGE("TEEC_InitializeContext failed with return value:%x\n", error_);
        goto cleanup_1;
    }

    RETRY_TEEC_FUNC(OpenSession, &context, &session,
                    &UUID_TrustKernelGateKeeper, TEEC_LOGIN_PUBLIC,
                    NULL, NULL, NULL);
    if (error_ != TEEC_SUCCESS) {
        ALOGE("TEEC_OpenSession failed with return value:%x\n", error_);
        goto cleanup_2;
    }

    ALOGI("Open session successfully\n");

    return 0;

cleanup_2:
    TEEC_FinalizeContext(&context);
cleanup_1:
    return error_;
#undef RETRY_TEEC_FUNC
}

void TrustKernelGateKeeperDevice::CloseTEESession() {
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
    ALOGI("Close session successfully\n");
}

hw_device_t* TrustKernelGateKeeperDevice::hw_device() {
    return &device_.common;
}

int TrustKernelGateKeeperDevice::close_device(hw_device_t* dev) {
    delete reinterpret_cast<TrustKernelGateKeeperDevice *>(dev);
    return 0;
}

TrustKernelGateKeeperDevice::~TrustKernelGateKeeperDevice() {
    ALOGI("Close device\n");
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
}

int TrustKernelGateKeeperDevice::Enroll(uint32_t uid, const uint8_t *current_password_handle,
        uint32_t current_password_handle_length, const uint8_t *current_password,
        uint32_t current_password_length, const uint8_t *desired_password,
        uint32_t desired_password_length, uint8_t **enrolled_password_handle,
        uint32_t *enrolled_password_handle_length) {

    ALOGI("Enroll\n");

    if (error_ != 0) {
        error_ = OpenTEESession();
        ALOGI("reopen session result 0x%x\n", error_);
    }

    if (error_ != 0)
        return error_;

    TEEC_Result result;


    password_handle_t handle;
    if (current_password_handle_length != 0 && current_password_handle_length != sizeof(password_handle_t))
        return -EINVAL;
    if (current_password_handle_length != 0)
        memcpy(&handle, current_password_handle, sizeof(password_handle_t));

    TEEC_Operation op;
    uint32_t orig;
    op.params[0].value.a = uid;
    op.params[0].value.b = (current_password_handle != NULL);
    op.params[1].tmpref.buffer = &handle;
    op.params[1].tmpref.size = sizeof(password_handle_t);
    op.params[2].tmpref.buffer = const_cast<uint8_t *>(current_password);
    op.params[2].tmpref.size = current_password_length;
    op.params[3].tmpref.buffer = const_cast<uint8_t *>(desired_password);
    op.params[3].tmpref.size = desired_password_length;
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_MEMREF_TEMP_INOUT, TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);

    result = TEEC_InvokeCommand(&session, CMD_ENROLL, &op, &orig);

    if (result != TEEC_SUCCESS) {
        ALOGE("Enroll invoke command failed with 0x%08x, orig 0x%08x\n", result, orig);
        if (orig != TEEC_ORIGIN_TRUSTED_APP) {
            CloseTEESession();
            error_ = OpenTEESession();
        }
        return result;
    }
    ALOGI("Enroll invoke command successfully, return %d\n", op.params[0].value.a);

    if (op.params[0].value.a == 0) {
        *enrolled_password_handle = new uint8_t[sizeof(password_handle_t)];
        memcpy(*enrolled_password_handle, &handle, sizeof(password_handle_t));
        *enrolled_password_handle_length = sizeof(password_handle_t);
    } else {
        *enrolled_password_handle = NULL;
        *enrolled_password_handle_length = 0;
    }

    return op.params[0].value.a;
}

int TrustKernelGateKeeperDevice::Verify(uint32_t uid, uint64_t challenge,
        const uint8_t *enrolled_password_handle, uint32_t enrolled_password_handle_length,
        const uint8_t *provided_password, uint32_t provided_password_length,
        uint8_t **auth_token, uint32_t *auth_token_length, bool *request_reenroll) {
    ALOGI("Verify\n");

    if (error_ != 0) {
        error_ = OpenTEESession();
        ALOGI("reopen session retval 0x%x\n", error_);
    }

    if (error_ != 0)
        return error_;


    TEEC_Result result;

    if (enrolled_password_handle_length == 0 || enrolled_password_handle_length != sizeof(password_handle_t)) {
        ALOGE("Wrong password_handle_t size, expected %uB, got %zuB\n", enrolled_password_handle_length, sizeof(password_handle_t));
        return -EINVAL;
    }

    struct {
        password_handle_t handle;
        hw_auth_token_t authtoken;
    } tmp;
    memcpy(&tmp.handle, enrolled_password_handle, enrolled_password_handle_length);

    TEEC_Operation op;
    uint32_t orig;
    op.params[0].value.a = uid;
    op.params[1].value.a = challenge & 0xFFFFFFFFUL;
    op.params[1].value.b = (challenge >> 32) & 0xFFFFFFFFUL;
    op.params[2].tmpref.buffer = &tmp;
    op.params[2].tmpref.size = sizeof(tmp);
    op.params[3].tmpref.buffer = const_cast<uint8_t *>(provided_password);
    op.params[3].tmpref.size = provided_password_length;
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_INOUT, TEEC_MEMREF_TEMP_INOUT, TEEC_MEMREF_TEMP_INPUT);

    result = TEEC_InvokeCommand(&session, CMD_VERIFY, &op, &orig);

    if (result != TEEC_SUCCESS) {
        ALOGE("Verify invoke command failed with 0x%08x, orig 0x%08x\n", result, orig);
        if (orig != TEEC_ORIGIN_TRUSTED_APP) {
            CloseTEESession();
            error_ = OpenTEESession();
        }
        return result;
    }
    ALOGI("Verify invoke command successfully, return %d\n", op.params[0].value.a);

    if (op.params[0].value.a == 0) {
        *auth_token = new uint8_t[sizeof(hw_auth_token_t)];
        memcpy(*auth_token, &tmp.authtoken, sizeof(hw_auth_token_t));
        *auth_token_length = sizeof(hw_auth_token_t);
        *request_reenroll = op.params[0].value.b;
    } else {
        *auth_token = NULL;
        *auth_token_length = 0;
        *request_reenroll = 0;
    }

    return op.params[0].value.a;
}

static inline TrustKernelGateKeeperDevice *convert_device(const gatekeeper_device *dev) {
    return reinterpret_cast<TrustKernelGateKeeperDevice *>(const_cast<gatekeeper_device *>(dev));
}

/* static */
int TrustKernelGateKeeperDevice::enroll(const struct gatekeeper_device *dev, uint32_t uid,
            const uint8_t *current_password_handle, uint32_t current_password_handle_length,
            const uint8_t *current_password, uint32_t current_password_length,
            const uint8_t *desired_password, uint32_t desired_password_length,
            uint8_t **enrolled_password_handle, uint32_t *enrolled_password_handle_length) {

    if (dev == NULL ||
            enrolled_password_handle == NULL || enrolled_password_handle_length == NULL ||
            desired_password == NULL || desired_password_length == 0)
        return -EINVAL;

    // Current password and current password handle go together
    if (current_password_handle == NULL || current_password_handle_length == 0 ||
            current_password == NULL || current_password_length == 0) {
        current_password_handle = NULL;
        current_password_handle_length = 0;
        current_password = NULL;
        current_password_length = 0;
    }

    return convert_device(dev)->Enroll(uid, current_password_handle, current_password_handle_length,
            current_password, current_password_length, desired_password, desired_password_length,
            enrolled_password_handle, enrolled_password_handle_length);
}

/* static */
int TrustKernelGateKeeperDevice::verify(const struct gatekeeper_device *dev, uint32_t uid,
        uint64_t challenge, const uint8_t *enrolled_password_handle,
        uint32_t enrolled_password_handle_length, const uint8_t *provided_password,
        uint32_t provided_password_length, uint8_t **auth_token, uint32_t *auth_token_length,
        bool *request_reenroll) {

    if (dev == NULL || enrolled_password_handle == NULL ||
            provided_password == NULL) {
        return -EINVAL;
    }

    return convert_device(dev)->Verify(uid, challenge, enrolled_password_handle,
            enrolled_password_handle_length, provided_password, provided_password_length,
            auth_token, auth_token_length, request_reenroll);
}

};
