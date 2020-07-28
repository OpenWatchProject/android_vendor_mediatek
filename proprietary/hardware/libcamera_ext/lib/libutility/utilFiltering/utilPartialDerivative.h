#ifndef _UTIL_PARTIAL_DERIVATIVE_H_
#define _UTIL_PARTIAL_DERIVATIVE_H_

#include "MTKUtilCommon.h"

/// step of partial derivative function
#define PARTIAL_DER_STEP    (1)

/**
 *  \details Partial derivative computation
 *  \fn UTIL_ERRCODE_ENUM utilPartialDerivative(MINT8 *dst_x, MINT8 *dst_y, P_UTIL_BASE_IMAGE_STRUCT src)
 *  \param[out] dst_x horizontal gradient image
 *  \param[out] dst_y vertical gradient image
 *  \param[in] src source image structure
 *  \return utility error code enumerator
 */
UTIL_ERRCODE_ENUM utilPartialDerivative(MINT8 *dst_x, MINT8 *dst_y, P_UTIL_BASE_IMAGE_STRUCT src);

#endif /* _UTIL_PARTIAL_DERIVATIVE_H_ */

