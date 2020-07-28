#include <stdio.h>
#include <string.h>
#include "utilMemOp.h"

#define NEON_OPT

#ifdef NEON_OPT
#ifdef ANDROID
#include "arm_neon.h"
#else
#include "neon_template.hpp"
#endif
#endif

static const MINT32 g_simd_bit = 4;
static const MINT32 g_step = 1 << g_simd_bit;

MINT32 vmemcpy(void *_Dst, void *_Src, MUINT32 Size)
{
    MINT32 ret_code = UTIL_OK;
    MUINT8 *p_dst = (MUINT8 *)_Dst;
    MUINT8 *p_src = (MUINT8 *)_Src;
  
    #ifdef NEON_OPT
    MINT32 i_simd, simd_bit = g_simd_bit, step = g_step, rest_size;
    i_simd = Size >> simd_bit;
    rest_size = Size - (i_simd << simd_bit);
    while(i_simd-- != 0)
    {
        uint8x16_t v_data = vld1q_u8(p_src);
        p_src += step;
        vst1q_u8(p_dst, v_data);
        p_dst += step;
    }
    if (rest_size != 0) memcpy(p_dst, p_src, rest_size);
    #else
    memcpy(p_dst, p_src, Size);
    #endif

    return ret_code;
}

MINT32 vmemset(void *_Dst, MINT32 Value, MUINT32 Size)
{
    MINT32 ret_code = UTIL_OK;
    MUINT8 *p_dst = (MUINT8 *)_Dst;

    #ifdef NEON_OPT
    MINT32 i_simd, simd_bit = g_simd_bit, step = g_step, rest_size;
    uint8x16_t v_data = vdupq_n_u8(Value);
    i_simd = Size >> simd_bit;
    rest_size = Size - (i_simd << simd_bit);
    while(i_simd-- != 0)
    {
        vst1q_u8(p_dst, v_data);
        p_dst += step;
    }
    if (rest_size != 0) memset(p_dst, Value, rest_size);
    #else
    memset(p_dst, Value, Size);
    #endif

    return ret_code;
}