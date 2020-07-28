/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   drError.h
 * @brief  Error id definitions
 */

#ifndef __DRERROR_H__
#define __DRERROR_H__

/**
 * Driver fatal error codes.
 */
/* Declare the error type as a pointer to a struct, so that the compiler can do
 * type checking and emit a warning when we are assign or compare two different
 * error types.
 */
typedef struct drError { int dummy; } * drError_t;

#define E_DR_OK               ((drError_t) 0) /**< Success */
#define E_DR_IPC              ((drError_t) 1) /**< IPC error */
#define E_DR_INTERNAL         ((drError_t) 2) /**< Internal error */
#define E_DR_BUSY             ((drError_t) 3) /**< Expect to be called again in same thread state */

/* session open error (range 0x100:0x1ff)*/
#define E_DR_OPEN_SESSION          ((drError_t) 0x100)
#define E_DR_TOUCH_OPEN            ((drError_t) 0x101)
#define E_DR_FB_MAP_FB             ((drError_t) 0x102)
#define E_DR_FB_MAP_WB             ((drError_t) 0x103)
#define E_DR_FB_PROTECT_FB         ((drError_t) 0x104)
#define E_DR_FB_PROTECT_WB         ((drError_t) 0x105)
#define E_DR_DISPLAY_MAP           ((drError_t) 0x106)
#define E_DR_DISPLAY_PROTECT       ((drError_t) 0x107)
#define E_DR_DISPLAY_INITIALIZE    ((drError_t) 0x108)
#define E_DR_TOUCH_INFO            ((drError_t) 0x109)
#define E_DR_TOUCH_NOT_ENOUGH_MEMORY ((drError_t) 0x10A)

/* session closing error (range 0x200:0x2ff)*/
#define E_DR_CLOSE_SESSION         ((drError_t) 0x200)
#define E_DR_TOUCH_CLOSE           ((drError_t) 0x201)
#define E_DR_FB_UNMAP_FB           ((drError_t) 0x202)
#define E_DR_FB_UNMAP_WB           ((drError_t) 0x203)
#define E_DR_FB_UNPROTECT_FB       ((drError_t) 0x204)
#define E_DR_FB_UNPROTECT_WB       ((drError_t) 0x205)
#define E_DR_DISPLAY_UNMAP         ((drError_t) 0x206)
#define E_DR_DISPLAY_UNPROTECT     ((drError_t) 0x207)
#define E_DR_DISPLAY_UNINITIALIZE  ((drError_t) 0x208)

/* session ongoing error (range 0x300:0x3ff)*/
#define E_DR_MISSED_TOUCH_EVENTS   ((drError_t) 0x300)
    /* ... add more error codes when required */


#endif // __DRERROR_H__


