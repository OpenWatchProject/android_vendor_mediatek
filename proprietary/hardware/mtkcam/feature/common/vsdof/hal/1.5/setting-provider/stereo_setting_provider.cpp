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
                              (mode & E_STEREO_FEATURE_CAPTURE))

using namespace NSCam;
using namespace android;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;
using namespace NS3Av3;

ENUM_STEREO_SENSOR_PROFILE StereoSettingProvider::m_stereoProfile = STEREO_SENSOR_PROFILE_REAR_REAR;
ENUM_STEREO_RATIO StereoSettingProvider::m_imageRatio = eRatio_Default;
MINT32 StereoSettingProvider::m_stereoFeatureMode = 0;
MINT32 StereoSettingProvider::m_stereoModuleType = -1;
MUINT32 StereoSettingProvider::m_stereoShotMode = 0;
bool StereoSettingProvider::m_isTestMode = false;
MUINT StereoSettingProvider::__sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
MUINT StereoSettingProvider::__sensorScenarioMain2 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;

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

    return getStereoSensorID(profile, main1Idx, main2Idx);
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
StereoSettingProvider::setStereoProfile(ENUM_STEREO_SENSOR_PROFILE profile)
{
    m_stereoProfile = profile;
    _updateImageSettings();
}

void
StereoSettingProvider::setStereoProfile(const int32_t sensorDev, const int32_t sensorNum)
{
    ENUM_STEREO_SENSOR_PROFILE profile = STEREO_SENSOR_PROFILE_UNKNOWN;
    profile = (sensorDev == SENSOR_DEV_MAIN || sensorDev == SENSOR_DEV_MAIN_2) ? STEREO_SENSOR_PROFILE_REAR_REAR : STEREO_SENSOR_PROFILE_FRONT_FRONT;
    if (sensorNum == 2)
        profile = STEREO_SENSOR_PROFILE_REAR_FRONT;
    MY_LOGD(" Profile: %d, IsRear: %d, sensorDev: %d, sensorNum: %d, SENSOR_DEV_MAIN: %d, SENSOR_DEV_MAIN_2: %d",
        profile, (sensorDev == SENSOR_DEV_MAIN || sensorDev == SENSOR_DEV_MAIN_2),
        sensorDev, sensorNum, SENSOR_DEV_MAIN, SENSOR_DEV_MAIN_2);
    StereoSettingProvider::setStereoProfile(profile);
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

    if(!IS_STEREO_MODE(stereoMode)) {
        StereoTuningProviderKernel::destroyInstance();
    }

    m_stereoFeatureMode = stereoMode;
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
    static bool _hasHWFE = true;
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
    int main1Idx, main2Idx;
    getStereoSensorIndex(main1Idx, main2Idx, profile);
    mainFOV  = FOV_LIST[main1Idx];
    main2FOV = FOV_LIST[main2Idx];
    // MY_LOGD_IF(isLogEnabled(), "FOV: %.1f,%.1f / %.1f,%.1f",
    //                             mainFOV.fov_horizontal, mainFOV.fov_vertical,
    //                             main2FOV.fov_horizontal, main2FOV.fov_vertical);
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
        CUST_FOV_CROP_T cropSetting = getFOVCropSetting(profile);

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
    int main1Index, main2Index;
    StereoSettingProvider::getStereoSensorIndex(main1Index, main2Index, profile);

    return MODULE_ROTATION[main1Index];
}

ENUM_STEREO_SENSOR_RELATIVE_POSITION
StereoSettingProvider::getSensorRelativePosition(ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(isWideTeleVSDoF()) {
        return static_cast<ENUM_STEREO_SENSOR_RELATIVE_POSITION>(1-getSensorRelation(profile));
    }

    return getSensorRelation(profile);
}

bool
StereoSettingProvider::isSensorAF(const int SENSOR_INDEX)
{
    bool isAF = false;
    if(!m_isTestMode) {
        IHal3A *pHal3A = MAKE_Hal3A(SENSOR_INDEX, LOG_TAG);
        if(NULL == pHal3A) {
            MY_LOGE("Cannot get 3A HAL of sensor %d", SENSOR_INDEX);
            return false;
        } else {
            FeatureParam_T rFeatureParam;
            if(pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, (MUINTPTR)&rFeatureParam, 0)) {
                isAF = (rFeatureParam.u4MaxFocusAreaNum > 0);
            } else {
                isAF = SENSOR_AF[SENSOR_INDEX];
                MY_LOGW("Cannot query AF ability from 3A, use default setting from custom: %d", isAF);
            }
        }
    } else {
        isAF = SENSOR_AF[SENSOR_INDEX];
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
    ENUM_STEREO_SENSOR_PROFILE profile = StereoSettingProvider::stereoProfile();
    std::ostringstream stringStream;
    MSize szOutput = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y_LARGE);
    stringStream << szOutput.w << "x" << szOutput.h;
    OutData.jpsSize     = stringStream.str();
    OutData.jpsSizesStr = stringStream.str();

    STEREO_CAPTURE_SIZE_T captureSizes = CAPTURE_SIZES;
    auto profileIdx = PROFILE_CAPTURE_SIZES.find(profile);
    if (profileIdx != PROFILE_CAPTURE_SIZES.end()) {
        captureSizes = profileIdx->second;
        MY_LOGD(" Current profile (%d)", profile);
    }
    // Specifiec profile not found, used CAPTURE_SIZES as default value
    else {
        MY_LOGW("Stereo capture size for profile %d not found, use CAPTURE_SIZES as default value", profile);
    }

    STEREO_CAPTURE_SIZE_T::const_iterator it_16_9 = captureSizes.find(eRatio_16_9);
    STEREO_CAPTURE_SIZE_T::const_iterator it_4_3 = captureSizes.find(eRatio_4_3);
    SIZE_LIST_T fullSizeList;

    if(it_16_9 != captureSizes.end()) {
        fullSizeList.insert( fullSizeList.end(), it_16_9->second.begin(), it_16_9->second.end() );
    }

    if(it_4_3 != captureSizes.end()) {
        fullSizeList.insert( fullSizeList.end(), it_4_3->second.begin(), it_4_3->second.end() );
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
    MY_LOGD("Refocus Sizes %s, PostView Sizes %s", OutData.refocusSizesStr.c_str(), OutData.postviewSizesStr.c_str());

    return true;
}

float
StereoSettingProvider::getStereoBaseline(ENUM_STEREO_SENSOR_PROFILE profile)
{
    return ::getStereoBaseline(profile);
}

MUINT
StereoSettingProvider::getSensorRawFormat(const int SENSOR_INDEX)
{
    MUINT sensorRawFormat = SENSOR_RAW_Bayer;
    if(!m_isTestMode) {
        IHalSensorList *sensorList = MAKE_HalSensorList();
        if (NULL == sensorList) {
            MY_LOGE("Cannot get sensor list");
            return SENSOR_RAW_FMT_NONE;
        }

        int32_t sensorCount = sensorList->queryNumberOfSensors();
        if(SENSOR_INDEX >= sensorCount) {
            MY_LOGW("Sensor index should be <= %d", sensorCount-1);
            return SENSOR_RAW_FMT_NONE;
        }

        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        int sendorDevIndex = sensorList->querySensorDevIdx(SENSOR_INDEX);
        sensorList->querySensorStaticInfo(sendorDevIndex, &sensorStaticInfo);

        // for dev
        char cForceMain2Type[PROPERTY_VALUE_MAX];
        ::property_get("vendor.STEREO.debug.main2Mono", cForceMain2Type, "0");
        if( (atoi(cForceMain2Type) == 1) && (SENSOR_INDEX == 2)){
            MY_LOGD("force main2 to be MONO sensor");
            sensorStaticInfo.rawFmtType = SENSOR_RAW_MONO;
        }

        sensorRawFormat = sensorStaticInfo.rawFmtType;
    } else {
        int main1Idx, main2Idx;
        getStereoSensorIndex(main1Idx, main2Idx);
        if(FOV_LIST[main1Idx].fov_horizontal == FOV_LIST[main2Idx].fov_horizontal) {
            if(SENSOR_INDEX == main2Idx) {
                sensorRawFormat = SENSOR_RAW_MONO;
            }
        }
    }

    return sensorRawFormat;
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
    CUST_FOV_CROP_T cropSetting = getFOVCropSetting(stereoProfile());

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
    int32_t main1Idx, main2Idx;
    _getOriginalSensorIndex(main1Idx, main2Idx, profile);
    return (fabs(FOV_LIST[main1Idx].fov_horizontal - FOV_LIST[main2Idx].fov_horizontal) > 20.0f);
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
                                         MINT32 sensorModuleType,
                                         MINT32 pipelineMode,
                                         MUINT &sensorScenarioMain1,
                                         MUINT &sensorScenarioMain2)
{
    if(NSCam::v1::Stereo::STEREO_3RDPARTY == (stereoMode) &&
       pipelineMode == PipelineMode_ZSD)
    {
        // MY_LOGD("STEREO_3RDPARTY: get 3rd party prv_cap table");
        sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorScenarioMain2 = NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    }
    else if(NSCam::v1::Stereo::STEREO_BB_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
            pipelineMode == PipelineMode_ZSD)
    {
        // MY_LOGD("get bb prv_cap table");
        sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorScenarioMain2 = (!isWidePlusTele()) ? NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE : NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
    else if(NSCam::v1::Stereo::STEREO_BB_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
            pipelineMode == PipelineMode_RECORDING)
    {
        // MY_LOGD("get bb rec table");
        sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        sensorScenarioMain2 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
    else if(NSCam::v1::Stereo::STEREO_BM_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
            pipelineMode == PipelineMode_ZSD)
    {
        // MY_LOGD("get bm prv_cap table");
        sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorScenarioMain2 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
    else if(NSCam::v1::Stereo::STEREO_BM_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
            pipelineMode == PipelineMode_RECORDING)
    {
        // MY_LOGD("get bm rec table");
        // record mode
        sensorScenarioMain1 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        sensorScenarioMain2 = NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
    else
    {
        MY_LOGE("should not happened!");
        return false;
    }

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