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
#define LOG_TAG "mtkcam-FeatureSettingPolicy"
//
#include <algorithm>
#include <tuple>
// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// MTKCAM3
#include <mtkcam3/feature/hdrDetection/Defs.h>
#include <mtkcam3/feature/lmv/lmv_ext.h>
#include <mtkcam3/feature/eis/eis_ext.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include <mtkcam3/feature/utils/FeatureProfileHelper.h>
#include <mtkcam3/feature/fsc/fsc_defs.h>
#include <mtkcam3/feature/vhdr/HDRPolicyHelper.h>
#include <camera_custom_eis.h>
#include <camera_custom_3dnr.h>
#include <camera_custom_fsc.h>
// dual cam
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <camera_custom_stereo.h>
#include <isp_tuning.h>
#include <mtkcam/aaa/aaa_hal_common.h>
//
#include "FeatureSettingPolicy.h"
#include <mtkcam/utils/hw/IScenarioControlV3.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FEATURE_SETTING_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::featuresetting;
using namespace NSCam::v3::pipeline::policy::scenariomgr;
using namespace NSCam::v3::pipeline::model;
using namespace NS3Av3; //IHal3A
using namespace NSCamHW; //HwInfoHelper
using namespace NSCam::NR3D;
using namespace NSCam::FSC;
using namespace NSCam::v3::pipeline::model;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

// TODO: rename
#define SENSOR_INDEX_MAIN   (0)
#define SENSOR_INDEX_SUB1   (1)
#define SENSOR_INDEX_SUB2   (2)
#define DEBUG_EISEM  (0)
#define DEBUG_EIS30  (0)
#ifndef NR3D_SUPPORTED
#define FORCE_3DNR   (0)
#else
#define FORCE_3DNR   (1)
#endif
#define DEBUG_TSQ    (0)

static inline MSize max(const MSize &lhs, const MSize &rhs)
{
    return lhs.size() > rhs.size() ? lhs : rhs;
}


auto
FeatureSettingPolicy::
updateStreamData(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> bool
{
    //CAM_ULOGM_APILIFE();
    // 1. Update stream state : decide App Mode
    MINT32 index = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    MY_LOGD_IF(mbDebug, "index=%d", index);
    int32_t recordState = -1;
    uint32_t AppMode = 0;
    bool isRepeating = in->pRequest_ParsedAppMetaControl->repeating;
    IMetadata const* pAppMetaControl = in->pRequest_AppControl;

    auto pParsedAppConfiguration = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;
    auto pParsedSMVRBatchInfo = (pParsedAppConfiguration != nullptr) ? pParsedAppConfiguration->pParsedSMVRBatchInfo : nullptr;

    if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_STREAMING_FEATURE_RECORD_STATE, recordState) )
    {   // App has set recordState Tag
        if(recordState == MTK_STREAMING_FEATURE_RECORD_STATE_PREVIEW)
        {
            if(in->pRequest_AppImageStreamInfo->hasVideoConsumer)
                AppMode = MTK_FEATUREPIPE_VIDEO_STOP;
            else
                AppMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
        }
        else
        {
            AppMode = mConfigOutputParams.StreamingParams.mLastAppInfo.appMode;
            MY_LOGW("Unknown or Not Supported app recordState(%d), use last appMode=%d",
                    recordState, mConfigOutputParams.StreamingParams.mLastAppInfo.appMode);
        }
    }
    else
    {   // App has NOT set recordState Tag
        // (slow motion has no repeating request)
        // no need process slow motion because use different policy?
        if( isRepeating /*|| isHighspped*/
            || pParsedSMVRBatchInfo != nullptr // SMVRBatch: need per-frame correct
            )
        {
            if(in->pRequest_AppImageStreamInfo->hasVideoConsumer)
                AppMode = MTK_FEATUREPIPE_VIDEO_RECORD;
            else if (in->Configuration_HasRecording)
                AppMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
            else
                AppMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
        }
        else
        {
            AppMode = mConfigOutputParams.StreamingParams.mLastAppInfo.appMode;
        }
    }

    MetadataPtr pOutMetaHal = std::make_shared<IMetadata>();
    MetadataPtr pOutMetaApp = std::make_shared<IMetadata>();
    IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_FEATUREPIPE_APP_MODE, AppMode);
    // 2. Update EIS data
    auto isEISOn = [&] (void) -> bool
    {
        MUINT8 appEisMode = 0;
        MINT32 advEisMode = 0;
        if( ( IMetadata::getEntry<MUINT8>(pAppMetaControl, MTK_CONTROL_VIDEO_STABILIZATION_MODE, appEisMode) &&
              appEisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON ) ||
            ( IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_EIS_FEATURE_EIS_MODE, advEisMode) &&
                 advEisMode == MTK_EIS_FEATURE_EIS_MODE_ON ) )
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    if( isEISOn() )
    {
        IMetadata::setEntry<MSize>(pOutMetaHal.get(), MTK_EIS_VIDEO_SIZE, in->pRequest_AppImageStreamInfo->videoImageSize);
    }

    if( AppMode != mConfigOutputParams.StreamingParams.mLastAppInfo.appMode ||
        recordState != mConfigOutputParams.StreamingParams.mLastAppInfo.recordState ||
        isEISOn() != mConfigOutputParams.StreamingParams.mLastAppInfo.eisOn )
    {
        MY_LOGI("AppInfo changed:appMode(%d=>%d),recordState(%d=>%d),eisOn(%d=>%d)",
                mConfigOutputParams.StreamingParams.mLastAppInfo.appMode, AppMode,
                mConfigOutputParams.StreamingParams.mLastAppInfo.recordState, recordState,
                mConfigOutputParams.StreamingParams.mLastAppInfo.eisOn, isEISOn());
        mConfigOutputParams.StreamingParams.mLastAppInfo.appMode = AppMode;
        mConfigOutputParams.StreamingParams.mLastAppInfo.recordState = recordState;
        mConfigOutputParams.StreamingParams.mLastAppInfo.eisOn = isEISOn();
    }

    // 2. Decide override timestamp mechanism or not
    if( mConfigOutputParams.StreamingParams.bEnableTSQ )
    {
        //MBOOL needOverrideTime = isEisQEnabled(pipelineParam.currentAdvSetting);
        // Now change to use Gralloc Extra to set timestamp
        //IMetadata::setEntry<MBOOL>(pOutMetaHal.get(), MTK_EIS_NEED_OVERRIDE_TIMESTAMP, 1);
        //MY_LOGD("TSQ ON");
    }
    uint32_t needP1Dma = 0;
    if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo != nullptr)
    {
        needP1Dma |= P1_RRZO;
    }
    auto needImgo = [this] (MSize imgSize, MSize rrzoSize) -> int
    {
        // if isZslMode=true, must output IMGO for ZSL buffer pool
        return (imgSize.w > rrzoSize.w) || (imgSize.h > rrzoSize.h) || mConfigOutputParams.isZslMode;
    };
    if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Imgo != nullptr &&
        needImgo(in->maxP2StreamSize, (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getImgSize()))
    {
        needP1Dma |= P1_IMGO;
    }
    if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Lcso != nullptr)
    {
        needP1Dma |= P1_LCSO;
    }
    if ((*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rsso != nullptr)
    {
        needP1Dma |= P1_RSSO;
    }

    if (pParsedSMVRBatchInfo != nullptr) // SMVRBatch
    {
        IMetadata::IEntry const& entry = pAppMetaControl->entryFor(MTK_CONTROL_AE_TARGET_FPS_RANGE);
        if  ( entry.isEmpty() )
        {
            MY_LOGW("SMVRBatch: no MTK_CONTROL_AE_TARGET_FPS_RANGE");
        }
        else
        {
            MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
            MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
            MUINT8 fps = i4MinFps == 30  ? MTK_SMVR_FPS_30 :
                         i4MinFps == 120 ? MTK_SMVR_FPS_120 :
                         i4MinFps == 240 ? MTK_SMVR_FPS_240 :
                         i4MinFps == 480 ? MTK_SMVR_FPS_480 :
                         i4MinFps == 960 ? MTK_SMVR_FPS_960 : MTK_SMVR_FPS_30;

            IMetadata::setEntry<MUINT8>(pOutMetaHal.get(), MTK_HAL_REQUEST_SMVR_FPS, fps);
            MY_LOGD_IF(2 <= pParsedSMVRBatchInfo->logLevel,
                "SMVRBatch: requestNo=%d, AppMode=%d, i4MinFps=%d, i4MaxFps=%d, SMVR_FPS=%d",
                in->requestNo, AppMode, i4MinFps, i4MaxFps, fps);
        }
    }
    else if (mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->isConstrainedHighSpeedMode)
    {
        // SMVRConstraint
        IMetadata::IEntry const& entry = pAppMetaControl->entryFor(MTK_CONTROL_AE_TARGET_FPS_RANGE);
        if  ( entry.isEmpty() )
        {
            MY_LOGW("SMVRConstraint: no MTK_CONTROL_AE_TARGET_FPS_RANGE");
        }
        else
        {
            MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
            MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
            MUINT8 fps = i4MinFps == 30  ? MTK_SMVR_FPS_30 :
                         i4MinFps == 120 ? MTK_SMVR_FPS_120 :
                         i4MinFps == 240 ? MTK_SMVR_FPS_240 :
                         i4MinFps == 480 ? MTK_SMVR_FPS_480 :
                         i4MinFps == 960 ? MTK_SMVR_FPS_960 : MTK_SMVR_FPS_30;

            IMetadata::setEntry<MUINT8>(pOutMetaHal.get(), MTK_HAL_REQUEST_SMVR_FPS, fps);

            MINT32 postDummyReqs = 0;
            if (mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->isConstrainedHighSpeedMode)
            {
                // only smvrConstraint need dummy frames for p1 dma
                postDummyReqs = i4MinFps == 30 ? (i4MaxFps/i4MinFps -1) : 0;
                if (postDummyReqs)
                {
                    std::shared_ptr<RequestResultParams> postDummyFrame = nullptr;
                    updateRequestResultParams(postDummyFrame, nullptr, nullptr, needP1Dma, index);
                    for (MINT32 i = 0 ; i < postDummyReqs ; i++) {
                        out->postDummyFrames.push_back(postDummyFrame);
                    }
                }
            }
            MY_LOGD_IF(2 <= mbDebug,
                "SMVRConstraint: requestNo=%d, AppMode=%d, i4MinFps=%d, i4MaxFps=%d, SMVR_FPS=%d",
                in->requestNo, AppMode, i4MinFps, i4MaxFps, fps);
        }
    }
    else
    {
        MY_LOGD_IF(2 <= mbDebug, "No need to update SMVRBatch or SMVRConstraint fps policy");
    }

    MINT32 appHDRMode = 0;
    bool isAppReprocess = ( in->pRequest_AppImageStreamInfo->pAppImage_Input_Yuv.get() ||
                            in->pRequest_AppImageStreamInfo->pAppImage_Input_Priv.get() ||
                            in->pRequest_AppImageStreamInfo->pAppImage_Input_RAW16.get() );
    if (isAppReprocess || in->needP2CaptureNode) {
        mHDRHelper->updateAppRequestMode(static_cast<HDRMode>(0/*bypass*/), AppMode);
    }
    else if (IMetadata::getEntry<MINT32>(pAppMetaControl, MTK_HDR_FEATURE_HDR_MODE, appHDRMode)) {
        mHDRHelper->updateAppRequestMode(static_cast<HDRMode>((uint8_t)appHDRMode), AppMode);
    }
    if (mHDRHelper->isHDR()) {
        IMetadata::setEntry<MUINT8>(pOutMetaHal.get(), MTK_3A_HDR_MODE, static_cast<MUINT8>(mHDRHelper->getHDRAppMode()));
        IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_HDR_FEATURE_HDR_HAL_MODE, mHDRHelper->getHDRHalRequestMode(HDRPolicyHelper::HDR_REQUEST_PREVIEW));
        if (mHDRHelper->isMulitFrameHDR()) {
            IMetadata::setEntry<MINT32>(pOutMetaHal.get(), MTK_3A_FEATURE_AE_EXPOSURE_LEVEL,
                                        mHDRHelper->getHDRExposure(FRAME_TYPE_MAIN));
        }
    }

    // prepare Stream Size
    auto rrzoSize = (*(in->pConfiguration_StreamInfo_P1))[index].pHalImage_P1_Rrzo->getImgSize();
    // if (isEISOn() || mConfigOutputParams.StreamingParams.targetRrzoRatio > 1.0f)
    {
        // RRZO should be the same size
        out->fixedRRZOSize = rrzoSize;
    }

    //proflie and MTK_3A_HDR_MODE
    MINT32 fMask = ProfileParam::FMASK_NONE;

    if(isEISOn())
    {
        fMask |= ProfileParam::FMASK_EIS_ON;
    }

    ProfileParam profileParam(
        rrzoSize,//stream size
        mHDRHelper->getHDRHalRequestMode(HDRPolicyHelper::HDR_REQUEST_PREVIEW), //hdrHalmode
        0,       //sensormode
        ProfileParam::FLAG_NONE, // TODO set flag by isZSDPureRawStreaming or not
        fMask
    );

    MUINT8 profile = 0;
    // Prepare query Feature Streaming ISP Profile
    if (!FeatureProfileHelper::getStreamingProf(profile, profileParam))
    {
        // Didn't use FeatureProfileHelper isp Profile, because is not VHDR/EIS feature.
        if(in->Configuration_HasRecording)
        {
            profile = NSIspTuning::EIspProfile_Video;
        }
        else
        {
            profile = NSIspTuning::EIspProfile_Preview;
        }
    }
    MY_LOGD_IF(mHDRHelper->getDebugLevel(), "updateStreamData IspProfile(%d)", profile);
    IMetadata::setEntry<MUINT8>(pOutMetaHal.get(), MTK_3A_ISP_PROFILE, profile);

    updateRequestResultParams(out->mainFrame,
                              pOutMetaApp,
                              pOutMetaHal,
                              needP1Dma,
                              index);

    if (!updateHDRMultiFrame(out, in, pOutMetaApp, pOutMetaHal)) {
        MY_LOGE("updateHDRFrame failed");
    }

    return true;
}

auto
FeatureSettingPolicy::
evaluateStreamSetting(
    RequestOutputParams* out,
    RequestInputParams const* in
) -> bool
{
    if (in->needP2StreamNode) {

        CAM_TRACE_CALL();
        //CAM_ULOGM_APILIFE();

        bool isCaptureUpdate = false;
        SensorInfoUpdater updater(in, isCaptureUpdate, mPolicyParams, &mSensorInfoList);

        if (in->needP2CaptureNode) {
            /**************************************************************
             * NOTE:
             * In this stage,
             * MTK_3A_ISP_PROFILE and sensor setting has been configured
             * for capture behavior.
             *************************************************************/
            // stream policy with capture policy and behavior.
            // It may occurs some quality limitation duting captue behavior.
            // For example, change sensor mode, 3A sensor setting, P1 ISP profile.
            // Capture+streaming feature combination policy
            // TODO: implement for customized streaming feature setting evaluate with capture behavior
            MY_LOGD("not yet implement for stream feature setting evaluate with capture behavior");
        }
        else {
            // TODO: only porting some streaming feature, temporary.
            // not yet implement all stream feature setting evaluate
            updateStreamData(out, in);
        }

        // update dual cam preview
        if (DualDevicePath::MultiCamControl == mDualDevicePath) {
            if(mMultiCamStreamingUpdater)
            {
                mMultiCamStreamingUpdater(out, in);
            }
            else
            {
                MY_LOGD("mMultiCamStreamingUpdater is nullptr");
            }
        }

        MY_LOGD_IF(2 <= mbDebug, "stream request frames count(mainFrame:%d, subFrames:%zu, preDummyFrames:%zu, postDummyFrames:%zu)",
                (out->mainFrame.get() != nullptr), out->subFrames.size(),
                out->preDummyFrames.size(), out->postDummyFrames.size());
    }
    return true;
}

auto
FeatureSettingPolicy::
evaluateStreamConfiguration(
    ConfigurationOutputParams* out,
    ConfigurationInputParams const* in __unused
) -> bool
{
    CAM_TRACE_CALL();
    CAM_ULOGM_APILIFE();
    auto const& pParsedAppConfiguration   = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;
    auto const& pParsedAppImageStreamInfo = mPolicyParams.pPipelineUserConfiguration->pParsedAppImageStreamInfo;
    auto const& pPipelineStaticInfo       = mPolicyParams.pPipelineStaticInfo;
    auto const& pParsedMultiCamInfo        = pParsedAppConfiguration->pParsedMultiCamInfo;
    MINT32 forceEisEMMode    = ::property_get_int32("vendor.debug.eis.EMEnabled", DEBUG_EISEM);
    MINT32 forceEis30        = ::property_get_int32("vendor.debug.camera.hal3.eis30", DEBUG_EIS30);
    MINT32 forceTSQ          = ::property_get_int32("vendor.debug.camera.hal3.tsq", DEBUG_TSQ);
    MINT32 force3DNR         = ::property_get_int32(NR3D_PROP_3DNR_ENABLE, FORCE_3DNR);//default on

    sp<IImageStreamInfo> pStreamInfo;
    MSize dispSize, recSize, extraSize, largeSize;
    auto pMetadataProvider = NSMetadataProviderManager::valueForByDeviceId(pPipelineStaticInfo->openId);
    IMetadata::IEntry entryMinDuration = pMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
    for (const auto& n : pParsedAppImageStreamInfo->vAppImage_Output_Proc)
    {
        if ((pStreamInfo = n.second).get())
        {
            if (pStreamInfo->getUsageForConsumer() & (GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_TEXTURE))
            {
                dispSize = pStreamInfo->getImgSize();
            }
            else if (pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER)
            {
                recSize = pStreamInfo->getImgSize();
            }
            else
            {
                MINT64 streamDuration = 0;
                if (getStreamDuration(entryMinDuration, pStreamInfo->getOriImgFormat(), pStreamInfo->getImgSize(), streamDuration))
                {
                    if (streamDuration <= 33333333)
                    {
                        extraSize = max(extraSize, pStreamInfo->getImgSize());
                    }
                    else
                    {
                        largeSize = max(largeSize, pStreamInfo->getImgSize());
                    }
                }
            }
        }
    }
    MY_LOGI("set MdpTargetSize : dispSize(%dx%d), recSize(%dx%d), extraSize(%dx%d), largeSize(%dx%d)",
        dispSize.w, dispSize.h, recSize.w, recSize.h, extraSize.w, extraSize.h, largeSize.w, largeSize.h);
    mMdpTargetSize = dispSize.size() ? dispSize :
                     recSize.size() ? recSize :
                     extraSize.size() ? extraSize :
                     largeSize;

    auto pParsedSMVRBatchInfo = (pParsedAppConfiguration != nullptr) ? pParsedAppConfiguration->pParsedSMVRBatchInfo : nullptr;

    if(::needControlMmdvfs() && (miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF))
    {
        MY_LOGD("vsdof enable bwc control");
        out->StreamingParams.BWCScenario = IScenarioControlV3::Scenario_ContinuousShot;
        FEATURE_CFG_ENABLE_MASK(
                        out->StreamingParams.BWCFeatureFlag,
                        IScenarioControlV3::FEATURE_VSDOF_PREVIEW);
    }
    // query features by scenario during config
    ScenarioFeatures scenarioFeatures;
    StreamingScenarioConfig scenarioConfig;
    ScenarioHint scenarioHint;
    toTPIDualHint(scenarioHint);
    scenarioHint.isVideoMode = pParsedAppImageStreamInfo->hasVideoConsumer;
    scenarioHint.operationMode = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->operationMode;
    //TODO:
    //scenarioHint.captureScenarioIndex = ?   /* hint from vendor tag */
    //scenarioHint.streamingScenarioIndex = ? /* hint from vendor tag */
    int32_t openId = pPipelineStaticInfo->openId;
    auto pAppMetadata = &mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->sessionParams;

    int32_t scenario = -1;
    if (!get_streaming_scenario(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata)) {
        MY_LOGE("cannot get streaming scenario");
        return false;
    }
    else {
        MY_LOGD("find scenario:%s for (openId:%d, scenario:%d)",
                scenarioFeatures.scenarioName.c_str(), openId, scenario);
    }

    for (auto featureSet : scenarioFeatures.vFeatureSet) {
        MY_LOGI("scenario(%s) support feature:%s(%#" PRIx64"), feature combination:%s(%#" PRIx64")",
                scenarioFeatures.scenarioName.c_str(),
                featureSet.featureName.c_str(),
                static_cast<MINT64>(featureSet.feature),
                featureSet.featureCombinationName.c_str(),
                static_cast<MINT64>(featureSet.featureCombination));
        out->StreamingParams.supportedScenarioFeatures |= (featureSet.feature | featureSet.featureCombination);
    }

    for(MUINT32 margin : scenarioConfig.fixedMargins)
    {
        if(margin <= 0 || margin > 80)
        {
            MY_LOGE("invalid streaming scenario margin(%d)", margin);
        }
        else
        {
            out->StreamingParams.targetRrzoRatio *= float(100 + margin) / 100.0f;
            MY_LOGI("add margin (%d)", margin);
        }
    }
    MY_LOGI("support features:%#" PRIx64", rrz ratio(%f), marginList(%zu), MdpTargetSize(%dx%d)", out->StreamingParams.supportedScenarioFeatures, out->StreamingParams.targetRrzoRatio, scenarioConfig.fixedMargins.size(), mMdpTargetSize.w, mMdpTargetSize.h);

    // HDR
    if( !evaluateHDRConfiguration(out, in)) {
        MY_LOGE("evaluateHDRConfiguration failed");
    }

    // ISP6.0 FD
    if ( pParsedAppConfiguration->operationMode != 1/* CONSTRAINED_HIGH_SPEED_MODE */ && in->isP1DirectFDYUV )
    {
        if (::property_get_int32("vendor.debug.camera.fd.disable", 0) == 0)
        {
            out->StreamingParams.bNeedP1FDYUV = 1;
        }
    }

    // ISP6.0 scaled yuv
    if ( miMultiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF && in->isP1DirectScaledYUV )
    {
        out->StreamingParams.bNeedP1ScaledYUV = 1;
    }

    // === 3DNR ===
    out->StreamingParams.nr3dMode = 0;
    MINT32 e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    MBOOL isAPSupport3DNR = MFALSE;
    if (IMetadata::getEntry<MINT32>(&pParsedAppConfiguration->sessionParams, MTK_NR_FEATURE_3DNR_MODE, e3DnrMode) &&
        e3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON) {
        isAPSupport3DNR = MTRUE;
    }
    if (force3DNR) {
        out->StreamingParams.nr3dMode |= E3DNR_MODE_MASK_UI_SUPPORT;
        out->StreamingParams.nr3dMode |= E3DNR_MODE_MASK_HAL_FORCE_SUPPORT;
    }

    // rule: 4k2k --> 3dnr off: move to P2Node layer -
    // rule: slow motion--> 3dnr off
    if ( pParsedAppConfiguration->operationMode == 1 /* smvrC */
        || pParsedSMVRBatchInfo != nullptr )
    {
        out->StreamingParams.nr3dMode = 0;
    }

    if( E3DNR_MODE_MASK_ENABLED(out->StreamingParams.nr3dMode, (E3DNR_MODE_MASK_UI_SUPPORT | E3DNR_MODE_MASK_HAL_FORCE_SUPPORT))) {
        if (::property_get_int32(NR3D_PROP_SL2E_ENABLE, 1)) {
            out->StreamingParams.nr3dMode |= E3DNR_MODE_MASK_SL2E_EN;
        }

        if (1)
        {
            MUINT32 nr3d_mask = NR3DCustom::USAGE_MASK_NONE;
            // cache dual cam feature mode for later check.
            MINT32 iDualFeatureMode = -1;
            if(pParsedMultiCamInfo != nullptr){
                iDualFeatureMode = pParsedMultiCamInfo->mDualFeatureMode;
            }

            if (pParsedAppConfiguration->operationMode == 1 /* smvrC */
                || pParsedSMVRBatchInfo != nullptr /* smvrB */
               )
            {
                nr3d_mask |= NR3DCustom::USAGE_MASK_HIGHSPEED;
            }
            if (pPipelineStaticInfo->isDualDevice ||
                iDualFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_ZOOM) {
                nr3d_mask |= NR3DCustom::USAGE_MASK_DUAL_ZOOM;
            }
            if (NR3DCustom::isEnabledRSC(nr3d_mask)) {
                out->StreamingParams.nr3dMode |= E3DNR_MODE_MASK_RSC_EN;
            }
            if (NR3DCustom::is3DNRSmvrBatchSupported()) {
                out->StreamingParams.nr3dMode |= E3DNR_MODE_MASK_SMVR_EN;
            }
        }
    }

    MY_LOGD("3DNR mode : %d, meta c(%d), force(%d) ap(%d) dual(%d) stereo(%d) 3dnr-dsdn(%d) op(%d)",
        out->StreamingParams.nr3dMode, pParsedAppConfiguration->sessionParams.count(),
        force3DNR, isAPSupport3DNR, pPipelineStaticInfo->isDualDevice, StereoSettingProvider::getStereoFeatureMode(), (out->StreamingParams.nr3dMode & E3DNR_MODE_MASK_DSDN_EN),
        pParsedAppConfiguration->operationMode);

    //FSC
    out->StreamingParams.fscMode = EFSC_MODE_MASK_FSC_NONE;
    if (pParsedAppImageStreamInfo->hasVideoConsumer) {//video mode
        MBOOL support_AF = MFALSE;
        // always query hal3a with sensorId index 0, not support multicam yet
        if (mHal3a.size() > 0) {
            auto pHal3A = mHal3a[0];
            NS3Av3::FeatureParam_T r3ASupportedParam;
            {
                std::lock_guard<std::mutex> _l(mHal3aLocker);
                if(pHal3A.get() && pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, reinterpret_cast<MINTPTR>(&r3ASupportedParam), 0))
                {
                    support_AF = (r3ASupportedParam.u4MaxFocusAreaNum > 0);
                }
                else {
                    MY_LOGW("Cannot query AF ability from 3A");
                }
            }
        } else {
            MY_LOGE("cannot get Hal3A instance!");
        }

        MUINT32 fsc_mask = FSCCustom::USAGE_MASK_NONE;
        if (pParsedAppConfiguration->operationMode==1 /* CONSTRAINED_HIGH_SPEED_MODE */
            || pParsedSMVRBatchInfo != nullptr        /* SMVRBatch */
           )
        {
            fsc_mask |= FSCCustom::USAGE_MASK_HIGHSPEED;
        }
        MINT32 dualFeatureMode = -1;
        if (pParsedMultiCamInfo != nullptr){
            dualFeatureMode = pParsedMultiCamInfo->mDualFeatureMode;
        }
        if (pPipelineStaticInfo->isDualDevice ||
            (dualFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_ZOOM)) {
            fsc_mask |= FSCCustom::USAGE_MASK_DUAL_ZOOM;
        }
        if (FSCCustom::isEnabledFSC(fsc_mask) && support_AF)
        {
            out->StreamingParams.fscMode |= EFSC_MODE_MASK_FSC_EN;
            if (::property_get_int32(FSC_DEBUG_ENABLE_PROPERTY, 0))
                out->StreamingParams.fscMode |= EFSC_MODE_MASK_DEBUG_LEVEL;
            if (::property_get_int32(FSC_SUBPIXEL_ENABLE_PROPERTY, 1)) // default on
                out->StreamingParams.fscMode |= EFSC_MODE_MASK_SUBPIXEL_EN;
        }
        MY_LOGD("FSC-info: mask=0x%x, mode=0x%x", fsc_mask, out->StreamingParams.fscMode);
    }

    // EIS
    MUINT8 appEisMode = 0;
    MINT32 advEisMode = 0;
    MINT32 previewEis = 0;
    MBOOL isMultiZoom = (pParsedMultiCamInfo != nullptr) && (pParsedMultiCamInfo->mDualFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_ZOOM);
    if( !IMetadata::getEntry<MUINT8>(&pParsedAppConfiguration->sessionParams, MTK_CONTROL_VIDEO_STABILIZATION_MODE, appEisMode) )
    {
        MY_LOGD("No MTK_CONTROL_VIDEO_STABILIZATION_MODE in sessionParams");
    }
    if( !IMetadata::getEntry<MINT32>(&pParsedAppConfiguration->sessionParams, MTK_EIS_FEATURE_EIS_MODE, advEisMode) )
    {
        MY_LOGD("No MTK_EIS_FEATURE_EIS_MODE in sessionParams");
    }
    IMetadata::getEntry<MINT32>(&pParsedAppConfiguration->sessionParams, MTK_EIS_FEATURE_PREVIEW_EIS, previewEis);

    out->StreamingParams.eisExtraBufNum = 0;
    out->StreamingParams.bPreviewEIS = (previewEis || forceEis30) && EISCustom::isSupportPreviewEIS() && !pParsedAppImageStreamInfo->hasVideo4K;

    if ( EISCustom::isSupportAdvEIS_HAL3() &&
         !isMultiZoom && pParsedAppConfiguration->operationMode != 1 /* CONSTRAINED_HIGH_SPEED_MODE */ &&
         ( advEisMode == MTK_EIS_FEATURE_EIS_MODE_ON || appEisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON || forceEis30 || out->StreamingParams.bPreviewEIS ) &&
         !forceEisEMMode )
    {
        out->StreamingParams.bIsEIS = MTRUE;
        MUINT32 eisMask = EISCustom::USAGE_MASK_NONE;
        MUINT32 videoCfg = EISCustom::VIDEO_CFG_FHD;
        if (out->StreamingParams.hdrHalMode != MTK_HDR_FEATURE_HDR_HAL_MODE_OFF) {
            eisMask |= EISCustom::USAGE_MASK_VHDR;
        }
        if (pParsedAppImageStreamInfo->hasVideo4K) {
            eisMask |= EISCustom::USAGE_MASK_4K2K;
            videoCfg = EISCustom::VIDEO_CFG_4K2K;
            out->StreamingParams.eisInfo.videoConfig = NSCam::EIS::VIDEO_CFG_4K2K;
        }
        else
        {
            out->StreamingParams.eisInfo.videoConfig = NSCam::EIS::VIDEO_CFG_FHD;
        }

        out->StreamingParams.eisInfo.mode = EISCustom::getEISMode(eisMask);
        // FSC+ only support EIS3.0
        if (EFSC_FSC_ENABLED(out->StreamingParams.fscMode))
        {
            //EIS1.2 per-frame on/off
            if(!EIS_MODE_IS_EIS_30_ENABLED(out->StreamingParams.eisInfo.mode) && !EIS_MODE_IS_EIS_12_ENABLED(out->StreamingParams.eisInfo.mode)) {
                MY_LOGI("disable FSC due to combine with EIS 2.x version!");
                out->StreamingParams.fscMode = EFSC_MODE_MASK_FSC_NONE;
            }
        }
        if (EFSC_FSC_ENABLED(out->StreamingParams.fscMode)) {
            eisMask |= EISCustom::USAGE_MASK_FSC;
        }
        out->StreamingParams.eisInfo.factor =
            EIS_MODE_IS_EIS_12_ENABLED(out->StreamingParams.eisInfo.mode) ?
            EISCustom::getEIS12Factor() : EISCustom::getEISFactor(videoCfg, eisMask);
        out->StreamingParams.eisInfo.queueSize = EISCustom::getForwardFrames(videoCfg);
        out->StreamingParams.eisInfo.startFrame = EISCustom::getForwardStartFrame();
        out->StreamingParams.eisInfo.lossless = EISCustom::isEnabledLosslessMode();
        out->StreamingParams.eisInfo.supportQ = advEisMode && EISCustom::isEnabledForwardMode(videoCfg);
        out->StreamingParams.eisInfo.supportRSC = out->StreamingParams.bPreviewEIS ||
                                                  EIS_MODE_IS_EIS_30_ENABLED(out->StreamingParams.eisInfo.mode);
        out->StreamingParams.eisInfo.previewEIS = out->StreamingParams.bPreviewEIS;
        out->StreamingParams.bEnableTSQ = MTRUE;
        if (forceTSQ == 2) { // force disable tsq
            out->StreamingParams.bEnableTSQ = MFALSE;
        }
        if ( !out->StreamingParams.bEnableTSQ ) {
            out->StreamingParams.eisExtraBufNum = EISCustom::getForwardFrames(videoCfg);
        }
        float desireRatio = EISCustom::getHeightToWidthRatio();
        float desireHeight = desireRatio * pParsedAppImageStreamInfo->maxYuvSize.w;
        if (desireHeight > pParsedAppImageStreamInfo->maxYuvSize.h)
        {
            out->StreamingParams.rrzoHeightToWidth = desireRatio;
        }
    }
    else if( pParsedAppConfiguration->operationMode != 1 /* CONSTRAINED_HIGH_SPEED_MODE */ &&
             EISCustom::isSupportAdvEIS_HAL3() && forceEisEMMode )
    {
        out->StreamingParams.bIsEIS = MFALSE;
        out->StreamingParams.eisInfo.mode = (1<<EIS_MODE_CALIBRATION);
        out->StreamingParams.fscMode = EFSC_MODE_MASK_FSC_NONE;
        out->StreamingParams.eisInfo.factor = 100;
        out->StreamingParams.bEnableTSQ = MFALSE;
    }
    else
    {
        out->StreamingParams.bIsEIS = MFALSE;
        out->StreamingParams.eisInfo.mode = 0;
        out->StreamingParams.eisInfo.factor = 100;
        out->StreamingParams.bEnableTSQ = MFALSE;
    }

    out->StreamingParams.minRrzoEisW = LMV_MIN_RRZ;

    // PQ
    int32_t proprietaryClient = 0;
    IMetadata::getEntry<MINT32>(&pParsedAppConfiguration->sessionParams, MTK_CONFIGURE_SETTING_PROPRIETARY, proprietaryClient);
    int32_t bForceSupportPQ = ::property_get_int32("vendor.camera.mdp.pq.enable", -1);
    out->StreamingParams.bSupportPQ = (bForceSupportPQ != -1) ? (bForceSupportPQ > 0) : (proprietaryClient > 0);
    MY_LOGD("ProprietaryClient : %d, bForceSupportPQ : %d, bSupportPQ : %d", proprietaryClient, bForceSupportPQ, out->StreamingParams.bSupportPQ);

    // CZ
    out->StreamingParams.bSupportCZ = ::property_get_int32("vendor.camera.mdp.cz.enable", 0);

    // DRE
    if (pParsedSMVRBatchInfo != nullptr
       || pParsedAppConfiguration->operationMode == 1
       )
    {
        out->StreamingParams.bSupportDRE = 0;
    }
    else
    {
        out->StreamingParams.bSupportDRE = ::property_get_int32("vendor.camera.mdp.dre.enable", 0);
    }
    // HFG
    out->StreamingParams.bSupportHFG = ::property_get_int32("vendor.camera.mdp.hfg.enable", 0);
    MY_LOGD_IF(2 <= mbDebug, "support(PQ=%d, CZ=%d, DRE=%d, HFG=%d), hasVideoConsumer=%d, EIS_30_ENABLED: %d",
        out->StreamingParams.bSupportPQ, out->StreamingParams.bSupportCZ, out->StreamingParams.bSupportDRE, out->StreamingParams.bSupportHFG,
        pParsedAppImageStreamInfo->hasVideoConsumer, EIS_MODE_IS_EIS_30_ENABLED(out->StreamingParams.eisInfo.mode));

    // DSDN : can customized by customer, 0: force off, 1: on
    out->StreamingParams.dsdnHint = 1;


    // LMV
    if( pParsedAppConfiguration->operationMode != 1 /* CONSTRAINED_HIGH_SPEED_MODE */ &&
        pParsedSMVRBatchInfo == nullptr &&
        ( E3DNR_MODE_MASK_ENABLED(out->StreamingParams.nr3dMode, (E3DNR_MODE_MASK_UI_SUPPORT | E3DNR_MODE_MASK_HAL_FORCE_SUPPORT)) ||
          out->StreamingParams.eisInfo.mode) )
    {
        out->StreamingParams.bNeedLMV = true;
    }

    // RSS
    if( pParsedAppConfiguration->operationMode != 1 /* CONSTRAINED_HIGH_SPEED_MODE */ &&
        pParsedSMVRBatchInfo == nullptr &&
        ( ( EIS_MODE_IS_EIS_30_ENABLED(out->StreamingParams.eisInfo.mode) &&
            EIS_MODE_IS_EIS_IMAGE_ENABLED(out->StreamingParams.eisInfo.mode) ) ||
          ( E3DNR_MODE_MASK_ENABLED(out->StreamingParams.nr3dMode, (E3DNR_MODE_MASK_UI_SUPPORT | E3DNR_MODE_MASK_HAL_FORCE_SUPPORT)) &&
            E3DNR_MODE_MASK_ENABLED(out->StreamingParams.nr3dMode, E3DNR_MODE_MASK_RSC_EN) ) ) )
    {
        out->StreamingParams.bNeedRSS = true;
        out->StreamingParams.bNeedLargeRsso = EIS_MODE_IS_EIS_30_ENABLED(out->StreamingParams.eisInfo.mode)
                                                && EIS_MODE_IS_EIS_IMAGE_ENABLED(out->StreamingParams.eisInfo.mode)
                                                && EFSC_FSC_ENABLED(out->StreamingParams.fscMode);
    }

    MY_LOGD("AppEis: %d, AdvEis: %d. Lmv: %d, Rss/largeRss : %d/%d, isEis : %d, Eis mode : %d, Eis factor : %d, TSQ : %d, mEisExtraBufNum : %d, hasVideoConsumer : %d, operation : %d, AdvEIS support : %d, EM : %d, PreviewEIS : %d, eis-videoConfig=%d, isMultiZoom=%d",
            appEisMode, advEisMode,
            out->StreamingParams.bNeedLMV, out->StreamingParams.bNeedRSS, out->StreamingParams.bNeedLargeRsso,
            out->StreamingParams.bIsEIS, out->StreamingParams.eisInfo.mode, out->StreamingParams.eisInfo.factor, out->StreamingParams.bEnableTSQ, out->StreamingParams.eisExtraBufNum,
            pParsedAppImageStreamInfo->hasVideoConsumer, pParsedAppConfiguration->operationMode,
            EISCustom::isSupportAdvEIS_HAL3(), forceEisEMMode, out->StreamingParams.bPreviewEIS,
            out->StreamingParams.eisInfo.videoConfig, isMultiZoom);

    MY_LOGD("out->StreamingParams:%s", policy::toString(out->StreamingParams).c_str());

    return true;
}

auto
FeatureSettingPolicy::
evaluateHDRConfiguration(
    ConfigurationOutputParams* out,
    ConfigurationInputParams const* in __unused
) -> bool
{
    CAM_ULOGM_APILIFE();

    auto const& pParsedAppConfiguration   = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;

    MINT32 hdrModeInt = 0;
    if( !IMetadata::getEntry<MINT32>(&pParsedAppConfiguration->sessionParams, MTK_HDR_FEATURE_SESSION_PARAM_HDR_MODE, hdrModeInt) ) {
        MY_LOGD_IF(mHDRHelper->getDebugLevel(), "App no set MTK_HDR_FEATURE_SESSION_PARAM_HDR_MODE");
    }
    mHDRHelper->updateAppConfigMode(static_cast<HDRMode>((uint8_t)hdrModeInt));

    if( mHDRHelper->isHDR() ) {
        out->StreamingParams.hdrHalMode = mHDRHelper->getHDRHalMode();
        out->StreamingParams.hdrSensorMode = mHDRHelper->getHDRSensorMode();
        if( mHDRHelper->isMulitFrameHDR() ) {
            out->StreamingParams.bDisableInitRequest = true;
            out->StreamingParams.groupSize = mHDRHelper->getGroupSize();
            // TODO: use capture plugin API and set in out->CaptureParams.additionalHalP1OutputBufferNum.imgo
            if( mConfigOutputParams.isZslMode ) {
                out->StreamingParams.additionalHalP1OutputBufferNum.imgo = 20;
            }
        }
    }
    else {
        out->StreamingParams.hdrHalMode = MTK_HDR_FEATURE_HDR_HAL_MODE_OFF;
        out->StreamingParams.hdrSensorMode = SENSOR_VHDR_MODE_NONE;
    }
    MY_LOGD("Stream=HDRHalMode(0x%x),HDRSensorMode(0x%x):rrzo(%d),imgo(%d),lcso(%d),bDisableInitRequest(%d),groupSize(%d).%s",
            out->StreamingParams.hdrHalMode, out->StreamingParams.hdrSensorMode,
            out->StreamingParams.additionalHalP1OutputBufferNum.rrzo,
            out->StreamingParams.additionalHalP1OutputBufferNum.imgo,
            out->StreamingParams.additionalHalP1OutputBufferNum.lcso,
            out->StreamingParams.bDisableInitRequest, out->StreamingParams.groupSize,
            mHDRHelper->getDebugMessage().string());

    return true;
}

auto
FeatureSettingPolicy::
updateHDRMultiFrame(
    RequestOutputParams* out,
    RequestInputParams const* in,
    NSPipelinePlugin::MetadataPtr pOutMetaApp_Additional,
    NSPipelinePlugin::MetadataPtr pOutMetaHal_Additional
) -> bool
{
    AdditionalFrameInfo additionalFrame;

    uint32_t sensorIndex = mSensorInfoList[SENSOR_INDEX_MAIN].sensorIdx;
    if (mHDRHelper->isHDR()) {
        if (!mHDRHelper->negotiateRequestPolicy(additionalFrame)) {
            MY_LOGW("negotiateRequestPolicy failed");
        }
        for (size_t i = 0; i < additionalFrame.getAdditionalFrameSet().preSubFrame.size(); i++) {
            std::shared_ptr<RequestResultParams> newFrame = nullptr;
            std::shared_ptr<NSCam::IMetadata>  outMetaApp = std::make_shared<IMetadata>();
            std::shared_ptr<NSCam::IMetadata>  outMetaHal = std::make_shared<IMetadata>();
            // Copy from main-frame then update preSubFrame
            if (pOutMetaApp_Additional.get() != nullptr) {
                *outMetaApp += *pOutMetaApp_Additional;
            }
            if (pOutMetaHal_Additional.get() != nullptr) {
                *outMetaHal += *pOutMetaHal_Additional;
            }
            if (additionalFrame.getAdditionalFrameSet().preSubFrame[i].appMetadata.get() != nullptr) {
                *outMetaApp += *additionalFrame.getAdditionalFrameSet().preSubFrame[i].appMetadata;
            }
            if (additionalFrame.getAdditionalFrameSet().preSubFrame[i].halMetadata.get() != nullptr) {
                *outMetaHal += *additionalFrame.getAdditionalFrameSet().preSubFrame[i].halMetadata;
            }
            updateRequestResultParams(
            newFrame,
            outMetaApp,
            outMetaHal,
            additionalFrame.getAdditionalFrameSet().preSubFrame[i].p1Dma,
            sensorIndex);

            if (out->preSubFrames.size() > i ) {
                out->preSubFrames[i] = newFrame;
            }
            else {
                out->preSubFrames.push_back(newFrame);
            }
        }

        for (auto &preDummyFrame : additionalFrame.getAdditionalFrameSet().preDummy) {
            uint32_t camP1Dma = 0;
            if ( !getCaptureP1DmaConfig(camP1Dma, in, sensorIndex) ){
                MY_LOGE("[getCaptureP1DmaConfig] main P1Dma output is invalid: 0x%X", camP1Dma);
                return MFALSE;
            }

            std::shared_ptr<RequestResultParams> newFrame = nullptr;
            std::shared_ptr<NSCam::IMetadata>  outMetaApp = std::make_shared<IMetadata>();
            std::shared_ptr<NSCam::IMetadata>  outMetaHal = std::make_shared<IMetadata>();
            // Copy from main-frame then update preDummyFrame
            if (pOutMetaApp_Additional.get() != nullptr) {
                *outMetaApp += *pOutMetaApp_Additional;
            }
            if (pOutMetaHal_Additional.get() != nullptr) {
                *outMetaHal += *pOutMetaHal_Additional;
            }
            if (preDummyFrame.appMetadata.get() != nullptr) {
                *outMetaApp += *preDummyFrame.appMetadata;
            }
            if (preDummyFrame.halMetadata.get() != nullptr) {
                *outMetaHal += *preDummyFrame.halMetadata;
            }
            updateRequestResultParams(
                newFrame,
                outMetaApp,
                outMetaHal,
                ( preDummyFrame.p1Dma | camP1Dma ),
                sensorIndex);

            out->preDummyFrames.push_back(newFrame);
        }
    }

    MY_LOGD_IF(mHDRHelper->getDebugLevel(), "main:%d,sub:%zu,preSub:%zu,preDummy:%zu,postDummy:%zu.HDRPolicy(%p)=%s",
               (out->mainFrame.get() != nullptr), out->subFrames.size(), out->preSubFrames.size(),
               out->preDummyFrames.size(), out->postDummyFrames.size(), mHDRHelper.get(), mHDRHelper->getDebugMessage().string());

    if (mHDRHelper->needUnpack()) {
        out->needUnpackRaw = true;
    }

    return true;
}


