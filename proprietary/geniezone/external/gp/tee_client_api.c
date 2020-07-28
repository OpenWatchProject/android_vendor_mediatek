#include <log/log.h>
#include "tee_client_api.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "gz_gp_client"
#endif

static UREE_SESSION_HANDLE gzMemSession = 0;
#ifndef __NEBULA_HEE__
static const char mem_srv_name[] = "com.mediatek.geniezone.srv.mem";
#else
static const char mem_srv_name[] = "nebula.com.mediatek.geniezone.srv.mem";
#endif

TEEC_Result TEEC_InitializeContext(
    const char   *name,
    TEEC_Context *context)
{
    if(context == NULL) {
        ALOGE("[ERROR] context is null \n");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    return TEEC_SUCCESS;
}

void TEEC_FinalizeContext(
    TEEC_Context *context)
{
    if(context == NULL) {
        ALOGE("[ERROR] context is null \n");
    }

    return;
}

TEEC_Result TEEC_OpenSession(
    TEEC_Context    *context,
    TEEC_Session    *session,
    const TEEC_UUID *destination,
    uint32_t        connectionMethod,
    const void      *connectionData,
    TEEC_Operation  *operation,
    uint32_t        *returnOrigin)
{
    char *srvName;
    TZ_RESULT gzResult;

    if(context == NULL) {
        ALOGE("[ERROR] context is null \n");
        if(returnOrigin) {
            *returnOrigin = TEEC_ORIGIN_API;
        }
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    if(session == NULL || destination == NULL) {
        ALOGE("[ERROR] session or destination is null \n");
        if(returnOrigin) {
            *returnOrigin = TEEC_ORIGIN_API;
        }
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    srvName = (char *)destination;
    gzResult = UREE_CreateSession(srvName, &session->imp.gzSession);

    if(gzResult != TZ_RESULT_SUCCESS) {
        ALOGE("[ERROR] create session fail \n");
        if(returnOrigin) {
            *returnOrigin = TEEC_ORIGIN_COMMS;
        }
        return gzResult;
    }

    return TEEC_SUCCESS;
}

void TEEC_CloseSession(
    TEEC_Session *session)
{
    TZ_RESULT gzResult;

    gzResult = UREE_CloseSession(session->imp.gzSession);
    if(gzResult != TZ_RESULT_SUCCESS) {
        ALOGE("[ERROR] close session fail \n");
    }

    session->imp.gzSession = 0;
}

TEEC_Result TEEC_InvokeCommand(
    TEEC_Session     *session,
    uint32_t         commandID,
    TEEC_Operation   *operation,
    uint32_t         *returnOrigin)
{
    TZ_RESULT gzResult;
    MTEEC_PARAM gzParam[4];
    int i, param;
    TZ_PARAM_TYPES gzParamType[4] = {0};
    uint32_t gzTypes;

    if(session == NULL) {
        ALOGE("[ERROR] session is null \n");
        if(returnOrigin) {
            *returnOrigin = TEEC_ORIGIN_API;
        }
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    // convert param from gp to gz
    for(i = 0; i<4; i++) {
        param = ((operation->paramTypes) >> (i*8)) & 0xFF;
        if(param == TEEC_NONE) {
            break;
        }
        if(param >= TEEC_VALUE_INPUT && param <= TEEC_VALUE_INOUT) {
            gzParamType[i] = param;
            gzParam[i].value = *(MTEEC_VALUE *)(&(operation->params[i].value));
        }
        else if (param >= TEEC_MEMREF_TEMP_INPUT && param <= TEEC_MEMREF_TEMP_INOUT) {
            gzParamType[i] = param - 1;
            gzParam[i].mem = *(MTEEC_MEM *)(&(operation->params[i].tmpref));
        }
        else if (param == TEEC_MEMREF_WHOLE) {
            gzParamType[i] = TZPT_MEMREF_INOUT;
            gzParam[i].memref = *(MTEEC_MEMREF *)(&(operation->params[i].memref));
        }
        else if (param >= TEEC_MEMREF_PARTIAL_INPUT && param <= TEEC_MEMREF_PARTIAL_INOUT) {
            gzParamType[i] = param - 6;
            gzParam[i].memref = *(MTEEC_MEMREF *)(&(operation->params[i].memref));
        }
    }
    gzTypes = TZ_ParamTypes(gzParamType[0], gzParamType[1], gzParamType[2], gzParamType[3]);

    gzResult = UREE_TeeServiceCall(session->imp.gzSession, commandID, gzTypes, gzParam);
    if(gzResult != TZ_RESULT_SUCCESS) {
        ALOGE("[ERROR] invoke command fail \n");
        if(returnOrigin) {
            *returnOrigin = TEEC_ORIGIN_COMMS;
        }
        return gzResult;
    }

    // copy back to gp param
    for(i = 0; i<4; i++) {
        param = ((operation->paramTypes) >> (i*8)) & 0xFF;
        if(param == TEEC_NONE) {
            break;
        }
        if(param >= TEEC_VALUE_INPUT && param <= TEEC_VALUE_INOUT) {
            operation->params[i].value = *(TEEC_Value *)(&(gzParam[i].value));
        }
        else if (param >= TEEC_MEMREF_TEMP_INPUT && param <= TEEC_MEMREF_TEMP_INOUT) {
            operation->params[i].tmpref = *(TEEC_TempMemoryReference *)(&(gzParam[i].mem));
        }
        else if (param >= TEEC_MEMREF_WHOLE && param <= TEEC_MEMREF_PARTIAL_INOUT) {
            operation->params[i].memref = *(TEEC_RegisteredMemoryReference *)(&(gzParam[i].memref));
        }
    }

    return TEEC_SUCCESS;
}

TEEC_Result TEEC_RegisterSharedMemory(
    TEEC_Context      *context,
    TEEC_SharedMemory *sharedMem)
{
    TZ_RESULT gzResult;
    UREE_SHAREDMEM_PARAM gzSharedMem;

    if(context == NULL) {
        ALOGE("[ERROR] context is null \n");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    if(sharedMem == NULL) {
        ALOGE("[ERROR] sharedMem is null \n");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    if(gzMemSession == 0) {
        gzResult = UREE_CreateSession(mem_srv_name, &gzMemSession);
        if(gzResult != TZ_RESULT_SUCCESS) {
            ALOGE("[ERROR] create memory session fail \n");
            return gzResult;
        }
    }

    gzSharedMem.buffer = (void *)sharedMem->buffer;
    gzSharedMem.size = sharedMem->size;
    gzSharedMem.region_id = 0;
    gzResult = UREE_RegisterSharedmem(gzMemSession, &(sharedMem->imp.gzSharedMem), &gzSharedMem);
    if(gzResult != TZ_RESULT_SUCCESS) {
        ALOGE("[ERROR] register shared memory fail \n");
        return gzResult;
    }

    return TEEC_SUCCESS;
}

void TEEC_ReleaseSharedMemory(
    TEEC_SharedMemory *sharedMem)
{
    TZ_RESULT gzResult;

    gzResult = UREE_UnregisterSharedmem(gzMemSession, sharedMem->imp.gzSharedMem);
    if(gzResult != TZ_RESULT_SUCCESS) {
        ALOGE("[ERROR] unregister shared memory fail \n");
    }

    sharedMem->imp.gzSharedMem = 0;
}

