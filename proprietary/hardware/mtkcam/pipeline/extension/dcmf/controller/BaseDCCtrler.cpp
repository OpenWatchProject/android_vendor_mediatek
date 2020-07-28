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

#define LOG_TAG "BaseDCCtrler"
static const char* __CALLERNAME__ = LOG_TAG;

#include "BaseDCCtrler.h"
// log
#include <mtkcam/utils/std/Log.h>
// stereo
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

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
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace NSCam::TuningUtils;
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
BaseDCCtrler::
BaseDCCtrler(
    DualCamMFType mode
) : BaseController(__CALLERNAME__)
  , mMode(mode)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    if(!StereoSettingProvider::getStereoSensorIndex(mMain1OpenId, mMain2OpenId))
    {
        MY_LOGE("Cannot get sensor ids from StereoSettingProvider!");
    }

    mUniqueKey = static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime()); // update unique key
    if (mRule.isREADEnable("MFNR") && mRule.getFile_uniqueKey("MFNR", getMain1OpenId())!= 0){
        MY_LOGD("get file unique key");
        mUniqueKey = mRule.getFile_uniqueKey("MFNR", getMain1OpenId());
    }

    MY_LOGD("open id: (%d:%d)  unique key: (%d)", getMain1OpenId(), getMain2OpenId(), mUniqueKey);
    mFrameSets.clear();

    // -1 means not exists
    m_originalAppSetting.resize(
            static_cast<size_t>(AppSetting::SIZE),
            -1);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
BaseDCCtrler::
~BaseDCCtrler()
{
    FUNCTION_SCOPE;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
intptr_t
BaseDCCtrler::
job(
    intptr_t arg1  /* = 0 */     __attribute__((unused)),
    intptr_t arg2  /* = 0 */     __attribute__((unused))
)
{
    SCOPE_TIMER(__t0, __FUNCTION__);
    sp<PipeReq> pPipeRequest = nullptr;
    // get pair frame and do denoise
    for(MUINT32 i = 0;i<getCaptureNum();i++)
    {
        auto main1Frame = dequeFrame();
        auto main2Frame = dequeFrame();
        if(main1Frame.get() == nullptr || main1Frame->isBroken())
        {
            if (main1Frame.get() == nullptr) {
                MY_LOGE("dequed a empty main1Frame, idx=%u", i);
            }
            else if (main1Frame->isBroken()) {
                MY_LOGE("dequed a main1Frame but marked as error, idx=%u", i);
            }
        }
        if(main2Frame.get() == nullptr || main2Frame->isBroken())
        {
            if (main2Frame.get() == nullptr) {
                MY_LOGE("dequed a empty main2Frame, idx=%u", i);
            }
            else if (main2Frame->isBroken()) {
                MY_LOGE("dequed a main2Frame but marked as error, idx=%u", i);
            }
        }
        pPipeRequest = generatePipeRequst(mpCB, main1Frame, main2Frame);
        if(pPipeRequest == nullptr)
        {
            MY_LOGE("build pipeRequest fail");
            continue;
        }
        if(!updateDCMFRequest(pPipeRequest, main1Frame, main2Frame, (i == 0), (i == getCaptureNum() - 1)))
        {
            MY_LOGE("update pipeRequest fail");
            continue;
        }

        if(!enquePipeRequest(pPipeRequest, main1Frame, main2Frame))
        {
            MY_LOGE("enque fail");
            continue;
        }
    }
    return 0;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
int
BaseDCCtrler::
enqueFrame(
    const MINT32 openId,
    MUINT32 requestNo,
    const IVendor::BufferParam& bufParam,
    const IVendor::MetaParam& metaParam,
    wp<IVendor::IDataCallback> cb
)
{
    SCOPE_TIMER(__t0, __FUNCTION__);
    MY_LOGD("[%d] request(%d) is enqued.", openId, requestNo);
    typedef std::mutex T_LOCKER;
    static T_LOCKER _locker;

    {
        std::lock_guard<T_LOCKER> _l(_locker);
        auto buildRequestFrame = [this](
                        const MINT32 openId,
                        MINT32 requestNo,
                        const IVendor::BufferParam& bufParam,
                        const IVendor::MetaParam& metaParam,
                        wp<IVendor::IDataCallback> cb)
        {
            return std::shared_ptr<RequestFrame>(
                            new RequestFrame
                            (
                                openId,
                                m_callerName,
                                bufParam,
                                metaParam,
                                cb,
                                m_streamIdMap_img,
                                m_streamIdMap_meta,
                                requestNo,
                                getEnquedTimes()
                            )
                    );
        };
        auto frame = buildRequestFrame(
                            openId,
                            requestNo,
                            bufParam,
                            metaParam,
                            cb);
        enqueToSpecificQueue(openId, frame);
        // check pair
        if(mMain1Frames.size()>0 && mMain2Frames.size()>0)
        {
            pushPairToIncomingQueue();
            mPairEnqueTimes++;
            MY_LOGD("mPairEnqueTimes(%d)", mPairEnqueTimes );
        }
    }
lbExit:
    return 0;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseDCCtrler::
enqueFrame(
    MINT32 openId,
    std::shared_ptr<RequestFrame> frame
)
{
    SCOPE_TIMER(__t0, __FUNCTION__);
    MY_LOGD("[%d] RequestFrame is enqued.", openId);
    typedef std::mutex T_LOCKER;
    static T_LOCKER _locker;

    {
        std::lock_guard<T_LOCKER> _l(_locker);
        enqueToSpecificQueue(openId, frame);
        // check pair
        if(mMain1Frames.size()>0 && mMain2Frames.size()>0)
        {
            pushPairToIncomingQueue();
        }
    }
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseDCCtrler::
clearFrames()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    typedef std::mutex T_LOCKER;
    static T_LOCKER _locker;
    std::lock_guard<T_LOCKER> _l(_locker);
    mMain1Frames.clear();
    mMain2Frames.clear();
    BaseController::clearFrames();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseDCCtrler::
doCancel()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    if(mFrameSets.size() != 0)
    {
        MY_LOGD("flush feature pipe");
        sp<Pipe> pipe = mwpPipe.promote();
        pipe->flush();
    }
    BaseController::doCancel();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseDCCtrler::
init(
    wp<Pipe> pipe
)
{
    mwpPipe = pipe;
    MY_LOGD("%p", mwpPipe.promote().get());
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseDCCtrler::
updateDCMFRequest(
    sp<PipeReq>& request __attribute__((unused)),
    std::shared_ptr<RequestFrame>& main1Frame           __attribute__((unused)),
    std::shared_ptr<RequestFrame>& main2Frame           __attribute__((unused)),
    MBOOL isMainFrame                                   __attribute__((unused)),
    MBOOL isLastFrame                                   __attribute__((unused))
)
{
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseDCCtrler::
restoreAppSetting(IMetadata* pAppMeta)
{
    MY_LOGD("pAppMeta:%p", pAppMeta);

    if (CC_UNLIKELY(pAppMeta == nullptr)) {
        MY_LOGE("%s: pAppMeta is NULL", __FUNCTION__);
        return;
    }
    // restore the original app setting back to pAppMeta if exists (value >= 0)
#define __CONTAINER m_originalAppSetting
#define __PMETADATA pAppMeta
#define __RESTORE_APP_SETTING(TAG, TYPE)                                        \
    if (__CONTAINER[static_cast<size_t>(AppSetting::TAG)] >= 0) {               \
        IMetadata::setEntry<TYPE>(                                              \
                __PMETADATA,                                                    \
                MTK_##TAG,                                                      \
                static_cast<TYPE>(                                              \
                    __CONTAINER[static_cast<size_t>(AppSetting::TAG)]));        \
    }                                                                           \
    else {                                                                      \
        __PMETADATA->remove(MTK_##TAG);                                         \
    }
    //
    __RESTORE_APP_SETTING(CONTROL_AE_MODE,                      MUINT8);
    __RESTORE_APP_SETTING(CONTROL_AWB_LOCK,                     MUINT8);
    __RESTORE_APP_SETTING(CONTROL_VIDEO_STABILIZATION_MODE,     MUINT8);
    __RESTORE_APP_SETTING(SENSOR_SENSITIVITY,                   MINT32);
    __RESTORE_APP_SETTING(SENSOR_EXPOSURE_TIME,                 MINT64);
    //
#undef __RESTORE_APP_SETTING
#undef __PMETADATA
#undef __CONTAINER
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseDCCtrler::
DCMFProcessingDone(
    PipeCB::EventId eventType __attribute__((unused)),
    sp<PipeReq>& request            __attribute__((unused))
)
{
    restoreAppSetting(request->getMetadata(DualCamMFBufferID::BID_META_IN_APP));
    restoreAppSetting(request->getMetadata(DualCamMFBufferID::BID_META_OUT_APP));

    // remove pipeRequest
    std::lock_guard<std::mutex> _l(mFrameSetLock);
    MY_LOGD("remove requestFrameSet(%d)", request->getRequestNo());
    int index = 0;
    bool found = false;
    for(auto itr:mFrameSets){
        if(itr->requestNo == request->getRequestNo())
        {
            found = true;
            break;
        }
        index++;
    }

    if(found){
        mFrameSets.erase(std::begin(mFrameSets) + index);
        return true;
    }else{
        MY_LOGE("remove requestFrameSet(%d) failed, can not be found in mFrameSets", request->getRequestNo());
        return false;
    }
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseDCCtrler::
postviewDone(
    PipeCB::EventId eventType  __attribute__((unused)),
    sp<PipeReq>& request
)
{
    FUNCTION_SCOPE;
    auto pMainFrame = m_mainRequestFrame.lock();
    if (pMainFrame.get() == NULL) {
        MY_LOGE("No main request frame ...");
        return false;
    }
    pMainFrame->releaseImageBuffer(RequestFrame::eRequestImg_PostviewOut);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseDCCtrler::
yuvDone(
    PipeCB::EventId eventType  __attribute__((unused)),
    sp<PipeReq>& request
)
{
    FUNCTION_SCOPE;

    std::lock_guard<std::mutex> _l(mFrameSetLock);
    MY_LOGD("release requestFrameSet(%d) YUVs and output Metas", request->getRequestNo());

    bool found = true;
    for(auto &e : mFrameSets){
        if( e->requestNo == request->getRequestNo() ){
            found = true;

            auto releaseYUVandMetas = [&](std::shared_ptr<RequestFrame> pFrame){
                pFrame->releaseImageBuffer(RequestFrame::eRequestImg_FullOut);
                pFrame->releaseImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
                pFrame->releaseImageBuffer(RequestFrame::eRequestImg_ReservedOut1);
                pFrame->releaseMetadata(RequestFrame::eRequestMeta_OutAppResult);
                pFrame->releaseMetadata(RequestFrame::eRequestMeta_OutHalResult);
            };

            releaseYUVandMetas(e->main1Frame);
            releaseYUVandMetas(e->main2Frame);
            break;
        }
    }

    if( !found ){
        MY_LOGE("remove requestFrameSet(%d) failed, can not be found in mFrameSets", request->getRequestNo());
        return false;
    }

    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MUINT32
BaseDCCtrler::
queryCaptureFrameCount(
)
{
    FUNCTION_SCOPE;
    // change this value if you want to change frame number for one capture
    return getDCMFCaptureCnt();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
BaseDCCtrler::
enqueToSpecificQueue(
    MINT32 openId,
    std::shared_ptr<RequestFrame> frame
)
{
    bool ret = false;
    if(openId == mMain1OpenId)
    {
        mMain1Frames.push_back(frame);
        ret = true;
    }
    else if(openId == mMain2OpenId)
    {
        mMain2Frames.push_back(frame);
        ret = true;
    }
    else
    {
        MY_LOGE("invaild open id (%d)", openId);
    }
    return ret;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
BaseDCCtrler::
pushPairToIncomingQueue(
)
{
    // step1. pop main1 queue first
    auto main1Frame = mMain1Frames.front();
    mMain1Frames.pop_front();
    // step2. pop main2 queue
    auto main2Frame = mMain2Frames.front();
    mMain2Frames.pop_front();
    // push main1Frame to inComing queue
    BaseController::enqueFrame(main1Frame);
    // push main2Frame to inComing queue
    BaseController::enqueFrame(main2Frame);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
sp<PipeReq>
BaseDCCtrler::
generatePipeRequst(
    sp<PipeCB> pipeCB __attribute__((unused)),
    std::shared_ptr<RequestFrame>& main1Frame           __attribute__((unused)),
    std::shared_ptr<RequestFrame>& main2Frame           __attribute__((unused))
)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);
    if(!main1Frame.get() || !main2Frame.get())
    {
        MY_LOGE("should not happened!");
        return nullptr;
    }
    sp<PipeReq> pPipeRequest = new PipeReq(
                                        main1Frame->getRequestNo(),
                                        pipeCB);

    auto packImageBufToRequest =
            [&pPipeRequest](const char* srcFrame,
                            std::shared_ptr<RequestFrame>& pFrame,
                            RequestFrame::eRequestImg tag,
                            DualCamMFBufferID pipeTag)
    {
        auto getImageBuffer = [&pFrame, &srcFrame](NSCam::plugin::RequestFrame::eRequestImg tag)
        -> IImageBuffer*
        {
            auto toString = [](NSCam::plugin::RequestFrame::eRequestImg tag)
            {
                if(tag == RequestFrame::eRequestImg_FullSrc)
                {
                    return "eRequestImg_FullSrc";
                }
                else if(tag == RequestFrame::eRequestImg_ResizedSrc)
                {
                    return "eRequestImg_ResizedSrc";
                }
                else if(tag == RequestFrame::eRequestImg_LcsoRaw)
                {
                    return "eRequestImg_LcsoRaw";
                }
                else if(tag == RequestFrame::eRequestImg_FullOut)
                {
                    return "eRequestImg_FullOut";
                }
                else if(tag == RequestFrame::eRequestImg_ThumbnailOut)
                {
                    return "eRequestImg_ThumbnailOut";
                }
                else if(tag == RequestFrame::eRequestImg_PostviewOut)
                {
                    return "eRequestImg_PostviewOut";
                }
                else if(tag == RequestFrame::eRequestImg_ReservedOut1)
                {
                    return "eRequestImg_ReservedOut1";
                }
                return "unknown";
            };
            IImageBuffer* pImgBuf = pFrame->getImageBuffer(tag);
            if(CC_LIKELY(pImgBuf))
            {
                return pImgBuf;
            }
            else
            {
                MY_LOGW("[%s] can not get image buffer: %s!", srcFrame, toString(tag));
                return nullptr;
            }
        };
        pPipeRequest->addImageBuffer(pipeTag, getImageBuffer(tag));
    };
    // get input buffer
    // main 1
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_FullSrc,    DualCamMFBufferID::BID_INPUT_FSRAW_1);
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_ResizedSrc, DualCamMFBufferID::BID_INPUT_RSRAW_1);
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_LcsoRaw,    DualCamMFBufferID::BID_LCS_1);
    // main 2
    packImageBufToRequest("main2Frame", main2Frame, RequestFrame::eRequestImg_FullSrc,    DualCamMFBufferID::BID_INPUT_FSRAW_2);
    packImageBufToRequest("main2Frame", main2Frame, RequestFrame::eRequestImg_ResizedSrc, DualCamMFBufferID::BID_INPUT_RSRAW_2);
    packImageBufToRequest("main2Frame", main2Frame, RequestFrame::eRequestImg_LcsoRaw,    DualCamMFBufferID::BID_LCS_2);
    // get out buffer
    // main 1
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_FullOut,        DualCamMFBufferID::BID_FS_YUV_1);
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_ThumbnailOut,   DualCamMFBufferID::BID_RS_YUV_1);
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_PostviewOut,    DualCamMFBufferID::BID_POSTVIEW);
    packImageBufToRequest("main1Frame", main1Frame, RequestFrame::eRequestImg_ReservedOut1,   DualCamMFBufferID::BID_SM_YUV);
    // main 2
    packImageBufToRequest("main2Frame", main2Frame, RequestFrame::eRequestImg_FullOut,        DualCamMFBufferID::BID_FS_YUV_2);
    packImageBufToRequest("main2Frame", main2Frame, RequestFrame::eRequestImg_ThumbnailOut,   DualCamMFBufferID::BID_RS_YUV_2);

    // online tuning, read file from sdcard
    {
        auto supportReadFile = [&pPipeRequest, this](DualCamMFBufferID pipeTag){
            sp<IImageBuffer> pBuf = pPipeRequest->getImageBuffer(pipeTag);
            mRule.getFile_RAW(mSubmittedCount, "MFNR", pBuf.get(), "MFNR", getMain1OpenId());
            pPipeRequest->addImageBuffer(pipeTag, pBuf);
        };

        supportReadFile(DualCamMFBufferID::BID_INPUT_FSRAW_1);
        supportReadFile(DualCamMFBufferID::BID_INPUT_RSRAW_1);
        supportReadFile(DualCamMFBufferID::BID_LCS_1);
    }

    auto packMetadataToRequest =
            [&pPipeRequest](const char* srcFrame,
                            std::shared_ptr<RequestFrame>& pFrame,
                            RequestFrame::eRequestMeta tag,
                            DualCamMFBufferID pipeTag)
    {
        auto getMetadataBuffer = [&pFrame, &srcFrame](NSCam::plugin::RequestFrame::eRequestMeta tag)
        -> IMetadata*
        {
            auto toString = [](NSCam::plugin::RequestFrame::eRequestMeta tag)
            {
                if(tag == RequestFrame::eRequestMeta_InAppRequest)
                {
                    return "eRequestMeta_InAppRequest";
                }
                else if(tag == RequestFrame::eRequestMeta_InHalP1)
                {
                    return "eRequestMeta_InHalP1";
                }
                return "";
            };
            IMetadata* pMetaBuf = pFrame->getMetadata(tag);
            if(CC_LIKELY(pMetaBuf))
            {
                return pMetaBuf;
            }
            else
            {
                MY_LOGD("[%s] can not get meta buffer: %s", srcFrame, toString(tag));
                return nullptr;
            }
        };
        pPipeRequest->addMetadata(pipeTag, getMetadataBuffer(tag));
    };
    packMetadataToRequest("main1Frame", main1Frame, RequestFrame::eRequestMeta_InAppRequest, DualCamMFBufferID::BID_META_IN_APP);

    packMetadataToRequest("main1Frame", main1Frame, RequestFrame::eRequestMeta_InHalP1,      DualCamMFBufferID::BID_META_IN_HAL);
    packMetadataToRequest("main2Frame", main2Frame, RequestFrame::eRequestMeta_InHalP1,      DualCamMFBufferID::BID_META_IN_HAL_2);

    packMetadataToRequest("main1Frame", main1Frame, RequestFrame::eRequestMeta_OutAppResult, DualCamMFBufferID::BID_META_OUT_APP);

    packMetadataToRequest("main1Frame", main1Frame, RequestFrame::eRequestMeta_OutHalResult, DualCamMFBufferID::BID_META_OUT_HAL);
    packMetadataToRequest("main2Frame", main2Frame, RequestFrame::eRequestMeta_OutHalResult, DualCamMFBufferID::BID_META_OUT_HAL_2);

    {
        IMetadata* pHalMeta_In = main1Frame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
        IMetadata* pHalMeta_Out = main1Frame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);

        // update EV value
        {
            MINT32 EV = 0;
            if( !IMetadata::getEntry<MINT32>(pHalMeta_In, MTK_STEREO_HDR_EV, EV) ){
                IMetadata::setEntry(pHalMeta_In, MTK_STEREO_HDR_EV, 0);
                MY_LOGD("hdr EV not exists in input meta, manual update to(%d)", 0);
            }
        }

        // The OutHalMeta should contain all values of InHalMeta
        *pHalMeta_Out = *pHalMeta_Out + *pHalMeta_In;

        // update normal data dump naming info
        {
            auto packDumpInfoToMeta = [](IMetadata* pMeta, MINT32 uniqueKey, MINT32 frameNum, MINT32 reqNum){
                IMetadata::setEntry(pMeta, MTK_PIPELINE_UNIQUE_KEY,      uniqueKey);
                IMetadata::setEntry(pMeta, MTK_PIPELINE_FRAME_NUMBER,    frameNum);
                IMetadata::setEntry(pMeta, MTK_PIPELINE_REQUEST_NUMBER,  reqNum);
            };
            MY_LOGD("set dumpHint (%d,%d,%d)", mUniqueKey, 0, mSubmittedCount);
            // for foreground shot
            packDumpInfoToMeta(pHalMeta_In,  mUniqueKey, 0, mSubmittedCount);
            // for background ispPipe
            packDumpInfoToMeta(pHalMeta_Out, mUniqueKey, 0, mSubmittedCount);
        }
    }

    mSubmittedCount ++;

    return pPipeRequest;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
MBOOL
BaseDCCtrler::
enquePipeRequest(
    sp<PipeReq>& request,
    std::shared_ptr<RequestFrame>& main1Frame           __attribute__((unused)),
    std::shared_ptr<RequestFrame>& main2Frame           __attribute__((unused))
)
{
    FUNCTION_SCOPE;
    MBOOL ret = MFALSE;
    sp<Pipe> pipe = mwpPipe.promote();
    if(CC_LIKELY(pipe.get()))
    {
        MY_LOGD("req(%d)", request->getRequestNo());
        ret = pipe->enque(request);
    }
    else
    {
        MY_LOGW("promote pipe fail");
    }
    // queue pipe request to vector
    if(ret)
    {
        std::lock_guard<std::mutex> _l(mFrameSetLock);
        MY_LOGD("build requestFrameSet(%d)", main1Frame->getRequestNo());
        mFrameSets.push_back(
                    std::shared_ptr<ProcessRequestFrameSet>(
                            new ProcessRequestFrameSet(
                                        main1Frame->getRequestNo(),
                                        main1Frame,
                                        main2Frame)));
    }
    MY_LOGD("ret(%d)", ret);
    return ret;
}