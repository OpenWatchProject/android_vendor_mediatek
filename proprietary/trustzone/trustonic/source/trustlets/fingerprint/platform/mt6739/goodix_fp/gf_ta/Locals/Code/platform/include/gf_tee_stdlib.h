/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_TEE_STDLIB_H__
#define __GF_TEE_STDLIB_H__
#include "gf_type_define.h"

void *memcpy(void* destaddr, void const *srcaddr, size_t len);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t count);
void abort(void);
#endif  // __GF_TEE_STDLIB_H__
