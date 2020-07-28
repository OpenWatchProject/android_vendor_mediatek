/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include "gf_error.h"

#define LOG_TAG "[gf_tee_memory]"

void* gf_malloc(int32_t size) {
    void *addr = NULL;

    // VOID_FUNC_ENTER();
    addr = TEE_Malloc(size, 0);
    // VOID_FUNC_EXIT();

    return addr;
}

void gf_free(void *buffer) {
    // VOID_FUNC_ENTER();
    if (NULL != buffer) {
        TEE_Free(buffer);
    }
    // VOID_FUNC_EXIT();
}

void* gf_malloc_with_default_value(int32_t size, uint8_t default_value) {
    void *addr = NULL;

    // VOID_FUNC_ENTER();
    addr = TEE_Malloc(size, default_value);
    // VOID_FUNC_EXIT();

    return addr;
}

void* gf_realloc(void* buf, uint32_t buf_len) {
    void *addr = NULL;

    // VOID_FUNC_ENTER();
    addr = TEE_Realloc(buf, buf_len);
    // VOID_FUNC_EXIT();

    return addr;
}
