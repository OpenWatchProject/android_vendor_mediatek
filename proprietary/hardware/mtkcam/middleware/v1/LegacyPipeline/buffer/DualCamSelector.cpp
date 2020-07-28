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

#define LOG_TAG "MtkCam/DualCamSelector"

#include "MyUtils.h"

#include <utils/RWLock.h>
#include <sstream>
#include <vector>

#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

using namespace android;
using namespace std;

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;

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

#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define SYNC_THRESHOLD_MS               1


/******************************************************************************
 * Implementor Declaration
 ******************************************************************************/
class DualCamSelector::Implementor
{
friend class DualCamSelector;

public:
                                        Implementor(
                                                MINT32 openId,
                                                DualCamSelector *selector
                                        );

                                        ~Implementor();

    status_t                            selectResult(
                                                MINT32&                          requestNo,
                                                Vector<MetaItemSet>&             vMetaSet,
                                                Vector<BufferItemSet>&           vBufferSet,
                                                MBOOL&                           errorResult
                                        );

    status_t                            getResult(
                                                MINT32&                          rRequestNo,
                                                Vector<MetaItemSet>&             rvMetaSet,
                                                Vector<BufferItemSet>&           rvBufferSet
                                        );

    status_t                            returnBuffer(BufferItemSet &rBuffer);

    status_t                            flush();

protected:

    typedef std::map<MINT32, Implementor*>  SelectorRegistry;

    struct ResultSet {
        MINT32                  requestNo;
        Vector<BufferItemSet>   bufferSet;
        Vector<MetaItemSet>     metaSet;
        MINT64                  timestamp;
        MBOOL                   afStable;

        ResultSet& operator=(ResultSet const & rhs) {
            requestNo = rhs.requestNo;
            bufferSet = rhs.bufferSet;
            metaSet   = rhs.metaSet;
            timestamp = rhs.timestamp;
            afStable  = rhs.afStable;

            return *this;
        }
    };

    static SelectorRegistry& registry()
    {
        static SelectorRegistry *gRegistry = new SelectorRegistry();
        return *gRegistry;
    }

    static MVOID                        addSelector(MINT32 openId, Implementor *pSelector);

    static MVOID                        removeSelector(MINT32 openId, Implementor *pSelector);

    static MVOID                        pollSelectors();

    MBOOL                               isTimeSync(MUINT64 t1, MUINT64 t2);

    MBOOL                               isTimeExpired(MUINT64 base, MUINT64 t);

    MVOID                               expire(MUINT64 timestamp);

    MBOOL                               tolerate(MUINT64 timestamp);

    MUINT64                             getPendingTimestamp();

    MVOID                               moveToAvailable();

    MVOID                               parseMetadata(Vector<MetaItemSet> &metaSet, ResultSet& result);

    status_t                            returnBuffersToPool(Vector<BufferItemSet>& rvBuffers);

protected:

    static Mutex                        sRegistryLock;
    DualCamSelector*                    mpDualCamSelector;

    vector<ResultSet>                   mPendingQueue;
    vector<ResultSet>                   mAvailableQueue;

    Vector<MINT32>                      mRequests;
    Vector<MINT32>                      mRequests_group2;

    MBOOL                               mRequestMode;
    Mutex                               mQueueLock;
    Condition                           mQueueCond;
    MBOOL                               mFlush;
    MUINT32                             mLogLevel;
    MINT32                              mOpenId;

    MUINT32                             mExpectedCount;
    MUINT32                             mExpectedCount_group2;

    MINT32                              mByPassMode;
    sp<ISelector>                       mpOriginSelector;
};


/******************************************************************************
 * DaulCamSelector Wrapper
 ******************************************************************************/
DualCamSelector::
DualCamSelector(MINT32 openId)
    : mpImp(new Implementor(openId, this))
{
}

DualCamSelector::
~DualCamSelector()
{
    if (mpImp) delete mpImp;
}

status_t
DualCamSelector::
selectResult(
    MINT32                             requestNo,
    Vector<MetaItemSet>                vMetaSet,
    Vector<BufferItemSet>              vBufferSet,
    MBOOL                              errorResult
)
{
    if (mPoolItemMap.size() == 0) {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

    return mpImp->selectResult(requestNo, vMetaSet, vBufferSet, errorResult);
}

status_t
DualCamSelector::
getResult(
    MINT32&                          rRequestNo,
    Vector<MetaItemSet>&             rvMetaSet,
    Vector<BufferItemSet>&           rvBufferSet
)
{
    if (mPoolItemMap.size() == 0) {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

    return mpImp->getResult(rRequestNo, rvMetaSet, rvBufferSet);
}

status_t
DualCamSelector::
flush()
{
    if (mPoolItemMap.size() == 0) {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

    return mpImp->flush();
}

status_t
DualCamSelector::
returnBuffer(BufferItemSet &rBuffer)
{
    return mpImp->returnBuffer(rBuffer);
}

void
DualCamSelector::
setRequests(Vector<MINT32> requests)
{
    if (requests.empty())
        return;

    mpImp->mRequests = requests;
    mpImp->mRequestMode = MTRUE;

    if (mLogLevel > 0) {
        std::stringstream s;
        std::copy(requests.begin(), requests.end(), std::ostream_iterator<MINT32>(s, " "));
        MY_LOGD("set requests[%d]: %s", mpImp->mOpenId, s.str().c_str());
    }
}

void
DualCamSelector::
setRequests(Vector<MINT32> requests, Vector<MINT32> requests_group2)
{
    if (requests.empty())
        return;

    if (requests_group2.empty())
        return;

    mpImp->mRequests = requests;
    mpImp->mRequests_group2 = requests_group2;

    mpImp->mRequestMode = MTRUE;

    {
        std::stringstream s;
        std::copy(requests.begin(), requests.end(), std::ostream_iterator<MINT32>(s, " "));
        MY_LOGD("set requests[%d]: %s", mpImp->mOpenId, s.str().c_str());
    }

    {
        std::stringstream s;
        std::copy(requests_group2.begin(), requests_group2.end(), std::ostream_iterator<MINT32>(s, " "));
        MY_LOGD("set requests_group2[%d]: %s", mpImp->mOpenId, s.str().c_str());
    }
}

void
DualCamSelector::
setExpectedCount(MINT32 expectedCount)
{
    mpImp->mExpectedCount = expectedCount;
    MY_LOGD("set expected count[%d]: %d", mpImp->mOpenId, expectedCount);
}

void
DualCamSelector::
setExpectedCount(MINT32 expectedCount, MINT32 expectedCount_group2)
{
    mpImp->mExpectedCount = expectedCount;
    mpImp->mExpectedCount_group2 = expectedCount_group2;
    MY_LOGD("set expected count[%d]: %d, %d", mpImp->mOpenId, expectedCount, mpImp->mExpectedCount_group2);
}

void
DualCamSelector::
setOriginSelector(sp<ISelector> pOriSelector)
{
    mpImp->mpOriginSelector = pOriSelector;
    MY_LOGD("set origin selector[%d]: %p", mpImp->mOpenId, mpImp->mpOriginSelector.get());
}

void
DualCamSelector::
onLastStrongRef(const void*)
{
    FUNC_START;
    flush();
    FUNC_END;
}

/******************************************************************************
 * Implementor
 ******************************************************************************/
Mutex DualCamSelector::Implementor::sRegistryLock;

DualCamSelector::Implementor::
Implementor(MINT32 openId, DualCamSelector *pSelector)
    : mpDualCamSelector(pSelector)
    , mRequestMode(MFALSE)
    , mFlush(MFALSE)
    , mOpenId(openId)
    , mExpectedCount(0)
    , mExpectedCount_group2(0)
    , mpOriginSelector(nullptr)
{
    mLogLevel = ::property_get_int32("vendor.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.camera.log.dualcamsel", 0);
    }

    mByPassMode = property_get_int32("vendor.camera.dualcamsel.bypass", 0);

    addSelector(mOpenId, this);
}

DualCamSelector::Implementor::
~Implementor()
{
    removeSelector(mOpenId, this);
}

status_t
DualCamSelector::Implementor::
selectResult(
    MINT32&                            requestNo,
    Vector<MetaItemSet>&               vMetaSet,
    Vector<BufferItemSet>&             vBufferSet,
    MBOOL&                             errorResult
)
{
    FUNC_START;

    MY_LOGD("select result[%d] error:%d request(%d) %s ",
        mOpenId, errorResult, requestNo, ISelector::logBufferSets(vBufferSet).string());

    // check group
    auto it     = std::find(mRequests.begin(), mRequests.end(), requestNo);
    auto it_g2  = std::find(mRequests_group2.begin(), mRequests_group2.end(), requestNo);

    if(errorResult)
    {
        MY_LOGW("don't reserved errorResult(%d) requestNo(%d)", errorResult, requestNo);

        if (it != mRequests.end()) {
            mRequests.erase(it);
        }

        if (it_g2 != mRequests_group2.end()) {
            mRequests_group2.erase(it_g2);
        }

        returnBuffersToPool(vBufferSet);
        return UNKNOWN_ERROR;
    }

    if (it_g2 != mRequests_group2.end()) {
        MY_LOGD("requests group 2");
        if(mExpectedCount != 0){
            MY_LOGE("group 1 not finished yet!");
            return UNKNOWN_ERROR;
        }
        mExpectedCount = mExpectedCount_group2;
        mExpectedCount_group2 = 0;
    }else{
        MY_LOGD("requests group 1");
    }

    {
        Mutex::Autolock _l(mQueueLock);
        MBOOL doFlush =  mFlush ||
                        (mExpectedCount == 0 && it == mRequests.end()) ||
                        (mExpectedCount == 0 && it_g2 == mRequests.end()) ||
                        (mExpectedCount <= mAvailableQueue.size());
        MY_LOGD_IF(doFlush, "flush(%d) mExpectedCount(%d) mExpectedCount_group2(%d) mAva(%d)",
            mFlush, mExpectedCount, mExpectedCount_group2, mAvailableQueue.size());
        if (doFlush) {
            returnBuffersToPool(vBufferSet);
            return OK;
        }
    }

    MBOOL isLastRequests = MFALSE;
    if (!mRequestMode) {
        isLastRequests = MFALSE;
    } else {
        /* return buffers if not in the list of request */
        if (it == mRequests.end() && it_g2 == mRequests_group2.end()) {
            MY_LOGD_IF(mLogLevel > 0, "return not belong to both group");
            returnBuffersToPool(vBufferSet);
            return OK;
        }

        /* force to be the available if not have enough requests */
        if (it != mRequests.end()) {
            MY_LOGD_IF(mLogLevel > 0, "group 1 select");
            isLastRequests = mRequests.size() <= mExpectedCount;
            mRequests.erase(it);
        }

        if (it_g2 != mRequests_group2.end()) {
            MY_LOGD_IF(mLogLevel > 0, "group 2 select");
            isLastRequests = mRequests_group2.size() <= mExpectedCount;
            mRequests_group2.erase(it_g2);
        }
    }

    MY_LOGD("isLastRequests(%d)", isLastRequests);

    /* construct a ResultSet, and parse metadata */
    ResultSet result = {
        .requestNo = requestNo,
        .metaSet   = vMetaSet,
        .bufferSet = vBufferSet
    };

    parseMetadata(vMetaSet, result);

    if (isLastRequests) {
        mQueueLock.lock();
        mAvailableQueue.push_back(move(result));
        mQueueCond.broadcast();
        mExpectedCount--;
        mQueueLock.unlock();
    } else {
        mQueueLock.lock();
        mPendingQueue.push_back(move(result));
        mQueueLock.unlock();
        pollSelectors();
    }

    FUNC_END;
    return OK;
}

status_t
DualCamSelector::Implementor::
getResult(
    MINT32&                          rRequestNo,
    Vector<MetaItemSet>&             rvMetaSet,
    Vector<BufferItemSet>&           rvBufferSet
)
{
    FUNC_START;
    Mutex::Autolock _l(mQueueLock);

    if (mAvailableQueue.empty()) {
        MY_LOGD_IF(mLogLevel > 0, "wait for available result.");

        //wait for 3 sec
        nsecs_t timeout = 3000000000LL;
        status_t status = mQueueCond.waitRelative(mQueueLock, timeout);
        if (status != OK) {
            MY_LOGE("timeout: wait 3s for a result");
            return UNKNOWN_ERROR;
        }
    }

    const ResultSet &result = mAvailableQueue[0];

    rRequestNo = result.requestNo;
    rvMetaSet = result.metaSet;
    rvBufferSet = result.bufferSet;

    mAvailableQueue.erase(mAvailableQueue.begin());
    // pure ZSD mode
    if (!mRequestMode)
        mExpectedCount--;

    MY_LOGD("get result[%d]: request(%d), %s",
           mOpenId, rRequestNo, ISelector::logBufferSets(rvBufferSet).string());

    FUNC_END;
    return OK;
}

status_t
DualCamSelector::Implementor::
returnBuffer(BufferItemSet &rBuffer)
{
    if(mpOriginSelector != nullptr){
        MY_LOGD_IF(mLogLevel > 0, "return buffer to original selector");
        return mpOriginSelector->returnBuffer(rBuffer);
    }else{
        MY_LOGE("mpOriginSelector is NULL!");
        return UNKNOWN_ERROR;
    }
}

status_t
DualCamSelector::Implementor::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mQueueLock);
    mFlush = MTRUE;

    vector<ResultSet>::iterator it;
    for (it = mPendingQueue.begin(); it != mPendingQueue.end(); ) {
        returnBuffersToPool((*it).bufferSet);
        it = mPendingQueue.erase(it);
    }

    for (it = mAvailableQueue.begin(); it != mAvailableQueue.end(); ) {
        returnBuffersToPool((*it).bufferSet);
        it = mAvailableQueue.erase(it);
    }

    FUNC_END;
    return OK;
}

MVOID
DualCamSelector::Implementor::
addSelector(MINT32 openId, Implementor *pSelector)
{
    Mutex::Autolock _l(sRegistryLock);
    SelectorRegistry& reg = registry();
    if (reg.count(openId) != 0) {
        MY_LOGE("The selector is already registered!");
        return;
    }
    reg[openId] = pSelector;
}

MVOID
DualCamSelector::Implementor::
removeSelector(MINT32 openId, Implementor *pSelector)
{
    Mutex::Autolock _l(sRegistryLock);
    SelectorRegistry& reg = registry();
    if (reg.count(openId) == 0) {
        MY_LOGE("The selector is not registed!");
        return;
    }

    if (reg[openId] != pSelector) {
        MY_LOGW("The selector to remove is not registered!");
    }

    reg.erase(openId);
}

MVOID
DualCamSelector::Implementor::
pollSelectors()
{
    Mutex::Autolock _l(sRegistryLock);
    SelectorRegistry& reg = registry();

    if (reg.size() < 2)
        return;

    SelectorRegistry::iterator it;
    Implementor *sel;
    MBOOL isAllTolerant = MTRUE;
    MUINT64 checkTime = 0;
    MUINT64 timestamp = 0;

    for (it = reg.begin(); it != reg.end(); ++it) {
        sel = it->second;
        timestamp = sel->getPendingTimestamp();

        if (timestamp == 0)
            return;
        if (checkTime < timestamp)
            checkTime = timestamp;
    }

    for (it = reg.begin(); it != reg.end(); ++it) {
        sel = it->second;
        sel->expire(checkTime);
        if (!sel->tolerate(checkTime))
            isAllTolerant = MFALSE;
    }

    if (!isAllTolerant)
        return;

    for (it = reg.begin(); it != reg.end(); ++it) {
        sel = it->second;
        sel->moveToAvailable();
    }
}

MBOOL
DualCamSelector::Implementor::
isTimeSync(MUINT64 t1, MUINT64 t2)
{
    if(mByPassMode == 1){
        MY_LOGD("skip timestamp check");
        return MTRUE;
    }else{
        MUINT64 diffInMs = std::abs((MINT64)(t1 - t2) / 1000000);
        return (diffInMs <= SYNC_THRESHOLD_MS);
    }
}

MBOOL
DualCamSelector::Implementor::
isTimeExpired(MUINT64 base, MUINT64 t)
{
    if (isTimeSync(base, t))
        return MFALSE;
    else
        if (base > t)
            return MTRUE;
        else
            return MFALSE;
}

status_t
DualCamSelector::Implementor::
returnBuffersToPool(Vector<BufferItemSet>& rvBuffers)
{
    MBOOL ret = MTRUE;

    auto it = rvBuffers.begin();
    for (; it != rvBuffers.end(); it++)
    {
        if (mpDualCamSelector->returnBuffer(*it) != NO_ERROR)
            ret = MFALSE;
    }

    rvBuffers.clear();

    return ret ? NO_ERROR : UNKNOWN_ERROR;
}

MVOID
DualCamSelector::Implementor::
expire(MUINT64 timestamp)
{
    Mutex::Autolock _l(mQueueLock);
    auto it = mPendingQueue.begin();
    for (; it != mPendingQueue.end();) {
        ResultSet& r = (*it);

        if (isTimeExpired(timestamp, r.timestamp)) {
            MY_LOGD("remove from pending queue[%d]: request(%d)", mOpenId, r.requestNo);
            returnBuffersToPool(r.bufferSet);
            it = mPendingQueue.erase(it);
        } else
            break;
    }
}

MBOOL
DualCamSelector::Implementor::
tolerate(MUINT64 timestamp)
{
    Mutex::Autolock _l(mQueueLock);
    if (!mPendingQueue.empty()) {
        if (isTimeSync(timestamp, mPendingQueue[0].timestamp))
            return MTRUE;
    }
    return MFALSE;
}

MUINT64
DualCamSelector::Implementor::
getPendingTimestamp()
{
    Mutex::Autolock _l(mQueueLock);
    MUINT64 timestamp = 0;
    if (!mPendingQueue.empty())
        timestamp = mPendingQueue[0].timestamp;
    return timestamp;
}

MVOID
DualCamSelector::Implementor::
moveToAvailable()
{
    Mutex::Autolock _l(mQueueLock);
    if (!mPendingQueue.empty()) {

        if (mExpectedCount <= mAvailableQueue.size() && !mAvailableQueue.empty()) {
            ResultSet& r = *mAvailableQueue.begin();
            returnBuffersToPool(r.bufferSet);
            MY_LOGD("Return a sync buffer[%d]: requestNo=%d", mOpenId, r.requestNo);
            mAvailableQueue.erase(mAvailableQueue.begin());
        }

        auto it = mPendingQueue.begin();
        MY_LOGD("Get a sync buffer[%d]: requestNo=%d", mOpenId, (*it).requestNo);

        mAvailableQueue.insert(mAvailableQueue.end(),
                make_move_iterator(it),
                make_move_iterator(it + 1));
        mPendingQueue.erase(it);

        // using request number mode
        if (mRequestMode)
            mExpectedCount--;
        mQueueCond.broadcast();
    }
}

MVOID
DualCamSelector::Implementor::
parseMetadata(Vector<MetaItemSet> &metaSet, ResultSet& result)
{
    MUINT64 timestamp = 0;
    MBOOL afStable = MFALSE;
    MUINT8 afState = -1;
    MUINT8 lensState = -1;

    IMetadata::IEntry entry;
    for (size_t i = 0; i < metaSet.size(); i++)
    {
        const IMetadata& meta = metaSet[i].meta;

        if (metaSet[i].id == eSTREAMID_META_HAL_DYNAMIC_P1) {
            entry = meta.entryFor(MTK_P1NODE_FRAME_START_TIMESTAMP);
            if (!entry.isEmpty())
                timestamp = entry.itemAt(0, Type2Type<MINT64>());

        } else if (metaSet[i].id == eSTREAMID_META_APP_DYNAMIC_P1) {
            entry = meta.entryFor(MTK_CONTROL_AF_STATE);
            if (!entry.isEmpty())
                afState = entry.itemAt(0, Type2Type<MUINT8>());

            entry = meta.entryFor(MTK_LENS_STATE);
            if (!entry.isEmpty())
                lensState = entry.itemAt(0, Type2Type<MUINT8>());
        }
    }

    if (afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN ||
        afState == MTK_CONTROL_AF_STATE_ACTIVE_SCAN ||
        lensState == MTK_LENS_STATE_MOVING)
        afStable = MFALSE;
    else
        afStable = MTRUE;

    MY_LOGD("Parse Metadata[%d]: requestNo=%d, timestamp=%" PRIu64 ", afState=%d,  lensState=%d,  afStable:%d",
            mOpenId, result.requestNo, timestamp, afState, lensState, afStable);

    result.timestamp = timestamp;
    result.afStable = afStable;
}

