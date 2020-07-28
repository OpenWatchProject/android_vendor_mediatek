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
#define LOG_TAG "MtkCam/StereoSynchronizer"
//
#include "MyUtils.h"
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Time.h>
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoSelector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferSynchronizer.h>
// STL
#include <map>
#include <list>
#include <set>

#include <random>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;

#define THREAD_NAME       ("Cam@StereoSynchronizer")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s][%s] " fmt, getUserName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_LOGW_NO_USERNAME(fmt, arg...)        CAM_LOGW("[%s][%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGD1(...)               MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF(2<=mLogLevel, __VA_ARGS__)

#define FUNC_START                  MY_LOGD2("+")
#define FUNC_END                    MY_LOGD2("-")

#define AVA_QUE_KEEP_COUNT              1
#define PEN_QUE_KEEP_COUNT              2
#define SYNC_THRESHOLD_MS               1
#define PAIR_THRESHOLD_MS               33
#define SYNC_FAIL_WARNING_COUNT         10
#define SYNC_FAIL_UPPER_LIMIT           30
#define SYNC_FAIL_TOLERANCE_COUNT       12
#define WAIT_CAPTURE_BUF_TIMEOUT_NS     3000000000LL

#define PAIR_SYNC_FAIL_UPPER_LIMIT      6
#define PAIR_SYNC_FAIL_RESET_COUNT      18

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline{

typedef enum _SYNC_RESULT_
{
    SYNC_RESULT_PAIR_OK       = 0,
    SYNC_RESULT_MAIN1_TOO_OLD = 1,
    SYNC_RESULT_MAIN2_TOO_OLD = 2,
    SYNC_RESULT_PAIR_NOT_SYNC = 3,
    SYNC_RESULT_TOLERANCE     = 4
} SYNC_RESULT;

typedef enum _PAIR_RESULT_
{
    PAIR_RESULT_PAIR_OK         = 0,
    PAIR_RESULT_FULL_TOO_OLD    = 1,
    PAIR_RESULT_RESIZED_TOO_OLD = 2,
    PAIR_RESULT_PAIR_INVALID    = 3
}PAIR_RESULT;

class StereoBufferSynchronizerImp
    : public StereoBufferSynchronizer
    , public Thread
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  StereoBufferSynchronizer Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual     MERROR                  addStream(
                                                StreamId_T                      streamId,
                                                wp<StereoSelector>              pSelector
                                            );

    virtual     MERROR                  removeStream(
                                                StreamId_T                      streamId
                                            );

    virtual     MERROR                  enqueBuffer(
                                            MINT32                          reqNo,
                                            StreamId_T                      streamId,
                                            Vector<ISelector::MetaItemSet>  vResultMeta,
                                            sp<IImageBufferHeap>            pHeap
                                        );

    virtual     MERROR                  dequeBuffer(
                                            MINT32&                         rRequestNo,
                                            StreamId_T                      streamId,
                                            Vector<ISelector::MetaItemSet>& rMeta,
                                            sp<IImageBufferHeap>&           rpHeap
                                        );

    virtual     MERROR                  dequeBufferCapture(
                                            MINT32&                         rRequestNo,
                                            StreamId_T                      streamId,
                                            Vector<ISelector::MetaItemSet>& rMeta,
                                            android::sp<IImageBufferHeap>&  rpHeap,
                                            MBOOL                           bNeedLockBuffer
                                        );

    virtual     MERROR                  returnBuffer(
                                            android::sp<IImageBufferHeap>&  rpHeap,
                                            StreamId_T                      streamId
                                        );

    virtual     MERROR                  lock(Vector<StreamId_T>& acquiredBufferSet, Vector<StreamId_T>& rvLockedBufferSet, SYNC_MODE syncMode = NOT_SET);

    virtual     MERROR                  unlock();

    virtual     MERROR                  waitAndLockCapture(MBOOL bNeedClearZSLQueue);

    virtual     MERROR                  unlockCapture();

    virtual     MERROR                  start();

    virtual     MERROR                  flush(MBOOL flushUnSyncQueue = MTRUE);

    virtual     MERROR                  flushCaptureQueue();

    virtual     sp<StereoSelector>      querySelector(StreamId_T streamId);

    virtual     MERROR                  setPreviewBufferUser(sp<IPreviewBufferUser>);

    virtual     void                    setDebugMode(MINT32 debug);

    virtual     void                    setZSLDelayCount(MINT32 delayCount);

    virtual     MERROR                  requestEnable(Vector<StreamId_T> streamsToSyncFromMain, Vector<StreamId_T> streamsToSyncFromAux);

    virtual     MERROR                  requestDisable(Vector<StreamId_T> streamsToUseFromMain, Vector<StreamId_T> streamsToUseFromAux);

    virtual     MERROR                  lockAndDequeBuffer(Vector<StreamId_T>& acquiredBufferSet, Vector<BUFFER_SET>& rvDequedBufferSet);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  RefBase Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                onLastStrongRef(const void* /*id*/);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:    //// thread interface
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    //virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Customized Data Types.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    typedef struct
    {
        Vector<BUFFER_SET>                  syncBufferSet;
        MBOOL                               isAllBufferReturned;
    }ZSL_BUFFER_SET;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Inner Classes
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    class DurationProfile {
public:
                        DurationProfile(char const*const szSubjectName)
                            : msSubjectName(szSubjectName)
                            , mi4Count(0)
                            , mnsStart(0)
                            , mnsEnd(0)
                            , mnsTotal(0)
                            , mLogLevel(0)
                            {}

    virtual            ~DurationProfile(){}

    virtual void        reset(int logLevel) { mi4Count = mnsStart = mnsEnd = mnsTotal = 0; mLogLevel = logLevel;}

    virtual void        pulse_up() {mnsStart = ::systemTime();}
    virtual void        pulse_down()
                        {
                            mnsEnd = ::systemTime();

                            MY_LOGD2("pulse_down:%lld / %lld", mnsStart, mnsEnd);

                            if (mnsStart != 0)
                            {
                                nsecs_t duration = mnsEnd - mnsStart;
                                mnsTotal += duration;
                                mi4Count++;
                                mnsStart = 0;
                            }

                            if(mi4Count != 0){
                                // MY_LOGD2("AvgDuration:%ld", ::ns2ms(mnsTotal)/mi4Count);
                                MY_LOGD2("AvgDuration:%f(us)", (((float)mnsTotal)/((float)mi4Count))/1000LL);
                            }
                            if(mnsTotal != 0){
                                MY_LOGD2("AvgFPS:%f", ((float)mi4Count/mnsTotal)*1000000000LL);
                            }
                        }

    char const*         getUserName() {return msSubjectName;};
protected:
    char const*         msSubjectName;
    int32_t             mi4Count;
    nsecs_t             mnsStart;
    nsecs_t             mnsEnd;
    nsecs_t             mnsTotal;
    int                 mLogLevel;
};
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Implementations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        StereoBufferSynchronizerImp(char const* szCallerName, MINT32 delayCount);

                                        ~StereoBufferSynchronizerImp();

    char const*                         getUserName() const {return mUserName.c_str();};
private:
    MBOOL                               isReadyToSyncCheck();

    MBOOL                               isReadyToLock(Vector<StreamId_T>& acquiredBufferSet, Vector<StreamId_T>& rvLockedBufferSet, SYNC_MODE syncMode);

    MBOOL                               isZSLBufferReady();

    void                                syncRoutine(
                                            BUFFER_SET* bufInfoResized,
                                            BUFFER_SET* bufInfoResized_main2,
                                            BUFFER_SET* bufInfoFull,
                                            BUFFER_SET* bufInfoFull_main2,
                                            BUFFER_SET* bufInfoLcso,
                                            BUFFER_SET* bufInfoLcso_main2
                                        );

    SYNC_RESULT                         isTimeSync(
                                            BUFFER_SET* bufInfoMain1,
                                            BUFFER_SET* bufInfoMain2
                                        );

    void                                returnOldestBufferToSelector(StreamId_T streamId, map< StreamId_T, list<BUFFER_SET> >& theMap);

    void                                checkPendingQueueSize(StreamId_T streamId);

    void                                checkAllPendingQueueSize();

    void                                checkAvailableQueueSize(StreamId_T streamId);

    void                                checkAllAvailableQueueSize();

    void                                checkZSLAvailableQueueSize(MINT32 keepSize);

    void                                keepZSLOldestBuffer();

    void                                clearQueAndReturnBufferToSelector(MBOOL flushUnSyncQueue);

    void                                showPreviewPendingSizes();

    void                                showPreviewAvailableSizes();

    void                                showPreviewUsingSizes();

    void                                showCaptureAvailableSizes();

    void                                showCaptureUsingSizes();

    void                                returnBufferToSelector(BUFFER_SET* bufInfo);

    void                                returnBufferToSelector(StreamId_T streamId, android::sp<IImageBufferHeap>& theHeap);

    MBOOL                               pushToUnSyncBufferQueue(
                                            MINT32                              rRequestNo,
                                            StreamId_T                          streamId,
                                            Vector<ISelector::MetaItemSet>      vMeta,
                                            android::sp<IImageBufferHeap>       pHeap
                                        );

    void                                returnBufferToPendingQue(BUFFER_SET* bufInfo);

    void                                enqueBufferToAvailableQue(BUFFER_SET* bufInfo, MINT32 assignedReqNo);

    MBOOL                               pairCheck(BUFFER_SET* bufInfoResized, BUFFER_SET* bufInfoFull, BUFFER_SET* bufInfoLSCO);

    PAIR_RESULT                         isPairSync(BUFFER_SET* bufInfoResized, BUFFER_SET* bufInfoFull);

    MBOOL                               isInQue(list<BUFFER_SET>& theQue, android::sp<IImageBufferHeap>& theHeap);

    BUFFER_SET                          removeFromQueue(list<BUFFER_SET>& theQue, android::sp<IImageBufferHeap>& theHeap);

    MBOOL                               isCaptureBufferReady();

    MBOOL                               isPreviewUsingQueEmpty();

    MBOOL                               isCaptureUsingQueEmpty();

    MBOOL                               isAFStable(
                                            BUFFER_SET* bufInfoMain1,
                                            BUFFER_SET* bufInfoMain2
                                        );

    void                                markAFState(BUFFER_SET* bufInfo, MBOOL isAFStable);

    void                                markTolerance(BUFFER_SET* bufInfo, SYNC_RESULT sync_result);

    void                                flushCaptureAvailableQueue();

    MBOOL                               isInVector(StreamId_T streamId, Vector<StreamId_T>& rv);

    MBOOL                               isInVector(StreamId_T streamId, Vector<BUFFER_SET>& rv);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data members.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    const string                        mUserName;
    int                                 mLogLevel                   = 0;
    MINT32                              mDebugMode                  = 0;
    MINT32                              mZSLDelayCount              = 3;
    MBOOL                               mbIsPreviewStopped          = MFALSE;
    MBOOL                               mbExitThread                = MFALSE;
    MBOOL                               mIsLocked                   = MFALSE;
    MBOOL                               mIsCaptureLocked            = MFALSE;
    MBOOL                               mIsAvailableQueStopped      = MFALSE;
    MBOOL                               mIsAFTimeout                = MFALSE;

    mutable Mutex                       mLock;
    mutable Mutex                       mPreviewPendingQueLock;
    mutable Mutex                       mPreviewAvailableQueLock;
    mutable Mutex                       mPreviewUsingQueLock;

    mutable Mutex                       mCaptureAvailableQueLock;
    mutable Mutex                       mCaptureUsingQueLock;

    mutable Condition                   mCondPushPreviewPending;
    mutable Condition                   mCondPushPreviewAvailable;
    mutable Condition                   mCondLockPreviewAvailable;
    mutable Condition                   mCondPushCaptureAvailable;
    mutable Condition                   mCondPreviewUsingReturned;
    mutable Condition                   mCondCaptureUsingReturned;
    mutable Condition                   mCondCaptureUnlock;

    map< StreamId_T, list<BUFFER_SET> > streamToPreviewPendingQue;
    map< StreamId_T, list<BUFFER_SET> > streamToPreviewAvailableQue;
    map< StreamId_T, list<BUFFER_SET> > streamToPreviewUsingQue;
    map< StreamId_T, list<BUFFER_SET> > streamToCaptureAvailableQue;
    map< StreamId_T, list<BUFFER_SET> > streamToCaptureUsingQue;

    typedef pair<StreamId_T, list<BUFFER_SET> > PairStreamToBufferQue;

    map< StreamId_T, wp<StereoSelector> > streamToSelector;
    typedef pair< StreamId_T, wp<StereoSelector> > PairStreamToSelector;

    int                                 failedCounter               = 0;
    int                                 pairedFailedCounter         = 0;
    MINT32                              mTimestamp                  = -1;

    DurationProfile                     mSyncThreadProfile;
    DurationProfile                     mEnqueThreadProfile;

    wp<IPreviewBufferUser>              mwpPreviewBufferUser          = nullptr;

    std::default_random_engine          mGenerator;
    std::uniform_int_distribution<int>  mDistribution;
};

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW_NO_USERNAME("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<StereoBufferSynchronizer>
StereoBufferSynchronizer::
createInstance(
    char const* szCallerName, MINT32 delayCount
)
{
    return new StereoBufferSynchronizerImp( szCallerName, delayCount);
}
/******************************************************************************
 *
 ******************************************************************************/
StereoBufferSynchronizerImp::
StereoBufferSynchronizerImp(char const* szCallerName, MINT32 delayCount)
    : mSyncThreadProfile("synchronizerProfile::sync")
    , mEnqueThreadProfile("synchronizerProfile::enque")
    , mZSLDelayCount(delayCount)
    , mDistribution(std::uniform_int_distribution<int>(0,9))
    , mUserName(szCallerName)
{
    MY_LOGD("StereoBufferSynchronizerImp ctor");

    // get log level
    mLogLevel = property_get_int32("vendor.STEREO.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = property_get_int32("vendor.STEREO.log.stereosync", 0);
    }

    // get debug mode
    switch(property_get_int32("vendor.STEREO.stereosync.dbg", 0)){
        case 1:
            setDebugMode(DEBUG_MODE::SKIP_TIMESTAMP_CHECK);
            break;
        case 2:
            setDebugMode(DEBUG_MODE::SKIP_METADATA_CHECK);
            break;
        default:
            break;
    }

    // get debug ZSL count
    mZSLDelayCount = property_get_int32("vendor.stereosync.zsl", delayCount);

    MY_LOGD("PAIR_THRESHOLD_MS:%d SYNC_THRESHOLD_MS:%d PEN_QUE_KEEP_COUNT:%d AVA_QUE_KEEP_COUNT:%d LogLevel:%d debugMode:%d ZSLDelayCount:%d",
        PAIR_THRESHOLD_MS,
        SYNC_THRESHOLD_MS,
        PEN_QUE_KEEP_COUNT,
        AVA_QUE_KEEP_COUNT,
        mLogLevel,
        mDebugMode,
        mZSLDelayCount
    );

    mSyncThreadProfile.reset(mLogLevel);
    mEnqueThreadProfile.reset(mLogLevel);
}

/******************************************************************************
 *
 ******************************************************************************/
StereoBufferSynchronizerImp::
~StereoBufferSynchronizerImp()
{
    MY_LOGD("~(%x)", this);
}

/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;
    flush();
    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
addStream(StreamId_T  streamId, wp<StereoSelector>  pSelector)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);

    if(streamToPreviewPendingQue.count(streamId)){
        MY_LOGW("add a same stream(%#" PRIx64 ")twice, ignored!", streamId);
        return OK;
    }

    list<BUFFER_SET>  thePreviewPendingQue;
    list<BUFFER_SET>  thePreviewAvailableQue;
    list<BUFFER_SET>  thePreviewUsingQue;

    list<BUFFER_SET>  theCaptureAvailableQue;
    list<BUFFER_SET>  theCaptureUsingQue;

    streamToPreviewPendingQue.insert(PairStreamToBufferQue(streamId, thePreviewPendingQue));
    streamToPreviewAvailableQue.insert(PairStreamToBufferQue(streamId, thePreviewAvailableQue));
    streamToPreviewUsingQue.insert(PairStreamToBufferQue(streamId, thePreviewUsingQue));

    streamToCaptureAvailableQue.insert(PairStreamToBufferQue(streamId, theCaptureAvailableQue));
    streamToCaptureUsingQue.insert(PairStreamToBufferQue(streamId, theCaptureUsingQue));

    streamToSelector.insert(PairStreamToSelector(streamId, pSelector));

    sp<StereoSelector> spSelector = pSelector.promote();
    if (spSelector == 0){
        MY_LOGE("Cannot promote spSelector!");
    }else{
        spSelector->setSynchronizer(this, streamId);
    }

    MY_LOGD("new stream %#" PRIx64 " added", streamId);

    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
removeStream(StreamId_T  streamId)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);

    streamToPreviewPendingQue.erase(streamId);
    streamToPreviewAvailableQue.erase(streamId);
    streamToPreviewUsingQue.erase(streamId);

    streamToCaptureAvailableQue.erase(streamId);
    streamToCaptureUsingQue.erase(streamId);

    streamToSelector.erase(streamId);

    MY_LOGD("stream %#" PRIx64 " removed", streamId);

    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
dequeBuffer(
                MINT32&                             rRequestNo,
                StreamId_T                          streamId,
                Vector<ISelector::MetaItemSet>&     rMeta,
                android::sp<IImageBufferHeap>&      rpHeap
            )
{
    MY_LOGD1("dequeBuffer %#" PRIx64 " + ", streamId);

    // find the preview available queue
    if(streamToPreviewAvailableQue.count(streamId) < 0){
        MY_LOGE("undefined streamId for previewAvailableQueue, should not have happended!");
        return NAME_NOT_FOUND;
    }

    list<BUFFER_SET>& theQue = streamToPreviewAvailableQue[streamId];

    // deque from preview available queue
    BUFFER_SET dequedSet;
    {
        Mutex::Autolock _l(mPreviewAvailableQueLock);
        if(theQue.empty()){
            MY_LOGE("try to deque but the size is 0, should not have happended! Did you call synchronizer->lock() before doing this?");
            return UNKNOWN_ERROR;
        }
        dequedSet = theQue.front();
        theQue.pop_front();
        rRequestNo = dequedSet.reqNo;
        rMeta = dequedSet.metadata;
        rpHeap = dequedSet.heap;
        dequedSet.isReturned = MFALSE;
    }

    // push to preview using queue
    {
        Mutex::Autolock _l(mPreviewUsingQueLock);
        list<BUFFER_SET>& thePreviewUsingQue = streamToPreviewUsingQue[streamId];
        thePreviewUsingQue.push_back(dequedSet);

        showPreviewUsingSizes();
    }

    MY_LOGD1("dequeBuffer %#" PRIx64 " - , rpHeap=%p, reqNo:%d", streamId, rpHeap.get(), rRequestNo);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
dequeBufferCapture(
                MINT32&                         rRequestNo,
                StreamId_T                      streamId,
                Vector<ISelector::MetaItemSet>& rMeta,
                android::sp<IImageBufferHeap>&  rpHeap,
                MBOOL                           bNeedLockBuffer
               )
{
    MY_LOGD1("dequeBufferCapture %#" PRIx64 " + ", streamId);

    // find the capture available queue
    if(streamToCaptureAvailableQue.count(streamId) < 0){
        MY_LOGE("undefined streamId, should not have happended!");
        return NAME_NOT_FOUND;
    }

    list<BUFFER_SET>& theCaptureAvailableQue = streamToCaptureAvailableQue[streamId];

    if(bNeedLockBuffer)
    {
        waitAndLockCapture(MFALSE);
    }

    // deque from capture available queue
    BUFFER_SET dequedSet;
    {
        Mutex::Autolock _l(mCaptureAvailableQueLock);
        if(theCaptureAvailableQue.empty()){
            MY_LOGE("try to deque but the size is 0, should not have happended! Did you call synchronizer->waitAndLockCapture() before doing this?");
            return UNKNOWN_ERROR;
        }
        dequedSet = theCaptureAvailableQue.front();
        theCaptureAvailableQue.pop_front();
        rRequestNo = dequedSet.reqNo;
        rMeta = dequedSet.metadata;
        rpHeap = dequedSet.heap;
        dequedSet.isReturned = MFALSE;
    }

    // push to capture using queue
    if(!bNeedLockBuffer)
    {
        Mutex::Autolock _l(mCaptureUsingQueLock);
        list<BUFFER_SET>& theCaptureUsingQue = streamToCaptureUsingQue[streamId];
        theCaptureUsingQue.push_back(dequedSet);

        showCaptureUsingSizes();
    }

    if(bNeedLockBuffer)
    {
        unlockCapture();
    }

    MY_LOGD1("dequeBufferCapture %#" PRIx64 " - , rpHeap=%p, reqNo:%d", streamId, rpHeap.get(), rRequestNo);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
checkZSLAvailableQueueSize(MINT32 keepSize){
    MY_LOGE("not implemented");
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
keepZSLOldestBuffer(){
    FUNC_START;

    for(auto &e : streamToCaptureAvailableQue){
        while(e.second.size() > 1){
            MY_LOGD1("streamId(%llx) capture available queue is %d > 1. return buffer",
                e.first,
                e.second.size()
            );

            BUFFER_SET dequedSet = e.second.back();
            e.second.pop_back();
            returnBufferToSelector(&dequedSet);
        }
    }

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
returnBuffer(
                android::sp<IImageBufferHeap>&  rpHeap,
                StreamId_T                      streamId
            )
{
    MY_LOGD1("returnBuffer %#" PRIx64  ", buf:%p + ", streamId, rpHeap.get());
    BUFFER_SET dequedSet;
    // check what kind of this buffer is
    {
        // if this is a preview using buffer, move it to capture available queue
        MBOOL moveToCaptureQue = MFALSE;
        {
            Mutex::Autolock _l(mPreviewUsingQueLock);
            list<BUFFER_SET>& thePreviewUsingQue = streamToPreviewUsingQue[streamId];
            if(isInQue(thePreviewUsingQue, rpHeap)){
                dequedSet = removeFromQueue(thePreviewUsingQue, rpHeap);
                // check this buffer_set's af state, if not stable, dont push to capture queue
                if(dequedSet.isAFStable && !dequedSet.isTolerance && !dequedSet.isPairCheckFailed){
                    moveToCaptureQue = MTRUE;
                }else{
                    MY_LOGD1("buffer not suitable for capture AF(%d) Tolerance(%d) pairFailed(%d), dont push to capture queue",
                        dequedSet.isAFStable, dequedSet.isTolerance, dequedSet.isPairCheckFailed);
                    returnBufferToSelector(streamId, dequedSet.heap);
                    mCondPreviewUsingReturned.signal();
                    return OK;
                }
                mCondPreviewUsingReturned.signal();
            }
        }

        // preview used and AF stable, push to capture queue
        if(moveToCaptureQue){
            Mutex::Autolock _l(mCaptureAvailableQueLock);

            if(mIsCaptureLocked){
                returnBufferToSelector(streamId, dequedSet.heap);
            }else{
                list<BUFFER_SET>& theCaptureAvailableQue = streamToCaptureAvailableQue[streamId];
                theCaptureAvailableQue.push_back(dequedSet);

                //check capture available queue size
                if(theCaptureAvailableQue.size() > mZSLDelayCount){
                    MY_LOGD2("theCaptureAvailableQue->size():%d > mZSLDelayCount:%d. return oldest buffer to selector",
                        theCaptureAvailableQue.size(),
                        mZSLDelayCount
                    );
                    returnOldestBufferToSelector(streamId, streamToCaptureAvailableQue);
                }

                showCaptureAvailableSizes();

                mCondPushCaptureAvailable.signal();
            }
            return OK;
        }
    }
    {
        // if this is a capture using buffer, return it to selector immediately
        Mutex::Autolock _l(mCaptureUsingQueLock);
        list<BUFFER_SET>& theCaptureUsingQue = streamToCaptureUsingQue[streamId];
        if(isInQue(theCaptureUsingQue, rpHeap)){
            dequedSet = removeFromQueue(theCaptureUsingQue, rpHeap);
            returnBufferToSelector(streamId, rpHeap);
            mCondCaptureUsingReturned.signal();
            return OK;
        }
    }
    MY_LOGE("cant find this buffer:%p in any of the queue, should not have happended!", rpHeap.get());
    return UNKNOWN_ERROR;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
enqueBuffer(
                MINT32                              rRequestNo,
                StreamId_T                          streamId,
                Vector<ISelector::MetaItemSet>      vMeta,
                android::sp<IImageBufferHeap>       pHeap
            )
{
    MY_LOGD1("req:%d, heap:%p, meta:%d stream:%#" PRIx64 " , is enqued to StereoSynchronizer ",
        rRequestNo,
        pHeap.get(),
        vMeta.size(),
        streamId
    );

    Mutex::Autolock _l(mPreviewPendingQueLock);

    // There are some conditions that buffers are not pushed to previewPendingQueue
    if(mbIsPreviewStopped){
        MY_LOGD("preview stopped, return buffer directly");
        returnBufferToSelector(streamId, pHeap);
        return OK;
    }

    // Otherwise, push to previewPendingQueue and wait for syncRoutine
    if(streamToPreviewPendingQue.count(streamId)<0){
        MY_LOGE("undefined streamId, should not have happended!");
        return NAME_NOT_FOUND;
    }

    list<BUFFER_SET>& thePendingQue = streamToPreviewPendingQue[streamId];

    // Push new buffer into pendingQueue
    mEnqueThreadProfile.pulse_up();

    BUFFER_SET newBuffer;
    newBuffer.reqNo = rRequestNo;
    newBuffer.heap = pHeap;
    newBuffer.metadata = vMeta;
    newBuffer.streamId = streamId;
    newBuffer.isReturned = MFALSE;
    thePendingQue.push_back(newBuffer);

    mCondPushPreviewPending.signal();

    mEnqueThreadProfile.pulse_down();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isReadyToSyncCheck()
{
    MBOOL bIsReady = MTRUE;

    // make sure there is no too much buffer pending here
    checkAllPendingQueueSize();

    showPreviewPendingSizes();

    // check if there is anyone empty
    for(map<StreamId_T, list<BUFFER_SET> >::iterator it=streamToPreviewPendingQue.begin(); it!=streamToPreviewPendingQue.end(); ++it){
        if(it->second.empty()){
         MY_LOGD1("streamId(%llx) pending Queue is empty, buffer not ready", it->first);
         return MFALSE;
        }
    }

    return bIsReady;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
checkPendingQueueSize(StreamId_T streamId)
{
    FUNC_START;

    // get front buffer from pending queue
    if(streamToPreviewPendingQue.count(streamId)<0){
        FUNC_END;
        return;
    }

    list<BUFFER_SET>& thePendingQue = streamToPreviewPendingQue[streamId];

    while(thePendingQue.size() > PEN_QUE_KEEP_COUNT){
        MY_LOGW("streamId%#" PRIx64 " thePendingQue.size=%d > PEN_QUE_KEEP_COUNT=%d return 1 buffer to selector",
            streamId,
            thePendingQue.size(),
            PEN_QUE_KEEP_COUNT
        );
        returnOldestBufferToSelector(streamId, streamToPreviewPendingQue);
    }

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
checkAllPendingQueueSize(){
    for(map<StreamId_T, list<BUFFER_SET>>::iterator it=streamToPreviewPendingQue.begin(); it!=streamToPreviewPendingQue.end(); ++it){
        checkPendingQueueSize(it->first);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
checkAvailableQueueSize(StreamId_T streamId)
{
    FUNC_START;

    // get front buffer from pending queue
    if(streamToPreviewAvailableQue.count(streamId)<0){
        FUNC_END;
        return;
    }

    list<BUFFER_SET>& theAvailableQue = streamToPreviewAvailableQue[streamId];

    while(theAvailableQue.size() > AVA_QUE_KEEP_COUNT){
        MY_LOGW("theAvailableQue.size=%d > AVA_QUE_KEEP_COUNT=%d return 1 buffer to selector",
            theAvailableQue.size(),
            AVA_QUE_KEEP_COUNT
        );
        returnOldestBufferToSelector(streamId, streamToPreviewAvailableQue);
    }

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
checkAllAvailableQueueSize(){
    for(map<StreamId_T, list<BUFFER_SET>>::iterator it=streamToPreviewAvailableQue.begin(); it!=streamToPreviewAvailableQue.end(); ++it){
        checkAvailableQueueSize(it->first);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isPreviewUsingQueEmpty(){
    for(auto &e : streamToPreviewUsingQue ){
        if(!e.second.empty()){
            return MFALSE;
        }
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isCaptureUsingQueEmpty(){
    for(auto &e : streamToCaptureUsingQue ){
        if(!e.second.empty()){
            return MFALSE;
        }
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
returnOldestBufferToSelector(StreamId_T streamId, map< StreamId_T, list<BUFFER_SET> >& theMap){
    FUNC_START;

    // get front buffer from pending queue
    if(theMap.count(streamId)<0){
        return;
    }

    list<BUFFER_SET>& theQue = theMap[streamId];

    BUFFER_SET dequedSet = theQue.front();
    theQue.pop_front();

    // return to selector
    returnBufferToSelector(streamId, dequedSet.heap);

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
returnBufferToSelector(BUFFER_SET* bufInfo){
    if(bufInfo == NULL){
        return;
    }

    FUNC_START;

    // return to selector
    wp<StereoSelector> wpSelector = streamToSelector[bufInfo->streamId];

    sp<StereoSelector> spSelector = wpSelector.promote();
    if (spSelector == 0){
        MY_LOGE("Cannot promote wpSelector!");
    }else{
        ISelector::BufferItemSet set{bufInfo->streamId, bufInfo->heap, NULL};
        spSelector->returnBuffer(set);
    }

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
returnBufferToSelector(StreamId_T streamId, android::sp<IImageBufferHeap>& theHeap){
    FUNC_START;

    wp<StereoSelector> wpSelector = streamToSelector[streamId];

    sp<StereoSelector> spSelector = wpSelector.promote();
    if (spSelector == 0){
        MY_LOGE("Cannot promote wpSelector!");
    }else{
        ISelector::BufferItemSet set{streamId, theHeap, NULL};
        spSelector->returnBuffer(set);
    }

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
pushToUnSyncBufferQueue(MINT32 rRequestNo, StreamId_T streamId, Vector<ISelector::MetaItemSet> vMeta, android::sp<IImageBufferHeap> pHeap)
{
    MY_LOGE("deprecated");
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
returnBufferToPendingQue(BUFFER_SET* bufInfo){
    FUNC_START;
    if(bufInfo == NULL){
        return;
    }

    if(streamToPreviewPendingQue.count(bufInfo->streamId)){
        list<BUFFER_SET>& thePendingQue = streamToPreviewPendingQue[bufInfo->streamId];
        Mutex::Autolock _l(mPreviewPendingQueLock);
        thePendingQue.push_front((*bufInfo));
    }

    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
clearQueAndReturnBufferToSelector(MBOOL flushUnSyncQueue){
    FUNC_START;

    //clear preview pendingQue
    {
        Mutex::Autolock _l(mPreviewPendingQueLock);
        MY_LOGD("clear preview pendingQue");
        for(map<StreamId_T, list<BUFFER_SET>>::iterator it=streamToPreviewPendingQue.begin(); it!=streamToPreviewPendingQue.end(); ++it){
            StreamId_T theStreamId = it->first;
            list<BUFFER_SET>& theQue = it->second;

            // return to selector
            while(!theQue.empty()){
                returnOldestBufferToSelector(theStreamId, streamToPreviewPendingQue);
            }
        }
    }

    //clear preview availableQue
    {
        Mutex::Autolock _l(mPreviewAvailableQueLock);
        MY_LOGD("clear preview availableQue");
        for(map<StreamId_T, list<BUFFER_SET>>::iterator it=streamToPreviewAvailableQue.begin(); it!=streamToPreviewAvailableQue.end(); ++it){
            StreamId_T theStreamId = it->first;
            list<BUFFER_SET>& theQue = it->second;

            // return to selector
            while(!theQue.empty()){
                returnOldestBufferToSelector(theStreamId, streamToPreviewAvailableQue);
            }
        }
    }

    //clear capture availableQue
    {
        Mutex::Autolock _l(mCaptureAvailableQueLock);
        MY_LOGD("clear capture availableQue");
        for(map<StreamId_T, list<BUFFER_SET>>::iterator it=streamToCaptureAvailableQue.begin(); it!=streamToCaptureAvailableQue.end(); ++it){
            StreamId_T theStreamId = it->first;
            list<BUFFER_SET>& theQue = it->second;

            // return to selector
            while(!theQue.empty()){
                returnOldestBufferToSelector(theStreamId, streamToCaptureAvailableQue);
            }
        }
    }

    //wait until preview/capture usingqueue is empty
    {
        Mutex::Autolock _l(mPreviewUsingQueLock);
        while(!isPreviewUsingQueEmpty()){
            MY_LOGD("preview using not empty, wait +");
            mCondPreviewUsingReturned.wait(mPreviewUsingQueLock);
            MY_LOGD("preview using not empty, wait -");
        }
        MY_LOGD("preview using is empty now");
    }

    {
        Mutex::Autolock _l(mCaptureUsingQueLock);
        while(!isCaptureUsingQueEmpty()){
            MY_LOGD("capture using not empty, wait +");
            mCondCaptureUsingReturned.wait(mCaptureUsingQueLock);
            MY_LOGD("capture using not empty, wait -");
        }
        MY_LOGD("capture using is empty now");
    }
    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
enqueBufferToAvailableQue(BUFFER_SET* bufInfo, MINT32 assignedReqNo){
    if(bufInfo == NULL){
        return;
    }
    FUNC_START;

    bufInfo->reqNo = assignedReqNo;

    if(streamToPreviewPendingQue.count(bufInfo->streamId)){
        list<BUFFER_SET>& theAvailableQue = streamToPreviewAvailableQue[bufInfo->streamId];
        theAvailableQue.push_back((*bufInfo));
    }else{
        MY_LOGE("pendingQue exist but availableQue not exist? should not have happended");
    }
    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
showPreviewPendingSizes()
{
    MY_LOGD2("\n(rez/ful/lcso/rez_2/ful_2/lcso_2)\npendingQueue:  (%d/%d/%d/%d/%d/%d)",
        (streamToPreviewPendingQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)         > 0 ? streamToPreviewPendingQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER].size()     : -1),
        (streamToPreviewPendingQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)          > 0 ? streamToPreviewPendingQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE].size()      : -1),
        (streamToPreviewPendingQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO)            > 0 ? streamToPreviewPendingQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO].size()        : -1),
        (streamToPreviewPendingQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01)      > 0 ? streamToPreviewPendingQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01].size()  : -1),
        (streamToPreviewPendingQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01)       > 0 ? streamToPreviewPendingQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01].size()   : -1),
        (streamToPreviewPendingQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01)         > 0 ? streamToPreviewPendingQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO_01].size()     : -1)
    );
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
showPreviewAvailableSizes()
{
    MY_LOGD2("\n(rez/ful/lcso/rez_2/ful_2/lcso_2)\navailableQueue:  (%d/%d/%d/%d/%d/%d)",
        (streamToPreviewAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)         > 0 ? streamToPreviewAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER].size()     : -1),
        (streamToPreviewAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)          > 0 ? streamToPreviewAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE].size()      : -1),
        (streamToPreviewAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO)            > 0 ? streamToPreviewAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO].size()        : -1),
        (streamToPreviewAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01)      > 0 ? streamToPreviewAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01].size()  : -1),
        (streamToPreviewAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01)       > 0 ? streamToPreviewAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01].size()   : -1),
        (streamToPreviewAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01)         > 0 ? streamToPreviewAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO_01].size()     : -1)
    );
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
showPreviewUsingSizes()
{
    MY_LOGD2("\n(rez/ful/lcso/rez_2/ful_2/lcso_2)\npreviewUsing:  (%d/%d/%d/%d/%d/%d)",
        (streamToPreviewUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)         > 0 ? streamToPreviewUsingQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER].size()     : -1),
        (streamToPreviewUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)          > 0 ? streamToPreviewUsingQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE].size()      : -1),
        (streamToPreviewUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO)            > 0 ? streamToPreviewUsingQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO].size()        : -1),
        (streamToPreviewUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01)      > 0 ? streamToPreviewUsingQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01].size()  : -1),
        (streamToPreviewUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01)       > 0 ? streamToPreviewUsingQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01].size()   : -1),
        (streamToPreviewUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01)         > 0 ? streamToPreviewUsingQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO_01].size()     : -1)
    );
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
showCaptureAvailableSizes()
{
    MY_LOGD2("\n(rez/ful/lcso/rez_2/ful_2/lcso_2)\ncaptureAvailable:  (%d/%d/%d/%d/%d/%d)",
        (streamToCaptureAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)         > 0 ? streamToCaptureAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER].size()     : -1),
        (streamToCaptureAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)          > 0 ? streamToCaptureAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE].size()      : -1),
        (streamToCaptureAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO)            > 0 ? streamToCaptureAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO].size()        : -1),
        (streamToCaptureAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01)      > 0 ? streamToCaptureAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01].size()  : -1),
        (streamToCaptureAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01)       > 0 ? streamToCaptureAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01].size()   : -1),
        (streamToCaptureAvailableQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01)         > 0 ? streamToCaptureAvailableQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO_01].size()     : -1)
    );
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
showCaptureUsingSizes()
{
    MY_LOGD2("\n(rez/ful/lcso/rez_2/ful_2/lcso_2)\ncaptureUsing:  (%d/%d/%d/%d/%d/%d)",
        (streamToCaptureUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)         > 0 ? streamToCaptureUsingQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER].size()     : -1),
        (streamToCaptureUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)          > 0 ? streamToCaptureUsingQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE].size()      : -1),
        (streamToCaptureUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO)            > 0 ? streamToCaptureUsingQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO].size()        : -1),
        (streamToCaptureUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01)      > 0 ? streamToCaptureUsingQue[eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01].size()  : -1),
        (streamToCaptureUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01)       > 0 ? streamToCaptureUsingQue[eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01].size()   : -1),
        (streamToCaptureUsingQue.count(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01)         > 0 ? streamToCaptureUsingQue[eSTREAMID_IMAGE_PIPE_RAW_LCSO_01].size()     : -1)
    );
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
syncRoutine(
    BUFFER_SET* bufInfoResized,
    BUFFER_SET* bufInfoResized_main2,
    BUFFER_SET* bufInfoFull,
    BUFFER_SET* bufInfoFull_main2,
    BUFFER_SET* bufInfoLcso,
    BUFFER_SET* bufInfoLcso_main2
){
    FUNC_START;

    SYNC_RESULT sync_result = SYNC_RESULT_PAIR_NOT_SYNC;
    MBOOL AFStable = MFALSE;

    // chose a suitable to-be-compared set
    if(bufInfoResized && bufInfoResized_main2){
        sync_result = isTimeSync(bufInfoResized, bufInfoResized_main2);
        AFStable = isAFStable(bufInfoResized, bufInfoResized_main2);
    }else
    if(bufInfoResized && bufInfoFull_main2){
        sync_result = isTimeSync(bufInfoResized, bufInfoFull_main2);
        AFStable = isAFStable(bufInfoResized, bufInfoFull_main2);
    }else
    if(bufInfoFull && bufInfoResized_main2){
        sync_result = isTimeSync(bufInfoFull, bufInfoResized_main2);
        AFStable = isAFStable(bufInfoFull, bufInfoResized_main2);
    }else
    if(bufInfoFull && bufInfoFull_main2){
        sync_result = isTimeSync(bufInfoFull, bufInfoFull_main2);
        AFStable = isAFStable(bufInfoFull, bufInfoFull_main2);
    }
    else{
        MY_LOGE("Can not find a suitable to-be-compared set! ");
        sync_result = SYNC_RESULT_PAIR_NOT_SYNC;
    }

    // mark all the BUFFER_SET with AF state
    markAFState(bufInfoResized,         AFStable);
    markAFState(bufInfoResized_main2,   AFStable);
    markAFState(bufInfoFull,            AFStable);
    markAFState(bufInfoFull_main2,      AFStable);
    markAFState(bufInfoLcso,            AFStable);
    markAFState(bufInfoLcso_main2,      AFStable);

    // Flush capture queue if AF not stable
    MY_LOGD1("AFStable:%d", AFStable);
    {
        Mutex::Autolock _l(mCaptureAvailableQueLock);
        if(!AFStable && !mIsAFTimeout && !mIsCaptureLocked){
            flushCaptureAvailableQueue();
        }
    }

    // mark all the BUFFER_SET whether it is tolerance buffer
    markTolerance(bufInfoResized,         sync_result);
    markTolerance(bufInfoResized_main2,   sync_result);
    markTolerance(bufInfoFull,            sync_result);
    markTolerance(bufInfoFull_main2,      sync_result);
    markTolerance(bufInfoLcso,            sync_result);
    markTolerance(bufInfoLcso_main2,      sync_result);

    switch(sync_result){
        case SYNC_RESULT_MAIN1_TOO_OLD:
            // Main1 too old
            // return main1 buffer to selector
            returnBufferToSelector(bufInfoResized);
            returnBufferToSelector(bufInfoFull);
            returnBufferToSelector(bufInfoLcso);
            // return main2 buffer back to pending queue
            returnBufferToPendingQue(bufInfoResized_main2);
            returnBufferToPendingQue(bufInfoFull_main2);
            returnBufferToPendingQue(bufInfoLcso_main2);
            break;
        case SYNC_RESULT_MAIN2_TOO_OLD:
            // Main2 too old
            // return main2 buffer to selector
            returnBufferToSelector(bufInfoResized_main2);
            returnBufferToSelector(bufInfoFull_main2);
            returnBufferToSelector(bufInfoLcso_main2);
            // return main1 buffer back to pending queue
            returnBufferToPendingQue(bufInfoResized);
            returnBufferToPendingQue(bufInfoFull);
            returnBufferToPendingQue(bufInfoLcso);
            break;
        case SYNC_RESULT_PAIR_NOT_SYNC:
            // This buffet pair is not sync.
            // return both to selector
            returnBufferToSelector(bufInfoResized);
            returnBufferToSelector(bufInfoFull);
            returnBufferToSelector(bufInfoLcso);
            returnBufferToSelector(bufInfoResized_main2);
            returnBufferToSelector(bufInfoFull_main2);
            returnBufferToSelector(bufInfoLcso_main2);
            break;
        default:
            // SYNC_RESULT_OK or SYNC_RESULT_TOLERANCE
            // This buffet pair is synchronized.
            {
                Mutex::Autolock _l(mPreviewAvailableQueLock);

                while(mIsLocked){
                    mCondLockPreviewAvailable.wait(mPreviewAvailableQueLock);
                }

                MINT32 assignedReqNo = bufInfoResized->reqNo;
                enqueBufferToAvailableQue(bufInfoResized, assignedReqNo);
                enqueBufferToAvailableQue(bufInfoFull, assignedReqNo);
                enqueBufferToAvailableQue(bufInfoLcso, assignedReqNo);

                enqueBufferToAvailableQue(bufInfoResized_main2, assignedReqNo);
                enqueBufferToAvailableQue(bufInfoFull_main2, assignedReqNo);
                enqueBufferToAvailableQue(bufInfoLcso_main2, assignedReqNo);

                showPreviewAvailableSizes();

                checkAllAvailableQueueSize();

                mCondPushPreviewAvailable.signal();
            }

            sp<IPreviewBufferUser> previewUser = mwpPreviewBufferUser.promote();
            if (previewUser != nullptr){
                previewUser->onPreviewBufferReady();
            }else{
                MY_LOGE("Cannot promote mwpPreviewBufferUser!");
            }
            break;
    }

    FUNC_END;
}
/*******************************************************************************
*
********************************************************************************/
SYNC_RESULT
StereoBufferSynchronizerImp::
isTimeSync(
    BUFFER_SET* bufInfoMain1,
    BUFFER_SET* bufInfoMain2
){
    SYNC_RESULT ret = SYNC_RESULT_PAIR_OK;

    MINT64 timestamp_main1 = -1;
    MINT64 timestamp_main2 = -1;

    // fetch the metadata with timestamp
    for ( size_t i = 0; i < bufInfoMain1->metadata.size(); ++i) {
        // MY_LOGD1("tryGetMetadata bufInfoMain1->metadata %d", i);
        if(eSTREAMID_META_HAL_DYNAMIC_P1 == bufInfoMain1->metadata[i].id ||
           eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2 == bufInfoMain1->metadata[i].id)
        {
            if ( tryGetMetadata<MINT64>(const_cast<IMetadata*>(&bufInfoMain1->metadata[i].meta), MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp_main1) ) {
            // MY_LOGD1("tryGetMetadata bufInfoMain1->metadata %d, suceess!", i);
            break;
        }
    }
    }
    for ( size_t i = 0; i < bufInfoMain2->metadata.size(); ++i) {
        // MY_LOGD1("tryGetMetadata bufInfoMain2->metadata %d", i);
        if(eSTREAMID_META_HAL_DYNAMIC_P1 == bufInfoMain2->metadata[i].id ||
           eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2 == bufInfoMain2->metadata[i].id)
        {
            if ( tryGetMetadata<MINT64>(const_cast<IMetadata*>(&bufInfoMain2->metadata[i].meta), MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp_main2) ) {
            // MY_LOGD1("tryGetMetadata bufInfoMain2->metadata %d, suceess!", i);
            break;
        }
    }
    }

if(mDebugMode == DEBUG_MODE::SKIP_METADATA_CHECK){
    MY_LOGD("Sync check always return OK in DEBUG_MODE_SKIP_METADATA_CHECK");
    return SYNC_RESULT_PAIR_OK;
}

    int timestamp_main1_ms;
    int timestamp_main2_ms;
    int timestamp_diff;

    if(timestamp_main1 != -1 && timestamp_main2 != -1)
    {
        timestamp_main1_ms = timestamp_main1/1000000;
        timestamp_main2_ms = timestamp_main2/1000000;
        timestamp_diff     = timestamp_main1_ms - timestamp_main2_ms;
    }else{
        MY_LOGE("can not get timestamp meta");
        MY_LOGD_IF(timestamp_main1 == -1, "timestamp_main1 is -1");
        MY_LOGD_IF(timestamp_main2 == -1, "timestamp_main2 is -1");
        MY_LOGD1("SYNC_RESULT_PAIR_NOT_SYNC");
        return SYNC_RESULT_PAIR_NOT_SYNC;
    }

    char result_msg[32];
    if(abs(timestamp_diff) < PAIR_THRESHOLD_MS){
        if(abs(timestamp_diff) <= SYNC_THRESHOLD_MS){
            ret = SYNC_RESULT_PAIR_OK;
            snprintf(result_msg,32,"SYNC_RESULT_PAIR_OK");
        }else{
            ret = SYNC_RESULT_PAIR_NOT_SYNC;
            snprintf(result_msg,32,"SYNC_RESULT_PAIR_NOT_SYNC");
        }
    }else{
        if(timestamp_main1 > timestamp_main2){
            ret = SYNC_RESULT_MAIN2_TOO_OLD;
            snprintf(result_msg,32,"SYNC_RESULT_MAIN2_TOO_OLD");
        }else{
            ret = SYNC_RESULT_MAIN1_TOO_OLD;
            snprintf(result_msg,32,"SYNC_RESULT_MAIN1_TOO_OLD");
        }
    }

    MINT32 currentTimestamp = (NSCam::Utils::TimeTool::getReadableTime()) % 100000;
    MY_LOGD1("TS_diff:(main1/main2/diff)(%09d/%09d/%09d)(ms), %25s, machineTimeDelta:%05d(ssmmm)",
        timestamp_main1_ms,
        timestamp_main2_ms,
        timestamp_diff,
        result_msg,
        currentTimestamp - mTimestamp
    );

    mTimestamp = currentTimestamp;

    if(ret != SYNC_RESULT_PAIR_OK){
        failedCounter ++;
        if(failedCounter >= SYNC_FAIL_WARNING_COUNT){
            MY_LOGW("consecutive failed count:%d >= %d! Please check frame sync modules", failedCounter, SYNC_FAIL_WARNING_COUNT);
        }
        if(failedCounter < SYNC_FAIL_TOLERANCE_COUNT){
            MY_LOGW("failedCounter:%d < toleranceCount:%d, do not drop this frame and mark as tolerance buffer", failedCounter, SYNC_FAIL_TOLERANCE_COUNT);
            ret = SYNC_RESULT_TOLERANCE;
        }
        if(failedCounter > SYNC_FAIL_UPPER_LIMIT){
            MY_LOGE("failedCounter:%d > upper limit:%d, let it go", failedCounter, SYNC_FAIL_UPPER_LIMIT);
            ret = SYNC_RESULT_PAIR_OK;
        }
    }else{
        failedCounter = 0;
    }

if(mDebugMode == DEBUG_MODE::SKIP_TIMESTAMP_CHECK){
    MY_LOGD("Sync check always return OK in debugMode=DEBUG_MODE_SKIP_TIMESTAMP_CHECK");
    ret = SYNC_RESULT_PAIR_OK;
}
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isReadyToLock(Vector<StreamId_T>& acquiredBufferSet, Vector<StreamId_T>& rvLockedBufferSet, SYNC_MODE syncMode)
{
    FUNC_START;

    showPreviewAvailableSizes();

    for(map<StreamId_T, list<BUFFER_SET> >::iterator it=streamToPreviewAvailableQue.begin(); it!=streamToPreviewAvailableQue.end(); ++it){
        if( it->second.empty()){
            return MFALSE;
        }
        rvLockedBufferSet.push_back(it->first);
    }
    MY_LOGD1("availablePools are ready");

    FUNC_END;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isZSLBufferReady()
{
    MY_LOGE("not implemented");
    return UNKNOWN_ERROR;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
lock(Vector<StreamId_T>& acquiredBufferSet, Vector<StreamId_T>& rvLockedBufferSet, SYNC_MODE syncMode){
    FUNC_START;
    Mutex::Autolock _l(mPreviewAvailableQueLock);
    MERROR ret = UNKNOWN_ERROR;

    if(mIsLocked){
        MY_LOGE("try to lock synchronizer when it is already locked!");
        return UNKNOWN_ERROR;
    }

    if(isReadyToLock(acquiredBufferSet, rvLockedBufferSet, syncMode)){
        ret = OK;
        mIsLocked = MTRUE;
        MY_LOGD1("mIsLocked:%d", mIsLocked);
    }else{
        ret = UNKNOWN_ERROR;
    }

    FUNC_END;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
unlock(){
    FUNC_START;
    Mutex::Autolock _l(mPreviewAvailableQueLock);

    if(!mIsLocked){
        MY_LOGE("try to unlock synchronizer when it is not locked!");
        return UNKNOWN_ERROR;
    }else{
        mIsLocked = MFALSE;
        mCondLockPreviewAvailable.signal();
    }

    FUNC_END;
    return OK;
}
/******************************************************************************
*
******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isCaptureBufferReady(){
    list<BUFFER_SET> theOldestBuffersFromEachQueue;

    for(auto &e : streamToCaptureAvailableQue){
        if(e.second.size() < mZSLDelayCount){
            MY_LOGD1("sync enabled streamId(%d) capture available queue is %d < ZSLDelayCount:%d. Buffer not ready",
                e.first,
                e.second.size(),
                mZSLDelayCount
            );
            return MFALSE;
        }
        theOldestBuffersFromEachQueue.push_back(e.second.front());
    }

    // Check whether the to-be-used buffer are in the same sync pair
    MY_LOGD1("reqNo check start");
    MINT32 reqNo = theOldestBuffersFromEachQueue.front().reqNo;
    for(auto e : theOldestBuffersFromEachQueue){
        MY_LOGD1("reqNo:%d", e.reqNo);
        if(e.reqNo != reqNo){
            MY_LOGE("reqNo check failed!");
            return MFALSE;
        }
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
waitAndLockCapture(
    MBOOL bNeedClearZSLQueue
){
    FUNC_START;

    Mutex::Autolock _l(mCaptureAvailableQueLock);

    if(mIsCaptureLocked){
        MY_LOGD1("wait for another capture user unlock wait +");
        while(mIsCaptureLocked){
            mCondCaptureUnlock.wait(mCaptureAvailableQueLock);
        }
        MY_LOGD1("wait for another capture user unlock wait -");
    }

    while(MTRUE){
        if(isCaptureBufferReady()){
            mIsCaptureLocked = MTRUE;
            MY_LOGD1("mIsCaptureLocked:%d", mIsCaptureLocked);
            break;
        }else{
            MY_LOGD1("capture buffer not ready wait +");
            status_t status = mCondPushCaptureAvailable.waitRelative(mCaptureAvailableQueLock, WAIT_CAPTURE_BUF_TIMEOUT_NS);
            MY_LOGD1("capture buffer not ready wait -");

            if(status != OK){
                MY_LOGW("capture buffer wating time > %lld", WAIT_CAPTURE_BUF_TIMEOUT_NS);
                mIsAFTimeout = MTRUE;
            }
        }
    }

    // flush and keep only 1 buffer in capture available queue
    if(bNeedClearZSLQueue)
        keepZSLOldestBuffer();

    mIsAFTimeout = MFALSE;

    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
unlockCapture(){
    FUNC_START;
    Mutex::Autolock _l(mCaptureAvailableQueLock);

    if(!mIsCaptureLocked){
        MY_LOGE("try to unlockCapture synchronizer when it is not capture locked!");
        return UNKNOWN_ERROR;
    }else{
        mIsCaptureLocked = MFALSE;
        mCondCaptureUnlock.broadcast();
    }

    FUNC_END;
    return OK;

}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
setDebugMode(MINT32 debug)
{
    mDebugMode = debug;
    MY_LOGD("mDebugMode=%d", mDebugMode);
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
setZSLDelayCount(MINT32 delayCount)
{
    mZSLDelayCount = delayCount;
    MY_LOGD("mZSLDelayCount=%d", mZSLDelayCount);
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
flush(MBOOL flushUnSyncQueue){
    FUNC_START;

    {
        Mutex::Autolock _l(mPreviewPendingQueLock);
        MY_LOGD("stop pushing to previewPendingQue");
        mbIsPreviewStopped = MTRUE;
    }

    MY_LOGD("return all pending/available buffers +");

    clearQueAndReturnBufferToSelector(flushUnSyncQueue);

    MY_LOGD("return all pending/available buffers -");

    Thread::requestExit();
    mbExitThread = MTRUE;
    mCondPushPreviewPending.signal();

    showPreviewPendingSizes();
    showPreviewAvailableSizes();
    showPreviewUsingSizes();
    showCaptureAvailableSizes();
    showCaptureUsingSizes();

    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
flushCaptureQueue(){
    FUNC_START;
    Mutex::Autolock _l(mCaptureAvailableQueLock);
    flushCaptureAvailableQueue();
    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<StereoSelector>
StereoBufferSynchronizerImp::
querySelector(StreamId_T streamId){
    if(streamToSelector.count(streamId)){
        wp<StereoSelector> wpSelector = streamToSelector[streamId];
        sp<StereoSelector> spSelector = wpSelector.promote();
        if (spSelector == 0){
            MY_LOGE("Cannot promote wpSelector!");
        }
        return spSelector;
    }else{
        MY_LOGE("cant find stereo selector for stream %#" PRIx64 ".", streamId);
        return nullptr;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
setPreviewBufferUser(sp<IPreviewBufferUser> spUser){
    mwpPreviewBufferUser = spUser;

    sp<IPreviewBufferUser> previewUser = mwpPreviewBufferUser.promote();
    if (previewUser != nullptr){
        MY_LOGD("previewUser:%p", previewUser.get());
    }else{
        MY_LOGE("Cannot promote mwpPreviewBufferUser!");
    }

    MY_LOGD("mwpPreviewBufferUser:%p", previewUser.get());
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
start(){
    MY_LOGE("deprecated!");
    return UNKNOWN_ERROR;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoBufferSynchronizerImp::
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
/*******************************************************************************
*
********************************************************************************/
PAIR_RESULT
StereoBufferSynchronizerImp::
isPairSync(
    BUFFER_SET* bufInfo_resized,
    BUFFER_SET* bufInfo_full
){
    PAIR_RESULT ret = PAIR_RESULT_PAIR_OK;

    MINT64 timestamp_resized = -1;
    MINT64 timestamp_full = -1;

    // fetch the metadata with timestamp
    for ( size_t i = 0; i < bufInfo_resized->metadata.size(); ++i) {
        if(eSTREAMID_META_HAL_DYNAMIC_P1 == bufInfo_resized->metadata[i].id ||
           eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2 == bufInfo_resized->metadata[i].id)
        {
            if ( tryGetMetadata<MINT64>(const_cast<IMetadata*>(&bufInfo_resized->metadata[i].meta), MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp_resized) ) {
            // MY_LOGD1("tryGetMetadata bufInfo_resized->metadata %d, suceess!", i);
            break;
        }
    }
    }
    for ( size_t i = 0; i < bufInfo_full->metadata.size(); ++i) {
        if(eSTREAMID_META_HAL_DYNAMIC_P1 == bufInfo_full->metadata[i].id ||
           eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2 == bufInfo_full->metadata[i].id)
        {
            if ( tryGetMetadata<MINT64>(const_cast<IMetadata*>(&bufInfo_full->metadata[i].meta), MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp_full) ) {
            // MY_LOGD1("tryGetMetadata bufInfo_full->metadata %d, suceess!", i);
            break;
            }
        }
    }

if(mDebugMode == DEBUG_MODE::SKIP_METADATA_CHECK){
    MY_LOGD("Sync check always return OK in DEBUG_MODE_SKIP_METADATA_CHECK");
    return PAIR_RESULT_PAIR_OK;
}

    int timestamp_resized_ms;
    int timestamp_full_ms;
    int timestamp_diff;
#if 1
    if(timestamp_resized != -1 && timestamp_full != -1)
    {
        timestamp_resized_ms = timestamp_resized/1000000;
        timestamp_full_ms = timestamp_full/1000000;
        timestamp_diff     = timestamp_resized_ms - timestamp_full_ms;
    }else{
        MY_LOGE("can not get timestamp meta");
        MY_LOGD_IF(timestamp_resized == -1, "timestamp_resized is -1");
        MY_LOGD_IF(timestamp_full == -1, "timestamp_full is -1");
        MY_LOGD1("PAIR_RESULT_PAIR_INVALID");
        return PAIR_RESULT_PAIR_INVALID;
    }

    char result_msg[32];
    if(timestamp_diff > 0){
        ret = PAIR_RESULT_FULL_TOO_OLD;
        snprintf(result_msg,32,"PAIR_RESULT_FULL_TOO_OLD");
    }else if(timestamp_diff < 0){
        ret = PAIR_RESULT_RESIZED_TOO_OLD;
        snprintf(result_msg,32,"PAIR_RESULT_RESIZED_TOO_OLD");
    }else{
        snprintf(result_msg,32,"PAIR_RESULT_PAIR_OK");
        ret = PAIR_RESULT_PAIR_OK;
    }

    MY_LOGD1("TS_diff:(resized/full/diff)(%09d/%09d/%09d)(ms), %25s",
        timestamp_resized_ms,
        timestamp_full_ms,
        timestamp_diff,
        result_msg
    );
#else
    // random test
    int dice_roll = mDistribution(mGenerator);

    switch(dice_roll){
        case 8:
            ret = PAIR_RESULT_RESIZED_TOO_OLD;
            break;
        case 9:
            ret = PAIR_RESULT_FULL_TOO_OLD;
            break;
        default:
            ret = PAIR_RESULT_PAIR_OK;
            break;
    }
    MY_LOGD("random dice_roll(%d) ret(%d)", dice_roll, ret);
#endif
    if(ret != PAIR_RESULT_PAIR_OK){
        pairedFailedCounter ++;
        if(pairedFailedCounter > PAIR_SYNC_FAIL_UPPER_LIMIT && pairedFailedCounter < PAIR_SYNC_FAIL_RESET_COUNT){
            MY_LOGW("consecutive pair failed count:%d > %d! Please check P1 node buffer status", pairedFailedCounter, PAIR_SYNC_FAIL_UPPER_LIMIT);
            bufInfo_resized->isPairCheckFailed  = MTRUE; // tolerance but marked pairCheck failed
            bufInfo_full->isPairCheckFailed     = MTRUE;
            ret = PAIR_RESULT_PAIR_OK;
        }else if(pairedFailedCounter >= PAIR_SYNC_FAIL_RESET_COUNT){
            MY_LOGW("consecutive pair failed count:%d > %d! Reset counter", pairedFailedCounter, PAIR_SYNC_FAIL_RESET_COUNT);
            pairedFailedCounter = 0;
        }else{
            // do nothing
        }
    }else{
        pairedFailedCounter = 0;
    }

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
pairCheck(
    BUFFER_SET* bufInfoResized,
    BUFFER_SET* bufInfoFull,
    BUFFER_SET* bufInfoLCSO
){
    MBOOL ret = MTRUE;
    MBOOL needReturnResized = MFALSE;
    MBOOL needReturnFull    = MFALSE;
    MBOOL needReturnLCSO    = MFALSE;
    PAIR_RESULT pair_result;

    // rrzo vs imgo
    pair_result = isPairSync(bufInfoResized, bufInfoFull);
    if(pair_result == PAIR_RESULT_PAIR_INVALID){
        needReturnResized = MTRUE;
        needReturnFull    = MTRUE;
        needReturnLCSO    = MTRUE;
        MY_LOGD1("set 0 invalid");
    }else if(pair_result == PAIR_RESULT_FULL_TOO_OLD){
        needReturnFull    = MTRUE;
        MY_LOGD1("set 0 return full");
    }else if(pair_result == PAIR_RESULT_RESIZED_TOO_OLD){
        needReturnResized = MTRUE;
        MY_LOGD1("set 0 return resized");
    }else{
        // OK
    }

    // rrzo vs lcso
    pair_result = isPairSync(bufInfoResized, bufInfoLCSO);
    if(pair_result == PAIR_RESULT_PAIR_INVALID){
        needReturnResized = MTRUE;
        needReturnFull    = MTRUE;
        needReturnLCSO    = MTRUE;
        MY_LOGD1("set 1 invalid");
    }else if(pair_result == PAIR_RESULT_FULL_TOO_OLD){
        needReturnLCSO    = MTRUE;
        MY_LOGD1("set 1 return lcso");
    }else if(pair_result == PAIR_RESULT_RESIZED_TOO_OLD){
        needReturnResized = MTRUE;
        MY_LOGD1("set 1 return resized");
    }else{
        // OK
    }

    // lcso vs imgo
    pair_result = isPairSync(bufInfoLCSO, bufInfoFull);
    if(pair_result == PAIR_RESULT_PAIR_INVALID){
        needReturnResized = MTRUE;
        needReturnFull    = MTRUE;
        needReturnLCSO    = MTRUE;
        MY_LOGD1("set 2 invalid");
    }else if(pair_result == PAIR_RESULT_FULL_TOO_OLD){
        needReturnFull    = MTRUE;
        MY_LOGD1("set 2 return full");
    }else if(pair_result == PAIR_RESULT_RESIZED_TOO_OLD){
        needReturnLCSO = MTRUE;
        MY_LOGD1("set 2 return lcso");
    }else{
        // OK
    }

    if(needReturnResized){
        returnBufferToSelector(bufInfoResized);
        bufInfoResized->isReturned = MTRUE;
    }

    if(needReturnFull){
        returnBufferToSelector(bufInfoFull);
        bufInfoFull->isReturned = MTRUE;
    }

    if(needReturnLCSO){
        returnBufferToSelector(bufInfoLCSO);
        bufInfoLCSO->isReturned = MTRUE;
    }

    if(needReturnResized || needReturnFull || needReturnLCSO){
        ret = MFALSE;
    }

    MY_LOGD1("ret(%d)", ret);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isInQue(
    list<BUFFER_SET>& theQue, android::sp<IImageBufferHeap>& theHeap
){
    for(auto e : theQue){
        if(e.heap.get() == theHeap.get()){
            return MTRUE;
        }
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isAFStable(
    BUFFER_SET* bufInfoMain1,
    BUFFER_SET* bufInfoMain2
){

    MUINT8 AFstate_main1 = -1;
    MUINT8 lensState_main1 = -1;
    MBOOL ret = MFALSE;

    // fetch the metadata with af state
    for ( size_t i = 0; i < bufInfoMain1->metadata.size(); ++i) {
        if (tryGetMetadata<MUINT8>(const_cast<IMetadata*>(&bufInfoMain1->metadata[i].meta), MTK_CONTROL_AF_STATE, AFstate_main1) &&
            tryGetMetadata<MUINT8>(const_cast<IMetadata*>(&bufInfoMain1->metadata[i].meta), MTK_LENS_STATE, lensState_main1)
            )
        {
            break;
        }
    }

    if( AFstate_main1 == MTK_CONTROL_AF_STATE_PASSIVE_SCAN ||
        AFstate_main1 == MTK_CONTROL_AF_STATE_ACTIVE_SCAN ||
        lensState_main1 == MTK_LENS_STATE_MOVING)
    {
        ret = MFALSE;
    }else{
        ret = MTRUE;
    }

    MY_LOGD1("AF state %d  lens state:%d,  return:%d", AFstate_main1, lensState_main1, ret);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
markAFState(
    BUFFER_SET* bufInfo,
    MBOOL isAFStable
){
    if(bufInfo == NULL){
        return;
    }

    if(mIsAFTimeout){
        MY_LOGW("AF state keep in unstable too long, use unstable buffe anyway!");
        bufInfo->isAFStable = MTRUE;
    }else{
        bufInfo->isAFStable = isAFStable;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
markTolerance(
    BUFFER_SET* bufInfo,
    SYNC_RESULT sync_result
){
    if(bufInfo == NULL){
        return;
    }

    if(sync_result == SYNC_RESULT_TOLERANCE){
        bufInfo->isTolerance = MTRUE;
    }else{
        bufInfo->isTolerance = MFALSE;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
StereoBufferSynchronizerImp::
flushCaptureAvailableQueue(){
    MY_LOGD1("+");
    for(auto &e : streamToCaptureAvailableQue){
        MY_LOGD1("flush capture available queue from stream:%#" PRIx64 " size:%d", e.first, e.second.size());
        while(!e.second.empty()){
            BUFFER_SET dequedSet = e.second.front();
            e.second.pop_front();

            // return to selector
            returnBufferToSelector(e.first, dequedSet.heap);
        }
    }
    MY_LOGD1("-");
}
/******************************************************************************
 *
 ******************************************************************************/
StereoBufferSynchronizerImp::BUFFER_SET
StereoBufferSynchronizerImp::
removeFromQueue(
    list<BUFFER_SET>& theQue, android::sp<IImageBufferHeap>& theHeap
){
    BUFFER_SET retSet;
    for (std::list<BUFFER_SET>::iterator itr = theQue.begin(); itr != theQue.end();){
        if ((*itr).heap.get() == theHeap.get()){
            retSet = (*itr);
            itr = theQue.erase(itr);
        }
        else{
            ++itr;
        }
    }
    return retSet;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
requestEnable(Vector<StreamId_T> streamsToSyncFromMain, Vector<StreamId_T> streamsToSyncFromAux)
{
    mbIsPreviewStopped = MFALSE;
    mbExitThread = MFALSE;
    run(LOG_TAG);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
requestDisable(Vector<StreamId_T> streamsToUseFromMain, Vector<StreamId_T> streamsToUseFromAux)
{
    flush();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoBufferSynchronizerImp::
lockAndDequeBuffer(Vector<StreamId_T>& acquiredBufferSet, Vector<BUFFER_SET>& rvDequedBufferSet)
{
    FUNC_START;

    MERROR ret = UNKNOWN_ERROR;

    //check is ready to lock
    Vector<StreamId_T> rvLockedBufferSet;
    if(lock(acquiredBufferSet, rvLockedBufferSet, SYNC_MODE::SYNC) != OK){
        MY_LOGD1("buffer not ready");
        return ret;
    }

    {
        Mutex::Autolock _l(mPreviewAvailableQueLock);
        for(map<StreamId_T, list<BUFFER_SET> >::iterator it=streamToPreviewAvailableQue.begin(); it!=streamToPreviewAvailableQue.end(); ++it){
            if( it->second.empty()){
                MY_LOGE("stream:%d empty, should not happend!", it->first);
                return ret;
            }else{
                BUFFER_SET dequedSet;
                dequedSet = it->second.front();
                it->second.pop_front();
                dequedSet.isReturned = MFALSE;
                rvDequedBufferSet.push_back(dequedSet);
            }
        }
    }
    // push to preview using queue
    {
        Mutex::Autolock _l(mPreviewUsingQueLock);
        for(auto e:rvDequedBufferSet){
            list<BUFFER_SET>& thePreviewUsingQue = streamToPreviewUsingQue[e.streamId];
            thePreviewUsingQue.push_back(e);
        }
        showPreviewUsingSizes();
    }
    MY_LOGD1("deque from availablePools done");

    unlock();

    ret = OK;

    FUNC_END;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isInVector(StreamId_T streamId, Vector<StreamId_T>& rv)
{
    for(auto e:rv){
        if(e == streamId){
            return MTRUE;
        }
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoBufferSynchronizerImp::
isInVector(StreamId_T streamId, Vector<BUFFER_SET>& rv)
{
    for(auto e:rv){
        if(e.streamId == streamId){
            return MTRUE;
        }
    }
    return MFALSE;
}
/*****************************************************************************
 *
 ******************************************************************************/
bool
StereoBufferSynchronizerImp::
threadLoop()
{
    FUNC_START;

    BUFFER_SET bufInfoResized;
    BUFFER_SET bufInfoResized_main2;
    BUFFER_SET bufInfoFull;
    BUFFER_SET bufInfoFull_main2;
    BUFFER_SET bufInfoLCSO;
    BUFFER_SET bufInfoLCSO_main2;

    // try to grab the to-be-checked buffer set
    {
        Mutex::Autolock _l(mPreviewPendingQueLock);

        MY_LOGD1("StereoSynchronizer threadLoop wait +");
        while(!mbExitThread && !isReadyToSyncCheck()){
            mCondPushPreviewPending.wait(mPreviewPendingQueLock);
        }
        MY_LOGD1("StereoSynchronizer threadLoop wait -");

        if(mbExitThread){
            MY_LOGD("exit thread loop");
            FUNC_END;
            return false;
        }

        mSyncThreadProfile.pulse_up();

        // grab raw buffers from each sensor
        auto grabBufferInfo = [this](StreamId_T streamId, BUFFER_SET& bufSet){
            if(streamToPreviewPendingQue.count(streamId)){
                bufSet = streamToPreviewPendingQue[streamId].front();
                streamToPreviewPendingQue[streamId].pop_front();
            }
        };

        grabBufferInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER,    bufInfoResized);
        grabBufferInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, bufInfoResized_main2);
        grabBufferInfo(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,     bufInfoFull);
        grabBufferInfo(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,  bufInfoFull_main2);
        grabBufferInfo(eSTREAMID_IMAGE_PIPE_RAW_LCSO,       bufInfoLCSO);
        grabBufferInfo(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,    bufInfoLCSO_main2);
    }

    // pair check: check whether imgo & rrzo are the same pair
    MBOOL pairCheck_result_main1 = MTRUE;
    MBOOL pairCheck_result_main2 = MTRUE;
    if(bufInfoResized.heap.get() && bufInfoFull.heap.get()){
        pairCheck_result_main1 = pairCheck(&bufInfoResized, &bufInfoFull, &bufInfoLCSO);

        if(!pairCheck_result_main1){
            MY_LOGD1("pair check failed for main1!");
        }
    }
    if(bufInfoResized_main2.heap.get() && bufInfoFull_main2.heap.get()){
        pairCheck_result_main2 = pairCheck(&bufInfoResized_main2, &bufInfoFull_main2, &bufInfoLCSO_main2);

        if(!pairCheck_result_main2){
            MY_LOGD1("pair check failed for main2!");
        }
    }
    if( !pairCheck_result_main1 || !pairCheck_result_main2 ){

        // return buffer to pending queue if pair sync failed
        auto checkAndReturnToPending = [this](BUFFER_SET& bufSet){
            if(bufSet.heap.get() && (!bufSet.isReturned)){
                returnBufferToPendingQue(&bufSet);
            }
        };

        checkAndReturnToPending(bufInfoResized);
        checkAndReturnToPending(bufInfoFull);
        checkAndReturnToPending(bufInfoLCSO);

        checkAndReturnToPending(bufInfoResized_main2);
        checkAndReturnToPending(bufInfoFull_main2);
        checkAndReturnToPending(bufInfoLCSO_main2);
        return true;
    }

    // synchcronization routine
    syncRoutine(
        (bufInfoResized.heap.get())       ? &bufInfoResized       : nullptr,
        (bufInfoResized_main2.heap.get()) ? &bufInfoResized_main2 : nullptr,
        (bufInfoFull.heap.get())          ? &bufInfoFull          : nullptr,
        (bufInfoFull_main2.heap.get())    ? &bufInfoFull_main2    : nullptr,
        (bufInfoLCSO.heap.get())          ? &bufInfoLCSO          : nullptr,
        (bufInfoLCSO_main2.heap.get())    ? &bufInfoLCSO_main2    : nullptr
    );

    mSyncThreadProfile.pulse_down();
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
}; //namespace NSLegacyPipeline
}; //namespace v1
}; //namespace NSCam
