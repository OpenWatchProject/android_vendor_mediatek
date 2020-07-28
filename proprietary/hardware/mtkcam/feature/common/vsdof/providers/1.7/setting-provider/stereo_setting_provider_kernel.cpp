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
#define LOG_TAG "StereoSettingProviderKernel"

#include "stereo_setting_provider_kernel.h"
#include <mtkcam/utils/std/Log.h>
#include <string.h>

#include <camera_custom_stereo.h>   //For DEFAULT_STEREO_SETTING
#include <vsdof/hal/ProfileUtil.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <cctype>   //isspace
#include <cmath>    //pow, tan, atan, sqrt

using namespace StereoHAL;

Mutex StereoSettingProviderKernel::__instanceLock;
StereoSettingProviderKernel * StereoSettingProviderKernel::__instance = NULL;

#define PROPERTY_ENABLE_CUSTOM_SETTING  STEREO_PROPERTY_PREFIX"custom_setting"      //rea/write SETTING_FILE_PATH
#define PROPERTY_EXPORT_SETTING         STEREO_PROPERTY_PREFIX"custom_setting.exp"  //write SETTING_HEADER_PATH
#define SETTING_HEADER_PATH             "/sdcard/camera_custom_stereo_setting.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

using namespace std;
using namespace NSCam;
using namespace StereoHAL;
using namespace NSCam::v1::Stereo;

std::string sizeToString(const MSize size)
{
    return std::to_string(size.w)+"x"+std::to_string(size.h);
}

std::string ratioToString(ENUM_STEREO_RATIO ratio)
{
    int m, n;
    imageRatioMToN(ratio, m, n);
    return std::to_string(m)+":"+std::to_string(n);
}

MUINT stringToSensorScenario(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    //Preview
    if(0 == s.compare("preview")) {
        return SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
    //Capture
    if(0 == s.compare("capture")) {
        return SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    }
    //Video
    if(0 == s.compare("video")) {
        return SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    }
    //High speed video
    if(0 == s.compare("hsvideo")) {
        return SENSOR_SCENARIO_ID_SLIM_VIDEO1;
    }
    //Slim video
    if(0 == s.compare("slimvideo")) {
        return SENSOR_SCENARIO_ID_SLIM_VIDEO2;
    }
    //Video
    if(0 == s.compare("video")) {
        return SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    }
    //Custom1
    if(0 == s.compare("custom1")) {
        return SENSOR_SCENARIO_ID_CUSTOM1;
    }
    //Custom2
    if(0 == s.compare("custom2")) {
        return SENSOR_SCENARIO_ID_CUSTOM2;
    }
    //Custom3
    if(0 == s.compare("custom3")) {
        return SENSOR_SCENARIO_ID_CUSTOM3;
    }
    //Custom4
    if(0 == s.compare("custom4")) {
        return SENSOR_SCENARIO_ID_CUSTOM4;
    }
    //Custom5
    if(0 == s.compare("custom5")) {
        return SENSOR_SCENARIO_ID_CUSTOM5;
    }

    return SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
}

std::string sensorScenarioToString(MUINT s)
{
    switch(s)
    {
    case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
        return std::string("preview");
        break;
    case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
    default:
        return std::string("capture");
        break;
    case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
        return std::string("video");
        break;
    case SENSOR_SCENARIO_ID_CUSTOM1:
        return std::string("custom1");
        break;
    case SENSOR_SCENARIO_ID_CUSTOM2:
        return std::string("custom2");
        break;
    case SENSOR_SCENARIO_ID_CUSTOM3:
        return std::string("custom3");
        break;
    case SENSOR_SCENARIO_ID_CUSTOM4:
        return std::string("custom4");
        break;
    case SENSOR_SCENARIO_ID_CUSTOM5:
        return std::string("custom5");
        break;
    }

    return std::string("capture");
}

int stringToFeatureMode(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    int featuerMode = 0;

    //StereoCapture
    if(0 == s.compare(CUSTOM_KEY_STEREO_CAPTURE)) {
        featuerMode = E_STEREO_FEATURE_CAPTURE;
    }
    //VSDoF
    if(s.find(CUSTOM_KEY_VSDOF) != std::string::npos) {
        featuerMode = E_STEREO_FEATURE_VSDOF;
    }
    //Denoise
    if(0 == s.compare(CUSTOM_KEY_DENOISE)) {
        featuerMode = E_STEREO_FEATURE_DENOISE;
    }
    //3rdParty
    if(0 == s.compare(CUSTOM_KEY_3RD_Party)) {
        featuerMode = E_STEREO_FEATURE_THIRD_PARTY;
    }
    //Zoom
    if(0 == s.compare(CUSTOM_KEY_ZOOM)) {
        featuerMode = E_DUALCAM_FEATURE_ZOOM;
    }
    //MtkDepthmap
    if(s.find(CUSTOM_KEY_MTK_DEPTHMAP) != std::string::npos) {
        featuerMode = E_STEREO_FEATURE_MTK_DEPTHMAP;
    }
    //Multi-cam
    if(s.find(CUSTOM_KEY_MULTI_CAM) != std::string::npos) {
        featuerMode = E_STEREO_FEATURE_MULTI_CAM;
    }

    if((s.find("+portrait") != std::string::npos)
        || (s.find("+p") != std::string::npos)
        || (s.find("+half") != std::string::npos))
    {
        featuerMode |= E_STEREO_FEATURE_PORTRAIT_FLAG;
    }

    return featuerMode;
}

vector<std::string> featureModeToStrings(int featureMode)
{
    vector<std::string> features;
    if(featureMode & E_STEREO_FEATURE_CAPTURE) {
        features.push_back(CUSTOM_KEY_STEREO_CAPTURE);
    }
    if(featureMode & E_STEREO_FEATURE_VSDOF) {
        features.push_back(CUSTOM_KEY_VSDOF);
    }
    if(featureMode & E_STEREO_FEATURE_DENOISE) {
        features.push_back(CUSTOM_KEY_DENOISE);
    }
    if(featureMode & E_STEREO_FEATURE_THIRD_PARTY) {
        features.push_back(CUSTOM_KEY_3RD_Party);
    }
    if(featureMode & E_DUALCAM_FEATURE_ZOOM) {
        features.push_back(CUSTOM_KEY_ZOOM);
    }
    if(featureMode & E_STEREO_FEATURE_MTK_DEPTHMAP) {
        features.push_back(CUSTOM_KEY_MTK_DEPTHMAP);
    }
    if(featureMode & E_STEREO_FEATURE_MULTI_CAM) {
        features.push_back(CUSTOM_KEY_MULTI_CAM);
    }

    return features;
}

MSize stringToSize(std::string str)
{
    MSize size;
    sscanf(str.c_str(), "%dx%d", &size.w, &size.h);
    return size;
}

ENUM_STEREO_RATIO stringToRatio(std::string str)
{
    int m, n;
    sscanf(str.c_str(), "%d:%d", &m, &n);
    return MToNToImageRatio(m, n);
}

void base64Decode(const char *base64Str, vector<MUINT8> &buffer)
{
    size_t base64_len = strlen(base64Str);
    size_t out_len = base64_len/4 * 3;
    for(int i = base64_len-1; i >= 0; --i) {
        if('=' == base64Str[i]) {
            out_len--;
            base64_len--;
        } else {
            break;
        }
    }
    // MY_LOGD("Base64 size %zu, out size %zu", base64_len, out_len);
    if(buffer.size() != out_len) {
        buffer.resize(out_len);
    }

    static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

    int i = 0;
    int in_pos = 0;
    int out_pos = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (base64_len--) {
        char_array_4[i++] = base64_chars.find(base64Str[in_pos++]);
        if (i == 4) {
            buffer[out_pos++] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
            buffer[out_pos++] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            buffer[out_pos++] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

            i = 0;
        }
    }

    if (i) {
        char_array_3[0] = ( char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (int j = 0; (j < i - 1); j++) {
            buffer[out_pos++] = char_array_3[j];
        }
    }
}

void from_json(const json &settingJson, StereoSensorSetting_T& setting)
{
    //Get Name(MUST)
    //  Name must copy from kernel-4.9\drivers\misc\mediatek\imgsensor\inc\kd_imgsensor.h,
    //  definition of SENSOR_DRVNAME_<sensor name>
    std::string name = settingJson[CUSTOM_KEY_NAME].get<std::string>();
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    // The name had been checked in __parserDocumentm no need to change here
    // if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
    //     name = SENSOR_DRVNAME_PREFIX + name;
    // }

    setting.name = name;
    MSize size;
    ENUM_STEREO_RATIO ratio;

    //Get Capture size(MUST for main cam)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_CAPTURE_SIZE)) {
        const json &sizeConfig = settingJson[CUSTOM_KEY_CAPTURE_SIZE];
        for(json::const_iterator it = sizeConfig.begin(); it != sizeConfig.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            vector<MSize> sizes;
            for(auto &sizeSetting : it.value()) {
                sizes.push_back(stringToSize(sizeSetting.get<std::string>()));
            }

            if(sizes.size() > 0) {
                setting.captureSizes[ratio] = std::move(sizes);
                setting.supportedImageRatios[ratio] = true;
            }
        }
    }

    //Get cusomized IMGO YUV size(optional, used by pure 3rd party flow)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_IMGOYUV_SIZE))
    {
        const json &sizeConfig = settingJson[CUSTOM_KEY_IMGOYUV_SIZE];
        for(json::const_iterator it = sizeConfig.begin(); it != sizeConfig.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            size = stringToSize(it.value().get<std::string>());
            setting.imgoYuvSize[ratio] = size;
        }
    }

    //Get cusomized RRZO YUV size(optional, used by pure 3rd party flow)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_RRZOYUV_SIZE))
    {
        const json &sizeConfig = settingJson[CUSTOM_KEY_RRZOYUV_SIZE];
        for(json::const_iterator it = sizeConfig.begin(); it != sizeConfig.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            size = stringToSize(it.value().get<std::string>());
            setting.rrzoYuvSize[ratio] = size;
        }
    }

    //Get cusomized PostView size(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_POSTVIEW_SIZE)) {
        const json &sizeConfig = settingJson[CUSTOM_KEY_POSTVIEW_SIZE];
        for(json::const_iterator it = sizeConfig.begin(); it != sizeConfig.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            size = stringToSize(it.value().get<std::string>());
            setting.postviewSize[ratio] = size;
        }
    }

    //Get sensor scenario of ZSD mode(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_SENSOR_SCENARIO_ZSD))
    {
        int feature;
        const json &config = settingJson[CUSTOM_KEY_SENSOR_SCENARIO_ZSD];
        for(json::const_iterator it = config.begin(); it != config.end(); ++it)
        {
            feature = stringToFeatureMode(it.key());
            setting.sensorScenarioMapZSD[feature] = stringToSensorScenario(it.value().get<std::string>());
        }
    }

    //Get sensor scenario of Recording mode(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_SENSOR_SCENARIO_REC))
    {
        int feature;
        const json &config = settingJson[CUSTOM_KEY_SENSOR_SCENARIO_REC];
        for(json::const_iterator it = config.begin(); it != config.end(); ++it)
        {
            feature = stringToFeatureMode(it.key());
            setting.sensorScenarioMapRecord[feature] = stringToSensorScenario(it.value().get<std::string>());
        }
    }

    //Get FOV(optional, use static info(integer) if not set)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_FOV)) {
        const json &fovValue = settingJson[CUSTOM_KEY_FOV];
        if(LogicalCamJSONUtil::HasMember(fovValue, CUSTOM_KEY_FOV_D)) {
            setting.fovDiagonal   = fovValue[CUSTOM_KEY_FOV_D].get<float>();
        } else {
            setting.fovHorizontal = fovValue[CUSTOM_KEY_FOV_H].get<float>();
            setting.fovVertical   = fovValue[CUSTOM_KEY_FOV_V].get<float>();
        }
    }

    //Get calibration data(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_CALIBRATION)) {
        const json &calibrationValue = settingJson[CUSTOM_KEY_CALIBRATION];
        setting.distanceMacro    = calibrationValue[CUSTOM_KEY_MACRO_DISTANCE].get<int>();
        setting.distanceInfinite = calibrationValue[CUSTOM_KEY_INFINITE_DISTANCE].get<int>();
    }
}

void from_json(const json &settingJson, StereoSensorConbinationSetting_T& setting)
{
    int m, n;
    MSize size;
    ENUM_STEREO_RATIO ratio;

    //Get module type(MUST)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_MODULE_TYPE)) {
        setting.moduleType = settingJson[CUSTOM_KEY_MODULE_TYPE].get<int>();
    } else {
        MY_LOGE("Module type is undefined");
        return;
    }

    //Get Baseline(optional for pure 3rd party)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_BASELINE)) {
        setting.baseline = settingJson[CUSTOM_KEY_BASELINE].get<float>();
    }

    //Get module variation(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_MODULE_VARIATION)) {
        setting.moduleVariation = settingJson[CUSTOM_KEY_MODULE_VARIATION].get<float>();
        if(setting.moduleVariation < 0.0f) {
            setting.moduleVariation = 0.0f;
        }
    }

    //Get working range(optional, should adjust for W+T VSDoF)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_WORKING_RANGE)) {
        setting.workingRange = settingJson[CUSTOM_KEY_WORKING_RANGE].get<float>();
        if(setting.workingRange <= 0) {
            setting.workingRange = DEFAULT_WORKING_RANGE;
        }
    }

    //Get FOV crop(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_FOV_CROP)) {
        const json &fovCropValue = settingJson[CUSTOM_KEY_FOV_CROP];
        if(LogicalCamJSONUtil::HasMember(fovCropValue, CUSTOM_KEY_CENTER_CROP)) {
            setting.isCenterCrop = !!(fovCropValue[CUSTOM_KEY_CENTER_CROP].get<int>());
        }

        if(LogicalCamJSONUtil::HasMember(fovCropValue, CUSTOM_KEY_DISABLE_CROP)) {
            setting.disableCrop = !!(fovCropValue[CUSTOM_KEY_DISABLE_CROP].get<int>());
        }
    }

    //Get customized depthmap size(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_DEPTHMAP_SIZE)) {
        const json &sizeConfig = settingJson[CUSTOM_KEY_DEPTHMAP_SIZE];
        for(json::const_iterator it = sizeConfig.begin(); it != sizeConfig.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            size = stringToSize(it.value().get<std::string>());
            setting.depthmapSize[ratio] = size;
        }
    }

    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_DEPTHMAP_CAPTURE_SIZE)) {
        const json &sizeConfig = settingJson[CUSTOM_KEY_DEPTHMAP_CAPTURE_SIZE];
        for(json::const_iterator it = sizeConfig.begin(); it != sizeConfig.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            size = stringToSize(it.value().get<std::string>());
            setting.depthmapSizeCapture[ratio] = size;
        }
    }

    //Get LDC(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_LDC)) {
        setting.LDC.clear();

        stringstream ss;
        float number;
        for(auto &line : settingJson[CUSTOM_KEY_LDC]) {
            std::string strLDC = line.get<std::string>();
            char *start = (char *)strLDC.c_str();
            if(start) {
                char *end = NULL;
                do {
                    number = ::strtof(start, &end);
                    if  ( start == end ) {
                        // MY_LOGD("No LDC data: %s", start);
                        break;
                    }
                    setting.LDC.push_back(number);
                    start = end + 1;
                } while ( end && *end );
            }
        }

        setting.enableLDC = (setting.LDC.size() > 0);
    }

    //Get offline calibration(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_CALIBRATION)) {
        setting.calibrationData.clear();
        base64Decode(settingJson[CUSTOM_KEY_CALIBRATION].get<std::string>().c_str(), setting.calibrationData);
    }

    //Get customized base size(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_SIZE_CONFIG)) {
        const json &sizeValue = settingJson[CUSTOM_KEY_SIZE_CONFIG];

        StereoArea area;
        MSize contentSize;

        for(json::const_iterator it = sizeValue.begin(); it != sizeValue.end(); ++it)
        {
            ratio = stringToRatio(it.key());
            const json &config = it.value();
            area.padding = stringToSize(config[CUSTOM_KEY_PADDING].get<std::string>());
            area.startPt.x = area.padding.w/2;
            area.startPt.y = area.padding.h/2;
            contentSize = stringToSize(config[CUSTOM_KEY_CONTENT_SIZE].get<std::string>());
            if(0 != contentSize.w % 16) {
                MSize newSize = contentSize;
                //Width must be 16-align
                applyNAlign(16, newSize.w);

                imageRatioMToN(ratio, m, n);

                //Height must be even
                if(1 == setting.moduleType ||
                   3 == setting.moduleType)
                {
                    newSize.h = newSize.w * m / n;
                } else {
                    newSize.h = newSize.w * n / m;
                }
                applyNAlign(2, newSize.h);

                MY_LOGW("Content width must be 16-aligned, adjust size for 16:9 from %dx%d to %dx%d",
                        contentSize.w, contentSize.h, newSize.w, newSize.h);
                contentSize = newSize;
            }

            area.size = contentSize + area.padding;
            setting.baseSize[ratio] = area;
            setting.hasSizeConfig = true;
        }
    }
}

StereoSettingProviderKernel *
StereoSettingProviderKernel::getInstance()
{
    Mutex::Autolock lock(__instanceLock);

    if(NULL == __instance) {
        __instance = new StereoSettingProviderKernel();
    }

    return __instance;
}

void
StereoSettingProviderKernel::destroyInstance()
{
    Mutex::Autolock lock(__instanceLock);

    if(__instance) {
        delete __instance;
        __instance = NULL;
    }
}

StereoSensorSetting_T *
StereoSettingProviderKernel::getSensorSetting(int sensorIndex)
{
    if(0 == __sensorSettings.size()) {
        init();
    }

    std::map<std::string, StereoSensorSetting_T>::iterator it = __sensorSettings.begin();
    for(;it != __sensorSettings.end(); ++it) {
        if(it->second.index == sensorIndex) {
            return &it->second;
        }
    }

    MY_LOGE("Cannot get sensor setting of sensor %d", sensorIndex);
    return NULL;
}

StereoSensorConbinationSetting_T *
StereoSettingProviderKernel::getSensorCombinationSetting(ENUM_STEREO_SENSOR_PROFILE profile)
{
    if(0 == __sensorCombinationSettings.size()) {
        init();
    }

    if(1 == __sensorCombinationSettings.size()) {
        return &__sensorCombinationSettings[0];
    }

    for(int i = __sensorCombinationSettings.size() - 1; i >= 0; --i) {
        if(profile == __sensorCombinationSettings[i].profile) {
            return &__sensorCombinationSettings[i];
        }
    }

    MY_LOGE("Sensor combination not found for profile: %d", profile);
    return NULL;
}

bool
StereoSettingProviderKernel::getSensorIndexesOfProfile(ENUM_STEREO_SENSOR_PROFILE profile, int &sensorIdx1, int &sensorIdx2)
{
    if(0 == __sensorCombinationSettings.size()) {
        init();
    }

    if(1 == __sensorCombinationSettings.size()) {
        sensorIdx1 = __sensorCombinationSettings[0].sensorSettings[0]->index;
        sensorIdx2 = __sensorCombinationSettings[0].sensorSettings[1]->index;
        return true;
    } else {
        for(auto &sc : __sensorCombinationSettings) {
            if(sc.profile == profile) {
                sensorIdx1 = sc.sensorSettings[0]->index;
                sensorIdx2 = sc.sensorSettings[1]->index;
                return true;
            }
        }
    }

    return false;
}

StereoSettingProviderKernel::StereoSettingProviderKernel()
    : __logger(LOG_TAG, PROPERTY_SETTING_LOG)
{
    __logger.setSingleLineMode(0);
}

StereoSettingProviderKernel::~StereoSettingProviderKernel()
{
    __reset();
}

void
StereoSettingProviderKernel::init()
{
    AutoProfileUtil profile(LOG_TAG, "init");
    Mutex::Autolock lock(__instanceLock);

    IHalSensorList *pSensorList = MAKE_HalSensorList();
    if (NULL == pSensorList) {
        MY_LOGE("Cannot get sensor list");
        return;
    }

    int32_t sensorCount = pSensorList->queryNumberOfSensors();
    for(int index = 0; index < sensorCount; ++index) {
        std::string s(pSensorList->queryDriverName(index));
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        __sensorNameMap[s] = index;
    }

    if(LOG_ENABLED) {
        std::ostringstream oss;
        oss << "Get " << sensorCount << " sensors: ";
        for(auto it = __sensorNameMap.begin(); it != __sensorNameMap.end(); ++it) {
            oss << it->first;
            if(std::next(it) != __sensorNameMap.end()) {
                oss << ", ";
            }
        }

        MY_LOGD("%s", oss.str().c_str());
    }

    //Init
    json loadedJson;
    int status = LogicalCamJSONUtil::parseLogicalCamCustomSetting(loadedJson);
    if(IS_JSON_FROM_DEFAULT(status) &&
       (1 == checkStereoProperty(PROPERTY_ENABLE_CUSTOM_SETTING)))
    {
        __saveSettingToFile(loadedJson);
    }

    if(status &&
       loadedJson != __json)  //Setting changes
    {
        __reset();

        try
        {
            __parseDocument(loadedJson);
        }
        catch (json::exception& e)
        {
            MY_LOGW("Parse error: %s", e.what());
        }
        __loadSettingsFromSensorHAL();
        __loadSettingsFromCalibration();

        __json = loadedJson;
    }

    // logSettings();
}

void
StereoSettingProviderKernel::__saveSettingToFile(json &json)
{
    //Save to json
    std::string s = json.dump(4);   //set indent of space
    char *data = (char *)s.c_str();

    FILE *fp = fopen(SETTING_FILE_PATH, "w");
    if(fp) {
        fwrite(data, 1, strlen(data), fp);

        fflush(fp);
        fclose(fp);
    }

    if(1 == checkStereoProperty(PROPERTY_EXPORT_SETTING)) {
        FILE *fp = fopen(SETTING_HEADER_PATH, "w");
        if(fp) {
            //Write copy right
            const char *COPY_RIGHT =
            "/* Copyright Statement:\n"
            " *\n"
            " * This software/firmware and related documentation (\"MediaTek Software\") are\n"
            " * protected under relevant copyright laws. The information contained herein\n"
            " * is confidential and proprietary to MediaTek Inc. and/or its licensors.\n"
            " * Without the prior written permission of MediaTek inc. and/or its licensors,\n"
            " * any reproduction, modification, use or disclosure of MediaTek Software,\n"
            " * and information contained herein, in whole or in part, shall be strictly prohibited.\n"
            " */\n"
            "/* MediaTek Inc. (C) 2018. All rights reserved.\n"
            " *\n"
            " * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES\n"
            " * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (\"MEDIATEK SOFTWARE\")\n"
            " * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON\n"
            " * AN \"AS-IS\" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,\n"
            " * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF\n"
            " * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.\n"
            " * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE\n"
            " * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR\n"
            " * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH\n"
            " * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES\n"
            " * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES\n"
            " * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK\n"
            " * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR\n"
            " * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND\n"
            " * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,\n"
            " * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,\n"
            " * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO\n"
            " * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.\n"
            " *\n"
            " * The following software/firmware and/or related documentation (\"MediaTek Software\")\n"
            " * have been modified by MediaTek Inc. All revisions are subject to any receiver's\n"
            " * applicable license agreements with MediaTek Inc.\n"
            " */\n\n"
            "/********************************************************************************************\n"
            " *     LEGAL DISCLAIMER\n"
            " *\n"
            " *     (Header of MediaTek Software/Firmware Release or Documentation)\n"
            " *\n"
            " *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES\n"
            " *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (\"MEDIATEK SOFTWARE\") RECEIVED\n"
            " *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN \"AS-IS\" BASIS\n"
            " *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,\n"
            " *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR\n"
            " *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY\n"
            " *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,\n"
            " *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK\n"
            " *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO\n"
            " *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION\n"
            " *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.\n"
            " *\n"
            " *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH\n"
            " *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,\n"
            " *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE\n"
            " *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.\n"
            " *\n"
            " *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS\n"
            " *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.\n"
            " ************************************************************************************************/\n"
            "#ifndef CAMERA_CUSTOM_STEREO_SETTING_H_\n"
            "#define CAMERA_CUSTOM_STEREO_SETTING_H_\n"
            "const char *DEFAULT_STEREO_SETTING =\n";
            fwrite(COPY_RIGHT, 1, strlen(COPY_RIGHT), fp);

            //Write JSON line-by-line
            char lineBuffer[1024];
            char *line = strtok(data, "\n");
            int lineSize;
            int i, j;
            bool isPreSpace = true;
            while(line) {
                lineSize = strlen(line);
                isPreSpace = true;
                // lineBuffer[0] = '\"';
                for(i = 0, j = 0; i < lineSize; ++i, ++j) {
                    if(isPreSpace &&
                       !isspace(line[i]))
                    {
                        isPreSpace = false;
                        lineBuffer[j++] = '"';
                    }

                    if(line[i] == '\"') {
                        lineBuffer[j++] = '\\';
                    }

                    lineBuffer[j] = line[i];
                }
                line  = strtok(NULL, "\n");
                lineBuffer[j++] = '\"';

                if(NULL == line) {
                    lineBuffer[j++] = ';';
                }
                lineBuffer[j++] = '\n';

                fwrite(lineBuffer, 1, j, fp);
            }

            const char *LAST_LINE = "#endif\n";
            size_t LAST_LINE_SIZE = strlen(LAST_LINE);
            ::memcpy(lineBuffer, LAST_LINE, LAST_LINE_SIZE);
            fwrite(lineBuffer, 1, LAST_LINE_SIZE, fp);

            fflush(fp);
            fclose(fp);
        }
    }
}

void
StereoSettingProviderKernel::__parseDocument(json &jsonObj)
{
    AutoProfileUtil profile(LOG_TAG, "  Parse docuemnt");

    json &sensorValues = jsonObj[CUSTOM_KEY_SENSORS];

    std::string name;
    for(auto &settingValue : sensorValues) {
        //Check if the sensor is installed
        name = settingValue[CUSTOM_KEY_NAME].get<std::string>();
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
            name = SENSOR_DRVNAME_PREFIX + name;
        }

        if(__sensorNameMap.find(name) == __sensorNameMap.end()) {
            MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Setting]Sensor %s is not installed on this device",
                    settingValue[CUSTOM_KEY_NAME].get<std::string>().c_str());
            continue;
        }

        //Use from_json(const json &settingJson, StereoSensorSetting_T& setting)
        StereoSensorSetting_T setting = settingValue;
        setting.index = __sensorNameMap[name];
        MY_LOGD_IF(LOG_ENABLED, "Set index %d to sensor %s", setting.index, setting.name.c_str());
        __sensorSettings[name] = std::move(setting);
    }

    //Get settings of sensor combinations
    std::map<std::string, StereoSensorSetting_T>::iterator sensorIt;
    std::vector<StereoSensorSetting_T *> sensorSettings;
    for(auto &settingValue : jsonObj[CUSTOM_KEY_SENSOR_COMBINATIONS])
    {
        sensorSettings.clear();
        //Check if the sensors are installed
        if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_SENSORS))
        {
            //Use "Sensors"
            bool hasError = false;
            for(auto &s : settingValue[CUSTOM_KEY_SENSORS]) {
                name = s.get<std::string>();
                std::transform(name.begin(), name.end(), name.begin(), ::toupper);
                if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
                    name = SENSOR_DRVNAME_PREFIX + name;
                }

                sensorIt = __sensorSettings.find(name);
                if(__sensorNameMap.find(name) == __sensorNameMap.end() ||
                   sensorIt == __sensorSettings.end())
                {
                    MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Combination]Sensor %s is not installed on this device",
                            s.get<std::string>().c_str());
                    hasError = true;
                    break;
                }

                sensorSettings.push_back(&sensorIt->second);
            }

            if(hasError) {
                continue;
            }
        }
        else if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_MAIN1_NAME) &&
                LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_MAIN2_NAME))
        {
            //Use "Main1 Name" &＂Main2 Name"
            name = settingValue[CUSTOM_KEY_MAIN1_NAME].get<std::string>();
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);
            if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
                name = SENSOR_DRVNAME_PREFIX + name;
            }

            sensorIt = __sensorSettings.find(name);
            if(__sensorNameMap.find(name) == __sensorNameMap.end() ||
               sensorIt == __sensorSettings.end())
            {
                MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Combination]Sensor %s is not installed on this device",
                        settingValue[CUSTOM_KEY_MAIN1_NAME].get<std::string>().c_str());
                continue;
            }

            sensorSettings.push_back(&sensorIt->second);

            name = settingValue[CUSTOM_KEY_MAIN2_NAME].get<std::string>();
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);
            if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
                name = SENSOR_DRVNAME_PREFIX + name;
            }
            sensorIt = __sensorSettings.find(name);
            if(__sensorNameMap.find(name) == __sensorNameMap.end() ||
               sensorIt == __sensorSettings.end())
            {
                MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Combination]Sensor %s is not installed on this device",
                        settingValue[CUSTOM_KEY_MAIN2_NAME].get<std::string>().c_str());
                continue;
            }

            sensorSettings.push_back(&sensorIt->second);
        }
        else
        {
            MY_LOGE("No sensor is defined");
            continue;
        }

        StereoSensorConbinationSetting_T setting;
        //Use from_json(const json &settingJson, StereoSensorConbinationSetting_T& setting)
        setting = settingValue;
        if(0 == sensorSettings[0]->staticInfo.facingDirection &&
           0 == sensorSettings[1]->staticInfo.facingDirection)
        {
            setting.profile = STEREO_SENSOR_PROFILE_REAR_REAR;
        }
        else if(1 == sensorSettings[0]->staticInfo.facingDirection &&
                1 == sensorSettings[1]->staticInfo.facingDirection)
        {
            setting.profile = STEREO_SENSOR_PROFILE_FRONT_FRONT;
        }
        else
        {
            setting.profile = STEREO_SENSOR_PROFILE_REAR_FRONT;
        }
        setting.sensorSettings = std::move(sensorSettings);
        __sensorCombinationSettings.push_back(std::move(setting));
    }

    //Get callback buffer list(optional)
    if(LogicalCamJSONUtil::HasMember(jsonObj, CUSTOM_KEY_CALLBACK_BUFFER_LIST)) {
        json &callbackSetting = jsonObj[CUSTOM_KEY_CALLBACK_BUFFER_LIST];
        if(LogicalCamJSONUtil::HasMember(callbackSetting, CUSTOM_KEY_VALUE)) {
            __callbackBufferListString = callbackSetting[CUSTOM_KEY_VALUE].get<std::string>();
        }
    }
}

void
StereoSettingProviderKernel::__loadSettingsFromSensorHAL()
{
    AutoProfileUtil profile(LOG_TAG, "  Load settings from Sensor HAL");

    //Update sensor ID & FOV
    IHalSensorList *sensorList = MAKE_HalSensorList();
    if (NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
        return;
    }

    SensorStaticInfo sensorStaticInfo;
    for(auto &setting : __sensorSettings) {
        auto &s = setting.second;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        s.devIndex = sensorList->querySensorDevIdx(s.index);
        sensorList->querySensorStaticInfo(s.devIndex, &sensorStaticInfo);
        s.staticInfo  = sensorStaticInfo;

        if(s.fovDiagonal != 0.0f) {
            //Calculate H&V FOVs
            IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, s.index);
            if(NULL == pIHalSensor) {
                MY_LOGE("Cannot get hal sensor for sensor %d(%s)", s.index, s.name.c_str());
                break;
            }

            //Get sensor crop win info
            SensorCropWinInfo rSensorCropInfo;
            int sensorScenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            MINT32 err = pIHalSensor->sendCommand(s.devIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                           (MUINTPTR)&sensorScenario, (MUINTPTR)&rSensorCropInfo, 0);
            if(!err)
            {
                float w = rSensorCropInfo.full_w;
                float h = rSensorCropInfo.full_h;
                s.fovVertical = 2.0f*radiansToDegree(atan(sqrt(pow(tan(degreeToRadians(s.fovDiagonal/2.0f)), 2)/(1+pow(w/h, 2)))));
                s.fovHorizontal = 2.0f*radiansToDegree(atan(w/h * tan(degreeToRadians(s.fovVertical/2.0f))));

                MY_LOGD("Convert diagonal FOV %f to H(%f) & V((%f), sensor size %dx%d",
                        s.fovDiagonal, s.fovHorizontal, s.fovVertical, rSensorCropInfo.full_w, rSensorCropInfo.full_h);
            }
            else
            {
                MY_LOGE("Cannot get sensor crop win info");
            }

            pIHalSensor->destroyInstance(LOG_TAG);
        } else {
            if(0.0f == s.fovHorizontal) {
                s.fovHorizontal = sensorStaticInfo.horizontalViewAngle;
            }

            if(0.0f == s.fovVertical) {
                s.fovVertical = sensorStaticInfo.verticalViewAngle;
            }
        }
    }
}

void
StereoSettingProviderKernel::__loadSettingsFromCalibration()
{
    AutoProfileUtil profile(LOG_TAG, "  Load calibration data");
#if (1==VSDOF_SUPPORTED)
    //Update calibration distance
    CAM_CAL_DATA_STRUCT calibrationData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MUINT32 queryResult;
    for(auto &setting : __sensorSettings) {
        auto &s = setting.second;
        if(0 == s.distanceMacro ||
           0 == s.distanceInfinite)
        {
            queryResult = pCamCalDrvObj->GetCamCalCalData(s.devIndex, CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&calibrationData);
            s.distanceMacro    = calibrationData.Single2A.S2aAF_t.AF_Macro_pattern_distance;    //unit: mm
            s.distanceInfinite = calibrationData.Single2A.S2aAF_t.AF_infinite_pattern_distance; //unit: mm
        }
    }
    pCamCalDrvObj->destroyInstance();
#endif
}

void
StereoSettingProviderKernel::logSettings()
{
    __logger
    .updateLogStatus()
    .FastLogD("=======================")
    .FastLogD("    Sensor Settings")
    .FastLogD("=======================");

    size_t size = __sensorSettings.size();
    for(auto &setting : __sensorSettings) {
        auto &s = setting.second;
        s.log(__logger);

        if(size-- > 1) {
            __logger .FastLogD("----------------------------------------------");
        }
    }

    __logger
    .FastLogD("=======================")
    .FastLogD("  Sensor Combinations")
    .FastLogD("=======================");
    for(auto &c : __sensorCombinationSettings) {
        c.log(__logger);
        __logger .FastLogD("----------------------------------------------");
    }

    __logger.print();
}

void
StereoSettingProviderKernel::__reset()
{
    __sensorSettings.clear();
    __sensorCombinationSettings.clear();
}