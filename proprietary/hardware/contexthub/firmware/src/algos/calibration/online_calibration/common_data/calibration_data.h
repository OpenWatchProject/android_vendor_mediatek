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

/*
 * This module provides the component definitions used to represent sensor
 * calibration data, labeled flags/enumerators, and the callback functionality
 * employed by the online sensor calibration algorithms.
 */

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_DATA_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_DATA_H_

#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "calibration/online_calibration/common_data/calibration_quality.h"
#include "calibration/online_calibration/common_data/sensor_data.h"
#include "calibration/over_temp/over_temp_model.h"

namespace online_calibration {

/*
 * Bitmask used to indicate which calibration values have changed for a given
 * calibration update.
 *
 * [Bit Flag]   | [Affected Sensor Calibration Value]
 * BIAS             - Quasi-static non-zero sensor output (offset), given
 *                    conditions where the sensor should ideally output zero.
 *                    Includes corrections for over-temperature compensation.
 * SCALE_FACTOR     - Sensor axis scaling (ideally unity).
 * CROSS_AXIS       - Output sensitivity due to variations of perpendicular
 *                    sensing axes (ideally zero).
 * OVER_TEMP        - Model parameters that capture variations in sensor
 *                    behavior with temperature (e.g., linear bias sensitivity
 *                    model).
 * QUALITY_DEGRADED - Indicates a degradation in calibration quality.
 */
enum class CalibrationTypeFlags : uint8_t {
  NONE = 0x00,
  BIAS = 0x01,
  SCALE_FACTOR = 0x02,
  CROSS_AXIS = 0x04,
  OVER_TEMP = 0x08,
  QUALITY_DEGRADED = 0x10,
  ALL = 0xFF
};

// Logic operators to assist with common bitmask setting/checking.
CalibrationTypeFlags operator|(CalibrationTypeFlags lhs,
                               CalibrationTypeFlags rhs);

bool operator&(CalibrationTypeFlags lhs, CalibrationTypeFlags rhs);

CalibrationTypeFlags& operator|=(CalibrationTypeFlags& lhs,
                                 CalibrationTypeFlags rhs);

/*
 * Defines the calibration data specific to a prototypical three-axis sensing
 * device (e.g., accelerometer, gyroscope, magnetometer).
 *
 * Calibration correction may be applied as:
 *   corrected_data = scale_skew_matrix * (input_data - offset);
 *
 * 'offset' is the sensor bias estimate (with temperature compensation applied
 * when supported by the underlying calibration algorithm).
 *
 * The 'scale_skew_matrix' is assumed to be in lower diagonal form where the
 * sensor frame reference definition is such that cross-axis sensitivities
 * cross_axis_xy, cross_axis_xz, and cross_axis_yz are set to zero.
 *
 *   scale_skew_matrix = [scale_factor_x    0                 0
 *                        cross_axis_yx     scale_factor_y    0
 *                        cross_axis_zx     cross_axis_zy     scale_factor_z]
 *
 * NOTE1: If over-temperature compensation is provided, then temperature
 * compensation is already applied to the 'offset'. Coefficients representative
 * of the sensor's temperature dependency model are provided, and may be used
 * for model initialization after a system restart.
 *
 *   temp_sensitivity - Modeled temperature sensitivity (i.e., linear slope).
 *   temp_intercept   - Linear model intercept.
 *
 * The model equation for the over-temperature compensated offset:
 *   compensated_offset = temp_sensitivity * current_temp + temp_intercept
 *
 * NOTE2: Unless otherwise stated, 3-dimensional array indices: 0=x, 1=y, 2=z.
 */
struct CalibrationDataThreeAxis {
  // Timestamp for the most recent calibration update.
  uint64_t cal_update_time_nanos = 0;

  // The sensor's offset (i.e., bias) in the x-, y-, z-axes at
  // 'offset_temp_celsius'.
  float offset[3];

  // The temperature associated with the sensor offset.
  float offset_temp_celsius;

  // The temperature sensitivity of the offset.
  float temp_sensitivity[3];  // [sensor_units/Celsius]

  // The sensor's offset at zero degrees Celsius.
  float temp_intercept[3];  // [sensor_units]

  // The sensor's scale factor for each axis.
  float scale_factor[3];

  // The cross-axis factors in order: [0=yx, 1=zx, 2=zy].
  float cross_axis[3];

  // Metrics used for the reported calibration accuracy. The behavior and
  // definition depends on the sensor calibration type. See the calibration
  // algorithm documentation for details.
  CalibrationQuality calibration_quality;

  // Indicates the type of sensing device being calibrated.
  SensorType type = SensorType::kUndefined;

  // Optional pointer to an array of over-temperature model data (null when not
  // used). For initialization, populating a model dataset will take precedence
  // over the linear model parameters provided in the calibration data.
  OverTempModelThreeAxis* otc_model_data = nullptr;
  int16_t num_model_pts = 0;

  // Helper function that resets the calibration data to a set of neutral
  // reference values where no calibration correction would be applied if used.
  void reset() {
    otc_model_data = nullptr;
    calibration_quality.reset();
    offset_temp_celsius = 0.0f;
    scale_factor[0] = 1.0f;
    scale_factor[1] = 1.0f;
    scale_factor[2] = 1.0f;
    memset(offset, 0, sizeof(offset));
    memset(temp_sensitivity, 0, sizeof(temp_sensitivity));
    memset(temp_intercept, 0, sizeof(temp_intercept));
    memset(cross_axis, 0, sizeof(cross_axis));
  }

  CalibrationDataThreeAxis() { reset(); }
};

/*
 * Defines the calibration data for single dimensional sensing device (e.g.,
 * thermometer, barometer).
 *
 * Calibration correction may be applied as:
 *   corrected_data = scale_factor * (input_data - offset);
 *
 * 'offset' is the sensor bias estimate (with temperature compensation applied,
 * if supported by the underlying calibration algorithm).
 *
 * NOTE: If over-temperature compensation is provided, then temperature
 * compensation is already applied to the 'offset'. Coefficients representative
 * of the sensor's temperature dependency model are provided, and may be used
 * for model initialization after a system restart.
 *
 *   temp_sensitivity - Modeled temperature sensitivity (i.e., linear slope).
 *   temp_intercept   - Linear model intercept.
 *
 * The model equation for the over-temperature compensated offset:
 *   compensated_offset = temp_sensitivity * current_temp + temp_intercept
 */
struct CalibrationDataSingleAxis {
  // Timestamp for the most recent calibration update.
  uint64_t cal_update_time_nanos = 0;

  // The sensor's offset (i.e., bias) at temperature, 'offset_temp_celsius'.
  float offset;

  // The temperature associated with the sensor offset.
  float offset_temp_celsius;

  // The temperature sensitivity of the offset.
  float temp_sensitivity;  // [sensor_units/Celsius]

  // The sensor's offset at zero degrees Celsius.
  float temp_intercept;  // [sensor_units]

  // The sensor's scale factor.
  float scale_factor;

  // Metrics used for the reported calibration accuracy. The behavior and
  // definition depends on the sensor calibration type. See the calibration
  // algorithm documentation for details.
  CalibrationQuality calibration_quality;

  // Indicates the type of sensing device being calibrated.
  SensorType type = SensorType::kUndefined;

  // Optional pointer to an array of over-temperature model data (null when not
  // used). For initialization, populating a model dataset will take precedence
  // over the linear model parameters provided in the calibration data.
  OverTempModelSingleAxis* otc_model_data = nullptr;
  int16_t num_model_pts = 0;

  // Helper function that resets the calibration data to a set of neutral
  // reference values where no calibration correction would be applied if used.
  void reset() {
    otc_model_data = nullptr;
    calibration_quality.reset();
    scale_factor = 1.0f;
    offset_temp_celsius = 0.0f;
    offset = 0.0f;
    temp_sensitivity = 0.0f;
    temp_intercept = 0.0f;
  }

  CalibrationDataSingleAxis() { reset(); }
};

}  // namespace online_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_DATA_H_
