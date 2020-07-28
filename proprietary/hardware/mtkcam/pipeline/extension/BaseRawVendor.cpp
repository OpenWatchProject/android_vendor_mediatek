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
#define LOG_TAG "BaseRawVendor"
static const char* __CALLERNAME__ = LOG_TAG;

#include "BaseRawVendor.h"

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
#define THREAD_SLOT_SIZE    1

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


using namespace android;
using namespace NSCam::plugin;


BaseRawVendor::BaseRawVendor(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
    )
    : BaseVendor(i4OpenId, vendorMode, String8::format("BaseRawVendor"))
{
    FUNCTION_SCOPE;
    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    setThreadShotsSize(THREAD_SLOT_SIZE);
}


BaseRawVendor::~BaseRawVendor()
{
}


status_t BaseRawVendor::get(
        MINT32           openId         __attribute__((unused)),
        const InputInfo& in             __attribute__((unused)),
        FrameInfoSet&    out            __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // copy from InputInfo.
    MetaItem item;
    item.setting.appMeta = in.appCtrl;
    item.setting.halMeta = in.halCtrl;
    out.settings.push_back(item);

    // configure it
    out.table.vendorMode    = mMode;
    out.table.inCategory    = FORMAT_RAW;
    out.table.outCategory   = FORMAT_RAW;
    out.table.bufferType    = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount          = out.settings.size();

    // create StreamId Map for RequestFrame
    {
        RequestFrame::StreamIdMap_Img map;

        if (in.fullRaw.get()) {
            map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
        }
        else {
            MY_LOGD("no src: fullsize img");
        }

        if (in.resizedRaw.get()) {
            map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
        }
        else {
            MY_LOGD("no src: resized img");
        }

        m_streamIdMap_img = std::move(map);
    }

    return OK;
}


status_t BaseRawVendor::set(
        MINT32              openId              __attribute__((unused)),
        const InputSetting& in                  __attribute__((unused)))
{
    FUNCTION_SCOPE;

    asyncRun([this](){
            ::prctl(PR_SET_NAME, "BaseRawVendor::asyncRun", 0, 0, 0);

            std::shared_ptr<RequestFrame> pFrame;
            {
                MY_LOGD("deque +");
                std::unique_lock<std::mutex> __l(m_requestFrameQueueMx);
                if (m_requestFrameQueue.empty()) {
                    m_requestFrameQueueCv.wait(__l);
                }
                pFrame = m_requestFrameQueue.front();
                m_requestFrameQueue.pop();
                MY_LOGD("deque -");
            }

            if (CC_UNLIKELY(pFrame.get() == nullptr)) {
                MY_LOGE("unexpected error, got a null frame from queue");
                return;
            }

            this->onProcessFullRaw(
                pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc),
                pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest),
                pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1)
                );
            this->onProcessRrzoRaw(
                pFrame->getImageBuffer(RequestFrame::eRequestImg_ResizedSrc),
                pFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest),
                pFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1)
                );

            return;
    });

    return OK;
}


status_t BaseRawVendor::queue(
        MINT32  const              openId           __attribute__((unused)),
        MUINT32 const              requestNo        __attribute__((unused)),
        wp<IVendor::IDataCallback> cb               __attribute__((unused)),
        BufferParam                bufParam         __attribute__((unused)),
        MetaParam                  metaParam        __attribute__((unused)))
{
    FUNCTION_SCOPE;

    // create a RequestFrame by call by reference variables
    std::shared_ptr<RequestFrame> f(new RequestFrame(
            openId,
            __CALLERNAME__,
            bufParam,
            metaParam,
            cb,
            m_streamIdMap_img,
            m_streamIdMap_meta,
            requestNo
            ));

    {
        std::lock_guard<std::mutex> __l(m_requestFrameQueueMx);
        m_requestFrameQueue.push(f);
        m_requestFrameQueueCv.notify_one();
    }

    return OK;
}


void BaseRawVendor::onProcessFullRaw(
        IImageBuffer*   /* pRaw */,
        IMetadata*      /* pAppMeta */,
        IMetadata*      /* pHalMeta */
        )
{
    FUNCTION_SCOPE;
}

void BaseRawVendor::onProcessRrzoRaw(
        IImageBuffer*   /* pRaw */,
        IMetadata*      /* pAppMeta */,
        IMetadata*      /* pHalMeta */
        )
{
    FUNCTION_SCOPE;
}


status_t BaseRawVendor::beginFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;

    drain(); // cancel flow, and drain.

    return OK;
}


status_t BaseRawVendor::endFlush(MINT32 /*openId*/)
{
    FUNCTION_SCOPE;
    return OK;
}


status_t BaseRawVendor::sendCommand(
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


status_t BaseRawVendor::dump(MINT32 /* openId */)
{
    FUNCTION_SCOPE;
    return OK;
}


void BaseRawVendor::drain(bool bCancel /* = true */)
{
    if (bCancel) {
        std::lock_guard<std::mutex> __l(m_requestFrameQueueMx);
        m_requestFrameQueueCv.notify_all();
    }
}
