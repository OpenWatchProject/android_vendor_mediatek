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

#include "calibration/online_calibration/common_data/calibration_data.h"

namespace online_calibration {

CalibrationTypeFlags operator|(CalibrationTypeFlags lhs,
                               CalibrationTypeFlags rhs) {
  return static_cast<CalibrationTypeFlags>(static_cast<char>(lhs) |
                                           static_cast<char>(rhs));
}

bool operator&(CalibrationTypeFlags lhs, CalibrationTypeFlags rhs) {
  return static_cast<char>(lhs) & static_cast<char>(rhs);
}

CalibrationTypeFlags& operator|=(CalibrationTypeFlags& lhs,
                                 CalibrationTypeFlags rhs) {
  lhs = static_cast<CalibrationTypeFlags>(static_cast<char>(lhs) |
                                          static_cast<char>(rhs));
  return lhs;
}

}  // namespace online_calibration
