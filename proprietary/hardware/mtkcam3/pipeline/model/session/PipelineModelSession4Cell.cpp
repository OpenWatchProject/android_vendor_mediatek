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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "mtkcam-PipelineModelSession4Cell"
//
#include "PipelineModelSession4Cell.h"
//
#include "MyUtils.h"
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/hwnode/P1Node.h>
#include <mtkcam3/pipeline/hwnode/P2StreamingNode.h>
#include <mtkcam3/pipeline/hwnode/P2CaptureNode.h>
#include <mtkcam3/pipeline/hwnode/JpegNode.h>
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
//
#include <impl/ControlMetaBufferGenerator.h>
#include <impl/PipelineContextBuilder.h>
#include <impl/PipelineFrameBuilder.h>
#include <mtkcam/utils/std/ULog.h>
//
using namespace NSCam::Utils::ULog;
//
#define RESOURCE_CONCURRENCY_TIMEOUT_MS (3000)
// function scope
#if 0
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>

static inline std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_ULOGMD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_ULOGMD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
#endif

CAM_ULOG_DECLARE_MODULE_ID(MOD_4CELL_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
    #define TRIGGER_AEE(_msg_)                                                  \
        do {                                                                    \
            aee_system_exception(LOG_TAG, NULL, DB_OPT_DEFAULT, _msg_);         \
        } while(0)
#else
    #define TRIGGER_AEE(_ops_)
#endif

#ifndef RETURN_ERROR_AEE_IF_NOT_OK
#define RETURN_ERROR_AEE_IF_NOT_OK(_expr_, cr_key, fmt, arg...)                 \
    do {                                                                        \
        int const err = (_expr_);                                               \
        if( CC_UNLIKELY(err != 0) ) {                                           \
            MY_LOGE("err:%d(%s) - " fmt, err, ::strerror(-err), ##arg);         \
            char msg[128] = {0};                                                \
            auto n __unused = ::snprintf(msg, sizeof(msg), fmt, ##arg);         \
            TRIGGER_AEE(                                                        \
                (std::string(" ") + msg +                                       \
                   "\nCRDISPATCH_KEY:" + cr_key).c_str());                      \
            return err;                                                         \
        }                                                                       \
    } while(0)
#endif


/******************************************************************************
 *
 ******************************************************************************/
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCamHW;
using namespace NSCam;
using namespace NSCam::v3::pipeline::prerelease;
using namespace NSCam::v3::Utils;
#define ThisNamespace   PipelineModelSession4Cell
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
    mbDebugLogLevel = property_get_int32("vendor.debug.camera.PipelineModelFCell", 0);
    mP1NodeConcurrency = IResourceConcurrency::createInstance("mP1NodeConcurrency", RESOURCE_CONCURRENCY_TIMEOUT_MS);
    sem_init(&mP1ConfigLock, 0, 1);
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~ThisNamespace()
{
    mP1NodeConcurrency = nullptr;
    // uninit P1Node for BGService's requirement
    IPreReleaseRequestMgr::getInstance()->uninit();
    sem_destroy(&mP1ConfigLock);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
submitOneRequest(
    std::shared_ptr<ParsedAppRequest>const& request __unused
) -> int
{
    // On this function, use this local variable to serve the request.
    std::shared_ptr<ConfigInfo2> pConfigInfo2;
    {
        android::RWLock::AutoRLock _l(mRWLock_ConfigInfo2);
        pConfigInfo2 = mConfigInfo2;
    }
    auto const requestNo = request->requestNo;
    CAM_ULOG_ENTER(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, requestNo);

    // Make a copy of the original App Meta Control
    auto pAppControl = std::make_shared<IMetadata>();
    {
        auto pTempAppMetaControl = request->pAppMetaControlStreamBuffer->tryReadLock(LOG_TAG);
        if  (CC_LIKELY(pTempAppMetaControl)) {
            *pAppControl = *pTempAppMetaControl;
            request->pAppMetaControlStreamBuffer->unlock(LOG_TAG, pTempAppMetaControl);
        }
    }

    // Evaluate a result for a request.
    pipelinesetting::RequestOutputParams out;
    {
        pipelinesetting::RequestInputParams const in{
            .requestNo                          = request->requestNo,
            .pRequest_AppImageStreamInfo        = request->pParsedAppImageStreamInfo.get(),
            .pRequest_AppControl                = pAppControl.get(),
            .pRequest_ParsedAppMetaControl      = request->pParsedAppMetaControl.get(),
            .pConfiguration_PipelineNodesNeed   = &pConfigInfo2->mPipelineNodesNeed,
            .pConfiguration_StreamInfo_NonP1    = &pConfigInfo2->mParsedStreamInfo_NonP1,
            .pConfiguration_StreamInfo_P1       = &pConfigInfo2->mvParsedStreamInfo_P1,
            .pSensorMode                        = &mSensorMode,
            .pSensorSize                        = &mSensorSize,
            .isZSLMode                          = pConfigInfo2->mIsZSLMode,
            .pP1HwSetting                       = &pConfigInfo2->mvP1HwSetting,
        };
        RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateRequest(out, in),
            "Fail on evaluateRequest - requestNo:%d", in.requestNo );
    }

    auto pReqOutParm = std::make_shared<pipelinesetting::RequestOutputParams>();
    *pReqOutParm =  out;

    HwInfoHelper infoHelper = HwInfoHelper(mStaticInfo.pPipelineStaticInfo->sensorId[0]);
    infoHelper.updateInfos();
    MSize  realCaptureSize;
    if ( CC_UNLIKELY( ! infoHelper.getSensorSize(pReqOutParm->sensorMode[0], realCaptureSize) ) )
    {
        MY_LOGW("get real Capture Size fail");
    }

    {
        IMetadata::IEntry e1(MTK_HAL_REQUEST_REAL_CAPTURE_SIZE);
        e1.push_back(realCaptureSize, Type2Type<MSize>());

        IMetadata::IEntry e2(MTK_P1NODE_CTRL_RECONFIG_SENSOR_SETTING);
        e2.push_back(MTRUE, Type2Type<MBOOL>());

        for(size_t i=0;i<pReqOutParm->mainFrame->additionalHal.size();++i)
        {
            pReqOutParm->mainFrame->additionalHal[i]->update(e1.tag(), e1);
            MY_LOGD_IF( mbDebugLogLevel == 1,"Update realCaptureSize for p2 from %d -> %d",
                mSensorMode[0],pReqOutParm->sensorMode[0]);

            if(mbNeedRcgSensor)
            {
                pReqOutParm->mainFrame->additionalHal[i]->update(e2.tag(), e2);
                MY_LOGD("Resume from capturing, need reconfig sensor");
            }
        }
        mbNeedRcgSensor = MFALSE;
    }

    // Reconfiguration Flow
    sem_wait(&mP1ConfigLock);
    processReconfiguration_4cell(pReqOutParm,pConfigInfo2,request,pAppControl,request->requestNo);

    // directly return OK for multi-thread capturing
    if(out.needReconfiguration &&
            (pReqOutParm->reconfigCategory != ReCfgCtg::STREAMING)){
        MY_LOGD("directly return OK for capture frame");
        mbNeedRcgSensor = MTRUE;
        return OK;
    }

    // Process and submit pipeline Frame
    auto pPipelineContext = getCurrentPipelineContext();
    RETURN_ERROR_IF_NOT_OK(
        processEvaluatedFrame(
        ProcessEvaluatedFrame{
            .pReqOutputParams   = pReqOutParm,
            .pAppMetaControl    = pAppControl,
            .pAppRequest        = request,
            .pConfigInfo2       = pConfigInfo2,
            .pPipelineContext   = pPipelineContext,
        }),
        "Fail on processEvaluatedFrame - requestNo:%d", request->requestNo );
    sem_post(&mP1ConfigLock);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
configure() -> int
{
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
            .pIsZSLMode                 = &mConfigInfo2->mIsZSLMode,
        };
        RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateConfiguration(out, {}), "Fail on evaluateConfiguration" );
        #if 0
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_DEBUG, "[configure] ");
        logPrinter.printLine("== Current ConfigInfo2 ==");
        print(logPrinter, *mConfigInfo2);
        #endif
    }

    // App Image Max. Buffer Number
    RETURN_ERROR_IF_NOT_OK(
        mPipelineSettingPolicy->decideConfiguredAppImageStreamMaxBufNum(
            mStaticInfo.pUserConfiguration->pParsedAppImageStreamInfo.get(),
            &mConfigInfo2->mStreamingFeatureSetting,
            &mConfigInfo2->mCaptureFeatureSetting
        ), "Fail on decideConfiguredAppImageStreamMaxBufNum"
    );

    // config ZSL
    MY_LOGI("ZSL mode enable = %d",mConfigInfo2->mIsZSLMode);
    if (mConfigInfo2->mIsZSLMode) {
        configZSL();
    }

    // configure scenario control
    configureDVFSControl(mConfigInfo2->mStreamingFeatureSetting.BWCScenario, mConfigInfo2->mStreamingFeatureSetting.BWCFeatureFlag);

    // some feature needs some information which get from config policy update.
    // And, it has to do related before build pipeline context.
    // This interface will help to do this.
    RETURN_ERROR_IF_NOT_OK(updateBeforeBuildPipelineContext(), "updateBeforeBuildPipelineContext faile");
    // create capture related instances, MUST be after FeatureSettingPolicy
    auto pImgoStreamInfo = mConfigInfo2->mvParsedStreamInfo_P1[0].pHalImage_P1_Imgo.get();
    configureCaptureInFlight(mConfigInfo2->mCaptureFeatureSetting.maxAppJpegStreamNum, pImgoStreamInfo ? pImgoStreamInfo->getMaxBufNum(): 0);
    // build pipeline context
    {
        BuildPipelineContextInputParams const in{
            .pipelineName               = getSessionName(),
            .pPipelineStaticInfo        = mStaticInfo.pPipelineStaticInfo.get(),
            .pPipelineUserConfiguration = mStaticInfo.pUserConfiguration.get(),
            .pPipelineUserConfiguration2= mStaticInfo.pUserConfiguration2.get(),
            .pParsedStreamInfo_NonP1    = &mConfigInfo2->mParsedStreamInfo_NonP1,
            .pParsedStreamInfo_P1       = &mConfigInfo2->mvParsedStreamInfo_P1,
            .bIsZslEnabled              = mpZslProcessor.get() ? true : false,
            .pSensorSetting             = &mConfigInfo2->mvSensorSetting,
            .pvP1HwSetting              = &mConfigInfo2->mvP1HwSetting,
            .pPipelineNodesNeed         = &mConfigInfo2->mPipelineNodesNeed,
            .pPipelineTopology          = &mConfigInfo2->mPipelineTopology,
            .pStreamingFeatureSetting   = &mConfigInfo2->mStreamingFeatureSetting,
            .pCaptureFeatureSetting     = &mConfigInfo2->mCaptureFeatureSetting,
            .batchSize                  = mConfigInfo2->mStreamingFeatureSetting.groupSize,
            .pOldPipelineContext        = nullptr,
            .pDataCallback              = this,
            .bUsingParallelNodeToBuildPipelineContext = mbUsingParallelNodeToBuildPipelineContext,
            .bUsingMultiThreadToBuildPipelineContext = mbUsingMultiThreadToBuildPipelineContext,
            .bIsReconfigure             = false,
            .pP1NodeResourceConcurrency = mP1NodeConcurrency,
        };

        MY_LOGD("mpZslProcessor(%p)", mpZslProcessor.get());
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
 *
 ******************************************************************************/
auto
ThisNamespace::
beginFlush() -> int
{
    // wait until multi-thread capture done
    RETURN_ERROR_IF_NOT_OK( waitUntilCaptureDone(), "waitUntilCaptureDone" );

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

    auto pPreRelease = IPreReleaseRequestMgr::getInstance()->getPreRelease();
    if (pPreRelease != NULL) {
        pPreRelease->start();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
processReconfigStream(
    std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
    MUINT32 requestNo
) -> int
{
    MY_LOGD("requestNo(%d) processReconfigStream +", requestNo);

    android::RWLock::AutoWLock _l(mRWLock_PipelineContext);

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
        .pCaptureFeatureSetting     = &pConfigInfo2->mCaptureFeatureSetting,
        .batchSize                  = pConfigInfo2->mStreamingFeatureSetting.groupSize,
        .pOldPipelineContext        = nullptr,
        .pDataCallback              = this,
        .bUsingParallelNodeToBuildPipelineContext = mbUsingParallelNodeToBuildPipelineContext,
        .bUsingMultiThreadToBuildPipelineContext = mbUsingMultiThreadToBuildPipelineContext,
        .bIsReconfigure             = true,
        .pP1NodeResourceConcurrency = mP1NodeConcurrency,
    };

    MY_LOGD("mpZslProcessor(%p)", mpZslProcessor.get());
    //config P1 only or Create New pipeline context for streaming
    MBOOL ret = INVALID_OPERATION;
    if(mSensorMode[0] == pConfigInfo2->mvSensorSetting[0].sensorMode){
        MY_LOGD("Sensor Setting no change: reconfigure p1 only %d -> %d",mSensorMode[0],pConfigInfo2->mvSensorSetting[0].sensorMode);
        ret = waitUntilP1NodeDrained(mCurrentPipelineContext);
        if (ret!=OK)
        {
            CAM_ULOGME("waitUntilP1NodeDrainedAFlush Fail!");
            return ret;
        }
        RETURN_ERROR_IF_NOT_OK( reconfigureP1ForPipelineContext(mCurrentPipelineContext, in), "Fail on pipelineContextConfigP1" );
    }else{
        MY_LOGD("Sensor Setting change: buildPipelineContext %d -> %d",mSensorMode[0], pConfigInfo2->mvSensorSetting[0].sensorMode);
        ret = waitUntilP1NodeDrainedAndFlush(mCurrentPipelineContext);
        if (ret!=OK)
        {
            CAM_ULOGME("waitUntilP1NodeDrainedAFlush Fail!");
            return ret;
        }

        ret = waitUntilP2DrainedAndFlush(mCurrentPipelineContext);
        if (ret!=OK)
        {
            CAM_ULOGME("waitUntilP2DrainedAndFlush Fail!");
            return ret;
        }
        mCurrentPipelineContext = NULL;
    RETURN_ERROR_IF_NOT_OK( buildPipelineContext(mCurrentPipelineContext, in), "Fail on buildPipelineContext" );
    }

    MY_LOGD("requestNo(%d) processReconfigStream -", requestNo);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
processReconfiguration_4cell(
    std::shared_ptr<pipelinesetting::RequestOutputParams> pReqOutParm,
    std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
    std::shared_ptr<ParsedAppRequest>const& request,
    std::shared_ptr<IMetadata> pAppControl,
     MUINT32 requestNo
) -> int
{
    if(!pReqOutParm->needReconfiguration)
    {
        return OK;
    }

    if(pReqOutParm->reconfigCategory == ReCfgCtg::STREAMING)
    {
        //reconfigCategory is Stream
        if(processReconfigStream(pConfigInfo2, requestNo) != OK)
        {
            MY_LOGE("reconfigCategory(%hhu): processReconfigStream Error", pReqOutParm->reconfigCategory);
            return BAD_VALUE;
        }
        return OK;
    }

    //create another thread to process capturing
    {
        android::RWLock::AutoWLock _l(mRWLock_vCapFut);
        //MY_LOGD("+ mvCaptureFuture this:%d use_count:%ld %ld %ld", this->getStrongCount(), pReqOutParm.use_count(), request.use_count(), pAppControl.use_count());
        mvCaptureFuture.push_back(
            std::async(std::launch::async,
                &ThisNamespace::doCapture,
                this, pReqOutParm, request, pAppControl, requestNo
            )
        );
        //MY_LOGD("- mvCaptureFuture this:%d use_count:%ld %ld %ld", this->getStrongCount(), pReqOutParm.use_count(), request.use_count(), pAppControl.use_count());
    }

    MBOOL ret = INVALID_OPERATION;
    auto pPipelineContext = getCurrentPipelineContext();
    ret = waitUntilP1NodeDrainedAndFlush(pPipelineContext);

    if (ret!=OK)
    {
        CAM_ULOGME("waitUntilP1NodeDrainedAFlush Fail!");
        return ret;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilP1NodeDrainedAndFlush(
    android::sp<IPipelineContextT> pPipelineContext
) -> MERROR
{
    MERROR err = OK;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP1nodeDrainedAndFlush");
    //P1
    {
        err = waitUntilNodeDrainedAndFlush(eNODEID_P1Node, pPipelineContext);
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
waitUntilP1NodeDrained(
    android::sp<IPipelineContextT> pPipelineContext
) -> MERROR
{
    MERROR err = OK;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP1nodeDrainedAndFlush");
    //P1
    {
        err = waitUntilNodeDrained(eNODEID_P1Node, pPipelineContext);
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
waitUntilP2DrainedAndFlush(
    android::sp<IPipelineContextT> pPipelineContext
) -> MERROR
{
    MERROR err = OK;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP2DrainedAndFlush");
    //P2
    {
        err = waitUntilNodeDrainedAndFlush(eNODEID_P2StreamNode, pPipelineContext);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P2StreamNode);
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
waitUntilP2JpegDrainedAndFlush(
    android::sp<IPipelineContextT> pPipelineContext
) -> MERROR
{
    MERROR err = OK;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP2JpegDrainedAndFlush");
    //P2
    {
        err = waitUntilNodeDrainedAndFlush(eNODEID_P2CaptureNode, pPipelineContext);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P2CaptureNode);
            return err;
        }
    }
    //Jpeg
    {
        err = waitUntilNodeDrainedAndFlush(eNODEID_JpegNode, pPipelineContext);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_JpegNode);
            return err;
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
    NodeId_T const nodeId,
    android::sp<IPipelineContextT> pPipelineContext
)
{
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

MERROR
ThisNamespace::
waitUntilNodeDrained(
    NodeId_T const nodeId,
    android::sp<IPipelineContextT> pPipelineContext
)
{
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }

    MERROR err = pPipelineContext->waitUntilNodeDrained(nodeId);
    if( err != OK )
    {
        MY_LOGW("wait until node(%" PRIdPTR ") drained fail", nodeId);
        return err;
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
    android::RWLock::AutoWLock _l(mRWLock_vCapConfigInfo2);


    if(mpZslProcessor.get())
        mpZslProcessor->onFrameUpdated(requestNo, result);


    if (result.bFrameEnd) {
        mpCaptureInFlightRequest->removeRequest(requestNo);
        mpScenarioCtrl->checkIfNeedExitBoost(result.frameNo, false);
        std::unordered_map<MUINT32, std::shared_ptr<ConfigInfo2>>::iterator it;
        if ((it = mvCapConfigInfo2.find(requestNo)) != mvCapConfigInfo2.end())
            mvCapConfigInfo2.erase(it);
        return;
    }

    StreamId_T streamId = -1L;
    {
        android::RWLock::AutoRLock _l(mRWLock_ConfigInfo2);
        streamId = mConfigInfo2->mvParsedStreamInfo_P1[0].pHalMeta_DynamicP1->getStreamId();
    }

    std::unordered_map<MUINT32, std::shared_ptr<ConfigInfo2>>::iterator it;
    if ((it = mvCapConfigInfo2.find(requestNo)) != mvCapConfigInfo2.end()) {
        streamId = it->second->mvParsedStreamInfo_P1[0].pHalMeta_DynamicP1->getStreamId();
    }
    auto timestampStartOfFrame = determineTimestampSOF(streamId, result.vHalOutMeta);
    updateFrameTimestamp(requestNo, userId, result, timestampStartOfFrame);
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
doCapture(
    std::shared_ptr<pipelinesetting::RequestOutputParams> pReqOutParm,
    std::shared_ptr<ParsedAppRequest>const request,
    std::shared_ptr<IMetadata> pAppControl,
    MUINT32 requestNo
) -> int
{
    HwInfoHelper infoHelper = HwInfoHelper(mStaticInfo.pPipelineStaticInfo->sensorId[0]);
    infoHelper.updateInfos();
    int32_t fps;
    MSize   Sensorsize;
    if ( CC_UNLIKELY( ! infoHelper.getSensorSize(pReqOutParm->sensorMode[0], Sensorsize) ) )
    {
        MY_LOGW("getSensorSize fail");
    }
    if ( CC_UNLIKELY( ! infoHelper.getSensorFps( pReqOutParm->sensorMode[0], fps) ) )
    {
        MY_LOGW("getSensorFps fail");
    }
    IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
    entry.push_back(Sensorsize, Type2Type<MSize>());
    for(size_t i=0;i<pReqOutParm->mainFrame->additionalHal.size();++i)
    {
        pReqOutParm->mainFrame->additionalHal[i]->update(entry.tag(), entry);
        MY_LOGD("Sensor Setting changes, need reconfigure pipeline %d -> %d",
            mSensorMode[0],pReqOutParm->sensorMode[0]);
    }

    auto pConfigInfo2 = std::make_shared<ConfigInfo2>();
    SensorSetting updatedSensorSetting
    {
        .sensorMode                  = pReqOutParm->sensorMode[0],
        .sensorFps                   = static_cast<uint32_t>(fps),
        .sensorSize                  = Sensorsize,
    };

    PipelineNodesNeed captureNodeNeed
    {
        .needP1Node = {true},
        .needP2StreamNode = false,
        .needP2CaptureNode = false,
        .needFDNode = false,
        .needJpegNode = false,
        .needRaw16Node = false,
        .needPDENode = false,
    };

#if MTKCAM_REMOSAIC_UNPACK_INOUTPUT_SUPPORT
    MINT32 fmt = eImgFmt_BAYER10_UNPAK;
#else
    MINT32 fmt = eImgFmt_BAYER10;
#endif

    if(request->pParsedAppImageStreamInfo->vAppImage_Output_RAW16.size() > 0)
    {
        fmt = eImgFmt_BAYER10;
        captureNodeNeed.needRaw16Node = true;
        MY_LOGI("Raw format and Raw16Node needed for Dng");
    }

    if(request->pParsedAppImageStreamInfo->vAppImage_Output_Proc.size() != 0)
    {
        captureNodeNeed.needP2CaptureNode = true;
        captureNodeNeed.needFDNode        = true;
    }

    if(request->pParsedAppImageStreamInfo->pAppImage_Jpeg != nullptr)
    {
        captureNodeNeed.needP2CaptureNode = true;
        captureNodeNeed.needFDNode        = true;
        captureNodeNeed.needJpegNode      = true;
    }

    if(mConfigInfo2->mPipelineNodesNeed.needP1RawIspTuningDataPackNode)
        captureNodeNeed.needP1RawIspTuningDataPackNode = true;

    if(mConfigInfo2->mPipelineNodesNeed.needP2YuvIspTuningDataPackNode)
        captureNodeNeed.needP2YuvIspTuningDataPackNode = true;

    if(mConfigInfo2->mPipelineNodesNeed.needP2RawIspTuningDataPackNode)
        captureNodeNeed.needP2RawIspTuningDataPackNode = true;

    pConfigInfo2->mvSensorSetting.push_back(updatedSensorSetting);
    pConfigInfo2->mPipelineNodesNeed = captureNodeNeed;

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
        .pIsZSLMode                 = nullptr,
    };
    pipelinesetting::ConfigurationInputParams rcgInParam{
        .bypassSensorSetting      = true,
        .bypassNodeNeedPolicy     = true,
    };
    RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateConfiguration(rcfOutParam, rcgInParam), "Fail on Pipeline Reconfiguration" );

    configureDVFSControl(pConfigInfo2->mStreamingFeatureSetting.BWCScenario, pConfigInfo2->mStreamingFeatureSetting.BWCFeatureFlag,pConfigInfo2);

    sp<IImageStreamInfo> imgoStreamInfo = (pConfigInfo2->mvParsedStreamInfo_P1)[0].pHalImage_P1_Imgo;
    if (imgoStreamInfo != nullptr) {
        IImageStreamInfo::BufPlanes_t bufPlanes;
        auto infohelper = IHwInfoHelperManager::get()->getHwInfoHelper(mStaticInfo.pPipelineStaticInfo->sensorId[0]);
        MY_LOGF_IF(infohelper == nullptr, "getHwInfoHelper");
        // Get buffer plane by hwHepler
        bool ret = infohelper->getDefaultBufPlanes_Imgo(bufPlanes, fmt, imgoStreamInfo->getImgSize());
        MY_LOGF_IF(!ret, "IHwInfoHelper::getDefaultBufPlanes_Imgo");

        IImageStreamInfo::BufPlanes_t allocPlanes;
        allocPlanes.count = 1;
        allocPlanes.planes[0].sizeInBytes      = bufPlanes.planes[0].sizeInBytes;
        allocPlanes.planes[0].rowStrideInBytes = bufPlanes.planes[0].sizeInBytes;
        auto pStreamInfo =
            ImageStreamInfoBuilder(imgoStreamInfo.get())
            .setBufPlanes(bufPlanes)
            .setImgFormat(fmt)
            .setAllocBufPlanes(allocPlanes)
            .setAllocImgFormat(eImgFmt_BLOB)
            .build();
        MY_LOGI("Format change %s", pStreamInfo->toString().c_str());
        (pConfigInfo2->mvParsedStreamInfo_P1)[0].pHalImage_P1_Imgo = pStreamInfo;
    }

    //update default Imgo format for config p1node
    pConfigInfo2->mvP1HwSetting[0].imgoDefaultRequest.format = fmt;

    auto AppRaw16Config =  mStaticInfo.pUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_RAW16;
    mStaticInfo.pUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_RAW16.clear();

    BuildPipelineContextInputParams const in{
        .pipelineName               = getSessionName(),
        .pPipelineStaticInfo        = mStaticInfo.pPipelineStaticInfo.get(),
        .pPipelineUserConfiguration = mStaticInfo.pUserConfiguration.get(),
        .pPipelineUserConfiguration2= mStaticInfo.pUserConfiguration2.get(),
        .pParsedStreamInfo_NonP1    = &pConfigInfo2->mParsedStreamInfo_NonP1,
        .pParsedStreamInfo_P1       = &pConfigInfo2->mvParsedStreamInfo_P1,
        .pSensorSetting             = &pConfigInfo2->mvSensorSetting,
        .pvP1HwSetting              = &pConfigInfo2->mvP1HwSetting,
        .pPipelineNodesNeed         = &pConfigInfo2->mPipelineNodesNeed,
        .pPipelineTopology          = &pConfigInfo2->mPipelineTopology,
        .pStreamingFeatureSetting   = &pConfigInfo2->mStreamingFeatureSetting,
        .pCaptureFeatureSetting     = &pConfigInfo2->mCaptureFeatureSetting,
        .batchSize                  = 0,
        .pOldPipelineContext        = nullptr,
        .pDataCallback              = this,
        .bUsingParallelNodeToBuildPipelineContext = true,
        .bUsingMultiThreadToBuildPipelineContext = false,
        .bIsReconfigure             = true,
        .pP1NodeResourceConcurrency = mP1NodeConcurrency,
    };

    {
        android::RWLock::AutoWLock _l(mRWLock_vCapConfigInfo2);
        mvCapConfigInfo2.insert(std::make_pair(requestNo,pConfigInfo2));
    }

    // Create New pipeline context for capture.
    android::sp<IPipelineContextT> pCapturePipelineContext;
    RETURN_ERROR_AEE_IF_NOT_OK( buildPipelineContext(pCapturePipelineContext,in),
        /*CRDISPATCH_KEY*/"PipelineModelSession4Cell::doCapture", "Fail on buildPipelineContext" );

    mStaticInfo.pUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_RAW16 = AppRaw16Config;

    sem_post(&mP1ConfigLock);
    processEvaluatedFrame(
        ProcessEvaluatedFrame{
            .pReqOutputParams   = pReqOutParm,
            .pAppMetaControl    = pAppControl,
            .pAppRequest        = request,
            .pConfigInfo2       = pConfigInfo2,
            .pPipelineContext   = pCapturePipelineContext,
        });

    MBOOL ret = INVALID_OPERATION;
    ret = waitUntilP1NodeDrainedAndFlush(pCapturePipelineContext);
    if (ret!=OK)
    {
        CAM_ULOGME("waitUntilP1NodeDrainedAndFlush - capture Fail!");
        return ret;
    }
    ret = pCapturePipelineContext->waitUntilDrained();
    //ret = waitUntilP2JpegDrainedAndFlush(pCapturePipelineContext);
    if (ret!=OK)
    {
        ALOGE("waitUntilCapturePipeDrained - capture Fail!");
        //return ret;
    }
    //release capture pipeline
    pCapturePipelineContext = NULL;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilCaptureDone() -> int
{
    android::RWLock::AutoWLock _l(mRWLock_vCapFut);
    for( auto &fut : mvCaptureFuture ) {
        int result = fut.get();
        if  ( CC_UNLIKELY( result != OK ) ) {
            MY_LOGE("Fail to finish capture");
            mvCaptureFuture.clear();
            return UNKNOWN_ERROR;
        }
    }
    //
    mvCaptureFuture.clear();
    return OK;
}
