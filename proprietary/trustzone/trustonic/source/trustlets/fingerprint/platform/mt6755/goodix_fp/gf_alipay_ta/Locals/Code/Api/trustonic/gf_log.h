#ifndef __GF_LOG_H__
#define __GF_LOG_H__
#ifndef __TLSTD_H__
#include "taStd.h"
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_VAR(X)   (void)(X)

/*************************************************************************
 *   Begin to define macros for printing log                             *
 *************************************************************************/
#define GF_LOG_DEBUG_LEVEL   0
#define GF_LOG_INFO_LEVEL    1
#define GF_LOG_ERROR_LEVEL   2

extern void TEE_LogPrintf(const char *fmt, ...);
extern void tlApiLogPrintf(const char *fmt, ...);

/* debug */
#define GF_LOGD(...)                     \
if(GF_LOG_DEBUG_LEVEL >= GF_LOG_LEVEL) { \
    TEE_LogPrintf("[DEBUG]"__VA_ARGS__); \
    TEE_LogPrintf("\n");                 \
}

/* info */
#define GF_LOGI(...)                     \
if(GF_LOG_INFO_LEVEL >= GF_LOG_LEVEL)  { \
    TEE_LogPrintf("[INFO]"__VA_ARGS__);  \
    TEE_LogPrintf("\n");                 \
}

/* error */
#define GF_LOGE(...)                     \
if(GF_LOG_ERROR_LEVEL >= GF_LOG_LEVEL) { \
    TEE_LogPrintf("[ERR]"__VA_ARGS__);   \
    TEE_LogPrintf("\n");                 \
}

/*************************************************************************
 *     End to define macros for printing log                             *
 *************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // __GF_TEE_INTERNAL_API_H__
