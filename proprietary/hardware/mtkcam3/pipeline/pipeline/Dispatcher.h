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

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_DISPATCHER_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_DISPATCHER_H_
//
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
//
#include <inttypes.h>
//
#include <iomanip>
#include <sstream>
//
#include <cutils/compiler.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>
#include <utils/StrongPointer.h>
//
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class DispatcherBase
    : public virtual IDispatcher
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    android::wp<IDataCallback>              mpDataCallback;

protected:
    mutable android::RWLock                 mFlushLock;
    bool                                    mInFlush = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations

    virtual MVOID   onCtrlSetting(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        StreamId_T const    metaAppStreamId __unused,
                        IMetadata&          rAppMetaData __unused,
                        StreamId_T const    metaHalStreamId __unused,
                        IMetadata&          rHalMetaData __unused,
                        MBOOL&              rIsChanged __unused
                    );

    virtual MVOID   onCtrlSync(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        MUINT32             index __unused,
                        MUINT32             type __unused,
                        MINT64              duration __unused
                    );

    virtual MVOID   onCtrlResize(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        StreamId_T const    metaAppStreamId __unused,
                        IMetadata&          rAppMetaData __unused,
                        StreamId_T const    metaHalStreamId __unused,
                        IMetadata&          rHalMetaData __unused,
                        MBOOL&              rIsChanged __unused
                    );

    virtual MVOID   onCtrlReadout(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        StreamId_T const    metaAppStreamId __unused,
                        IMetadata&          rAppMetaData __unused,
                        StreamId_T const    metaHalStreamId __unused,
                        IMetadata&          rHalMetaData __unused,
                        MBOOL&              rIsChanged __unused
                    );

    virtual MVOID   onEarlyCallback(
                        MUINT32           requestNo,
                        Pipeline_NodeId_T nodeId,
                        StreamId_T        streamId,
                        IMetadata const&  rMetaData,
                        MBOOL             errorResult
                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDispatcher Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations

    virtual int     setDataCallback(android::wp<IDataCallback> pCallback)
                    {
                        mpDataCallback = pCallback;
                        return 0;
                    }

    virtual MVOID   beginFlush()
                    {
                        android::RWLock::AutoWLock _l(mFlushLock);
                        mInFlush = MTRUE;
                    }

    virtual MVOID   endFlush()
                    {
                        android::RWLock::AutoWLock _l(mFlushLock);
                        mInFlush = MFALSE;
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNodeCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations

    virtual auto    onMetaResultAvailable(
                      MetaResultAvailable&& arg
                    ) -> void;

    virtual MBOOL   needCtrlCb(eCtrlType eType);

    virtual MVOID   onNextCaptureCallBack(
                         MUINT32             requestNo __unused,
                         Pipeline_NodeId_T   nodeId __unused,
                         MUINT32             requestCnt __unused,
                         MBOOL               bSkipCheck __unused
                    );

};


/******************************************************************************
 *
 ******************************************************************************/
class DefaultDispatcher
    : public DispatcherBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    mutable android::RWLock         mRWLock;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations

    static  auto    create() -> android::sp<DefaultDispatcher> { return new DefaultDispatcher(); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNodeCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations

    virtual MVOID   onDispatchFrame(
                        android::sp<IPipelineFrame> const& pFrame,
                        Pipeline_NodeId_T nodeId
                    );

};


/******************************************************************************
 *
 ******************************************************************************/
static inline auto makeDispatchString(
    NodeId_T from,
    NodeId_T to,
    const char* postfix
) -> std::ostringstream
{
    std::ostringstream oss;

    if ( eNODEID_UNKNOWN != from ) {
        oss << "0x" << std::setbase(16) << from;
    }
    else {
        oss << "(X)";
    }

    oss << " -> ";

    if ( eNODEID_UNKNOWN != to ) {
        oss << "0x" << std::setbase(16) << to;
    }
    else {
        oss << "(X)";
    }

    if ( postfix ) {
        oss << " " << postfix;
    }

    return oss;
}


/******************************************************************************
 *
 ******************************************************************************/
static inline auto logDispatch(
    IPipelineBufferSetFrameControl* pFrame,
    NodeId_T from,
    NodeId_T to,
    const char* postfix = nullptr
) -> void
{
    if (CC_LIKELY(pFrame != nullptr)) {
        pFrame->logDebugInfo(makeDispatchString(from, to, postfix).str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
struct AutoLogDispatch
{
    nsecs_t const                   mStartTime;
    IPipelineBufferSetFrameControl* mpFrame;
    std::ostringstream              mOss;

    AutoLogDispatch(
        IPipelineBufferSetFrameControl* pFrame,
        NodeId_T from,
        NodeId_T to,
        const char* postfix = nullptr
    )
        : mStartTime(::systemTime())
        , mpFrame(pFrame)
    {
        if (CC_LIKELY(mpFrame != nullptr)) {
            mOss = makeDispatchString(from, to, postfix);
            mpFrame->logDebugInfo(mOss.str() + " +");
        }
    }

    ~AutoLogDispatch()
    {
        if (CC_LIKELY(mpFrame != nullptr)) {
            nsecs_t const elapsedTime = ::ns2us(::systemTime() - mStartTime);
            mpFrame->logDebugInfo(mOss.str() + " - " + std::to_string(elapsedTime) + "us");
        }
    }
};


/******************************************************************************
*
******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_DISPATCHER_H_

