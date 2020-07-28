/*
 * This module provides a data structure, initialization, and fit
 * routine for algorithms that use the Kasa method for determining the
 * 3-dimensional offset vector from a set of points on a sphere.
 *
 * Reference: I. Kåsa, "A circle fitting procedure and its error analysis," in
 * IEEE Transactions on Instrumentation and Measurement, vol. IM-25, no. 1, pp.
 * 8-14, March 1976.
 */

#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_COMMON_MATH_KASA_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_COMMON_MATH_KASA_H_

#include <stdbool.h>

#include "common/math/vec.h"

#ifdef __cplusplus
extern "C" {
#endif

struct KasaFit {
  float acc_x, acc_y, acc_z, acc_w;
  float acc_xx, acc_xy, acc_xz, acc_xw;
  float acc_yy, acc_yz, acc_yw, acc_zz, acc_zw;
  size_t nsamples;
};

// Resets the KasaFit data structure (sets all variables to zero).
void kasaReset(struct KasaFit *kasa);

// Initializes the KasaFit data structure.
void kasaInit(struct KasaFit *kasa);

// Accumulates the Kasa acc_** variables with the input vector [x, y, z], and
// updates the number of samples.
void kasaAccumulate(struct KasaFit *kasa, float x, float y, float z);

// Normalizes the Kasa acc_** variables. Returns 'false' if the number of
// samples is zero, otherwise 'true'.
bool kasaNormalize(struct KasaFit *kasa);

// Uses the Kasa sphere-fit method to extract a 'bias' estimate (centroid) for
// the best-fit sphere using the normal equations, and the sphere's 'radius'.
// Returns '1' if the radius of the fit sphere is within the bounds
// (min_fit, max_fit), otherwise '0'.
int kasaFit(struct KasaFit *kasa, struct Vec3 *bias, float *radius,
            float max_fit, float min_fit);

#ifdef __cplusplus
}
#endif

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_COMMON_MATH_KASA_H_
