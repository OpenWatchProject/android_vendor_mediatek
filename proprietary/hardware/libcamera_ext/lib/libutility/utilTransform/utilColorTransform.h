#ifndef _UTIL_COLOR_TRANSFORM_H_
#define _UTIL_COLOR_TRANSFORM_H_

#include "MTKUtilCommon.h"

/// \name YUV -> RGB888
/// @{
#define UTL_YUV_TO_RGB888_R(Y, U, V)    ((  ( Y<<10 ) + 1436 * ( V -128 ) + 512) >> 10)
#define UTL_YUV_TO_RGB888_G(Y, U, V)    ((  ( Y<<10 ) - 354 * ( U -128 ) - 732 * ( V -128 ) + 512) >> 10)
#define UTL_YUV_TO_RGB888_B(Y, U, V)    ((  ( Y<<10 ) + 1814 * ( U -128 ) + 512) >> 10)
/// @}

/// \name mask of RGB565
/// @{
#define UTL_RGB565_R_MASK (0xF800)
#define UTL_RGB565_G_MASK (0x07E0)
#define UTL_RGB565_B_MASK (0x001F)
/// @}

/// \name RGB565 -> RGB888
/// @{
#define UTL_RGB565_TO_RGB888_R(VALUE)   ((((VALUE) & UTL_RGB565_R_MASK)>>8))
#define UTL_RGB565_TO_RGB888_G(VALUE)   ((((VALUE) & UTL_RGB565_G_MASK)>>3))
#define UTL_RGB565_TO_RGB888_B(VALUE)   ((((VALUE) & UTL_RGB565_B_MASK)<<3))
/// @}

/// \name RGB888 -> RGB565
#define UTL_RGB888_TO_RGB565(R,G,B)     (((((R)+4) >> 3)<<11) | ((((G)+2)>>2)<<5) | (((B)+4)>>3))

/// \name RGB888 -> Y
#define UTL_RGB888_TO_YUV_Y(R, G, B)    ((  306 * (R) + 601 * (G) + 117 * (B) + 512) >> 10)

/// \name 8-bit data width clipping
/// @{
#define UTL_CLIP8(x)    ( ((x)>255) ? 255 : ( ((x)<0) ? 0 : (x)) )
/// @}

/**
 * \brief YV12 to NV21
 * \fn UTIL_ERRCODE_ENUM UtlYV12toNV21(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
 * \param[in] pImg input image structure
 * \param[in] pBuffer working buffer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlYV12toNV21(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer);

/**
 * \brief I420 to NV21
 * \fn UTIL_ERRCODE_ENUM UtlI420toNV21(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
 * \param[in] pImg input image structure
 * \param[in] pBuffer working buffer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlI420toNV21(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer);

/**
 * \brief NV21 to YV12
 * \fn UTIL_ERRCODE_ENUM UtlNV21toYV12(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
 * \param[in] pImg input image structure
 * \param[in] pBuffer working buffer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlNV21toYV12(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer);

/**
 * \brief NV21 to I420
 * \fn UTIL_ERRCODE_ENUM UtlNV21toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
 * \param[in] pImg input image structure
 * \param[in] pBuffer working buffer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlNV21toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer);

/**
 * \brief RGB888 to I420
 * \fn UTIL_ERRCODE_ENUM UtlRGB888toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlRGB888toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief RGB888A to I420
 * \fn UTIL_ERRCODE_ENUM UtlRGBA8888toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlRGBA8888toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief RGB888A to I420 (muilti-core version)
 * \fn UTIL_ERRCODE_ENUM UtlRGBA8888toI420(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlRGBA8888toI420(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief RGB888A to Grey
 * \fn UTIL_ERRCODE_ENUM UtlRGBA8888toGrey(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlRGBA8888toGrey(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief RGB888A to Grey (multi-core version)
 * \fn UTIL_ERRCODE_ENUM UtlRGBA8888toGrey(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlRGBA8888toGrey(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief I420 to RGB888
 * \fn UTIL_ERRCODE_ENUM UtlI420toRGB888(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlI420toRGB888(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief I420 to ARGB888
 * \fn UTIL_ERRCODE_ENUM UtlYV12toARGB888(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlI420toRGBA8888(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief I420 to ARGB888 (multi-core version)
 * \fn UTIL_ERRCODE_ENUM UtlYV12toARGB888(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst)
 * \param[in] pImg input image structure
 * \param[in] pDst output image data pointer
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlI420toRGBA8888(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst);

/**
 * \brief RGB565 to Gray image
 * \fn UTIL_ERRCODE_ENUM UtlRgb565ToGray(P_UTIL_BASE_IMAGE_STRUCT pImg)
 * \param[in] pImg input image structure
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlRgb565ToGray(P_UTIL_BASE_IMAGE_STRUCT pImg);

/**
 * \brief YUYV to I420
 * \fn UtlYUYVtoI420(P_UTIL_BASE_IMAGE_STRUCT pDstImg, P_UTIL_BASE_IMAGE_STRUCT pSrcImg)
 * \param[in] pDst output image data pointer
 * \param[in] pSrc input image structure
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlYUYVtoI420(P_UTIL_BASE_IMAGE_STRUCT pDst, P_UTIL_BASE_IMAGE_STRUCT pSrc);

/**
 * \brief I420 to YUYV
 * \fn UtlI420toYUYV(P_UTIL_BASE_IMAGE_STRUCT pDstImg, P_UTIL_BASE_IMAGE_STRUCT pSrcImg)
 * \param[in] pDst output image data pointer
 * \param[in] pSrc input image structure
 * \return utility error code
 */
UTIL_ERRCODE_ENUM UtlI420toYUYV(P_UTIL_BASE_IMAGE_STRUCT pDst, P_UTIL_BASE_IMAGE_STRUCT pSrc);

#endif /* _UTIL_COLOR_TRANSFORM_H_ */

