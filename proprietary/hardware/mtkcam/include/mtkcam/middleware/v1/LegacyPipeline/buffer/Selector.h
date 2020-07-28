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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_ZSDSELECTOR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_ZSDSELECTOR_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <map>
#include <unordered_map>
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBufferProvider.h>
#include "StreamBufferProvider.h"
#include <mtkcam/middleware/v1/LegacyPipeline/processor/ResultProcessor.h>
#include <mtkcam/middleware/v1/camutils/IBuffer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/SimpleRawDumpCmdQueThread.h>


typedef NSCam::v3::Utils::HalImageStreamBuffer      HalImageStreamBuffer;
typedef NSCam::v3::IImageStreamInfo                 IImageStreamInfo;
typedef NSCam::v3::Utils::IStreamBufferProvider     IStreamBufferProvider;

using namespace android::MtkCamUtils;
using namespace NSCam::v1;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline{

// indicate that selector keeps arbitrary received result
enum { REQUEST_LATEST = -1 };

/******************************************************************************
 *
 ******************************************************************************/
class SimpleDumpBufferSelector
    : public virtual android::RefBase
{
public:
                                        SimpleDumpBufferSelector();

                                        ~SimpleDumpBufferSelector();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                                setDumpConfig( MINT32 const i4OpenId );
    virtual void                                sendDataToDumpThread(
                                                    MINT32                  rRequestNo,
                                                    Vector<ISelector::MetaItemSet>     rvResult,
                                                    Vector<ISelector::BufferItemSet>   rvBuffers
                                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MBOOL                           mNeedDump;
    MINT32                          mOpenId;
    MUINT32                         mu4MaxRawDumpCnt;
    sp<ISimpleRawDumpCmdQueThread>  mpSimpleRawDumpCmdQueThread;
    MINT32                          mLastStartId;
    MUINT32                         mu4RawDumpCnt;
    MBOOL                           mEnableRawDump;
};

/******************************************************************************
 *
 ******************************************************************************/

// ZsdSelector supports the base function: get the ZSL (Zero Shutter Latency)
// buffer from the buffer container of ZsdSelector. However, ZsdSelector also
// supports other features. Here are the features that ZsdSelector supports:
//
//  - Get a ZSL buffer.
//  - Get a set of continuous buffers.
//  - Sepcify a set of request numbers for waiting, and get them.
//
// Notice that, the buffer size is limited. Which means the queue buffer has
// ability to contain some buffers, and caller can take all buffers at the
// same time makes his feature much faster. (ZsdSelector makes sure this
// buffers are continuous)
class ZsdSelector
    : public BaseSelector
    , public SimpleDumpBufferSelector
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    struct ResultSet_T {
        MINT32                  requestNo;
        Vector<BufferItemSet>   bufferSet;
        Vector<MetaItemSet>     resultMeta;
        MINT64                  timestamp;
        MINT32                  duration;
        MUINT8                  afState;
    };

public:
                                        ZsdSelector();

                                        ~ZsdSelector() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISelector Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual android::status_t           selectResult(
                                            MINT32                          rRequestNo,
                                            Vector<MetaItemSet>             rvResult,
                                            Vector<BufferItemSet>           rvBuffers,
                                            MBOOL                           errorResult
                                        );

    virtual android::status_t           getResult(
                                            MINT32&                          rRequestNo,
                                            Vector<MetaItemSet>&             rvResultMeta,
                                            Vector<BufferItemSet>&           rvBuffers
                                        );

    virtual android::status_t           returnBuffer(
                                            BufferItemSet&    rBuffer
                                        );

    virtual android::status_t           flush();

    virtual android::status_t           sendCommand( MINT32 cmd,
                                                        MINT32 arg1,
                                                        MINT32 arg2,
                                                        MINT32 arg3,
                                                        MVOID* arg4 = NULL
                                                        );

    virtual SelectorType                getSelectorType() const
                                        { return SelectorType_ZsdSelector; }

    virtual android::status_t           getResultCount(
                                            MUINT32&                         rCount
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Get a set of result, if the buffers are not enough, this caller thread will be
    // blocked ( maximum timeoutMs ms * rvRequestNo.size() ) until all buffers are ready.
    // The buffer count is the size of the first argument, rvRequestNo.size().
    //  @param rvRequestNo [in/out]     Caller has to resize a container with the
    //                                  size as buffer cout. If this method returns
    //                                  OK, the request number of these frames will
    //                                  be stored in this container too.
    //  @param rvResultMeta [out]       The output metadata set.
    //  @param rvBuffers [out]          The output image buffers set.
    //  @param timeoutMs [in]           Timeout of this method.
    //  @return                         The operation result.
    //  @reval OK                       OK.
    //  @reval TIMED_OUT                Operation timed out.
    //  @reval NO_MEMORY                Requested size is over-exceed than the
    //                                  size of buffer container.
    //
    //  @complexity                     O(N)
    //  @note This method guarantees all buffers are continuous.
    android::status_t                   getContinuousResults(
                                            Vector<MINT32>&                     rvRequestNo,
                                            Vector< Vector<MetaItemSet> >&      rvResultMeta,
                                            Vector< Vector<BufferItemSet> >&    rvBuffers,
                                            MINT32                              timeoutMs = 3000
                                        );

    // Set a set of requestNo for waiting.
    //  @param vRequestNo [in/out]      A set of request number for waiting. If
    //                                  the request number is already out-of-date,
    //                                  this method will update it as -1.
    //  @return                         If the request number is out-of-date, this
    //                                  method returns NOT_ENOUGH_DATA. And all
    //                                  out-of-date requests will be ignored.
    //  @reval OK                       OK.
    //  @reval NOT_ENOUGH_DATA          There's any request number is out-of-date.
    //  @reval NO_MEMORY                Requested size is over-exceed than the
    //                                  capability of buffer container.
    //  @complexity                     O(N).
    android::status_t                   setWaitRequestNo(
                                            Vector< MINT32 >&   vRequestNo
                                        );

    // Clear a set of the request number for waiting, if there's pending buffer
    // in waiting buffer container, this method will also return them to pool.
    //  @param vRequestNo [in]          A set of request number for waiting that
    //                                  set from ZsdSelector::setWaitRequestNo.
    //                                  It's much better to make sure all pending
    //                                  request has been removed after invoked
    //                                  ZsdSelector::setWaitRequestNo.
    //  @return OK                      OK.
    //  @complexity                     O(N).
    android::status_t                   clearWaitRequestNo(
                                            Vector< MINT32 >&   vRequestNo
                                        );

    // If caller has invoked setWaitRequestNo, to using this method to retrieve
    // the requested frame.
    //  @param requestNo [in]           A request number to wait.
    //  @param rvResultMeta [out]       The returned metadata set.
    //  @param rvBuffers [out]          The returned image buffers set.
    //  @param timeoutMs [in]           Timeout of this method.
    //  @return                         If the requested frame has been dropped,
    //                                  this method returns NOT_ENOUGH_DATA.
    //  @reval OK                       OK.
    //  @reval TIMED_OUT                Operation timed out.
    //  @reval NOT_ENOUGH_DATA          The requested frame has been dropped.
    //  @complexity                     Constant.
    //
    //  @note This method will block caller's thread until timeout or result occurred.
    android::status_t                   getRequestedResult(
                                            const MINT32                requestNo,
                                            Vector<MetaItemSet>&        rvResultMeta,
                                            Vector<BufferItemSet>&      rvBuffers,
                                            MINT32                      timeoutMs = 3000
                                        );


    void                                setDelayTime( MINT32 delayTimeMs, MINT32 tolerationTimeMs);

    // Dynamically change container size. If the target size is smaller than the
    // size in using, buffers in unused container may be cleared, but buffers in
    // requested container won't be cleared. Notice the buffer usage.
    //  @param bufferSize               The size of unused buffer container.
    //  @complexity                     Constant.
    void                                setContainerSize( MINT32 bufferSize );

    // Get the current container size.
    MINT32                              getContainerSize() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // This method will update unused buffer container buffers following min,max buffer size.
    //  @param incomingBuffer           Update buffer including buffer count of incoming buffers.
    //  @complexity                     Constant on average, worst case linear in the size of the container.
    MVOID                               updateQueueBufferLocked(MINT32 incomingBuffer = 0);

    // This method is to remove the record of in-using buffer of ZsdSelector
    MVOID                               removeInUsingBufferRecordLocked(const BufferItemSet& buffer);

    MBOOL                               isOkBuffer(MUINT8 afMode, MUINT8 afState,MUINT8 lensState);

    // Check if this result set is OK for ISP
    MBOOL                               isIspOkBuffer(
                                            MINT32                          requestNo,
                                            const Vector<MetaItemSet>&      metaSet,
                                            const Vector<BufferItemSet>&    bufferSet,
                                            MBOOL                           errorResult
                                        );

    // Removes all exits buffers
    //  @complexity                     O(N).
    MVOID                               removeAllUnusedBufferLocked();
    MVOID                               removeAllRequestedBufferLocked();
    MVOID                               removeExistBackupBuffer();
    MVOID                               insertOkBuffer(ResultSet_T &resultSet);
    MVOID                               insertBackupBuffer(ResultSet_T &resultSet);

    // Check the request number, if it's the buffer we are waiting for, add it
    // to mReqResultMap and return MTRUE, otherwise return MFALSE
    //  @param requestNo                The request number of the buffer.
    //  @param resultSet                The result set to be inserted into the
    //                                  requested buffer container.
    //  @complexity                     Constant.
    android::status_t                   insertRequestedBuffer(MINT32 requestNo, ResultSet_T &resultSet);
    MINT64                              getTimeStamp(Vector<MetaItemSet> &rvResult);
    MBOOL                               getAfInfo(Vector<MetaItemSet> &rvResult, MUINT8 &afMode, MUINT8 &afState, MUINT8 &lensState);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                        onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MINT32                              mDelayTimeMs;
    MINT32                              mContainerSize;
    MINT64                              mLatestResultTsNs;
    MINT32                              mTolerationMs;
    Vector<ResultSet_T>                 mResultSetMap;
    mutable Mutex                       mResultSetLock;
    mutable Condition                   mCondResultSet;
    //
    std::unordered_map<MINT32, ResultSet_T> mReqResultMap;
    std::unordered_map<MINT32, MINT32>      mReqWaitNumbers;
    mutable Condition                       mCondReqResultMap;
    //
    MBOOL                               mIsBackupQueue;
    MBOOL                               mIsNeedWaitAfDone;
    MBOOL                               mIsAlreadyStartedCapture;
    MBOOL                               mIsAllBuffersKept;
    MBOOL                               mIsAllBuffersTransferring;
    MINT32                              mInactiveCount;
    MBOOL                               mUseNewestBuffer;
    uint32_t                            mLensStationaryCount; // counting lens stationary times

protected:
    Vector< Vector<BufferItemSet> >                         mvUsingBufSet;

};

/******************************************************************************
 *
 ******************************************************************************/

class ZsdRequestSelector
    : public BaseSelector
{
public:
                                        ZsdRequestSelector();

                                        ~ZsdRequestSelector() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISelector Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual android::status_t           selectResult(
                                            MINT32                          rRequestNo,
                                            Vector<MetaItemSet>             rvResult,
                                            Vector<BufferItemSet>           rvBuffers,
                                            MBOOL                           errorResult
                                        );

    virtual android::status_t           getResult(
                                            MINT32&                         rRequestNo,
                                            Vector<MetaItemSet>&            rvResultMeta,
                                            Vector<BufferItemSet>&          rvBuffers
                                        );

    virtual android::status_t           returnBuffer(
                                            BufferItemSet&                  rBuffer
                                        );

    virtual android::status_t           flush();

    virtual android::status_t           sendCommand(
                                            MINT32 cmd,
                                            MINT32 arg1,
                                            MINT32 arg2,
                                            MINT32 arg3,
                                            MVOID* arg4 = NULL
                                        );

    virtual SelectorType                getSelectorType() const
                                        { return SelectorType_ZsdRequestSelector; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                        onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                setWaitRequestNo(Vector< MINT32 > requestNo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    struct ResultSet_t {
        MINT32                              requestNo;
        Vector<MetaItemSet>                 resultMeta;
        Vector<BufferItemSet>               bufferSet;
    };
    Vector<ResultSet_t>                 mResultSet;
    Vector< MINT32 >                    mWaitRequestNo;
    mutable Mutex                       mResultSetLock;
    Condition                           mResultQueueCond;

protected:    ////                      timeout handle
    ResultSet_t                         mLatestResult;
    MBOOL                               mFlush;
    // for rrzo keep and transfer hint
    MBOOL                               mIsAllBuffersKept;
    MBOOL                               mIsAllBuffersTransferring;

protected:
    MINT32                              mLogLevel;

};

/******************************************************************************
 *
 ******************************************************************************/
class VssSelector
    : public BaseSelector
    , public SimpleDumpBufferSelector
{
public:
                                        VssSelector();

                                        ~VssSelector() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISelector Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual android::status_t           selectResult(
                                            MINT32                          rRequestNo,
                                            Vector<MetaItemSet>             rvResult,
                                            Vector<BufferItemSet>           rvBuffers,
                                            MBOOL                           errorResult
                                        );

    virtual android::status_t           getResult(
                                            MINT32&                          rRequestNo,
                                            Vector<MetaItemSet>&             rvResultMeta,
                                            Vector<BufferItemSet>&           rvBuffers
                                        );

    virtual android::status_t           returnBuffer(
                                            BufferItemSet&    rBuffer
                                        );

    virtual android::status_t           flush();

    virtual android::status_t           sendCommand(
                                            MINT32                          /*cmd*/,
                                            MINT32                          /*arg1*/,
                                            MINT32                          /*arg2*/,
                                            MINT32                          /*arg3*/,
                                            MVOID*                          /*arg4*/
                                        ) { return OK; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                        onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    struct ResultSet_T {
        MINT32                        requestNo;
        Vector<BufferItemSet>         bufferSet;
        Vector<MetaItemSet>           resultMeta;
    };

    Vector<ResultSet_T>                 mResultSetMap;
    mutable Mutex                       mResultSetLock;
    mutable Condition                   mCondResultSet;

};


/******************************************************************************
 *
 ******************************************************************************/
class DumpBufferSelector
    : public BaseSelector
    , public SimpleDumpBufferSelector
{
public:
                                        DumpBufferSelector();

                                        ~DumpBufferSelector();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISelector Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual android::status_t           selectResult(
                                            MINT32                          rRequestNo,
                                            Vector<MetaItemSet>             rvResult,
                                            Vector<BufferItemSet>           rvBuffers,
                                            MBOOL                           errorResult
                                        );

    virtual android::status_t           getResult(
                                            MINT32&                          /* rRequestNo */,
                                            Vector<MetaItemSet>&             /* rvResultMeta */,
                                            Vector<BufferItemSet>&           /* rvBuffers */
                                        ){ return OK; }

    virtual android::status_t           returnBuffer(
                                            BufferItemSet&    rBuffer
                                        );

    virtual android::status_t           flush();

    virtual android::status_t           sendCommand(
                                            MINT32                          /* cmd */,
                                            MINT32                          /* arg1 */,
                                            MINT32                          /* arg2 */,
                                            MINT32                          /* arg3 */,
                                            MVOID*                          /* arg4 */
                                        ) { return OK; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                        onLastStrongRef( const void* /*id*/);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    struct ResultSet_T {
        MUINT32                       requestNo;
        Vector<BufferItemSet>         bufferSet;
        Vector<MetaItemSet>           resultMeta;
    };
    //
    Vector<ResultSet_T>                 mResultSetMap;
    mutable Mutex                       mResultSetLock;
    mutable Condition                   mCondResultSet;
    //
};

/******************************************************************************
 *
 ******************************************************************************/
class DualCamSelector
    : public BaseSelector
{
public:
                                        DualCamSelector(MINT32 openId);

                                        ~DualCamSelector();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISelector Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual android::status_t           selectResult(
                                            MINT32                           requestNo,
                                            Vector<MetaItemSet>              vMetaSet,
                                            Vector<BufferItemSet>            vBufferSet,
                                            MBOOL                            errorResult
                                        );

    virtual android::status_t           getResult(
                                            MINT32&                          rRequestNo,
                                            Vector<MetaItemSet>&             rvMetaSet,
                                            Vector<BufferItemSet>&           rvBufferSet
                                        );

    virtual android::status_t           returnBuffer(
                                            BufferItemSet&    rBuffer
                                        );

    virtual android::status_t           flush();

    virtual android::status_t           sendCommand(
                                            MINT32                          /*cmd*/,
                                            MINT32                          /*arg1*/,
                                            MINT32                          /*arg2*/,
                                            MINT32                          /*arg3*/,
                                            MVOID*                          /*arg4*/
                                        ) { return OK; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualCamSelecot Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    MVOID                               setRequests(Vector<MINT32> requests);

    MVOID                               setRequests(Vector<MINT32> requests, Vector<MINT32> requests_group2);

    MVOID                               setExpectedCount(MINT32 count);

    MVOID                               setExpectedCount(MINT32 count, MINT32 count_group2);

    MVOID                               setOriginSelector(sp<ISelector> pOriSelector);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    virtual void                        onLastStrongRef(const void*         /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
                                        class Implementor;
    Implementor*                        mpImp;

};



};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_ZSDSELECTOR_H_

