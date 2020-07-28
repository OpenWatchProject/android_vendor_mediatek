/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __RTST_LOG_H__
#define __RTST_LOG_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <Log.h>

/*****************************************************************************
 * Example
 *****************************************************************************/
/* Add a debug log with your tag, write it like:
 * RTST_LOG_D(tag, "this is a sample");
 *
 * Print a variable, write it like:
 * RTST_LOG_D(tag, "this is a sample %d", variable);
 *
 * Print multi-variable, write it like:
 * RTST_LOG_D(tag, "this is a sample %d,%d", variable1, variable2);
 *
 * Staple output format
 * %c  char
 * %s  char* string
 * %d  sign decimal
 * %p  pointer
 * %x  hex
 *
 * Add a debug log with your condition and tag, write it like:
 * RTST_LOG_D_IF(condition, tag, "this is a sample");
 * When condition is not 0 (this is true), the log will be printed, otherwise, no log printed.
 *
 */

/*****************************************************************************
 * Define
 *****************************************************************************/

/*
 * Simplified macro to send a verbose radio log message using the user given tag - _rtst_tag.
 */
#ifndef RTST_LOG_V
#define __RTST_LOG_V(_rtst_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_VERBOSE, _rtst_tag, __VA_ARGS__))
#if LOG_NDEBUG
#define RTST_LOG_V(_rtst_tag, ...) do { if (0) { __RTST_LOG_V(_rtst_tag, __VA_ARGS__); } } while (0)
#else
#define RTST_LOG_V(_rtst_tag, ...) __RTST_LOG_V(_rtst_tag, __VA_ARGS__)
#endif
#endif

#define CONDITION(cond)     (__builtin_expect((cond) != 0, 0))

#ifndef RTST_LOG_V_IF
#if LOG_NDEBUG
#define RTST_LOG_V_IF(cond, _rtst_tag, ...)   ((void)0)
#else
#define RTST_LOG_V_IF(cond, _rtst_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_VERBOSE, _rtst_tag, __VA_ARGS__)) \
    : (void)0 )
#endif
#endif

/*
 * Simplified macro to send a debug radio log message using the user given tag - _rtst_tag.
 */
#ifndef RTST_LOG_D
#define RTST_LOG_D(_rtst_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_DEBUG, _rtst_tag, __VA_ARGS__))
#endif

#ifndef RTST_LOG_D_IF
#define RTST_LOG_D_IF(cond, _rtst_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_DEBUG, _rtst_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

/*
 * Simplified macro to send an info radio log message using the user given tag - _rtst_tag.
 */
#ifndef RTST_LOG_I
#define RTST_LOG_I(_rtst_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_INFO, _rtst_tag, __VA_ARGS__))
#endif

#ifndef RTST_LOG_I_IF
#define RTST_LOG_I_IF(cond, _rtst_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_INFO, _rtst_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

/*
 * Simplified macro to send a warning radio log message using the user given tag - _rtst_tag.
 */
#ifndef RTST_LOG_W
#define RTST_LOG_W(_rtst_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_WARN, _rtst_tag, __VA_ARGS__))
#endif

#ifndef RTST_LOG_W_IF
#define RTST_LOG_W_IF(cond, _rtst_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_WARN, _rtst_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

/*
 * Simplified macro to send an error radio log message using the user given tag - _rtst_tag.
 */
#ifndef RTST_LOG_E
#define RTST_LOG_E(_rtst_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_ERROR, _rtst_tag, __VA_ARGS__))
#endif

#ifndef RTST_LOG_E_IF
#define RTST_LOG_E_IF(cond, _rtst_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_ERROR, _rtst_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef RTST_ASSERT
#define RTST_ASSERT(_expr)                                                         \
    do {                                                                          \
        if (!(_expr)) {                                                           \
            RTST_LOG_E("RTST_ASSERT", "RTST_ASSERT:%s, %d", __FILE__, __LINE__);     \
            LOG_ALWAYS_FATAL();                                        \
        }                                                                         \
    } while(0)
#endif

#endif /* __RTST_LOG_H__ */

