/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef _CPL_MATH_H_
#define _CPL_MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

#define cpl_min(a, b)   ((a) > (b) ? (b): (a))
#define cpl_max(a, b)   ((a) > (b) ? (a): (b))
#define cpl_abs(a)      ((a) > 0 ? (a) : -(a))

#ifdef __cplusplus
}
#endif

#endif //_CPL_MATH_H_
