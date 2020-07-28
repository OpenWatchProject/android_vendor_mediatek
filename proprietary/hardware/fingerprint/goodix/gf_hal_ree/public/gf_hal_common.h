#ifndef __GF_HAL_COMMON_H__
#define __GF_HAL_COMMON_H__

#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GF_LOG_LEVEL
#define GF_LOG_LEVEL 2
#endif

#define GF_LOG_DEBUG_LEVEL   3
#define GF_LOG_INFO_LEVEL    2
#define GF_LOG_ERROR_LEVEL   1

#define LOG_D(...) \
do { \
    if( GF_LOG_LEVEL >= GF_LOG_DEBUG_LEVEL ) { \
        __android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__); \
    } \
} while (0);


#define LOG_I(...) \
do { \
    if( GF_LOG_LEVEL >= GF_LOG_INFO_LEVEL ) { \
        __android_log_print(ANDROID_LOG_INFO, __VA_ARGS__); \
    } \
} while (0);

#define LOG_E(...) \
do { \
    if( GF_LOG_LEVEL >= GF_LOG_ERROR_LEVEL ) { \
        __android_log_print(ANDROID_LOG_ERROR, __VA_ARGS__); \
    } \
} while (0);

#ifndef UNUSED_VAR
#define UNUSED_VAR(X)   (void)(X)
#endif

#ifdef __cplusplus
}
#endif

#endif // __GF_HAL_COMMON_H__
