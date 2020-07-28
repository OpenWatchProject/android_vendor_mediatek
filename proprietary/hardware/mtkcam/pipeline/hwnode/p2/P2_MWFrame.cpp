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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2_MWFrame.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWFrame
#define P2_TRACE        TRACE_MW_FRAME
#include "P2_LogHeader.h"

namespace P2
{

MWFrame::MWFrame(const IPipelineNode::NodeId_T &nodeID, const NodeName_T &nodeName, const sp<IPipelineFrame> &frame, const Logger &logger)
    : LoggerHolder(logger)
    , mNodeID(nodeID)
    , mNodeName(nodeName)
    , mFrame(frame)
    , mDirty(MFALSE)
    , mBatchMode(0)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( ATRACE_ENABLED() )
    {
        mTraceName = android::String8::format("Cam:%d:IspP2|%d|request:%d frame:%d", mLogger->getLogSensorID(), mFrame->getRequestNo(), mFrame->getRequestNo(), mFrame->getFrameNo());
        CAM_TRACE_ASYNC_BEGIN(mTraceName.string(), 0);
    }
    CAM_TRACE_ASYNC_BEGIN("P2_MWFrame", mFrame->getFrameNo());
    TRACE_S_FUNC_EXIT(mLogger);
}

MWFrame::~MWFrame()
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( mDirty )
    {
        doRelease();
    }
    dispatchFrame(mFrame, mNodeID, getLogger());
    if( ATRACE_ENABLED() )
    {
        CAM_TRACE_ASYNC_END(mTraceName.string(), 0);
    }
    CAM_TRACE_ASYNC_END("P2_MWFrame", mFrame->getFrameNo());
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::dispatchFrame(const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID, const Logger &logger)
{
    (void)logger;
    TRACE_S_FUNC_ENTER(logger);
    CAM_TRACE_NAME("P2:DispatchFrame");
    sp<IPipelineNodeCallback> cb;
    if( frame != NULL )
    {
        cb = frame->getPipelineNodeCallback();
        if( cb != NULL )
        {
            cb->onDispatchFrame(frame, nodeID);
        }
    }
    TRACE_S_FUNC_EXIT(logger);
}

MVOID releaseStream(const IPipelineNode::NodeId_T &nodeID, IStreamBufferSet &streamBufferSet, const sp<const IStreamInfoSet> &stream, const Logger &logger)
{
    (logger);
    TRACE_S_FUNC_ENTER(logger);
    sp<IStreamInfoSet::IMap<IMetaStreamInfo>> meta;
    sp<IStreamInfoSet::IMap<IImageStreamInfo>> img;
    if( stream != NULL )
    {
        meta = stream->getMetaInfoMap();
        img = stream->getImageInfoMap();
    }
    for(unsigned i = 0, n = (meta != NULL ? meta->size() : 0); i < n; ++i )
    {
        StreamId_T sID = meta->valueAt(i)->getStreamId();
        sp<IStreamBuffer> buffer = streamBufferSet.getMetaBuffer(sID, nodeID);
        if( buffer != NULL )
        {
            buffer->markUserStatus(nodeID, IUsersManager::UserStatus::RELEASE);
        }
    }
    for(unsigned i = 0, n = (img != NULL ? img->size() : 0); i < n; ++i )
    {
        StreamId_T sID = img->valueAt(i)->getStreamId();
        sp<IStreamBuffer> buffer = streamBufferSet.getImageBuffer(sID, nodeID);
        if( buffer != NULL )
        {
            buffer->markUserStatus(nodeID, IUsersManager::UserStatus::RELEASE);
        }
    }
    streamBufferSet.applyRelease(nodeID);
    TRACE_S_FUNC_EXIT(logger);
}

MVOID MWFrame::releaseFrameStream(const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    sp<const IStreamInfoSet> iStream, oStream;
    if( frame == NULL )
    {
        MY_S_LOGW(logger, "invalid frame = NULL");
    }
    else if( 0 != frame->queryIOStreamInfoSet(nodeID, iStream, oStream) )
    {
        MY_S_LOGW(logger, "queryIOStreamInfoSet failed");
    }
    else
    {
        IStreamBufferSet &streamBufferSet = frame->getStreamBufferSet();
        releaseStream(nodeID, streamBufferSet, iStream, logger);
        releaseStream(nodeID, streamBufferSet, oStream, logger);
    }
    TRACE_S_FUNC_EXIT(logger);
}

MVOID MWFrame::flushFrame(const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    if( frame == NULL )
    {
        MY_S_LOGW(logger, "invalid frame = NULL");
    }
    else
    {
        MWFrame::releaseFrameStream(frame, nodeID, logger);
        MWFrame::dispatchFrame(frame, nodeID, logger);
    }
    TRACE_S_FUNC_EXIT(logger);
}

MUINT32 MWFrame::getMWFrameID() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MUINT32 id = 0;
    if( mFrame != NULL )
    {
        id = mFrame->getFrameNo();
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return id;
}

MUINT32 MWFrame::getMWFrameRequestID() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MUINT32 id = 0;
    if( mFrame != NULL )
    {
        id = mFrame->getRequestNo();
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return id;
}

MUINT32 MWFrame::getFrameID() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mLogger->getLogFrameID();
}

MVOID MWFrame::notifyRelease()
{
    TRACE_S_FUNC_ENTER(mLogger);
    android::Mutex::Autolock _lock(mMutex);
    if( !mBatchMode )
    {
        doRelease();
    }
    else
    {
        mDirty = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::beginBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLogger);
    android::Mutex::Autolock _lock(mMutex);
    ++mBatchMode;
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::endBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLogger);
    android::Mutex::Autolock _lock(mMutex);
    --mBatchMode;
    if( !mBatchMode && mDirty )
    {
        doRelease();
        mDirty = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MBOOL MWFrame::getInfoIOMapSet(IPipelineFrame::InfoIOMapSet &ioMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    ret = (0 ==  mFrame->queryInfoIOMapSet(mNodeID, ioMap));
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

sp<IMetaStreamBuffer> MWFrame::acquireMetaStream(const StreamId_T &sID) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<IMetaStreamBuffer> streamBuffer;
    IStreamBufferSet &bufferSet = mFrame->getStreamBufferSet();
    streamBuffer = bufferSet.getMetaBuffer(sID, mNodeID);
    if( !validateStream(sID, bufferSet, streamBuffer) )
    {
        streamBuffer = NULL;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return streamBuffer;
}

sp<IImageStreamBuffer> MWFrame::acquireImageStream(const StreamId_T &sID) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<IImageStreamBuffer> streamBuffer;
    IStreamBufferSet &bufferSet = mFrame->getStreamBufferSet();
    streamBuffer = bufferSet.getImageBuffer(sID, mNodeID);
    if( !validateStream(sID, bufferSet, streamBuffer) )
    {
        streamBuffer = NULL;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return streamBuffer;
}

IMetadata* MWFrame::acquireMeta(const sp<IMetaStreamBuffer> &stream, IO_DIR dir) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    IMetadata *meta = NULL;
    if( stream != NULL )
    {
        meta = ( dir & IO_DIR_OUT ) ? stream->tryWriteLock(mNodeName)
                                    : stream->tryReadLock(mNodeName);
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return meta;
}

sp<IImageBuffer> MWFrame::acquireImage(const sp<IImageStreamBuffer> &stream, IO_DIR dir) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<IImageBuffer> image;
    if( stream != NULL )
    {
        sp<IImageBufferHeap> heap;
        heap = ( dir & IO_DIR_OUT ) ? stream->tryWriteLock(mNodeName)
                                    : stream->tryReadLock(mNodeName);

        if( heap == NULL )
        {
            MY_S_LOGW(mLogger, "streamBuffer->tryLock() failed");
        }
        else
        {
            image = heap->createImageBuffer();
            if( image == NULL )
            {
                MY_S_LOGW(mLogger, "heap->createImageBuffer() failed");
            }
            else
            {
                MUINT32 usage = stream->queryGroupUsage(mNodeID);
                image->lockBuf(mNodeName, usage);
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return image;
}

MVOID MWFrame::releaseMeta(const sp<IMetaStreamBuffer> &stream, IMetadata *meta) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( stream != NULL && meta )
    {
        stream->unlock(mNodeName, meta);
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::releaseMetaStream(const sp<IMetaStreamBuffer> &stream, IO_DIR dir, IO_STATUS status)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( stream != NULL )
    {
        if( dir & IO_DIR_OUT )
        {
            stream->markStatus(toStreamBufferStatus(status));
        }
        StreamId_T streamID = stream->getStreamInfo()->getStreamId();
        mFrame->getStreamBufferSet().markUserStatus(streamID, mNodeID, toUserStatus(status));
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::releaseImage(const sp<IImageStreamBuffer> &stream, const sp<IImageBuffer> &image) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( stream != NULL && image != NULL )
    {
        image->unlockBuf(mNodeName);
        stream->unlock(mNodeName, image->getImageBufferHeap());
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::releaseImageStream(const sp<IImageStreamBuffer> &stream, IO_DIR dir, IO_STATUS status)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( stream != NULL )
    {
        if( dir & IO_DIR_OUT )
        {
            stream->markStatus(toStreamBufferStatus(status));
        }
        StreamId_T streamID = stream->getStreamInfo()->getStreamId();
        mFrame->getStreamBufferSet().markUserStatus(streamID, mNodeID, toUserStatus(status));
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::doRelease()
{
    TRACE_S_FUNC_ENTER(mLogger);
    CAM_TRACE_NAME("P2:ApplyRelease");
    mFrame->getStreamBufferSet().applyRelease(mNodeID);
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrame::acquireFence(const sp<IStreamBuffer> &stream) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<IFence> fence = IFence::create(stream->createAcquireFence(mNodeID));
    NSCam::MERROR ret = fence->waitForever(mNodeName);
    if( ret != 0 )
    {
        MY_S_LOGE(mLogger, "acquireFence->waitForever() failed buffer:%s fence:%d[%s] err:%d[%s]", stream->getName(), fence->getFd(), fence->name(), ret, ::strerror(-ret));
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MBOOL MWFrame::validateStream(const StreamId_T &sID, IStreamBufferSet &bufferSet, const sp<IStreamBuffer> &stream, MBOOL acquire) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MTRUE;
    if( stream == NULL )
    {
        MY_S_LOGD(mLogger, "stream [%zd] getStreamBuffer() failed", sID);
        ret = MFALSE;
    }
    else
    {
        if( acquire )
        {
            acquireFence(stream);
            bufferSet.markUserStatus(sID, mNodeID, IUsersManager::UserStatus::ACQUIRE);
        }
        if( stream->hasStatus(STREAM_BUFFER_STATUS::ERROR) )
        {
            MY_S_LOGW(mLogger, "stream buffer:%s bad status:%u", stream->getName(), stream->getStatus());
            bufferSet.markUserStatus(sID, mNodeID, IUsersManager::UserStatus::RELEASE);
            ret = MFALSE;
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MUINT32 MWFrame::toStreamBufferStatus(IO_STATUS status) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return (status == IO_STATUS_OK) ? STREAM_BUFFER_STATUS::WRITE_OK :
                                      STREAM_BUFFER_STATUS::WRITE_ERROR;
}

MUINT32 MWFrame::toUserStatus(IO_STATUS status) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MUINT32 userStatus = IUsersManager::UserStatus::RELEASE;
    userStatus |= (status == IO_STATUS_INVALID) ? 0 : IUsersManager::UserStatus::USED;
    TRACE_S_FUNC_EXIT(mLogger);
    return userStatus;
}

} // namespace P2
