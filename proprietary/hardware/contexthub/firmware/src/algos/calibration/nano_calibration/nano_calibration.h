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

/*
 * This module provides a containing class (NanoSensorCal) for dynamic runtime
 * calibration algorithms that affect the following sensors:
 *       - Accelerometer (offset)
 *       - Gyroscope (offset, with over-temperature compensation)
 *       - Magnetometer (offset)
 *
 * Sensor Units:
 *       - Accelerometer [meters/sec^2]
 *       - Gyroscope     [radian/sec]
 *       - Magnetometer  [micro Tesla, uT]
 *       - Temperature   [Celsius].
 *
 * NOTE1: Define NANO_SENSOR_CAL_DBG_ENABLED to enable debug messaging.
 *
 * NOTE2: This module uses pointers to runtime calibration algorithm objects.
 * These must be constructed and initialized outside of this class. The owner
 * bares the burden of managing the lifetime of these objects with respect to
 * the NanoSensorCal class which depends on these objects and handles their
 * interaction with the Android ASH/CHRE system. This arrangement makes it
 * convenient to modify the specific algorithm implementations (i.e., choice of
 * calibration algorithm, parameter tuning, etc.) at the nanoapp level without
 * the need to specialize the standard functionality implemented here.
 */

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_NANO_CALIBRATION_NANO_CALIBRATION_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_NANO_CALIBRATION_NANO_CALIBRATION_H_

#include <stdbool.h>
#include <stdint.h>

#include <ash.h>
#include <chre.h>

#include "calibration/online_calibration/common_data/calibration_callback.h"
#include "calibration/online_calibration/common_data/calibration_data.h"
#include "calibration/online_calibration/common_data/online_calibration.h"
#include "calibration/online_calibration/common_data/sensor_data.h"
#include "common/math/macros.h"

namespace nano_calibration {

// Common log message sensor-specific identifiers.
constexpr char kAccelTag[] = {"[NanoSensorCal:ACCEL_MPS2]"};
constexpr char kGyroTag[] = {"[NanoSensorCal:GYRO_RPS]"};
constexpr char kMagTag[] = {"[NanoSensorCal:MAG_UT]"};

// Limits NanoSensorCal notifications to once every minute.
constexpr uint64_t kNanoSensorCalMessageIntervalNanos = MIN_TO_NANOS(1);

/*
 * NanoSensorCal is a container class for dynamic runtime calibration sensor
 * algorithms used by the IMU_Cal CHRE nanoapp. The main purpose of this class
 * is to transfer sensor data to the sensor calibration algorithms and provide
 * calibration updates to CHRE using the ASH API.
 */
class NanoSensorCal {
 public:
  // Alias used to reference the three-axis OnlineCalibration baseclass used by
  // the runtime calibration sensor wrappers. This is for convenience and to
  // help with code readability.
  using OnlineCalibrationThreeAxis = online_calibration::OnlineCalibration<
      online_calibration::CalibrationDataThreeAxis>;

  NanoSensorCal() = default;

  // Sets the sensor calibration object pointers and initializes the algorithms
  // using runtime values recalled using Android Sensor Hub (ASH). A nullptr may
  // be passed in to disable a particular sensor calibration.
  void Initialize(OnlineCalibrationThreeAxis *accel_cal,
                  OnlineCalibrationThreeAxis *gyro_cal,
                  OnlineCalibrationThreeAxis *mag_cal);

  // Sends new sensor samples to the calibration algorithms.
  void HandleSensorSamples(uint16_t event_type,
                           const chreSensorThreeAxisData *event_data);

  // Provides temperature updates to the calibration algorithms.
  void HandleTemperatureSamples(uint16_t event_type,
                                const chreSensorFloatData *event_data);

 private:
  // Passes sensor data to the runtime calibration algorithms.
  void ProcessSample(const online_calibration::SensorData &sample);

  // Loads runtime calibration data using the Android Sensor Hub API. Returns
  // 'true' when runtime calibration values were successfully recalled and used
  // for algorithm initialization. 'sensor_tag' is a string that identifies a
  // sensor-specific identifier for log messages. Updates 'flags' to indicate
  // which runtime calibration parameters were recalled.
  bool LoadAshCalibration(uint8_t chreSensorType,
                          OnlineCalibrationThreeAxis *online_cal,
                          online_calibration::CalibrationTypeFlags* flags,
                          const char *sensor_tag);

  // Provides sensor calibration updates using the ASH API for the specified
  // sensor type. 'cal_data' contains the new calibration data. 'flags' is used
  // to indicate all of the valid calibration values that should be provided
  // with the update. Returns 'true' with a successful ASH update.
  bool NotifyAshCalibration(
      uint8_t chreSensorType,
      const online_calibration::CalibrationDataThreeAxis &cal_data,
      online_calibration::CalibrationTypeFlags flags, const char *sensor_tag);

  // Checks whether 'ash_cal_parameters' is a valid set of runtime calibration
  // data and can be used for algorithm initialization. Updates 'flags' to
  // indicate which runtime calibration parameters were detected.
  bool DetectRuntimeCalibration(uint8_t chreSensorType, const char *sensor_tag,
                                online_calibration::CalibrationTypeFlags *flags,
                                ashCalParams *ash_cal_parameters);

  // Helper functions for logging calibration information.
  void PrintAshCalParams(const ashCalParams &cal_params,
                         const char *sensor_tag);

  void PrintCalibration(
      const online_calibration::CalibrationDataThreeAxis &cal_data,
      online_calibration::CalibrationTypeFlags flags, const char *sensor_tag);

  // Pointer to the accelerometer runtime calibration object.
  OnlineCalibrationThreeAxis *accel_cal_ = nullptr;

  // Pointer to the gyroscope runtime calibration object.
  OnlineCalibrationThreeAxis *gyro_cal_ = nullptr;

  // Limits the log messaging update rate for the gyro calibrations since these
  // can occur frequently with rapid temperature changes.
  uint64_t gyro_notification_time_nanos_ = 0;

  // Pointer to the magnetometer runtime calibration object.
  OnlineCalibrationThreeAxis *mag_cal_ = nullptr;

  // Flags that determine which calibration elements are updated with the ASH
  // API. These are reset during initialization, and latched when a particular
  // calibration update is detected upon a valid recall of parameters and/or
  // during runtime. The latching behavior is used to start sending calibration
  // values of a given type (e.g., bias, over-temp model, etc.) once they are
  // detected and thereafter.
  online_calibration::CalibrationTypeFlags accel_cal_update_flags_ =
      online_calibration::CalibrationTypeFlags::NONE;
  online_calibration::CalibrationTypeFlags gyro_cal_update_flags_ =
      online_calibration::CalibrationTypeFlags::NONE;
  online_calibration::CalibrationTypeFlags mag_cal_update_flags_ =
      online_calibration::CalibrationTypeFlags::NONE;
};

}  // namespace nano_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_NANO_CALIBRATION_NANO_CALIBRATION_H_
