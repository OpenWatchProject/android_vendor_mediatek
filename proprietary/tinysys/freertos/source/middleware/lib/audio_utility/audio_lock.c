#include <audio_lock.h>

#include <errno.h>

#include <mt_gpt.h>

#include <audio_log_hal.h>

#include <audio_memory_control.h>



#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     LOG
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[ALOCK]"


#ifdef MTK_AUDIO_LOCK_ENABLE_LOG
#ifdef ALOCK_LOG
#undef ALOCK_LOG
#endif
#define ALOCK_LOG(x...) AUD_LOG_D(x)
#else
#define ALOCK_LOG(x...)
#endif


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define LOCK_TIMEOUT_WARN_NS (500000000) /* 500 ms */

#define NS_TO_SEC(time_ns) ((uint32_t)(time_ns / 1000000000))
#define NS_TO_MS(time_ns)  ((uint16_t)((time_ns % 1000000000) / 1000000))


/*
 * =============================================================================
 *                     dump history
 * =============================================================================
 */

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE

enum {
    ALOCK_LOCK,
    ALOCK_TRYLOCK,
    ALOCK_UNLOCK,
    ALOCK_WAIT_IN,
    ALOCK_WAIT_OUT,
    ALOCK_SIGNAL,
    NUM_ALOCK_TYPE
};


static const char *alock_type_name[NUM_ALOCK_TYPE] = {
    "lock",
    "trylock",
    "unlock",
    "+wait",
    "-wait",
    "signal"
};

static void alock_update_trace_info(
    const uint8_t type, struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    struct alock_log_unit_t *log_unit = NULL;
    uint8_t idx = 0;
    uint64_t time_ns = timer_get_ap_timestamp();

    if (type >= NUM_ALOCK_TYPE) {
        AUD_LOG_E("%s(), %p: %s, %s, %s(), %uL, type %d FAIL!!",
                  __FUNCTION__, p_alock, alock_name, file, func, line, type);
        return;
    }

    xSemaphoreTake(p_alock->trace_info.idx_lock, portMAX_DELAY);
    idx = p_alock->trace_info.idx;
    p_alock->trace_info.idx++;
    if (p_alock->trace_info.idx == ALOCK_TRACE_NUM) {
        p_alock->trace_info.idx = 0;
    }

    log_unit = &p_alock->trace_info.log[idx];
    xSemaphoreGive(p_alock->trace_info.idx_lock);

    log_unit->type = alock_type_name[type];
    log_unit->name = alock_name;
    log_unit->file = file;
    log_unit->func = func;
    log_unit->line = (uint16_t)(line & 0xFFFF);
    log_unit->time_ms = NS_TO_MS(time_ns);
    log_unit->time_sec = NS_TO_SEC(time_ns);
}


static void alock_dump_trace_info(struct alock_t *p_alock)
{
    struct alock_log_unit_t *log_unit = NULL;
    uint8_t idx = 0;

    p_alock->trace_info.timeout = true;

    AUD_LOG_E("%p: ========================= dump(+) =========================", p_alock);

    xSemaphoreTake(p_alock->trace_info.idx_lock, portMAX_DELAY);

    idx = p_alock->trace_info.idx + 1;
    if (idx == ALOCK_TRACE_NUM) {
        idx = 0;
    }

    while (idx != p_alock->trace_info.idx) {
        log_unit = &p_alock->trace_info.log[idx];
        if (log_unit->type != NULL &&
            log_unit->name != NULL &&
            log_unit->file != NULL &&
            log_unit->func != NULL) {

            AUD_LOG_E("%p: time [%u.%03d], %s(%s), %s, %s(), %uL",
                      p_alock,
                      log_unit->time_sec,
                      log_unit->time_ms,
                      log_unit->type,
                      log_unit->name,
                      log_unit->file,
                      log_unit->func,
                      log_unit->line);
        }

        idx++;
        if (idx == ALOCK_TRACE_NUM) {
            idx = 0;
        }
    }

    xSemaphoreGive(p_alock->trace_info.idx_lock);

    AUD_LOG_E("%p: ========================= dump(-) =========================", p_alock);
}
#endif



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

const char *get_filename(const char *file)
{
    const char *slash = strrchr(file, '/');
    return (slash) ? slash + 1 : file;
}


int alock_new(
    struct alock_t **pp_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    struct alock_t *p_alock = (struct alock_t *)AUDIO_MALLOC(sizeof(struct alock_t));

    if (p_alock == NULL) {
        AUD_LOG_E("%p: new(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        *pp_alock = NULL;
        return -1;
    }
    *pp_alock = p_alock;

    p_alock->mutex = xSemaphoreCreateMutex();
    p_alock->cond  = xQueueCreate(1, sizeof(uint8_t));

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    memset(&p_alock->trace_info, 0, sizeof(struct alock_trace_info_t));
    p_alock->trace_info.idx_lock = xSemaphoreCreateMutex();
#endif

    ALOCK_LOG("%p: new(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);
    return 0;
}


int alock_free(
    struct alock_t **pp_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    struct alock_t *p_alock = (struct alock_t *)*pp_alock;

    if (p_alock == NULL) {
        AUD_LOG_E("%p: free(%s), %s, %s(), %uL FAIL!!",
                  p_alock, alock_name, file, func, line);
        return -1;
    }

    ALOCK_LOG("%p: free(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    vSemaphoreDelete(p_alock->trace_info.idx_lock);
#endif

    vSemaphoreDelete(p_alock->mutex);
    vQueueDelete(p_alock->cond);

    AUDIO_FREE(p_alock);
    p_alock = NULL;
    *pp_alock = NULL;

    return 0;
}


int alock_lock_no_timeout(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: lock(%s), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, file, func, line, is_in_isr());
        return -1;
    }

    ALOCK_LOG("%p: lock(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#if 0
    xSemaphoreTake(p_alock->mutex, portMAX_DELAY);
#else
    while (xSemaphoreTake(p_alock->mutex, pdMS_TO_TICKS(LOCK_TIMEOUT_WARN_NS)) == pdFALSE) {
        AUD_LOG_W("%p: lock(%s), %s, %s(), %uL, lock too long... try again",
                  p_alock, alock_name, file, func, line);
    }
#endif

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_LOCK, p_alock, alock_name, file, func, line);
    p_alock->trace_info.lock_time_start = read_xgpt_stamp_ns();
#endif

    return 0;
}


int alock_trylock(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    int retval = 0;

    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: trylock(%s), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, file, func, line, is_in_isr());
        return -1;
    }

    retval = (xSemaphoreTake(p_alock->mutex, 0) == pdTRUE) ? 0 : -EBUSY ;

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    if (retval == 0) {
        alock_update_trace_info(ALOCK_TRYLOCK, p_alock, alock_name, file, func, line);
        p_alock->trace_info.lock_time_start = read_xgpt_stamp_ns();
    }
#endif

    ALOCK_LOG("%p: trylock(%s), %s, %s(), %uL, retval: %d",
              p_alock, alock_name, file, func, line, retval);

    return retval;
}


int alock_lock_ms(
    struct alock_t *p_alock, const char *alock_name, const uint32_t ms,
    const char *file, const char *func, const uint32_t line)
{
    int retval = 0;

    uint64_t time_start;

    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: lock(%s, %u), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, ms, file, func, line, is_in_isr());
        return -1;
    }

    if (ms == 0) {
        AUD_LOG_W("%p: lock(%s, %u), %s, %s(), %uL call alock_trylock() due to ms = 0!!",
                  p_alock, alock_name, ms, file, func, line);
        return alock_trylock(p_alock, alock_name, file, func, line);
    }

    ALOCK_LOG("%p: lock(%s, %u), %s, %s(), %uL",
              p_alock, alock_name, ms, file, func, line);

    time_start = timer_get_ap_timestamp();
    retval = (xSemaphoreTake(p_alock->mutex,
                             pdMS_TO_TICKS(ms)) == pdTRUE) ? 0 : -ETIMEDOUT ;

    if (retval == 0) {
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
        alock_update_trace_info(ALOCK_LOCK, p_alock, alock_name, file, func, line);
        p_alock->trace_info.lock_time_start = read_xgpt_stamp_ns();
#endif
    } else {
        AUD_LOG_E("%p: lock(%s, %u), %s, %s(), %uL FAIL!! retval: %d, try to lock when [%u.%u]",
                  p_alock, alock_name, ms, file, func, line, retval,
                  NS_TO_SEC(time_start), NS_TO_MS(time_start));
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
        alock_dump_trace_info(p_alock);
#endif
    }

    return retval;
}


int alock_unlock(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    uint64_t time_diff_ns = 0;
#endif

    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: unlock(%s), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, file, func, line, is_in_isr());
        return -1;
    }

    ALOCK_LOG("%p: unlock(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    p_alock->trace_info.lock_time_stop = read_xgpt_stamp_ns();
    alock_update_trace_info(ALOCK_UNLOCK, p_alock, alock_name, file, func, line);

    time_diff_ns = p_alock->trace_info.lock_time_stop - p_alock->trace_info.lock_time_start;

    if (p_alock->trace_info.timeout == true || time_diff_ns > LOCK_TIMEOUT_WARN_NS) {
        p_alock->trace_info.timeout = false;
        AUD_LOG_W("%p: unlock(%s), %s, %s(), %uL, lock time %u.%u sec",
                  p_alock, alock_name, file, func, line,
                  NS_TO_SEC(time_diff_ns), NS_TO_MS(time_diff_ns));
    }
#endif

    xSemaphoreGive(p_alock->mutex);

    return 0;
}


int alock_wait_no_timeout(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    uint8_t tmp = 0;

    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: wait(%s), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, file, func, line, is_in_isr());
        return -1;
    }

    ALOCK_LOG("%p: +wait(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_IN, p_alock, alock_name, file, func, line);
#endif

    xSemaphoreGive(p_alock->mutex);
    xQueueReceive(p_alock->cond, &tmp, portMAX_DELAY);
    while (xSemaphoreTake(p_alock->mutex, pdMS_TO_TICKS(LOCK_TIMEOUT_WARN_NS)) == pdFALSE) {
        AUD_LOG_W("%p: wait(%s), %s, %s(), %uL, re-lock too long... try again",
                  p_alock, alock_name, file, func, line);
    }

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_OUT, p_alock, alock_name, file, func, line);
#endif

    ALOCK_LOG("%p: -wait(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

    return 0;
}


int alock_wait_ms(
    struct alock_t *p_alock, const char *alock_name, const uint32_t ms,
    const char *file, const char *func, const uint32_t line)
{
    uint8_t tmp = 0;
    int retval = 0;

    uint64_t time_start;
    uint64_t time_wait;
    uint64_t time_stop;

    uint64_t time_diff_wait;
    uint64_t time_diff_lock;

    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: wait(%s, %u), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, ms, file, func, line, is_in_isr());
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

    xSemaphoreGive(p_alock->mutex);
    time_start = read_xgpt_stamp_ns();
    retval = (xQueueReceive(p_alock->cond,
                            &tmp,
                            pdMS_TO_TICKS(ms)) == pdTRUE) ? 0 : -ETIMEDOUT;
    time_wait = read_xgpt_stamp_ns();
    while (xSemaphoreTake(p_alock->mutex, pdMS_TO_TICKS(LOCK_TIMEOUT_WARN_NS)) == pdFALSE) {
        AUD_LOG_W("%p: wait(%s, %u), %s, %s(), %uL, re-lock too long... try again",
                  p_alock, alock_name, ms, file, func, line);
    }
    time_stop = read_xgpt_stamp_ns();

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_WAIT_OUT, p_alock, alock_name, file, func, line);
#endif

    if (retval == 0 && (time_stop - time_start) < (ms * 1000000)) {
        ALOCK_LOG("%p: -wait(%s, %u), %s, %s(), %uL",
                  p_alock, alock_name, ms, file, func, line);
    } else {
        time_diff_wait = time_wait - time_start;
        time_diff_lock = time_stop - time_wait;

        AUD_LOG_W("%p: -wait(%s, %u), %s, %s(), %uL FAIL!! retval: %d, wait: %u.%u sec, lock: %u.%u sec",
                  p_alock, alock_name, ms, file, func, line, retval,
                  NS_TO_SEC(time_diff_wait), NS_TO_MS(time_diff_wait),
                  NS_TO_SEC(time_diff_lock), NS_TO_MS(time_diff_lock));
    }

    return retval;
}


int alock_signal(
    struct alock_t *p_alock, const char *alock_name,
    const char *file, const char *func, const uint32_t line)
{
    uint8_t tmp = 0;

    if (p_alock == NULL || is_in_isr()) {
        AUD_LOG_E("%p: signal(%s), %s, %s(), %uL FAIL!! is_in_isr() = %d",
                  p_alock, alock_name, file, func, line, is_in_isr());
        return -1;
    }

    ALOCK_LOG("%p: signal(%s), %s, %s(), %uL",
              p_alock, alock_name, file, func, line);

    xQueueSendToBack(p_alock->cond, &tmp, portMAX_DELAY);

#ifdef MTK_AUDIO_LOCK_ENABLE_TRACE
    alock_update_trace_info(ALOCK_SIGNAL, p_alock, alock_name, file, func, line);
#endif

    return 0;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

