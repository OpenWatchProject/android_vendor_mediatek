
#include <taStd.h>
#include <tee_internal_api.h>
#include <comm.h>
#include <alipay_oem.h>

DECLARE_TRUSTED_APPLICATION_MAIN_STACK(5 * 1024)

#define CMD_ALIPAY                      0x20
#define CMD_GET_LAST_ID     (CMD_ALIPAY + 1)
#define CMD_GET_VERSION     (CMD_GET_LAST_ID + 1)
#define CMD_GET_DEVICE_ID   (CMD_GET_VERSION + 1)

#define CMD_ALIPAY_PTYPES  TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_INOUT,  TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE)

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
    uint8_t* in;
    size_t in_len;
    uint8_t* out;
    size_t* out_len;
    TEE_Result ret;
    int32_t id = 0;

    UNUSED_VAR(sessionContext);
    LOG_ERROR("%d charlie\n", CMD_ALIPAY_PTYPES);

    switch (commandID) {
        case CMD_ALIPAY: {
            if (paramTypes != CMD_ALIPAY_PTYPES) {
                LOG_ERROR("bad params\n");
                return TEE_ERROR_BAD_PARAMETERS;
            }

             in = params[0].memref.buffer;
             in_len = params[0].memref.size;
             out = params[1].memref.buffer;
             out_len = &params[1].memref.size;
            if (in == NULL || in_len == 0 || out == NULL || out_len == NULL || *out_len == 0) {
                return TEE_ERROR_BAD_PARAMETERS;
            }

            ret = TEE_CheckMemoryAccessRights(
                    TEE_MEMORY_ACCESS_READ | TEE_MEMORY_ACCESS_ANY_OWNER, in, in_len);
            if (ret != TEE_SUCCESS) {
                LOG_ERROR("wrong input access rights!\n");
                return ret;
            }

            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE | TEE_MEMORY_ACCESS_ANY_OWNER,
                    out, *out_len);
            if (ret != TEE_SUCCESS) {
                LOG_ERROR("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }
            return alipay_tz_invoke_command(in, in_len, out, out_len);
        }
        case CMD_GET_LAST_ID: {
            if (paramTypes != CMD_ALIPAY_PTYPES) {
                LOG_ERROR("bad params\n");
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
                LOG_ERROR("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }

            ret = TEE_GetFpLastIdentifiedResult(&id);
            LOG_ERROR("get finger id :%d \n", id);
            if (ret != TEE_SUCCESS) {
                LOG_ERROR("fail to get id!\n");
            }

            *((int32_t *)(void*)out) = id;
            return ret;
        }

        case CMD_GET_VERSION: {
            if (paramTypes != CMD_ALIPAY_PTYPES) {
                LOG_ERROR("bad params\n");
                return TEE_ERROR_BAD_PARAMETERS;
            }

            out = params[1].memref.buffer;
            out_len = &params[1].memref.size;
            if ( out == NULL || out_len == NULL || *out_len == 0) {
                return TEE_ERROR_BAD_PARAMETERS;
            }

            ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE | TEE_MEMORY_ACCESS_ANY_OWNER,
                    out, *out_len);
            if (ret != TEE_SUCCESS) {
                LOG_ERROR("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }

            *((int *)(void*)out) = TEE_GetAuthenticatorVersion();
            return TEE_SUCCESS;
        }

        case CMD_GET_DEVICE_ID: {
            if (paramTypes != CMD_ALIPAY_PTYPES) {
                LOG_ERROR("bad params\n");
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
                LOG_ERROR("wrong output access rights!\n");
                return TEE_ERROR_ACCESS_CONFLICT;
            }

            TEE_GetDeviceID ((void*) out, out_len);
            return TEE_SUCCESS;
        }
    }

    LOG_ERROR("invalid command ID: 0x%X\n", commandID);
    return TEE_ERROR_BAD_PARAMETERS;
}
