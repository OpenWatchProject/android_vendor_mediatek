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

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_OVER_TEMP_OVER_TEMP_MODEL_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_OVER_TEMP_OVER_TEMP_MODEL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Defines the maximum size of the OverTempCal 'model_data' array.
#define OTC_MODEL_SIZE (40)

/*
 * Over-temperature data structures that contain a modeled sensor offset
 * estimate, an associated temperature, and the age of the data point since it
 * first entered the model.
 */

struct OverTempModelThreeAxis {
  // Sensor offset estimate, temperature, and age timestamp.
  uint64_t offset_age_nanos;  // [nanoseconds]
  float offset_temp_celsius;  // [Celsius]

  // Three-axis offset estimate (indices: 0=x, 1=y, 2=z).
  float offset[3];
};

struct OverTempModelSingleAxis {
  // Sensor offset estimate, temperature, and age timestamp.
  uint64_t offset_age_nanos;  // [nanoseconds]
  float offset_temp_celsius;  // [Celsius]

  // Single-axis offset estimate.
  float offset;
};

#ifdef __cplusplus
}
#endif

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_OVER_TEMP_OVER_TEMP_MODEL_H_
