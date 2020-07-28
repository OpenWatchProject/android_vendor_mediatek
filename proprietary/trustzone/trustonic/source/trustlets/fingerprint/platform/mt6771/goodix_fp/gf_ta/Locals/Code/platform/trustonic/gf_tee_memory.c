/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include "gf_error.h"

#define LOG_TAG "[gf_tee_memory]"

void* gf_malloc(int32_t size) {
    return TEE_Malloc(size, 0);
}

void gf_free(void *buffer) {
    if (NULL != buffer) {
        TEE_Free(buffer);
    }
}

void* gf_malloc_with_default_value(int32_t size, uint8_t default_value) {
    return TEE_Malloc(size, default_value);
}

void* gf_realloc(void* buf, uint32_t buf_len) {
    return TEE_Realloc(buf, buf_len);
}
