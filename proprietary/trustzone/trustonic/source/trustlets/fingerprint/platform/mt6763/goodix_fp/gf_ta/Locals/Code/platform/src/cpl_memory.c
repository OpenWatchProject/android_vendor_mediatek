/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_tee_stdlib.h"
#include "cpl_memory.h"

#include "gf_memmgr.h"
#include "gf_tee_internal_api.h"

#define LOG_TAG "cpl_memory"
#ifdef __SUPPORT_GF_MEMMGR
    extern gf_mem_config_t g_mem_config;
#endif

void *cpl_malloc(int32_t size, uint8_t defaultValue, const int8_t *pFilename,
        const int8_t *pFuncName, int32_t line) {
    void* addr = NULL;
    UNUSED_VAR(pFilename);
    UNUSED_VAR(line);
    UNUSED_VAR(pFuncName);
#ifdef __SUPPORT_GF_MEMMGR
    if (g_mem_config.memmgr_enable > 0) {
        addr = gf_memmgr_malloc(size, defaultValue, pFilename, pFuncName, line);
    } else {
        addr = gf_malloc_with_default_value(size, defaultValue);
    }
#else
    addr = gf_malloc_with_default_value(size, defaultValue);
#endif
    return addr;
}

void *cpl_realloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName,
        int32_t line) {
    void* addr = NULL;
    UNUSED_VAR(pFilename);
    UNUSED_VAR(line);
    UNUSED_VAR(pFuncName);
#ifdef __SUPPORT_GF_MEMMGR
    if (g_mem_config.memmgr_enable > 0) {
        addr = gf_memmgr_realloc(pSrc, size, pFilename, pFuncName, line);
    } else {
        addr = gf_realloc(pSrc, size);
    }
#else
    addr = gf_realloc(pSrc, size);
#endif
    return addr;
}

void cpl_free(void *pMemAddr) {
    // Note: both gf_memmgr_free and gf_free check the validity of input pointer themselves,
    // so we do not need to check it again here.
#ifdef __SUPPORT_GF_MEMMGR
    // if (NULL != pMemAddr) {
    if (g_mem_config.memmgr_enable > 0) {
        gf_memmgr_free(pMemAddr);
    } else {
        gf_free(pMemAddr);
    }
    // }
#else
    gf_free(pMemAddr);
#endif
}

int32_t cpl_memcmp(const void *buf1, const void *buf2, uint32_t count) {
    return memcmp(buf1, buf2, count);
}
