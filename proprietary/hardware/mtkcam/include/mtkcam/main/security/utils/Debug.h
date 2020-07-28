/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <cassert>
#include <mtkcam/utils/std/Log.h>

#ifndef DEBUG_LOG_TAG
#error "DEBUG_LOG_TAG is not defined!!"
#endif

#define IRIS_LOGV(x, ...)   CAM_LOGV("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define IRIS_LOGD(x, ...)   CAM_LOGD("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define IRIS_LOGI(x, ...)   CAM_LOGI("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define IRIS_LOGW(x, ...)   CAM_LOGW("[%s] " x, DEBUG_LOG_TAG, ##__VA_ARGS__)
#define IRIS_LOGE(x, ...)   CAM_LOGE("[%s] <%s:#%d>" x, DEBUG_LOG_TAG, \
                                 __FILE__, __LINE__, ##__VA_ARGS__)
#define IRIS_LOGA(x, ...)                           \
    do { CAM_LOGE("[%s] <%s:#%d>" x, DEBUG_LOG_TAG, \
            __FILE__, __LINE__, ##__VA_ARGS__);     \
         assert(false);                             \
    } while (0)

#define IRIS_LOGD_IF(cond, ...)  do { if ((cond)) IRIS_LOGD(__VA_ARGS__); } while (0)
#define IRIS_LOGI_IF(cond, ...)  do { if ((cond)) IRIS_LOGI(__VA_ARGS__); } while (0)
#define IRIS_LOGW_IF(cond, ...)  do { if ((cond)) IRIS_LOGW(__VA_ARGS__); } while (0)
#define IRIS_LOGE_IF(cond, ...)  do { if ((cond)) IRIS_LOGE(__VA_ARGS__); } while (0)
#define IRIS_LOGA_IF(cond, ...)  do { if ((cond)) IRIS_LOGA(__VA_ARGS__); } while (0)

// ---------------------------------------------------------------------------

#define CHECK_OBJECT(obj, ret)  do { if (obj == NULL) \
    { IRIS_LOGE("%s is NULL", #obj); return ret; } } while (0)

// ---------------------------------------------------------------------------

#ifdef USE_SYSTRACE
#include <mtkcam/utils/std/Trace.h>

#define IRIS_TRACE_CALL()                    CAM_TRACE_CALL()
#define IRIS_TRACE_NAME(name)                CAM_TRACE_NAME(name)
#define IRIS_TRACE_INT(name, value)          CAM_TRACE_INT(name, value)
#define IRIS_TRACE_ASYNC_BEGIN(name, cookie) CAM_TRACE_ASYNC_BEGIN(name, cookie)
#define IRIS_TRACE_ASYNC_END(name, cookie)   CAM_TRACE_ASYNC_END(name, cookie)
#else
#define IRIS_TRACE_CALL()
#define IRIS_TRACE_NAME(name)
#define IRIS_TRACE_INT(name, value)
#define IRIS_TRACE_ASYNC_BEGIN(name, cookie)
#define IRIS_TRACE_ASYNC_END(name, cookie)
#endif // USE_SYSTRACE

// ---------------------------------------------------------------------------

#include <cutils/compiler.h>
#include <type_traits>
#include <string>

namespace NSCam {
namespace security {

class ScopeLog final
{
public:
    ScopeLog()
    {
        ScopeLog("");
    }

    ScopeLog(const char* msg) : message(msg)
    {
        if (CC_UNLIKELY(message == nullptr))
            return;

        IRIS_LOGD("%s +", message);
    }

    ~ScopeLog()
    {
        IRIS_LOGD("%s -", message);
    }

private:
    const char *message;
};

#define AutoLog() ScopeLog __ScopeLog__(__func__)

} // namespace security
} // namespace NSCam

// ---------------------------------------------------------------------------

#endif // _UTILS_DEBUG_H_
