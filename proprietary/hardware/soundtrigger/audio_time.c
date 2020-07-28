#include "audio_time.h"

#include <time.h>

#include <audio_log.h>


#ifdef __cplusplus
extern "C" {
#endif


void audio_get_timespec_realtime(struct timespec *ts) {
    clock_gettime(CLOCK_REALTIME, ts);
}


void audio_get_timespec_monotonic(struct timespec *ts) {
    clock_gettime(CLOCK_MONOTONIC, ts);
}


void audio_get_timespec_timeout(struct timespec *ts, uint32_t ms) {
    audio_get_timespec_realtime(ts);

    ts->tv_sec += ms / 1000;
    ts->tv_nsec += (ms % 1000) * 1000000;
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec  += 1;
    }
}


uint64_t audio_timespec_to_ns(struct timespec *ts) {
    return ((uint64_t)ts->tv_sec * 1000000000L) + ts->tv_nsec;
}


uint64_t audio_timespec_to_ms(struct timespec *ts) {
    return audio_timespec_to_ns(ts) / 1000000L;
}


bool t1_older_then_t2(struct timespec *t1, struct timespec *t2) {
    return (audio_timespec_to_ns(t1) < audio_timespec_to_ns(t2));
}


uint64_t get_time_diff_ns(struct timespec *t_old, struct timespec *t_new) {
    uint64_t t_old_ns = audio_timespec_to_ns(t_old);
    uint64_t t_new_ns = audio_timespec_to_ns(t_new);

    return ((t_new_ns > t_old_ns)
            ? (t_new_ns - t_old_ns)
            : ((UINTMAX_MAX - t_old_ns) + t_new_ns + 1));
}


uint64_t get_time_diff_ms(struct timespec *t_old, struct timespec *t_new) {
    uint64_t t_old_ms = audio_timespec_to_ms(t_old);
    uint64_t t_new_ms = audio_timespec_to_ms(t_new);

    return ((t_new_ms > t_old_ms)
            ? (t_new_ms - t_old_ms)
            : ((UINTMAX_MAX - t_old_ms) + t_new_ms + 1));
}


#ifdef __cplusplus
}  /* extern "C" */
#endif

