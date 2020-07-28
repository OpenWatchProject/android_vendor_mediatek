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

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_MAGNETOMETER_MAG_DIVERSE_CAL_MAG_DIVERSE_CAL_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_MAGNETOMETER_MAG_DIVERSE_CAL_MAG_DIVERSE_CAL_H_

#include "calibration/diversity_checker/diversity_checker.h"
#include "calibration/magnetometer/mag_cal/mag_cal.h"
#include "calibration/online_calibration/common_data/calibration_callback.h"
#include "calibration/online_calibration/common_data/calibration_data.h"
#include "calibration/online_calibration/common_data/online_calibration.h"
#include "calibration/online_calibration/common_data/sensor_data.h"

namespace online_calibration {

/*
 * This class is a wrapper for the magnetometer offset calibration with
 * diversity checking.
 *
 * NOTE: Calibration quality reporting:
 *   Initialize             --> CalibrationQualityLevel::UNDETERMINED
 *                              CalibrationQuality.value =
 *                                kUndeterminedCalibrationQuality
 *   SetInitialCalibration  --> CalibrationQualityLevel::UNDETERMINED
 *                              CalibrationQuality.value =
 *                                kUndeterminedCalibrationQuality
 *   New Calibration Update --> CalibrationQualityLevel::HIGH_QUALITY
 *                              CalibrationQuality.value = kHighQualityUt
 */
class MagDiverseCal final : public OnlineCalibration<CalibrationDataThreeAxis> {
 public:
  // Empirically estimated upper bounds on offset error.
  static constexpr float kLowQualityUt = 1000.0f;  // Units of micro Tesla
  static constexpr float kHighQualityUt = 5.0f;    // Units of micro Tesla

  MagDiverseCal() = default;

  // Creates an MagDiverseCal with specified algorithm parameters.
  MagDiverseCal(const MagCalParameters& mag_cal_parameters,
                const DiversityCheckerParameters& diversity_parameters) {
    Initialize(mag_cal_parameters, diversity_parameters);
  }

  // Initializes with specified algorithm parameters.
  void Initialize(const MagCalParameters& mag_cal_parameters,
                  const DiversityCheckerParameters& diversity_parameters);

  // Sends new sensor data to the calibration algorithm, and returns the state
  // of the calibration update flags, 'cal_update_polling_flags_'.
  CalibrationTypeFlags SetMeasurement(const SensorData& sample) final;

  // Sets the initial calibration data of the calibration algorithm. Returns
  // true if set successfully.
  bool SetInitialCalibration(
      const CalibrationDataThreeAxis& input_cal_data) final;

  // Returns the calibration sensor type.
  SensorType get_sensor_type() const final {
    return SensorType::kMagnetometerUt;
  };

 private:
  // MagCal algorithm data structure.
  MagCal mag_cal_;
};

}  // namespace online_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_MAGNETOMETER_MAG_DIVERSE_CAL_MAG_DIVERSE_CAL_H_
