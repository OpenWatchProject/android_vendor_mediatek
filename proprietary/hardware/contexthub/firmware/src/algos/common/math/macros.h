/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This file contains frequently used constants and helper macros.

#include <stdint.h>

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_COMMON_MATH_MACROS_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_COMMON_MATH_MACROS_H_

// Constants.
#define NANO_PI                     (3.14159265359f)
#define INVALID_TEMPERATURE_CELSIUS (-274.0f)

// Common math operations.
#define NANO_ABS(x) ((x) > 0 ? (x) : -(x))
#define NANO_MAX(a, b) ((a) > (b)) ? (a) : (b)
#define NANO_MIN(a, b) ((a) < (b)) ? (a) : (b)
#define SIGMOID(x) (1 / (1 + expf(-x)))

// Timestamp conversion macros.
#ifdef __cplusplus
#define MSEC_TO_NANOS(x) (static_cast<uint64_t>(x * UINT64_C(1000000)))
#else
#define MSEC_TO_NANOS(x) ((uint64_t)(x * UINT64_C(1000000)))  // NOLINT
#endif

#define SEC_TO_NANOS(x)  MSEC_TO_NANOS(x * UINT64_C(1000))
#define MIN_TO_NANOS(x)  SEC_TO_NANOS (x * UINT64_C(60))
#define HRS_TO_NANOS(x)  MIN_TO_NANOS (x * UINT64_C(60))
#define DAYS_TO_NANOS(x) HRS_TO_NANOS (x * UINT64_C(24))

// Sample rate to period conversion.
#ifdef __cplusplus
#define HZ_TO_PERIOD_NANOS(hz) \
  (SEC_TO_NANOS(1024) / (static_cast<uint64_t>(hz * 1024)))
#else
#define HZ_TO_PERIOD_NANOS(hz) \
  (SEC_TO_NANOS(1024) / ((uint64_t)(hz * 1024)))  // NOLINT
#endif

// Unit conversion: nanoseconds to seconds.
#define NANOS_TO_SEC (1.0e-9f)

// Unit conversion: milli-degrees to radians.
#define MDEG_TO_RAD (NANO_PI / 180.0e3f)

// Unit conversion: radians to milli-degrees.
#define RAD_TO_MDEG (180.0e3f / NANO_PI)

// Time check helper macro that returns true if:
//    i.  't1' is equal to or exceeds 't2' plus 't_delta'.
//    ii. Or, a negative timestamp delta occurred since,
//        't1' should always >= 't2'. This prevents potential lockout conditions
//        if the timer count 't1' rolls over or an erroneously large
//        timestamp is passed through.
#define NANO_TIMER_CHECK_T1_GEQUAL_T2_PLUS_DELTA(t1, t2, t_delta) \
  (((t1) >= (t2) + (t_delta)) || ((t1) < (t2)))

#ifdef __cplusplus
extern "C" {
#endif

// This conversion function may be necessary for embedded hardware that can't
// cast a uint64_t to a float directly. This conversion function was taken from:
// [android]//device/google/contexthub/firmware/os/core/floatRt.c
static inline float floatFromUint64(uint64_t v) {
  uint32_t hi = v >> 32;
  uint32_t lo = (uint32_t) v;

  if (!hi) {  // This is very fast for cases where 'v' fits into a uint32_t.
    return (float)lo;
  } else {
    return ((float)hi) * 4294967296.0f + (float)lo;
  }
}

#ifdef __cplusplus
}
#endif

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_COMMON_MATH_MACROS_H_
