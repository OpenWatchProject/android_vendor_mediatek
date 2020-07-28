#ifndef _UTIL_CONVOLVE_H_
#define _UTIL_CONVOLVE_H_

#include "MTKUtilCommon.h"

/// get element data from image/kernel
#define UTIL_ELM(A,i,j) ((MUINT8*)((A).data))[ (i)*(A).width + (j) ]

/**
 * \details image convolution with given kernel
 * \fn UTIL_ERRCODE_ENUM Convolve(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* kernel, UTIL_BASE_IMAGE_STRUCT* C)
 * \param[in] A input image
 * \param[in] kernel kernel function
 * \param[out] C output image
 * \return utility error code
 */
UTIL_ERRCODE_ENUM Convolve(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* kernel, UTIL_BASE_IMAGE_STRUCT* C);

#endif /* _UTIL_CONVOLVE_H_ */

