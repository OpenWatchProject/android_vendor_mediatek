#ifndef __GF_LOG_H__
#define __GF_LOG_H__
#ifndef __TLSTD_H__
#include "taStd.h"
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *   Begin to define macros for printing log                             *
 *************************************************************************/
#define GF_LOG_DEBUG_LEVEL   3
#define GF_LOG_INFO_LEVEL    2
#define GF_LOG_ERROR_LEVEL   1

#ifndef GF_LOG_LEVEL
#define GF_LOG_LEVEL    1
#endif

#ifndef __TEE_INTERNAL_API_H__
extern void TEE_LogPrintf(const char *fmt, ...);
#endif

/* debug */
#define GF_LOGD(...)                     \
if( GF_LOG_LEVEL >= GF_LOG_DEBUG_LEVEL ) { \
    TEE_LogPrintf("[GF_TA][DEBUG]"__VA_ARGS__); \
    TEE_LogPrintf("\n");                 \
}

/* info */
#define GF_LOGI(...)                     \
if( GF_LOG_LEVEL >= GF_LOG_INFO_LEVEL )  { \
    TEE_LogPrintf("[GF_TA][INFO]"__VA_ARGS__);  \
    TEE_LogPrintf("\n");                 \
}

/* error */
#define GF_LOGE(...)                     \
if( GF_LOG_LEVEL >= GF_LOG_ERROR_LEVEL ) { \
    TEE_LogPrintf("[GF_TA][ERR]"__VA_ARGS__);   \
    TEE_LogPrintf("\n");                 \
}

/*************************************************************************
 *     End to define macros for printing log                             *
 *************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
