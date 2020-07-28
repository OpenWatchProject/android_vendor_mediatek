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
 * This module provides quality definitions that represent the accuracy
 * associated with calibration data. This information may be used to affect how
 * a system uses available sensor calibration data.
 */

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_QUALITY_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_QUALITY_H_

#include <float.h>
#include <stdint.h>

namespace online_calibration {

/*
 * In general, calibration quality values may be thought of in terms of
 * something akin to an error standard deviation. That is, it's a non-negative
 * value where lower values imply higher calibration quality, and larger values
 * indicate poorer quality. However, the units and numerical interpretation is
 * ultimately dictated by the sensor type and calibration algorithm. Consult the
 * calibration code implementation for the actual calibration quality metric
 * details.
 */

/*
 * Bitmask used to provide a qualitative ranking of the calibration data's
 * accuracy. Many systems use this sort of interpretation for calibration
 * quality (e.g., Android).
 *
 * [Bit Flag]   | [Qualitative Calibration Quality]
 * UNDETERMINED    - Calibration quality has not yet been determined (e.g., a
 *                   calibration hasn't occurred, or a metric hasn't been
 *                   established).
 * LOW_QUALITY     - Sensor calibration is needed. System properties have
 *                   changed that may have affected the applicability of the
 *                   current calibration (e.g., calibration data is old, anomaly
 *                   detected, etc.).
 * MEDIUM_QUALITY  - The reported sensor calibration has an average level of
 *                   accuracy, updated calibration may improve the readings.
 * HIGH_QUALITY    - The reported calibration has maximal accuracy.
 */
enum class CalibrationQualityLevel : uint8_t {
  UNDETERMINED = 0x00,
  LOW_QUALITY = 0x01,
  MEDIUM_QUALITY = 0x02,
  HIGH_QUALITY = 0x04,
};

// Sets the calibration quality value when this metric is either not
// implemented, or has not yet been determined (e.g., a calibration hasn't
// occurred).
constexpr float kUndeterminedCalibrationQuality = -1.0f;

/*
 * Calibration quality structure that contains a quantitative (float) and
 * qualitative (enum) measure of a sensor's calibration accuracy. Both entries
 * should be co-defined by the algorithm providing the calibration update. The
 * enum sets the qualitative interpretation of the float value, this is often
 * used in systems (Android, etc.) to label quality and affect the use of the
 * calibration data.
 */
struct CalibrationQuality {
  // Provides a qualitative measure for sensor calibration accuracy.
  CalibrationQualityLevel level = CalibrationQualityLevel::UNDETERMINED;

  // Quantitative metric for the reported calibration accuracy. The behavior and
  // definition depends on the sensor calibration type. See the calibration
  // algorithm documentation for details.
  float value = kUndeterminedCalibrationQuality;

  // Helper function that resets the calibration quality to an initial state.
  void reset() {
    level = CalibrationQualityLevel::UNDETERMINED;
    value = kUndeterminedCalibrationQuality;
  }
};

}  // namespace online_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_QUALITY_H_
