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

#include "calibration/sample_rate_estimator/sample_rate_estimator.h"

#include <string.h>

#include "common/math/macros.h"
#include "calibration/util/cal_log.h"
#include "util/nano_assert.h"

// Helper function used to reset the sampling rate estimator accumulator.
static void sampleRateEstimatorResetAccumulator(
    struct SampleRateEstimator* sample_rate_estimator) {
  sample_rate_estimator->last_timestamp_nanos = 0.0f;
  sample_rate_estimator->interval_accumulator_nanos = 0.0f;
  sample_rate_estimator->num_intervals_collected = 0;
}

void sampleRateEstimatorInit(struct SampleRateEstimator* sample_rate_estimator,
                             size_t num_intervals_to_collect,
                             float max_interval_sec) {
  ASSERT_NOT_NULL(sample_rate_estimator);
  memset(sample_rate_estimator, 0, sizeof(struct SampleRateEstimator));
  sample_rate_estimator->mean_sampling_rate_estimate_hz =
      SAMPLE_RATE_ESTIMATOR_INVALID_SAMPLE_RATE_HZ;
  sample_rate_estimator->num_intervals_to_collect = num_intervals_to_collect;
  sample_rate_estimator->max_interval_nanos =
      max_interval_sec * SEC_TO_NANOS(1);
}

float sampleRateEstimatorGetHz(
    struct SampleRateEstimator* sample_rate_estimator) {
  sample_rate_estimator->new_sampling_rate_estimate_ready = false;
  return sample_rate_estimator->mean_sampling_rate_estimate_hz;
}

void sampleRateEstimatorUpdate(
    struct SampleRateEstimator* sample_rate_estimator,
    uint64_t timestamp_nanos) {
  // Resets the current interval capture and returns if:
  //   1. A bad timestamp was received (i.e., time not monotonic).
  //   2. 'last_timestamp_nanos' is zero. NOTE: 'last_timestamp_nanos' = 0
  //      indicates that the first complete time interval has not been captured
  //      yet (so, set it and return).
  if (timestamp_nanos <= sample_rate_estimator->last_timestamp_nanos ||
      sample_rate_estimator->last_timestamp_nanos == 0) {
    sample_rate_estimator->last_timestamp_nanos = timestamp_nanos;
    return;
  }

  // Computes the current sampling interval. This conversion will be very fast
  // for intervals less than ~4.3 seconds (i.e., 2^32 nano-seconds).
  const float next_interval_nanos = floatFromUint64(
      timestamp_nanos - sample_rate_estimator->last_timestamp_nanos);

  // Helps prevent corruption of the estimator when there are gaps in the input
  // sampling intervals greater than 'max_interval_nanos' (i.e., intermittant
  // periods where there are no input timestamps).
  if (next_interval_nanos >= sample_rate_estimator->max_interval_nanos) {
    // Resets the estimator and returns.
    sampleRateEstimatorResetAccumulator(sample_rate_estimator);
    return;
  }

  // Accumulates the next sampling interval.
  sample_rate_estimator->interval_accumulator_nanos += next_interval_nanos;
  sample_rate_estimator->last_timestamp_nanos = timestamp_nanos;
  sample_rate_estimator->num_intervals_collected++;

  // If the number of collected time intervals exceed the target number, then
  // this computes a new sample rate estimate.
  if (sample_rate_estimator->num_intervals_collected >
      sample_rate_estimator->num_intervals_to_collect) {
    sample_rate_estimator->mean_sampling_rate_estimate_hz =
        sample_rate_estimator->num_intervals_collected *
        (SEC_TO_NANOS(1) / sample_rate_estimator->interval_accumulator_nanos);

    // Sets the polling flag to indicate that a new estimate is ready.
    sample_rate_estimator->new_sampling_rate_estimate_ready = true;

    // Resets the estimator variables.
    sampleRateEstimatorResetAccumulator(sample_rate_estimator);
  }
}
