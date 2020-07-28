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
#define __DEBUG
#define LOG_TAG "DCMFVendorImp"
static const char* __CALLERNAME__ = LOG_TAG;
#include "DCMFImp.h"
#include "controller/DCMFCtrler.h"
// mtkcam
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/StlUtils.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
// AOSP
#include <sys/prctl.h>
#include <cutils/compiler.h>
// ----------------------------------------------------------------------------
//
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
// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#ifdef __DEBUG
#define FUNCTION_SCOPE(id)      auto __scope_logger__ = create_scope_logger(id, __FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(MINT32 id, const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%d] [%s] +", id, pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
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
#define FEATURE_SLOT_SIZE (2)
//
using namespace android;
using namespace NSCam::plugin;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace NS3Av3;
using NS3Av3::CaptureParam_T;
using NS3Av3::IHal3A;
//
wp<DCMFVendor> DCMFVendorImp::mThis = nullptr;
sp<IDualCamMFPipe> DCMFVendorImp::gspFeaturePipe = nullptr;
MINT32 DCMFVendorImp::gReferenceCount = 0;
bool DCMFVendorImp::gbAlreadyGet = false;
bool DCMFVendorImp::gbFirstSet = true;
ControllerContainer<DCMFVendorImp::CtrlerType> DCMFVendorImp::sUnusedCtrlers;
static Mutex DCMFVendorCreateLock;
    //
static std::mutex mCtrlerLock;
//
std::future<void> DCMFVendorImp::gPipeSyncThread;
// ----------------------------------------------------------------------------
// helper function
// ----------------------------------------------------------------------------
static
inline
std::shared_ptr<BaseDCCtrler>
create_dcmf_controller(
    DualCamMFType mode
)
{
    std::shared_ptr<BaseDCCtrler> p = nullptr;
    if(mode == DualCamMFType::TYPE_DCMF_3rdParty || mode == DualCamMFType::TYPE_DCMF_3rdParty_HDR){
        MY_LOGD("create dcmf ctrler");
        p = std::shared_ptr<BaseDCCtrler>(
                            new DCMFCtrler(mode));
    }else{
        MY_LOGE("mode(%d) not support", mode);
    }
    return p;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
sp<DCMFVendor> DCMFVendor::createInstance(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
        )
{
    FUNCTION_SCOPE(i4OpenId);
    Mutex::Autolock _l(DCMFVendorCreateLock);
    sp<DCMFVendor> pInstance = DCMFVendorImp::mThis.promote();
    if(pInstance != nullptr)
    {
    }
    else
    {
        DCMFVendorImp::mThis = new DCMFVendorImp(pcszName, i4OpenId, vendorMode);
        pInstance = DCMFVendorImp::mThis.promote();
        MY_LOGD("[create by %d]createInstance() new %p", i4OpenId, pInstance.get());
    }
    DCMFVendorImp::gReferenceCount++;
    MY_LOGD("[%d] Count(%d): caller(%s)", i4OpenId, DCMFVendorImp::gReferenceCount, pcszName);
    return pInstance;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
DCMFVendorImp::
DCMFVendorImp(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
) : BaseVendor(i4OpenId, vendorMode, String8::format("DCMFvendor"))
{
    FUNCTION_SCOPE(i4OpenId);
    MY_LOGD("[%d] create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);
    setThreadShotsSize(FEATURE_SLOT_SIZE);
    // get main1 & main2 openId
    if(!StereoSettingProvider::getStereoSensorIndex(miMain1OpenId, miMain2OpenId))
    {
        MY_LOGE("get sensor id fail");
    }
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
DCMFVendorImp::
~DCMFVendorImp()
{
    FUNCTION_SCOPE(-1);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
get(
    MINT32           openId         __attribute__((unused)),
    const InputInfo& in             __attribute__((unused)),
    FrameInfoSet&    out            __attribute__((unused))
)
{
    FUNCTION_SCOPE(openId);
    SCOPE_TIMER(__t0, __FUNCTION__);

    auto udpateFeatureMode = [](const InputInfo& in, DualCamMFType& mode, const IMetadata& halCtrl)
    {
        _DualCamMFType originFeaureMode = mode;
        MINT32 dcmfFeatureMode = 0;
        for(auto itr : in.combination)
        {
            if(itr == MTK_PLUGIN_MODE_DCMF_3RD_PARTY)
            {
                auto b = IMetadata::getEntry<MINT32>(&halCtrl, MTK_STEREO_DCMF_FEATURE_MODE, dcmfFeatureMode);

                if(b){
                    switch(dcmfFeatureMode){
                        case MTK_DCMF_FEATURE_BOKEH:
                        case MTK_DCMF_FEATURE_MFNR_BOKEH:
                            mode = DualCamMFType::TYPE_DCMF_3rdParty;
                            break;
                        case MTK_DCMF_FEATURE_HDR_BOKEH:
                            mode = DualCamMFType::TYPE_DCMF_3rdParty_HDR;
                            break;
                        default:
                            MY_LOGW("unknown feature mode (%d)", dcmfFeatureMode);
                            mode = DualCamMFType::TYPE_DCMF_3rdParty;
                    }
                }else{
                    MY_LOGW("can not get MTK_STEREO_DCMF_FEATURE_MODE");
                    mode = DualCamMFType::TYPE_DCMF_3rdParty;
                }
            }
        }

        if(originFeaureMode != mode){
            return true;
        }else{
            return false;
        }
    };

    // lock
    std::shared_ptr<CtrlerType> ctrler = nullptr;
    {
        std::lock_guard<std::mutex> _g(mCtrlerLock);

        if(udpateFeatureMode(in, mFeatureMode, in.halCtrl))
        {
            MY_LOGD("feature mode changed to %d", mFeatureMode);
            // wait until drained
            drain();
        }

        // check need to create dualcamMF pipe
        if(DCMFVendorImp::gPipeSyncThread.valid())
        {
            SCOPE_TIMER(__t0, "wait sync");
            MY_LOGD("have share state");
            DCMFVendorImp::gPipeSyncThread.get();
        }
        if(    isJpegSizeChanged(openId, in)
            || gspFeaturePipe == nullptr
        ){
            if(CC_LIKELY(gspFeaturePipe.get()))
            {
                // release pipeline
                SCOPE_TIMER(__t1, "remove old feature pipe");
                gspFeaturePipe->sync();
                gspFeaturePipe->uninit();
                gspFeaturePipe = nullptr;
                //clean ctler
                DCMFVendorImp::sUnusedCtrlers.clear();
                mUsedCtrlers.clear();
            }
            DCMFVendorImp::gPipeSyncThread =
                std::async(std::launch::async,
                            [this, &openId]()
                            {
                                SCOPE_TIMER(__t1, "dualcamMF pipe create");
                                MY_LOGD("[%d] create new pipe", openId);
                                gspFeaturePipe = IDualCamMFPipe::createInstance(
                                                    miMain1OpenId,
                                                    miMain2OpenId,
                                                    mFeatureMode
                                                );
                                if(gspFeaturePipe != nullptr)
                                {
                                    gspFeaturePipe->init();
                                    gspFeaturePipe->sync();
                                }
                            });
        }
        ctrler = getCtrler(CTRLER_UNUSED, mFeatureMode);
        if(ctrler.get() == nullptr)
        {
                MY_LOGE("[%d] get controller fail", openId);
            return UNKNOWN_ERROR;
        }
    }
    MUINT32 frameCount = ctrler->getCaptureNum();
    MUINT32 frameDelayCount = ctrler->getDelayFrameNum();
#ifdef __DEBUG
    // {{{ show info
    MY_LOGD("[%d] %s: cap  = %u", openId, __FUNCTION__, frameCount);
    MY_LOGD("[%d] %s: delay = %u", openId, __FUNCTION__, frameDelayCount);
    // }}}
#endif
    // config capture setting
    for(MUINT32 i = 0 ; i<frameCount ; i++)
    {
        bool bLastFrame = ((i + 1) == frameCount);
        // default, copy from InputInfo
        MetaItem item;
        item.setting.appMeta = in.appCtrl;
        item.setting.halMeta = in.halCtrl;
        // update AE manual settings
        IMetadata& appSetting(item.setting.appMeta);
        if (i == 0) {
            // saves the default app setting to controller
            store_app_setting(&appSetting, ctrler.get());
        }

        MINT32 EV = 0;
        updateAppSettings(&appSetting, openId, mFeatureMode, i, EV);
        // modify hal control metadata
        IMetadata& halSetting(item.setting.halMeta);
        {
            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_FOCUS_PAUSE,
                    bLastFrame ? 0 : 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    &halSetting, MTK_HAL_REQUEST_DUMP_EXIF, 1);

            // HDR related info
            if(mFeatureMode == DualCamMFType::TYPE_DCMF_3rdParty_HDR){
                IMetadata::setEntry<MINT32>(
                    &halSetting, MTK_STEREO_HDR_EV, EV);
                IMetadata::setEntry<MINT32>(
                    &halSetting, MTK_STEREO_DELAY_FRAME_COUNT, frameDelayCount);
            }
        }
        out.settings.push_back(item);
    }
    // set out info
    out.table.vendorMode = mMode;
    out.table.inCategory = FORMAT_RAW;
    out.table.outCategory = FORMAT_YUV;
    out.table.bufferType  = BUFFER_NO_WORKING_OUTPUT;
    out.frameCount        = out.settings.size();
    toggleManualFlare(openId, MTRUE);
    // create streamid map for request frame
    {
        std::lock_guard<std::mutex> _g(mCtrlerLock);
        // set streamid map by main id only
        if(openId == miMain1OpenId)
        {
            // this varible is set true here, and set false when
            // frame is queued by p2.
            // To avoid set streamid map twice.
            if(!gbAlreadyGet)
            {
                SCOPE_TIMER (__t1, "Create StreamID map");
                RequestFrame::StreamIdMap_Img map;
                if (CC_LIKELY(in.fullRaw.get())) {
                    map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
                    MY_LOGD("set source size=%dx%d",
                            in.fullRaw->getImgSize().w, in.fullRaw->getImgSize().h);
                }
                else {
                    MY_LOGE("no full size RAW");
                }
                if (in.resizedRaw.get()) {
                    map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
                    MY_LOGD("set rrzo size=%dx%d",
                            in.resizedRaw->getImgSize().w, in.resizedRaw->getImgSize().h);
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
                    MY_LOGW("no output YUV");
                }
                if (CC_LIKELY(in.thumbnailYuv.get())) {
                    map[RequestFrame::eRequestImg_ThumbnailOut] = in.thumbnailYuv->getStreamId();
                }
                else {
                    MY_LOGW("no output Thumbnail");
                }
                if (CC_LIKELY(in.workingbuf.get())) {
                    map[RequestFrame::eRequestImg_WorkingBufferOut] = in.workingbuf->getStreamId();
                }
                else {
                    MY_LOGD("no working buffer info");
                }
                if (CC_LIKELY(in.postview.get())) {
                    map[RequestFrame::eRequestImg_PostviewOut] = in.postview->getStreamId();
                }
                else {
                    MY_LOGD("no post view buffer info");
                }
                if ( in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End ){
                    MY_LOGE("output yuv too much.");
                }
                for(size_t i = 0; i < in.vYuv.size(); ++i) {
                    if (CC_LIKELY(in.vYuv[i].get())) {
                        MY_LOGD("resrv(%d)", i);
                        map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
                    }
                }
                {
                    ctrler->setStreamIdMapImg(std::move(map));
                }
                gbAlreadyGet = true;
            }
        }
    }
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
set(
    MINT32      openId              __attribute__((unused)),
    const InputSetting& in          __attribute__((unused))
)
{
    FUNCTION_SCOPE(openId);
    std::lock_guard<std::mutex> _g(mCtrlerLock);
    // step 1: take the first unused denoise controller
    std::shared_ptr<DCMFVendorImp::CtrlerType> ctrler = nullptr;
    if(gbFirstSet)
    {
        // for first set, just can set requestNo and do not remove
        // from container.
        MY_LOGD("set request only");
        ctrler = getCtrler(CTRLER_UNUSED, mFeatureMode);
    }
    else
    {
        MY_LOGD("take controller and set request");
        ctrler = takeCtrler(CTRLER_UNUSED);
    }
    if (CC_UNLIKELY(ctrler.get() == nullptr)) {
        MY_LOGE("No unused DN controller");
        return UNKNOWN_ERROR;
    }
    // step 2:check capture num
    if (CC_UNLIKELY(ctrler->getCaptureNum() > in.vFrame.size())) {
        MY_LOGE("frame amount is not enough, re-set size to %zu",
                in.vFrame.size());
        if (in.vFrame.size() <= 0) {
            MY_LOGE("frame size is 0");
            return UNKNOWN_ERROR;
        }
    }
    // step 3.1: set request to this controller
    for(MUINT32 i=0;i<in.vFrame.size();i++)
    {
        if (i >= ctrler->getCaptureNum())
            break;
        MY_LOGD("[%d]set request(%d) to this controller", openId, in.vFrame[i].frameNo);
        ctrler->setBelong(in.vFrame[i].frameNo);
    }
    {
        MY_LOGD("ctler(%p)", ctrler.get());
        pushCtrler(ctrler, CTRLER_USED);
        //  3.2: set message callback
        if(openId == miMain1OpenId)
        {
            // only set callback for main1
            ctrler->setMessageCallback(in.pMsgCb);
        }
        if(!gbFirstSet)
        {
            MY_LOGD("[%d] second set done", openId);
            // Step4: fire denoise asynchronously and push this controller to
            //        used controller container
            asyncRun([this, ctrler]() mutable {
                    ::prctl(PR_SET_NAME, "doDCMF", 0, 0, 0);
                    ctrler->init(gspFeaturePipe);
                    ctrler->execute();
                    ctrler->waitExecution();
                    removeCtrler(ctrler, CTRLER_USED); // remove self from container
                    ctrler = nullptr; // clear smart pointer after use, this scope may
                                      // not exit until std::future::get has invoked.
                });
        }
        else
        {
            MY_LOGD("[%d] first set done", openId);
            gbFirstSet = MFALSE;
        }
    }
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
queue(
    MINT32  const              openId           __attribute__((unused)),
    MUINT32 const              requestNo        __attribute__((unused)),
    wp<IVendor::IDataCallback> cb               __attribute__((unused)),
    BufferParam                bufParam         __attribute__((unused)),
    MetaParam                  metaParam        __attribute__((unused))
)
{
    FUNCTION_SCOPE(openId);
    {
        std::lock_guard<std::mutex> _g(mCtrlerLock);
        if(DCMFVendorImp::gPipeSyncThread.valid())
        {
            SCOPE_TIMER(__t0, "wait sync");
            MY_LOGD("have share state");
            DCMFVendorImp::gPipeSyncThread.get();
        }
    }
#ifdef __DEBUG
    MY_LOGI("queue frame, requestNo=%u", requestNo);
#endif
    // Step 1:
    // find the used controller by request number
    auto spCtrl = getCtrler(requestNo, CTRLER_USED);
    MY_LOGD("spCtrl(%p)", spCtrl.get());
    if (CC_UNLIKELY(spCtrl.get() == nullptr)) {
        MY_LOGW("error, this frame is not belong to anyone.");
        drop(openId, cb, bufParam, metaParam);
        return OK;
    }
    spCtrl->init(gspFeaturePipe);
    // Step 2:
    // enque frame to controller.
    {
        MY_LOGD("enque frame");
        spCtrl->enqueFrame(openId, requestNo, bufParam, metaParam, cb);
    }
    {
        // reset status
        std::lock_guard<std::mutex> _g(mCtrlerLock);
        MY_LOGD("spCtrl->getPairEnqueTimes():%d spCtrl->getCaptureNum():%d",
            spCtrl->getPairEnqueTimes(),
            spCtrl->getCaptureNum()
        );
        if(spCtrl->getPairEnqueTimes() == spCtrl->getCaptureNum())
        {
            // reset
            MY_LOGD("reset flags");
            gbAlreadyGet = false;
            gbFirstSet = true;
            toggleManualFlare(miMain1OpenId, MFALSE);
            toggleManualFlare(miMain2OpenId, MFALSE);
        }
    }
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
beginFlush(
    MINT32 openId
)
{
    FUNCTION_SCOPE(openId);
    if(DCMFVendorImp::gPipeSyncThread.valid())
    {
        SCOPE_TIMER(__t0, "wait sync");
        MY_LOGD("have share state");
        DCMFVendorImp::gPipeSyncThread.get();
    }
    // use global lock to avoid drain same controller twice.
    std::lock_guard<std::mutex> _g(mCtrlerLock);
    drain(); // cancel flow, and drain.
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
endFlush(
    MINT32 openId
)
{
    FUNCTION_SCOPE(openId);
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
sendCommand(
    MINT32      cmd,
    MINT32      openId,
    MetaItem&   /* meta */,
    MINT32&     /* arg1 */,
    MINT32&     /* arg2 */,
    void*       /* arg3 */
)
{
    FUNCTION_SCOPE(openId);
    MY_LOGD("cmd(%d)", cmd);
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
status_t
DCMFVendorImp::
dump(
    MINT32 openId
)
{
    FUNCTION_SCOPE(openId);
    return OK;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFVendorImp::
onLastStrongRef(
    const void* id          __attribute__((unused))
)
{
    FUNCTION_SCOPE(-1);
    // reset all static member
    DCMFVendorImp::mThis = nullptr;
    DCMFVendorImp::gspFeaturePipe = nullptr;
    DCMFVendorImp::gReferenceCount = 0;
    DCMFVendorImp::gbAlreadyGet = false;
    DCMFVendorImp::gbFirstSet = true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFVendorImp::
drain(bool bCancel /* = true */)
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

    if(gspFeaturePipe != nullptr){
        gspFeaturePipe->sync();
        gspFeaturePipe->uninit();
        gspFeaturePipe = nullptr;
    }
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
ControllerContainer<NSCam::plugin::DCMFVendorImp::CtrlerType>*
DCMFVendorImp::
getCtrlerContainer(CTRLER u)
{
    switch(u)
    {
        case CTRLER_USED:
            return &mUsedCtrlers;
        case CTRLER_UNUSED:
            return &sUnusedCtrlers;
        default:
            MY_LOGE("get wrong controller container (%d)", u);
            return nullptr;
    }
    return nullptr;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::shared_ptr<NSCam::plugin::DCMFVendorImp::CtrlerType>
DCMFVendorImp::
getCtrler(CTRLER u, DualCamMFType mode)
{
    auto pCC = getCtrlerContainer(u);
    std::shared_ptr<CtrlerType> ctrler;
    // Type of ControllerContainer::iterate's argument
    typedef ControllerContainer<CtrlerType>::ContainerType Data_T;
    pCC->iterate
    (
        [&ctrler, mode]
        (Data_T& data)
        {
            if(data.size() <= 0)
            {
                ctrler = create_dcmf_controller(mode);
                data.push_back(ctrler);
            }
            else
            {
                ctrler = data.front();
            }
        }
    );
    return ctrler;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::shared_ptr<NSCam::plugin::DCMFVendorImp::CtrlerType>
DCMFVendorImp::
getCtrler(
    MUINT32 requestNo,
    CTRLER u
)
{
    return getCtrlerContainer(u)->belong_to(requestNo);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
std::shared_ptr<NSCam::plugin::DCMFVendorImp::CtrlerType>
DCMFVendorImp::
takeCtrler(
    CTRLER u
)
{
    return getCtrlerContainer(u)->take_first();
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFVendorImp::
pushCtrler(
    const std::shared_ptr<CtrlerType>& c,
    CTRLER u
)
{
    getCtrlerContainer(u)->push_back(c);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFVendorImp::
removeCtrler(
    std::shared_ptr<CtrlerType> c,
    CTRLER u
)
{
    return getCtrlerContainer(u)->remove(c);
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
void
DCMFVendorImp::
store_app_setting(IMetadata* pAppMeta, BaseDCCtrler* ctrler)
{
    // {{{
    // get the data and saves to BaseDCCtrler,
    MUINT8 __aemode = 0;
    MUINT8 __awblock = 0;
    MUINT8 __eis = 0;
    MINT32 __iso = 0;
    MINT64 __exp = 0;
    // Back up the original AppSetting if it exists.
    auto b = IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, __aemode);
    if (b) {
        ctrler->setAppSetting(
                BaseDCCtrler::AppSetting::CONTROL_AE_MODE,
                static_cast<int64_t>(__aemode));
    }
    b = IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_AWB_LOCK, __awblock);
    if (b) {
        ctrler->setAppSetting(
                BaseDCCtrler::AppSetting::CONTROL_AWB_LOCK,
                static_cast<int64_t>(__awblock));
    }
    b = IMetadata::getEntry<MUINT8>(pAppMeta, MTK_CONTROL_VIDEO_STABILIZATION_MODE, __eis);
    if (b) {
        ctrler->setAppSetting(
                BaseDCCtrler::AppSetting::CONTROL_VIDEO_STABILIZATION_MODE,
                static_cast<int64_t>(__eis));
    }
    b = IMetadata::getEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, __iso);
    if (b) {
        ctrler->setAppSetting(
                BaseDCCtrler::AppSetting::SENSOR_SENSITIVITY,
                static_cast<int64_t>(__iso));
    }
    b = IMetadata::getEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, __exp);
    if (b) {
        ctrler->setAppSetting(
                BaseDCCtrler::AppSetting::SENSOR_EXPOSURE_TIME,
                static_cast<int64_t>(__exp));
    }
    // }}}
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFVendorImp::
updateAppSettings(
    IMetadata* pAppMeta,
    MINT32 openId,
    DualCamMFType featureMode,
    MINT32 shotIndex,
    MINT32& rEV
)
{
    FUNCTION_SCOPE(openId);

    MINT32 enable = ::property_get_int32("vendor.dg.dcmf.updateAE", 1);
    if(!enable){
        MY_LOGD("do not updateAppSettings");
        return true;
    }
    if(pAppMeta == nullptr){
        MY_LOGE("pAppMeta is nullptr!");
        return false;
    }

    MBOOL isPureZSD = MFALSE;
    switch(featureMode){
        case DualCamMFType::TYPE_DCMF_3rdParty:
            isPureZSD = getIsPureZSD_DCMF();
            break;
        case DualCamMFType::TYPE_DCMF_3rdParty_HDR:
            isPureZSD = MFALSE; // HDR can never be pure zsd
            break;
        default:
            MY_LOGW("unknown feature type:%d", featureMode);
            isPureZSD = MFALSE;
    }
    if(isPureZSD){
        MY_LOGD("no AE manual control in pure-zsd mode");
        return true;
    }

    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(openId, __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );
    if (hal3a.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return false;
    }

    // current setting
    ExpSettingParam_T   expParam;
    CaptureParam_T      capParam;

    // modified setting
    CaptureParam_T      modifiedCap3AParam;

#if 1
    // get the current 3A, just get at fitst time
    hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0);
    hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&capParam, 0);

    modifiedCap3AParam = capParam;

    MY_LOGD("need update 3A current iso:%d, exp:%d", capParam.u4RealISO, capParam.u4Eposuretime);
    if( featureMode == DualCamMFType::TYPE_DCMF_3rdParty_HDR){
        MY_LOGD("HDR Mode");
        // temp tuning value, should refine
        MUINT32 Hdr_input_EV_under = 3;
        MUINT32 Hdr_input_EV_over = 35;

        switch(shotIndex){
            case 0:
                // +0 EV, first frame should be +0 EV because it is the source of postview
                modifiedCap3AParam.u4Eposuretime  = (MUINT32)capParam.u4Eposuretime;
                modifiedCap3AParam.u4AfeGain      = capParam.u4AfeGain;
                modifiedCap3AParam.u4IspGain      = capParam.u4IspGain;
                rEV = 0;
                break;
            case 1:
                // -1 EV
                modifiedCap3AParam.u4Eposuretime  = (MUINT32)capParam.u4Eposuretime*Hdr_input_EV_under/10;
                modifiedCap3AParam.u4AfeGain      = capParam.u4AfeGain;
                modifiedCap3AParam.u4IspGain      = capParam.u4IspGain;
                rEV = -1;
                break;
            default:
                MY_LOGW("not expected to have more than 2 frames in HDR mode, use +0EV");
                modifiedCap3AParam.u4Eposuretime  = (MUINT32)capParam.u4Eposuretime;
                modifiedCap3AParam.u4AfeGain      = capParam.u4AfeGain;
                modifiedCap3AParam.u4IspGain      = capParam.u4IspGain;
                rEV = 0;
        }
    }else{
        MY_LOGW("not support, use current AE settings");
    }
#else
    MY_LOGW("currently not support");
    return true;
#endif
    MINT32 newISO = modifiedCap3AParam.u4RealISO;
    MINT32 newExp = modifiedCap3AParam.u4Eposuretime;
    MY_LOGD("new iso:%d, exp:%d", newISO, newExp);
    // update to appMeta
    IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_OFF);
    IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AWB_LOCK, MTRUE);
    IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_VIDEO_STABILIZATION_MODE, MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON);
    IMetadata::setEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, newISO);
    IMetadata::setEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, newExp * 1000); // ms->us
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFVendorImp::
toggleManualFlare(
    MINT32 openId,
    MBOOL enable
)
{
    FUNCTION_SCOPE(openId);
    std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(openId, __CALLERNAME__),
                [](IHal3A* p){ if (p) p->destroyInstance(__CALLERNAME__); }
            );
    if (hal3a.get() == nullptr) {
        MY_LOGE("create IHal3A instance failed");
        return false;
    }
    MINT32 __enable = (enable) ? 1 : 0;
    hal3a->send3ACtrl(E3ACtrl_EnableFlareInManualCtrl, 1, 0);
    MY_LOGD("de-flare for opendId(%d) => %d", openId, __enable);
    return true;
}
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
bool
DCMFVendorImp::
isJpegSizeChanged(
    MINT32 openId,
    const InputInfo& in
)
{
    FUNCTION_SCOPE(openId);
    // buffer allocated by upper layer have no effect on pipe
    return false;
}