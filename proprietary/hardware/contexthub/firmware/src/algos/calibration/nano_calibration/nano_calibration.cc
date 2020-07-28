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

#include "calibration/nano_calibration/nano_calibration.h"

#include <cstring>

#include "chre/util/nanoapp/log.h"

namespace nano_calibration {
namespace {

using ::online_calibration::CalibrationDataThreeAxis;
using ::online_calibration::CalibrationTypeFlags;
using ::online_calibration::SensorData;
using ::online_calibration::SensorIndex;
using ::online_calibration::SensorType;

// NanoSensorCal logging macros.
#ifdef NANO_SENSOR_CAL_DBG_ENABLED
#ifndef LOG_TAG
#define LOG_TAG "[ImuCal]"
#endif
#define NANO_CAL_LOGD(tag, format, ...) LOGD("%s " format, tag, ##__VA_ARGS__)
#define NANO_CAL_LOGI(tag, format, ...) LOGI("%s " format, tag, ##__VA_ARGS__)
#define NANO_CAL_LOGW(tag, format, ...) LOGW("%s " format, tag, ##__VA_ARGS__)
#define NANO_CAL_LOGE(tag, format, ...) LOGE("%s " format, tag, ##__VA_ARGS__)
#else
#define NANO_CAL_LOGD(tag, format, ...) CHRE_LOG_NULL(format, ##__VA_ARGS__)
#define NANO_CAL_LOGI(tag, format, ...) CHRE_LOG_NULL(format, ##__VA_ARGS__)
#define NANO_CAL_LOGW(tag, format, ...) CHRE_LOG_NULL(format, ##__VA_ARGS__)
#define NANO_CAL_LOGE(tag, format, ...) CHRE_LOG_NULL(format, ##__VA_ARGS__)
#endif  // NANO_SENSOR_CAL_DBG_ENABLED

}  // namespace

void NanoSensorCal::Initialize(OnlineCalibrationThreeAxis *accel_cal,
                               OnlineCalibrationThreeAxis *gyro_cal,
                               OnlineCalibrationThreeAxis *mag_cal) {
  // Loads stored calibration data and initializes the calibration algorithms.
  accel_cal_ = accel_cal;
  if (accel_cal_ != nullptr) {
    if (accel_cal_->get_sensor_type() == SensorType::kAccelerometerMps2) {
      LoadAshCalibration(CHRE_SENSOR_TYPE_ACCELEROMETER, accel_cal_,
                         &accel_cal_update_flags_, kAccelTag);
      NANO_CAL_LOGI(kAccelTag,
                    "Accelerometer runtime calibration initialized.");
    } else {
      accel_cal_ = nullptr;
      NANO_CAL_LOGE(kAccelTag, "Failed to initialize: wrong sensor type.");
    }
  }

  gyro_cal_ = gyro_cal;
  if (gyro_cal_ != nullptr) {
    if (gyro_cal_->get_sensor_type() == SensorType::kGyroscopeRps) {
      LoadAshCalibration(CHRE_SENSOR_TYPE_GYROSCOPE, gyro_cal_,
                         &gyro_cal_update_flags_, kGyroTag);
      NANO_CAL_LOGI(kGyroTag, "Gyroscope runtime calibration initialized.");
    } else {
      gyro_cal_ = nullptr;
      NANO_CAL_LOGE(kGyroTag, "Failed to initialize: wrong sensor type.");
    }
  }

  mag_cal_ = mag_cal;
  if (mag_cal != nullptr) {
    if (mag_cal->get_sensor_type() == SensorType::kMagnetometerUt) {
      LoadAshCalibration(CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD, mag_cal_,
                         &mag_cal_update_flags_, kMagTag);
      NANO_CAL_LOGI(kMagTag, "Magnetometer runtime calibration initialized.");
    } else {
      mag_cal_ = nullptr;
      NANO_CAL_LOGE(kMagTag, "Failed to initialize: wrong sensor type.");
    }
  }
}

void NanoSensorCal::HandleSensorSamples(
    uint16_t event_type, const chreSensorThreeAxisData *event_data) {
  // Converts CHRE Event -> SensorData::SensorType.
  SensorData sample;
  switch (event_type) {
    case CHRE_EVENT_SENSOR_UNCALIBRATED_ACCELEROMETER_DATA:
      sample.type = SensorType::kAccelerometerMps2;
      break;
    case CHRE_EVENT_SENSOR_UNCALIBRATED_GYROSCOPE_DATA:
      sample.type = SensorType::kGyroscopeRps;
      break;
    case CHRE_EVENT_SENSOR_UNCALIBRATED_GEOMAGNETIC_FIELD_DATA:
      sample.type = SensorType::kMagnetometerUt;
      break;
    default:
      // This sensor type is not used.
      NANO_CAL_LOGW("[NanoSensorCal]",
                    "Unexpected 3-axis sensor type received.");
      return;
  }

  // Sends the sensor payload to the calibration algorithms and checks for
  // calibration updates.
  const auto &header = event_data->header;
  const auto *data = event_data->readings;
  sample.timestamp_nanos = header.baseTimestamp;
  for (size_t i = 0; i < header.readingCount; i++) {
    sample.timestamp_nanos += data[i].timestampDelta;
    memcpy(sample.data, data[i].v, sizeof(sample.data));
    ProcessSample(sample);
  }
}

void NanoSensorCal::HandleTemperatureSamples(
    uint16_t event_type, const chreSensorFloatData *event_data) {
  // Computes the mean of the batched temperature samples and delivers it to the
  // calibration algorithms. Note, the temperature sensor batch size determines
  // its minimum update interval.
  if (event_type == CHRE_EVENT_SENSOR_ACCELEROMETER_TEMPERATURE_DATA &&
      event_data->header.readingCount > 0) {
    const auto header = event_data->header;
    const auto *data = event_data->readings;

    SensorData sample;
    sample.type = SensorType::kTemperatureCelsius;
    sample.timestamp_nanos = header.baseTimestamp;

    float accum_temperature_celsius = 0.0f;
    for (size_t i = 0; i < header.readingCount; i++) {
      sample.timestamp_nanos += data[i].timestampDelta;
      accum_temperature_celsius += data[i].value;
    }
    sample.data[SensorIndex::kSingleAxis] =
        accum_temperature_celsius / header.readingCount;
    ProcessSample(sample);
  } else {
    NANO_CAL_LOGW("[NanoSensorCal]",
                  "Unexpected single-axis sensor type received.");
  }
}

void NanoSensorCal::ProcessSample(const SensorData &sample) {
  // Sends a new sensor sample to each active calibration algorithm and sends
  // out notifications for new calibration updates.
  if (accel_cal_ != nullptr) {
    const CalibrationTypeFlags new_cal_flags =
        accel_cal_->SetMeasurement(sample);
    if (new_cal_flags != CalibrationTypeFlags::NONE) {
      accel_cal_update_flags_ |= new_cal_flags;
      NotifyAshCalibration(CHRE_SENSOR_TYPE_ACCELEROMETER,
                           accel_cal_->GetSensorCalibration(),
                           accel_cal_update_flags_, kAccelTag);
      PrintCalibration(accel_cal_->GetSensorCalibration(),
                       accel_cal_update_flags_, kAccelTag);
    }
  }

  if (gyro_cal_ != nullptr) {
    const CalibrationTypeFlags new_cal_flags =
        gyro_cal_->SetMeasurement(sample);
    if (new_cal_flags != CalibrationTypeFlags::NONE) {
      gyro_cal_update_flags_ |= new_cal_flags;
      if (NotifyAshCalibration(CHRE_SENSOR_TYPE_GYROSCOPE,
                               gyro_cal_->GetSensorCalibration(),
                               gyro_cal_update_flags_, kGyroTag)) {
        // Limits the log messaging update rate for the gyro calibrations since
        // these can occur frequently with rapid temperature changes.
        if (NANO_TIMER_CHECK_T1_GEQUAL_T2_PLUS_DELTA(
                sample.timestamp_nanos, gyro_notification_time_nanos_,
                kNanoSensorCalMessageIntervalNanos)) {
          gyro_notification_time_nanos_ = sample.timestamp_nanos;
          PrintCalibration(gyro_cal_->GetSensorCalibration(),
                           gyro_cal_update_flags_, kGyroTag);
        }
      }
    }
  }

  if (mag_cal_ != nullptr) {
    const CalibrationTypeFlags new_cal_flags = mag_cal_->SetMeasurement(sample);
    if (new_cal_flags != CalibrationTypeFlags::NONE) {
      mag_cal_update_flags_ |= new_cal_flags;
      NotifyAshCalibration(CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD,
                           mag_cal_->GetSensorCalibration(),
                           mag_cal_update_flags_, kMagTag);
      PrintCalibration(mag_cal_->GetSensorCalibration(), mag_cal_update_flags_,
                       kMagTag);
    }
  }
}

bool NanoSensorCal::NotifyAshCalibration(
    uint8_t chreSensorType, const CalibrationDataThreeAxis &cal_data,
    CalibrationTypeFlags flags, const char *sensor_tag) {
  // Updates the sensor offset calibration using the ASH API.
  ashCalInfo ash_cal_info;
  memset(&ash_cal_info, 0, sizeof(ashCalInfo));
  ash_cal_info.compMatrix[0] = 1.0f;  // Sets diagonal to unity (scale factor).
  ash_cal_info.compMatrix[4] = 1.0f;
  ash_cal_info.compMatrix[8] = 1.0f;
  memcpy(ash_cal_info.bias, cal_data.offset, sizeof(ash_cal_info.bias));

  // Maps CalibrationQualityLevel to ASH calibration accuracy.
  switch (cal_data.calibration_quality.level) {
    case online_calibration::CalibrationQualityLevel::HIGH_QUALITY:
      ash_cal_info.accuracy = ASH_CAL_ACCURACY_HIGH;
      break;

    case online_calibration::CalibrationQualityLevel::MEDIUM_QUALITY:
      ash_cal_info.accuracy = ASH_CAL_ACCURACY_MEDIUM;
      break;

    case online_calibration::CalibrationQualityLevel::LOW_QUALITY:
      ash_cal_info.accuracy = ASH_CAL_ACCURACY_LOW;
      break;

    default:
      ash_cal_info.accuracy = ASH_CAL_ACCURACY_UNRELIABLE;
      break;
  }

  if (!ashSetCalibration(chreSensorType, &ash_cal_info)) {
    NANO_CAL_LOGE(sensor_tag, "ASH failed to apply calibration update.");
    return false;
  }

  // Uses the ASH API to store all calibration parameters relevant to a given
  // algorithm as indicated by the input calibration type flags.
  ashCalParams ash_cal_parameters;
  memset(&ash_cal_parameters, 0, sizeof(ashCalParams));
  if (flags & CalibrationTypeFlags::BIAS) {
    ash_cal_parameters.offsetTempCelsius = cal_data.offset_temp_celsius;
    memcpy(ash_cal_parameters.offset, cal_data.offset,
           sizeof(ash_cal_parameters.offset));
    ash_cal_parameters.offsetSource = ASH_CAL_PARAMS_SOURCE_RUNTIME;
    ash_cal_parameters.offsetTempCelsiusSource = ASH_CAL_PARAMS_SOURCE_RUNTIME;
  }

  if (flags & CalibrationTypeFlags::OVER_TEMP) {
    memcpy(ash_cal_parameters.tempSensitivity, cal_data.temp_sensitivity,
           sizeof(ash_cal_parameters.tempSensitivity));
    memcpy(ash_cal_parameters.tempIntercept, cal_data.temp_intercept,
           sizeof(ash_cal_parameters.tempIntercept));
    ash_cal_parameters.tempSensitivitySource = ASH_CAL_PARAMS_SOURCE_RUNTIME;
    ash_cal_parameters.tempInterceptSource = ASH_CAL_PARAMS_SOURCE_RUNTIME;
  }

  if (!ashSaveCalibrationParams(chreSensorType, &ash_cal_parameters)) {
    NANO_CAL_LOGE(sensor_tag, "ASH failed to write calibration update.");
    return false;
  }

  return true;
}

bool NanoSensorCal::LoadAshCalibration(uint8_t chreSensorType,
                                       OnlineCalibrationThreeAxis *online_cal,
                                       CalibrationTypeFlags* flags,
                                       const char *sensor_tag) {
  ashCalParams recalled_ash_cal_parameters;
  if (ashLoadCalibrationParams(chreSensorType, ASH_CAL_STORAGE_ASH,
                               &recalled_ash_cal_parameters)) {
    // Checks whether a valid set of runtime calibration parameters was received
    // and can be used for initialization.
    if (DetectRuntimeCalibration(chreSensorType, sensor_tag, flags,
                                 &recalled_ash_cal_parameters)) {
      CalibrationDataThreeAxis cal_data;
      cal_data.type = online_cal->get_sensor_type();
      cal_data.cal_update_time_nanos = chreGetTime();

      // Analyzes the calibration flags and sets only the runtime calibration
      // values that were received.
      if (*flags & CalibrationTypeFlags::BIAS) {
        cal_data.offset_temp_celsius =
            recalled_ash_cal_parameters.offsetTempCelsius;
        memcpy(cal_data.offset, recalled_ash_cal_parameters.offset,
               sizeof(cal_data.offset));
      }

      if (*flags & CalibrationTypeFlags::OVER_TEMP) {
        memcpy(cal_data.temp_sensitivity,
               recalled_ash_cal_parameters.tempSensitivity,
               sizeof(cal_data.temp_sensitivity));
        memcpy(cal_data.temp_intercept,
               recalled_ash_cal_parameters.tempIntercept,
               sizeof(cal_data.temp_intercept));
      }

      // Sets the algorithm's initial calibration data and notifies ASH to apply
      // the recalled calibration data.
      if (online_cal->SetInitialCalibration(cal_data)) {
        return NotifyAshCalibration(chreSensorType,
                                    online_cal->GetSensorCalibration(), *flags,
                                    sensor_tag);
      } else {
        NANO_CAL_LOGE(sensor_tag,
                      "Calibration data failed to initialize algorithm.");
      }
    }
  } else {
    NANO_CAL_LOGE(sensor_tag, "ASH failed to recall calibration data.");
  }

  return false;
}

bool NanoSensorCal::DetectRuntimeCalibration(uint8_t chreSensorType,
                                             const char *sensor_tag,
                                             CalibrationTypeFlags *flags,
                                             ashCalParams *ash_cal_parameters) {
  // Analyzes calibration source flags to determine whether runtime
  // calibration values have been loaded and may be used for initialization. A
  // valid runtime calibration source will include at least an offset.
  *flags = CalibrationTypeFlags::NONE;  // Resets the calibration flags.

  // Uses the ASH calibration source flags to set the appropriate
  // CalibrationTypeFlags. These will be used to determine which values to copy
  // from 'ash_cal_parameters' and provide to the calibration algorithms for
  // initialization.
  bool runtime_cal_detected = false;
  if (ash_cal_parameters->offsetSource == ASH_CAL_PARAMS_SOURCE_RUNTIME &&
      ash_cal_parameters->offsetTempCelsiusSource ==
          ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    runtime_cal_detected = true;
    *flags = CalibrationTypeFlags::BIAS;
  }

  if (ash_cal_parameters->tempSensitivitySource ==
          ASH_CAL_PARAMS_SOURCE_RUNTIME &&
      ash_cal_parameters->tempInterceptSource ==
          ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    *flags |= CalibrationTypeFlags::OVER_TEMP;
  }

  if (runtime_cal_detected) {
    // Prints the retrieved runtime calibration data.
    NANO_CAL_LOGI(sensor_tag, "Runtime calibration data detected.");
    PrintAshCalParams(*ash_cal_parameters, sensor_tag);
  } else {
    // This is a warning (not an error) since the runtime algorithms will
    // function correctly with no recalled calibration values. They will
    // eventually trigger and update the system with valid calibration data.
    NANO_CAL_LOGW(sensor_tag, "No runtime offset calibration data found.");
  }

  return runtime_cal_detected;
}

// Helper functions for logging calibration information.
void NanoSensorCal::PrintAshCalParams(const ashCalParams &cal_params,
                                      const char *sensor_tag) {
  if (cal_params.offsetSource == ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    NANO_CAL_LOGI(sensor_tag,
                  "Offset | Temperature [C]: %.6f, %.6f, %.6f | %.2f",
                  cal_params.offset[0], cal_params.offset[1],
                  cal_params.offset[2], cal_params.offsetTempCelsius);
  }

  if (cal_params.tempSensitivitySource == ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    NANO_CAL_LOGI(sensor_tag, "Temp Sensitivity [units/C]: %.6f, %.6f, %.6f",
                  cal_params.tempSensitivity[0], cal_params.tempSensitivity[1],
                  cal_params.tempSensitivity[2]);
  }

  if (cal_params.tempInterceptSource == ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    NANO_CAL_LOGI(sensor_tag, "Temp Intercept [units]: %.6f, %.6f, %.6f",
                  cal_params.tempIntercept[0], cal_params.tempIntercept[1],
                  cal_params.tempIntercept[2]);
  }

  if (cal_params.scaleFactorSource == ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    NANO_CAL_LOGI(sensor_tag, "Scale Factor: %.6f, %.6f, %.6f",
                  cal_params.scaleFactor[0], cal_params.scaleFactor[1],
                  cal_params.scaleFactor[2]);
  }

  if (cal_params.crossAxisSource == ASH_CAL_PARAMS_SOURCE_RUNTIME) {
    NANO_CAL_LOGI(sensor_tag,
                  "Cross-Axis in [yx, zx, zy] order: %.6f, %.6f, %.6f",
                  cal_params.crossAxis[0], cal_params.crossAxis[1],
                  cal_params.crossAxis[2]);
  }
}

void NanoSensorCal::PrintCalibration(const CalibrationDataThreeAxis &cal_data,
                                     CalibrationTypeFlags flags,
                                     const char *sensor_tag) {
  if (flags & CalibrationTypeFlags::BIAS) {
    NANO_CAL_LOGI(sensor_tag,
                  "Offset | Temperature [C]: %.6f, %.6f, %.6f | %.2f",
                  cal_data.offset[0], cal_data.offset[1], cal_data.offset[2],
                  cal_data.offset_temp_celsius);
  }

  if (flags & CalibrationTypeFlags::OVER_TEMP) {
    NANO_CAL_LOGI(sensor_tag, "Temp Sensitivity: %.6f, %.6f, %.6f",
                  cal_data.temp_sensitivity[0], cal_data.temp_sensitivity[1],
                  cal_data.temp_sensitivity[2]);
    NANO_CAL_LOGI(sensor_tag, "Temp Intercept: %.6f, %.6f, %.6f",
                  cal_data.temp_intercept[0], cal_data.temp_intercept[1],
                  cal_data.temp_intercept[2]);
  }
}

}  // namespace nano_calibration
