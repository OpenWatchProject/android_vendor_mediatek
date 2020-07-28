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
 * This module provides the callback functionality employed by the online sensor
 * calibration algorithms.
 */

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_CALLBACK_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_CALLBACK_H_

#include "calibration/online_calibration/common_data/calibration_data.h"

namespace online_calibration {

/*
 * This codebase avoids Standard Template Library (STL) containers to maximize
 * its usefullness on embedded hardware with basic C++ compiler support. The
 * following uses type erasure to implement callback functionality to a user's
 * desired class method. The idea here is to store an object pointer by
 * instantiating a class template (Callback) which implements a virtual
 * interface function (CallbackInterface::Call).
 *
 * USAGE:
 * See unit testing for a simple example of how to use this for callback
 * functionality.
 */

// CalibrationType: Sets the calibration type (e.g., CalibrationDataThreeAxis).
template <class CalibrationType>
class CallbackInterface {
 public:
  // Interface function that is defined to implement the desired callback.
  virtual void Call(const CalibrationType& cal_data,
                    CalibrationTypeFlags cal_update_flags) = 0;
  virtual ~CallbackInterface() {}
};

// ClassCallerType: Sets the object's class type for the callback.
// CalibrationType: Sets the calibration type (e.g., CalibrationDataThreeAxis).
template <class ClassCallerType, class CalibrationType>
class Callback : public CallbackInterface<CalibrationType> {
 public:
  // Constructors.
  Callback() = delete;
  Callback(ClassCallerType* obj,
           void (ClassCallerType::*func)(const CalibrationType& cal_data,
                                         CalibrationTypeFlags cal_update_flags))
      : object_ptr_(obj), function_ptr_(func) {}

  // Implements the callback to the desired class-method.
  void Call(const CalibrationType& cal_data,
            CalibrationTypeFlags cal_update_flags) final {
    (object_ptr_->*function_ptr_)(cal_data, cal_update_flags);
  }

 private:
  // Pointer to the class that owns the callback method.
  ClassCallerType* object_ptr_;

  // Templated function pointer with the required function signature.
  // Calibration callbacks must accept:
  //   1. Constant reference to the calibration.
  //   2. Bitmask indicating which calibration components have been updated.
  void (ClassCallerType::*function_ptr_)(const CalibrationType& cal_data,
                                         CalibrationTypeFlags cal_update_flags);
};

}  // namespace online_calibration

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_ONLINE_CALIBRATION_COMMON_DATA_CALIBRATION_CALLBACK_H_
