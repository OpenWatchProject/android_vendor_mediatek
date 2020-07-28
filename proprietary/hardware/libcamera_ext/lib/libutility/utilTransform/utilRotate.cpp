#define LOG_TAG "utilRotate"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif
#include <string.h>
#include "utilRotate.h"

UTIL_ERRCODE_ENUM utilRotate(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTIL_ANGLE angle)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *srcAddr = (MUINT8 *)src->data;
    MUINT8 *dstAddr = (MUINT8 *)dst->data;
    MUINT8 *srcRowAddr, *dstRowAddr, *dstColumnAddr;
    MINT32 srcWidth  = src->clip_width;
    MINT32 srcHeight = src->clip_height;
    MINT32 srcPitch  = src->width;
    MINT32 dstWidth  = dst->clip_width;
    MINT32 dstHeight = dst->clip_height;
    MINT32 dstPitch  = dst->width;
    MINT32 srcX, srcY;
    MINT32 minusDstPitch = -dstPitch;

    // error check
    if ((srcAddr == NULL) || (dstAddr == NULL))
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // rotate with crop
    switch(angle)
    {
    case UTIL_ANGLE_000:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));

            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                memcpy(dstAddr, srcAddr, dstWidth);
                srcAddr += srcPitch;
                dstAddr += dstPitch;
            }
        }
        break;
    case UTIL_ANGLE_090:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));
            srcRowAddr = srcAddr;
            dstColumnAddr = dstAddr + dstWidth  - 1 ;

            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                srcAddr = srcRowAddr;
                dstAddr = dstColumnAddr;
                srcX = srcWidth;

                while (--srcX >= 0)
                {
                    *dstAddr = *srcAddr;
                    srcAddr++;
                    dstAddr += dstPitch;
                }

                srcRowAddr += srcPitch;
                dstColumnAddr--;
            }
        }
        break;
    case UTIL_ANGLE_180:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));
            srcRowAddr = srcAddr;
            dstRowAddr = dstAddr + (dstPitch * (dstHeight - 1)) + (dstWidth - 1) ;

            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                srcAddr = srcRowAddr;
                dstAddr = dstRowAddr;

                srcX = srcWidth;
                while (--srcX >= 0)
                {
                    *dstAddr = *srcAddr;
                    srcAddr++;
                    dstAddr--;
                }

                srcRowAddr += srcPitch;
                dstRowAddr -= dstPitch;
            }
        }
        break;
    case UTIL_ANGLE_270:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));
            srcRowAddr = srcAddr;
            dstColumnAddr = dstAddr + (dstPitch * (dstHeight - 1));

            // using minusDstPitch, not dstPitch => speed up 10% in armulator
            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                srcAddr = srcRowAddr;
                dstAddr = dstColumnAddr;

                srcX = srcWidth;
                while (--srcX >= 0)
                {
                    *dstAddr = *srcAddr;
                    srcAddr++;
                    dstAddr += minusDstPitch;
                }

                srcRowAddr += srcPitch;
                dstColumnAddr++;
            }
        }
        break;
    default:
        result = UTIL_COMMON_ERR_INVALID_PARAMETER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
        break;
    }

    return result;
}

UTIL_ERRCODE_ENUM utilMirror(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTIL_ANGLE angle)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *srcAddr = (MUINT8 *)src->data;
    MUINT8 *dstAddr = (MUINT8 *)dst->data;
    MUINT8 *srcRowAddr, *dstRowAddr, *dstColumnAddr;
    MINT32 srcWidth  = src->clip_width;
    MINT32 srcHeight = src->clip_height;
    MINT32 srcPitch  = src->width;
    MINT32 dstWidth  = dst->clip_width;
    MINT32 dstHeight = dst->clip_height;
    MINT32 dstPitch  = dst->width;
    MINT32 srcX, srcY;
    MINT32 minusDstPitch = -dstPitch;

    // error check
    if ((srcAddr == NULL) || (dstAddr == NULL))
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // rotate with crop
    switch(angle)
    {
    case UTIL_ANGLE_000:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));
            srcRowAddr = srcAddr;
            dstRowAddr = dstAddr + dstWidth - 1 ;

            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                srcAddr = srcRowAddr;
                dstAddr = dstRowAddr;
                srcX = srcWidth;

                while (--srcX >= 0)
                {
                    *dstAddr = *srcAddr;
                    srcAddr++;
                    dstAddr--;
                }

                srcRowAddr += srcPitch;
                dstRowAddr += dstPitch;
            }
        }
        break;
    case UTIL_ANGLE_090:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));
            srcRowAddr = srcAddr;
            dstColumnAddr = dstAddr;

            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                srcAddr = srcRowAddr;           // set to the start of row
                dstAddr = dstColumnAddr;        // set to the start of column
                srcX = srcWidth;                // set loop time

                while (--srcX >= 0)
                {
                    *dstAddr = *srcAddr;
                    srcAddr++;
                    dstAddr += dstPitch;
                }

                srcRowAddr += srcPitch;         // read from top to bottom
                dstColumnAddr++;                // write from right to left
            }
        }
        break;
    case UTIL_ANGLE_180:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y + dstHeight - 1) * (dstPitch));

            srcY = srcHeight;
            while (--srcY >= 0)
            {
                memcpy(dstAddr, srcAddr, dstWidth);
                srcAddr += srcPitch;
                dstAddr -= dstPitch;
            }
        }
        break;
    case UTIL_ANGLE_270:
        {
            // image pointer
            srcAddr += (src->clip_x) + ((src->clip_y) * (srcPitch));
            dstAddr += (dst->clip_x) + ((dst->clip_y) * (dstPitch));
            srcRowAddr = srcAddr;
            dstColumnAddr = dstAddr + (dstPitch * (dstHeight - 1)) + (dstWidth - 1) ;

            // using minusDstPitch, not dstPitch => speed up 10% in armulator
            // image copy
            srcY = srcHeight;
            while (--srcY >= 0)
            {
                srcAddr = srcRowAddr;
                dstAddr = dstColumnAddr;

                srcX = srcWidth;

                while (--srcX >= 0)
                {
                    *dstAddr = *srcAddr;
                    srcAddr++;
                    dstAddr += minusDstPitch;
                }

                srcRowAddr += srcPitch;
                dstColumnAddr--;
            }
        }
        break;
    default:
        result = UTIL_COMMON_ERR_INVALID_PARAMETER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
        break;
    }

    return result;
}

UTIL_ERRCODE_ENUM utilImageClip(P_UTIL_CLIP_IMAGE_STRUCT dst, P_UTIL_CLIP_IMAGE_STRUCT src, UTL_IMAGE_FORMAT_ENUM img_fmt)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;

    if (img_fmt == UTL_IMAGE_FORMAT_YUV400)
    {
        result = utilRotate(dst, src, UTIL_ANGLE_000);
    }
    else if (img_fmt == UTL_IMAGE_FORMAT_YUV420)
    {
        UTIL_CLIP_IMAGE_STRUCT dst_uv, src_uv;

        // y data
        result = utilRotate(dst, src, UTIL_ANGLE_000);

        // u data
        if (result == UTIL_OK)
        {
            dst_uv.data = (MUINT8 *)(dst->data) + (dst->width * dst->height);
            dst_uv.width = dst->width >> 1;
            dst_uv.height = dst->height >> 1;
            dst_uv.clip_x = dst->clip_x >> 1;
            dst_uv.clip_y = dst->clip_y >> 1;
            dst_uv.clip_width = dst->clip_width >> 1;
            dst_uv.clip_height = dst->clip_height >> 1;
            src_uv.data = (MUINT8 *)(src->data) + (src->width * src->height);
            src_uv.width = src->width >> 1;
            src_uv.height = src->height >> 1;
            src_uv.clip_x = src->clip_x >> 1;
            src_uv.clip_y = src->clip_y >> 1;
            src_uv.clip_width = src->clip_width >> 1;
            src_uv.clip_height = src->clip_height >> 1;
            result = utilRotate(&dst_uv, &src_uv, UTIL_ANGLE_000);
        }

        // v data
        if (result == UTIL_OK)
        {
            dst_uv.data = (MUINT8 *)(dst->data) + (dst->width * dst->height) + ((dst->width * dst->height)>>2);
            src_uv.data = (MUINT8 *)(src->data) + (src->width * src->height) + ((src->width * src->height)>>2);
            result = utilRotate(&dst_uv, &src_uv, UTIL_ANGLE_000);
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