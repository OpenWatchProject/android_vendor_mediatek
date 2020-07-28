 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdlib.h>
#include <string.h>
#include <tlrpmb_Api.h>
#include "tee_client_api.h"
#include "log.h"

#define LOG_TAG "TLC RPMB"


const TEEC_UUID uuid = TL_RPMB_UUID;
TEEC_Context  *context;
TEEC_Session  *session;

/**
*  Function r13Initialize:
*  Description:
*           Initialize: create a device context.
*  Output : TEEC_Context **context     = points to the device context
*
**/
static TEEC_Result r13Initialize(OUT TEEC_Context **context)
{
    TEEC_Result    nError;
    TEEC_Context  *pContext;

    LOG_I("[Tlc rpmb] %s", __func__);

    *context = NULL;
    printf("r13Initialize ++ \n");
    pContext = (TEEC_Context *)malloc(sizeof(TEEC_Context));
    if (pContext == NULL) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }
    memset(pContext, 0, sizeof(TEEC_Context));
    /* Create Device context  */
    nError = TEEC_InitializeContext(NULL, pContext);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_InitializeContext failed (%08x), %d", __func__, nError, __LINE__);
        if (nError == TEEC_ERROR_COMMUNICATION) {
            LOG_E("[Tlc rpmb] %s: The client could not communicate with the service, %d", __func__, __LINE__);
        }
        free(pContext);
    } else {
        *context = pContext;
    }
    printf("r13Initialize --\n");

    LOG_I("[Tlc rpmb] %s finished", __func__);
    return nError;
}

/**
*  Function r13Finalize:
*  Description:
*           Finalize: delete the device context.
*  Input :  TEEC_Context *context     = the device context
*
**/

static TEEC_Result r13Finalize(IN OUT TEEC_Context *context)
{

    LOG_I("[Tlc rpmb] %s", __func__);

    if (context == NULL) {
        LOG_E("[Tlc rpmb] %s: Device handle invalid, %d", __func__, __LINE__);
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    TEEC_FinalizeContext(context);
    free(context);
    return TEEC_SUCCESS;
}

/**
*  Function r13CloseSession:
*  Description:
*           Close the client session.
*  Input :  TEEC_Session *session - session handler
*
**/
static TEEC_Result r13CloseSession(IN TEEC_Session *session)
{
    LOG_I("[Tlc rpmb] %s", __func__);

    if (session == NULL) {
        LOG_E("[Tlc rpmb] %s: Invalid session handle, %d", __func__, __LINE__);
        return TEEC_ERROR_BAD_PARAMETERS;
    }
    TEEC_CloseSession(session);
    free(session);
    return TEEC_SUCCESS;
}

/**
*  Function r13OpenSession:
*  Description:
*           Open a client session with a specified service.
*  Input :  TEEC_Context  *context,    = the device context
*  Output:  TEEC_Session  **session   = points to the session handle
*
**/
static TEEC_Result r13OpenSession(
    IN    TEEC_Context  *context,
    OUT   TEEC_Session  **session)
{
    TEEC_Operation sOperation;
    TEEC_Result    nError;

    LOG_I("[Tlc rpmb] %s", __func__);
    printf("r13OpenSession ++\n");

    *session = (TEEC_Session *)malloc(sizeof(TEEC_Session));
    if (*session == NULL) {
        return TEEC_ERROR_OUT_OF_MEMORY;
    }
    memset(*session, 0, sizeof(TEEC_Session));
    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = 0;
    nError = TEEC_OpenSession(context,
                              *session,                    /* OUT session */
                              &uuid,                      /* destination UUID */
                              TEEC_LOGIN_PUBLIC,          /* connectionMethod */
                              NULL,                       /* connectionData */
                              &sOperation,                /* IN OUT operation */
                              NULL                        /* OUT returnOrigin, optional */
                             );
    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_OpenSession failed (%08x), %d", __func__, nError, __LINE__);
        free(*session);
        return nError;
    }
	printf("r13OpenSession --\n");
    return TEEC_SUCCESS;
}

// -------------------------------------------------------------
TEEC_Result caOpen(void)
{
    TEEC_Result     nError;

    LOG_I("[Tlc rpmb] %s", __func__);

    printf("caOpen ++\n");
    nError = r13Initialize(&context);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: r13Initialize failed (%08x), %d", __func__, nError, __LINE__);
        return nError;
    }

    /* Open a session */
    nError = r13OpenSession(context, &session);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: r13OpenSession failed (%08x), %d", __func__, nError, __LINE__);
        return nError;
    }
    printf("caOpen --\n");
    return nError;
}

// -------------------------------------------------------------
TEEC_Result caRpmbRead(
    const void* plainText, uint32_t plainTextLength,
    uint32_t* result)
{
    TEEC_Result    nError;
    TEEC_Operation sOperation;

    LOG_I("[Tlc rpmb] %s", __func__);

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(
                                TEEC_MEMREF_TEMP_OUTPUT,   /* TEE write to buffer */
                                TEEC_MEMREF_TEMP_OUTPUT,
                                TEEC_NONE,
                                TEEC_NONE);
    sOperation.params[0].tmpref.buffer = (void*)plainText;
    sOperation.params[0].tmpref.size   = plainTextLength;
    sOperation.params[1].tmpref.buffer = result;
    sOperation.params[1].tmpref.size   = 4;

    nError = TEEC_InvokeCommand(session,
                                CMD_RPMB_READ_DATA,
                                &sOperation,       /* IN OUT operation */
                                NULL               /* OUT returnOrigin, optional */
                               );

    //*nCipherTextLength = sOperation.params[1].tmpref.size;

    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_InvokeCommand failed (%08x), %d", __func__, nError, __LINE__);
    }

    return nError;
}

TEEC_Result caRpmbReadByOffset(
    const void* plainText, uint32_t plainTextLength,
    uint32_t offset, uint32_t userID, uint32_t* result)
{
    TEEC_Result    nError;
    TEEC_Operation sOperation;

    LOG_I("[Tlc rpmb] %s", __func__);

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(
                                TEEC_MEMREF_TEMP_OUTPUT,   /* TEE write to buffer */
                                TEEC_MEMREF_TEMP_OUTPUT,
                                TEEC_MEMREF_TEMP_INPUT,
                                TEEC_NONE);
    sOperation.params[0].tmpref.buffer = (void*)plainText;
    sOperation.params[0].tmpref.size   = plainTextLength;
    sOperation.params[1].tmpref.buffer = &offset;
    sOperation.params[1].tmpref.size   = 4;
    sOperation.params[2].tmpref.buffer = &userID;
    sOperation.params[2].tmpref.size   = 4;

    printf("caRpmbReadByOffset ++\n");
    nError = TEEC_InvokeCommand(session,
                                CMD_RPMB_READ_BY_OFFSET,
                                &sOperation,       /* IN OUT operation */
                                NULL               /* OUT returnOrigin, optional */
                               );
    printf("caRpmbReadByOffset --\n");
    //*nCipherTextLength = sOperation.params[1].tmpref.size;

    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_InvokeCommand failed (%08x), %d", __func__, nError, __LINE__);
    }

    return nError;
}

// -------------------------------------------------------------
TEEC_Result caRpmbWrite(
    const void* plainText, uint32_t plainTextLength,
    uint32_t* result)
{
    TEEC_Result    nError;
    TEEC_Operation sOperation;

    LOG_I("[Tlc rpmb] %s", __func__);

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(
                                TEEC_MEMREF_TEMP_INPUT,   /* TEE read from buffer */
                                TEEC_MEMREF_TEMP_OUTPUT,
                                TEEC_NONE,
                                TEEC_NONE);
    sOperation.params[0].tmpref.buffer = (void*)plainText;
    sOperation.params[0].tmpref.size   = plainTextLength;
    sOperation.params[1].tmpref.buffer = result;
    sOperation.params[1].tmpref.size   = 4;

    nError = TEEC_InvokeCommand(session,
                                CMD_RPMB_WRITE_DATA,
                                &sOperation,       /* IN OUT operation */
                                NULL               /* OUT returnOrigin, optional */
                               );

    //*nCipherTextLength = sOperation.params[1].tmpref.size;

    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_InvokeCommand failed (%08x), %d", __func__, nError, __LINE__);
    }

    return nError;
}

TEEC_Result caRpmbWriteByOffset(
    const void* plainText, uint32_t plainTextLength,
    uint32_t offset, uint32_t userID, uint32_t* result)
{
    TEEC_Result    nError;
    TEEC_Operation sOperation;

    LOG_I("[Tlc rpmb] %s", __func__);

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(
                                TEEC_MEMREF_TEMP_INPUT,   /* TEE read from buffer */
                                TEEC_MEMREF_TEMP_OUTPUT,
                                TEEC_MEMREF_TEMP_INPUT,
                                TEEC_NONE);

    sOperation.params[0].tmpref.buffer = (void*)plainText;
    sOperation.params[0].tmpref.size   = plainTextLength;
    sOperation.params[1].tmpref.buffer = &offset;
    sOperation.params[1].tmpref.size   = 4;
    sOperation.params[2].tmpref.buffer = &userID;
    sOperation.params[2].tmpref.size   = 4;
    printf("size=%d\n", plainTextLength);

    nError = TEEC_InvokeCommand(session,
                                CMD_RPMB_WRITE_BY_OFFSET,
                                &sOperation,       /* IN OUT operation */
                                NULL               /* OUT returnOrigin, optional */
                               );

    //*nCipherTextLength = sOperation.params[1].tmpref.size;

    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_InvokeCommand failed (%08x), %d", __func__, nError, __LINE__);
    }

    return nError;
}

#ifdef CFG_RPMB_KEY_PROGRAMED_IN_KERNEL
TEEC_Result caRpmbSetKey(uint32_t *result)
{
    TEEC_Result    nError;
    TEEC_Operation sOperation;

    LOG_I("[Tlc rpmb] %s", __func__);

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(
                                TEEC_NONE,
                                TEEC_NONE,
                                TEEC_MEMREF_TEMP_INPUT,
                                TEEC_VALUE_OUTPUT);

    uint32_t userID = 0;
    sOperation.params[2].tmpref.buffer = &userID;
    sOperation.params[2].tmpref.size   = 4;
    nError = TEEC_InvokeCommand(session,
                                CMD_RPMB_PROGRAM_KEY,
                                &sOperation,       /* IN OUT operation */
                                NULL               /* OUT returnOrigin, optional */
                               );

    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: TEEC_InvokeCommand failed (%08x), %d", __func__, nError, __LINE__);
    }
    *result = sOperation.params[3].value.a;

    return nError;
}
#endif


// -------------------------------------------------------------
void caClose(void)
{
    TEEC_Result       nError;

    LOG_I("[Tlc rpmb] %s", __func__);

    /* Close the session */
    nError = r13CloseSession(session);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: r13CloseSession failed (%08x), %d", __func__, nError, __LINE__);
    }

    /* Finalize */
    nError = r13Finalize(context);
    if (nError != TEEC_SUCCESS) {
        LOG_E("[Tlc rpmb] %s: r13Finalize failed (%08x), %d", __func__, nError, __LINE__);
    }
}



