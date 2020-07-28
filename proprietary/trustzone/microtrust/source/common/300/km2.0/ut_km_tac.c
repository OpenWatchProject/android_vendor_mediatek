/*
 * Copyright (C) 2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#define IMSG_TAG "ut_kmsetkey"

#include <tee_client_api.h>
#include <string.h>
#include <imsg_log.h>
#include "ut_km_tac.h"

static const char* host_name = "bta_loader";

static const TEEC_UUID KEYBOX_UUID =  { 0xd91f322a, 0xd5a4, 0x41d5, { 0x95, 0x51, 0x10, 0xed, 0xa3, 0x27, 0x2f, 0xc0 } };


int ut_km_import_google_key(unsigned char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0)
        return -1;

    TEEC_Context context;
    TEEC_Session session ;
    TEEC_Operation operation ;
	TEEC_SharedMemory inputSM;
    TEEC_Result result;
    uint32_t returnOrigin = 0;

    memset((void *)&context, 0, sizeof(TEEC_Context));
    memset((void *)&session, 0, sizeof(TEEC_Session));
    memset((void *)&operation, 0, sizeof(TEEC_Operation));

    result = TEEC_InitializeContext(host_name, &context);
    if(result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_InitializeContext FAILED, err:%x",result);
        goto cleanup_1;
    }

    result = TEEC_OpenSession(
            &context,
            &session,
            &KEYBOX_UUID,
            TEEC_LOGIN_PUBLIC,
            NULL,
            NULL,
            &returnOrigin);
    if(result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_OpenSession FAILED, err:%x",result);
        goto cleanup_2;
    }

	inputSM.size = datalen;
	inputSM.flags = TEEC_MEM_INPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		IMSG_ERROR("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);
	memcpy(inputSM.buffer, data, datalen);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
			TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
    operation.started = 1;

    result = TEEC_InvokeCommand(&session,0,&operation,NULL);
    if (result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_InvokeCommand FAILED, err:%x",result);
        goto cleanup_4;
    }

    result = operation.params[0].value.a;
    if (result != TEEC_SUCCESS) {
        IMSG_ERROR("ta busness error, %d", result);
    }

	cleanup_4:
        TEEC_ReleaseSharedMemory(&inputSM);
    cleanup_3:
        TEEC_CloseSession(&session);
    cleanup_2:
        TEEC_FinalizeContext(&context);
    cleanup_1:
        return result;
}
