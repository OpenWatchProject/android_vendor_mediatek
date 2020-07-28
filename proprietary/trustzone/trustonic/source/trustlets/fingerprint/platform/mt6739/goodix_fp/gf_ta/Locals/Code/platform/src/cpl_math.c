/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <stdint.h>
#include "cpl_math.h"

# define attribute_hidden

typedef union {
    double value;
    struct {
        uint32_t msw;
        uint32_t lsw;
    } parts;
} ieee_double_shape_type;


/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0, ix1, d)  \
do {                                \
  ieee_double_shape_type ew_u;      \
  ew_u.value = (d);                 \
  (ix0) = ew_u.parts.msw;           \
  (ix1) = ew_u.parts.lsw;           \
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d, ix0, ix1)   \
do {                                \
  ieee_double_shape_type iw_u;      \
  iw_u.parts.msw = (ix0);           \
  iw_u.parts.lsw = (ix1);           \
  (d) = iw_u.value;                 \
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i, d)         \
do {                                \
  ieee_double_shape_type gh_u;      \
  gh_u.value = (d);                 \
  (i) = gh_u.parts.msw;             \
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i, d)          \
do {                                \
  ieee_double_shape_type gl_u;      \
  gl_u.value = (d);                 \
  (i) = gl_u.parts.lsw;             \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d, v)          \
do {                                \
  ieee_double_shape_type sl_u;      \
  sl_u.value = (d);                 \
  sl_u.parts.lsw = (v);             \
  (d) = sl_u.value;                 \
} while (0)

static const double tiny = 1.0e-300;
static const double one = 1.00000000000000000000e+00; /* 0x3FF00000, 0x00000000 */
static const double pi = 3.14159265358979311600e+00; /* 0x400921FB, 0x54442D18 */
static const double pio2_hi = 1.57079632679489655800e+00; /* 0x3FF921FB, 0x54442D18 */
static const double pio2_lo = 6.12323399573676603587e-17; /* 0x3C91A626, 0x33145C07 */
static const double pS0 = 1.66666666666666657415e-01; /* 0x3FC55555, 0x55555555 */
static const double pS1 = -3.25565818622400915405e-01; /* 0xBFD4D612, 0x03EB6F7D */
static const double pS2 = 2.01212532134862925881e-01; /* 0x3FC9C155, 0x0E884455 */
static const double pS3 = 4.00555345006794114027e-02; /* 0xBFA48228, 0xB5688F3B */
static const double pS4 = 7.91534994289814532176e-04; /* 0x3F49EFE0, 0x7501B288 */
static const double pS5 = 3.47933107596021167570e-05; /* 0x3F023DE1, 0x0DFDF709 */
static const double qS1 = -2.40339491173441421878e+00; /* 0xC0033A27, 0x1C8A2D4B */
static const double qS2 = 2.02094576023350569471e+00; /* 0x40002AE5, 0x9C598AC8 */
static const double qS3 = -6.88283971605453293030e-01; /* 0xBFE6066C, 0x1B8D0159 */
static const double qS4 = 7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */
static const double huge = 1.0e300;

double cpl_sqrt(double x) {
    double z;
    int32_t sign = (int) 0x80000000;
    int32_t ix0, s0, q, m, t;
    uint32_t r, s1, ix1, q1;

    EXTRACT_WORDS(ix0, ix1, x);

    /* take care of Inf and NaN */
    if ((ix0 & 0x7ff00000) == 0x7ff00000) {
        return x * x + x; /* cpl_sqrt(NaN)=NaN, cpl_sqrt(+inf)=+inf cpl_sqrt(-inf)=sNaN */
    }
    /* take care of zero */
    if (ix0 <= 0) {
        if (((ix0 & (~sign)) | ix1) == 0)
            return x;/* cpl_sqrt(+-0) = +-0 */
        else if (ix0 < 0)
            return 1; /* cpl_sqrt(-ve) = sNaN */
    }
    /* normalize x */
    m = (ix0 >> 20);
    if (m == 0) { /* subnormal x */
        int32_t i;
        while (ix0 == 0) {
            m -= 21;
            ix0 |= (ix1 >> 11);
            ix1 <<= 21;
        }
        for (i = 0; (ix0 & 0x00100000) == 0; i++)
            ix0 <<= 1;
        m -= i - 1;
        ix0 |= (ix1 >> (32 - i));
        ix1 <<= i;
    }
    m -= 1023; /* unbias exponent */
    ix0 = (ix0 & 0x000fffff) | 0x00100000;
    if (m & 1) { /* odd m, double x to make it even */
        ix0 += ix0 + ((ix1 & sign) >> 31);
        ix1 += ix1;
    }
    m >>= 1; /* m = [m/2] */

    /* generate cpl_sqrt(x) bit by bit */
    ix0 += ix0 + ((ix1 & sign) >> 31);
    ix1 += ix1;
    q = q1 = s0 = s1 = 0; /* [q, q1] = cpl_sqrt(x) */
    r = 0x00200000; /* r = moving bit from right to left */

    while (r != 0) {
        t = s0 + r;
        if (t <= ix0) {
            s0 = t + r;
            ix0 -= t;
            q += r;
        }
        ix0 += ix0 + ((ix1 & sign) >> 31);
        ix1 += ix1;
        r >>= 1;
    }

    r = sign;
    while (r != 0) {
        uint32_t t1 = s1 + r;
        t = s0;
        if ((t < ix0) || ((t == ix0) && (t1 <= ix1))) {
            s1 = t1 + r;
            if (((int32_t)(t1 & sign) == sign) && (int32_t)(s1 & sign) == 0)
                s0 += 1;
            ix0 -= t;
            if (ix1 < t1)
                ix0 -= 1;
            ix1 -= t1;
            q1 += r;
        }
        ix0 += ix0 + ((ix1 & sign) >> 31);
        ix1 += ix1;
        r >>= 1;
    }

    /* use floating add to find out rounding direction */
    if ((ix0 | ix1) != 0) {
        z = one - tiny; /* trigger inexact flag */
        if (z >= one) {
            z = one + tiny;
            if (q1 == (uint32_t) 0xffffffff) {
                q1 = 0;
                q += 1;
            } else if (z > one) {
                if (q1 == (uint32_t) 0xfffffffe)
                    q += 1;
                q1 += 2;
            } else {
                q1 += (q1 & 1);
            }
        }
    }
    ix0 = (q >> 1) + 0x3fe00000;
    ix1 = q1 >> 1;
    if ((q & 1) == 1)
        ix1 |= sign;
    ix0 += (m << 20);
    INSERT_WORDS(z, ix0, ix1);
    return z;
}

double cpl_floor(double x) {
    int32_t i0, i1, _j0;
    uint32_t i, j;
    EXTRACT_WORDS(i0, i1, x);
    _j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
    if (_j0 < 20) {
        if (_j0 < 0) { /* raise inexact if x != 0 */
            if (huge + x > 0.0) {/* return 0*sign(x) if |x|<1 */
                if (i0 >= 0) {
                    i0 = i1 = 0;
                } else if (((i0 & 0x7fffffff) | i1) != 0) {
                    i0 = 0xbff00000;
                    i1 = 0;
                }
            }
        } else {
            i = (0x000fffff) >> _j0;
            if (((i0 & i) | i1) == 0)
                return x; /* x is integral */
            if (huge + x > 0.0) { /* raise inexact flag */
                if (i0 < 0)
                    i0 += (0x00100000) >> _j0;
                i0 &= (~i);
                i1 = 0;
            }
        }
    } else if (_j0 > 51) {
        if (_j0 == 0x400)
            return x + x; /* inf or NaN */
        else
            return x; /* x is integral */
    } else {
        i = ((uint32_t) (0xffffffff)) >> (_j0 - 20);
        if ((i1 & i) == 0)
            return x; /* x is integral */
        if (huge + x > 0.0) { /* raise inexact flag */
            if (i0 < 0) {
                if (_j0 == 20) {
                    i0 += 1;
                } else {
                    j = i1 + (1 << (52 - _j0));
                    if ((int32_t) j < i1)
                        i0 += 1; /* got a carry */
                    i1 = j;
                }
            }
            i1 &= (~i);
        }
    }
    INSERT_WORDS(x, i0, i1);
    return x;
}

double cpl_acos(double x) {
    double z, p, q, r, w, s, c, df;
    int32_t hx, ix;
    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x3ff00000) { /* |x| >= 1 */
        uint32_t lx;
        GET_LOW_WORD(lx, x);
        if (((ix - 0x3ff00000) | lx) == 0) { /* |x|==1 */
            if (hx > 0)
                return 0.0; /* acos(1) = 0  */
            else
                return pi + 2.0 * pio2_lo; /* acos(-1)= pi */
        }
        return 1;     /* acos(|x|>1) is NaN */
    }
    if (ix < 0x3fe00000) { /* |x| < 0.5 */
        if (ix <= 0x3c600000)
            return pio2_hi + pio2_lo;/*if|x|<2**-57*/
        z = x * x;
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
        q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        r = p / q;
        return pio2_hi - (x - (pio2_lo - x * r));
    } else if (hx < 0) { /* x < -0.5 */
        z = (one + x) * 0.5;
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
        q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        s = cpl_sqrt(z);
        r = p / q;
        w = r * s - pio2_lo;
        return pi - 2.0 * (s + w);
    } else { /* x > 0.5 */
        z = (one - x) * 0.5;
        s = cpl_sqrt(z);
        df = s;
        SET_LOW_WORD(df, 0);
        c = (z - df * df) / (s + df);
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
        q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        r = p / q;
        w = r * s + c;
        return 2.0 * (df + w);
    }
}
