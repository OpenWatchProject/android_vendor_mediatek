/* @(#)s_isnan.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */



#include "math.h"
#include "math_private.h"

#undef isnan
int
isnan (double x)
{
  int32_t hx, lx;
  EXTRACT_WORDS (hx, lx, x);
  hx &= 0x7fffffff;
  hx |= (uint32_t) (lx | (-lx)) >> 31;
  hx = 0x7ff00000 - hx;
  return (int) (((uint32_t) hx) >> 31);
}
