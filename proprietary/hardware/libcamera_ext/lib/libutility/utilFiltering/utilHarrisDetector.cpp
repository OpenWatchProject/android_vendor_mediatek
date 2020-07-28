#define LOG_TAG "utilHarrisDetector"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <string.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif
#include "utilHarrisDetector.h"

MINT32 GetCornerResponse(MINT8 *grdx, MINT8 *grdy, MINT32 w)
{
    //kappa = FE_Harris_kappa
    MINT32 yy, xx;

    MUINT32 dxavg = 0;
    MUINT32 dyavg = 0;
    MINT32 dxyavg = 0;

    MINT32 Det, Tr;
    MINT32 rc;
    MINT32 tmp;
    MINT32 dx, dy, dxy;
    MINT8 *grdx_ind2,*grdy_ind2;

    for(yy = 4; yy >= 0; yy-- )
    {
        grdx_ind2 = grdx + (yy*w);
        grdy_ind2 = grdy + (yy*w);

        for(xx = 4; xx >= 0; xx-- )
        {
            dx = *grdx_ind2++;
            dy = *grdy_ind2++;

            dxy = dx * dy;
            dx *= dx;
            dy *= dy;

            dxavg  += dx;
            dyavg  += dy;
            dxyavg += dxy;
        }
    }

    dxavg = (denom * dxavg + (1<<(HARRIS_AVG_BITS-1))) >> HARRIS_AVG_BITS;
    dyavg = (denom * dyavg + (1<<(HARRIS_AVG_BITS-1))) >> HARRIS_AVG_BITS;

    dxyavg = (denom * dxyavg) >> HARRIS_AVG_BITS;

    Det = dxavg * dyavg - dxyavg * dxyavg;
    Tr = dxavg + dyavg;
    tmp  = (((KAPPA + 1) * Tr * Tr + (1<<(FE_HARRIS_KAPPA_BITS-1))) >> FE_HARRIS_KAPPA_BITS);
    rc = Det - tmp;

    return rc;
}

UTIL_ERRCODE_ENUM utilHarrisDetector(P_UTIL_CLIP_IMAGE_STRUCT dst, MINT8* src_x, MINT8* src_y, MINT32 *range)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 index,index2;
    MINT32 count=0;
    MINT32 *rc = (MINT32 *)dst->data;
    MINT8 *grdx = src_x;
    MINT8 *grdy = src_y;
    MINT32 *pVarNei = range;
    MINT32 w = dst->width;
    MINT32 h = dst->height;
    MINT32 x_offset = dst->clip_x;
    MINT32 y_offset = dst->clip_y;

    // data pointer check
    if (!rc || !grdx || !grdy)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // init
    memset(rc, 0, w*h*sizeof(MINT32));
    index = y_offset*w+x_offset;

    //for( y = h-6; y !=0 ; y-- )
    MINT32 y_count = h - 2*y_offset;
    while (--y_count>=0)
    {
        // x:(x_offset, w - x_offset - 1)
        MINT32 x_count = w - 2*x_offset;
        while (--x_count>=0)
        {
            if(!( (unsigned)(*(grdx+index)+4) <= 8 ) && !( (unsigned)(*(grdy+index)+4) <= 8 ))
            {
                // start point of GetCornerResponse function
                index2 = index - (2*w+2);
                *(rc+index) = GetCornerResponse(grdx+index2, grdy+index2, w);
                count++;
            }
            index++;
        }
        index += 2*x_offset;
    }

    *pVarNei = count/10000;
    return result;
}
