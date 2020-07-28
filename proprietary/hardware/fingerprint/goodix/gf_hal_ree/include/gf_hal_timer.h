#ifndef __GF_TIMER_H__
#define __GF_TIMER_H__

#include <signal.h>
#include <sys/time.h>

#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*timer_thread_t)(union sigval v);

gf_error_t gf_hal_create_timer(timer_t* timer_id, timer_thread_t timer_thread);
gf_error_t gf_hal_set_timer(timer_t* timer_id,
            time_t interval_second, time_t value_second, long value_nanosecond);
gf_error_t gf_hal_destroy_timer(timer_t* timer_id);

void hal_detected_key_down(void);
void hal_detected_key_up(void);

#ifdef __cplusplus
}
#endif

#endif //__GF_TIMER_H__
