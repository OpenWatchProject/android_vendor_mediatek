/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#include <stdlib.h>
#include <string.h>
#include "gf_error.h"
#include <tee_client_api.h>
#include <android/log.h>
#include "gf_ta_uuid.h"
#include "gf_ca_entry.h"
#include "gf_type_define.h"

#define LOG_TAG "[gf_ca_entry] "

#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define GF_CMD_TEEC_PARAM_TYPES TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE)

static TEEC_Context *g_context = NULL;
static TEEC_Session *g_session = NULL;
static const TEEC_UUID UUID = gf_ta_UUID;

#define GF_CA_VERSION   "gf_ca_v1.1.1"

void gf_ca_set_handle(int fd){

}

gf_error_t gf_ca_open_session(void) {
    gf_error_t err = GF_SUCCESS;

    LOG_D("[%s] enter", __func__);

    LOG_D("[%s] GF_CA's version:%s", __func__, GF_CA_VERSION);
    do {
        g_context = (TEEC_Context *) malloc(sizeof(TEEC_Context));
        if (NULL == g_context) {
            LOG_E("[%s], malloc g_context failed", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(g_context, 0, sizeof(TEEC_Context));

        TEEC_Result result = TEEC_InitializeContext(NULL, g_context);
        if (TEEC_SUCCESS != result) {
            LOG_E("[%s], TEEC_InitializeContext failed result = 0x%x", __func__, result);
            err = GF_ERROR_OPEN_TA_FAILED;
            break;
        }

        TEEC_Operation operation;

        g_session = (TEEC_Session *) malloc(sizeof(TEEC_Session));
        if (NULL == g_session) {
            LOG_E("[%s], malloc g_session failed", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(g_session, 0, sizeof(TEEC_Session));
        memset(&operation, 0, sizeof(TEEC_Operation));

        result = TEEC_OpenSession(g_context, g_session, &UUID, TEEC_LOGIN_PUBLIC, NULL, &operation, NULL);
        if (TEEC_SUCCESS != result) {
            LOG_E("[%s], TEEC_OpenSession failed result = 0x%x", __func__, result);
            err = GF_ERROR_OPEN_TA_FAILED;
            break;
        }
    } while (0);

    if (err != GF_SUCCESS) {
        gf_ca_close_session();
    }

    LOG_D("[%s] exit", __func__);
    return err;
}

void gf_ca_close_session(void) {
    LOG_D("[%s] enter", __func__);

    if (NULL != g_session) {
        TEEC_CloseSession(g_session);
        free(g_session);
        g_session = NULL;
    }

    if (NULL != g_context) {
        TEEC_FinalizeContext(g_context);
        free(g_context);
        g_context = NULL;
    }
}

gf_error_t gf_ca_invoke_command(uint32_t operation_id, uint32_t cmd_id, void *cmd, int len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t ret = TEEC_SUCCESS;
    TEEC_Operation operation = { 0 };
    operation.paramTypes = GF_CMD_TEEC_PARAM_TYPES;
    operation.params[0].tmpref.buffer = cmd;
    operation.params[0].tmpref.size = len;
    operation.params[1].value.a = cmd_id;

    ret = TEEC_InvokeCommand(g_session, operation_id, &operation, NULL);
    if( GF_SUCCESS == ret) {
        err = operation.params[1].value.b;
    } else if (ret < GF_ERROR_MAX) {
        err = ret;
    } else {
        LOG_E("[%s] tee error ret = 0x%08x", __func__, ret);
        err = GF_ERROR_TA_DEAD;
        gf_ca_close_session();
    }

    return err;
}
