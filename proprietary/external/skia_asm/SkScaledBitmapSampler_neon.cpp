
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorPriv.h"

// 8888

bool Sample_Gray_D8888_neon(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;

    if (deltaSrc == 1)
    {
#if defined(__LP32__) && defined(__arm__)
        asm volatile(
                "pld        [%[src]]                    \n\t"
                "cmp        %[width], #0                \n\t"
                "ble        3f                          \n\t"
                "subs       %[width], %[width], #8      \n\t"
                "blt        2f                          \n\t"
                "vmov.i8    d3, #0xFF                   \n\t"
                "1:                                     \n\t"
                "vld1.8     {d0}, [%[src]]!             \n\t"
                "pld        [%[src], #64]               \n\t"
                "subs       %[width], %[width], #8      \n\t"
                "vmov       d1, d0                      \n\t"
                "vmov       d2, d0                      \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!     \n\t"
                "bge        1b                          \n\t"
                "2:                                     \n\t"
                "add        %[width], %[width], #8      \n\t"
                "3:                                     \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3"
                );
#elif defined(__LP64__) && defined(ARCH_AARCH64)
        asm volatile(
                "prfm       PLDL1KEEP,        [%[src]]                    \n\t"
                "cmp        %[width], #0                \n\t"
                "ble        3f                          \n\t"
                "subs       %[width], %[width], #8      \n\t"
                "blt        2f                          \n\t"
                "movi       v3.8B, #0xFF                   \n\t"
                "1:                                     \n\t"
                "ld1     {v0.8B}, [%[src]],#8             \n\t"
                "prfm       PLDL1KEEP,        [%[src], #64]               \n\t"//???7/14,64or?
                "subs       %[width], %[width], #8      \n\t"
                "mov       v1.8B, v0.8B                      \n\t"
                "mov       v2.8B, v0.8B                      \n\t"
                "st4     {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32     \n\t"
                "bge        1b                          \n\t"
                "2:                                     \n\t"
                "add        %[width], %[width], #8      \n\t"
                "3:                                     \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3"
                );
#else
#endif
    }
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[0], src[0]);
        src += deltaSrc;
    }
    return false;
}

bool Sample_GrayAlpha_D8888_neon(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;
    if (deltaSrc == 2)
    {
#if defined(__LP32__) && defined(__arm__)
    	//use d0,d1,d2,d3,q2,q5,q8,d18
        asm volatile(
                "subs       %[width], %[width], #8                  \n\t"
                "blt        2f                                      \n\t"
                "vmov.i8    d18, #0xFF                              \n\t"
                "vmov.u16   q8, #0x80                               \n\t"
                "1:                                                 \n\t"
                "vld2.8     {d0, d1}, [%[src]]!                     \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "vmull.u8   q2, d0, d1                              \n\t"//a*b
                "vmov       d3, d1                                  \n\t"
                "vadd.u16   q2, q2, q8                              \n\t"//prod = a*b+128
                "vshr.u16   q5, q2, #8                              \n\t"//prod >> 8
                "vaddhn.u16  d0, q2, q5                             \n\t"
                "vmov       d1, d0                                  \n\t"
                "vmov       d2, d0                                  \n\t"
                "vand       d18, d18, d3                            \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "bge        1b                                      \n\t"

                "vmov       r5, r6, d18                             \n\t"
                "and        r5, r6, r5                              \n\t"
                "and        r5, r5, r5, lsr #16                     \n\t"
                "and        r5, r5, r5, lsr #8                      \n\t"
                "mov        %[alphaMask], r5                        \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #8                  \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst), [alphaMask] "+r" (alphaMask)
                :
                :"memory", "r5", "r6", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18"
                );
        
#elif defined(__LP64__) && defined(ARCH_AARCH64)
        //d0->v0,d1->v1,d2->v2,d3->v3,q2->v4,q5->v5,q8->v6,d18->v7
		asm volatile(
				"subs       %[width], %[width], #8                  \n\t"
				"blt        2f                                      \n\t"
				"movi       v7.8B, #0xFF                              \n\t"
				"movi       v6.8H, #0x80                               \n\t"
				"1:                                                 \n\t"
				"ld2        {v0.8B, v1.8B}, [%[src]],#16                     \n\t"
				//"pld        [%[src], #64]                           \n\t"
				"subs       %[width], %[width], #8                  \n\t"
				"umull      v4.8H, v0.8B, v1.8B                              \n\t"//a*b//?7/14,v4.8H or 8B?
				"mov        v3.8B, v1.8B                                  \n\t"
				"add        v4.8H, v4.8H, v6.8H                              \n\t"//prod = a*b+128
				"ushr       v5.8H, v4.8H, #8                              \n\t"//prod >> 8
				"addhn      v0.8B, v4.8H, v5.8H                             \n\t"
				"mov        v1.8B, v0.8B                                  \n\t"
				"mov        v2.8B, v0.8B                                  \n\t"
				"and        v7.8B, v7.8B, v3.8B                            \n\t"
				"st4        {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"//?7/14,32??
				"bge        1b                                      \n\t"

				"mov       w5, v7.s[0]                             \n\t"//??7/14not mov instrc.
				"mov       w6, v7.s[1]                             \n\t"//??7/14not mov instrc.vmov divide into2mov
				"and       w5, w6, w5                              \n\t"
				"and       w5, w5, w5, lsr #16                     \n\t"
				"and       w5, w5, w5, lsr #8                      \n\t"
				"mov       %w[alphaMask], w5                        \n\t"
				"2:                                                 \n\t"
				"add       %[width], %[width], #8                  \n\t"
				:[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst), [alphaMask] "+r" (alphaMask)
				:
				:"memory", "w5", "w6", "d0", "d1", "d2", "d3", "q4", "q5", "q6", "d7"
		);
		
#else
#endif
    }
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[1];
        dst[x] = SkPreMultiplyARGB(alpha, src[0], src[0], src[0]);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

bool Sample_GrayX_D8888_neon(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;

    if (deltaSrc == 2)
    {
#if defined(__LP32__) && defined(__arm__)
        asm volatile(
                "vmov.i8    d3, #0xFF                               \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "blt        2f                                      \n\t"
                "1:                                                 \n\t"
                "vld2.8     {d0, d1}, [%[src]]!                     \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "vmov       d1, d0                                  \n\t"
                "vmov       d2, d0                                  \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "bge        1b                                      \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #8                  \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3"
                );
#elif defined (__LP64__) && defined(ARCH_AARCH64)
        asm volatile(
                "movi       v3.8B, #0xFF                               \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "blt        2f                                      \n\t"
                "1:                                                 \n\t"
                "ld2     {v0.8B, v1.8B}, [%[src]],#16                     \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "mov       v1.8B, v0.8B                                  \n\t"
                "mov       v2.8B, v0.8B                                  \n\t"
                "st4     {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"
                "bge        1b                                      \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #8                  \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3"
                );
#else
#endif

    }
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[0], src[0]);
        src += deltaSrc;
    }
    return false;
}

bool Sample_RGBx_D8888_neon(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    if (deltaSrc == 3)
    {
#if defined(__LP32__) && defined(__arm__)
        asm volatile(
                "pld        [%[src], #0]                            \n\t"
                "cmp        %[width], #0                            \n\t"
                "ble        3f                                      \n\t"
                "vmov.i8    d3, #0xFF                               \n\t"
                "vmov.i8    d7, #0xFF                               \n\t"
                "subs       %[width], %[width], #16                 \n\t"
                "blt        2f                                      \n\t"
                "1:                                                 \n\t"
                "vld3.8     {d0, d1, d2}, [%[src]]!                 \n\t"
                "vld3.8     {d4, d5, d6}, [%[src]]!                 \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #16                 \n\t"

                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "vst4.8     {d4, d5, d6, d7}, [%[dst]]!             \n\t"
                "bge        1b                                      \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #16                 \n\t"
                "cmp        %[width], #8                            \n\t"
                "blt        3f                                      \n\t"
                "vld3.8     {d0, d1, d2}, [%[src]]!                 \n\t"

                "sub       %[width], %[width], #8                   \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "3:                                                 \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
                );
#elif defined (__LP64__) && defined(ARCH_AARCH64)
        asm volatile(
                "prfm       PLDL1KEEP,        [%[src], #0]                            \n\t"
                "cmp        %[width], #0                            \n\t"
                "ble        3f                                      \n\t"
				"movi       v3.8B, #0xFF                               \n\t"
				"movi       v7.8B, #0xFF                               \n\t"
                "subs       %[width], %[width], #16                 \n\t"
                "blt        2f                                      \n\t"
                "1:                                                 \n\t"
				"ld3        {v0.8B, v1.8B, v2.8B}, [%[src]],#24                 \n\t"
				"ld3        {v4.8B, v5.8B, v6.8B}, [%[src]],#24                 \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #16                 \n\t"
              
                "st4        {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"
                "st4        {v4.8B, v5.8B, v6.8B, v7.8B}, [%[dst]],#32             \n\t"
                "bge        1b                                      \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #16                 \n\t"
                "cmp        %[width], #8                            \n\t"
                "blt        3f                                      \n\t"
				"ld3      {v0.8B, v1.8B, v2.8B}, [%[src]],#24                 \n\t"

                "sub       %[width], %[width], #8                   \n\t"
                "st4     {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"
				"3:                                                 \n\t"
				:[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
				:
				:"memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
		);
#else

#endif
    }
    else if (deltaSrc == 4)
    {
#if defined(__LP32__) && defined(__arm__)

        asm volatile(
                "pld        [%[src], #0]                            \n\t"
                "cmp        %[width], #0                            \n\t"
                "ble        3f                                      \n\t"
                "subs       %[width], %[width], #16                 \n\t"
                "blt        2f                                      \n\t"
                "1:                                                 \n\t"
                "vld4.8     {d0, d1, d2, d3}, [%[src]]!             \n\t"
                "vld4.8     {d4, d5, d6, d7}, [%[src]]!             \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "pld        [%[src], #128]                          \n\t"
                //"pld        [%[src], #256]                           \n\t"
                "subs       %[width], %[width], #16                 \n\t"
                "vmov.i8    d3, #0xFF                               \n\t"
                "vmov.i8    d7, #0xFF                               \n\t"

                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "vst4.8     {d4, d5, d6, d7}, [%[dst]]!             \n\t"
                "bge        1b                                      \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #16                 \n\t"
                "cmp        %[width], #8                            \n\t"
                "blt        3f                                      \n\t"
                "vld4.8     {d0, d1, d2, d3}, [%[src]]!             \n\t"

                "sub       %[width], %[width], #8                   \n\t"
                "vmov.i8    d3, #0xFF                               \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "3:                                                 \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
                );

#elif defined(__LP64__) && defined(ARCH_AARCH64)

		asm volatile(
				"prfm       PLDL1KEEP,        [%[src], #0]                            \n\t"
				"cmp        %[width], #0                            \n\t"
				"ble        3f                                      \n\t"
				"subs       %[width], %[width], #16                 \n\t"
				"blt        2f                                      \n\t"
				"1:                                                 \n\t"
				"ld4        {v0.8B, v1.8B, v2.8B, v3.8B}, [%[src]],#32             \n\t"
				"ld4        {v4.8B, v5.8B, v6.8B, v7.8B}, [%[src]],#32             \n\t"
				//"pld        [%[src], #64]                           \n\t"
				"prfm       PLDL1KEEP,        [%[src], #128]                          \n\t"
				//"pld        [%[src], #256]                           \n\t"
				"subs       %[width], %[width], #16                 \n\t"
				"movi       v3.8B, #0xFF                               \n\t"
				"movi       v7.8B, #0xFF                               \n\t"

				"st4        {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"
				"st4        {v4.8B, v5.8B, v6.8B, v7.8B}, [%[dst]],#32             \n\t"
				"bge        1b                                      \n\t"
				"2:                                                 \n\t"
				"add        %[width], %[width], #16                 \n\t"
				"cmp        %[width], #8                            \n\t"
				"blt        3f                                      \n\t"
				"ld4        {v0.8B, v1.8B, v2.8B, v3.8B}, [%[src]],#32             \n\t"

				"sub       %[width], %[width], #8                   \n\t"
				"movi      v3.8B, #0xFF                               \n\t"
				"st4       {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"
                "3:                                                 \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst)
                :
                :"memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
                );
#else
#endif

    }
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return false;
}

bool Sample_RGBA_D8888_neon(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int, const SkPMColor[]) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;

    if (deltaSrc == 4)
    {
#if defined(__LP32__) && defined(__arm__)

    	//use r5,r6,d0,d1,d2,d3,q2,q3,q4,q5,q6,q7,q8,d18,no lap
        asm volatile(
                "subs       %[width], %[width], #8                  \n\t"
                "blt        2f                                      \n\t"
                "vmov.i8    d18, #0xFF                              \n\t"
                "vmov.u16   q8, #0x80                               \n\t"
                "1:                                                 \n\t"
                "vld4.8     {d0, d1, d2, d3}, [%[src]]!             \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "vand       d18, d18, d3                            \n\t"
                "vmull.u8   q2, d0, d3                              \n\t"//a*b
                "vmull.u8   q3, d1, d3                              \n\t"
                "vmull.u8   q4, d2, d3                              \n\t"
                
                "vadd.u16   q2, q2, q8                              \n\t"//prod = a*b+128
                "vadd.u16   q3, q3, q8                              \n\t"
                "vadd.u16   q4, q4, q8                              \n\t"

                "vshr.u16   q5, q2, #8                              \n\t"//prod >> 8
                "vshr.u16   q6, q3, #8                              \n\t"
                "vshr.u16   q7, q4, #8                              \n\t"

                "vaddhn.u16  d0, q2, q5                             \n\t"
                "vaddhn.u16  d1, q3, q6                             \n\t"
                "vaddhn.u16  d2, q4, q7                             \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "bge        1b                                      \n\t"
                "vmov       r5, r6, d18                             \n\t"
                "and        r5, r6, r5                              \n\t"
                "and        r5, r5, r5, lsr #16                     \n\t"
                "and        r5, r5, r5, lsr #8                      \n\t"
                "mov        %[alphaMask], r5                        \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #8                  \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst), [alphaMask] "+r" (alphaMask)
                :
                :"memory", "r5", "r6", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18"
                );
        
#elif defined (__LP64__) && defined(ARCH_AARCH64)
        //use r5,r6,d0->v0,d1->v1,d2->v2,d3->v3,q2->v4,q3->v5,q4->v6,q5->v7,q6->v8,q7->v9,q8->v10,d18->v11
        asm volatile(
                "subs       %[width], %[width], #8                  \n\t"
                "blt        2f                                      \n\t"
                 "movi      v11.8B, 0xFF                  \n\t"
                "movi       v10.8H, #0x80                  \n\t"
                "1:                                                 \n\t"
                "ld4       {v0.8B, v1.8B, v2.8B, v3.8B}, [%[src]], #32    \n\t"
        		//"add        [%[src]], #64                              \n\t"
                //"pld        [%[src], #64]                           \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "and       v11.8B, v11.8B, v3.8B                   \n\t"
                "umull     v4.8H, v0.8B,  v3.8B                              \n\t"//a*b
                "umull     v5.8H, v1.8B,  v3.8B                              \n\t"
                "umull     v6.8H, v2.8B,  v3.8B                              \n\t"
                
                "add      v4.8H, v4.8H, v10.8H                              \n\t"//prod = a*b+128
                "add      v5.8H, v5.8H, v10.8H                              \n\t"
                "add      v6.8H, v6.8H, v10.8H                              \n\t"

                "ushr     v7.8H, v4.8H, #8                              \n\t"//prod >> 8
                "ushr     v8.8H, v5.8H, #8                              \n\t"
                "ushr     v9.8H, v6.8H, #8                              \n\t"

                "addhn    v0.8B, v4.8H, v7.8H                             \n\t"
                "addhn    v1.8B, v5.8H, v8.8H                             \n\t"
                "addhn    v2.8B, v6.8H, v9.8H                             \n\t"
                "st4      {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]], #32             \n\t"
        		//"pld        [%[dst], #64]                           \n\t"
                "bge        1b                                      \n\t"
                "mov        w5, v11.s[0]                             \n\t"
        	    "mov        w6, v11.s[1]                            \n\t"
                "and        w5, w6, w5                            \n\t"
                "and        w5, w5, w5, lsr #16                    \n\t"
                "and        w5, w5, w5, lsr #8                      \n\t"
				"mov        %w[alphaMask], w5                        \n\t"//unsigned int =32bits/wx
                "2:                                                 \n\t"
                "add        %[width], %[width], #8                  \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst), [alphaMask] "+r" (alphaMask)
                :
                :"memory", "w5", "w6", "d0", "d1", "d2", "d3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "d11"
                );
 
#else
#endif
    }
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}


bool Sample_RGBA_D8888_Unpremul_neon(void* SK_RESTRICT dstRow,
                                       const uint8_t* SK_RESTRICT src,
                                       int width, int deltaSrc, int,
                                       const SkPMColor[]) {
    uint32_t* SK_RESTRICT dst = reinterpret_cast<uint32_t*>(dstRow);
    unsigned alphaMask = 0xFF;
    if (deltaSrc == 4)
    {
    	
#if defined(__LP32__) && defined(__arm__)
        asm volatile(
                "subs       %[width], %[width], #8                  \n\t"
                "blt        2f                                      \n\t"
                "vmov.i8    d4, #0xFF                               \n\t"
                "1:                                                 \n\t"
                "vld4.8     {d0, d1, d2, d3}, [%[src]]!             \n\t"
                //"pld        [%[src], #128]                           \n\t"
                "subs       %[width], %[width], #8                  \n\t"
                "vand       d4, d4, d3                              \n\t"
                "vst4.8     {d0, d1, d2, d3}, [%[dst]]!             \n\t"
                "bge        1b                                      \n\t"
                "vmov       r5, r6, d4                              \n\t"
                "and        r5, r6, r5                              \n\t"
                "and        r5, r5, r5, lsr #16                     \n\t"
                "and        r5, r5, r5, lsr #8                      \n\t"
                "mov        %[alphaMask], r5                        \n\t"
                "2:                                                 \n\t"
                "add        %[width], %[width], #8                  \n\t"
                :[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst), [alphaMask] "+r" (alphaMask)
                :
                :"memory", "r5", "r6", "d0", "d1", "d2", "d3", "d4", "d5"
                );
        
#elif defined(__LP64__) && defined(ARCH_AARCH64)
		asm volatile(
				"subs       %[width], %[width], #8                  \n\t"
				"blt        2f                                      \n\t"
				"movi       v4.8B, #0xFF                               \n\t"
				"1:                                                 \n\t"
				"ld4        {v0.8B, v1.8B, v2.8B, v3.8B}, [%[src]],#32             \n\t"
				//"pld        [%[src], #128]                           \n\t"
				"subs       %[width], %[width], #8                  \n\t"
				"and       v4.8B, v4.8B, v3.8B                              \n\t"
				"st4       {v0.8B, v1.8B, v2.8B, v3.8B}, [%[dst]],#32             \n\t"
				"bge        1b                                      \n\t"
				"mov        w5, v4.s[0]                              \n\t"
				"mov        w6, v4.s[1]                              \n\t"
				"and        w5, w6, w5                              \n\t"
				"and        w5, w5, w5, lsr #16                     \n\t"
				"and        w5, w5, w5, lsr #8                      \n\t"
				"mov        %w[alphaMask], w5                        \n\t"
				"2:                                                 \n\t"
				"add        %[width], %[width], #8                  \n\t"
				:[width] "+r" (width), [src] "+r" (src), [dst] "+r" (dst), [alphaMask] "+r" (alphaMask)
				:
				:"memory", "w5", "w6", "d0", "d1", "d2", "d3", "d4", "d5"
		);
		
#else
#endif
    }
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPackARGB32NoCheck(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

// Index

#define A32_MASK_IN_PLACE   (SkPMColor)(SK_A32_MASK << SK_A32_SHIFT)

bool Sample_Index_D8888_neon(void* SK_RESTRICT dstRow,
                               const uint8_t* SK_RESTRICT src,
                       int width, int deltaSrc, int, const SkPMColor ctable[]) {

    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    SkPMColor cc = A32_MASK_IN_PLACE;

    if ((deltaSrc == 1) && (width > 16))
    {
        //while ((((unsigned int)src) & 0x3) !=0)//pointer is diff in 32/64bit.
        while ((((size_t)src) & 0x3) !=0)
        {
            SkPMColor c = ctable[*src];//panjie
            cc &= c;
            *dst++ = c;
            src++;
            width--;
        }
#if defined(__LP32__) && defined(__arm__)
        asm volatile(
                "cmp    %[width], #0                    \n\t"
                "ble    4f                              \n\t"
                "subs   %[width], %[width], #4          \n\t"
                "blt    2f                              \n\t"
                "1:                                     \n\t"
                "ldr    r4, [%[src]], #4                \n\t"
                "subs   %[width], %[width], #4          \n\t"
                "uxtb   r5, r4, ror #8                  \n\t"
                "uxtb   r6, r4, ror #16                 \n\t"
                "uxtb   r7, r4, ror #24                 \n\t"
                "uxtb   r4, r4                          \n\t"
                "ldr    r5, [%[ctable], r5, lsl #2]     \n\t"
                "ldr    r6, [%[ctable], r6, lsl #2]     \n\t"
                "ldr    r7, [%[ctable], r7, lsl #2]     \n\t"
                "ldr    r4, [%[ctable], r4, lsl #2]     \n\t"
                "and    %[cc], %[cc], r5                \n\t"
                "and    %[cc], %[cc], r6                \n\t"
                "and    %[cc], %[cc], r7                \n\t"
                "and    %[cc], %[cc], r4                \n\t"
                "stm    %[dst]!, {r4, r5, r6, r7}       \n\t"
                "bge    1b                              \n\t"
                "2:                                     \n\t"
                "add    %[width], %[width], #4          \n\t"
                "cmp    %[width], #0                    \n\t"
                "ble    4f                              \n\t"
                "sub    %[width], %[width], #1          \n\t"
                "3:                                     \n\t"
                "ldrb   r4, [%[src]], #1                \n\t"
                "subs   %[width], %[width], #1          \n\t"
                "ldr    r4, [%[ctable], r4, lsl #2]     \n\t"
                "and    %[cc], %[cc], r4                \n\t"
                "str    r4, [%[dst]], #4                \n\t"
                "bge    3b                              \n\t"
                "4:                                     \n\t"
                :[width] "+r" (width), [src] "+r" (src), [ctable] "+r" (ctable), [cc] "+r" (cc), [dst] "+r" (dst)
                :
                :"memory", "r4", "r5", "r6", "r7"
                );
        /*
#elif defined (__LP64__) && defined(ARCH_AARCH64)
		asm volatile(
				"cmp    %[width], #0                    \n\t"
				"ble    4f                              \n\t"
				"subs   %[width], %[width], #4          \n\t"
				"blt    2f                              \n\t"
				"1:                                     \n\t"
				"ldr    w4, [%[src]], #4                \n\t"
				"subs   %[width], %[width], #4          \n\t"
				"uxtb   w5, w4, ror #8                  \n\t" //spec use wn
				"uxtb   w6, w4, ror #16                 \n\t"
				"uxtb   w7, w4, ror #24                 \n\t"
				"uxtb   w4, w4                          \n\t"
				"ldr    w5, [%w[ctable], w5, lsl #2]     \n\t"
				"ldr    w6, [%w[ctable], w6, lsl #2]     \n\t"
				"ldr    w7, [%w[ctable], w7, lsl #2]     \n\t"
				"ldr    w4, [%w[ctable], w4, lsl #2]     \n\t"
				"and    %w[cc], %w[cc], w5                \n\t"
				"and    %w[cc], %w[cc], w6                \n\t"
				"and    %w[cc], %w[cc], w7                \n\t"
				"and    %w[cc], %w[cc], w4                \n\t"
				"stm    %[dst]!, {w4, w5, w6, w7}       \n\t"//??7/15 armv8is swhat?
				"bge    1b                              \n\t"
				"2:                                     \n\t"
				"add    %[width], %[width], #4          \n\t"
				"cmp    %[width], #0                    \n\t"
				"ble    4f                              \n\t"
				"sub    %[width], %[width], #1          \n\t"
				"3:                                     \n\t"
				"ldrb   w4, [%[src]], #1                \n\t"//7/15 v8 is LDRB wn, xn/sp #sim
				"subs   %[width], %[width], #1          \n\t"
				"ldr    w4, [%[ctable], w4, lsl #2]     \n\t"//from here ldr to str
				"and    %w[cc], %w[cc], w4                \n\t"
				"str    w4, [%[dst]], #4                \n\t"//spec wn/xn all ok
				"bge    3b                              \n\t"
				"4:                                     \n\t"
				:[width] "+r" (width), [src] "+r" (src), [ctable] "+r" (ctable), [cc] "+r" (cc), [dst] "+r" (dst)
				:
				:"memory", "w4", "w5", "w6", "w7"
		);
		*/
#else
		for (int x = 0; x < width; x++) {
			SkPMColor c = ctable[*src];
			cc &= c;
			dst[x] = c;
			src += deltaSrc;
    }
#endif
    }
    else
    {
        for (int x = 0; x < width; x++) {
            SkPMColor c = ctable[*src];
            cc &= c;
            dst[x] = c;
            src += deltaSrc;
        }
    }

    return cc != A32_MASK_IN_PLACE;
}

