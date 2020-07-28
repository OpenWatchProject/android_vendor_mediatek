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
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include "../tuning-provider/stereo_tuning_provider_kernel.h"

#include <sstream>  //For ostringstream

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/def/ImageFormat.h>
#include <camera_custom_stereo.h>       // For CUST_STEREO_* definitions.
#include <mtkcam/utils/std/Log.h>
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
#include "stereo_setting_def.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START          MY_LOGD_IF(LOG_ENABLED, "+")
#define FUNC_END            MY_LOGD_IF(LOG_ENABLED, "-")

#define IS_STEREO_MODE(mode) ((mode & E_STEREO_FEATURE_VSDOF) || \
                              (mode & E_STEREO_FEATURE_DENOISE) || \
                              (mode & E_STEREO_FEATURE_CAPTURE) || \
                              (mode & E_STEREO_FEATURE_THIRD_PARTY) || \
                              (mode & E_STEREO_FEATURE_MTK_DEPTHMAP) || \
                              (mode & E_STEREO_FEATURE_MULTI_CAM))

using namespace NSCam;
using namespace android;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;
using namespace NS3Av3;

ENUM_STEREO_SENSOR_PROFILE StereoSettingProvider::m_stereoProfile = STEREO_SENSOR_PROFILE_UNKNOWN;
ENUM_STEREO_RATIO StereoSettingProvider::m_imageRatio = eRatio_Default;
MINT32 StereoSettingProvider::m_stereoFeatureMode = 0;
MINT32 StereoSettingProvider::m_stereoModuleType = -1;
MUINT32 StereoSettingProvider::m_stereoShotMode = 0;
bool StereoSettingProvider::m_isTestMode = false;
MUINT StereoSettingProvider::__sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
MUINT StereoSettingProvider::__sensorScenarioMain2 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
MUINT32 StereoSettingProvider::__featureStartTime = 0;
ENUM_STEREO_CAM_SCENARIO StereoSettingProvider::__vsdofScenario = eStereoCamPreview;

StereoSensorSetting_T *g_sensorSettings[2] = {NULL, NULL};
StereoSensorConbinationSetting_T *g_currentSensorCombination = NULL;

StereoSensorConbinationSetting_T *
__getSensorCombinationSetting(ENUM_STEREO_SENSOR_PROFILE profile)
{
    return StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(profile);
}

bool __getSensorSettings(ENUM_STEREO_SENSOR_PROFILE profile,
                         StereoSensorSetting_T *&main1Setting,
                         StereoSensorSetting_T *&main2Setting)
{
    bool result = true;
    StereoSensorConbinationSetting_T *sensorCombination =
        StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(profile);

    if(sensorCombination)
    {
        main1Setting = sensorCombination->sensorSettings[0];
        main2Setting = sensorCombination->sensorSettings[1];
    } else {
        MY_LOGW("Cannot get sensor settings");
        result = false;
    }

    return result;
}

bool __getSensorSettingsAndCombination(ENUM_STEREO_SENSOR_PROFILE profile,
                                       StereoSensorSetting_T *&main1Setting,
                                       StereoSensorSetting_T *&main2Setting,
                                       StereoSensorConbinationSetting_T *&sensorCombination)
{
    bool result = true;
    sensorCombination = StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting(profile);

    if(sensorCombination)
    {
        main1Setting = sensorCombination->sensorSettings[0];
        main2Setting = sensorCombination->sensorSettings[1];
    } else {
        MY_LOGW("Cannot get sensor settings");
        result = false;
    }

    return result;
}

bool
StereoSettingProvider::getStereoSensorIndex(int32_t &main1Idx, int32_t &main2Idx, ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(!StereoSettingProviderKernel::getInstance()->getSensorIndexesOfProfile(profile, main1Idx, main2Idx))
    {
        const int SENSOR_COUNT = (!m_isTestMode) ? MAKE_HalSensorList()->queryNumberOfSensors() : 3;
        if(2 == SENSOR_COUNT) {
            // MY_LOGW("Only two sensors were found on the device");
            main1Idx = 0;
            main2Idx = 1;
        } else {
            switch(profile)
            {
            case STEREO_SENSOR_PROFILE_REAR_REAR:
            default:
                main1Idx = 0;
                main2Idx = 2;
                break;
            case STEREO_SENSOR_PROFILE_FRONT_FRONT:
                main1Idx = 1;
                main2Idx = 3;
                break;
            case STEREO_SENSOR_PROFILE_REAR_FRONT:
                main1Idx = 0;    //main cam in rear
                main2Idx = 1;
                break;
            }
        }

        MY_LOGW("Cannot get sensor index by profile %s, return %d, %d",
                __getStereoProfileString(profile).c_str(), main1Idx, main2Idx);

        return false;
    }

    return true;
}

bool
StereoSettingProvider::getStereoSensorDevIndex(int32_t &main1DevIdx, int32_t &main2DevIdx, ENUM_STEREO_SENSOR_PROFILE profile)
{
    int32_t main1Idx = 0;
    int32_t main2Idx = 0;
    if (!getStereoSensorIndex(main1Idx, main2Idx, profile)) {
        return false;
    }

    if(!m_isTestMode) {
        IHalSensorList *sensorList = MAKE_HalSensorList();
        if (NULL == sensorList) {
            return false;
        }

        main1DevIdx = sensorList->querySensorDevIdx(main1Idx);
        main2DevIdx = sensorList->querySensorDevIdx(main2Idx);
    } else {
        main1DevIdx = 1<<main1Idx;
        main2DevIdx = 1<<main2Idx;
    }
    // MY_LOGD_IF(isLogEnabled(), "Main sensor DEV idx %d, Main2 sensor DEV idx %d", main1DevIdx, main2DevIdx);

    return true;
}

void
StereoSettingProvider::setStereoProfile(const int32_t sensorDev, const int32_t sensorNum)
{
    ENUM_STEREO_SENSOR_PROFILE profile = STEREO_SENSOR_PROFILE_UNKNOWN;

    if (sensorNum == 2) {
        profile = STEREO_SENSOR_PROFILE_REAR_FRONT;
    }
    else if(sensorDev == SENSOR_DEV_MAIN ||
            sensorDev == SENSOR_DEV_MAIN_2)
    {
        profile = STEREO_SENSOR_PROFILE_REAR_REAR;
    }
    else if(sensorNum > 3)
    {
        profile = STEREO_SENSOR_PROFILE_FRONT_FRONT;
    }

    MY_LOGD("Profile: %s, sensorDev: %d, sensorNum: %d", __getStereoProfileString(profile).c_str(), sensorDev, sensorNum);

    StereoSettingProvider::setStereoProfile(profile);
}

void
StereoSettingProvider::setStereoProfile(ENUM_STEREO_SENSOR_PROFILE profile)
{
    MY_LOGD("Set stereo profile: %s -> %s",
            __getStereoProfileString(m_stereoProfile).c_str(),
            __getStereoProfileString(profile).c_str());

    if(profile == m_stereoProfile) {
        return;
    }

    if(STEREO_SENSOR_PROFILE_UNKNOWN == profile) {
        // MY_LOGD("Reset stereo setting");
        // StereoSettingProviderKernel::destroyInstance();
#if (1==VSDOF_SUPPORTED)
        FEFMSettingProvider::destroyInstance();
#endif
    } else {
        //only init once when profile is set
        // if(STEREO_SENSOR_PROFILE_UNKNOWN == m_stereoProfile) {
            StereoSettingProviderKernel::getInstance()->init();
            StereoSettingProviderKernel::getInstance()->logSettings();
        // }

        __getSensorSettingsAndCombination(profile, g_sensorSettings[0], g_sensorSettings[1], g_currentSensorCombination);
        MY_LOGD("Sensor settings: %p %p, combination %p",
                g_sensorSettings[0], g_sensorSettings[1], g_currentSensorCombination);
    }

    m_stereoProfile = profile;
}

void
StereoSettingProvider::setImageRatio(ENUM_STEREO_RATIO ratio)
{
    if(STEREO_SENSOR_PROFILE_UNKNOWN == m_stereoProfile) {
        MY_LOGE("Must set stereo profile before set image ratio");
    }

    if(!IS_STEREO_MODE(m_stereoFeatureMode)) {
        MY_LOGE("Must set stereo feature mode before set image ratio, current feature mode: %s",
                _getStereoFeatureModeString(m_stereoFeatureMode).c_str());
    }

    char value[PROP_VALUE_MAX];
    size_t len = ::property_get("vendor.STEREO.ratio", value, NULL);
    if(len > 0) {
        int m, n;
        sscanf(value, "%d:%d", &m, &n);
        ratio = static_cast<ENUM_STEREO_RATIO>(m<<STEREO_RATIO_CONSEQUENT_BITS|n);
        MY_LOGD("Manually set image ratio to %d:%d (value %s)", m, n, value);
    }

    int m, n;
    imageRatioMToN(ratio, m, n);
    MY_LOGD("Set image ratio to %d:%d", m, n);

    StereoSettingProviderKernel::getInstance()->logSettings();

    m_imageRatio = ratio;
    _updateImageSettings();

    //Note: set image ratio will be called after setStereoFeatureMode
    // so we init tuning here
    if(IS_STEREO_MODE(m_stereoFeatureMode)) {
        StereoTuningProviderKernel::getInstance()->init();  //Do init and loading
    }
}

void
StereoSettingProvider::setStereoFeatureMode(MINT32 stereoMode, bool isPortrait)
{
    MY_LOGD("Set stereo feature mode: %s -> %s, portrait: %d",
            _getStereoFeatureModeString(m_stereoFeatureMode).c_str(),
            _getStereoFeatureModeString(stereoMode).c_str(), isPortrait);

    if((m_stereoFeatureMode & ~E_STEREO_FEATURE_PORTRAIT_FLAG) == stereoMode) {
        return;
    }

    m_stereoFeatureMode = stereoMode;
    if(!IS_STEREO_MODE(stereoMode)) {
        StereoTuningProviderKernel::destroyInstance();

        // MY_LOGD("Reset stereo setting");
        // StereoSettingProviderKernel::destroyInstance();
#if (1==VSDOF_SUPPORTED)
        FEFMSettingProvider::destroyInstance();
#endif
        return;
    }

    // Notice: We'll init FEFMSettingProvider in N3D HAL to sync FEFM setting

    //Update sensor scrnario
    if(isPortrait) {
        m_stereoFeatureMode |= E_STEREO_FEATURE_PORTRAIT_FLAG;
    }
    getSensorScenario(m_stereoFeatureMode, 0,  PipelineMode_ZSD, __sensorScenarioMain1, __sensorScenarioMain2);
    updateSensorScenario(__sensorScenarioMain1, __sensorScenarioMain2);

    //Update FOV Crip
    if(g_currentSensorCombination) {
        MY_LOGE_IF((NULL==g_sensorSettings[0] || NULL == g_sensorSettings[1]),
                   "Sensor setting not found: %p %p", g_sensorSettings[0], g_sensorSettings[1]);

        if(g_sensorSettings[0]) {
            g_sensorSettings[0]->updateRuntimeFOV(m_stereoFeatureMode);
            MY_LOGD("Main1 Runtime FOV: H/V: %.2f/%.2f(spec: %.2f/%.2f) Degrees",
                    g_sensorSettings[0]->fovHRuntime, g_sensorSettings[0]->fovVRuntime,
                    g_sensorSettings[0]->fovHorizontal, g_sensorSettings[0]->fovVertical);
        }
        if(g_sensorSettings[1]) {
            g_sensorSettings[1]->updateRuntimeFOV(m_stereoFeatureMode);
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
    std::string s;
    int featureCount = 0;
    if(stereoMode & E_STEREO_FEATURE_CAPTURE) {
        if(featureCount++ > 0) {
            s += "+cap";
        } else {
            s += "cap";
        }
    }
    if(stereoMode & E_STEREO_FEATURE_VSDOF) {
        if(featureCount++ > 0) {
            s += "+vsdof";
        } else {
            s += "vsdof";
        }
    }
    if(stereoMode & E_STEREO_FEATURE_DENOISE) {
        if(featureCount++ > 0) {
            s += "+denoise";
        } else {
            s += "denoise";
        }
    }
    if(stereoMode & E_DUALCAM_FEATURE_ZOOM) {
        if(featureCount++ > 0) {
            s += "+zoom";
        } else {
            s += "zoom";
        }
    }
    if(stereoMode & E_STEREO_FEATURE_THIRD_PARTY) {
        if(featureCount++ > 0) {
            s += "+3rdParty";
        } else {
            s += "3rdParty";
        }
    }
    if(stereoMode & E_STEREO_FEATURE_MTK_DEPTHMAP) {
        if(featureCount++ > 0) {
            s += "+mtkdepthmap";
        } else {
            s += "mtkdepthmap";
        }
    }
    if(stereoMode & E_STEREO_FEATURE_MULTI_CAM) {
        if(featureCount++ > 0) {
            s += "+multicam";
        } else {
            s += "multicam";
        }
    }

    if(0 == featureCount) {
        s = "none";
    }

    return s;
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
StereoSettingProvider::getStereoCameraFOV(SensorFOV &mainFOV, SensorFOV &main2FOV, ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorSetting_T *sensorSettings[2] = {g_sensorSettings[0], g_sensorSettings[1]};
    if(profile != m_stereoProfile) {
        __getSensorSettings(profile, sensorSettings[0], sensorSettings[1]);
    }

    bool result = true;
    if(NULL != sensorSettings[0] &&
       NULL != sensorSettings[1])
    {
        mainFOV.fov_horizontal  = sensorSettings[0]->fovHorizontal;
        mainFOV.fov_vertical    = sensorSettings[0]->fovVertical;
        main2FOV.fov_horizontal = sensorSettings[1]->fovHorizontal;
        main2FOV.fov_vertical   = sensorSettings[1]->fovVertical;
    } else if(!isTestMode()) {
        result = false;
        MY_LOGW("Cannot get sensor settings, no FOV information");
    }

    return result;
}

ENUM_ROTATION
StereoSettingProvider::getModuleRotation(ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;

    if(profile != m_stereoProfile) {
        sensorCombination = __getSensorCombinationSetting(profile);
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
    } else if(!isTestMode()) {
        MY_LOGW("Cannot get sensor combination setting, use %d as module rotation", rotate);
    }

    return rotate;
}

ENUM_STEREO_SENSOR_RELATIVE_POSITION
StereoSettingProvider::getSensorRelativePosition(ENUM_STEREO_SENSOR_PROFILE profile)
{
    int sensorRalation = 0;
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;

    if(profile != m_stereoProfile) {
        sensorCombination = __getSensorCombinationSetting(profile);
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
    } else if(!isTestMode()) {
        sensorRalation = 1;
        MY_LOGW("Cannot get sensor combination setting, use %d as relative position", sensorRalation);
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

bool
StereoSettingProvider::getStereoParams(STEREO_PARAMS_T &OutData)
{
    std::ostringstream stringStream;
    MSize szOutput = (NULL == g_currentSensorCombination) ? MSize(544, 288) : StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y_LARGE).size;
    stringStream << szOutput.w << "x" << szOutput.h;
    OutData.jpsSize     = stringStream.str();
    OutData.jpsSizesStr = stringStream.str();

    SIZE_LIST_T fullSizeList;
    if(g_sensorSettings[0]) {
        for(auto &v : g_sensorSettings[0]->captureSizes) {
            fullSizeList.insert(std::end(fullSizeList), std::begin(v.second), std::end(v.second));
        }
    } else {
        fullSizeList = {MSize(4160, 3120), MSize(4160, 2340), MSize(3120, 3120), MSize(4160, 1970)};
    }

    std::ostringstream oss;
    std::ostringstream ossSizes;
    if(0 == fullSizeList.size()) {
        MSize szCap = StereoSizeProvider::getInstance()->captureImageSize();
        MY_LOGW("No any stereo size found, use capture size(%dx%d), please check captureSizes in camera_custom_stereo.cpp", szCap.w, szCap.h);
        oss << szCap.w << "x" << szCap.h;
        ossSizes << szCap.w << "x" << szCap.h;
    } else {
        oss << fullSizeList[0].w << "x" << fullSizeList[0].h;

        SIZE_LIST_T::iterator it = fullSizeList.begin();
        for(; it != fullSizeList.end(); ++it) {
            ossSizes << it->w << "x" << it->h;
            if(it != fullSizeList.end() - 1) {
                ossSizes << ",";
            }
        }
    }

    OutData.refocusSize = oss.str();    //Only for reference, should be set from AP
    OutData.refocusSizesStr = ossSizes.str();

    // Post view sizes
    std::ostringstream postviewSizes;
    std::ostringstream depthmapSizes;
    if(g_sensorSettings[0]) {
        MSize postViewSize;
        MSize depthmapSize;
        int size = g_sensorSettings[0]->supportedImageRatios.size();
        ENUM_STEREO_RATIO ratio;
        int i = 0;
        for(auto &s : g_sensorSettings[0]->supportedImageRatios) {
            ratio = static_cast<ENUM_STEREO_RATIO>(s.first);
            postViewSize = StereoSizeProvider::getInstance()->postViewSize(ratio);
            postviewSizes << postViewSize.w << "x" << postViewSize.h;
            if(i < size-1) {
                postviewSizes << ",";
            }

            depthmapSize = StereoSizeProvider::getInstance()->thirdPartyDepthmapSize(StereoSettingProvider::stereoProfile(),
                                                                                     ratio);
            depthmapSizes << depthmapSize.w << "x" << depthmapSize.h;
            if(i < size-1) {
                depthmapSizes << ",";
            }

            ++i;
        }
    }
    OutData.postviewSizesStr = postviewSizes.str();
    OutData.depthmapSizeStr = depthmapSizes.str();
    MY_LOGD(" PostView Sizes: %s", OutData.postviewSizesStr.c_str());
    MY_LOGD(" Depthmap Sizes: %s", OutData.depthmapSizeStr.c_str());

    // N3D buffer size
    OutData.n3dSizes = StereoSettingProvider::getMaxN3DDebugBufferSizeInBytes();

    // Extra buffer size
    OutData.extraSizes = StereoSettingProvider::getExtraDataBufferSizeInBytes();

    return true;
}

float
StereoSettingProvider::getStereoBaseline(ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;
    if(profile != m_stereoProfile) {
        sensorCombination = __getSensorCombinationSetting(profile);
    }

    float baseline = 1.0f;
    if(sensorCombination) {
        baseline = sensorCombination->baseline;
    } else if(!isTestMode()) {
        MY_LOGW("Cannot get sensor combination setting, use %.2f as baseline", baseline);
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
    return (m_stereoFeatureMode & E_STEREO_FEATURE_DENOISE);
#else
    return false;
#endif
}

bool
StereoSettingProvider::isBayerPlusMono()
{
    int32_t main1Idx, main2Idx;
    getStereoSensorIndex(main1Idx, main2Idx, m_stereoProfile);
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
    MUINT32 supportedMode = (E_STEREO_FEATURE_CAPTURE|E_STEREO_FEATURE_THIRD_PARTY);
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

    return (m_stereoFeatureMode & supportedMode);
#endif

    return false;
}

void
StereoSettingProvider::_updateImageSettings()
{
#if (1 == STEREO_DENOISE_SUPPORTED)
    if( isDeNoise() ) {
        StereoSizeProvider::getInstance()->__updateBMDeNoiseSizes();
    }
#endif
    if(NULL != g_sensorSettings[0] &&
       NULL != g_sensorSettings[1])
    {
        MSize imgoSizes[2] = { MSIZE_ZERO, MSIZE_ZERO };
        MSize rrzoSizes[2] = { MSIZE_ZERO, MSIZE_ZERO };

        for(int i = 0; i < 2; ++i) {
            if(g_sensorSettings[i]->imgoYuvSize.find(m_imageRatio) != g_sensorSettings[i]->imgoYuvSize.end()) {
                imgoSizes[i] = g_sensorSettings[i]->imgoYuvSize[m_imageRatio];
            }

            if(g_sensorSettings[i]->rrzoYuvSize.find(m_imageRatio) != g_sensorSettings[i]->rrzoYuvSize.end()) {
                rrzoSizes[i] = g_sensorSettings[i]->rrzoYuvSize[m_imageRatio];
            }
        }

        StereoSizeProvider::getInstance()->__setIMGOYUVSize(imgoSizes[0], imgoSizes[1]);
        StereoSizeProvider::getInstance()->__setRRZOYUVSize(rrzoSizes[0], rrzoSizes[1]);
        StereoSizeProvider::getInstance()->__setPostviewSizes(g_sensorSettings[0]->postviewSize);
    }

    if(NULL != g_currentSensorCombination &&
       g_currentSensorCombination->hasSizeConfig)
    {
        ENUM_STEREO_RATIO ratio;
        for(auto &m1 : g_currentSensorCombination->baseSize) {
            ratio = static_cast<ENUM_STEREO_RATIO>(m1.first);
            StereoSizeProvider::getInstance()->__setCustomizedSize(ratio, m1.second);
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
    if(NULL == g_sensorSettings[0] ||
       NULL == g_sensorSettings[1])
    {
        MY_LOGE("Sensor settings not found %p %p", g_sensorSettings[0], g_sensorSettings[1]);
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
StereoSettingProvider::getStereoCameraFOVRatio(ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(isDeNoise()) {
        return 1.0f;
    }

    float fovRatio = 1.0f;
    StereoSensorSetting_T *sensorSettings[2] = {g_sensorSettings[0], g_sensorSettings[1]};
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;
    if(profile != m_stereoProfile) {
        __getSensorSettingsAndCombination(profile, sensorSettings[0], sensorSettings[1], sensorCombination);
    }

    if(NULL != sensorCombination &&
       sensorCombination->disableCrop)
    {
        return 1.0f;
    }

    if(NULL == sensorSettings[0] ||
       NULL == sensorSettings[1] ||
       NULL == sensorCombination)
    {
        MY_LOGE("Sensor settings of profile %s not found: %p %p, combination: %p",
                __getStereoProfileString(profile).c_str(),
                sensorSettings[0], sensorSettings[1], sensorCombination);
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

    if(NULL == g_sensorSettings[0] ||
       NULL == g_sensorSettings[1])
    {
        MY_LOGE("Sensor settings not found %p %p", g_sensorSettings[0], g_sensorSettings[1]);
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

    if(NULL != g_sensorSettings[0]) {
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

    if(NULL != g_sensorSettings[1]) {
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
    m_stereoModuleType = moduleType;
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
    m_stereoShotMode = stereoShotMode;
}

size_t
StereoSettingProvider::getMaxN3DDebugBufferSizeInBytes()
{
    return 1024*1024*3;
}

bool
StereoSettingProvider::isWidePlusTele(ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorSetting_T *sensorSettings[2] = {NULL, NULL};
    __getSensorSettings(profile, sensorSettings[0], sensorSettings[1]);

    bool result = false;
    if(NULL != sensorSettings[0]) {
        //W+T will exchange sensor index
        if(sensorSettings[0]->index != SENSOR_DEV_MAIN &&
           sensorSettings[0]->index != SENSOR_DEV_SUB)
        {
            return true;
        }
    } else if(!isTestMode()) {
        MY_LOGW("Cannot get sensor settings, set to not W+T by default");
    }

    return result;
}

bool
StereoSettingProvider::isWideTeleVSDoF(ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(isWidePlusTele(profile) &&
       IS_STEREO_MODE(m_stereoFeatureMode))
    {
        return true;
    }

    return false;
}

bool
StereoSettingProvider::getSensorScenario(MINT32 stereoMode,
                                         MINT32 sensorModuleType __attribute__((unused)),
                                         MINT32 pipelineMode,
                                         MUINT &sensorScenarioMain1,
                                         MUINT &sensorScenarioMain2)
{
    bool result[2] = {true, true};
    if(g_sensorSettings[0]) {
        sensorScenarioMain1 = g_sensorSettings[0]->getSensorScenario(stereoMode, pipelineMode);
        if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenarioMain1) {
            result[0] = false;
        }
    } else {
        result[0] = false;
    }

    if(!result[0]) {
        sensorScenarioMain1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        MY_LOGW("Cannot get sensor setting of main1, use %s as sensor scenario", SENSOR_SCENARIO_TO_STRING_MAP.at(sensorScenarioMain1));
    }

    if(g_sensorSettings[1]) {
        sensorScenarioMain2 = g_sensorSettings[1]->getSensorScenario(stereoMode, pipelineMode);
        if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenarioMain2) {
            result[1] = false;
        }
    } else {
        result[1] = false;
    }

    if(!result[1]) {
        sensorScenarioMain2 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;

        //If is tk flow and sensor size < 8M, use capture sensor scenario
        if(g_sensorSettings[1] &&
           g_sensorSettings[1]->staticInfo.captureWidth * g_sensorSettings[1]->staticInfo.captureHeight < 8000000)
        {
            sensorScenarioMain2 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }

        MY_LOGW("Cannot get sensor setting of main1, use %s as sensor scenario", SENSOR_SCENARIO_TO_STRING_MAP.at(sensorScenarioMain2));
    }

    return (result[0] && result[1]);
}

bool
StereoSettingProvider::getMain1SensorScenario(MINT32 stereoMode,
                                              MINT32 pipelineMode,
                                              MUINT &sensorScenarioMain1)
{
    bool result = true;
    if(g_sensorSettings[0]) {
        sensorScenarioMain1 = g_sensorSettings[0]->getSensorScenario(stereoMode, pipelineMode);
        if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenarioMain1) {
            result = false;
        }
    } else {
        result = false;
    }

    if(!result) {
        sensorScenarioMain1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        MY_LOGW("Cannot get sensor setting of main1, use %s as sensor scenario", SENSOR_SCENARIO_TO_STRING_MAP.at(sensorScenarioMain1));
    }

    return result;
}

bool
StereoSettingProvider::getMain2SensorScenario(MINT32 stereoMode,
                                              MINT32 pipelineMode,
                                              MUINT &sensorScenarioMain2)
{
    bool result = true;
    if(g_sensorSettings[1]) {
        sensorScenarioMain2 = g_sensorSettings[1]->getSensorScenario(stereoMode, pipelineMode);
        if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenarioMain2) {
            result = false;
        }
    } else {
        result = false;
    }

    if(!result) {
        sensorScenarioMain2 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        MY_LOGW("Cannot get sensor setting of main2, use %s as sensor scenario", SENSOR_SCENARIO_TO_STRING_MAP.at(sensorScenarioMain2));
    }

    return result;
}

void
StereoSettingProvider::updateSensorScenario(MUINT sensorScenarioMain1,
                                            MUINT sensorScenarioMain2)
{
    __sensorScenarioMain1 = sensorScenarioMain1;
    __sensorScenarioMain2 = sensorScenarioMain2;
    MY_LOGD("Update sensor scenario: %d(%s) %d(%s)",
            __sensorScenarioMain1, SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarioMain1),
            __sensorScenarioMain2, SENSOR_SCENARIO_TO_STRING_MAP.at(__sensorScenarioMain2));
}

void
StereoSettingProvider::updateMain1SensorScenario(MUINT sensorScenarioMain1)
{
    __sensorScenarioMain1 = sensorScenarioMain1;
    MY_LOGD("Update main1 sensor scenario: %d", __sensorScenarioMain1);
}

void
StereoSettingProvider::updateMain2SensorScenario(MUINT sensorScenarioMain2)
{
    __sensorScenarioMain2 = sensorScenarioMain2;
    MY_LOGD("Update main2 sensor scenario: %d", __sensorScenarioMain2);
}

bool
StereoSettingProvider::getCalibrationDistance(ENUM_STEREO_SENSOR sensor, MUINT32 &macroDistance, MUINT32 &infiniteDistance)
{
    bool result = true;
    if(eSTEREO_SENSOR_MAIN1 == sensor) {
        if(g_sensorSettings[0]) {
            macroDistance    = g_sensorSettings[0]->distanceMacro;
            infiniteDistance = g_sensorSettings[0]->distanceInfinite;
        } else {
            macroDistance    = 100;
            infiniteDistance = 5000;
            result = false;
            MY_LOGW("Cannot get sensor setting of main1, use %d & %d for calibration distance", macroDistance, infiniteDistance);
        }
    } else {
        if(g_sensorSettings[1]) {
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
        const char *DEFAULT_LIST = "ci,bi,mbd,mdb,mbm";
        MY_LOGD("Callback list are not set, use default(%s)", DEFAULT_LIST);
        result.assign(DEFAULT_LIST);
    }

    MY_LOGD_IF(isLogEnabled(), "Callback list: %s", result.c_str());

    return result;
}

void
StereoSettingProvider::setPreviewSize(NSCam::MSize pvSize)
{
    MY_LOGD("Set preview size %dx%d", pvSize.w, pvSize.h);

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

    StereoSizeProvider::getInstance()->setPreviewSize(pvSize);
}

size_t
StereoSettingProvider::getCalibrationData(void *calibrationData, ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(NULL == calibrationData) {
        MY_LOGE("Invalid input calibration data address");
        return 0;
    }

    StereoSensorConbinationSetting_T *pSC = __getSensorCombinationSetting(profile);
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