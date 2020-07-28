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
 * [Sample Rate Estimator]
 * This module periodically estimates the mean sampling rate of a uniformly
 * sampled signal. Note, this estimator uses a floating point accumulator for
 * the timing intervals. This trades-off some accumulation of finite precision
 * error to enhance the range of estimated sampling rates and prevent overflow
 * when an equivalent 32bit integer accumulator is used. In typical use cases
 * (sample rates: 5Hz - 2kHz, num_intervals_to_collect 10 - 100), the sample
 * rate accuracy is typically much better than 0.1Hz.
 *
 * FUNCTIONALITY:
 * sampleRateEstimatorInit -- Initializes the estimator. Sets the number of time
 *   intervals require to compute the sample rate mean, and the upper limit for
 *   the acceptable time interval.
 *
 * new_sampling_rate_estimate_ready -- Check this boolean flag if polling for
 *   new estimates is desired.
 *
 * sampleRateEstimatorGetHz -- Returns the latest sample rate estimate and
 *   resets the polling flag.
 *
 * sampleRateEstimatorUpdate -- Processes new timestamp data and computes new
 *   sample rate estimates.
 */

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_SAMPLE_RATE_ESTIMATOR_SAMPLE_RATE_ESTIMATOR_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_SAMPLE_RATE_ESTIMATOR_SAMPLE_RATE_ESTIMATOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Designates the value used to indicate an invalid sample rate estimate.
#define SAMPLE_RATE_ESTIMATOR_INVALID_SAMPLE_RATE_HZ (-1.0f)

// Sample rate estimator data structure.
struct SampleRateEstimator {
  // Used to compute sample intervals to estimate the sampling rate.
  uint64_t last_timestamp_nanos;

  // Accumulator used for computing the mean sampling interval.
  float interval_accumulator_nanos;

  // The upper limit on the acceptance of valid time intervals (in nanoseconds).
  // Larger time deltas result in a reset of the interval accumulator.
  float max_interval_nanos;

  // The most recent mean sampling rate estimate.
  float mean_sampling_rate_estimate_hz;

  /*
   * The targeted number of time intervals required for a new sample rate mean
   * calculation.
   *
   * NOTE: Assuming that the time interval noise is independent and identically
   * distributed and drawn from a zero-mean normal distribution with variance
   * 'Sigma^2'. The expected noise reduction is related to the choice of
   * 'num_intervals_to_collect' as:
   *
   *   Output RMS Noise = Sigma / sqrt(num_intervals_to_collect).
   *
   * Example, a value of 100 for a 100Hz signal would provide updates once every
   * second and provide a 90% reduction (i.e., [1 - 1/sqrt(100)] * 100%) in time
   * interval noise.
   */
  size_t num_intervals_to_collect;

  // The number of time intervals currently collected.
  size_t num_intervals_collected;

  // Polling flag used to signal that a new estimate is ready. This flag is
  // reset when 'sampleRateEstimatorGetHz' is called.
  bool new_sampling_rate_estimate_ready;
};

// Initializes the sample rate estimator, sets the number of time intervals
// for the mean sample rate calculation, and defines the tolerable gap in timing
// data (in seconds).
void sampleRateEstimatorInit(struct SampleRateEstimator* sample_rate_estimator,
                             size_t num_intervals_to_collect,
                             float max_interval_sec);

// Rather than using a function to poll for new sample rate estimates, which
// would incur an additional function call, simply check
// 'new_sampling_rate_estimate_ready' directly.

// Returns the most recent sampling rate estimate, and resets the
// 'new_sampling_rate_estimate_ready' polling flag. Note, if polling is not
// used, one may access the sample rate estimate directly from the struct and
// avoid this function call.
float sampleRateEstimatorGetHz(
    struct SampleRateEstimator* sample_rate_estimator);

// Takes in a new timestamp and updates the sampling rate estimator.
void sampleRateEstimatorUpdate(
    struct SampleRateEstimator* sample_rate_estimator,
    uint64_t timestamp_nanos);

#ifdef __cplusplus
}
#endif

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_CALIBRATION_SAMPLE_RATE_ESTIMATOR_SAMPLE_RATE_ESTIMATOR_H_
