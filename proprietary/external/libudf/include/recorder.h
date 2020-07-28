#ifndef RECORDER_H
#define RECORDER_H

#include <stdlib.h>
#include <ubrd_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "ubrd_debug_mtk"
#include <log/log.h>

#define UBRD_EXPORT __attribute__ ((visibility("default")))

// #define LOCAL_DEBUG   // define for dump debug log

#define BYPASS_XLOG_TAG 0

#if BYPASS_XLOG_TAG
int __ubrd_log_buf_print(int bufID, int prio, const char *tag, const char *fmt, ...);

#ifdef LOCAL_DEBUG
#define ubrd_debug_log(...)  \
    __ubrd_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__);
#else
#define ubrd_debug_log(format, ...)  do { } while(0)
#endif

#define ubrd_error_log(...)  \
    __ubrd_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ubrd_warn_log(...)  \
    __ubrd_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ubrd_info_log(...)  \
    __ubrd_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#else
#ifdef LOCAL_DEBUG
#define ubrd_debug_log(format, ...)  \
    ALOGD(format, ##__VA_ARGS__)
#else
#define ubrd_debug_log(format, ...)  do { } while(0)
#endif

#define ubrd_error_log(format, ...)  \
    ALOGE(format, ##__VA_ARGS__)
#define ubrd_warn_log(format, ...)  \
    ALOGW(format, ##__VA_ARGS__)
#define ubrd_info_log(format, ...)  \
    ALOGI(format, ##__VA_ARGS__)
#endif

#define SIZE_FLAG_ZYGOTE_CHILD  (1<<31)
#define SIZE_FLAG_MASK          (SIZE_FLAG_ZYGOTE_CHILD)

#define BT_HASH_TABLE_SIZE      1543

#define HASH_TABLE_BITS 15
#define HASH_TABLE_SIZE (1 << HASH_TABLE_BITS)

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

typedef enum {
    UBRD_HISTORICAL_TABLE = 0,
    UBRD_RING_BUFFER = 1
} UBRD_RINGBUF_MODE;

PUBRD_BtEntry recordBacktrace(PUBRD pUBRD, size_t size);
PUBRD_HashEntry recordToHashTable(PUBRD pUBRD, size_t hashKey, PUBRD_EntryInfo pEntryInfo, PUBRD_BtEntry pBtEntry);
void* mspaceAllocate(PUBRD pUBRD, size_t bytes);
void mspaceFree(PUBRD pUBRD, void* mem);


#ifndef mspace
typedef void* mspace;
#endif

mspace create_mspace_with_base(void* base, size_t capacity, int locked);
void* mspace_malloc(mspace msp, size_t bytes);
void mspace_free(mspace msp, void* mem);
size_t destroy_mspace(mspace msp);
mspace create_mspace(size_t capacity, int locked);


#ifdef __cplusplus
}
#endif
#endif // ##ifndef RECORDER_H
