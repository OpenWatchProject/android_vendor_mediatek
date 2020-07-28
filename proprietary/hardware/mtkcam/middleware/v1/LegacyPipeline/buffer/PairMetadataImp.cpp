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

#define LOG_TAG "MtkCam/PairMetadata"
//
#include "MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
//
#include "PairMetadataImp.h"

using namespace android;
using namespace NSCam;
using namespace NSCam::v1;


#define THREAD_NAME       ("Cam@PairMetadata")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
android::sp<PairMetadata>
PairMetadata::
createInstance( const char* name )
{
    return new PairMetadataImp(name);
}

/******************************************************************************
 *
 ******************************************************************************/
PairMetadataImp::
PairMetadataImp( const char* name )
    : mLogLevel(0)
    , mName(name)
    , mNumberOfWaitingMeta(0)
    , mNumberOfWaitingBuffer(1)
    , mbExitThread(false)
{
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.PairMetadata", 0);
    }
    status_t status = run(THREAD_NAME);
    if  ( OK != status ) {
        MY_LOGE("Fail to run the thread - status:%d(%s)", status, ::strerror(-status));
    }
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PairMetadataImp::
getResultBufferFromProvider(
    MUINT32                       rRequestNo,
    android::sp<IImageStreamInfo> pStreamInfo,
    android::sp<IImageBufferHeap> rpHeap,
    MUINT64                       rTimeStamp,
    MBOOL                         rbErrorBuffer
)
{
    //FUNC_START;
    MY_LOGD1("rRequestNo(%d) streamID(0x%" PRIx64 ") rpHeap(%p)",rRequestNo,pStreamInfo->getStreamId(),rpHeap.get());
    {
        Mutex::Autolock _l(mTempSetLock);

        Vector<ISelector::BufferItemSet> v;
        v.push_back(ISelector::BufferItemSet{pStreamInfo->getStreamId(), rpHeap, pStreamInfo});

        mTempSetMap.push_back(
            ResultSet_T{
                .requestNo  = rRequestNo,
                .bufferSet  = v,
                .resultMeta = Vector<ISelector::MetaItemSet>(),
                .error      = rbErrorBuffer,
                .timestamp  = rTimeStamp
            }
        );
        mTempQueueCond.signal();
    }

    //FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
PairMetadataImp::
onResultReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    MY_LOGD1("rRequestNo(%d) streamID(0x%" PRIx64 ")",requestNo,streamId);
    Mutex::Autolock _l(mTempSetLock);
    Vector<ISelector::MetaItemSet> v;
    v.push_back( ISelector::MetaItemSet{streamId, result});
    mTempSetMap.push_back(
        ResultSet_T{
            .requestNo  = requestNo,
            .bufferSet  = Vector<ISelector::BufferItemSet>(),
            .resultMeta = v,
            .error      = errorResult,
            .timestamp  = 0
        }
    );
    mTempQueueCond.signal();
}

/******************************************************************************
 *
 ******************************************************************************/
void
PairMetadataImp::
onFrameEnd(
    MUINT32         const requestNo
)
{
    Mutex::Autolock _l(mLock);
    mvFrameEnd.push_back(requestNo);
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PairMetadataImp::
setNumberOfMetadata( MINT32 num )
{
    mNumberOfWaitingMeta = num;
    MY_LOGD("mNumberOfWaitingMeta:%zu", mNumberOfWaitingMeta);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PairMetadataImp::
setSelector(
    android::sp< ISelector > pRule
)
{
    Mutex::Autolock _l(mRuleLock);

    if ( mpRule != 0 ) {
        mpRule->flush();
    }

    mpRule = pRule;

    // calculate how many buffers current selector need to wait for.
    Vector<sp<IImageStreamInfo>> infoList;
    mpRule->queryCollectImageStreamInfo(infoList, MTRUE);
    mNumberOfWaitingBuffer = infoList.size();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
String8
PairMetadataImp::
getUserName()
{
    return String8::format("PM:%s", mName);
}

/******************************************************************************
 *
 ******************************************************************************/
void
PairMetadataImp::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;

    struct Log
    {
        static  MVOID
        dump(KeyedVector<MUINT32, ResultSet_T>& rMap, String8 str) {
            for (size_t i = 0; i < rMap.size(); i++) {
                CAM_LOGW("%s [%d] buffer:%s timeStamp:%" PRId64 " error:%d meta:%zu",
                    str.string(),
                    rMap[i].requestNo,
                    ISelector::logBufferSets(rMap[i].bufferSet).string(),
                    rMap[i].timestamp,
                    rMap[i].error,
                    rMap[i].resultMeta.size()
                );
            }
        }
    };

    Log::dump(mResultSetMap , String8::format("[%s] %s" , mName, "mResultSetMap"));

    join();

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
PairMetadataImp::
threadLoop()
{
    Vector<ResultSet_T> vResult;
    MERROR err = dequeResult(vResult);
    if  ( OK == err && ! vResult.isEmpty() )
    {
        handleResult(vResult);
    }
    //
    if(mbExitThread)
    {
        MY_LOGD("exit thread loop");
        return false;
    }
    //
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
PairMetadataImp::
dequeResult(
    Vector<ResultSet_T>& rvResult
)
{
    status_t err = OK;
    //
    Mutex::Autolock _l(mTempSetLock);
    //
    while ( ! exitPending() && mTempSetMap.isEmpty() )
    {
        err = mTempQueueCond.wait(mTempSetLock);
        MY_LOGW_IF(
            OK != err,
            "exitPending:%d ResultQueue#:%zu err:%d(%s)",
            exitPending(), mTempSetMap.size(), err, ::strerror(-err)
        );
    }
    //
    if  ( mTempSetMap.isEmpty() )
    {
        MY_LOGD_IF(
            mLogLevel >= 1,
            "empty queue"
        );
        rvResult.clear();
        err = NOT_ENOUGH_DATA;
    }
    else
    {
        rvResult = mTempSetMap;
        mTempSetMap.clear();
        err = OK;
    }
    //
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PairMetadataImp::
updateResult(
    Vector<ResultSet_T> const& rvResult
)
{
    for ( size_t i = 0; i < rvResult.size(); ++i ) {
        ssize_t const index = mResultSetMap.indexOfKey(rvResult[i].requestNo);
        if ( index < 0 ) {
            mResultSetMap.add(rvResult[i].requestNo, rvResult[i]);
        } else {
            ResultSet_T* result = &mResultSetMap.editValueAt(index);
            result->bufferSet.appendVector(rvResult[i].bufferSet);
            result->resultMeta.appendVector(rvResult[i].resultMeta);
            result->error    &= rvResult[i].error;
            result->timestamp = ( rvResult[i].timestamp != 0 ) ? rvResult[i].timestamp : result->timestamp;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PairMetadataImp::
handleResult(
    Vector<ResultSet_T> const& rvResult
)
{
    updateResult(rvResult);
    handleReturn();
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
PairMetadataImp::
handleReturn()
{
    Vector< MUINT32 > rvFrameEnd;
    {
        Mutex::Autolock _l(mLock);
        if (!mvFrameEnd.isEmpty())
        {
            // check more to avoid timing issue
            Mutex::Autolock _l(mTempSetLock);
            if (!mTempSetMap.isEmpty())
            {
                updateResult(mTempSetMap);
                mTempSetMap.clear();
            }
        }
        rvFrameEnd = mvFrameEnd;
        mvFrameEnd.clear();
    }
    //
    for ( int i = 0; i < (int)(mResultSetMap.size()); ++i ) {
        ResultSet_T* r = &mResultSetMap.editValueAt(i);
        if (r->bufferSet.size() > mNumberOfWaitingBuffer) {
            MY_LOGW("BufferSet size(%zu) > WaitingBufSize(%zu),  buffer:%s request:%d", r->bufferSet.size(),
                        mNumberOfWaitingBuffer,  ISelector::logBufferSets(r->bufferSet).string(), r->requestNo);
        }
        MY_LOGD2("req(%d), BufferSet size(%zu), resultMeta(%zu),  buffer:%s", r->requestNo, r->bufferSet.size(),
                        r->resultMeta.size(),  ISelector::logBufferSets(r->bufferSet).string() );
        if (    r->resultMeta.size() >= mNumberOfWaitingMeta
             && r->bufferSet.size() >= mNumberOfWaitingBuffer)
        {
            if (ISelector::hasEmptyBuffer(r->bufferSet)) {
                MY_LOGD("request(%d) hasEmptyBuffer, it will remove from map and won't sent to selector, buffer: %s",
                            r->requestNo, ISelector::logBufferSets(r->bufferSet).string());
                rvFrameEnd.push_back(r->requestNo);
                continue;
            }
            MY_LOGD1("request(%d) will send to selector, buffer: %s",
                            r->requestNo, ISelector::logBufferSets(r->bufferSet).string());
            Mutex::Autolock _l(mRuleLock);
            if ( mpRule != 0 ) {
                mpRule->selectResult( r->requestNo, r->resultMeta, r->bufferSet, r->error );
                i = mResultSetMap.removeItemsAt(i);
            } else {
                MY_LOGE("No selector. pending buffer:%s request:%d", ISelector::logBufferSets(r->bufferSet).string(), r->requestNo);
            }
        }
    }
    //
    // remove invalid result set.
    for ( size_t j = 0; j < rvFrameEnd.size(); ++j) {
        ssize_t const index = mResultSetMap.indexOfKey(rvFrameEnd[j]);
        if(index >= 0)
        {
            ResultSet_T* r = &mResultSetMap.editValueAt(index);
            Mutex::Autolock _l(mRuleLock);
            MY_LOGD1("request(%d) has FrameEnd and not be handled, it will remove from map and return buffer, meta[%u] buffer[%u] : %s",
                r->requestNo, r->resultMeta.size(), r->bufferSet.size(), ISelector::logBufferSets(r->bufferSet).string());
            if ( mpRule == 0 ) {
                MY_LOGW("No selector to return buffer: %s", ISelector::logBufferSets(r->bufferSet).string());
            } else {
                for (size_t k = 0; k < r->bufferSet.size() ; k++) {
                    if(r->bufferSet[k].heap != NULL)
                        mpRule->returnBuffer(r->bufferSet.editItemAt(k));
                }
            }
            mResultSetMap.removeItem(rvFrameEnd[j]);
        }
    }

    MY_LOGD1("ResultSetMap remain size(%zu)", mResultSetMap.size());

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
PairMetadataImp::
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
status_t
PairMetadataImp::
flush()
{
    MY_LOGD("flush PairMetadataImp");
    bool ret = OK;
    {
        Mutex::Autolock _l(mTempSetLock);
        Thread::requestExit();
        mbExitThread = true;
        mTempQueueCond.signal();
    }
    Thread::join();

    if( mpRule != 0 )
    {
        while ( mResultSetMap.size() > 0 ) {
            ResultSet_T* r = &mResultSetMap.editValueAt(0);
            if ( r->bufferSet.size() != 0 ) {
                ISelector::removeEmptyBuffer(r->bufferSet);// don't let selector receive empty heap
                Mutex::Autolock _l(mRuleLock);
                MY_LOGD("mResultSetMap to selector: [%d] heap:%s", r->requestNo, ISelector::logBufferSets(r->bufferSet).string());
                mpRule->selectResult( r->requestNo, r->resultMeta, r->bufferSet, r->error );
            }
            mResultSetMap.removeItemsAt(0);
        }
        //
        while ( mTempSetMap.size() > 0 ) {
            ResultSet_T* r = &mTempSetMap.editItemAt(0);
            if ( r->bufferSet.size() != 0 ) {
                ISelector::removeEmptyBuffer(r->bufferSet);// don't let selector receive empty heap
                Mutex::Autolock _l(mRuleLock);
                MY_LOGD("mTempSetMap to selector: [%d] heap:%s", r->requestNo, ISelector::logBufferSets(r->bufferSet).string());
                mpRule->selectResult( r->requestNo, r->resultMeta, r->bufferSet, r->error );
            }
            mTempSetMap.removeItemsAt(0);
        }
        //
        MY_LOGD("flush mpRule");
        ret = mpRule->flush();
    }

    return ret;
}
