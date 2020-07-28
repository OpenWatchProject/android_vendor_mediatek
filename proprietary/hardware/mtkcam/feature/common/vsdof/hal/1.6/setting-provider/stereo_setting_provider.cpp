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
#include "../inc/stereo_dp_util.h"
#include <mtkcam/aaa/IHal3A.h>

#include "stereo_setting_provider_kernel.h"
#if (1==VSDOF_SUPPORTED)
#include <swfefm_setting_provider.h>
#endif

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
                              (mode & E_DUALCAM_FEATURE_ZOOM) || \
                              (mode & E_STEREO_FEATURE_DENOISE) || \
                              (mode & E_STEREO_FEATURE_THIRD_PARTY) || \
                              (mode & E_STEREO_FEATURE_CAPTURE))

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

StereoSensorSetting_T *g_sensorSettings[2] = {NULL};
StereoSensorConbinationSetting_T *g_currentSensorCombination = NULL;

const bool GET_AF_INFO = true;

bool __getSensorSettingsAndCombination(int main1Index,
                                       int main2Index,
                                       StereoSensorSetting_T *&main1Setting,
                                       StereoSensorSetting_T *&main2Setting,
                                       StereoSensorConbinationSetting_T *&sensorCombination)
{
    bool result = true;

    main1Setting = StereoSettingProviderKernel::getInstance()->getSensorSetting(main1Index);
    main2Setting = StereoSettingProviderKernel::getInstance()->getSensorSetting(main2Index);
    if(NULL != main1Setting &&
       NULL != main2Setting)
    {
        sensorCombination = StereoSettingProviderKernel::getInstance()->
                            getSensorCombinationSetting(main1Setting->uid, main2Setting->uid);
    } else {
        MY_LOGW("Cannot get sensor settings");
        result = false;
    }

    return result;
}

bool
StereoSettingProvider::getStereoSensorIndex(int32_t &main1Idx, int32_t &main2Idx, ENUM_STEREO_SENSOR_PROFILE profile)
{
    bool result = _getOriginalSensorIndex(main1Idx, main2Idx, profile);
    if(isWideTeleVSDoF())
    {
        int32_t tmp = main1Idx;
        main1Idx = main2Idx;
        main2Idx = tmp;
        // MY_LOGD_IF(isLogEnabled(), "Get sensor id for W+T VSDoF: %d, %d", main1Idx, main2Idx);
    } else {
        // MY_LOGD_IF(isLogEnabled(), "Get id for Feature %s: %d, %d", _getStereoFeatureModeString(m_stereoFeatureMode).c_str(), main1Idx, main2Idx);
    }

    return result;
}

bool
StereoSettingProvider::_getOriginalSensorIndex(int32_t &main1Idx, int32_t &main2Idx, ENUM_STEREO_SENSOR_PROFILE profile)
{
    const int SENSOR_COUNT = (!m_isTestMode) ? MAKE_HalSensorList()->queryNumberOfSensors() : 3;
    if(2 == SENSOR_COUNT) {
        // MY_LOGW("Only two sensors were found on the device");
        main1Idx = 0;
        main2Idx = 1;
        return true;
    }

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

    MY_LOGD("Profile: %d, sensorDev: %d, sensorNum: %d", profile, sensorDev, sensorNum);

    StereoSettingProvider::setStereoProfile(profile);
}

void
StereoSettingProvider::setStereoProfile(ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(profile == m_stereoProfile) {
        return;
    }

    m_stereoProfile = profile;

    if(STEREO_SENSOR_PROFILE_UNKNOWN == profile) {
        MY_LOGW("Set unknown stereo profile(normal for single cam)");
        // MY_LOGD("Reset stereo setting");
        // StereoSettingProviderKernel::destroyInstance();
#if (1==VSDOF_SUPPORTED)
        SWFEFMSettingProvider::destroyInstance();
#endif
        // g_sensorSettings[0]        = NULL;
        // g_sensorSettings[1]        = NULL;
        // g_currentSensorCombination = NULL;
        return;
    } else {
        MY_LOGD("Set stereo profile: %d", profile);
    }

    StereoSettingProviderKernel::getInstance()->init();

    int main1Idx, main2Idx;
    _getOriginalSensorIndex(main1Idx, main2Idx, profile);
    __getSensorSettingsAndCombination(main1Idx, main2Idx, g_sensorSettings[0], g_sensorSettings[1], g_currentSensorCombination);
    MY_LOGD("Sensor settings: %p %p, combination %p",
                g_sensorSettings[0], g_sensorSettings[1], g_currentSensorCombination);

    _updateImageSettings();
}

void
StereoSettingProvider::setImageRatio(ENUM_STEREO_RATIO ratio)
{
    if(isLogEnabled()) {
        switch(ratio) {
        case eRatio_16_9:
            MY_LOGD("Set image ratio to 16:9");
            break;
        case eRatio_4_3:
            MY_LOGD("Set image ratio to 4:3");
            break;
        default:
            MY_LOGD("Set image ratio to %d", ratio);
            break;
        }
    }

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
    MY_LOGD_IF(isLogEnabled(), "SetStereoFeatureMode: %s", _getStereoFeatureModeString(stereoMode).c_str());
    if(m_stereoFeatureMode == stereoMode) {
        return;
    }

    m_stereoFeatureMode = stereoMode;

    if(!IS_STEREO_MODE(stereoMode)) {
        StereoTuningProviderKernel::destroyInstance();

        MY_LOGW("Set non stereo feature(normal for single cam)");
        // MY_LOGD("Reset stereo setting");
        // StereoSettingProviderKernel::destroyInstance();
#if (1==VSDOF_SUPPORTED)
        SWFEFMSettingProvider::destroyInstance();
#endif
        // g_sensorSettings[0]        = NULL;
        // g_sensorSettings[1]        = NULL;
        // g_currentSensorCombination = NULL;
        return;
    }

    if(STEREO_SENSOR_PROFILE_UNKNOWN == m_stereoProfile)
    {
        MY_LOGW("Stereo profile is not set yet, use REAR+REAR as default");
        setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);
    }

    int main1Idx, main2Idx;
    getStereoSensorIndex(main1Idx, main2Idx, stereoProfile());
    g_sensorSettings[0] = StereoSettingProviderKernel::getInstance()->getSensorSetting(main1Idx);
    g_sensorSettings[1] = StereoSettingProviderKernel::getInstance()->getSensorSetting(main2Idx);
    MY_LOGD("Sensor settings: %p %p", g_sensorSettings[0], g_sensorSettings[1]);

#if (1==VSDOF_SUPPORTED)
    //Init here instead of setStereoProfile since feature mode may change index
    SWFEFMSettingProvider::getInstance()->init(main1Idx, main2Idx);
#endif

    _updateImageSettings();
}

std::string
StereoSettingProvider::_getStereoFeatureModeString(int stereoMode)
{
    std::string s;
    int featureCount = 0;
    if((stereoMode & E_STEREO_FEATURE_CAPTURE) > 0) {
        if(featureCount++ > 0) {
            s += "+cap";
        } else {
            s += "cap";
        }
    }
    if((stereoMode & E_STEREO_FEATURE_VSDOF) > 0) {
        if(featureCount++ > 0) {
            s += "+vsdof";
        } else {
            s += "vsdof";
        }
    }
    if((stereoMode & E_STEREO_FEATURE_DENOISE) > 0) {
        if(featureCount++ > 0) {
            s += "+denoise";
        } else {
            s += "denoise";
        }
    }
    if((stereoMode & E_DUALCAM_FEATURE_ZOOM) > 0) {
        if(featureCount++ > 0) {
            s += "+zoom";
        } else {
            s += "zoom";
        }
    }
    if((stereoMode & E_STEREO_FEATURE_THIRD_PARTY) > 0) {
        if(featureCount++ > 0) {
            s += "+3rdParty";
        } else {
            s += "3rdParty";
        }
    }

    if(0 == featureCount) {
        s = "unset";
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
        StereoSensorConbinationSetting_T *sensorCombination = NULL;
        int main1Idx, main2Idx;
        getStereoSensorIndex(main1Idx, main2Idx, profile);
        __getSensorSettingsAndCombination(main1Idx, main2Idx, sensorSettings[0], sensorSettings[1], sensorCombination);
    }
    mainFOV.fov_horizontal  = sensorSettings[0]->fovHorizontal;
    mainFOV.fov_vertical    = sensorSettings[0]->fovVertical;
    main2FOV.fov_horizontal = sensorSettings[1]->fovHorizontal;
    main2FOV.fov_vertical   = sensorSettings[1]->fovVertical;

    return true;
}

float
StereoSettingProvider::getStereoCameraFOVRatio(ENUM_STEREO_SENSOR_PROFILE profile)
{
    static ENUM_STEREO_SENSOR_PROFILE lastProfile = STEREO_SENSOR_PROFILE_UNKNOWN;
    static MINT32 lastFeatureMode                 = m_stereoFeatureMode;
    static float fovRatio                         = 1.0f;

    if(lastProfile == profile &&
       lastFeatureMode == m_stereoFeatureMode)
    {
        return fovRatio;
    }

    fovRatio        = 1.0f;
    lastProfile     = profile;
    lastFeatureMode = m_stereoFeatureMode;
    if( !isDeNoise() )
    {
        SensorFOV main1FOV, main2FOV;
        getStereoCameraFOV(main1FOV, main2FOV, profile);
        StereoSensorConbinationSetting_T *sensorCombination = NULL;
        StereoSensorSetting_T *sensorSettings[2] = {NULL};
        int main1Idx, main2Idx;
        getStereoSensorIndex(main1Idx, main2Idx, profile);
        __getSensorSettingsAndCombination(main1Idx, main2Idx, sensorSettings[0], sensorSettings[1], sensorCombination);
        CUST_FOV_CROP_T cropSetting = sensorCombination->fovCrop;

        float fov1, fov2;
        float cropDegree = cropSetting.FOV_CROP_DEGREE;
        if(  0 == getModuleRotation() ||
           180 == getModuleRotation() )
        {
            fov1 = main1FOV.fov_horizontal - cropDegree;
        }
        else
        {
            //Calculate long edge cropping degree
            fov1 = main1FOV.fov_vertical;
            fov2 = main1FOV.fov_vertical - cropDegree;
            fovRatio = tan(fov1/2.0f * M_PI/180.0f) /
                       tan(fov2/2.0f * M_PI/180.0f);
            fov1 = 2.0f*atan(tan(main1FOV.fov_horizontal/2.0f * M_PI/180.0f)/fovRatio)*180.0f/M_PI;
        }

        fov2 = main2FOV.fov_horizontal;

        fovRatio = tan(fov2/2.0f * M_PI/180.0f) /
                   tan(fov1/2.0f * M_PI/180.0f);
    }

    return fovRatio;
}

ENUM_ROTATION
StereoSettingProvider::getModuleRotation(ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;

    if(profile != m_stereoProfile) {
        StereoSensorSetting_T *sensorSettings[2] = {NULL};
        int main1Idx, main2Idx;
        _getOriginalSensorIndex(main1Idx, main2Idx, profile);
        __getSensorSettingsAndCombination(main1Idx, main2Idx, sensorSettings[0], sensorSettings[1], sensorCombination);
    }

    if(1 == sensorCombination->moduleType ||
       3 == sensorCombination->moduleType)
    {
        return eRotate_90;
    }

    return eRotate_0;
}

ENUM_STEREO_SENSOR_RELATIVE_POSITION
StereoSettingProvider::getSensorRelativePosition(ENUM_STEREO_SENSOR_PROFILE profile)
{
    int sensorRalation = 0;
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;

    if(profile != m_stereoProfile) {
        StereoSensorSetting_T *sensorSettings[2] = {NULL};
        int main1Idx, main2Idx;
        _getOriginalSensorIndex(main1Idx, main2Idx, profile);
        __getSensorSettingsAndCombination(main1Idx, main2Idx, sensorSettings[0], sensorSettings[1], sensorCombination);
    }

    if(STEREO_SENSOR_PROFILE_FRONT_FRONT == profile) {
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

    if(isWideTeleVSDoF()) {
        sensorRalation = 1 - sensorRalation;
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
        fullSizeList.insert(std::end(fullSizeList), std::begin(g_sensorSettings[0]->captureSizes_16_9), std::end(g_sensorSettings[0]->captureSizes_16_9));
        fullSizeList.insert(std::end(fullSizeList), std::begin(g_sensorSettings[0]->captureSizes_4_3), std::end(g_sensorSettings[0]->captureSizes_4_3));
    } else {
        fullSizeList = {MSize(4160, 3120), MSize(4160, 2340)};
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
    MSize postViewSize = StereoSizeProvider::getInstance()->postViewSize(eRatio_16_9);
    postviewSizes << postViewSize.w << "x" << postViewSize.h << ",";
    postViewSize = StereoSizeProvider::getInstance()->postViewSize(eRatio_4_3);
    postviewSizes << postViewSize.w << "x" << postViewSize.h;
    OutData.postviewSizesStr = postviewSizes.str();
    MY_LOGD(" Sizes (%s)", ossSizes.str().c_str());

    return true;
}

float
StereoSettingProvider::getStereoBaseline(ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorConbinationSetting_T *sensorCombination = g_currentSensorCombination;
    if(profile != m_stereoProfile) {
        StereoSensorSetting_T *sensorSettings[2] = {NULL};
        int main1Idx, main2Idx;
        _getOriginalSensorIndex(main1Idx, main2Idx, profile);
        __getSensorSettingsAndCombination(main1Idx, main2Idx, sensorSettings[0], sensorSettings[1], sensorCombination);
    }

    return sensorCombination->baseline;
}

MUINT
StereoSettingProvider::getSensorRawFormat(const int SENSOR_INDEX)
{
    // for dev
    char cForceMain2Type[PROPERTY_VALUE_MAX];
    ::property_get("vendor.STEREO.debug.main2Mono", cForceMain2Type, "0");
    if( (atoi(cForceMain2Type) == 1) && (SENSOR_INDEX == 2)){
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
#if (1 == MTK_CAM_STEREO_CAMERA_SUPPORT)
    MUINT32 supportedMode = 0;
    #if (1 == STEREO_DENOISE_SUPPORTED)
        supportedMode |= E_STEREO_FEATURE_DENOISE;
    #endif
    #if (1 == VSDOF_SUPPORTED)
        supportedMode |= E_STEREO_FEATURE_VSDOF;
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
        switch(m_imageRatio) {
        case eRatio_16_9:
        default:
            StereoSizeProvider::getInstance()->__setIMGOYUVSize(g_sensorSettings[0]->imgoYuvSize_16_9, g_sensorSettings[1]->imgoYuvSize_16_9);
            StereoSizeProvider::getInstance()->__setRRZOYUVSize(g_sensorSettings[0]->rrzoYuvSize_16_9, g_sensorSettings[1]->rrzoYuvSize_16_9);
            break;
        case eRatio_4_3:
            StereoSizeProvider::getInstance()->__setIMGOYUVSize(g_sensorSettings[0]->imgoYuvSize_4_3, g_sensorSettings[1]->imgoYuvSize_4_3);
            StereoSizeProvider::getInstance()->__setRRZOYUVSize(g_sensorSettings[0]->rrzoYuvSize_4_3, g_sensorSettings[1]->rrzoYuvSize_4_3);
            break;
        }
    }
}

vector<float> &
StereoSettingProvider::getLDCTable()
{
    return g_currentSensorCombination->LDC;
}

bool
StereoSettingProvider::LDCEnabled()
{
    return g_currentSensorCombination->enableLDC;
}

CUST_FOV_CROP_T
StereoSettingProvider::getFOVCropSetting()
{
    return g_currentSensorCombination->fovCrop;
}

float
StereoSettingProvider::getFOVCropRatio()
{
    if( StereoSettingProvider::isDeNoise() ||
        isWideTeleVSDoF() )
    {
        return 1.0f;
    }

    float ratio = 1.0f;
    SensorFOV main1FOV, main2FOV;
    StereoSettingProvider::getStereoCameraFOV(main1FOV, main2FOV);
    CUST_FOV_CROP_T cropSetting = getFOVCropSetting();

    if(cropSetting.FOV_CROP_DEGREE > 0.001f) {
        float fov1, fov2;
        if(0   == getModuleRotation() ||
           180 == getModuleRotation() )
        {
            fov1 = main1FOV.fov_horizontal;
            fov2 = main1FOV.fov_horizontal - cropSetting.FOV_CROP_DEGREE;
        } else {
            fov1 = main1FOV.fov_vertical;
            fov2 = main1FOV.fov_vertical - cropSetting.FOV_CROP_DEGREE;
        }

        ratio = tan(fov2/2.0f * M_PI/180.0f) /
                tan(fov1/2.0f * M_PI/180.0f);
    }

    return ratio;
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
    MSize bufferSize = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, eSTEREO_SCENARIO_CAPTURE);
    MSize ldcSize = StereoSizeProvider::getInstance()->getBufferSize(E_LDC, eSTEREO_SCENARIO_CAPTURE);
    size_t header_size = 500;
    return (bufferSize.w * bufferSize.h * 4 + ldcSize.w*ldcSize.h +header_size);   //all Y-only
    // return (bufferSize.w * bufferSize.h * 4.5f + ldcSize.w*ldcSize.h + header_size);   //Main1 is YV12
}

bool
StereoSettingProvider::isWidePlusTele(ENUM_STEREO_SENSOR_PROFILE profile)
{
    StereoSensorConbinationSetting_T *sensorCombination = NULL;
    StereoSensorSetting_T *sensorSettings[2] = {NULL};
    int main1Idx, main2Idx;
    _getOriginalSensorIndex(main1Idx, main2Idx, profile);
    __getSensorSettingsAndCombination(main1Idx, main2Idx, sensorSettings[0], sensorSettings[1], sensorCombination);
    return (fabs(sensorSettings[0]->fovHorizontal - sensorSettings[1]->fovHorizontal) > 20.0f);
}

bool
StereoSettingProvider::isWideTeleVSDoF(ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(isWidePlusTele(profile) &&
       m_stereoFeatureMode > 0 &&
       (m_stereoFeatureMode & E_STEREO_FEATURE_VSDOF) > 0)
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
    sensorScenarioMain1 = g_sensorSettings[0]->getSensorScenario(stereoMode, pipelineMode);
    sensorScenarioMain2 = g_sensorSettings[1]->getSensorScenario(stereoMode, pipelineMode);
    return true;
}

void
StereoSettingProvider::updateSensorScenario(MUINT sensorScenarioMain1,
                                            MUINT sensorScenarioMain2)
{
    __sensorScenarioMain1 = sensorScenarioMain1;
    __sensorScenarioMain2 = sensorScenarioMain2;
    MY_LOGD("Update sensor scenario: %d %d",
            __sensorScenarioMain1, __sensorScenarioMain2);
}

bool
StereoSettingProvider::getCalibrationDistance(ENUM_STEREO_SENSOR sensor, MUINT32 &macroDistance, MUINT32 &infiniteDistance)
{
    if(eSTEREO_SENSOR_MAIN1 == sensor) {
        macroDistance    = g_sensorSettings[0]->distanceMacro;
        infiniteDistance = g_sensorSettings[0]->distanceInfinite;
    } else {
        macroDistance    = g_sensorSettings[1]->distanceMacro;
        infiniteDistance = g_sensorSettings[1]->distanceInfinite;
    }

    return true;
}
