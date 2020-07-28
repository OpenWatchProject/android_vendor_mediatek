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

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace P2
{

MWFrame::ReleaseToken::ReleaseToken(const ILog &log, const NodeName_T &nodeName, const IPipelineNode::NodeId_T &nodeID)
    : mLog(log)
    , mNodeName(nodeName)
    , mNodeID(nodeID)
{

}

MWFrame::ReleaseToken::~ReleaseToken()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mIsPreRelease )
    {
        LogString str;
        refreshStreamStatus(str);
        MY_S_LOGD(mLog, "%s", str.c_str());
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::ReleaseToken::registerStream(const StreamId_T &sID, const std::string &name)
{
    TRACE_S_FUNC_ENTER(mLog);
    mMWStreamMap[sID] = MWStreamInfo(name, ReleaseToken::STATE_USING);
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::ReleaseToken::update(const StreamId_T &sID, const StreamState &status)
{
    TRACE_S_FUNC_ENTER(mLog, "sID:0x%09" PRIx64 "(%s) status:0x%x",
                        sID, (mMWStreamMap.count(sID) > 0) ? mMWStreamMap[sID].mName.c_str() : "??", status);
    if( status == STATE_PRERELEASED )
    {
        mIsPreRelease = MTRUE;
    }
    mMWStreamMap[sID].mState = status;
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::ReleaseToken::refreshStreamStatus(LogString &str)
{
    TRACE_S_FUNC_ENTER(mLog);
    MUINT32 usingStream = 0;
    for(auto &it : mMWStreamMap)
    {
        if( (it.second.mState == STATE_USING) ||
            (it.second.mState == STATE_PRERELEASED) )
        {
            ++usingStream;
        }
    }
    str.append("all streams(%zu), using(%u), status: ",
               mMWStreamMap.size(), usingStream);
    for(auto &it : mMWStreamMap)
    {
        str.append("%s(%u),", it.second.mName.c_str(), it.second.mState );
        if( it.second.mState == STATE_RELEASING )
        {
            it.second.mState = STATE_RELEASED;
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MWFrame::MWFrame(const ILog &log, const IPipelineNode::NodeId_T &nodeID, const NodeName_T &nodeName, const sp<IPipelineFrame> &frame)
    : mLog(log)
    , mNodeID(nodeID)
    , mNodeName(nodeName)
    , mFrame(frame)
    , mDirty(MFALSE)
    , mBatchMode(0)
{
    TRACE_S_FUNC_ENTER(mLog);
    if( ATRACE_ENABLED() )
    {
        mTraceName = android::String8::format("Cam:%d:IspP2|%d|request:%d frame:%d", mLog.getLogSensorID(), mFrame->getRequestNo(), mFrame->getRequestNo(), mFrame->getFrameNo());
        P2_CAM_TRACE_ASYNC_BEGIN(TRACE_DEFAULT, mTraceName.string(), 0);
    }
    mReleaseToken = new ReleaseToken(mLog, mNodeName, mNodeID);
    P2_CAM_TRACE_ASYNC_BEGIN(TRACE_ADVANCED, "P2_MWFrame", mFrame->getFrameNo());
    TRACE_S_FUNC_EXIT(mLog);
}

MWFrame::~MWFrame()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mDirty )
    {
        doRelease();
    }
    dispatchFrame(mLog, mFrame, mNodeID);
    if( ATRACE_ENABLED() )
    {
        P2_CAM_TRACE_ASYNC_END(TRACE_DEFAULT, mTraceName.string(), 0);
    }
    P2_CAM_TRACE_ASYNC_END(TRACE_ADVANCED, "P2_MWFrame", mFrame->getFrameNo());
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::dispatchFrame(const ILog &log, const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID)
{
    (void)log;
    TRACE_S_FUNC_ENTER(log);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2:DispatchFrame");
    sp<IPipelineNodeCallback> cb;
    if( frame != NULL )
    {
        cb = frame->getPipelineNodeCallback();
        if( cb != NULL )
        {
            cb->onDispatchFrame(frame, nodeID);
        }
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID releaseStream(const ILog &log, const IPipelineNode::NodeId_T &nodeID, IStreamBufferSet &streamBufferSet, const sp<const IStreamInfoSet> &stream)
{
    (void)log;
    TRACE_S_FUNC_ENTER(log);
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
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::releaseFrameStream(const ILog &log, const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID)
{
    TRACE_S_FUNC_ENTER(log);
    sp<const IStreamInfoSet> iStream, oStream;
    if( frame == NULL )
    {
        MY_S_LOGW(log, "invalid frame = NULL");
    }
    else if( 0 != frame->queryIOStreamInfoSet(nodeID, iStream, oStream) )
    {
        MY_S_LOGW(log, "queryIOStreamInfoSet failed");
    }
    else
    {
        MY_S_LOGI(log, "Node%#" PRIxPTR " Flush FrameNo(%d)", nodeID, frame->getFrameNo());
        IStreamBufferSet &streamBufferSet = frame->getStreamBufferSet();
        releaseStream(log, nodeID, streamBufferSet, iStream);
        releaseStream(log, nodeID, streamBufferSet, oStream);
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::flushFrame(const ILog &log, const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID)
{
    TRACE_S_FUNC_ENTER(log);
    if( frame == NULL )
    {
        MY_S_LOGW(log, "invalid frame = NULL");
    }
    else
    {
        MWFrame::releaseFrameStream(log, frame, nodeID);
        MWFrame::dispatchFrame(log, frame, nodeID);
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::releaseMeta(const sp<ReleaseToken> &releaseToken, const sp<IMetaStreamBuffer> &stream, IMetadata *meta)
{
    ILog log = spToILog(releaseToken);
    TRACE_S_FUNC_ENTER(log, "name:%s", (stream != NULL) ? stream->getName() : "??");
    if( releaseToken != NULL && stream != NULL && meta )
    {
        stream->unlock(releaseToken->mNodeName, meta);
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::releaseMetaStream(const sp<ReleaseToken> &releaseToken, const sp<IMetaStreamBuffer> &stream, IO_DIR dir, IO_STATUS status)
{
    ILog log = spToILog(releaseToken);
    TRACE_S_FUNC_ENTER(log, "name:%s, dir:0x%x, status:0x%x", (stream != NULL) ? stream->getName() : "??", dir, status);
    if( releaseToken != NULL && stream != NULL )
    {
        if( dir & IO_DIR_OUT )
        {
            stream->markStatus(toStreamBufferStatus(log, status));
        }
        StreamId_T streamID = stream->getStreamInfo()->getStreamId();
        releaseToken->update(streamID, ReleaseToken::STATE_RELEASING);
        stream->markUserStatus(releaseToken->mNodeID, toUserStatus(log, status));
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::releaseImage(const sp<ReleaseToken> &releaseToken, const std::vector<sp<IImageBuffer>> &images)
{
    ILog log = spToILog(releaseToken);
    TRACE_S_FUNC_ENTER(log);
    if( releaseToken != NULL )
    {
        for( const sp<IImageBuffer> &buf : images )
        {
            if( buf != NULL )
            {
                buf->unlockBuf(releaseToken->mNodeName);
            }
        }
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::releaseImageHeap(const sp<ReleaseToken> &releaseToken, const sp<IImageStreamBuffer> &stream, const sp<IImageBufferHeap> &heap)
{
    ILog log = spToILog(releaseToken);
    TRACE_S_FUNC_ENTER(log, "name:%s", (stream != NULL) ? stream->getName() : "??");
    if( heap != NULL && releaseToken != NULL && stream != NULL )
    {
        stream->unlock(releaseToken->mNodeName, heap.get());
    }
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWFrame::releaseImageStream(const sp<ReleaseToken> &releaseToken, const sp<IImageStreamBuffer> &stream, IO_DIR dir, IO_STATUS status)
{
    ILog log = spToILog(releaseToken);
    TRACE_S_FUNC_ENTER(log, "name:%s, dir:0x%x, status::0x%x", (stream != NULL) ? stream->getName() : "??", dir, status);
    if( releaseToken != NULL && stream != NULL )
    {
        if( dir & IO_DIR_OUT )
        {
            stream->markStatus(toStreamBufferStatus(log, status));
        }
        StreamId_T streamID = stream->getStreamInfo()->getStreamId();
        releaseToken->update(streamID, ReleaseToken::STATE_RELEASING);
        stream->markUserStatus(releaseToken->mNodeID, toUserStatus(log, status));
    }
    TRACE_S_FUNC_EXIT(log);
}

MUINT32 MWFrame::getMWFrameID() const
{
    TRACE_S_FUNC_ENTER(mLog);
    MUINT32 id = 0;
    if( mFrame != NULL )
    {
        id = mFrame->getFrameNo();
    }
    TRACE_S_FUNC_EXIT(mLog);
    return id;
}

MUINT32 MWFrame::getMWFrameRequestID() const
{
    TRACE_S_FUNC_ENTER(mLog);
    MUINT32 id = 0;
    if( mFrame != NULL )
    {
        id = mFrame->getRequestNo();
    }
    TRACE_S_FUNC_EXIT(mLog);
    return id;
}

MUINT32 MWFrame::getFrameID() const
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return mLog.getLogFrameID();
}

MVOID MWFrame::notifyRelease()
{
    TRACE_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    if( !mBatchMode )
    {
        doRelease();
    }
    else
    {
        mDirty = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::beginBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    ++mBatchMode;
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::endBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mMutex);
    --mBatchMode;
    if( !mBatchMode && mDirty )
    {
        doRelease();
        mDirty = MFALSE;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::notifyNextCapture(int requestCnt, MBOOL bSkipCheck)
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IPipelineNodeCallback> cb = mFrame->getPipelineNodeCallback();
    if( cb != NULL )
    {
        cb->onNextCaptureCallBack(mFrame->getRequestNo(), mNodeID, requestCnt, bSkipCheck);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::metaResultAvailable(IMetadata* partialMeta)
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IPipelineNodeCallback> cb = mFrame->getPipelineNodeCallback();
    if( cb != NULL ) {
        cb->onMetaResultAvailable({partialMeta, mFrame, mNodeID, mNodeName.string()});
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL MWFrame::getInfoIOMapSet(IPipelineFrame::InfoIOMapSet &ioMap)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    ret = (0 ==  mFrame->queryInfoIOMapSet(mNodeID, ioMap));
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

sp<IMetaStreamBuffer> MWFrame::acquireMetaStream(const StreamId_T &sID)
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IMetaStreamBuffer> streamBuffer;
    IStreamBufferSet &bufferSet = mFrame->getStreamBufferSet();
    streamBuffer = bufferSet.getMetaBuffer(sID, mNodeID);
    if( !validateStream(sID, bufferSet, streamBuffer) )
    {
        streamBuffer = NULL;
    }
    else
    {
        const IStreamInfo *sInfo = streamBuffer->getStreamInfo();
        if(sInfo != NULL)
        {
            mReleaseToken->registerStream(sID, sInfo->getStreamName());
        }
        else
        {
            MY_S_LOGE(mLog,"error getStreamInfo return null");
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return streamBuffer;
}

sp<IImageStreamBuffer> MWFrame::acquireImageStream(const StreamId_T &sID)
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IImageStreamBuffer> streamBuffer;
    IStreamBufferSet &bufferSet = mFrame->getStreamBufferSet();
    streamBuffer = bufferSet.getImageBuffer(sID, mNodeID);
    if( !validateStream(sID, bufferSet, streamBuffer) )
    {
        streamBuffer = NULL;
    }
    else
    {
        const IStreamInfo *sInfo = streamBuffer->getStreamInfo();
        if(sInfo != NULL)
        {
            mReleaseToken->registerStream(sID, sInfo->getStreamName());
        }
        else
        {
            MY_S_LOGE(mLog,"error getStreamInfo return null");
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return streamBuffer;
}

IMetadata* MWFrame::acquireMeta(const sp<IMetaStreamBuffer> &stream, IO_DIR dir) const
{
    TRACE_S_FUNC_ENTER(mLog);
    IMetadata *meta = NULL;
    if( stream != NULL )
    {
        meta = ( dir & IO_DIR_OUT ) ? stream->tryWriteLock(mNodeName)
                                    : stream->tryReadLock(mNodeName);
        if(meta == NULL)
        {
            MY_S_LOGW(mLog, "(%s)metaStreamBuffer->tryLock() failed", stream->getName());
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return meta;
}

sp<IImageBufferHeap> MWFrame::acquireImageHeap(const sp<IImageStreamBuffer> &stream, IO_DIR dir) const
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IImageBufferHeap> heap;
    if( stream != NULL )
    {
        heap = ( dir & IO_DIR_OUT ) ? stream->tryWriteLock(mNodeName)
                                    : stream->tryReadLock(mNodeName);
        if( heap == NULL )
        {
            MY_S_LOGW(mLog, "(%s)imageStreamBuffer->tryLock() failed dir:0x%x", stream->getName(), dir);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return heap;
}

std::vector<sp<IImageBuffer>> MWFrame::acquireImage(const sp<IImageStreamBuffer> &stream, const sp<IImageBufferHeap> &heap,
                                                    const NSCam::ImageBufferInfo &blobInfo, IO_DIR dir, MBOOL needSWRW, MBOOL isOpaque) const
{
    TRACE_S_FUNC_ENTER(mLog);
    std::vector<sp<IImageBuffer>> images;
    if( stream != NULL && heap != NULL)
    {
        MBOOL isBlob = (heap->getImgFormat() == NSCam::eImgFmt_BLOB);
        if( isOpaque )
        {
            createOpaqueImages(heap, images);
        }
        else if ( isBlob )
        {
            createBlobImages(heap, blobInfo, images);
        }
        else
        {
            createNormalImages(heap, images);
        }

        if( images.size() == 0 || images[0] == NULL )
        {
            MY_S_LOGW(mLog, "(%s) heap->createImageBuffer() failed isOpaque:%d isBlob:%d count:%zu", stream->getName(), isOpaque, isBlob, images.size());
        }

        MUINT32 usage = stream->queryGroupUsage(mNodeID);
        if( needSWRW && (dir & IO_DIR_OUT) )
        {
            usage |= NSCam::eBUFFER_USAGE_SW_MASK;
        }
        for( sp<IImageBuffer> &buf : images )
        {
            if( buf != NULL )
            {
                if( !buf->lockBuf(mNodeName, usage) )
                {
                    MY_S_LOGW(mLog, "(%s) image buffer lock usage(0x%x) failed", stream->getName(), usage);
                    // releaseImage(stream, buf);
                    buf = NULL;
                }
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return images;
}

MVOID MWFrame::preReleaseMetaStream(const sp<IMetaStreamBuffer> &stream, IO_DIR dir)
{
    (void)dir;
    TRACE_S_FUNC_ENTER(mLog, "name:%s, dir:0x%x", (stream != NULL) ? stream->getName() : "??", dir);
    if( stream != NULL )
    {
        const IStreamInfo *sInfo = stream->getStreamInfo();
        if(sInfo != NULL)
        {
            StreamId_T streamID = sInfo->getStreamId();
            if( streamID == NSCam::eSTREAMID_META_APP_DYNAMIC_02_CAP )
            {
                // in pre-release flow, we only need to mark appOutStreaming as ERROR
                TRACE_S_FUNC(mLog, "force mark stream, eSTREAMID_META_APP_DYNAMIC_02_CAP, status as ERROR ");
                stream->markStatus(STREAM_BUFFER_STATUS::ERROR);
            }
            mReleaseToken->update(streamID, ReleaseToken::STATE_PRERELEASED);
            stream->markUserStatus(mNodeID, IUsersManager::UserStatus::PRE_RELEASE);
            mIsPreRelease = MTRUE;
        }
        else
        {
            MY_S_LOGE(mLog,"error getStreamInfo return null");
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::preReleaseImageStream(const sp<IImageStreamBuffer> &stream, IO_DIR dir)
{
    (void)dir;
    TRACE_S_FUNC_ENTER(mLog, "name:%s, dir:0x%x", (stream != NULL) ? stream->getName() : "??", dir);
    if( stream != NULL )
    {
        const StreamId_T streamID = stream->getStreamInfo()->getStreamId();
        mReleaseToken->update(streamID, ReleaseToken::STATE_PRERELEASED);
        stream->markUserStatus(mNodeID, IUsersManager::UserStatus::PRE_RELEASE);
        mIsPreRelease = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

sp<MWFrame::ReleaseToken> MWFrame::getReleaseToken()
{
    return mReleaseToken;
}

MVOID MWFrame::print(const ILog &log, const sp<IMetaStreamInfo> &info, StreamId_T id, unsigned s, unsigned i, const char* io)
{
    const char *name = "NA";
    if( info != NULL )
    {
        name = info->getStreamName();
    }
    MY_S_LOGD(log, "StreamInfo: metaSet[%d].%s[%d: 0x%09" PRIx64 "] %s", s, io, i, id, name);
}

MVOID MWFrame::print(const ILog &log, const sp<IImageStreamInfo> &info, StreamId_T id, unsigned s, unsigned i, const char* io)
{
    const char *name = "NA";
    MINT physicalID = -1;
    MINT imgFmt = 0;
    MSize imgSize;
    if( info != NULL )
    {
        name = info->getStreamName();
        imgFmt = info->getImgFormat();
        imgSize = info->getImgSize();
        physicalID = info->getPhysicalCameraId();
    }
    MY_S_LOGD(log, "StreamInfo: imgSet[%d].%s[%d: 0x%09" PRIx64 "] %s (%dx%d) (fmt:0x%08x) phy(%d)", s, io, i, id, name, imgSize.w, imgSize.h, imgFmt, physicalID);
}

MVOID MWFrame::print(const ILog &log, const IPipelineFrame::InfoIOMapSet &ioMap)
{
    const IPipelineFrame::ImageInfoIOMapSet &imgs = ioMap.mImageInfoIOMapSet;
    const IPipelineFrame::MetaInfoIOMapSet &metas = ioMap.mMetaInfoIOMapSet;

    for( unsigned i = 0, size = imgs.size(); i < size; ++i )
    {
        for( unsigned in = 0, inSize = imgs[i].vIn.size(); in < inSize; ++in )
        {
            print(log, imgs[i].vIn.valueAt(in), imgs[i].vIn.keyAt(in), i, in, "in");
        }
        for( unsigned out = 0, outSize = imgs[i].vOut.size(); out < outSize; ++out )
        {
            print(log, imgs[i].vOut.valueAt(out), imgs[i].vOut.keyAt(out), i, out, "out");
        }
    }

    for( unsigned i = 0, size = metas.size(); i < size; ++i )
    {
        for( unsigned in = 0, inSize = metas[i].vIn.size(); in < inSize; ++in )
        {
            print(log, metas[i].vIn.valueAt(in), metas[i].vIn.keyAt(in), i, in, "in");
        }
        for( unsigned out = 0, outSize = metas[i].vOut.size(); out < outSize; ++out )
        {
            print(log, metas[i].vOut.valueAt(out), metas[i].vOut.keyAt(out), i, out, "out");
        }
    }
}

MVOID MWFrame::createOpaqueImages(const sp<IImageBufferHeap> &heap, std::vector<sp<IImageBuffer>> &images) const
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IImageBuffer> image;
    heap->lockBuf(mNodeName);
    OpaqueReprocUtil::getImageBufferFromHeap(heap, image);
    heap->unlockBuf(mNodeName);
    images.push_back(image);
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::createBlobImages(const sp<IImageBufferHeap> &heap, const NSCam::ImageBufferInfo &blobInfo, std::vector<sp<IImageBuffer>> &images) const
{
    TRACE_S_FUNC_ENTER(mLog);
    std::vector<IImageBuffer*> ptrs;
    ptrs = heap->createImageBuffers_FromBlobHeap(blobInfo, mNodeName);
    size_t size = ptrs.size();
    images.resize(size);
    for( size_t i = 0; i < size; ++i )
    {
        images[i] = ptrs[i];
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::createNormalImages(const sp<IImageBufferHeap> &heap, std::vector<sp<IImageBuffer>> &images) const
{
    TRACE_S_FUNC_ENTER(mLog);
    images.push_back(heap->createImageBuffer());
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::doRelease()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2:ApplyRelease");
    LogString str;
    mReleaseToken->refreshStreamStatus(str);
    MY_S_LOGD(mLog, "%s", str.c_str());
    if( mIsPreRelease )
    {
        mFrame->getStreamBufferSet().applyPreRelease(mNodeID);
    }
    else
    {
        mFrame->getStreamBufferSet().applyRelease(mNodeID);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrame::acquireFence(const sp<IStreamBuffer> &stream) const
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<IFence> fence = IFence::create(stream->createAcquireFence(mNodeID));
    NSCam::MERROR ret = fence->waitForever(mNodeName);
    if( ret != 0 )
    {
        MY_S_LOGE(mLog, "acquireFence->waitForever() failed buffer:%s fence:%d[%s] err:%d[%s]", stream->getName(), fence->getFd(), fence->name(), ret, ::strerror(-ret));
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL MWFrame::validateStream(const StreamId_T &sID, IStreamBufferSet &bufferSet, const sp<IStreamBuffer> &stream, MBOOL acquire) const
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MTRUE;
    if( stream == NULL )
    {
        MY_S_LOGW(mLog, "stream [0x%09" PRIx64 "] getStreamBuffer() failed", sID);
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
            MY_S_LOGW(mLog, "stream buffer:%s bad status:%u", stream->getName(), stream->getStatus());
            bufferSet.markUserStatus(sID, mNodeID, IUsersManager::UserStatus::RELEASE);
            ret = MFALSE;
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MUINT32 MWFrame::toStreamBufferStatus(const ILog &log, IO_STATUS status)
{
    TRACE_S_FUNC_ENTER(log);
    TRACE_S_FUNC_EXIT(log);
    return (status == IO_STATUS_OK) ? STREAM_BUFFER_STATUS::WRITE_OK :
                                      STREAM_BUFFER_STATUS::WRITE_ERROR;
}

MUINT32 MWFrame::toUserStatus(const ILog &log, IO_STATUS status)
{
    TRACE_S_FUNC_ENTER(log);
    MUINT32 userStatus = IUsersManager::UserStatus::RELEASE;
    userStatus |= (status == IO_STATUS_INVALID) ? 0 : IUsersManager::UserStatus::USED;
    TRACE_S_FUNC_EXIT(log);
    return userStatus;
}

} // namespace P2
