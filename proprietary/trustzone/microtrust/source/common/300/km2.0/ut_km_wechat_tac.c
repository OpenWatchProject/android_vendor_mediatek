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
#include <malloc.h>
#include <imsg_log.h>
#include <ctype.h>
#include <stdio.h>
#include "ut_km_wechat_tac.h"
#include "ut_km_def.h"
#include "ut_kmwechat.h"

static const char* host_name = "bta_loader";

static const TEEC_UUID WECHAT_UUID =  { 0xd91f322a, 0xd5a4, 0x41d5, { 0x95, 0x51, 0x10, 0xed, 0xa3, 0x27, 0x2f, 0xc0 } };
static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char* p_pubkey_begin_string = "-----BEGIN PUBLIC KEY-----\n";
const char* p_pubkey_end_string = "\n-----END PUBLIC KEY-----";

#if 0
#define HEXDUMP_COLS 16
void kb_dump(uint8_t *mem, uint32_t len)
{
    uint32_t i, j;

    if (mem == NULL || len ==0)
        return;

    for (i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len %
	                       HEXDUMP_COLS) : 0); i++) {
		/* print offset */
		if (i % HEXDUMP_COLS == 0) {
			printf("0x%06x: ", i);
		}

		/* print hex data */
		if (i < len) {
			printf("%02x ", 0xFF & ((char*) mem)[i]);
		} else { /* end of block, just aligning for ASCII dump */
			printf("   ");
		}

		/* print ASCII dump */
		if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
			for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
				if (j >= len) { /* end of block, not really printing */
					putchar(' ');
				} else if (isprint(((char*) mem)[j])) { /* printable char */
					putchar(0xFF & ((char*) mem)[j]);
				} else { /* other char */
					putchar('.');
				}
			}

			putchar('\n');
		}
	}

}
#endif

static void base64_encodeblock(uint8_t in[3], uint8_t out[4], int len)
{
	out[0] = cb64[in[0] >> 2];
	out[1] = cb64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
	out[2] = (uint8_t) (len > 1 ? cb64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)] : '=');
	out[3] = (uint8_t) (len > 2 ? cb64[in[2] & 0x3f] : '=');
}

int base64_encode(const uint8_t *in_data, unsigned long in_len, uint8_t *out_data, int cr_lf)
{
	uint8_t in[3], out[4];
	int i, len = 0;
	int CRLF = 0;
	unsigned int in_count = 0, out_count = 0;

	while (in_count < in_len) {
		len = 0;

		for (i = 0; i < 3; i++) {
			if (in_count < in_len) {
				in[i] = (uint8_t) in_data[in_count++];
				len++;
			} else {
				in[i] = 0;
			}
		}

		if (len) {
			base64_encodeblock(in, out, len);

			for (i = 0; i < 4; i++) {
				out_data[out_count++] = out[i];

				if (cr_lf > 0 && ++CRLF % cr_lf == 0) {
					out_data[out_count++] = '\n';
					CRLF = 0;
				}
			}
		}
	}

	return out_count;
}

kmwechat_error_t wechat_generate_attk(const unsigned char copy_num,
                  unsigned char *device, unsigned int dlen)
{

    if (copy_num == 0 || device == NULL || dlen == 0 || dlen >4) {
        IMSG_ERROR("invalid input, copy_num=%d, dlen=%d", copy_num, dlen);
        return KMSK_ERROR_INVALID_INPUT;
    }
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
            &WECHAT_UUID,
            TEEC_LOGIN_PUBLIC,
            NULL,
            NULL,
            &returnOrigin);
    if(result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_OpenSession FAILED, err:%x",result);
        goto cleanup_2;
    }

	inputSM.size = dlen;
	inputSM.flags = TEEC_MEM_INPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		IMSG_ERROR("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);
	memcpy(inputSM.buffer, device, dlen);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
			TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
    operation.started = 1;

    operation.params[0].value.a = copy_num;

    result = TEEC_InvokeCommand(&session,CMD_CA_GENERATE_ATTK,&operation,NULL);
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


kmwechat_error_t wechat_verify_attk(void)
{
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
            &WECHAT_UUID,
            TEEC_LOGIN_PUBLIC,
            NULL,
            NULL,
            &returnOrigin);
    if(result != TEEC_SUCCESS) {
        IMSG_ERROR("TEEC_OpenSession FAILED, err:%x",result);
        goto cleanup_2;
    }


    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
            TEEC_NONE, TEEC_NONE, TEEC_NONE);

    result = TEEC_InvokeCommand(&session, CMD_CA_VERIFY_ATTK, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        IMSG_ERROR("TEEC_InvokeCommand FAILED, err:%x",result);
        goto cleanup_3;
    }

    result = operation.params[0].value.a;
    if (result != TEEC_SUCCESS) {
        IMSG_ERROR("ta busness error, %d", result);
    }

    cleanup_3:
        TEEC_CloseSession(&session);
    cleanup_2:
        TEEC_FinalizeContext(&context);
    cleanup_1:
        return result;
}


kmwechat_error_t wechat_export_attk(unsigned char** key_data, size_t* key_datalen)
{

    if (key_data == NULL || key_datalen == NULL) {
        IMSG_ERROR("wechat_export_attk invalid input");
        return KMSK_ERROR_INVALID_INPUT;
    }
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
            &WECHAT_UUID,
            TEEC_LOGIN_PUBLIC,
            NULL,
            NULL,
            &returnOrigin);
    if(result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_OpenSession FAILED, err:%x",result);
        goto cleanup_2;
    }

	inputSM.size = 2048;  //attk size 2048 bytes
	inputSM.flags = TEEC_MEM_OUTPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		IMSG_ERROR("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
			TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
    operation.started = 1;

    result = TEEC_InvokeCommand(&session,CMD_CA_EXPORT_ATTK,&operation,NULL);
    if (result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_InvokeCommand FAILED, err:%x",result);
        goto cleanup_4;
    }

    result = operation.params[0].value.a;
    if (result != TEEC_SUCCESS) {
        IMSG_ERROR("ta busness error, %d", result);
        goto cleanup_4;
    }




    *key_datalen = operation.params[1].memref.size;
    *key_data = (unsigned char *)malloc(*key_datalen + 1);
    if (*key_data == NULL) {
        IMSG_ERROR("malloc error");
        result = KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
        goto cleanup_4;
    }
    memset(*key_data, 0, *key_datalen + 1);

    memcpy(*key_data, inputSM.buffer, *key_datalen);

	cleanup_4:
        TEEC_ReleaseSharedMemory(&inputSM);
    cleanup_3:
        TEEC_CloseSession(&session);
    cleanup_2:
        TEEC_FinalizeContext(&context);
    cleanup_1:

    return result;
}


kmwechat_error_t wechat_get_device_id(unsigned char** key_data, size_t* key_datalen)
{

    if (key_data == NULL || key_datalen == NULL) {
        IMSG_ERROR("wechat_export_attk invalid input");
        return KMSK_ERROR_INVALID_INPUT;
    }
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
            &WECHAT_UUID,
            TEEC_LOGIN_PUBLIC,
            NULL,
            NULL,
            &returnOrigin);
    if(result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_OpenSession FAILED, err:%x",result);
        goto cleanup_2;
    }

	inputSM.size = 128;  //device id 128 bytes
	inputSM.flags = TEEC_MEM_OUTPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		IMSG_ERROR("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
			TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
    operation.started = 1;

    result = TEEC_InvokeCommand(&session,CMD_CA_GET_UUID,&operation,NULL);
    if (result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_InvokeCommand FAILED, err:%x",result);
        goto cleanup_4;
    }

    result = operation.params[0].value.a;
    if (result != TEEC_SUCCESS) {
        IMSG_ERROR("ta busness error, %d", result);
        goto cleanup_4;
    }

    *key_datalen = operation.params[1].memref.size;
    *key_data = (unsigned char *)malloc(*key_datalen);
    if (*key_data == NULL) {
        IMSG_ERROR("malloc error");
        result = KMSK_ERROR_MEMORY_ALLOCATION_FAILED;
        goto cleanup_4;
    }

    memcpy(*key_data, inputSM.buffer, *key_datalen);

	cleanup_4:
        TEEC_ReleaseSharedMemory(&inputSM);
    cleanup_3:
        TEEC_CloseSession(&session);
    cleanup_2:
        TEEC_FinalizeContext(&context);
    cleanup_1:
        return result;
}


kmwechat_error_t wechat_get_wechat_info(wechat_info_t *wechat_info)
{

    if (wechat_info == NULL) {
        IMSG_ERROR("wechat_export_attk invalid input");
        return KMSK_ERROR_INVALID_INPUT;
    }
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
            &WECHAT_UUID,
            TEEC_LOGIN_PUBLIC,
            NULL,
            NULL,
            &returnOrigin);
    if(result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_OpenSession FAILED, err:%x",result);
        goto cleanup_2;
    }

	inputSM.size = sizeof(wechat_info_t);  //device id 128 bytes
	inputSM.flags = TEEC_MEM_OUTPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		IMSG_ERROR("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
			TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
    operation.started = 1;

    result = TEEC_InvokeCommand(&session,CMD_CA_GET_WECHAT_INFO,&operation,NULL);
    if (result != TEEC_SUCCESS) {
		IMSG_ERROR("TEEC_InvokeCommand FAILED, err:%x",result);
        goto cleanup_4;
    }

    result = operation.params[0].value.a;
    if (result != TEEC_SUCCESS) {
        IMSG_ERROR("ta busness error, %d", result);
        goto cleanup_4;
    }

    memcpy(wechat_info, inputSM.buffer, inputSM.size);

	cleanup_4:
        TEEC_ReleaseSharedMemory(&inputSM);
    cleanup_3:
        TEEC_CloseSession(&session);
    cleanup_2:
        TEEC_FinalizeContext(&context);
    cleanup_1:
        return result;
}
