/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
/**
 * Copyright (C) 2016 The YunOS Project. All rights reserved.
 */

#ifndef __TEE_TYPES_H__
#define __TEE_TYPES_H__

#include <stdbool.h>
#include <stddef.h> /* size_t */

#ifndef false
#define false   (0)
#endif

#ifndef true
#define true    (1)
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef __ASSEMBLY__

typedef char            int8_t;
typedef unsigned char   uint8_t;
typedef short           int16_t;  // NOLINT
typedef unsigned short  uint16_t;  // NOLINT
typedef int             int32_t;
typedef unsigned int    uint32_t;

typedef long            long_t;  // NOLINT
typedef unsigned long   ulong_t;  // NOLINT

typedef signed long int ssize_t;  // NOLINT

typedef unsigned long long    uint64_t;  // NOLINT
typedef long long    int64_t;  // NOLINT

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFFUL
#endif

#endif /* __ASSEMBLY__ */

#endif  // __TEE_TYPES_H__
