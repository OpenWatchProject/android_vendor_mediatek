#ifndef _UTIL_BLUR_H_
#define _UTIL_BLUR_H_

#include "MTKUtilCommon.h"

/**
 *  \details blur function with 5x5 gaussion kernel
 *  \fn UTIL_ERRCODE_ENUM utilBlur(MUINT8 *dst, P_UTIL_CLIP_IMAGE_STRUCT src)
 *  \param[out] dst destination image
 *  \param[in] src source image structure
 *  \return utility error code enumerator
 */
UTIL_ERRCODE_ENUM utilBlur(MUINT8 *dst, P_UTIL_CLIP_IMAGE_STRUCT src);

/**
 *  \details sobel filter
 *  \fn UTIL_ERRCODE_ENUM utilSobel(MUINT8 *dst, P_UTIL_CLIP_IMAGE_STRUCT src)
 *  \param[out] dst destination image
 *  \param[in] src source image structure
 *  \return utility error code enumerator
 */
UTIL_ERRCODE_ENUM utilSobel(MUINT32 *dst, P_UTIL_CLIP_IMAGE_STRUCT src);

#endif /* _UTIL_BLUR_H_ */

