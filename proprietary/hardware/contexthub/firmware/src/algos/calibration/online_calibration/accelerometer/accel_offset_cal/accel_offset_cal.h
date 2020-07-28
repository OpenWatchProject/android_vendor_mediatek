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

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_ACCELEROMETER_ACCEL_OFFSET_CAL_ACCEL_OFFSET_CAL_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_ACCELEROMETER_ACCEL_OFFSET_CAL_ACCEL_OFFSET_CAL_H_

#include "calibration/accelerometer/accel_cal.h"
#include "calibration/online_calibration/common_data/calibration_callback.h"
#include "calibration/online_calibration/common_data/calibration_data.h"
#include "calibration/online_calibration/common_data/online_calibration.h"
#include "calibration/online_calibration/common_data/sensor_data.h"

namespace online_calibration {

/*
 * This class is a wrapper for accelerometer offset calibration.
 *
 * NOTE: Calibration quality reporting (quantitative metric is not used):
 *   Initialize             --> CalibrationQualityLevel::UNDETERMINED
 *                              CalibrationQuality.value =
 *                                kUndeterminedCalibrationQuality
 *   SetInitialCalibration  --> CalibrationQualityLevel::LOW_QUALITY
 *                              CalibrationQuality.value =
 *                                kUndeterminedCalibrationQuality
 *   New Calibration Update --> CalibrationQualityLevel::HIGH_QUALITY
 *                              CalibrationQuality.value =
 *                                kUndeterminedCalibrationQuality
 */
class AccelOffsetCal final
    : public OnlineCalibration<CalibrationDataThreeAxis> {
 public:
  // Default constructor.
  AccelOffsetCal() = default;

  // Creates an AccelOffsetCal with specified algorithm parameters.
  explicit AccelOffsetCal(const AccelCalParameters& accel_cal_parameters) {
    Initialize(accel_cal_parameters);
  }

  // Initializes with specified algorithm parameters, and resets the calibration
  // data.
  void Initialize(const AccelCalParameters& accel_cal_parameters);

  // Sends new sensor data to the calibration algorithm, and returns the state
  // of the calibration update flags, 'cal_update_polling_flags_'.
  CalibrationTypeFlags SetMeasurement(const SensorData& sample) final;

  // Sets the initial calibration data of the calibration algorithm. Returns
  // true if set successfully.
  bool SetInitialCalibration(
      const CalibrationDataThreeAxis& input_cal_data) final;

  // Returns the calibration sensor type.
  SensorType get_sensor_type() const final {
    return SensorType::kAccelerometerMps2;
  };

 private:
  // Accelerometer offset calibration algorithm data structure.
  AccelCal accel_cal_;
};

}  // namespace online_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_ACCELEROMETER_ACCEL_OFFSET_CAL_ACCEL_OFFSET_CAL_H_
