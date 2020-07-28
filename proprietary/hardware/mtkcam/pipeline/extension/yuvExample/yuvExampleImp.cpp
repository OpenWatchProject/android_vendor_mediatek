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
#include "yuvExampleImp.h"

#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer
//
#define LOG_TAG "yuvExampleImp"
static const char* __CALLERNAME__ = LOG_TAG;

// LINUX
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// AOSP
#include <cutils/compiler.h>

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


yuvExampleImp::yuvExampleImp(
    const char*     pcszName,
    const MINT32    i4OpenId,
    const MINT64    vendorMode
)
    : ControllerHandler(pcszName, i4OpenId, vendorMode)
{
    FUNCTION_SCOPE;
}


yuvExampleImp::~yuvExampleImp()
{
    FUNCTION_SCOPE;
}

std::shared_ptr<BaseController>
yuvExampleImp::create_controller(
    int  openId,
    int  logLevel,
    bool isZsd
)
{
    std::shared_ptr<BaseController> c
        (
            dynamic_cast<BaseController*>(new yuvCtrler(openId, logLevel >= 1, isZsd))
        );

    return c;
}

status_t
yuvExampleImp::get(
    MINT32           openId,
    const InputInfo& in,
    FrameInfoSet&    out
)
{
    FUNCTION_SCOPE;

    // copy from InputInfo.
    MetaItem item;
    item.setting.appMeta = in.appCtrl;
    item.setting.halMeta = in.halCtrl;
    out.settings.push_back(item);

    // configure it
    out.table.vendorMode    = mMode;
    out.table.inCategory    = FORMAT_YUV;
    out.table.outCategory   = FORMAT_YUV;
    out.table.bufferType    = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount          = out.settings.size();

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
// yuvCtrler
// ----------------------------------------------------------------------------
yuvCtrler::yuvCtrler(int sensorId, bool enableLog, bool zsdFlow)
    : BaseController(__CALLERNAME__)
    , m_bFired(false)
    , m_OpenId(sensorId)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

}


yuvCtrler::~yuvCtrler()
{
    FUNCTION_SCOPE;
}

intptr_t yuvCtrler::job(
        intptr_t arg1 /* = 0 */     __attribute__((unused)),
        intptr_t arg2 /* = 0 */     __attribute__((unused)))
{
    onProcess();
    return 0;
}

bool yuvCtrler::onProcess()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    if (m_bFired) {
        MY_LOGE("Has been fired, cannot fire twice");
        return false;
    }

    m_bFired = true;

    auto t1 = std::async(std::launch::async, [this] {
        ::prctl(PR_SET_NAME, "PROC_DEQUE_FRAMES", 0, 0, 0);

        // deque frame.
        for (size_t i = 0; i < static_cast<size_t>(getFrameSize()); i++) {
            MY_LOGD("deque RequestFrame %zu", i);

            auto pFrame = yuvCtrler::dequeFrame();

            if(isInvalidated()) {
                std::lock_guard<std::mutex> __l(m_DoneMx);
                m_DoneCond.notify_one();
                break;
            }

            // check frame.
            if (pFrame.get() == nullptr || pFrame->isBroken()) {
                continue;
            }

            /* retrieve metadata */
            IMetadata* pAppMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
            IMetadata* pHalMeta         = pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
            IMetadata* pAppMetaOut      = pFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
            IMetadata* pHalMetaOut      = pFrame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);
            /* retrieve image buffer that we care */
            IImageBuffer* pImageBuffer  = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
            IImageBuffer* pImageLcsoRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_LcsoRaw);


        {
            std::lock_guard<std::mutex> __l(m_DoneMx);
            m_DoneCond.notify_one();
        }

        } // for-loop: for every frames
    }); // async call

    {
        // wait done
        std::unique_lock<std::mutex> locker(m_DoneMx);
        MY_LOGD("Wait...");
        m_DoneCond.wait(locker);
        /*onNextCaptureReady();
        onShutter();*/
        MY_LOGD("gogo");

        // clear resources.
        cleanUp();
    }

    return true;
}

void yuvCtrler::cleanUp()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    invalidate();
}

void yuvCtrler::doCancel()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);


    // Call BaseController::doCancel to set state as invalid and
    // wake deque.
    BaseController::doCancel();
}

