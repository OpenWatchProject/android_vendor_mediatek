/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
/**
 * @file libmdmonitor.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The file was globally included by all files in the library for basic utility functions and data types.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef _MDMONITOR_H_
#define _MDMONITOR_H_

#include <stdio.h>
#include <climits>
#include <stdlib.h>
#include <string.h>
#include "MonitorTypes.h"
#include "MonitorDefs.h"
#include "MonitorUtils.h"

#define MDM_LOG_ENABLE 1

#if MDM_LOG_ENABLE == 1
#define MDM_V_ENABLE 0
#define MDM_D_ENABLE 1
#define MDM_I_ENABLE 1
#define MDM_W_ENABLE 1
#define MDM_E_ENABLE 1
#else
#define MDM_V_ENABLE 0
#define MDM_D_ENABLE 0
#define MDM_I_ENABLE 0
#define MDM_W_ENABLE 0
#define MDM_E_ENABLE 0
#endif // if MDM_LOG_ENABLE == 1

/**
 * @brief A macro to output debug log. 
 *
 * @param level [IN] The value of MONITOR_DEBUG_LEVEL.
 * @param ... [IN] The variadic arguments.
 */
#ifdef _MDM_WITHOUT_NDK_
  #define MDM_LOG(level, suffix, ...) do { \
     if (level < monitorDebugLevel) { \
         break; \
     } \
     printf("[%s][%d] ", strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__, __LINE__); \
     printf(__VA_ARGS__); \
     printf("\n"); \
     fflush(stdout); \
  } while (false)
#else
  #include <android/log.h>
  #define LIB_LOG_TAG "libmdmonitor"
  #define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LIB_LOG_TAG, __VA_ARGS__)
  #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LIB_LOG_TAG, __VA_ARGS__)
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LIB_LOG_TAG, __VA_ARGS__)
  #define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LIB_LOG_TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LIB_LOG_TAG, __VA_ARGS__)
  #define MDM_LOG(level, suffix, ...) do { \
     if (MDM_##suffix##_ENABLE && level >= monitorDebugLevel) { \
         LOG##suffix(__VA_ARGS__); \
     } \
  } while (0)
#endif

#define MDM_LOGE(...) MDM_LOG(MONITOR_DEBUG_LEVEL_ERROR, E, __VA_ARGS__)
#define MDM_LOGW(...) MDM_LOG(MONITOR_DEBUG_LEVEL_WARN, W, __VA_ARGS__)
#define MDM_LOGI(...) MDM_LOG(MONITOR_DEBUG_LEVEL_INFO, I, __VA_ARGS__)
#define MDM_LOGD(...) MDM_LOG(MONITOR_DEBUG_LEVEL_DEBUG, D, __VA_ARGS__)
#define MDM_LOGV(...) MDM_LOG(MONITOR_DEBUG_LEVEL_VERBOSE, V, __VA_ARGS__)

/**
 * @brief Modem Monitor Library APIs are declared inside the namespace to<br>
 * prevent name conflicts in large projects. 
 */
namespace libmdmonitor {
    /**
     * @brief The debug level which is defined in @c MonitorCore.cpp
     */
    extern MONITOR_DEBUG_LEVEL monitorDebugLevel;

    void SetDebugLevel(MONITOR_DEBUG_LEVEL level);

/**
 * @brief A macro to dump raw debug log.
 *
 * @param level [IN] The value of MONITOR_DEBUG_LEVEL.
 * @param buffer [IN] The data buffer to dump.
 * @param len [IN] The length of @a buffer.
 * @param modifier [IN] The modifier to print data.
 */
#define MDM_DUMP_RAW(level, buffer, len, modifier) (void *)0

} // namespace libmdmonitor

#endif /* _MDMONITOR_H_ */
