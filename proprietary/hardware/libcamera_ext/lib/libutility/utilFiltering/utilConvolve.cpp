#define LOG_TAG "utilConvolve"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif
#include "utilConvolve.h"


UTIL_ERRCODE_ENUM Convolve(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* kernel, UTIL_BASE_IMAGE_STRUCT* C)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MINT32 i, j, ii, jj, k_center_height, k_center_width;
    MINT32 width, height;
    MINT32 kernel_height, kernel_width;
    UTIL_BASE_IMAGE_STRUCT MA, MKernel;
    MUINT8 *pOut;

    MA.data = A->data;
    MA.height = A->height;
    MA.width = A->width;

    width = A->width;
    height = A->height;

    MKernel.data = kernel->data;
    MKernel.width = kernel->width;
    MKernel.height = kernel->height;

    kernel_height = kernel->height;
    kernel_width = kernel->width;

    C->width = A->width;
    C->height = A->height;

    pOut = (MUINT8 *)(C->data);

    // data pointer check
    if (!MA.data || !MKernel.data || !pOut)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    k_center_height = kernel->height/2;
    k_center_width = kernel->width/2;

    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            int sum = 0;

            for(ii=0;ii<kernel_height;ii++)
            {
                for(jj=0;jj<kernel_width;jj++)
                {
                    int r = i + (ii - k_center_height);
                    int c = j  + (jj - k_center_width);

                    if (r<0)
                        r=-r;
                    else if (height <= r)
                        r = 2*height - r - 2;

                    if (c<0)
                        c=-c;
                    else if (width <= c)
                        c = 2*width - c - 2;

                    sum += UTIL_ELM(MA,r,c) * UTIL_ELM(MKernel,ii,jj);
                }
            }
            sum = (sum + 2)>>2;                            // cause gaussian kernel's weighting is 4  (3x3)
            *pOut = (MUINT8)(sum);
            pOut++;
        }
    }

    return result;
}

