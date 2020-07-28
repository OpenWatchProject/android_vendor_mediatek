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

#define LOG_TAG "mtkcam-customer_scenario_mgr"
//
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/common.h>
//
#include <mtkcam3/pipeline/policy/types.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
//
#include <mtkcam3/3rdparty/customer/customer_scenario_mgr.h>
//
#include <camera_custom_feature_table.h>
//
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FEATURE_SETTING_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::v3::pipeline::policy::scenariomgr;
/******************************************************************************
 *
 ******************************************************************************/

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
//
/******************************************************************************
 *
 ******************************************************************************/

namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace scenariomgr {

bool gIsLowMemoryDevice = ::property_get_bool("ro.config.low_ram", false);

// gCustomerScenarioFeaturesMaps, gCustomerScenarioFeaturesMapsPhyMaster, gCustomerScenarioFeaturesMapsPhySlave
// features maps has been moved to custom folder for different platform,
// the path is ./vendor/mediatek/proprietary/custom/<platform>/hal/camera/camera_custom_feature_table.cpp

auto customer_get_capture_scenario(
    int32_t &scenario, /*eCameraScenario*/
    const ScenarioHint& scenarioHint,
    IMetadata const* pAppMetadata
) -> bool
{
    if (CC_UNLIKELY(pAppMetadata == nullptr)) {
        MY_LOGE("pAppMetadata is invalid nullptr!");
        return false;
    }

    //CAM_ULOGM_APILIFE();

    scenario = CUSTOMER_CAMERA_SCENARIO_UNKNOW;
    MY_LOGD("scenarioHint(isCShot:%d, isDualCam:%d, dualDevicePath:%d, multiCamFeatureMode:%d, captureFeatureMode:%d, isLogical:%d captureScenarioIndex:%d)",
        scenarioHint.isCShot,
        scenarioHint.isDualCam, static_cast<MINT32>(scenarioHint.dualDevicePath), scenarioHint.multiCamFeatureMode,
        scenarioHint.captureFeatureMode,
        scenarioHint.isLogical,
        scenarioHint.captureScenarioIndex);

    // TODO: customer can modified the logic/flow to decide the streaming scenario.
    if (scenarioHint.captureScenarioIndex > 0) {  // force by vendor tag (ex:Pro mode))
        MY_LOGI("forced captureScenarioIndex:%d", scenarioHint.captureScenarioIndex);
        scenario = scenarioHint.captureScenarioIndex;
    }
    else if (scenarioHint.isRawRequestForIspHidl) {
        MY_LOGD("request one raw frame for isp hidl reprocessing, ispTuningHint:%d", scenarioHint.ispTuningHint);
        // it will execute full image processing feature to isp hidl, not support multiframe in camerahalserver.
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_REQUEST_FRAME_FOR_ISP_HIDL_RAW;
    }
    else if (scenarioHint.isYuvRequestForIspHidl) {
        MY_LOGD("request one yuv frame for isp hidl reprocessing, ispTuningHint:%d", scenarioHint.ispTuningHint);
        // it will execute full image processing feature to isp hidl, not support multiframe in camerahalserver.
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_REQUEST_FRAME_FOR_ISP_HIDL_YUV;
    }
    else if (scenarioHint.isRawOrDngRequest) {
        MY_LOGD("request dng or opaque raw");
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_DNG_OPAQUE_RAW;
    }
    else if (scenarioHint.isSuperNightMode &&
             scenarioHint.isRawReprocess) {
        // super night mode (only execute raw reprocess)
        scenario = CUSTOMER_CAMERA_SCENARIO_CAPTURE_SUPER_NIGHT_RAW_REPROCESS;
    }
    else if (scenarioHint.isSuperNightMode /*parsed info from operation mode*/) {
        // super night mode (support hal capture features)
        scenario = CUSTOMER_CAMERA_SCENARIO_CAPTURE_SUPER_NIGHT;
    }
#if 0 // for test, as example code only
    else if (scenarioHint.operationMode == 0x8019 /*VENDOR_SUPER_NIGHT*/) {
        /*example code*/
        scenario = CUSTOMER_CAMERA_SCENARIO_CAPTURE_SUPER_NIGHT;
    }
#endif
    else if (scenarioHint.isRawReprocess) {
        MY_LOGD("Raw reprocessing scenario!");
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_RAW_REPROCESS;
    }
    else if (scenarioHint.isYuvReprocess) {
        MY_LOGD("Yuv reprocessing scenario!");
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_YUV_REPROCESS;
    }
    else if (scenarioHint.isCShot) { // CShot
        MY_LOGD("CShot scenario!");
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_CSHOT;
    }
    else if (scenarioHint.isDualCam || (!scenarioHint.isLogical)) {
        if (scenarioHint.dualDevicePath == static_cast<int32_t>(DualDevicePath::MultiCamControl)) {
            if (scenarioHint.captureFeatureMode == NSCam::v1::Stereo::E_STEREO_FEATURE_MULTI_CAM) {
                MY_LOGD("Multicam scenario!");
                scenario = MTK_CAMERA_SCENARIO_CAPTURE_MULTICAM;
            }
            else if (scenarioHint.multiCamFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF) {
                MY_LOGD("DualCam VSDoF scenario!");
                scenario = MTK_CAMERA_SCENARIO_CAPTURE_VSDOF;
            }
            else {
                MY_LOGD("Zoom scenario!");
                scenario = MTK_CAMERA_SCENARIO_CAPTURE_ZOOM;
            }
        } else {
            MY_LOGD("using single cam flow in multicam device.");
            scenario = MTK_CAMERA_SCENARIO_CAPTURE_NORMAL;
        }
    }
    else {
        MY_LOGD("no dedicated scenario, normal scenario");
        scenario = MTK_CAMERA_SCENARIO_CAPTURE_NORMAL;
    }

    MY_LOGD("scenario:%d", scenario);

    return true;
}

bool customer_update_capture_feature_combination(
    ScenarioFeatures &scenarioFeatures,
    const IMetadata *pAppMetadata /*for extension*/
)
{
    if (CC_UNLIKELY(pAppMetadata == nullptr)) {
        MY_LOGW("pAppMetadata is invalid nullptr!");
        return false;
    }
    // NOTE: To enable/disable sub feature(single yuv domain capture feature) for App by App control Metadata.
    // Single yuv nr will be auto-on-by-iso/off if the App control tag "MTK_CONTROL_CAPTURE_SINGLE_YUV_NR"
    // is set to be 1(auto-on-by-iso)/0(off),
    // The single yuv nr will be executed mostly during RAW to YUV stage.
    MINT32 hintForSigleYuvNr = -1;
    if (IMetadata::getEntry<MINT32>(pAppMetadata, MTK_CONTROL_CAPTURE_SINGLE_YUV_NR, hintForSigleYuvNr)) {
        MY_LOGI("App set MTK_CONTROL_CAPTURE_SINGLE_YUV_NR(%d)", hintForSigleYuvNr);
    }
    MINT32 hintForHighQualityYuv = -1;
    if (IMetadata::getEntry<MINT32>(pAppMetadata, MTK_CONTROL_CAPTURE_HIGH_QUALITY_YUV, hintForHighQualityYuv)) {
        MY_LOGI("App set MTK_CONTROL_CAPTURE_HIGH_QUALITY_YUV(%d)", hintForHighQualityYuv);
    }
    //
    for (auto& featureSet : scenarioFeatures.vFeatureSet) {
        auto& featureName = featureSet.featureName;
        auto& featureCombination = featureSet.featureCombination;

        MINT64 oldFeatureCombination = featureCombination;
        // update single yuv noise reduction requirement by app control (default enable in raw to yuv stage.)
        if (hintForSigleYuvNr == 0) {
            featureCombination &= ~(MTK_FEATURE_NR);
            MY_LOGD("key feature(%s): App disable MTK_FEATURE_NR(%x), featureCombination(%#" PRIx64" --> %#" PRIx64")",
                    featureName.c_str(), MTK_FEATURE_NR,
                    oldFeatureCombination, featureCombination);
        }
        else if (hintForSigleYuvNr == 1) {
            featureCombination |= MTK_FEATURE_NR;
            MY_LOGD("key feature(%s): App enable MTK_FEATURE_NR(%x), featureCombination(%#" PRIx64" --> %#" PRIx64")",
                    featureName.c_str(), MTK_FEATURE_NR,
                    oldFeatureCombination, featureCombination);
        }

        // update P2A high quality yuv requirement by app control (default enable in raw to yuv stage.)
        oldFeatureCombination = featureCombination;
        if (hintForHighQualityYuv == 0) {
            featureCombination &= ~(MTK_FEATURE_HIGH_QUALITY_YUV);
            MY_LOGD("key feature(%s): App disable MTK_FEATURE_HIGH_QUALITY_YUV(%x), featureCombination(%#" PRIx64" --> %#" PRIx64")",
                    featureName.c_str(), MTK_FEATURE_HIGH_QUALITY_YUV,
                    oldFeatureCombination, featureCombination);
        }
        else if (hintForHighQualityYuv == 1) {
            featureCombination |= MTK_FEATURE_HIGH_QUALITY_YUV;
            MY_LOGD("key feature(%s): APP enable MTK_FEATURE_HIGH_QUALITY_YUV(%x), featureCombination(%#" PRIx64" --> %#" PRIx64")",
                    featureName.c_str(), MTK_FEATURE_HIGH_QUALITY_YUV,
                    oldFeatureCombination, featureCombination);
        }
    }

    return true;
}

auto customer_get_streaming_scenario(
    int32_t &scenario, /*eCameraScenario*/
    const ScenarioHint& scenarioHint,
    IMetadata const* pAppMetadata
) -> bool
{
    if (CC_UNLIKELY(pAppMetadata == nullptr)) {
        MY_LOGE("pAppMetadata is invalid nullptr!");
        return false;
    }

    //CAM_ULOGM_APILIFE();

    scenario = CUSTOMER_CAMERA_SCENARIO_UNKNOW;
    MY_LOGD("scenarioHint(isDualCam:%d(0x%x), streamingScenarioIndex:%d), isVideo(%d)",
            scenarioHint.isDualCam, scenarioHint.mDualFeatureMode, scenarioHint.streamingScenarioIndex, scenarioHint.isVideoMode);

    // TODO: customer can modified the logic/flow to decide the streaming scenario.
    if (scenarioHint.streamingScenarioIndex > 0) { // forced by vendor tag
        MY_LOGI("forced streamingScenarioIndex:%d", scenarioHint.streamingScenarioIndex);
        scenario = scenarioHint.streamingScenarioIndex;
    }
    else if (scenarioHint.isDualCam) {
        MY_LOGI("DualCam scenario!");
        switch( scenarioHint.mDualFeatureMode )
        {
        case DualFeatureMode_YUV:
            scenario = MTK_CAMERA_SCENARIO_STREAMING_DUAL_YUV; break;
        case DualFeatureMode_HW_DEPTH:
            scenario = MTK_CAMERA_SCENARIO_STREAMING_DUAL_HWDEPTH; break;
        case DualFeatureMode_MTK_VSDOF:
            scenario = MTK_CAMERA_SCENARIO_STREAMING_DUAL_HWVSDOF; break;
        case DualFeatureMode_MTK_FOVA:
            scenario = MTK_CAMERA_SCENARIO_STREAMING_NORMAL; break;
        default:
            scenario = MTK_CAMERA_SCENARIO_STREAMING_NORMAL; break;
        }
    }
    else {
        MY_LOGI("no dedicated scenario, normal scenario");
        scenario = scenarioHint.isVideoMode ? MTK_CAMERA_SCENARIO_STREAMING_NORMAL_VIDEO
                                            : MTK_CAMERA_SCENARIO_STREAMING_NORMAL;
    }

    MY_LOGI("scenario:%d", scenario);
    return true;
}

auto customer_get_features_table_by_scenario(
    int32_t const scenario, /*eCameraScenario*/
    ScenarioFeatures& scenarioFeatures,
    const ScenarioHint& scenarioHint
) -> bool
{
    //CAM_ULOGM_APILIFE();

    auto scenarioFeatureMaps = gCustomerScenarioFeaturesMaps;
    std::string tableName = "logical map";

    if ( !scenarioHint.isLogical ) {
        if ( scenarioHint.isMaster ) {
            scenarioFeatureMaps = gCustomerScenarioFeaturesMapsPhyMaster;
            tableName = "physical master map";
        }
        else {
            scenarioFeatureMaps = gCustomerScenarioFeaturesMapsPhySlave;
            tableName = "physical slave map";
        }
    }

    size_t tableSize = scenarioFeatureMaps.size();
    MY_LOGD("scenario:%d, table size:%zu", scenario, tableSize);

    auto scenarioFeaturesMap = scenarioFeatureMaps[0];

    auto iter_got = scenarioFeaturesMap.find(scenario);
    if ( iter_got != scenarioFeaturesMap.end()) {
        scenarioFeatures = iter_got->second;
        MY_LOGI("find features for scenario(%d : %s) in %s", scenario, scenarioFeatures.scenarioName.c_str(), tableName.c_str());
    }
    else if (!scenarioHint.isLogical) { // physical stream find in logical table when not found in physical table
        scenarioFeaturesMap = gCustomerScenarioFeaturesMaps[0];
        iter_got = scenarioFeaturesMap.find(scenario);
        if ( iter_got != scenarioFeaturesMap.end()) {
            scenarioFeatures = iter_got->second;
            MY_LOGW("cannot find features in %s, find features for scenario(%d : %s) in logical map",
                        tableName.c_str(), scenario, scenarioFeatures.scenarioName.c_str());
        }
        else {
            MY_LOGE("cannot find features for scenario(%d) in %s and logical map", scenario, tableName.c_str());
            return false;
        }
    }
    else {
        MY_LOGE("cannot find features for scenario(%d) in logical map", scenario);
        return false;
    }

    return true;
}

auto customer_get_capture_config_by_scenario(
    const int32_t scenario, /*eCameraScenario*/
    const ScenarioHint& scenarioHint,
    CaptureScenarioConfig& captureConfig
) -> bool
{
    // config by scenario
    switch (scenario) {
        case CUSTOMER_CAMERA_SCENARIO_CAPTURE_SUPER_NIGHT:
            captureConfig.needUnpackRaw = false;
            captureConfig.maxAppJpegStreamNum = 8;
            captureConfig.maxAppRaw16OutputBufferNum = 17;
            break;
        case CUSTOMER_CAMERA_SCENARIO_CAPTURE_SUPER_NIGHT_RAW_REPROCESS:
            captureConfig.needUnpackRaw = true;
            captureConfig.maxAppJpegStreamNum = 8;
            captureConfig.maxAppRaw16OutputBufferNum = 17;
            break;
        case MTK_CAMERA_SCENARIO_CAPTURE_RAW_REPROCESS:
            captureConfig.needUnpackRaw = false;   // reprocess input format has been chosen
            captureConfig.maxAppJpegStreamNum = 8;
            captureConfig.maxAppRaw16OutputBufferNum = 3;
            break;
        default:
            MUINT32 jpegNum = static_cast<MUINT32>(::property_get_int32("vendor.debug.default.jpegnum", 6));
            captureConfig.needUnpackRaw = false;
            captureConfig.maxAppJpegStreamNum = jpegNum;
            captureConfig.maxAppRaw16OutputBufferNum = 2;
            break;

    }

    // config by scenario hint
    if (scenarioHint.isIspHidlTuningEnable) {
        captureConfig.maxAppRaw16OutputBufferNum = 8;
        MY_LOGD("set maxAppRaw16OutputBufferNum=%d for ISP HIDL flow", captureConfig.maxAppRaw16OutputBufferNum);
    }

    return true;
}

bool customer_get_streaming_config(
    int32_t /*scenario*/,
    const ScenarioFeatures &/*scenarioFeatures*/,
    StreamingScenarioConfig &scenarioConfig,
    const ScenarioHint &scenarioHint __unused,
    const IMetadata */*pAppMetadata*/
)
{
    (void)scenarioHint;
    // TODO get TP eis run or not, currently use prop control
    MUINT32 eisMargin = (MUINT32)::property_get_int32("vendor.debug.scenario.s.eisMargin", 0);
    if(/*scenarioHint.isVideoMode &&*/ eisMargin)
    {
        scenarioConfig.fixedMargins.push_back(eisMargin);
    }

    MY_LOGI("streaming scenario config, fixMarginList(%zu)", scenarioConfig.fixedMargins.size());
    for(MUINT32 margin : scenarioConfig.fixedMargins)
    {
        MY_LOGI("fix margin add (%d)", margin);
    }
    return true;
}

bool customer_get_capture_scenario(
    int32_t &scenario,
    ScenarioFeatures &scenarioFeatures,
    CaptureScenarioConfig &captureConfig,
    const ScenarioHint &scenarioHint,
    const IMetadata *pAppMetadata
)
{
    return customer_get_capture_scenario(scenario, scenarioHint, pAppMetadata) &&
           customer_get_capture_config_by_scenario(scenario, scenarioHint, captureConfig) &&
           customer_get_features_table_by_scenario(scenario, scenarioFeatures, scenarioHint);
}

bool customer_get_streaming_scenario(
    int32_t &scenario,
    ScenarioFeatures &scenarioFeatures,
    StreamingScenarioConfig &scenarioConfig,
    const ScenarioHint &scenarioHint,
    const IMetadata *pAppMetadata
)
{
    return customer_get_streaming_scenario(scenario, scenarioHint, pAppMetadata) &&
           customer_get_features_table_by_scenario(scenario, scenarioFeatures, scenarioHint) &&
           customer_get_streaming_config(scenario, scenarioFeatures, scenarioConfig, scenarioHint, pAppMetadata);
}

};  //namespace scenariomgr
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

