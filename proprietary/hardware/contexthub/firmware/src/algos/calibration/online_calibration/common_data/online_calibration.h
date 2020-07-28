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

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_ONLINE_CALIBRATION_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_ONLINE_CALIBRATION_H_

#include <string.h>

#include "calibration/online_calibration/common_data/calibration_callback.h"
#include "calibration/online_calibration/common_data/calibration_data.h"
#include "calibration/online_calibration/common_data/sensor_data.h"

namespace online_calibration {

/*
 * This abstract base class provides a set of general interface functions for
 * calibration algorithms. The data structures used are intended to be lean and
 * portable to a wide variety of software and hardware systems. Algorithm
 * wrappers may use this as a basis for providing the following functionality:
 *
 *   SetMeasurement - Delivers new sensor data.
 *   SetInitialCalibration - Initializes the algorithm's calibration data.
 *   GetSensorCalibration - Retrieves the latest calibration data set.
 *   new_calibration_ready - Used to poll for new calibration updates.
 *   SetCalibrationCallback - User provides a pointer its own Callback object.
 *   UpdateDynamicSystemSettings - Provides feedback to adjust system behavior.
 *   get_sensor_type - Returns the sensor type which is being calibrated.
 *
 * NOTE 1: This class accomodates two methods of providing calibration updates.
 * Either, or both, may be used depending on system requirements. 1) Polling can
 * be achieved with new_calibration_ready/GetSensorCalibration functions. 2)
 * Callback notification of new calibration updates can managed using the
 * SetCalibrationCallback function.
 *
 * NOTE 2: This code implementation specifically avoids using standard template
 * libraries (STL) and other external API’s since they may not be fully
 * supported on embedded hardware targets. Only basic C/C++ support will be
 * assumed.
 */

// CalibrationType: Sets the calibration type (e.g., CalibrationDataThreeAxis).
template <class CalibrationType>
class OnlineCalibration {
 public:
  // Virtual destructor.
  virtual ~OnlineCalibration() {}

  // Sends new sensor data to the calibration algorithm, and returns the state
  // of the calibration update flags, 'cal_update_polling_flags_'.
  virtual CalibrationTypeFlags SetMeasurement(const SensorData& sample) = 0;

  // Sets the initial calibration data of the calibration algorithm. Returns
  // "true" if set successfully.
  virtual bool SetInitialCalibration(const CalibrationType& cal_data) = 0;

  // Polling Updates: New calibration updates are generated during
  // SetMeasurement and the 'cal_update_polling_flags_' are set according to
  // which calibration values have changed. To prevent missing updates in
  // systems that use polling, this bitmask remains latched until the
  // calibration data is retrieved with this function.
  const CalibrationType& GetSensorCalibration() const {
    cal_update_polling_flags_ = CalibrationTypeFlags::NONE;
    return cal_data_;
  }

  // Polling Updates: This function returns 'cal_update_polling_flags_' to
  // indicate which calibration components have a pending update. The updated
  // calibration data may be retrieved with GetSensorCalibration, and the
  // 'cal_update_polling_flags_' will reset.
  CalibrationTypeFlags new_calibration_ready() const {
    return cal_update_polling_flags_;
  }

  // Sets the pointer to the CallbackInterface object used for notification of
  // new calibration updates.
  void SetCalibrationCallback(
      CallbackInterface<CalibrationType>* calibration_callback) {
    calibration_callback_ = calibration_callback;
  }

  // Returns the sensor-type this calibration algorithm provides updates for.
  virtual SensorType get_sensor_type() const = 0;

 protected:
  // Helper function that activates the registered callback.
  void OnNotifyCalibrationUpdate(CalibrationTypeFlags cal_update_flags) const {
    if (calibration_callback_ != nullptr) {
      calibration_callback_->Call(cal_data_, cal_update_flags);
    }
  }

  // Helper function used to initialize the calibration data.
  void InitializeCalData() {
    cal_data_.reset();
    cal_data_.type = get_sensor_type();
    cal_update_polling_flags_ = CalibrationTypeFlags::NONE;
  }

  // Stores the sensor calibration data.
  CalibrationType cal_data_;

  // Tracks the most recent sensor temperature value.
  float temperature_celsius_ = kInvalidTemperatureCelsius;

  // This bitmask indicates which subset of calibration parameters have changed
  // and is used specifically for polling; the callback notification passes its
  // own set of update flags which do not need this latching behavior. Marked
  // mutable so that these flags may be reset when GetSensorCalibration is
  // called.
  mutable CalibrationTypeFlags cal_update_polling_flags_ =
      CalibrationTypeFlags::NONE;

 private:
  // Pointer to a callback object.
  CallbackInterface<CalibrationType>* calibration_callback_ = nullptr;
};

}  // namespace online_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_ONLINE_CALIBRATION_H_
