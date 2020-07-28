/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __CPL_STRING_H__
#define __CPL_STRING_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int8_t *cpl_strcpy(int8_t *pDestStr, const int8_t *pSrcStr);

int8_t *cpl_strncpy(int8_t *pDestStr, const int8_t *pSrcStr, uint32_t count);

uint32_t cpl_strlen(const int8_t *pStr);

void cpl_memcpy(void *pDest, const void *pSrc, uint32_t count);

void *cpl_memset(void *pSrc, int32_t c, uint32_t count);

int cpl_strncmp(const int8_t *s1, const int8_t *s2, uint32_t n);

int8_t *cpl_strcat(int8_t *pDestStr, const int8_t *pSrcStr);

#ifdef __cplusplus
}
#endif

#endif  // __CPL_STRING_H__
