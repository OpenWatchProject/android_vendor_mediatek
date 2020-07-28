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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_CALLSTACK_LOGGER_H_
#define MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_CALLSTACK_LOGGER_H_

//Usage:
//1. Add to makefile:
//  LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder
//  LOCAL_SHARED_LIBRARIES += libladder
//2. Add to source code:
// #include <mtkcam/utils/std/CallStackLogger.h>
// NSCam::Utils::CallStackLogger csl;
// csl.logThread(LOG_TAG);  //print callstack of current thread
// csl.logThread(LOG_TAG, ANDROID_LOG_INFO, tid);  //print callstack of specified thread
// csl.logProcess(LOG_TAG); //print callstack of current process

#include <android/log.h>
//#include <libladder.h>
#include <vector>
#include <string>

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


/******************************************************************************
 *
 ******************************************************************************/
class CallStackLogger {
public:
    CallStackLogger()
    {
    }

    ~CallStackLogger()
    {
    }

    void logThread(const char* logTag, android_LogPriority priority = ANDROID_LOG_INFO, pid_t tid=gettid())
    {
        const char *tag = logTag;
#ifdef LOG_TAG
        if(nullptr == tag) {
            tag = LOG_TAG;
        }
#else
        if(nullptr == tag) {
            tag = __DEFAULT_LOG_TAG;
        }
#endif

        std::string callstack;
        //UnwindThreadBT(tid, &callstack);
        __logCallstack(tag, callstack, priority);
    }

    void logProcess(const char* logTag, android_LogPriority priority = ANDROID_LOG_INFO)
    {
        const char *tag = logTag;
#ifdef LOG_TAG
        if(nullptr == tag) {
            tag = LOG_TAG;
        }
#else
        if(nullptr == tag) {
            tag = __DEFAULT_LOG_TAG;
        }
#endif

        std::string callstack;
        //UnwindCurProcessBT(&callstack);
        __logCallstack(tag, callstack, priority);
    }

private:
    void __logCallstack(const char* logTag, std::string &callstack, android_LogPriority priority)
    {
        size_t len = callstack.size();
        if(len < MAX_LOG_SIZE) {
            __android_log_write(priority, logTag, callstack.c_str());
        } else {
            // Flow: replace newline(\n) by NULL for android log to print
            int logSize   = 0;  //real log size, include newline
            int logOffset = 0;  //offset from c_str()
            const char *HEAD       = callstack.c_str();
            const char *log        = nullptr;
            const char *newlinePos = nullptr;

            while(len > 0) {
                log = HEAD + logOffset;

                if(len >= MAX_LOG_SIZE) {
                    newlinePos = (const char*)memrchr(log, '\n', MAX_LOG_SIZE-1);
                    if(newlinePos) {
                        logSize = newlinePos - log + 1;

                        callstack.replace(newlinePos-HEAD, 1, 1, 0);

                        //c_str() may change after replace(), so we call c_str() here
                        HEAD = callstack.c_str();
                        log = HEAD + logOffset;
                        __android_log_write(priority, logTag, log);

                        logOffset += logSize;
                        len       -= logSize;

                        // Uncomment for running benchmark of this function
                        // callstack.replace(newlinePos-HEAD, 1, 1, '\n');
                    } else {
                        len = 0;
                        __android_log_write(priority, logTag, log);
                    }
                } else {
                    len = 0;
                    __android_log_write(priority, logTag, log);
                }
            }
        }
    }

private:
    const char *__DEFAULT_LOG_TAG = "Mtkcam/CallStackLogger";
    const unsigned int MAX_LOG_SIZE        = 1024;
};

/******************************************************************************
 *
 ******************************************************************************/
} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_CALLSTACK_LOGGER_H_

