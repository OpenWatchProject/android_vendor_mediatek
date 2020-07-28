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

#include "calibration/magnetometer/mag_cal/mag_cal.h"

#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include "calibration/util/cal_log.h"

// Local helper macro for printing log messages.
#ifdef MAG_CAL_DEBUG_ENABLE
#ifdef CAL_NO_FLOAT_FORMAT_STRINGS
#define CAL_FORMAT_MAG_MEMORY                                          \
  "%s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, " \
  "%s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, " \
  "%s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, %s%d.%03d, " \
  "%s%d.%03d, %s%d.%03d"
#else
#define CAL_FORMAT_MAG_MEMORY                                                \
  "%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, " \
  "%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f"
#endif  // CAL_NO_FLOAT_FORMAT_STRINGS
#endif  // MAG_CAL_DEBUG_ENABLE

// clang-format off
#define MAX_EIGEN_RATIO 15.0f
#define MAX_EIGEN_MAG 70.0f          // uT
#define MIN_EIGEN_MAG 20.0f          // uT
#define MAX_FIT_MAG 70.0f
#define MIN_FIT_MAG 20.0f
#define MAX_BATCH_WINDOW 15000000UL  // 15 sec
#define MIN_BATCH_SIZE 25            // samples
#define MAX_DISTANCE_VIOLATIONS 2
// clang-format

// eigen value magnitude and ratio test.
static int moc_eigen_test(struct KasaFit *kasa) {
  // covariance matrix.
  struct Mat33 S;
  S.elem[0][0] = kasa->acc_xx - kasa->acc_x * kasa->acc_x;
  S.elem[0][1] = S.elem[1][0] = kasa->acc_xy - kasa->acc_x * kasa->acc_y;
  S.elem[0][2] = S.elem[2][0] = kasa->acc_xz - kasa->acc_x * kasa->acc_z;
  S.elem[1][1] = kasa->acc_yy - kasa->acc_y * kasa->acc_y;
  S.elem[1][2] = S.elem[2][1] = kasa->acc_yz - kasa->acc_y * kasa->acc_z;
  S.elem[2][2] = kasa->acc_zz - kasa->acc_z * kasa->acc_z;

  struct Vec3 eigenvals;
  struct Mat33 eigenvecs;
  mat33GetEigenbasis(&S, &eigenvals, &eigenvecs);

  float evmax = (eigenvals.x > eigenvals.y) ? eigenvals.x : eigenvals.y;
  evmax = (eigenvals.z > evmax) ? eigenvals.z : evmax;

  float evmin = (eigenvals.x < eigenvals.y) ? eigenvals.x : eigenvals.y;
  evmin = (eigenvals.z < evmin) ? eigenvals.z : evmin;

  float eigenvals_sum = eigenvals.x + eigenvals.y + eigenvals.z;

  // Testing for negative number.
  float evmag = (eigenvals_sum > 0) ? sqrtf(eigenvals_sum) : 0;

  int eigen_pass = (evmin * MAX_EIGEN_RATIO > evmax) &&
                   (evmag > MIN_EIGEN_MAG) && (evmag < MAX_EIGEN_MAG);

  return eigen_pass;
}

void magCalReset(struct MagCal *moc) {
  kasaReset(&moc->kasa);
  diversityCheckerReset(&moc->diversity_checker);
  moc->start_time = 0;
  moc->kasa_batching = false;
}

static bool moc_batch_complete(struct MagCal *moc, uint64_t sample_time_us) {
  bool complete = false;

  if ((sample_time_us - moc->start_time > moc->min_batch_window_in_micros) &&
      (moc->kasa.nsamples > MIN_BATCH_SIZE)) {
    complete = true;

  } else if (sample_time_us - moc->start_time > MAX_BATCH_WINDOW) {
    // not enough samples collected in MAX_BATCH_WINDOW or too many
    // maximum distance violations detected.
    magCalReset(moc);
  }

  return complete;
}

void initMagCal(struct MagCal *moc,
                const struct MagCalParameters *mag_cal_parameters,
                const struct DiversityCheckerParameters *diverse_parameters) {
  magCalReset(moc);
  moc->update_time = 0;
  moc->min_batch_window_in_micros =
      mag_cal_parameters->min_batch_window_in_micros;
  moc->radius = 0.0f;

  moc->x_bias = mag_cal_parameters->x_bias;
  moc->y_bias = mag_cal_parameters->y_bias;
  moc->z_bias = mag_cal_parameters->z_bias;

  moc->c00 = mag_cal_parameters->c00;
  moc->c01 = mag_cal_parameters->c01;
  moc->c02 = mag_cal_parameters->c02;
  moc->c10 = mag_cal_parameters->c10;
  moc->c11 = mag_cal_parameters->c11;
  moc->c12 = mag_cal_parameters->c12;
  moc->c20 = mag_cal_parameters->c20;
  moc->c21 = mag_cal_parameters->c21;
  moc->c22 = mag_cal_parameters->c22;

#ifdef MAG_CAL_DEBUG_ENABLE
  moc->mag_dbg.mag_trigger_count = 0;
  moc->mag_dbg.kasa_count = 0;
#endif  // MAG_CAL_DEBUG_ENABLE

  // Diversity Checker
  diversityCheckerInit(&moc->diversity_checker, diverse_parameters);
}

void magCalDestroy(struct MagCal *moc) { (void)moc; }

enum MagUpdate magCalUpdate(struct MagCal *moc, uint64_t sample_time_us,
                            float x, float y, float z) {
  enum MagUpdate new_bias = NO_UPDATE;

  // Diversity Checker Update.
  diversityCheckerUpdate(&moc->diversity_checker, x, y, z);

  // 1. run accumulators
  kasaAccumulate(&moc->kasa, x, y, z);

  if (moc->kasa.nsamples == 1) {
    moc->start_time = sample_time_us;
    moc->kasa_batching = true;
  }

  // 2. batch has enough samples?
  if (moc_batch_complete(moc, sample_time_us)) {
    kasaNormalize(&moc->kasa);

    // 3. eigen test
    if (moc_eigen_test(&moc->kasa)) {
      struct Vec3 bias;
      float radius;
      // 4. Kasa sphere fitting
      if (kasaFit(&moc->kasa, &bias, &radius, MAX_FIT_MAG, MIN_FIT_MAG)) {
#ifdef MAG_CAL_DEBUG_ENABLE
        moc->mag_dbg.kasa_count++;
        CAL_DEBUG_LOG("[MAG_CAL:KASA UPDATE] :", CAL_FORMAT_3DIGITS_TRIPLET
                      ", " CAL_FORMAT_3DIGITS ", %" PRIu32 ", %" PRIu32,
                      CAL_ENCODE_FLOAT(bias.x, 3), CAL_ENCODE_FLOAT(bias.y, 3),
                      CAL_ENCODE_FLOAT(bias.z, 3), CAL_ENCODE_FLOAT(radius, 3),
                      moc->mag_dbg.kasa_count, moc->mag_dbg.mag_trigger_count);
#endif  // MAG_CAL_DEBUG_ENABLE

        // Update the local field.
        diversityCheckerLocalFieldUpdate(&moc->diversity_checker, radius);

        // checking if data is diverse.
        if (diversityCheckerNormQuality(&moc->diversity_checker, bias.x, bias.y,
                                        bias.z) &&
            moc->diversity_checker.num_max_dist_violations <=
                MAX_DISTANCE_VIOLATIONS) {
          // DEBUG PRINT OUT.
#ifdef MAG_CAL_DEBUG_ENABLE
          moc->mag_dbg.mag_trigger_count++;
          moc->diversity_checker.diversity_dbg.new_trigger = 1;
          CAL_DEBUG_LOG(
              "[MAG_CAL:BIAS UPDATE] :", CAL_FORMAT_3DIGITS_TRIPLET ", "
              CAL_FORMAT_3DIGITS ", " CAL_FORMAT_6DIGITS ", "
              CAL_FORMAT_3DIGITS_TRIPLET ", %zu, " CAL_FORMAT_3DIGITS ", "
              CAL_FORMAT_3DIGITS ", %" PRIu32 ", %" PRIu32 ", %" PRIu64 ", "
              CAL_FORMAT_3DIGITS_TRIPLET ", %" PRIu64 "",
              CAL_ENCODE_FLOAT(bias.x, 3), CAL_ENCODE_FLOAT(bias.y, 3),
              CAL_ENCODE_FLOAT(bias.z, 3), CAL_ENCODE_FLOAT(radius, 3),
              CAL_ENCODE_FLOAT(moc->diversity_checker.diversity_dbg.var_log, 6),
              CAL_ENCODE_FLOAT(moc->diversity_checker.diversity_dbg.mean_log,
                               3),
              CAL_ENCODE_FLOAT(moc->diversity_checker.diversity_dbg.max_log, 3),
              CAL_ENCODE_FLOAT(moc->diversity_checker.diversity_dbg.min_log, 3),
              moc->diversity_checker.num_points,
              CAL_ENCODE_FLOAT(moc->diversity_checker.threshold, 3),
              CAL_ENCODE_FLOAT(moc->diversity_checker.max_distance, 3),
              moc->mag_dbg.mag_trigger_count,
              moc->mag_dbg.kasa_count,
              sample_time_us,
              CAL_ENCODE_FLOAT(moc->x_bias, 3),
              CAL_ENCODE_FLOAT(moc->y_bias, 3),
              CAL_ENCODE_FLOAT(moc->z_bias, 3),
              moc->update_time);
#endif  // MAG_CAL_DEBUG_ENABLE
          moc->x_bias = bias.x;
          moc->y_bias = bias.y;
          moc->z_bias = bias.z;

          moc->radius = radius;
          moc->update_time = sample_time_us;

          new_bias = UPDATE_BIAS;
        }
      }
    }
    // 5. reset for next batch
    magCalReset(moc);
  }

  return new_bias;
}

void magCalGetBias(const struct MagCal *moc, float *x, float *y, float *z) {
  *x = moc->x_bias;
  *y = moc->y_bias;
  *z = moc->z_bias;
}

void magCalAddBias(struct MagCal *moc, float x, float y, float z) {
  moc->x_bias += x;
  moc->y_bias += y;
  moc->z_bias += z;
}

void magCalRemoveBias(struct MagCal *moc, float xi, float yi, float zi,
                      float *xo, float *yo, float *zo) {
  *xo = xi - moc->x_bias;
  *yo = yi - moc->y_bias;
  *zo = zi - moc->z_bias;
}

void magCalSetSoftiron(struct MagCal *moc, float c00, float c01, float c02,
                       float c10, float c11, float c12, float c20, float c21,
                       float c22) {
  moc->c00 = c00;
  moc->c01 = c01;
  moc->c02 = c02;
  moc->c10 = c10;
  moc->c11 = c11;
  moc->c12 = c12;
  moc->c20 = c20;
  moc->c21 = c21;
  moc->c22 = c22;
}

void magCalRemoveSoftiron(struct MagCal *moc, float xi, float yi, float zi,
                          float *xo, float *yo, float *zo) {
  *xo = moc->c00 * xi + moc->c01 * yi + moc->c02 * zi;
  *yo = moc->c10 * xi + moc->c11 * yi + moc->c12 * zi;
  *zo = moc->c20 * xi + moc->c21 * yi + moc->c22 * zi;
}

#if defined MAG_CAL_DEBUG_ENABLE
// This function prints every second sample parts of the dbg diverse_data_log,
// which ensures that all the messages get printed into the log file.
void magLogPrint(struct DiversityChecker *diverse_data, float temp) {
  // Sample counter.
  static size_t sample_counter = 0;
  const float *data_log_ptr = &diverse_data->diversity_dbg.diverse_data_log[0];
  if (diverse_data->diversity_dbg.new_trigger == 1) {
    sample_counter++;
    if (sample_counter == 2) {
      CAL_DEBUG_LOG(
          "[MAG_CAL:MEMORY X] :", "%" PRIu32 ", " CAL_FORMAT_MAG_MEMORY ", "
          CAL_FORMAT_3DIGITS,
          diverse_data->diversity_dbg.diversity_count,
          CAL_ENCODE_FLOAT(data_log_ptr[0 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[1 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[2 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[3 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[4 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[5 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[6 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[7 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[8 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[9 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[10 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[11 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[12 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[13 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[14 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[15 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[16 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[17 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[18 * 3], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[19 * 3], 3), CAL_ENCODE_FLOAT(temp, 3));
    }

    if (sample_counter == 4) {
      CAL_DEBUG_LOG(
          "[MAG_CAL:MEMORY Y] :", "%" PRIu32 ", " CAL_FORMAT_MAG_MEMORY,
          diverse_data->diversity_dbg.diversity_count,
          CAL_ENCODE_FLOAT(data_log_ptr[0 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[1 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[2 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[3 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[4 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[5 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[6 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[7 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[8 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[9 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[10 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[11 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[12 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[13 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[14 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[15 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[16 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[17 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[18 * 3 + 1], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[19 * 3 + 1], 3));
    }
    if (sample_counter == 6) {
      CAL_DEBUG_LOG(
          "[MAG_CAL:MEMORY Z] :", "%" PRIu32 ", " CAL_FORMAT_MAG_MEMORY,
          diverse_data->diversity_dbg.diversity_count,
          CAL_ENCODE_FLOAT(data_log_ptr[0 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[1 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[2 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[3 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[4 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[5 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[6 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[7 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[8 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[9 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[10 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[11 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[12 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[13 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[14 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[15 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[16 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[17 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[18 * 3 + 2], 3),
          CAL_ENCODE_FLOAT(data_log_ptr[19 * 3 + 2], 3));
      sample_counter = 0;
      diverse_data->diversity_dbg.new_trigger = 0;
    }
  }
}
#endif  // MAG_CAL_DEBUG_ENABLE
