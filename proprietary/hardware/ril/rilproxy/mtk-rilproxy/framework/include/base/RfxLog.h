/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __RFX_LOG_H__
#define __RFX_LOG_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <Log.h>

/*****************************************************************************
 * Example
 *****************************************************************************/
/* Add a debug log with your tag, write it like:
 * RFX_LOG_D(tag, "this is a sample");
 *
 * Print a variable, write it like:
 * RFX_LOG_D(tag, "this is a sample %d", variable);
 *
 * Print multi-variable, write it like:
 * RFX_LOG_D(tag, "this is a sample %d,%d", variable1, variable2);
 *
 * Staple output format
 * %c  char
 * %s  char* string
 * %d  sign decimal
 * %p  pointer
 * %x  hex
 *
 * Add a debug log with your condition and tag, write it like:
 * RFX_LOG_D_IF(condition, tag, "this is a sample");
 * When condition is not 0 (this is true), the log will be printed, otherwise, no log printed.
 *
 */

/*****************************************************************************
 * Define
 *****************************************************************************/

// for log reduction
typedef enum {
    LOG_VERBOSE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

/*
 * Simplified macro to send a verbose radio log message using the user given tag - _rfx_tag.
 */
#ifndef RFX_LOG_V
#define __RFX_LOG_V(_rfx_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_VERBOSE, _rfx_tag, __VA_ARGS__))
#if LOG_NDEBUG
#define RFX_LOG_V(_rfx_tag, ...) do { if (0) { __RFX_LOG_V(_rfx_tag, __VA_ARGS__); } } while (0)
#else
#define RFX_LOG_V(_rfx_tag, ...) __RFX_LOG_V(_rfx_tag, __VA_ARGS__)
#endif
#endif

#define CONDITION(cond)     (__builtin_expect((cond) != 0, 0))

#ifndef RFX_LOG_V_IF
#if LOG_NDEBUG
#define RFX_LOG_V_IF(cond, _rfx_tag, ...)   ((void)0)
#else
#define RFX_LOG_V_IF(cond, _rfx_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_VERBOSE, _rfx_tag, __VA_ARGS__)) \
    : (void)0 )
#endif
#endif

/*
 * Simplified macro to send a debug radio log message using the user given tag - _rfx_tag.
 */
#ifndef RFX_LOG_D
#define RFX_LOG_D(_rfx_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_DEBUG, _rfx_tag, __VA_ARGS__))
#endif

#ifndef RFX_LOG_D_IF
#define RFX_LOG_D_IF(cond, _rfx_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_DEBUG, _rfx_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

/*
 * Simplified macro to send an info radio log message using the user given tag - _rfx_tag.
 */
#ifndef RFX_LOG_I
#define RFX_LOG_I(_rfx_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_INFO, _rfx_tag, __VA_ARGS__))
#endif

#ifndef RFX_LOG_I_IF
#define RFX_LOG_I_IF(cond, _rfx_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_INFO, _rfx_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

/*
 * Simplified macro to send a warning radio log message using the user given tag - _rfx_tag.
 */
#ifndef RFX_LOG_W
#define RFX_LOG_W(_rfx_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_WARN, _rfx_tag, __VA_ARGS__))
#endif

#ifndef RFX_LOG_W_IF
#define RFX_LOG_W_IF(cond, _rfx_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_WARN, _rfx_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

/*
 * Simplified macro to send an error radio log message using the user given tag - _rfx_tag.
 */
#ifndef RFX_LOG_E
#define RFX_LOG_E(_rfx_tag, ...) \
    ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_ERROR, _rfx_tag, __VA_ARGS__))
#endif

#ifndef RFX_LOG_E_IF
#define RFX_LOG_E_IF(cond, _rfx_tag, ...) \
    ( (CONDITION(cond)) \
    ? ((void)__android_log_buf_print(LOG_ID_RADIO, ANDROID_LOG_ERROR, _rfx_tag, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef RFX_ASSERT
#define RFX_ASSERT(_expr)                                                         \
    do {                                                                          \
        if (!(_expr)) {                                                           \
            RFX_LOG_E("RFX_ASSERT", "RFX_ASSERT:%s, %d", __FILE__, __LINE__);     \
            LOG_ALWAYS_FATAL();                                        \
        }                                                                         \
    } while(0)
#endif

#endif /* __RFX_LOG_H__ */

