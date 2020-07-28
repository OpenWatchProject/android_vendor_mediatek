/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_LOG_H__
#define __GF_LOG_H__
#ifndef __TLSTD_H__
#include "taStd.h"
#endif
#include <stdint.h>
#include "gf_error.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 *   Begin to define macros for printing log                             *
 *************************************************************************/
#define GF_LOG_VERBOSE_LEVEL   4
#define GF_LOG_DEBUG_LEVEL   3
#define GF_LOG_INFO_LEVEL    2
#define GF_LOG_ERROR_LEVEL   1

#ifndef GF_LOG_LEVEL
#define GF_LOG_LEVEL    1
#endif

extern void TEE_LogPrintf(const char *fmt, ...);
// extern void tlApiLogPrintf(const char *fmt, ...);

/* verbose */
#define GF_LOGV(...)                                 \
if( GF_LOG_LEVEL >= GF_LOG_VERBOSE_LEVEL ) {           \
    TEE_LogPrintf("[GF_IFAA_TA][V]"__VA_ARGS__); \
    TEE_LogPrintf("\n");                             \
}

/* debug */
#define GF_LOGD(...)                                 \
if( GF_LOG_LEVEL >= GF_LOG_DEBUG_LEVEL ) {           \
    TEE_LogPrintf("[GF_IFAA_TA][D]"__VA_ARGS__); \
    TEE_LogPrintf("\n");                             \
}

/* info */
#define GF_LOGI(...)                                 \
if( GF_LOG_LEVEL >= GF_LOG_INFO_LEVEL )  {           \
    TEE_LogPrintf("[GF_IFAA_TA]I]"__VA_ARGS__);  \
    TEE_LogPrintf("\n");                             \
}

/* error */
#define GF_LOGE(...)                                 \
if( GF_LOG_LEVEL >= GF_LOG_ERROR_LEVEL ) {           \
    TEE_LogPrintf("[GF_IFAA_TA][E]"__VA_ARGS__);   \
    TEE_LogPrintf("\n");                             \
}

/* FUNC_ENTER */
#define FUNC_ENTER() GF_LOGV(LOG_TAG "[%s] enter", __func__)

/* FUNC_EXIT */
#define FUNC_EXIT(err)                               \
    do {                                             \
        if (GF_SUCCESS == (err)) {                     \
            GF_LOGV(LOG_TAG "[%s] exit", __func__);  \
        } else {                                     \
            GF_LOGE(LOG_TAG "[%s] exit. err=%s, errno=%d", __func__, gf_strerror(err), err); \
        }                                            \
    } while (0)

/* VOID_FUNC_ENTER */
#define VOID_FUNC_ENTER() GF_LOGV(LOG_TAG "[%s] enter", __func__)

/* VOID_FUNC_EXIT */
#define VOID_FUNC_EXIT()  GF_LOGV(LOG_TAG "[%s] exit", __func__)

/*************************************************************************
 *     End to define macros for printing log                             *
 *************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  // __GF_LOG_H__
