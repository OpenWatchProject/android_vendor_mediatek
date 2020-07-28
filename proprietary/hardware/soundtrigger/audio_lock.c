#include "audio_lock.h"

#include <string.h>

#include <errno.h>

#include <sys/time.h>
#include <sys/prctl.h>

#include <sys/types.h>
#include <unistd.h>




#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[ALOCK]"

#define MAX_SYS_TIME_TRY_COUNT (10)
#define MAX_WAIT_BLOCKED_BY_LOCK_MS (20)


#ifndef ANDROID
#define pthread_mutex_timedlock(a,b) pthread_mutex_lock(a)
#define pthread_mutex_trylock(a) pthread_mutex_lock(a)
#endif


enum {
    ALOCK_LOCK,
    ALOCK_TRYLOCK,
    ALOCK_UNLOCK,
    ALOCK_WAIT_IN,
    ALOCK_WAIT_OUT,
    ALOCK_SIGNAL,
    NUM_ALOCK_TYPE
};

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
static const char *alock_type_name[NUM_ALOCK_TYPE] = {
    "lock",
    "trylock",
    "unlock",
    "+wait",
    "-wait",
    "signal"
};
#endif

/*
 * =============================================================================
 *                     utilities implementation
 * =============================================================================
 */

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
static void alock_update_trace_info(
    const uint8_t type, struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    if (type >= NUM_ALOCK_TYPE) {
        AUD_LOG_E("%s(), %p: %s, %s, %s(), %uL, type %d FAIL!!",
                  __FUNCTION__, p_alock, alock_name, file, func, line, type);
        return;
    }

    pthread_mutex_lock(&p_alock->trace_info.idx_lock);
    uint8_t idx = p_alock->trace_info.idx;
    p_alock->trace_info.idx++;
    if (p_alock->trace_info.idx == MAX_TRACE_NUM) {
        p_alock->trace_info.idx = 0;
    }

    struct alock_log_unit_t *log_unit = &p_alock->trace_info.log[idx];

    log_unit->type = alock_type_name[type];
    log_unit->name = alock_name;
    log_unit->file = file;
    log_unit->func = func;

    char time_h[4] = {0};
    char time_m[4] = {0};
    char time_s[4] = {0};

    struct timespec ts;
    struct tm t;

    uint32_t ms = 0;

    audio_get_timespec_realtime(&ts);
    ms = ts.tv_nsec / 1000000L;
    if (ms > 999) {
        AUD_LOG_E("%s(), %p: %s, %s, %s(), %uL, ms %u FAIL!!",
                  __FUNCTION__, p_alock, alock_name, file, func, line, ms);
        ms = 0;
    }

    if (localtime_r(&ts.tv_sec, &t) == NULL) {
        memset(&log_unit->time, 0, sizeof(struct alock_time_t));
    } else if (strftime(time_h, 3, "%H", &t) == 0 ||
               strftime(time_m, 3, "%M", &t) == 0 ||
               strftime(time_s, 3, "%S", &t) == 0) {
        memset(&log_unit->time, 0, sizeof(struct alock_time_t));
    } else {
        log_unit->time.h = (uint8_t)atoi(time_h);
        log_unit->time.m = (uint8_t)atoi(time_m);
        log_unit->time.s = (uint8_t)atoi(time_s);
        log_unit->time.ms = ms;
    }

    log_unit->line = line;
    log_unit->pid = getpid();
    log_unit->tid = gettid();

    pthread_mutex_unlock(&p_alock->trace_info.idx_lock);
}
#endif


#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
static void alock_dump_trace_info(struct alock_t *p_alock) {
    p_alock->trace_info.timeout = true;

    pthread_mutex_lock(&p_alock->trace_info.idx_lock);

    struct alock_log_unit_t *log_unit = NULL;

    uint8_t idx  = p_alock->trace_info.idx + 1;
    if (idx == MAX_TRACE_NUM) {
        idx = 0;
    }

    AUD_LOG_E("%p: ========================= dump(+) =========================", p_alock);

    while (idx != p_alock->trace_info.idx) {
        log_unit = &p_alock->trace_info.log[idx];
        if (log_unit->type != NULL &&
            log_unit->name != NULL &&
            log_unit->file != NULL &&
            log_unit->func != NULL) {
            AUD_LOG_E("%p: time: \"%02u:%02u:%02u.%03u\", pid tid: \" %5d %5d \", %s(%s), %s, %s(), %uL",
                      p_alock,
                      log_unit->time.h,
                      log_unit->time.m,
                      log_unit->time.s,
                      log_unit->time.ms,
                      log_unit->pid,
                      log_unit->tid,
                      log_unit->type,
                      log_unit->name,
                      log_unit->file,
                      log_unit->func,
                      log_unit->line);
        }

        idx++;
        if (idx == MAX_TRACE_NUM) {
            idx = 0;
        }
    }

    AUD_LOG_E("%p: ========================= dump(-) =========================", p_alock);

    pthread_mutex_unlock(&p_alock->trace_info.idx_lock);
}
#endif


void alock_cleanup_handler(void *arg) {
    if (arg == NULL) {
        return;
    }

    struct alock_t *p_alock = (struct alock_t *)arg;
    UNLOCK_ALOCK(p_alock);
}


const char *get_filename(const char *file) {
    const char *slash = strrchr(file, '/');
    return (slash) ? slash + 1 : file;
}



int alock_new(
    struct alock_t **pp_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    struct alock_t *p_alock = (struct alock_t *)AUDIO_MALLOC(sizeof(struct alock_t));
    if (p_alock == NULL) {
        AUD_LOG_E("%p: new(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        *pp_alock = NULL;
        return -1;
    }
    *pp_alock = p_alock;

    pthread_mutex_init(&p_alock->mutex, NULL);
    pthread_cond_init(&p_alock->cond, NULL);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    memset(&p_alock->trace_info, 0, sizeof(struct alock_trace_info_t));
    pthread_mutex_init(&p_alock->trace_info.idx_lock, NULL);
#endif

    ALOCK_LOG("%p: new(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);
    return 0;
}


int alock_free(
    struct alock_t **pp_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    struct alock_t *p_alock = (struct alock_t *)*pp_alock;
    if (p_alock == NULL) {
        AUD_LOG_E("%p: free(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: free(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    pthread_mutex_destroy(&p_alock->trace_info.idx_lock);
#endif

    pthread_mutex_destroy(&p_alock->mutex);
    pthread_cond_destroy(&p_alock->cond);

    AUDIO_FREE(p_alock);
    p_alock = NULL;
    *pp_alock = NULL;

    return 0;
}


int alock_lock_no_timeout(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    if (p_alock == NULL) {
        AUD_LOG_E("%p: lock(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: lock(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

    pthread_mutex_lock(&p_alock->mutex);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_LOCK, p_alock, alock_name, file, func, line);
    audio_get_timespec_monotonic(&p_alock->trace_info.ts_start);
#endif

    return 0;
}


int alock_trylock(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    int retval = 0;

    if (p_alock == NULL) {
        AUD_LOG_E("%p: trylock(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    retval = -pthread_mutex_trylock(&p_alock->mutex);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    if (retval == 0) {
        alock_update_trace_info(ALOCK_TRYLOCK, p_alock, alock_name, file, func, line);
        audio_get_timespec_monotonic(&p_alock->trace_info.ts_start);
    }
#endif

    ALOCK_LOG("%p: trylock(%s), %s, %s(), %uL, retval: %d",
              p_alock, alock_name, file, func, line, retval);

    return retval;
}


int alock_lock_ms(
    struct alock_t *p_alock, const char *alock_name, const uint32_t ms,
    const char *file, const char *func, const uint32_t line) {
    struct timespec ts_timeout;

    struct timespec ts_start;
    struct timespec ts_stop;

    uint32_t ms_spend = 0;
    uint32_t ms_left  = ms;

    int try_count = 0;

    int retval = 0;

    if (p_alock == NULL) {
        AUD_LOG_E("%p: lock(%s, %u), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, ms, file, func, line);
        return -1;
    }

    if (ms == 0) {
        AUD_LOG_W("%p: lock(%s, %u), %s, %s(), %uL call alock_lock_no_timeout() due to ms = 0!!",
                  p_alock, alock_name, ms, file, func, line);
        return alock_lock_no_timeout(p_alock, alock_name, file, func, line);
    }

    ALOCK_LOG("%p: lock(%s, %u), %s, %s(), %uL",
              p_alock, alock_name, ms, file, func, line);

    do {
        audio_get_timespec_monotonic(&ts_start);
        audio_get_timespec_timeout(&ts_timeout, ms_left);
        retval = -pthread_mutex_timedlock(&p_alock->mutex, &ts_timeout);
        audio_get_timespec_monotonic(&ts_stop);

        /* pass or other error which is not timeout */
        if (retval == 0 || retval != -ETIMEDOUT) {
            break;
        }

        /* -ETIMEDOUT */
        ms_spend += get_time_diff_ms(&ts_start, &ts_stop);
        if (ms_spend >= ms) { /* monotonic also timeout */
            break;
        }
        ms_left = ms - ms_spend;

        /* AlarmManagerService: Setting time of day to sec=xxx */
        /* SimStateMonitor: onReceive action : android.intent.action.ACTION_SUBINFO_RECORD_UPDATED */
        AUD_LOG_W("%p: lock(%s, %u), %s, %s(), %uL, systime changed, ms_left: %u",
                  p_alock, alock_name, ms, file, func, line, ms_left);
    } while (++try_count < MAX_SYS_TIME_TRY_COUNT);

    if (retval == 0) {
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
        alock_update_trace_info(ALOCK_LOCK, p_alock, alock_name, file, func, line);
        audio_get_timespec_monotonic(&p_alock->trace_info.ts_start);
#endif
    } else {
        AUD_LOG_E("%p: lock(%s, %u), %s, %s(), %uL FAIL!! retval: %d",
                  p_alock, alock_name, ms, file, func, line, retval);
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
        alock_dump_trace_info(p_alock);
#endif
    }

    return retval;
}


int alock_unlock(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    if (p_alock == NULL) {
        AUD_LOG_E("%p: unlock(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: unlock(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    audio_get_timespec_monotonic(&p_alock->trace_info.ts_stop);
    alock_update_trace_info(ALOCK_UNLOCK, p_alock, alock_name, file, func, line);

    if (p_alock->trace_info.timeout == true) {
        p_alock->trace_info.timeout = false;
        AUD_LOG_W("%p: unlock(%s), %s, %s(), %uL, lock time %u ms",
                  p_alock, alock_name, file, func, line,
                  (uint32_t)get_time_diff_ms(&p_alock->trace_info.ts_start,
                                             &p_alock->trace_info.ts_stop));
    }
#endif

    pthread_mutex_unlock(&p_alock->mutex);

    return 0;
}



int alock_wait_no_timeout(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    if (p_alock == NULL) {
        AUD_LOG_E("%p: wait(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: +wait(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_IN, p_alock, alock_name, file, func, line);
#endif

    pthread_cond_wait(&p_alock->cond, &p_alock->mutex);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_OUT, p_alock, alock_name, file, func, line);
#endif

    ALOCK_LOG("%p: -wait(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

    return 0;
}


int alock_wait_ms(
    struct alock_t *p_alock, const char *alock_name, const uint32_t ms,
    const char *file, const char *func, const uint32_t line) {
    struct timespec ts_timeout;

    struct timespec ts_start;
    struct timespec ts_stop;

    uint32_t ms_spend = 0;
    uint32_t ms_left  = ms;

    int try_count = 0;

    int retval = 0;

    if (p_alock == NULL) {
        AUD_LOG_E("%p: wait(%s, %u), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, ms, file, func, line);
        return -1;
    }

    if (ms == 0) {
        AUD_LOG_W("%p: wait(%s, %u), %s, %s(), %uL not wait due to ms = 0!!",
                  p_alock, alock_name, ms, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: +wait(%s, %u), %s, %s(), %uL",
              p_alock, alock_name, ms, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_IN, p_alock, alock_name, file, func, line);
#endif

    do {
        audio_get_timespec_monotonic(&ts_start);
        audio_get_timespec_timeout(&ts_timeout, ms_left);
        retval = -pthread_cond_timedwait(&p_alock->cond, &p_alock->mutex, &ts_timeout);
        audio_get_timespec_monotonic(&ts_stop);

        /* pass or other error which is not timeout */
        if (retval == 0 || retval != -ETIMEDOUT) {
            break;
        }

        /* -ETIMEDOUT */
        ms_spend += get_time_diff_ms(&ts_start, &ts_stop);
        if (ms_spend >= ms) { /* monotonic also timeout */
            break;
        }
        ms_left = ms - ms_spend;

        /* AlarmManagerService: Setting time of day to sec=xxx */
        /* SimStateMonitor: onReceive action : android.intent.action.ACTION_SUBINFO_RECORD_UPDATED */
        AUD_LOG_W("%p: wait(%s, %u), %s, %s(), %uL, systime changed, ms_left: %u",
                  p_alock, alock_name, ms, file, func, line, ms_left);
    } while (++try_count < MAX_SYS_TIME_TRY_COUNT);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_OUT, p_alock, alock_name, file, func, line);
#endif

    if (retval == 0) {
        ALOCK_LOG("%p: -wait(%s, %u), %s, %s(), %uL",
                  p_alock, alock_name, ms, file, func, line);
    } else if (retval == -ETIMEDOUT) {
        if (ms_spend > ms + MAX_WAIT_BLOCKED_BY_LOCK_MS) {
            AUD_LOG_W("%p: -wait(%s, %u), %s, %s(), %uL FAIL!! retval: %d, ms_spend: %u",
                      p_alock, alock_name, ms, file, func, line, retval, ms_spend);
        }
    } else {
        AUD_LOG_E("%p: -wait(%s, %u), %s, %s(), %uL FAIL!! retval: %d",
                  p_alock, alock_name, ms, file, func, line, retval);
    }

    return retval;
}


int alock_signal(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line) {
    if (p_alock == NULL) {
        AUD_LOG_E("%p: signal(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: signal(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

    pthread_cond_signal(&p_alock->cond);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_SIGNAL, p_alock, alock_name, file, func, line);
#endif

    return 0;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

