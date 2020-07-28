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

#define LOG_TAG "mtkcam-PreReleaseRequest"
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
//
#include <utils/Mutex.h> // android::Mutex
#include <utils/Condition.h>
//
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include <future>
#include <chrono>
//
#include <mtkcam3/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>
//
#include <mtkcam3/feature/eventCallback/EventCallbackMgr.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include "MyUtils.h"
//
CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_PRERELEASE);

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::prerelease;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;


/******************************************************************************
 *
 ******************************************************************************/
#define PRERELEASEREQUEST_NAME       "Cam@PreReleaseRequest"
#define MAX_PRE_RELEASE              (10)
#define MAX_PRE_RELEASE_COUNT_WARN   (5)
#ifdef MTK_LEGACY_PLATFORM
#define MAX_PRE_RELEASE_PIPELINE     (1)
#else
#define MAX_PRE_RELEASE_PIPELINE     (2)
#endif
#define ENABLE_TIMESTAMP_CALLBACK

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_TRACE_API_LIFE()         CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()        CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)     CAM_ULOGM_TAGLIFE(name)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
class PreReleaseRequestMgr : public IPreReleaseRequestMgr
{
    struct  RequestItem
    {
        uint32_t       frameNo;
        uint32_t       preReleaseUid;
        Status         status;
        int32_t        imgReaderID;
        uint32_t       requestStatus;
        int64_t        timestamp;
    };

    struct  InFlightRequestItem
    {
        uint32_t       frameNo;
        bool           bCallback;   // false : callback readerId with -1
        bool           bDone;       // true : jpeg done
        uint32_t       status;
        int64_t        timestamp;
    };

public:
                PreReleaseRequestMgr() {mReqItems.clear();};
  virtual      ~PreReleaseRequestMgr() {};
  virtual sp<IPreReleaseRequest>
                createPreRelease(sp<IPipelineContext> pContext);
  virtual sp<IPreReleaseRequest>
                getPreRelease();
  virtual void  configBGService(IMetadata& meta);
  virtual bool  registerStreamBuffer(android::sp<IPipelineFrame>const& pFrame);
  virtual bool  notifyCaptureCompleted(android::sp<IPipelineFrame>const& pFrame, uint32_t bufferStatus);
  virtual void  setPreReleaseRequest(android::sp<IPreReleaseRequest> pPreRelease);
  virtual void  setRequestFinished(uint32_t frameNo, uint32_t preReleaseUid, uint32_t requestStatus);
  virtual void  addPreReleaseRequest(uint32_t frameNo, uint32_t preReleaseUid, int32_t imgReaderID, int64_t timestamp);
  virtual uint32_t getPreleaseRequestCnt();
  virtual void  uninit();
  virtual void  addCaptureRequest(uint32_t requestNo, bool bCallback);
  virtual void  checkInFlightRequest();
  virtual void  dump();
  virtual void  incPreRelease();
  virtual void  decPreRelease();

protected:
  /* called by configBGService() */
  virtual void  setImageReaderID(uint32_t id);
  /* called by notifyCaptureCompleted, add completed request to mReqItems and return number of request to callback */
  virtual int32_t addCompletedReqAndCheckCbCnt(uint32_t requestNo, uint32_t status, int64_t timestamp);
  /* called by addPreReleaseRequest, remove item from mvInFlightRequests while addPreReleaseRequest called and return true if it is cshot request */
  virtual bool  removeInFlightRequest(uint32_t requestNo);

private:
    mutable android::Mutex                         mLock;
    wp<IPreReleaseRequest>                         mwpPreRelease;
    wp<IPreReleaseRequest>                         mwpPreReleaseP1; // for P1 uninit
    int32_t                                        mImgReaderID = -1;

    mutable android::Mutex                         mFutureLock;
    std::shared_future<void>                       mvFutures[MAX_PRE_RELEASE];
    uint32_t                                       mIdx = 0;
    uint32_t                                       mPreReleaseUid = 0;
    uint32_t                                       mPreReleaseCnt = 0;

    mutable android::Mutex                         mPreReleaseCountLock;
    mutable android::Condition                     mWaitPreReleaseCond;
    bool                                           mbNeedWaitPreRelease = false;
    uint32_t                                       mPreReleaseCount = 0;

    using RequestsT  = Vector<RequestItem>;
    mutable android::Mutex                         mReqItemLock;
    RequestsT                                      mReqItems;           /* the items will callback to AP in-order */

    using InFlightRequestsT  = Vector<InFlightRequestItem>;
    mutable android::Mutex                         mRequestLock;
    InFlightRequestsT                              mvInFlightRequests;  /* add item while addCaptureRequest called */
    InFlightRequestsT                              mvPendingRequests;   /* add item if request NOT found in mvInFlightRequests while notifyCaptureCompleted called */
};


/******************************************************************************
 *
 ******************************************************************************/
class PreReleaseRequest : public IPreReleaseRequest
{

public:
                                         PreReleaseRequest(sp<IPipelineContext> pContext, uint32_t id, uint32_t uid);
virtual                                 ~PreReleaseRequest();

public:
    virtual void                         start();
    virtual void                         uninit();
    virtual void                         waitUntilDrained();
    /* JpegNode will call this API before markPreRelease */
    virtual void                         registerStreamBuffer(
                                             sp<IPipelineFrame>const& pFrame
                                         );
    virtual void                         dump();

protected:   /// data members
    mutable android::Mutex                   mLock;
    android::Condition                       mRequestCond;
    MINT32                                   mLogLevel;

    using PreReleaseBufferT  = KeyedVector<StreamId_T, sp<IStreamBuffer>>;
    using PreReleaseRequestT = KeyedVector<MUINT32, PreReleaseBufferT>;
    MBOOL                                    mbPreRelease = MFALSE;
    PreReleaseRequestT                       mPreReleaseRequests;

    sp<IPipelineContext>                     mpContext;
    int32_t                                  mImgReaderID;
    uint32_t                                 mUid = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
IPreReleaseRequestMgr *
IPreReleaseRequestMgr::
getInstance()
{
    static PreReleaseRequestMgr gPreReleaseMgr;
    return &gPreReleaseMgr;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPreReleaseRequest>
PreReleaseRequestMgr::
createPreRelease(sp<IPipelineContext> pContext)
{
    MY_LOGD("+");
    mPreReleaseUid += 1;
    sp<PreReleaseRequest> pPreRelease = new PreReleaseRequest(pContext, mImgReaderID, mPreReleaseUid);
    {
        Mutex::Autolock _l(mLock);
        mwpPreRelease = pPreRelease;
    }

    // create thread to exec pre-release
    {
        Mutex::Autolock _l(mFutureLock);
        for (auto i = 0; i < MAX_PRE_RELEASE ; i++) {
            mIdx = (mIdx + 1) % MAX_PRE_RELEASE;
            MY_LOGD("mIdx(%d), valid(%d)",mIdx, mvFutures[mIdx].valid());
            std::chrono::milliseconds span (0);
            if (!mvFutures[mIdx].valid() || mvFutures[mIdx].wait_for(span) != std::future_status::timeout) {
                mvFutures[mIdx] = std::async( std::launch::async, [](sp<IPreReleaseRequest> pPreRelease) {
                                                MY_TRACE_TAG_LIFE(PRERELEASEREQUEST_NAME);
                                                ::prctl(PR_SET_NAME, (unsigned long)PRERELEASEREQUEST_NAME, 0, 0, 0);
                                                //
                                                pPreRelease->waitUntilDrained();
                                              }, pPreRelease
                                            );
                break;
            } else {
                MY_LOGD("mIndx(%d) is occupied", mIdx);
            }
            MY_LOGW_IF(i > MAX_PRE_RELEASE_COUNT_WARN, "PreReleaseCount(%d) is over %d", i, MAX_PRE_RELEASE_COUNT_WARN);
        }
    }
    MY_LOGD("-");
    return pPreRelease;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPreReleaseRequest>
PreReleaseRequestMgr::
getPreRelease()
{
    Mutex::Autolock _l(mLock);
    return mwpPreRelease.promote();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PreReleaseRequestMgr::
registerStreamBuffer(
   sp<IPipelineFrame>const& pFrame
)
{
    MY_LOGD("+ (%d)", pFrame->getRequestNo());

    Mutex::Autolock _l(mLock);
    sp<IPreReleaseRequest> pPreRelease = mwpPreRelease.promote();
    if (pPreRelease) {
        pPreRelease->registerStreamBuffer(pFrame);
    }
    MY_LOGD("- (%d)", pFrame->getRequestNo());
    return true;
}



/******************************************************************************
 *
 ******************************************************************************/
bool
PreReleaseRequestMgr::
notifyCaptureCompleted(
   sp<IPipelineFrame>const& pFrame, uint32_t bufferStatus
)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("requestNo(%d)", pFrame->getRequestNo());

    if (mImgReaderID == -1){
        MY_LOGW("mImgReaderID is -1");
        return false;
    }

    uint32_t requestStatus = bufferStatus ? RequestStatus::ERROR : RequestStatus::SUCCESS;
    // add to queue
#ifndef ENABLE_TIMESTAMP_CALLBACK
    int64_t timestamp = 0;
#else
    int64_t timestamp = pFrame->getSensorTimestamp();
#endif
    int32_t count = addCompletedReqAndCheckCbCnt(pFrame->getRequestNo(), requestStatus, timestamp);
    // call requests in-order
    if (count > 0) {
        Mutex::Autolock _l(mRequestLock);
        int32_t readerID;
        bool    bExist;
        for ( auto i = 0 ; i < count ; i++) {
            auto& request = mvInFlightRequests[i];
            bExist = false;
            readerID = request.bCallback ? mImgReaderID : -1;
            Mutex::Autolock _reql(mReqItemLock);
            if (mReqItems.size() > 0){

                for(auto& item : mReqItems) {
                    if (item.frameNo == request.frameNo && item.preReleaseUid == mPreReleaseUid){
                        MY_LOGD("already in mReqItems, skip this request(%d)", request.frameNo);
                        bExist = true;
                        break;
                    }
                }
                if (!bExist) {
                    //add callback to queue until last pre-release finished
                    RequestItem ReqItem = {request.frameNo, 0, Status::RELEASE, readerID, request.status, request.timestamp};
                    mReqItems.add(ReqItem);
                    MY_LOGD("mReqItems size(%zu), requestNo(%d), requestStatus(%d)", mReqItems.size(), request.frameNo, request.status);
                }

            } else {
                //callback to AP
                MY_LOGD("callback requestNo(%d), mImgReaderID(%d), requestStatus(%d)", request.frameNo, readerID, request.status);
                EventCallbackMgr::getInstance()->onRequestFinishedCB(readerID, request.frameNo, request.timestamp, request.status);
            }
        }

        for ( auto i = 0 ; i < count ; i++) {
            mvInFlightRequests.removeAt(0);
        }
    } else {
        MY_LOGD("add requestNo(%d) to queue", pFrame->getRequestNo());
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
configBGService
(
    IMetadata& meta
)
{
    MY_LOGD("+");
    MINT32 prerelease = -1;
    MINT32 imageReaderID = -1;
    //TODO: when IT done, change default value to 0
    MINT32 bgserviceMode = property_get_int32("vendor.debug.camera.bgservice.mode", 0);
    //force off bgservice
    if (bgserviceMode == 2){
        if (IMetadata::getEntry<MINT32>(&meta, MTK_BGSERVICE_FEATURE_IMAGEREADERID, imageReaderID)){
            setImageReaderID(imageReaderID);
        }
        else{
            //force disable but still callback to app
            setImageReaderID(0);
        }
        return;
    }
    //force enable bgservice
    else if(bgserviceMode == 1){
        setImageReaderID(0);
        return;
    }
    if (IMetadata::getEntry<MINT32>(&meta, MTK_BGSERVICE_FEATURE_PRERELEASE, prerelease)){
        if (IMetadata::getEntry<MINT32>(&meta, MTK_BGSERVICE_FEATURE_IMAGEREADERID, imageReaderID)){
            MY_LOGD("isSupportedBGPrerelease=%d, imageReaderID=%d", prerelease==MTK_BGSERVICE_FEATURE_PRERELEASE_MODE_ON, imageReaderID);
            setImageReaderID(imageReaderID);
        }
    }
    else{
        MY_LOGD("can not get prerelease mode from sessionParms, bgserviceMode=%d", bgserviceMode);
        //reset the ImageReaderID
        setImageReaderID(imageReaderID);
    }
    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
setImageReaderID(
    uint32_t id
)
{
    MY_LOGD("+");
    {
        Mutex::Autolock _l(mPreReleaseCountLock);
        if (mPreReleaseCount >= MAX_PRE_RELEASE_PIPELINE) {
            MY_LOGI("wait mPreReleaseCount(%d)+", mPreReleaseCount);
            dump();
            mbNeedWaitPreRelease = true;
            mWaitPreReleaseCond.wait(mPreReleaseCountLock);
            MY_LOGI("wait mPreReleaseCount(%d)-", mPreReleaseCount);
        }
    }

    {
        Mutex::Autolock _l(mLock);
        mImgReaderID = id;
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
setPreReleaseRequest(
    sp<IPreReleaseRequest> pPreRelease
)
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);
    mwpPreReleaseP1 = pPreRelease;

    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
uninit()
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);
    sp<IPreReleaseRequest> pPreRelease = mwpPreReleaseP1.promote();
    if (pPreRelease) {
        pPreRelease->uninit();
    }
    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
addPreReleaseRequest(uint32_t frameNo, uint32_t preReleaseUid, int32_t imgReaderID, int64_t timestamp)
{
    int32_t readerID    = removeInFlightRequest(frameNo) ? imgReaderID : -1;
    RequestItem ReqItem = {frameNo, preReleaseUid, Status::PRERELEASE, readerID, RequestStatus::UNKNOWN, timestamp};
    {
        Mutex::Autolock _reql(mReqItemLock);
        mReqItems.add(ReqItem);
        mPreReleaseCnt++;
        MY_LOGD("mReqItems size(%zu)", mReqItems.size());
    }

    // add items to ready queue in-order
    {
        Mutex::Autolock _l(mRequestLock);
        int32_t count = 0;
        for(auto& item : mvInFlightRequests){
            if(!item.bDone) {
                break;
            }

            readerID = item.bCallback ? imgReaderID : -1;
            ReqItem = {item.frameNo, preReleaseUid, Status::RELEASE, readerID, item.status, item.timestamp};
            MY_LOGD("set requestNo: %d to ReqItems", item.frameNo);
            {
                Mutex::Autolock _reql(mReqItemLock);
                mReqItems.add(ReqItem);
            }
            count++;
        }

        for ( auto i = 0 ; i < count ; i++) {
            mvInFlightRequests.removeAt(0);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
setRequestFinished(uint32_t frameNo, uint32_t preReleaseUid, uint32_t requestStatus)
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);

    Mutex::Autolock _reql(mReqItemLock);
    for(auto& item : mReqItems){
        if (item.frameNo == frameNo && item.preReleaseUid == preReleaseUid){
            MY_LOGD("set status to release (%d)", Status::RELEASE);
            item.status = Status::RELEASE;
            item.requestStatus = requestStatus;
            mPreReleaseCnt--;
            break;
        }
    }
    // start to callback
    bool needCB = true;
    while(needCB == true && mReqItems.size() >0) {
        auto& item = mReqItems[0];
        if(item.status == Status::RELEASE){
            MY_LOGD("callback requestNo(%d)", item.frameNo);
            EventCallbackMgr::getInstance()->onRequestFinishedCB(item.imgReaderID, item.frameNo, item.timestamp, item.requestStatus);
            mReqItems.removeAt(0);
            MY_LOGD("mReqItems size(%zu)", mReqItems.size());
        }
        else{
            needCB = false;
        }
    }
    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
PreReleaseRequestMgr::
getPreleaseRequestCnt()
{
    return mPreReleaseCnt;
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
checkInFlightRequest()
{
    Mutex::Autolock _rl(mRequestLock);
    bool bError = false;
    for(auto& request : mvInFlightRequests){
        bool bFound = false;
        for (size_t i = 0 ; i < mvPendingRequests.size() ; i++){
            if (request.frameNo == mvPendingRequests[i].frameNo) {
                bFound =true;
                request.status = mvPendingRequests[i].status;
                request.bDone  = true;
                request.timestamp = mvPendingRequests[i].timestamp;
                mvPendingRequests.erase(mvPendingRequests.begin()+i);
                break;
            }
        }

        if (!bFound) {
            bError = true;
        } else {
            bool bExist = false;
            int32_t readerID = request.bCallback ? mImgReaderID : -1;
            Mutex::Autolock _reql(mReqItemLock);
            if (mReqItems.size() > 0){

                for(auto& item : mReqItems){
                    if (item.frameNo == request.frameNo && item.preReleaseUid == mPreReleaseUid){
                        MY_LOGD("already in mReqItems, skip this request(%d)", request.frameNo);
                        bExist = true;
                        break;
                    }
                }
                if (!bExist) {
                    //add callback to queue until last pre-release finished
                    RequestItem ReqItem = {request.frameNo, 0, Status::RELEASE, readerID, request.status, request.timestamp};
                    mReqItems.add(ReqItem);
                    MY_LOGD("mReqItems size(%zu), requestNo(%d), requestStatus(%d)", mReqItems.size(), request.frameNo, request.status);
                }

            } else {
                //callback to AP
                MY_LOGD("callback requestNo(%d), mImgReaderID(%d), requestStatus(%d)", request.frameNo, readerID, request.status);
                EventCallbackMgr::getInstance()->onRequestFinishedCB(readerID, request.frameNo, request.timestamp, request.status);
            }
        }
    }

    if (bError || mvPendingRequests.size() > 0) {
        MY_LOGW("bError %d, PendingRequests(%zu)", bError, mvPendingRequests.size());
    }

    mvPendingRequests.clear();
    mvInFlightRequests.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
dump()
{
    MY_LOGI("========== dump PreReleaseRequestMgr ===========");
    {
        Mutex::Autolock _l(mRequestLock);
        MY_LOGI("========== dump InFlightRequests(%zu) ============", mvInFlightRequests.size());
        for(auto& item : mvInFlightRequests){
            MY_LOGI("requestNo: %d, bCallback %d, bDone %d, status %d", item.frameNo, item.bCallback, item.bDone, item.status);
        }
        MY_LOGI("========== dump PendingRequests(%zu) ============", mvPendingRequests.size());
        for(auto& item : mvPendingRequests){
            MY_LOGI("requestNo: %d, bCallback %d, bDone %d, status %d", item.frameNo, item.bCallback, item.bDone, item.status);
        }
    }

    MY_LOGD("========== dump RequestItems ====================");
    {
        Mutex::Autolock _reql(mReqItemLock);
        for(auto& item : mReqItems){
            MY_LOGI("requestNo: %d, preReleaseUid %d, status %d, imgReaderID %d, requestStatus %d",
                     item.frameNo, item.preReleaseUid, item.status, item.imgReaderID, item.requestStatus);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
incPreRelease()
{
    Mutex::Autolock _l(mPreReleaseCountLock);
    mPreReleaseCount++;
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
decPreRelease()
{
    Mutex::Autolock _l(mPreReleaseCountLock);
    mPreReleaseCount--;
    if (mbNeedWaitPreRelease) {
        mbNeedWaitPreRelease = false;
        mWaitPreReleaseCond.broadcast();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequestMgr::
addCaptureRequest(uint32_t requestNo, bool bCallback)
{

#ifndef DISABLE_CSHOT_CALLBACK
    bCallback = true;
#endif
    {
        Mutex::Autolock _l(mLock);
        if (mImgReaderID < 0) {
            MY_LOGD("skip non-bgs capture, requestNo: %d", requestNo);
            return ;
        }
    }
    {
        Mutex::Autolock _l(mRequestLock);

        InFlightRequestItem item;
        item.frameNo   = requestNo;
        item.bCallback = bCallback;
        item.bDone     = false;
        item.status    = 0;
        item.timestamp = 0;

        mvInFlightRequests.push_back(item);
        MY_LOGD("add requestNo: %d, cshot %d, inFlightRequests %zu", requestNo, !bCallback, mvInFlightRequests.size());
    }
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
PreReleaseRequestMgr::
addCompletedReqAndCheckCbCnt(uint32_t requestNo, uint32_t status, int64_t timestamp)
{
    Mutex::Autolock _l(mRequestLock);
    int32_t count = 0;
    bool bCheck = true;
    bool bFound = false;

    for(auto& item : mvInFlightRequests){
        if (requestNo == item.frameNo) {
            item.bDone  = true;
            item.status = status;
            item.timestamp = timestamp;
            bFound = true;
            MY_LOGD("set requestNo: %d to inFlightRequests", requestNo);
        }
        if (bCheck && item.bDone) {
            count++;
        } else {
            bCheck = false;
        }
    }

    if (!bFound) {
        InFlightRequestItem item;
        item.frameNo   = requestNo;
        item.bCallback = true;    // need to sync with addCaptureRequest's bCallback
        item.bDone     = true;
        item.status    = status;
        item.timestamp = timestamp;
        mvPendingRequests.push_back(item);
        MY_LOGW("requestNo: %d Not in inFlightRequests(size %zu), add to PendingRequests(size %zu)", requestNo, mvInFlightRequests.size(), mvPendingRequests.size());
    }
    return count;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
PreReleaseRequestMgr::
removeInFlightRequest(uint32_t requestNo)
{
    Mutex::Autolock _l(mRequestLock);
    bool bRet = true;
    for (size_t i = 0 ; i < mvInFlightRequests.size() ; i++) {
        if (requestNo == mvInFlightRequests[i].frameNo) {
            bRet = mvInFlightRequests[i].bCallback;
            mvInFlightRequests.erase(mvInFlightRequests.begin()+i);
            MY_LOGD("remove requestNo: %d, bCallback %d, size %zu, idx %zu", requestNo, bRet, mvInFlightRequests.size(), i);
            return bRet;
        }
    }
    MY_LOGW("not found in InFlightRequests!! size %zu", mvInFlightRequests.size());
    return bRet;
}

/******************************************************************************
 *
 ******************************************************************************/
PreReleaseRequest::
PreReleaseRequest(sp<IPipelineContext> pContext, uint32_t id, uint32_t uid)
{
    mLogLevel = property_get_int32("vendor.debug.camera.log", 0);
    if ( 0 == mLogLevel ) {
        mLogLevel = property_get_int32("vendor.debug.camera.log.PreReleaseRequest", 0);
    }
    mpContext = pContext;
    mImgReaderID = id;
    mUid = uid;
    IPreReleaseRequestMgr::getInstance()->incPreRelease();
}


/******************************************************************************
 *
 ******************************************************************************/
PreReleaseRequest::
~PreReleaseRequest()
{
    IPreReleaseRequestMgr::getInstance()->decPreRelease();
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequest::
start()
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);
    mbPreRelease = MTRUE;
    dump();
    IPreReleaseRequestMgr::getInstance()->dump();
    IPreReleaseRequestMgr::getInstance()->checkInFlightRequest();
    mRequestCond.broadcast();
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequest::
uninit()
{
    MY_LOGD("+, uninit P1Node");
//    Mutex::Autolock _l(mLock);
    NodeId_T P1NodeId = eNODEID_UNKNOWN;
    int i = 0;
    do
    {
        P1NodeId = NodeIdUtils::getP1NodeId(i);
        auto pNode = mpContext->queryINodeActor(P1NodeId);
        if (pNode != NULL) {
          pNode->uninit();
          MY_LOGD("Node(%s) done", toHexString(P1NodeId).c_str());
        }
        i++;
    } while(P1NodeId != eNODEID_UNKNOWN);

    auto pP2StreamNode = mpContext->queryINodeActor(eNODEID_P2StreamNode);
    if (pP2StreamNode != NULL) {
      pP2StreamNode->uninit();
      MY_LOGI("uninit P2StreamNode done");
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequest::
waitUntilDrained()
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);
    MBOOL isHasRequest = MFALSE;
    while (mbPreRelease == MFALSE) {
        MY_LOGD("wait+");
        auto ret = mRequestCond.waitRelative(mLock, 60000000000); // timeout: 60 secs
        if (ret != OK) {
            MY_LOGW("timeout!!");
            mbPreRelease = MTRUE;
            break;
        }
        MY_LOGD("wait-");
    }

    MY_LOGD("Request size: %zu", mPreReleaseRequests.size());
    if (mPreReleaseRequests.size() > 0) {
        isHasRequest = MTRUE;
        IPreReleaseRequestMgr::getInstance()->setPreReleaseRequest(this);
    }

    uint32_t requestStatus = IPreReleaseRequestMgr::RequestStatus::SUCCESS;
    while(mPreReleaseRequests.size() > 0) {
        auto requestNo = mPreReleaseRequests.keyAt(0);
        auto preReleaseBuffers = mPreReleaseRequests.valueAt(0);
        for (size_t i = 0 ; i < preReleaseBuffers.size() ; i++) {
            auto streambuffer = preReleaseBuffers.valueAt(i);
            MY_LOGI("requestNo(%d), preReleaseBuffers.size(%zu), idx(%zu)", requestNo, preReleaseBuffers.size(), i);
            // wait streambuffer done
            if (streambuffer == nullptr){
                MY_LOGD("requestNo(%d) streambuffer is null, idx(%zu)", requestNo, i);
            }
            else{
                auto ret = streambuffer->waitUserReleaseStatus(LOG_TAG);
                if (ret != OK) {
                    MY_LOGE("requestNo(%d) streamName(%s) wait streambuffer status fail!!", requestNo, streambuffer->getName());

                    // dump PreReleaseRequest and PreReleaseRequestMgr
                    dump();
                    IPreReleaseRequestMgr::getInstance()->dump();

                    // dump P2CaptureNode and getStatus
                    std::string str;
                    auto pP2CaptureNode = mpContext->queryINodeActor(eNODEID_P2CaptureNode);
                    if ( pP2CaptureNode != nullptr) {
                        auto pNode = pP2CaptureNode->getNode();
                        if (pNode != nullptr) {
                            str = pNode->getStatus();
                            MY_LOGI("%s", str.c_str());
                            IPipelineNode::TriggerDB arg;
                            arg.msg = str.c_str();
                            arg.needDumpCallstack = true;
                            arg.needTerminateCurrentProcess = true;
                            pNode->triggerdb(arg);
                        }
                    }
                }
                if (streambuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ){
                    MY_LOGE("requestNo(%d) streambuffer status is ERROR", requestNo);
                    requestStatus = IPreReleaseRequestMgr::RequestStatus::ERROR;
                }
            }
        }
        mPreReleaseRequests.removeItemsAt(0);
        IPreReleaseRequestMgr::getInstance()->setRequestFinished(requestNo, mUid, requestStatus);
        // callback to AP
        // EventCallbackMgr::getInstance()->onRequestFinishedCB(mImgReaderID, requestNo, 0, 0);
    }
    if (isHasRequest == MTRUE) {
        IPreReleaseRequestMgr::getInstance()->setPreReleaseRequest(nullptr);
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequest::
registerStreamBuffer(
    sp<IPipelineFrame>const& pFrame
)
{
    auto frameNo = pFrame->getFrameNo();
    auto requestNo = pFrame->getRequestNo();
    MY_LOGD("+ R/F Num: %d/%d", requestNo, frameNo);
    Mutex::Autolock _l(mLock);

    if (mbPreRelease == MTRUE) {
        MY_LOGW("Something wrong");
        return;
    }

    if (mPreReleaseRequests.indexOfKey(requestNo) >= 0) {
        MY_LOGD("requestNo(%d) is already in", requestNo);
        return;
    }
    sp<IStreamInfoSet const> pIStreams, pOStreams;
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();

#ifndef ENABLE_TIMESTAMP_CALLBACK
    int64_t timestamp = 0;
#else
    int64_t timestamp = pFrame->getSensorTimestamp();
#endif

    for (size_t i = 0 ; i < pPipelineNodeMap->size() ; i++)
    {
        PreReleaseBufferT streambuffers;
        auto pNode = pPipelineNodeMap->nodeAt(i);
        auto nodeId = pNode->getNodeId();
        MERROR err = pFrame->queryIOStreamInfoSet(nodeId, pIStreams, pOStreams);
        if  ( OK != err ) {
            MY_LOGW("nodeId:%#" PRIxPTR " frameNo:%d queryIOStreamInfoSet", nodeId, frameNo);
            continue;
        }

        if  ( IStreamInfoSet const* pStreams = pIStreams.get() ) {
            {// I:Image
                sp<IStreamInfoSet::IMap<IImageStreamInfo> > pMap = pStreams->getImageInfoMap();
                for (size_t i = 0; i < pMap->size(); i++) {
                    StreamId_T const streamId = pMap->valueAt(i)->getStreamId();

                    sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, nodeId);
                    if (streambuffers.indexOfKey(streamId) < 0) {
                        streambuffers.add(streamId, pStreamBuffer);
                        //MY_LOGD("add input stream id: %ld",streamId);
                    }
                }
            }
        }
        else {
            MY_LOGE("nodeId:%#" PRIxPTR " frameNo:%d NULL IStreams", nodeId, frameNo);
        }
        //
        if  ( IStreamInfoSet const* pStreams = pOStreams.get() ) {
            {// O:Image
                sp<IStreamInfoSet::IMap<IImageStreamInfo> > pMap = pStreams->getImageInfoMap();
                for (size_t i = 0; i < pMap->size(); i++) {
                    StreamId_T const streamId = pMap->valueAt(i)->getStreamId();

                    sp<IStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, nodeId);
                    if (streambuffers.indexOfKey(streamId) < 0) {
                        streambuffers.add(streamId, pStreamBuffer);
                        //MY_LOGD("add output stream id: %ld",streamId);
                    }
                }
            }
        }
        else {
            MY_LOGE("nodeId:%#" PRIxPTR " frameNo:%d NULL OStreams", nodeId, frameNo);
        }
        mPreReleaseRequests.add(requestNo, streambuffers);
    }
    IPreReleaseRequestMgr::getInstance()->addPreReleaseRequest(requestNo, mUid, mImgReaderID, timestamp);
    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
void
PreReleaseRequest::
dump()
{
    MY_LOGD("========== dump PreReleaseRequest ===========");
    MY_LOGD("mUid %d, mImgReaderID %d", mUid, mImgReaderID);
    for (size_t i = 0 ; i < mPreReleaseRequests.size() ; i++) {
        MY_LOGD("requestNo: %d", mPreReleaseRequests.keyAt(i));
    }
}
