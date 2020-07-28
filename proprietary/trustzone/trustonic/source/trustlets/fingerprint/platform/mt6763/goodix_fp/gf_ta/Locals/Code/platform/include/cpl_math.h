/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __CPL_MATH_H__
#define __CPL_MATH_H__

#define cpl_min(a, b)   ((a) > (b) ? (b): (a))
#define cpl_max(a, b)   ((a) > (b) ? (a): (b))
#define cpl_abs(a)      ((a) > 0 ? (a) : -(a))
#define cpl_fabs(a)     ((a) > 0.0 ? (a) : -(a))
#define cpl_fabsf(a)    ((a) > 0.0 ? (a) : -(a))



#ifdef __cplusplus
extern "C" {
#endif

// int abs(int);
double cpl_sqrt(double);
double cpl_acos(double);
double cpl_floor(double);

#ifdef __cplusplus
}
#endif

#endif  // __CPL_MATH_H__
