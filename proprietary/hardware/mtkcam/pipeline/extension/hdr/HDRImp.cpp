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
//
#define LOG_TAG "HdrVendorImp"
static const char* __CALLERNAME__ = LOG_TAG;

#include "HDRImp.h"

#include "controller/HDRCtrler.h"
#include "../utils/VendorUtils.h"
#include "../utils/RequestFrame.h"
#include "../utils/ControllerContainer.h"

// LINUX
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// AOSP
#include <cutils/compiler.h>
#include <cutils/properties.h>

// STL
#include <memory> // std::shared_ptr
#include <deque> // std::deque
#include <mutex> // std::mutex
#include <algorithm> // std::find
#include <thread>
#include <chrono>

// ----------------------------------------------------------------------------
// definitions
// ----------------------------------------------------------------------------
#define THREAD_SLOT_SIZE    2

// ----------------------------------------------------------------------------
// MY_LOG
// ----------------------------------------------------------------------------
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
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
#endif // SCOPE_TIMER


using namespace android;
using namespace NSCam::plugin;
using namespace NSCam::HDR2;

using NS3Av3::CaptureParam_T;
using NS3Av3::IHal3A;


// ----------------------------------------------------------------------------
// helper
// ----------------------------------------------------------------------------
static inline std::shared_ptr<HDRCtrler> create_hdr_controller(
        int openId,
        int logLevel,
        int isZsd)
{
    std::shared_ptr<HDRCtrler> c
        (
            dynamic_cast<HDRCtrler*>(new HDRCtrler(openId, logLevel >= 1, isZsd))
        );

    return c;
}

// ----------------------------------------------------------------------------
// static member
// ----------------------------------------------------------------------------
ControllerContainer<HDRVendorImp::CtrlerType> HDRVendorImp::sUnusedCtrlers;


// ----------------------------------------------------------------------------
// Implementations of IVendor
// ----------------------------------------------------------------------------
HDRVendorImp::HDRVendorImp(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
    ) noexcept
    : BaseVendor(i4OpenId, vendorMode, String8::format("HDR vendor"))
    , mZsdFlow(MTRUE)
    , mAlgoMode(0)
{
    FUNCTION_SCOPE;
    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    setThreadShotsSize(THREAD_SLOT_SIZE);
}


HDRVendorImp::~HDRVendorImp()
{
}


status_t HDRVendorImp::get(
        MINT32           openId         __attribute__((unused)),
        const InputInfo& in             __attribute__((unused)),
        FrameInfoSet&    out            __attribute__((unused)))
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // Step 1: create (or get) the first unused HDR controller
    std::shared_ptr<CtrlerType> ctrler = getCtrler(CTRLER_UNUSED);
    if (ctrler.get() == nullptr) {
        MY_LOGE("unexpected error, get controller failed");
        return UNKNOWN_ERROR;
    }

    // Step 2: configure settings
    int frameCount = ctrler->getCaptureNum();
    int frameDelay = ctrler->getDelayFrameNum();

#ifdef __DEBUG
    // {{{ show info
    MY_LOGD("%s: cap  = %d", __FUNCTION__, frameCount);
    MY_LOGD("%s: dely = %d", __FUNCTION__, frameDelay);
    // }}}
#endif

    //

    // Step 2.1: configure capture setting.
    for (int i = 0; i < frameCount; i++) {
        bool bLastFrame = ((i + 1) == frameCount);

        // default, copy from InputInfo.
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;

        // modify hal control metadata
        IMetadata& appSetting(item.setting.appMeta);
        IMetadata& halSetting(item.setting.halMeta);
        {
            const HDRCaptureParam& captureParam(ctrler->get3ACapParam(i));

            IMetadata::Memory capParams;
            capParams.resize(sizeof(CaptureParam_T));
            memcpy(capParams.editArray(), &(captureParam.exposureParam), sizeof(CaptureParam_T));

            IMetadata::setEntry<IMetadata::Memory>(
                    &halSetting, MTK_3A_AE_CAP_PARAM, capParams);

            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_FOCUS_PAUSE,
                    bLastFrame ? 0 : 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);
            // configure LTM per HDR input frame
            if (CC_LIKELY(captureParam.ltmMode != LTMMode::NONE))
            {
                IMetadata::setEntry<MINT32>(
                        &halSetting, MTK_3A_ISP_BYPASS_LCE,
                        captureParam.bypassLTM());
            }

            //Inform P1Node we want processed raw
            IMetadata::setEntry<int>(
                    &halSetting, MTK_P1NODE_RAW_TYPE, 0);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_3A_PGN_ENABLE, 0);
        }

        out.settings.push_back(item);
    }

    // Step 2.2: add delay frames.
    {
        for (int i = 0; i < frameDelay; i++) {
            // default, copy from InputInfo.
            MetaItem item;
            item.setting.appMeta = in.appCtrl;
            item.setting.halMeta = in.halCtrl;

            const HDRCaptureParam& captureParam(ctrler->get3ADelayCapParam(i));
            IMetadata::Memory capParams;
            capParams.resize(sizeof(CaptureParam_T));
            memcpy(capParams.editArray(), &(captureParam.exposureParam), sizeof(CaptureParam_T));

            IMetadata& halSetting(item.setting.halMeta);
            IMetadata::setEntry<IMetadata::Memory>(
                        &halSetting, MTK_3A_AE_CAP_PARAM, capParams);

            // configure LTM per HDR input frame
            if (CC_LIKELY(captureParam.ltmMode != LTMMode::NONE))
            {
                IMetadata::setEntry<MINT32>(
                        &halSetting, MTK_3A_ISP_BYPASS_LCE, captureParam.bypassLTM());
            }

            out.settings.push_back(item);
        }
    }

    // Step 2.3: configure it
    out.table.vendorMode    = mMode;
    out.table.inCategory    = FORMAT_RAW;
    out.table.outCategory   = FORMAT_YUV;
    out.table.bufferType    = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount          = out.settings.size();

    // Step 3: create StreamId Map
    // create StreamId Map for RequestFrame
    {
        SCOPE_TIMER (__t1, "Create StreamID map");
        RequestFrame::StreamIdMap_Img map;
        if (CC_LIKELY(in.fullRaw.get())) {
            map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
            ctrler->setSizeSrc(in.fullRaw->getImgSize());
            MY_LOGD("set source size=%dx%d",
                    ctrler->getSizeSrc().w, ctrler->getSizeSrc().h);
        }
        else {
            MY_LOGE("no full size RAW");
        }

        if (in.resizedRaw.get()) {
            map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
            ctrler->setSizeRrzo(in.resizedRaw->getImgSize());
            MY_LOGD("set rrzo size=%dx%d",
                    ctrler->getSizeRrzo().w, ctrler->getSizeRrzo().h);
        }
        else {
            MY_LOGD("no rrzo");
        }

        // lcso
        if (CC_LIKELY(in.lcsoRaw.get())) {
            map[RequestFrame::eRequestImg_LcsoRaw] = in.lcsoRaw->getStreamId();
        }
        else {
            MY_LOGD("no lcso");
        }

        if (CC_LIKELY(in.jpegYuv.get())) {
            map[RequestFrame::eRequestImg_FullOut] = in.jpegYuv->getStreamId();
        }
        else {
            MY_LOGE("no output YUV");
        }

        if (CC_LIKELY(in.thumbnailYuv.get())) {
            map[RequestFrame::eRequestImg_ThumbnailOut] = in.thumbnailYuv->getStreamId();
        }
        else {
            MY_LOGW("no thumbnail info");
        }

        if (CC_LIKELY(in.workingbuf.get())) {
            map[RequestFrame::eRequestImg_WorkingBufferOut] = in.workingbuf->getStreamId();
        }
        else {
            MY_LOGD("no working buffer info");
        }

        if ( in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End )
            MY_LOGF("output yuv too much.");
        for(size_t i = 0; i < in.vYuv.size(); ++i) {
            map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
        }

        ctrler->setStreamIdMapImg(std::move(map));
        ctrler->enterCaptureScenario(openId, in.sensorMode);
    }

    return OK;
}


status_t HDRVendorImp::set(
        MINT32              openId              __attribute__((unused)),
        const InputSetting& in                  __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // Step 1: Take the first unused HDR controller.
    auto ctrler = takeCtrler(CTRLER_UNUSED);
    if (CC_UNLIKELY(ctrler.get() == nullptr)) {
        MY_LOGE("No unused HDR controller");
        return UNKNOWN_ERROR;
    }

    // Step 2: check capture frame number is matched or not.
    if (CC_UNLIKELY(ctrler->getCaptureNum() + ctrler->getDelayFrameNum() > static_cast<int>(in.vFrame.size()))) {
        MY_LOGE("frame amount is not enough, re-set size to %zu",
                in.vFrame.size());
        if (in.vFrame.size() <= 0) {
            MY_LOGE("frame size is 0");
            return UNKNOWN_ERROR;
        }
    }

    // Step3:
    //  3.1: set request to this controller
    //for (size_t i = 0; i < in.vFrame.size(); i++) {
    for (int i = 0; i < ctrler->getCaptureNum(); i++) {
        ctrler->setBelong(in.vFrame[i].frameNo);
        MY_LOGD("HDRCtrler is belong to frame requestNo:%d", in.vFrame[i].frameNo);
    }

    //  3.2: set message callback
    ctrler->setMessageCallback(in.pMsgCb);


    // Step4: fire HDR asynchronously and push this controller to
    //        used controller container
    pushCtrler(ctrler, CTRLER_USED);
    // if the number of used thread slot reaches to max,
    // this method will be blocked until there's a free thread slot.
    asyncRun([this, ctrler]() mutable {
            ::prctl(PR_SET_NAME, "doHDR", 0, 0, 0);
            ctrler->checkHdrParam(); // init HDR controller
            ctrler->execute();
            ctrler->waitExecution();
            removeCtrler(ctrler, CTRLER_USED); // remove self from container
            ctrler = nullptr; // clear smart pointer after use, this scope may
                              // not exit until std::future::get has invoked.
        });

    return OK;
}


status_t HDRVendorImp::queue(
        MINT32  const              openId           __attribute__((unused)),
        MUINT32 const              requestNo        __attribute__((unused)),
        wp<IVendor::IDataCallback> cb               __attribute__((unused)),
        BufferParam                bufParam         __attribute__((unused)),
        MetaParam                  metaParam        __attribute__((unused)))
{
    FUNCTION_SCOPE;

#ifdef __DEBUG
    MY_LOGI("queue frame, requestNo=%u", requestNo);
#endif

    // Step 1:
    // find the used(fired) controller by request number
    auto spCtrl = getCtrler(requestNo, CTRLER_USED);
    if (CC_UNLIKELY(spCtrl.get() == nullptr)) {
        MY_LOGW("error, this frame is not belong to anyone.");
        return BAD_INDEX;
    }

    // Step 2:
    // enque frame to controller.
    if (spCtrl->getEnquedTimes() < spCtrl->getCaptureNum()) {
        MY_LOGD("enque frame");
        spCtrl->enqueFrame(openId, requestNo, bufParam, metaParam, cb);
    }

    return OK;
}


status_t HDRVendorImp::beginFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;

    drain(); // cancel flow, and drain.

    return OK;
}


status_t HDRVendorImp::endFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;
    return OK;
}


status_t HDRVendorImp::sendCommand(
        MINT32      /* cmd */,
        MINT32      /* openId */,
        MetaItem&   /* meta */,
        MINT32&     /* arg1 */,
        MINT32&     /* arg2 */,
        void*       /* arg3 */
    )
{
    FUNCTION_SCOPE;
    return OK;
}


status_t HDRVendorImp::dump(MINT32 /* openId */)
{
    FUNCTION_SCOPE;
    return OK;
}

void HDRVendorImp::drain(bool bCancel /* = true */)
{
    // moves all controller from container(s)
    auto CC = std::move(mUsedCtrlers);

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<CtrlerType>::ContainerType Data_T;
    CC.iterate([bCancel](Data_T& data) {
        for (auto&& itr : data)
            if (itr.get() && bCancel)
                itr->doCancel();

        for (auto&& itr : data)
            if (itr.get())
                itr->waitExecution();

    });
}


ControllerContainer<HDRVendorImp::CtrlerType>*
HDRVendorImp::getCtrlerContainer(CTRLER u)
{
    switch (u) {
    case CTRLER_USED:
        return &mUsedCtrlers;
    case CTRLER_UNUSED:
        return &sUnusedCtrlers;
    default:
        MY_LOGE("get wrong controller container (enum=%d)", u);
        return nullptr;
    }
    return nullptr;
}


std::shared_ptr<HDRVendorImp::CtrlerType>
HDRVendorImp::getCtrler(CTRLER u)
{
    auto pCC = getCtrlerContainer(u);

    std::shared_ptr<CtrlerType> ctrler;
    auto openId  = getOpenId();
    auto isZsd   = mZsdFlow;

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<CtrlerType>::ContainerType Data_T;

    pCC->iterate
    (
        [&ctrler, openId, isZsd]
        (Data_T& data)
        {
            // if no controller in container, create a new one and push back into it
            if (data.size() <= 0) {
                ctrler = create_hdr_controller(openId, 1, isZsd);
                data.push_back(ctrler);
            }
            // or get the last controller
            else {
                ctrler = data.back();
            }
        }
    );

    return ctrler;
}


std::shared_ptr<HDRVendorImp::CtrlerType>
HDRVendorImp::getCtrler(MUINT32 requestNo, CTRLER u)
{
    return getCtrlerContainer(u)->belong_to(requestNo);
}


std::shared_ptr<HDRVendorImp::CtrlerType>
HDRVendorImp::takeCtrler(CTRLER u)
{
    return getCtrlerContainer(u)->take_last();
}


void HDRVendorImp::pushCtrler(const std::shared_ptr<CtrlerType>& c, CTRLER u)
{
    getCtrlerContainer(u)->push_back(c);
}


bool HDRVendorImp::removeCtrler(std::shared_ptr<CtrlerType> c, CTRLER u)
{
    return getCtrlerContainer(u)->remove(c);
}
