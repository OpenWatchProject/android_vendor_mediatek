/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <stdlib.h>
#include "cpl_memory.h"
#include "gf_tee_internal_api.h"
#include "string.h"

#ifdef GF_MEMORY_DEBUG_ENABLE
#include "mem_manager.h"

void *cpl_malloc(int32_t size, uint8_t defaultValue, const int8_t *pFilename, const int8_t *pFuncName, int32_t line) {

    return MemMalloc(size, defaultValue, pFilename, pFuncName, line);
}

void *cpl_realloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName, int32_t line) {
    return MemRealloc(pSrc, size, pFilename, pFuncName, line);
}

void cpl_free(void *pMemAddr) {

    if (NULL != pMemAddr) {
        MemFree(pMemAddr);
    }
}

int32_t cpl_memcmp(const void *buf1, const void *buf2, uint32_t count) {
    return memcmp(buf1, buf2, count);
}

#else

void *cpl_malloc(int32_t size, uint8_t defaultValue, const int8_t *pFilename,
        const int8_t *pFuncName, int32_t line) {

    UNUSED_VAR(pFilename);
    UNUSED_VAR(line);
    UNUSED_VAR(pFuncName);

    return gf_malloc_with_default_value(size, defaultValue);
}

void *cpl_realloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName,
        int32_t line) {
    UNUSED_VAR(pFilename);
    UNUSED_VAR(line);
    UNUSED_VAR(pFuncName);

    return gf_realloc(pSrc, size);
}

void cpl_free(void *pMemAddr) {

    if (NULL != pMemAddr) {
        gf_free(pMemAddr);
    }
}

int32_t cpl_memcmp(const void *buf1, const void *buf2, uint32_t count) {
    return memcmp(buf1, buf2, count);
}
#endif
