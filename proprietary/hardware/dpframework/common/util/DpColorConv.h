#ifndef __DP_COLOR_CONV_H__
#define __DP_COLOR_CONV_H__

#define UTIL_USE_FIXED_POINT        1

// Macros to convert RGB565 to RGB888
#define RGB565_TO_RGB888_R(VALUE)	(((VALUE) & 0x0F800) >> 8)
#define RGB565_TO_RGB888_G(VALUE)   (((VALUE) & 0x007E0) >> 3)
#define RGB565_TO_RGB888_B(VALUE)   (((VALUE) & 0x0001F) << 3)

// Macro to convert RGB888 to RGB565
#define RGB888_TO_RGB565(R, G, B)   ((((R >> 3) & 0x01F) << 11) | (((G >> 2) & 0x03F) << 5) | (((B >> 3) & 0x01F) << 0))

// Macros to convert BGR565 to RGB888
#define BGR565_TO_RGB888_R(VALUE)   (((VALUE) & 0x0001F) << 3)
#define BGR565_TO_RGB888_G(VALUE)   (((VALUE) & 0x007E0) >> 3)
#define BGR565_TO_RGB888_B(VALUE)	(((VALUE) & 0x0F800) >> 8)

// Macro to convert RGB888 to BGR565
#define RGB888_TO_BGR565(R, G, B)   ((((B >> 3) & 0x01F) << 11) | (((G >> 2) & 0x03F) << 5) | (((R >> 3) & 0x01F) << 0))

#if UTIL_USE_FIXED_POINT
#define UTIL_ROUND_CLAMP(value)     ((value) > 255.0? 255: (value) < 0? 0: (value))

// Macros to convert RGB to YUV
#define RGB888_TO_YUV_Y(R, G, B)    UTIL_ROUND_CLAMP((  4898 * (R) + 9617 * (G) + 1867 * (B)) >> 14       )
#define RGB888_TO_YUV_U(R, G, B)    UTIL_ROUND_CLAMP(((-2763 * (R) - 5428 * (G) + 8192 * (B)) >> 14) + 128)
#define RGB888_TO_YUV_V(R, G, B)    UTIL_ROUND_CLAMP((( 8192 * (R) - 6860 * (G) - 1332 * (B)) >> 14) + 128)

// Macros to convert YUV to RGB
#define YUV_TO_RGB888_R(Y, Cb, Cr)  UTIL_ROUND_CLAMP((16384 * (Y)                        + 22970 * ((Cr) - 128)) >> 14)
#define YUV_TO_RGB888_G(Y, Cb, Cr)  UTIL_ROUND_CLAMP((16384 * (Y) -  5638 * ((Cb) - 128) - 11700 * ((Cr) - 128)) >> 14)
#define YUV_TO_RGB888_B(Y, Cb, Cr)  UTIL_ROUND_CLAMP((16384 * (Y) + 29032 * ((Cb) - 128)                       ) >> 14)
#else
#define UTIL_ROUND_CLAMP(value)     ((value) > 255.0? 255: (value) < 0.0? 0: (int32_t)((value) + 0.5))

// Macros to convert RGB to YUV
#define RGB888_TO_YUV_Y(R, G, B)    UTIL_ROUND_CLAMP( 0.299  * (R) + 0.587  * (G) + 0.114  * (B)        )
#define RGB888_TO_YUV_U(R, G, B)    UTIL_ROUND_CLAMP(-0.1687 * (R) - 0.3313 * (G) + 0.5    * (B) + 128.0)
#define RGB888_TO_YUV_V(R, G, B)    UTIL_ROUND_CLAMP( 0.5    * (R) - 0.4187 * (G) - 0.0813 * (B) + 128.0)

// Macros to convert YUV to RGB
#define YUV_TO_RGB888_R(Y, Cb, Cr)  UTIL_ROUND_CLAMP((Y) +                                1.402 * ((Cr) - 128.0))
#define YUV_TO_RGB888_G(Y, Cb, Cr)  UTIL_ROUND_CLAMP((Y) - 0.344136 * ((Cb) - 128.0) - 0.714136 * ((Cr) - 128.0))
#define YUV_TO_RGB888_B(Y, Cb, Cr)  UTIL_ROUND_CLAMP((Y) +    1.772 * ((Cb) - 128.0))
#endif

#define FLOAT_TO_FIXED(ft, prec)    ((int32_t)roundf((float)(ft) * (float)(1 << (prec))))
#define FIXED_TO_FLOAT(fx, prec)    ((float)(fx) / (float)(1 << (prec)))

#define FLOAT_TO_UNSIGNED_FIXED(ft, n, p)   (FLOAT_TO_FIXED(ft, p) & ((1 << ((n) + (p))) - 1))
#define FLOAT_TO_SIGNED_FIXED(ft, n, p)     (FLOAT_TO_FIXED(ft, p) & ((1 << (1 + (n) + (p))) - 1))

#endif  // __DP_COLOR_CONV_H__
