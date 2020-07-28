#ifndef _UTIL_ROTATE_H_
#define _UTIL_ROTATE_H_

#include "MTKUtilCommon.h"

/// \details rotation of mirrorring angle
typedef enum UTIL_ANGLE
{
    UTIL_ANGLE_000,
    UTIL_ANGLE_090,
    UTIL_ANGLE_180,
    UTIL_ANGLE_270
} UTIL_ANGLE;

/**
 * \brief rotation function for Y
 * \details image can be rotated for 4 kind of angles, 0 deg can be used for \b crop
 * \fn UTIL_ERRCODE_ENUM utilRotate(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTIL_ANGLE angle)
 * \param[out] dst output image data
 * \param[in] src input image data
 * \param[in] angle rotation angle
 * \return utility error code
 */
UTIL_ERRCODE_ENUM utilRotate(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTIL_ANGLE angle);

/**
 * \brief mirror function for Y
 * \details image can be mirror for 4 kind of angles
 * \fn UTIL_ERRCODE_ENUM utilMirror(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTIL_ANGLE angle)
 * \param[out] dst output image data
 * \param[in] src input image data
 * \param[in] angle angle of mirrorring line
 * \return utility error code
 */
UTIL_ERRCODE_ENUM utilMirror(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTIL_ANGLE angle);

/**
 * \brief clip function for YUV400 or YUV420 format
 * \details image clipping by utilRotate by 0 degree
 * \fn UTIL_ERRCODE_ENUM utilImageClip(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTL_IMAGE_FORMAT_ENUM img_fmt)
 * \param[out] dst output image data
 * \param[in] src input image data
 * \param[in] img_fmt input image format
 * \return utility error code
 */
UTIL_ERRCODE_ENUM utilImageClip(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTL_IMAGE_FORMAT_ENUM img_fmt);

#endif /* _UTIL_ROTATE_H_ */

