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

#define LOG_TAG "beanpodkeymaster"

// TODO: make this generic in lib

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <algorithm>
#include <log/log.h>

#include <ipc/keymaster_ipc.h>
#include <ipc/beanpod_keymaster_ipc.h>

#include <tee_client_api.h>
#include <cutils/properties.h>

#define BEANPOD_DEVICE_NAME "/dev/ut_keymaster"
static const char* host_name = "bta_loader";
static const TEEC_UUID KM_TA_UUID =  { 0xc09c9c5d, 0xaa50, 0x4b78, { 0xb0, 0xe4, 0x6e, 0xda, 0x61, 0x55, 0x6c, 0x3a } };
TEEC_Context context;
TEEC_Session session;
TEEC_SharedMemory inputSM;
TEEC_SharedMemory outputSM;

static int handle_ = -1;

int bp_keymaster_connect() {

    uint32_t returnOrigin = 0;

    if (handle_ <= 0) {

        memset((void *)&context, 0, sizeof(TEEC_Context));
        memset((void *)&session, 0, sizeof(TEEC_Session));

        int result = TEEC_InitializeContext(host_name, &context);
        if (result != TEEC_SUCCESS) {
            ALOGE("open km context failed, %x", result);
            return result;
        }

        result = TEEC_OpenSession(
                &context,
                &session,
                &KM_TA_UUID,
                TEEC_LOGIN_PUBLIC,
                NULL,
                NULL,
                &returnOrigin);

        if (result != TEEC_SUCCESS) {
            ALOGE("open km session failed, %x", result);
            TEEC_FinalizeContext(&context);
            return result;
        }

        inputSM.size = BP_KEYMASTER_RECV_BUF_SIZE;
    	inputSM.flags = TEEC_MEM_INPUT;
    	result = TEEC_AllocateSharedMemory(&context, &inputSM);
    	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
    		ALOGE("alloc input mem failed, size=%zu err:%x", inputSM.size, result);
    		TEEC_CloseSession(&session);
            TEEC_FinalizeContext(&context);
    	}

        outputSM.size = BP_KEYMASTER_SEND_BUF_SIZE;
    	outputSM.flags = TEEC_MEM_OUTPUT;
    	result = TEEC_AllocateSharedMemory(&context, &outputSM);
    	if (result != TEEC_SUCCESS || outputSM.buffer == 0) {
    		ALOGE("alloc out mem failed, size=%zu err:%x", outputSM.size, result);
            TEEC_ReleaseSharedMemory(&inputSM);
    		TEEC_CloseSession(&session);
            TEEC_FinalizeContext(&context);
    	}

        handle_ = context.fd;
    }
    ALOGI("km handle_=%d", handle_);

    return 0;
}

int bp_keymaster_call(uint32_t cmd, void* in, uint32_t in_size, uint8_t* out,
                          uint32_t* out_size) {
    if (handle_ < 0) {
        ALOGE("not connected\n");
        return -EINVAL;
    }

    if (in_size > BP_KEYMASTER_RECV_BUF_SIZE ||
        *out_size > BP_KEYMASTER_SEND_BUF_SIZE) {
        ALOGE("send msg too long, %d", in_size);
        return KM_ERROR_INVALID_INPUT_LENGTH;
    }

    TEEC_Operation operation;

    memset((void *)&operation, 0, sizeof(TEEC_Operation));

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
			TEEC_MEMREF_PARTIAL_INPUT, TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE);

    operation.params[0].value.a = 0;

    inputSM.size = in_size;
    memcpy(inputSM.buffer, in, in_size);
	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;

    outputSM.size = BP_KEYMASTER_SEND_BUF_SIZE;
    memset(outputSM.buffer, 0, BP_KEYMASTER_SEND_BUF_SIZE);
    operation.params[2].memref.parent = &outputSM;
	operation.params[2].memref.offset = 0;
	operation.params[2].memref.size = outputSM.size;
    operation.started = 1;

    int rc = TEEC_InvokeCommand(&session, cmd, &operation, NULL);
    if (rc) {
        ALOGE("invoke cmd failed, cmd=%x, rc=%x", cmd, rc);
        return rc;
    }

    if (operation.params[0].value.a) {
        ALOGE("busness failed, %d", operation.params[0].value.a);
        return operation.params[0].value.a;
    }

    *out_size = operation.params[2].memref.size;
    memcpy(out, outputSM.buffer, *out_size);

    return rc;
}

int bp_keymaster_call(uint32_t cmd, uint8_t* out, uint32_t* out_size) {
    if (handle_ < 0) {
        ALOGE("BeanpodKeymaster not connected\n");
        return -EINVAL;
    }
    ALOGI("enter BeanpodKeymaster bp_keymaster_call");

    TEEC_Operation operation;

    memset((void *)&operation, 0, sizeof(TEEC_Operation));

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
			TEEC_NONE, TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE);

    operation.params[0].value.a = 0;

    outputSM.size = BP_KEYMASTER_SEND_BUF_SIZE;
    memset(outputSM.buffer, 0, BP_KEYMASTER_SEND_BUF_SIZE);
    operation.params[2].memref.parent = &outputSM;
	operation.params[2].memref.offset = 0;
	operation.params[2].memref.size = outputSM.size;
    operation.started = 1;

    int rc = TEEC_InvokeCommand(&session, cmd, &operation, NULL);
    if (rc) {
        ALOGE("BeanpodKeymaster invoke cmd failed, cmd=%x, rc=%x", cmd, rc);
        return rc;
    }

    if (operation.params[0].value.a) {
        ALOGE("BeanpodKeymaster busness failed, %d", operation.params[0].value.a);
        return operation.params[0].value.a;
    }

    *out_size = operation.params[2].memref.size;
    memcpy(out, outputSM.buffer, *out_size);
    ALOGI("exit BeanpodKeymaster bp_keymaster_call");

    return rc;
}

void bp_keymaster_disconnect() {
    ALOGI("km disconnect");
    if (handle_ > 0) {
        TEEC_ReleaseSharedMemory(&outputSM);
        TEEC_ReleaseSharedMemory(&inputSM);
        TEEC_CloseSession(&session);
        TEEC_FinalizeContext(&context);
        memset((void *)&context, 0, sizeof(TEEC_Context));
        memset((void *)&session, 0, sizeof(TEEC_Session));
    }
    handle_ = -1;
}

keymaster_error_t translate_error(int err) {
    switch (err) {
        case 0:
            return KM_ERROR_OK;
        case -EPERM:
        case -EACCES:
            return KM_ERROR_SECURE_HW_ACCESS_DENIED;

        case -ECANCELED:
            return KM_ERROR_OPERATION_CANCELLED;

        case -ENODEV:
            return KM_ERROR_UNIMPLEMENTED;

        case -ENOMEM:
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;

        case -EBUSY:
            return KM_ERROR_SECURE_HW_BUSY;

        case -EIO:
            return KM_ERROR_SECURE_HW_COMMUNICATION_FAILED;

        case -EOVERFLOW:
            return KM_ERROR_INVALID_INPUT_LENGTH;

        default:
            return KM_ERROR_UNKNOWN_ERROR;
    }
}

keymaster_error_t bp_keymaster_send(uint32_t command, const keymaster::Serializable& req,
                                        keymaster::KeymasterResponse* rsp) {
    uint32_t req_size = req.SerializedSize();
    if (req_size > BP_KEYMASTER_SEND_BUF_SIZE) {
        ALOGE("Request too big: %u Max size: %u", req_size, BP_KEYMASTER_SEND_BUF_SIZE);
        return KM_ERROR_INVALID_INPUT_LENGTH;
    }

    uint8_t send_buf[BP_KEYMASTER_SEND_BUF_SIZE];
    keymaster::Eraser send_buf_eraser(send_buf, BP_KEYMASTER_SEND_BUF_SIZE);
    req.Serialize(send_buf, send_buf + req_size);

    // Send it
    uint8_t recv_buf[BP_KEYMASTER_RECV_BUF_SIZE];
    keymaster::Eraser recv_buf_eraser(recv_buf, BP_KEYMASTER_RECV_BUF_SIZE);
    uint32_t rsp_size = BP_KEYMASTER_RECV_BUF_SIZE;
    int rc = bp_keymaster_call(command, send_buf, req_size, recv_buf, &rsp_size);
    if (rc < 0) {
        // Reset the connection on tipc error
        bp_keymaster_disconnect();
        bp_keymaster_connect();
        ALOGE("invoke keymaster ta failed: %d\n", rc);
        // TODO(swillden): Distinguish permanent from transient errors and set error_ appropriately.
        return translate_error(rc);
    } else {
        ALOGE("Received %d byte response\n", rsp_size);
    }

    const uint8_t* p = recv_buf;
    if (!rsp->Deserialize(&p, p + rsp_size)) {
        ALOGE("Error deserializing response of size %d\n", (int)rsp_size);
        return KM_ERROR_UNKNOWN_ERROR;
    } else if (rsp->error != KM_ERROR_OK) {
        ALOGE("Response of size %d contained error code %d\n", (int)rsp_size, (int)rsp->error);
        return rsp->error;
    }
    return rsp->error;
}

keymaster_error_t bp_keymaster_send_no_request(uint32_t command, keymaster::KeymasterResponse* rsp) {

    uint8_t recv_buf[BP_KEYMASTER_RECV_BUF_SIZE];
    keymaster::Eraser recv_buf_eraser(recv_buf, BP_KEYMASTER_RECV_BUF_SIZE);
    uint32_t rsp_size = BP_KEYMASTER_RECV_BUF_SIZE;
    int rc = bp_keymaster_call(command, recv_buf, &rsp_size);
    if (rc < 0) {
        // Reset the connection on tipc error
        bp_keymaster_disconnect();
        bp_keymaster_connect();
        ALOGE("BeanpodKeymaster tipc error: %d\n", rc);
        // TODO(swillden): Distinguish permanent from transient errors and set error_ appropriately.
        return translate_error(rc);
    } else {
        ALOGE("BeanpodKeymaster Received %d byte response\n", rsp_size);
    }

    const uint8_t* p = recv_buf;
    if (!rsp->Deserialize(&p, p + rsp_size)) {
        ALOGE("BeanpodKeymaster Error deserializing response of size %d\n", (int)rsp_size);
        return KM_ERROR_UNKNOWN_ERROR;
    } else if (rsp->error != KM_ERROR_OK) {
        ALOGE("BeanpodKeymaster Response of size %d contained error code %d\n", (int)rsp_size, (int)rsp->error);
        return rsp->error;
    }
    return rsp->error;
}
