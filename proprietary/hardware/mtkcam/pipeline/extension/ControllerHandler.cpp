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
#define LOG_TAG "ControllerHandler"
static const char* __CALLERNAME__ = LOG_TAG;

#include "ControllerHandler.h"

// utils
#include "utils/VendorUtils.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h>

// AOSP
#include <cutils/compiler.h>

// STL
#include <memory> // std::shared_ptr
#include <deque> // std::deque
#include <mutex> // std::mutex
#include <algorithm> // std::find
#include <thread>
#include <chrono>

// LINUX
#include <sys/prctl.h> // ::prctl

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
#define LOG_SCOPE(log)          auto __scope_logger__ = create_scope_logger(log)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#define LOG_SCOPE
#endif


using namespace android;
using namespace NSCam::plugin;


ControllerHandler::ControllerHandler(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
    )
    : BaseVendor(i4OpenId, vendorMode, String8::format("ControllerHandler"))
{
    FUNCTION_SCOPE;
    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    setThreadShotsSize(THREAD_SLOT_SIZE);
}


ControllerHandler::~ControllerHandler()
{
    FUNCTION_SCOPE;
    // due to asyncRun is a thread from based class, before releasing this instance,
    // we have to make sure all resource of this instance is available after the
    // asyncRun finished.
    syncAllThread();
}


status_t ControllerHandler::get(
        MINT32           openId         __attribute__((unused)),
        const InputInfo& in             __attribute__((unused)),
        FrameInfoSet&    out            __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // configure it
    out.table.vendorMode    = mMode;
    out.table.inCategory    = FORMAT_YUV;
    out.table.outCategory   = FORMAT_YUV;
    out.table.bufferType    = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount          = 1;

    // create (or get) the first unused controller
    std::shared_ptr<BaseController> ctrler = getCtrler(CTRLER_UNUSED);
    if (ctrler.get() == nullptr) {
        MY_LOGE("unexpected error, get controller failed");
        return UNKNOWN_ERROR;
    }

    {
        // check in.sensorMode
        if (CC_UNLIKELY( in.sensorMode == SENSOR_SCENARIO_ID_UNNAMED_START )) {
            MY_LOGW("sensorMode is unnamed for IScenarioControl, use Capture as default");
            ctrler->enterCaptureScenario(openId, SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
        }
        else {
            ctrler->enterCaptureScenario(openId, in.sensorMode);

        }
    }

    // create StreamId Map for RequestFrame
    setMap(in, out, ctrler);

    return OK;
}

status_t ControllerHandler::setMap(
        const InputInfo& in                     __attribute__((unused)),
        FrameInfoSet&    out                    __attribute__((unused)),
        std::shared_ptr<BaseController> ctrler  __attribute__((unused)))
{
    ctrler->setFrameSize(out.frameCount);

    RequestFrame::StreamIdMap_Img map;
    if (CC_LIKELY(in.fullRaw.get())) {
        map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
    }

    if (CC_LIKELY(in.resizedRaw.get())) {
        map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
    }

    // lcso
    if (CC_LIKELY(in.lcsoRaw.get())) {
        map[RequestFrame::eRequestImg_LcsoRaw] = in.lcsoRaw->getStreamId();
    }

    if (CC_LIKELY(in.postview.get())) {
            map[RequestFrame::eRequestImg_PostviewOut] = in.postview->getStreamId();
    }

    if (CC_LIKELY(in.jpegYuv.get())) {
        map[RequestFrame::eRequestImg_FullOut] = in.jpegYuv->getStreamId();
    }

    if (CC_LIKELY(in.thumbnailYuv.get())) {
        map[RequestFrame::eRequestImg_ThumbnailOut] = in.thumbnailYuv->getStreamId();
    }

    if (CC_LIKELY(in.workingbuf.get())) {
        map[RequestFrame::eRequestImg_WorkingBufferIn]  = in.workingbuf->getStreamId();
        map[RequestFrame::eRequestImg_WorkingBufferOut] = in.workingbuf->getStreamId();
    }

    if ( in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End )
        MY_LOGF("output yuv too much.");
    for(size_t i = 0; i < in.vYuv.size(); ++i) {
        if (CC_LIKELY(in.vYuv[i].get()))
            map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
    }

    ctrler->setStreamIdMapImg(std::move(map));

    return OK;
}

status_t ControllerHandler::set(
        MINT32              openId              __attribute__((unused)),
        const InputSetting& in                  __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // Step 1: Take the first unused controller.
    auto ctrler = takeCtrler(CTRLER_UNUSED);
    if (CC_UNLIKELY(ctrler.get() == nullptr)) {
        MY_LOGE("No unused controller");
        return UNKNOWN_ERROR;
    }

    // Step 2: check capture frame number is matched or not.
    if (CC_UNLIKELY(ctrler->getFrameSize() > static_cast<int>(in.vFrame.size()))) {
        MY_LOGE("frame amount is not enough, re-set size to %zu",
                in.vFrame.size());
        if (in.vFrame.size() <= 0) {
            MY_LOGE("frame size is 0");
            return UNKNOWN_ERROR;
        }
    }

    // Step3:
    //  3.1: set request to this controller
    for (int i = 0; i < ctrler->getFrameSize(); i++) {
        ctrler->setBelong(in.vFrame[i].frameNo);
    }

    //  3.2: set message callback
    ctrler->setMessageCallback(in.pMsgCb);


    // Step4: fire HDR asynchronously and push this controller to
    //        used controller container
    pushCtrler(ctrler, CTRLER_USED);
    // if the number of used thread slot reaches to max,
    // this method will be blocked until there's a free thread slot.
    asyncRun([this, ctrler]() mutable {
            ::prctl(PR_SET_NAME, "Processing", 0, 0, 0);
            LOG_SCOPE("ControllerHandler: asyncRun");
            ctrler->execute();
            ctrler->waitExecution();
            removeCtrler(ctrler, CTRLER_USED); // remove self from container
            ctrler = nullptr; // clear smart pointer after use, this scope may
                              // not exit until std::future::get has invoked.
        });

    return OK;
}


status_t ControllerHandler::queue(
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
    if (spCtrl->getEnquedTimes() < spCtrl->getFrameSize()) {
        MY_LOGD("enque frame");
        auto _result = spCtrl->enqueFrame(openId, requestNo, bufParam, metaParam, cb);
        if (_result < 0) {
            switch (_result) {
            case -1:
                MY_LOGW("error, this controller is invalidated, cannot enque frame");
                break;
            default:
                MY_LOGW("error, enqueFrame returns failed");
                break;
            }
            return BAD_INDEX;
        }
    }

    return OK;
}

status_t ControllerHandler::beginFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;

    drain(); // cancel flow, and drain.

    return OK;
}


status_t ControllerHandler::endFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;
    return OK;
}


status_t ControllerHandler::sendCommand(
        MINT32       cmd ,
        MINT32      /* openId */,
        MetaItem&   /* meta */,
        MINT32&      arg1 ,
        MINT32&     /* arg2 */,
        void*       /* arg3 */
    )
{
    FUNCTION_SCOPE;
    switch(cmd) {
        case INFO_CANCEL:{
            auto spCtrl = getCtrler(arg1, CTRLER_USED);
            if (CC_UNLIKELY(spCtrl.get() == nullptr)) {
                MY_LOGW("error, cannot fine req %d.", arg1);
                return OK;
            }
            spCtrl->clearBelong(arg1);
            if(spCtrl->isEmptyBelong())
                spCtrl->doCancel();
        } break;
    };
    return OK;
}


status_t ControllerHandler::dump(MINT32 /* openId */)
{
    FUNCTION_SCOPE;
    return OK;
}


void ControllerHandler::drain(bool bCancel /* = true */)
{
    FUNCTION_SCOPE;
    // moves all controller from container(s)
    auto CC = std::move(mUsedCtrlers);

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<BaseController>::ContainerType Data_T;
    CC.iterate([bCancel](Data_T& data) {
        for (auto&& itr : data)
            if (itr.get() && bCancel)
                itr->doCancel();

        for (auto&& itr : data)
            if (itr.get())
                itr->waitExecution();

    });
}

// ----------------------------------------------------------------------------
// Controller
// ----------------------------------------------------------------------------
std::shared_ptr<BaseController>
ControllerHandler::create_controller(
    int  /*openId*/,
    int  /*logLevel*/,
    bool /*isZsd*/
)
{
    return nullptr;
}

ControllerContainer<BaseController>*
ControllerHandler::getCtrlerContainer(CTRLER u)
{
    switch (u) {
    case CTRLER_USED:
        return &mUsedCtrlers;
    case CTRLER_UNUSED:
        return &mUnusedCtrlers;
    default:
        MY_LOGE("get wrong controller container (enum=%d)", u);
        return nullptr;
    }
    return nullptr;
}


std::shared_ptr<BaseController>
ControllerHandler::getCtrler(CTRLER u)
{
    FUNCTION_SCOPE;
    auto pCC = getCtrlerContainer(u);

    std::shared_ptr<BaseController> ctrler;
    auto openId  = getOpenId();

    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<BaseController>::ContainerType Data_T;

    pCC->iterate
    (
        [this, &ctrler, openId]
        (Data_T& data)
        {
            // if no controller in container, create a new one and push back into it
            if (data.size() <= 0) {
                ctrler = create_controller(openId, 1, true);
                data.push_back(ctrler);
            }
            // or get the first controller
            else {
                ctrler = data.front();
            }
        }
    );

    return ctrler;
}


std::shared_ptr<BaseController>
ControllerHandler::getCtrler(MUINT32 requestNo, CTRLER u)
{
    FUNCTION_SCOPE;
    return getCtrlerContainer(u)->belong_to(requestNo);
}


std::shared_ptr<BaseController>
ControllerHandler::takeCtrler(CTRLER u)
{
    FUNCTION_SCOPE;
    return getCtrlerContainer(u)->take_first();
}


void ControllerHandler::pushCtrler(const std::shared_ptr<BaseController>& c, CTRLER u)
{
    FUNCTION_SCOPE;
    getCtrlerContainer(u)->push_back(c);
}


bool ControllerHandler::removeCtrler(std::shared_ptr<BaseController> c, CTRLER u)
{
    FUNCTION_SCOPE;
    return getCtrlerContainer(u)->remove(c);
}

