/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_NANO_CALIBRATION_AOSP_NANO_CAL_PARAMETERS_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_NANO_CALIBRATION_AOSP_NANO_CAL_PARAMETERS_H_

#ifdef ACCEL_CAL_ENABLED
#include "calibration/accelerometer/accel_cal.h"
#endif  // ACCEL_CAL_ENABLED

#ifdef GYRO_CAL_ENABLED
#include "calibration/gyroscope/gyro_cal.h"
#include "calibration/over_temp/over_temp_cal.h"
#endif  // GYRO_CAL_ENABLED

#ifdef MAG_CAL_ENABLED
#include "calibration/diversity_checker/diversity_checker.h"
#include "calibration/magnetometer/mag_cal/mag_cal.h"
#endif  // MAG_CAL_ENABLED

#include "common/math/macros.h"

namespace nano_calibration {

//////// ACCELEROMETER CALIBRATION ////////
#ifdef ACCEL_CAL_ENABLED
constexpr AccelCalParameters kAccelCalParameters{
    MSEC_TO_NANOS(800),  // t0
    5,                   // n_s
    15,                  // fx
    15,                  // fxb
    15,                  // fy
    15,                  // fyb
    15,                  // fz
    15,                  // fzb
    15,                  // fle
    0.00025f             // th
};
#endif  // ACCEL_CAL_ENABLED

//////// GYROSCOPE CALIBRATION ////////
#ifdef GYRO_CAL_ENABLED
constexpr GyroCalParameters kGyroCalParameters{
    SEC_TO_NANOS(1.4),    // min_still_duration_nanos
    SEC_TO_NANOS(1.4),    // max_still_duration_nanos [see, NOTE 1]
    0,                    // calibration_time_nanos
    MSEC_TO_NANOS(500),   // window_time_duration_nanos
    0,                    // bias_x
    0,                    // bias_y
    0,                    // bias_z
    0.95f,                // stillness_threshold
    MDEG_TO_RAD * 60.0f,  // stillness_mean_delta_limit [rad/sec]
    3.0e-5f,              // gyro_var_threshold [rad/sec]^2
    3.0e-6f,              // gyro_confidence_delta [rad/sec]^2
    4.5e-3f,              // accel_var_threshold [m/sec^2]^2
    9.0e-4f,              // accel_confidence_delta [m/sec^2]^2
    5.0f,                 // mag_var_threshold [uTesla]^2
    1.0f,                 // mag_confidence_delta [uTesla]^2
    1.5f,                 // temperature_delta_limit_celsius
    true                  // gyro_calibration_enable
};
// [NOTE 1]: 'max_still_duration_nanos' is set to 1.4 seconds to achieve a max
// stillness period of 1.5 seconds and avoid buffer boundary conditions that
// could push the max stillness to the next multiple of the analysis window
// length (i.e., 2.0 seconds).

constexpr OverTempCalParameters kGyroOtcParameters{
    MSEC_TO_NANOS(100),    // min_temp_update_period_nanos
    DAYS_TO_NANOS(2),      // age_limit_nanos
    0.75f,                 // delta_temp_per_bin
    40.0f * MDEG_TO_RAD,   // jump_tolerance
    100.0f * MDEG_TO_RAD,  // outlier_limit
    250.0f * MDEG_TO_RAD,  // temp_sensitivity_limit
    8.0e3f * MDEG_TO_RAD,  // sensor_intercept_limit
    0.1f * MDEG_TO_RAD,    // significant_offset_change
    5,                     // min_num_model_pts
    true                   // over_temp_enable
};
#endif  // GYRO_CAL_ENABLED

//////// MAGNETOMETER CALIBRATION ////////
#ifdef MAG_CAL_ENABLED
constexpr MagCalParameters kMagCalParameters{
    3000000,  // min_batch_window_in_micros
    0.0f,     // x_bias
    0.0f,     // y_bias
    0.0f,     // z_bias
    1.0f,     // c00
    0.0f,     // c01
    0.0f,     // c02
    0.0f,     // c10
    1.0f,     // c11
    0.0f,     // c12
    0.0f,     // c20
    0.0f,     // c21
    1.0f      // c22
};

constexpr DiversityCheckerParameters kMagDiversityParameters{
    6.0f,    // var_threshold
    10.0f,   // max_min_threshold
    48.0f,   // local_field
    0.5f,    // threshold_tuning_param
    2.552f,  // max_distance_tuning_param
    8,       // min_num_diverse_vectors
    1        // max_num_max_distance
};
#endif  // MAG_CAL_ENABLED

}  // namespace nano_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_NANO_CALIBRATION_AOSP_NANO_CAL_PARAMETERS_H_
