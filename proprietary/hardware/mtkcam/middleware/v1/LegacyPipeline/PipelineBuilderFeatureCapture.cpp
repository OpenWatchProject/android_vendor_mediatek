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

#define LOG_TAG "MtkCam/PipeBuilderFeatureCapture"

#include "MyUtils.h"
#include "PipelineBuilderFeatureCapture.h"
#include "LegacyPipelineCapture.h"
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2FeatureNode.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")


/******************************************************************************
*
*******************************************************************************/
static
MVOID   add_stream_to_set( StreamSet& set, sp<IStreamInfo> pInfo ) {
    if( pInfo.get() ) set.add(pInfo->getStreamId());
}

/******************************************************************************
*
*******************************************************************************/
enum
{
    eConfigP1   = 0x1,
    eConfigP2   = 0x2,
    eConfigJpeg = 0x4,
};

struct PipelineBuilderFeatureCapture::myConfig
    : public LegacyPipelineCapture::Configuration
{
};


/******************************************************************************
*
*******************************************************************************/
struct iomapImageHelper
{
    struct Param
    {
        bool                                    bMemoryIn;
        MINT32                                  dualcamMode;
        LegacyPipelineCapture::Configuration*   pCfg;
        IOMap*                                  pMapP1;
        IOMap*                                  pMapP2;
        IOMap*                                  pMapJpeg;
    };

    MBOOL operator() (Param& rParam) {
        return
            updateJpeg(rParam) &&
            updateYuv(rParam) &&
            updateRaw(rParam);
    }

#define ASSERT_NOT_NULL( _StreamInfo_, errmsg ) do{                   \
    if( _StreamInfo_ == 0 ) { MY_LOGE("%s", errmsg); return MFALSE; } \
} while(0)

MBOOL updateJpeg(Param& rParam) {
    sp<IImageStreamInfo>& pImgoRaw      = rParam.pCfg->pImage_ImgoRaw;
    sp<IImageStreamInfo>& pLcsoRaw      = rParam.pCfg->pImage_LcsoRaw;
    sp<IImageStreamInfo>& pYuvJpeg      = rParam.pCfg->pImage_YuvJpeg;
    sp<IImageStreamInfo>& pYuvThumbnail = rParam.pCfg->pImage_YuvThumbnail;
    sp<IImageStreamInfo>& pJpeg         = rParam.pCfg->pImage_Jpeg;
    //
    if( pJpeg == 0 ) return MTRUE;
    ASSERT_NOT_NULL(pYuvJpeg, "should config yuv");
    ASSERT_NOT_NULL(pImgoRaw, "should config imgo");
    //
    if(!rParam.bMemoryIn)
    {
        rParam.pMapP1->addOut  ( pImgoRaw->getStreamId() );
        if ( pLcsoRaw != NULL)
            rParam.pMapP1->addOut  ( pLcsoRaw->getStreamId() );
    }
    rParam.pMapP2->addIn   ( pImgoRaw->getStreamId() );
    if ( pLcsoRaw != NULL)
        rParam.pMapP2->addIn  ( pLcsoRaw->getStreamId() );
    rParam.pMapP2->addOut  ( pYuvJpeg->getStreamId() );
    rParam.pMapJpeg->addIn ( pYuvJpeg->getStreamId() );
    rParam.pMapJpeg->addOut( pJpeg->getStreamId() );
    if( pYuvThumbnail != 0 ) {
        rParam.pMapP2->addOut ( pYuvThumbnail->getStreamId() );
        rParam.pMapJpeg->addIn( pYuvThumbnail->getStreamId() );
    }
    return MTRUE;
}
MBOOL updateYuv(Param& rParam) {
    sp<IImageStreamInfo>& pImgoRaw = rParam.pCfg->pImage_ImgoRaw;
    sp<IImageStreamInfo>& pLcsoRaw = rParam.pCfg->pImage_LcsoRaw;
    sp<IImageStreamInfo>& pYuv     = rParam.pCfg->pImage_Yuv00;
    //
    if( pYuv == 0 ) return MTRUE;
    ASSERT_NOT_NULL(pImgoRaw, "should config imgo");
    //
    if(!rParam.bMemoryIn)
    {
        rParam.pMapP1->addOut  ( pImgoRaw->getStreamId() );
        if ( pLcsoRaw != NULL)
            rParam.pMapP1->addOut  ( pLcsoRaw->getStreamId() );
    }
    rParam.pMapP2->addIn   ( pImgoRaw->getStreamId() );
    if ( pLcsoRaw != NULL)
        rParam.pMapP2->addIn  ( pLcsoRaw->getStreamId() );
    rParam.pMapP2->addOut  ( pYuv->getStreamId() );
    return MTRUE;
}
MBOOL updateRaw(Param& rParam) {
    sp<IImageStreamInfo>& pRrzoRaw = rParam.pCfg->pImage_RrzoRaw;
    sp<IImageStreamInfo>& pImgoRaw = rParam.pCfg->pImage_ImgoRaw;
    sp<IImageStreamInfo>& pLcsoRaw = rParam.pCfg->pImage_LcsoRaw;
    //
    if( pRrzoRaw == 0 && pImgoRaw == 0 ) {
        MY_LOGE("should config imgo or rrzo at least");
        return MFALSE;
    }
    //
    if( pRrzoRaw != 0 && !rParam.bMemoryIn ) {
        rParam.pMapP1->addOut ( pRrzoRaw->getStreamId() );
    }
    if( pImgoRaw != 0 && !rParam.bMemoryIn ) {
        rParam.pMapP1->addOut ( pImgoRaw->getStreamId() );
    }
    if( pLcsoRaw != 0 && !rParam.bMemoryIn ) {
        rParam.pMapP1->addOut ( pLcsoRaw->getStreamId() );
    }
    return MTRUE;
}
# undef ASSERT_NOT_NULL
};


/******************************************************************************
*
*******************************************************************************/
PipelineBuilderFeatureCapture::
PipelineBuilderFeatureCapture(
    MINT32 const openId,
    char const* pipeName,
    ConfigParams const & rParam
)
    : PipelineBuilderBase(openId, pipeName, rParam)
    , mpConfig( new myConfig() )
{}

/******************************************************************************
*
*******************************************************************************/
PipelineBuilderFeatureCapture::
~PipelineBuilderFeatureCapture()
{
    if( mpConfig ) delete mpConfig;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
PipelineBuilderFeatureCapture::
buildStream(
    sp<PipelineContext> rpPipelineContext,
    const PipelineImageParam& params
)
{
    CAM_TRACE_CALL();

    if ( params.pProvider != 0 ) {
        mpLegacyPipeline->setStreamBufferProvider(params.pProvider);
        StreamBuilder(eStreamType_IMG_HAL_PROVIDER, params.pInfo)
            .setProvider(params.pProvider)
            .build(rpPipelineContext);
    } else {
        StreamBuilder(eStreamType_IMG_HAL_POOL, params.pInfo)
            .build(rpPipelineContext);
    }
}

/******************************************************************************
*
*******************************************************************************/
MERROR
PipelineBuilderFeatureCapture::
configureP1Node(
    sp<PipelineContext> rpPipelineContext,
    MUINT32& flag
)
{
    FUNC_START;

    CAM_TRACE_CALL();

    sp<IMetaStreamInfo>& pControlMeta_App   = mpConfig->pControlMeta_App;
    sp<IMetaStreamInfo>& pControlMeta_Hal   = mpConfig->pControlMeta_Hal;
    sp<IMetaStreamInfo>& pResultMeta_P1_App = mpConfig->pResultMeta_P1_App;
    sp<IMetaStreamInfo>& pResultMeta_P1_Hal = mpConfig->pResultMeta_P1_Hal;
    sp<IImageStreamInfo>& pImage_RrzoRaw    = mpConfig->pImage_RrzoRaw;
    sp<IImageStreamInfo>& pImage_ImgoRaw    = mpConfig->pImage_ImgoRaw;
    sp<IImageStreamInfo>& pImage_LcsoRaw    = mpConfig->pImage_LcsoRaw;

    Vector<PipelineImageParam> vParams;
    if ( !queryRawDst(vParams) ) {
        MY_LOGD("No need to configure P1 node.");
        return OK;
    }

    // Meta
    {
         CAM_TRACE_NAME("Meta");

         pResultMeta_P1_App =
            new MetaStreamInfo(
                "App:Meta:ResultP1",
                eSTREAMID_META_APP_DYNAMIC_P1,
                eSTREAMTYPE_META_OUT,
                0
                );
         pResultMeta_P1_Hal =
            new MetaStreamInfo(
                "Hal:Meta:ResultP1",
                eSTREAMID_META_HAL_DYNAMIC_P1,
                eSTREAMTYPE_META_INOUT,
                0
                );

        StreamBuilder(eStreamType_META_APP, pResultMeta_P1_App)
            .build(rpPipelineContext);
        StreamBuilder(eStreamType_META_HAL, pResultMeta_P1_Hal)
            .build(rpPipelineContext);
    }

    // Image
    {
        CAM_TRACE_NAME("Image");
        for ( size_t i = 0; i < vParams.size(); ++i ) {
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE ) {
                pImage_ImgoRaw = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO ) {
                pImage_LcsoRaw = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER ) {
                pImage_RrzoRaw = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            //
            MY_LOGW("not supported stream %#" PRIx64 "(%s)",
                    vParams[i].pInfo->getStreamId(),
                    vParams[i].pInfo->getStreamName());
            return BAD_VALUE;
        }
    }

    // Node
    {
        CAM_TRACE_NAME("Node");

        PipelineSensorParam aParam;
        if ( !querySrcParam(aParam) ) {
            MY_LOGD("No sensor setting.");
            return BAD_VALUE;
        }
        typedef P1Node                  NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder p1 +");
        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_P1Node;
            initParam.nodeName = "P1Node";
        }
        NodeT::SensorParams sensorParam;
        {
            sensorParam.mode = aParam.mode;
            sensorParam.size = aParam.size;
            sensorParam.fps  = aParam.fps;
            sensorParam.pixelMode  = aParam.pixelMode;
#ifdef FEATURE_MODIFY
            sensorParam.vhdrMode = aParam.vhdrMode;
#endif // FEATURE_MODIFY
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta         = pControlMeta_App;
            cfgParam.pInHalMeta         = pControlMeta_Hal;
            cfgParam.pOutAppMeta        = pResultMeta_P1_App;
            cfgParam.pOutHalMeta        = pResultMeta_P1_Hal;
            cfgParam.pOutImage_resizer  = pImage_RrzoRaw;
            cfgParam.pOutImage_lcso     = pImage_LcsoRaw;
            cfgParam.sensorParams       = sensorParam;
            cfgParam.rawProcessed       = aParam.rawType;
            cfgParam.enableEIS          = mConfigParams.enableEIS;
            cfgParam.enableLCS          = mConfigParams.enableLCS;
            cfgParam.enableDualPD          = mConfigParams.enableDualPD;
            cfgParam.disableFrontalBinning = mConfigParams.disableFrontalBinning;
            cfgParam.enableUNI          = mConfigParams.enableUNI;
            cfgParam.receiveMode = mConfigParams.receiveMode;
            cfgParam.packedEisInfo      = mConfigParams.packedEisInfo;
            cfgParam.disableHLR = mConfigParams.disableHLR;
            cfgParam.pipeBit = mConfigParams.pipeBit;
            update3AConfigMeta(&cfgParam.cfgAppMeta, &cfgParam.cfgHalMeta);
            if ( pImage_RrzoRaw != 0 ) {
                cfgParam.pStreamPool_resizer =
                    rpPipelineContext->queryImageStreamPool(pImage_RrzoRaw->getStreamId());
            }
            if ( pImage_ImgoRaw != 0 ) {
                cfgParam.pvOutImage_full.push_back(pImage_ImgoRaw);
                cfgParam.pStreamPool_full =
                    rpPipelineContext->queryImageStreamPool(pImage_ImgoRaw->getStreamId());
            }
            if ( pImage_LcsoRaw != 0 ) {
                cfgParam.pStreamPool_lcso =
                    rpPipelineContext->queryImageStreamPool(pImage_LcsoRaw->getStreamId());
            }
        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, pControlMeta_App);
        add_stream_to_set(vIn, pControlMeta_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, pImage_ImgoRaw);
        add_stream_to_set(vOut, pImage_RrzoRaw);
        add_stream_to_set(vOut, pImage_LcsoRaw);
        add_stream_to_set(vOut, pResultMeta_P1_App);
        add_stream_to_set(vOut, pResultMeta_P1_Hal);
        //
        NodeBuilder aNodeBuilder(
                eNODEID_P1Node,
                pNode
                );
        aNodeBuilder.addStream(
                    NodeBuilder::eDirection_IN,
                    vIn
                    );
        aNodeBuilder.addStream(
                    NodeBuilder::eDirection_OUT,
                    vOut
                    );
        if ( pImage_ImgoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_ImgoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( pImage_RrzoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_RrzoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( pImage_LcsoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_LcsoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );

        MERROR ret = aNodeBuilder.build(rpPipelineContext);
        MY_LOGD("Nodebuilder p1 -");

        if( ret != OK ) {
            MY_LOGE("build p1 node error");
            return ret;
        }
    }
    FUNC_END;

    flag |= eConfigP1;
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
PipelineBuilderFeatureCapture::
configureP2FeatureNode(
    sp<PipelineContext> rpPipelineContext,
    MUINT32& flag
)
{
    FUNC_START;
    CAM_TRACE_CALL();

    Vector<PipelineImageParam> srcParams;
    MBOOL bMemoryIn = querySrcParam(srcParams);
    if(bMemoryIn)
    {
        for(size_t i = 0 ; i < srcParams.size() ; i++)
        {
            if(srcParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
                mpConfig->pImage_ImgoRaw = srcParams[i].pInfo;
            if(srcParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
                mpConfig->pImage_LcsoRaw = srcParams[i].pInfo;
        }
        mpConfig->pResultMeta_P1_Hal = mpConfig->pControlMeta_Hal;
        mpConfig->pResultMeta_P1_App =
            new MetaStreamInfo(
                    "App:Meta:ResultP1",
                    eSTREAMID_META_APP_DYNAMIC_P1,
                    eSTREAMTYPE_META_OUT,
                    0
                    );
        StreamBuilder(eStreamType_META_APP, mpConfig->pResultMeta_P1_App)
            .build(rpPipelineContext);
    }

    sp<IMetaStreamInfo>& pControlMeta_App     = mpConfig->pControlMeta_App;
    sp<IMetaStreamInfo>& pResultMeta_P1_App   = mpConfig->pResultMeta_P1_App;
    sp<IMetaStreamInfo>& pResultMeta_P1_Hal   = mpConfig->pResultMeta_P1_Hal;
    sp<IImageStreamInfo>& pImage_RrzoRaw      = mpConfig->pImage_RrzoRaw;
    sp<IImageStreamInfo>& pImage_ImgoRaw      = mpConfig->pImage_ImgoRaw;
    sp<IImageStreamInfo>& pImage_LcsoRaw      = mpConfig->pImage_LcsoRaw;
    sp<IMetaStreamInfo>& pResultMeta_P2_App   = mpConfig->pResultMeta_P2_App;
    sp<IMetaStreamInfo>& pResultMeta_P2_Hal   = mpConfig->pResultMeta_P2_Hal;
    sp<IImageStreamInfo>& pImage_Yuv00        = mpConfig->pImage_Yuv00;
    sp<IImageStreamInfo>& pImage_YuvJpeg      = mpConfig->pImage_YuvJpeg;
    sp<IImageStreamInfo>& pImage_YuvThumbnail = mpConfig->pImage_YuvThumbnail;

    Vector<PipelineImageParam> vParams;

    if ( !queryYuvDst(vParams) ) {
        MY_LOGD("No need to configure P2 feature node.");
        return OK;
    }

    // Meta
    {
         CAM_TRACE_NAME("Meta");
         pResultMeta_P2_App =
            new MetaStreamInfo(
                "App:Meta:ResultP2",
                eSTREAMID_META_APP_DYNAMIC_P2,
                eSTREAMTYPE_META_OUT,
                0
                );
         pResultMeta_P2_Hal =
            new MetaStreamInfo(
                "Hal:Meta:ResultP2",
                eSTREAMID_META_HAL_DYNAMIC_P2,
                eSTREAMTYPE_META_INOUT,
                0
                );

        StreamBuilder(eStreamType_META_APP, pResultMeta_P2_App)
            .build(rpPipelineContext);
        StreamBuilder(eStreamType_META_HAL, pResultMeta_P2_Hal)
            .build(rpPipelineContext);
    }

    // Image
    {
        CAM_TRACE_NAME("Image");
        if (bMemoryIn)
        {
            for (size_t i = 0 ; i < srcParams.size() ; i++)
                buildStream(rpPipelineContext, srcParams[i]);
        }
        //
        for ( size_t i = 0; i < vParams.size(); ++i ) {
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_00 ) {
                pImage_Yuv00 = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_JPEG ) {
                pImage_YuvJpeg = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL ) {
                pImage_YuvThumbnail = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            //
            MY_LOGW("not supported stream %#" PRIx64 "(%s)",
                    vParams[i].pInfo->getStreamId(),
                    vParams[i].pInfo->getStreamName());
            return BAD_VALUE;
        }
    }

    // Node
    {
        CAM_TRACE_NAME("Node");
        typedef P2FeatureNode           NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder p2 +");
        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_P2Node;
            initParam.nodeName = "P2FeatureNode";
        }
        MINT32 supportDRE = ::property_get_int32("camera.mdp.dre.enable", 0);
        MINT32 supportCZ = ::property_get_int32("camera.mdp.cz.enable", 0);
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta  = pControlMeta_App;
            cfgParam.pInAppRetMeta = pResultMeta_P1_App;
            cfgParam.pInHalMeta  = pResultMeta_P1_Hal;
            cfgParam.pOutAppMeta = pResultMeta_P2_App;
            cfgParam.pOutHalMeta = pResultMeta_P2_Hal;
            if ( pImage_ImgoRaw != 0 )
                cfgParam.pvInFullRaw.push_back(pImage_ImgoRaw);
            cfgParam.pInResizedRaw = pImage_RrzoRaw;
            cfgParam.pInLcsoRaw = pImage_LcsoRaw;
            if ( pImage_Yuv00 != 0 )
                cfgParam.vOutImage.push_back(pImage_Yuv00);
            if ( pImage_YuvJpeg != 0 )
                cfgParam.vOutImage.push_back(pImage_YuvJpeg);
            cfgParam.pOutFDImage = pImage_YuvThumbnail;
            if(supportCZ)
                cfgParam.customOption |= v3::P2FeatureNode::CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT;
            if(supportDRE)
                cfgParam.customOption |= v3::P2FeatureNode::CUSTOM_OPTION_DRE_SUPPORT;
        }
        //
        sp<MyNodeActorT> pNode;
        if(isNeedP2TimeSharingMode())
        {
            MY_LOGD("P2 feature time sharing");
            pNode = new MyNodeActorT( NodeT::createInstance(P2FeatureNode::PASS2_TIMESHARING, P2FeatureNode::UsageHint(P2FeatureNode::USAGE_TIMESHARE_CAPTURE)) );
        }
        else
        {
            pNode = new MyNodeActorT( NodeT::createInstance(P2FeatureNode::PASS2_STREAM, P2FeatureNode::USAGE_CAPTURE) );
        }
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, pImage_ImgoRaw);
        add_stream_to_set(vIn, pImage_RrzoRaw);
        add_stream_to_set(vIn, pImage_LcsoRaw);
        add_stream_to_set(vIn, pControlMeta_App);
        add_stream_to_set(vIn, pResultMeta_P1_App);
        add_stream_to_set(vIn, pResultMeta_P1_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, pImage_Yuv00);
        add_stream_to_set(vOut, pImage_YuvJpeg);
        add_stream_to_set(vOut, pImage_YuvThumbnail);
        add_stream_to_set(vOut, pResultMeta_P2_App);
        add_stream_to_set(vOut, pResultMeta_P2_Hal);
        //
        NodeBuilder aNodeBuilder(
                eNODEID_P2Node,
                pNode
                );
        aNodeBuilder.addStream(
                    NodeBuilder::eDirection_IN,
                    vIn
                    );
        aNodeBuilder.addStream(
                    NodeBuilder::eDirection_OUT,
                    vOut
                    );
        if ( pImage_ImgoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_ImgoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( pImage_RrzoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_RrzoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( pImage_LcsoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_LcsoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( pImage_Yuv00 != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_Yuv00->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( pImage_YuvJpeg != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_YuvJpeg->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( pImage_YuvThumbnail != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_YuvThumbnail->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );

        MERROR ret = aNodeBuilder.build(rpPipelineContext);
        MY_LOGD("Nodebuilder p2 feature -");

        if( ret != OK ) {
            MY_LOGE("build p2 feature node error");
            return ret;
        }
    }

    FUNC_END;

    flag |= eConfigP2;
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
PipelineBuilderFeatureCapture::
configureJpegNode(
    sp<PipelineContext> rpPipelineContext,
    MUINT32& flag
)
{
    FUNC_START;

    CAM_TRACE_CALL();

    sp<IMetaStreamInfo>& pControlMeta_App     = mpConfig->pControlMeta_App;
    sp<IMetaStreamInfo>& pResultMeta_P2_Hal   = mpConfig->pResultMeta_P2_Hal;
    sp<IImageStreamInfo>& pImage_YuvJpeg      = mpConfig->pImage_YuvJpeg;
    sp<IImageStreamInfo>& pImage_YuvThumbnail = mpConfig->pImage_YuvThumbnail;
    sp<IMetaStreamInfo>& pResultMeta_Jpeg_App = mpConfig->pResultMeta_Jpeg_App;
    sp<IImageStreamInfo>& pImage_Jpeg         = mpConfig->pImage_Jpeg;

    Vector<PipelineImageParam> vParams;
    if ( !queryJpegDst(vParams) ) {
        MY_LOGD("No need to configure Jpeg node.");
        return OK;
    }

    // Meta
    {
         CAM_TRACE_NAME("Meta");
         pResultMeta_Jpeg_App =
            new MetaStreamInfo(
                "App:Meta:ResultJpeg",
                eSTREAMID_META_APP_DYNAMIC_JPEG,
                eSTREAMTYPE_META_OUT,
                0
                );
        StreamBuilder(eStreamType_META_APP, pResultMeta_Jpeg_App)
            .build(rpPipelineContext);
    }

    // Image
    {
        CAM_TRACE_NAME("Image");
        for ( size_t i = 0; i < vParams.size(); ++i ) {
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_JPEG ) {
                pImage_Jpeg = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            //
            MY_LOGW("not supported stream %#" PRIx64 "(%s)",
                    vParams[i].pInfo->getStreamId(),
                    vParams[i].pInfo->getStreamName());
            return BAD_VALUE;
        }
    }

    // Node
    {
        CAM_TRACE_NAME("Node");
        typedef JpegNode                  NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder Jpeg +");
        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_JpegNode;
            initParam.nodeName = "JpegNode";
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta       = pControlMeta_App;
            cfgParam.pInHalMeta       = pResultMeta_P2_Hal;
            cfgParam.pOutAppMeta      = pResultMeta_Jpeg_App;
            cfgParam.pInYuv_Main      = pImage_YuvJpeg;
            cfgParam.pInYuv_Thumbnail = pImage_YuvThumbnail;
            cfgParam.pOutJpeg         = pImage_Jpeg;
        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, pImage_YuvJpeg);
        add_stream_to_set(vIn, pImage_YuvThumbnail);
        add_stream_to_set(vIn, pControlMeta_App);
        add_stream_to_set(vIn, pResultMeta_P2_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, pImage_Jpeg);
        add_stream_to_set(vOut, pResultMeta_Jpeg_App);
        //
        NodeBuilder aNodeBuilder(
                eNODEID_JpegNode,
                pNode
                );
        aNodeBuilder.addStream(
                    NodeBuilder::eDirection_IN,
                    vIn
                    );
        aNodeBuilder.addStream(
                    NodeBuilder::eDirection_OUT,
                    vOut
                    );
        if ( pImage_YuvJpeg != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_YuvJpeg->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( pImage_YuvThumbnail != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_YuvThumbnail->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( pImage_Jpeg != 0 )
            aNodeBuilder.setImageStreamUsage(
                    pImage_Jpeg->getStreamId(),
                    eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );

        MERROR ret = aNodeBuilder.build(rpPipelineContext);
        MY_LOGD("Nodebuilder jpeg -");

        if( ret != OK ) {
            MY_LOGE("build jpeg node error");
            return ret;
        }
    }

    FUNC_END;

    flag |= eConfigJpeg;
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
sp<ILegacyPipeline>
PipelineBuilderFeatureCapture::
create()
{
    FUNC_START;

    CAM_TRACE_CALL();

    sp<IMetaStreamInfo>&  pControlMeta_App      = mpConfig->pControlMeta_App;
    sp<IMetaStreamInfo>&  pControlMeta_Hal      = mpConfig->pControlMeta_Hal;
    sp<IMetaStreamInfo>&  pResultMeta_P1_App    = mpConfig->pResultMeta_P1_App;
    sp<IMetaStreamInfo>&  pResultMeta_P1_Hal    = mpConfig->pResultMeta_P1_Hal;
    sp<IMetaStreamInfo>&  pResultMeta_P2_App    = mpConfig->pResultMeta_P2_App;
    sp<IMetaStreamInfo>&  pResultMeta_P2_Hal    = mpConfig->pResultMeta_P2_Hal;
    sp<IMetaStreamInfo>&  pResultMeta_Jpeg_App  = mpConfig->pResultMeta_Jpeg_App;

    sp<IImageStreamInfo>& pImage_ImgoRawSrc     = mpConfig->pImage_ImgoRawSrc;
    sp<IImageStreamInfo>& pImage_RrzoRaw        = mpConfig->pImage_RrzoRaw;
    sp<IImageStreamInfo>& pImage_ImgoRaw        = mpConfig->pImage_ImgoRaw;
    sp<IImageStreamInfo>& pImage_LcsoRaw        = mpConfig->pImage_LcsoRaw;
    sp<IImageStreamInfo>& pImage_Yuv00          = mpConfig->pImage_Yuv00;
    sp<IImageStreamInfo>& pImage_YuvJpeg        = mpConfig->pImage_YuvJpeg;
    sp<IImageStreamInfo>& pImage_YuvThumbnail   = mpConfig->pImage_YuvThumbnail;
    sp<IImageStreamInfo>& pImage_Jpeg           = mpConfig->pImage_Jpeg;

    mpLegacyPipeline = new LegacyPipelineCapture(getOpenId());
    sp<PipelineContext> pPipelineContext;

    pPipelineContext = PipelineContext::create( getName() );
    if( !pPipelineContext.get() ) {
        MY_LOGE("cannot create context");
        return NULL;
    }
    //
    CAM_TRACE_BEGIN("beginConfig");
    int ret = pPipelineContext->beginConfigure();
    if( ret != OK )
    {
        MY_LOGE("build beginConfigure error");
        return NULL;
    }
    CAM_TRACE_END();
    //
    MBOOL bMemoryIn = true;
    {
        PipelineSensorParam aParam;
        if ( querySrcParam(aParam) )
        {
            bMemoryIn = false;
            sp<IScenarioControl> pControl;
            if( queryScenarioControl(pControl) )
            {
                pPipelineContext->setScenarioControl(pControl);
            }
            else
            {
                MY_LOGE("get Scenario Control fail");
                return NULL;
            }
        }
        //
        sp<IDispatcher> pDispatcher;
        if( queryDispatcher(pDispatcher) )
        {
            pPipelineContext->setDispatcher(pDispatcher);
        }
    }
    // 1. Control Meta   ***************
    //
    {
        CAM_TRACE_NAME("ControlMeta");
        pControlMeta_App =
            new MetaStreamInfo(
                "App:Meta:Control",
                eSTREAMID_META_APP_CONTROL,
                eSTREAMTYPE_META_IN,
                0
                );
        pControlMeta_Hal =
            new MetaStreamInfo(
                "Hal:Meta:Control",
                eSTREAMID_META_HAL_CONTROL,
                eSTREAMTYPE_META_IN,
                0
                );
        // Meta
        StreamBuilder(eStreamType_META_APP, pControlMeta_App)
            .build(pPipelineContext);
        StreamBuilder(eStreamType_META_HAL, pControlMeta_Hal)
            .build(pPipelineContext);
    }
    //
    // 2. Nodes   ***************
    //
    MUINT32 configFlag = 0;
    {
            if(OK != configureP1Node(pPipelineContext, configFlag))
                MY_LOGW("configureP1Node failed");
            if(OK != configureP2FeatureNode(pPipelineContext, configFlag))
                MY_LOGW("configureP2FeatureNode failed");
            if(OK != configureJpegNode(pPipelineContext, configFlag))
                MY_LOGW("configureJpegNode failed");
    }
    //
    // 3. Pipeline **************
    //
    // edge
    NodeEdgeSet edges;
    if( configFlag & eConfigP1 )    edges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
    if( configFlag & eConfigJpeg )  edges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);
    //
    {
        CAM_TRACE_NAME("PipelineBuilder");
        MERROR ret = PipelineBuilder()
            .setRootNode(
                    NodeSet().add(((configFlag & eConfigP1) ? eNODEID_P1Node : eNODEID_P2Node))
                    )
            .setNodeEdges(edges)
            .build(pPipelineContext);
        if( ret != OK ) {
            MY_LOGE("build pipeline error");
            return NULL;
        }
    }
    //
    CAM_TRACE_BEGIN("endConfig");
    ret = pPipelineContext->endConfigure(true);
    if( ret != OK )
    {
        MY_LOGE("build endConfigure error");
        return NULL;
    }
    CAM_TRACE_END();
    //
    // 4. Request Builder **************

    CAM_TRACE_BEGIN("RequestBuilder");
    IOMap p1_Image_IOMap;
    IOMap p2_Image_IOMap;
    IOMap jpeg_Image_IOMap;
    {
        iomapImageHelper::Param params;
        params.bMemoryIn = bMemoryIn;
        params.dualcamMode = mConfigParams.dualcamMode;
        params.pCfg     = mpConfig;
        params.pMapP1   = &p1_Image_IOMap;
        params.pMapP2   = &p2_Image_IOMap;
        params.pMapJpeg = &jpeg_Image_IOMap;
        if( ! iomapImageHelper() (params) ) {
            MY_LOGE("cannot construct iomap for image");
            CAM_TRACE_END();
            return NULL;
        }
    }

    sp<ResultProcessor> pResultProcessor = ResultProcessor::createInstance();
    sp<RequestBuilder> pRequestBuilder = new RequestBuilder();
    if( configFlag & eConfigP1 )
        pRequestBuilder->setIOMap(
                eNODEID_P1Node,
                IOMapSet().add(
                    p1_Image_IOMap
                    ),
                IOMapSet().add(
                    IOMap()
                    .addIn(pControlMeta_App->getStreamId())
                    .addIn(pControlMeta_Hal->getStreamId())
                    .addOut(pResultMeta_P1_App->getStreamId())
                    .addOut(pResultMeta_P1_Hal->getStreamId())
                    )
                );
    if( configFlag & eConfigP2 )
        pRequestBuilder->setIOMap(
                eNODEID_P2Node,
                IOMapSet().add(p2_Image_IOMap),
                IOMapSet().add(IOMap()
                    .addIn(pControlMeta_App->getStreamId())
                    .addIn(pResultMeta_P1_App->getStreamId())
                    .addIn(((configFlag & eConfigP1) ? pResultMeta_P1_Hal->getStreamId() : pControlMeta_Hal->getStreamId()))
                    .addOut(pResultMeta_P2_App->getStreamId())
                    .addOut(pResultMeta_P2_Hal->getStreamId())
                    )
                );
    if( configFlag & eConfigJpeg )
        pRequestBuilder->setIOMap(
                eNODEID_JpegNode,
                IOMapSet().add(jpeg_Image_IOMap),
                IOMapSet().add(IOMap()
                    .addIn(pControlMeta_App->getStreamId())
                    .addIn(pResultMeta_P2_Hal->getStreamId())
                    .addOut(pResultMeta_Jpeg_App->getStreamId())
                    )
                );
    // root node
    pRequestBuilder->setRootNode(NodeSet().add(((configFlag & eConfigP1) ? eNODEID_P1Node : eNODEID_P2Node)));
    pRequestBuilder->setNodeEdges(edges);
    //
    pRequestBuilder->updateFrameCallback(pResultProcessor);

    CAM_TRACE_END();

    //
    // 4. Register Listener **************
    //
    CAM_TRACE_BEGIN("RegisterListener");
    sp<TimestampProcessor> pTimestampProcessor = TimestampProcessor::createInstance(getOpenId());

    #if MTK_CAM_DISPAY_FRAME_CONTROL_ON
    if(pResultMeta_P1_Hal !=0)
    {
        pResultProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P1,
                            pTimestampProcessor);
    }
    #else
    if( pResultMeta_P1_App != 0 )
    {
        pResultProcessor->registerListener(
                            eSTREAMID_META_APP_DYNAMIC_P1,
                            pTimestampProcessor);
    }
    #endif
    //
    {
        // Image input image
        {
            Vector<PipelineImageParam> params;
            if ( !querySrcParam(params) ) {
                MY_LOGD("No Src Image.");
            } else {
                for(size_t i = 0; i < params.size() ; i++)
                {
                    MY_LOGD("Src buffer provider StreamId:%#" PRIx64 ,params[i].pInfo->getStreamId() );
                    pTimestampProcessor->registerCB(params[i].pProvider);
                }
            }
        }
        // RAW image
        {
            Vector<PipelineImageParam> vParams;
            if ( !queryRawDst(vParams) ) {
                MY_LOGD("No Raw Image.");
            } else {
                for ( size_t i = 0; i < vParams.size(); ++i ) {
                    if ( vParams[i].pProvider != 0 ) {
                        MY_LOGD("Raw buffer provider StreamId:%#" PRIx64 ,vParams[i].pInfo->getStreamId() );
                        pTimestampProcessor->registerCB(vParams[i].pProvider);
                    }
                }
            }
        }
        // YUV image
        {
            Vector<PipelineImageParam> vParams;
            if ( !queryYuvDst(vParams) ) {
                MY_LOGD("No Yuv Image.");
            }else {
                for ( size_t i = 0; i < vParams.size(); ++i ) {
                    if ( vParams[i].pProvider != 0 ) {
                        MY_LOGD("Yuv buffer provider StreamId:%#" PRIx64 ,vParams[i].pInfo->getStreamId() );
                        pTimestampProcessor->registerCB(vParams[i].pProvider);
                    }
                }
            }
        }
        // JPEG image
        {
            Vector<PipelineImageParam> vParams;
            if ( !queryJpegDst(vParams) ) {
                MY_LOGD("No JPEG Image.");
            }else {
                for ( size_t i = 0; i < vParams.size(); ++i ) {
                    if ( vParams[i].pProvider != 0 ) {
                        MY_LOGD("JPEG buffer provider StreamId:%#" PRIx64 ,vParams[i].pInfo->getStreamId() );
                        pTimestampProcessor->registerCB(vParams[i].pProvider);
                    }
                }
            }
        }
    }
    CAM_TRACE_END();
    //
    //
    // 5. Legacy Pipeline **************
    //
    MY_LOGD("Legacy Pipeline +");
    CAM_TRACE_BEGIN("LegacyPipeline");
    mpLegacyPipeline->setPipelineContext(pPipelineContext);
    mpLegacyPipeline->setRequestBuilder(pRequestBuilder);
    mpLegacyPipeline->setResultProcessor(pResultProcessor);
    mpLegacyPipeline->setTimestampProcessor(pTimestampProcessor);
    mpLegacyPipeline->setConfiguration(*mpConfig);
    CAM_TRACE_END();
    MY_LOGD("Legacy Pipeline -");

    FUNC_END;

    return mpLegacyPipeline;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
PipelineBuilderFeatureCapture::
isNeedP2TimeSharingMode(void)
{
    MBOOL ret = MFALSE;
    if ( getMode() == LegacyPipelineMode_T::PipelineMode_Capture_VSS ||
         getMode() == LegacyPipelineMode_T::PipelineMode_MShotCapture ||
         getMode() == LegacyPipelineMode_T::PipelineMode_Feature_MShotCapture ||
         getMode() == LegacyPipelineMode_T::PipelineMode_Feature_Capture_VSS ||
         getMode() == LegacyPipelineMode_T::PipelineMode_Capture ||
         getMode() == LegacyPipelineMode_T::PipelineMode_Feature_Capture )
    {
        MY_LOGD("Set P2 feature TS mode");
        ret = MTRUE;
    }
    return ret;
}

