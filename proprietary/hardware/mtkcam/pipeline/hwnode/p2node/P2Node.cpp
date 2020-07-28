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

#include "P2Common.h"
#include "FrameUtils.h"
#include "Processor.h"
#include "mtkcam/def/BuiltinTypes.h"

#include <mtkcam/pipeline/hwnode/P2Node.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include "p2node/proc/P2Procedure.h"
#include "p2node/proc/MdpProcedure.h"
#include "p2node/proc/SwnrProcedure.h"
#if SUPPORT_3RD_PARTY
#include "p2node/proc/RawProcedure.h"
#include "p2node/proc/YuvProcedure.h"
#include <mtkcam/pipeline/extension/IVendorManager.h>
#endif
#include <utils/RWLock.h>

#if SUPPORT_PLUGIN
#include "p2node/proc/PluginProcedure.h"
#endif
#define LOG_TAG "MtkCam/P2Node"

using namespace NSCam::Utils::Sync;
using namespace NSCamHW;
using namespace NSCam::v3;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;
using namespace NS3Av3;
using namespace NSIspTuning;


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

    virtual MERROR flush(
            sp<IPipelineFrame> const &pFrame
    );

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

    MBOOL isMetaStreamAtInfoIOMap(
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId
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

    MERROR acquireImageBufferHeap(
            StreamId_T const streamId,
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBufferHeap> &rpImageBufferHeap
    ) const;

    MVOID releaseImageBufferHeap(
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBufferHeap> const pImageBufferHeap
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

    inline MBOOL isOpaqueRawLocked(StreamId_T const streamId) const {
        for (size_t i = 0; i < mpvInOpaque.size(); i++) {
            if (isStream(mpvInOpaque[i], streamId))
                return MTRUE;
        }
        return MFALSE;
    }

    inline MBOOL isResizeRawLocked(StreamId_T const streamId) const {
        return isStream(mpInResizedRaw, streamId);
    }

    inline MBOOL isLcsoRawLocked(StreamId_T const streamId) const {
        return isStream(mpInLcsoRaw, streamId);
    }

    inline MBOOL isYuvReprocLocked(StreamId_T const streamId) const {
        return isStream(mpInYuv, streamId);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Data Members. (Config)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // meta
    sp<IMetaStreamInfo> mpInAppMeta_Request;
    sp<IMetaStreamInfo> mpInAppRetMeta_Request;
    sp<IMetaStreamInfo> mpInHalMeta_P1;
    sp<IMetaStreamInfo> mpOutAppMeta_Result;
    sp<IMetaStreamInfo> mpOutHalMeta_Result;
    // image
    Vector<sp<IImageStreamInfo>> mpvInFullRaw;
    Vector<sp<IImageStreamInfo>> mpvInOpaque;
    sp<IImageStreamInfo> mpInResizedRaw;
    sp<IImageStreamInfo> mpInLcsoRaw;
    ImageStreamInfoSetT mvOutImages;
    sp<IImageStreamInfo> mpOutCaptureImage;
    sp<IImageStreamInfo> mpOutFd;
    sp<IImageStreamInfo> mpInYuv;
    // feature
    MUINT8 mBurstNum;
    MBOOL mEnableVencStream;
    MBOOL mInitVencStream;
    MBOOL mCustomOption;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Data Members. (Others)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ePass2Type const mType;
    mutable RWLock mConfigRWLock;
    mutable Mutex mOperationLock;
    MINT32 mLogLevel;
    MINT32 mForceNrMode;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   Data Members. (Processor)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
#if SUPPORT_PLUGIN
    sp<Processor> mpPreRawProcessor;
    sp<Processor> mpPreYuvProcessor;
    PostProcessing *mpRawPostProcessing;
    PostProcessing *mpYuvPostProcessing;
#endif
    sp<Processor> mpP2Processor;
#if SUPPORT_SWNR
    sp<Processor> mpSwnrProcessor;
#endif
#if SUPPORT_3RD_PARTY
    sp<Processor> mpRawProcessor;
    sp<Processor> mpYuvProcessor;
    sp<NSCam::plugin::IVendorManager> mpVendor; // config get
    MUINT64 muUserId;//config get
#endif
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
          mpInAppRetMeta_Request(),
          mpInHalMeta_P1(),
          mpOutAppMeta_Result(),
          mpOutHalMeta_Result(),
          mpvInFullRaw(),
          mpvInOpaque(),
          mpInResizedRaw(),
          mvOutImages(),
          mpOutCaptureImage(),
          mpOutFd(),
          mpInYuv(),
          //
          mBurstNum(0),
          mEnableVencStream(MFALSE),
          mInitVencStream(MFALSE),
          mCustomOption(0),
          //
          mType(type),
          mConfigRWLock(),
          mOperationLock(),
          //
          mpP2Processor(NULL),
#if SUPPORT_PLUGIN
          mpPreRawProcessor(NULL),
          mpPreYuvProcessor(NULL),
          mpRawPostProcessing(NULL),
          mpYuvPostProcessing(NULL),
#endif
#if SUPPORT_SWNR
          mpSwnrProcessor(NULL),
#endif
#if SUPPORT_3RD_PARTY
          mpRawProcessor(NULL),
          mpYuvProcessor(NULL),
          mpVendor(NULL),
          muUserId(0),
#endif
          mpMdpProcessor(NULL)
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if (mLogLevel == 0)
        mLogLevel = ::property_get_int32("debug.camera.log.p2node", 0);

    mForceNrMode = ::property_get_int32("debug.camera.force.nr", 0);
}


/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
~P2NodeImp() {
    MY_LOGD("~P2Node");
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
#ifdef USING_MTK_LDVT /*[EP_TEMP]*/ //[FIXME] TempTestOnly
            activeArray = MRect(1600, 1200);// hard-code sensor size
            MY_LOGIO("set sensor size to active array(%d, %d, %dx%d)",
                activeArray.p.x, activeArray.p.y,
                activeArray.s.w, activeArray.s.h);
            #else
            return UNKNOWN_ERROR;
#endif
        }
#if SUPPORT_3RD_PARTY
        {
            mpRawProcessor = NULL;
            RawProcedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.bEnableLog = mLogLevel >= 1;
            mpRawProcessor = RawProcedure::createProcessor(param);
            //
            if (mpRawProcessor.get() == NULL)
                MY_LOGD("RAW Plugin not existed");
        }
#endif
        {
            mpP2Processor = NULL;
            P2Procedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.type = mType;
            param.activeArray = activeArray;
            param.bEnableLog = mLogLevel >= 1;
            mpP2Processor = P2Procedure::createProcessor(param);
            if (mpP2Processor == NULL)
                return BAD_VALUE;
        }
#if SUPPORT_SWNR
        {
            mpSwnrProcessor = NULL;
            SwnrProcedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.bEnableLog = mLogLevel >= 1;
            mpSwnrProcessor = SwnrProcedure::createProcessor(param);
            if (mpSwnrProcessor == NULL)
                return BAD_VALUE;
        }
#endif
#if SUPPORT_PLUGIN
        {
            mpPreRawProcessor = NULL;
            mpPreYuvProcessor = NULL;
            PluginProcedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.bEnableLog = mLogLevel >= 1;
            param.bRawDomain = MTRUE;
            mpPreRawProcessor = PluginRegistry::createProcessor(PluginRegistry::RawDomain, param);
            mpRawPostProcessing = param.pPostProcessing;
            if (mpPreRawProcessor == NULL)
                MY_LOGD("RAW Plugin not existed");
            param.bRawDomain = MFALSE;
            mpPreYuvProcessor = PluginRegistry::createProcessor(PluginRegistry::YuvDomain, param);
            mpYuvPostProcessing = param.pPostProcessing;
            if (mpPreYuvProcessor == NULL)
                MY_LOGD("YUV Plugin not existed");
        }
#endif
#if SUPPORT_3RD_PARTY
        {
            mpYuvProcessor = NULL;
            YuvProcedure::CreateParams param;
            param.uOpenId = mOpenId;
            param.bEnableLog = mLogLevel >= 1;
            mpYuvProcessor = YuvProcedure::createProcessor(param);
            //
            if (mpYuvProcessor.get() == NULL)
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
#if SUPPORT_PLUGIN && SUPPORT_3RD_PARTY
        if (mpPreRawProcessor != NULL){
            mpPreRawProcessor->setNextProcessor(mpRawProcessor);
            mpRawProcessor->setNextProcessor(mpP2Processor);
        }
#elif SUPPORT_3RD_PARTY
        if (mpRawProcessor != NULL)
            mpRawProcessor->setNextProcessor(mpP2Processor);
#elif SUPPORT_PLUGIN
        if (mpPreRawProcessor != NULL)
            mpPreRawProcessor->setNextProcessor(mpP2Processor);
#endif

#if SUPPORT_SWNR
        pLinker->setNextProcessor(mpSwnrProcessor);
        pLinker = mpSwnrProcessor;
#endif

#if SUPPORT_PLUGIN
        if (mpPreYuvProcessor != NULL) {
            pLinker->setNextProcessor(mpPreYuvProcessor);
            pLinker = mpPreYuvProcessor;
        }
#endif
#if SUPPORT_3RD_PARTY
        if (mpYuvProcessor != NULL) {
            pLinker->setNextProcessor(mpYuvProcessor);
            pLinker = mpYuvProcessor;
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
        mpInHalMeta_P1 = rParams.pInHalMeta;
        mpInAppRetMeta_Request = rParams.pInAppRetMeta;
        mpOutAppMeta_Result = rParams.pOutAppMeta;
        mpOutHalMeta_Result = rParams.pOutHalMeta;
        // image
        mpvInFullRaw = rParams.pvInFullRaw;
        mpvInOpaque = rParams.pvInOpaque;
        mpInResizedRaw = rParams.pInResizedRaw;
        mpInLcsoRaw = rParams.pInLcsoRaw;
        mvOutImages = rParams.vOutImage;
        mpOutCaptureImage = rParams.pOutCaptureImage;
        mpOutFd = rParams.pOutFDImage;
        mpInYuv = rParams.pInYuvImage;
        // property
        mBurstNum = rParams.burstNum;

        mCustomOption = rParams.customOption;
        if(mCustomOption == 0)
            mCustomOption = ::property_get_int32("debug.camera.force.4cell", 0);
#if 0
        // This is for HAL3. HAL1 cannot get buffer size in config phase.
        if (mEnableVencStream) {
            for (size_t i = 0; i < rParams.vOutImage.size(); i++) {
                if (rParams.vOutImage[i]->getUsageForAllocator() &
                    GRALLOC_USAGE_HW_VIDEO_ENCODER) {
                    MSize size = rParams.vOutImage[i]->getImgSize();
                    MY_LOGDO("start the direct link of video encorder. fps:%d, width:%d, hight:%d",
                        rParams.fps, size.w, size.h);
                    mpP2Processor->notify(P2Procedure::eP2_START_VENC_STREAM, rParams.fps, size.w, size.h);
                    break;
                }
            }
        }
#endif
    }
#if SUPPORT_3RD_PARTY
    mpVendor = rParams.pVendor;
    muUserId = rParams.uUserId;
#endif

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
uninit() {
    CAM_TRACE_NAME("P2:uninit");
    MY_LOGDO("uninit +");

    if (OK != flush())
        MY_LOGEO("flush failed");
#if SUPPORT_PLUGIN
    if (mpPreRawProcessor.get()) {
        mpPreRawProcessor->waitForIdle();
        mpPreRawProcessor->close();
        mpPreRawProcessor = NULL;
    }
#endif
#if SUPPORT_3RD_PARTY
    if (mpRawProcessor.get()) {
        mpRawProcessor->waitForIdle();
        mpRawProcessor->close();
        mpRawProcessor = NULL;
    }
#endif

    if (mpP2Processor.get()) {
        mpP2Processor->waitForIdle();
        mpP2Processor->close();
        //MY_LOGDO("[P2] reference count:%d",mpP2Processor->getStrongCount());
        mpP2Processor = NULL;
    }
#if SUPPORT_3RD_PARTY
    if (mpYuvProcessor.get()) {
        mpYuvProcessor->waitForIdle();
        mpYuvProcessor->close();
        mpYuvProcessor = NULL;
    }
#endif

#if SUPPORT_SWNR
    if (mpSwnrProcessor.get()) {
        mpSwnrProcessor->waitForIdle();
        mpSwnrProcessor->close();
        mpSwnrProcessor = NULL;
    }
#endif
    if (mpMdpProcessor.get()) {
        mpMdpProcessor->waitForIdle();
        mpMdpProcessor->close();
        //MY_LOGDO("MDP] reference count:%d",mpMdpProcessor->getStrongCount());
        mpMdpProcessor = NULL;
    }

    MY_LOGDO("uninit -");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    Mutex::Autolock _l(mOperationLock);
    if(mBurstNum > 1) {
      if (mpP2Processor.get()) {
          MY_LOGD("flush unpaired-request in burst mode ");
          mpP2Processor->flushRequests();
      }
    }
    return BaseNode::flush(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
flush() {
    CAM_TRACE_NAME("P2:flush");
    MY_LOGDO("flush +");

    Mutex::Autolock _l(mOperationLock);
#if SUPPORT_PLUGIN
    if (mpPreRawProcessor.get()) {
        mpPreRawProcessor->flushRequests();
    }
#endif
#if SUPPORT_3RD_PARTY
    if(mpVendor.get())
        mpVendor->beginFlush(
            muUserId
        );

    if (mpRawProcessor.get()) {
        mpRawProcessor->flushRequests();
    }
    if (mpYuvProcessor.get()) {
        mpYuvProcessor->flushRequests();
    }
#endif

    // 1. wait for P2 thread
    if (mpP2Processor.get())
        mpP2Processor->flushRequests();
#if SUPPORT_SWNR
    // 2. wait for SWNR thread
    if (mpSwnrProcessor.get()) {
        mpSwnrProcessor->flushRequests();
    }
#endif
#if SUPPORT_3RD_PARTY
    if(mpVendor.get())
        mpVendor->endFlush(
            muUserId
        );
#endif

    // 3. wait for MDP thread
    if (mpMdpProcessor.get())
        mpMdpProcessor->flushRequests();

    MY_LOGDO("flush -");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
queue(sp<IPipelineFrame> pFrame) {
    FUNC_START;
    CAM_TRACE_FMT_BEGIN("P2:queue frame:%d", pFrame->getFrameNo());
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
    MY_LOGDO_IF(mLogLevel >= 2, "queue pass2 @ frame(%d)", pFrame->getFrameNo());

    // map IPipelineFrame to requests
    MERROR ret;
    if (OK != (ret = mapToRequests(pFrame))) {
        MY_LOGWO("map to jobs failed");
        CAM_TRACE_BEGIN("P2:BaseNode:flush");
        BaseNode::flush(pFrame);
        CAM_TRACE_END();
        return ret;
    }
    CAM_TRACE_FMT_END();
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
    if (0 == rParams.vOutImage.size() && !rParams.pOutFDImage.get()) {
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
    dumpStreamIfExist("[meta] in appRet", rParams.pInAppRetMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);
    for (size_t i = 0; i < rParams.pvInFullRaw.size(); i++) {
        dumpStreamIfExist("[img] in full", rParams.pvInFullRaw[i]);
    }
    for (size_t i = 0; i < rParams.pvInOpaque.size(); i++) {
        dumpStreamIfExist("[img] in opaque", rParams.pvInOpaque[i]);
    }
    dumpStreamIfExist("[img] in resized", rParams.pInResizedRaw);
    for (size_t i = 0; i < rParams.vOutImage.size(); i++) {
        dumpStreamIfExist("[img] out yuv", rParams.vOutImage[i]);
    }
    dumpStreamIfExist("[img] out fd", rParams.pOutFDImage);
    dumpStreamIfExist("[img] in yuv", rParams.pInYuvImage);
    dumpStreamIfExist("[img] in cap", rParams.pOutCaptureImage);
#undef dumpStreamIfExist

    if  ( rParams.pVendor.get() )
        MY_LOGDO("vendor instance: %p UserId: %" PRIX64 " ", rParams.pVendor.get(), rParams.uUserId);
    MY_LOGDO("custom option: %d",rParams.customOption);
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
        // if ( imageIOMap.vIn.size() == 0 ||imageIOMap.vIn.size() > 2 || imageIOMap.vOut.size() == 0) {
        if ( imageIOMap.vIn.size() == 0 ||imageIOMap.vIn.size() > 3 ) {
            MY_LOGEO("[img] #%zu wrong size vIn %zu, vOut %zu",
                    i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
            return BAD_VALUE;
        }
        MY_LOGDO_IF(mLogLevel >= 1, "frame %d:[img] %zu, in %zu, out %zu",
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
        //
        MY_LOGDO_IF(mLogLevel >= 2, "frame %d:[meta] %zu: in %zu, out %zu",
                   pFrame->getFrameNo(), i, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isMetaStreamAtInfoIOMap(
    sp<IPipelineFrame> const &pFrame,
    StreamId_T const streamId
) const
{
    IPipelineFrame::InfoIOMapSet rIOMapSet;

    if (OK != pFrame->queryInfoIOMapSet(getNodeId(), rIOMapSet)) {
        MY_LOGEO("queryInfoIOMap failed");
        return MFALSE;
    }

    IPipelineFrame::MetaInfoIOMapSet &metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if (!metaIOMapSet.size()) {
        MY_LOGWO("no metaIOMap in frame");
        return MFALSE;
    }

    for (size_t i = 0; i < metaIOMapSet.size(); i++) {
        IPipelineFrame::MetaInfoIOMap const &metaIOMap = metaIOMapSet[i];
        if ( 0 <= metaIOMap.vIn.indexOfKey(streamId) || 0 <= metaIOMap.vOut.indexOfKey(streamId) ) {
            return MTRUE;
        }
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInImageStream(StreamId_T const streamId) const {
    RWLock::AutoRLock _l(mConfigRWLock);
    if (isFullRawLocked(streamId) || isResizeRawLocked(streamId) ||
        isLcsoRawLocked(streamId) || isOpaqueRawLocked(streamId)  ||
        isYuvReprocLocked(streamId)) {
        return MTRUE;
    }
    MY_LOGDO_IF(0, "stream id %#" PRIx64 " is not in-stream", streamId);
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
           isStream(mpInHalMeta_P1, streamId) ||
           isStream(mpInAppRetMeta_Request, streamId);
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
acquireImageBufferHeap(
    StreamId_T const streamId,
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBufferHeap> &rpImageBufferHeap
) const
{
    if (pStreamBuffer == NULL) {
        MY_LOGEO("pStreamBuffer == NULL");
        return BAD_VALUE;
    }
    //  Query the group usage.
    MUINT groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());

    rpImageBufferHeap = isInImageStream(streamId) ?
                                            pStreamBuffer->tryReadLock(getNodeName()) :
                                            pStreamBuffer->tryWriteLock(getNodeName());

    if (rpImageBufferHeap == NULL) {
        MY_LOGEO("[node:%#" PRIxPTR "][stream buffer:%s] cannot get ImageBufferHeap",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGDO_IF(mLogLevel >= 1, "stream %#" PRIx64 ": buffer heap: %p, usage: %x",
               streamId, rpImageBufferHeap.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseImageBufferHeap(
        sp<IImageStreamBuffer> const pStreamBuffer,
        sp<IImageBufferHeap> const pImageBufferHeap
) const
{
    if (pStreamBuffer == NULL) {
        MY_LOGEO("pStreamBuffer should not be NULL");
        return;
    }

    pStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
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
        MY_LOGEO("[node:%#" PRIxPTR "][stream buffer:%s] cannot get ImageBufferHeap",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (isOpaqueRawLocked(streamId)) {
        pImageBufferHeap->lockBuf(getNodeName());
        OpaqueReprocUtil::getImageBufferFromHeap(pImageBufferHeap, rpImageBuffer);
        pImageBufferHeap->unlockBuf(getNodeName());
    } else {
        rpImageBuffer = pImageBufferHeap->createImageBuffer();
    }

    if (rpImageBuffer == NULL) {
        MY_LOGEO("[node:%#" PRIxPTR "][stream buffer:%s] cannot create ImageBuffer",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }
    groupUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGDO_IF(mLogLevel >= 1, "stream %#" PRIx64 ": buffer: %p, usage: %x",
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
        MY_LOGEO("StreamId %#" PRIx64 ": pStreamBuffer == NULL", streamId);
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
        MY_LOGEO("[node:%#" PRIxPTR "][stream buffer:%s] cannot get metadata",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGDO_IF(0, "stream %#" PRIx64 ": stream buffer %p, metadata: %p",
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
    //MY_LOGDO(1, "frame %u done", pFrame->getFrameNo());
    onDispatchFrame(pFrame);
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
    sp<MetaHandle> pMeta_InApp      = mpInAppMeta_Request.get() ?
                                      MetaHandle::create(this, pFrame, mpInAppMeta_Request->getStreamId()) : NULL;

    sp<MetaHandle> pMeta_InHal      = mpInHalMeta_P1.get() ?
                                      MetaHandle::create(this, pFrame, mpInHalMeta_P1->getStreamId()) : NULL;

    sp<MetaHandle> pMeta_InAppRet   = mpInAppRetMeta_Request.get() ?
                                      MetaHandle::create(this, pFrame, mpInAppRetMeta_Request->getStreamId()) : NULL;

    sp<MetaHandle> pMeta_OutApp     = ( mpOutAppMeta_Result.get() &&
                                        isMetaStreamAtInfoIOMap(pFrame, mpOutAppMeta_Result->getStreamId()) )?
                                      MetaHandle::create(this, pFrame, mpOutAppMeta_Result->getStreamId()) : NULL;

    sp<MetaHandle> pMeta_OutHal     = ( mpOutHalMeta_Result.get() &&
                                        isMetaStreamAtInfoIOMap(pFrame, mpOutHalMeta_Result->getStreamId()) )?
                                      MetaHandle::create(this, pFrame, mpOutHalMeta_Result->getStreamId()) : NULL;

    if (CC_UNLIKELY( pMeta_InApp == NULL || pMeta_InHal == NULL) ) {
        MY_LOGWO("meta check failed");
        return BAD_VALUE;
    }

    // 3. get general info
    MINT32 uniqueKey = 0;
    MINT32 magicNo   = 0;
    MUINT32 requestNo = pFrame->getRequestNo();
    MUINT32 frameNo   = pFrame->getFrameNo();
    MINT32 iso       = 0;

    IMetadata *pHalMeta = pMeta_InHal->getMetadata();
    tryGetMetadata<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
    if( !tryGetMetadata<MINT32>(pHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNo) )
        MY_LOGW("no MTK_P1NODE_PROCESSOR_MAGICNUM in HalMeta");

    IMetadata *pAppRetMetaIn = pMeta_InAppRet->getMetadata();
    IMetadata *pAppMetaIn    = pMeta_InApp->getMetadata();
    if( CC_LIKELY(pMeta_InAppRet!=NULL) ) {
       tryGetMetadata<MINT32>(pAppRetMetaIn, MTK_SENSOR_SENSITIVITY, iso);
    }
    if( CC_LIKELY(pMeta_InAppRet!=NULL) && iso == (-1) ){
       if( !tryGetMetadata<MINT32>(pAppMetaIn, MTK_SENSOR_SENSITIVITY, iso) )
           MY_LOGW("no MTK_SENSOR_SENSITIVITY in AppMeta");
    }

    // 4. create FrameLifeHolder
    sp<FrameLifeHolder> pFrameLife = new FrameLifeHolder(mOpenId, this, pFrame, mLogLevel >= 1);

    // 5. Check video encoder direct link
    MINT32 highSpeedVdoFps = 0;
    MSize highSpeedVdoSize;
    MBOOL highSpeedCmdReceived = (
        tryGetMetadata<MINT32>(pHalMeta, MTK_P2NODE_HIGH_SPEED_VDO_FPS, highSpeedVdoFps) &&
        tryGetMetadata<MSize>(pHalMeta, MTK_P2NODE_HIGH_SPEED_VDO_SIZE, highSpeedVdoSize));

    if (highSpeedCmdReceived) {
        MY_LOGDO("received venc command. fps:%d, width:%d, hight:%d",
                 highSpeedVdoFps, highSpeedVdoSize.w, highSpeedVdoSize.h);

        mEnableVencStream = highSpeedVdoFps && highSpeedVdoSize.w && highSpeedVdoSize.h;
        if (mEnableVencStream) {
            if (!mInitVencStream) {
                MY_LOGDO("start the direct link of video encorder.");
                mpP2Processor->notify(P2Procedure::eP2_START_VENC_STREAM,
                        highSpeedVdoFps, highSpeedVdoSize.w, highSpeedVdoSize.h);
                mInitVencStream = MTRUE;
            }
        } else if (mInitVencStream) {
                MY_LOGDO("stop the direct link of video encorder.");
                mpP2Processor->notify(P2Procedure::eP2_STOP_VENC_STREAM);
                mInitVencStream = MFALSE;
        }
    }

    // 6. process image IO
    IPipelineFrame::ImageInfoIOMapSet &imageIOMapSet = IOMapSet.mImageInfoIOMapSet;
    // Each member in IOMapSet may use the same LCSO ImageStream
    // We should use the same buffer handle to store it
    sp<BufferHandle> lcsoBufHandle = NULL;
    for (size_t run_idx = 0; run_idx < imageIOMapSet.size(); run_idx++) {
        IPipelineFrame::ImageInfoIOMap const &imageIOMap = imageIOMapSet[run_idx];
        sp<Request> pRequest = new Request(
                pFrameLife,
                uniqueKey,
                requestNo,
                frameNo,
                run_idx*10+imageIOMapSet.size());

        pRequest->context.customOption = mCustomOption;
        pRequest->context.fd_stream_id = mpOutFd.get() ? mpOutFd->getStreamId() : -1;
        pRequest->context.capture_stream_id = mpOutCaptureImage.get() ? mpOutCaptureImage->getStreamId() : -1;
        pRequest->context.iso = iso;
#if SUPPORT_3RD_PARTY
        pRequest->setVendorMgr(mpVendor);
        pRequest->context.userId = muUserId;
        tryGetMetadata<MINT64>(pHalMeta, MTK_PLUGIN_P2_COMBINATION, pRequest->context.processorMask);
        if(pRequest->context.processorMask)
        {
            if(!mpVendor.get() &&
               (pRequest->context.processorMask&MTK_P2_YUV_PROCESSOR ||
                pRequest->context.processorMask&MTK_P2_RAW_PROCESSOR)
              )
            {
                MY_LOGE("Vendor not exit, but need pass vendor");
            }

            MY_LOGDO_IF(mLogLevel, "VendorMgr addr: %p, userId %" PRIX64 ", combination: %" PRIX64 "",
                        mpVendor.get(), muUserId, pRequest->context.processorMask);
        }
        else
        {
            pRequest->context.processorMask = MTK_P2_ISP_PROCESSOR | MTK_P2_MDP_PROCESSOR;
            MY_LOGWO_IF(mLogLevel,"p2 combination not set, set to %" PRIX64 ": ISP->MDP", pRequest->context.processorMask);
        }

#endif

        if (mLogLevel >= 2) {
            MY_LOGDO("[StreamID] run_idx(%zu) vIn.size(%zu) +++",
                    run_idx, imageIOMap.vIn.size());
            for (size_t i = 0; i < imageIOMap.vIn.size(); i++) {
                StreamId_T const sId = imageIOMap.vIn.keyAt(i);
                MY_LOGDO("[StreamID] In(%zu) sId(%#" PRIx64 ") F(%d) O(%d) Y(%d) R(%d)",
                        i, sId, isFullRawLocked(sId), isOpaqueRawLocked(sId),
                        isYuvReprocLocked(sId), isResizeRawLocked(sId));
                if (0 && isYuvReprocLocked(sId)) {
                    MUINT8 uEdgeMode = 0;
                    tryGetMetadata<MUINT8>(pMeta_InApp->getMetadata(), MTK_EDGE_MODE, uEdgeMode);
                    MY_LOGD_IF(1, "YUV Reprocessing - frame:%d, edge:%d" ,frameNo, uEdgeMode);
                }
            }
            MY_LOGDO("[StreamID] run_idx(%zu) vIn.size(%zu) ---",
                    run_idx, imageIOMap.vIn.size());
        }
        // source
        for (size_t i = 0; i < imageIOMap.vIn.size(); i++)
        {
            StreamId_T const sId = imageIOMap.vIn.keyAt(i);
            if (isLcsoRawLocked(sId)) {
                if (lcsoBufHandle == NULL)
                    lcsoBufHandle = StreamBufferHandle::create(this, pFrame, sId);
                pRequest->context.in_lcso_buffer = lcsoBufHandle;
            } else {
                sp<BufferHandle> pBufferHandle = StreamBufferHandle::create(this, pFrame, sId);
                if(pBufferHandle.get() != NULL)
                {
                    pRequest->context.in_buffer.push_back(pBufferHandle);
                    pRequest->context.is_yuv_reproc = isYuvReprocLocked(sId);
                    {
                        RWLock::AutoRLock _l(mConfigRWLock);
                        pRequest->context.resized = isResizeRawLocked(sId);
                    }
                }
            }
        }
        if (pRequest->context.in_buffer.size() == 0)
        {
            MY_LOGW("sensor(%d) get input buffer failed", mOpenId);
            return BAD_VALUE;
        }
#if SUPPORT_3RD_PARTY
        else if(pRequest->context.in_buffer.size() > 1 )
        {
            // need check if Request be bypass mode of raw plugin
            if(! (pRequest->context.processorMask&MTK_P2_RAW_PROCESSOR) )
                MY_LOGE("not support multi-raw input in A iomap");
        }
#endif
        // determine whether burst or not
#if FORCE_BURST_ON
        pRequest->context.burst_num = pRequest->context.resized ? 4 : 0;
#else
        pRequest->context.burst_num = pRequest->context.resized ? mBurstNum : 0;
#endif
        // determine whether direck link to video encorder or not
        pRequest->context.enable_venc_stream = mEnableVencStream && mInitVencStream;

        // downscale: slowmotion off
        MUINT32 idx = 1;
        MINT32 downscaleThres = property_get_int32("debug.camera.p2node.2run.threshold", -1); // threshold > 0, iso > threshold will enable downscale
        MINT32 downscaleRatio = property_get_int32("debug.camera.p2node.2run.ratio", 2);     // downscale ratio range=1~2, default:2
        MINT32 forceDownscale = property_get_int32("debug.camera.p2node.force_downscale", -1);// 0 or 1
        MINT32 x = -1;
#if MTK_CAM_NEW_NVRAM_SUPPORT
        NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT* t = (NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT*)getTuningFromNvram(mOpenId, idx, magicNo, NVRAM_TYPE_SWNR_THRES, mLogLevel);
        if(downscaleThres < 0) {
            if(t!=NULL) {
                downscaleThres = t->downscale_thres;
                downscaleRatio = t->downscale_ratio;
                x = t->x;
            }
            else MY_LOGE("query nvram fail");
        } else {
            MY_LOGD("[debug mode] set downscale thres:%d ratio:1/%d", downscaleThres, downscaleRatio);
        }
#else
        forceDownscale = 0;
#endif
        pRequest->context.downscaleRatio = downscaleRatio;
        pRequest->context.downscaleThres = downscaleThres;
        pRequest->context.swnr_thres_temp = x;
        pRequest->context.is2run =   (pRequest->context.customOption&CUSTOM_OPTION_DOWNSCALE_DN) &&
                                     (downscaleRatio>0) &&
                                     (pRequest->context.iso > downscaleThres) &&
                                     (pRequest->context.burst_num < 2);
        if(forceDownscale > -1)
            pRequest->context.is2run = forceDownscale;
        MY_LOGD("fn(%d),rq(%d),idx(%d),cusOp(%d),mask(%" PRIX64 "),burst(%d),ds_t(%d)/r(%d)/f(%d),iso(%d),x(%d)",
                    frameNo, requestNo,
                    run_idx*10+imageIOMapSet.size(),
                    pRequest->context.customOption, pRequest->context.processorMask,
                    pRequest->context.burst_num,
                    downscaleThres, downscaleRatio, forceDownscale,
                    iso,x
                );
        // destination
        MBOOL supportDRE = mCustomOption & NSCam::v3::P2Node::CUSTOM_OPTION_DRE_SUPPORT;
        IMetadata *inHal = pMeta_InHal->getMetadata();
        MBOOL enableDRE = MFALSE;
        if (inHal!=NULL &&
            tryGetMetadata<MINT32>(inHal, MTK_P2NODE_CTRL_CALTM_ENABLE, enableDRE)) {
            MY_LOGDO_IF(mLogLevel , "supportDRE(%d) meta enableDRE(%d)", supportDRE, enableDRE);
            supportDRE = supportDRE && enableDRE;
        } else {
            MY_LOGWO_IF(mLogLevel , "no MTK_P2NODE_CTRL_CALTM_ENABLE from halMeta");
        }
        for (size_t i = 0; i < imageIOMap.vOut.size(); i++) {
            StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
            //MUINT32 const transform = imageIOMap.vOut.valueAt(i)->getTransform();
            sp<BufferHandle> pBufferHandle = StreamBufferHandle::create(this, pFrame, streamId);
            pRequest->context.out_buffers.push_back(pBufferHandle);
            MINT32 forceDRE = ::property_get_int32("debug.camera.force.dre", 0);

            if((forceDRE||supportDRE) && (streamId==pRequest->context.capture_stream_id )) {
                pRequest->context.captureFrame = true;
            }
#if 0
            if (pBufferHandle.get() &&
                imageIOMap.vOut[i]->getUsageForAllocator() & GRALLOC_USAGE_HW_VIDEO_ENCODER) {

                if (OK != pBufferHandle->waitState(BufferHandle::STATE_READABLE))
                    MY_LOGWO("get video buffer failed");

                if (pBufferHandle->getBuffer()) {
                    MSize size = pBufferHandle->getBuffer()->getImgSize();
                    MY_LOGDO("[debug] high speed recording.  width:%d, hight:%d",
                             size.w, size.h);
                }
            }
#endif
        }

        pRequest->context.in_app_meta = pMeta_InApp;
        pRequest->context.in_hal_meta = pMeta_InHal;
#if SUPPORT_3RD_PARTY
        pRequest->context.in_app_ret_meta = pMeta_InAppRet;
        pRequest->context.out_app_meta = pMeta_OutApp;
        pRequest->context.out_hal_meta = pMeta_OutHal;
#else
        if (run_idx == 0) {
            pRequest->context.out_app_meta = pMeta_OutApp;
            pRequest->context.out_hal_meta = pMeta_OutHal;
        }
#endif
#if SUPPORT_PLUGIN
        PostProcessing::PreConditionParams preConditionParams;
        preConditionParams.pInAppMeta = pRequest->context.in_app_meta->getMetadata();
        preConditionParams.pInHalMeta = pRequest->context.in_hal_meta->getMetadata();
        preConditionParams.bResized = pRequest->context.resized;
        // Yuv plugin decision
        if (mpPreYuvProcessor != NULL && mpYuvPostProcessing->doOrNot(preConditionParams)) {
            pRequest->context.run_plugin_yuv = MTRUE;
            MUINT32 outImgForamt = (mpYuvPostProcessing->profile().outImageFormat == PostProcessing::eIMG_FMT_DEFAULT) ?
                                   eImgFmt_YUY2 :
                                   mpYuvPostProcessing->profile().inImageFormat;
            pRequest->context.work_buffer_format = mpYuvPostProcessing->profile().inImageFormat;
        }
        // Raw plugin decision

        if (mpPreRawProcessor != NULL && (mCustomOption&CUSTOM_OPTION_SENSOR_4CELL) && mpRawPostProcessing->doOrNot(preConditionParams)) {
            pRequest->context.run_plugin_raw = MTRUE;
            mpPreRawProcessor->queueRequest(pRequest);
            #if SUPPORT_3RD_PARTY
            pRequest->context.mbSkipNextProcessor = MTRUE;
            #endif
            MY_LOGD("skip raw processor.");
        }
        else
#endif
       {
            // prevent HAL1 where a frame has no buffer from dispatching to next node faster than this scope
            // when queued to processor, ensure that no one except request holds the buffer handle
            if (run_idx == imageIOMapSet.size() - 1) {
                pMeta_InApp.clear();
                pMeta_InHal.clear();
                pMeta_InAppRet.clear();
                pMeta_OutApp.clear();
                pMeta_OutHal.clear();
            }
 #if SUPPORT_3RD_PARTY
         if(1)
         {
            if( !(pRequest->context.processorMask & MTK_P2_RAW_PROCESSOR))
            {
                pRequest->context.mbSkipNextProcessor = MTRUE;
                //pRequest->context.processorMask = MTK_P2_ISP_PROCESSOR | MTK_P2_MDP_PROCESSOR;
                MY_LOGDO_IF(mLogLevel >= 2, "skip, combination: %" PRIX64 " ", pRequest->context.processorMask);
            } else {
                pRequest->context.mbSkipNextProcessor = MFALSE;
            }
            mpRawProcessor->queueRequest(pRequest);
         }
        else
#endif
            mpP2Processor->queueRequest(pRequest);
       }
    } // for imageIOMapSet.size()
    return OK;
}
