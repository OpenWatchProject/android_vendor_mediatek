#ifndef AUDIO_LOCK_H
#define AUDIO_LOCK_H

#include <string.h>

#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>

#include <semphr.h>
#include <queue.h>

#include <audio_assert.h>


#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define LOCK_TIMEOUT_MS (1000)


#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
#define ALOCK_TRACE_NUM (8) /* MAX 127 */
#endif


/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE

struct alock_log_unit_t {
    const char *type;
    const char *name;
    const char *file;
    const char *func;
    uint16_t line;
    uint16_t time_ms;
    uint32_t time_sec;
};

struct alock_trace_info_t {
    struct alock_log_unit_t log[ALOCK_TRACE_NUM];
    uint64_t lock_time_start;
    uint64_t lock_time_stop;
    SemaphoreHandle_t idx_lock;
    uint8_t idx     : 7;
    uint8_t timeout : 1;
};
#endif


struct alock_t {
    SemaphoreHandle_t mutex;
    QueueHandle_t     cond;
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    struct alock_trace_info_t trace_info;
#endif
};



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

const char *get_filename(const char *file);

int alock_new(
    struct alock_t **pp_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);

int alock_free(
    struct alock_t **pp_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);

int alock_lock_no_timeout(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);

int alock_trylock(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);

int alock_lock_ms(
    struct alock_t *p_alock, const char *alock_name, const uint32_t ms,
    const char *file, const char *func, const uint32_t line);

int alock_unlock(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);

int alock_wait_no_timeout(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);

int alock_wait_ms(
    struct alock_t *p_alock, const char *alock_name, const uint32_t ms,
    const char *file, const char *func, const uint32_t line);

int alock_signal(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line);




#define NEW_ALOCK(alock) \
    ({ \
        int __ret = alock_new(&(alock), #alock, \
                              get_filename(__FILE__), \
                              (const char *)__FUNCTION__, \
                              __LINE__); \
        if (__ret != 0) { AUD_WARNING("new lock fail!!"); } \
        __ret; \
    })


#define FREE_ALOCK(alock) \
    ({ \
        int __ret = alock_free(&(alock), #alock, \
                               get_filename(__FILE__), \
                               (const char *)__FUNCTION__, \
                               __LINE__); \
        if (__ret != 0) { AUD_WARNING("free lock fail!!"); } \
        __ret; \
    })


#define LOCK_ALOCK_NO_TIMEOUT(alock) \
    ({ \
        int __ret = alock_lock_no_timeout((alock), #alock, \
                                          get_filename(__FILE__), \
                                          (const char *)__FUNCTION__, \
                                          __LINE__); \
        if (__ret != 0) { AUD_WARNING("lock fail!!"); } \
        __ret; \
    })


#define LOCK_ALOCK_TRYLOCK(alock) \
    ({ \
        int __ret = alock_trylock((alock), #alock, \
                                  get_filename(__FILE__), \
                                  (const char *)__FUNCTION__, \
                                  __LINE__); \
        __ret; \
    })


#define LOCK_ALOCK_MS(alock, ms) \
    ({ \
        int __ret = alock_lock_ms((alock), #alock, ms, \
                                  get_filename(__FILE__), \
                                  (const char *)__FUNCTION__, \
                                  __LINE__); \
        if (__ret != 0) { AUD_WARNING("lock timeout!!"); } \
        __ret; \
    })


#define LOCK_ALOCK(alock) \
    LOCK_ALOCK_MS((alock), LOCK_TIMEOUT_MS)


#define UNLOCK_ALOCK(alock) \
    ({ \
        int __ret = alock_unlock((alock), #alock, \
                                 get_filename(__FILE__), \
                                 (const char *)__FUNCTION__, \
                                 __LINE__); \
        if (__ret != 0) { AUD_WARNING("unlock fail!!"); } \
        __ret; \
    })


#define WAIT_ALOCK(alock) \
    ({ \
        int __ret = alock_wait_no_timeout((alock), #alock, \
                                          get_filename(__FILE__), \
                                          (const char *)__FUNCTION__, \
                                          __LINE__); \
        __ret; \
    })


#define WAIT_ALOCK_MS(alock, ms) \
    ({ \
        int __ret = alock_wait_ms((alock), #alock, ms, \
                                  get_filename(__FILE__), \
                                  (const char *)__FUNCTION__, \
                                  __LINE__); \
        __ret; \
    })


#define SIGNAL_ALOCK(alock) \
    ({ \
        int __ret = alock_signal((alock), #alock, \
                                 get_filename(__FILE__), \
                                 (const char *)__FUNCTION__, \
                                 __LINE__); \
        if (__ret != 0) { AUD_WARNING("signal fail!!"); } \
        __ret; \
    })



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_LOCK_H */

