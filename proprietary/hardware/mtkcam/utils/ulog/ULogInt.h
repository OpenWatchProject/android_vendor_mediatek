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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __MTKCAM_UTILS_STD_ULOG_INT_H__
#define __MTKCAM_UTILS_STD_ULOG_INT_H__

#include <cstddef>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "ULogTable.h"


#define __likely(exp) (__builtin_expect((exp), true))
#define __unlikely(exp) (__builtin_expect((exp), false))


namespace NSCam {
namespace Utils {
namespace ULog {

size_t printIntArray(char *buffer, size_t bufferSize, const unsigned int *value, size_t lenOfValue);

inline const char *getModuleName(ModuleId moduleId)
{
    const char *moduleName = ULogTable::getSingleton().getModuleName(moduleId);
    return (moduleName != nullptr) ? moduleName : "?";
}


inline const char *getRequestTypeName(RequestTypeId req)
{
    const char *requestName = ULogTable::getSingleton().getRequestTypeName(req);
    return (requestName != nullptr) ? requestName : "?";
}


inline int timeDiffMs(const timespec &begin, const timespec &end)
{
    return static_cast<int>(
        (std::int64_t{end.tv_sec} - std::int64_t{begin.tv_sec}) * 1000 +
        (std::int64_t{end.tv_nsec} - std::int64_t{begin.tv_nsec}) / 1000000);
}


}
}
}


namespace {

thread_local pid_t sTid = -1;

inline int getTid()
{
    if (__unlikely(sTid < 0))
        sTid = static_cast<int>(gettid());
    return sTid;
}

const char * const ULOG_TIME_FORMAT = "%m-%d %H:%M:%S";
clockid_t ANDROID_CLOCK_ID = CLOCK_REALTIME;

inline struct tm *ulocaltime_r(const time_t *clock, struct tm *logTm)
{
    if (__unlikely(localtime_r(clock, logTm) == NULL)) // Coverity may warn for the possible NULL return
        memset(logTm, 0, sizeof(struct tm));
    return logTm; // Never return NULL
}

}


#ifndef USING_MTK_LDVT
#define ULOG_IMP_LOGE(fmt, args...) ALOGE(fmt, ##args)
#define ULOG_IMP_LOGW(fmt, args...) ALOGW(fmt, ##args)
#define ULOG_IMP_LOGI(fmt, args...) ALOGI(fmt, ##args)
#define ULOG_IMP_LOGD(fmt, args...) ALOGD(fmt, ##args)
#define ULOG_FW_LOGE(logTag, fmt, args...) (void)ALOG(LOG_ERROR, logTag, fmt, ##args)
#define ULOG_FW_LOGW(logTag, fmt, args...) (void)ALOG(LOG_WARN, logTag, fmt, ##args)
#define ULOG_FW_LOGI(logTag, fmt, args...) (void)ALOG(LOG_INFO, logTag, fmt, ##args)
#define ULOG_FW_LOGD(logTag, fmt, args...) (void)ALOG(LOG_DEBUG, logTag, fmt, ##args)
#define ULOG_FW_LOGV(logTag, fmt, args...) (void)ALOG(LOG_VERBOSE, logTag, fmt, ##args)
#else
#define ULOG_IMP_LOGE(fmt, args...) do { } while (0)
#define ULOG_IMP_LOGW(fmt, args...) do { } while (0)
#define ULOG_IMP_LOGI(fmt, args...) do { } while (0)
#define ULOG_IMP_LOGD(fmt, args...) do { } while (0)
#define ULOG_FW_LOGE(logTag, fmt, args...) do { } while (0)
#define ULOG_FW_LOGW(logTag, fmt, args...) do { } while (0)
#define ULOG_FW_LOGI(logTag, fmt, args...) do { } while (0)
#define ULOG_FW_LOGD(logTag, fmt, args...) do { } while (0)
#define ULOG_FW_LOGV(logTag, fmt, args...) do { } while (0)
#endif

#define CAM_ULOGI_ALWAYS(_module, _fmt, _args...) \
    do { \
        if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { ALOGI(_fmt, ##_args); } \
        if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
            NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
            CAM_ULOG_DETAILS_FILE(_uModuleId, NSCam::Utils::ULog::DETAILS_INFO, _fmt, ##_args); \
        } \
    } while (0)

#define CAM_ULOGMI_ALWAYS(_fmt, _args...) CAM_ULOGI(__ULOG_MODULE_ID, _fmt, ##_args)

#endif

