#ifndef _UTIL_SET_THREAD_PROP_H_
#define _UTIL_SET_THREAD_PROP_H_

#include <pthread.h>
#include "MTKUtilCommon.h"

/**
 * \details image convolution with given kernel
 * \fn UTIL_ERRCODE_ENUM Convolve(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* kernel, UTIL_BASE_IMAGE_STRUCT* C)
 * \param[in] A input image
 * \param[in] kernel kernel function
 * \param[out] C output image
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlSetpThreadProp(pthread_attr_t *pthreadAttr_ptr);

#endif /* _UTIL_CONVOLVE_H_ */

