#define LOG_TAG "utilResize"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif
#include "utilResize.h"
#include "utilColorTransform.h"

UTIL_ERRCODE_ENUM utilBilinearResizer(P_UTIL_BASE_IMAGE_STRUCT dst, P_UTIL_BASE_IMAGE_STRUCT src, UTL_IMAGE_FORMAT_ENUM ImgFmt)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    const MUINT32 srcPitch = src->width;
    const MUINT32 srcStepX = UTL_IUL_I_TO_X(src->width) / dst->width;
    const MUINT32 srcStepY = UTL_IUL_I_TO_X(src->height) / dst->height;
    const MUINT32 img_w = dst->width;

    size_t src_addr = (size_t)src->data;
    size_t dst_addr = (size_t)dst->data;
    MUINT32 srcCoordY = 0;
    MINT32 h = dst->height;
    MINT32 srcHeight = src->height;

    // error check
    if (!dst_addr || !src_addr)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    if (ImgFmt == UTL_IMAGE_FORMAT_YUV400)
    {
        MUINT8 *src_buffer = (MUINT8 *)src_addr;
        MUINT8 *dstAddr = (MUINT8 *)dst_addr;
        while (--h >= 0)
        {
            MINT32 w = img_w;
            MUINT32 srcCoordX = 0;

            MINT32 srcOffset_1;
            MINT32 srcOffset_2;
            MUINT8 *src_ptr_1;
            MUINT8 *src_ptr_2;

            MINT32 y_carry = UTL_IUL_X_TO_I_CARRY(srcCoordY);
            MINT32 y_chop  = UTL_IUL_X_TO_I_CHOP(srcCoordY);

            if ((y_carry < 0) || (y_carry >= srcHeight))
            {
                result = UTIL_COMMON_ERR_INVALID_PARAMETER;
                LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
                return result;
            }
            if ((y_chop < 0) || (y_chop >= srcHeight))
            {
                result = UTIL_COMMON_ERR_INVALID_PARAMETER;
                LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
                return result;
            }

            srcOffset_1 = y_chop * srcPitch;
            srcOffset_2 = y_carry * srcPitch;
            src_ptr_1 = src_buffer + srcOffset_1;
            src_ptr_2 = src_buffer + srcOffset_2;

            while (--w >= 0)
            {
                MUINT8 pixel_1, pixel_2;
                MINT32 y, y1;

                MINT32 x_carry = UTL_IUL_X_TO_I_CARRY(srcCoordX);
                MINT32 x_chop = UTL_IUL_X_TO_I_CHOP(srcCoordX);

                MINT32 weighting2;

                weighting2 = UTL_IUL_X_FRACTION(srcCoordX);

                // 1st horizontal interpolation.
                pixel_1 = *(src_ptr_1 + x_chop);
                pixel_2 = *(src_ptr_1 + x_carry);
                y = LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

                // 2nd horizontal interpolation.
                pixel_1 = *(src_ptr_2 + x_chop);
                pixel_2 = *(src_ptr_2 + x_carry);
                y1 = LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

                // Vertical interpolation.
                weighting2 = UTL_IUL_X_FRACTION(srcCoordY);

                y = LINEAR_INTERPOLATION(y, y1, weighting2);

                *dstAddr++ = (MUINT8)y;

                srcCoordX += srcStepX;
            }
            srcCoordY += srcStepY;
        }
    }
    else if (ImgFmt == UTL_IMAGE_FORMAT_RGB565)
    {
        MUINT16 *src_buffer = (MUINT16 *)src_addr;
        MUINT16 *dstAddr = (MUINT16 *)dst_addr;
        while (--h >= 0)
        {
            MINT32 w = img_w;
            MUINT32 srcCoordX = 0;

            MINT32 srcOffset_1;
            MINT32 srcOffset_2;
            MUINT16 *src_ptr_1;
            MUINT16 *src_ptr_2;

            MINT32 y_carry = UTL_IUL_X_TO_I_CARRY(srcCoordY);
            MINT32 y_chop  = UTL_IUL_X_TO_I_CHOP(srcCoordY);

            srcOffset_1 = y_chop * srcPitch;
            srcOffset_2 = y_carry * srcPitch;
            src_ptr_1 = src_buffer + srcOffset_1;
            src_ptr_2 = src_buffer + srcOffset_2;

            while (--w >= 0)
            {
                MUINT16 pixel_1, pixel_2;
                MINT32 r, g, b;
                MINT32 r1, g1, b1;
                MINT32 r2, g2, b2;

                MINT32 x_carry = UTL_IUL_X_TO_I_CARRY(srcCoordX);
                MINT32 x_chop = UTL_IUL_X_TO_I_CHOP(srcCoordX);

                MINT32 weighting2;

                weighting2 = UTL_IUL_X_FRACTION(srcCoordX);

                /// 1st horizontal interpolation.
                pixel_1 = *(src_ptr_1 + x_chop);
                pixel_2 = *(src_ptr_1 + x_carry);
                r1 = UTL_RGB565_TO_RGB888_R(pixel_1);
                g1 = UTL_RGB565_TO_RGB888_G(pixel_1);
                b1 = UTL_RGB565_TO_RGB888_B(pixel_1);
                r2 = UTL_RGB565_TO_RGB888_R(pixel_2);
                g2 = UTL_RGB565_TO_RGB888_G(pixel_2);
                b2 = UTL_RGB565_TO_RGB888_B(pixel_2);
                r = LINEAR_INTERPOLATION(r1, r2, weighting2);
                g = LINEAR_INTERPOLATION(g1, g2, weighting2);
                b = LINEAR_INTERPOLATION(b1, b2, weighting2);

                /// 2nd horizontal interpolation.
                pixel_1 = *(src_ptr_2 + x_chop);
                pixel_2 = *(src_ptr_2 + x_carry);
                r1 = UTL_RGB565_TO_RGB888_R(pixel_1);
                g1 = UTL_RGB565_TO_RGB888_G(pixel_1);
                b1 = UTL_RGB565_TO_RGB888_B(pixel_1);
                r2 = UTL_RGB565_TO_RGB888_R(pixel_2);
                g2 = UTL_RGB565_TO_RGB888_G(pixel_2);
                b2 = UTL_RGB565_TO_RGB888_B(pixel_2);
                r1 = LINEAR_INTERPOLATION(r1, r2, weighting2);
                g1 = LINEAR_INTERPOLATION(g1, g2, weighting2);
                b1 = LINEAR_INTERPOLATION(b1, b2, weighting2);

                /// Vertical interpolation.
                weighting2 = UTL_IUL_X_FRACTION(srcCoordY);

                r = LINEAR_INTERPOLATION(r, r1, weighting2);
                g = LINEAR_INTERPOLATION(g, g1, weighting2);
                b = LINEAR_INTERPOLATION(b, b1, weighting2);

                *dstAddr++ = UTL_RGB888_TO_RGB565(r, g, b);

                srcCoordX += srcStepX;
            }
            srcCoordY += srcStepY;
        }
    }
    else
    {
        result = UTIL_COMMON_ERR_UNSUPPORTED_IMAGE_FORMAT;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    return result;
}
