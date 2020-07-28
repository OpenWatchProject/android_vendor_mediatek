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

#ifndef _MTKCAM_FEATURE_UTILS_P2_TRACE_H_
#define _MTKCAM_FEATURE_UTILS_P2_TRACE_H_

#include <mtkcam/utils/std/Trace.h>
#include <cutils/properties.h>

#define TRACE_DEFAULT 0
#define TRACE_ADVANCED 1

#define P2_NEED_TRACE NSCam::CamSysTrace::needP2Trace
#define _PASTE(x, y) x ## y
#define PASTE(x, y) _PASTE(x,y)

#define P2_CAM_TRACE_NAME(lv,name) NSCam::CamSysTrace PASTE(___camtracer, __LINE__) (P2_NEED_TRACE(lv), name)
#define P2_CAM_TRACE_CALL(lv) P2_CAM_TRACE_NAME(lv, __FUNCTION__)
#define P2_CAM_TRACE_BEGIN(lv,name) NSCam::CamSysTrace::begin(P2_NEED_TRACE(lv), name)
#define P2_CAM_TRACE_END(lv) NSCam::CamSysTrace::end(P2_NEED_TRACE(lv))
#define P2_CAM_TRACE_ASYNC_BEGIN(lv,name, cookie) NSCam::CamSysTrace::asyncBegin(P2_NEED_TRACE(lv), name, cookie)
#define P2_CAM_TRACE_ASYNC_END(lv, name, cookie) NSCam::CamSysTrace::asyncEnd(P2_NEED_TRACE(lv), name, cookie)

#define P2_CAM_TRACE_FMT_BEGIN(lv, fmt, arg...)             \
do{                                                         \
    if( P2_NEED_TRACE(lv))                                  \
    {                                                       \
        CAM_TRACE_FMT_BEGIN(fmt, ##arg);                    \
    }                                                       \
}while(0)


namespace NSCam {

class CamSysTrace {
public:
    inline CamSysTrace(bool need, const char* name) : mNeed(need){
        if(mNeed)
        {
            CAM_TRACE_BEGIN(name);
        }
    }
    inline ~CamSysTrace() {
        if(mNeed)
        {
            CAM_TRACE_END();
        }
    }

    static inline void begin(bool need, const char* name)
    {
        if(need)
        {
            CAM_TRACE_BEGIN(name);
        }
    }

    static inline void end(bool need)
    {
        if(need)
        {
            CAM_TRACE_END();
        }
    }

    static inline void asyncBegin(bool need, const char* name, int32_t cookie)
    {
        if(need)
        {
            CAM_TRACE_ASYNC_BEGIN(name, cookie);
        }
    }

    static inline void asyncEnd(bool need, const char* name, int32_t cookie)
    {
        if(need)
        {
            CAM_TRACE_ASYNC_END(name, cookie);
        }
    }

    static inline bool needP2Trace(int level)
    {
        static int32_t camProp = property_get_int32("vendor.debug.mtkcam.systrace.level", MTKCAM_SYSTRACE_LEVEL_DEFAULT);
        static int32_t debugProp = property_get_int32("vendor.debug.systrace.p2", 0);
        static int32_t persistProp = property_get_int32("persist.vendor.systrace.p2", 0);
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) ((a) >= (b)) ? (a) : (b)
        static int32_t threshold = MAX(MAX(camProp, debugProp), persistProp);
#undef MAX
        return threshold >= level;
    }

private:
    bool mNeed;
};
};// namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_TRACE_H_
