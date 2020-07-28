/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_MAGNETOMETER_MAG_CAL_MAG_CAL_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_MAGNETOMETER_MAG_CAL_MAG_CAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "calibration/diversity_checker/diversity_checker.h"
#include "common/math/kasa.h"
#include "common/math/mat.h"
#include "common/math/vec.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MagUpdate {
  NO_UPDATE = 0x00,
  UPDATE_BIAS = 0x01,
  UPDATE_SPHERE_FIT = 0x02,
  UPDATE_BIAS_MAGGYRO_MEDIUM = 0x04,
  UPDATE_BIAS_MAGGYRO_HIGH = 0x08,
  MAGGYRO_TIMEOUT = 0x10,
};

#ifdef MAG_CAL_DEBUG_ENABLE
struct MagDbg {
  uint32_t mag_trigger_count;
  uint32_t kasa_count;
};
#endif

// MagCal algorithm parameters (see MagCal for details).
struct MagCalParameters {
  uint32_t min_batch_window_in_micros;
  float x_bias;  // [micro-Tesla]
  float y_bias;  // [micro-Tesla]
  float z_bias;  // [micro-Tesla]
  float c00;
  float c01;
  float c02;
  float c10;
  float c11;
  float c12;
  float c20;
  float c21;
  float c22;
};

struct MagCal {
  struct DiversityChecker diversity_checker;
  struct KasaFit kasa;

  uint64_t start_time;   // [micro-seconds]
  uint64_t update_time;  // [micro-seconds]
  uint32_t min_batch_window_in_micros;
  float x_bias, y_bias, z_bias;
  float radius;  // [micro-Tesla]
  bool kasa_batching;
  float c00, c01, c02, c10, c11, c12, c20, c21, c22;

#ifdef MAG_CAL_DEBUG_ENABLE
  struct MagDbg mag_dbg;
#endif
};

void initMagCal(struct MagCal *moc,
                const struct MagCalParameters *mag_cal_parameters,
                const struct DiversityCheckerParameters *diverse_parameters);

void magCalDestroy(struct MagCal *moc);

enum MagUpdate magCalUpdate(struct MagCal *moc, uint64_t sample_time_us,
                            float x, float y, float z);

void magCalGetBias(const struct MagCal *moc, float *x, float *y, float *z);

void magCalAddBias(struct MagCal *moc, float x, float y, float z);

void magCalRemoveBias(struct MagCal *moc, float xi, float yi, float zi,
                      float *xo, float *yo, float *zo);

void magCalSetSoftiron(struct MagCal *moc, float c00, float c01, float c02,
                       float c10, float c11, float c12, float c20, float c21,
                       float c22);

void magCalRemoveSoftiron(struct MagCal *moc, float xi, float yi, float zi,
                          float *xo, float *yo, float *zo);

void magCalReset(struct MagCal *moc);

#if defined MAG_CAL_DEBUG_ENABLE
void magLogPrint(struct DiversityChecker *moc, float temp);
#endif

#ifdef __cplusplus
}
#endif

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_MAGNETOMETER_MAG_CAL_MAG_CAL_H_
