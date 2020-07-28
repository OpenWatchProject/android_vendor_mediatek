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

#include "calibration/online_calibration/magnetometer/mag_diverse_cal/mag_diverse_cal.h"

#include "calibration/util/cal_log.h"

namespace online_calibration {

// Empirically estimated upper bounds on offset error.
constexpr float MagDiverseCal::kLowQualityUt;
constexpr float MagDiverseCal::kHighQualityUt;

void MagDiverseCal::Initialize(
    const MagCalParameters& mag_cal_parameters,
    const DiversityCheckerParameters& diversity_parameters) {
  initMagCal(&mag_cal_, &mag_cal_parameters, &diversity_parameters);
  InitializeCalData();
}

CalibrationTypeFlags MagDiverseCal::SetMeasurement(const SensorData& sample) {
  // Routes the input sensor sample to the calibration algorithm.
  MagUpdate new_calibration_update = MagUpdate::NO_UPDATE;
  switch (sample.type) {
    case SensorType::kMagnetometerUt:
      new_calibration_update = magCalUpdate(
          &mag_cal_, NanoToMicroseconds(sample.timestamp_nanos),
          sample.data[SensorIndex::kXAxis], sample.data[SensorIndex::kYAxis],
          sample.data[SensorIndex::kZAxis]);
      break;

    case SensorType::kTemperatureCelsius:
      temperature_celsius_ = sample.data[SensorIndex::kSingleAxis];
      break;

    default:
      // This sample is not required.
      return cal_update_polling_flags_;
  }

  // Checks for a new offset estimate, and updates the calibration data.
  if (MagUpdate::UPDATE_BIAS & new_calibration_update) {
    magCalGetBias(&mag_cal_, &cal_data_.offset[0], &cal_data_.offset[1],
                  &cal_data_.offset[2]);

    cal_data_.calibration_quality.level = CalibrationQualityLevel::HIGH_QUALITY;
    cal_data_.calibration_quality.value = kHighQualityUt;
    cal_data_.offset_temp_celsius = temperature_celsius_;
    cal_data_.cal_update_time_nanos = sample.timestamp_nanos;
    cal_update_polling_flags_ = CalibrationTypeFlags::BIAS;
    OnNotifyCalibrationUpdate(CalibrationTypeFlags::BIAS);
  }

  return cal_update_polling_flags_;
}

bool MagDiverseCal::SetInitialCalibration(
    const CalibrationDataThreeAxis& input_cal_data) {
  // Checks that the input calibration type matches the algorithm type.
  if (input_cal_data.type != get_sensor_type()) {
    CAL_DEBUG_LOG("[MagDiverseCal]",
                  "SetInitialCalibration failed due to wrong sensor type.");
    return false;
  }

  // Sets the magnetometer algorithm's calibration data.
  magCalReset(&mag_cal_);  // Resets the magnetometer's offset vector.
  magCalAddBias(&mag_cal_, input_cal_data.offset[0], input_cal_data.offset[1],
                input_cal_data.offset[2]);

  // Sync's all initial calibration data.
  cal_data_ = input_cal_data;

  // Sets the calibration quality to undetermined (uncertain magnetic history
  // makes the usefulness of the input calibration value unknown).
  cal_data_.calibration_quality.reset();

  return true;
}

}  // namespace online_calibration
