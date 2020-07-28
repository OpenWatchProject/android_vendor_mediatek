/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __CPL_STDIO_H__
#define __CPL_STDIO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t cpl_sprintf(int8_t* buffer, const int8_t* format, ...);

int32_t cpl_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif  // __CPL_STDIO_H__
