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
#define LOG_TAG "mixYuvImp"
#include "mixYuvImp.h"

#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer
//
static const char* __CALLERNAME__ = LOG_TAG;

// LINUX
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// AOSP
#include <cutils/compiler.h>

// MixYuv deque frame timeout. Unit is: ms
#if defined(MTKCAM_LOG_LEVEL_DEFAULT) && (MTKCAM_LOG_LEVEL_DEFAULT <= 2)
// user load, reduce timeout
#define MIXYUVCTRLER_DEQUE_FRAME_TIMEOUT            5000
#else
#define MIXYUVCTRLER_DEQUE_FRAME_TIMEOUT            10000
#endif

// MY_LOG
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
#endif

using namespace android;
using namespace NSCam;
using namespace NSCam::plugin;


mixYuvImp::mixYuvImp(
    const char*     pcszName,
    const MINT32    i4OpenId,
    const MINT64    vendorMode
)
    : ControllerHandler(pcszName, i4OpenId, vendorMode)
{
    FUNCTION_SCOPE;
}


mixYuvImp::~mixYuvImp()
{
    FUNCTION_SCOPE;
}

std::shared_ptr<BaseController>
mixYuvImp::create_controller(
    int  openId,
    int  logLevel,
    bool isZsd
)
{
    std::shared_ptr<BaseController> c
        (
            dynamic_cast<BaseController*>(new mixYuvCtrler(openId, logLevel >= 1, isZsd))
        );

    return c;
}

status_t
mixYuvImp::get(
    MINT32           openId,
    const InputInfo& in,
    FrameInfoSet&    out
)
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

    // create StreamId Map for RequestFrame
    setMap(in, out, ctrler);

    return OK;
    // return ControllerHandler::get(openId, in, out);
}

// ----------------------------------------------------------------------------
// mixYuvCtrler
// ----------------------------------------------------------------------------
mixYuvCtrler::mixYuvCtrler(int sensorId, bool enableLog __attribute__((unused)) , bool zsdFlow __attribute__((unused)) )
    : BaseController(__CALLERNAME__)
    , m_bFired(false)
    , m_OpenId(sensorId)
    , m_DoneState(0)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

}


mixYuvCtrler::~mixYuvCtrler()
{
    FUNCTION_SCOPE;
}

intptr_t mixYuvCtrler::job(
        intptr_t arg1 /* = 0 */     __attribute__((unused)),
        intptr_t arg2 /* = 0 */     __attribute__((unused)))
{
    onProcess();
    return 0;
}

bool mixYuvCtrler::onProcess()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    if (m_bFired) {
        MY_LOGE("xxx has been fired, cannot fire twice");
        return false;
    }

    m_bFired = true;

    auto t1 = std::async(std::launch::async, [this] {
        ::prctl(PR_SET_NAME, "PROC_DEQUE_FRAMES", 0, 0, 0);
        auto __scope_logger__ = create_scope_logger("thread_deque_frame");

        bool hasResult = false;
        // deque frame.
        for (size_t i = 0; i < static_cast<size_t>(getFrameSize()); i++) {
            MY_LOGD("deque RequestFrame %zu", i);

            MY_LOGD("deque RequestFrame %zu [+]", i);
            // deque frame with a timeout
            std::shared_ptr<RequestFrame> pFrame;
            constexpr const int __TIMEOUT = MIXYUVCTRLER_DEQUE_FRAME_TIMEOUT; // ms
            auto status = mixYuvCtrler::dequeFrame(pFrame, __TIMEOUT); // deque frame until timeout
            if (CC_UNLIKELY( status == std::cv_status::timeout )) {
                MY_LOGE("deque frame timout (%dms), ignore this frame", __TIMEOUT);
                pFrame = nullptr;
            }
            MY_LOGD("deque RequestFrame %zu [-]", i);

            if(isInvalidated()) {
                MY_LOGD("invalidated, discard this frame");
                std::lock_guard<std::mutex> __l(m_DoneMx);
                m_DoneState = 1;
                m_DoneCond.notify_one();
                return;
            }

            // check frame.
            if (pFrame.get() == nullptr || pFrame->isBroken()) {
                MY_LOGW("dequed frame but it's nullptr, discard this frame");
                continue;
            }

            /* retrieve metadata */
            IMetadata* pAppMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
            IMetadata* pHalMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
            IMetadata* pAppMetaOut      = pFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
            IMetadata* pHalMetaOut      = pFrame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);
            MUINT32 requestNo           = pFrame->getRequestNo();
            /* retrieve input source */
            IImageBuffer* pResult       = pFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);

            {
                CAM_TRACE_NAME("handle_jpeg_thumb");
                // handle jpeg main & thumbnail
                auto pFrameYuvJpeg = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
                auto pFrameYuvThub = pFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
                auto outOrientaion = pFrame->getOrientation(RequestFrame::eRequestImg_FullOut);

                bool ret = true;
                IImageBuffer* dst1 = pFrameYuvJpeg;
                IImageBuffer* dst2 = pFrameYuvThub;
                hasResult = (dst1!=nullptr || dst2!=nullptr) ? true: false;
                ret  = handleYuvOutput("Process output capture buffer.",
                            pResult, pFrameYuvJpeg, pFrameYuvThub,
                            pAppMeta, pHalMeta,
                            getOpenId(), outOrientaion, requestNo, true/*clearzoom*/, true/*crop1*/, true/*crop2*/,
                            pHalMetaOut/*exifMeta*/);
                if (!ret) MY_LOGE("handle yuv output fail.");

                pFrame->releaseImageBuffer(RequestFrame::eRequestImg_FullOut);
                pFrame->releaseImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
            }

            std::vector<RequestFrame::eRequestImg> outputList;
            {
                int begin = static_cast<int>(RequestFrame::eRequestImg_Out_Start);
                int end   = static_cast<int>(RequestFrame::eRequestImg_Out_End);

                for (int i = begin; i < end; i++){
                    auto reqId = static_cast<RequestFrame::eRequestImg>(i);
                    IImageBuffer* pImage = pFrame->getImageBuffer(reqId);
                    if(pImage!=nullptr) outputList.push_back(reqId);
                }
            }
            // handle other yuv output
            if(!outputList.empty()) {
                CAM_TRACE_NAME("handle_other_yuv");
                hasResult = true;
                bool ret  = true;
                IImageBuffer* dst1 = pFrame->getImageBuffer(outputList[0]);
                IImageBuffer* dst2 = (outputList.size() > 1) ? pFrame->getImageBuffer(outputList[1]) : nullptr;
                ret  = handleYuvOutput("Process other yuv.",
                            pResult, dst1, dst2,
                            pAppMeta, pHalMeta,
                            getOpenId(),    // openId
                            0,              // outOrientaion
                            requestNo,      // requestNo
                            true,           // clearzoom
                            true,           // crop1
                            true,           // crop2
                            pHalMetaOut     // exifMeta
                            );
                if (!ret) MY_LOGE("handle yuv output fail.");
            }
            else {
                MY_LOGW("output list is empty");
            }

            if(hasResult) {
                MY_LOGD("got result, notify condition");
                std::lock_guard<std::mutex> __l(m_DoneMx);
                m_DoneState = 1;
                m_DoneCond.notify_one();
                return;
            }
            else {
                MY_LOGD("still no result, keep going");
            }

        } // for-loop: for every frames

        {
            std::lock_guard<std::mutex> __l(m_DoneMx);
            MY_LOGW("deque frames failed, do nothing");
            m_DoneState = 1;
            m_DoneCond.notify_one();
        }
    }); // async call

    {
        // wait done
        std::unique_lock<std::mutex> locker(m_DoneMx);
        // check if finished or not
        if (m_DoneState == 0) {
            MY_LOGD("Wait... +");
            m_DoneCond.wait(locker);
            MY_LOGD("Wait... -");
        }
        else {
            MY_LOGD("No need to wait, already finished");
        }
        MY_LOGD("gogo");

        // clear resources.
        cleanUp();
    }

    return true;
}

void mixYuvCtrler::cleanUp()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    invalidate();
}

void mixYuvCtrler::doCancel()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);


    BaseController::doCancel();
}

