#define LOG_TAG "utilImageArithmetic"

#if defined(__ANDROID__) || defined(ANDROID)
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // OAPC
#include <stdio.h>
#include <string.h>
#define LOGD(...)
#endif /* SIM_MAIN */

#ifdef NEON_OPT
#ifdef ANDROID
#include "arm_neon.h"
#else
#include "neon_template.hpp"
#endif
#endif /* NEON_OPT */

#include "utilImageArithmetic.h"
#include "utilMath.h"

UTIL_ERRCODE_ENUM ImageSubstract(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* B, MINT32 residue, UTIL_BASE_IMAGE_STRUCT* C)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 i;
    MUINT8* Adata;
    MUINT8* Bdata;
    MUINT8* Cdata;

    C->height = A->height;
    C->width = A->width;

    Adata = (MUINT8 *)A->data;
    Bdata = (MUINT8 *)B->data;
    Cdata = (MUINT8 *)C->data;

#ifndef NEON_OPT
    for(i=C->width*C->height;i!=0; i--)
    {
        int value = (*Adata) - (*Bdata) + residue;
        if(value > 255)
            value = 255;
        if(value < 0)
            value = 0;
        *Cdata = value;
        Adata++;
        Bdata++;
        Cdata++;
    }
#else

    uint8x8_t u8_A_neon, u8_B_neon;
    uint16x8_t u16_A_neon, u16_B_neon;
    int16x8_t s16_A_neon, s16_B_neon;
    int16x8_t s16_resudue = vdupq_n_s16(residue);
    for(i=C->width*C->height;i>7; i-=8)
    {
        u8_A_neon = vld1_u8(Adata);
        Adata+=8;

        u8_B_neon = vld1_u8(Bdata);
        Bdata+=8;

        u16_A_neon = vmovl_u8(u8_A_neon);
        u16_B_neon = vmovl_u8(u8_B_neon);

        s16_A_neon = vreinterpretq_s16_u16(u16_A_neon);
        s16_B_neon = vreinterpretq_s16_u16(u16_B_neon);

        s16_A_neon = vsubq_s16(s16_A_neon, s16_B_neon);
        s16_A_neon = vaddq_s16(s16_A_neon, s16_resudue);
        u8_A_neon = vqmovun_s16(s16_A_neon);

        vst1_u8(Cdata,u8_A_neon);
        Cdata+=8;
    }
    for(;i!=0; i--)
    {
        int value = (*Adata) - (*Bdata) + residue;
        if(value > 255)
            value = 255;
        if(value < 0)
            value = 0;
        *Cdata = value;
        Adata++;
        Bdata++;
        Cdata++;
    }
#endif

    return result;

}

UTIL_ERRCODE_ENUM ImageAdd(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* B, MINT32 residue, UTIL_BASE_IMAGE_STRUCT* C)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 i;
    MUINT8* Adata;
    MUINT8* Bdata;
    MUINT8* Cdata;

    C->height = A->height;
    C->width = A->width;

    Adata = (MUINT8 *)A->data;
    Bdata = (MUINT8 *)B->data;
    Cdata = (MUINT8 *)C->data;
#ifndef NEON_OPT
    for(i=C->width*C->height;i!=0; i--)
    {
        int value = (*Adata) + (*Bdata) + residue;
        if(value > 255)
            value = 255;
        if(value < 0)
            value = 0;
        *Cdata = value;
        Adata++;
        Bdata++;
        Cdata++;
    }
#else
    uint8x8_t u8_A_neon, u8_B_neon;
    uint16x8_t u16_A_neon, u16_B_neon;
    int16x8_t s16_A_neon, s16_B_neon;
    int16x8_t s16_resudue = vdupq_n_s16(residue);
    for(i=A->width*A->height;i>7; i-=8)
    {
        u8_A_neon = vld1_u8(Adata);
        Adata+=8;

        u8_B_neon = vld1_u8(Bdata);
        Bdata+=8;

        u16_A_neon = vmovl_u8(u8_A_neon);
        u16_B_neon = vmovl_u8(u8_B_neon);

        s16_A_neon = vreinterpretq_s16_u16(u16_A_neon);
        s16_B_neon = vreinterpretq_s16_u16(u16_B_neon);

        s16_A_neon = vaddq_s16(s16_A_neon, s16_B_neon);
        s16_A_neon = vaddq_s16(s16_A_neon, s16_resudue);
        u8_A_neon = vqmovun_s16(s16_A_neon);

        vst1_u8(Cdata,u8_A_neon);
        Cdata+=8;
    }
    for(;i!=0; i--)
    {
        int value = (*Adata) + (*Bdata) + residue;
        if(value > 255)
            value = 255;
        if(value < 0)
            value = 0;
        *Cdata = value;
        Adata++;
        Bdata++;
        Cdata++;
    }
#endif

    return result;
}

UTIL_ERRCODE_ENUM utilImageSad(P_UTIL_IMAGE_SAD_STRUCT sad_data, MUINT32 *sum, MUINT32 *count)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *pImg1X, *pImg1Y, *pImg2X, *pImg2Y;
    MUINT32 xoffset, yoffset1, yoffset2;
    MUINT32 SadSum = 0, Count = 0;
    MUINT32 x_count, y_count, i;
    MINT16 saturation_value = sad_data->saturation_value;
    P_UTIL_CLIP_IMAGE_STRUCT src1 = &sad_data->src1;
    P_UTIL_BASE_IMAGE_STRUCT src2 = &sad_data->src2;
    MINT32 h = src1->clip_x;
    MINT32 v = src1->clip_y;

    pImg1Y = (MUINT8 *)src1->data;
    pImg2Y = (MUINT8 *)src2->data;
    yoffset1 = sad_data->sub_h * src1->width;
    yoffset2 = sad_data->sub_h * src2->width;
    xoffset = sad_data->sub_w;

    if (v >= 0)
    {
        pImg1Y += v*src1->width;
        y_count = (src1->clip_height-1-v)/(sad_data->sub_h) + 1;
    }
    else
    {
        pImg2Y += (-v)*src1->width;
        y_count = (src1->clip_height-1+v)/(sad_data->sub_h) + 1;
    }
    if (h >= 0)
    {
        x_count = (src1->clip_width-1-h)/(sad_data->sub_w) + 1;
        pImg1Y += h;
    }
    else
    {
        x_count = (src1->clip_width-1+h)/(sad_data->sub_w) + 1;
        pImg2Y -= h;
    }

    for (; y_count!=0; y_count--)
    {
        pImg1X = pImg1Y;
        pImg2X = pImg2Y;
        for(i = x_count; i>3; i-=4)
        {
            Count+=4;

            if(saturation_value==0)
            {
                SadSum += UTL_ABS((MINT32)*pImg2X - (MINT32)*pImg1X);
                pImg1X+=xoffset;
                pImg2X+=xoffset;

                SadSum += UTL_ABS((MINT32)*pImg2X - (MINT32)*pImg1X);
                pImg1X+=xoffset;
                pImg2X+=xoffset;

                SadSum += UTL_ABS((MINT32)*pImg2X - (MINT32)*pImg1X);
                pImg1X+=xoffset;
                pImg2X+=xoffset;

                SadSum += UTL_ABS((MINT32)*pImg2X - (MINT32)*pImg1X);
                pImg1X+=xoffset;
                pImg2X+=xoffset;
            }
            else
            {
                if((MINT32)(*pImg2X)==saturation_value)
                {
                    SadSum += 0;
                    Count--;
                }
                else
                    SadSum += UTL_ABS((MINT32)(*pImg2X) - (MINT32)(*pImg1X));

                pImg1X+=xoffset;
                pImg2X+=xoffset;

                if((MINT32)(*pImg2X)==saturation_value)
                {
                    SadSum += 0;
                    Count--;
                }
                else
                    SadSum += UTL_ABS((MINT32)(*pImg2X) - (MINT32)(*pImg1X));

                pImg1X+=xoffset;
                pImg2X+=xoffset;

                if((MINT32)(*pImg2X)==saturation_value)
                {
                    SadSum += 0;
                    Count--;
                }
                else
                    SadSum += UTL_ABS((MINT32)(*pImg2X) - (MINT32)(*pImg1X));

                pImg1X+=xoffset;
                pImg2X+=xoffset;

                if((MINT32)(*pImg2X)==saturation_value)
                {
                    SadSum += 0;
                    Count--;
                }
                else
                    SadSum += UTL_ABS((MINT32)(*pImg2X) - (MINT32)(*pImg1X));

                pImg1X+=xoffset;
                pImg2X+=xoffset;
            }
        }
        for (; i!=0; i--)
        {
            SadSum += UTL_ABS((MINT32)*pImg2X - (MINT32)*pImg1X);
            Count+=1;

            pImg1X+=xoffset;
            pImg2X+=xoffset;
        }
        pImg1Y+=yoffset1;
        pImg2Y+=yoffset2;
    }

    *sum = SadSum;
    *count = Count;

    return result;
}

