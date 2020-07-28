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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "MtkCam/DualFeatureNode"
//
#include "hwnode_utilities.h"
//
#include <mtkcam/pipeline/hwnode/DualFeatureNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
#include <utils/String8.h>
// for metadata
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/DualCam/IDualCamStreamingFeaturePipe.h>
#include <mtkcam/feature/DualCam/DualFeatureCommon.h>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <mtkcam/utils/std/Misc.h>
#include <mtkcam/utils/std/Trace.h>
//
#include <list>
#include <algorithm>
//
#include <chrono>
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

#define BY_PASS_FEATURE_PIPE 3389

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

// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
/******************************************************************************
 *
 ******************************************************************************/
class DualFeatureNodeImp :
    public BaseNode,
    public DualFeatureNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DualFeatureNodeImp();
    virtual ~DualFeatureNodeImp();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MERROR init(InitParams const& rParams) override;
    MERROR config(ConfigParams const& rParams) override;
    MERROR uninit() override;
    MERROR flush() override;
    MERROR queue(sp<IPipelineFrame> pFrame) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  callback function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static MBOOL featurePipeCB(
                NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam::MSG_TYPE msg,
                NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam &data);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  process done function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MVOID onProcessFinish(
                NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam::MSG_TYPE msg,
                NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam &data);
    MVOID dumpDebugInfo(std::shared_ptr<NSCam::v3::DualFeature::process_frame> pFrame);
private:
    MERROR getImageBuffer(
                android::sp<IPipelineFrame> const& pFrame,
                StreamId_T const streamId,
                sp<IImageStreamBuffer>& rpStreamBuffer,
                sp<IImageBuffer>& rpImageBuffer,
                MBOOL const isInStream
            );
    MERROR checkMetadata(
                std::shared_ptr<NSCam::v3::DualFeature::process_frame> const pProcessFrame);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  config stream member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<IMetaStreamInfo> mpInAppMeta = nullptr;
    sp<IMetaStreamInfo> mpInHalMeta = nullptr;
    sp<IMetaStreamInfo> mpInAppRetMeta = nullptr;
    sp<IMetaStreamInfo> mpOutAppMeta = nullptr;
    sp<IMetaStreamInfo> mpOutHalMeta = nullptr;
    //
    Vector<sp<IImageStreamInfo> > mpvInFullRaw;
    //
    sp<IImageStreamInfo> mpInResizedRaw = nullptr;
    sp<IImageStreamInfo> mpInLcsoRaw = nullptr;
    ImageStreamInfoSetT mpvOutImage;
    sp<IImageStreamInfo> mpOutFDImage = nullptr;
    //
    MINT32 mLogLevel = 0;
    MINT mDualCamMode = -1;
    //
    NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe* mpPipe = nullptr;
    // to check whether param is callback or not.
    RWLock          mRequestQueueLock;
    Vector<MUINT32> mEnequeIdSet;
    // keep main1 & main2 id
    MINT32 mMain1Id = -1;
    MINT32 mMain2Id = -1;
    //
    Vector<StreamId_T> mNeedStreamIdList;
    //
    MBOOL           mSkipCheck = MTRUE;
};
/******************************************************************************
 *
 ******************************************************************************/
sp<DualFeatureNode>
DualFeatureNode::
createInstance()
{
    return new DualFeatureNodeImp();
}
/******************************************************************************
 *
 ******************************************************************************/
DualFeatureNodeImp::
DualFeatureNodeImp()
{
    FUNCTION_SCOPE;
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 )
    {
        mLogLevel = ::property_get_int32("debug.camera.log.dualfeaturenode", 0);
    }
    if ( 0 == mLogLevel )
    {
        mLogLevel = ::property_get_int32("debug.camera.log.basenode", 0);
    }
#if MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT
    mSkipCheck = MTRUE;
#else
    mSkipCheck = MFALSE;
#endif
    MINT32 enableInitReq = ::property_get_int32("debug.camera.initreq", -1);
    if(enableInitReq > 0)
    {
        mSkipCheck = MTRUE;
    }
    else if(enableInitReq == 0)
    {
        mSkipCheck = MFALSE;
    }
    MY_LOGD("log(%d) skipcheck(%d)", mLogLevel, mSkipCheck);
}
/******************************************************************************
 *
 ******************************************************************************/
DualFeatureNodeImp::
~DualFeatureNodeImp()
{
    FUNCTION_SCOPE;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
init(
    InitParams const& rParams
)
{
    CAM_TRACE_NAME("DualFeatureNode:init");
    FUNCTION_SCOPE;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    mpvInFullRaw.clear();
    mpvOutImage.clear();
    mEnequeIdSet.clear();
    mNeedStreamIdList.clear();
    //
    MY_LOGD("OpenId %d, mNodeId %d, mNodeName %s mLogLevel %d",
            getOpenId(),
            getNodeId(),
            getNodeName(),
            mLogLevel);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
config(
    ConfigParams const& rParams
)
{
    FUNCTION_SCOPE;
    mpInAppMeta = rParams.pInAppMeta;
    mpInHalMeta = rParams.pInHalMeta;
    mpInAppRetMeta = rParams.pInAppRetMeta;
    mpOutAppMeta = rParams.pOutAppMeta;
    mpOutHalMeta = rParams.pOutHalMeta;
    //
    mpvInFullRaw = rParams.pvInFullRaw;
    //
    mpInResizedRaw = rParams.pInResizedRaw;
    mpInLcsoRaw = rParams.pInLcsoRaw;
    //
    mpvOutImage = rParams.vOutImage;
    //
    mpOutFDImage = rParams.pOutFDImage;
    //
    mDualCamMode = rParams.iDualCamMode;
    NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe::UsageHint usageHint;
    usageHint.mStreamingSize = rParams.rrzoSize;
    // create dual bokeh feature pipe
    mpPipe = NSCam::NSCamFeature::NSFeaturePipe::IDualCamStreamingFeaturePipe::createInstance(
                                getOpenId(),
                                usageHint);
    if(mpPipe == nullptr)
    {
        MY_LOGE("create pipe fail.");
        return UNKNOWN_ERROR;
    }
    mpPipe->init(LOG_TAG);
    // query main1 & main2 id
    if(!StereoSettingProvider::getStereoSensorIndex(mMain1Id, mMain2Id))
    {
        MY_LOGE("cannot get sensor id");
        return UNKNOWN_ERROR;
    }
    //
    /******************************************************************************
     * dump stream info
     ******************************************************************************/
    {
        String8 value("");
        auto dumpMetaStreamInfo = [&value](const sp<IMetaStreamInfo>& info)
        {
            if(info == nullptr)
            {
                return;
            }
            value += String8::format("streamInfo(%#" PRIx64 ") name(%s)\n",
                    info->getStreamId(), info->getStreamName());
        };
        auto dumpImgStreamInfo = [&value](const sp<IImageStreamInfo>& info)
        {
            if(info == nullptr)
            {
                return;
            }
            value += String8::format("streamInfo(%#" PRIx64 ") name(%s) format(%#08x) size(%dx%d)\n",
                    info->getStreamId(), info->getStreamName(),
                    info->getImgFormat(), info->getImgSize().w, info->getImgSize().h);
        };
        //
        dumpMetaStreamInfo(mpInAppMeta);
        dumpMetaStreamInfo(mpInHalMeta);
        dumpMetaStreamInfo(mpInAppRetMeta);
        dumpMetaStreamInfo(mpOutAppMeta);
        dumpMetaStreamInfo(mpOutHalMeta);
        for(auto streaminfo : mpvInFullRaw)
        {
            dumpImgStreamInfo(streaminfo);
        }
        dumpImgStreamInfo(mpInResizedRaw);
        dumpImgStreamInfo(mpInLcsoRaw);
        for(auto streaminfo : mpvOutImage)
        {
            dumpImgStreamInfo(streaminfo);
        }
        dumpImgStreamInfo(mpOutFDImage);
        //
        value += String8::format("dual cam mode(%d)\n", mDualCamMode);
        MY_LOGI("%s", value.string());
    }
    // set needed stream by openid
    if(mMain1Id == getOpenId())
    {
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_RAW_RESIZER); // rrzo
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_RAW_LCSO); // lcso
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_YUV_00); // preview buffer
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV); // dmbg working buffer
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV); // depth working buffer
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV);// main image
        mNeedStreamIdList.add(eSTREAMID_META_APP_CONTROL);
        mNeedStreamIdList.add(eSTREAMID_META_HAL_DYNAMIC_P1);
        mNeedStreamIdList.add(eSTREAMID_META_APP_DYNAMIC_P2);
        mNeedStreamIdList.add(eSTREAMID_META_HAL_DYNAMIC_P2);
    }
    else
    {
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_RAW_RESIZER); // rrzo
        mNeedStreamIdList.add(eSTREAMID_IMAGE_PIPE_RAW_LCSO); // lcso
        mNeedStreamIdList.add(eSTREAMID_META_APP_CONTROL);
        mNeedStreamIdList.add(eSTREAMID_META_HAL_DYNAMIC_P1);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
uninit()
{
    FUNCTION_SCOPE;
    //
    if (OK != flush())
        MY_LOGE("sensor(%d) flush failed", mOpenId);
    //
    if(mpPipe)
    {
        if(!mpPipe->uninit(LOG_TAG))
        {
            MY_LOGE("sensor(%d) pipe uninit failed", mOpenId);
        }
        mpPipe->destroyInstance();
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
flush()
{
    if(mpPipe)
    {
        mpPipe->flush();
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
queue(
    sp<IPipelineFrame> pFrame
)
{
    CAM_TRACE_NAME("dfn:queue");
    if(!pFrame.get())
    {
        MY_LOGE("null pipeline frame");
        return BAD_VALUE;
    }
    IPipelineFrame::InfoIOMapSet IOMapSet;
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    std::shared_ptr<NSCam::v3::DualFeature::process_frame> pProcessFrame = nullptr;
    //
    auto isNeededStream = [&](StreamId_T streamid)
    {
        Vector<StreamId_T>::iterator iter = std::find(mNeedStreamIdList.begin(), mNeedStreamIdList.end(), streamid);
        if(iter != mNeedStreamIdList.end())
        {
            return true;
        }
        return false;
    };
    //
    auto suspendThisFrame = [&pFrame, &pProcessFrame, &streamBufferSet, this]()
    {
        MY_LOGE("Discard request id=%d frame no=%d", pFrame->getRequestNo(), pFrame->getFrameNo());
        //
        pProcessFrame->uninit(MFALSE, getNodeName());
        pProcessFrame = nullptr;

        // mark input/output buffer to release
        MERROR err = BaseNode::flush(pFrame);
        return err;
    };
    //
    if(OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )) {
        MY_LOGE("Get IOMapSet failed.");
        return BAD_VALUE;
    }
    pProcessFrame = std::shared_ptr<NSCam::v3::DualFeature::process_frame>(
                            new NSCam::v3::DualFeature::process_frame(
                                                        pFrame,
                                                        getOpenId(),
                                                        pFrame->getRequestNo(),
                                                        getNodeId(),
                                                        mLogLevel),
                                                        [&](NSCam::v3::DualFeature::process_frame* p)
                                                        {
                                                            if(p!=nullptr)
                                                            {
                                                                delete p;
                                                                p = nullptr;
                                                            }
                                                        });
    // 1. parse metadata
    IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];
    {
        sp<IMetaStreamBuffer> pStreamBuffer = nullptr;
        StreamId_T streamId = 0;
        IMetadata* pMetadata = nullptr;
        // in metadata
        for( size_t i=0; i<metaIOMap.vIn.size() ; ++i )
        {
            streamId = metaIOMap.vIn.valueAt(i)->getStreamId();
            MERROR const err = ensureMetaBufferAvailable_(
                                            pProcessFrame->getFrame()->getFrameNo(),
                                            streamId,
                                            streamBufferSet,
                                            pStreamBuffer);
            if(err == OK)
            {
                pMetadata = pStreamBuffer->tryReadLock(getNodeName());
                MY_LOGD_IF(mLogLevel > 0, "streamId (%llx) metadata(%p)", streamId, pMetadata);
                if(pMetadata != nullptr)
                {
                    pProcessFrame->mvInMetaStreamBuffer.add(streamId, pStreamBuffer);
                    pProcessFrame->mvInMetaBufferContainer.add(streamId, pMetadata);
                }
                else
                {
                    MY_LOGW("lock metadata fail %#" PRIx64 ".");
                    if(isNeededStream(streamId))
                    {
                        return suspendThisFrame();
                    }
                }
            }
            else
            {
                MY_LOGW("getImageBuffer err = %d StreamId(%#" PRIx64 ") name (%s)",
                                    err,
                                    streamId,
                                    metaIOMap.vIn.valueAt(i)->getStreamName());
                if(isNeededStream(streamId))
                {
                    return suspendThisFrame();
                }
            }
        }
    }
    // 2. parse output metadata
    {
        sp<IMetaStreamBuffer> pStreamBuffer = nullptr;
        StreamId_T streamId = 0;
        IMetadata* pMetadata = nullptr;
        for( size_t i=0; i<metaIOMap.vOut.size() ; ++i )
        {
            streamId = metaIOMap.vOut.valueAt(i)->getStreamId();
            MERROR const err = ensureMetaBufferAvailable_(
                                            pProcessFrame->getFrame()->getFrameNo(),
                                            streamId,
                                            streamBufferSet,
                                            pStreamBuffer);
            if(err == OK)
            {
                pMetadata = pStreamBuffer->tryWriteLock(getNodeName());
                MY_LOGD_IF(mLogLevel > 0, "streamId (%llx) metadata(%p)", streamId, pMetadata);
                if(pMetadata != nullptr)
                {
                    pProcessFrame->mvOutMetaStreamBuffer.add(streamId, pStreamBuffer);
                    pProcessFrame->mvOutMetaBufferContainer.add(streamId, pMetadata);
                }
                else
                {
                    MY_LOGW("lock metadata fail %#" PRIx64 ".");
                    if(isNeededStream(streamId))
                    {
                        return suspendThisFrame();
                    }
                }
            }
            else
            {
                MY_LOGW("getImageBuffer err = %d StreamId(%#" PRIx64 ") name (%s)",
                                    err,
                                    streamId,
                                    metaIOMap.vOut.valueAt(i)->getStreamName());
                if(isNeededStream(streamId))
                {
                    return suspendThisFrame();
                }
            }
        }
    }
    // 3. parse input buffer
    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
    {
        StreamId_T streamId = 0;
        sp<IImageStreamBuffer> pStreamBuffer = nullptr;
        sp<IImageBuffer> pImgBuffer = nullptr;
        for( size_t i=0; i<imageIOMap.vIn.size() ; ++i )
        {
            streamId = imageIOMap.vIn.valueAt(i)->getStreamId();
            MERROR const err = getImageBuffer(
                        pFrame,
                        streamId,
                        pStreamBuffer,
                        pImgBuffer,
                        MTRUE
                        );
            if(err == OK)
            {
                MY_LOGD_IF(mLogLevel > 0, "streamId (%llx)", streamId);
                // query group usage
                MUINT const groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
                pImgBuffer->lockBuf(getNodeName(), groupUsage);
                pProcessFrame->mvInImgStreamBuffer.add(streamId, pStreamBuffer);
                pProcessFrame->mvInImgBufContainer.add(streamId, pImgBuffer);
            }
            else
            {
                MY_LOGW("getImageBuffer err = %d StreamId(%#" PRIx64 ")", err, streamId);
                if(isNeededStream(streamId))
                {
                    return suspendThisFrame();
                }
            }
        }
    }
    // 4. parse output buffer
    {
        StreamId_T streamId = 0;
        sp<IImageStreamBuffer> pStreamBuffer = nullptr;
        sp<IImageBuffer> pImgBuffer = nullptr;
        for( size_t i=0; i<imageIOMap.vOut.size() ; ++i )
        {
            streamId = imageIOMap.vOut.valueAt(i)->getStreamId();
            MERROR const err = getImageBuffer(
                        pFrame,
                        streamId,
                        pStreamBuffer,
                        pImgBuffer,
                        MFALSE
                        );
            if(err == OK)
            {
                MY_LOGD_IF(mLogLevel > 0, "streamId (%llx)", streamId);
                // query group usage
                MUINT const groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
                pImgBuffer->lockBuf(getNodeName(), groupUsage);
                pProcessFrame->mvOutImgStreamBuffer.add(streamId, pStreamBuffer);
                pProcessFrame->mvOutImgBufContainer.add(streamId, pImgBuffer);
            }
            else
            {
                MY_LOGW("getImageBuffer err = %d StreamId(%#" PRIx64 ")", err, streamId);
                if(isNeededStream(streamId))
                {
                    return suspendThisFrame();
                }
            }
        }
    }
    if(mLogLevel > 0)
    {
        dumpDebugInfo(pProcessFrame);
    }
    // if output buffer is zero, suspendThisFrame
    if(pProcessFrame->mvOutImgBufContainer.size() == 0 && getOpenId() != mMain2Id)
    {
        MY_LOGD("no output frame, drop");
        return suspendThisFrame();
    }
    // re-pack metadata (for timestamp)
    if(OK != checkMetadata(pProcessFrame))
    {
        return suspendThisFrame();
    }
    // prepare feature param
    NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam featureEnqueParams(featurePipeCB);
    // add process_frame
    featureEnqueParams.mVarMap.set<std::shared_ptr<NSCam::v3::DualFeature::process_frame> >(
                                                VAR_DUAL_FEATURE_PROCESS_DATA, pProcessFrame);
    // add dual feature node intance
    featureEnqueParams.mVarMap.set<DualFeatureNodeImp*>(VAR_DUAL_FEATURE_INSTANCE, this);
    NSCam::NSCamFeature::NSFeaturePipe::IDualCamStreamingFeaturePipe::prepareFeatureData(
                                            featureEnqueParams,
                                            pProcessFrame->getFrame()->getRequestNo(),
                                            pProcessFrame->mvInMetaBufferContainer.editValueFor(mpInAppMeta->getStreamId()),
                                            pProcessFrame->mvInMetaBufferContainer.editValueFor(mpInHalMeta->getStreamId()),
                                            pProcessFrame->mvOutMetaBufferContainer.editValueFor(mpOutAppMeta->getStreamId()),
                                            pProcessFrame->mvOutMetaBufferContainer.editValueFor(mpOutHalMeta->getStreamId()),
                                            mDualCamMode);
    // workaround for sync manager. (syncmanager will check FeaturePipeParam output)
    {
        NSCam::NSIoPipe::FrameParams frameParams;
        frameParams.mvOut.add(NSCam::NSIoPipe::Output());
        featureEnqueParams.mQParams.mvFrameParams.add(frameParams);
    }
    // enque to dual streaming pipe
    if(mpPipe)
    {
        CAM_TRACE_NAME("dfn:enque to pipe");
        if(mLogLevel == BY_PASS_FEATURE_PIPE)
        {
            {
                RWLock::AutoWLock _l(mRequestQueueLock);
                MY_LOGD("[%d] add frame id(%d)", getOpenId(), pProcessFrame->getFrame()->getFrameNo());
                mEnequeIdSet.add(pProcessFrame->getFrame()->getFrameNo());
            }
            onProcessFinish(
                        NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam::MSG_TYPE::MSG_FRAME_DONE,
                        featureEnqueParams);
        }
        else
        {
            // add first
            {
                RWLock::AutoWLock _l(mRequestQueueLock);
                MY_LOGD_IF(mLogLevel > 0, "[%d] add frame id(%d) req(%d)",
                                    getOpenId(),
                                    pProcessFrame->getFrame()->getFrameNo(),
                                    pProcessFrame->getFrame()->getRequestNo());
                mEnequeIdSet.add(pProcessFrame->getFrame()->getFrameNo());
            }
            if(!mpPipe->enque(featureEnqueParams))
            {
                RWLock::AutoWLock _l(mRequestQueueLock);
                MY_LOGE("enque fail, please check previous log");
                // if fail remove request id
                Vector<MUINT32>::iterator iter = std::find(
                                                        mEnequeIdSet.begin(),
                                                        mEnequeIdSet.end(),
                                                        pProcessFrame->getFrame()->getFrameNo());
                if(iter!=mEnequeIdSet.end())
                {
                    MY_LOGD_IF(mLogLevel > 0, "[%d] remove frame id(%d)", getOpenId(), pProcessFrame->getFrame()->getFrameNo());
                    mEnequeIdSet.erase(iter);
                }
            }
        }
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualFeatureNodeImp::
featurePipeCB(
    NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam::MSG_TYPE msg,
    NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam &data
)
{
    DualFeatureNodeImp* _instance = nullptr;
    if(!data.mVarMap.tryGet<DualFeatureNodeImp*>(VAR_DUAL_FEATURE_INSTANCE, _instance))
    {
        MY_LOGE("get instance fail. should not happened");
        return MFALSE;
    }
    if(_instance == nullptr)
    {
        MY_LOGE("_instance is nullptr");
        return MFALSE;
    }
    _instance->onProcessFinish(msg, data);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualFeatureNodeImp::
onProcessFinish(
    NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam::MSG_TYPE msg,
    NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam &data
)
{
    std::shared_ptr<NSCam::v3::DualFeature::process_frame> frame = nullptr;
    data.mVarMap.tryGet<std::shared_ptr<NSCam::v3::DualFeature::process_frame> >(
                                        VAR_DUAL_FEATURE_PROCESS_DATA,
                                        frame);
    if(frame != nullptr)
    {
        sp<IPipelineFrame> const pFrame = frame->getFrame();
        if(pFrame == 0)
        {
            MY_LOGE("mpFrame is null");
        }
        MBOOL vaild = MFALSE;
        MUINT32 frameId = pFrame->getFrameNo();
        MY_LOGD_IF(mLogLevel > 0, "[%d] reqid: %d frameNo:%d msg:%d dequeSuccess:%d",
                            getOpenId(),
                            pFrame->getRequestNo(),
                            frameId,
                            msg,
                            data.mQParams.mDequeSuccess);
        if(NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam::MSG_TYPE::MSG_FRAME_DONE == msg)
        {
            // finish
            if(data.mQParams.mDequeSuccess)
            {
                vaild = MTRUE;
            }
            else
            {
                vaild = MFALSE;
            }
            //
            // if this op is main2, it has to drop main2.
            // otherwise, it has to control display client return order.(this order needs deal with main1 return order)
            if(getOpenId() == mMain2Id)
            {
                frame->uninit(MFALSE, getNodeName());
                // main2 no needs, just flush this frame.
                // In flush implement, it will dispatch this frame.
                BaseNode::flush(pFrame);
            }
            else
            {
                frame->uninit(vaild, getNodeName());
                onDispatchFrame(pFrame);
            }
            frame = nullptr;
            {
                RWLock::AutoWLock _l(mRequestQueueLock);
                Vector<MUINT32>::iterator iter = std::find(mEnequeIdSet.begin(), mEnequeIdSet.end(), frameId);
                if(iter!=mEnequeIdSet.end())
                {
                    MY_LOGD_IF(mLogLevel > 0, "[%d] remove frame id(%d)", getOpenId(), frameId);
                    mEnequeIdSet.erase(iter);
                }
            }
        }
        else
        {
            //partial release
        }
    }
    else
    {
        MY_LOGE("frame missing, should not happened");
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualFeatureNodeImp::
dumpDebugInfo(
    std::shared_ptr<NSCam::v3::DualFeature::process_frame> pFrame
)
{
    if(pFrame != nullptr)
    {
        MY_LOGD("dump debug");
        pFrame->dumpInfo();
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
getImageBuffer(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    MBOOL const isInStream
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    MY_LOGD_IF(mLogLevel>0, "RequestNo(%d) StreamId(%#" PRIx64 ")", pFrame->getRequestNo(), streamId);
    if(streamId == 0)
    {
        MY_LOGE("StreamId is 0");
        return BAD_VALUE;
    }
    MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
    {
        MY_LOGD_IF(mLogLevel>0, "ensureImageBufferAvailable_ fail");
        return err;
    }

    if(isInStream)
    {
        pImageBufferHeap = rpStreamBuffer->tryReadLock(getNodeName());
    }
    else{
        pImageBufferHeap = rpStreamBuffer->tryWriteLock(getNodeName());
    }

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    MY_LOGD_IF(mLogLevel>0, "@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }

    MY_LOGD_IF(mLogLevel>0, "stream buffer: (%llx) %p, heap: %p, buffer: %p",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get());

    return OK;
}
/******************************************************************************
 * some metadatas are store in different metadata, so it has to re-pack it manually.
 ******************************************************************************/
MERROR
DualFeatureNodeImp::
checkMetadata(
    std::shared_ptr<NSCam::v3::DualFeature::process_frame> const pProcessFrame
)
{
    MERROR ret = UNKNOWN_ERROR;
    IMetadata* pInHalMetadata = nullptr;
    IMetadata* pInHalRetMetadata = nullptr;
    IMetadata* pInAppMetadata = nullptr;
    MINT64 timestamp = 0;
    MINT32 iso = -1;
    MUINT8 afState = 0;
    MINT32 afTopLeftX = 0, afTopLeftY = 0, afBottomRightX = 0, afBottomRightY = 0;
    // ---------------------------------------------------------------------------------------------
    ssize_t index = pProcessFrame->mvInMetaBufferContainer.indexOfKey(mpInHalMeta->getStreamId());
    if(index >= 0)
    {
        pInHalMetadata = pProcessFrame->mvInMetaBufferContainer.editValueAt(index);
    }
    else
    {
        MY_LOGE("Get mpInHalMeta fail.");
        goto lbExit;
    }
    // ---------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------
    index = pProcessFrame->mvInMetaBufferContainer.indexOfKey(mpInAppRetMeta->getStreamId());
    if(index >= 0)
    {
        pInHalRetMetadata = pProcessFrame->mvInMetaBufferContainer.editValueAt(index);
    }
    else
    {
        MY_LOGE("Get mpInAppRetMeta fail.");
        goto lbExit;
    }
    // ---------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------
    {
        //
        if(pInHalMetadata != nullptr)
        {
            if(!tryGetMetadata<MINT64>(pInHalMetadata, MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp))
            {
                MY_LOGE("Get timestamp fail.");
                goto lbExit;
            }
        }
    }
    // ---------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------
    {
        if(pInHalRetMetadata != nullptr)
        {
            if(!tryGetMetadata<MINT32>(pInHalRetMetadata, MTK_SENSOR_SENSITIVITY, iso))
            {
                MY_LOGW("cannot get iso");
                goto lbExit;
            }
        }
    }
    // ---------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------
    {
        if(pInHalRetMetadata != nullptr)
        {
            if(!tryGetMetadata<MUINT8>(pInHalRetMetadata, MTK_CONTROL_AF_STATE, afState))
            {
                MY_LOGW("cannot get afstate in pInHalRetMetadata");
                goto lbExit;
            }
            IMetadata::IEntry entry = pInHalRetMetadata->entryFor(MTK_3A_FEATURE_AF_ROI);
            // af roi
            if(entry.isEmpty())
            {
                MY_LOGW("cannot get af region");
                goto lbExit;
            }
        }
    }
    // ---------------------------------------------------------------------------------------------
    ret = OK;
    mSkipCheck = MFALSE;
lbExit:
    if(mSkipCheck)
    {
        MY_LOGW("skip check result");
        return OK;
    }
    return ret;
}