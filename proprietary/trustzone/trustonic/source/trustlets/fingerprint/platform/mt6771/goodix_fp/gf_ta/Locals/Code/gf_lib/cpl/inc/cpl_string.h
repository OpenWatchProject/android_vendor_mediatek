/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef _CPL_STRING_H_
#define _CPL_STRING_H_

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

#ifdef __cplusplus
}
#endif

#endif //_CPL_STRING_H_
