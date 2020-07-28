#define LOG_TAG "utilBlur"

#include <stdio.h>
#include <string.h>
#ifdef ANDROID // Android
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else           // OAPC
#define LOGD(...)
#endif /* ANDROID */

#ifdef NEON_OPT
#ifdef ANDROID
#include "arm_neon.h"
#else
#include "neon_template.hpp"
#endif
#endif /* NEON_OPT */

#include "utilBlur.h"
#include "utilMath.h"

// neon debug
//#include "neon_template.hpp"
//#define NEON_OPT
//#define uint8x8_t MUINT8x8
//#define uint16x8_t MUINT16x8

#ifndef NEON_OPT
UTIL_ERRCODE_ENUM utilBlur(MUINT8 *dst, P_UTIL_CLIP_IMAGE_STRUCT src)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 x_offset = src->clip_x;
    MINT32 y_offset = src->clip_y;
    MINT32 width = src->width;
    MUINT8 *p_src = (MUINT8 *)(src->data);
    MUINT8 *p_dst = dst + (width*y_offset + x_offset);

    // data pointer check
    if (!p_src || !p_dst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    MINT32 val;
    MUINT8 *tmp = 0;
    MINT32 y_count = src->clip_height;
    while(--y_count>=0)
    {
        MINT32 x_count = src->clip_width;
        while(--x_count>=0)
        {
            tmp = p_src++;
            val  = (*(tmp  ));      // x1
            val += (*(tmp+1))<<2;   // x4
            val += (*(tmp+2))*6;    // x6
            val += (*(tmp+3))<<2;   // x4
            val += (*(tmp+4));      // x1

            tmp += width;
            val += (*(tmp  ))<<2;   // x4
            val += (*(tmp+1))<<4;   // x16
            val += (*(tmp+2))*24;   // x24
            val += (*(tmp+3))<<4;   // x16
            val += (*(tmp+4))<<2;   // x4

            tmp += width;
            val += (*(tmp  ))*6;    // x6
            val += (*(tmp+1))*24;   // x24
            val += (*(tmp+2))*36;   // x36
            val += (*(tmp+3))*24;   // x24
            val += (*(tmp+4))*6;    // x6

            tmp += width;
            val += (*(tmp  ))<<2;   // x4
            val += (*(tmp+1))<<4;   // x16
            val += (*(tmp+2))*24;   // x24
            val += (*(tmp+3))<<4;   // x16
            val += (*(tmp+4))<<2;   // x4

            tmp += width;
            val += (*(tmp  ));      // x1
            val += (*(tmp+1))<<2;   // x4
            val += (*(tmp+2))*6;    // x6
            val += (*(tmp+3))<<2;   // x4
            val += (*(tmp+4));      // x1

            *p_dst++ = (MUINT8)((val+128)>>8);
        }

        p_src += 2*x_offset;
        p_dst += 2*x_offset;
    }

    return result;
}
#else
UTIL_ERRCODE_ENUM utilBlur(MUINT8 *dst, P_UTIL_CLIP_IMAGE_STRUCT src)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 x_offset = src->clip_x;
    MINT32 y_offset = src->clip_y;
    MINT32 width = src->width;
    MUINT8 *src_Image = (MUINT8 *)(src->data);
    MUINT8 *des_image = dst + (width*y_offset + x_offset);
    MINT32 xcount = src->clip_width;
    MINT32 ycount = src->clip_height;

    // data pointer check
    if (!src_Image || !des_image)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    uint8x8_t row1, row2, row3, row4, row5;
    uint8x8_t left_row, right_row;
    uint16x8_t sum_row;
    uint8x8_t filter4, filter6;

    uint16x8_t filter_4, filter_6;

    uint16x8_t sum_row1 = vdupq_n_u16(0);
    uint16x8_t sum_row2 = vdupq_n_u16(0);
    uint16x8_t sum_row3 = vdupq_n_u16(0);
    uint16x8_t sum_row4 = vdupq_n_u16(0);
    uint16x8_t sum_row5 = vdupq_n_u16(0);

    uint16x8_t sum_row_final;

    uint8x8_t final;  //5x5 result
    uint8x8_t zero = vdup_n_u8(0);

    int i, j, k, w;
    int right_side;
    int val;
    register unsigned char *Image, *tmpImage, *ptrImg, *tmp;


    Image = src_Image;

    filter4 = vdup_n_u8(4);
    filter6 = vdup_n_u8(6);
    filter_4 = vdupq_n_u16(4);
    filter_6 = vdupq_n_u16(6);

    sum_row = vdupq_n_u16(0);
    for(i=0; i<xcount-16; i+=8) //for each stripe
    {
        for(k=0; k<5; k++)
        {
            tmpImage = Image + (k * width);
            left_row = vld1_u8(tmpImage+(i));
            right_row = vld1_u8(tmpImage+i+8);

            row1 = left_row;
            row2 = vext_u8(left_row, right_row,  1);
            row3 = vext_u8(left_row, right_row,  2);
            row4 = vext_u8(left_row, right_row,  3);
            row5 = vext_u8(left_row, right_row,  4);

            sum_row = vmovl_u8(row1);
            sum_row = vmlal_u8(sum_row, row2, filter4);
            sum_row = vmlal_u8(sum_row, row3, filter6);
            sum_row = vmlal_u8(sum_row, row4, filter4);
            sum_row = vaddw_u8(sum_row, row5);

            if(k==0)
                sum_row1 = sum_row;

            else if(k==1)
                sum_row2 = sum_row;

            else if(k==2)
                sum_row3 = sum_row;

            else if(k==3)
                sum_row4 = sum_row;

            else //k==4
                sum_row5 = sum_row;
        }
        sum_row_final = sum_row1;
        sum_row_final = vmlaq_u16(sum_row_final, sum_row2, filter_4);
        sum_row_final = vmlaq_u16(sum_row_final, sum_row3, filter_6);
        sum_row_final = vmlaq_u16(sum_row_final, sum_row4, filter_4);
        sum_row_final = vaddq_u16(sum_row_final, sum_row5);

        final = vrshrn_n_u16(sum_row_final, 8);

        vst1_u8( des_image + (i)  , final);

        for(j=1; j<ycount; j++) //for each line
        {
            sum_row1 = sum_row2;
            sum_row2 = sum_row3;
            sum_row3 = sum_row4;
            sum_row4 = sum_row5;
            //row5
            tmpImage = Image + ((j+4) * width);
            //tmpImage = tmpImage + ((j+4) * width);
            left_row = vld1_u8(tmpImage+(i));
            right_row = vld1_u8(tmpImage+i+8);
            row1 = left_row;
            row2 = vext_u8(left_row, right_row,  1);
            row3 = vext_u8(left_row, right_row,  2);
            row4 = vext_u8(left_row, right_row,  3);
            row5 = vext_u8(left_row, right_row,  4);
            sum_row = vmovl_u8(row1);
            sum_row = vmlal_u8(sum_row, row2, filter4);
            sum_row = vmlal_u8(sum_row, row3, filter6);
            sum_row = vmlal_u8(sum_row, row4, filter4);
            sum_row = vaddw_u8(sum_row, row5);
            sum_row5 = sum_row;
            //SUM
            sum_row_final = sum_row1;
            sum_row_final = vmlaq_u16(sum_row_final, sum_row2, filter_4);
            sum_row_final = vmlaq_u16(sum_row_final, sum_row3, filter_6);
            sum_row_final = vmlaq_u16(sum_row_final, sum_row4, filter_4);
            sum_row_final = vaddq_u16(sum_row_final, sum_row5);

            final = vrshrn_n_u16(sum_row_final, 8);

            vst1_u8( des_image + (i) + ((j)*(width))  , final);

        }
    }

    /*
     * right_side stripe
     */
    right_side = xcount - i;

    if(right_side>0)
    {
        w = width;
        ptrImg = des_image + i;
        tmpImage = Image + i;

        for(j=0; j<ycount; j++)
        {
            for(i=0; i<right_side; i++)
            {
                tmp = tmpImage + i;
                val  = (*(tmp  ));      // x1
                val += (*(tmp+1))<<2;   // x4
                val += (*(tmp+2))*6;    // x6
                val += (*(tmp+3))<<2;   // x4
                val += (*(tmp+4));      // x1

                tmp += w;
                val += (*(tmp  ))<<2;   // x4
                val += (*(tmp+1))<<4;   // x16
                val += (*(tmp+2))*24;   // x24
                val += (*(tmp+3))<<4;   // x16
                val += (*(tmp+4))<<2;   // x4

                tmp += w;
                val += (*(tmp  ))*6;    // x6
                val += (*(tmp+1))*24;   // x24
                val += (*(tmp+2))*36;   // x36
                val += (*(tmp+3))*24;   // x24
                val += (*(tmp+4))*6;    // x6

                tmp += w;
                val += (*(tmp  ))<<2;   // x4
                val += (*(tmp+1))<<4;   // x16
                val += (*(tmp+2))*24;   // x24
                val += (*(tmp+3))<<4;   // x16
                val += (*(tmp+4))<<2;   // x4

                tmp += w;
                val += (*(tmp  ));      // x1
                val += (*(tmp+1))<<2;   // x4
                val += (*(tmp+2))*6;    // x6
                val += (*(tmp+3))<<2;   // x4
                val += (*(tmp+4));      // x1

                *(ptrImg+i) = (MUINT8)((val+128)>>8);
            }
            ptrImg += width;
            tmpImage += width;
        }
    }

    return result;
}
#endif

UTIL_ERRCODE_ENUM utilSobel(MUINT32 *dst, P_UTIL_CLIP_IMAGE_STRUCT src)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT32 width = src->width;
    MUINT32 height = src->height;
    MUINT32 margin_x = src->clip_x;
    MUINT32 margin_y = src->clip_y;
    MUINT8 *p_src = (MUINT8 *)src->data;
    MUINT32 xx, yy;
    MINT32 x, y;
    MINT32 dy, dx;
    MUINT32 width_4, height_4;
    MINT32 value_temp1, value_temp2, value_temp3, value_temp4,value_temp5, value_temp6;
    MINT32 place, place_temp1, place_temp2;
    MUINT32 place_block;

    // data pointer check
    if (!p_src || !dst)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    for(yy=0; yy<16; yy++)
        dst[yy] = 0;  //sperate the preview image to 16 sub-region and count their graident sum

    height_4 = (height-margin_y*2)>>2;  //Calculate the height of each sub-block
    width_4 = (width-margin_x*2)>>2;    //Calculate the width  of each sub-block

    for(yy=0; yy<4 ; yy++){
        for(xx=0; xx<4 ; xx++){
            place_block = (yy<<2) + xx;
            place = (yy*height_4 + margin_y)*width + xx*width_4 + margin_x;
            for(y=height_4; y>0; y-=2){
                for(x=width_4; x>0; x-=2){

                    MUINT8* addr;
                    place_temp1 = place-width;
                    place_temp2 = place+width;
                    addr = p_src+place_temp1-1;
                    value_temp1 = (MINT32)(*addr++);
                    value_temp2 = (MINT32)(*addr++);
                    value_temp3 = (MINT32)(*addr);

                    addr = p_src+place_temp2-1;
                    value_temp4 = (MINT32)(*addr++);
                    value_temp5 = (MINT32)(*addr++);
                    value_temp6 = (MINT32)(*addr);

                    //sobel cofficient
                    //sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}}
                    //sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}
                    dy = -value_temp1 - (value_temp2<<1) - value_temp3 +
                          value_temp4 + (value_temp5<<1) + value_temp6;

                    dx = -value_temp1 + value_temp3
                         -((MINT32)p_src[place-1]<<1) + ((MINT32)p_src[place+1]<<1)
                         -value_temp4 + value_temp6;

                    dy = (UTL_ABS(dx) + UTL_ABS(dy))>>1;
                    dst[place_block] += dy;

                    place+=2;
                }
                place = place - width_4 + width*2;
            }
        }
    }
    return result;
}
