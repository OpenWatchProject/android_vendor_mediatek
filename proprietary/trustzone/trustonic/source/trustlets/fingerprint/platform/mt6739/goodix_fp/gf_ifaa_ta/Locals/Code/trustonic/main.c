/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_log.h"
#include "ifaa_ta_common.h"
#include "ifaa_gf_interface.h"


#define CMD_ALIPAY          0x20
#define CMD_GET_LAST_ID     (CMD_ALIPAY + 1)
#define CMD_GET_VERSION     (CMD_ALIPAY + 2)
#define CMD_GET_DEVICE_ID   (CMD_ALIPAY + 3)
#define CMD_IFAA_GET_LAST_ID  (CMD_ALIPAY + 4)
#define CMD_IFAA_GET_FINGER_LIST  (CMD_ALIPAY + 5)
#define CMD_IFAA_GET_AUTHENTICATOR_VERSION  (CMD_ALIPAY + 6)

#define CMD_ALIPAY_PTYPES TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, \
        TEE_PARAM_TYPE_MEMREF_INOUT, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE)
#define MAX_FINGER_COUNT  32

DECLARE_TRUSTED_APPLICATION_MAIN_STACK(5 * 1024)

TEE_Result TA_EXPORT TA_CreateEntryPoint(void) {
    return TEE_SUCCESS;
}

void TA_EXPORT TA_DestroyEntryPoint(void) {
}

TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(uint32_t paramTypes, TEE_Param params[4],
        void** sessionContext) {
    UNUSED_VAR(paramTypes);
    UNUSED_VAR(params);
    UNUSED_VAR(sessionContext);
    return TEE_SUCCESS;
}

void TA_EXPORT TA_CloseSessionEntryPoint(void* sessionContext) {
    UNUSED_VAR(sessionContext);
}

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(void* sessionContext, uint32_t commandID,
        uint32_t paramTypes, TEE_Param params[4]) {
    uint8_t* out;
    size_t* out_len;
    TEE_Result ret;
    IFAA_Result result = IFAA_ERR_SUCCESS;

    UNUSED_VAR(sessionContext);
    GF_LOGE("commandid:%d\n", commandID);

    switch (commandID) {
        case CMD_IFAA_GET_LAST_ID: {
            uint32_t fingerID = 0;
            uint32_t bufferLen = 4;

            if (paramTypes != CMD_ALIPAY_PTYPES) {
                GF_LOGE("bad params\n");
                return TEE_ERROR_BAD_PARAMETERS;
            }

            out = params[1].memref.buffer;
            out_len = &params[1].memref.size;
            if (out == NULL || out_len == NULL || *out_len == 0) {
                return TEE_ERROR_BAD_PARAMETERS;
            }

            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE | TEE_MEMORY_ACCESS_ANY_OWNER,
                    out, *out_len);
            if (ret != TEE_SUCCESS) {
                GF_LOGE("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }

            result = ifaa_gf_getFpLastIdentifiedResult((uint8_t *)&fingerID, &bufferLen);
            GF_LOGI("get finger id :%d \n", fingerID);
            if (result != IFAA_ERR_SUCCESS) {
                GF_LOGE("fail to get id!\n");
            }

            *((int32_t *) (void*) out) = fingerID;
            return ret;
        }

        case CMD_IFAA_GET_FINGER_LIST: {
            uint32_t IDs[MAX_FINGER_COUNT];
            uint32_t len = 0;

            if (paramTypes != CMD_ALIPAY_PTYPES) {
                GF_LOGE("bad params\n");
                return TEE_ERROR_BAD_PARAMETERS;
            }

            out = params[1].memref.buffer;
            out_len = &params[1].memref.size;
            if (out == NULL || out_len == NULL || *out_len == 0) {
                return TEE_ERROR_BAD_PARAMETERS;
            }

            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE | TEE_MEMORY_ACCESS_ANY_OWNER,
                    out, *out_len);
            if (ret != TEE_SUCCESS) {
                GF_LOGE("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }

            result = ifaa_gf_getIdList((uint8_t *)IDs, &len);
            if (result != IFAA_ERR_SUCCESS) {
                GF_LOGE("fail to get id list!\n");
            }

            *((uint32_t*) (void*) out) = len;
            memcpy((out + 4), IDs, len * sizeof(uint32_t));
            return ret;
        }

        case CMD_IFAA_GET_AUTHENTICATOR_VERSION: {
            int32_t version = 0;
            if (paramTypes != CMD_ALIPAY_PTYPES) {
                GF_LOGE("bad params\n");
                return TEE_ERROR_BAD_PARAMETERS;
            }

            out = params[1].memref.buffer;
            out_len = &params[1].memref.size;
            if (out == NULL || out_len == NULL || *out_len == 0) {
                return TEE_ERROR_BAD_PARAMETERS;
            }

            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE | TEE_MEMORY_ACCESS_ANY_OWNER,
                    out, *out_len);
            if (ret != TEE_SUCCESS) {
                GF_LOGE("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }

            result = ifaa_gf_getAuthenticatorVersion(&version);
            if (result != IFAA_ERR_SUCCESS) {
                GF_LOGE("fail to get version!\n");
            }

            *((int32_t *) (void*) out) = version;
            return ret;
        }
        default: {
            return TEE_ERROR_BAD_PARAMETERS;
        }
    }

    GF_LOGE("invalid command ID: 0x%X\n", commandID);
    return TEE_ERROR_BAD_PARAMETERS;
}
