/******************************************************************************
 *
 *  $Id: AKMDeviceF.h 300 2015-01-05 05:31:57Z suzuki.thj $
 *
 * -- Copyright Notice --
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 * -- End Asahi Kasei Microdevices Copyright Notice --
 *
 ******************************************************************************/
#ifndef AKSC_INC_AKMDEVICEF_H
#define AKSC_INC_AKMDEVICEF_H

#include "AKConfigure.h"
#include <math.h>

#ifdef AKSC_MATH_DOUBLE
typedef double AKSC_FLOAT;
#else
typedef float AKSC_FLOAT;
#endif

#ifdef AKSC_USE_STD_FLOAT
#include <float.h>
#ifdef AKSC_MATH_DOUBLE
#define AKSC_EPSILON    DBL_EPSILON
#define AKSC_FLTMAX     DBL_MAX
#else
#define AKSC_EPSILON    FLT_EPSILON
#define AKSC_FLTMAX     FLT_MAX
#endif
#else
#ifdef AKSC_MATH_DOUBLE
#define AKSC_EPSILON    ((AKSC_FLOAT)2.2204460492503131e-016)
#define AKSC_FLTMAX     ((AKSC_FLOAT)1.7976931348623158e+308)
#else
#define AKSC_EPSILON    ((AKSC_FLOAT)1.192092896e-07F)
#define AKSC_FLTMAX     ((AKSC_FLOAT)3.402823466e+38F)
#endif
#endif

#define AKSC_PI     ((AKSC_FLOAT)3.14159265358979323846264338)

//========================= macro definition =================================//
#ifdef AKSC_MATH_DOUBLE
//  inplementation double
#define AKSC_COSF(ang)      cos((ang))
#define AKSC_SINF(ang)      sin((ang))
#define AKSC_ATAN2F(y, x)    atan2((y), (x))
#define AKSC_ACOSF(x)       acos(x)
#define AKSC_FABSF(val)     fabs((val))
#define AKSC_SQRTF(val)     sqrt((val))
#define AKSC_POWF(x, y)      pow((x), (y))
#else
//  inplementation float
#define AKSC_COSF(ang)      cosf((ang))
#define AKSC_SINF(ang)      sinf((ang))
#define AKSC_ATAN2F(y, x)    atan2f((y), (x))
#define AKSC_ACOSF(x)       acosf(x)
#define AKSC_FABSF(val)     fabsf((val))
#define AKSC_SQRTF(val)     sqrtf((val))
#define AKSC_POWF(x, y)      powf((x), (y))
#endif

//========================= Vectors =====================================//
// AKSC_FLOAT-precision floating-point 3 dimensional vector
typedef union _AKSC_FVEC {
    struct {
        AKSC_FLOAT x;
        AKSC_FLOAT y;
        AKSC_FLOAT z;
    } u;
    AKSC_FLOAT v[3];
} AKSC_FVEC;

//========================= Matrix ======================================//
// AKSC_FLOAT-precision floating-point 3x3 matrix
typedef union _AKSC_FMAT {
    struct {
        AKSC_FLOAT _11, _12, _13;
        AKSC_FLOAT _21, _22, _23;
        AKSC_FLOAT _31, _32, _33;
    } u;
    AKSC_FLOAT m[3][3];
} AKSC_FMAT;

//========================= Quaternion ===================================//
// AKSC_FLOAT-precision floating-point Quaternion
typedef union _AKSC_FQUAT {
    struct {
        AKSC_FLOAT w;
        AKSC_FLOAT x;
        AKSC_FLOAT y;
        AKSC_FLOAT z;
    } u;
    AKSC_FLOAT q[4];
} AKSC_FQUAT;

#endif

