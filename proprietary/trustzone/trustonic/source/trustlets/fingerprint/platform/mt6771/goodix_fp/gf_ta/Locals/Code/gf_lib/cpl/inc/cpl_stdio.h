/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef _CPL_STDIO_H_
#define _CPL_STDIO_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t cpl_sprintf(int8_t* buffer, const int8_t* format, ...);

int32_t cpl_printf(const int8_t *format, ...);

#ifdef __cplusplus
}
#endif

#endif //_CPL_STDIO_H_
