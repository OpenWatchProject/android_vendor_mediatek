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

#define LOG_TAG "mtkcam-PipelineModelSessionSMVR"
//
#include "PipelineModelSessionSMVR.h"
//
// for scenario control
#include <mtkcam/drv/IHalSensor.h>
//
#include <impl/ControlMetaBufferGenerator.h>
#include <impl/PipelineContextBuilder.h>
#include <impl/PipelineFrameBuilder.h>
#include <impl/AppRequestParser.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_SMVR_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::Utils::ULog;
using namespace NSCam::v3::pipeline::prerelease;

#define ThisNamespace   PipelineModelSessionSMVR

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
makeInstance(
    CtorParams const& rCtorParams
) -> android::sp<IPipelineModelSession>
{
    android::sp<ThisNamespace> pSession = new ThisNamespace(rCtorParams);
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
ThisNamespace(CtorParams const& rCtorParams)
    : PipelineModelSessionBase(
        {"SMVR/" + std::to_string(rCtorParams.staticInfo.pPipelineStaticInfo->openId)},
        rCtorParams)
{
        mAeTargetFpsMin   = 0;
        mAeTargetFpsMax   = 0;
        mDefaultBatchSize = 1;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getCurrentPipelineContext() const -> android::sp<IPipelineContextT>
{
    android::RWLock::AutoRLock _l(mRWLock_PipelineContext);
    return mCurrentPipelineContext;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
configure() -> int
{
    auto const& pPipelineStaticInfo         = mStaticInfo.pPipelineStaticInfo;
    auto const& pUserConfiguration          = mStaticInfo.pUserConfiguration;
    auto const& pParsedAppImageStreamInfo   = pUserConfiguration->pParsedAppImageStreamInfo;
    // Allocate mConfigInfo2
    mConfigInfo2 = std::make_shared<ConfigInfo2>();
    RETURN_ERROR_IF_NULLPTR(mConfigInfo2, -ENODEV, "Fail on make_shared<ConfigInfo2>");

    // Initialize mConfigInfo2
    {
        pipelinesetting::ConfigurationOutputParams out{
            .pStreamingFeatureSetting   = &mConfigInfo2->mStreamingFeatureSetting,
            .pCaptureFeatureSetting     = &mConfigInfo2->mCaptureFeatureSetting,
            .pPipelineNodesNeed         = &mConfigInfo2->mPipelineNodesNeed,
            .pPipelineTopology          = &mConfigInfo2->mPipelineTopology,
            .pSensorSetting             = &mConfigInfo2->mvSensorSetting,
            .pP1HwSetting               = &mConfigInfo2->mvP1HwSetting,
            .pP1DmaNeed                 = &mConfigInfo2->mvP1DmaNeed,
            .pParsedStreamInfo_P1       = &mConfigInfo2->mvParsedStreamInfo_P1,
            .pParsedStreamInfo_NonP1    = &mConfigInfo2->mParsedStreamInfo_NonP1,
            .pIsZSLMode                 = nullptr,
        };
        RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateConfiguration(out, {}), "Fail on evaluateConfiguration" );
    }

    // App Image Max. Buffer Number
    RETURN_ERROR_IF_NOT_OK(
        mPipelineSettingPolicy->decideConfiguredAppImageStreamMaxBufNum(
            pUserConfiguration->pParsedAppImageStreamInfo.get(),
            &mConfigInfo2->mStreamingFeatureSetting,
            &mConfigInfo2->mCaptureFeatureSetting
        ), "Fail on decideConfiguredAppImageStreamMaxBufNum"
    );

    // configure scenario control
    configureDVFSControl();

    // get batchSize
    android::sp<IMetadataProvider const>
    pMetadataProvider = NSMetadataProviderManager::valueFor(mStaticInfo.pPipelineStaticInfo->openId);
    IMetadata::IEntry const& entry = pMetadataProvider->getMtkStaticCharacteristics()
                                     .entryFor(MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS);
    if  ( entry.isEmpty() ) {
        MY_LOGW("SMVRConstraint: no static MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS");
    }
    else {
        // [width, height, fps_min, fps_max, batch_size]
        MBOOL hit = MFALSE;
        for ( size_t i=0; i<entry.count(); i+=5 )
        {
            MINT32 width      = entry.itemAt(i, Type2Type<MINT32>());
            MINT32 height     = entry.itemAt(i+1, Type2Type<MINT32>());
            MINT32 fps_min    = entry.itemAt(i+2, Type2Type<MINT32>());
            MINT32 fps_max    = entry.itemAt(i+3, Type2Type<MINT32>());
            MINT32 batch_size = entry.itemAt(i+4, Type2Type<MINT32>());

            MY_LOGI("SMVRConstraint: [width(%d), height(%d), fps_min(%d), fps_max(%d), batch_size(%d)]",
                width, height, fps_min, fps_max, batch_size);

            if ( ( pParsedAppImageStreamInfo->videoImageSize.w == width &&
                   pParsedAppImageStreamInfo->videoImageSize.h == height)
                 ||
                 ( pParsedAppImageStreamInfo->maxYuvSize.w == width &&
                 pParsedAppImageStreamInfo->maxYuvSize.h == height)
               )
            {
                mAeTargetFpsMin   = fps_min;
                mAeTargetFpsMax   = fps_max;
                mDefaultBatchSize = batch_size;
                hit = MTRUE;
                break;
            }
        }

        if ( !hit )
        {
            MY_LOGE("SMVRConstraint: no matching high speed profile: videoSize(%dx%d), maxYuvSize(%dx%d)",
                pParsedAppImageStreamInfo->videoImageSize.w, pParsedAppImageStreamInfo->videoImageSize.h,
                pParsedAppImageStreamInfo->maxYuvSize.w, pParsedAppImageStreamInfo->maxYuvSize.h
            );
        }
        else if (pParsedAppImageStreamInfo->videoImageSize.w == 0 || pParsedAppImageStreamInfo->videoImageSize.h == 0)
        {
            MY_LOGW("SMVRConstraint: matching high speed profile: videoSize(%dx%d), maxYuvSize(%dx%d)",
                pParsedAppImageStreamInfo->videoImageSize.w, pParsedAppImageStreamInfo->videoImageSize.h,
                pParsedAppImageStreamInfo->maxYuvSize.w, pParsedAppImageStreamInfo->maxYuvSize.h
            );
        }
    }

    // build pipeline context
    {
        BuildPipelineContextInputParams const in{
            .pipelineName               = getSessionName(),
            .pPipelineStaticInfo        = pPipelineStaticInfo.get(),
            .pPipelineUserConfiguration = pUserConfiguration.get(),
            .pPipelineUserConfiguration2= mStaticInfo.pUserConfiguration2.get(),
            .pParsedStreamInfo_NonP1    = &mConfigInfo2->mParsedStreamInfo_NonP1,
            .pParsedStreamInfo_P1       = &mConfigInfo2->mvParsedStreamInfo_P1,
            .pSensorSetting             = &mConfigInfo2->mvSensorSetting,
            .pvP1HwSetting              = &mConfigInfo2->mvP1HwSetting,
            .pPipelineNodesNeed         = &mConfigInfo2->mPipelineNodesNeed,
            .pPipelineTopology          = &mConfigInfo2->mPipelineTopology,
            .pStreamingFeatureSetting   = &mConfigInfo2->mStreamingFeatureSetting,
            .pCaptureFeatureSetting     = &mConfigInfo2->mCaptureFeatureSetting,
            .batchSize                  = mDefaultBatchSize,
            .pOldPipelineContext        = nullptr,
            .pDataCallback              = nullptr,
            .bUsingParallelNodeToBuildPipelineContext = true,
            .bUsingMultiThreadToBuildPipelineContext = false,
            .bIsReconfigure             = false,
        };
        RETURN_ERROR_IF_NOT_OK( buildPipelineContext(mCurrentPipelineContext, in), "Fail on buildPipelineContext" );
        mCurrentPipelineContext->setSessionLifeHolder(mpSessionLifeHolder);
        MY_LOGD("mpSessionLifeHolder:%p ", mpSessionLifeHolder.get());
    }

    ////////////////////////////////////////////////////////////////////////////

    // Initialize the current sensor settings.
    for (auto const& v : mConfigInfo2->mvSensorSetting) {
        mSensorMode.push_back(v.sensorMode);
        mSensorSize.push_back(v.sensorSize);
    }

    return OK;
}


/******************************************************************************
 * TODO: Need to be optimized
 ******************************************************************************/
auto
ThisNamespace::
submitOneRequest(
    std::shared_ptr<ParsedAppRequest>const& request
) -> int
{
    // Make a copy of the original App Meta Control
    IMetadata appControl; // original app control
    {
        auto pTempAppMetaControl = request->pAppMetaControlStreamBuffer->tryReadLock(LOG_TAG);
        if  (CC_LIKELY(pTempAppMetaControl)) {
            appControl = *pTempAppMetaControl;
            request->pAppMetaControlStreamBuffer->unlock(LOG_TAG, pTempAppMetaControl);
        }
    }
    auto const requestNo = request->requestNo;
    CAM_ULOG_ENTER(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, requestNo);

    // Evaluate a result for a request.
    pipelinesetting::RequestOutputParams out;
    {
        pipelinesetting::RequestInputParams const in{
            .requestNo                          = request->requestNo,
            .pRequest_AppImageStreamInfo        = request->pParsedAppImageStreamInfo.get(),
            .pRequest_AppControl                = &appControl,
            .pRequest_ParsedAppMetaControl      = request->pParsedAppMetaControl.get(),
            .pConfiguration_PipelineNodesNeed   = &mConfigInfo2->mPipelineNodesNeed,
            .pConfiguration_StreamInfo_NonP1    = &mConfigInfo2->mParsedStreamInfo_NonP1,
            .pConfiguration_StreamInfo_P1       = &mConfigInfo2->mvParsedStreamInfo_P1,
            .pSensorMode                        = &mSensorMode,
            .pSensorSize                        = &mSensorSize,
        };
        RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateRequest(out, in),
            "Fail on evaluateRequest - requestNo:%d", in.requestNo );
    }

    // Reconfiguration Flow
    if  (out.needReconfiguration) {
        MY_LOGW("[TODO] needReconfiguration - Not Implement");
    }

    // PipelineContext
    auto pPipelineContext = getCurrentPipelineContext();
    RETURN_ERROR_IF_NULLPTR(pPipelineContext, -ENODEV, "Bad PipelineContext");

    ////////////////////////////////////////////////////////////////////////////
    // process each frame
    auto processFrame = [&] (pipelinesetting::RequestResultParams const& result, bool isMain) -> int
    {
        std::vector<android::sp<IMetaStreamBuffer>> vAppMeta;
        generateControlAppMetaBuffer(
            &vAppMeta,
            (isMain) ? request->pAppMetaControlStreamBuffer : nullptr,
            &appControl, result.additionalApp.get(),
            mConfigInfo2->mParsedStreamInfo_NonP1.pAppMeta_Control.get());

        std::vector<android::sp<IMetaStreamBuffer>> vHalMeta;
        for(size_t i=0;i<mConfigInfo2->mvParsedStreamInfo_P1.size();++i)
        {
            MY_LOGD("generate (%zu) in metadata", i);
            generateControlHalMetaBuffer(
                &vHalMeta,
                result.additionalHal[i].get(),
                mConfigInfo2->mvParsedStreamInfo_P1[i].pHalMeta_Control.get());
        }

        android::sp<IPipelineFrame> pPipelineFrame;
        BuildPipelineFrameInputParams const params = {
            .requestNo = request->requestNo,
            .pAppImageStreamInfo    = (isMain ? request->pParsedAppImageStreamInfo.get() : nullptr),
            .pAppImageStreamBuffers = (isMain ? request->pParsedAppImageStreamBuffers.get() : nullptr),
            .pAppMetaStreamBuffers  = (vAppMeta.empty() ? nullptr : &vAppMeta),
            .pHalImageStreamBuffers = nullptr,
            .pHalMetaStreamBuffers  = (vHalMeta.empty() ? nullptr : &vHalMeta),
            .pvUpdatedImageStreamInfo = &(result.vUpdatedImageStreamInfo),
            .pnodeSet = &result.nodeSet,
            .pnodeIOMapImage = &(result.nodeIOMapImage),
            .pnodeIOMapMeta = &(result.nodeIOMapMeta),
            .pRootNodes = &(result.roots),
            .pEdges = &(result.edges),
            .pCallback = (isMain ? this : nullptr),
            .pPipelineContext = pPipelineContext
        };
        RETURN_ERROR_IF_NOT_OK( buildPipelineFrame(pPipelineFrame, params), "buildPipelineFrame fail - requestNo:%u", request->requestNo );
        RETURN_ERROR_IF_NOT_OK( pPipelineContext->queue(pPipelineFrame), "PipelineContext::queue fail - requestNo:%u", request->requestNo );
        return OK;
    };
    // main frame
    {
        if (out.mainFrame->nodesNeed.needJpegNode)
        {
            IPreReleaseRequestMgr::getInstance()->addCaptureRequest(request->requestNo, !out.bCshotRequest);
        }
        RETURN_ERROR_IF_NOT_OK(processFrame(*out.mainFrame, true), "processFrame mainFrame fail");
    }
    // post-dummy frames
    for (auto const& frame : out.postDummyFrames) {
        RETURN_ERROR_IF_NOT_OK(processFrame(*frame, false), "processFrame postDummyFrame fail");
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
configureDVFSControl() -> int
{
    auto const& pPipelineStaticInfo         = mStaticInfo.pPipelineStaticInfo;

    MY_LOGW("Draft configure mmdvfs, just porting old flow");
    mpScenarioCtrl = IScenarioControlV3::create(pPipelineStaticInfo->openId);
    IScenarioControlV3::ControlParam param;

    // basic component
    param.sensorSize = mConfigInfo2->mvSensorSetting[0].sensorSize;
    param.sensorFps  = mConfigInfo2->mvSensorSetting[0].sensorFps;
    param.scenario   = IScenarioControlV3::Scenario_HighSpeedVideo;

    // vhdr
    if( mConfigInfo2->mStreamingFeatureSetting.hdrSensorMode == SENSOR_VHDR_MODE_MVHDR )
        FEATURE_CFG_ENABLE_MASK(param.featureFlag, IScenarioControlV3::FEATURE_MVHDR);

    mpScenarioCtrl->enterScenario(param);
    return OK;
}

/******************************************************************************
 * TODO: Need to be optimized
 ******************************************************************************/
auto
ThisNamespace::
submitRequest(
    std::vector<std::shared_ptr<UserRequestParams>>const& requests,
    uint32_t& numRequestProcessed
) -> int
{
    numRequestProcessed = 0;

    std::vector<std::shared_ptr<ParsedAppRequest>> reqs;
    reqs.reserve(requests.size());

    //Convert: UserRequestParams -> ParsedAppRequest
    for (size_t i = 0; i < requests.size(); i++) {

        auto r = std::make_shared<ParsedAppRequest>();
        RETURN_ERROR_IF_NULLPTR( r, -ENODEV, "i:%zu Fail to make_shared<ParsedAppRequest>", i );

        RETURN_ERROR_IF_NOT_OK(
            parseAppRequest(ParseAppRequest{
                .to = r.get(), .from = requests[i].get(),
                .pUserConfiguration = mStaticInfo.pUserConfiguration.get(),}),
            "parseAppRequest fail on requestNo:%u - %zu/%zu requests parsed sucessfully",
            requests[i]->requestNo, i, requests.size() );

        reqs.emplace_back(r);
    }

    //Submit ParsedAppRequest one by one
    for (size_t i = 0; i < reqs.size(); i++, numRequestProcessed++) {
        RETURN_ERROR_IF_NOT_OK( submitOneRequest(reqs[i]),
            "submitOneRequest fail on requestNo:%u - %u/%zu requests submitted sucessfully",
            reqs[i]->requestNo, numRequestProcessed, reqs.size() );
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ThisNamespace::
updateFrame(
    MUINT32 const requestNo,
    MINTPTR const userId,
    Result const& result
)
{
    if (result.bFrameEnd) {
        return;
    }

    auto streamId = mConfigInfo2->mvParsedStreamInfo_P1[0].pHalMeta_DynamicP1->getStreamId();
    auto timestampStartOfFrame = determineTimestampSOF(streamId, result.vHalOutMeta);
    updateFrameTimestamp(requestNo, userId, result, timestampStartOfFrame);
}

