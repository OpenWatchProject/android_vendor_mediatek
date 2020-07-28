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

#define LOG_TAG "MtkCam/ResultProcessor"
//
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include "MyUtils.h"
#include <utils/List.h>
#include <utils/KeyedVector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/ResultProcessor.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v3;
//
/******************************************************************************
 *
 ******************************************************************************/
#define RESULTPROCESSOR_NAME       ("Cam@ResultProcessor")
#define RESULTPROCESSOR_POLICY     (SCHED_OTHER)
#define RESULTPROCESSOR_PRIORITY   (0)

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
class ResultProcessorImp
    : public ResultProcessor
    , protected Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ResultProcessor Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     //// for normal callback
    virtual status_t                registerListener(
                                        StreamId_T      const streamId,
                                        wp< IListener > const listener
                                    );

    virtual status_t                registerListener(
                                        MUINT32         const startRequestNo,
                                        MUINT32         const endRequestNo,
                                        MBOOL           const needPartial,
                                        wp< IListener > const listener
                                    );

    virtual status_t                removeListener(
                                        StreamId_T      const streamId,
                                        wp< IListener > const listener
                                    );

    virtual status_t                removeListener(
                                        MUINT32         const startRequestNo,
                                        MUINT32         const endRequestNo,
                                        MBOOL           const needPartial,
                                        wp< IListener > const listener
                                    );

public:     //// for early callback
    virtual status_t                registerListener(
                                        wp< IDataListener > const listener
                                    );

    virtual status_t                removeListener(
                                        wp< IDataListener > const listener
                                    );

public:
    virtual void                    flush(MBOOL reuse = MFALSE);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IAppCallback Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                    updateFrame(
                                        MUINT32 const requestNo,
                                        MINTPTR const userId,
                                        Result const& result
                                    );

    virtual MVOID                   onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDataCallback Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MVOID                   onImageCallback(
                                        MUINT32                         /*requestNo*/,
                                        Pipeline_NodeId_T               /*nodeId*/,
                                        StreamId_T                      /*streamId*/,
                                        android::sp<IImageBufferHeap>&  /*rpImageBuffer*/,
                                        android::sp<IImageStreamInfo>&  /*rpStreamInfo*/,
                                        MBOOL                           /*errorResult*/
                                    ) { MY_LOGE("Currently not support."); }

    virtual MVOID                   onMetaCallback(
                                        MUINT32              requestNo,
                                        Pipeline_NodeId_T    nodeId,
                                        StreamId_T           streamId,
                                        IMetadata const&     rMetaData,
                                        MBOOL                errorResult
                                    );

    virtual MBOOL                   isCtrlSetting();
    virtual MVOID                   onCtrlSetting(
                                        MUINT32              requestNo,
                                        Pipeline_NodeId_T    nodeId,
                                        StreamId_T const     metaAppStreamId,
                                        IMetadata&           rAppMetaData,
                                        StreamId_T const     metaHalStreamId,
                                        IMetadata&           rHalMetaData,
                                        MBOOL&               rIsChanged
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

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
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: ////                     Structure.

    class ListenerBase : public RefBase
    {
    public:
                                ListenerBase() {};

        virtual                 ~ListenerBase() {};

        virtual bool            canCallback(
                                    MUINT32    requestNo,
                                    StreamId_T id
                                )                                      = 0;

        virtual sp< IListener > getListener()                          = 0;

        virtual bool            equal(
                                    StreamId_T      /*id*/,
                                    wp< IListener > /*rlistener*/
                                 ) {
                                    MY_LOGW("should not happen!");
                                    return false;
                                 }

        virtual bool            equal(
                                    MUINT32         /*start*/,
                                    MUINT32         /*end*/,
                                    wp< IListener > /*rlistener*/
                                ) {
                                    MY_LOGW("should not happen!");
                                    return false;
                                }

    };

    class RangeListener : public ListenerBase
    {
    public:
                                RangeListener(
                                    MUINT32 start,
                                    MUINT32 end,
                                    wp< IListener > rlistener)
                                    : startRequestNo(start)
                                    , endRequestNo(end)
                                    , listener(rlistener)
                                {}

        virtual sp< IListener > getListener() {
                                    return listener.promote();
                                }

        virtual bool            canCallback(
                                    MUINT32    requestNo,
                                    StreamId_T /*id*/
                                ) {
                                    return (requestNo >= startRequestNo
                                         && requestNo <= endRequestNo);
                                }

        virtual bool            equal(
                                    MUINT32         start,
                                    MUINT32         end,
                                    wp< IListener > rlistener
                                ) {
                                    return ( start == startRequestNo
                                            && end == endRequestNo
                                            && rlistener == listener
                                            );
                                }

    protected:
        MUINT32         startRequestNo;
        MUINT32         endRequestNo;
        wp< IListener > listener;
    };

    class StreamListener : public ListenerBase
    {
    public:
                                StreamListener(
                                    StreamId_T id,
                                    wp< IListener > rlistener
                                )
                                    : streamId(id)
                                    , listener(rlistener)
                                {}

        virtual sp< IListener > getListener() {
                                    return listener.promote();
                                }

        virtual bool            canCallback(
                                    MUINT32    /*requestNo*/,
                                    StreamId_T id
                                ) {
                                    return (id == streamId);
                                }

        virtual bool            equal(
                                    StreamId_T      id,
                                    wp< IListener > rlistener
                                ) {
                                    return ( id == streamId && rlistener == listener );
                                }

    protected:
        StreamId_T      streamId;
        wp< IListener > listener;
    };

    enum BufferBit
    {
        RETURNED,
        LAST_PARTIAL
    };

    struct ResultItem : public RefBase
    {
        MUINT32                         requestNo;
        BitSet32                        status;
        Vector< sp<IMetaStreamBuffer> > outAppBuffer;
        Vector< sp<IMetaStreamBuffer> > outHalBuffer;
    };

    struct EarlyResultItem : public RefBase
    {
        MUINT32         requestNo;
        StreamId_T      streamId;
        MBOOL           errorResult;
        IMetadata       meta;
        /*android::sp<IImageBuffer>       buffer;*/
    };

    struct Callback : public RefBase
    {
        MUINT32         requestNo;
        StreamId_T      streamId;
        MBOOL           errorResult;
        IMetadata*      buffer;
        sp< IListener > listener;
    };

public:     ////                    Definitions.

    typedef KeyedVector< MUINT32, sp< ResultItem > > BufQue_T;
    typedef KeyedVector< MUINT32, sp< EarlyResultItem > > EBufQue_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    ResultProcessorImp();

                                    ~ResultProcessorImp();

protected:

            status_t                dequeResult(
                                        BufQue_T&        rvResult,
                                        EBufQue_T&       rvEarlyResult,
                                        List< MUINT32 >& rvFrameEnd
                                    );

            MVOID                   handleResult(
                                        BufQue_T const&                     rvResult,
                                        EBufQue_T const&                    rvEarlyResult,
                                        List< MUINT32 > const&              rvFrameEnd
                                    );

            MVOID                   handlePartialResultCallback(
                                        BufQue_T const& rvResult
                                    );

            MVOID                   handleFullResultCallback();

            MVOID                   handleListener(
                                        sp< ResultItem >&                   pItem,
                                        List< sp<ListenerBase> >&           listenerList,
                                        /*out*/
                                        List< sp<Callback> >&               callbackList
                                    );

            MVOID                   addPartialCallback(
                                        MUINT32                             requestNo,
                                        sp<ListenerBase>&                   item,
                                        Vector< sp<IMetaStreamBuffer> >&    vOutMeta,
                                        /*out*/
                                        List< sp<Callback> >&               callbackList
                                    );

protected:  ////                    Full result callback.
            MVOID                   collectPartialResult(
                                        MUINT32          const requestNo,
                                        sp< ResultItem > const rpItem
                                    );

            MVOID                   addFullCallback(
                                        MUINT32                             requestNo,
                                        StreamId_T                    const streamId,
                                        sp<ListenerBase>&                   item,
                                        Vector< sp<IMetaStreamBuffer> >&    vOutMeta,
                                        /*out*/
                                        List< sp<Callback> >&               callbackList
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: ////                     Data Members. (partial result listener).
    List< sp<ListenerBase> >        mRangeListenerList;
    List< sp<ListenerBase> >        mStreamListenerList;
    List< wp<IDataListener> >       mEarlyListenerList;
    mutable Mutex                   mListenerListLock;

protected: ////                     Data Members. (partial result queue).
    BufQue_T                        mResultQueue;
    mutable Mutex                   mResultQueueLock;
    Condition                       mResultQueueCond;
    List< MUINT32 >                 mFrameEndList;

protected: ////                     Data Members. (partial result queue).
    EBufQue_T                       mEarlyResultQueue;

protected: ////                     Data Members. (full result).
    BufQue_T                        mFullResultQueue;
    mutable Mutex                   mFullResultQueueLock;
    List< sp<ListenerBase> >        mFullListenerList;

protected: ////                     Logs.
    MINT32                          mLogLevel;
};


/******************************************************************************
 *
 ******************************************************************************/
sp< ResultProcessor >
ResultProcessor::
createInstance()
{
    return new ResultProcessorImp();
}

/******************************************************************************
 *
 ******************************************************************************/
ResultProcessorImp::
ResultProcessorImp()
    : ResultProcessor()
{
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.LegacyPipeline", 0);
    }
    //
    status_t status = run(RESULTPROCESSOR_NAME);
    if  ( OK != status ) {
        MY_LOGE("Fail to run the thread - status:%d(%s)", status, ::strerror(-status));
    }
}


/******************************************************************************
 *
 ******************************************************************************/
ResultProcessorImp::
~ResultProcessorImp()
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
ResultProcessorImp::
onLastStrongRef(const void* /*id*/)
{
    MY_LOGD("+");
    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
void
ResultProcessorImp::
flush(MBOOL reuse)
{
    MY_LOGD("+");

    MY_LOGD("request exit thread to flush unused data");
    Thread::requestExit();
    {
        Mutex::Autolock _l(mListenerListLock);
        mRangeListenerList.clear();
        mFullListenerList.clear();
        mStreamListenerList.clear();
    }
    //
    {
        Mutex::Autolock _l(mResultQueueLock);
        mResultQueue.clear();
        mFrameEndList.clear();
        mResultQueueCond.signal();
    }
    //
    {
        Mutex::Autolock _l(mFullResultQueueLock);
        //let deque thread back
        mFullResultQueue.clear();
    }
    MY_LOGD("thread join...");
    join();

    if (reuse) {
        MY_LOGD("resume the ResultProcessor thread for pipeline reuse");
        status_t status = run(RESULTPROCESSOR_NAME);
        if  ( OK != status ) {
            MY_LOGE("Fail to resume the thread - status:%d(%s)", status, ::strerror(-status));
        }
    }

    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)RESULTPROCESSOR_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, RESULTPROCESSOR_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, RESULTPROCESSOR_PRIORITY);
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
ResultProcessorImp::
registerListener(
    wp< IDataListener > const listener
)
{
    Mutex::Autolock _l(mListenerListLock);
    //
    sp< IDataListener > l = listener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener.");
        return OK;
    }
    List< wp<IDataListener> >::iterator item = mEarlyListenerList.begin();
    while ( item != mEarlyListenerList.end() ) {
        sp< IDataListener > lis = (*item).promote();
        if ( lis == nullptr) {
            // invalid listener
            item = mEarlyListenerList.erase(item);
            continue;
        }
        if ( lis == l ) {
            // already registered, just return
            MY_LOGD_IF(
                1,
                "Attempt to register the same client twice, ignoring"
            );
            return OK;
        }
        item++;
    }
    //
    mEarlyListenerList.push_back(listener);
    //
    MY_LOGD_IF(
        1,
        "DataListener:%p(%zu)",
        listener.unsafe_get(), mEarlyListenerList.size()
    );

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
registerListener(
    StreamId_T      const streamId,
    wp< IListener > const listener
)
{
    Mutex::Autolock _l(mListenerListLock);
    //
    List< sp<ListenerBase> >::iterator item = mStreamListenerList.begin();
    while ( item != mStreamListenerList.end() ) {
        if ( (*item)->equal(streamId, listener) ) {
            // already registered, just return
            MY_LOGD_IF(
                1,
                "Attempt to register the same client twice, ignoring"
            );
            return OK;
        }
        item++;
    }
    //
    sp< IListener > l = listener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener. stream:%#" PRIx64 , streamId);
    } else {
        sp<ListenerBase> sListener = new StreamListener(streamId, listener);
        mStreamListenerList.push_back(sListener);
        //
        MY_LOGD_IF(
            1,
            "StreamListener:%s(%zu) stream:%#" PRIx64 ,
            l->getUserName().string(), mStreamListenerList.size(), streamId
        );
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
registerListener(
    MUINT32         const startRequestNo,
    MUINT32         const endRequestNo,
    MBOOL           const needPartial,
    wp< IListener > const listener
)
{
    Mutex::Autolock _l(mListenerListLock);
    //
    List< sp<ListenerBase> >* listeners;
    if ( needPartial ) {
        listeners = &mRangeListenerList;
    } else {
        listeners = &mFullListenerList;
    }
    //
    List< sp<ListenerBase> >::iterator item = (*listeners).begin();
    while ( item != (*listeners).end() ) {
        if ( (*item)->equal(startRequestNo, endRequestNo, listener) ) {
            // already registered, just return
            MY_LOGD_IF(
                1,
                "Attempt to register the same client twice, ignoring"
            );
            return OK;
        }
        item++;
    }
    //
    sp< IListener > l = listener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener. range(%d,%d) partial:%d", startRequestNo, endRequestNo, needPartial);
    } else {
        sp<ListenerBase> rListener = new RangeListener(startRequestNo, endRequestNo, listener);
        (*listeners).push_back(rListener);
        //
        MY_LOGD_IF(
            1,
            "RangeListener:%s(%zu) range(%d,%d) partial:%d",
            l->getUserName().string(), (*listeners).size(), startRequestNo, endRequestNo, needPartial
        );
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
removeListener(
    MUINT32         const startRequestNo,
    MUINT32         const endRequestNo,
    MBOOL           const needPartial,
    wp< IListener > const listener
)
{
    Mutex::Autolock _l(mListenerListLock);
    //
    List< sp<ListenerBase> >* listeners;
    if ( needPartial ) {
        listeners = &mRangeListenerList;
    } else {
        listeners = &mFullListenerList;
    }
    //
    List< sp<ListenerBase> >::iterator item = (*listeners).begin();
    while ( item != (*listeners).end() ) {
        if ( (*item)->equal(startRequestNo, endRequestNo, listener) ) {
            item = (*listeners).erase(item);
            MY_LOGD_IF(
                1,
                "Remove listener for range(%d,%d) partial:%d",
                startRequestNo, endRequestNo, needPartial
            );
        } else {
            item++;
        }
    }
    //
    sp< IListener > l = listener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener. range(%d,%d) partial:%d", startRequestNo, endRequestNo, needPartial);
    } else {
        MY_LOGD_IF(
            1,
            "RangeListener:%s(%zu) range(%d,%d) partial:%d",
            l->getUserName().string(), (*listeners).size(), startRequestNo, endRequestNo, needPartial
        );
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
removeListener(
    wp< IDataListener > const listener
)
{
    Mutex::Autolock _l(mListenerListLock);
    //
    sp< IDataListener > l = listener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener.");
        return OK;
    }
    List< wp<IDataListener> >::iterator item = mEarlyListenerList.begin();
    while ( item != mEarlyListenerList.end() ) {
        sp< IDataListener > lis = (*item).promote();
        if ( lis == nullptr || lis == l ) {
            // invalid listener or target to remove
            item = mEarlyListenerList.erase(item);
            continue;
        }
        item++;
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
removeListener(
    StreamId_T      const streamId,
    wp< IListener > const listener
)
{
    Mutex::Autolock _l(mListenerListLock);
    //
    List< sp<ListenerBase> >::iterator item = mStreamListenerList.begin();
    while ( item != mStreamListenerList.end() ) {
        if ( (*item)->equal(streamId, listener) ) {
            item = mStreamListenerList.erase(item);
            MY_LOGD_IF(
                1,
                "Remove listener for stream id %#" PRIx64 ,
                streamId
            );
        } else {
            item++;
        }
    }
    //
    sp< IListener > l = listener.promote();
    if ( l == 0 ) {
        MY_LOGW("Bad listener. stream:%#" PRIx64 , streamId);
    } else {
        MY_LOGD_IF(
            1,
            "RangeListener:%s(%zu) stream:%#" PRIx64 ,
            l->getUserName().string(), mStreamListenerList.size(), streamId
        );
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
onMetaCallback(
    MUINT32              requestNo,
    Pipeline_NodeId_T    nodeId,
    StreamId_T           streamId,
    IMetadata const&     rMetaData,
    MBOOL                errorResult
)
{
    Mutex::Autolock _l(mResultQueueLock);
    //
#if 1
    {
        String8 str = String8::format("requestNo:%u userId:%#" PRIxPTR " streamId:%#" PRIx64 " error:%d"
            , requestNo, nodeId, streamId, errorResult);
        MY_LOGD_IF(
            mLogLevel >= 1,
            "%s", str.string()
        );
    }
#endif
    //
    if  ( exitPending() ) {
        MY_LOGW("Dead ResultQueue");
        return;
    }
    //
    ssize_t const index = mEarlyResultQueue.indexOfKey(requestNo);

    if  ( 0 <= index ) {
        MY_LOGW("requestNo:%u existed @ index:%zd something wrong.", requestNo, index);
    } else {
        sp< EarlyResultItem > pItem = new EarlyResultItem;
        pItem->requestNo       = requestNo;
        pItem->streamId        = streamId;
        pItem->errorResult     = errorResult;
        pItem->meta            = rMetaData;
        //
        mEarlyResultQueue.add(requestNo, pItem);
        mResultQueueCond.signal();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ResultProcessorImp::
isCtrlSetting()
{
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
onCtrlSetting(
    MUINT32              requestNo,
    Pipeline_NodeId_T    nodeId,
    StreamId_T const     metaAppStreamId,
    IMetadata&           rAppMetaData,
    StreamId_T const     metaHalStreamId,
    IMetadata&           rHalMetaData,
    MBOOL&               rIsChanged
)
{
    {
        Mutex::Autolock _l(mListenerListLock);
        List< wp<IDataListener> >::iterator item = mEarlyListenerList.begin();
        while ( item != mEarlyListenerList.end() ) {
            sp< IDataListener > lis = (*item).promote();
            if ( lis == nullptr ) {
                // invalid listener or target to remove
                item = mEarlyListenerList.erase(item);
                continue;
            }
            MY_LOGD_IF(
                //mLogLevel >= 1,
                1,
                "[CtrlCallback] requestNo:%d metaAppStreamId:(%#" PRIxPTR ") metaHalStreamId:(%#" PRIxPTR ") error:%d",
                requestNo, metaAppStreamId, metaHalStreamId
            );
            lis->onCtrlSetting(requestNo, metaAppStreamId, rAppMetaData, metaHalStreamId, rHalMetaData, rIsChanged);
            item++;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
updateFrame(
    MUINT32 const  requestNo,
    MINTPTR const  userId,
    Result  const& result
)
{
    Mutex::Autolock _l(mResultQueueLock);
    //
    if(mLogLevel >= 1)
    {
        String8 str = String8::format("requestNo:%u userId:%#" PRIxPTR " appMeta(left:%zd) ", requestNo, userId, result.nAppOutMetaLeft);
        for ( size_t i = 0; i < result.vAppOutMeta.size(); ++i)
            str += String8::format("%s ", result.vAppOutMeta[i]->getName());
        str += String8::format("halMeta(left:%zd) ", result.nHalOutMetaLeft);
        for ( size_t i = 0; i < result.vHalOutMeta.size(); ++i)
            str += String8::format("%s ", result.vHalOutMeta[i]->getName());
        MY_LOGD("%s", str.string());
    }

    //
    if  ( exitPending() ) {
        MY_LOGW("Dead ResultQueue");
        return;
    }
    //
    if ( result.bFrameEnd ) {
        mFrameEndList.push_back(requestNo);
        return;
    }
    //
    ssize_t const index = mResultQueue.indexOfKey(requestNo);

    if  ( 0 <= index ) {
        MY_LOGD("requestNo:%u existed @ index:%zd frameEnd:%d", requestNo, index, result.bFrameEnd);
        //
        sp<ResultItem> pItem = mResultQueue.editValueAt(index);
        pItem->outAppBuffer.appendVector(result.vAppOutMeta);
        pItem->outHalBuffer.appendVector(result.vHalOutMeta);
        //
        if ( result.nAppOutMetaLeft == 0 && result.nHalOutMetaLeft == 0)
            pItem->status.markBit(BufferBit::LAST_PARTIAL);

        mResultQueueCond.signal();
    }
    else {
        sp< ResultItem > pItem = new ResultItem;
        pItem->requestNo       = requestNo;
        pItem->outAppBuffer    = result.vAppOutMeta;
        pItem->outHalBuffer    = result.vHalOutMeta;
        //
        if ( result.nAppOutMetaLeft == 0 && result.nHalOutMetaLeft == 0 )
            pItem->status.markBit(BufferBit::LAST_PARTIAL);
        //
        mResultQueue.add(requestNo, pItem);
        mResultQueueCond.signal();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
ResultProcessorImp::
threadLoop()
{
    BufQue_T        vResult;
    EBufQue_T       vEarlyResult;
    List< MUINT32 > vFrameEnd;
    MERROR err = dequeResult(vResult, vEarlyResult, vFrameEnd);
    if  ( OK == err && ! (vResult.isEmpty() && vEarlyResult.isEmpty()) )
    {
        handleResult(vResult, vEarlyResult, vFrameEnd);
    }
    //
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ResultProcessorImp::
dequeResult(
    BufQue_T&        rvResult,
    EBufQue_T&       rvEarlyResult,
    List< MUINT32 >& rvFrameEnd
)
{
    status_t err = OK;
    //
    Mutex::Autolock _l(mResultQueueLock);
    //
    while ( ! exitPending() && mResultQueue.isEmpty() )
    {
        err = mResultQueueCond.wait(mResultQueueLock);
        MY_LOGW_IF(
            OK != err,
            "exitPending:%d ResultQueue#:%zu err:%d(%s)",
            exitPending(), mResultQueue.size(), err, ::strerror(-err)
        );
    }
    //
    if  ( mResultQueue.isEmpty() )
    {
        MY_LOGD_IF(
            mLogLevel >= 1,
            "empty queue"
        );
        rvResult.clear();
        rvEarlyResult.clear();
        rvFrameEnd.clear();
        err = NOT_ENOUGH_DATA;
    }
    else
    {
        rvResult = mResultQueue;
        mResultQueue.clear();
        //
        rvEarlyResult = mEarlyResultQueue;
        mEarlyResultQueue.clear();
        //
        rvFrameEnd = mFrameEndList;
        mFrameEndList.clear();
        err = OK;
    }
    //
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
handleResult(
    BufQue_T        const& rvResult,
    EBufQue_T       const& rvEarlyResult,
    List< MUINT32 > const& rvFrameEnd
)
{
    // handle partial callback
    handlePartialResultCallback( rvResult );
    //
    // handle full result callback
    {
        Mutex::Autolock _l(mFullResultQueueLock);
        //
        handleFullResultCallback();
        //
        // remove full result
        size_t i = 0;
        while ( i < mFullResultQueue.size() ) {
            MUINT32 const requestNo = mFullResultQueue.keyAt(i);
            sp< ResultItem > pItem  = mFullResultQueue.valueAt(i);
            if  ( pItem == 0 ) {
                MY_LOGW_IF(
                    mLogLevel >= 1,
                    "requestNo %d NULL ResultItem", requestNo
                );
                continue;
            }
            //
            if ( pItem->status.hasBit(BufferBit::LAST_PARTIAL) ) {
                MY_LOGW_IF(
                    mLogLevel >= 1,
                    "remove mFullResultQueue[%zu]", i
                );
                i = mFullResultQueue.removeItemsAt(i);
                continue;
            }
            //
            ++i;
        }
    }
    //
    {
        Mutex::Autolock _l(mListenerListLock);
        //
        List< MUINT32 >::const_iterator item = rvFrameEnd.begin();
        while (item != rvFrameEnd.end()) {
            MY_LOGD_IF(
                mLogLevel >= 1,
                "[frameEnd] requestNo:%d ", (*item)
            );
            //
            // callback frame end & check invalid listener.
            #define HANDLE_FRAME_END( _ListenerList_ , _RequestNo_ ) {              \
                List< sp<ListenerBase> >::iterator pItem = _ListenerList_.begin();  \
                while (pItem != _ListenerList_.end()) {                             \
                    sp< IListener > listener = (*pItem)->getListener();             \
                    if (listener != 0) {                                            \
                        MY_LOGD_IF(                                                 \
                            0,                                                      \
                            "[frameEnd] %s requestNo:%d",                           \
                            listener->getUserName().string(), _RequestNo_           \
                        );                                                          \
                        listener->onFrameEnd( _RequestNo_ );                        \
                        pItem++;                                                    \
                    } else {                                                        \
                        pItem = _ListenerList_.erase(pItem);                        \
                    }                                                               \
                }                                                                   \
            }
            //
            HANDLE_FRAME_END(mRangeListenerList  , *item);
            HANDLE_FRAME_END(mStreamListenerList , *item);
            HANDLE_FRAME_END(mFullListenerList   , *item);
            //
            item++;
            //
            #undef HANDLE_FRAME_END
        }
    }

    {
        for ( size_t i = 0; i < rvEarlyResult.size(); ++i ) {
            Mutex::Autolock _l(mListenerListLock);
            //
            sp< EarlyResultItem > pItem = rvEarlyResult[i];
            List< wp<IDataListener> >::iterator item = mEarlyListenerList.begin();
            while ( item != mEarlyListenerList.end() ) {
                sp< IDataListener > lis = (*item).promote();
                if ( lis == nullptr ) {
                    // invalid listener or target to remove
                    item = mEarlyListenerList.erase(item);
                    continue;
                }
                MY_LOGD_IF(
                    mLogLevel >= 1,
                    "[early meta] requestNo:%d streamId:%#" PRIx64 " error:%d",
                    pItem->requestNo, pItem->streamId, pItem->errorResult
                );
                lis->onMetaReceived(pItem->requestNo, pItem->streamId, pItem->errorResult, pItem->meta);
                item++;
            }
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
handlePartialResultCallback(
    BufQue_T        const& rvResult
)
{
    for (size_t i = 0; i < rvResult.size(); i++) {
        MUINT32 const requestNo = rvResult.keyAt(i);
        sp< ResultItem > pItem  = rvResult.valueAt(i);
        if  ( pItem == 0 ) {
            MY_LOGW_IF(
                mLogLevel >= 1,
                "requestNo %d NULL ResultItem", requestNo
            );
            continue;
        }
        //
        collectPartialResult( requestNo, pItem );
        //
        List< sp<Callback> >  callbackList;
        {
            Mutex::Autolock _l(mListenerListLock);
            handleListener(pItem, mRangeListenerList, callbackList);
            handleListener(pItem, mStreamListenerList, callbackList);
        }
        //
        List< sp<Callback> >::iterator item = callbackList.begin();
        for (; item != callbackList.end(); item++) {
            MY_LOGD_IF(
                mLogLevel >= 1,
                "[partial] %s requestNo:%d streamId:%#" PRIx64 " error:%d buffer:%p(%d)",
                (*item)->listener->getUserName().string(), (*item)->requestNo, (*item)->streamId, (*item)->errorResult, (*item)->buffer, (*item)->buffer->count()
            );
            //
            CAM_TRACE_FMT_BEGIN("PartCBtoListner No%d,ID%#" PRIx64 ,(*item)->requestNo, (*item)->streamId);
            (*item)->listener->onResultReceived(
                                    (*item)->requestNo,
                                    (*item)->streamId,
                                    (*item)->errorResult,
                                    *((*item)->buffer)
                               );
            CAM_TRACE_FMT_END();
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
handleFullResultCallback()
{
    for (size_t i = 0; i < mFullResultQueue.size(); i++) {
        MUINT32 const requestNo = mFullResultQueue.keyAt(i);
        sp< ResultItem > pItem  = mFullResultQueue.valueAt(i);
        if  ( pItem == 0 ) {
            MY_LOGW_IF(
                mLogLevel >= 1,
                "requestNo %d NULL ResultItem", requestNo
            );
            continue;
        }
        //
        List< sp<Callback> >  callbackList;
        {
            Mutex::Autolock _l(mListenerListLock);
            //
            List< sp<ListenerBase> >::iterator item = mFullListenerList.begin();
            while (item != mFullListenerList.end()) {
                // app metadata
                if (    pItem->status.hasBit(BufferBit::LAST_PARTIAL)
                    && !pItem->status.hasBit(BufferBit::RETURNED) ) {
                    MY_LOGD("%d handle full result", pItem->requestNo);
                    addFullCallback(pItem->requestNo, eSTREAMID_META_APP_FULL, (*item), pItem->outAppBuffer, /*out*/callbackList);
                    addFullCallback(pItem->requestNo, eSTREAMID_META_HAL_FULL, (*item), pItem->outHalBuffer, /*out*/callbackList);
                    pItem->status.markBit(BufferBit::RETURNED);
                }

                item++;
            }
        }
        //
        List< sp<Callback> >::iterator item = callbackList.begin();
        for (; item != callbackList.end(); item++) {
            MY_LOGD_IF(
                mLogLevel >= 1,
                "[full] %s requestNo:%d streamId:%#" PRIx64 " error%d buffer:%p(%d)",
                (*item)->listener->getUserName().string(),
                (*item)->requestNo,
                (*item)->streamId,
                (*item)->errorResult, (*item)->buffer, (*item)->buffer->count()
            );
            //
            CAM_TRACE_FMT_BEGIN("FullCBtoListner No%d,ID%#" PRIx64 ,(*item)->requestNo, (*item)->streamId);
            (*item)->listener->onResultReceived(
                                    (*item)->requestNo,
                                    (*item)->streamId,
                                    (*item)->errorResult,
                                    *((*item)->buffer)
                               );
            CAM_TRACE_FMT_END();
        }

    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
collectPartialResult(
    MUINT32          const requestNo,
    sp< ResultItem > const rpItem
)
{
    Mutex::Autolock _l(mFullResultQueueLock);

    ssize_t const index = mFullResultQueue.indexOfKey(requestNo);

    if  ( 0 <= index ) {
        MY_LOGD_IF(
            mLogLevel >= 1,
            "requestNo:%u existed @ index:%zd", requestNo, index
        );
        //
        sp<ResultItem> pItem = mFullResultQueue.editValueAt(index);
        pItem->outAppBuffer.appendVector(rpItem->outAppBuffer);
        pItem->outHalBuffer.appendVector(rpItem->outHalBuffer);
        //
        if ( rpItem->status.hasBit(BufferBit::LAST_PARTIAL) )
            pItem->status.markBit(BufferBit::LAST_PARTIAL);
    }
    else {
        sp< ResultItem > pItem = new ResultItem;
        pItem->requestNo       = requestNo;
        pItem->outAppBuffer    = rpItem->outAppBuffer;
        pItem->outHalBuffer    = rpItem->outHalBuffer;
        //
        if ( rpItem->status.hasBit(BufferBit::LAST_PARTIAL) )
            pItem->status.markBit(BufferBit::LAST_PARTIAL);
        //
        mFullResultQueue.add(requestNo, pItem);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
handleListener(
    sp< ResultItem >&         pItem,
    List< sp<ListenerBase> >& listenerList,
    /*out*/
    List< sp<Callback> >&     callbackList
)
{
    List< sp<ListenerBase> >::iterator item = listenerList.begin();
    while (item != listenerList.end()) {
        addPartialCallback(pItem->requestNo, (*item), pItem->outAppBuffer, /*out*/callbackList);
        addPartialCallback(pItem->requestNo, (*item), pItem->outHalBuffer, /*out*/callbackList);
        item++;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
addPartialCallback(
    MUINT32                             requestNo,
    sp<ListenerBase>&                   item,
    Vector< sp<IMetaStreamBuffer> >&    vOutMeta,
    /*out*/
    List< sp<Callback> >&               callbackList
)
{
    for ( size_t i = 0; i < vOutMeta.size(); ++i) {
        if ( item->canCallback(requestNo, vOutMeta[i]->getStreamInfo()->getStreamId()) ) {
            sp< IListener > listener = item->getListener();
            if (listener == 0) {
                //item = listenerList.erase(item);
                //continue;
                return;
            } else {
                sp< Callback > pCallback = new Callback;
                pCallback->requestNo     = requestNo;
                pCallback->streamId      = vOutMeta[i]->getStreamInfo()->getStreamId();
                pCallback->errorResult   = vOutMeta[i]->hasStatus(STREAM_BUFFER_STATUS::ERROR);
                pCallback->buffer        = vOutMeta[i]->tryReadLock(LOG_TAG);
                pCallback->listener      = listener;
                callbackList.push_back(pCallback);
                //
                vOutMeta[i]->unlock(LOG_TAG, pCallback->buffer);
            }
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResultProcessorImp::
addFullCallback(
    MUINT32                             requestNo,
    StreamId_T                    const streamId,
    sp<ListenerBase>&                   item,
    Vector< sp<IMetaStreamBuffer> >&    vOutMeta,
    /*out*/
    List< sp<Callback> >&               callbackList
)
{
    if ( !item->canCallback(requestNo, 0) ) return;
    //
    if( vOutMeta.size()==0 )
    {
        MY_LOGW("requestNo:%u streamId:%#" PRIx64 " vOutMeta size == 0", requestNo, streamId);
        return;
    }
    //
    sp< IListener > listener = item->getListener();
    if (listener == 0) {
        //item = listenerList.erase(item);
        return;
    } else {
        if( vOutMeta.size() > 0 && vOutMeta[0].get() != nullptr ) {
            IMetadata* result = vOutMeta[0]->tryReadLock(LOG_TAG);
            vOutMeta[0]->unlock(LOG_TAG, result);
            for ( size_t i = 1; i < vOutMeta.size(); ++i) {
                if (vOutMeta[i].get() != nullptr) {
                    IMetadata* meta = vOutMeta[i]->tryReadLock(LOG_TAG);
                    (*result) += (*meta);
                    vOutMeta[i]->unlock(LOG_TAG, meta);
                }
            }
            //
            sp< Callback > pCallback = new Callback;
            pCallback->requestNo     = requestNo;
            pCallback->streamId      = streamId;
            pCallback->errorResult   = vOutMeta[vOutMeta.size() - 1]->hasStatus(STREAM_BUFFER_STATUS::ERROR);
            pCallback->buffer        = result;
            pCallback->listener      = listener;
            callbackList.push_back(pCallback);
        }
        else {
            MY_LOGD_IF(
                mLogLevel >= 1,
                "vOutMeta is zero, streamId:%d requestNo:%d",
                streamId, requestNo
            );
        }
    }
}
