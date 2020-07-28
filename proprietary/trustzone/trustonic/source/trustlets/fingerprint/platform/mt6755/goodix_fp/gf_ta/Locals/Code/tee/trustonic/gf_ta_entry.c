#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_type_define.h"
#include "gf_ta.h"
#include "gf_tee_internal_api.h"


typedef struct SessionContext {
    uint32_t callsCount; //count number of calls in this session
    uint32_t encryptedCount; //count number of encrypted data in this session
} SessionContext;

// Reserve 500KB for stack.
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(512000)

#define LOG_TAG "[gf_ta_entry] "

/* ----------------------------------------------------------------------------
 *   Service Entry Points
 * ---------------------------------------------------------------------------- */

/**
 *  Function TA_CreateEntryPoint:
 *  Description:
 *        The function TA_CreateEntryPoint is the service constructor, which the system
 *        calls when it creates a new instance of the service.
 *        Here this function implements nothing.
 **/
TEE_Result TA_EXPORT TA_CreateEntryPoint(void) {
    GF_LOGI(LOG_TAG " TA_CreateEntryPoint");

    return gf_ta_create_entry_point();
}

/**
 *  Function TA_DestroyEntryPoint:
 *  Description:
 *        The function TA_DestroyEntryPoint is the service destructor, which the system
 *        calls when the instance is being destroyed.
 *        Here this function implements nothing.
 **/
void TA_EXPORT TA_DestroyEntryPoint(void) {
    GF_LOGI(LOG_TAG " TA_DestroyEntryPoint");
    gf_ta_destroy_entry_point();
}

/**
 *  Function TA_OpenSessionEntryPoint:
 *  Description:
 *        The system calls the function TA_OpenSessionEntryPoint when a new client
 *        connects to the service instance.
 **/
TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(uint32_t paramTypes, IN OUT TEE_Param params[4],
        OUT void** ppSessionContext) {
    SessionContext *pClientSession;

    UNUSED_VAR(paramTypes);
    UNUSED_VAR(params[0]);
    UNUSED_VAR(ppSessionContext);

    GF_LOGI(LOG_TAG " TA_OpenSessionEntryPoint");

    pClientSession = (SessionContext *) TEE_Malloc(sizeof(SessionContext), 0);
    if (NULL == pClientSession) {
        GF_LOGE(LOG_TAG " out of memory!");
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    GF_LOGI(LOG_TAG " allocated session at 0x%x", (unsigned int ) pClientSession);

    if (TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_READ | TEE_MEMORY_ACCESS_WRITE,
            pClientSession, sizeof(SessionContext)) != TEE_SUCCESS) {
        GF_LOGE(LOG_TAG " wrong access rights!");
        return TEE_ERROR_ACCESS_CONFLICT;
    }

    pClientSession->callsCount = 0;
    pClientSession->encryptedCount = 0;

    *ppSessionContext = pClientSession;

    return TEE_SUCCESS;
}

/**
 *  Function TA_CloseSessionEntryPoint:
 *  Description:
 *        The system calls this function to indicate that a session is being closed.
 **/
void TA_EXPORT TA_CloseSessionEntryPoint(IN OUT void* pSessionContext) {

    SessionContext *pClientSession = (SessionContext *) pSessionContext;

    GF_LOGI(LOG_TAG " TA_CloseSessionEntryPoint");
    GF_LOGI(LOG_TAG " client called %i times, encoded %i bytes.", pClientSession->callsCount,
            pClientSession->encryptedCount);

    TEE_Free(pClientSession);
}

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(IN OUT void* pSessionContext, uint32_t cmdId,
        uint32_t paramTypes, TEE_Param params[4]) {

    TEE_Result ret = TEE_SUCCESS;
    SessionContext *clientSession = (SessionContext *) pSessionContext;

    UNUSED_VAR(paramTypes);

    clientSession->callsCount++;

    switch (cmdId) {
        case GF_OPERATION_ID: {
            if ((NULL == params) || (NULL == params[0].memref.buffer)) {
                GF_LOGE(LOG_TAG "gf_ta_invoke_cmd_entry_point invalid parameters");
                ret = (TEE_Result) GF_ERROR_BAD_PARAMS;
            } else {
                ret = (TEE_Result) gf_ta_invoke_cmd_entry_point( params[0].memref.buffer);
            }

            break;
        }

        default: {
            GF_LOGE(LOG_TAG " invalid command ID: 0x%X", cmdId);
            ret = (TEE_Result) GF_ERROR_BAD_PARAMS;
            break;
        }
    }

    return ret;
}
