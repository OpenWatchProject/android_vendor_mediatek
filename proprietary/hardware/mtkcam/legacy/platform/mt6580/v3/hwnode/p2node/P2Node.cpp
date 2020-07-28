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

#define DEFINE_OPEN_ID      (getOpenId())


#include <mtkcam/v3/hwnode/P2Node.h>
#include "P2Common.h"
#include "FrameUtils.h"
#include "Processor.h"

//
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
//
#include "p2node/proc/P2Procedure.h"
#include "p2node/proc/MdpProcedure.h"
#if SUPPORT_PLUGIN
#include "p2node/proc/PluginProcedure.h"
#endif

using namespace NSCam::Utils::Sync;

using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc_FrmB;
using namespace NS3Av3;


/******************************************************************************
 *
 ******************************************************************************/
inline
MBOOL isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId) {
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}


/******************************************************************************
 *
 ******************************************************************************/
class P2NodeImp
        : public BaseNode, public P2Node, public StreamControl {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef sp<IPipelineFrame> QueNode_T;
    typedef android::List <QueNode_T> Que_T;

public:
    P2NodeImp(ePass2Type const type);

    ~P2NodeImp();

    virtual MERROR config(ConfigParams const &rParams);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MERROR init(InitParams const &rParams);

    virtual MERROR uninit();

    virtual MERROR flush();

    virtual MERROR queue(
            sp<IPipelineFrame> pFrame
    );


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MERROR verifyConfigParams(
            ConfigParams const &rParams
    ) const;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StreamControl Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MERROR getInfoIOMapSet(
            sp<IPipelineFrame> const &pFrame,
            IPipelineFrame::InfoIOMapSet &rIOMapSet
    ) const;

    MBOOL isInImageStream(
            StreamId_T const streamId
    ) const;

    MBOOL isInMetaStream(
            StreamId_T const streamId
    ) const;

    MERROR acquireImageStream(
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer> &rpStreamBuffer
    );

    MVOID releaseImageStream(
            sp<IPipelineFrame> const &pFrame,
            sp<IImageStreamBuffer> const pStreamBuffer,
            MUINT32 const status
    ) const;

    MERROR acquireImageBuffer(
            StreamId_T const streamId,
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBuffer> &rpImageBuffer,
            MBOOL const bForceWriteLock
    ) const;

    MVOID releaseImageBuffer(
            sp<IImageStreamBuffer> const rpStreamBuffer,
            sp<IImageBuffer> const pImageBuffer
    ) const;

    MERROR acquireMetaStream(
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId,
            sp<IMetaStreamBuffer> &rpStreamBuffer
    );

    MVOID releaseMetaStream(
            sp<IPipelineFrame> const &pFrame,
            sp<IMetaStreamBuffer> const pStreamBuffer,
            MUINT32 const status
    ) const;

    MERROR acquireMetadata(
            StreamId_T const streamId,
            sp<IMetaStreamBuffer> const pStreamBuffer,
            IMetadata *&rpMetadata
    ) const;

    MVOID releaseMetadata(
            sp<IMetaStreamBuffer> const pStreamBuffer,
            IMetadata *const pMetadata
    ) const;

    MVOID onPartialFrameDone(
            sp<IPipelineFrame> const &pFrame
    );

    MVOID onFrameDone(
            sp<IPipelineFrame> const &pFrame
    );

    MBOOL isP2HalYuvJpegSupport(
            sp<IPipelineFrame> const &pFrame
    );

public:
    MERROR mapToRequests(
            sp<IPipelineFrame> const &pFrame
    );

    inline MBOOL isFullRawLocked(StreamId_T const streamId) const {
        for (size_t i = 0; i < mpvInFullRaw.size(); i++) {
            if (isStream(mpvInFullRaw[i], streamId))
                return MTRUE;
        }
        return MFALSE;
    }

    inline MBOOL isResizeRawLocked(StreamId_T const streamId) const {
        return isStream(mpInResizedRaw, streamId);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Data Members. (Config)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // meta
    sp<IMetaStreamInfo> mpInAppMeta_Request;
    sp<IMetaStreamInfo> mpInHalMeta_P1;
    sp<IMetaStreamInfo> mpOutAppMeta_Result;
    sp<IMetaStreamInfo> mpOutHalMeta_Result;
    // image
    Vector<sp<IImageStreamInfo>> mpvInFullRaw;
    sp<IImageStreamInfo> mpInResizedRaw;
    ImageStreamInfoSetT mvOutImages;
    sp<IImageStreamInfo> mpOutFd;
    sp<IImageStreamInfo> mpInYuv;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Data Members. (Others)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ePass2Type const mType;
    mutable RWLock mConfigRWLock;
    mutable Mutex mOperationLock;
    MINT32 mLogLevel;
    MINT32 mSupportPlugin;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Data Members. (Processor)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
#if SUPPORT_PLUGIN
    sp<Processor> mpRawProcessor;
    sp<Processor> mpYuvProcessor;
    PostProcessing *mpRawPostProcessing;
    PostProcessing *mpYuvPostProcessing;
#endif
    PoolBufferAllocatorMap mPoolAllocatorMap;
    sp<Processor> mpP2Processor;
    sp<Processor> mpMdpProcessor;
};


/******************************************************************************
 *
 ******************************************************************************/
sp<P2Node>
P2Node::
createInstance(ePass2Type const type) {
    if (type < 0 || type >= PASS2_TYPE_TOTAL) {
        MY_LOGE("not supported p2 type %d", type);
        return NULL;
    }

    return new P2NodeImp(type);
}


/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
P2NodeImp(ePass2Type const type)
        : BaseNode(),
          P2Node(),
          mpInAppMeta_Request(),
          mpInHalMeta_P1(),
          mpOutAppMeta_Result(),
          mpOutHalMeta_Result(),
          mpvInFullRaw(),
          mpInResizedRaw(),
          mvOutImages(),
          mpOutFd(),
          mpInYuv(),
          mType(type),
          mConfigRWLock(),
          mOperationLock(),
          //
          mpP2Processor(NULL),
          mpRawProcessor(NULL),
          mpYuvProcessor(NULL),
          mpRawPostProcessing(NULL),
          mpYuvPostProcessing(NULL),
          mpMdpProcessor(NULL)
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if (mLogLevel == 0)
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.p2node", 0);

    mSupportPlugin = ::property_get_int32("vendor.debug.camera.support.plugin", 0);

}


/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
~P2NodeImp() {
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
init(InitParams const &rParams) {
    CAM_TRACE_NAME("P2:init");
    FUNC_START;

    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;

    MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName());
    MRect activeArray;
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if (!pMetadataProvider.get()) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray)) {
            MY_LOGDO_IF(1, "active array(%d, %d, %dx%d)",
                       activeArray.p.x, activeArray.p.y,
                       activeArray.s.w, activeArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }

        {
            mpP2Processor = NULL;
            P2Procedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.type = mType;
            param.activeArray = activeArray;
            param.bEnableLog = mLogLevel >= 1;
            param.pPoolAllocatorMap = &mPoolAllocatorMap;
            mpP2Processor = P2Procedure::createProcessor(param);
            if (mpP2Processor == NULL)
                return BAD_VALUE;
        }
#if SUPPORT_PLUGIN
        if (mSupportPlugin == 1)
        {
            mpRawProcessor = NULL;
            mpYuvProcessor = NULL;
            //
            PluginProcedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.bEnableLog = mLogLevel >= 1;
            param.bRawDomain = MTRUE;
            param.pPoolAllocatorMap = &mPoolAllocatorMap;
            mpRawProcessor = PluginRegistry::createProcessor(PluginRegistry::RawDomain, param);
            mpRawPostProcessing = param.pPostProcessing;
            if (mpRawProcessor == NULL)
                MY_LOGD("RAW Plugin not existed");
            param.bRawDomain = MFALSE;
            mpYuvProcessor = PluginRegistry::createProcessor(PluginRegistry::YuvDomain, param);
            mpYuvPostProcessing = param.pPostProcessing;
            if (mpYuvProcessor == NULL)
                MY_LOGD("YUV Plugin not existed");
        }
#endif
        {
            mpMdpProcessor = NULL;
            MdpProcedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.bEnableLog = mLogLevel >= 1;
            mpMdpProcessor = MdpProcedure::createProcessor(param);
            if (mpMdpProcessor == NULL)
                return BAD_VALUE;
        }

        // link processor chain
        sp<Processor> pLinker = mpP2Processor;
#if SUPPORT_PLUGIN
        if (mSupportPlugin == 1)
        {
            if (mpRawProcessor != NULL)
                mpRawProcessor->setNextProcessor(mpP2Processor);

            if (mpYuvProcessor != NULL) {
                pLinker->setNextProcessor(mpYuvProcessor);
                pLinker = mpYuvProcessor;
            }
        }
#endif
        pLinker->setNextProcessor(mpMdpProcessor);
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
config(ConfigParams const &rParams) {
    CAM_TRACE_NAME("P2:config");
    {
        MERROR const err = verifyConfigParams(rParams);
        if (err != OK) {
            MY_LOGEO("verifyConfigParams failed, err = %d", err);
            return err;
        }
    }

    flush();

    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta_Request = rParams.pInAppMeta;
        mpInHalMeta_P1      = rParams.pInHalMeta;
        mpOutAppMeta_Result = rParams.pOutAppMeta;
        mpOutHalMeta_Result = rParams.pOutHalMeta;
        // image
        mpvInFullRaw        = rParams.pvInFullRaw;
        mpInResizedRaw      = rParams.pInResizedRaw;
        mvOutImages         = rParams.vOutImage;
        mpOutFd             = rParams.pOutFDImage;
        //
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
uninit() {
    CAM_TRACE_NAME("P2:uninit");
    FUNC_START;

    if (OK != flush())
        MY_LOGEO("flush failed");

    if (mpP2Processor.get()) {
        mpP2Processor->waitForIdle();
        mpP2Processor->close();
        mpP2Processor = NULL;
    }

    if (mpMdpProcessor.get()) {
        mpMdpProcessor->waitForIdle();
        mpMdpProcessor->close();
        mpMdpProcessor = NULL;
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
flush() {
    CAM_TRACE_NAME("P2:flush");
    FUNC_START;

    Mutex::Autolock _l(mOperationLock);
    // 1. wait for P2 thread
    mpP2Processor->flushRequests();

    // 2. wait for MDP thread
    mpMdpProcessor->flushRequests();

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
queue(sp<IPipelineFrame> pFrame) {
    FUNC_START;

    if (!pFrame.get()) {
        MY_LOGEO("Null frame");
        return BAD_VALUE;
    }
    if (!mpP2Processor.get()) {
        MY_LOGWO("may not configured yet");
        BaseNode::flush(pFrame);
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mOperationLock);
    MY_LOGDO_IF(1, "queue pass2 @ frame(%d)", pFrame->getFrameNo());

    // map IPipelineFrame to requests
    MERROR ret;
    if (OK != (ret = mapToRequests(pFrame))) {
        MY_LOGWO("map to jobs failed");
        BaseNode::flush(pFrame);
        return ret;
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
verifyConfigParams(ConfigParams const &rParams) const {
    if (!rParams.pInAppMeta.get()) {
        MY_LOGEO("no in app meta");
        return BAD_VALUE;
    }
    if (!rParams.pInHalMeta.get()) {
        MY_LOGEO("no in hal meta");
        return BAD_VALUE;
    }
    //if  ( ! rParams.pOutAppMeta.get() ) {
    //    return BAD_VALUE;
    //}
    //if  ( ! rParams.pOutHalMeta.get() ) {
    //    return BAD_VALUE;
    //}
    if (rParams.pvInFullRaw.size() == 0 && !rParams.pInResizedRaw.get()) {
        MY_LOGEO("no in image fullraw or resized raw");
        return BAD_VALUE;
    }
    if (0 == rParams.vOutImage.size()) {
        MY_LOGEO("no out yuv image");
        return BAD_VALUE;
    }

#define dumpStreamIfExist(str, stream)                         \
    do {                                                       \
        MY_LOGDO_IF(stream.get(), "%s: id %#" PRIx64 ", %s",     \
                str,                                           \
                stream->getStreamId(), stream->getStreamName() \
               );                                              \
    } while(0)

    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);
    for (size_t i = 0; i < rParams.pvInFullRaw.size(); i++) {
        dumpStreamIfExist("[img] in full", rParams.pvInFullRaw[i]);
    }
    dumpStreamIfExist("[img] in resized", rParams.pInResizedRaw);
    for (size_t i = 0; i < rParams.vOutImage.size(); i++) {
        dumpStreamIfExist("[img] out yuv", rParams.vOutImage[i]);
    }
    dumpStreamIfExist("[img] out fd", rParams.pOutFDImage);
#undef dumpStreamIfExist
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
getInfoIOMapSet(
        sp<IPipelineFrame> const &pFrame,
        IPipelineFrame::InfoIOMapSet &rIOMapSet) const
{
    if (OK != pFrame->queryInfoIOMapSet(getNodeId(), rIOMapSet)) {
        MY_LOGEO("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }

    // do some check
    IPipelineFrame::ImageInfoIOMapSet &imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
    if (!imageIOMapSet.size()) {
        MY_LOGWO("no imageIOMap in frame");
        return BAD_VALUE;
    }

    for (size_t i = 0; i < imageIOMapSet.size(); i++) {
        IPipelineFrame::ImageInfoIOMap const &imageIOMap = imageIOMapSet[i];
        if ( imageIOMap.vIn.size() < 0 || imageIOMap.vIn.size() > 2 || imageIOMap.vOut.size() == 0) {
            MY_LOGEO("[img] #%d wrong size vIn %d, vOut %d",
                    i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
            return BAD_VALUE;
        }
        MY_LOGDO_IF(mLogLevel >= 1, "frame %zu:[img] #%zu, in %d, out %d",
                   pFrame->getFrameNo(), i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
    }

    IPipelineFrame::MetaInfoIOMapSet &metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if (!metaIOMapSet.size()) {
        MY_LOGWO("no metaIOMap in frame");
        return BAD_VALUE;
    }

    for (size_t i = 0; i < metaIOMapSet.size(); i++) {
        IPipelineFrame::MetaInfoIOMap const &metaIOMap = metaIOMapSet[i];
        if (!mpInAppMeta_Request.get() ||
            0 > metaIOMap.vIn.indexOfKey(mpInAppMeta_Request->getStreamId())) {
            MY_LOGEO("[meta] no in app");
            return BAD_VALUE;
        }
        if (!mpInHalMeta_P1.get() ||
            0 > metaIOMap.vIn.indexOfKey(mpInHalMeta_P1->getStreamId())) {
            MY_LOGEO("[meta] no in hal");
            return BAD_VALUE;
        }
        MY_LOGDO_IF(mLogLevel >= 2, "frame %zu:[meta] #%zu: in %d, out %d",
                   pFrame->getFrameNo(), i, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInImageStream(StreamId_T const streamId) const {
    RWLock::AutoRLock _l(mConfigRWLock);
    if (isFullRawLocked(streamId) || isResizeRawLocked(streamId)) {
        return MTRUE;
    }
    MY_LOGDO_IF(0, "stream id %#" PRIxPTR " is not in-stream", streamId);
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInMetaStream(StreamId_T const streamId) const {
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInAppMeta_Request, streamId) ||
           isStream(mpInHalMeta_P1, streamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireImageStream(
        sp<IPipelineFrame> const &pFrame,
        StreamId_T const streamId,
        sp<IImageStreamBuffer> &rpStreamBuffer)
{
    return ensureImageBufferAvailable_(
            pFrame->getFrameNo(), streamId,
            pFrame->getStreamBufferSet(), rpStreamBuffer
    );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseImageStream(
        sp<IPipelineFrame> const &pFrame,
        sp<IImageStreamBuffer> const pStreamBuffer,
        MUINT32 const status) const
{
    IStreamBufferSet &streamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();

    if (pStreamBuffer == NULL) {
        MY_LOGEO("pStreamBuffer == NULL");
        return;
    }

    if (!isInImageStream(streamId)) {
        pStreamBuffer->markStatus((status != eStreamStatus_FILLED) ?
                                  STREAM_BUFFER_STATUS::WRITE_ERROR :
                                  STREAM_BUFFER_STATUS::WRITE_OK
        );
    }

    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    streamBufferSet.markUserStatus(
            streamId,
            getNodeId(),
            ((status != eStreamStatus_NOT_USED) ?
             IUsersManager::UserStatus::USED : 0) | IUsersManager::UserStatus::RELEASE
    );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireImageBuffer(
        StreamId_T const streamId,
        sp<IImageStreamBuffer> const pStreamBuffer,
        sp<IImageBuffer> &rpImageBuffer,
        MBOOL const bForceWriteLock) const
{
    if (pStreamBuffer == NULL) {
        MY_LOGEO("pStreamBuffer == NULL");
        return BAD_VALUE;
    }
    //  Query the group usage.
    MUINT groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
    if (bForceWriteLock)
        groupUsage |= eBUFFER_USAGE_SW_WRITE_MASK;

    sp<IImageBufferHeap> pImageBufferHeap = isInImageStream(streamId) ?
                                            pStreamBuffer->tryReadLock(getNodeName()) :
                                            pStreamBuffer->tryWriteLock(getNodeName());

    if (pImageBufferHeap == NULL) {
        MY_LOGEO("[node:%d][stream buffer:%s] cannot get ImageBufferHeap",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL) {
        MY_LOGEO("[node:%d][stream buffer:%s] cannot create ImageBuffer",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGDO_IF(mLogLevel >= 1, "stream %#" PRIxPTR ": buffer: %p, usage: %x",
               streamId, rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseImageBuffer(
        sp<IImageStreamBuffer> const pStreamBuffer,
        sp<IImageBuffer> const pImageBuffer) const
{
    if (pStreamBuffer == NULL || pImageBuffer == NULL) {
        MY_LOGEO("pStreamBuffer or pImageBuffer should not be NULL");
        return;
    }

    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireMetaStream(
        sp<IPipelineFrame> const &pFrame,
        StreamId_T const streamId,
        sp<IMetaStreamBuffer> &rpStreamBuffer)
{
    return ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            pFrame->getStreamBufferSet(),
            rpStreamBuffer
    );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseMetaStream(
        sp<IPipelineFrame> const &pFrame,
        sp<IMetaStreamBuffer> const pStreamBuffer,
        MUINT32 const status) const
{
    IStreamBufferSet &rStreamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();

    if (pStreamBuffer.get() == NULL) {
        MY_LOGEO("StreamId %d: pStreamBuffer == NULL", streamId);
        return;
    }

    //Buffer Producer must set this status.
    if (!isInMetaStream(streamId)) {
        pStreamBuffer->markStatus(
                (status != eStreamStatus_FILLED) ?
                STREAM_BUFFER_STATUS::WRITE_ERROR :
                STREAM_BUFFER_STATUS::WRITE_OK
        );
    }

    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            ((status != eStreamStatus_NOT_USED) ? IUsersManager::UserStatus::USED : 0) |
            IUsersManager::UserStatus::RELEASE);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireMetadata(
        StreamId_T const streamId,
        sp<IMetaStreamBuffer> const pStreamBuffer,
        IMetadata *&rpMetadata) const
{
    rpMetadata = isInMetaStream(streamId) ?
                 pStreamBuffer->tryReadLock(getNodeName()) :
                 pStreamBuffer->tryWriteLock(getNodeName());

    if (rpMetadata == NULL) {
        MY_LOGEO("[node:%d][stream buffer:%s] cannot get metadata",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGDO_IF(0, "stream %#" PRIxPTR ": stream buffer %p, metadata: %p",
               streamId, pStreamBuffer.get(), rpMetadata);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseMetadata(
        sp<IMetaStreamBuffer> const pStreamBuffer,
        IMetadata *const pMetadata) const
{
    if (pMetadata == NULL) {
        MY_LOGWO("pMetadata == NULL");
        return;
    }
    pStreamBuffer->unlock(getNodeName(), pMetadata);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onPartialFrameDone(sp<IPipelineFrame> const &pFrame) {
    CAM_TRACE_NAME("P2:PartialFrameDone");
//FUNC_START;
    IStreamBufferSet &rStreamBufferSet = pFrame->getStreamBufferSet();
    rStreamBufferSet.applyRelease(getNodeId());
//FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onFrameDone(sp<IPipelineFrame> const &pFrame) {
    CAM_TRACE_NAME("P2:FrameDone");
    //MY_LOGDO("frame %u done", pFrame->getFrameNo());
    onDispatchFrame(pFrame);
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isP2HalYuvJpegSupport(
    sp<IPipelineFrame> const &pFrame
)
{
    MBOOL result = MFALSE;
    if ( pFrame->getIsP2HalYuvJpegSupport()) {
        result = MTRUE;
        MY_LOGDO_IF(mLogLevel >= 1, "P2 hal yuv jpeg out port support");
    }

    return result;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
mapToRequests(sp<IPipelineFrame> const &pFrame) {

    // 1. get IOMap
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if (OK != getInfoIOMapSet(pFrame, IOMapSet)) {
        MY_LOGEO("queryInfoIOMap failed");
        return BAD_VALUE;
    }

    // 2. create metadata handle (based on IOMap)
    sp<MetaHandle> pMeta_InApp = mpInAppMeta_Request.get() ?
                                 MetaHandle::create(this, pFrame, mpInAppMeta_Request->getStreamId()) : NULL;
    sp<MetaHandle> pMeta_InHal = mpInHalMeta_P1.get() ?
                                 MetaHandle::create(this, pFrame, mpInHalMeta_P1->getStreamId()) : NULL;

    if (pMeta_InApp == NULL || pMeta_InHal == NULL) {
        MY_LOGWO("meta check failed");
        return BAD_VALUE;
    }

    // 3. create FrameLifeHolder
    sp<FrameLifeHolder> pFrameLife = new FrameLifeHolder(mOpenId, this, pFrame, mLogLevel >= 1);

    // 4. process image IO
    IPipelineFrame::ImageInfoIOMapSet &imageIOMapSet = IOMapSet.mImageInfoIOMapSet;
    for (size_t run_idx = 0; run_idx < imageIOMapSet.size(); run_idx++) {
        IPipelineFrame::ImageInfoIOMap const &imageIOMap = imageIOMapSet[run_idx];
        sp<Request> pRequest = new Request(
                pFrameLife,
                0 /*uniqueKey*/,
                pFrame->getRequestNo(),
                pFrame->getFrameNo(),
                run_idx);

        pRequest->context.fd_stream_id = mpOutFd.get() ? mpOutFd->getStreamId() : 0;

        // source
        // for (size_t i = 0; i < imageIOMap.vIn.size(); i++)
        {
            StreamId_T const sId = imageIOMap.vIn.keyAt(0);
            pRequest->context.in_buffer = StreamBufferHandle::create(this, pFrame, sId);
        }
        if (pRequest->context.in_buffer == NULL)
        {
            MY_LOGW("sensor(%d) get input buffer failed", mOpenId);
            return BAD_VALUE;
        }
        StreamId_T const streamId = pRequest->context.in_buffer->getStreamId();
        {
            RWLock::AutoRLock _l(mConfigRWLock);
            pRequest->context.resized = isResizeRawLocked(streamId);
        }

        // destination
        for (size_t i = 0; i < imageIOMap.vOut.size(); i++) {
            StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
            //MUINT32 const transform = imageIOMap.vOut.valueAt(i)->getTransform();
            sp<BufferHandle> pBufferHandle = StreamBufferHandle::create(this, pFrame, streamId);
            pRequest->context.out_buffers.push_back(pBufferHandle);
        }

        if (isP2HalYuvJpegSupport(pFrame)){
            pRequest->context.captureFrame = true;
        }

        pRequest->context.in_app_meta = pMeta_InApp;
        pRequest->context.in_hal_meta = pMeta_InHal;
        if (run_idx == 0) {
            pRequest->context.out_app_meta = mpOutAppMeta_Result.get() ?
                                             MetaHandle::create(this, pFrame, mpOutAppMeta_Result->getStreamId())
                                                                       : NULL;
            pRequest->context.out_hal_meta = mpOutHalMeta_Result.get() ?
                                             MetaHandle::create(this, pFrame, mpOutHalMeta_Result->getStreamId())
                                                                       : NULL;
        }

#if SUPPORT_PLUGIN
        if (mSupportPlugin == 1)
        {
            PostProcessing::ConditionParams condition;
            condition.pInAppMeta = pRequest->context.in_app_meta->getMetadata();
            condition.pInHalMeta = pRequest->context.in_hal_meta->getMetadata();
            condition.bResized = pRequest->context.resized;
            PostProcessing::DecisionParams decision;
            // Yuv plugin decision
            if (mpYuvProcessor != NULL &&
                mpYuvPostProcessing->decide(condition, decision) &&
                decision.verify())
            {
                pRequest->context.run_plugin_yuv = MTRUE;
                MUINT32 outImgForamt = (decision.outImgFmt == PostProcessing::eIMG_FMT_DEFAULT) ?
                                   eImgFmt_YUY2 :
                                   decision.inImgFmt;
                pRequest->context.work_buffer_format = outImgForamt;
            }
            // Raw plugin decision
            if (mpRawProcessor != NULL &&
                mpRawPostProcessing->decide(condition, decision) &&
                decision.verify())
            {
                pRequest->context.run_plugin_raw = MTRUE;
                mpRawProcessor->queueRequest(pRequest);
            }
            else
            {
                // prevent HAL1 where a frame has no buffer from dispatching to next node faster than this scope
                // when queued to processor, ensure that no one except request holds the buffer handle
                if (run_idx == imageIOMapSet.size() - 1) {
                    pMeta_InApp.clear();
                    pMeta_InHal.clear();
                }
                mpP2Processor->queueRequest(pRequest);
            }
        }
        else
        {
            // prevent HAL1 where a frame has no buffer from dispatching to next node faster than this scope
            // when queued to processor, ensure that no one except request holds the buffer handle
            if (run_idx == imageIOMapSet.size() - 1) {
                pMeta_InApp.clear();
                pMeta_InHal.clear();
            }
            mpP2Processor->queueRequest(pRequest);
        }
#else
        {
            // prevent HAL1 where a frame has no buffer from dispatching to next node faster than this scope
            // when queued to processor, ensure that no one except request holds the buffer handle
            if (run_idx == imageIOMapSet.size() - 1) {
                pMeta_InApp.clear();
                pMeta_InHal.clear();
            }
            mpP2Processor->queueRequest(pRequest);
        }
#endif
    }
    return OK;
}
