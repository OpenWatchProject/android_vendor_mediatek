#ifndef _UTIL_RESIZE_BILINEAR_H_
#define _UTIL_RESIZE_BILINEAR_H_

#include "MTKUtilCommon.h"

/**
 * \details bilinear resizer for YUV400 or RGB565 format
 * \fn UTIL_ERRCODE_ENUM utilBilinearResizer(P_UTIL_BASE_IMAGE_STRUCT dst, P_UTIL_BASE_IMAGE_STRUCT src, UTL_IMAGE_FORMAT_ENUM ImgFmt)
 * \param[out] dst output image data
 * \param[in] src input image data
 * \param[in] ImgFmt input image format
 * \return utility error code
 */
UTIL_ERRCODE_ENUM utilBilinearResizer(P_UTIL_BASE_IMAGE_STRUCT dst, P_UTIL_BASE_IMAGE_STRUCT src, UTL_IMAGE_FORMAT_ENUM ImgFmt);

#endif /* _UTIL_RESIZE_BILINEAR_H_ */

