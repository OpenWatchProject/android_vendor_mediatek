#ifndef AUDIO_TIME_H
#define AUDIO_TIME_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct timespec;

void audio_get_timespec_realtime(struct timespec *ts);

void audio_get_timespec_monotonic(struct timespec *ts);

void audio_get_timespec_timeout(struct timespec *ts, uint32_t ms);

uint64_t audio_timespec_to_ns(struct timespec *ts);

uint64_t audio_timespec_to_ms(struct timespec *ts);

bool t1_older_then_t2(struct timespec *t1, struct timespec *t2);

uint64_t get_time_diff_ns(struct timespec *t_old, struct timespec *t_new);

uint64_t get_time_diff_ms(struct timespec *t_old, struct timespec *t_new);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of AUDIO_TIME_H */

