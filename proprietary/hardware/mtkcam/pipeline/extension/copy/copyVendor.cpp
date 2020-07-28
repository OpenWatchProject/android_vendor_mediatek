/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#define LOG_TAG "vmgr_copy"
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include "../MyUtils.h"
#include "copyVendor.h"
#include "../utils/VendorUtils.h"
//
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>

using namespace android;
using namespace NSCam::plugin;

using NSCam::NSIoPipe::NSSImager::IImageTransform;

/******************************************************************************
 *
 ******************************************************************************/
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


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1, "%d mode:%" PRId64 "+", getOpenId(), mMode);
#define FUNCTION_OUT            MY_LOGD_IF(1, "%d mode:%" PRId64 "-", getOpenId(), mMode);

#define COPY_NAME       ("Cam@pluginCopy")
#define COPY_POLICY     (SCHED_OTHER)
#define COPY_PRIORITY   (0)

/******************************************************************************
 *
 ******************************************************************************/
android::sp<CopyVendor>
CopyVendor::
createInstance(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
{
    return new CopyVendorImp(pcszName, i4OpenId, vendorMode);
}

/******************************************************************************
 *
 ******************************************************************************/
CopyVendorImp::
CopyVendorImp(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
    : BaseVendor(i4OpenId, vendorMode, String8::format("CopyVendorImp"))
    , mIsAsync(false)
    , mFlush(false)
    , mPluginExit(false)
    , mPluginDrained(false)
    , mPluginDrainedCond()
{
    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);
    mPluginThread = new PluginThread(this);
    if( mPluginThread->run(COPY_NAME) != OK ) {
        MY_LOGE("tun thread fail");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
CopyVendorImp::
~CopyVendorImp()
{
    MY_LOGD("[%d]vendor %" PRId64 " destroy.", mOpenId, mMode);
}

/******************************************************************************
 *
 ******************************************************************************/

template<typename T>
inline MINT32 UPDATE_ENTRY(IMetadata& metadata, MINT32 entry_tag, T value)
{
    IMetadata::IEntry entry(entry_tag);
    entry.push_back(value, Type2Type< T >());
    return metadata.update(entry_tag, entry);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
get(
    MINT32           /*openId*/,
    const InputInfo& in,
    FrameInfoSet&    out
)
{
    MY_LOGD("+");
    if ( mFlush ) return -ENODEV;
    out.frameCount = 1;
    out.table.vendorMode  = getVendorMode();
    out.table.inCategory  = FORMAT_YUV;
    out.table.outCategory = FORMAT_YUV;
    //
    out.table.bufferType   = BUFFER_NO_WORKING_OUTPUT;
    //
    // create StreamId Map
    // create StreamId Map for RequestFrame
    RequestFrame::StreamIdMap_Img map;
    {
        if (CC_LIKELY(in.fullRaw.get())) {
            map[RequestFrame::eRequestImg_FullSrc] = in.fullRaw->getStreamId();
        }
        else {
            MY_LOGE("no full size RAW");
        }

        if (in.resizedRaw.get()) {
            map[RequestFrame::eRequestImg_ResizedSrc] = in.resizedRaw->getStreamId();
        }
        else {
            MY_LOGD("no rrzo");
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
            map[RequestFrame::eRequestImg_WorkingBufferIn] = in.workingbuf->getStreamId();
        }
        else {
            MY_LOGD("no working buffer info");
        }

        if ( in.vYuv.size() + RequestFrame::eRequestImg_ReservedOut1 >= RequestFrame::eRequestImg_Out_End )
            MY_LOGE("output yuv too much.");
        for(size_t i = 0; i < in.vYuv.size(); ++i) {
            if (CC_LIKELY(in.vYuv[i].get())) {
                map[RequestFrame::eRequestImg_ReservedOut1 + i] = in.vYuv[i]->getStreamId();
            }
        }
    }

    sp<RequestSet> pReq = new RequestSet(getOpenId());
    pReq->setImgMap(map);
    mRequestList.push_back(pReq);

    MY_LOGD("-");
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
set(
    MINT32              /*openId*/,
    const InputSetting& in
)
{
    Mutex::Autolock _l(mLock);
    if ( mFlush ) return -ENODEV;
    mRequestList[mRequestList.size() - 1]->setBelong(in);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
queue(
    MINT32  const              openId,
    MUINT32 const              requestNo,
    wp<IVendor::IDataCallback> cb,
    BufferParam                bufParam,
    MetaParam                  metaParam
)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mLock);
    if ( mFlush ) return -ENODEV;
    //
    sp<IVendor::IDataCallback> spCb = cb.promote();
    if ( !spCb.get() ) return -ENODEV;
    //
    for( size_t i = 0; i < mRequestList.size(); ++i ) {
        if ( mRequestList[i]->belong(requestNo) ) {
            mRequestList[i]->add(openId, requestNo, bufParam, metaParam, spCb);
            if ( RequestSet::STATE_FRAME_READY == mRequestList[i]->getState() ) {
                //MY_LOGW_IF (mRequestList[i]->execute() != OK, " execute fail ");
                Mutex::Autolock _l(mReadyLock);
                mvReadyList.push_back(mRequestList[i]);
                mRequestList.removeAt(i);
                mReadyCond.signal();
            }
            break;
        }
    }
    FUNCTION_OUT;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
beginFlush( MINT32 /*openId*/ )
{
    FUNCTION_IN;
    {
        Mutex::Autolock _l(mLock);
        mFlush = true;
        //
        for( size_t i = 0; i < mRequestList.size(); ++i )
            mRequestList[i]->flush();
        mRequestList.clear();
    }
    //
    {
        Mutex::Autolock _l(mReadyLock);
        for( size_t i = 0; i < mvReadyList.size(); ++i )
            mvReadyList[i]->flush();
        mvReadyList.clear();
        mReadyCond.signal();
    }
    //
    waitPluginDrained();
    //
    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
endFlush( MINT32 /*openId*/ )
{
    FUNCTION_IN;
    Mutex::Autolock _l(mLock);
    mReadyCond.signal();
    if ( mPluginThread.get() ) {
        mPluginThread->requestExit();
        mPluginThread->join();
        mPluginThread = NULL;
    }
    mFlush = false;
    //
    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
sendCommand(
    MINT32      cmd,
    MINT32      /*openId*/,
    MetaItem&   /*meta*/,
    MINT32& arg1, MINT32& /*arg2*/,
    void*       /*arg3*/
)
{
    switch(cmd) {
        case INFO_CANCEL: {
            Mutex::Autolock _l(mLock);
            if ( mFlush ) return -ENODEV;
            for( size_t i = 0; i < mRequestList.size(); ++i ) {
                if ( mRequestList[i]->belong(arg1) ) {
                    MY_LOGD("remove request:%d", arg1);
                    mRequestList[i]->remove(arg1);
                    if ( mRequestList[i]->empty() ) mRequestList.removeAt(i);
                    break;
                }
            }
        } break;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::
dump( MINT32 /*openId*/ )
{
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CopyVendorImp::
waitPluginDrained()
{
    FUNCTION_IN;
    //
    Mutex::Autolock _l(mReadyLock);
    if( !mPluginDrained ) {
        MY_LOGD("wait for request drained");
        mPluginDrainedCond.wait(mReadyLock);
    }
    //
    FUNCTION_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CopyVendorImp::
onDequeRequest(sp<RequestSet>& pRequest)
{
    Mutex::Autolock _l(mReadyLock);
    while ( mvReadyList.empty() && !mPluginExit ) {
        mPluginDrained = MTRUE;
        mPluginDrainedCond.signal();
        //
        status_t status = mReadyCond.wait(mReadyLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, list size:%zu, exitPlugin:%d",
                status, ::strerror(-status), mvReadyList.size(), mPluginExit
            );
        }
    }
    //
    if ( mPluginExit ) {
        MY_LOGW_IF(mvReadyList.size(), "[flush] mvReadyList.size:%zu", mvReadyList.size());
        return DEAD_OBJECT;
    }
    //
    mPluginDrained = MFALSE;
    pRequest = *mvReadyList.begin();
    mvReadyList.erase(mvReadyList.begin());
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CopyVendorImp::RequestSet::
remove( MUINT32 requestNo )
{
    for( size_t i = 0; i < mInput.vFrame.size(); ++i) {
        if ( static_cast<int>(requestNo) == mInput.vFrame[i].frameNo ) {
            mInput.vFrame.removeAt(i);
            break;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::RequestSet::
add(
    const MINT32               openId,
    MUINT32                    requestNo,
    BufferParam                bufParam,
    MetaParam                  metaParam,
    sp<IVendor::IDataCallback> cb
)
{
    std::shared_ptr<RequestFrame> pFrame;
    {
        pFrame = std::shared_ptr<RequestFrame>
            (
                new RequestFrame
                (
                    openId,
                    LOG_TAG,
                    bufParam,
                    metaParam,
                    cb,
                    m_streamIdMap_img,
                    m_streamIdMap_meta,
                    requestNo
                )
            );
    }

    mRequestList.push_back(pFrame);
    if ( mRequestList.size() == mInput.vFrame.size() )
        mState = RequestSet::STATE_FRAME_READY;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CopyVendorImp::RequestSet::
belong(MUINT32 requestNo)
{
    for (size_t i = 0; i < mInput.vFrame.size(); ++i) {
        if ( mInput.vFrame[i].frameNo == static_cast<int>(requestNo) )
            return true;
    }
    return false;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
CopyVendorImp::RequestSet::
getState()
{
    return mState;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::RequestSet::
execute()
{
    CAM_TRACE_NAME("execute plugin");
    mState = STATE_RUNNING;
    //
    MERROR err = onExecute();
    //
    flush();

    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::RequestSet::
flush()
{
    mRequestList.clear();
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
CopyVendorImp::RequestSet::
~RequestSet()
{
    mRequestList.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
void
CopyVendorImp::PluginThread::
requestExit()
{
    Mutex::Autolock _l(mpImp->mReadyLock);
    mpImp->mPluginExit = true;
    mpImp->mReadyCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::PluginThread::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)COPY_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, COPY_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, COPY_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CopyVendorImp::PluginThread::
threadLoop()
{
    sp<RequestSet> pRequest;
    while( ! exitPending()
        && OK == mpImp->onDequeRequest(pRequest)
        && pRequest.get()
        )
    {
        pRequest->execute();
        return true;
    }
    MY_LOGD("exit PluginThread");
    return  false;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CopyVendorImp::RequestSet::
onExecute()
{
    MY_LOGD("+");
#if 1
    if ( mRequestList.empty() ) {
        MY_LOGD("no frame");
        return OK;
    }
    std::shared_ptr<RequestFrame> pMainFrame = mRequestList[0];

    if (pMainFrame.get() == nullptr) {
        MY_LOGE("main request frame is NULL");
        return -ENODEV;
    }
    auto appMetaIn     = pMainFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    auto halMetaIn     = pMainFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
    auto pFrameYuvJpeg = pMainFrame->getImageBuffer(RequestFrame::eRequestImg_FullOut);
    auto pFrameYuvThub = pMainFrame->getImageBuffer(RequestFrame::eRequestImg_ThumbnailOut);
    auto outOrientaion = pMainFrame->getOrientation(RequestFrame::eRequestImg_FullOut);

    auto pResult       = pMainFrame->getImageBuffer(RequestFrame::eRequestImg_WorkingBufferIn);

    if (appMetaIn == nullptr ) {
        MY_LOGW("input app metadata is NULL");
    }

    if (halMetaIn == nullptr ) {
        MY_LOGW("input hal metadata is NULL");
    }

    if (pFrameYuvJpeg == nullptr) {
        MY_LOGE("output YUV is NULL");
        return -ENODEV;
    }

    if (pFrameYuvThub == nullptr) {
        MY_LOGW("output thumbnail is NULL");
    }

    if (pResult == nullptr) {
        MY_LOGE("input YUV is NULL");
        return -ENODEV;
    }

    // get input/output size, and crop size
    MSize dst1Size =
        (
         outOrientaion == eTransform_ROT_90
         ||
         outOrientaion == eTransform_ROT_270
        )
        ? MSize(pFrameYuvJpeg->getImgSize().h, pFrameYuvJpeg->getImgSize().w)
        : pFrameYuvJpeg->getImgSize();

    MRect rectDst1(MPoint(0, 0), dst1Size);
    MRect rectDst2;
    MRect rectSrc = calCropRegin(appMetaIn, halMetaIn, pResult->getImgSize(), mOpenId);

    if (pFrameYuvThub) {
        rectDst2.p = MPoint(0, 0);
        rectDst2.s = pFrameYuvThub->getImgSize();
    }

    // create JPEG YUV and thumbnail YUV using MDP (IImageTransform)
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
            IImageTransform::createInstance(), // constructor
            [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
            );

    if (transform.get() == nullptr) {
        MY_LOGE("IImageTransform is NULL, cannot generate output");
        return -ENODEV;
    }

    {
        MY_LOGD("apply clear zoom");
        IImageTransform::PQParam config;
        // apply PQ: ClearZoom
        config.type = IImageTransform::PQType::ClearZoom;
        config.enable = MTRUE;
        config.portIdx = 0; // DST_BUF_0
        // query info for CZConfig
        MINT32 czTimeStamp = 0;
        MINT32 czFrameCount = 0;
        MINT32 czIso = 0;
        if (CC_LIKELY(halMetaIn != nullptr)) {
            IMetadata exifMeta;
            if (!IMetadata::getEntry<IMetadata>(const_cast<IMetadata*>(halMetaIn), MTK_3A_EXIF_METADATA, exifMeta)) {
                MY_LOGW("no MTK_3A_EXIF_METADATA from HalMetaData");
            }
            else {
                if (!IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, czIso)) {
                    MY_LOGW("no MTK_3A_EXIF_AE_ISO_SPEED from HalMetaData");
                }
            }
            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(halMetaIn), MTK_PIPELINE_UNIQUE_KEY, czTimeStamp))
            {
                MY_LOGW("no MTK_PIPELINE_UNIQUE_KEY from HalMetaData");
            }
            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(halMetaIn), MTK_PLUGIN_PROCESSED_FRAME_COUNT, czFrameCount))
            {
                MY_LOGW("no MTK_CLEARZOOM_FRAME_COUNT from HalMetaData");
            }

        }
        // set CZConfig
        config.sensorId = mOpenId;
        config.iso = czIso;
        config.timestamp = czTimeStamp;
        config.frameNo   = pMainFrame->getRequestNo();
        config.requestNo = pMainFrame->getRequestNo();
        if (czFrameCount > 1) {
            config.mode = IImageTransform::Mode::Capture_Multi;
        }
        else {
            config.mode = IImageTransform::Mode::Capture_Single;
        }
        MY_LOGD("{CZ sensorId:%d mode:%d(frameCount:%d) ISO:%d Timestamp:%d FrameNo:%d RequestNo:%d}",
                            config.sensorId,
                            config.mode, czFrameCount,
                            config.iso,
                            config.timestamp,
                            config.frameNo,
                            config.requestNo);
        transform->setPQParameter(config);
    }

    MBOOL ret = MTRUE;
    if (pFrameYuvThub)
        ret = transform->execute(
                pResult,
                pFrameYuvJpeg,
                pFrameYuvThub,
                calCrop(rectSrc, rectDst1),
                calCrop(rectSrc, rectDst2),
                outOrientaion,
                0,
                3000);
    else
        ret = transform->execute(
                pResult,
                pFrameYuvJpeg,
                nullptr,
                calCrop(rectSrc, rectDst1),
                outOrientaion,
                3000);

    if (ret != MTRUE) {
        MY_LOGE("execute IImageTransform::execute returns fail");
        return -ENODEV;
    }
#endif
    MY_LOGD("-");
    return OK;
}

