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

#include "PipelineModelImpl_ISP.h"
#include "EventLog.h"
#include "MyUtils.h"

#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/IFDContainer.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam3/pipeline/hwnode/P2CaptureNode.h>
#include <mtkcam3/pipeline/hwnode/FDNode.h>
#include <mtkcam3/pipeline/hwnode/JpegNode.h>
#include <mtkcam3/pipeline/hwnode/IspTuningDataPackNode.h>

#include <mtkcam3/pipeline/pipeline/IPipelineContextBuilder.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::isppipeline::model;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::v3::pipeline::policy::featuresetting_isp;
using namespace NSCam::v3::pipeline::policy;

//using namespace NSCamHW;


/******************************************************************************
 *
 ******************************************************************************/

CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_PIPELINE_MODEL);
#define MY_ERROR(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%u:%s] " fmt, mModel, __FUNCTION__, ##arg); \
        mErrorPrinter->printFormatLine(#level" [%u:%s] " fmt, mModel, __FUNCTION__, ##arg); \
    } while(0)

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(...)                MY_ERROR(F, __VA_ARGS__)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_LOGD1(...)               MY_LOGD_IF(           (mLogLevel>=1), __VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF(CC_UNLIKELY(mLogLevel>=2), __VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF(CC_UNLIKELY(mLogLevel>=3), __VA_ARGS__)

#if 1
#define FUNC_START MY_LOGD("%s +", __FUNCTION__);
#define FUNC_END MY_LOGD("%s -", __FUNCTION__);
#else
#define FUNC_START
#define FUNC_END
#endif

#define RETURN_IF_ERROR(_err_, _ops_)                                \
    do {                                                  \
        int const err = (_err_);                          \
        if( CC_UNLIKELY(err != 0) ) {                     \
            MY_LOGE("err:%d(%s) ops:%s", err, ::strerror(-err), _ops_); \
            return err;                                   \
        }                                                 \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
#define add_stream_to_set(_set_, _IStreamInfo_)                                 \
    do {                                                                        \
        if( _IStreamInfo_.get() ) { _set_.add(_IStreamInfo_->getStreamId()); }  \
    } while(0)
//
#define setImageUsage( _IStreamInfo_, _usg_ )                                   \
    do {                                                                        \
        if( _IStreamInfo_.get() ) {                                             \
            builder.setImageStreamUsage( _IStreamInfo_->getStreamId(), _usg_ ); \
        }                                                                       \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
template <class DstStreamInfoT, class SrcStreamInfoT>
static void add_meta_stream(
    StreamSet& set,
    DstStreamInfoT& dst,
    SrcStreamInfoT const& src
)
{
    if ( src != nullptr ) {
        dst = src;
        set.add(src->getStreamId());
    }
}


template <class DstStreamInfoT, class SrcStreamInfoT>
static void add_image_stream(
    StreamSet& set,
    DstStreamInfoT& dst,
    SrcStreamInfoT const& src
)
{
    if ( src != nullptr ) {
        dst = src;
        set.add(src->getStreamId());
    }
}


template <class DstStreamInfoT, class SrcStreamInfoT>
static void add_image_stream(
    StreamSet& set,
    android::Vector<DstStreamInfoT>& dst,
    SrcStreamInfoT const& src
)
{
    if ( src != nullptr ) {
        dst.push_back(src);
        set.add(src->getStreamId());
    }
}

static
auto
parseOutputStreamInfo(std::shared_ptr<UserConfigurationParams> config __unused,
                                PipelineModelISP::ConfigureInfo& OutInfo) -> bool
{

    MSize maxStreamSize = MSize(0, 0);
    for (auto const& s : config->vImageStreams)
    {
        bool isOut = false;
        auto const& pStreamInfo = s.second;
        if  ( CC_LIKELY(pStreamInfo != nullptr) && pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_OUT)
        {
            isOut = true;
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_ISP_TUING:
                MY_LOGD("has output tuning isp stream");
                break;
            case eImgFmt_RAW16:
            case eImgFmt_BAYER8_UNPAK:
            case eImgFmt_BAYER10_UNPAK:
            case eImgFmt_BAYER12_UNPAK:
            case eImgFmt_BAYER14_UNPAK:
            case eImgFmt_BAYER15_UNPAK:
                OutInfo.pvISPImage_RawOut.emplace(s.first, pStreamInfo);
                break;
            case eImgFmt_BLOB://AS-IS: should be removed in the future
            case eImgFmt_JPEG://TO-BE: Jpeg Capture
                OutInfo.pISPImage_JpegOut = pStreamInfo;
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
                {
                    OutInfo.pvISPImage_YuvOut.emplace(s.first, pStreamInfo);
                }
                break;
                //
            default:
                CAM_LOGE("[%s] Unsupported format:0x%x", __FUNCTION__, pStreamInfo->getImgFormat());
                break;
            }
        }

        if (isOut)
        {
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size() ) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }

    }

    OutInfo.maxImageSize = maxStreamSize;

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
static
int
buildContextLocked_Streams(
    android::sp<IPipelineContext> pContext,
    PipelineModelISP::ConfigureInfo& OutInfo
)
{
    FUNC_START;
#define BuildStream(_type_, _IStreamInfo_)                                     \
    do {                                                                       \
        if( _IStreamInfo_.get() ) {                                            \
            int err = 0;                                                       \
            if ( 0 != (err = StreamBuilder(_type_, _IStreamInfo_)              \
                    .build(pContext)) )                                        \
            {                                                                  \
                MY_LOGE("StreamBuilder fail stream %#" PRIx64 " of type %d",   \
                    _IStreamInfo_->getStreamId(), _type_);                     \
                return err;                                                    \
            }                                                                  \
        }                                                                      \
    } while(0)

    // app meta stream
    BuildStream(eStreamType_META_APP, OutInfo.pAppMeta_Control);
    BuildStream(eStreamType_META_APP, OutInfo.pHalMeta_P2CIn);
    BuildStream(eStreamType_META_APP, OutInfo.pAppMeta_P1Result);
    BuildStream(eStreamType_META_APP, OutInfo.pAppMeta_P2COut);
    BuildStream(eStreamType_META_APP, OutInfo.pAppMeta_JpegOut);
    BuildStream(eStreamType_META_APP, OutInfo.pAppMeta_P2PackOut);

    // hal meta stream
    BuildStream(eStreamType_META_HAL, OutInfo.pHalMeta_P2COut);

    // hal image stream
    {
        BuildStream(eStreamType_IMG_HAL_RUNTIME, OutInfo.pHalImage_Jpeg_YUV);
        BuildStream(eStreamType_IMG_HAL_RUNTIME, OutInfo.pHalImage_Thumbnail_YUV);
    }

    // ISP input image stream
    BuildStream(eStreamType_IMG_APP, OutInfo.pISPImage_Imgo_Raw_In);
    BuildStream(eStreamType_IMG_APP, OutInfo.pISPImage_Resized_In);
    BuildStream(eStreamType_IMG_APP, OutInfo.pISPImage_Lcso_In);
    BuildStream(eStreamType_IMG_APP, OutInfo.pISPImage_Imgo_Yuv_In);


    //  ISP output image stream
    for( const auto& n : OutInfo.pvISPImage_YuvOut ) {
        BuildStream(eStreamType_IMG_APP, n.second);
    }
    BuildStream(eStreamType_IMG_APP, OutInfo.pISPImage_JpegOut);
    for( const auto& n : OutInfo.pvISPImage_RawOut ) {
        BuildStream(eStreamType_IMG_APP, n.second);
    }
    BuildStream(eStreamType_IMG_APP, OutInfo.pISPImage_TuningOut);
    //
#undef BuildStream
    FUNC_END;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
static
int
buildContextLocked_P2CNode(
    android::sp<IPipelineContext> pContext,
    PipelineModelISP::ConfigureInfo& OutInfo,
    int32_t camId,
    std::vector<int32_t>& sensorIds
)
{
    typedef P2CaptureNode           NodeT;

    NodeId_T const nodeId = eNODEID_P2CaptureNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = (MUINT32)sensorIds[0];
        initParam.nodeId = nodeId;
        for (size_t i = 1; i < sensorIds.size(); i++)
            initParam.subOpenIdList.push_back((MUINT32)sensorIds[i]);
        initParam.nodeName = "P2CaptureNode";
    }
    if (OutInfo.pHalImage_Jpeg_YUV == nullptr && OutInfo.pvISPImage_YuvOut.size() == 0 && OutInfo.pvISPImage_RawOut.size() == 0)
    {
        OutInfo.needP2C = false;
        MY_LOGI("No need P2C node");
        return OK;
    }
    if (OutInfo.OnlyUseJpegNode)
    {
        OutInfo.needP2C = false;
        MY_LOGI("Only use jpegnode, No need P2C node");
        return OK;
    }
    OutInfo.needP2C = true;
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    NodeT::ConfigParams cfgParam;
    {
        add_meta_stream (outStreamSet, cfgParam.pOutAppMeta,    OutInfo.pAppMeta_P2COut);
        add_meta_stream (outStreamSet, cfgParam.pOutHalMeta,    OutInfo.pHalMeta_P2COut);
        add_meta_stream ( inStreamSet, cfgParam.pInAppMeta,     OutInfo.pAppMeta_Control);

        add_meta_stream ( inStreamSet, cfgParam.pInAppRetMeta,  OutInfo.pAppMeta_P1Result);
        add_meta_stream ( inStreamSet, cfgParam.pInHalMeta,     OutInfo.pHalMeta_P2CIn);

        // input image
        if (auto pImgoStreamInfo = OutInfo.pISPImage_Imgo_Raw_In.get()) {
            add_image_stream( inStreamSet, cfgParam.pvInFullRaw, pImgoStreamInfo);
        }
        add_image_stream( inStreamSet, cfgParam.pInResizedRaw,     OutInfo.pISPImage_Resized_In);
        add_image_stream( inStreamSet, cfgParam.pInLcsoRaw,     OutInfo.pISPImage_Lcso_In);
        //
        add_image_stream( inStreamSet, cfgParam.pInFullYuv,     OutInfo.pISPImage_Imgo_Yuv_In);

        // output image
        cfgParam.pOutPostView = nullptr;
        for( const auto& n : OutInfo.pvISPImage_YuvOut )
        {
            {
                add_image_stream(outStreamSet, cfgParam.pvOutImage, n.second);
            }
        }
        for( const auto& n : OutInfo.pvISPImage_RawOut )
        {
            {
                add_image_stream(outStreamSet, cfgParam.pvOutRaw, n.second);
            }
        }
        //
        add_image_stream(outStreamSet, cfgParam.pOutJpegYuv,        OutInfo.pHalImage_Jpeg_YUV);
        add_image_stream(outStreamSet, cfgParam.pOutThumbnailYuv,   OutInfo.pHalImage_Thumbnail_YUV);
        // [Jpeg packed]
    }
    //
    P2Common::Capture::UsageHint p2Usage;
    {
        //p2Usage.mSupportedScenarioFeatures = pCaptureFeatureSetting->supportedScenarioFeatures;
        //p2Usage.mIsSupportedBGPreRelease = isSupportedBGPreRelease;
        p2Usage.mSupportedScenarioFeatures = OutInfo.featureInfo.ispCaptureParams.supportedScenarioFeatures;
        p2Usage.mPluginUniqueKey = OutInfo.featureInfo.ispCaptureParams.pluginUniqueKey;
        p2Usage.mIsHidlIsp = true;
    }
    cfgParam.mUsageHint = p2Usage;
    //
    auto pNode = makeINodeActor( NodeT::createInstance(P2CaptureNode::PASS2_TIMESHARING, p2Usage) );
    pNode->setInitOrder(-1); //P2CaptureNode (default order: P2StreamNode -> P2CaptureNode)
    pNode->setInitTimeout(30000000000);
    pNode->setInitParam(initParam);
    pNode->setConfigParam(&cfgParam);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(cfgParam.pInFullYuv  , eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    //
    for (size_t i = 0; i < cfgParam.pvInFullRaw.size(); i++) {
        setImageUsage(cfgParam.pvInFullRaw[i], eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    }
    setImageUsage(cfgParam.pInResizedRaw, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(cfgParam.pInLcsoRaw,    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    for (size_t i = 0; i < cfgParam.pvOutRaw.size(); i++) {
        setImageUsage(cfgParam.pvOutRaw[i], eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    }
/*
    for (size_t i = 0; i < cfgParam.vP1SubStreamsInfo.size(); i++) {
        setImageUsage(cfgParam.vP1SubStreamsInfo[i].pInFullRawSub,   eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(cfgParam.vP1SubStreamsInfo[i].pInResizedRawSub,eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        setImageUsage(cfgParam.vP1SubStreamsInfo[i].pInLcsoRawSub,   eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    }
*/
    for (size_t i = 0; i < cfgParam.pvOutImage.size(); i++) {
        setImageUsage(cfgParam.pvOutImage[i], eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    }
    //
    setImageUsage(cfgParam.pOutJpegYuv,      eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    setImageUsage(cfgParam.pOutThumbnailYuv, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);

    //
    int err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
static
int
buildContextLocked_JpegNode(
    android::sp<IPipelineContext> pContext,
    PipelineModelISP::ConfigureInfo& OutInfo,
    int32_t camId,
    std::vector<int32_t>& sensorIds
)
{
    typedef JpegNode                NodeT;
    //
    NodeId_T const nodeId = eNODEID_JpegNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = (MUINT32)sensorIds[0];
        initParam.nodeId = nodeId;
        for (size_t i = 1; i < sensorIds.size(); i++)
            initParam.subOpenIdList.push_back((MUINT32)sensorIds[i]);
        initParam.nodeName = "JpegNode";
    }
    if (OutInfo.pISPImage_JpegOut == nullptr)
    {
        OutInfo.needJpeg = false;
        MY_LOGI("No need Jpeg node");
        return OK;
    }
    OutInfo.needJpeg = true;
    NodeT::ConfigParams cfgParam;
    {
        cfgParam.pInAppMeta                 = OutInfo.pAppMeta_Control;
        if (OutInfo.OnlyUseJpegNode)
        {
            cfgParam.pInHalMeta_capture         = OutInfo.pHalMeta_P2CIn;
        }
        else
        {
            cfgParam.pInHalMeta_capture         = OutInfo.pHalMeta_P2COut;
        }
        cfgParam.pInHalMeta_streaming       = nullptr;
        cfgParam.pOutAppMeta                = OutInfo.pAppMeta_JpegOut;
        if (OutInfo.AppYuvDirectJpeg)
        {
            MY_LOGD("direct user yuv to jpeg");
            cfgParam.pInYuv_Main                = OutInfo.pISPImage_Imgo_Yuv_In;
        }
        else
        {
            cfgParam.pInYuv_Main                = OutInfo.pHalImage_Jpeg_YUV;
        }
        cfgParam.pInYuv_Thumbnail           = OutInfo.pHalImage_Thumbnail_YUV;
        cfgParam.pOutJpeg                   = OutInfo.pISPImage_JpegOut;
        cfgParam.bIsPreReleaseEnable = false;
    }
    //
    auto pNode = makeINodeActor( NodeT::createInstance() );
    pNode->setInitOrder(-1); //JpegNode
    pNode->setInitTimeout(30000000000);
    pNode->setInitParam(initParam);
    pNode->setConfigParam(&cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, cfgParam.pInAppMeta);
    add_stream_to_set(inStreamSet, cfgParam.pInHalMeta_capture);
    add_stream_to_set(inStreamSet, cfgParam.pInYuv_Main);
    add_stream_to_set(inStreamSet, cfgParam.pInYuv_Thumbnail);
    //
    add_stream_to_set(outStreamSet, cfgParam.pOutAppMeta);
    add_stream_to_set(outStreamSet, cfgParam.pOutJpeg);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(cfgParam.pInYuv_Main, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(cfgParam.pInYuv_Thumbnail, eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);

    setImageUsage(cfgParam.pOutJpeg, eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    int err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
static
int
buildContextLocked_P2IspTuningDataPackNode(
    android::sp<IPipelineContext> pContext,
    PipelineModelISP::ConfigureInfo& OutInfo,
    int32_t camId,
    std::vector<int32_t>& sensorIds
)
{
    typedef IspTuningDataPackNode                NodeT;
    //
    NodeId_T const nodeId = eNODEID_P2YuvIspTuningDataPackNode;
    //
    NodeT::InitParams initParam;
    {
        initParam.openId = sensorIds[0];
        initParam.nodeId = nodeId;
        initParam.nodeName = "P2YuvIspTuningDataPackNode";
    }
    if (OutInfo.pISPImage_TuningOut == nullptr || OutInfo.needP2C == false)
    {
        OutInfo.needISPPack = false;
        MY_LOGI("No need P2YuvIspTuningDataPackNode node");
        return OK;
    }
    NodeT::ConfigParams cfgParam;
    NodeT::ConfigParams::PackStreamInfo streamInfo;
    {
        cfgParam.sourceType               = IspTuningDataPackNode::YuvFromP2;
        streamInfo.pvInAppCtlMetaInfo.push_back(OutInfo.pAppMeta_Control);
        streamInfo.pvInP1AppMetaInfo.push_back(OutInfo.pAppMeta_P1Result);
        streamInfo.pvInP2AppMetaInfo.push_back(OutInfo.pAppMeta_P2COut);
        streamInfo.pvInHalMetaInfo.push_back(OutInfo.pHalMeta_P2COut);
        streamInfo.pvInImgInfo_statistics.push_back(OutInfo.pISPImage_Lcso_In);
        streamInfo.pOutImgInfo            = OutInfo.pISPImage_TuningOut;
        streamInfo.pOutAppMetaInfo        = OutInfo.pAppMeta_P2PackOut;

        cfgParam.pvStreamInfoMap.emplace(0, streamInfo);
    }
    //
    auto pNode = makeINodeActor( NodeT::createInstance() );
    pNode->setInitOrder(-1);
    pNode->setInitTimeout(30000000000);
    pNode->setInitParam(initParam);
    pNode->setConfigParam(&cfgParam);
    //
    StreamSet inStreamSet;
    StreamSet outStreamSet;
    //
    add_stream_to_set(inStreamSet, OutInfo.pAppMeta_Control);
    add_stream_to_set(inStreamSet, OutInfo.pAppMeta_P1Result);
    add_stream_to_set(inStreamSet, OutInfo.pAppMeta_P2COut);
    add_stream_to_set(inStreamSet, OutInfo.pHalMeta_P2COut);
    add_stream_to_set(inStreamSet, OutInfo.pISPImage_Lcso_In);
    //
    add_stream_to_set(outStreamSet, OutInfo.pISPImage_TuningOut);
    add_stream_to_set(outStreamSet, OutInfo.pAppMeta_P2PackOut);
    //
    NodeBuilder builder(nodeId, pNode);
    builder
        .addStream(NodeBuilder::eDirection_IN, inStreamSet)
        .addStream(NodeBuilder::eDirection_OUT, outStreamSet);
    //
    setImageUsage(OutInfo.pISPImage_Lcso_In,   eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
    setImageUsage(OutInfo.pISPImage_TuningOut, eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
    //
    int err = builder.build(pContext);
    if( err != OK )
        MY_LOGE("build node %#" PRIxPTR " failed", nodeId);
    return err;
}

static auto calcThumbnailYuvSize(
    MSize const rPicSize,
    MSize const rThumbnailsize
) -> MSize
{
#define align16(x) (((x) + 15) & (~0xF))
    MSize size;
    MUINT32 const val0 = rPicSize.w * rThumbnailsize.h;
    MUINT32 const val1 = rPicSize.h * rThumbnailsize.w;
    if( val0 > val1 ) {
        size.w = align16(val0/rPicSize.h);
        size.h = align16(rThumbnailsize.h);
    }
    else if( val0 < val1 ) {
        size.w = align16(rThumbnailsize.w);
        size.h = align16(val1/rPicSize.w);
    }
    else {
        size = rThumbnailsize;
    }
#undef align16
    MY_LOGD_IF(1, "thumb %dx%d, pic %dx%d -> yuv for thumb %dx%d",
            rThumbnailsize.w, rThumbnailsize.h,
            rPicSize.w, rPicSize.h,
            size.w, size.h
            );
    return size;
}

static auto
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform
) -> sp<IImageStreamInfo>
{
    IImageStreamInfo::BufPlanes_t bufPlanes;

#define ALIGN(_value_, _align_) (((_value_) + (_align_-1)) & ~(_align_-1))
    auto w = (size_t)ALIGN(imgSize.w, 16);
    auto h = (size_t)ALIGN(imgSize.h, 16);
    bufPlanes.count = 2;
    bufPlanes.planes[0].sizeInBytes = (h * w);
    bufPlanes.planes[0].rowStrideInBytes = w;
    bufPlanes.planes[1].sizeInBytes = ((h >> 1) * w);
    bufPlanes.planes[1].rowStrideInBytes = w;
#undef ALIGN
    sp<IImageStreamInfo>
        pStreamInfo = ImageStreamInfoBuilder()
                        .setStreamName(streamName)
                        .setStreamId(streamId)
                        .setStreamType(streamType)
                        .setMaxBufNum(maxBufNum)
                        .setMinInitBufNum(minInitBufNum)
                        .setUsageForAllocator(usageForAllocator)
                        .setAllocImgFormat(imgFormat)
                        .setAllocBufPlanes(bufPlanes)
                        .setImgFormat(imgFormat)
                        .setBufPlanes(bufPlanes)
                        .setImgSize(imgSize)
                        .setBufCount(1)
                        .setBufStep(0)
                        .setStartOffset(0)
                        .setTransform(transform)
                        .build();

    MY_LOGE_IF(pStreamInfo==nullptr, "new ImageStreamInfo - %s %#" PRIx64, streamName, streamId);
    return pStreamInfo;
}

static auto createStreamInfo_Thumbnail_YUV(
    IMetadata *pAppControl,
    sp<IImageStreamInfo> pConfigMainInfo,
    sp<IImageStreamInfo> pConfigThumbInfo,
    sp<IImageStreamInfo>& pOutThumbInfo
) -> int
{
    auto const pMetadata    = pAppControl;
    auto const pCfgThumbYUV  = pConfigThumbInfo;
    auto const pCfgJpegYUV  = pConfigMainInfo;
    if ( CC_UNLIKELY( ! pMetadata || ! pConfigMainInfo.get() ) ) {
        MY_LOGE("invlaid input params");
        return NO_INIT;
    }

    IMetadata::IEntry const& entryThumbnailSize = pMetadata->entryFor(MTK_JPEG_THUMBNAIL_SIZE);
    MSize thumbnailSize = MSize(176, 128);
    bool useDefault = false;
    if  ( entryThumbnailSize.isEmpty() )
    {
        MY_LOGW("Bad thumbnail size, cannot get thumbnail size, use default");
        useDefault = true;
    } else {
        thumbnailSize = entryThumbnailSize.itemAt(0, Type2Type<MSize>());
        if ( !thumbnailSize )
        {
            MY_LOGW("Bad thumbnail size: %dx%d, use default",  thumbnailSize.w, thumbnailSize.h);
            useDefault = true;
        }
    }
    //
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
    }

    if (useDefault)
    {
        if ((pCfgJpegYUV->getImgSize().w >= pCfgJpegYUV->getImgSize().h))
        {
            thumbnailSize.w = 176;
            thumbnailSize.h = 128;
        }
        else
        {
            thumbnailSize.w = 128;
            thumbnailSize.h = 176;
        }
    }
    MY_LOGD_IF( 1, "thumbnail size from metadata: %dx%d", thumbnailSize.w, thumbnailSize.h);

    //
    MSize const yuvthumbnailsize = calcThumbnailYuvSize(
                                        pCfgJpegYUV->getImgSize(),
                                        thumbnailSize
                                        );

    if (useDefault)
    {
        IMetadata::IEntry Entry1(MTK_JPEG_THUMBNAIL_SIZE);
        Entry1.push_back(yuvthumbnailsize, Type2Type<MSize>());
        pMetadata->update(MTK_JPEG_THUMBNAIL_SIZE, Entry1);
    }
    //
    MINT32  jpegOrientation = 0;
    MUINT32 jpegTransform   = 0;
    MSize   thunmbSize      = yuvthumbnailsize; // default thumbnail size
    //
    MINT const format = pCfgThumbYUV->getImgFormat();
    IImageStreamInfo::BufPlanes_t bufPlanes;
    switch (format)
    {
    case eImgFmt_YUY2:{
        bufPlanes.count = 1;
        bufPlanes.planes[0].rowStrideInBytes = (yuvthumbnailsize.w << 1);
        bufPlanes.planes[0].sizeInBytes = bufPlanes.planes[0].rowStrideInBytes * yuvthumbnailsize.h;
        }break;
    default:
        MY_LOGE("not supported format: %#x", format);
        break;
    }
    //
    ImageBufferInfo imageBufferInfo;
    imageBufferInfo.bufStep = 0;
    imageBufferInfo.bufPlanes = bufPlanes;
    imageBufferInfo.imgFormat = format;
    imageBufferInfo.imgWidth   = thunmbSize.w;
    imageBufferInfo.imgHeight   = thunmbSize.h;
    sp<IImageStreamInfo> pStreamInfo = ImageStreamInfoBuilder(pCfgThumbYUV.get())
                                            .setAllocImgFormat(format)
                                            .setAllocBufPlanes(bufPlanes)
                                            .setImageBufferInfo(imageBufferInfo)
                                            .setTransform(jpegTransform)
                                            .build();

    if  ( CC_UNLIKELY( ! pStreamInfo.get() ) ) {
        MY_LOGE( "fail to new thumbnail ImageStreamInfo: %s %#" PRIx64,
                 pCfgThumbYUV->getStreamName(), pCfgThumbYUV->getStreamId() );
        return NO_MEMORY;
    }

    pOutThumbInfo = pStreamInfo;

    MY_LOGD_IF( 1, "streamId:%#" PRIx64 " name(%s) req(%p) cfg(%p) mainjpegsize:%dx%d yuvthumbnailsize:%dx%d jpegOrientation:%d",
        pStreamInfo->getStreamId(),
        pStreamInfo->getStreamName(),
        pStreamInfo.get(), pCfgThumbYUV.get(),
        pCfgJpegYUV->getImgSize().w, pCfgJpegYUV->getImgSize().h,
        thunmbSize.w, thunmbSize.h, jpegOrientation
    );

    return OK;
}


static auto createRotationStreamInfo_Main_YUV(
    IMetadata *pAppControl,
    sp<IImageStreamInfo> pConfigMainInfo,
    sp<IImageStreamInfo>& pOutMainInfo
) -> int
{
    auto const pMetadata    = pAppControl;
    auto const pCfgMainYUV  = pConfigMainInfo;
    if ( CC_UNLIKELY( ! pMetadata || ! pCfgMainYUV.get() ) ) {
        MY_LOGE("invlaid input params");
        return NO_INIT;
    }

    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return -EINVAL;
    }

    int32_t jpegFlip = 0;
    IMetadata::IEntry const& entryJpegFlip = pMetadata->entryFor(MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE);
    if (entryJpegFlip.isEmpty()) {
        MY_LOGD("No tag: MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE");
    } else {
        jpegFlip = entryJpegFlip.itemAt(0, Type2Type<MINT32>());
    }

    int32_t jpegFlipProp = ::property_get_int32("vendor.debug.isphidl.Jpeg.flip", 0);

    int32_t const jpegOrientation = entryJpegOrientation.itemAt(0, Type2Type<MINT32>());
    uint32_t      reqTransform   = 0;

    if (jpegFlip || jpegFlipProp) {
        if ( 0==jpegOrientation )
            reqTransform = eTransform_FLIP_H;
        else if ( 90==jpegOrientation )
            reqTransform = eTransform_ROT_90 | eTransform_FLIP_V;
        else if ( 180==jpegOrientation )
            reqTransform = eTransform_FLIP_V;
        else if ( 270==jpegOrientation )
            reqTransform = eTransform_ROT_90 | eTransform_FLIP_H;
        else
            MY_LOGW("Invalid Jpeg Orientation value: %d", jpegOrientation);
    } else {
        if ( 0==jpegOrientation )
            reqTransform = 0;
        else if ( 90==jpegOrientation )
            reqTransform = eTransform_ROT_90;
        else if ( 180==jpegOrientation )
            reqTransform = eTransform_ROT_180;
        else if ( 270==jpegOrientation )
            reqTransform = eTransform_ROT_270;
        else
            MY_LOGW("Invalid Jpeg Orientation value: %d", jpegOrientation);
    }

    uint32_t const cfgTransform   = pCfgMainYUV->getTransform();
    MY_LOGD_IF( 1, "Jpeg orientation metadata: %d degrees; transform request(%d) & config(%d) & flip(%d)",
                jpegOrientation, reqTransform, cfgTransform, jpegFlip);
    if ( reqTransform == cfgTransform ) {
        pOutMainInfo      = nullptr;
        return OK;
    }

    MSize size, cfgSize(pCfgMainYUV->getImgSize().w, pCfgMainYUV->getImgSize().h);
    if ( reqTransform&eTransform_ROT_90 )
    {
        size.w = std::min(cfgSize.w, cfgSize.h);
        size.h = std::max(cfgSize.w, cfgSize.h);
    }
    else
    {
        size = cfgSize;
    }

    sp<IImageStreamInfo>
        pStreamInfo = createImageStreamInfo(
                            pCfgMainYUV->getStreamName(),
                            pCfgMainYUV->getStreamId(),
                            pCfgMainYUV->getStreamType(),
                            pCfgMainYUV->getMaxBufNum(),
                            pCfgMainYUV->getMinInitBufNum(),
                            pCfgMainYUV->getUsageForAllocator(),
                            pCfgMainYUV->getImgFormat(),
                            size, reqTransform
                        );
    if ( CC_UNLIKELY( ! pStreamInfo.get() ) ) {
        MY_LOGE( "fail to new ImageStreamInfo: %s %#" PRIx64,
                 pCfgMainYUV->getStreamName(), pCfgMainYUV->getStreamId() );
        return NO_MEMORY;
    }

    pOutMainInfo = pStreamInfo;

    MY_LOGD_IF( 1, "streamId:%#" PRIx64 " name(%s) req(%p) cfg(%p) yuvsize(%dx%d) jpegOrientation(%d)",
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName(),
                pStreamInfo.get(), pCfgMainYUV.get(),
                pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h, jpegOrientation );

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
IISPPipelineModel::
createInstance(int32_t id, int32_t camId, std::vector<int32_t> &sensorIds) -> android::sp<IISPPipelineModel>
{
    PipelineModelISP::ModelCreationParams Params;
    Params.id = id;
    Params.camId = camId;
    Params.sensorIds = sensorIds;
    Params.errorPrinter = std::make_shared<EventLogPrinter>(15, 25);
    sp<IISPPipelineModel> pPipeline = new PipelineModelISP(Params);
    if ( CC_UNLIKELY( ! pPipeline.get() ) ) {
        CAM_LOGE("create pipelinemodel instance fail");
        return nullptr;
    }

    return pPipeline;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineModelISP::
PipelineModelISP(ModelCreationParams const& creationParams __unused)
    : mErrorPrinter(creationParams.errorPrinter)
    , mLogLevel(0)
    , mModelName()
    , mModel(creationParams.id)
    , mCamId(creationParams.camId)
{
    mSensorIds = creationParams.sensorIds;
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.pipelinemodel", 0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
dumpState(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    //  Instantiation data

    auto timeout = std::chrono::milliseconds(100);

    //  Open data & Configuration data
    if  ( mLock.try_lock_for(timeout) ) {
        printer.printFormatLine(
            "{.mUserName=%s, .mCallback=%p}",
            mModelName.c_str(), mCallback.unsafe_get()
        );
        mLock.unlock();
    }
    else {
        printer.printLine("timeout: try_lock_for()");
    }

    //  PipelineContext
    if  ( mLock.try_lock_for(timeout) ) {
    }
    else {
        printer.printLine("timeout: try_lock_for()");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
open(
    std::string const& userName,
    android::wp<IISPPipelineModelCallback> const& callback
) -> int
{
    MY_LOGD("+");
    //
    {
        std::lock_guard<std::timed_mutex> _l(mLock);

        mModelName = userName;
        mCallback = callback;
    }
    //
    MY_LOGD("-");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
configure(
    std::shared_ptr<UserConfigurationParams>const& params
) -> int
{
    int err = OK;
    MY_LOGD("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);

        {
            featuresetting_isp::CreationParams featureParams;
            featureParams.sessionParams = &params->sessionParams;
            featureParams.openId = mCamId;
            featureParams.sensorId = mSensorIds;
            mpFeaturePolicy = createFeatureSettingPolicyInstance_ISP(featureParams);
            ConfigurationInputParams input;
            input.pipelineUniqueKey = (int32_t)((MINTPTR)this);
            mpFeaturePolicy->evaluateConfiguration(&mConfigureInfo.featureInfo, &input);
        }
        mCurrentPipelineContext = nullptr;
        auto pPipelineContext = IPipelineContextManager::get()->create(mModelName.c_str());
        pPipelineContext->beginConfigure();
        RETURN_IF_ERROR(parseConfigureParams(params), "parse configure params failed");
        RETURN_IF_ERROR(buildISPStreams(params, pPipelineContext), "build ISP streams failed");
        RETURN_IF_ERROR(buildISPNodes(pPipelineContext), "build ISP nodes failed");
        RETURN_IF_ERROR(buildISPPipeline(pPipelineContext), "build ISP pipeline failed");
        RETURN_IF_ERROR(pPipelineContext->setDataCallback(this), "setDataCallback failed");
        RETURN_IF_ERROR(pPipelineContext->endConfigure(1, 0), "endConfigure failed");
        mCurrentPipelineContext = pPipelineContext;
    }
    MY_LOGD("- err:%d", err);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
close() -> void
{
    MY_LOGD1("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);

        NSCam::Utils::CamProfile profile(__FUNCTION__, mModelName.c_str());;

        MY_LOGD1("destroying pipeline context");
        mCurrentPipelineContext = nullptr;
        mCallback = nullptr;

        mModelName.clear();
    }
    MY_LOGD1("-");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
submitRequest(
    std::vector<std::shared_ptr<UserRequestParams>>const& requests,
    uint32_t& numRequestProcessed __unused
) -> int
{
    int err = OK;
    android::sp<IPipelineContext> pContext;

    MY_LOGD2("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);
        pContext = mCurrentPipelineContext;
    }
    {
        if ( CC_UNLIKELY( pContext==nullptr ) ) {
            MY_LOGE("null Context");
            err = DEAD_OBJECT;
        }
        else {
            android::sp<IPipelineFrame> pPipelineFrame;
            int32_t count = requests.size();
            int32_t index = 0;
            for (auto const& req : requests)
            {
                RETURN_IF_ERROR(buildISPPipelineFrame(req, pContext, pPipelineFrame, count, index), "handle request failed");
                RETURN_IF_ERROR(pContext->queue(pPipelineFrame), "queue request failed");
                index++;
            }
        }
        pContext = nullptr;
    }
    MY_LOGD2("- err:%d", err);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
beginFlush() -> int
{
    int err = OK;
    android::sp<IPipelineContext> pContext;
    MY_LOGD1("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);
        pContext = mCurrentPipelineContext;
    }
    {
        if ( CC_UNLIKELY( pContext==nullptr ) ) {
            MY_LOGE("null Context");
            err = DEAD_OBJECT;
        }
        else {
            RETURN_IF_ERROR( pContext->beginFlush(), "PipelineContext::beginFlush()" );
        }
        pContext = nullptr;
    }

    MY_LOGD1("-");
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
endFlush() -> void
{
    int err = OK;
    android::sp<IPipelineContext> pContext;
    MY_LOGD1("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);
        pContext = mCurrentPipelineContext;
    }
    {
        if ( CC_UNLIKELY( pContext==nullptr ) ) {
            MY_LOGE("null Context");
            err = DEAD_OBJECT;
        }
        else {
            pContext->endFlush();
        }
        pContext = nullptr;
    }

    MY_LOGD1("-");
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
queryFeature(IMetadata const* pInMeta, std::vector<std::shared_ptr<IMetadata>> &outSetting) -> void
{
    int err = OK;
    MY_LOGD1("+");
    if (mpFeaturePolicy)
    {
        featuresetting_isp::RecommendedParams params;
        params.pRequest_AppMetadata = pInMeta;
        mpFeaturePolicy->queryRecommendedSetting(&params);
        outSetting = params.framesSetting;
    }

    MY_LOGD1("-");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineModelISP::
updateFrame(
    MUINT32 const requestNo,
    MINTPTR const userId,
    Result const& result
)
{
    sp<IISPPipelineModelCallback> pCallback;
    pCallback = mCallback.promote();
    if ( CC_UNLIKELY(! pCallback.get()) ) {
        MY_LOGE("Have not set callback to ISP device");
        return;
    }

    {
        UserOnFrameUpdated params;
        params.requestNo = requestNo;
        params.userId = userId;
        params.nOutMetaLeft = result.nAppOutMetaLeft;
        params.timestampStartOfFrame = 0;
        for ( size_t i=0; i < result.vAppOutMeta.size(); ++i ) {
            params.vOutMeta.push_back(result.vAppOutMeta[i]);
        }

        for ( size_t i=0; i < result.vPhysicalOutMeta.size(); ++i) {
            params.vPhysicalOutMeta.add(
                        result.vPhysicalOutMeta.keyAt(i),
                        result.vPhysicalOutMeta.valueAt(i));
        }
        params.bFrameEnd = result.bFrameEnd;
        pCallback->onFrameUpdated(params);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineModelISP::
onMetaCallback(
    MUINT32              requestNo,
    Pipeline_NodeId_T    nodeId,
    StreamId_T           streamId __unused,
    IMetadata const&     rMetaData,
    MBOOL                errorResult __unused
)
{
    sp<IISPPipelineModelCallback> pCallback;
    pCallback = mCallback.promote();
    if ( CC_UNLIKELY(! pCallback.get()) ) {
        MY_LOGE("Have not set callback to ISP device");
        return;
    }

    {
        UserOnFrameUpdated params;
        params.requestNo = requestNo;
        params.userId = nodeId;
        params.bFrameEnd = false;
        params.bisEarlyCB = true;
        params.pMetadata = &rMetaData;
        pCallback->onFrameUpdated(params);
    }
}



// internal APIs
/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
parseConfigureParams(
    std::shared_ptr<UserConfigurationParams>const& params
) -> int
{
    IMetadata::IEntry const& entryDirectJpeg = params->sessionParams.entryFor(MTK_CONFIGURE_ISP_YUV_DIRECT_JPEG);
    if  ( entryDirectJpeg.isEmpty() )
    {
        MY_LOGD("Didn't set yuv direct jpeg flag");
        return OK;
    }
    else
    {
        mConfigureInfo.AppYuvDirectJpeg = ((entryDirectJpeg.itemAt(0, Type2Type<MINT32>())) & 0x01) != 0;
        MY_LOGD("App set yuv direct jpeg flag(%d)", mConfigureInfo.AppYuvDirectJpeg);
        mConfigureInfo.WithoutThumbnail = ((entryDirectJpeg.itemAt(0, Type2Type<MINT32>())) & 0x02) != 0;
        MY_LOGD("App set yuv to jpeg without thumbnail flag(%d)", mConfigureInfo.WithoutThumbnail);
        mConfigureInfo.OnlyUseJpegNode = mConfigureInfo.AppYuvDirectJpeg && mConfigureInfo.WithoutThumbnail;
        MY_LOGD("App set yuv direct jpeg and only use jpeg node(%d)", mConfigureInfo.OnlyUseJpegNode);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
buildISPStreams(
    std::shared_ptr<UserConfigurationParams>const& params,
    android::sp<IPipelineContext> pContext
) -> int
{
    int err = OK;
    MY_LOGD("+");
    // build meta
    #define SET_METASTREAM(_id_, _s_)                            \
        do {                                                     \
            auto stream = params->vMetaStreams.find(_id_);       \
            if (stream != params->vMetaStreams.end())            \
            {                                                    \
                _s_ = stream->second;                            \
            }                                                    \
        } while(0)
    #define SET_IMAGESTREAM(_id_, _s_)                           \
        do {                                                     \
            auto stream = params->vImageStreams.find(_id_);      \
            if (stream != params->vImageStreams.end())           \
            {                                                    \
                _s_ = stream->second;                            \
            }                                                    \
        } while(0)
    SET_METASTREAM(eSTREAMID_META_APP_CONTROL, mConfigureInfo.pAppMeta_Control);
    SET_METASTREAM(eSTREAMID_META_APP_DYNAMIC_01, mConfigureInfo.pAppMeta_P1Result);
    SET_METASTREAM(eSTREAMID_META_PIPE_DYNAMIC_01, mConfigureInfo.pHalMeta_P2CIn);

    mConfigureInfo.pAppMeta_P2COut =
            new MetaStreamInfo(
                "ISP:App:Meta:DynamicP2Cap",
                eSTREAMID_META_APP_DYNAMIC_02_CAP,
                eSTREAMTYPE_META_OUT,
                10, 1
                );
    mConfigureInfo.pHalMeta_P2COut =
            new MetaStreamInfo(
                "ISP:Hal:Meta:P2C:Dynamic",
                eSTREAMID_META_PIPE_DYNAMIC_02_CAP,
                eSTREAMTYPE_META_INOUT,
                10, 1
                );
    mConfigureInfo.pAppMeta_JpegOut =
            new MetaStreamInfo(
                "ISP:App:Meta:Jpeg",
                eSTREAMID_META_APP_DYNAMIC_JPEG,
                eSTREAMTYPE_META_OUT,
                10, 1
                );
    mConfigureInfo.pAppMeta_P2PackOut =
            new MetaStreamInfo(
                "ISP:App:Meta:P2Pack",
                eSTREAMID_META_APP_DYNAMIC_P2ISPPack,
                eSTREAMTYPE_META_OUT,
                10, 1
                );

    SET_IMAGESTREAM(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RAW, mConfigureInfo.pISPImage_Imgo_Raw_In);
    SET_IMAGESTREAM(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED, mConfigureInfo.pISPImage_Resized_In);
    SET_IMAGESTREAM(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO, mConfigureInfo.pISPImage_Lcso_In);
    SET_IMAGESTREAM(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_YUV, mConfigureInfo.pISPImage_Imgo_Yuv_In);
    SET_IMAGESTREAM(eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING, mConfigureInfo.pISPImage_TuningOut);

    parseOutputStreamInfo(params, mConfigureInfo);

    if (mConfigureInfo.pISPImage_JpegOut != nullptr)
    {
        #define ALIGN(_value_, _align_) (((_value_) + (_align_-1)) & ~(_align_-1))
        // create jpeg/thumbnail hal stream info
        // Jpeg YUV
        {
            MSize const& size = mConfigureInfo.pISPImage_JpegOut->getImgSize();
            MINT const format = eImgFmt_NV21;
            MUINT const usage = 0;
            IImageStreamInfo::BufPlanes_t bufPlanes;
            auto w = (size_t)ALIGN(size.w, 16);
            auto h = (size_t)ALIGN(size.h, 16);
            bufPlanes.count = 2;
            bufPlanes.planes[0].sizeInBytes = (h * w);
            bufPlanes.planes[0].rowStrideInBytes = w;
            bufPlanes.planes[1].sizeInBytes = ((h >> 1) * w);
            bufPlanes.planes[1].rowStrideInBytes = w;

            mConfigureInfo.pHalImage_Jpeg_YUV =
                ImageStreamInfoBuilder()
                    .setStreamName("ISPHIDL:Image:YuvJpeg")
                    .setStreamId(eSTREAMID_IMAGE_PIPE_YUV_JPEG_00)
                    .setStreamType(eSTREAMTYPE_IMAGE_INOUT)
                    .setMaxBufNum(1)
                    .setMinInitBufNum(0)
                    .setUsageForAllocator(usage)
                    .setAllocImgFormat(format)
                    .setAllocBufPlanes(bufPlanes)
                    .setImgFormat(format)
                    .setBufPlanes(bufPlanes)
                    .setImgSize(size)
                    .setBufCount(1)
                    .setBufStep(0)
                    .setStartOffset(0)
                    .build();
        }
        // Thumbnail YUV
        {
            MSize size(-1L, -1L); //unknown now
            MINT const format = eImgFmt_YUY2;
            MUINT const usage = 0;
            IImageStreamInfo::BufPlanes_t bufPlanes;
            /*if (mConfigureInfo.AppYuvDirectJpeg)
            {
                size = calcThumbnailYuvSize(mConfigureInfo.pISPImage_JpegOut->getImgSize(), MSize(320, 240));
            }*/
            auto w = (size_t)(size.w << 1);
            auto h = (size_t)size.h;
            bufPlanes.count = 1;
            bufPlanes.planes[0].sizeInBytes = (h * w);
            bufPlanes.planes[0].rowStrideInBytes = w;

            mConfigureInfo.pHalImage_Thumbnail_YUV =
                ImageStreamInfoBuilder()
                    .setStreamName("ISPHIDL:Image:YuvThumbnail")
                    .setStreamId(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00)
                    .setStreamType(eSTREAMTYPE_IMAGE_INOUT)
                    .setMaxBufNum(1)
                    .setMinInitBufNum(0)
                    .setUsageForAllocator(usage)
                    .setAllocImgFormat(format)
                    .setAllocBufPlanes(bufPlanes)
                    .setImgFormat(format)
                    .setBufPlanes(bufPlanes)
                    .setImgSize(size)
                    .setBufCount(1)
                    .setBufStep(0)
                    .setStartOffset(0)
                    .build();
        }
    }

    err = buildContextLocked_Streams(pContext, mConfigureInfo);

    MY_LOGD("- err:%d", err);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
buildISPNodes(
    android::sp<IPipelineContext> pContext
) -> int
{
    int err = OK;
    MY_LOGD("+");
    RETURN_IF_ERROR(buildContextLocked_P2CNode(pContext, mConfigureInfo, mCamId, mSensorIds), "build p2c failed");
    RETURN_IF_ERROR(buildContextLocked_JpegNode(pContext, mConfigureInfo, mCamId, mSensorIds), "build jpeg failed");
    RETURN_IF_ERROR(buildContextLocked_P2IspTuningDataPackNode(pContext, mConfigureInfo, mCamId, mSensorIds), "build ISPPack failed");

    MY_LOGD("- err:%d", err);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
buildISPPipeline(
    android::sp<IPipelineContext> pContext
) -> int
{
    int err = OK;
    MY_LOGD("+");

    /**
     * The root nodes of a pipeline.
     */
    NodeSet                                     roots;
    /**
     * The edges to connect pipeline nodes.
     */
    NodeEdgeSet                                 edges;

    MY_LOGD("needP2C[%d], needJpeg[%d]", mConfigureInfo.needP2C, mConfigureInfo.needJpeg);

    if (mConfigureInfo.needP2C)
    {
        roots.add(eNODEID_P2CaptureNode);
        mConfigureInfo.rootNode = eNODEID_P2CaptureNode;
    }
    else if (mConfigureInfo.needJpeg)
    {
        roots.add(eNODEID_JpegNode);
        mConfigureInfo.rootNode = eNODEID_JpegNode;
    }

    if (mConfigureInfo.needP2C && mConfigureInfo.needJpeg)
    {
        edges.addEdge(eNODEID_P2CaptureNode, eNODEID_JpegNode);
    }
    if (mConfigureInfo.needP2C && mConfigureInfo.needISPPack)
    {
        MY_LOGD("set edge : P2C -> ISPTuningDataPack");
        edges.addEdge(eNODEID_P2CaptureNode, eNODEID_P2YuvIspTuningDataPackNode);
    }
    err = PipelineBuilder()
            .setRootNode(roots)
            .setNodeEdges(edges)
            .build(pContext);

    MY_LOGD("- err:%d", err);
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelISP::
buildISPPipelineFrame(
    std::shared_ptr<UserRequestParams>const& request,
    android::sp<IPipelineContext> pContext,
    android::sp<IPipelineFrame>& pFrame,
    int32_t frameCount,
    int32_t frameIndex
) -> int
{
    int err = OK;
    MY_LOGD("+");
    auto FrameBuilder = IFrameBuilder::make();
    IOMap P2CImageMap;
    IOMap P2CMetaMap;
    IOMap JpegImageMap;
    IOMap JpegMetaMap;
    IOMap ISPPackImageMap;
    IOMap ISPPackMetaMap;
    bool needP2C = true;
    bool needJpeg = false;
    bool needISPPack = false;
    NodeId_T rootNode = eNODEID_P2CaptureNode;
    IMetadata *pAppMetaControl = nullptr;
    android::sp<IMetaStreamBuffer> pAppMetaBuffer = nullptr;
    IMetadata *pAppMetaResult = nullptr;
    android::sp<IMetaStreamBuffer> pAppResultBuffer = nullptr;
    IMetadata *pHalMetaControl = nullptr;
    android::sp<IMetaStreamBuffer> pHalMetaBuffer = nullptr;
    sp<IImageStreamInfo> pMainJpegYuv = nullptr;
    sp<IImageStreamInfo> pThumbnailYuv = nullptr;
    RequestParams featureParams;
    android::sp<IImageStreamBuffer> pRawInputBuffer = nullptr;

    featureParams.requestNo = request->requestNo;
    featureParams.frameCount = frameCount;
    featureParams.frameIndex = frameIndex;

    // store FD information data to FDContainer(ISP)
    if (request->pFDData != nullptr && request->pFDData->size() != 0)
    {
        auto fdWriter = IFDContainer::createInstance(LOG_TAG, IFDContainer::eFDContainer_ISP_Opt_Write);
        auto fddata = fdWriter->editLock(request->FDDataTimestamp);
        MY_LOGD("store to fd isp container, key(%lld), size = %d", request->FDDataTimestamp, request->pFDData->size());
        if (fddata != NULL)
        {
            MY_LOGD("size of MtkCameraFaceMetadata = %d", sizeof(MtkCameraFaceMetadata));
            memcpy(&(fddata->facedata), request->pFDData->data(), sizeof(MtkCameraFaceMetadata));
            fddata->sensorId = mSensorIds[0];
            fdWriter->editUnlock(fddata);
        }
        else
        {
            MY_LOGW("get container FD buffer null");
        }
    }
    if (mConfigureInfo.OnlyUseJpegNode == true)
    {
        needP2C = false;
        rootNode = eNODEID_JpegNode;
    }

    for( const auto& buffer : request->vIImageBuffers )
    {
        switch (buffer.first)
        {
            case eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RAW:
                P2CImageMap.addIn(buffer.first);
                FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                if (eImgFmt_CAMERA_OPAQUE == mConfigureInfo.pISPImage_Imgo_Raw_In->getImgFormat())
                {
                    featureParams.pAppImage_Input_Priv  = mConfigureInfo.pISPImage_Imgo_Raw_In;
                    pRawInputBuffer = buffer.second;
                }
                else
                {
                    featureParams.pAppImage_Input_RAW16 = mConfigureInfo.pISPImage_Imgo_Raw_In;
                    pRawInputBuffer = buffer.second;
                }
                break;
            case eSTREAMID_IMAGE_ISP_DEVICE_INPUT_YUV:
                P2CImageMap.addIn(buffer.first);
                FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                featureParams.pAppImage_Input_Yuv = mConfigureInfo.pISPImage_Imgo_Yuv_In;
                break;
            case eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO:
                if (rootNode == eNODEID_P2CaptureNode)
                {
                    P2CImageMap.addIn(buffer.first);
                    ISPPackImageMap.addIn(buffer.first);
                    FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                }
                break;
            case eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED:
                if (rootNode == eNODEID_P2CaptureNode)
                {
                    P2CImageMap.addIn(buffer.first);
                    FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                }
                break;
                //
            default:
                MY_LOGW("unsupport input buffer id(%#" PRIx64 ")",buffer.first);
                break;
        }

    }

    for( const auto& buffer : request->vOImageBuffers )
    {
        if (buffer.first == eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING)
        {
            if (rootNode == eNODEID_P2CaptureNode)
            {
                needISPPack = true;
                ISPPackImageMap.addOut(buffer.first);
                FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
            }
        }
        else
        {
            switch (buffer.second->getStreamInfo()->getImgFormat())
            {
                case eImgFmt_RAW16:
                case eImgFmt_BAYER8_UNPAK:
                case eImgFmt_BAYER10_UNPAK:
                case eImgFmt_BAYER12_UNPAK:
                case eImgFmt_BAYER14_UNPAK:
                case eImgFmt_BAYER15_UNPAK:
                    P2CImageMap.addOut(buffer.first);
                    FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                    featureParams.pAppImage_Output_RAW16 = mConfigureInfo.pvISPImage_RawOut[buffer.second->getStreamInfo()->getStreamId()];
                    break;
                case eImgFmt_BLOB:
                case eImgFmt_JPEG:
                    JpegImageMap.addOut(buffer.first);
                    if (mConfigureInfo.AppYuvDirectJpeg)
                    {
                        MY_LOGD("direct user yuv to jpeg");
                        JpegImageMap.addIn(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_YUV);
                    }
                    else
                    {
                        JpegImageMap.addIn(eSTREAMID_IMAGE_PIPE_YUV_JPEG_00);
                        P2CImageMap.addOut(eSTREAMID_IMAGE_PIPE_YUV_JPEG_00);
                    }
                    if (!mConfigureInfo.WithoutThumbnail)
                    {
                        if (!mConfigureInfo.OnlyUseJpegNode)
                        {
                            JpegImageMap.addIn(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00);
                        }
                        P2CImageMap.addOut(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00);
                    }
                    needJpeg = true;
                    FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                    featureParams.pAppImage_Output_Jpeg = mConfigureInfo.pISPImage_JpegOut;
                    break;
                case eImgFmt_YV12:
                case eImgFmt_NV21:
                case eImgFmt_YUY2:
                case eImgFmt_Y8:
                case eImgFmt_Y16:
                    P2CImageMap.addOut(buffer.first);
                    FrameBuilder->setAppImageStreamBuffer(buffer.first, buffer.second);
                    featureParams.pAppImage_Output_Yuv = mConfigureInfo.pvISPImage_YuvOut[buffer.second->getStreamInfo()->getStreamId()];
                    break;
                    //
                default:
                    CAM_LOGE("[%s] Unsupported format:0x%x", __FUNCTION__, buffer.second->getStreamInfo()->getImgFormat());
                    break;
            }
        }
    }

    for( const auto& buffer : request->vIMetaBuffers )
    {
        switch (buffer.first)
        {
            case eSTREAMID_META_APP_CONTROL:
                P2CMetaMap.addIn(buffer.first);
                JpegMetaMap.addIn(buffer.first);
                ISPPackMetaMap.addIn(buffer.first);
                FrameBuilder->setAppMetaStreamBuffer(buffer.first, buffer.second);
                pAppMetaBuffer = buffer.second;
                pAppMetaControl = buffer.second->tryWriteLock(LOG_TAG);
                featureParams.pRequest_AppControl = pAppMetaControl;
                break;
            case eSTREAMID_META_APP_DYNAMIC_01:
                P2CMetaMap.addIn(buffer.first);
                ISPPackMetaMap.addIn(buffer.first);
                FrameBuilder->setAppMetaStreamBuffer(buffer.first, buffer.second);
                pAppResultBuffer = buffer.second;
                pAppMetaResult = buffer.second->tryWriteLock(LOG_TAG);
                featureParams.pRequest_AppResult = pAppMetaResult;
                break;
            case eSTREAMID_META_PIPE_DYNAMIC_01:
                P2CMetaMap.addIn(buffer.first);
                FrameBuilder->setAppMetaStreamBuffer(buffer.first, buffer.second);
                pHalMetaBuffer = buffer.second;
                pHalMetaControl = buffer.second->tryWriteLock(LOG_TAG);
                featureParams.pRequest_HalResult = pHalMetaControl;
                if (pRawInputBuffer != nullptr && pHalMetaControl != nullptr)
                {
                    auto pBufferHeap = pRawInputBuffer->tryWriteLock(LOG_TAG);
                    IMetadata::IEntry Entry = pHalMetaControl->entryFor(MTK_P1NODE_SENSOR_RAW_ORDER);
                    if (!Entry.isEmpty())
                    {
                        MINT32 order = Entry.itemAt(0, Type2Type<MINT32>());
                        pBufferHeap->setColorArrangement(order);
                        MY_LOGD("set bayer-order = %d", order);
                    }
                    else
                    {
                        MY_LOGD("bayer-order info is empty");
                    }
                    pRawInputBuffer->unlock(LOG_TAG, pBufferHeap);
                }
                // Add ISP Hint
                if (pHalMetaControl != nullptr)
                {
                    IMetadata::IEntry ispEntry(MTK_HAL_REQUEST_ISP_PIPELINE_MODE);
                    ispEntry.push_back(1, Type2Type<MINT32>());
                    pHalMetaControl->update(MTK_HAL_REQUEST_ISP_PIPELINE_MODE, ispEntry);
                }
                break;
        }
    }

    FrameBuilder->setReprocessFrame(false);
    NodeSet roots;
    NodeEdgeSet edges;

    roots.add(rootNode);

    FrameBuilder->setRootNode( roots );

    if (needP2C)
    {
        P2CMetaMap.addOut(eSTREAMID_META_APP_DYNAMIC_02_CAP);
        P2CMetaMap.addOut(eSTREAMID_META_PIPE_DYNAMIC_02_CAP);
        FrameBuilder->setIOMap(eNODEID_P2CaptureNode, IOMapSet().add(P2CImageMap), IOMapSet().add(P2CMetaMap));
    }

    if (needISPPack)
    {
        ISPPackMetaMap.addIn(eSTREAMID_META_APP_DYNAMIC_02_CAP);
        ISPPackMetaMap.addIn(eSTREAMID_META_PIPE_DYNAMIC_02_CAP);
        ISPPackMetaMap.addOut(eSTREAMID_META_APP_DYNAMIC_P2ISPPack);
        edges.addEdge(eNODEID_P2CaptureNode, eNODEID_P2YuvIspTuningDataPackNode);
        FrameBuilder->setIOMap(eNODEID_P2YuvIspTuningDataPackNode, IOMapSet().add(ISPPackImageMap), IOMapSet().add(ISPPackMetaMap));
    }

    if (needJpeg)
    {
        if (needP2C == false)
        {
            MY_LOGI("direct to jpeg, use user hal meta");
            JpegMetaMap.addIn(eSTREAMID_META_PIPE_DYNAMIC_01);
        }
        else
        {
            JpegMetaMap.addIn(eSTREAMID_META_PIPE_DYNAMIC_02_CAP);
        }
        JpegMetaMap.addOut(eSTREAMID_META_APP_DYNAMIC_JPEG);
        // create new streaminfo if need
        createRotationStreamInfo_Main_YUV(pAppMetaControl, mConfigureInfo.pHalImage_Jpeg_YUV, pMainJpegYuv);
        if (mConfigureInfo.WithoutThumbnail == false)
        {
            createStreamInfo_Thumbnail_YUV(pAppMetaControl
                                         , mConfigureInfo.pHalImage_Jpeg_YUV
                                         , mConfigureInfo.pHalImage_Thumbnail_YUV
                                         , pThumbnailYuv);
        }
        if (pMainJpegYuv != nullptr && mConfigureInfo.AppYuvDirectJpeg == false)
        {
            FrameBuilder->replaceStreamInfo( eSTREAMID_IMAGE_PIPE_YUV_JPEG_00, pMainJpegYuv );
        }
        if (pThumbnailYuv != nullptr && mConfigureInfo.OnlyUseJpegNode == false && mConfigureInfo.WithoutThumbnail == false)
        {
            FrameBuilder->replaceStreamInfo( eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00, pThumbnailYuv );
        }
        if (mConfigureInfo.OnlyUseJpegNode == false)
        {
            edges.addEdge(eNODEID_P2CaptureNode, eNODEID_JpegNode);
        }
        FrameBuilder->setIOMap(eNODEID_JpegNode, IOMapSet().add(JpegImageMap), IOMapSet().add(JpegMetaMap));
    }

    FrameBuilder->setNodeEdges( edges );
    mpFeaturePolicy->evaluateRequest(&featureParams);

    pAppMetaBuffer->unlock(LOG_TAG, pAppMetaControl);
    pAppResultBuffer->unlock(LOG_TAG, pAppMetaResult);
    pHalMetaBuffer->unlock(LOG_TAG, pHalMetaControl);

    if (frameIndex == 0)
    {
        FrameBuilder->updateFrameCallback(this);
    }
    else
    {
        MY_LOGD("frame index is not zero, assume it is subframe, no need callback");
        FrameBuilder->updateFrameCallback(nullptr);
    }
    FrameBuilder->setRequestNo(request->requestNo);

    auto RequestBuilder = IRequestBuilder::make();

    pFrame = RequestBuilder->build(pContext, FrameBuilder);

    MY_LOGD("- err:%d, pPipelineFrame : %p", err, pFrame.get());
    return err;
}


