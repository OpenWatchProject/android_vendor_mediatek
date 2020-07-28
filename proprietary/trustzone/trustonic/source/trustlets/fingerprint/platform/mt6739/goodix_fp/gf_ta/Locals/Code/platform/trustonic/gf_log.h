/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_LOG_H__
#define __GF_LOG_H__

#include "gf_error.h"

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

extern uint32_t gf_tee_log(const char *fmt, ...);

/*verbose*/
#if( GF_LOG_LEVEL >= GF_LOG_VERBOSE_LEVEL )
#define GF_LOGV(...) \
    gf_tee_log("[GF_TA][V]"__VA_ARGS__); \
    gf_tee_log("\n");
#else
#define GF_LOGV(...)
#endif

/* debug */
#if( GF_LOG_LEVEL >= GF_LOG_DEBUG_LEVEL )
#define GF_LOGD(...) \
    gf_tee_log("[GF_TA][D]"__VA_ARGS__); \
    gf_tee_log("\n");
#else
#define GF_LOGD(...)
#endif

/* info */
#if( GF_LOG_LEVEL >= GF_LOG_INFO_LEVEL )
#define GF_LOGI(...) \
    gf_tee_log("[GF_TA][I]"__VA_ARGS__); \
    gf_tee_log("\n");
#else
#define GF_LOGI(...)
#endif

/* error */
#if( GF_LOG_LEVEL >= GF_LOG_ERROR_LEVEL )
#define GF_LOGE(...) \
    gf_tee_log("[GF_TA][E]"__VA_ARGS__); \
    gf_tee_log("\n");
#else
#define GF_LOGE(...)
#endif

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
