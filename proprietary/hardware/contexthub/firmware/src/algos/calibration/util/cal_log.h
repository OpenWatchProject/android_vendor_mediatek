/*
 * Copyright (C) 2016 The Android Open Source Project
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

///////////////////////////////////////////////////////////////
/*
 * Logging macros for printing user-debug messages.
 */
///////////////////////////////////////////////////////////////
#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_UTIL_CAL_LOG_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_UTIL_CAL_LOG_H_

#include <seos.h>
#ifndef LOG_FUNC
#define LOG_FUNC(level, fmt, ...) osLog(level, fmt, ##__VA_ARGS__)
#endif  // LOG_FUNC
#define LOGD_TAG(tag, fmt, ...) \
   LOG_FUNC(LOG_DEBUG, "%s " fmt "\n", tag, ##__VA_ARGS__)
#define CAL_DEBUG_LOG(tag, fmt, ...) \
   osLog(LOG_INFO, "%s " fmt "\n", tag, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// Floor macro implementation for platforms that do not supply the standard
// floorf() math function.
#define CAL_FLOOR(x) ((int)(x) - ((x) < (int)(x)))  // NOLINT

/*
 * On some embedded software platforms numerical string formatting is not fully
 * supported. Defining CAL_NO_FLOAT_FORMAT_STRINGS will enable a workaround that
 * prints floating-point values having a specified number of digits using the
 * CAL_ENCODE_FLOAT macro.
 *   Examples:
 *     - Nanohub does not support floating-point format strings.
 *     - CHRE/SLPI allows %.Xf for printing float values.
 */
#ifdef CAL_NO_FLOAT_FORMAT_STRINGS
// Macro used to print floating point numbers with a specified number of digits.
# define CAL_ENCODE_FLOAT(x, num_digits)           \
  ((x < 0) ? "-" : ""), (int)CAL_FLOOR(fabsf(x)),  \
      (int)((fabsf(x) - CAL_FLOOR(fabsf(x))) *     \
            powf(10, num_digits))  // NOLINT

// Helper definitions for CAL_ENCODE_FLOAT to specify the print format with
// desired significant digits.
# define CAL_FORMAT_3DIGITS "%s%d.%03d"
# define CAL_FORMAT_6DIGITS "%s%d.%06d"
# define CAL_FORMAT_3DIGITS_TRIPLET "%s%d.%03d, %s%d.%03d, %s%d.%03d"
# define CAL_FORMAT_6DIGITS_TRIPLET "%s%d.%06d, %s%d.%06d, %s%d.%06d"
#else
// Pass-through when float string formatting (e.g., %.6f) is available.
# define CAL_ENCODE_FLOAT(x, num_digits) (double)(x)

// Float string formatting helpers.
# define CAL_FORMAT_3DIGITS "%.3f"
# define CAL_FORMAT_6DIGITS "%.6f"
# define CAL_FORMAT_3DIGITS_TRIPLET "%.3f, %.3f, %.3f"
# define CAL_FORMAT_6DIGITS_TRIPLET "%.6f, %.6f, %.6f"
#endif  // CAL_NO_FLOAT_FORMAT_STRINGS

#ifdef __cplusplus
}
#endif

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_UTIL_CAL_LOG_H_
