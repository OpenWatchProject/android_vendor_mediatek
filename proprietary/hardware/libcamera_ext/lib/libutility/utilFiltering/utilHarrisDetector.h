#ifndef _UTIL_HARRIS_DETECTOR_H_
#define _UTIL_HARRIS_DETECTOR_H_

#include "MTKUtilCommon.h"

#define denom                   (41943)                     ///< 20-bit is used
#define KAPPA                   (3)                         ///< Harris kappa
#define FE_HARRIS_KAPPA_BITS    (5)                         ///< FE Harris Kappa bits
#define HARRIS_AVG_BITS         (20)                        ///< Harris average bits

/**
 *  \details Harris corner detection
 *  \fn UTIL_ERRCODE_ENUM utilHarrisDetector(P_UTIL_CLIP_IMAGE_STRUCT dst, MINT8* src_x, MINT8* src_y, MINT32 *range)
 *  \param[out] dst destination image structure
 *  \param[in] src_x horizontal gradient image
 *  \param[in] src_y vertical gradient image
 *  \param[in,out] range variable window range
 *  \return utility error code enumerator
 */
UTIL_ERRCODE_ENUM utilHarrisDetector(P_UTIL_CLIP_IMAGE_STRUCT dst, MINT8* src_x, MINT8* src_y, MINT32 *range);

#endif /* _UTIL_HARRIS_DETECTOR_H_ */

