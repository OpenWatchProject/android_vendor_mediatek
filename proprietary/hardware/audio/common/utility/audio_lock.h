#ifndef AUDIO_LOCK_H
#define AUDIO_LOCK_H

/* TODO: kernel & scp */
#include <string.h>

#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>
#include <time.h>

#include <audio_log.h>
#include <audio_assert.h>
#include <audio_memory_control.h>

#include <audio_time.h>


#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_LOCK_TIMEOUT_MS (1000)

#ifdef MTK_AUDIO_LOCK_ENABLE_LOG
#ifdef ALOCK_LOG
#undef ALOCK_LOG
#endif
#define ALOCK_LOG(x...) AUD_LOG_D(x)
#else
#define ALOCK_LOG(x...)
#endif

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
#define MAX_TRACE_NUM (8)
#endif


/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE

struct alock_time_t {
    uint32_t h: 5;   /* [0, 24)   in [0, 32) */
    uint32_t m: 6;   /* [0, 60)   in [0, 64) */
    uint32_t s: 6;   /* [0, 60)   in [0, 64) */
    uint32_t ms: 10; /* [0, 1000) in [0, 1024) */
};

struct alock_log_unit_t {
    const char *type;
    const char *name;
    const char *file;
    const char *func;
    struct alock_time_t time;
    uint32_t line;
    uint16_t pid;
    uint16_t tid;
};

struct alock_trace_info_t {
    struct alock_log_unit_t log[MAX_TRACE_NUM];
    struct timespec ts_start;
    struct timespec ts_stop;
    pthread_mutex_t idx_lock;
    uint8_t idx;
    bool timeout;
};
#endif


struct alock_t {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    struct alock_trace_info_t trace_info;
#endif
};



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

void alock_cleanup_handler(void *arg);

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
        if (__ret != 0) { AUD_WARNING_FT("new lock fail!!"); } \
        __ret; \
    })


#define FREE_ALOCK(alock) \
    ({ \
        int __ret = alock_free(&(alock), #alock, \
                               get_filename(__FILE__), \
                               (const char *)__FUNCTION__, \
                               __LINE__); \
        if (__ret != 0) { AUD_WARNING_FT("free lock fail!!"); } \
        __ret; \
    })


#define LOCK_ALOCK_NO_TIMEOUT(alock) \
    ({ \
        int __ret = alock_lock_no_timeout((alock), #alock, \
                                          get_filename(__FILE__), \
                                          (const char *)__FUNCTION__, \
                                          __LINE__); \
        if (__ret != 0) { AUD_WARNING_FT("lock fail!!"); } \
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
        if (__ret != 0) { AUD_WARNING_FT("lock timeout!!"); } \
        __ret; \
    })


#define LOCK_ALOCK(alock) \
    LOCK_ALOCK_MS((alock), MAX_LOCK_TIMEOUT_MS)


#define UNLOCK_ALOCK(alock) \
    ({ \
        int __ret = alock_unlock((alock), #alock, \
                                 get_filename(__FILE__), \
                                 (const char *)__FUNCTION__, \
                                 __LINE__); \
        if (__ret != 0) { AUD_WARNING_FT("unlock fail!!"); } \
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
        if (__ret != 0) { AUD_WARNING_FT("signal fail!!"); } \
        __ret; \
    })


#define CLEANUP_PUSH_ALOCK(alock) \
    pthread_cleanup_push(alock_cleanup_handler, (alock))


#define CLEANUP_POP_ALOCK(alock) \
    pthread_cleanup_pop(0)



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_LOCK_H */

