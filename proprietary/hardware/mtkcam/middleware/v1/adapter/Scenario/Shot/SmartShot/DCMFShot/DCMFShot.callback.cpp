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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "MtkCam/DCMFShot"

#include <sstream>
#include <dirent.h>
#include <cassert>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>

#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#include <camera_custom_stereo.h>

#include "ImpShot.h"
#include "DCMFShot.h"

using namespace NSShot::NSDCMFShot;
using namespace android::NSPostProc;

#define THREAD_NAME       ("Cam@DCMFShot")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
//
#define MYU_LOGE(fmt, arg...)       CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

#define FUNCTION_SCOPE_D            ScopeLogger __scope_logger(::gettid(), getOpenId(), true,  mDebugLog, getShotName(), __FUNCTION__)
#define FUNCTION_SCOPE_I            ScopeLogger __scope_logger(::gettid(), getOpenId(), false, mDebugLog, getShotName(), __FUNCTION__)

/******************************************************************************
 * CaptureCBParcel
 ******************************************************************************/
android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>>
CaptureCBParcel::
getPPInputData_Image() const
{
    android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>> ppPackage;
    for (auto it=mvDstStreamsCollect.begin(); it!=mvDstStreamsCollect.end(); ++it){
        ppPackage.add(it->first, it->second);
    }
    return ppPackage;
}

/******************************************************************************
 *
 ******************************************************************************/
const char*
CaptureCBParcel::
getDumpStr()
{
    String8 ret = String8::format("reqNo:%d \n", mReqNo);
    ret = ret + "Buffer:\n";
    for (auto it=mvDstStreamsCollect.begin(); it!=mvDstStreamsCollect.end(); ++it){
        ret = ret + String8::format("[%" PRId64 "] => [%p] \n", it->first, it->second.get());
    }
    ret = ret + "Meta:\n";
    for (auto it=mvMetadataCollect.begin(); it!=mvMetadataCollect.end(); ++it){
        ret = ret + String8::format("[%" PRId64 "] => [%p][%d] \n", it->first, &(it->second), it->second.count());
    }
    return ret.string();
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CaptureCBParcel::
isFinished() const
{
    for (auto it=mvDstStreamsCollect.begin(); it!=mvDstStreamsCollect.end(); ++it){
        if(it->second == nullptr){
            // CAM_LOGD("buffer [%" PRId64 "] not finished", it->first);
            return MFALSE;
        }
    }
    for (auto it=mvMetadataCollect.begin(); it!=mvMetadataCollect.end(); ++it){
        if(it->second.isEmpty()){
            // CAM_LOGD("meta [%" PRId64 "] not finished", it->first);
            return MFALSE;
        }
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
IMetadata
CaptureCBParcel::
getPPInputData_AppMeta() const
{
    for (auto it=mvMetadataCollect.begin(); it!=mvMetadataCollect.end(); ++it){
        if(it->first == NSCam::eSTREAMID_META_APP_CONTROL){
            return it->second;
        }
    }

    CAM_LOGE("Can't find App metadata!");
    return IMetadata();
}

/******************************************************************************
 *
 ******************************************************************************/
IMetadata
CaptureCBParcel::
getPPInputData_HalMeta() const
{
    for (auto it=mvMetadataCollect.begin(); it!=mvMetadataCollect.end(); ++it){
        if(it->first == NSCam::eSTREAMID_META_HAL_DYNAMIC_P2){
            return it->second;
        }
    }

    CAM_LOGE("Can't find Hal metadata!");
    return IMetadata();
}

/******************************************************************************
 * DCMFShot::CaptureCBHandler
 ******************************************************************************/
DCMFShot::CaptureCBHandler::
CaptureCBHandler
(
    MUINT32 const mainReqNo,
    MINT64 const timestamp,
    ShotParam const &shotParam,
    JpegParam const &jpegParam,
    MUINT32 const openIdMain1,
    MUINT32 const openIdMain2
)
: mMain1OpenId(openIdMain1)
, mMain2OpenId(openIdMain2)
, mMainReqNo(mainReqNo)
, mTimestamp(timestamp)
, mShotParam(shotParam)
, mJpegParam(jpegParam)
{
    MY_LOGD("ctor mainReqNo(%d) timestamp(%" PRId64 ") openId(%d/%d)",
        mMainReqNo, timestamp, mMain1OpenId, mMain2OpenId
    );
}

/******************************************************************************
 *
 ******************************************************************************/
DCMFShot::CaptureCBHandler::
~CaptureCBHandler
(
)
{
   MY_LOGD("dtor mainReqNo(%d)", mMainReqNo);
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
DCMFShot::CaptureCBHandler::
getShotName() const
{
    return LOG_TAG;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
registerCapReq
(
    MUINT32 const openId,
    CaptureRequest& rCapReq,
    Vector<PipelineImageParam>& vDstStreams,
    Vector<StreamId>& vDstMetas
)
{
    MY_LOGD("openId(%d) reqNo(%d) +", openId, rCapReq.mCapReqNo);
    Mutex::Autolock _l(mLock);

    if(openId != mMain1OpenId &&
       openId != mMain2OpenId){
        MY_LOGE("opendId(%d) not belong to stereo camera!", openId);
        return MFALSE;
    }

    if(mCapReqCollect.count(openId) == 0){
        mCapReqCollect[openId] = std::vector<CaptureCBParcel>();

        if(mCapReqCollect.size() > 2){
            MY_LOGE("Maximum number of opened sensors should be 2!");
            return MFALSE;
        }
    }

    CaptureCBParcel newCapCBParcel;

    newCapCBParcel.mReqNo = rCapReq.mCapReqNo;

    newCapCBParcel.mvDstStreamsCollect.clear();
    for(auto &e : vDstStreams){
        // MY_LOGD("buffer insert [%" PRId64 "] => ptr[%p]", e.pInfo->getStreamId(), nullptr);
        newCapCBParcel.mvDstStreamsCollect.insert(pair<StreamId, sp<IImageBuffer> >(e.pInfo->getStreamId(), nullptr));
    }

    newCapCBParcel.mvMetadataCollect.clear();
    for(auto &e : vDstMetas){
        // MY_LOGD("meta insert [%" PRId64 "] => empty[%d]", e, IMetadata().isEmpty());
        newCapCBParcel.mvMetadataCollect.insert(pair<StreamId, IMetadata >(e, IMetadata()));
    }

    mCapReqCollect[openId].push_back(newCapCBParcel);

    rCapReq.mPPReqNo = mMainReqNo;

    mvAllBelongReq[openId].push_back(rCapReq.mCapReqNo);

    static int debugLog = ::property_get_int32(DEBUG_LOG, 0);
    MY_LOGD_IF(debugLog, "%s", newCapCBParcel.getDumpStr());

    MY_LOGD("openId(%d) reqNo(%d) -", openId, rCapReq.mCapReqNo);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
isBelong
(
    MUINT32 const openId,
    MUINT32 const reqNo
)
{
    Mutex::Autolock _l(mLock);

    MBOOL found = MFALSE;
    if(mCapReqCollect.count(openId) == 0){
        MY_LOGE("openId(%d) not registered!", openId);
        return MFALSE;
    }

    std::vector<CaptureCBParcel>& cbList = mCapReqCollect[openId];

    for(auto &e : cbList){
        if(e.mReqNo == reqNo){
            found = MTRUE;
        }
    }
    return found;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
isFinalPP() const
{
    MINT32 MASTER = mMain1OpenId;
    MINT32 SLAVE  = mMain2OpenId;

    const std::vector<CaptureCBParcel>& parcelCollet_Master = mCapReqCollect.at(MASTER);
    const std::vector<CaptureCBParcel>& parcelCollet_Slave = mCapReqCollect.at(SLAVE);

    MBOOL ret = parcelCollet_Master.empty() && parcelCollet_Slave.empty();

    MY_LOGD("(M:%zu,S:%zu) ret(%d)",
        parcelCollet_Master.size(), parcelCollet_Slave.size(), ret);

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
isAllPPSubmitted() const
{
    Mutex::Autolock _l(mLock);

    MINT32 MASTER = mMain1OpenId;
    MINT32 SLAVE  = mMain2OpenId;

    const std::vector<CaptureCBParcel>& parcelCollet_Master = mCapReqCollect.at(MASTER);
    const std::vector<CaptureCBParcel>& parcelCollet_Slave = mCapReqCollect.at(SLAVE);

    MBOOL ret = parcelCollet_Master.empty() && parcelCollet_Slave.empty() && mvPendingPP.empty();

    MY_LOGD("(M:%zu,S:%zu,PP:%zu) ret(%d)",
        parcelCollet_Master.size(), parcelCollet_Slave.size(), mvPendingPP.size(), ret);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
onBufFinished(
    MUINT32 const openId,
    MUINT32 const reqNo,
    StreamId const id,
    android::sp<IImageBuffer>& pBuffer
)
{
    {
        Mutex::Autolock _l(mLock);
        if(mCapReqCollect.count(openId) == 0){
            MY_LOGE("openId(%d) not registered!", openId);
            return MFALSE;
        }

        std::vector<CaptureCBParcel>& cbList = mCapReqCollect[openId];

        MBOOL found = MFALSE;
        for(auto &e : cbList){
            if(e.mReqNo == reqNo){
                e.mvDstStreamsCollect[id] = pBuffer;
                found = MTRUE;
            }
        }

        if(!found){
            MY_LOGE("openId(%d) reqNo(%d) steam[%" PRId64 "] not registered!",
                openId, reqNo, id
            );
            return MFALSE;
        }
    }

    MY_LOGD("openId(%d) reqNo(%d) steam[%" PRId64 "] buf(%p) callback collected",
        openId, reqNo, id, pBuffer.get()
    );
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
onMetaFinished(
    MUINT32 const openId,
    MUINT32 const reqNo,
    StreamId const id,
    IMetadata const meta
)
{
    {
        Mutex::Autolock _l(mLock);
        if(mCapReqCollect.count(openId) == 0){
            MY_LOGE("openId(%d) not registered!", openId);
            return MFALSE;
        }

        std::vector<CaptureCBParcel>& cbList = mCapReqCollect[openId];

        MBOOL found = MFALSE;
        for(auto &e : cbList){
            if(e.mReqNo == reqNo){
                e.mvMetadataCollect[id] = meta;
                found = MTRUE;
            }
        }

        if(!found){
            MY_LOGE("openId(%d) reqNo(%d) steam[%" PRId64 "] not registered!",
                openId, reqNo, id
            );
            return MFALSE;
        }
    }

    MY_LOGD("openId(%d) reqNo(%d) steam[%" PRId64 "] meta(%p) callback collected",
        openId, reqNo, id, &meta
    );
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
preparePostProcess()
{
    Mutex::Autolock _l(mLock);

    if(mCapReqCollect.size() != 2){
        MY_LOGE("capture collect size != 2, it should be 2 (master & slave)!");
        return MFALSE;
    }

    MINT32 MASTER = mMain1OpenId;
    MINT32 SLAVE  = mMain2OpenId;

    std::vector<CaptureCBParcel>& parcelCollet_Master = mCapReqCollect[MASTER];
    std::vector<CaptureCBParcel>& parcelCollet_Slave = mCapReqCollect[SLAVE];

    while( !parcelCollet_Master.empty() && !parcelCollet_Slave.empty() ){
        auto parcel_M = parcelCollet_Master.front();
        auto parcel_S = parcelCollet_Slave.front();

        if(parcel_M.isFinished() && parcel_S.isFinished()){
            MY_LOGD("reqNo(%d) is finished", parcel_M.mReqNo);
            // both sensor are finished, create a PPData package
            sp<PPData> ppData = new PPData();
            ppData->mProcessType = PostProcessorType::THIRDPARTY_DCMF;
            ppData->mRequestNo = parcel_M.mReqNo;
            ppData->miTimeStamp = mTimestamp;

            android::NSPostProc::InputData ppInpuData;
            // buffer
            ppInpuData.mImageQueue.push_back(parcel_M.getPPInputData_Image());
            ppInpuData.mImageQueue.push_back(parcel_S.getPPInputData_Image());
            // app meta
            ppInpuData.mAppMetadataQueue.push_back(parcel_M.getPPInputData_AppMeta());
            ppInpuData.mAppMetadataQueue.push_back(parcel_S.getPPInputData_AppMeta());
            // hal meta
            ppInpuData.mHalMetadataQueue.push_back(parcel_M.getPPInputData_HalMeta());
            ppInpuData.mHalMetadataQueue.push_back(parcel_S.getPPInputData_HalMeta());
            //
            ppData->mvInputData.push_back(ppInpuData);

            // transfer to PostProc defined streamIds
            if( !reMapImageStreams(ppData) ){
                MY_LOGE("failed remapping stream id!");
                return MFALSE;
            }

            ppData->mShotParam = mShotParam;
            ppData->mJpegParam = mJpegParam;

            if( mvIPostProcCB.count(MASTER) > 0){
                ppData->mpCb = mvIPostProcCB.at(MASTER);
            }else{
               MY_LOGW("no IPostProcCB!");
            }

            parcelCollet_Master.erase(parcelCollet_Master.begin());
            parcelCollet_Slave.erase(parcelCollet_Slave.begin());

            if( !isFinalPP() ){
                ppData->mIsFinalData = false;
            }

            mvPendingPP.push_back(ppData);
            mvPendingPPReq.push_back(ppData->mRequestNo);

            MY_LOGD("deffered PostProc requests (%zu)", mvPendingPP.size());
        }else{
            // someone is not finished, stop checking
            // if( !parcel_M.isFinished() ){
            //     MY_LOGD("main1 not finished yet: %s", parcel_M.getDumpStr());
            // }
            // if( !parcel_S.isFinished() ){
            //     MY_LOGD("main2 not finished yet: %s", parcel_M.getDumpStr());
            // }
            break;
        }
    }

    if( !mvPendingPP.empty() ){
        // there are some deffered PP request pedning
        return MTRUE;
    }else{
        // nothing to do
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
triggerPostProcess()
{
    Mutex::Autolock _l(mLock);

    if(mvPendingPP.empty()){
        return MTRUE;
    }

    IImagePostProcessManager* pIIPPManager = IImagePostProcessManager::getInstance();

    if(pIIPPManager == nullptr){
        MY_LOGE("Cannot get IImagePostProcessManager instance!");
        return MFALSE;
    }

    if(mbSupportBGService){
        // supports BG service, which means the jpeg calback is via mmsdk, no need to provide aosp callback to BG
        pIIPPManager->setShotCallback(nullptr);
    }else{
        // does not support BG service, which means the jpeg calback is via AOSP, need to provide aosp callback to BG
        pIIPPManager->setShotCallback(mpIShotCallback);
    }

    for(auto &e : mvPendingPP){
        MY_LOGI("before enque PPReq(%d) supportBG(%d)", e->mRequestNo, mbSupportBGService);
        static int debugLog = ::property_get_int32(DEBUG_LOG, 0);
        if( debugLog ){
            e->dump();
        }
        pIIPPManager->enque(e);
        MY_LOGI("after enque PPReq(%d)", e->mRequestNo);
    }

    mvPendingPP.clear();
    mvPendingPPReq.clear();

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DCMFShot::CaptureCBHandler::
reMapImageStreams(
    sp<PPData> rPPData
)
{
    // mapping function, mapping from Camera phase to PostProc phase
    auto mappedId = [&](MINT32 oiginId){
        switch(oiginId){
            // fullsize yuv from IMGO
            case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                return (MINT32)eSTREAMID_IMAGE_PIPE_YUV_00;
            // resize yuv from RRZO
            case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                return (MINT32)eSTREAMID_IMAGE_PIPE_YUV_01;
            // small size yuv from IMGO
            case eSTREAMID_IMAGE_PIPE_YUV_01:
                return (MINT32)eSTREAMID_IMAGE_PIPE_YUV_02;
            // postview
            case eSTREAMID_IMAGE_PIPE_YUV_00:
                return (MINT32)eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL;

            default:
                return oiginId;
        }
    };

    for( auto &e : rPPData->mvInputData){
        for(size_t i=0 ; i<e.mImageQueue.size() ; i++){
            auto& theQue = e.mImageQueue[i];
            android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>> remappedStreams;

            String8 str = String8::format("[stream]->[remapped stream]: ");
            for(size_t j=0 ; j<theQue.size() ; j++){
                str = str + String8::format("[%x]->[%x],", theQue.keyAt(j), mappedId(theQue.keyAt(j)));
                remappedStreams.add(
                    mappedId(theQue.keyAt(j)),
                    theQue.valueAt(j)
                );
            }
            MY_LOGD("%s", str.c_str());

            e.mImageQueue[i] = remappedStreams;
        }
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFShot::
updateHalMeta(IMetadata* const halMeta, MUINT32 const requestNo) const
{
    // open id
    {
        IMetadata::IEntry entry(MTK_STEREO_FEATURE_OPEN_ID);
        entry.push_back(getOpenId(), Type2Type<MINT32>());
        halMeta->update(entry.tag(), entry);
    }

    // stereo profile
    {
        IMetadata::IEntry entry(MTK_STEREO_FEATURE_SENSOR_PROFILE);
        entry.push_back(StereoSettingProvider::stereoProfile(), Type2Type< MINT32 >());
        halMeta->update(entry.tag(), entry);
    }

    // force using hw to encode all jpeg
    {
        IMetadata::IEntry entry(MTK_JPG_ENCODE_TYPE);
        entry.push_back(NSCam::NSIoPipe::NSSImager::JPEGENC_HW_ONLY, Type2Type< MUINT8 >());
        halMeta->update(entry.tag(), entry);
    }

#if 1 //debug
    {
        IMetadata::IEntry entry = halMeta->entryFor(MTK_STEREO_FEATURE_FOV_CROP_REGION);
        String8 str = String8::format("FOV crop info:");
        if( !entry.isEmpty() && entry.count() >= 6 ){
            for(int i=0 ; i<6 ; i++){
                str = str + String8::format("%d,", entry.itemAt(i, Type2Type<MINT32>()));
            }
        }
        MY_LOGD("%s", str.c_str());
    }
#endif

    MBOOL   found = MFALSE;
    MINT64  timestamp = 0;
    MBOOL   foregroundPostview = MFALSE;

    for (auto it=gvCapCBHandlers.begin(); it!=gvCapCBHandlers.end(); ++it){
        if(it->second->isBelong(getOpenId(), requestNo)){
            found = MTRUE;
            timestamp = it->second->getTimestamp();
            foregroundPostview = it->second->getForegroundPostviewSupport();
            break;
        }
    }

    if( !found ){
        MY_LOGE("requestNo %d, timestamp not registered!", requestNo);
    }

    MBOOL needBGPostview = !foregroundPostview;

    if( needBGPostview && isMaster() ){
        // no foreground postview, need BG pipeline to produce postview
        // the callback client is stored in hal meta so any module in BG pipeline can trigger the postview callback on it's own
        MINT64 pCallbackClient = reinterpret_cast<MINT64>(mpShotCallback.get());
        {
            IMetadata::IEntry entry(MTK_STEREO_CALLBACK_CLIENT);
            entry.push_back(pCallbackClient, Type2Type< MINT64 >());
            halMeta->update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_DUALCAM_TIMESTAMP);
            entry.push_back(timestamp, Type2Type< MINT64 >());
            halMeta->update(entry.tag(), entry);
        }

        MSize postviewSize = StereoSizeProvider::getInstance()->postViewSize();
        {
            IMetadata::IEntry entry(MTK_STEREO_DCMF_POSTVIEW_SIZE);
            entry.push_back(postviewSize, Type2Type< MSize >());
            halMeta->update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_STEREO_BG_SERVICE_SUPPORT);
            entry.push_back(
                (mbBGService)? 1 : 0,
                Type2Type< MINT32 >()
            );
            halMeta->update(entry.tag(), entry);
        }

        {
            IMetadata::IEntry entry(MTK_STEREO_NEED_BG_POSTVIEW);
            entry.push_back(1, Type2Type< MINT32 >());
            halMeta->update(entry.tag(), entry);
        }

        MY_LOGD("callbackClient(%" PRId64 ")(%p) BGSupport(%d) needBGPostview(%d) timestamp(%" PRId64 ") postviewSize(%dx%d)",
            pCallbackClient,
            mpShotCallback.get(),
            mbBGService,
            needBGPostview,
            timestamp,
            postviewSize.w, postviewSize.h
        );
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DCMFShot::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    CAM_TRACE_FMT_BEGIN("onMetaReceived No%d,StreamID [%" PRId64 "]", requestNo,streamId);
    MY_LOGD("requestNo %d, stream [%" PRId64 "], errResult:%d", requestNo, streamId, errorResult);
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        int idx = mResultMetadataSetMap.indexOfKey(requestNo);
        if(idx < 0 )
        {
            MY_LOGE("mResultMetadataSetMap can't find requestNo(%d)",requestNo);
            for ( size_t i=0; i<mResultMetadataSetMap.size(); i++) {
                MY_LOGD( "mResultMetadataSetMap(%zu/%zu)  requestNo(%d) buf(%p)",
                         i, mResultMetadataSetMap.size(), mResultMetadataSetMap[i].requestNo,
                         mResultMetadataSetMap[i].selectorGetBufs[0].heap.get() );
            }
            return;
        }
    }
    //
    MINT32 callbackState = NONE_CB_DONE;
    switch (streamId)
    {
        case eSTREAMID_META_APP_FULL:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata = result;
                mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata += mResultMetadataSetMap.editValueFor(requestNo).selectorAppMetadata;
                mResultMetadataSetMap.editValueFor(requestNo).callbackState |= FULL_APP_META_CB_DONE;
                callbackState = mResultMetadataSetMap.editValueFor(requestNo).callbackState;
            }

            if( (callbackState & FULL_ALL_META_CB_DONE) == FULL_ALL_META_CB_DONE )
            {
                if(isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    handleDNGMetaCB(requestNo);//dng metadata callback
                }
            }
            //
            if( (mResultMetadataSetMap.editValueFor(requestNo).callbackState & DNG_CB_DONE) == DNG_CB_DONE )
            {
                //clear result metadata
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.removeItem(requestNo);
                MY_LOGD("clear mResultMetadataSetMap(%d)",requestNo);
            }
            break;
        case eSTREAMID_META_HAL_FULL:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).halResultMetadata = result;
                mResultMetadataSetMap.editValueFor(requestNo).callbackState |= FULL_HAL_META_CB_DONE;
                callbackState = mResultMetadataSetMap.editValueFor(requestNo).callbackState;
            }
            //
            if( (callbackState & FULL_ALL_META_CB_DONE) == FULL_ALL_META_CB_DONE )
            {
                if(isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    handleDNGMetaCB(requestNo);//dng metadata callback
                }
            }
            //
            if( (callbackState & DNG_CB_DONE) == DNG_CB_DONE )
            {
                //clear result metadata
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.removeItem(requestNo);
                MY_LOGD("clear mResultMetadataSetMap(%d)",requestNo);
            }
            break;
        case eSTREAMID_META_HAL_DYNAMIC_P2:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).halResultMetadata = result;
            }
            break;
        case eSTREAMID_META_APP_DYNAMIC_P2:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata += result;
            }
            break;
        default:
            MY_LOGD("unsupported steamId[%" PRId64 "]", streamId);
    }

    // update metadata to gvCapCBHandlers
    switch(streamId){
        case eSTREAMID_META_HAL_DYNAMIC_P2:
            {
                Mutex::Autolock _l(gCapCBHandlerLock);
                IMetadata newMeta = result;
                updateHalMeta(&newMeta, requestNo);

                auto pCBHandler = getBelongCBHandlerLocked(requestNo);

                if( pCBHandler != nullptr ){
                    pCBHandler->onMetaFinished(getOpenId(), requestNo, streamId, newMeta);
                }else{
                    MY_LOGE("requestNo %d, stream[%" PRId64 "], not registered!", requestNo, streamId);
                }
            }
            break;
        default:
            {
                // do nothing for other streams
            }
    }

    prepareAndTriggerPostProc(requestNo);

    CAM_TRACE_FMT_END();
}

/*******************************************************************************
*
********************************************************************************/
MVOID
DCMFShot::
onDataReceived(
    MUINT32 const               requestNo,
    StreamId_T const            streamId,
    MBOOL   const               errorBuffer,
    android::sp<IImageBuffer>&  pBuffer
)
{
    CAM_TRACE_FMT_BEGIN("requestNo %d, steamId[%" PRId64 "]", requestNo, streamId);
    MY_LOGD("requestNo %d, steamId[%" PRId64 "], buffer %p, error %d +", requestNo, streamId, pBuffer.get(), errorBuffer);
    //
    if( pBuffer != nullptr )
    {
        MINT32 data = NSCamShot::ECamShot_DATA_MSG_NONE;
        //
        switch (streamId)
        {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                data = NSCamShot::ECamShot_DATA_MSG_RAW;
                if(isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    handleRaw16CB(pBuffer.get());
                    pBuffer->unlockBuf(LOG_TAG);
                }
                //
                if ( mbZsdFlow ) {
                    sp<StreamBufferProvider> pProvider = IResourceContainer::getInstance(getOpenId())->queryConsumer( streamId );
                    MY_LOGD("Query Consumer OpenID(%d) StreamID[%" PRId64 "]", getOpenId(), streamId);
                    if( pProvider == NULL) {
                        MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
                    }
                    else {
                        Vector<ISelector::BufferItemSet> bufferSet;
                        {
                            Mutex::Autolock _l(mResultMetadataSetLock);
                            bufferSet = mResultMetadataSetMap.editValueFor(requestNo).selectorGetBufs;
                        };

                        sp< ISelector > pSelector = pProvider->querySelector();
                        if(pSelector == NULL) {
                            MY_LOGE("can't find Selector in Consumer when p2done");
                        } else {
                            for(size_t i = 0; i < bufferSet.size() ; i++)
                                pSelector->returnBuffer(bufferSet.editItemAt(i));
                        }
                    }
                    //
                    {
                        Mutex::Autolock _l(mResultMetadataSetLock);
                        for (auto& itr : mResultMetadataSetMap.editValueFor(requestNo).selectorGetBufs) {
                            itr = ISelector::BufferItemSet();
                        }
                    }
                }
                //
                {
                    Mutex::Autolock _l(mResultMetadataSetLock);
                    mResultMetadataSetMap.editValueFor(requestNo).callbackState |= RAW_CB_DONE;
                    if( (mResultMetadataSetMap.editValueFor(requestNo).callbackState & DNG_CB_DONE) == DNG_CB_DONE )
                    {
                        //clear result metadata
                        mResultMetadataSetMap.removeItem(requestNo);
                        MY_LOGD("clear mResultMetadataSetMap(%d)",requestNo);
                    }
                }
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
            case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
            case eSTREAMID_IMAGE_PIPE_YUV_01:
                data = NSCamShot::ECamShot_DATA_MSG_YUV;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_00:{
                    Mutex::Autolock _l(gCapCBHandlerLock);
                    data = NSCamShot::ECamShot_DATA_MSG_POSTVIEW;
                    handlePostView(pBuffer.get(), requestNo);
                    handleCaptureDoneCallback();
                }
                break;
            case eSTREAMID_IMAGE_JPEG:
                data = NSCamShot::ECamShot_DATA_MSG_JPEG;
                handleJpegData(requestNo, pBuffer.get());
                break;
            default:
                data = NSCamShot::ECamShot_DATA_MSG_NONE;
                break;
        }
        // dump flag:
        // ECamShot_DATA_MSG_NONE           = 0x0000,           /*!< none data  callback message */
        // ECamShot_DATA_MSG_RAW            = 0x0001,           /*!< raw data callback message */
        // ECamShot_DATA_MSG_YUV            = 0x0002,           /*!< yuv data callback message */
        // ECamShot_DATA_MSG_POSTVIEW       = 0x0004,           /*!< postview data callback message */
        // ECamShot_DATA_MSG_JPEG           = 0x0008,           /*!< jpeg data callback message */
        if( mDumpFlag & data )
        {
            String8 filename = String8::format("%s/DCMFShot_req[%d]_open[%d]_%" PRId64 "_%dx%d",
                    DUMP_PATH, requestNo, getOpenId(), streamId,
                    pBuffer->getImgSize().w, pBuffer->getImgSize().h);
            switch( data )
            {
                case NSCamShot::ECamShot_DATA_MSG_RAW:
                    filename += String8::format("_%zd.raw", pBuffer->getBufStridesInBytes(0));
                    break;
                case NSCamShot::ECamShot_DATA_MSG_YUV:
                case NSCamShot::ECamShot_DATA_MSG_POSTVIEW:
                    filename += String8(".yuv");
                    break;
                case NSCamShot::ECamShot_DATA_MSG_JPEG:
                    filename += String8(".jpeg");
                    break;
                default:
                    break;
            }
            pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
            pBuffer->saveToFile(filename);
            pBuffer->unlockBuf(LOG_TAG);
            //
            MY_LOGD("dump buffer in %s", filename.string());
        }
    }

    // update buffer to gvCapCBHandlers
    switch(streamId){
        case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
        case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
        case eSTREAMID_IMAGE_PIPE_YUV_00:
        case eSTREAMID_IMAGE_PIPE_YUV_01:
            {
                Mutex::Autolock _l(gCapCBHandlerLock);

                auto pCBHandler = getBelongCBHandlerLocked(requestNo);

                if( pCBHandler != nullptr ){
                    pCBHandler->onBufFinished(getOpenId(), requestNo, streamId, pBuffer);
                }else{
                    MY_LOGE("requestNo %d, stream[%" PRId64 "], not registered!", requestNo, streamId);
                }
            }
            break;
        default:
            {
                // do nothing for other streams
                MY_LOGD("requestNo %d, stream[%" PRId64 "], not registered to callback handler", requestNo, streamId);
            }
    }

    {
        Mutex::Autolock _l(mCapReqLock);
        mvCaptureRequests[requestNo].vDstStreams.erase(streamId);

        // use another thread to handle mvCaptureRequests to avoid deadlock in buffeProvider
        // Deadlock:
        // bufferProvider::releaseBuffer()->...->DCMFShot::onDataRecieved()->...->ImageStreamManager::flush()->bufferProvider
        ++mWorkerTriggerCnt;
        mCondCapReqLock.signal();
    }

    prepareAndTriggerPostProc(requestNo);

    MY_LOGD("requestNo %d, steamId[%" PRId64 "] -", requestNo, streamId);
    CAM_TRACE_FMT_END();
}

/*******************************************************************************
*
********************************************************************************/
sp<DCMFShot::CaptureCBHandler>
DCMFShot::
getBelongCBHandlerLocked(const MUINT32 reqNo) const
{
    for(auto it=gvCapCBHandlers.begin(); it!=gvCapCBHandlers.end(); ++it){
        if( it->second->isBelong(getOpenId(), reqNo) ){
            return it->second;
        }
    }
    return nullptr;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
DCMFShot::
prepareAndTriggerPostProc(const MUINT32 reqNo)
{
    Mutex::Autolock _l(gCapCBHandlerLock);

    sp<CaptureCBHandler> pCapCBHandler = getBelongCBHandlerLocked(reqNo);

    if( pCapCBHandler != nullptr ){
        if( pCapCBHandler->preparePostProcess() ){
            MY_LOGD("trigger PostProc (mainReq:%d req:%d)", pCapCBHandler->getMainReqNo(), reqNo);
            pCapCBHandler->triggerPostProcess();

            if( pCapCBHandler->isAllPPSubmitted() ){
                MY_LOGD("erase finished PP request");
                gvCapCBHandlers.erase(pCapCBHandler->getMainReqNo());
                gCondCapCBHandlerLock.broadcast();
            }
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID
DCMFShot::
handleCaptureDoneCallback()
{
    // no need, use p2_done callback with capture done notify instead
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DCMFShot::
handlePostView(
    IImageBuffer *pImgBuffer,
    MUINT32 const requestNo
)
{
    CAM_TRACE_CALL();
    MY_LOGD("+ (pImgBuffer) = (%p)", pImgBuffer);

    class scopedVar
    {
    public:
                    scopedVar(IImageBuffer* pBuf)
                    : mBuffer(pBuf) {
                        if( mBuffer )
                            mBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    }
                    ~scopedVar() {
                        if( mBuffer )
                            mBuffer->unlockBuf(LOG_TAG);
                    }
    private:
        IImageBuffer* mBuffer;
    } _local(pImgBuffer);

    MBOOL   found = MFALSE;
    MINT64  timestamp = 0;
    MBOOL   foregroundPostview = MFALSE;

    for (auto it=gvCapCBHandlers.begin(); it!=gvCapCBHandlers.end(); ++it){
        if(it->second->isBelong(getOpenId(), requestNo)){
            found = MTRUE;
            timestamp = it->second->getTimestamp();
            foregroundPostview = it->second->getForegroundPostviewSupport();
            break;
        }
    }
    if( !found ){
        MY_LOGE("requestNo %d, timestamp not registered!", requestNo);
    }

    if( foregroundPostview ){
        MY_LOGD("send Postview Cb with timestamp(%" PRId64 ")(%p)", timestamp,  mpShotCallback.get());
        mpShotCallback->onCB_PostviewClient(
            timestamp,
            pImgBuffer,
            0, // never ask for offset
            mbBGService
        );
    }else{
        MY_LOGD("no need foregroundPostview");
    }

    MY_LOGD("-");
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DCMFShot::
returnCapReqBuffers(
    __attribute__((unused)) MUINT32 ppReqNo
)
{
    FUNC_START;
// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
    Mutex::Autolock _l(mCapReqLock);

    auto itr = mvCaptureRequests.begin();
    while(itr != mvCaptureRequests.end()){
        if (itr->second.mPPReqNo == ppReqNo){
            MY_LOGD("return ppReqNo(%d) capReq(%d) used capture buffer", ppReqNo, itr->first);
            for( auto &stream : itr->second.vDstStreams){
                sp<CallbackBufferPool> pPool = mpPipeline->queryBufferPool(stream.first);

                sp<IImageBufferHeap> pHeap = stream.second->getImageBufferHeap();

                pPool->returnBufferToPool(LOG_TAG, pHeap);
            }

            itr = mvCaptureRequests.erase(itr);
            mCondCapReqErased.signal();

            if(mvCaptureRequests.empty()){
                afterCapture();
            }
        }else{
            itr++;
        }
    }
#else
    MY_LOGE("Should not call without common capture pipeline support!");
    assert(0);
    return MFALSE;
#endif
    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
android::status_t
DCMFShot::
onPostProcEvent(
    MINT32  requestNo,
    PostProcRequestCB callbackType,
    MUINT32 streamId,
    MBOOL bError,
    void* params1 __attribute__((unused)),
    void* params2 __attribute__((unused))
)
{
    // Notice that the requestNo callback from PostProcMgr is "PostProc" req number.
    MINT32 ppReqNo = requestNo;

    MY_LOGD("PPReqNo(%d) onEvent(%d) steamId(%d) error(%d)",
        ppReqNo, callbackType, streamId, bError
    );

    switch(callbackType){
        case PostProcRequestCB::POSTPROC_DONE:
// #if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#if 0
            gDualOperation.perform(DualOperation::eOp::RETURN_CAP_BUFFERS, ppReqNo);
#endif
            break;
        default:
            MY_LOGW("unknown callbackType(%d)", callbackType);
            return BAD_TYPE;
    }


    {
        Mutex::Autolock _l(mOnGoingPPLock);

        auto itr = mvPPReqMapping.begin();
        while(itr != mvPPReqMapping.end()){
            if(itr->second == ppReqNo){
                MY_LOGD_IF(mDebugLog, "remove req(%d) belong to (%d)", itr->first, ppReqNo);
                mvOnGoingPP.erase(itr->first);
                itr = mvPPReqMapping.erase(itr);
            }else{
                itr++;
            }
        }

        if( mvOnGoingPP.empty() ){
            mCondOnGoingPPLock.broadcast();
            exitHighPerf();
        }
    }

    handleP2Done();

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
DCMFShot::
handleP2Done()
{
    if( !isMaster() ){
        return;
    }

    FUNC_START;
    Mutex::Autolock _l(mOnGoingPPLock);
    MINT32 LIMIT = get_vsdof_cap_queue_size();

    // update mvCapShotBundle, erase finished bundles
    {
        String8 str = String8::format("before refresh mCapShotBundle:");
        for (auto &bundle : mvCapShotBundle){
            std::stringstream s;
            std::copy(bundle.begin(), bundle.end(), std::ostream_iterator<MUINT32>(s, ","));
            str = str + String8::format(" [%s]", s.str().c_str());
        }
        MY_LOGD("%s\n", str.c_str());

        for(auto bundle_itr = mvCapShotBundle.begin(); bundle_itr != mvCapShotBundle.end();){
            // if req not exist in mvOngingPP, which means it is finished, erase it
            for(auto req_itr = (*bundle_itr).begin(); req_itr != (*bundle_itr).end();){
                bool exist = mvOnGoingPP.count((*req_itr)) > 0 ? true : false;

                if( !exist ){
                    req_itr = (*bundle_itr).erase(req_itr);
                }else{
                    ++req_itr;
                }
            }

            // if bundle is empty, erase it
            if( (*bundle_itr).empty() ){
                bundle_itr = mvCapShotBundle.erase(bundle_itr);
            }else{
                ++bundle_itr;
            }
        }

        String8 str2 = String8::format("after refresh mCapShotBundle:");
        for (auto &bundle : mvCapShotBundle){
            std::stringstream s;
            std::copy(bundle.begin(), bundle.end(), std::ostream_iterator<MUINT32>(s, ","));
            str2 = str2 + String8::format(" [%s]", s.str().c_str());
        }
        MY_LOGD("%s\n", str2.c_str());
    }

    MY_LOGI("mP2DoneCnt(%d), onGoingPP(%zu)->bundle(%zu), limit(%d)",
        mP2DoneCnt, mvOnGoingPP.size(), mvCapShotBundle.size(), LIMIT);

    if(mP2DoneCnt > 0 && static_cast<MINT32>(mvCapShotBundle.size()) < LIMIT){
        CAM_TRACE_NAME("trigger P2_Done");
        MY_LOGI("trigger P2_Done");
        mpShotCallback->onCB_P2done(
            mbBGService ? 1 : 0 // callback together with MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE
        );
        mP2DoneCnt --;
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DCMFShot::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)THREAD_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, THREAD_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    // MY_LOGD(
    //     "tid(%d) policy(%d) priority(%d)"
    //     , ::gettid(), ::sched_getscheduler(0)
    //     , sched_p.sched_priority
    // );
    return OK;
}
/*****************************************************************************
 *
 ******************************************************************************/
bool
DCMFShot::
threadLoop()
{
    MY_LOGD_IF(mDebugLog, "+");
    Mutex::Autolock _l(mCapReqLock);

    while(!exitPending() && mWorkerTriggerCnt == 0){
        MY_LOGD_IF(mDebugLog, "worker thread wait + , exit(%d) cnt:(%d)", exitPending(), mWorkerTriggerCnt);

        mCondCapReqLock.wait(mCapReqLock);

        MY_LOGD_IF(mDebugLog, "worker thread wait -");
    }

    if(exitPending()){
        MY_LOGD("exit worker thread");
        FUNC_END;
        return false;
    }

    auto itr = mvCaptureRequests.begin();
    while(itr != mvCaptureRequests.end()){
        if(itr->second.vDstStreams.empty()){
            MY_LOGD_IF(mDebugLog, "worker thread remove req(%d)", itr->first);
            itr = mvCaptureRequests.erase(itr);
            mCondCapReqErased.signal();
        }else{
            itr++;
        }
    }

    mWorkerTriggerCnt --;

    if(mvCaptureRequests.empty()){
        afterCapture();
    }

    MY_LOGD_IF(mDebugLog, "-");
    return  true;
}