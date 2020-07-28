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

#define LOG_TAG "MtkCam/PipeBuilderFeaturePreview"

#include "MyUtils.h"
#include "PipelineBuilderFeaturePreview.h"
#include "LegacyPipelinePreview.h"
#ifdef FEATURE_MODIFY
#include <camera_custom_3dnr.h>
#include <camera_custom_fsc.h>
#endif
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2FeatureNode.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>
#include <mtkcam/feature/fsc/fsc_defs.h>
#include <mtkcam/feature/eis/EisInfo.h>
//
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1) && (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/pipeline/hwnode/DualFeatureNode.h>
#include <functional>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#define DUALCAM 1
#else
#define DUALCAM 0
#endif

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::EIS;
using namespace NSCam::FSC;

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
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
/******************************************************************************
*
*******************************************************************************/
PipelineBuilderFeaturePreview::
PipelineBuilderFeaturePreview(
    MINT32 const openId,
    char const* pipeName,
    ConfigParams const & rParam
)
    : PipelineBuilderBase(openId, pipeName, rParam)
{}

/******************************************************************************
*
*******************************************************************************/
PipelineBuilderFeaturePreview::
~PipelineBuilderFeaturePreview()
{}

/******************************************************************************
*
*******************************************************************************/
MVOID
PipelineBuilderFeaturePreview::
buildStream(
    sp<PipelineContext> rpPipelineContext,
    const PipelineImageParam& params
)
{
    CAM_TRACE_NAME("PB:[Prv]buildStream");

    mpLegacyPipeline->setImageStreamInfo(params.pInfo);

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
PipelineBuilderFeaturePreview::
configureP1Node(
    sp<PipelineContext> rpPipelineContext
)
{
    FUNC_START;

    CAM_TRACE_NAME("PB:[Prv]configureP1Node");

    Vector<PipelineImageParam> vParams;
    if ( !queryRawDst(vParams) ) {
        MY_LOGD("No need to configure P1 node.");
        return OK;
    }

    // Meta
    {
         CAM_TRACE_NAME("PB:[Prv]Meta");
         mpResultMeta_P1_App =
            new MetaStreamInfo(
                "App:Meta:ResultP1",
                eSTREAMID_META_APP_DYNAMIC_P1,
                eSTREAMTYPE_META_OUT,
                0
                );
         mpResultMeta_P1_Hal =
            new MetaStreamInfo(
                "Hal:Meta:ResultP1",
                eSTREAMID_META_HAL_DYNAMIC_P1,
                eSTREAMTYPE_META_INOUT,
                0
                );

        StreamBuilder(eStreamType_META_APP, mpResultMeta_P1_App)
            .build(rpPipelineContext);
        StreamBuilder(eStreamType_META_HAL, mpResultMeta_P1_Hal)
            .build(rpPipelineContext);

        mpLegacyPipeline->setMetaStreamInfo(mpResultMeta_P1_App);
        mpLegacyPipeline->setMetaStreamInfo(mpResultMeta_P1_Hal);
    }

    // Image
    {
        CAM_TRACE_NAME("PB:[Prv]Image");
        for ( size_t i = 0; i < vParams.size(); ++i ) {
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE ) {
                mpImage_ImgoRaw = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO ) {
                mpImage_LcsoRaw = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RSSO ) {
                mpImage_RssoRaw = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER ) {
                mpImage_RrzoRaw = vParams[i].pInfo;
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
        CAM_TRACE_NAME("PB:[Prv]Node");
        PipelineSensorParam aParam;
        if ( !querySrcParam(aParam) ) {
            MY_LOGD("No sensor setting.");
            return BAD_VALUE;
        }
        //
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
#endif
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta         = mpControlMeta_App;
            cfgParam.pInHalMeta         = mpControlMeta_Hal;
            cfgParam.pOutAppMeta        = mpResultMeta_P1_App;
            cfgParam.pOutHalMeta        = mpResultMeta_P1_Hal;
            cfgParam.pOutImage_resizer  = mpImage_RrzoRaw;
            cfgParam.pOutImage_lcso     = mpImage_LcsoRaw;
            cfgParam.pOutImage_rsso     = mpImage_RssoRaw;
            cfgParam.sensorParams       = sensorParam;
            cfgParam.rawProcessed       = aParam.rawType;
            {
                int debugProcRaw = property_get_int32("debug.camera.cfg.ProcRaw", -1);
                if(debugProcRaw >= 0)
                {
                    MY_LOGD("set debug.camera.ProcRaw(%d) => 0:config pure raw  1:config processed raw",debugProcRaw);
                    cfgParam.rawProcessed = debugProcRaw;
                }
            }
            cfgParam.enableEIS          = mConfigParams.enableEIS;
            cfgParam.enableLCS          = mConfigParams.enableLCS;
            cfgParam.enableRSS          = mConfigParams.enableRSS;
            cfgParam.enableFSC          = mConfigParams.enableFSC;
            cfgParam.enableDualPD          = mConfigParams.enableDualPD;
            cfgParam.disableFrontalBinning = mConfigParams.disableFrontalBinning;
            cfgParam.enableUNI          = mConfigParams.enableUNI;
            cfgParam.receiveMode = mConfigParams.receiveMode;
            cfgParam.packedEisInfo      = mConfigParams.packedEisInfo;
            cfgParam.disableHLR = mConfigParams.disableHLR;
            cfgParam.pipeBit = mConfigParams.pipeBit;
            cfgParam.initRequest = mConfigParams.initRequest;
            cfgParam.pResourceConcurrency = mConfigParams.p1ConfigConcurrency;
            cfgParam.cfgAppMeta = mConfigParams.cfgAppMeta;
            cfgParam.cfgHalMeta = mConfigParams.cfgHalMeta;
            update3AConfigMeta(&cfgParam.cfgAppMeta, &cfgParam.cfgHalMeta);
            if (mConfigParams.dualcamMode > 0)
            {
                MY_LOGD("Current is dual cam mode, set 2 TG and enable dynamic twin");
                cfgParam.tgNum = 2;
                cfgParam.disableDynamicTwin = MFALSE;
            }
            if ( mpImage_RrzoRaw != 0 ) {
                cfgParam.pStreamPool_resizer = NULL;
            }
            if ( mpImage_ImgoRaw != 0 ) {
                cfgParam.pvOutImage_full.push_back(mpImage_ImgoRaw);
                cfgParam.pStreamPool_full = NULL;
            }
            if ( mpImage_LcsoRaw != 0 ) {
                cfgParam.pStreamPool_lcso = NULL;
            }
            if ( mpImage_RssoRaw != 0 ) {
                cfgParam.pStreamPool_rsso = NULL;
            }

        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance());
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, mpControlMeta_App);
        add_stream_to_set(vIn, mpControlMeta_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, mpImage_ImgoRaw);
        add_stream_to_set(vOut, mpImage_RrzoRaw);
        add_stream_to_set(vOut, mpImage_LcsoRaw);
        add_stream_to_set(vOut, mpImage_RssoRaw);
        add_stream_to_set(vOut, mpResultMeta_P1_App);
        add_stream_to_set(vOut, mpResultMeta_P1_Hal);
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
        if ( mpImage_ImgoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_ImgoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_RrzoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_RrzoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_LcsoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_LcsoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_RssoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_RssoRaw->getStreamId(),
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

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
PipelineBuilderFeaturePreview::
configureP2FeatureNode(
    sp<PipelineContext> rpPipelineContext
)
{
    FUNC_START;

    CAM_TRACE_NAME("PB:[Prv]configureP2Node");

    Vector<PipelineImageParam> vParams;
    if ( !queryYuvDst(vParams) ) {
        MY_LOGD("No need to configure P2 node.");
        return OK;
    }

    // Meta
    {
         CAM_TRACE_NAME("PB:[Prv]Meta");
         mpResultMeta_P2_App =
            new MetaStreamInfo(
                "App:Meta:ResultP2",
                eSTREAMID_META_APP_DYNAMIC_P2,
                eSTREAMTYPE_META_OUT,
                0
                );
         mpResultMeta_P2_Hal =
            new MetaStreamInfo(
                "Hal:Meta:ResultP2",
                eSTREAMID_META_HAL_DYNAMIC_P2,
                eSTREAMTYPE_META_INOUT,
                0
                );

        StreamBuilder(eStreamType_META_APP, mpResultMeta_P2_App)
            .build(rpPipelineContext);
        StreamBuilder(eStreamType_META_HAL, mpResultMeta_P2_Hal)
            .build(rpPipelineContext);

        mpLegacyPipeline->setMetaStreamInfo(mpResultMeta_P2_App);
        mpLegacyPipeline->setMetaStreamInfo(mpResultMeta_P2_Hal);
    }

    // Image
    {
        CAM_TRACE_NAME("PB:[Prv]Image");
        for ( size_t i = 0; i < vParams.size(); ++i ) {
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_00 ) {
                mpImage_Yuv00 = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_01 ) {
                mpImage_Yuv01 = vParams[i].pInfo;
                buildStream(rpPipelineContext, vParams[i]);
                continue;
            }
            if ( vParams[i].pInfo->getStreamId() == eSTREAMID_IMAGE_YUV_FD ) {
                mpImage_YuvFd = vParams[i].pInfo;
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
        CAM_TRACE_NAME("PB:[Prv]Node");
        typedef P2FeatureNode           NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder p2 feature +");
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
            cfgParam.pInAppMeta  = mpControlMeta_App;
#ifdef FEATURE_MODIFY
            cfgParam.pInAppRetMeta = mpResultMeta_P1_App;
#endif // FEATURE_MODIFY
            cfgParam.pInHalMeta  = mpResultMeta_P1_Hal;
            cfgParam.pOutAppMeta = mpResultMeta_P2_App;
            cfgParam.pOutHalMeta = mpResultMeta_P2_Hal;
            if ( mpImage_ImgoRaw != 0 )
                cfgParam.pvInFullRaw.push_back(mpImage_ImgoRaw);
            cfgParam.pInResizedRaw = mpImage_RrzoRaw;
            cfgParam.pInLcsoRaw = mpImage_LcsoRaw;
            cfgParam.pInRssoRaw = mpImage_RssoRaw;
            if ( mpImage_Yuv00 != 0 )
                cfgParam.vOutImage.push_back(mpImage_Yuv00);
            if ( mpImage_Yuv01 != 0 )
                cfgParam.vOutImage.push_back(mpImage_Yuv01);
            cfgParam.pOutFDImage = mpImage_YuvFd;
            if(supportCZ)
                cfgParam.customOption |= v3::P2FeatureNode::CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT;
            if(supportDRE && !isVideoRecord())
                cfgParam.customOption |= v3::P2FeatureNode::CUSTOM_OPTION_DRE_SUPPORT;

        }
        //
#ifdef FEATURE_MODIFY
        P2FeatureNode::UsageHint usageHint;
        usageHint.mUsageMode = isVideoRecord() ? P2FeatureNode::USAGE_RECORD : P2FeatureNode::USAGE_PREVIEW;
        usageHint.mStreamingSize = mpImage_RrzoRaw->getImgSize();
        usageHint.mPreviewSize = mConfigParams.previewSize;
        usageHint.mVideoSize = mConfigParams.videoSize;
        usageHint.mEisInfo = EisInfo(mConfigParams.packedEisInfo);
        usageHint.mDualMode = (mConfigParams.dualcamMode < 0) ? 0 : (MUINT32)mConfigParams.dualcamMode;
        usageHint.pResourceConcurrency = mConfigParams.p2ConfigConcurrency;
        MY_LOGD("dual mode(%d)", usageHint.mDualMode);
        if (usageHint.mDualMode
            && mConfigParams.streamingSize.w != 0
            && mConfigParams.streamingSize.h != 0)
        {
            usageHint.mStreamingSize = mConfigParams.streamingSize;
            MY_LOGD("streamingSize: old size(%dx%d), new size(%dx%d)",
                mpImage_RrzoRaw->getImgSize().w, mpImage_RrzoRaw->getImgSize().h,
                mConfigParams.streamingSize.w, mConfigParams.streamingSize.h);
        }
        if (::property_get_int32("debug.camera.3dnr.level", 0))
            usageHint.m3DNRMode |= NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT;
#ifdef MTKCAM_SUPPORT_3DNR
            usageHint.m3DNRMode |= NSCam::NR3D::E3DNR_MODE_MASK_UI_SUPPORT;
#endif // MTKCAM_SUPPORT_3DNR
        if (::property_get_int32("vendor.debug.3dnr.sl2e.enable", 1)) // sl2e: default on
            usageHint.m3DNRMode |= NSCam::NR3D::E3DNR_MODE_MASK_SL2E_EN;
        if (mConfigParams.enableRSS && NR3DCustom::isSupportRSC())
            usageHint.m3DNRMode |= NSCam::NR3D::E3DNR_MODE_MASK_RSC_EN;
        if (mConfigParams.enableFSC)
        {
            usageHint.mFSCMode |= EFSC_MODE_MASK_FSC_EN;
            if (FSCCustom::getDebugLevel())
                usageHint.mFSCMode |= EFSC_MODE_MASK_DEBUG_LEVEL;
            if (::property_get_int32(FSC_SUBPIXEL_ENABLE_PROPERTY, 1)) // default on
                usageHint.mFSCMode |= EFSC_MODE_MASK_SUBPIXEL_EN;
        }
#endif // FEATURE_MODIFY
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance(P2FeatureNode::PASS2_STREAM, usageHint) );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, mpImage_ImgoRaw);
        add_stream_to_set(vIn, mpImage_RrzoRaw);
        add_stream_to_set(vIn, mpImage_LcsoRaw);
        add_stream_to_set(vIn, mpImage_RssoRaw);
        add_stream_to_set(vIn, mpControlMeta_App);
        add_stream_to_set(vIn, mpResultMeta_P1_Hal);

#ifdef FEATURE_MODIFY
        add_stream_to_set(vIn, mpResultMeta_P1_App);
#endif // FEATURE_MODIFY
        //
        StreamSet vOut;
        add_stream_to_set(vOut, mpImage_Yuv00);
        add_stream_to_set(vOut, mpImage_Yuv01);
        add_stream_to_set(vOut, mpImage_YuvFd);
        add_stream_to_set(vOut, mpResultMeta_P2_App);
        add_stream_to_set(vOut, mpResultMeta_P2_Hal);
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
        if ( mpImage_ImgoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_ImgoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_RrzoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_RrzoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_LcsoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_LcsoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_RssoRaw != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_RssoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_Yuv00 != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Yuv00->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_Yuv01 != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Yuv01->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_YuvFd != 0 )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_YuvFd->getStreamId(),
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

    return OK;
}
/******************************************************************************
*
*******************************************************************************/
MERROR
PipelineBuilderFeaturePreview::
configureDualFeatureNode(
    sp<PipelineContext> rpPipelineContext
)
{
    FUNC_START;
    CAM_TRACE_NAME("PB:[Prv]configureDualFeatureNode");
    MERROR ret = BAD_VALUE;
#if DUALCAM
    //
    Vector<PipelineImageParam> vParams;
    queryYuvDst(vParams);
    MY_LOGD("[%d] output yuv size(%d)", getOpenId(), vParams.size());
    // Meta
    {
        CAM_TRACE_NAME("PB:[Prv]DualMeta");
        mpResultMeta_P2_App =
            new MetaStreamInfo(
            "App:Meta:ResultP2",
            eSTREAMID_META_APP_DYNAMIC_P2,
            eSTREAMTYPE_META_OUT,
            0);
        mpResultMeta_P2_Hal =
            new MetaStreamInfo(
            "Hal:Meta:ResultP2",
            eSTREAMID_META_HAL_DYNAMIC_P2,
            eSTREAMTYPE_META_INOUT,
            0);
        StreamBuilder(eStreamType_META_APP, mpResultMeta_P2_App).build(rpPipelineContext);
        StreamBuilder(eStreamType_META_HAL, mpResultMeta_P2_Hal).build(rpPipelineContext);

        mpLegacyPipeline->setMetaStreamInfo(mpResultMeta_P2_App);
        mpLegacyPipeline->setMetaStreamInfo(mpResultMeta_P2_Hal);
    }
    // Image
    {
        CAM_TRACE_NAME("PB:[Prv]Image");
        for(auto params : vParams)
        {
            if(params.pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_00)
            {
                mpImage_Yuv00 = params.pInfo;
                buildStream(rpPipelineContext, params);
                continue;
            }
            if(params.pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_YUV_01)
            {
                mpImage_Yuv01 = params.pInfo;
                buildStream(rpPipelineContext, params);
                continue;
            }
            if(params.pInfo->getStreamId() == eSTREAMID_IMAGE_YUV_FD)
            {
                mpImage_YuvFd = params.pInfo;
                buildStream(rpPipelineContext, params);
                continue;
            }
            // for dual cam bokeh, it has to more allocate two working buffer
            // one for clean image, one for blur map
            if(params.pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV)
            {
                mpImage_Yuv02 = params.pInfo;
                buildStream(rpPipelineContext, params);
                continue;
            }
            if(params.pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV)
            {
                mpImage_Y8_01 = params.pInfo;
                buildStream(rpPipelineContext, params);
                continue;
            }
            else if(params.pInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV)
            {
                mpImage_Y8_01 = params.pInfo;
                buildStream(rpPipelineContext, params);
                continue;
            }
            //
            MY_LOGW("not supported stream %#" PRIx64 "(%s)",
                    params.pInfo->getStreamId(),
                    params.pInfo->getStreamName());
            return BAD_VALUE;
        }
    }
    // Config dual feature node
    {
        CAM_TRACE_NAME("PB:[Prv]Node");
        typedef DualFeatureNode         NodeT;
        typedef NodeActor< NodeT >      NodeActorT;
        //
        MY_LOGD("Nodebuilder dual feature +");
        NodeT::InitParams initParam;
        {
            initParam.openId = getOpenId();
            initParam.nodeId = eNODEID_P2Node;
            initParam.nodeName = "DualFeatureNode";
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta = mpControlMeta_App;
            cfgParam.pInAppRetMeta = mpResultMeta_P1_App;
            cfgParam.pInHalMeta  = mpResultMeta_P1_Hal;
            cfgParam.pOutAppMeta = mpResultMeta_P2_App;
            cfgParam.pOutHalMeta = mpResultMeta_P2_Hal;
            if ( mpImage_ImgoRaw != 0 )
            {
                cfgParam.pvInFullRaw.push_back(mpImage_ImgoRaw);
            }
            cfgParam.pInResizedRaw = mpImage_RrzoRaw;
            cfgParam.pInLcsoRaw = mpImage_LcsoRaw;
            //cfgParam.pInRssoRaw = mpImage_RssoRaw;
            if ( mpImage_Yuv00 != 0 )
                cfgParam.vOutImage.push_back(mpImage_Yuv00);
            if ( mpImage_Yuv01 != 0 )
                cfgParam.vOutImage.push_back(mpImage_Yuv01);
            if ( mpImage_Yuv02 != 0 )
                cfgParam.vOutImage.push_back(mpImage_Yuv02);
            if ( mpImage_Y8_01 != 0 )
                cfgParam.vOutImage.push_back(mpImage_Y8_01);
            cfgParam.pOutFDImage = mpImage_YuvFd;
            // set dual cam mode
            cfgParam.iDualCamMode = mConfigParams.dualcamMode;
            // pass rrzo size to dual feature effecthal (for information sync)
            cfgParam.rrzoSize = mpImage_RrzoRaw->getImgSize();
        }
        //
        sp<NodeActorT> pNode = new NodeActorT(NodeT::createInstance());
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, mpImage_ImgoRaw);
        add_stream_to_set(vIn, mpImage_RrzoRaw);
        add_stream_to_set(vIn, mpImage_LcsoRaw);
        add_stream_to_set(vIn, mpImage_RssoRaw);
        add_stream_to_set(vIn, mpControlMeta_App);
        add_stream_to_set(vIn, mpResultMeta_P1_Hal);
        add_stream_to_set(vIn, mpResultMeta_P1_App);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, mpImage_Yuv00);
        add_stream_to_set(vOut, mpImage_Yuv01);
        add_stream_to_set(vOut, mpImage_YuvFd);
        add_stream_to_set(vOut, mpImage_Yuv02);
        add_stream_to_set(vOut, mpImage_Y8_01);
        add_stream_to_set(vOut, mpResultMeta_P2_App);
        add_stream_to_set(vOut, mpResultMeta_P2_Hal);
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
        if (mpImage_ImgoRaw != nullptr)
            aNodeBuilder.setImageStreamUsage(
                    mpImage_ImgoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_RrzoRaw != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_RrzoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_LcsoRaw != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_LcsoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_RssoRaw != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_RssoRaw->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ
                    );
        if ( mpImage_Yuv00 != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Yuv00->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_Yuv01 != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Yuv01->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_Yuv02 != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Yuv02->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_Y8_01 != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Y8_01->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN
                    );
        if ( mpImage_YuvFd != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_YuvFd->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE
                    );
        if ( mpImage_Yuv02 != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Yuv02->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE
                    );
        if ( mpImage_Y8_01 != nullptr )
            aNodeBuilder.setImageStreamUsage(
                    mpImage_Y8_01->getStreamId(),
                    eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN
                    );

        ret = aNodeBuilder.build(rpPipelineContext);
        MY_LOGD("Nodebuilder dual feature -");

        if( ret != OK ) {
            MY_LOGE("build Dual feature node error");
            return ret;
        }
    }
#endif // DUALCAM
    ret = OK;
lbExit:
    FUNC_END;
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
sp<ILegacyPipeline>
PipelineBuilderFeaturePreview::
create()
{
    FUNC_START;

    CAM_TRACE_NAME("PB:[Prv]create");

    mpLegacyPipeline = new LegacyPipelinePreview(getOpenId());
    sp<PipelineContext> pPipelineContext;

    pPipelineContext = PipelineContext::create( getName() );
    if( !pPipelineContext.get() ) {
        MY_LOGE("cannot create context");
        return NULL;
    }
    //
    CAM_TRACE_BEGIN("PB:[Prv]beginConfig");
    int ret = pPipelineContext->beginConfigure();
    if( ret != OK )
    {
        MY_LOGE("build beginConfigure error");
        return NULL;
    }
    CAM_TRACE_END();
    //
    {
        PipelineSensorParam aParam;
        if ( !querySrcParam(aParam) ) {
            MY_LOGE("No sensor setting.");
            return NULL;
        }
        //
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
        //
        sp<IDispatcher> pDispatcher;
        if( queryDispatcher(pDispatcher) )
        {
            pPipelineContext->setDispatcher(pDispatcher);
        }
    }
    // 1. Control Meta   ***************
    //
    //
    {
        CAM_TRACE_NAME("PB:[Prv]ControlMeta");
        mpControlMeta_App =
            new MetaStreamInfo(
                "App:Meta:Control",
                eSTREAMID_META_APP_CONTROL,
                eSTREAMTYPE_META_IN,
                0
                );
        mpControlMeta_Hal =
            new MetaStreamInfo(
                "Hal:Meta:Control",
                eSTREAMID_META_HAL_CONTROL,
                eSTREAMTYPE_META_IN,
                0
                );
        // Meta
        StreamBuilder(eStreamType_META_APP, mpControlMeta_App)
            .build(pPipelineContext);
        StreamBuilder(eStreamType_META_HAL, mpControlMeta_Hal)
            .build(pPipelineContext);

        mpLegacyPipeline->setMetaStreamInfo(mpControlMeta_App);
        mpLegacyPipeline->setMetaStreamInfo(mpControlMeta_Hal);
    }
    //
    // 2. Nodes   ***************
    //
    //
    {
#if DUALCAM
        auto p2Function = std::bind(&PipelineBuilderFeaturePreview::configureP2FeatureNode, this, std::placeholders::_1);
        MINT32 featureMode = StereoSettingProvider::getStereoFeatureMode();
        if(featureMode == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
           || featureMode == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
        {
            p2Function = std::bind(&PipelineBuilderFeaturePreview::configureDualFeatureNode, this, std::placeholders::_1);
        }
        if(
                OK != configureP1Node(pPipelineContext) ||
                OK != p2Function(pPipelineContext)
          )
        {
            MY_LOGE("node config failed");
            return NULL;
        }
#else
        if(
                OK != configureP1Node(pPipelineContext) ||
                OK != configureP2FeatureNode(pPipelineContext)
          )
        {
            MY_LOGE("node config failed");
            return NULL;
        }
#endif
    }
    //
    // 3. Pipeline **************
    //
    {
        CAM_TRACE_NAME("PB:[Prv]PipelineBuilder");
        MERROR ret = PipelineBuilder()
            .setRootNode(
                    NodeSet().add(eNODEID_P1Node)
                    )
            .setNodeEdges(
                    NodeEdgeSet()
                    .addEdge(eNODEID_P1Node, eNODEID_P2Node)
                    )
            .build(pPipelineContext);
        if( ret != OK ) {
            MY_LOGE("build pipeline error");
            return NULL;
        }
    }
    //
    CAM_TRACE_BEGIN("PB:[Prv]endConfig");
    sp<ResultProcessor> pResultProcessor = ResultProcessor::createInstance();
    pPipelineContext->setDataCallback(pResultProcessor);
    ret = pPipelineContext->endConfigure(true);
    if( ret != OK )
    {
        MY_LOGE("build endConfigure error");
        return NULL;
    }
    CAM_TRACE_END();
    //
    // 4. Request Builder **************
    //
    CAM_TRACE_BEGIN("PB:[Prv]RequestBuilder");
    IOMap p1_Image_IOMap;
    IOMap p2_Image_IOMap;
    {
        if ( mpImage_ImgoRaw != 0 && isImageRawOutput() ) {
            p1_Image_IOMap.addOut( mpImage_ImgoRaw->getStreamId() );
#if MTKCAM_HAVE_VSDOF_SUPPORT
            MINT32 bForceImgoStream = property_get_int32("debug.camera.ffdump", 0);
            MY_LOGD("bForceImgoStream(%d)", bForceImgoStream);
            if(bForceImgoStream)
                p2_Image_IOMap.addIn( mpImage_ImgoRaw->getStreamId() );
#endif // MTKCAM_HAVE_VSDOF_SUPPORT
        }
        if ( mpImage_RrzoRaw != 0 ){
            p1_Image_IOMap.addOut( mpImage_RrzoRaw->getStreamId() );
            p2_Image_IOMap.addIn( mpImage_RrzoRaw->getStreamId() );
        }
        if ( mpImage_LcsoRaw != 0 ){
            p1_Image_IOMap.addOut( mpImage_LcsoRaw->getStreamId() );
            p2_Image_IOMap.addIn( mpImage_LcsoRaw->getStreamId() );
        }
        if ( mpImage_RssoRaw != 0 ){
            p1_Image_IOMap.addOut( mpImage_RssoRaw->getStreamId() );
            p2_Image_IOMap.addIn( mpImage_RssoRaw->getStreamId() );
        }
        if ( mpImage_Yuv00 != 0 )
            p2_Image_IOMap.addOut( mpImage_Yuv00->getStreamId() );
        if ( mpImage_Yuv01 != 0 )
            p2_Image_IOMap.addOut( mpImage_Yuv01->getStreamId() );
        if ( mpImage_YuvFd != 0 )
            p2_Image_IOMap.addOut( mpImage_YuvFd->getStreamId() );
        if ( mpImage_Yuv02 != 0 )
            p2_Image_IOMap.addOut( mpImage_Yuv02->getStreamId() );
        if ( mpImage_Y8_01 != 0 )
            p2_Image_IOMap.addOut( mpImage_Y8_01->getStreamId() );
    }

    sp<RequestBuilder> pRequestBuilder = new RequestBuilder();
    pRequestBuilder->setIOMap(
                    eNODEID_P1Node,
                    IOMapSet().add(
                        p1_Image_IOMap
                        ),
                    IOMapSet().add(
                        IOMap()
                        .addIn(mpControlMeta_App->getStreamId())
                        .addIn(mpControlMeta_Hal->getStreamId())
                        .addOut(mpResultMeta_P1_App->getStreamId())
                        .addOut(mpResultMeta_P1_Hal->getStreamId())
                        )
                    );
    pRequestBuilder->setIOMap(
                    eNODEID_P2Node,
                    IOMapSet().add(
                        p2_Image_IOMap
                        ),
                    IOMapSet().add(
                        IOMap()
                        .addIn(mpControlMeta_App->getStreamId())
#ifdef FEATURE_MODIFY
                        .addIn(mpResultMeta_P1_App->getStreamId())
#endif // FEATURE_MODIFY
                        .addIn(mpResultMeta_P1_Hal->getStreamId())
                        .addOut(mpResultMeta_P2_App->getStreamId())
                        .addOut(mpResultMeta_P2_Hal->getStreamId())
                        )
                    );
    pRequestBuilder->setRootNode(
                    NodeSet().add(eNODEID_P1Node)
                    );
    pRequestBuilder->setNodeEdges(
                    NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
                    );
    pRequestBuilder->updateFrameCallback(pResultProcessor);
    CAM_TRACE_END();
    //
    // set meta iomap
    mpLegacyPipeline->setMetaIOMap(eNODEID_P1Node,
                IOMapSet().add(
                    IOMap()
                    .addIn(mpControlMeta_App->getStreamId())
                    .addIn(mpControlMeta_Hal->getStreamId())
                    .addOut(mpResultMeta_P1_App->getStreamId())
                    .addOut(mpResultMeta_P1_Hal->getStreamId())
                    )
                );
    mpLegacyPipeline->setMetaIOMap(eNODEID_P2Node,
                IOMapSet().add(
                    IOMap()
                    .addIn(mpControlMeta_App->getStreamId())
#ifdef FEATURE_MODIFY
                    .addIn(mpResultMeta_P1_App->getStreamId())
#endif // FEATURE_MODIFY
                    .addIn(mpResultMeta_P1_Hal->getStreamId())
                    .addOut(mpResultMeta_P2_App->getStreamId())
                    .addOut(mpResultMeta_P2_Hal->getStreamId())
                    )
                );
    //
    //
    //
    // 4. Register Listener **************
    //
    CAM_TRACE_BEGIN("PB:[Prv]RegisterListener");
    sp<TimestampProcessor> pTimestampProcessor = TimestampProcessor::createInstance(getOpenId());

    #if MTK_CAM_DISPAY_FRAME_CONTROL_ON
    if(mpResultMeta_P1_Hal !=0)
    {
        pResultProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P1,
                            pTimestampProcessor);
    }
    #else
    if( mpResultMeta_P1_App != 0 )
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
                MY_LOGD("Raw buffer provider size:%zu",vParams.size() );
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
                MY_LOGD("Yuv buffer provider size:%zu",vParams.size() );
                for ( size_t i = 0; i < vParams.size(); ++i ) {
                    if ( vParams[i].pProvider != 0 ) {
                        MY_LOGD("Yuv buffer provider StreamId:%#" PRIx64 ,vParams[i].pInfo->getStreamId() );
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
    CAM_TRACE_BEGIN("PB:[Prv]LegacyPipeline");
    mpLegacyPipeline->setPipelineContext(pPipelineContext);
    mpLegacyPipeline->setRequestBuilder(pRequestBuilder);
    mpLegacyPipeline->setResultProcessor(pResultProcessor);
    mpLegacyPipeline->setTimestampProcessor(pTimestampProcessor);
    CAM_TRACE_END();
    MY_LOGD("Legacy Pipeline -");

    FUNC_END;

    return mpLegacyPipeline;
}

/******************************************************************************
*
*******************************************************************************/

MBOOL
PipelineBuilderFeaturePreview::
isImageRawOutput(void)
{
    MBOOL ret = MTRUE;
    if( getMode() == LegacyPipelineMode_T::PipelineMode_VideoRecord ||
        getMode() == LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord )
    {
        int enableImgo = property_get_int32("rawDump.imgo.enable", 0);
        if(enableImgo==0)
            ret = MFALSE;
        else
            ret = MTRUE;
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
#ifdef FEATURE_MODIFY
MBOOL
PipelineBuilderFeaturePreview::
isVideoRecord() const
{
    switch( getMode() )
    {
    case LegacyPipelineMode_T::PipelineMode_VideoRecord:
    case LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord:
        return MTRUE;
    default:
        return MFALSE;
    }
}
#endif // FEATURE_MODIFY
