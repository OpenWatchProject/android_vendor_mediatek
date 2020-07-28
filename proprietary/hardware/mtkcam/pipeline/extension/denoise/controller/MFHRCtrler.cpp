/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "MFHRCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

#include "MFHRCtrler.h"
// log
#include <mtkcam/utils/std/Log.h>
// stereo
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
// custom setting
#include <camera_custom_stereo.h>
// STL
#include <atomic>
#include <cstdint>
#include <functional> // std::function
#include <mutex>


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#endif
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif

using namespace android;
using namespace NSCam::plugin;
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MFHRCtrler::
MFHRCtrler(
    MINT32 mode
) : BaseDNCtrler(mode)
{
    FUNCTION_SCOPE;
    setCaptureNum(getMFHRCaptureCnt());
    setDelayFrameNum(getMFHRDelayFrameCnt());
    setHWSyncStableNum(getHWSyncStableCnt());
    mpCB = new MFHRListenerImp(this);
    MY_LOGD("CapNum(%d) DelayNum(%d) HWSyncNum(%d) mpCB(%p)", getCaptureNum(), getDelayFrameNum(), getHWSyncStableCnt(), mpCB.get());
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MFHRCtrler::
~MFHRCtrler()
{
    FUNCTION_SCOPE;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
MFHRCtrler::
updateDenoiseRequest(
    sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request __attribute((unused)),
    std::shared_ptr<RequestFrame>& main1Frame          __attribute((unused)),
    std::shared_ptr<RequestFrame>& main2Frame          __attribute((unused)),
    MBOOL isMainFrame                                   __attribute__((unused)))
{
    FUNCTION_SCOPE;
    MY_LOGD("CKH: request(%p) main1Frame(%p) main2Frame(%p) isMainFrame(%d)",
                    request.get(),
                    main1Frame.get(),
                    main2Frame.get(),
                    isMainFrame);
    if(isMainFrame){
        request->addParam(NSCamFeature::NSFeaturePipe::PID_MFHR_IS_MAIN, 1);
    }
    if(main1Frame != nullptr)
    {
        if(isMainFrame){
            m_mainRequestFrame = main1Frame;
        }
        MY_LOGD("main1Frame: [%d] imgo(%p) rrzo(%p) lsco(%p) jpeg(%p) thumb(%p)",
                main1Frame->getOpenId(),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullSrc),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_ResizedSrc),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut),
                main1Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut));
    }
    if(main2Frame != nullptr)
    {
        MY_LOGD("main2Frame: [%d] imgo(%p) rrzo(%p) lsco(%p) jpeg(%p) thumb(%p)",
                main2Frame->getOpenId(),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_FullSrc),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_ResizedSrc),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_FullOut),
                main2Frame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut));
    }
    // after main2 finish, mark it to error
    MY_LOGD("mark to error");
    main2Frame->markError(RequestFrame::eRequestImg_All, true);
    main2Frame->markError(RequestFrame::eRequestMeta_All, true);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
MFHRCtrler::
denoiseProcessingDone(
    NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB::EventId eventType,
    sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request
)
{
    FUNCTION_SCOPE;
    // do somthing you want, and then call denoiseProcessingDone
    BaseDNCtrler::denoiseProcessingDone(eventType, request);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
MFHRCtrler::
postviewDone(
    NSCamFeature::NSFeaturePipe::IBMDeNoisePipeCB::EventId eventType  __attribute__((unused)),
    sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request
)
{
    FUNCTION_SCOPE;
    // do somthing you want, and then call denoiseProcessingDone
    BaseDNCtrler::postviewDone(eventType, request);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
android::status_t
MFHRCtrler::
onEvent(
    PipeCB::EventId eventType,
    sp<NSCamFeature::NSFeaturePipe::PipeRequest>& request,
    void* param1          __attribute((unused)),
    void* param2          __attribute((unused))
)
{
    FUNCTION_SCOPE;
    auto toString = [](PipeCB::EventId eventType)
    {
        switch(eventType)
        {
            case PipeCB::eCompleted:
                return "eCompleted";
            case PipeCB::eFailed:
                return "eFailed";
            case PipeCB::eShutter:
                return "eShutter";
            case PipeCB::eP2Done:
                return "eP2Done";
            case PipeCB::eFlush:
                return "eFlush";
            case PipeCB::ePostview:
                return "ePostview";
        }
        return "UNKNOWN";
    };
    MY_LOGD("cmd(%s) id(%d)", toString(eventType), eventType);
    switch(eventType)
    {
        case PipeCB::eShutter:
            onShutter();
            break;
        case PipeCB::eP2Done:
            onNextCaptureReady();
            break;
        case PipeCB::eFailed:
        case PipeCB::eFlush:
        case PipeCB::eCompleted:
            denoiseProcessingDone(eventType, request);
            break;
        case PipeCB::ePostview:
            postviewDone(eventType, request);
            break;
    }
    return OK;
}