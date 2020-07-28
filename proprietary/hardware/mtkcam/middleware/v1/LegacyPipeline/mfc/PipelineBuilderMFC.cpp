/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/PipeBuilderMFC"

#include "PipelineBuilderMFC.h"
#include "LegacyPipelineMFC.h"

#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/HDRNode.h>
#include <mtkcam/pipeline/hwnode/MfllNode.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

#define FUNCTION_LOG_START      CAM_LOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        CAM_LOGD("[%s] - X. ret: %d.", __FUNCTION__, ret)
#define FUNCTION_LOG_END_MUM    CAM_LOGD("[%s] - X.", __FUNCTION__)

/**
 *  For BSP, MFNR is not built-in feature
 */
#if MTKCAM_HAVE_MFB_SUPPORT
typedef MfllNode    MfllNode_T;
#else
typedef MFCNode     MfllNode_T;
#endif

// ---------------------------------------------------------------------------

using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;

// ---------------------------------------------------------------------------

inline bool isHdrMode(const MINT32& m)
{
    return (m == LegacyPipelineMode_T::PipelineMode_Hdr) ||
           (m == LegacyPipelineMode_T::PipelineMode_ZsdHdr);
};

inline bool isMfllMode(const MINT32& m)
{
    return (m == LegacyPipelineMode_T::PipelineMode_Mfll) ||
           (m == LegacyPipelineMode_T::PipelineMode_ZsdMfll);
};

auto addStreamToSet = [](StreamSet& set, const sp<IStreamInfo>& info)
{
    if (info.get()) set.add(info->getStreamId());
};

auto linkNode = [](
        NodeEdgeSet& edgeSet, const MINT32 pipelineMode, const MUINT32 configFlag)
{
    if (isHdrMode(pipelineMode))
    {
        if ((configFlag & eNodeConfigHdr) && (configFlag & eNodeConfigP1))
        {
            edgeSet.addEdge(eNODEID_P1Node, eNODEID_HdrNode);
        }

        if (configFlag & eNodeConfigJpeg)
        {
            edgeSet.addEdge(eNODEID_HdrNode, eNODEID_JpegNode);
        }
    }
    else if (isMfllMode(pipelineMode))
    {
        if ((configFlag & eNodeConfigMfll) && (configFlag & eNodeConfigP1))
        {
            edgeSet.addEdge(eNODEID_P1Node, eNODEID_MfllNode);
        }

        if (configFlag & eNodeConfigJpeg)
        {
            edgeSet.addEdge(eNODEID_MfllNode, eNODEID_JpegNode);
        }
    }
};

#define ASSERT_NOT_NULL(_StreamInfo_, errmsg)                                \
    do {                                                                     \
        if (_StreamInfo_ == NULL) { CAM_LOGE("%s", errmsg); return MFALSE; } \
    } while(0)

struct IOMapDefs
{
    struct Param
    {
        sp<Configuration>  config;
        IOMap*             mapP1;
        IOMap*             mapMfc;
        IOMap*             mapJpeg;

        Param()
        : mapP1(NULL),
          mapMfc(NULL),
          mapJpeg(NULL)
        {}
    };
};

class IOMapImageHelper : public IOMapDefs
{
public:
    static MBOOL update(Param& param, MBOOL isMemoryIn, MBOOL isMainResuest = MTRUE)
    {
        sp<Configuration> config = param.config;
        if (config == NULL)
        {
            CAM_LOGE("configuration is not set yet");
            return MFALSE;
        }

        sp<IImageStreamInfo>& imgoRaw       = config->imageInfo_imgoRaw;
        sp<IImageStreamInfo>& rrzoRaw       = config->imageInfo_rrzoRaw;
        sp<IImageStreamInfo>& lcsoRaw       = config->imageInfo_lcsoRaw;
        sp<IImageStreamInfo>& yuvJpeg       = config->imageInfo_yuvJpeg;
        sp<IImageStreamInfo>& yuvThumbnail  = config->imageInfo_yuvThumbnail;
        sp<IImageStreamInfo>& yuv00         = config->imageInfo_yuv00;
        sp<IImageStreamInfo>& jpeg          = config->imageInfo_jpeg;

        ASSERT_NOT_NULL(imgoRaw, "should config imgo");

        if (!isMemoryIn)
        {
            param.mapP1->addOut(imgoRaw->getStreamId());

            if (rrzoRaw.get())
                param.mapP1->addOut(rrzoRaw->getStreamId());

            if (lcsoRaw.get())
                param.mapP1->addOut(lcsoRaw->getStreamId());
        }

        param.mapMfc->addIn(imgoRaw->getStreamId());

        if (lcsoRaw.get())
            param.mapMfc->addIn(lcsoRaw->getStreamId());

        if (isMainResuest)
        {
            ASSERT_NOT_NULL(jpeg, "should config jpeg");
            ASSERT_NOT_NULL(yuvJpeg, "should config yuv");

            param.mapMfc->addOut(yuvJpeg->getStreamId());
            param.mapJpeg->addIn(yuvJpeg->getStreamId());

            if (yuv00.get())
                param.mapMfc->addOut(yuv00->getStreamId());

            // set thumbnail if exists
            if (yuvThumbnail.get())
            {
                param.mapMfc->addOut(yuvThumbnail->getStreamId());
                param.mapJpeg->addIn(yuvThumbnail->getStreamId());
            }

            param.mapJpeg->addOut(jpeg->getStreamId());
        }

        return MTRUE;
    }
};

class IOMapMetaHelper : public IOMapDefs
{
public:
    static MBOOL update(Param& param, MBOOL isMemoryIn, MBOOL isMainResuest = MTRUE)
    {
        return updateControl(param, isMemoryIn, isMainResuest) &&
               updateDynamic(param, isMemoryIn, isMainResuest);
    }

private:
    static MBOOL updateControl(Param& param, MBOOL isMemoryIn, MBOOL isMainResuest)
    {
        sp<Configuration> config = param.config;
        if (config == NULL)
        {
            CAM_LOGE("configuration is not set yet");
            return MFALSE;
        }

        sp<IMetaStreamInfo>& ctrlApp = config->controlMeta_App;
        sp<IMetaStreamInfo>& ctrlHal = config->controlMeta_Hal;
        if (ctrlApp == NULL || ctrlHal == NULL)
        {
            CAM_LOGE("both app(0x%p) and hal(0x%p) metadata should be set",
                    ctrlApp.get(), ctrlHal.get());
            return MFALSE;
        }

        if (isMemoryIn)
        {
            param.mapMfc->addIn(ctrlApp->getStreamId());
            param.mapMfc->addIn(ctrlHal->getStreamId());
        }
        else
        {
            param.mapP1->addIn(ctrlApp->getStreamId());
            param.mapP1->addIn(ctrlHal->getStreamId());
            param.mapMfc->addIn(ctrlApp->getStreamId());
        }

        if (isMainResuest)
        {
            param.mapJpeg->addIn(ctrlApp->getStreamId());
        }

        return MTRUE;
    }

    static MBOOL updateDynamic(Param& param, MBOOL isMemoryIn, MBOOL isMainResuest)
    {
        sp<Configuration> config = param.config;
        if (config == NULL)
        {
            CAM_LOGE("configuration is not set yet");
            return MFALSE;
        }

        sp<IMetaStreamInfo>& p1App   = config->resultMeta_P1_App;
        sp<IMetaStreamInfo>& p1Hal   = config->resultMeta_P1_Hal;
        sp<IMetaStreamInfo>& hdrApp  = config->resultMeta_MFC_App;
        sp<IMetaStreamInfo>& hdrHal  = config->resultMeta_MFC_Hal;
        sp<IMetaStreamInfo>& jpegApp = config->resultMeta_Jpeg_App;

        if (!isMemoryIn)
        {
            ASSERT_NOT_NULL(p1App, "should config P1 result metadata");
        }
        ASSERT_NOT_NULL(hdrApp, "should config HDR result metadata");
        ASSERT_NOT_NULL(jpegApp, "should config Jpeg result metadata");

        if (isMainResuest)
        {
            if (!isMemoryIn)
            {
                param.mapP1->addOut(p1App->getStreamId());
            }
            param.mapMfc->addOut(hdrApp->getStreamId());
            param.mapJpeg->addOut(jpegApp->getStreamId());

            if (p1Hal.get())
            {
                if (!isMemoryIn)
                {
                    param.mapP1->addOut(p1Hal->getStreamId());
                    param.mapMfc->addIn(p1Hal->getStreamId());
                }
            }

            if (hdrHal.get())
            {
                param.mapMfc->addOut(hdrHal->getStreamId());
                param.mapJpeg->addIn(hdrHal->getStreamId());
            }
        }
        else
        {
            if (!isMemoryIn)
            {
                param.mapP1->addOut(p1App->getStreamId());
            }

            if (p1Hal.get())
            {
                if (!isMemoryIn)
                {
                    param.mapP1->addOut(p1Hal->getStreamId());
                    param.mapMfc->addIn(p1Hal->getStreamId());
                }
            }
        }

        return MTRUE;
    }
};
# undef ASSERT_NOT_NULL

static MINT32 getRootNode(const MUINT32 configFlag)
{
    MINT32 rootNode = -1;
    if (configFlag & eNodeConfigP1)
    {
        rootNode = eNODEID_P1Node;
    }
    else if (configFlag & eNodeConfigHdr)
    {
        rootNode = eNODEID_HdrNode;
    }
    else if (configFlag & eNodeConfigMfll)
    {
        rootNode = eNODEID_MfllNode;
    }
    else
    {
        CAM_LOGE("get root node failed configFlag(%#x)", configFlag);
    }

    return rootNode;
}

// ---------------------------------------------------------------------------

PipelineBuilderMFC::PipelineBuilderMFC(
        MINT32 const openId, char const* pipeName, ConfigParams const & rParam)
    : PipelineBuilderBase(openId, pipeName, rParam),
      mConfiguration (new Configuration())
{
    switch (getMode())
    {
        case LegacyPipelineMode_T::PipelineMode_Mfll:
        case LegacyPipelineMode_T::PipelineMode_ZsdMfll:
            CAM_LOGD("create PipelineBuilderMFC for MFLL");
            break;
        case LegacyPipelineMode_T::PipelineMode_Hdr:
        case LegacyPipelineMode_T::PipelineMode_ZsdHdr:
            CAM_LOGD("create PipelineBuilderMFC for HDR");
            break;
        default:
            CAM_LOGE("invalid pipeline mode(%d)", getMode());
    }
}

PipelineBuilderMFC::~PipelineBuilderMFC()
{
    mLegacyPipeline.clear();
    mConfiguration.clear();
}

MVOID PipelineBuilderMFC::buildStream(
        sp<PipelineContext> pipelineContext,
        const PipelineImageParam& params)
{
    // set stream buffer provider if available
    if (params.pProvider.get())
    {
        mLegacyPipeline->setStreamBufferProvider(params.pProvider);
        StreamBuilder(eStreamType_IMG_HAL_PROVIDER, params.pInfo)
            .setProvider(params.pProvider)
            .build(pipelineContext);
        return;
    }

    // otherwise, allocate buffer pool in configuration stage
    // and get streambuffer from pool to use
    StreamBuilder(eStreamType_IMG_HAL_POOL, params.pInfo)
        .build(pipelineContext);
}

MERROR PipelineBuilderMFC::configureP1Node(
        sp<PipelineContext>& pipelineContext, MUINT32& flag)
{
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    Vector<PipelineImageParam> vParams;

    // do nothing if either
    // raw image buffers do not exist or node has been configured before
    if (!queryRawDst(vParams) || (flag & eNodeConfigP1))
    {
        CAM_LOGD("no need to configure P1 node: hasDst(%d) flag(%#08x)",
                queryRawDst(vParams), flag);
        return OK;
    }

    // port definition
    // incoming port: App:Meta:Control, Hal:Meta:Control
    // outgoing port: App:Meta:ResultP1, Hal:Meta:ResultP1,
    //                Hal:Imge:P1_imgoRaw, Hal:Image:P1_rrzoRaw

    // input (metadata)
    sp<IMetaStreamInfo>& controlMeta_App    = mConfiguration->controlMeta_App;
    sp<IMetaStreamInfo>& controlMeta_Hal    = mConfiguration->controlMeta_Hal;
    // output (metadata)
    sp<IMetaStreamInfo>& resultMeta_P1_App  = mConfiguration->resultMeta_P1_App;
    sp<IMetaStreamInfo>& resultMeta_P1_Hal  = mConfiguration->resultMeta_P1_Hal;
    // output (image)
    sp<IImageStreamInfo>& imageInfo_imgoRaw = mConfiguration->imageInfo_imgoRaw;
    sp<IImageStreamInfo>& imageInfo_rrzoRaw = mConfiguration->imageInfo_rrzoRaw;
    sp<IImageStreamInfo>& imageInfo_lcsoRaw = mConfiguration->imageInfo_lcsoRaw;

    // metadata
    {
        resultMeta_P1_App =
            new MetaStreamInfo(
                    "App:Meta:ResultP1",
                    eSTREAMID_META_APP_DYNAMIC_P1,
                    eSTREAMTYPE_META_OUT,
                    0);

        resultMeta_P1_Hal =
            new MetaStreamInfo(
                    "Hal:Meta:ResultP1",
                    eSTREAMID_META_HAL_DYNAMIC_P1,
                    eSTREAMTYPE_META_INOUT,
                    0);

        StreamBuilder(eStreamType_META_APP, resultMeta_P1_App)
            .build(pipelineContext);
        StreamBuilder(eStreamType_META_HAL, resultMeta_P1_Hal)
            .build(pipelineContext);
    }

    // image
    {
        // set to pipeline context
        for (size_t i = 0; i < vParams.size(); i++)
        {
            switch (vParams[i].pInfo->getStreamId())
            {
                case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                    imageInfo_imgoRaw = vParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                    imageInfo_rrzoRaw = vParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                    imageInfo_lcsoRaw = vParams[i].pInfo;
                    break;
                default:
                    CAM_LOGW("not supported stream %#" PRIx64 "(%s)",
                            vParams[i].pInfo->getStreamId(),
                            vParams[i].pInfo->getStreamName());
                    return BAD_VALUE;
            }

            buildStream(pipelineContext, vParams[i]);
        }
    }

    // configure node
    {
        PipelineSensorParam param;

        // get sensor parameters from pipeline builder
        if (!querySrcParam(param))
        {
            CAM_LOGE("no sensor setting");
            return BAD_VALUE;
        }

        typedef P1Node             NodeT;
        typedef NodeActor< NodeT > NodeActorT;

        CAM_LOGD("Nodebuilder p1 +");

        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_P1Node;
            initParam.nodeName = "P1Node";
        }

        // set input/output stream infos and sensor parameters into node
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta        = controlMeta_App;
            cfgParam.pInHalMeta        = controlMeta_Hal;
            cfgParam.pOutAppMeta       = resultMeta_P1_App;
            cfgParam.pOutHalMeta       = resultMeta_P1_Hal;
            cfgParam.pOutImage_resizer = imageInfo_rrzoRaw;
            cfgParam.enableEIS         = mConfigParams.enableEIS;
            cfgParam.enableLCS         = mConfigParams.enableLCS;
            cfgParam.enableDualPD      = mConfigParams.enableDualPD;
            cfgParam.packedEisInfo     = mConfigParams.packedEisInfo;
            NodeT::SensorParams sensorParam;
            {
                sensorParam.mode = param.mode;
                sensorParam.size = param.size;
                sensorParam.fps  = param.fps;
                sensorParam.pixelMode  = param.pixelMode;
            }
            cfgParam.sensorParams       = sensorParam;

            if (imageInfo_imgoRaw.get())
            {
                const StreamId_T streamID = imageInfo_imgoRaw->getStreamId();
                cfgParam.pvOutImage_full.push_back(imageInfo_imgoRaw);
                cfgParam.pStreamPool_full =
                    pipelineContext->queryImageStreamPool(streamID);
            }

            if (imageInfo_rrzoRaw.get())
            {
                const StreamId_T streamID = imageInfo_rrzoRaw->getStreamId();
                cfgParam.pStreamPool_resizer =
                    pipelineContext->queryImageStreamPool(streamID);
            }

            if (imageInfo_lcsoRaw.get())
            {
                const StreamId_T streamID = imageInfo_lcsoRaw->getStreamId();
                cfgParam.pOutImage_lcso = imageInfo_lcsoRaw;
                cfgParam.pStreamPool_lcso =
                    pipelineContext->queryImageStreamPool(streamID);
            }
        }

        sp<NodeActorT> pNode = new NodeActorT(NodeT::createInstance());
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);

        StreamSet vIn;
        addStreamToSet(vIn, controlMeta_App);
        addStreamToSet(vIn, controlMeta_Hal);

        StreamSet vOut;
        addStreamToSet(vOut, imageInfo_imgoRaw);
        addStreamToSet(vOut, imageInfo_rrzoRaw);
        addStreamToSet(vOut, imageInfo_lcsoRaw);
        addStreamToSet(vOut, resultMeta_P1_App);
        addStreamToSet(vOut, resultMeta_P1_Hal);

        NodeBuilder aNodeBuilder(eNODEID_P1Node, pNode);

        aNodeBuilder.addStream(
                NodeBuilder::eDirection_IN, vIn);
        aNodeBuilder.addStream(
                NodeBuilder::eDirection_OUT, vOut);

        // output
        {
            if (imageInfo_imgoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_imgoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }

            if (imageInfo_rrzoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_rrzoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }

            if (imageInfo_lcsoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_lcsoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);

            }
        }

        MERROR ret = aNodeBuilder.build(pipelineContext);

        CAM_LOGD("Nodebuilder p1 -");

        if (ret != OK)
        {
            CAM_LOGE("build p1 node error");
            return ret;
        }
    }

    flag |= eNodeConfigP1;

    FUNCTION_LOG_END_MUM;

    return OK;
}

MERROR PipelineBuilderMFC::configureHDRNode(
        sp<PipelineContext>& pipelineContext, MUINT32& flag)
{
#ifndef MTKCAM_HAVE_HDR
    CAM_LOGE("HDR is not supported");
    return INVALID_OPERATION;
#else
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    Vector<PipelineImageParam> srcParams;
    MBOOL isMemoryIn = querySrcParam(srcParams);
    if (isMemoryIn)
    {
        CAM_LOGI("get image from memory");
        for (size_t i = 0; i < srcParams.size(); i++)
        {
            switch (srcParams[i].pInfo->getStreamId())
            {
                case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                    mConfiguration->imageInfo_imgoRaw = srcParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                    mConfiguration->imageInfo_lcsoRaw = srcParams[i].pInfo;
                    break;
                default:
                    CAM_LOGW("not supported stream %#" PRIx64 ,
                            srcParams[i].pInfo->getStreamId());
            }
        }
        mConfiguration->resultMeta_P1_Hal = mConfiguration->controlMeta_Hal;
    }

    Vector<PipelineImageParam> vParams;

    // do nothing if either
    // yuv/jpeg image buffers do not exist or node has been configured before
    if (!queryYuvDst(vParams) || (flag & eNodeConfigHdr))
    {
        CAM_LOGD("no need to configure MFC node: hasDst(%d) flag(%#08x)",
                queryYuvDst(vParams), flag);
        return OK;
    }

    // port definition
    // incoming port: App:Meta:Control, Hal:Meta:ResultP1,
    //                Hal:Imge:P1_FullRaw, Hal:Image:P1_ResizedRaw
    // outgoing port: App:Meta:ResultMFC, Hal:Meta:ResultMFC,
    //                App:Image:yuv00, Hal:Image:yuvJpeg, Hal:Image:yuvThumbnail

    // input (metadata)
    sp<IMetaStreamInfo>& controlMeta_App         = mConfiguration->controlMeta_App;
    sp<IMetaStreamInfo>& resultMeta_P1_Hal       = mConfiguration->resultMeta_P1_Hal;
    // input (image)
    sp<IImageStreamInfo>& imageInfo_imgoRaw      = mConfiguration->imageInfo_imgoRaw;
    sp<IImageStreamInfo>& imageInfo_lcsoRaw      = mConfiguration->imageInfo_lcsoRaw;
    // output (metadata)
    sp<IMetaStreamInfo>& resultMeta_MFC_App      = mConfiguration->resultMeta_MFC_App;
    sp<IMetaStreamInfo>& resultMeta_MFC_Hal      = mConfiguration->resultMeta_MFC_Hal;
    // output (image)
    sp<IImageStreamInfo>& imageInfo_yuv00        = mConfiguration->imageInfo_yuv00;
    sp<IImageStreamInfo>& imageInfo_yuvJpeg      = mConfiguration->imageInfo_yuvJpeg;
    sp<IImageStreamInfo>& imageInfo_yuvThumbnail = mConfiguration->imageInfo_yuvThumbnail;

    // metadata
    {
        resultMeta_MFC_App =
            new MetaStreamInfo(
                    "App:Meta:ResultMFC",
                    eSTREAMID_META_APP_DYNAMIC_HDR,
                    eSTREAMTYPE_META_OUT,
                    0);

        resultMeta_MFC_Hal =
            new MetaStreamInfo(
                    "Hal:Meta:ResultMFC",
                    eSTREAMID_META_HAL_DYNAMIC_HDR,
                    eSTREAMTYPE_META_INOUT,
                    0);

        StreamBuilder(eStreamType_META_APP, resultMeta_MFC_App)
            .build(pipelineContext);
        StreamBuilder(eStreamType_META_HAL, resultMeta_MFC_Hal)
            .build(pipelineContext);
    }

    // image
    {
        if (isMemoryIn)
        {
            for (size_t i = 0 ; i < srcParams.size() ; i++)
                buildStream(pipelineContext, srcParams[i]);
        }
        // set to pipeline context
        for (size_t i = 0; i < vParams.size(); i++)
        {
            switch (vParams[i].pInfo->getStreamId())
            {
                case eSTREAMID_IMAGE_PIPE_YUV_00:
                    imageInfo_yuv00 = vParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                    imageInfo_yuvJpeg = vParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                    imageInfo_yuvThumbnail = vParams[i].pInfo;
                    break;
                default:
                    CAM_LOGW("not supported stream %#" PRIx64 "(%s)",
                            vParams[i].pInfo->getStreamId(),
                            vParams[i].pInfo->getStreamName());
                    return BAD_VALUE;
            }

            buildStream(pipelineContext, vParams[i]);
        }
    }

    // configure node
    {
        typedef HDRNode            NodeT;
        typedef NodeActor< NodeT > NodeActorT;

        CAM_LOGD("Nodebuilder MFC +");

        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_HdrNode;
            initParam.nodeName = "HDRNode";
        }

        // set input/output stream infos and sensor parameters into node
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta  = controlMeta_App;
            cfgParam.pInHalMeta  = resultMeta_P1_Hal;
            cfgParam.pOutAppMeta = resultMeta_MFC_App;
            cfgParam.pOutHalMeta = resultMeta_MFC_Hal;

            if (imageInfo_imgoRaw.get())
                cfgParam.vInFullRaw.push_back(imageInfo_imgoRaw);

            if (imageInfo_lcsoRaw.get())
                cfgParam.vInLcsoRaw.push_back(imageInfo_lcsoRaw);

            if (imageInfo_yuv00.get())
                cfgParam.vOutImage.push_back(imageInfo_yuv00);

            if (imageInfo_yuvJpeg.get())
                cfgParam.vOutYuvJpeg = imageInfo_yuvJpeg;

            if (imageInfo_yuvThumbnail.get())
                cfgParam.vOutYuvThumbnail = imageInfo_yuvThumbnail;
        }

        #ifdef MTKCAM_HAVE_HDR
            sp<NodeActorT> pNode = new NodeActorT(NodeT::createInstance());
        #else
            sp<NodeActorT> pNode = new NodeActorT(NULL);
        #endif

        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);

        StreamSet vIn;
        addStreamToSet(vIn, imageInfo_imgoRaw);
        addStreamToSet(vIn, imageInfo_lcsoRaw);
        addStreamToSet(vIn, controlMeta_App);
        addStreamToSet(vIn, resultMeta_P1_Hal);

        StreamSet vOut;
        addStreamToSet(vOut, imageInfo_yuv00);
        addStreamToSet(vOut, imageInfo_yuvJpeg);
        addStreamToSet(vOut, imageInfo_yuvThumbnail);
        addStreamToSet(vOut, resultMeta_MFC_App);
        addStreamToSet(vOut, resultMeta_MFC_Hal);

        NodeBuilder aNodeBuilder(eNODEID_HdrNode, pNode);

        aNodeBuilder.addStream(
                NodeBuilder::eDirection_IN, vIn);
        aNodeBuilder.addStream(
                NodeBuilder::eDirection_OUT, vOut);

        // input
        {
            if (imageInfo_imgoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_imgoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
            }

            if (imageInfo_lcsoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_lcsoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
            }
        }

        // output
        {
            if (imageInfo_yuv00.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuv00->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }

            if (imageInfo_yuvJpeg.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuvJpeg->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }

            if (imageInfo_yuvThumbnail.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuvThumbnail->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }
        }

        MERROR ret = aNodeBuilder.build(pipelineContext);

        CAM_LOGD("Nodebuilder MFC -");

        if (ret != OK)
        {
            CAM_LOGE("build MFC node error");
            return ret;
        }
    }

    flag |= eNodeConfigHdr;

    FUNCTION_LOG_END_MUM;

    return OK;
#endif
}

MERROR PipelineBuilderMFC::configureMfllNode(
        sp<PipelineContext>& pipelineContext, MUINT32& flag)
{
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    Vector<PipelineImageParam> srcParams;
    MBOOL isMemoryIn = querySrcParam(srcParams);
    if (isMemoryIn)
    {
        CAM_LOGI("get image from memory");
        for(size_t i = 0 ; i < srcParams.size() ; i++)
        {
            switch (srcParams[i].pInfo->getStreamId())
            {
                case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                    mConfiguration->imageInfo_imgoRaw = srcParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                    mConfiguration->imageInfo_lcsoRaw = srcParams[i].pInfo;
                    break;
                default:
                    CAM_LOGW("not supported stream (%#" PRIx64 ")",
                            srcParams[i].pInfo->getStreamId());
            }
        }
        mConfiguration->resultMeta_P1_Hal = mConfiguration->controlMeta_Hal;
    }

    Vector<PipelineImageParam> vParams;

    // do nothing if either
    // yuv/jpeg image buffers do not exist or node has been configured before
    if (!queryYuvDst(vParams) || (flag & eNodeConfigMfll))
    {
        CAM_LOGD("no need to configure MFC node: hasDst(%d) flag(%#08x)",
                queryYuvDst(vParams), flag);
        return OK;
    }

    // port definition
    // incoming port: App:Meta:Control, Hal:Meta:ResultP1,
    //                Hal:Imge:P1_FullRaw, Hal:Image:P1_ResizedRaw
    // outgoing port: App:Meta:ResultMFC, Hal:Meta:ResultMFC,
    //                App:Image:yuv00, Hal:Image:yuvJpeg, Hal:Image:yuvThumbnail

    // input (metadata)
    sp<IMetaStreamInfo>& controlMeta_App         = mConfiguration->controlMeta_App;
    sp<IMetaStreamInfo>& resultMeta_P1_Hal       = mConfiguration->resultMeta_P1_Hal;
    // input (image)
    sp<IImageStreamInfo>& imageInfo_imgoRaw      = mConfiguration->imageInfo_imgoRaw;
    sp<IImageStreamInfo>& imageInfo_lcsoRaw      = mConfiguration->imageInfo_lcsoRaw;
    // output (metadata)
    sp<IMetaStreamInfo>& resultMeta_MFC_App      = mConfiguration->resultMeta_MFC_App;
    sp<IMetaStreamInfo>& resultMeta_MFC_Hal      = mConfiguration->resultMeta_MFC_Hal;
    // output (image)
    sp<IImageStreamInfo>& imageInfo_yuv00        = mConfiguration->imageInfo_yuv00;
    sp<IImageStreamInfo>& imageInfo_yuvJpeg      = mConfiguration->imageInfo_yuvJpeg;
    sp<IImageStreamInfo>& imageInfo_yuvThumbnail = mConfiguration->imageInfo_yuvThumbnail;

    // metadata
    {
        resultMeta_MFC_App =
            new MetaStreamInfo(
                    "App:Meta:ResultMFC",
                    eSTREAMID_META_APP_DYNAMIC_MFLL,
                    eSTREAMTYPE_META_OUT,
                    0);

        resultMeta_MFC_Hal =
            new MetaStreamInfo(
                    "Hal:Meta:ResultMFC",
                    eSTREAMID_META_HAL_DYNAMIC_MFLL,
                    eSTREAMTYPE_META_INOUT,
                    0);

        StreamBuilder(eStreamType_META_APP, resultMeta_MFC_App)
            .build(pipelineContext);
        StreamBuilder(eStreamType_META_HAL, resultMeta_MFC_Hal)
            .build(pipelineContext);
    }

    // image
    {
        if (isMemoryIn)
        {
            for (size_t i = 0 ; i < srcParams.size() ; i++)
                buildStream(pipelineContext, srcParams[i]);
        }
        // set to pipeline context
        for (size_t i = 0; i < vParams.size(); i++)
        {
            switch (vParams[i].pInfo->getStreamId())
            {
                case eSTREAMID_IMAGE_PIPE_YUV_00:
                    imageInfo_yuv00 = vParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                    imageInfo_yuvJpeg = vParams[i].pInfo;
                    break;
                case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                    imageInfo_yuvThumbnail = vParams[i].pInfo;
                    break;
                default:
                    CAM_LOGW("not supported stream %#" PRIx64 "(%s)",
                            vParams[i].pInfo->getStreamId(),
                            vParams[i].pInfo->getStreamName());
                    return BAD_VALUE;
            }

            buildStream(pipelineContext, vParams[i]);
        }
    }

    // configure node
    {
        /**
         *  Trick: using NfllNode_T
         */
        typedef MfllNode_T         NodeT;
        typedef NodeActor< NodeT > NodeActorT;

        CAM_LOGD("Nodebuilder MFC +");

        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_MfllNode;
            initParam.nodeName = "MFCNode";
        }

        // set input/output stream infos and sensor parameters into node
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta  = controlMeta_App;
            cfgParam.pInHalMeta  = resultMeta_P1_Hal;
            cfgParam.pOutAppMeta = resultMeta_MFC_App;
            cfgParam.pOutHalMeta = resultMeta_MFC_Hal;

            if (imageInfo_imgoRaw.get())
                cfgParam.vInFullRaw.push_back(imageInfo_imgoRaw);

            if (imageInfo_lcsoRaw.get())
                cfgParam.vInLcsoRaw.push_back(imageInfo_lcsoRaw);

            if (imageInfo_yuv00.get())
                cfgParam.vOutImage.push_back(imageInfo_yuv00);

            if (imageInfo_yuvJpeg.get())
                cfgParam.vOutYuvJpeg = imageInfo_yuvJpeg;

            if (imageInfo_yuvThumbnail.get())
                cfgParam.vOutYuvThumbnail = imageInfo_yuvThumbnail;
        }

        sp<NodeActorT> pNode = [this]()->NodeActorT*
        {
            MFCNode::MfcAttribute attr;
            switch (getMode())
            {
                case LegacyPipelineMode_T::PipelineMode_Mfll:
                    return new NodeActorT(NodeT::createInstance());
                case LegacyPipelineMode_T::PipelineMode_ZsdMfll:
                    attr.is_time_sharing = 1;
                    return new NodeActorT(NodeT::createInstance(attr));
                default:
                    CAM_LOGE("create MfllNode fail: not support pipeline mode");
                    return NULL;
            }
        }();

        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);

        StreamSet vIn;
        addStreamToSet(vIn, imageInfo_imgoRaw);
        addStreamToSet(vIn, imageInfo_lcsoRaw);
        addStreamToSet(vIn, controlMeta_App);
        addStreamToSet(vIn, resultMeta_P1_Hal);

        StreamSet vOut;
        addStreamToSet(vOut, imageInfo_yuv00);
        addStreamToSet(vOut, imageInfo_yuvJpeg);
        addStreamToSet(vOut, imageInfo_yuvThumbnail);
        addStreamToSet(vOut, resultMeta_MFC_App);
        addStreamToSet(vOut, resultMeta_MFC_Hal);

        NodeBuilder aNodeBuilder(eNODEID_MfllNode, pNode);

        aNodeBuilder.addStream(
                NodeBuilder::eDirection_IN, vIn);
        aNodeBuilder.addStream(
                NodeBuilder::eDirection_OUT, vOut);

        // input
        {
            if (imageInfo_imgoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_imgoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
            }

            if (imageInfo_lcsoRaw.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_lcsoRaw->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
            }
        }

        // output
        {
            if (imageInfo_yuv00.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuv00->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }

            if (imageInfo_yuvJpeg.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuvJpeg->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }

            if (imageInfo_yuvThumbnail.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuvThumbnail->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
            }
        }

        MERROR ret = aNodeBuilder.build(pipelineContext);

        CAM_LOGD("Nodebuilder MFC -");

        if (ret != OK)
        {
            CAM_LOGE("build MFC node error");
            return ret;
        }
    }

    flag |= eNodeConfigMfll;

    FUNCTION_LOG_END_MUM;

    return OK;
}

MERROR PipelineBuilderMFC::configureJpegNode(
        sp<PipelineContext>& pipelineContext, MUINT32& flag)
{
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    Vector<PipelineImageParam> vParams;

    // do nothing if either
    // jpeg bitstream does not exist or node has been configured before
    if (!queryJpegDst(vParams) || (flag & eNodeConfigJpeg))
    {
        CAM_LOGD("no need to configure Jpeg node: hasDst(%d) flag(%#08x)",
                queryJpegDst(vParams), flag);
        return OK;
    }

    // port definition
    // incoming port: App:Meta:Control, Hal:Meta:ResultMFC,
    //                Hal:Imge:yuvJpeg, Hal:Image:yuvThumbnail
    // outgoing port: App:Meta:ResultJpeg, App:Image:Jpeg

    // input (metadata)
    sp<IMetaStreamInfo>& controlMeta_App     = mConfiguration->controlMeta_App;
    sp<IMetaStreamInfo>& resultMeta_MFC_Hal  = mConfiguration->resultMeta_MFC_Hal;
    // input (image)
    sp<IImageStreamInfo>& imageInfo_yuvJpeg      = mConfiguration->imageInfo_yuvJpeg;
    sp<IImageStreamInfo>& imageInfo_yuvThumbnail = mConfiguration->imageInfo_yuvThumbnail;
    // output (metadata)
    sp<IMetaStreamInfo>& resultMeta_Jpeg_App = mConfiguration->resultMeta_Jpeg_App;
    // output (image)
    sp<IImageStreamInfo>& imageInfo_jpeg     = mConfiguration->imageInfo_jpeg;

    // metadata
    {
        resultMeta_Jpeg_App =
            new MetaStreamInfo(
                    "App:Meta:ResultJpeg",
                    eSTREAMID_META_APP_DYNAMIC_JPEG,
                    eSTREAMTYPE_META_OUT,
                    0);

        StreamBuilder(eStreamType_META_APP, resultMeta_Jpeg_App)
            .build(pipelineContext);
    }

    // image
    {
        // set to pipeline context
        for (size_t i = 0; i < vParams.size(); i++)
        {
            /* HDR/MFLL case */
            if (vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_HDR ||
                vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_MFLL)
            {
                imageInfo_jpeg = vParams[i].pInfo;
                buildStream(pipelineContext, vParams[i]);
                continue;
            }

            CAM_LOGW("not supported stream %#" PRIx64 "(%s)",
                    vParams[i].pInfo->getStreamId(),
                    vParams[i].pInfo->getStreamName());
            return BAD_VALUE;
        }
    }

    // configure node
    {
        typedef JpegNode           NodeT;
        typedef NodeActor< NodeT > NodeActorT;

        CAM_LOGD("Nodebuilder Jpeg +");

        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_JpegNode;
            initParam.nodeName = "JpegNode";
        }

        // set input/output stream infos and sensor parameters into node
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta       = controlMeta_App;
            cfgParam.pInHalMeta       = resultMeta_MFC_Hal;
            cfgParam.pOutAppMeta      = resultMeta_Jpeg_App;
            cfgParam.pInYuv_Main      = imageInfo_yuvJpeg;
            cfgParam.pInYuv_Thumbnail = imageInfo_yuvThumbnail;
            cfgParam.pOutJpeg         = imageInfo_jpeg;
        }

        sp<NodeActorT> pNode = new NodeActorT(NodeT::createInstance());
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);

        StreamSet vIn;

        addStreamToSet(vIn, imageInfo_yuvJpeg);
        addStreamToSet(vIn, imageInfo_yuvThumbnail);
        addStreamToSet(vIn, controlMeta_App);
        addStreamToSet(vIn, resultMeta_MFC_Hal);

        StreamSet vOut;
        addStreamToSet(vOut, imageInfo_jpeg);
        addStreamToSet(vOut, resultMeta_Jpeg_App);

        NodeBuilder aNodeBuilder(eNODEID_JpegNode, pNode);

        aNodeBuilder.addStream(
                NodeBuilder::eDirection_IN, vIn);
        aNodeBuilder.addStream(
                NodeBuilder::eDirection_OUT, vOut);

        // input
        {
            if (imageInfo_yuvJpeg.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuvJpeg->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
            }

            if (imageInfo_yuvThumbnail.get())
            {
                aNodeBuilder.setImageStreamUsage(
                        imageInfo_yuvThumbnail->getStreamId(),
                        eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
            }
        }

        // output
        if (imageInfo_jpeg.get())
        {
            aNodeBuilder.setImageStreamUsage(
                    imageInfo_jpeg->getStreamId(),
                    eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_CAMERA_READWRITE);
        }

        MERROR ret = aNodeBuilder.build(pipelineContext);

        CAM_LOGD("Nodebuilder jpeg -");

        if (ret != OK)
        {
            CAM_LOGE("build jpeg node error");
            return ret;
        }
    }

    flag |= eNodeConfigJpeg;

    FUNCTION_LOG_END_MUM;

    return OK;
}

sp<ILegacyPipeline> PipelineBuilderMFC::create()
{
    FUNCTION_LOG_START;

    CAM_TRACE_CALL();

    // 1. create pipeline context
    mLegacyPipeline = new LegacyPipelineMFC();

    sp<PipelineContext> pipelineContext = PipelineContext::create(getName());
    if (!pipelineContext.get())
    {
        CAM_LOGE("cannot create context");
        return NULL;
    }

    int ret = pipelineContext->beginConfigure();
    if( ret != OK )
    {
        CAM_LOGE("build beginConfigure error");
        return NULL;
    }

    // 2. set scenarol control if the image source is from sensor
    {
        PipelineSensorParam sensorParam;
        if (querySrcParam(sensorParam))
        {
            sp<IScenarioControl> scenarioCtrl;
            if (!queryScenarioControl(scenarioCtrl))
            {
                CAM_LOGE("get Scenario Control failed");
                return NULL;
            }
            pipelineContext->setScenarioControl(scenarioCtrl);
        }

        sp<IDispatcher> dispatcher;
        if (queryDispatcher(dispatcher))
        {
            pipelineContext->setDispatcher(dispatcher);
        }
    }

    // 3. build control metadata stream
    {
        sp<IMetaStreamInfo>& controlMeta_App(mConfiguration->controlMeta_App);
        sp<IMetaStreamInfo>& controlMeta_Hal(mConfiguration->controlMeta_Hal);

        controlMeta_App =
            new MetaStreamInfo(
                    "App:Meta:Control",
                    eSTREAMID_META_APP_CONTROL,
                    eSTREAMTYPE_META_IN,
                    0);

        controlMeta_Hal =
            new MetaStreamInfo(
                    "Hal:Meta:Control",
                    eSTREAMID_META_HAL_CONTROL,
                    eSTREAMTYPE_META_IN,
                    0);

        StreamBuilder(eStreamType_META_APP, controlMeta_App)
            .build(pipelineContext);
        StreamBuilder(eStreamType_META_HAL, controlMeta_Hal)
            .build(pipelineContext);
    }

    // 4. build nodes
    const MINT32 pipelineMode = getMode();
    MUINT32& configFlag(mConfiguration->configFlag);
    {
        if (isHdrMode(pipelineMode))
        {
            if ((configureP1Node(pipelineContext, configFlag) != OK) ||
                (configureHDRNode(pipelineContext, configFlag) != OK) ||
                (configureJpegNode(pipelineContext, configFlag) != OK))
            {
                CAM_LOGE("configure nodes failed");
                pipelineContext->endConfigure();
                return NULL;
            }
        }
        else if (isMfllMode(pipelineMode))
        {
            if ((configureP1Node(pipelineContext, configFlag) != OK) ||
                (configureMfllNode(pipelineContext, configFlag) != OK) ||
                (configureJpegNode(pipelineContext, configFlag) != OK))
            {
                CAM_LOGE("configure nodes failed");
                pipelineContext->endConfigure();
                return NULL;
            }
        }
    }

    // 5. build pipeline
    {
        // edge
        NodeEdgeSet edgeSet;
        linkNode(edgeSet, pipelineMode, configFlag);

        const MINT32 rootNode = getRootNode(configFlag);
        MERROR ret = PipelineBuilder()
            .setRootNode(NodeSet().add(rootNode))
            .setNodeEdges(edgeSet)
            .build(pipelineContext);

        if (ret != OK)
        {
            CAM_LOGE("build pipeline failed");
            return NULL;
        }
    }

    ret = pipelineContext->endConfigure(true);
    if( ret != OK )
    {
        CAM_LOGE("build endConfigure error");
        return NULL;
    }

    // 6. create request builders for different capture settings
    // NOTE: result processor serves several pipeline requests
    sp<ResultProcessor> resultProcessor = ResultProcessor::createInstance();

    Vector< sp<RequestBuilder> > vRequestBuilder;
    for (size_t i = 0; i < 2; i++)
    {
        MERROR ret;

        sp<RequestBuilder> requestBuilder = new RequestBuilder();

        ret = (i == 0) ?
            mainRequestHelper(requestBuilder) : subRequestHelper(requestBuilder);

        if (ret == OK)
        {
            // register listener for main request
            requestBuilder->updateFrameCallback(resultProcessor);
            registerListener(resultProcessor);

            vRequestBuilder.push_back(requestBuilder);
        }
        else
        {
            CAM_LOGE("create %srequest failed", (i == 0) ? "main " : "sub");
        }
    }

    // 7. create and equip legacy pipeline with the following items
    //    - pipeline context
    //    - request builder
    //    - result processor
    //    - timestamp processor
    //    - configuration
    mLegacyPipeline->setPipelineContext(pipelineContext);
    mLegacyPipeline->setRequestBuilder(vRequestBuilder);
    mLegacyPipeline->setResultProcessor(resultProcessor);
    mLegacyPipeline->setTimestampProcessor(mpTimestampProcessor);
    mLegacyPipeline->setConfiguration(mConfiguration);

    FUNCTION_LOG_END_MUM;

    return mLegacyPipeline;
}

MERROR PipelineBuilderMFC::mainRequestHelper(sp<RequestBuilder>& requestBuilder)
{
    CAM_TRACE_CALL();

    IOMap imageIOMapP1;
    IOMap imageIOMapMfc;
    IOMap imageIOMapMfcResized;
    IOMap imageIOMapJpeg;
    Vector<PipelineImageParam> srcParams;
    MBOOL isMemoryIn = querySrcParam(srcParams);
    {
        IOMapImageHelper::Param param;
        param.config        = mConfiguration;
        param.mapP1         = &imageIOMapP1;
        param.mapMfc        = &imageIOMapMfc;
        param.mapJpeg       = &imageIOMapJpeg;
        if (IOMapImageHelper::update(param, isMemoryIn) != MTRUE)
        {
            CAM_LOGE("cannot construct IOMap for image streams");
            return UNKNOWN_ERROR;
        }
    }

    IOMap metaIOMapP1;
    IOMap metaIOMapMfc;
    IOMap metaIOMapJpeg;
    {
        IOMapMetaHelper::Param param;
        param.config  = mConfiguration;
        param.mapP1   = &metaIOMapP1;
        param.mapMfc  = &metaIOMapMfc;
        param.mapJpeg = &metaIOMapJpeg;
        if (IOMapMetaHelper::update(param, isMemoryIn) != MTRUE)
        {
            CAM_LOGE("cannot construct IOMap for meta streams");
            return UNKNOWN_ERROR;
        }
    }

    const MUINT32& configFlag(mConfiguration->configFlag);

    // P1
    if (configFlag & eNodeConfigP1)
    {
        requestBuilder->setIOMap(
                eNODEID_P1Node,
                IOMapSet().add(imageIOMapP1),
                IOMapSet().add(metaIOMapP1));
    }

    // HDR
    if (configFlag & eNodeConfigHdr)
    {
        requestBuilder->setIOMap(
                eNODEID_HdrNode,
                IOMapSet().add(imageIOMapMfc),
                IOMapSet().add(metaIOMapMfc));
    }

    // Mfll
    if (configFlag & eNodeConfigMfll)
    {
        requestBuilder->setIOMap(
                eNODEID_MfllNode,
                IOMapSet().add(imageIOMapMfc),
                IOMapSet().add(metaIOMapMfc));
    }

    // Jpeg
    if (configFlag & eNodeConfigJpeg)
    {
        requestBuilder->setIOMap(
                eNODEID_JpegNode,
                IOMapSet().add(imageIOMapJpeg),
                IOMapSet().add(metaIOMapJpeg));
    }

    // set root node
    MINT32 rootNode = getRootNode(configFlag);
    requestBuilder->setRootNode(NodeSet().add(rootNode));

    // set node edges
    NodeEdgeSet edgeSet;
    linkNode(edgeSet, getMode(), configFlag);
    requestBuilder->setNodeEdges(edgeSet);

    return OK;
}

MERROR PipelineBuilderMFC::subRequestHelper(sp<RequestBuilder>& requestBuilder)
{
    CAM_TRACE_CALL();

    IOMap imageIOMapP1;
    IOMap imageIOMapMfc;
    Vector<PipelineImageParam> srcParams;
    MBOOL isMemoryIn = querySrcParam(srcParams);
    {
        IOMapImageHelper::Param param;
        param.config = mConfiguration;
        param.mapP1  = &imageIOMapP1;
        param.mapMfc = &imageIOMapMfc;
        if (IOMapImageHelper::update(param, isMemoryIn, MFALSE) != MTRUE)
        {
            CAM_LOGE("cannot construct IOMap for image streams");
            return UNKNOWN_ERROR;
        }
    }

    IOMap metaIOMapP1;
    IOMap metaIOMapMfc;
    {
        IOMapMetaHelper::Param param;
        param.config  = mConfiguration;
        param.mapP1   = &metaIOMapP1;
        param.mapMfc  = &metaIOMapMfc;
        if (IOMapMetaHelper::update(param, isMemoryIn, MFALSE) != MTRUE)
        {
            CAM_LOGE("cannot construct IOMap for meta streams");
            return UNKNOWN_ERROR;
        }
    }

    const MUINT32& configFlag(mConfiguration->configFlag);

    // P1
    if (configFlag & eNodeConfigP1)
    {
        requestBuilder->setIOMap(
                eNODEID_P1Node,
                IOMapSet().add(imageIOMapP1),
                IOMapSet().add(metaIOMapP1));
    }

    // HDR
    if (configFlag & eNodeConfigHdr)
    {
        requestBuilder->setIOMap(
                eNODEID_HdrNode,
                IOMapSet().add(imageIOMapMfc),
                IOMapSet().add(metaIOMapMfc));
    }


    // Mfll
    if (configFlag & eNodeConfigMfll)
    {
        requestBuilder->setIOMap(
                eNODEID_MfllNode,
                IOMapSet().add(imageIOMapMfc),
                IOMapSet().add(metaIOMapMfc));
    }

    // set root node
    MINT32 rootNode = getRootNode(configFlag);
    requestBuilder->setRootNode(NodeSet().add(rootNode));

    // set node edges
    NodeEdgeSet edgeSet;
    linkNode(edgeSet, getMode(), (configFlag & ~eNodeConfigJpeg));
    requestBuilder->setNodeEdges(edgeSet);

    return OK;
}

MERROR PipelineBuilderMFC::registerListener(
        const sp<ResultProcessor>& resultProcessor)
{
    CAM_TRACE_CALL();

    Vector<PipelineImageParam> vParams;
    mpTimestampProcessor = TimestampProcessor::createInstance(getOpenId());

    #if MTK_CAM_DISPAY_FRAME_CONTROL_ON
    resultProcessor->registerListener(
                        eSTREAMID_META_HAL_DYNAMIC_P1,
                        mpTimestampProcessor);
    #else
    resultProcessor->registerListener(
            eSTREAMID_META_APP_DYNAMIC_P1, mpTimestampProcessor);
    #endif

    // Image input image
    {
        Vector<PipelineImageParam> params;
        if ( !querySrcParam(params) ) {
            CAM_LOGD("No Src Image.");
        } else {
            for(size_t i = 0; i < params.size() ; i++)
            {
                CAM_LOGD("Src buffer provider StreamId:%#" PRIx64 ,params[i].pInfo->getStreamId() );
                mpTimestampProcessor->registerCB(params[i].pProvider);
            }
        }
    }

    // RAW buffers
    if (!queryRawDst(vParams))
    {
        CAM_LOGD("no need to configure listener for raw buffer");
    }
    else
    {
        for (size_t i = 0; i < vParams.size(); i++)
        {
            if (vParams[i].pProvider.get())
            {
                CAM_LOGD("raw buffer provider streamId(%#" PRIx64 ")",
                        vParams[i].pInfo->getStreamId());
                mpTimestampProcessor->registerCB(vParams[i].pProvider);
            }
        }
    }
    vParams.clear();

    // YUV buffers
    if (!queryYuvDst(vParams))
    {
        CAM_LOGD("no need to configure listener for yuv buffer");
    } else
    {
        for (size_t i = 0; i < vParams.size(); i++)
        {
            if (vParams[i].pProvider.get())
            {
                CAM_LOGD("yuv buffer provider streamId(%#" PRIx64 ")",
                        vParams[i].pInfo->getStreamId());
                mpTimestampProcessor->registerCB(vParams[i].pProvider);
            }
        }
    }
    vParams.clear();

    // JPEG bitstream
    if (!queryJpegDst(vParams))
    {
        CAM_LOGD("no need to configure listener for jpeg buffer");
    }
    else
    {
        for (size_t i = 0; i < vParams.size(); i++)
        {
            if (vParams[i].pProvider.get())
            {
                CAM_LOGD("jpeg buffer provider streamId(%#" PRIx64 ")",
                        vParams[i].pInfo->getStreamId());
                mpTimestampProcessor->registerCB(vParams[i].pProvider);
            }
        }
    }
    vParams.clear();

    return OK;
}
