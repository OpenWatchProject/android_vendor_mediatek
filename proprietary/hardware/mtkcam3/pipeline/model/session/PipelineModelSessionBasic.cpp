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

#define LOG_TAG "mtkcam-PipelineModelSessionBasic"
//
#include "PipelineModelSessionBasic.h"
//
#include <impl/PipelineContextBuilder.h>
//
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include "MyUtils.h"
#include <mtkcam/utils/std/ULog.h>
#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>

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
using namespace NSCamHW;
using namespace NSCam::Utils::ULog;
using namespace vendor::mediatek::hardware::power::V2_0;

#define ThisNamespace   PipelineModelSessionBasic


/******************************************************************************
 *
 ******************************************************************************/
namespace {
struct AutoMetaStreamBufferReader
{
public:
    using IMetaStreamBuffer = NSCam::v3::IMetaStreamBuffer;

private:
    android::sp<IMetaStreamBuffer>  mpMetaStreamBuffer;
    IMetadata*                      mMetadata = nullptr;

public:
    IMetadata* get() { return mMetadata; }

    AutoMetaStreamBufferReader(android::sp<IMetaStreamBuffer> pMetaStreamBuffer)
        : mpMetaStreamBuffer(pMetaStreamBuffer)
    {
        if (CC_LIKELY( mpMetaStreamBuffer != nullptr )) {
            mMetadata = mpMetaStreamBuffer->tryReadLock(LOG_TAG);
        }
    }

    ~AutoMetaStreamBufferReader()
    {
        if (CC_LIKELY( mMetadata != nullptr )) {
            mpMetaStreamBuffer->unlock(LOG_TAG, mMetadata);
            mMetadata = nullptr;
        }
    }
};
};

/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace(
    std::string const& name,
    CtorParams const& rCtorParams)
    : PipelineModelSessionBase(
        {name + std::to_string(rCtorParams.staticInfo.pPipelineStaticInfo->openId)},
        rCtorParams)
{
    auto PowerHalService = IPower::getService();
    int32_t coreNum = 0;
    int32_t clusterNum = PowerHalService->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_NUM,0);
    for( int32_t i = 0; i < clusterNum; i++)
    {
        coreNum += PowerHalService->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_CPU_NUM, i);
    }
    MY_LOGD("clusterNum num : %d", clusterNum);
    MY_LOGD("core num : %d", coreNum);

    int32_t threshold = ::property_get_int32("persist.vendor.camera3.buildPipelineContext.ParallelNodes.CPUThreshold", 4);

    if (coreNum <= threshold)
    {
        mbUsingParallelNodeToBuildPipelineContext = false;
        mbUsingMultiThreadToBuildPipelineContext = false;
    }
    else
    {
        mbUsingParallelNodeToBuildPipelineContext = true;
        mbUsingMultiThreadToBuildPipelineContext = true;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
namespace local {
template<typename T>
static android::String8 toString(const std::vector<T>& o)
{
    android::String8 os;
    for (size_t i = 0; i < o.size(); i++) {
        if  (o.size() > 1) { os += android::String8::format(" [%zu] ", i); }
        os += toString(o[i]);
    }
    return os;
};
}
auto
ThisNamespace::
print(android::Printer& printer, ConfigInfo2 const& o __unused)
{
    auto toStringP1DmaNeed = [](std::vector<uint32_t>const& o) -> android::String8 {
        android::String8 os;
        for (size_t i = 0; i < o.size(); i++) {
            auto const v = o[i];
            if  (o.size() > 1) { os += android::String8::format(" [%zu] ", i); }
            os += "{ ";
            if  (v & P1_IMGO)       { os += "IMGO "; }
            if  (v & P1_RRZO)       { os += "RRZO "; }
            if  (v & P1_LCSO)       { os += "LCSO "; }
            if  (v & P1_RSSO)       { os += "RSSO "; }
            if  (v & P1_FDYUV)      { os += "FDYUV "; }
            if  (v & P1_FULLYUV)    { os += "FULLYUV "; }
            if  (v & P1_SCALEDYUV)  { os += "SCALEDYUV "; }
            os += "}";
        }
        return os;
    };

    printer.printFormatLine(".mStreamingFeatureSetting=%s", toString(o.mStreamingFeatureSetting).c_str());
    printer.printFormatLine(".mCaptureFeatureSetting=%s", toString(o.mCaptureFeatureSetting).c_str());
    printer.printFormatLine(".mPipelineNodesNeed=%s", toString(o.mPipelineNodesNeed).c_str());
    printer.printFormatLine(".mPipelineTopology=%s", toString(o.mPipelineTopology).c_str());
    printer.printFormatLine(".mvSensorSetting=%s", local::toString(o.mvSensorSetting).c_str());
    printer.printFormatLine(".mvP1HwSetting=%s", local::toString(o.mvP1HwSetting).c_str());
    printer.printFormatLine(".mvP1DmaNeed=%s", toStringP1DmaNeed(o.mvP1DmaNeed).c_str());
    printer.printFormatLine(".mIsZSLMode=%d", o.mIsZSLMode);
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
    {// ConfigInfo2
        std::shared_ptr<ConfigInfo2> pConfigInfo2 = getCurrentConfigInfo2();
        if  ( pConfigInfo2 != nullptr ) {
            printer.printLine("\n== Current ConfigInfo2 ==");
            android::PrefixPrinter prefixPrinter(printer, "    ");
            print(prefixPrinter, *pConfigInfo2);
        }
    }

    PipelineModelSessionBase::dumpState(printer, options);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getCurrentConfigInfo2() const -> std::shared_ptr<ConfigInfo2>
{
    android::RWLock::AutoRLock _l(mRWLock_ConfigInfo2);
    return mConfigInfo2;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
configure() -> int
{
    // allocate & initialize mConfigInfo2
    RETURN_ERROR_IF_NOT_OK(  onConfig_ConfigInfo2(),
                            "onConfig_ConfigInfo2");

    RETURN_ERROR_IF_NOT_OK(  onConfig_AppImageStreamMaxBufNum(),
                            "onConfig_AppImageStreamMaxBufNum");

    RETURN_ERROR_IF_NOT_OK(  onConfig_Capture(),
                            "onConfig_Capture");

    RETURN_ERROR_IF_NOT_OK(  onConfig_DVFS(),
                            "onConfig_DVFS");

    // before building PipelineContext
    RETURN_ERROR_IF_NOT_OK(  onConfig_BeforeBuildingPipelineContext(),
                            "onConfig_BeforeBuildingPipelineContext");

    // building PipelineContext
    RETURN_ERROR_IF_NOT_OK(  onConfig_BuildingPipelineContext(),
                            "onConfig_BuildingPipelineContext");

    RETURN_ERROR_IF_NOT_OK(  onConfig_Ending(),
                            "onConfig_Ending");

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_ConfigInfo2() -> int
{
    CAM_TRACE_CALL();
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

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_AppImageStreamMaxBufNum() -> int
{
    CAM_TRACE_CALL();
    // App Image Max. Buffer Number
    RETURN_ERROR_IF_NOT_OK(
        mPipelineSettingPolicy->decideConfiguredAppImageStreamMaxBufNum(
            mStaticInfo.pUserConfiguration->pParsedAppImageStreamInfo.get(),
            &mConfigInfo2->mStreamingFeatureSetting,
            &mConfigInfo2->mCaptureFeatureSetting
        ), "Fail on decideConfiguredAppImageStreamMaxBufNum"
    );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_Capture() -> int
{
    CAM_TRACE_CALL();
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_DVFS() -> int
{
    // configure scenario control
    CAM_TRACE_CALL();
    configureDVFSControl(mConfigInfo2->mStreamingFeatureSetting.BWCScenario, mConfigInfo2->mStreamingFeatureSetting.BWCFeatureFlag);

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
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_BuildingPipelineContext() -> int
{
    CAM_TRACE_CALL();
    auto pZslProcessor = getZslProcessor();

    // build pipeline context
    BuildPipelineContextInputParams const in{
        .pipelineName               = getSessionName(),
        .pPipelineStaticInfo        = mStaticInfo.pPipelineStaticInfo.get(),
        .pPipelineUserConfiguration = mStaticInfo.pUserConfiguration.get(),
        .pPipelineUserConfiguration2= mStaticInfo.pUserConfiguration2.get(),
        .pParsedStreamInfo_NonP1    = &mConfigInfo2->mParsedStreamInfo_NonP1,
        .pParsedStreamInfo_P1       = &mConfigInfo2->mvParsedStreamInfo_P1,
        .bIsZslEnabled              = pZslProcessor.get() ? true : false,
        .pSensorSetting             = &mConfigInfo2->mvSensorSetting,
        .pvP1HwSetting              = &mConfigInfo2->mvP1HwSetting,
        .pPipelineNodesNeed         = &mConfigInfo2->mPipelineNodesNeed,
        .pPipelineTopology          = &mConfigInfo2->mPipelineTopology,
        .pStreamingFeatureSetting   = &mConfigInfo2->mStreamingFeatureSetting,
        .pCaptureFeatureSetting     = &mConfigInfo2->mCaptureFeatureSetting,
        .batchSize                  = 0,
        .pOldPipelineContext        = nullptr,
        .pDataCallback              = this,
        .bUsingParallelNodeToBuildPipelineContext = mbUsingParallelNodeToBuildPipelineContext,
        .bUsingMultiThreadToBuildPipelineContext = mbUsingMultiThreadToBuildPipelineContext,
        .bIsReconfigure             = false,
    };

    BuildPipelineContextOutputParams outParam{
        .mvRootNodeOpenIdList       = &mvRootNodeOpenIdList,
    };

    MY_LOGD("pZslProcessor(%p)", pZslProcessor.get());
    RETURN_ERROR_IF_NOT_OK( buildPipelineContext(mCurrentPipelineContext, in, &outParam), "Fail on buildPipelineContext" );

    for(auto&& item:mvRootNodeOpenIdList)
    {
        MY_LOGD("set map root(%#" PRIxPTR ") openId(%d)", item.first, item.second);
    }
    mCurrentPipelineContext->setSessionLifeHolder(mpSessionLifeHolder);
    MY_LOGD("mpSessionLifeHolder:%p ", mpSessionLifeHolder.get());

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfig_Ending() -> int
{
    CAM_TRACE_CALL();
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
configureDVFSControl(
    uint32_t scenario,
    uint32_t featureFlag,
    std::shared_ptr<ConfigInfo2> pUpdatedConfigInfo2
) -> int
{
    auto const& pUserConfiguration          = mStaticInfo.pUserConfiguration;
    auto const& pParsedAppImageStreamInfo   = pUserConfiguration->pParsedAppImageStreamInfo;

    std::shared_ptr<ConfigInfo2> pConfigInfo2;
    if( pUpdatedConfigInfo2 != nullptr)
        pConfigInfo2 = pUpdatedConfigInfo2;
    else
        pConfigInfo2 = getCurrentConfigInfo2();

    mpScenarioCtrl = IScenarioControlV3::create(mStaticInfo.pPipelineStaticInfo->openId);
    IScenarioControlV3::ControlParam param;
    bool isCapture = false;

    // basic component
    HwInfoHelper helper(mStaticInfo.pPipelineStaticInfo->sensorId[0]);
    if (!helper.getSensorOutputSize(pConfigInfo2->mvSensorSetting[0].sensorMode, param.sensorSize))
    {
        param.sensorSize = pConfigInfo2->mvSensorSetting[0].sensorSize;
    }
    param.sensorFps  = pConfigInfo2->mvSensorSetting[0].sensorFps;

    if (scenario != (uint32_t)-1 && scenario != (uint32_t)IScenarioControlV3::Scenario_None)
    {
        param.scenario = scenario;
    }
    else
    {
        if  (pParsedAppImageStreamInfo->hasVideoConsumer) {
            if  (pParsedAppImageStreamInfo->pAppImage_Jpeg.get() || pConfigInfo2->mPipelineNodesNeed.needP2CaptureNode) {
                param.scenario = IScenarioControlV3::Scenario_VSS;
            }
            else {
                param.scenario = IScenarioControlV3::Scenario_VideoRecord;
            }
        }
        else
        {
            if  (
                   pConfigInfo2->mPipelineNodesNeed.needP2CaptureNode
                || pConfigInfo2->mPipelineNodesNeed.needRaw16Node
                || pParsedAppImageStreamInfo->pAppImage_Jpeg.get()
                || pParsedAppImageStreamInfo->vAppImage_Output_RAW16.empty()
                || pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.empty()
                )
            {
                param.scenario = IScenarioControlV3::Scenario_Capture;
                isCapture = true;
            }
            else {
                param.scenario = IScenarioControlV3::Scenario_NormalPreivew;
                MY_LOGD("scenario : %d , preview : %d", param.scenario, IScenarioControlV3::Scenario_NormalPreivew);
            }
        }
    }
    //
    std::vector<IScenarioControlV3::SensorParam> vParams;
    for (size_t i = 0; i < pConfigInfo2->mvSensorSetting.size(); i++)
    {
        IScenarioControlV3::SensorParam Sensor;
        float bbp = 1.25;
        Sensor.sensorMode = pConfigInfo2->mvSensorSetting[i].sensorMode;
        Sensor.id = mStaticInfo.pPipelineStaticInfo->sensorId[i];
        Sensor.fps = pConfigInfo2->mvSensorSetting[i].sensorFps;
        Sensor.sensorSize = pConfigInfo2->mvSensorSetting[i].sensorSize.size();
        //Sensor.pdSize = 0;
        Sensor.videoSize = pParsedAppImageStreamInfo->videoImageSize.size();
        //Sensor.vbRatio = 0;
        //Sensor.vbTime = 0;
        Sensor.P1RawIn = 0;
        if ( i < pConfigInfo2->mvParsedStreamInfo_P1.size() )
        {
            Sensor.P1YuvOut =  ( pConfigInfo2->mvParsedStreamInfo_P1[i].pHalImage_P1_FDYuv != nullptr
                              || pConfigInfo2->mvParsedStreamInfo_P1[i].pHalImage_P1_ScaledYuv != nullptr);
            if (isCapture && pConfigInfo2->mvParsedStreamInfo_P1[i].pHalImage_P1_Imgo != nullptr)
            {
                bbp = (float)(pConfigInfo2->mvParsedStreamInfo_P1[i].pHalImage_P1_Imgo->getAllocBufPlanes().planes[0].sizeInBytes) /
                        pConfigInfo2->mvParsedStreamInfo_P1[i].pHalImage_P1_Imgo->getImgSize().w /
                        pConfigInfo2->mvParsedStreamInfo_P1[i].pHalImage_P1_Imgo->getImgSize().h;
                if (bbp > 2.0)
                {
                    bbp = 2.0;
                }
            }
        }
        else
        {
            Sensor.P1YuvOut = 0;
        }
        Sensor.raw_bbp = bbp;
        vParams.push_back(Sensor);
    }
    //
    mpScenarioCtrl->setQOSParams(vParams);
    param.featureFlag = featureFlag;
    mpScenarioCtrl->enterScenario(param);
    return OK;
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
    CAM_TRACE_CALL();

    auto const requestNo = request->requestNo;

    CAM_ULOG_ENTER(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, requestNo);

    // On this function, use this local variable to serve the request.
    auto pConfigInfo2 = getCurrentConfigInfo2();

    // Evaluate a result for a request.
    auto pReqOutputParams = std::make_shared<pipelinesetting::RequestOutputParams>();
    MY_LOGF_IF(pReqOutputParams==nullptr, "[requestNo:%u] Fail on make_shared<pipelinesetting::RequestOutputParams>", requestNo);

    RETURN_ERROR_IF_NOT_OK( onRequest_EvaluateRequest(*pReqOutputParams, request, pConfigInfo2),
            "[requestNo:%u] onRequest_EvaluateRequest", requestNo );

    // pConfigInfo2 could change after this call.
    RETURN_ERROR_IF_NOT_OK( onRequest_Reconfiguration(pConfigInfo2, *pReqOutputParams, request),
            "[requestNo:%u] onRequest_Reconfiguration", requestNo );

    RETURN_ERROR_IF_NOT_OK( onRequest_ProcessEvaluatedFrame(pReqOutputParams, request, pConfigInfo2),
            "[requestNo:%u] onRequest_ProcessEvaluatedFrame", requestNo );

    RETURN_ERROR_IF_NOT_OK( onRequest_Ending(*pReqOutputParams),
            "[requestNo:%u] onRequest_Ending", requestNo );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onRequest_EvaluateRequest(
    policy::pipelinesetting::RequestOutputParams& reqOutput,
    std::shared_ptr<ParsedAppRequest>const& pRequest,
    std::shared_ptr<ConfigInfo2>const& pConfigInfo2
) -> int
{
    CAM_TRACE_CALL();

    /**
     * The following data members are used on this function:
     *  - mSensorMode, mSensorSize, mPipelineSettingPolicy
     */

    // Evaluate a result for a request.
    {
        AutoMetaStreamBufferReader autoAppMetaControl(pRequest->pAppMetaControlStreamBuffer);
        RETURN_ERROR_IF_NULLPTR(autoAppMetaControl.get(), -ENODEV,
                "[requestNo:%u] autoAppMetaControl.get()==nullptr", pRequest->requestNo );

        pipelinesetting::RequestInputParams const in{
            .requestNo                          = pRequest->requestNo,
            .pRequest_AppImageStreamInfo        = pRequest->pParsedAppImageStreamInfo.get(),
            .pRequest_AppControl                = autoAppMetaControl.get(),
            .pRequest_ParsedAppMetaControl      = pRequest->pParsedAppMetaControl.get(),
            .pConfiguration_PipelineNodesNeed   = &pConfigInfo2->mPipelineNodesNeed,
            .pConfiguration_StreamInfo_NonP1    = &pConfigInfo2->mParsedStreamInfo_NonP1,
            .pConfiguration_StreamInfo_P1       = &pConfigInfo2->mvParsedStreamInfo_P1,
            .pSensorMode                        = &mSensorMode,
            .pSensorSize                        = &mSensorSize,
            .isZSLMode                          = pConfigInfo2->mIsZSLMode,
            .pP1HwSetting                       = &pConfigInfo2->mvP1HwSetting,
        };
        RETURN_ERROR_IF_NOT_OK( mPipelineSettingPolicy->evaluateRequest(reqOutput, in),
                "[requestNo:%u] IPipelineSettingPolicy::evaluateRequest", pRequest->requestNo );
    }

#if 0
    // Log some debug information
    if ( reqOutput.subFrames.size()
      || reqOutput.preDummyFrames.size()
      || reqOutput.postDummyFrames.size() )
    {
        MY_LOGI("[requestNo:%u] multi-frame request: sub:#%zu preDummy:#%zu postDummy:#%zu needZslFlow:%d",
            pRequest->requestNo,
            reqOutput.subFrames.size(),
            reqOutput.preDummyFrames.size(),
            reqOutput.postDummyFrames.size(),
            reqOutput.needZslFlow);
    }
#endif
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

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onRequest_ProcessEvaluatedFrame(
    std::shared_ptr<policy::pipelinesetting::RequestOutputParams>const& pReqOutputParams __unused,
    std::shared_ptr<ParsedAppRequest>const& pRequest __unused,
    std::shared_ptr<ConfigInfo2>const& pConfigInfo2 __unused
) -> int
{
    CAM_TRACE_CALL();

    // Make a copy of the original App Meta Control
    //
    // The App metadata controls of main frame, sub frame, dummy frames could be appended
    // during the following procedure, so we have to ensure their correctness via making
    // a copy of the original App metadata control.
    auto pAppMetaControl = std::make_shared<IMetadata>(); // original app control
    {
        AutoMetaStreamBufferReader autoAppMetaControl(pRequest->pAppMetaControlStreamBuffer);
        RETURN_ERROR_IF_NULLPTR( autoAppMetaControl.get(), -ENODEV,
            "[requestNo:%d] autoAppMetaControl.get()==nullptr", pRequest->requestNo );
        *pAppMetaControl = *autoAppMetaControl.get();
    }

    auto pPipelineContext = getCurrentPipelineContext();
    int res = processEvaluatedFrame(
                ProcessEvaluatedFrame{
                    .pReqOutputParams   = pReqOutputParams,
                    .pAppMetaControl    = pAppMetaControl,
                    .pAppRequest        = pRequest,
                    .pConfigInfo2       = pConfigInfo2,
                    .pPipelineContext   = pPipelineContext,
                });

    RETURN_ERROR_IF_NOT_OK( res, "[requestNo:%u] processEvaluatedFrame", pRequest->requestNo );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onRequest_Ending(
    policy::pipelinesetting::RequestOutputParams const& reqOutput __unused
) -> int
{
    CAM_TRACE_CALL();

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
processEvaluatedFrame(
    ProcessEvaluatedFrame const& in __unused
) -> int
{
    CAM_TRACE_CALL();

    auto const& reqOutput = *in.pReqOutputParams;
    auto const& request = in.pAppRequest;

    RETURN_ERROR_IF_NULLPTR(in.pPipelineContext, -ENODEV, "[requestNo:%u] Bad PipelineContext", request->requestNo);

    // Log some debug information
    if ( reqOutput.subFrames.size()
      || reqOutput.preSubFrames.size()
      || reqOutput.preDummyFrames.size()
      || reqOutput.postDummyFrames.size() )
    {
        MY_LOGI("[requestNo:%u] multi-frame request: sub:#%zu preSub:#%zu preDummy:#%zu postDummy:#%zu needZslFlow:%d",
            request->requestNo,
            reqOutput.subFrames.size(),
            reqOutput.preSubFrames.size(),
            reqOutput.preDummyFrames.size(),
            reqOutput.postDummyFrames.size(),
            reqOutput.needZslFlow);
    }

    ////////////////////////////////////////////////////////////////////////////
    // process each frame
    if (reqOutput.mainFrame->nodesNeed.needJpegNode)
    {
        auto pCaptureInFlightRequest = getCaptureInFlightRequest();
        if ( pCaptureInFlightRequest != nullptr ) {
            pCaptureInFlightRequest->insertRequest(request->requestNo, eMSG_INFLIGHT_NORMAL);
        }
        IPreReleaseRequestMgr::getInstance()->addCaptureRequest(request->requestNo, !reqOutput.bCshotRequest);
    }

    uint32_t lastFrameNo = 0;
    RETURN_ERROR_IF_NOT_OK( onProcessEvaluatedFrame(lastFrameNo, in),
            "[requestNo:%u] onProcessEvaluatedFrame failed", request->requestNo);

    for (auto& control : reqOutput.vboostControl)
    {
        if (control.boostScenario != -1 && control.boostScenario != (int32_t)IScenarioControlV3::Scenario_None)
        {
            mpScenarioCtrl->boostScenario(control.boostScenario, control.featureFlag, lastFrameNo);
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onMetaResultAvailable(MetaResultAvailable&& arg) -> void
{
    CAM_TRACE_CALL();

    {
        auto pCallback = mPipelineModelCallback.promote();
        if ( CC_UNLIKELY( pCallback == nullptr )) {
            MY_LOGE("Bad IPipelineModelCallback");
            return;
        }

        switch ( arg.frame->getGroupFrameType() )
        {
        case NSPipelineContext::GroupFrameType::MAIN:
            pCallback->onMetaResultAvailable(
                   UserOnMetaResultAvailable{
                    .resultMetadata = arg.resultMetadata,
                    .requestNo = arg.frame->getRequestNo(),
                    .callerName = std::move(arg.callerName),
                });
            break;
        default:
            break;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ThisNamespace::
onNextCaptureCallBack(
    MUINT32   requestNo __unused,
    MINTPTR   nodeId __unused,
    MUINT32   requestCnt __unused,
    MBOOL     bSkipCheck __unused
)
{
    //MY_LOGD("requestNo:%u nodeId:%" PRIxPTR " - Not Supported", requestNo, nodeId);
}

