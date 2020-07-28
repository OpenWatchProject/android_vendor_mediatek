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

#include "calibration/online_calibration/gyroscope/gyro_offset_over_temp_cal/gyro_offset_over_temp_cal.h"

#include "calibration/util/cal_log.h"

namespace online_calibration {

// Estimated upper bounds on gyro offset error (i.e., 3-sigma values).
constexpr float GyroOffsetOtcCal::kMediumQualityRps;
constexpr float GyroOffsetOtcCal::kHighQualityRps;

void GyroOffsetOtcCal::Initialize(const GyroCalParameters& gyro_cal_parameters,
                                  const OverTempCalParameters& otc_parameters) {
  gyroCalInit(&gyro_cal_, &gyro_cal_parameters);
  overTempCalInit(&over_temp_cal_, &otc_parameters);
  InitializeCalData();
}

CalibrationTypeFlags GyroOffsetOtcCal::SetMeasurement(
    const SensorData& sample) {
  // Routes the input sensor sample to the calibration algorithm.
  switch (sample.type) {
    case SensorType::kAccelerometerMps2:
      gyroCalUpdateAccel(&gyro_cal_, sample.timestamp_nanos,
                         sample.data[SensorIndex::kXAxis],
                         sample.data[SensorIndex::kYAxis],
                         sample.data[SensorIndex::kZAxis]);  // [m/sec^2]
      break;

    case SensorType::kGyroscopeRps:
      gyroCalUpdateGyro(&gyro_cal_, sample.timestamp_nanos,
                        sample.data[SensorIndex::kXAxis],
                        sample.data[SensorIndex::kYAxis],
                        sample.data[SensorIndex::kZAxis],  // [rad/sec]
                        temperature_celsius_);
      break;

    case SensorType::kMagnetometerUt:
      gyroCalUpdateMag(&gyro_cal_, sample.timestamp_nanos,
                       sample.data[SensorIndex::kXAxis],
                       sample.data[SensorIndex::kYAxis],
                       sample.data[SensorIndex::kZAxis]);  // [micro-Tesla]
      break;

    case SensorType::kTemperatureCelsius:
      temperature_celsius_ = sample.data[SensorIndex::kSingleAxis];
      overTempCalSetTemperature(&over_temp_cal_, sample.timestamp_nanos,
                                temperature_celsius_);
      break;

    default:
      // This sample is not required.
      return cal_update_polling_flags_;
  }

  // Checks for a new calibration, and updates the OTC.
  if (gyroCalNewBiasAvailable(&gyro_cal_)) {
    float offset[3];
    float temperature_celsius = kInvalidTemperatureCelsius;
    uint64_t calibration_time_nanos = 0;
    gyroCalGetBias(&gyro_cal_, &offset[0], &offset[1], &offset[2],
                   &temperature_celsius, &calibration_time_nanos);
    overTempCalUpdateSensorEstimate(&over_temp_cal_, calibration_time_nanos,
                                    offset, temperature_celsius);
  }

  // Checks the OTC for a new calibration model update.
  const bool new_otc_model_update =
      overTempCalNewModelUpdateAvailable(&over_temp_cal_);

  // Checks for a change in the temperature compensated offset estimate.
  const bool new_otc_offset = overTempCalNewOffsetAvailable(&over_temp_cal_);

  // Sets the new calibration data.
  CalibrationTypeFlags cal_update_callback_flags = CalibrationTypeFlags::NONE;
  if (new_otc_offset) {
    overTempCalGetOffset(&over_temp_cal_, &cal_data_.offset_temp_celsius,
                         cal_data_.offset);
    cal_data_.cal_update_time_nanos = sample.timestamp_nanos;
    cal_update_callback_flags |= CalibrationTypeFlags::BIAS;
  }

  if (new_otc_model_update) {
    // Sets the pointer to the OTC model dataset and the number of model points.
    cal_data_.otc_model_data = over_temp_cal_.model_data;
    cal_data_.num_model_pts = over_temp_cal_.num_model_pts;

    cal_update_callback_flags |= CalibrationTypeFlags::OVER_TEMP;
    overTempCalGetModel(&over_temp_cal_, cal_data_.offset,
                        &cal_data_.offset_temp_celsius,
                        &cal_data_.cal_update_time_nanos,
                        cal_data_.temp_sensitivity, cal_data_.temp_intercept);
  }

  // Sets the new calibration quality, polling flag, and notifies a calibration
  // callback listener of the new update.
  if (new_otc_model_update || new_otc_offset) {
    cal_data_.calibration_quality.level = CalibrationQualityLevel::HIGH_QUALITY;
    cal_data_.calibration_quality.value = kHighQualityRps;
    cal_update_polling_flags_ |= cal_update_callback_flags;
    OnNotifyCalibrationUpdate(cal_update_callback_flags);
  }

  // Print debug data reports.
#ifdef GYRO_CAL_DBG_ENABLED
  gyroCalDebugPrint(&gyro_cal_, sample.timestamp_nanos);
#endif  // GYRO_CAL_DBG_ENABLED
#ifdef OVERTEMPCAL_DBG_ENABLED
  overTempCalDebugPrint(&over_temp_cal_, sample.timestamp_nanos);
#endif  // OVERTEMPCAL_DBG_ENABLED

  return cal_update_polling_flags_;
}

bool GyroOffsetOtcCal::SetInitialCalibration(
    const CalibrationDataThreeAxis& input_cal_data) {
  // Checks that the input calibration type matches the algorithm type.
  if (input_cal_data.type != get_sensor_type()) {
    CAL_DEBUG_LOG("[GyroOffsetOtcCal]",
                  "SetInitialCalibration failed due to wrong sensor type.");
    return false;
  }

  // Sync's all initial calibration data.
  cal_data_ = input_cal_data;

  // Sets the initial calibration data (offset and OTC model parameters).
  gyroCalSetBias(&gyro_cal_, cal_data_.offset[0], cal_data_.offset[1],
                 cal_data_.offset[2], cal_data_.offset_temp_celsius,
                 cal_data_.cal_update_time_nanos);

  overTempCalSetModel(&over_temp_cal_, cal_data_.offset,
                      cal_data_.offset_temp_celsius,
                      cal_data_.cal_update_time_nanos,
                      cal_data_.temp_sensitivity, cal_data_.temp_intercept,
                      /*jump_start_model=*/false);

  // Sets the calibration quality.
  cal_data_.calibration_quality.level = CalibrationQualityLevel::MEDIUM_QUALITY;
  cal_data_.calibration_quality.value = kMediumQualityRps;

  const bool load_new_model_dataset =
      (input_cal_data.otc_model_data != nullptr &&
       input_cal_data.num_model_pts > 0);

  if (load_new_model_dataset) {
    // Loads the new model dataset and uses it to update the linear model
    // parameters.
    overTempCalSetModelData(&over_temp_cal_, input_cal_data.num_model_pts,
                            cal_data_.cal_update_time_nanos,
                            input_cal_data.otc_model_data);
  }

  // Sets the pointer to the OTC model dataset and the number of model points.
  cal_data_.otc_model_data = over_temp_cal_.model_data;
  cal_data_.num_model_pts = over_temp_cal_.num_model_pts;

  return true;
}

}  // namespace online_calibration
