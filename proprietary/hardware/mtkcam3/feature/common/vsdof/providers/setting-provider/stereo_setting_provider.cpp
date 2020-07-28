/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StereoSettingProvider"

#include <algorithm>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>

#include <sstream>  //For ostringstream

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/def/ImageFormat.h>
#include <camera_custom_stereo.h>       // For CUST_STEREO_* definitions.
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);
#include "../tuning-provider/stereo_tuning_provider_kernel.h"
#include <math.h>

#include <cutils/properties.h>
// #include "../inc/stereo_dp_util.h"
#include <mtkcam/aaa/IHal3A.h>

#include "stereo_setting_provider_kernel.h"
#if (1==VSDOF_SUPPORTED)
#include <fefm_setting_provider.h>
#endif

#include <ctime>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include "stereo_setting_def.h"
#include <isp_tuning.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START          MY_LOGD_IF(LOG_ENABLED, "+")
#define FUNC_END            MY_LOGD_IF(LOG_ENABLED, "-")

using namespace NSCam;
using namespace android;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;
using namespace NS3Av3;

MUINT32 StereoSettingProvider::__logicalDeviceID = 0;
ENUM_STEREO_RATIO StereoSettingProvider::__imageRatio = eRatio_Default;
MINT32 StereoSettingProvider::__stereoFeatureMode = 0;
MINT32 StereoSettingProvider::__stereoModuleType = -1;
MUINT32 StereoSettingProvider::__stereoShotMode = 0;
std::vector<MUINT> StereoSettingProvider::__sensorScenarios;
MUINT32 StereoSettingProvider::__featureStartTime = 0;
ENUM_STEREO_CAM_SCENARIO StereoSettingProvider::__vsdofScenario = eStereoCamPreview;
bool StereoSettingProvider::__3rdCaptureDepthRunning = false;
bool StereoSettingProvider::__3rdCaptureBokehRunning = false;

std::vector<StereoSensorSetting_T *> g_sensorSettings;
StereoSensorConbinationSetting_T *g_currentSensorCombination = NULL;

StereoSensorConbinationSetting_T *
__getSensorCombinationSetting(MUINT32 logicalDeviceID)
{
    return StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(logicalDeviceID);
}

bool __getSensorSettings(MUINT32 logicalDeviceID,
                         std::vector<StereoSensorSetting_T *> &settings)
{
    bool result = true;
    StereoSensorConbinationSetting_T *sensorCombination =
        StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(logicalDeviceID);

    if(sensorCombination &&
       sensorCombination->sensorSettings.size() > 0)
    {
        settings = sensorCombination->sensorSettings;
    } else {
        MY_LOGW("Cannot get sensor settings");
        result = false;
    }

    return result;
}

bool __getSensorSettingsAndCombination(MUINT32 logicalDeviceID,
                                       std::vector<StereoSensorSetting_T *> &sensorSettings,
                                       StereoSensorConbinationSetting_T *&sensorCombination)
{
    bool result = true;
    sensorCombination = StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(logicalDeviceID);

    if(sensorCombination)
    {
        sensorSettings = sensorCombination->sensorSettings;
    } else {
        MY_LOGW("Cannot get sensor settings");
        result = false;
    }

    return result;
}

ENUM_STEREO_SENSOR_PROFILE
StereoSettingProvider::stereoProfile(MUINT32 logicalDeviceID)
{
    StereoSensorConbinationSetting_T *sensorCombination = StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(logicalDeviceID);
    if(sensorCombination) {
        if(0 == sensorCombination->sensorSettings[0]->staticInfo.facingDirection) {
            if(0 == sensorCombination->sensorSettings[1]->staticInfo.facingDirection) {
                return STEREO_SENSOR_PROFILE_REAR_REAR;
            } else {
                return STEREO_SENSOR_PROFILE_REAR_FRONT;
            }
        } else {
            if(1 == sensorCombination->sensorSettings[1]->staticInfo.facingDirection) {
                return STEREO_SENSOR_PROFILE_FRONT_FRONT;
            } else {
                return STEREO_SENSOR_PROFILE_REAR_FRONT;
            }
        }
    }

    return STEREO_SENSOR_PROFILE_UNKNOWN;
}

bool
StereoSettingProvider::getStereoSensorIndex(int32_t &main1Idx, int32_t &main2Idx, MUINT32 logicalDeviceID)
{
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    vector<MINT32> sensorIDs;
    if(pHalDeviceList) {
        sensorIDs = pHalDeviceList->getSensorId(logicalDeviceID);
    }

    if(sensorIDs.size() > 1)
    {
        main1Idx = sensorIDs[0];
        main2Idx = sensorIDs[1];
    }
    else
    {
        auto pHalSensorList = MAKE_HalSensorList();
        const int SENSOR_COUNT = (pHalSensorList) ? pHalSensorList->queryNumberOfSensors() : 3;
        if(2 == SENSOR_COUNT) {
            // MY_LOGW("Only two sensors were found on the device");
            main1Idx = 0;
            main2Idx = 1;
        } else {
            main1Idx = 0;
            main2Idx = 2;
        }

        MY_LOGW("Cannot get sensor index by logical device ID %d, return %d, %d",
                logicalDeviceID, main1Idx, main2Idx);

        return false;
    }

    return true;
}

bool
StereoSettingProvider::getStereoSensorDevIndex(int32_t &main1DevIdx, int32_t &main2DevIdx, MUINT32 logicalDeviceID)
{
    int32_t main1Idx = 0;
    int32_t main2Idx = 0;
    if (!getStereoSensorIndex(main1Idx, main2Idx, logicalDeviceID)) {
        return false;
    }

    IHalSensorList *sensorList = MAKE_HalSensorList();
    if (NULL == sensorList) {
        return false;
    }

    main1DevIdx = sensorList->querySensorDevIdx(main1Idx);
    main2DevIdx = sensorList->querySensorDevIdx(main2Idx);
    // MY_LOGD_IF(isLogEnabled(), "Main sensor DEV idx %d, Main2 sensor DEV idx %d", main1DevIdx, main2DevIdx);

    return true;
}

void
StereoSettingProvider::setStereoProfile( ENUM_STEREO_SENSOR_PROFILE profile __attribute__((unused)) )
{
// #warning "[Dualcam]This function is only for debug now, will call setLogicalDeviceID(sensor count)"
    IHalSensorList *sensorList = MAKE_HalSensorList();
    if(sensorList) {
        setLogicalDeviceID(sensorList->queryNumberOfSensors());
    }
}

void
StereoSettingProvider::setLogicalDeviceID(const MUINT32 logicalDeviceID)
{
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if(pHalDeviceList) {
        std::string s(pHalDeviceList->queryDriverName(logicalDeviceID));
        size_t prefixPos = s.find(SENSOR_DRVNAME_PREFIX);
        if(prefixPos != std::string::npos) {
            s.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
        }

        MY_LOGD("Set logical device ID %d(%s)", logicalDeviceID, s.c_str());
    }
    else
    {
        MY_LOGD("Set logical device ID %d", logicalDeviceID);
    }

#if (1==VSDOF_SUPPORTED)
        FEFMSettingProvider::destroyInstance();
#endif
    StereoSettingProviderKernel::getInstance()->init();
    // StereoSettingProviderKernel::getInstance()->logSettings();

    __getSensorSettingsAndCombination(logicalDeviceID, g_sensorSettings, g_currentSensorCombination);

    if(g_currentSensorCombination == nullptr)
    {
        StereoSizeProvider::destroyInstance();
    }
    else
    {
        StereoSizeProvider::getInstance()->reset();
    }
    __logicalDeviceID = logicalDeviceID;

    const size_t SETTING_SIZE = g_sensorSettings.size();
    if(SETTING_SIZE > 0) {
        std::ostringstream oss;
        oss << "Sensor settings(" << SETTING_SIZE << "): ";
        std::copy(g_sensorSettings.begin(), g_sensorSettings.end()-1, std::ostream_iterator<void*>(oss, " "));
        oss << g_sensorSettings.back() << ", combination " << g_currentSensorCombination;
        MY_LOGD("%s", oss.str().c_str());
    }
    else
    {
        MY_LOGD("Cannot find any sensor setting for logical device ID %d", logicalDeviceID);
    }
}

void
StereoSettingProvider::setImageRatio(ENUM_STEREO_RATIO ratio)
{
    char value[PROP_VALUE_MAX];
    size_t len = ::property_get("vendor.STEREO.ratio", value, NULL);
    if(len > 0) {
        int m, n;
        sscanf(value, "%d:%d", &m, &n);
        ratio = static_cast<ENUM_STEREO_RATIO>(m<<STEREO_RATIO_CONSEQUENT_BITS|n);
        MY_LOGD("Manually set image ratio to %d:%d (value %s)", m, n, value);
    }

    __imageRatio = ratio;
    int m, n;
    imageRatioMToN(__imageRatio, m, n);
    MY_LOGD("Set image ratio to %d:%d", m, n);

    StereoSizeProvider::getInstance()->reset();
    _updateImageSettings();
    __initTuningIfReady();
}

void
StereoSettingProvider::setStereoFeatureMode(MINT32 stereoMode, bool isPortrait, bool isRecording)
{
    MY_LOGD("Set stereo feature mode: %s -> %s, portrait: %d, record: %d",
            _getStereoFeatureModeString(__stereoFeatureMode).c_str(),
            _getStereoFeatureModeString(stereoMode).c_str(), isPortrait, isRecording);

    if((__stereoFeatureMode & ~E_STEREO_FEATURE_PORTRAIT_FLAG) == stereoMode) {
        return;
    }

    //Update sensor scrnario
    if(isPortrait) {
        __stereoFeatureMode |= E_STEREO_FEATURE_PORTRAIT_FLAG;
    }

    if(stereoMode == 0) {
        StereoSizeProvider::destroyInstance();
    }
    else
    {
        StereoSizeProvider::getInstance()->reset();
    }

    __stereoFeatureMode = stereoMode;

    getSensorScenarios(__stereoFeatureMode, isRecording, __sensorScenarios, true);

    _updateImageSettings();
    if(!IS_STEREO_MODE(stereoMode)) {
        StereoTuningProviderKernel::destroyInstance();

        // MY_LOGD("Reset stereo setting");
        // StereoSettingProviderKernel::destroyInstance();
#if (1==VSDOF_SUPPORTED)
        FEFMSettingProvider::destroyInstance();
#endif
        return;
    }

    //Update FOV Crip
    if(g_currentSensorCombination &&
       g_sensorSettings.size() > 1)
    {
        MY_LOGE_IF((NULL==g_sensorSettings[0] || NULL == g_sensorSettings[1]),
                   "Sensor setting not found: %p %p", g_sensorSettings[0], g_sensorSettings[1]);

        if(g_sensorSettings[0]) {
            g_sensorSettings[0]->updateRuntimeFOV(__stereoFeatureMode);
            MY_LOGD("Main1 Runtime FOV: H/V: %.2f/%.2f(spec: %.2f/%.2f) Degrees",
                    g_sensorSettings[0]->fovHRuntime, g_sensorSettings[0]->fovVRuntime,
                    g_sensorSettings[0]->fovHorizontal, g_sensorSettings[0]->fovVertical);
        }
        if(g_sensorSettings[1]) {
            g_sensorSettings[1]->updateRuntimeFOV(__stereoFeatureMode);
            MY_LOGD("Main2 Runtime FOV: H/V: %.2f/%.2f(spec: %.2f/%.2f) Degrees",
                    g_sensorSettings[1]->fovHRuntime, g_sensorSettings[1]->fovVRuntime,
                    g_sensorSettings[1]->fovHorizontal, g_sensorSettings[1]->fovVertical);
        }

        if(FOVCropUtil::updateFOVCropRatios(*g_currentSensorCombination)) {
            if(g_sensorSettings[0] &&
               g_sensorSettings[0]->cropSetting.keepRatio < 1.0f)
            {
                MY_LOGD("Main1 FOV Crop: Keep ratio %.4f(Crop H/V: %.2f/%.2f Degrees) Baseline Ratio %.4f",
                        g_sensorSettings[0]->cropSetting.keepRatio,
                        g_sensorSettings[0]->cropSetting.cropDegreeH, g_sensorSettings[0]->cropSetting.cropDegreeV,
                        g_sensorSettings[0]->cropSetting.cropRatio);
            }
            else if(g_sensorSettings[1] &&
                    g_sensorSettings[1]->cropSetting.keepRatio < 1.0f)
            {
                MY_LOGD("Main2 FOV Crop: Keep ratio %.4f(Crop H/V: %.2f/%.2f Degrees) Baseline Ratio %.4f",
                        g_sensorSettings[1]->cropSetting.keepRatio,
                        g_sensorSettings[1]->cropSetting.cropDegreeH, g_sensorSettings[1]->cropSetting.cropDegreeV,
                        g_sensorSettings[1]->cropSetting.cropRatio);
            }
        }
    } else {
        MY_LOGE("Sensor combination is NULL, please call setLogicalDeviceID first");
    }
}

std::string
StereoSettingProvider::_getStereoFeatureModeString(int stereoMode)
{
    auto features = featureModeToStrings(stereoMode);
    if(features.size() > 0) {
        std::ostringstream oss;
        std::copy(features.begin(), features.end()-1, std::ostream_iterator<string>(oss, "+"));
        oss << features.back();

        if(stereoMode & E_STEREO_FEATURE_PORTRAIT_FLAG) {
            oss << "(portrait mode)";
        }

        return oss.str();
    }

    return "none";
}

bool
StereoSettingProvider::hasHWFE()
{
    static bool _hasHWFE = false;
    return _hasHWFE;
}

MUINT32
StereoSettingProvider::fefmBlockSize(const int FE_MODE)
{
    switch(FE_MODE)
    {
        case 0:
           return 32;
            break;
        case 1:
           return 16;
            break;
        case 2:
           return 8;
            break;
        default:
            break;
    }

    return 0;
}

bool
StereoSettingProvider::getStereoCameraFOV(SensorFOV &mainFOV, SensorFOV &main2FOV, MUINT32 logicalDeviceID)
{
    std::vector<StereoSensorSetting_T *> sensorSettings;
    if(logicalDeviceID == __logicalDeviceID) {
        sensorSettings = g_sensorSettings;
    } else {
        __getSensorSettings(logicalDeviceID, sensorSettings);
    }

    bool result = true;
    if(sensorSettings.size() > 1 &&
       NULL != sensorSettings[0] &&
       NULL != sensorSettings[1])
    {
        mainFOV.fov_horizontal  = sensorSettings[0]->fovHorizontal;
        mainFOV.fov_vertical    = sensorSettings[0]->fovVertical;
        main2FOV.fov_horizontal = sensorSettings[1]->fovHorizontal;
        main2FOV.fov_vertical   = sensorSettings[1]->fovVertical;
    }

    return result;
}

ENUM_ROTATION
StereoSettingProvider::getModuleRotation(MUINT32 logicalDeviceID)
{
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;

    if(logicalDeviceID != __logicalDeviceID) {
        sensorCombination = __getSensorCombinationSetting(logicalDeviceID);
    }

    ENUM_ROTATION rotate = eRotate_90;
    if(NULL != sensorCombination &&
       NULL != sensorCombination->sensorSettings[0])
    {
        if(1 == sensorCombination->moduleType ||
           3 == sensorCombination->moduleType)
        {
            if(0 == sensorCombination->sensorSettings[0]->staticInfo.facingDirection) {
                rotate =  eRotate_90;
            } else {
                rotate =  eRotate_270;
            }
        } else {
            rotate = eRotate_0;
        }
    }

    return rotate;
}

ENUM_STEREO_SENSOR_RELATIVE_POSITION
StereoSettingProvider::getSensorRelativePosition(MUINT32 logicalDeviceID)
{
    int sensorRalation = 0;
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;

    if(logicalDeviceID != __logicalDeviceID) {
        sensorCombination = __getSensorCombinationSetting(logicalDeviceID);
    }

    if(sensorCombination &&
       NULL != sensorCombination->sensorSettings[0])
    {
        if(1 == sensorCombination->sensorSettings[0]->staticInfo.facingDirection) {
            if(3 == sensorCombination->moduleType)
            {
                sensorRalation = 1;
            }
        } else {
            if(3 == sensorCombination->moduleType ||
               4 == sensorCombination->moduleType)
            {
                sensorRalation = 1;
            }
        }
    }

    return static_cast<ENUM_STEREO_SENSOR_RELATIVE_POSITION>(sensorRalation);
}

bool
StereoSettingProvider::isSensorAF(const int SENSOR_INDEX)
{
    bool isAF = false;
    IHal3A *pHal3A = MAKE_Hal3A(SENSOR_INDEX, LOG_TAG);
    if(NULL == pHal3A) {
        MY_LOGE("Cannot get 3A HAL of sensor %d", SENSOR_INDEX);
    } else {
        FeatureParam_T rFeatureParam;
        if(pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, (MUINTPTR)&rFeatureParam, 0)) {
            isAF = (rFeatureParam.u4MaxFocusAreaNum > 0);
        } else {
            MY_LOGW("Cannot query AF ability from 3A of sensor %d", SENSOR_INDEX);
        }

        pHal3A->destroyInstance(LOG_TAG);
        pHal3A = NULL;
    }

    MY_LOGD_IF(isLogEnabled(), "Sensor: %d, AF: %d", SENSOR_INDEX, isAF);

    return isAF;
}

bool
StereoSettingProvider::enableLog()
{
    return setProperty(PROPERTY_ENABLE_LOG, 1);
}

bool
StereoSettingProvider::enableLog(const char *LOG_PROPERTY_NAME)
{
    return setProperty(PROPERTY_ENABLE_LOG, 1) &&
           setProperty(LOG_PROPERTY_NAME, 1);
}

bool
StereoSettingProvider::disableLog()
{
    return setProperty(PROPERTY_ENABLE_LOG, 0);
}

bool
StereoSettingProvider::isLogEnabled()
{
    return (checkStereoProperty(PROPERTY_ENABLE_LOG) != 0);
}

bool
StereoSettingProvider::isLogEnabled(const char *LOG_PROPERTY_NAME)
{
    return isLogEnabled() && (checkStereoProperty(LOG_PROPERTY_NAME) != 0);
}

bool
StereoSettingProvider::isProfileLogEnabled()
{
    return isLogEnabled() || (checkStereoProperty(PROPERTY_ENABLE_PROFILE_LOG) != 0);
}

MUINT32
StereoSettingProvider::getExtraDataBufferSizeInBytes()
{
    return 8192;
}

MUINT32
StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes()
{
    return 100 * sizeof(MFLOAT);
}

MUINT32
StereoSettingProvider::getMaxSceneInfoBufferSizeInBytes()
{
    return 15 * sizeof(MINT32);
}

float
StereoSettingProvider::getStereoBaseline(MUINT32 logicalDeviceID)
{
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;
    if(logicalDeviceID != __logicalDeviceID) {
        sensorCombination = __getSensorCombinationSetting(logicalDeviceID);
    }

    float baseline = 1.0f;
    if(sensorCombination) {
        baseline = sensorCombination->baseline;
    }

    return baseline;
}

MUINT
StereoSettingProvider::getSensorRawFormat(const int SENSOR_INDEX)
{
    // for dev
    if( SENSOR_INDEX != SENSOR_DEV_MAIN &&
        SENSOR_INDEX != SENSOR_DEV_SUB &&
        checkStereoProperty("vendor.STEREO.debug.main2Mono") == 1 )
    {
        MY_LOGD("force main2 to be MONO sensor");
        return SENSOR_RAW_MONO;
    }

    //Use cached result if ready
    StereoSensorSetting_T *sensorSetting = StereoSettingProviderKernel::getInstance()->getSensorSetting(SENSOR_INDEX);
    if(sensorSetting) {
        return sensorSetting->staticInfo.rawFmtType;
    }

    IHalSensorList *sensorList = MAKE_HalSensorList();
    if (NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
        return SENSOR_RAW_Bayer;
    }

    SensorStaticInfo sensorStaticInfo;
    ::memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    int sensorDevIndex = sensorList->querySensorDevIdx(SENSOR_INDEX);
    sensorList->querySensorStaticInfo(sensorDevIndex, &sensorStaticInfo);

    return sensorStaticInfo.rawFmtType;
}

bool
StereoSettingProvider::isDeNoise()
{
#if (1 == STEREO_DENOISE_SUPPORTED)
    return (__stereoFeatureMode & E_STEREO_FEATURE_DENOISE);
#else
    return false;
#endif
}

bool
StereoSettingProvider::is3rdParty(ENUM_STEREO_SCENARIO scenario)
{
    if(__stereoFeatureMode & NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY)
    {
        return true;
    }

    if(eSTEREO_SCENARIO_CAPTURE == scenario)
    {
         return (   (__stereoFeatureMode & NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY)
                 || (__stereoFeatureMode & NSCam::v1::Stereo::E_STEREO_FEATURE_MULTI_CAM)
                );
    }

    return !IS_TK_VSDOF_MODE(__stereoFeatureMode);
}

bool
StereoSettingProvider::isBayerPlusMono()
{
    int32_t main1Idx, main2Idx;
    getStereoSensorIndex(main1Idx, main2Idx, __logicalDeviceID);
    return (SENSOR_RAW_MONO == getSensorRawFormat(main2Idx));
}

bool
StereoSettingProvider::isBMVSDoF()
{
    return (!isDeNoise() && isBayerPlusMono());
}

bool
StereoSettingProvider::isDualCamMode()
{
#if (1 == STEREO_CAMERA_SUPPORTED)
    MUINT32 supportedMode = (E_STEREO_FEATURE_CAPTURE|E_STEREO_FEATURE_THIRD_PARTY|E_STEREO_FEATURE_MULTI_CAM);
    #if (1 == STEREO_DENOISE_SUPPORTED)
        supportedMode |= E_STEREO_FEATURE_DENOISE;
    #endif
    #if (1 == VSDOF_SUPPORTED)
        supportedMode |= E_STEREO_FEATURE_VSDOF;
        supportedMode |= E_STEREO_FEATURE_MTK_DEPTHMAP;
    #endif
    #if (1 == DUAL_ZOOM_SUPPORTED)
        supportedMode |= E_DUALCAM_FEATURE_ZOOM;
    #endif

    return (__stereoFeatureMode & supportedMode);
#endif

    return false;
}

void
StereoSettingProvider::_updateImageSettings()
{
    for(auto &sensorSetting : g_sensorSettings)
    {
        auto imgoIter = sensorSetting->imgoYuvSize.find(__imageRatio);
        if(imgoIter != sensorSetting->imgoYuvSize.end()) {
            StereoSizeProvider::getInstance()->setIMGOYUVSize(sensorSetting->index, imgoIter->second);
        }

        auto rrzoIter = sensorSetting->rrzoYuvSize.find(__imageRatio);
        if(rrzoIter != sensorSetting->rrzoYuvSize.end()) {
            StereoSizeProvider::getInstance()->setRRZOYUVSize(sensorSetting->index, rrzoIter->second);
        }
    }

    if(NULL != g_currentSensorCombination &&
       g_currentSensorCombination->hasSizeConfig)
    {
        ENUM_STEREO_RATIO ratio;
        for(auto &m1 : g_currentSensorCombination->baseSize) {
            ratio = static_cast<ENUM_STEREO_RATIO>(m1.first);
            StereoSizeProvider::getInstance()->setCustomizedSize(ratio, m1.second);
        }
    }
}

vector<float> EMPTY_LDC;
vector<float> &
StereoSettingProvider::getLDCTable()
{
    if(g_currentSensorCombination) {
        return g_currentSensorCombination->LDC;
    }

    return EMPTY_LDC;
}

bool
StereoSettingProvider::LDCEnabled()
{
    if(g_currentSensorCombination) {
        return g_currentSensorCombination->enableLDC;
    }

    return false;
}

CUST_FOV_CROP_T
StereoSettingProvider::getFOVCropSetting()
{
    if(g_sensorSettings.size() < 2 ||
       NULL == g_sensorSettings[0] ||
       NULL == g_sensorSettings[1])
    {
        MY_LOGE("Sensor settings not found");
        return CUST_FOV_CROP_T();
    }

    if(NULL != g_currentSensorCombination &&
       g_currentSensorCombination->disableCrop)
    {
        return CUST_FOV_CROP_T();
    }

    if(g_sensorSettings[0]->cropSetting.keepRatio < 1.0f) {
        return g_sensorSettings[0]->cropSetting.toCusCrop();
    }

    if(g_sensorSettings[1]->cropSetting.keepRatio < 1.0f) {
        return g_sensorSettings[1]->cropSetting.toCusCrop();
    }

    return CUST_FOV_CROP_T();
}

float
StereoSettingProvider::getStereoCameraFOVRatio(MUINT32 logicalDeviceID)
{
    if(isDeNoise()) {
        return 1.0f;
    }

    float fovRatio = 1.0f;
    std::vector<StereoSensorSetting_T *> sensorSettings = g_sensorSettings;
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;
    if(logicalDeviceID == __logicalDeviceID) {
        sensorSettings = g_sensorSettings;
    } else {
        __getSensorSettingsAndCombination(logicalDeviceID, sensorSettings, sensorCombination);
    }

    if(sensorSettings.size() < 2 ||
       NULL == sensorSettings[0] ||
       NULL == sensorSettings[1] ||
       NULL == sensorCombination)
    {
        MY_LOGE("Sensor settings of logical device ID %d not found: %p %p, combination: %p",
                logicalDeviceID, sensorSettings[0], sensorSettings[1], sensorCombination);
        return 1.0f;
    }

    float main1FOV = sensorSettings[0]->fovHRuntime;
    float main2FOV = sensorSettings[1]->fovHRuntime;
    if(StereoSettingProvider::getModuleRotation() & 0x2) {
        main1FOV = sensorSettings[0]->fovVRuntime;
        main2FOV = sensorSettings[1]->fovVRuntime;
    }

    if(fabs(main1FOV - main2FOV) < 2.0f*sensorCombination->moduleVariation) {
        //Main1 is cropped and enlarged by FOV crop, so need to apply same ratio to main2
        fovRatio = 1.0f/sensorSettings[0]->cropSetting.keepRatio;
    }
    else if(main1FOV < main2FOV)
    {
        //Crop main2 FOV to be a little bigger than main1
        fovRatio = tan(degreeToRadians(main2FOV/2.0f)) * sensorSettings[1]->cropSetting.keepRatio / tan(degreeToRadians(main1FOV/2.0f));
    }
    else if(main1FOV > main2FOV)
    {
        //Crop main1 FOV to be smaller than main2
        fovRatio = tan(degreeToRadians(main2FOV/2.0f))/(tan(degreeToRadians(main1FOV/2.0f)) * sensorSettings[0]->cropSetting.keepRatio);
    }

    return fovRatio;
}

float
StereoSettingProvider::getFOVCropRatio()
{
    if( StereoSettingProvider::isDeNoise() )
    {
        return 1.0f;
    }

    if(g_sensorSettings.size() < 2 ||
       NULL == g_sensorSettings[0] ||
       NULL == g_sensorSettings[1])
    {
        MY_LOGE("Sensor settings not found");
        return 1.0f;
    }

    if(NULL != g_currentSensorCombination &&
       g_currentSensorCombination->disableCrop)
    {
        return 1.0f;
    }

    float ratio = std::min(g_sensorSettings[0]->cropSetting.keepRatio, g_sensorSettings[1]->cropSetting.keepRatio);
    MY_LOGD_IF(isLogEnabled(), "FOV Crop Ratio %.4f", ratio);
    return ratio;
}

float
StereoSettingProvider::getMain1FOVCropRatio()
{
    if( StereoSettingProvider::isDeNoise() )
    {
        return 1.0f;
    }

    if(NULL != g_currentSensorCombination &&
       g_currentSensorCombination->disableCrop)
    {
        return 1.0f;
    }

    if(g_sensorSettings.size() > 0 &&
       NULL != g_sensorSettings[0])
    {
        return g_sensorSettings[0]->cropSetting.keepRatio;
    } else {
        MY_LOGE("Sensor setting of main1 not found");
    }

    return 1.0f;
}

float
StereoSettingProvider::getMain2FOVCropRatio()
{
    if( StereoSettingProvider::isDeNoise() )
    {
        return 1.0f;
    }

    if(NULL != g_currentSensorCombination &&
       g_currentSensorCombination->disableCrop)
    {
        return 1.0f;
    }

    if(g_sensorSettings.size() > 1 &&
       NULL != g_sensorSettings[1])
    {
        return g_sensorSettings[1]->cropSetting.keepRatio;
    } else {
        MY_LOGE("Sensor setting of main1 not found");
    }

    return 1.0f;
}

EShotMode
StereoSettingProvider::getShotMode()
{
    return eShotMode_ZsdShot;
}

void
StereoSettingProvider::setStereoModuleType(MINT32 moduleType)
{
    __stereoModuleType = moduleType;
}

size_t
StereoSettingProvider::getDPECaptureRound()
{
    int dpeRound = checkStereoProperty("vendor.STEREO.tuning.dpe_round", DPE_CAPTURE_ROUND);
    if(dpeRound < 0) {
        dpeRound = 0;
    }

    return (size_t)dpeRound;
}

void
StereoSettingProvider::setStereoShotMode(MUINT32 stereoShotMode)
{
    __stereoShotMode = stereoShotMode;
}

size_t
StereoSettingProvider::getMaxN3DDebugBufferSizeInBytes()
{
    return 1024*1024*3;
}

bool
StereoSettingProvider::isWidePlusTele(MUINT32 logicalDeviceID)
{
    std::vector<StereoSensorSetting_T *> sensorSettings;
    if(logicalDeviceID == __logicalDeviceID) {
        sensorSettings = g_sensorSettings;
    } else {
        __getSensorSettings(logicalDeviceID, sensorSettings);
    }

    bool result = false;
    if(sensorSettings.size() > 0 &&
       NULL != sensorSettings[0])
    {
        //W+T will exchange sensor index
        if(sensorSettings[0]->index != SENSOR_DEV_MAIN &&
           sensorSettings[0]->index != SENSOR_DEV_SUB)
        {
            return true;
        }
    }

    return result;
}

bool
StereoSettingProvider::isWideTeleVSDoF(MUINT32 logicalDeviceID)
{
    if(isWidePlusTele(logicalDeviceID) &&
       IS_STEREO_MODE(__stereoFeatureMode))
    {
        return true;
    }

    return false;
}

bool
StereoSettingProvider::getSensorScenario(MINT32 stereoMode,
                                         bool isRecording,
                                         MUINT &sensorScenarioMain1,
                                         MUINT &sensorScenarioMain2,
                                         bool updateToo)
{
    std::vector<MUINT> scenarios;
    bool isOK = getSensorScenarios(stereoMode, isRecording, scenarios, updateToo);
    if(isOK &&
       scenarios.size() > 1)
    {
        sensorScenarioMain1 = scenarios[0];
        sensorScenarioMain2 = scenarios[1];
    }
    else
    {
        sensorScenarioMain1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorScenarioMain2 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    }

    return isOK;
}

void
StereoSettingProvider::updateSensorScenario(MUINT sensorScenarioMain1,
                                            MUINT sensorScenarioMain2)
{
    __sensorScenarios.clear();
    __sensorScenarios.push_back(sensorScenarioMain1);
    __sensorScenarios.push_back(sensorScenarioMain2);
    MY_LOGD("Update sensor scenario: %d(%s) %d(%s)",
            __sensorScenarios[0], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[0]),
            __sensorScenarios[1], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[1]));
}

bool
StereoSettingProvider::getSensorScenarios(MINT32 stereoMode,
                                          bool isRecording,
                                          std::vector<MUINT> &sensorScenarios,
                                          bool updateToo)
{
    bool isOK = (g_sensorSettings.size() > 0);
    auto &src = (!isRecording) ? g_currentSensorCombination->sensorScenariosZSD
                               : g_currentSensorCombination->sensorScenariosRecord;
    if(src.size() == g_sensorSettings.size())
    {
        sensorScenarios.assign(src.begin(), src.end());
    }
    else
    {
        sensorScenarios.clear();
        MINT32 pipelineMode = (!isRecording) ? PipelineMode_ZSD : PipelineMode_RECORDING;
        const MUINT DEFAULT_SCENARIO = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        for(auto &setting : g_sensorSettings) {
            if(setting) {
                MUINT sensorScenario = setting->getSensorScenario(stereoMode, pipelineMode);
                if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenario) {
                    sensorScenario = DEFAULT_SCENARIO;
                    if(stereoMode)
                    {
                        isOK = false;
                        MY_LOGW("Cannot get sensor setting of main%zu, use %s as sensor scenario",
                                sensorScenarios.size()+1,
                                SENSOR_SCENARIO_TO_STRING_MAP.at(DEFAULT_SCENARIO));
                    }
                }
                sensorScenarios.push_back(sensorScenario);
            }
        }
    }

    if(isOK &&
       updateToo &&
       stereoMode != 0)
    {
        updateSensorScenarios(sensorScenarios);
        return true;
    }

    return isOK;
}

void
StereoSettingProvider::updateSensorScenarios(const std::vector<MUINT> &sensorScenarios)
{
    __sensorScenarios = sensorScenarios;

    const size_t SIZE = __sensorScenarios.size();
    if(SIZE > 2) {
        MY_LOGD("Update sensor scenarios(%zu): %d(%s) %d(%s) %d(%s)", SIZE,
                __sensorScenarios[0], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[0]),
                __sensorScenarios[1], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[1]),
                __sensorScenarios[2], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[2]));
    } else if(SIZE > 1) {
        MY_LOGD("Update sensor scenarios(2): %d(%s) %d(%s)",
                __sensorScenarios[0], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[0]),
                __sensorScenarios[1], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[1]));
    } else if(SIZE > 0) {
        MY_LOGD("Update sensor scenario(1): %d(%s)",
                __sensorScenarios[0], SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarios[0]));
    } else {
        MY_LOGD("Cannot find sensor scenario");
    }
}

MUINT
StereoSettingProvider::getSensorScenarioMain1()
{
    if(__sensorScenarios.size() > 0) {
        return __sensorScenarios[0];
    }

    MY_LOGW("Cannot get sensor scenario of main1, return CAPTURE as default");
    return SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
}

MUINT
StereoSettingProvider::getSensorScenarioMain2()
{
    if(__sensorScenarios.size() > 1) {
        return __sensorScenarios[1];
    }

    MY_LOGW("Cannot get sensor scenario of main2, return CAPTURE as default");
    return SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
}

MUINT
StereoSettingProvider::getCurrentSensorScenario(StereoHAL::ENUM_STEREO_SENSOR sensor)
{
    if(sensor > 0 &&
       sensor <= __sensorScenarios.size())
    {
        return __sensorScenarios[sensor-1];
    }

    MY_LOGE("Cannot get current sensor scenario of main%d, return CAPTURE as default", sensor);
    return SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
}

bool
StereoSettingProvider::getCalibrationDistance(ENUM_STEREO_SENSOR sensor, MUINT32 &macroDistance, MUINT32 &infiniteDistance)
{
    bool result = true;
    if(eSTEREO_SENSOR_MAIN1 == sensor) {
        if(g_sensorSettings.size() > 0 &&
           g_sensorSettings[0])
        {
            macroDistance    = g_sensorSettings[0]->distanceMacro;
            infiniteDistance = g_sensorSettings[0]->distanceInfinite;
        } else {
            macroDistance    = 100;
            infiniteDistance = 5000;
            result = false;
            MY_LOGW("Cannot get sensor setting of main1, use %d & %d for calibration distance", macroDistance, infiniteDistance);
        }
    } else {
        if(g_sensorSettings.size() > 1 &&
           g_sensorSettings[1])
        {
            macroDistance    = g_sensorSettings[1]->distanceMacro;
            infiniteDistance = g_sensorSettings[1]->distanceInfinite;
        } else {
            macroDistance    = 100;
            infiniteDistance = 5000;
            result = false;
            MY_LOGW("Cannot get sensor setting of main2, use %d & %d for calibration distance", macroDistance, infiniteDistance);
        }
    }

    return result;
}

std::string
StereoSettingProvider::getCallbackBufferList()
{
    char value[PROP_VALUE_MAX];
    std::string result = StereoSettingProviderKernel::getInstance()->getCallbackBufferListString();
    size_t len = ::property_get("vendor.STEREO.callback_list", value, NULL);
    if(len > 0) {
        result.assign(value);
    }

    if(0 == result.size()) {
        // ci: Clean Image, bi:Bokeh Image, mbd: MTK Bokeh Depth, mdb: MTK Debug Buffer, mbm: MTK Bokeh Metadata
        const char *DEFAULT_LIST = "bi";
        MY_LOGD_IF(__stereoFeatureMode != 0 && isLogEnabled(), "Callback list are not set, use default(%s)", DEFAULT_LIST);
        result.assign(DEFAULT_LIST);
    }

    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    MY_LOGD_IF((__stereoFeatureMode != 0 && isLogEnabled())||(len>0), "Callback list: %s", result.c_str());

    return result;
}

std::bitset<NSCam::v1::Stereo::CallbackBufferType::E_DUALCAM_JPEG_ENUM_SIZE>
StereoSettingProvider::getBokehJpegBufferList()
{
    std::string value = getCallbackBufferList();
    std::bitset<CallbackBufferType::E_DUALCAM_JPEG_ENUM_SIZE> result;
    result.reset();
    std::stringstream ss(value);
    std::string token;

    while(std::getline(ss, token, ','))
    {
        if(token.compare("ci") == 0)
        {
            result.set(E_DUALCAM_JPEG_CLEAN_IMAGE);
        }
        else if(token.compare("bi") == 0)
        {
            result.set(E_DUALCAM_JPEG_BOKEH_IMAGE);
        }
        else if(token.compare("mbd") == 0)
        {
            result.set(E_DUALCAM_JPEG_MTK_BOKEH_DEPTH);
        }
        else if(token.compare("mdb") == 0)
        {
            result.set(E_DUALCAM_JPEG_MTK_DEBUG_BUFFER);
        }
        else if(token.compare("mdm") == 0)
        {
            result.set(E_DUALCAM_JPEG_MTK_BOKEH_METADATA);
        }
        else if(token.compare("rbi") == 0)
        {
            result.set(E_DUALCAM_JPEG_RELIGHTING_BOKEH_IMAGE);
        }
        else
        {
            MY_LOGE("not support please check code consist or not. check target: stereo_setting, StereoCamEnum.");
        }
    }

    if(isLogEnabled())
    {
        string s = result.to_string();
        MY_LOGD("Callback list: %s(%s)", s.c_str(), value.c_str());
    }

    return result;
}

void
StereoSettingProvider::setPreviewSize(NSCam::MSize pvSize)
{
    MY_LOGD("Set preview size %dx%d", pvSize.w, pvSize.h);
    StereoSizeProvider::getInstance()->setPreviewSize(pvSize);

    const int MAX_ERROR = 2;    // percent
    static const std::map<std::pair<int, int>, ENUM_STEREO_RATIO> RATIO_MAP =
    {   //{m, n}
        {{16, 9}, eRatio_16_9},
        {{4,  3}, eRatio_4_3},
    };

    int m, n;
    bool isKnownRatio = false;
    for(auto &r : RATIO_MAP) {
        m = r.first.first;
        n = r.first.second;
        if(abs(pvSize.w * n / m - pvSize.h) < pvSize.h * MAX_ERROR / 100) {
            setImageRatio(r.second);
            isKnownRatio = true;
            break;
        }
    }

    if(!isKnownRatio) {
        int gcd = StereoGCD(pvSize.w, pvSize.h);
        m = pvSize.w/gcd;
        n = pvSize.h/gcd;
        MY_LOGW("Set preview size %dx%d in unknown ratio, set image ratio as %d:%d",
                 pvSize.w, pvSize.h, m, n);

        setImageRatio(MToNToImageRatio(m, n));
    }
}

size_t
StereoSettingProvider::getCalibrationData(void *calibrationData, MUINT32 logicalDeviceID)
{
    if(NULL == calibrationData) {
        MY_LOGE("Invalid input calibration data address");
        return 0;
    }

    StereoSensorConbinationSetting_T *pSC = __getSensorCombinationSetting(logicalDeviceID);
    if(NULL == pSC ||
       0 == pSC->calibrationData.size() ||
       pSC->calibrationData.size() > CAM_CAL_Stereo_Data_SIZE)
    {
        MY_LOGD("Invalid offline calibration data");
        return 0;
    }

    ::memcpy(calibrationData, &(pSC->calibrationData[0]), pSC->calibrationData.size());
    MY_LOGD("return calibration data in %zu bytes", pSC->calibrationData.size());
    return pSC->calibrationData.size();
}

int
StereoSettingProvider::getISPProfileForP1YUV()
{
#if (1==HAS_P1YUV)
    int profile = isP1YUVSupported() ? NSIspTuning::EIspProfile_P1_YUV_Depth : NSIspTuning::EIspProfile_N3D_Preview;
    profile = ::property_get_int32("vendor.p1yuv.profile", profile);
    return profile;
#else
    return NSIspTuning::EIspProfile_N3D_Preview;
#endif
}

bool
StereoSettingProvider::isP1YUVSupported()
{
#if (1==HAS_P1YUV)
    bool result = true;
#else
    bool result = false;
#endif

    return ::property_get_bool("ro.vendor.camera.directscaledyuv.support", result);
}

NSCam::v3::P1Node::RESIZE_QUALITY
StereoSettingProvider::getVSDoFP1ResizeQuality()
{
    NSCam::v3::P1Node::RESIZE_QUALITY quality = NSCam::v3::P1Node::RESIZE_QUALITY_H;
// #if (1==HAS_P1_RESIZE_QUALITY)
//     if(IS_STEREO_MODE(__stereoFeatureMode) &&
//        NULL != g_currentSensorCombination &&
//        NULL != g_currentSensorCombination->sensorSettings[0])
//     {
//         NSCam::SensorStaticInfo &staticInfo =g_currentSensorCombination->sensorSettings[0]->staticInfo;
//         MY_LOGD("Feature mode: %s, sensor %dx%d",
//                 _getStereoFeatureModeString(__stereoFeatureMode).c_str(),
//                 staticInfo.captureWidth, staticInfo.captureHeight);
//         if(staticInfo.captureWidth * staticInfo.captureHeight > 17000000)
//         {
//             quality = NSCam::v3::P1Node::RESIZE_QUALITY_L;
//         }
//     }
// #endif

    quality = static_cast<NSCam::v3::P1Node::RESIZE_QUALITY>(::property_get_int32("vendor.STEREO.p1resizequality", quality));
    MY_LOGD("P1 resize quality: %s", (NSCam::v3::P1Node::RESIZE_QUALITY_H==quality)?"High":"Low");
    return quality;
}

bool
StereoSettingProvider::getSensorFOV(const int SENSOR_INDEX, float &fovHorizontal, float &fovVertical)
{
    StereoSensorSetting_T *setting = StereoSettingProviderKernel::getInstance()->getSensorSetting(SENSOR_INDEX);
    if(setting)
    {
        fovHorizontal = setting->fovHorizontal;
        fovVertical   = setting->fovVertical;
        return true;
    }

    return false;
}

MTK_DEPTHMAP_INFO_T
StereoSettingProvider::getDepthmapInfo(ENUM_STEREO_RATIO ratio, ENUM_STEREO_SCENARIO eScenario)
{
    MTK_DEPTHMAP_INFO_T result;
    result.size = StereoSizeProvider::getInstance()->thirdPartyDepthmapSize(ratio, eScenario);
    result.stride = result.size.w;
    result.format = StereoSettingProviderKernel::getInstance()->getDepthmapFormat();

    /**
     * Y8 is a YUV planar format comprised of a WxH Y plane, with each pixel
     * being represented by 8 bits. It is equivalent to just the Y plane from
     * YV12.
     *
     * This format assumes
     * - an even width
     * - an even height
     * - a horizontal stride multiple of 16 pixels
     * - a vertical stride equal to the height
     */

    /**
     * Y16 is a YUV planar format comprised of a WxH Y plane, with each pixel
     * being represented by 16 bits. It is just like Y8, but has double the
     * bits per pixel (little endian).
     *
     * This format assumes
     * - an even width
     * - an even height
     * - a horizontal stride multiple of 16 pixels
     * - a vertical stride equal to the height
     * - strides are specified in pixels, not in bytes
     */
    StereoHAL::applyNAlign(2,  result.size.w);
    StereoHAL::applyNAlign(2,  result.size.h);
    StereoHAL::applyNAlign(16, result.stride);

    return result;
}

MUINT32
StereoSettingProvider::getMain2OutputFrequency(ENUM_STEREO_SCENARIO scenario)
{
    MUINT32 freq = 1;
    if(is3rdParty(scenario)) {
        freq = StereoSettingProviderKernel::getInstance()->getMain2OutputFrequecy(scenario);
        MY_LOGD("Main2 output frequency: %u(%s)", freq,
                (scenario==eSTEREO_SCENARIO_CAPTURE) ? "Capture" : "Preview");
    }

    return freq;
}

void
StereoSettingProvider::set3rdDepthAlgoRunning(bool isDepthRunning)
{
    __3rdCaptureDepthRunning = isDepthRunning;
}

void
StereoSettingProvider::set3rdBokehAlgoRunning(bool isBokehRunning)
{
    __3rdCaptureBokehRunning = isBokehRunning;
}

bool
StereoSettingProvider::get3rdDepthOrBokehAlgoRunning()
{
    return (__3rdCaptureDepthRunning || __3rdCaptureBokehRunning);
}

bool
StereoSettingProvider::isAFSyncNeeded(MUINT32 logicalDeviceID)
{
    bool isNeeded = false;
    // if(IS_STEREO_MODE(__stereoFeatureMode))
    {
        int sensorIndex[2];
        getStereoSensorIndex(sensorIndex[0], sensorIndex[1], logicalDeviceID);
        //For most of back dual cam, main2 sensor is usually FF, so we check main2 first
        isNeeded = (isSensorAF(sensorIndex[1]) && isSensorAF(sensorIndex[0]));
        MY_LOGD_IF(isLogEnabled(), "Logical device %d, Feature %s, need AF sync %d",
                    logicalDeviceID, _getStereoFeatureModeString(__stereoFeatureMode).c_str(),
                    isNeeded)
    }
    return isNeeded;
}

ENUM_DEPTHMAP_REFINE_LEVEL
StereoSettingProvider::getDepthmapRefineLevel()
{
    if(g_currentSensorCombination) {
        return g_currentSensorCombination->depthmapRefineLevel;
    }

    return E_DEPTHMAP_REFINE_DEFAULT;
}

std::vector<float>
StereoSettingProvider::getMulticamZoomRange(MUINT32 logicalDeviceID)
{
    StereoSensorConbinationSetting_T *sensorCombination =
        (logicalDeviceID == __logicalDeviceID)
        ? g_currentSensorCombination
        : __getSensorCombinationSetting(logicalDeviceID);

    if(sensorCombination) {
        return sensorCombination->multicamZoomRanges;
    }

    return {1.0f};
}

std::vector<float>
StereoSettingProvider::getMulticamZoomSteps(MUINT32 logicalDeviceID)
{
    StereoSensorConbinationSetting_T *sensorCombination =
        (logicalDeviceID == __logicalDeviceID)
        ? g_currentSensorCombination
        : __getSensorCombinationSetting(logicalDeviceID);

    if(sensorCombination) {
        return sensorCombination->multicamZoomSteps;
    }

    return {1.0f};
}

float
StereoSettingProvider::getFRZRatio(int32_t sensorIndex, int featureMode)
{
    float frzRatio = -1;

    StereoSensorSetting_T *sensorSetting = StereoSettingProviderKernel::getInstance()->getSensorSetting(sensorIndex);
    if(sensorSetting)
    {
        auto it = sensorSetting->frzRatioSetting.find(featureMode);
        if(it != sensorSetting->frzRatioSetting.end())
        {
            frzRatio = it->second;
        }
    }

    if(frzRatio < 0 ||
       frzRatio > 1.0f)
    {
        frzRatio = 0.5f;
        MY_LOGD("Unknown FRZ ratio in setting for sensor %d, feature %s, use %.1f",
                sensorIndex, _getStereoFeatureModeString(__stereoFeatureMode).c_str(), frzRatio);
    }

    return frzRatio;
}

int32_t
StereoSettingProvider::sensorEnumToId(ENUM_STEREO_SENSOR sensor, int logicalDeviceID)
{
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if(NULL == pHalDeviceList) {
        MY_LOGE("Cannot get sensor list");
        return -1;
    }

    auto sensorIDs = pHalDeviceList->getSensorId(logicalDeviceID);
    if(sensor <= sensorIDs.size()) {
        return sensorIDs[sensor-1];
    }
    else
    {
        MY_LOGE("Invalid stereo sensor index %d, sensor count of logical device %d: %zu",
                sensor, logicalDeviceID, sensorIDs.size());
        return -1;
    }
}

ENUM_STEREO_SENSOR
StereoSettingProvider::sensorIdToEnum(const int32_t SENSOR_INDEX, int logicalDeviceID)
{
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if(NULL == pHalDeviceList) {
        MY_LOGE("Cannot get sensor list");
        return eSTEREO_SENSOR_UNKNOWN;
    }

    auto sensorIDs = pHalDeviceList->getSensorId(logicalDeviceID);
    int index = 1;
    for(auto &id : sensorIDs) {
        if(id == SENSOR_INDEX) {
            return static_cast<ENUM_STEREO_SENSOR>(index);
        }
        ++index;
    }

    MY_LOGE("Sensor index %d is not found in logical device %d",
            SENSOR_INDEX, logicalDeviceID);
    return eSTEREO_SENSOR_UNKNOWN;
}

std::vector<float>
StereoSettingProvider::getLensPoseTranslation(int32_t sensorId)
{
    return StereoSettingProviderKernel::getInstance()->getLensPoseTranslation(sensorId);
}

bool
StereoSettingProvider::__initTuningIfReady()
{
    IHalSensorList *sensorList = MAKE_HalSensorList();
    const MUINT32 SENSOR_COUNT = (sensorList) ? sensorList->queryNumberOfSensors() : 3;
    if(__logicalDeviceID < SENSOR_COUNT) {
        MY_LOGD("Wait to set logical device, current %d", __logicalDeviceID);
        return false;
    }

    if(0 == __imageRatio) {
        MY_LOGD("Wait to set image ratio");
        return false;
    }

    if(!IS_STEREO_MODE(__stereoFeatureMode))
    {
        MY_LOGD("Wait to set stereo feature mode, current: %s",
                _getStereoFeatureModeString(__stereoFeatureMode).c_str());
        return false;
    }

    StereoTuningProviderKernel::getInstance()->init();  //Do init and loading
    return true;
}
