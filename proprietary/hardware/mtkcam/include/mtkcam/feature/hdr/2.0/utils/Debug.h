/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _UTILS_DEBUG_H_
#define _UTILS_DEBUG_H_

#include <log/log.h>

#ifndef DEBUG_LOG_TAG
#error "DEBUG_LOG_TAG is not defined!!"
#endif

#define HDR_LOGV(x, ...)   ALOGV("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define HDR_LOGD(x, ...)   ALOGD("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define HDR_LOGI(x, ...)   ALOGI("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define HDR_LOGW(x, ...)   ALOGW("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define HDR_LOGE(x, ...)   ALOGE("[%s] <%s:#%d>" x, DEBUG_LOG_TAG, \
                                 __FILE__, __LINE__, ##__VA_ARGS__)

#define HDR_LOGD_IF(cond, ...)  do { if ((cond)) HDR_LOGD(__VA_ARGS__); } while (0)
#define HDR_LOGI_IF(cond, ...)  do { if ((cond)) HDR_LOGI(__VA_ARGS__); } while (0)
#define HDR_LOGW_IF(cond, ...)  do { if ((cond)) HDR_LOGW(__VA_ARGS__); } while (0)
#define HDR_LOGE_IF(cond, ...)  do { if ((cond)) HDR_LOGE(__VA_ARGS__); } while (0)

// ---------------------------------------------------------------------------

#define FUNCTION_LOG_START      HDR_LOGD("[%s] +", __FUNCTION__)
#define FUNCTION_LOG_END        HDR_LOGD("[%s] - ret(%d)", __FUNCTION__, ret)
#define FUNCTION_LOG_END_MUM    HDR_LOGD("[%s] -", __FUNCTION__)

// ---------------------------------------------------------------------------

#define HDR_DUMP_PATH "/sdcard/hdr_output/"

// ---------------------------------------------------------------------------

#ifdef USE_SYSTRACE
#include <mtkcam/Trace.h>

#define HDR_TRACE_CALL()                    CAM_TRACE_CALL()
#define HDR_TRACE_NAME(name)                CAM_TRACE_NAME(name)
#define HDR_TRACE_INT(name, value)          CAM_TRACE_INT(name, value)
#define HDR_TRACE_ASYNC_BEGIN(name, cookie) CAM_TRACE_ASYNC_BEGIN(name, cookie)
#define HDR_TRACE_ASYNC_END(name, cookie)   CAM_TRACE_ASYNC_END(name, cookie)
#else
#define HDR_TRACE_CALL()
#define HDR_TRACE_NAME(name)
#define HDR_TRACE_INT(name, value)
#define HDR_TRACE_ASYNC_BEGIN(name, cookie)
#define HDR_TRACE_ASYNC_END(name, cookie)
#endif // USE_SYSTRACE

#endif // _UTILS_DEBUG_H_
