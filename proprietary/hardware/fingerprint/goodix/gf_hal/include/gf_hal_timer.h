/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_HAL_TIMER_H__
#define __GF_HAL_TIMER_H__

#include <signal.h>
#include <sys/time.h>

#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*timer_thread_t)(union sigval v);

gf_error_t gf_hal_create_timer(timer_t* timer_id, timer_thread_t timer_thread);
gf_error_t gf_hal_set_timer(timer_t* timer_id,
            time_t interval_second, time_t value_second, long value_nanosecond);  // NOLINT
gf_error_t gf_hal_destroy_timer(timer_t* timer_id);

#ifdef __cplusplus
}
#endif

#endif  // __GF_HAL_TIMER_H__
