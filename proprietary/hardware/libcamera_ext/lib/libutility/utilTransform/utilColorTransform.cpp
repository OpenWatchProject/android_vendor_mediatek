#define LOG_TAG "utilColorTransform"

#include <stdio.h>
#include <string.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif
#include "utilColorTransform.h"

#define NEON_OPT
#ifdef NEON_OPT
#if defined(_WIN32) && defined(_MSC_VER)
#include <neon_template.hpp>
#else
#include <arm_neon.h>
#endif
#endif /* NEON_OPT */

UTIL_ERRCODE_ENUM UtlYV12toNV21(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT32 v_offset = pImg->width*pImg->height;
    MUINT32 uv_size = v_offset>>2;
    MUINT32 u_offset = v_offset + uv_size;
    MUINT8 *src_v = (MUINT8*)(pImg->data) + v_offset;
    MUINT8 *src_u = (MUINT8*)(pImg->data) + u_offset;
    MUINT8 *buffer_uv = (MUINT8*)pBuffer;

    // data pointer check
    if (!pImg->data || !pBuffer)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // convert format and store in buffer
    for(MINT32 i=uv_size; i>0; i--)
    {
        *buffer_uv++ = *src_v++;
        *buffer_uv++ = *src_u++;
    }

    // copy from buffer to source
    memcpy( (MUINT8*)(pImg->data) + v_offset, pBuffer, uv_size*2);

    return result;
}

UTIL_ERRCODE_ENUM UtlI420toNV21(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT32 u_offset = pImg->width*pImg->height;
    MUINT32 uv_size = u_offset>>2;
    MUINT32 v_offset = u_offset + uv_size;
    MUINT8 *src_u = (MUINT8*)(pImg->data) + u_offset;
    MUINT8 *src_v = (MUINT8*)(pImg->data) + v_offset;
    MUINT8 *buffer_uv = (MUINT8*)pBuffer;

    // data pointer check
    if (!pImg->data || !pBuffer)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // convert format and store in buffer
    for(MINT32 i=uv_size; i>0; i--)
    {
        *buffer_uv++ = *src_v++;
        *buffer_uv++ = *src_u++;
    }

    // copy from buffer to source
    memcpy( (MUINT8*)(pImg->data) + u_offset, pBuffer, uv_size*2);

    return result;
}

UTIL_ERRCODE_ENUM UtlNV21toYV12(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT32 uv_offset = pImg->width*pImg->height;
    MUINT32 uv_size = uv_offset>>2;
    MUINT8 *src_uv = (MUINT8 *)(pImg->data) + uv_offset;
    MUINT8 *buffer_v = (MUINT8 *)pBuffer;
    MUINT8 *buffer_u = buffer_v + uv_size;

    // data pointer check
    if (!pImg->data || !pBuffer)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // convert format and store in buffer
    for(MINT32 i=uv_size; i>0; i--)
    {
        *buffer_v++ = *src_uv++;
        *buffer_u++ = *src_uv++;
    }

    // copy from buffer to source
    memcpy((MUINT8 *)(pImg->data) + uv_offset, pBuffer, uv_size*2);

    return result;
}

UTIL_ERRCODE_ENUM UtlNV21toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pBuffer)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT32 uv_offset = pImg->width*pImg->height;
    MUINT32 uv_size = uv_offset>>2;
    MUINT8 *src_uv = (MUINT8 *)(pImg->data) + uv_offset;
    MUINT8 *buffer_u = (MUINT8 *)pBuffer;
    MUINT8 *buffer_v = buffer_u + uv_size;

    // data pointer check
    if (!pImg->data || !pBuffer)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // convert format and store in buffer
    for(MINT32 i=uv_size; i>0; i--)
    {
        *buffer_v++ = *src_uv++;
        *buffer_u++ = *src_uv++;
    }

    // copy from buffer to source
    memcpy((MUINT8 *)(pImg->data) + uv_offset, pBuffer, uv_size*2);

    return result;
}
/*
 * YCbCr in Rec. 601 format
 * RGB values are in the range [0..255]
 *
 * [ Y  ]   [  16 ]    1    [  66   129    25 ]   [ R ]
 * [ Cb ] = [ 128 ] + --- * [ -38   -74   112 ] * [ G ]
 * [ Cr ]   [ 128 ]   256   [ 112   -94   -18 ]   [ B ]
 */
UTIL_ERRCODE_ENUM UtlRGB888toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_rgb = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MUINT8 *data_yuv = (MUINT8 *)pDst;

    MINT32 x, y, row_stride = width * 3;
    MUINT8 *rgb, *Y, *U, *V;

    MUINT8 u00, u01, u10, u11;
    MUINT8 v00, v01, v10, v11;

    MINT32 RtoYCoeff = (MINT32) ( 66 * 256);
    MINT32 GtoYCoeff = (MINT32) (129 * 256);
    MINT32 BtoYCoeff = (MINT32) ( 25 * 256);

    MINT32 RtoUCoeff = (MINT32) (-38 * 256);
    MINT32 GtoUCoeff = (MINT32) (-74 * 256);
    MINT32 BtoUCoeff = (MINT32) (112 * 256);

    MINT32 RtoVCoeff = (MINT32) (112 * 256);
    MINT32 GtoVCoeff = (MINT32) (-94 * 256);
    MINT32 BtoVCoeff = (MINT32) (-18 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* Y plane */
    rgb = data_rgb;
    Y   = data_yuv;

    for (y = height; y-- > 0; )
    {
        for (x = width; x-- > 0; )
        {
            /* No need to saturate between 16 and 235 */
            *Y = 16 + ((32768 +
                        RtoYCoeff * *(rgb) +
                        GtoYCoeff * *(rgb + 1) +
                        BtoYCoeff * *(rgb + 2)) >> 16);
            Y++;
            rgb += 3;
        }
    }

    /* U and V planes */
    rgb = data_rgb;
    U   = data_yuv + (width*height);
    V   = data_yuv + (width*height + ((width*height)/4));

    for (y = height / 2; y-- > 0; )
    {
        for (x = width / 2; x-- > 0; )
        {
            /* No need to saturate between 16 and 240 */
            u00 = 128 + ((32768 +
                        RtoUCoeff * *(rgb) +
                        GtoUCoeff * *(rgb + 1) +
                        BtoUCoeff * *(rgb + 2)) >> 16);
            u01 = 128 + ((32768 +
                        RtoUCoeff * *(rgb + 3) +
                        GtoUCoeff * *(rgb + 4) +
                        BtoUCoeff * *(rgb + 5)) >> 16);
            u10 = 128 + ((32768 +
                        RtoUCoeff * *(rgb + row_stride) +
                        GtoUCoeff * *(rgb + row_stride + 1) +
                        BtoUCoeff * *(rgb + row_stride + 2)) >> 16);
            u11 = 128 + ((32768 +
                        RtoUCoeff * *(rgb + row_stride + 3) +
                        GtoUCoeff * *(rgb + row_stride + 4) +
                        BtoUCoeff * *(rgb + row_stride + 5)) >> 16);
            *U++ = (2 + u00 + u01 + u10 + u11) >> 2;

            v00 = 128 + ((32768 +
                        RtoVCoeff * *(rgb) +
                        GtoVCoeff * *(rgb + 1) +
                        BtoVCoeff * *(rgb + 2)) >> 16);
            v01 = 128 + ((32768 +
                        RtoVCoeff * *(rgb + 3) +
                        GtoVCoeff * *(rgb + 4) +
                        BtoVCoeff * *(rgb + 5)) >> 16);
            v10 = 128 + ((32768 +
                        RtoVCoeff * *(rgb + row_stride) +
                        GtoVCoeff * *(rgb + row_stride + 1) +
                        BtoVCoeff * *(rgb + row_stride + 2)) >> 16);
            v11 = 128 + ((32768 +
                        RtoVCoeff * *(rgb + row_stride + 3) +
                        GtoVCoeff * *(rgb + row_stride + 4) +
                        BtoVCoeff * *(rgb + row_stride + 5)) >> 16);
            *V++ = (2 + v00 + v01 + v10 + v11) >> 2;

            rgb += 6;
        }

        rgb += row_stride;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlRGBA8888toI420(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_rgba = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MUINT8 *data_yuv = (MUINT8 *)pDst;

    MINT32 x, y, row_stride = width * 4;
    MUINT8 *rgba, *Y, *U, *V;

    MUINT8 u00, u01, u10, u11;
    MUINT8 v00, v01, v10, v11;

    MINT32 RtoYCoeff = (MINT32) ( 66 * 256);
    MINT32 GtoYCoeff = (MINT32) (129 * 256);
    MINT32 BtoYCoeff = (MINT32) ( 25 * 256);

    MINT32 RtoUCoeff = (MINT32) (-38 * 256);
    MINT32 GtoUCoeff = (MINT32) (-74 * 256);
    MINT32 BtoUCoeff = (MINT32) (112 * 256);

    MINT32 RtoVCoeff = (MINT32) (112 * 256);
    MINT32 GtoVCoeff = (MINT32) (-94 * 256);
    MINT32 BtoVCoeff = (MINT32) (-18 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* Y plane */
    rgba = data_rgba;
    Y   = data_yuv;

    MINT32 width8 = 0;
#ifdef NEON_OPT
    width8 = (width & 0x7) ? (((width - 8)>>3)<<3) : width;
    int32x4_t v_RtoYCoeff = vdupq_n_s32(RtoYCoeff);
    int32x4_t v_GtoYCoeff = vdupq_n_s32(GtoYCoeff);
    int32x4_t v_BtoYCoeff = vdupq_n_s32(BtoYCoeff);
    uint8x8_t v_u16 = vdup_n_u8(16);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width8; x+=8)
        {
            // read rgba data
            uint8x8x4_t v_rgba = vld4_u8(rgba);
            int32x4_t v_r[2], v_g[2], v_b[2];
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            v_temp[0] = vmulq_s32(v_RtoYCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoYCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoYCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoYCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoYCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoYCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            uint8x8_t v_Y = vreinterpret_u8_s8(vmovn_s16(vcombine_s16(v_result[0], v_result[1])));
            v_Y = vadd_u8(v_Y, v_u16);
            vst1_u8(Y, v_Y);

            // increment
            Y += 8;
            rgba += 32;
        }
    }
#endif

    rgba = data_rgba;
    Y   = data_yuv;
    for (y = 0; y < height; y++)
    {
        Y += width8;
        rgba += width8*4;
        for (x = width8; x < width; x++)
        {
            /* No need to saturate between 16 and 235 */
            *Y = 16 + ((32768 +
                        RtoYCoeff * *(rgba) +
                        GtoYCoeff * *(rgba + 1) +
                        BtoYCoeff * *(rgba + 2)) >> 16);
            Y++;
            rgba += 4;
        }
    }

    /* U and V planes */
    rgba = data_rgba;
    U   = data_yuv + (width*height);
    V   = data_yuv + (width*height + ((width*height)/4));

    MINT32 width16 = 0;
#ifdef NEON_OPT
    width16 = (width & 0xF) ? (((width-16)>>4)<<4) : width;
    int32x4_t v_RtoUCoeff = vdupq_n_s32(RtoUCoeff);
    int32x4_t v_GtoUCoeff = vdupq_n_s32(GtoUCoeff);
    int32x4_t v_BtoUCoeff = vdupq_n_s32(BtoUCoeff);
    int32x4_t v_RtoVCoeff = vdupq_n_s32(RtoVCoeff);
    int32x4_t v_GtoVCoeff = vdupq_n_s32(GtoVCoeff);
    int32x4_t v_BtoVCoeff = vdupq_n_s32(BtoVCoeff);
    int16x8_t v_s128 = vdupq_n_s16(128);
    for (y = 0; y < height; y+=2)
    {
        for (x = 0; x < width16; x+=16)
        {
            // variables
            uint8x8x4_t v_rgba;
            int32x4_t v_r[2], v_g[2], v_b[2];
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            int16x8_t v_tempU[4], v_tempV[4];
            uint8x8_t v_U, v_V;

            // P0
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[0] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[0] = vaddq_s16(v_tempU[0], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[0] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[0] = vaddq_s16(v_tempV[0], v_s128);
            //================================================================================

            // P1
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba + 32);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[1] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[1] = vaddq_s16(v_tempU[1], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[1] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[1] = vaddq_s16(v_tempV[1], v_s128);
            //================================================================================

            // P2
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba + row_stride);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[2] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[2] = vaddq_s16(v_tempU[2], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[2] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[2] = vaddq_s16(v_tempV[2], v_s128);
            //================================================================================

            // P3
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba + row_stride + 32);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[3] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[3] = vaddq_s16(v_tempU[3], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[3] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[3] = vaddq_s16(v_tempV[3], v_s128);
            //================================================================================

            // average of 4 points
            v_tempU[0] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempU[0]), vget_high_s16(v_tempU[0])), vpadd_s16(vget_low_s16(v_tempU[1]), vget_high_s16(v_tempU[1])));
            v_tempU[1] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempU[2]), vget_high_s16(v_tempU[2])), vpadd_s16(vget_low_s16(v_tempU[3]), vget_high_s16(v_tempU[3])));
            v_tempU[2] = vaddq_s16(v_tempU[0], v_tempU[1]);
            v_U = vrshrn_n_u16(vreinterpretq_u16_s16(v_tempU[2]), 2);
            vst1_u8(U, v_U);

            v_tempV[0] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempV[0]), vget_high_s16(v_tempV[0])), vpadd_s16(vget_low_s16(v_tempV[1]), vget_high_s16(v_tempV[1])));
            v_tempV[1] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempV[2]), vget_high_s16(v_tempV[2])), vpadd_s16(vget_low_s16(v_tempV[3]), vget_high_s16(v_tempV[3])));
            v_tempV[2] = vaddq_s16(v_tempV[0], v_tempV[1]);
            v_V = vrshrn_n_u16(vreinterpretq_u16_s16(v_tempV[2]), 2);
            vst1_u8(V, v_V);

            // increment
            U += 8;
            V += 8;
            rgba += 64;
        }
        rgba += row_stride;
    }
#endif
    for (y = 0; y < (height / 2); y++)
    {
        for (x = width16; x < width; x+=2)
        {
            /* No need to saturate between 16 and 240 */
            u00 = 128 + ((32768 +
                        RtoUCoeff * *(rgba) +
                        GtoUCoeff * *(rgba + 1) +
                        BtoUCoeff * *(rgba + 2)) >> 16);
            u01 = 128 + ((32768 +
                        RtoUCoeff * *(rgba + 4) +
                        GtoUCoeff * *(rgba + 5) +
                        BtoUCoeff * *(rgba + 6)) >> 16);
            u10 = 128 + ((32768 +
                        RtoUCoeff * *(rgba + row_stride) +
                        GtoUCoeff * *(rgba + row_stride + 1) +
                        BtoUCoeff * *(rgba + row_stride + 2)) >> 16);
            u11 = 128 + ((32768 +
                        RtoUCoeff * *(rgba + row_stride + 4) +
                        GtoUCoeff * *(rgba + row_stride + 5) +
                        BtoUCoeff * *(rgba + row_stride + 6)) >> 16);
            *U++ = (2 + u00 + u01 + u10 + u11) >> 2;

            v00 = 128 + ((32768 +
                        RtoVCoeff * *(rgba) +
                        GtoVCoeff * *(rgba + 1) +
                        BtoVCoeff * *(rgba + 2)) >> 16);
            v01 = 128 + ((32768 +
                        RtoVCoeff * *(rgba + 4) +
                        GtoVCoeff * *(rgba + 5) +
                        BtoVCoeff * *(rgba + 6)) >> 16);
            v10 = 128 + ((32768 +
                        RtoVCoeff * *(rgba + row_stride) +
                        GtoVCoeff * *(rgba + row_stride + 1) +
                        BtoVCoeff * *(rgba + row_stride + 2)) >> 16);
            v11 = 128 + ((32768 +
                        RtoVCoeff * *(rgba + row_stride + 4) +
                        GtoVCoeff * *(rgba + row_stride + 5) +
                        BtoVCoeff * *(rgba + row_stride + 6)) >> 16);
            *V++ = (2 + v00 + v01 + v10 + v11) >> 2;

            rgba += 8;
        }

        rgba += row_stride;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlRGBA8888toI420(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_rgba = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MINT32 proc_idx = pImg->proc_idx;
    MINT32 proc_width = pImg->proc_width;
    MINT32 proc_height = pImg->proc_height;
    MUINT8 *data_yuv = (MUINT8 *)pDst;

    MINT32 x, y, row_stride = width * 4;
    MUINT8 *rgba, *Y, *U, *V;

    MUINT8 u00, u01, u10, u11;
    MUINT8 v00, v01, v10, v11;

    MINT32 RtoYCoeff = (MINT32) ( 66 * 256);
    MINT32 GtoYCoeff = (MINT32) (129 * 256);
    MINT32 BtoYCoeff = (MINT32) ( 25 * 256);

    MINT32 RtoUCoeff = (MINT32) (-38 * 256);
    MINT32 GtoUCoeff = (MINT32) (-74 * 256);
    MINT32 BtoUCoeff = (MINT32) (112 * 256);

    MINT32 RtoVCoeff = (MINT32) (112 * 256);
    MINT32 GtoVCoeff = (MINT32) (-94 * 256);
    MINT32 BtoVCoeff = (MINT32) (-18 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* Y plane */
    rgba = data_rgba + proc_idx*proc_width*proc_height*4;
    Y   = data_yuv + proc_idx*proc_width*proc_height;

    MINT32 width8 = 0;
#ifdef NEON_OPT
    width8 = (proc_width & 0x7) ? (((proc_width - 8)>>3)<<3) : proc_width;
    int32x4_t v_RtoYCoeff = vdupq_n_s32(RtoYCoeff);
    int32x4_t v_GtoYCoeff = vdupq_n_s32(GtoYCoeff);
    int32x4_t v_BtoYCoeff = vdupq_n_s32(BtoYCoeff);
    uint8x8_t v_u16 = vdup_n_u8(16);
    for (y = 0; y < proc_height; y++)
    {
        for (x = 0; x < width8; x+=8)
        {
            // read rgba data
            uint8x8x4_t v_rgba = vld4_u8(rgba);
            int32x4_t v_r[2], v_g[2], v_b[2];
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            v_temp[0] = vmulq_s32(v_RtoYCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoYCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoYCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoYCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoYCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoYCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            uint8x8_t v_Y = vreinterpret_u8_s8(vmovn_s16(vcombine_s16(v_result[0], v_result[1])));
            v_Y = vadd_u8(v_Y, v_u16);
            vst1_u8(Y, v_Y);

            // increment
            Y += 8;
            rgba += 32;
        }
    }
#endif

    rgba = data_rgba + proc_idx*proc_width*proc_height*4;
    Y   = data_yuv + proc_idx*proc_width*proc_height;
    for (y = 0; y < proc_height; y++)
    {
        Y += width8;
        rgba += width8*4;
        for (x = width8; x < proc_width; x++)
        {
            /* No need to saturate between 16 and 235 */
            *Y = 16 + ((32768 +
                        RtoYCoeff * *(rgba) +
                        GtoYCoeff * *(rgba + 1) +
                        BtoYCoeff * *(rgba + 2)) >> 16);
            Y++;
            rgba += 4;
        }
    }

    /* U and V planes */
    rgba = data_rgba + proc_idx*proc_width*proc_height*4;
    U   = data_yuv + (width*height) + proc_idx*(proc_width/2)*(proc_height/2);
    V   = data_yuv + (width*height + ((width*height)/4)) + proc_idx*(proc_width/2)*(proc_height/2);

    MINT32 width16 = 0;
#ifdef NEON_OPT
    width16 = (proc_width & 0xF) ? (((proc_width-16)>>4)<<4) : proc_width;
    int32x4_t v_RtoUCoeff = vdupq_n_s32(RtoUCoeff);
    int32x4_t v_GtoUCoeff = vdupq_n_s32(GtoUCoeff);
    int32x4_t v_BtoUCoeff = vdupq_n_s32(BtoUCoeff);
    int32x4_t v_RtoVCoeff = vdupq_n_s32(RtoVCoeff);
    int32x4_t v_GtoVCoeff = vdupq_n_s32(GtoVCoeff);
    int32x4_t v_BtoVCoeff = vdupq_n_s32(BtoVCoeff);
    int16x8_t v_s128 = vdupq_n_s16(128);
    for (y = 0; y < proc_height; y+=2)
    {
        for (x = 0; x < width16; x+=16)
        {
            // variables
            uint8x8x4_t v_rgba;
            int32x4_t v_r[2], v_g[2], v_b[2];
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            int16x8_t v_tempU[4], v_tempV[4];
            uint8x8_t v_U, v_V;

            // P0
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[0] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[0] = vaddq_s16(v_tempU[0], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[0] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[0] = vaddq_s16(v_tempV[0], v_s128);
            //================================================================================

            // P1
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba + 32);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[1] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[1] = vaddq_s16(v_tempU[1], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[1] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[1] = vaddq_s16(v_tempV[1], v_s128);
            //================================================================================

            // P2
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba + row_stride);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[2] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[2] = vaddq_s16(v_tempU[2], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[2] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[2] = vaddq_s16(v_tempV[2], v_s128);
            //================================================================================

            // P3
            //================================================================================
            // read rgba data
            v_rgba = vld4_u8(rgba + row_stride + 32);
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoUCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoUCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoUCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoUCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoUCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoUCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempU[3] = vcombine_s16(v_result[0], v_result[1]);
            v_tempU[3] = vaddq_s16(v_tempU[3], v_s128);

            // color conversion
            v_temp[0] = vmulq_s32(v_RtoVCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoVCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoVCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoVCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoVCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoVCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            v_tempV[3] = vcombine_s16(v_result[0], v_result[1]);
            v_tempV[3] = vaddq_s16(v_tempV[3], v_s128);
            //================================================================================

            // average of 4 points
            v_tempU[0] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempU[0]), vget_high_s16(v_tempU[0])), vpadd_s16(vget_low_s16(v_tempU[1]), vget_high_s16(v_tempU[1])));
            v_tempU[1] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempU[2]), vget_high_s16(v_tempU[2])), vpadd_s16(vget_low_s16(v_tempU[3]), vget_high_s16(v_tempU[3])));
            v_tempU[2] = vaddq_s16(v_tempU[0], v_tempU[1]);
            v_U = vrshrn_n_u16(vreinterpretq_u16_s16(v_tempU[2]), 2);
            vst1_u8(U, v_U);

            v_tempV[0] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempV[0]), vget_high_s16(v_tempV[0])), vpadd_s16(vget_low_s16(v_tempV[1]), vget_high_s16(v_tempV[1])));
            v_tempV[1] = vcombine_s16(vpadd_s16(vget_low_s16(v_tempV[2]), vget_high_s16(v_tempV[2])), vpadd_s16(vget_low_s16(v_tempV[3]), vget_high_s16(v_tempV[3])));
            v_tempV[2] = vaddq_s16(v_tempV[0], v_tempV[1]);
            v_V = vrshrn_n_u16(vreinterpretq_u16_s16(v_tempV[2]), 2);
            vst1_u8(V, v_V);

            // increment
            U += 8;
            V += 8;
            rgba += 64;
        }
        rgba += row_stride;
    }
#endif
    for (y = 0; y < proc_height; y+=2)
    {
        for (x = width16; x < proc_width; x+=2)
        {
            /* No need to saturate between 16 and 240 */
            u00 = 128 + ((32768 +
                        RtoUCoeff * *(rgba) +
                        GtoUCoeff * *(rgba + 1) +
                        BtoUCoeff * *(rgba + 2)) >> 16);
            u01 = 128 + ((32768 +
                        RtoUCoeff * *(rgba + 4) +
                        GtoUCoeff * *(rgba + 5) +
                        BtoUCoeff * *(rgba + 6)) >> 16);
            u10 = 128 + ((32768 +
                        RtoUCoeff * *(rgba + row_stride) +
                        GtoUCoeff * *(rgba + row_stride + 1) +
                        BtoUCoeff * *(rgba + row_stride + 2)) >> 16);
            u11 = 128 + ((32768 +
                        RtoUCoeff * *(rgba + row_stride + 4) +
                        GtoUCoeff * *(rgba + row_stride + 5) +
                        BtoUCoeff * *(rgba + row_stride + 6)) >> 16);
            *U++ = (2 + u00 + u01 + u10 + u11) >> 2;

            v00 = 128 + ((32768 +
                        RtoVCoeff * *(rgba) +
                        GtoVCoeff * *(rgba + 1) +
                        BtoVCoeff * *(rgba + 2)) >> 16);
            v01 = 128 + ((32768 +
                        RtoVCoeff * *(rgba + 4) +
                        GtoVCoeff * *(rgba + 5) +
                        BtoVCoeff * *(rgba + 6)) >> 16);
            v10 = 128 + ((32768 +
                        RtoVCoeff * *(rgba + row_stride) +
                        GtoVCoeff * *(rgba + row_stride + 1) +
                        BtoVCoeff * *(rgba + row_stride + 2)) >> 16);
            v11 = 128 + ((32768 +
                        RtoVCoeff * *(rgba + row_stride + 4) +
                        GtoVCoeff * *(rgba + row_stride + 5) +
                        BtoVCoeff * *(rgba + row_stride + 6)) >> 16);
            *V++ = (2 + v00 + v01 + v10 + v11) >> 2;

            rgba += 8;
        }

        rgba += row_stride;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlRGBA8888toGrey(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_rgba = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MUINT8 *data_grey = (MUINT8 *)pDst;

    MINT32 x, y;
    MUINT8 *rgba, *Y;

    MINT32 RtoYCoeff = (MINT32) ( 66 * 256);
    MINT32 GtoYCoeff = (MINT32) (129 * 256);
    MINT32 BtoYCoeff = (MINT32) ( 25 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* Y plane */
    rgba = data_rgba;
    Y   = data_grey;

    MINT32 width8 = 0;
#ifdef NEON_OPT
    width8 = (width & 0x7) ? (((width - 8)>>3)<<3) : width;
    int32x4_t v_RtoYCoeff = vdupq_n_s32(RtoYCoeff);
    int32x4_t v_GtoYCoeff = vdupq_n_s32(GtoYCoeff);
    int32x4_t v_BtoYCoeff = vdupq_n_s32(BtoYCoeff);
    uint8x8_t v_u16 = vdup_n_u8(16);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width8; x+=8)
        {
            // read rgba data
            uint8x8x4_t v_rgba = vld4_u8(rgba);
            int32x4_t v_r[2], v_g[2], v_b[2];
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            v_temp[0] = vmulq_s32(v_RtoYCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoYCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoYCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoYCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoYCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoYCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            uint8x8_t v_Y = vreinterpret_u8_s8(vmovn_s16(vcombine_s16(v_result[0], v_result[1])));
            v_Y = vadd_u8(v_Y, v_u16);
            vst1_u8(Y, v_Y);

            // increment
            Y += 8;
            rgba += 32;
        }
    }
#endif

    rgba = data_rgba;
    Y   = data_grey;
    for (y = 0; y < height; y++)
    {
        Y += width8;
        rgba += width8*4;
        for (x = width8; x < width; x++)
        {
            /* No need to saturate between 16 and 235 */
            *Y = 16 + ((32768 +
                        RtoYCoeff * *(rgba) +
                        GtoYCoeff * *(rgba + 1) +
                        BtoYCoeff * *(rgba + 2)) >> 16);
            Y++;
            rgba += 4;
        }
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlRGBA8888toGrey(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_rgba = (MUINT8 *)(pImg->data);
    MINT32 proc_index = pImg->proc_idx;
    MINT32 proc_width = pImg->proc_width;
    MINT32 proc_height = pImg->proc_height;
    MUINT8 *data_grey = (MUINT8 *)pDst;

    MINT32 x, y;
    MUINT8 *rgba, *Y;

    MINT32 RtoYCoeff = (MINT32) ( 66 * 256);
    MINT32 GtoYCoeff = (MINT32) (129 * 256);
    MINT32 BtoYCoeff = (MINT32) ( 25 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* Y plane */
    rgba = data_rgba + proc_index*proc_width*proc_height*4;
    Y   = data_grey + proc_index*proc_width*proc_height;

    MINT32 width8 = 0;
#ifdef NEON_OPT
    width8 = (proc_width & 0x7) ? (((proc_width - 8)>>3)<<3) : proc_width;
    int32x4_t v_RtoYCoeff = vdupq_n_s32(RtoYCoeff);
    int32x4_t v_GtoYCoeff = vdupq_n_s32(GtoYCoeff);
    int32x4_t v_BtoYCoeff = vdupq_n_s32(BtoYCoeff);
    uint8x8_t v_u16 = vdup_n_u8(16);
    for (y = 0; y < proc_height; y++)
    {
        for (x = 0; x < width8; x+=8)
        {
            // read rgba data
            uint8x8x4_t v_rgba = vld4_u8(rgba);
            int32x4_t v_r[2], v_g[2], v_b[2];
            v_r[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[0]))));
            v_r[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[0]))));
            v_g[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[1]))));
            v_g[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[1]))));
            v_b[0] = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(v_rgba.val[2]))));
            v_b[1] = vreinterpretq_s32_u32(vmovl_u16(vget_high_u16(vmovl_u8(v_rgba.val[2]))));

            // color conversion
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            v_temp[0] = vmulq_s32(v_RtoYCoeff, v_r[0]);
            v_temp[1] = vmulq_s32(v_RtoYCoeff, v_r[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_GtoYCoeff, v_g[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_GtoYCoeff, v_g[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_BtoYCoeff, v_b[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_BtoYCoeff, v_b[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);

            // data combine
            uint8x8_t v_Y = vreinterpret_u8_s8(vmovn_s16(vcombine_s16(v_result[0], v_result[1])));
            v_Y = vadd_u8(v_Y, v_u16);
            vst1_u8(Y, v_Y);

            // increment
            Y += 8;
            rgba += 32;
        }
    }
#endif

    rgba = data_rgba + proc_index*proc_width*proc_height*4;
    Y   = data_grey + proc_index*proc_width*proc_height;
    for (y = 0; y < proc_height; y++)
    {
        Y += width8;
        rgba += width8*4;
        for (x = width8; x < proc_width; x++)
        {
            /* No need to saturate between 16 and 235 */
            *Y = 16 + ((32768 +
                        RtoYCoeff * *(rgba) +
                        GtoYCoeff * *(rgba + 1) +
                        BtoYCoeff * *(rgba + 2)) >> 16);
            Y++;
            rgba += 4;
        }
    }

    return result;
}

/*
 * YCbCr in Rec. 601 format
 * RGB values are in the range [0..255]
 *
 * [ R ]    1    [ 298      0    409 ]   [ Y -  16 ]
 * [ G ] = --- * [ 298   -100   -208 ] * [ U - 128 ]
 * [ B ]   256   [ 298    516      0 ]   [ V - 128 ]
 */
UTIL_ERRCODE_ENUM UtlI420toRGB888(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_yuv = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MUINT8 *data_rgb = (MUINT8 *)pDst;

    MINT32 x, y, row_stride = width * 3;
    MUINT8 *R, *G, *B, *Y, *U, *V;

    MINT32 YtoRCoeff = (MINT32) ( 298 * 256);
    MINT32 UtoRCoeff = (MINT32) (   0 * 256);
    MINT32 VtoRCoeff = (MINT32) ( 409 * 256);

    MINT32 YtoGCoeff = (MINT32) ( 298 * 256);
    MINT32 UtoGCoeff = (MINT32) (-100 * 256);
    MINT32 VtoGCoeff = (MINT32) (-208 * 256);

    MINT32 YtoBCoeff = (MINT32) (298 * 256);
    MINT32 UtoBCoeff = (MINT32) (516 * 256);
    MINT32 VtoBCoeff = (MINT32) (  0 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* R plane */
    R = data_rgb;
    G = data_rgb + 1;
    B = data_rgb + 2;
    Y = data_yuv;
    U = data_yuv + (width*height);
    V = data_yuv + (width*height + ((width*height)>>2));

    for (y = height/2; y-- > 0; )
    {
        for (x = width/2; x-- > 0; )
        {
            /* P00 */
            *R = UTL_CLIP8(
                 ((32768 +
                   YtoRCoeff * (*(Y) -  16) +
                   UtoRCoeff * (*(U) - 128) +
                   VtoRCoeff * (*(V) - 128)) >> 16) );
            *G = UTL_CLIP8(
                 ((32768 +
                   YtoGCoeff * (*(Y) -  16) +
                   UtoGCoeff * (*(U) - 128) +
                   VtoGCoeff * (*(V) - 128)) >> 16) );
            *B = UTL_CLIP8(
                 ((32768 +
                   YtoBCoeff * (*(Y) -  16) +
                   UtoBCoeff * (*(U) - 128) +
                   VtoBCoeff * (*(V) - 128)) >> 16) );

            /* P01 */
            *(R+3) = UTL_CLIP8(
                     ((32768 +
                       YtoRCoeff * (*(Y+1) -  16) +
                       UtoRCoeff * (*(U)   - 128) +
                       VtoRCoeff * (*(V)   - 128)) >> 16) );
            *(G+3) = UTL_CLIP8(
                     ((32768 +
                       YtoGCoeff * (*(Y+1) -  16) +
                       UtoGCoeff * (*(U)   - 128) +
                       VtoGCoeff * (*(V)   - 128)) >> 16) );
            *(B+3) = UTL_CLIP8(
                     ((32768 +
                       YtoBCoeff * (*(Y+1) -  16) +
                       UtoBCoeff * (*(U)   - 128) +
                       VtoBCoeff * (*(V)   - 128)) >> 16) );

            /* P10 */
            *(R+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoRCoeff * (*(Y+width) -  16) +
                                UtoRCoeff * (*(U)       - 128) +
                                VtoRCoeff * (*(V)       - 128)) >> 16) );
            *(G+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoGCoeff * (*(Y+width) -  16) +
                                UtoGCoeff * (*(U)       - 128) +
                                VtoGCoeff * (*(V)       - 128)) >> 16) );
            *(B+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoBCoeff * (*(Y+width) -  16) +
                                UtoBCoeff * (*(U)       - 128) +
                                VtoBCoeff * (*(V)       - 128)) >> 16) );

            /* P11 */
            *(R+row_stride+3) = UTL_CLIP8(
                                ((32768 +
                                  YtoRCoeff * (*(Y+width+1) -  16) +
                                  UtoRCoeff * (*(U)         - 128) +
                                  VtoRCoeff * (*(V)         - 128)) >> 16) );
            *(G+row_stride+3) = UTL_CLIP8(
                                ((32768 +
                                  YtoGCoeff * (*(Y+width+1) -  16) +
                                  UtoGCoeff * (*(U)         - 128) +
                                  VtoGCoeff * (*(V)         - 128)) >> 16) );
            *(B+row_stride+3) = UTL_CLIP8(
                                ((32768 +
                                  YtoBCoeff * (*(Y+width+1) -  16) +
                                  UtoBCoeff * (*(U)         - 128) +
                                  VtoBCoeff * (*(V)         - 128)) >> 16) );

            R += 6;
            G += 6;
            B += 6;

            Y += 2;
            U += 1;
            V += 1;
        }
        R += row_stride;
        G += row_stride;
        B += row_stride;

        Y += width;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlI420toRGBA8888(P_UTIL_BASE_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_yuv = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MUINT8 *data_rgba = (MUINT8 *)pDst;

    MINT32 x, y, row_stride = width * 4;
    MUINT8 *A, *R, *G, *B, *Y, *U, *V;

    MINT32 YtoRCoeff = (MINT32) ( 298 * 256);
    MINT32 UtoRCoeff = (MINT32) (   0 * 256);
    MINT32 VtoRCoeff = (MINT32) ( 409 * 256);

    MINT32 YtoGCoeff = (MINT32) ( 298 * 256);
    MINT32 UtoGCoeff = (MINT32) (-100 * 256);
    MINT32 VtoGCoeff = (MINT32) (-208 * 256);

    MINT32 YtoBCoeff = (MINT32) (298 * 256);
    MINT32 UtoBCoeff = (MINT32) (516 * 256);
    MINT32 VtoBCoeff = (MINT32) (  0 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* R plane */
    R = data_rgba;
    G = data_rgba + 1;
    B = data_rgba + 2;
    A = data_rgba + 3;
    Y = data_yuv;
    U = data_yuv + (width*height);
    V = data_yuv + (width*height + ((width*height)>>2));

    MINT32 width16 = 0;
#ifdef NEON_OPT
    int32x4_t v_YtoRCoeff = vdupq_n_s32(YtoRCoeff);
    int32x4_t v_UtoRCoeff = vdupq_n_s32(UtoRCoeff);
    int32x4_t v_VtoRCoeff = vdupq_n_s32(VtoRCoeff);
    int32x4_t v_YtoGCoeff = vdupq_n_s32(YtoGCoeff);
    int32x4_t v_UtoGCoeff = vdupq_n_s32(UtoGCoeff);
    int32x4_t v_VtoGCoeff = vdupq_n_s32(VtoGCoeff);
    int32x4_t v_YtoBCoeff = vdupq_n_s32(YtoBCoeff);
    int32x4_t v_UtoBCoeff = vdupq_n_s32(UtoBCoeff);
    int32x4_t v_VtoBCoeff = vdupq_n_s32(VtoBCoeff);
    width16 = (width & 0xF) ? (((width-16)>>4)<<4) : width;
    int16x8_t v_s16 = vdupq_n_s16(16);
    int16x8_t v_s128 = vdupq_n_s16(128);
    int16x8_t v_s0 = vdupq_n_s16(0);
    int16x8_t v_s255 = vdupq_n_s16(255);
    uint8x8_t v_A = vdup_n_u8(255);
    for (y = 0; y < height; y += 2)
    {
        for (x = 0; x < width16; x += 16)
        {
            // variables
            uint8x16_t v_srcY[2], v_dupU, v_dupV;
            uint8x8_t v_srcU, v_srcV;
            int16x8_t v_tmpY[4], v_tmpU[4], v_tmpV[4], v_tmp;
            int32x4_t v_Y[2], v_U[2], v_V[2];
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            uint8x8x4_t v_RGBA;

            // read yuv data
            v_srcY[0] = vld1q_u8(Y);
            v_srcY[1] = vld1q_u8(Y + width);
            v_srcU = vld1_u8(U);
            v_srcV = vld1_u8(V);
            v_tmpY[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_srcY[0]))), v_s16);
            v_tmpY[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_srcY[0]))), v_s16);
            v_tmpY[2] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_srcY[1]))), v_s16);
            v_tmpY[3] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_srcY[1]))), v_s16);
            v_dupU = vreinterpretq_u8_u16(vaddq_u16(vshlq_n_u16(vmovl_u8(v_srcU), 8), vmovl_u8(v_srcU)));
            v_dupV = vreinterpretq_u8_u16(vaddq_u16(vshlq_n_u16(vmovl_u8(v_srcV), 8), vmovl_u8(v_srcV)));
            v_tmpU[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_dupU))), v_s128);
            v_tmpU[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_dupU))), v_s128);
            v_tmpU[2] = v_tmpU[0];
            v_tmpU[3] = v_tmpU[1];
            v_tmpV[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_dupV))), v_s128);
            v_tmpV[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_dupV))), v_s128);
            v_tmpV[2] = v_tmpV[0];
            v_tmpV[3] = v_tmpV[1];

            // P00
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[0]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[0]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[0]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[0]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[0]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[0]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R, v_RGBA);
            //=====================================================================================

            // P01
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[1]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[1]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[1]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[1]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[1]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[1]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R + 32, v_RGBA);
            //=====================================================================================

            // P10
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[2]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[2]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[2]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[2]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[2]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[2]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R + row_stride, v_RGBA);
            //=====================================================================================

            // P11
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[3]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[3]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[3]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[3]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[3]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[3]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R + row_stride + 32, v_RGBA);
            //=====================================================================================

            R += 64;
            Y += 16;
            U += 8;
            V += 8;
        }

        // increment
        R += row_stride;
        Y += width;
    }
#endif
    R = data_rgba;
    G = data_rgba + 1;
    B = data_rgba + 2;
    A = data_rgba + 3;
    Y = data_yuv;
    U = data_yuv + (width*height);
    V = data_yuv + (width*height + ((width*height)>>2));
    for (y = 0; y < height; y += 2)
    {
        for (x = width16; x < width; x += 2)
        {
            /* P00 */
            *R = UTL_CLIP8(
                 ((32768 +
                   YtoRCoeff * (*(Y) -  16) +
                   UtoRCoeff * (*(U) - 128) +
                   VtoRCoeff * (*(V) - 128)) >> 16) );
            *G = UTL_CLIP8(
                 ((32768 +
                   YtoGCoeff * (*(Y) -  16) +
                   UtoGCoeff * (*(U) - 128) +
                   VtoGCoeff * (*(V) - 128)) >> 16) );
            *B = UTL_CLIP8(
                 ((32768 +
                   YtoBCoeff * (*(Y) -  16) +
                   UtoBCoeff * (*(U) - 128) +
                   VtoBCoeff * (*(V) - 128)) >> 16) );
            *A = 0xFF;

            /* P01 */
            *(R+4) = UTL_CLIP8(
                     ((32768 +
                       YtoRCoeff * (*(Y+1) -  16) +
                       UtoRCoeff * (*(U)   - 128) +
                       VtoRCoeff * (*(V)   - 128)) >> 16) );
            *(G+4) = UTL_CLIP8(
                     ((32768 +
                       YtoGCoeff * (*(Y+1) -  16) +
                       UtoGCoeff * (*(U)   - 128) +
                       VtoGCoeff * (*(V)   - 128)) >> 16) );
            *(B+4) = UTL_CLIP8(
                     ((32768 +
                       YtoBCoeff * (*(Y+1) -  16) +
                       UtoBCoeff * (*(U)   - 128) +
                       VtoBCoeff * (*(V)   - 128)) >> 16) );
            *(A+4) = 0xFF;

            /* P10 */
            *(R+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoRCoeff * (*(Y+width) -  16) +
                                UtoRCoeff * (*(U)       - 128) +
                                VtoRCoeff * (*(V)       - 128)) >> 16) );
            *(G+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoGCoeff * (*(Y+width) -  16) +
                                UtoGCoeff * (*(U)       - 128) +
                                VtoGCoeff * (*(V)       - 128)) >> 16) );
            *(B+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoBCoeff * (*(Y+width) -  16) +
                                UtoBCoeff * (*(U)       - 128) +
                                VtoBCoeff * (*(V)       - 128)) >> 16) );
            *(A+row_stride) = 0xFF;

            /* P11 */
            *(R+row_stride+4) = UTL_CLIP8(
                                ((32768 +
                                  YtoRCoeff * (*(Y+width+1) -  16) +
                                  UtoRCoeff * (*(U)         - 128) +
                                  VtoRCoeff * (*(V)         - 128)) >> 16) );
            *(G+row_stride+4) = UTL_CLIP8(
                                ((32768 +
                                  YtoGCoeff * (*(Y+width+1) -  16) +
                                  UtoGCoeff * (*(U)         - 128) +
                                  VtoGCoeff * (*(V)         - 128)) >> 16) );
            *(B+row_stride+4) = UTL_CLIP8(
                                ((32768 +
                                  YtoBCoeff * (*(Y+width+1) -  16) +
                                  UtoBCoeff * (*(U)         - 128) +
                                  VtoBCoeff * (*(V)         - 128)) >> 16) );
            *(A+row_stride+4) = 0xFF;

            R += 8;
            G += 8;
            B += 8;
            A += 8;

            Y += 2;
            U += 1;
            V += 1;
        }
        R += row_stride;
        G += row_stride;
        B += row_stride;
        A += row_stride;

        Y += width;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlI420toRGBA8888(P_UTIL_MP_IMAGE_STRUCT pImg, void *pDst)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *data_yuv = (MUINT8 *)(pImg->data);
    MINT32 width = pImg->width;
    MINT32 height = pImg->height;
    MINT32 proc_index = pImg->proc_idx;
    MINT32 proc_width = pImg->proc_width;
    MINT32 proc_height = pImg->proc_height;
    MUINT8 *data_rgba = (MUINT8 *)pDst;

    MINT32 x, y, row_stride = width * 4;
    MUINT8 *A, *R, *G, *B, *Y, *U, *V;

    MINT32 YtoRCoeff = (MINT32) ( 298 * 256);
    MINT32 UtoRCoeff = (MINT32) (   0 * 256);
    MINT32 VtoRCoeff = (MINT32) ( 409 * 256);

    MINT32 YtoGCoeff = (MINT32) ( 298 * 256);
    MINT32 UtoGCoeff = (MINT32) (-100 * 256);
    MINT32 VtoGCoeff = (MINT32) (-208 * 256);

    MINT32 YtoBCoeff = (MINT32) (298 * 256);
    MINT32 UtoBCoeff = (MINT32) (516 * 256);
    MINT32 VtoBCoeff = (MINT32) (  0 * 256);

    // data pointer check
    if (!pImg->data || !pDst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    /* R plane */
    data_rgba += proc_index*proc_width*proc_height*4;
    R = data_rgba;
    G = data_rgba + 1;
    B = data_rgba + 2;
    A = data_rgba + 3;
    Y = data_yuv + proc_index*proc_width*proc_height;
    U = data_yuv + (width*height) + proc_index*(proc_width/2)*(proc_height/2);
    V = data_yuv + (width*height + ((width*height)>>2)) + proc_index*(proc_width/2)*(proc_height/2);

    MINT32 width16 = 0;
    MINT32 widthDiff = 0;
#ifdef NEON_OPT
    int32x4_t v_YtoRCoeff = vdupq_n_s32(YtoRCoeff);
    int32x4_t v_UtoRCoeff = vdupq_n_s32(UtoRCoeff);
    int32x4_t v_VtoRCoeff = vdupq_n_s32(VtoRCoeff);
    int32x4_t v_YtoGCoeff = vdupq_n_s32(YtoGCoeff);
    int32x4_t v_UtoGCoeff = vdupq_n_s32(UtoGCoeff);
    int32x4_t v_VtoGCoeff = vdupq_n_s32(VtoGCoeff);
    int32x4_t v_YtoBCoeff = vdupq_n_s32(YtoBCoeff);
    int32x4_t v_UtoBCoeff = vdupq_n_s32(UtoBCoeff);
    int32x4_t v_VtoBCoeff = vdupq_n_s32(VtoBCoeff);
    width16 = (proc_width & 0xF) ? (((proc_width-16)>>4)<<4) : proc_width;
    widthDiff = width - width16;
    int16x8_t v_s16 = vdupq_n_s16(16);
    int16x8_t v_s128 = vdupq_n_s16(128);
    int16x8_t v_s0 = vdupq_n_s16(0);
    int16x8_t v_s255 = vdupq_n_s16(255);
    uint8x8_t v_A = vdup_n_u8(255);
    for (y = 0; y < proc_height; y += 2)
    {
        for (x = 0; x < width16; x += 16)
        {
            // variables
            uint8x16_t v_srcY[2], v_dupU, v_dupV;
            uint8x8_t v_srcU, v_srcV;
            int16x8_t v_tmpY[4], v_tmpU[4], v_tmpV[4], v_tmp;
            int32x4_t v_Y[2], v_U[2], v_V[2];
            int32x4_t v_temp[2];
            int16x4_t v_result[2];
            uint8x8x4_t v_RGBA;

            // read yuv data
            v_srcY[0] = vld1q_u8(Y);
            v_srcY[1] = vld1q_u8(Y + width);
            v_srcU = vld1_u8(U);
            v_srcV = vld1_u8(V);
            v_tmpY[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_srcY[0]))), v_s16);
            v_tmpY[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_srcY[0]))), v_s16);
            v_tmpY[2] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_srcY[1]))), v_s16);
            v_tmpY[3] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_srcY[1]))), v_s16);
            v_dupU = vreinterpretq_u8_u16(vaddq_u16(vshlq_n_u16(vmovl_u8(v_srcU), 8), vmovl_u8(v_srcU)));
            v_dupV = vreinterpretq_u8_u16(vaddq_u16(vshlq_n_u16(vmovl_u8(v_srcV), 8), vmovl_u8(v_srcV)));
            v_tmpU[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_dupU))), v_s128);
            v_tmpU[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_dupU))), v_s128);
            v_tmpU[2] = v_tmpU[0];
            v_tmpU[3] = v_tmpU[1];
            v_tmpV[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(v_dupV))), v_s128);
            v_tmpV[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(v_dupV))), v_s128);
            v_tmpV[2] = v_tmpV[0];
            v_tmpV[3] = v_tmpV[1];

            // P00
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[0]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[0]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[0]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[0]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[0]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[0]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R, v_RGBA);
            //=====================================================================================

            // P01
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[1]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[1]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[1]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[1]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[1]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[1]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R + 32, v_RGBA);
            //=====================================================================================

            // P10
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[2]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[2]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[2]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[2]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[2]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[2]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R + row_stride, v_RGBA);
            //=====================================================================================

            // P11
            //=====================================================================================
            // get yuv data
            v_Y[0] = vmovl_s16(vget_low_s16(v_tmpY[3]));
            v_Y[1] = vmovl_s16(vget_high_s16(v_tmpY[3]));
            v_U[0] = vmovl_s16(vget_low_s16(v_tmpU[3]));
            v_U[1] = vmovl_s16(vget_high_s16(v_tmpU[3]));
            v_V[0] = vmovl_s16(vget_low_s16(v_tmpV[3]));
            v_V[1] = vmovl_s16(vget_high_s16(v_tmpV[3]));

            // color conversion (yuv -> r)
            v_temp[0] = vmulq_s32(v_YtoRCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoRCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoRCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoRCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoRCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoRCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[0] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> g)
            v_temp[0] = vmulq_s32(v_YtoGCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoGCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoGCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoGCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoGCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoGCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[1] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // color conversion (yuv -> b)
            v_temp[0] = vmulq_s32(v_YtoBCoeff, v_Y[0]);
            v_temp[1] = vmulq_s32(v_YtoBCoeff, v_Y[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_UtoBCoeff, v_U[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_UtoBCoeff, v_U[1]);
            v_temp[0] = vmlaq_s32(v_temp[0], v_VtoBCoeff, v_V[0]);
            v_temp[1] = vmlaq_s32(v_temp[1], v_VtoBCoeff, v_V[1]);
            v_result[0] = vrshrn_n_s32(v_temp[0], 16);
            v_result[1] = vrshrn_n_s32(v_temp[1], 16);
            v_tmp = vcombine_s16(v_result[0], v_result[1]);
            v_tmp = vmaxq_s16(v_tmp, v_s0);
            v_tmp = vminq_s16(v_tmp, v_s255);
            v_RGBA.val[2] = vmovn_u16(vreinterpretq_u16_s16(v_tmp));

            // fill alpha
            v_RGBA.val[3] = v_A;
            vst4_u8(R + row_stride + 32, v_RGBA);
            //=====================================================================================

            R += 64;
            Y += 16;
            U += 8;
            V += 8;
        }

        // increment
        R += row_stride + widthDiff*4;
        Y += width + widthDiff;
        U += widthDiff/2;
        V += widthDiff/2;
    }
#endif

    R = data_rgba + width16 * 4;
    G = data_rgba + width16 * 4 + 1;
    B = data_rgba + width16 * 4 + 2;
    A = data_rgba + width16 * 4 + 3;
    Y = data_yuv + proc_index*proc_width*proc_height + width16;
    U = data_yuv + (width*height) + proc_index*(proc_width/2)*(proc_height/2) + (width16 / 2);
    V = data_yuv + (width*height + ((width*height)>>2)) + proc_index*(proc_width/2)*(proc_height/2) + (width16 / 2);
    for (y = 0; y < proc_height; y += 2)
    {
        for (x = width16; x < proc_width; x += 2)
        {
            /* P00 */
            *R = UTL_CLIP8(
                 ((32768 +
                   YtoRCoeff * (*(Y) -  16) +
                   UtoRCoeff * (*(U) - 128) +
                   VtoRCoeff * (*(V) - 128)) >> 16) );
            *G = UTL_CLIP8(
                 ((32768 +
                   YtoGCoeff * (*(Y) -  16) +
                   UtoGCoeff * (*(U) - 128) +
                   VtoGCoeff * (*(V) - 128)) >> 16) );
            *B = UTL_CLIP8(
                 ((32768 +
                   YtoBCoeff * (*(Y) -  16) +
                   UtoBCoeff * (*(U) - 128) +
                   VtoBCoeff * (*(V) - 128)) >> 16) );
            *A = 0xFF;

            /* P01 */
            *(R+4) = UTL_CLIP8(
                     ((32768 +
                       YtoRCoeff * (*(Y+1) -  16) +
                       UtoRCoeff * (*(U)   - 128) +
                       VtoRCoeff * (*(V)   - 128)) >> 16) );
            *(G+4) = UTL_CLIP8(
                     ((32768 +
                       YtoGCoeff * (*(Y+1) -  16) +
                       UtoGCoeff * (*(U)   - 128) +
                       VtoGCoeff * (*(V)   - 128)) >> 16) );
            *(B+4) = UTL_CLIP8(
                     ((32768 +
                       YtoBCoeff * (*(Y+1) -  16) +
                       UtoBCoeff * (*(U)   - 128) +
                       VtoBCoeff * (*(V)   - 128)) >> 16) );
            *(A+4) = 0xFF;

            /* P10 */
            *(R+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoRCoeff * (*(Y+width) -  16) +
                                UtoRCoeff * (*(U)       - 128) +
                                VtoRCoeff * (*(V)       - 128)) >> 16) );
            *(G+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoGCoeff * (*(Y+width) -  16) +
                                UtoGCoeff * (*(U)       - 128) +
                                VtoGCoeff * (*(V)       - 128)) >> 16) );
            *(B+row_stride) = UTL_CLIP8(
                              ((32768 +
                                YtoBCoeff * (*(Y+width) -  16) +
                                UtoBCoeff * (*(U)       - 128) +
                                VtoBCoeff * (*(V)       - 128)) >> 16) );
            *(A+row_stride) = 0xFF;

            /* P11 */
            *(R+row_stride+4) = UTL_CLIP8(
                                ((32768 +
                                  YtoRCoeff * (*(Y+width+1) -  16) +
                                  UtoRCoeff * (*(U)         - 128) +
                                  VtoRCoeff * (*(V)         - 128)) >> 16) );
            *(G+row_stride+4) = UTL_CLIP8(
                                ((32768 +
                                  YtoGCoeff * (*(Y+width+1) -  16) +
                                  UtoGCoeff * (*(U)         - 128) +
                                  VtoGCoeff * (*(V)         - 128)) >> 16) );
            *(B+row_stride+4) = UTL_CLIP8(
                                ((32768 +
                                  YtoBCoeff * (*(Y+width+1) -  16) +
                                  UtoBCoeff * (*(U)         - 128) +
                                  VtoBCoeff * (*(V)         - 128)) >> 16) );
            *(A+row_stride+4) = 0xFF;

            R += 8;
            G += 8;
            B += 8;
            A += 8;

            Y += 2;
            U += 1;
            V += 1;
        }
        R += row_stride*2 - widthDiff*4;
        G += row_stride*2 - widthDiff*4;
        B += row_stride*2 - widthDiff*4;
        A += row_stride*2 - widthDiff*4;

        Y += proc_width*2 - widthDiff;
        U += (proc_width - widthDiff)/2;
        V += (proc_width - widthDiff)/2;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlRgb565ToGray(P_UTIL_BASE_IMAGE_STRUCT pImg)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT32 r, g, b;

    MUINT32 *pimg;
    MUINT32 *pimg_end;
    MUINT32 val;
    MUINT32 data;
    MUINT8  *yimg;

    // data pointer check
    if (!pImg->data)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    yimg = (MUINT8*)pImg->data;
    pimg = (MUINT32*)pImg->data;
    pimg_end = pimg + ((pImg->width*pImg->height)>>1);

    while (pimg < pimg_end)
    {
        data = *pimg;
        r = UTL_RGB565_TO_RGB888_R(data>>16);
        g = UTL_RGB565_TO_RGB888_G(data>>16);
        b = UTL_RGB565_TO_RGB888_B(data>>16);
        val = UTL_RGB888_TO_YUV_Y(r, g, b);
        r = UTL_RGB565_TO_RGB888_R(data&0xFFFF);
        g = UTL_RGB565_TO_RGB888_G(data&0xFFFF);
        b = UTL_RGB565_TO_RGB888_B(data&0xFFFF);
        *(yimg + 1) = val;
        *(yimg + 0) = UTL_RGB888_TO_YUV_Y(r, g, b);
        pimg++;
        yimg += 2;
    }

    return result;
}

UTIL_ERRCODE_ENUM UtlYUYVtoI420(P_UTIL_BASE_IMAGE_STRUCT pDst, P_UTIL_BASE_IMAGE_STRUCT pSrc)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 width = pSrc->width;
    MINT32 height = pSrc->height;
    MUINT8 *p_src = (MUINT8 *)pSrc->data;
    MUINT8 *p_dst_y = (MUINT8 *)pDst->data;
    MUINT8 *p_dst_u = p_dst_y + width*height;
    MUINT8 *p_dst_v = p_dst_u + ((width*height)>>2);

    MINT32 double_width = width << 1;
    MINT32 half_width = width >> 1;
    MINT32 tmp_src_idx = 0;
    MINT32 tmp_y_idx = 0;
    MINT32 tmp_uv_idx = 0;
    for (MINT32 i=0; i<height/2; i++)
    {
        MINT32 j8 = 0;
#ifdef NEON_OPT
        uint16x8_t v_const_1 = vdupq_n_u16(1);
        uint16x8_t v_const_256 = vdupq_n_u16(256);
        j8 = (width >> 4) << 3;
        for (MINT32 j=0; j<j8; j+=8)
        {
            MINT32 src_idx = tmp_src_idx + (j<<2);
            MINT32 y_idx = tmp_y_idx + (j<<1);
            MINT32 uv_idx = tmp_uv_idx + j;

            // [0]: current line, [1]: next line
            uint8x8x4_t v_src[2];
            uint8x8x2_t v_yy[2];
            uint8x16_t v_y[2];
            uint16x8_t v_u_tmp, v_v_tmp;

            // load YUYV
            v_src[0] = vld4_u8(&p_src[src_idx]);
            v_src[1] = vld4_u8(&p_src[src_idx + double_width]);

            // zip Y
            v_yy[0] = vzip_u8(v_src[0].val[0], v_src[0].val[2]);
            v_yy[1] = vzip_u8(v_src[1].val[0], v_src[1].val[2]);

            // combine Y
            v_y[0] = vcombine_u8(v_yy[0].val[0], v_yy[0].val[1]);
            v_y[1] = vcombine_u8(v_yy[1].val[0], v_yy[1].val[1]);

            // store Y
            vst1q_u8(&p_dst_y[y_idx], v_y[0]);
            vst1q_u8(&p_dst_y[y_idx + width], v_y[1]);

            // add u/v
            v_u_tmp = vaddl_u8(v_src[0].val[1], v_src[1].val[1]);
            v_v_tmp = vaddl_u8(v_src[0].val[3], v_src[1].val[3]);

            // comparison -> [0]: u , [1]: v
            uint8x8_t v_cond[2];
            v_cond[0] = vmovn_u16(vcgtq_u16(v_u_tmp, v_const_256));
            v_cond[1] = vmovn_u16(vcgtq_u16(v_v_tmp, v_const_256));

            // average for both case
            uint8x8_t v_u_avg[2], v_v_avg[2];
            v_u_avg[0] = vrshrn_n_u16(v_u_tmp, 1);
            v_u_avg[1] = vshrn_n_u16(vsubq_u16(v_u_tmp, v_const_1), 1);
            v_v_avg[0] = vrshrn_n_u16(v_v_tmp, 1);
            v_v_avg[1] = vshrn_n_u16(vsubq_u16(v_v_tmp, v_const_1), 1);

            // bit select
            uint8x8_t v_u, v_v;
            v_u = vbsl_u8(v_cond[0], v_u_avg[1], v_u_avg[0]);
            v_v = vbsl_u8(v_cond[1], v_v_avg[1], v_v_avg[0]);

            // store u/v
            vst1_u8(&p_dst_u[uv_idx], v_u);
            vst1_u8(&p_dst_v[uv_idx], v_v);
        }
#endif
        for (MINT32 j=j8; j<half_width; j++)
        {
            MINT32 src_idx = tmp_src_idx + (j<<2);
            MINT32 y_idx = tmp_y_idx + (j<<1);
            MINT32 uv_idx = tmp_uv_idx + j;

            p_dst_y[y_idx + 0] = p_src[src_idx + 0];
            p_dst_y[y_idx + 1] = p_src[src_idx + 2];
            p_dst_y[y_idx + width + 0] = p_src[src_idx + double_width + 0];
            p_dst_y[y_idx + width + 1] = p_src[src_idx + double_width + 2];

            MUINT32 u_temp = p_src[src_idx + 1] + p_src[src_idx + double_width + 1];
            MUINT32 v_temp = p_src[src_idx + 3] + p_src[src_idx + double_width + 3];
            if(u_temp>256)
                u_temp = (u_temp - 1)>>1;
            else
                u_temp = (u_temp + 1)>>1;
            if(v_temp>256)
                v_temp = (v_temp - 1)>>1;
            else
                v_temp = (v_temp + 1)>>1;

            p_dst_u[uv_idx] = u_temp;
            p_dst_v[uv_idx] = v_temp;
        }

        // next row
        tmp_src_idx += (width << 2);
        tmp_y_idx += double_width;
        tmp_uv_idx += half_width;
    }
    return result;
}

UTIL_ERRCODE_ENUM UtlI420toYUYV(P_UTIL_BASE_IMAGE_STRUCT pDst, P_UTIL_BASE_IMAGE_STRUCT pSrc)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 width = pSrc->width;
    MINT32 height = pSrc->height;
    MUINT8 *p_src_y = (MUINT8 *)pSrc->data;
    MUINT8 *p_src_u = p_src_y + width*height;
    MUINT8 *p_src_v = p_src_u + ((width*height)>>2);
    MUINT8 *p_dst = (MUINT8 *)pDst->data;

    MINT32 double_width = width << 1;
    MINT32 half_width = width >> 1;
    MINT32 j8 = 0;
    MINT32 tmp_dst_idx = 0;
    MINT32 tmp_y_idx = 0;
    MINT32 tmp_uv_idx = 0;
    // the most top row: i = 0
#ifdef NEON_OPT
    uint8x8_t v_const_3 = vdup_n_u8(3);
    j8 = (width >> 4) << 3;
    for (MINT32 j=0; j<j8; j+=8)
    {
        MINT32 dst_idx = (j<<2);
        MINT32 y_idx = (j<<1);
        MINT32 uv_idx = j;

        // load Y -> [0]: current line, [1]: next line
        uint8x16_t v_src_y[2];
        v_src_y[0] = vld1q_u8(&p_src_y[y_idx]);
        v_src_y[1] = vld1q_u8(&p_src_y[y_idx + width]);

        // unzip Y -> [0]: current line, [1]: next line
        uint8x8x2_t v_yy[2];
        v_yy[0] = vuzp_u8(vget_low_u8(v_src_y[0]), vget_high_u8(v_src_y[0]));
        v_yy[1] = vuzp_u8(vget_low_u8(v_src_y[1]), vget_high_u8(v_src_y[1]));

        // load U & V -> [0]: current line, [1]: next line
        uint8x8_t v_u[2], v_v[2];
        v_u[0] = vld1_u8(&p_src_u[uv_idx]);
        v_u[1] = vld1_u8(&p_src_u[uv_idx + half_width]);
        v_v[0] = vld1_u8(&p_src_v[uv_idx]);
        v_v[1] = vld1_u8(&p_src_v[uv_idx + half_width]);

        // filter U & V
        uint16x8_t v_u_tmp, v_v_tmp;
        uint8x8_t v_u_avg, v_v_avg;
        v_u_tmp = vmlal_u8(vmovl_u8(v_u[1]), v_u[0], v_const_3);
        v_v_tmp = vmlal_u8(vmovl_u8(v_v[1]), v_v[0], v_const_3);
        v_u_avg = vrshrn_n_u16(v_u_tmp, 2);
        v_v_avg = vrshrn_n_u16(v_v_tmp, 2);

        // combine YUYV -> [0]: current line, [1]: next line
        uint8x8x4_t v_dst[2];
        v_dst[0].val[0] = v_yy[0].val[0];
        v_dst[0].val[1] = v_u[0];
        v_dst[0].val[2] = v_yy[0].val[1];
        v_dst[0].val[3] = v_v[0];
        v_dst[1].val[0] = v_yy[1].val[0];
        v_dst[1].val[1] = v_u_avg;
        v_dst[1].val[2] = v_yy[1].val[1];
        v_dst[1].val[3] = v_v_avg;

        // store YUYV
        vst4_u8(&p_dst[dst_idx], v_dst[0]);
        vst4_u8(&p_dst[dst_idx + double_width], v_dst[1]);
    }
#endif
    for (MINT32 j=j8; j<half_width; j++)
    {
        MINT32 dst_idx = (j<<2);
        MINT32 y_idx = (j<<1);
        MINT32 uv_idx = j;

        // Y
        p_dst[dst_idx + 0] = p_src_y[y_idx + 0];
        p_dst[dst_idx + 2] = p_src_y[y_idx + 1];
        p_dst[dst_idx + double_width + 0] = p_src_y[y_idx + width + 0];
        p_dst[dst_idx + double_width + 2] = p_src_y[y_idx + width + 1];

        // U & V
        p_dst[dst_idx + 1] = p_src_u[uv_idx];
        p_dst[dst_idx + 3] = p_src_v[uv_idx];
        p_dst[dst_idx + double_width + 1] = (p_src_u[uv_idx]*3 + p_src_u[uv_idx + half_width] + 2) >> 2;
        p_dst[dst_idx + double_width + 3] = (p_src_v[uv_idx]*3 + p_src_v[uv_idx + half_width] + 2) >> 2;
    }

    // next row
    tmp_dst_idx += (width << 2);
    tmp_y_idx += double_width;
    tmp_uv_idx += half_width;

    // middlw rows
    for (MINT32 i=1; i<(height/2)-1; i++)
    {
#ifdef NEON_OPT
        j8 = (width >> 4) << 3;
        for (MINT32 j=0; j<j8; j+=8)
        {
            MINT32 dst_idx = tmp_dst_idx + (j<<2);
            MINT32 y_idx = tmp_y_idx + (j<<1);
            MINT32 uv_idx = tmp_uv_idx + j;

            // load Y -> [0]: current line, [1]: next line
            uint8x16_t v_src_y[2];
            v_src_y[0] = vld1q_u8(&p_src_y[y_idx]);
            v_src_y[1] = vld1q_u8(&p_src_y[y_idx + width]);

            // unzip Y -> [0]: current line, [1]: next line
            uint8x8x2_t v_yy[2];
            v_yy[0] = vuzp_u8(vget_low_u8(v_src_y[0]), vget_high_u8(v_src_y[0]));
            v_yy[1] = vuzp_u8(vget_low_u8(v_src_y[1]), vget_high_u8(v_src_y[1]));

            // load U & V -> [0]: previous line, [1]: current line, [2]: next line
            uint8x8_t v_u[3], v_v[3];
            v_u[0] = vld1_u8(&p_src_u[uv_idx - half_width]);
            v_u[1] = vld1_u8(&p_src_u[uv_idx]);
            v_u[2] = vld1_u8(&p_src_u[uv_idx + half_width]);
            v_v[0] = vld1_u8(&p_src_v[uv_idx - half_width]);
            v_v[1] = vld1_u8(&p_src_v[uv_idx]);
            v_v[2] = vld1_u8(&p_src_v[uv_idx + half_width]);

            // filter U & V -> [0]: filter result of previous and current line, [1]: filter result of current and next line
            uint16x8_t v_u_tmp, v_v_tmp;
            uint8x8_t v_u_avg[2], v_v_avg[2];
            v_u_tmp = vmlal_u8(vmovl_u8(v_u[0]), v_u[1], v_const_3);
            v_v_tmp = vmlal_u8(vmovl_u8(v_v[0]), v_v[1], v_const_3);
            v_u_avg[0] = vrshrn_n_u16(v_u_tmp, 2);
            v_v_avg[0] = vrshrn_n_u16(v_v_tmp, 2);
            v_u_tmp = vmlal_u8(vmovl_u8(v_u[2]), v_u[1], v_const_3);
            v_v_tmp = vmlal_u8(vmovl_u8(v_v[2]), v_v[1], v_const_3);
            v_u_avg[1] = vrshrn_n_u16(v_u_tmp, 2);
            v_v_avg[1] = vrshrn_n_u16(v_v_tmp, 2);

            // combine YUYV -> [0]: current line, [1]: next line
            uint8x8x4_t v_dst[2];
            v_dst[0].val[0] = v_yy[0].val[0];
            v_dst[0].val[1] = v_u_avg[0];
            v_dst[0].val[2] = v_yy[0].val[1];
            v_dst[0].val[3] = v_v_avg[0];
            v_dst[1].val[0] = v_yy[1].val[0];
            v_dst[1].val[1] = v_u_avg[1];
            v_dst[1].val[2] = v_yy[1].val[1];
            v_dst[1].val[3] = v_v_avg[1];

            // store YUYV
            vst4_u8(&p_dst[dst_idx], v_dst[0]);
            vst4_u8(&p_dst[dst_idx + double_width], v_dst[1]);
        }
#endif
        for (MINT32 j=j8; j<half_width; j++)
        {
            MINT32 dst_idx = tmp_dst_idx + (j<<2);
            MINT32 y_idx = tmp_y_idx + (j<<1);
            MINT32 uv_idx = tmp_uv_idx + j;

            // Y
            p_dst[dst_idx + 0] = p_src_y[y_idx + 0];
            p_dst[dst_idx + 2] = p_src_y[y_idx + 1];
            p_dst[dst_idx + double_width + 0] = p_src_y[y_idx + width + 0];
            p_dst[dst_idx + double_width + 2] = p_src_y[y_idx + width + 1];

            // U & V
            p_dst[dst_idx + 1] = (p_src_u[uv_idx - half_width] + p_src_u[uv_idx]*3 + 2) >> 2;
            p_dst[dst_idx + 3] = (p_src_v[uv_idx - half_width] + p_src_v[uv_idx]*3 + 2) >> 2;
            p_dst[dst_idx + double_width + 1] = (p_src_u[uv_idx]*3 + p_src_u[uv_idx + half_width] + 2) >> 2;
            p_dst[dst_idx + double_width + 3] = (p_src_v[uv_idx]*3 + p_src_v[uv_idx + half_width] + 2) >> 2;
        }

        // next row
        tmp_dst_idx += (width << 2);
        tmp_y_idx += double_width;
        tmp_uv_idx += half_width;
    }

    // the most bottom row
#ifdef NEON_OPT
    j8 = (width >> 4) << 3;
    for (MINT32 j=0; j<j8; j+=8)
    {
        MINT32 dst_idx = tmp_dst_idx + (j<<2);
        MINT32 y_idx = tmp_y_idx + (j<<1);
        MINT32 uv_idx = tmp_uv_idx + j;

        // load Y -> [0]: current line, [1]: next line
        uint8x16_t v_src_y[2];
        v_src_y[0] = vld1q_u8(&p_src_y[y_idx]);
        v_src_y[1] = vld1q_u8(&p_src_y[y_idx + width]);

        // unzip Y -> [0]: current line, [1]: next line
        uint8x8x2_t v_yy[2];
        v_yy[0] = vuzp_u8(vget_low_u8(v_src_y[0]), vget_high_u8(v_src_y[0]));
        v_yy[1] = vuzp_u8(vget_low_u8(v_src_y[1]), vget_high_u8(v_src_y[1]));

        // load U & V -> [0]: previous line, [1]: current line
        uint8x8_t v_u[2], v_v[2];
        v_u[0] = vld1_u8(&p_src_u[uv_idx - half_width]);
        v_u[1] = vld1_u8(&p_src_u[uv_idx]);
        v_v[0] = vld1_u8(&p_src_v[uv_idx - half_width]);
        v_v[1] = vld1_u8(&p_src_v[uv_idx]);

        // filter U & V
        uint16x8_t v_u_tmp, v_v_tmp;
        uint8x8_t v_u_avg, v_v_avg;
        v_u_tmp = vmlal_u8(vmovl_u8(v_u[0]), v_u[1], v_const_3);
        v_v_tmp = vmlal_u8(vmovl_u8(v_v[0]), v_v[1], v_const_3);
        v_u_avg = vrshrn_n_u16(v_u_tmp, 2);
        v_v_avg = vrshrn_n_u16(v_v_tmp, 2);

        // combine YUYV -> [0]: current line, [1]: next line
        uint8x8x4_t v_dst[2];
        v_dst[0].val[0] = v_yy[0].val[0];
        v_dst[0].val[1] = v_u_avg;
        v_dst[0].val[2] = v_yy[0].val[1];
        v_dst[0].val[3] = v_v_avg;
        v_dst[1].val[0] = v_yy[1].val[0];
        v_dst[1].val[1] = v_u[1];
        v_dst[1].val[2] = v_yy[1].val[1];
        v_dst[1].val[3] = v_v[1];

        // store YUYV
        vst4_u8(&p_dst[dst_idx], v_dst[0]);
        vst4_u8(&p_dst[dst_idx + double_width], v_dst[1]);
    }
#endif
    for (MINT32 j=j8; j<half_width; j++)
    {
        MINT32 dst_idx = tmp_dst_idx + (j<<2);
        MINT32 y_idx = tmp_y_idx + (j<<1);
        MINT32 uv_idx = tmp_uv_idx + j;

        // Y
        p_dst[dst_idx + 0] = p_src_y[y_idx + 0];
        p_dst[dst_idx + 2] = p_src_y[y_idx + 1];
        p_dst[dst_idx + double_width + 0] = p_src_y[y_idx + width + 0];
        p_dst[dst_idx + double_width + 2] = p_src_y[y_idx + width + 1];

        // U & V
        p_dst[dst_idx + 1] = (p_src_u[uv_idx - half_width] + p_src_u[uv_idx]*3 + 2) >> 2;
        p_dst[dst_idx + 3] = (p_src_v[uv_idx - half_width] + p_src_v[uv_idx]*3 + 2) >> 2;
        p_dst[dst_idx + double_width + 1] = p_src_u[uv_idx];
        p_dst[dst_idx + double_width + 3] = p_src_v[uv_idx];
    }
    return result;
}

