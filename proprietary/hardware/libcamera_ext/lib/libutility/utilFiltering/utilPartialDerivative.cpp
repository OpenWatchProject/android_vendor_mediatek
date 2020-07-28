#define LOG_TAG "utilPartialDerivative"

#include <stdio.h>
#include <string.h>
#ifdef ANDROID // Android
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else           // OAPC
#define LOGD(...)
#endif /* SIM_MAIN */

#ifdef NEON_OPT
#ifdef ANDROID
#include "arm_neon.h"
#else
#include "neon_template.hpp"
#endif
#endif /* NEON_OPT */

#include "utilPartialDerivative.h"

#if !(defined NEON_OPT) || defined(_MSC_VER)
/* For the purpose of speed-up, the leftmost and rightmost column is garbage. */
UTIL_ERRCODE_ENUM utilPartialDerivative(MINT8 *dst_x, MINT8 *dst_y, P_UTIL_BASE_IMAGE_STRUCT src)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *pBlurImage = (MUINT8 *)(src->data);
    MINT8 *pGrdx = dst_x;
    MINT8 *pGrdy = dst_y;
    MUINT32 width = src->width;
    MUINT32 height = src->height;

    MINT32 ub;
    MUINT8 *source_ind1, *source_ind2;
    MINT8 *out_ind_x;
    MINT8 *out_ind_y;

    // data pointer check
    if (!pBlurImage || !pGrdx || !pGrdy)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // horizontal
    ub = height*width - 2*PARTIAL_DER_STEP;
    source_ind1 = pBlurImage;
    out_ind_x = pGrdx + PARTIAL_DER_STEP;
    source_ind2 = pBlurImage + 2*PARTIAL_DER_STEP;
    for(MINT32 ind=ub; ind!=0; ind-- )
    {
        MINT32 temp = (*source_ind1++) - (*source_ind2++);

        // for 8-bit input, set output to 8-bit -128~127
        *out_ind_x = temp >> 1;
        out_ind_x++;
    }

    // vertical
    ub = (height-2)*width*PARTIAL_DER_STEP;
    source_ind1 = pBlurImage;
    out_ind_y = pGrdy + width*PARTIAL_DER_STEP;
    source_ind2 = pBlurImage + 2*PARTIAL_DER_STEP*width;
    for(MINT32 ind=ub; ind!=0; ind-- )
    {
        MINT32 temp = (*source_ind1++) - (*source_ind2++);

        // for 8-bit input, set output to 8-bit -128~127
        *out_ind_y = temp >> 1;
        out_ind_y++;
    }
    // vertical

    return result;
}
#else
__asm void VerticalDerivation(MUINT8* source, MINT32 w, MINT8* out, MINT32 ub)
{
    push {r4};
    add r4, r0, r1;
    add r4, r4, r1;
vertical_loop
    sub r3, r3, #16;
    vld1.8 q0, [r0]!;
    vld1.8 q1, [r4]!;
    vhsub.u8 q2, q0, q1;
    vst1.s8 q2, [r2]!;
    cmp r3, #0;
    bgt vertical_loop

    pop {r4};
    BX lr;
}
__asm void HorizontalDerivation(MUINT8* source, MINT8* out, MINT32 ub)
{
    vld1.8 q0, [r0]!;
horizontal_loop
    sub r2, r2, #16;
    vld1.8 q1, [r0]!;
    vext.8 q2, q0, q1, #2;
    vhsub.u8 q3, q0, q2;
    vst1.s8 q3, [r1]!;
    cmp r2, #0;
    vswp q0, q1;
    bgt horizontal_loop

    BX lr;
}

/* For the purpose of speed-up, the leftmost and rightmost column is garbage. */
UTIL_ERRCODE_ENUM utilPartialDerivative(MINT8 *dst_x, MINT8 *dst_y, P_UTIL_BASE_IMAGE_STRUCT src)
{
    UTIL_ERRCODE_ENUM result = UTIL_OK;
    MUINT8 *pBlurImage = (MUINT8 *)(src->data);
    MINT8 *pGrdx = dst_x;
    MINT8 *pGrdy = dst_y;
    MUINT32 width = src->width;
    MUINT32 height = src->height;

    MINT32 ub;
    MUINT8 *source_ind1;
    MINT8 *out_ind_x;
    MINT8 *out_ind_y;

    // data pointer check
    if (!pBlurImage || !pGrdx || !pGrdy)
    {
        result = UTIL_COMMON_ERR_NULL_BUFFER_POINTER;
        LOGD("[%s] Error Message: %s\n", LOG_TAG, UTIL_GET_ERRCODE_NAME(result));
        return result;
    }

    // horizontal
    ub = height*width - 2*PARTIAL_DER_STEP;
    source_ind1 = pBlurImage;
    out_ind_x = pGrdx + PARTIAL_DER_STEP;
    HorizontalDerivation(source_ind1, out_ind_x, ub);

    // vertical
    ub = (height-2)*width*PARTIAL_DER_STEP;
    source_ind1 = pBlurImage;
    out_ind_y = pGrdy + width*PARTIAL_DER_STEP;
    VerticalDerivation(source_ind1, width, out_ind_y, ub);

    return result;
}
#endif

