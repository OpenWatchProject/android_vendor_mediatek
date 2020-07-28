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

#define LOG_TAG "mtkcam-PipelineModelSessionStreaming"
//
#include "PipelineModelSessionStreaming.h"
//
// for scenario control
#include <mtkcam/drv/IHalSensor.h>
//
#include <impl/ControlMetaBufferGenerator.h>
#include <impl/PipelineContextBuilder.h>
#include <impl/PipelineFrameBuilder.h>
//
#include "MyUtils.h"
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/hwnode/P1Node.h>
#include <mtkcam3/pipeline/hwnode/P2StreamingNode.h>
#include <mtkcam3/pipeline/hwnode/JpegNode.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
//
#include <impl/ControlMetaBufferGenerator.h>
#include <impl/PipelineContextBuilder.h>
#include <impl/PipelineFrameBuilder.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAM_PIPELINE_MODEL);



/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCamHW;
using namespace NSCam::v3::Utils;

#define ThisNamespace   PipelineModelSessionStreaming


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
makeInstance(
    std::string const& name,
    CtorParams const& rCtorParams __unused
) -> android::sp<IPipelineModelSession>
{
    android::sp<ThisNamespace> pSession = new ThisNamespace(name, rCtorParams);
    if  ( CC_UNLIKELY(pSession==nullptr) ) {
        CAM_ULOGME("[%s] Bad pSession", __FUNCTION__);
        return nullptr;
    }

    int const err = pSession->configure();
    if  ( CC_UNLIKELY(err != 0) ) {
        CAM_ULOGME("[%s] err:%d(%s) - Fail on configure()", __FUNCTION__, err, ::strerror(-err));
        return nullptr;
    }

    return pSession;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace(
    std::string const& name,
    CtorParams const& rCtorParams)
    : PipelineModelSessionDefault(name, rCtorParams)
{
    mConfigInfo2 = nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~ThisNamespace()
{
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
processReconfiguration(
    pipelinesetting::RequestOutputParams const& rcfOutputParam __unused,
    std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
    MUINT32 requestNo
) -> int
{
    if(!rcfOutputParam.needReconfiguration)
    {
        return OK;
    }
    else if(rcfOutputParam.reconfigCategory == ReCfgCtg::STREAMING)
    {
        //reconfigCategory is Stream
        if(processReconfigStream(rcfOutputParam, pConfigInfo2, requestNo) != OK)
        {
            MY_LOGE("reconfigCategory(%hhu): processReconfigStream Error", rcfOutputParam.reconfigCategory);
            return BAD_VALUE;
        }
        return OK;
    }
    else if(rcfOutputParam.reconfigCategory == ReCfgCtg::CAPTURE)
    {
        //reconfigCategory is Capture
        return OK;
    }

    return BAD_VALUE;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
beginFlush() -> int
{
    std::lock_guard<std::mutex> _l(mMutexLock_Context);
    return PipelineModelSessionDefault::beginFlush();
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
processReconfigStream(
    pipelinesetting::RequestOutputParams const& rcfOutputParam __unused,
    std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
    MUINT32 requestNo
) -> int
{
    MY_LOGD("requestNo(%d) processReconfigStream +", requestNo);
    std::lock_guard<std::mutex> _l(mMutexLock_Context);

    MBOOL ret = INVALID_OPERATION;
    ret = waitUntilP1NodeDrainedAndFlush();
    if (ret!=OK)
    {
        CAM_ULOGME("waitUntilP1NodeDrainedAFlush Fail!");
        return ret;
    }

    ret = waitUntilP2DrainedAndFlush();
    if (ret!=OK)
    {
        CAM_ULOGME("waitUntilP2DrainedAndFlush Fail!");
        return ret;
    }
    mCurrentPipelineContext = NULL;

    pConfigInfo2 = std::make_shared<ConfigInfo2>();

    RETURN_ERROR_IF_NULLPTR(pConfigInfo2, -ENODEV, "Fail on make_shared<ConfigInfo2>");
    pipelinesetting::ConfigurationOutputParams rcfOutParam{
        .pStreamingFeatureSetting   = &pConfigInfo2->mStreamingFeatureSetting,
        .pCaptureFeatureSetting     = &pConfigInfo2->mCaptureFeatureSetting,
        .pPipelineNodesNeed         = &pConfigInfo2->mPipelineNodesNeed,
        .pPipelineTopology          = &pConfigInfo2->mPipelineTopology,
        .pSensorSetting             = &pConfigInfo2->mvSensorSetting,
        .pP1HwSetting               = &pConfigInfo2->mvP1HwSetting,
        .pP1DmaNeed                 = &pConfigInfo2->mvP1DmaNeed,
        .pParsedStreamInfo_P1       = &pConfigInfo2->mvParsedStreamInfo_P1,
        .pParsedStreamInfo_NonP1    = &pConfigInfo2->mParsedStreamInfo_NonP1,
        .pIsZSLMode                 = &pConfigInfo2->mIsZSLMode,
    };

    RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateConfiguration(rcfOutParam, {}), "Fail on Pipeline Reconfiguration" );

    // configure scenario control
    configureDVFSControl(pConfigInfo2->mStreamingFeatureSetting.BWCScenario, pConfigInfo2->mStreamingFeatureSetting.BWCFeatureFlag);

    // create capture related instances, MUST be after FeatureSettingPolicy
    auto pImgoStreamInfo = pConfigInfo2->mvParsedStreamInfo_P1[0].pHalImage_P1_Imgo.get();
    configureCaptureInFlight(pConfigInfo2->mCaptureFeatureSetting.maxAppJpegStreamNum, pImgoStreamInfo ? pImgoStreamInfo->getMaxBufNum(): 0);

    BuildPipelineContextInputParams const in{
        .pipelineName               = getSessionName(),
        .pPipelineStaticInfo        = mStaticInfo.pPipelineStaticInfo.get(),
        .pPipelineUserConfiguration = mStaticInfo.pUserConfiguration.get(),
        .pPipelineUserConfiguration2= mStaticInfo.pUserConfiguration2.get(),
        .pParsedStreamInfo_NonP1    = &pConfigInfo2->mParsedStreamInfo_NonP1,
        .pParsedStreamInfo_P1       = &pConfigInfo2->mvParsedStreamInfo_P1,
        .bIsZslEnabled              = mpZslProcessor.get() ? true : false,
        .pSensorSetting             = &pConfigInfo2->mvSensorSetting,
        .pvP1HwSetting              = &pConfigInfo2->mvP1HwSetting,
        .pPipelineNodesNeed         = &pConfigInfo2->mPipelineNodesNeed,
        .pPipelineTopology          = &pConfigInfo2->mPipelineTopology,
        .pStreamingFeatureSetting   = &pConfigInfo2->mStreamingFeatureSetting,
        .pCaptureFeatureSetting     = &mConfigInfo2->mCaptureFeatureSetting,
        .batchSize                  = 0,
        .pOldPipelineContext        = nullptr,
        .pDataCallback              = this,
        .bUsingParallelNodeToBuildPipelineContext = mbUsingParallelNodeToBuildPipelineContext,
        .bUsingMultiThreadToBuildPipelineContext = false,
        .bIsReconfigure             = true,
    };

    MY_LOGD("mpZslProcessor(%p)", mpZslProcessor.get());
    // Create New pipeline context for streaming.
    RETURN_ERROR_IF_NOT_OK( buildPipelineContext(mCurrentPipelineContext, in), "Fail on buildPipelineContext" );
    mCurrentPipelineContext->setSessionLifeHolder(mpSessionLifeHolder);
    MY_LOGD("mpSessionLifeHolder:%p ", mpSessionLifeHolder.get());

    MY_LOGD("requestNo(%d) processReconfigStream -", requestNo);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilP1NodeDrainedAndFlush() -> MERROR
{
    MERROR err = OK;
    auto pPipelineContext = getCurrentPipelineContext();
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP1nodeDrainedAndFlush");
    //P1
    {
        err = waitUntilNodeDrainedAndFlush(eNODEID_P1Node);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P1Node);
            return err;
        }
    }
    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilP2DrainedAndFlush() -> MERROR
{
    MERROR err = OK;
    auto pPipelineContext = getCurrentPipelineContext();
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP2DrainedAndFlush");
    //P2
    {
        err = waitUntilNodeDrainedAndFlush(eNODEID_P2StreamNode);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P2StreamNode);
            return err;
        }

        android::sp<INodeActor> pNode = pPipelineContext->queryINodeActor(eNODEID_P2StreamNode);
        if ( pNode != nullptr ) {
            pNode->uninit(); // must uninit old P2 before call new P2 config
        }
    }

    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
ThisNamespace::
waitUntilNodeDrainedAndFlush(
    NodeId_T const nodeId
)
{
    auto pPipelineContext = getCurrentPipelineContext();
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }

    MERROR err = OK;
    auto pNodeActor = pPipelineContext->queryINodeActor(nodeId);
    {
        if( err != OK || pNodeActor == NULL )
        {
            MY_LOGW("get NodeActor(%" PRIdPTR ") fail", nodeId);
            return err;
        }
        //
        err = pPipelineContext->waitUntilNodeDrained(nodeId);
        if( err != OK )
        {
            MY_LOGW("wait until node(%" PRIdPTR ") drained fail", nodeId);
            return err;
        }
        //
        IPipelineNode* node = pNodeActor->getNode();
        if( node == NULL )
        {
            MY_LOGW("get node(%" PRIdPTR ") fail", nodeId);
            return UNKNOWN_ERROR;
        }
        //
        err = node->flush();
        if( err != OK )
        {
            MY_LOGW("flush node(%" PRIdPTR ") fail", nodeId);
            return err;
        }
    }
    return OK;
}

