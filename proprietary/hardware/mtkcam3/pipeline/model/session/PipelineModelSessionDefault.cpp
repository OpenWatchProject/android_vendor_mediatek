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

#define LOG_TAG "mtkcam-PipelineModelSessionDefault"
//
#include "PipelineModelSessionDefault.h"
//
#include <impl/ControlMetaBufferGenerator.h>
#include <impl/PipelineContextBuilder.h>
#include <impl/PipelineFrameBuilder.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/utils/streaminfo/IStreamInfoSetControl.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_DEFAULT_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::pipeline::prerelease;

using namespace NSCam::Utils::ULog;

#define ThisNamespace   PipelineModelSessionDefault


/******************************************************************************
 *
 ******************************************************************************/
static inline bool isZoom(std::shared_ptr<ParsedMultiCamInfo> const& p)
{
    return (p->mDualDevicePath == NSCam::v3::pipeline::policy::DualDevicePath::MultiCamControl &&
            p->mDualFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_ZOOM);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
makeInstance(
    std::string const& name,
    CtorParams const& rCtorParams,
    ExtCtorParams const& rExtCtorParams
) -> android::sp<IPipelineModelSession>
{
    android::sp<ThisNamespace> pSession = new ThisNamespace(name, rCtorParams, rExtCtorParams);
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
auto
ThisNamespace::
makeInstance(
    std::string const& name,
    CtorParams const& rCtorParams
) -> android::sp<IPipelineModelSession>
{
    return ThisNamespace::makeInstance(name, rCtorParams, ExtCtorParams{});
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace(
    std::string const& name,
    CtorParams const& rCtorParams,
    ExtCtorParams const& rExtCtorParams)
    : PipelineModelSessionBasic(name, rCtorParams)
    , mpAppRaw16Reprocessor(rExtCtorParams.pAppRaw16Reprocessor)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace(
    std::string const& name,
    CtorParams const& rCtorParams)
    : PipelineModelSessionBasic(name, rCtorParams)
    , mpZslProcessor(nullptr)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~ThisNamespace()
{
    // uninit P1Node for BGService's requirement
    IPreReleaseRequestMgr::getInstance()->uninit();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    PipelineModelSessionBasic::dumpState(printer, options);

    {
        auto pAppRaw16Reprocessor = getAppRaw16Reprocessor();
        if ( pAppRaw16Reprocessor != nullptr ) {
            pAppRaw16Reprocessor->dumpState(printer);
        }
    }
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
getCaptureInFlightRequest() -> android::sp<ICaptureInFlightRequest>
{
    return mpCaptureInFlightRequest;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getZslProcessor() -> android::sp<IZslProcessor>
{
    return mpZslProcessor;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getAppRaw16Reprocessor() const -> android::sp<IAppRaw16Reprocessor>
{
    return mpAppRaw16Reprocessor;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
configure() -> int
{
    return PipelineModelSessionBasic::configure();
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_Capture() -> int
{
    // config ZSL
    CAM_TRACE_CALL();
    MY_LOGI("ZSL mode enable = %d", mConfigInfo2->mIsZSLMode);
    if (mConfigInfo2->mIsZSLMode) {
        configZSL();
    }

    // create capture related instances, MUST be after FeatureSettingPolicy
    auto pImgoStreamInfo = mConfigInfo2->mvParsedStreamInfo_P1[0].pHalImage_P1_Imgo.get();
    configureCaptureInFlight(mConfigInfo2->mCaptureFeatureSetting.maxAppJpegStreamNum, pImgoStreamInfo ? pImgoStreamInfo->getMaxBufNum(): 0);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_BeforeBuildingPipelineContext() -> int
{
    CAM_TRACE_CALL();
    // some feature needs some information which get from config policy update.
    // And, it has to do related before build pipeline context.
    // This interface will help to do this.
    RETURN_ERROR_IF_NOT_OK(updateBeforeBuildPipelineContext(), "updateBeforeBuildPipelineContext fail");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateBeforeBuildPipelineContext() -> int
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onRequest_Reconfiguration(
    std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
    policy::pipelinesetting::RequestOutputParams const& reqOutput __unused,
    std::shared_ptr<ParsedAppRequest>const& pRequest __unused
) -> int
{
    CAM_TRACE_CALL();

    RETURN_ERROR_IF_NOT_OK( processReconfiguration(reqOutput, pConfigInfo2, pRequest->requestNo),
            "[requestNo:%u] processReconfiguration", pRequest->requestNo);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
processReconfiguration(
    pipelinesetting::RequestOutputParams const& rcfOutputParam __unused,
    std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
    MUINT32 requestNo __unused
) -> int
{
    if(!rcfOutputParam.needReconfiguration)
    {
        return OK;
    }
    MY_LOGW("[TODO] needReconfiguration - Not Implement");
    return BAD_VALUE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
tryHandleAppRaw16ReprocessFrame(
    TryHandleAppRaw16ReprocessFrameParams const& params
) -> bool
{
    CAM_TRACE_CALL();

    auto pAppRaw16Reprocessor = params.pAppRaw16Reprocessor;

    bool isAppRaw16ReprocessFrame =
        pAppRaw16Reprocessor->isReprocessFrame(
        IAppRaw16Reprocessor::IsReprocessFrameParams{
            .isMainFrame = params.isMainFrame,
            .pAppRequest = params.pAppRequest,
        });

    if ( isAppRaw16ReprocessFrame ) {
        // Prepare HAL stream buffers for this Reprocess Frame if it is.
        pAppRaw16Reprocessor->handleReprocessFrame(
        IAppRaw16Reprocessor::HandleReprocessFrameParams{
            .pFrameBuilder          = params.pFrameBuilder,
            .pReqResult             = params.pReqResult,
            .pAppMetaControl        = params.pAppMetaControl,
            .pAppRequest            = params.pAppRequest,
            .pConfigStreamInfo_P1   = &params.pConfigInfo2->mvParsedStreamInfo_P1,
        });
    }

    return isAppRaw16ReprocessFrame;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onProcessEvaluatedFrame(
    uint32_t& lastFrameNo __unused,
    ProcessEvaluatedFrame const& in __unused
) -> int
{
    CAM_TRACE_CALL();
    auto const& reqOutput = *in.pReqOutputParams;
    auto const  requestNo = in.pAppRequest->requestNo;

    std::vector<std::shared_ptr<pipelinesetting::RequestResultParams>>
                                                vReqResult;
    FrameBuildersOfOneRequest                   vFrameBuilder;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Lambda Functions
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto addTo = [&] (auto const& pReqResult, GroupFrameType frameType) -> int
    {
        auto pFrameBuilder = convertRequestResultToFrameBuilder(frameType, *pReqResult, in);
        if (CC_UNLIKELY( pFrameBuilder == nullptr )) {
            return UNKNOWN_ERROR;
        }

        vReqResult.push_back(pReqResult);
        vFrameBuilder.push_back(pFrameBuilder);
        return OK;
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto processAndSubmitZslResultPipelineFrames = [&](auto const& vResults) -> int
    {
        for (auto const& frameBuilders : vResults)
        {
            // [3] a vector of IFrameBuilder -> a vector of IPipelineFrame
            std::vector<android::sp<IPipelineFrame>> pplFrames;
            RETURN_ERROR_IF_NOT_OK( makePipelineFrame(pplFrames, frameBuilders, in.pPipelineContext),
                "fail to make ZslResult pipelineFrames");

            if (!pplFrames.empty()) {
                // dump zsl request new frameNo
                std::ostringstream oss;
                oss << "zsl requestNo:" << pplFrames.front()->getRequestNo() << " new frames: ";

                // [4] a vector of IPipelineFrame: before IPipelineContext::queue()
                for (auto const& pPipelineFrame : pplFrames) {
                    oss << "[F" << pPipelineFrame->getFrameNo() << "] ";
                    CAM_ULOG_SUBREQS(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, pPipelineFrame->getRequestNo(), REQ_PIPELINE_FRAME, pPipelineFrame->getFrameNo());
                }
                MY_LOGI("%s", oss.str().c_str());

                // [5] a vector of IPipelineFrame ~~> IPipelineContext
                lastFrameNo = pplFrames.back()->getFrameNo();
                RETURN_ERROR_IF_NOT_OK(in.pPipelineContext->queue(pplFrames),
                    "[requestNo:%u] IPipelineContext::queue() #pplFrames:%zu", pplFrames.back()->getRequestNo(), pplFrames.size());
            }
        }
        return OK;
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // [1] ProcessEvaluatedFrame -> a vector of IFrameBuilder.
    {
        // pre-dummy frames
        for (auto const& frame : reqOutput.preDummyFrames) {
            RETURN_ERROR_IF_NOT_OK( addTo(frame, GroupFrameType::PREDUMMY),
                    "[requestNo:%u] addTo() failed - preDummyFrame", requestNo);
        }
        // pre-sub frames
        for (auto const& frame : reqOutput.preSubFrames) {
            RETURN_ERROR_IF_NOT_OK( addTo(frame, GroupFrameType::PRESUB),
                    "[requestNo:%u] addTo() failed - preSubFrame", requestNo);
        }
        // main frame
        {
            RETURN_ERROR_IF_NOT_OK( addTo((reqOutput.mainFrame), GroupFrameType::MAIN),
                    "[requestNo:%u] addTo() failed - mainFrame", requestNo);
        }
        // sub frames
        for (auto const& frame : reqOutput.subFrames) {
            RETURN_ERROR_IF_NOT_OK( addTo(frame, GroupFrameType::SUB),
                    "[requestNo:%u] addTo() failed - subFrame", requestNo);
        }
        // post-dummy frames
        for (auto const& frame : reqOutput.postDummyFrames) {
            RETURN_ERROR_IF_NOT_OK( addTo(frame, GroupFrameType::POSTDUMMY),
                    "[requestNo:%u] addTo() failed - postDummyFrame", requestNo);
        }
    }

    // [2-1] process Zsl request:  (1) zsl capture & (2) zsl pending
    if (mpZslProcessor.get())
    {
        if (reqOutput.needZslFlow)  // (1) zsl capture
        {
            MY_LOGD("[requestNo:%u] submit Zsl Request", requestNo);
            RETURN_ERROR_IF_NOT_OK(processAndSubmitZslResultPipelineFrames(
                mpZslProcessor->submitZslRequest(
                    ZslRequestParams{
                        .requestNo = requestNo,
                        .pCapParams = std::make_shared<policy::pipelinesetting::RequestOutputParams>(reqOutput),
                        .pPipelineContext = in.pPipelineContext,
                        .vFrameBuilder = std::move(vFrameBuilder),
                        .isZoom = isZoom(mStaticInfo.pUserConfiguration->pParsedAppConfiguration->pParsedMultiCamInfo)
                    })),
                "[requestNo:%u] fail to processAndSubmitZslResultPipelineFrames", requestNo);

            return OK;  // zsl request end
        }
        else  // (2) zsl pending  <-- [zsl streaming / normal capture] will trigger this
        {
            if (mpZslProcessor->hasZslPendingRequest())
            {
                MY_LOGD("[requestNo:%u] process zsl pending request", requestNo);
                RETURN_ERROR_IF_NOT_OK( processAndSubmitZslResultPipelineFrames(
                    mpZslProcessor->processZslPendingRequest(requestNo)),
                    "[requestNo:%u] fail to processAndSubmitZslResultPipelineFrames for pending", requestNo);
            }
        }
    }

    // [2-2] process this request:  (1) non-zsl & (2) zsl streaming / normal capture (eg. HDR)
    {
        // [3] a vector of IFrameBuilder -> a vector of IPipelineFrame
        std::vector<android::sp<IPipelineFrame>> pplFrames;
        RETURN_ERROR_IF_NOT_OK( makePipelineFrame(pplFrames, vFrameBuilder, in.pPipelineContext),
            "fail to make pipelineFrames");
        MY_LOGF_IF((pplFrames.size() != vReqResult.size()),
            "mismatch - #pplFrames:%zu #vReqResult:%zu", pplFrames.size(), vReqResult.size());

        if (!pplFrames.empty()) {
            // [4] a vector of IPipelineFrame: before IPipelineContext::queue()
            for (size_t i = 0; i < pplFrames.size(); i++)  // per frame
            {
                auto const& pPipelineFrame = pplFrames[i];
                auto const pReqResult = vReqResult[i].get();

                // [4-1] register streams to track for this frame (exclude: normal capture & init request & dummies)
                if (mpZslProcessor.get())  //(2) zsl streaming / normal capture
                {
                    if (auto const& pAppConfig = mStaticInfo.pUserConfiguration->pParsedAppConfiguration;
                        reqOutput.keepZslBuffer &&
                        (pPipelineFrame->getFrameNo() >= static_cast<uint32_t>(pAppConfig->initRequest)) &&
                        ((pPipelineFrame->getGroupFrameType() == GroupFrameType::MAIN) ||
                        (pPipelineFrame->getGroupFrameType() == GroupFrameType::SUB) ||
                        (pPipelineFrame->getGroupFrameType() == GroupFrameType::PRESUB)))
                    {
                        // Zoom specify streams to track, else track default streams
                        mpZslProcessor->registerTrackFrameParams(pPipelineFrame,
                            isZoom(pAppConfig->pParsedMultiCamInfo) ? pReqResult : nullptr);
                    }
                }

                // [4-2] keep raw16 frame for super night mode (?)
                tryKeepFrameForAppRaw16Reprocess(pPipelineFrame, pReqResult, in.pAppMetaControl, in.pAppRequest);

                // [4-3] register ULog SUBREQS (requestNo -> frameNos)
                CAM_ULOG_SUBREQS(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, pPipelineFrame->getRequestNo(), REQ_PIPELINE_FRAME, pPipelineFrame->getFrameNo());
            }

            // [5] a vector of IPipelineFrame ~~> IPipelineContext
            lastFrameNo = pplFrames.back()->getFrameNo();
            RETURN_ERROR_IF_NOT_OK(in.pPipelineContext->queue(pplFrames),
                "[requestNo:%u] IPipelineContext::queue() #pplFrames:%zu", requestNo, pplFrames.size());
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
convertRequestResultToFrameBuilder(
    GroupFrameType frameType __unused,
    policy::pipelinesetting::RequestResultParams const& reqResult __unused,
    ProcessEvaluatedFrame const& in __unused
) -> std::shared_ptr<IFrameBuilderT>
{
    CAM_TRACE_CALL();

    int res = OK;

    auto const& pAppMetaControl = in.pAppMetaControl;
    auto const& request = in.pAppRequest;
    auto const& pConfigInfo2 = in.pConfigInfo2;

    // App Meta stream buffers
    std::vector<android::sp<IMetaStreamBuffer>> vAppMeta;
    {
        res = generateControlAppMetaBuffer(
                &vAppMeta,
                (frameType == GroupFrameType::MAIN) ? request->pAppMetaControlStreamBuffer : nullptr,
                pAppMetaControl.get(), reqResult.additionalApp.get(),
                pConfigInfo2->mParsedStreamInfo_NonP1.pAppMeta_Control.get());
        RETURN_NULLPTR_IF_NOT_OK( res, "[requestNo:%u] generateControlAppMetaBuffer", request->requestNo );
    }

    // Hal Meta stream buffers
    // handle multicam case, add more then one hal metadata.
    std::vector<android::sp<IMetaStreamBuffer>> vHalMeta;
    for (size_t i = 0;
        (i < reqResult.additionalHal.size() && i < pConfigInfo2->mvParsedStreamInfo_P1.size());
        i++)
    {
        res = generateControlHalMetaBuffer(
                &vHalMeta,
                reqResult.additionalHal[i].get(),
                pConfigInfo2->mvParsedStreamInfo_P1[i].pHalMeta_Control.get());
        RETURN_NULLPTR_IF_NOT_OK( res, "[requestNo:%u] generateControlHalMetaBuffer, i:%zu", request->requestNo, i );
    }

    BuildPipelineFrameInputParams const params = {
        .requestNo = request->requestNo,
        .pAppImageStreamInfo    = (frameType == GroupFrameType::MAIN ? request->pParsedAppImageStreamInfo.get() : nullptr),
        .pAppImageStreamBuffers = (frameType == GroupFrameType::MAIN ? request->pParsedAppImageStreamBuffers.get() : nullptr),
        .pAppMetaStreamBuffers  = (vAppMeta.empty() ? nullptr : &vAppMeta),
        .pHalImageStreamBuffers = nullptr,
        .pHalMetaStreamBuffers  = (vHalMeta.empty() ? nullptr : &vHalMeta),
        .pvUpdatedImageStreamInfo = &(reqResult.vUpdatedImageStreamInfo),
        .pnodeSet = &reqResult.nodeSet,
        .pnodeIOMapImage = &(reqResult.nodeIOMapImage),
        .pnodeIOMapMeta = &(reqResult.nodeIOMapMeta),
        .pRootNodes = &(reqResult.roots),
        .pEdges = &(reqResult.edges),
        .pCallback = (frameType == GroupFrameType::MAIN ? this : nullptr),
        .physicalMetaStreamIds = &(reqResult.physicalMetaStreamIds),
    };
    auto pFrameBuilder = makeFrameBuilder(params);
    RETURN_NULLPTR_IF_NULLPTR( pFrameBuilder, "[requestNo:%u] makeFrameBuilder", request->requestNo );

    pFrameBuilder->setGroupFrameType(frameType);

    // App Raw16 Reprocess Frame?
    auto pAppRaw16Reprocessor = getAppRaw16Reprocessor();
    if ( pAppRaw16Reprocessor != nullptr ) {
        bool isAppRaw16ReprocessFrame __unused =
            tryHandleAppRaw16ReprocessFrame(
            TryHandleAppRaw16ReprocessFrameParams{
                .pFrameBuilder          = pFrameBuilder,
                .pReqResult             = &reqResult,
                .isMainFrame            = (frameType==GroupFrameType::MAIN),
                .pAppMetaControl        = pAppMetaControl,
                .pAppRequest            = request,
                .pConfigInfo2           = pConfigInfo2,
                .pAppRaw16Reprocessor   = pAppRaw16Reprocessor,
            });
    }

    return pFrameBuilder;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
tryKeepFrameForAppRaw16Reprocess(
    android::sp<IPipelineFrame> pPipelineFrame,
    policy::pipelinesetting::RequestResultParams const* pReqResult,
    std::shared_ptr<IMetadata> pAppMetaControl,
    std::shared_ptr<ParsedAppRequest> request
) -> void
{
    CAM_TRACE_CALL();
    if ( auto pAppRaw16Reprocessor = getAppRaw16Reprocessor(); pAppRaw16Reprocessor != nullptr )
    {
        // App Raw16 Reprocess Frame?
        bool isAppRaw16ReprocessFrame =
            pAppRaw16Reprocessor->isReprocessFrame(
            IAppRaw16Reprocessor::IsReprocessFrameParams{
                .isMainFrame = (pPipelineFrame->getGroupFrameType() == GroupFrameType::MAIN),
                .pAppRequest = request,
            });

        // Keep this frame before queuing it to the pipeline.
        pAppRaw16Reprocessor->keepFrameIfNeeded(
        IAppRaw16Reprocessor::KeepFrameIfNeededParams{
            .isReprocessRequest = isAppRaw16ReprocessFrame,
            .pPipelineFrame     = pPipelineFrame,
            .pReqResult         = pReqResult,
            .isMainFrame        = (pPipelineFrame->getGroupFrameType() == GroupFrameType::MAIN),
            .pAppMetaControl    = pAppMetaControl,
            .pAppRequest        = request,
        });

        CAM_LOGI_IF(isAppRaw16ReprocessFrame,
                "[requestNo:%u frameNo:%u] a reprocess frame is ready to be queued to pipeline",
                pPipelineFrame->getRequestNo(), pPipelineFrame->getFrameNo());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
beginFlush() -> int
{
    {
        auto pAppRaw16Reprocessor = getAppRaw16Reprocessor();
        if ( pAppRaw16Reprocessor != nullptr ) {
            pAppRaw16Reprocessor->beginFlush();
        }
    }
    auto pPipelineContext = getCurrentPipelineContext();
    RETURN_ERROR_IF_NULLPTR( pPipelineContext, OK, "No current pipeline context" );
    // create a thread to handle pre-release request
    IPreReleaseRequestMgr::getInstance()->createPreRelease(pPipelineContext);
    flushZslPendingReq(pPipelineContext);
    RETURN_ERROR_IF_NOT_OK( pPipelineContext->beginFlush(), "PipelineContext::beginFlush()" );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
endFlush() -> void
{
    auto pPipelineContext = getCurrentPipelineContext();
    if (CC_LIKELY( pPipelineContext != nullptr )) {
        pPipelineContext->endFlush();
    }

    {
        auto pAppRaw16Reprocessor = getAppRaw16Reprocessor();
        if ( pAppRaw16Reprocessor != nullptr ) {
            pAppRaw16Reprocessor->endFlush();
        }
    }

    auto pPreRelease = IPreReleaseRequestMgr::getInstance()->getPreRelease();
    if (pPreRelease != NULL) {
        pPreRelease->start();
    }
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
    auto pAppRaw16Reprocessor = getAppRaw16Reprocessor();
    if ( pAppRaw16Reprocessor != nullptr ) {
        pAppRaw16Reprocessor->debugRequestResult(
        IAppRaw16Reprocessor::DebugRequestResultParams{
            .requestNo      = requestNo,
            .pvAppOutMeta   = &result.vAppOutMeta,
        });
    }

    if(mpZslProcessor.get())
        mpZslProcessor->onFrameUpdated(requestNo, result);

    if (result.bFrameEnd) {
        if ( pAppRaw16Reprocessor != nullptr ) {
            pAppRaw16Reprocessor->notifyRequestDone(requestNo);
        }
        mpCaptureInFlightRequest->removeRequest(requestNo);
        mpScenarioCtrl->checkIfNeedExitBoost(result.frameNo, false);
        return;
    }

    StreamId_T streamId = -1L;
    {
        android::RWLock::AutoRLock _l(mRWLock_ConfigInfo2);
        streamId = mConfigInfo2->mvParsedStreamInfo_P1[0].pHalMeta_DynamicP1->getStreamId();
    }
    auto timestampStartOfFrame = determineTimestampSOF(streamId, result.vHalOutMeta);
    updateFrameTimestamp(requestNo, userId, result, timestampStartOfFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateFrameTimestamp(
    MUINT32 const requestNo,
    MINTPTR const userId,
    Result const& result,
    int64_t timestampStartOfFrame
) -> void
{
    //ZSL flow


    PipelineModelSessionBase::updateFrameTimestamp(requestNo, userId, result, timestampStartOfFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
configureCaptureInFlight(const int maxJpegNum, const int maxImgoNum) -> int
{
    mpCaptureInFlightRequest = ICaptureInFlightRequest::createInstance(
                            mStaticInfo.pPipelineStaticInfo->openId,
                            mSessionName);
    if(!mpCaptureInFlightRequest.get())
    {
        MY_LOGE("fail to create CaptureInFlighRequest");
        return ~OK;
    }
    INextCaptureListener::CtorParams ctorParams;
    ctorParams.maxJpegNum  = maxJpegNum;
    ctorParams.maxImgoNum  = maxImgoNum;
    ctorParams.pCallback   = mPipelineModelCallback;

    mpNextCaptureListener = INextCaptureListener::createInstance(
                            mStaticInfo.pPipelineStaticInfo->openId,
                            mSessionName, ctorParams);
    if (mpNextCaptureListener.get())
    {
        mpCaptureInFlightRequest->registerListener(mpNextCaptureListener);
    }
    else
    {
        MY_LOGE("fail to create NextCaptureListener");
        return ~OK;
    }

    return OK;
}

/******************************************************************************
 *  Internal Operations for ZSL.
 ******************************************************************************/
auto
ThisNamespace::
configZSL() -> void
{
    // check enable zsl or not (0:off; 1:0n; 2:desided by app<TO-DO>)
    int32_t ZSLOpen = ::property_get_int32("vendor.debug.camera.zsl.enable", 1);
    if (ZSLOpen == 1) {
        mpZslProcessor = IZslProcessor::createInstance(
                            mStaticInfo.pPipelineStaticInfo->openId,
                            mSessionName
                        );

        if (!mpZslProcessor.get()) {
            MY_LOGE("fail to create ZSLProcessor (ZSLOpen = %d)", ZSLOpen);
            return;
        }
    } else if (ZSLOpen == 0) {
        mpZslProcessor = nullptr;
    }

    if (mpZslProcessor.get()) {
        auto err = mpZslProcessor->configure(
                        ZslConfigParams{
                            .pParsedStreamInfo_P1 = &mConfigInfo2->mvParsedStreamInfo_P1,
                            .pCallback = mPipelineModelCallback,
                            .maxRequestSize = (mConfigInfo2->mCaptureFeatureSetting.maxAppJpegStreamNum + mConfigInfo2->mCaptureFeatureSetting.maxAppRaw16OutputBufferNum)
                            /* max jpeg + raw16 count -> max zsl request size */
                    });

        if (err != 0) {
            MY_LOGE("fail to config ZSLProcessor (err = %d), release ZslProcessor", err);
            mpZslProcessor = nullptr;
        }
    }
}


auto
ThisNamespace::
flushZslPendingReq(
    const android::sp<IPipelineContextT>& pPipelineContext
) -> int
{
    if ((mpZslProcessor.get()) && (mpZslProcessor->hasZslPendingRequest()))
    {
        for (auto const& vFrameBuilder : mpZslProcessor->flush())
        {
            // [3] a vector of IFrameBuilder -> a vector of IPipelineFrame
            std::vector<android::sp<IPipelineFrame>> pplFrames;
            if (!vFrameBuilder.empty()) {
                makePipelineFrame(pplFrames, vFrameBuilder, pPipelineContext);
            }

            if (!pplFrames.empty()) {
                // [4] before IPipelineContext::queue()
                for (auto const& pPipelineFrame : pplFrames) {
                    CAM_ULOG_SUBREQS(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, pPipelineFrame->getRequestNo(), REQ_PIPELINE_FRAME, pPipelineFrame->getFrameNo());
                }

                // [5] a vector of IPipelineFrame ~~> IPipelineContext
                MY_LOGI("[requestNo:%u] flush Zsl pending #pplFrames:%zu", pplFrames.back()->getRequestNo(), pplFrames.size());
                RETURN_ERROR_IF_NOT_OK(pPipelineContext->queue(pplFrames),
                    "[requestNo:%u] IPipelineContext::queue() #pplFrames:%zu", pplFrames.back()->getRequestNo(), pplFrames.size());
            }
        }

        pPipelineContext->waitUntilRootNodeDrained();
    }

    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ThisNamespace::
onNextCaptureCallBack(
    MUINT32   requestNo,
    MINTPTR   nodeId __unused,
    MUINT32   requestCnt,
    MBOOL     bSkipCheck
)
{
    mpNextCaptureListener->onNextCaptureCallBack(requestNo, requestCnt, bSkipCheck);
}

