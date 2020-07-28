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

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);
#include "stereo_setting_provider_kernel.h"
#include <string.h>

#include <camera_custom_stereo.h>   //For DEFAULT_STEREO_SETTING
#include <vsdof/hal/ProfileUtil.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <cctype>   //isspace
#include <cmath>    //pow, tan, atan, sqrt


using namespace StereoHAL;

Mutex StereoSettingProviderKernel::__instanceLock;
StereoSettingProviderKernel * StereoSettingProviderKernel::__instance = NULL;

#define SETTING_HEADER_PATH             "/sdcard/camera_custom_stereo_setting.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s]" fmt"\n", __func__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s]" fmt"\n", __func__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

using namespace std;
using namespace NSCam;
using namespace StereoHAL;
using namespace NSCam::v1::Stereo;

/**
 * \brief Get internal used sensor name
 * \details The name in the setting may contains driver prefix and sensor index
 *          e.g.1 For given name "SENSOR_DRVNAME_imx386_mipi_raw:3"
 *                we'll return "IMX386_MIPI_RAW" and return 3
 *          e.g.2 For given name "imx386_mipi_raw"
 *                we'll return "IMX386_MIPI_RAW" and return -1
 *
 * \param name name to input and return
 * \return specified sensor index, if not specified, return -1
 */
int32_t __parseAndRefineSensorName(std::string &name)
{
    // Remove prefix
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    size_t prefixPos = name.find(SENSOR_DRVNAME_PREFIX);
    if(prefixPos != std::string::npos) {
        name.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
    }

    // Handle specified sensor ID
    size_t deliPos = name.find(DELIMITER);
    int customizeId = -1;
    if(deliPos != std::string::npos) {
        auto idString = name.substr(deliPos+DELIMITER.length());
        customizeId = std::stoi(idString);
        name = name.substr(0, deliPos);
    }

    return customizeId;
}

void from_json(const json &settingJson, StereoSensorSetting_T& setting)
{
    //Get Name(MUST)
    //  Name must copy from kernel-4.9\drivers\misc\mediatek\imgsensor\inc\kd_imgsensor.h,
    //  definition of SENSOR_DRVNAME_<sensor name>
    std::string name = settingJson[CUSTOM_KEY_NAME].get<std::string>();
    int customizeId = __parseAndRefineSensorName(name);
    setting.name = name;
    if(customizeId >= 0)
    {
        setting.index = customizeId;
    }

    MSize size;
    ENUM_STEREO_RATIO ratio;

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

    //Get FRZ Ratio(optional)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_FRZ_RATIO))
    {
        int feature;
        const json &config = settingJson[CUSTOM_KEY_FRZ_RATIO];
        for(json::const_iterator it = config.begin(); it != config.end(); ++it)
        {
            feature = stringToFeatureMode(it.key());
            setting.frzRatioSetting[feature] = it.value().get<float>();
        }
    }
}

void from_json(const json &settingJson, StereoSensorConbinationSetting_T& setting)
{
    int m, n;
    MSize size;
    ENUM_STEREO_RATIO ratio;

    //Get logical device(MUST for HAL3)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_LOGICAL_DEVICE)) {
        //Name(MUST)
        const json &logicalDeviceJson = settingJson[CUSTOM_KEY_LOGICAL_DEVICE];
        if(LogicalCamJSONUtil::HasMember(logicalDeviceJson, CUSTOM_KEY_NAME)) {
            setting.logicalDeviceName = logicalDeviceJson[CUSTOM_KEY_NAME].get<std::string>();
        } else {
            MY_LOGE("Name is undefined for logical device");
            return;
        }

        //Feature list(MUST)
        if(LogicalCamJSONUtil::HasMember(logicalDeviceJson, CUSTOM_KEY_FEATURES)) {
            for(auto &feature : logicalDeviceJson[CUSTOM_KEY_FEATURES]) {
                setting.logicalDeviceFeatureSet |= stringToFeatureMode(feature.get<std::string>());
            }
        } else {
            MY_LOGE("Features is undefined for logical device");
            return;
        }
    }

    //Get module type(MUST for stereo, will check later)
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_MODULE_TYPE)) {
        setting.moduleType = settingJson[CUSTOM_KEY_MODULE_TYPE].get<int>();
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

        //Workaround for N3D crash issue
        if(setting.LDC.size() == 0) {
            setting.LDC.push_back(0);
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

    // Get customized depthmap refine level
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_REFINE_LEVEL))
    {
        setting.depthmapRefineLevel = settingJson[CUSTOM_KEY_REFINE_LEVEL].get<ENUM_DEPTHMAP_REFINE_LEVEL>();
        if(setting.depthmapRefineLevel < E_DEPTHMAP_REFINE_NONE ||
           setting.depthmapRefineLevel >= E_DEPTHMAP_REFINE_MAX)
        {
            setting.depthmapRefineLevel = E_DEPTHMAP_REFINE_NONE;
        }
    }

    // Get customized zoom range
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_MULTICAM_ZOOM_RANGE)) {
        setting.multicamZoomRanges = settingJson[CUSTOM_KEY_MULTICAM_ZOOM_RANGE].get<std::vector<float>>();
        if(setting.multicamZoomRanges.size() == 0) {
            setting.multicamZoomRanges.push_back(1.0f);
        } else {
            std::sort(setting.multicamZoomRanges.begin(), setting.multicamZoomRanges.end());
        }
    }

    // Get customized zoom steps
    if(LogicalCamJSONUtil::HasMember(settingJson, CUSTOM_KEY_MULTICAM_ZOOM_STEPS)) {
        setting.multicamZoomSteps = settingJson[CUSTOM_KEY_MULTICAM_ZOOM_STEPS].get<std::vector<float>>();
        if(setting.multicamZoomSteps.size() == 0) {
            setting.multicamZoomSteps.push_back(1.0f);
        } else {
            std::sort(setting.multicamZoomSteps.begin(), setting.multicamZoomSteps.end());
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

    for(auto &s : __sensorSettings) {
        if(s.second.index == sensorIndex) {
            return &(s.second);
        }
    }

    MY_LOGD("Cannot get sensor setting of sensor %d", sensorIndex);
    return NULL;
}

StereoSensorConbinationSetting_T *
StereoSettingProviderKernel::getSensorCombinationSetting(MUINT32 logicalDeviceID)
{
    if(0 == __sensorCombinationSettings.size()) {
        init();
    }

    auto iter = __sensorCombinationSettings.find(logicalDeviceID);
    if(iter != __sensorCombinationSettings.end()) {
        return &(iter->second);
    }

    MY_LOGD("Sensor combination not found for logical device ID: %d", logicalDeviceID);
    return NULL;
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

    __sensorCount = pSensorList->queryNumberOfSensors();
    for(size_t index = 0; index < __sensorCount; ++index) {
        std::string s(pSensorList->queryDriverName(index));
        size_t prefixPos = s.find(SENSOR_DRVNAME_PREFIX);
        if(prefixPos != std::string::npos) {
            s.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
        }

        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        auto iter = __sensorNameMap.find(s);
        if(iter == __sensorNameMap.end())
        {
            std::vector<int32_t> v;
            v.push_back(index);
            __sensorNameMap.emplace(s, v);
        }
        else
        {
            iter->second.push_back(index);
        }
    }

    // We'll Parse and save json in parsed order for saving to files
    // However, parsing in order will be 2~3 times slower, we only do it when customization is enabled
    __IS_CUSTOM_SETTING_ENABLED = (1 == checkStereoProperty(PROPERTY_ENABLE_CUSTOM_SETTING));
    __IS_EXPORT_ENABLED         = (1 == checkStereoProperty(PROPERTY_EXPORT_SETTING));

    json loadedJson;
    int status = 0;
    if(!__IS_CUSTOM_SETTING_ENABLED && !__IS_EXPORT_ENABLED) {
        json readJson;
        {
            AutoProfileUtil profile(LOG_TAG, "Parse logical cam setting");
            status = LogicalCamJSONUtil::parseLogicalCamCustomSetting(readJson);
        }
        loadedJson = std::move(readJson);
    } else {
        KeepOrderJSON readJson;
        {
            AutoProfileUtil profile(LOG_TAG, "Parse logical cam setting");
            status = LogicalCamJSONUtil::parseLogicalCamCustomSetting(readJson);
        }

        if(__IS_EXPORT_ENABLED ||
           IS_JSON_FROM_DEFAULT(status))
        {
            __saveSettingToFile(readJson);
        }
        loadedJson = std::move(readJson);
    }

    if(status &&
       loadedJson != __json)  //Setting changes
    {
        __reset();

        try
        {
            __parseDocument(loadedJson);
            __loadSettingsFromSensorHAL();
            // __loadSettingsFromCalibration();
            __json = std::move(loadedJson);
        }
        catch (json::exception& e)
        {
            MY_LOGW("Parse error: %s", e.what());
        }
    }

    logSettings();
}

std::vector<float>
StereoSettingProviderKernel::getLensPoseTranslation(int32_t sensorId)
{
    std::vector<float> result(3, 0.0f);
    auto sensorSetting = getSensorSetting(sensorId);
    if(sensorSetting)
    {
        result = sensorSetting->lensPoseTranslation;
    }

    return result;
}

void
StereoSettingProviderKernel::__saveSettingToFile(KeepOrderJSON &jsonObj)
{
    AutoProfileUtil profile(LOG_TAG, "saveSettingToFile");
    //Save to json
    std::string s = jsonObj.dump(4);   //set indent of space
    char *data = (char *)s.c_str();

    // Only save to JSON when it does not exist
    struct stat st;
    if(__IS_CUSTOM_SETTING_ENABLED &&
       stat(STEREO_SETTING_FILE_PATH, &st))
    {
        FILE *fp = fopen(STEREO_SETTING_FILE_PATH, "w");
        if(fp) {
            fwrite(data, 1, strlen(data), fp);

            fflush(fp);
            fclose(fp);
        }
    }

    if(__IS_EXPORT_ENABLED) {
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
            "/* MediaTek Inc. (C) 2019. All rights reserved.\n"
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
        StereoSensorSetting_T setting = settingValue;
        name = setting.name;
        auto iter = __sensorNameMap.find(name);
        if(iter == __sensorNameMap.end()) {
            MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Setting]Sensor %s is not installed",
                       name.c_str());
            continue;
        }
        auto &nameIds = iter->second;

        auto tryToAddSetting = [&](string &name, StereoSensorSetting_T &setting)
        {
            // For printing
            #ifdef __func__
            #undef __func__
            #endif
            #define __func__ "__parseDocument"

            auto settingIt = __sensorSettings.find(name);
            if(settingIt == __sensorSettings.end())
            {
                __sensorSettings.emplace(name, std::move(setting));
                MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Setting]Sensor %s(%d) is installed", setting.name.c_str(), setting.index);
            }
            else
            {
                MY_LOGE("[Parse Sensor Setting]Sensor setting of %s(%d) exists, there is duplcated settings\n"
                        "If you want to sepecify sensor IDs for duplicated sensors, you'll need to sepcify for all of them",
                        setting.name.c_str(), setting.index);
            }

            #undef __func__
        };

        //Use from_json(const json &settingJson, StereoSensorSetting_T& setting)
        if(setting.index >= 0)
        {
            //Check if specified sensor ID is correct
            auto idIter = std::find(nameIds.begin(), nameIds.end(), setting.index);
            if(idIter == nameIds.end())
            {
                std::ostringstream oss;
                oss << " Need to correct sensor id for sensor setting: \""
                    << name << DELIMITER << setting.index << "\"" << endl
                    << " Available settings:" << endl;
                for(auto &id : nameIds) {
                    oss << "    \"" << name << DELIMITER << id << "\"" << endl;
                }
                MY_LOGE("%s", oss.str().c_str());
                continue;
            }

            // User should config lens post translation like this:
            // "Lens Pose Translation": [-0.05, 0, 0] and must not be list of array
            if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_LENS_POSE_TRANSLATION))
            {
                const json &config = settingValue[CUSTOM_KEY_LENS_POSE_TRANSLATION];
                if(config.size() > 0)
                {
                    if(config[0].type() != json::value_t::array)
                    {
                        //Config by "Lens Pose Translation": [-0.05, 0, 0],
                        setting.lensPoseTranslation = config.get<std::vector<float>>();
                    }
                    else
                    {
                        MY_LOGE("Please check \"%s\" for %s, the list must be 3 numbers",
                                CUSTOM_KEY_LENS_POSE_TRANSLATION, name.c_str());
                    }
                }
            }

            // If there are duplicated sensors, we'll append sensor index to senso name for key
            name = setting.name + DELIMITER + std::to_string(setting.index);
            tryToAddSetting(name, setting);
        }
        else
        {
            //Get lens pose translation(optional)
            //If not set, we'll update later when logical device is parsed
            // User should config lens post translation like this:
            // "Lens Pose Translation": [[-1.05, 0, 0], [-2.0, 0, 0]] and must be list of array
            // And the list size must match duplicated sensor size
            std::vector<std::vector<float>> lensPoseTranslations;
            if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_LENS_POSE_TRANSLATION))
            {
                const json &config = settingValue[CUSTOM_KEY_LENS_POSE_TRANSLATION];
                if(config.size() > 0)
                {
                    if(config.size() == nameIds.size() &&
                       config[0].type() == json::value_t::array)
                    {
                        //Config by "Lens Pose Translation": [[-1.05, 0, 0], [-2.0, 0, 0]], for duplicated sensor
                        for(auto &n : config)
                        {
                            lensPoseTranslations.push_back(n.get<std::vector<float>>());
                        }
                    }
                    else
                    {
                        MY_LOGE("Please check \"%s\" for %s, the list size must be %zu and each element must be array with 3 elements",
                                CUSTOM_KEY_LENS_POSE_TRANSLATION, name.c_str(), nameIds.size());
                    }
                }
            }

            int idx = 0;
            for(auto &id : nameIds)
            {
                setting.index = id;
                if(lensPoseTranslations.size() > 0)
                {
                    setting.lensPoseTranslation = lensPoseTranslations[idx];
                    ++idx;
                }

                name = setting.name + DELIMITER + std::to_string(setting.index);
                tryToAddSetting(name, setting);
            }
        }
    }

    //Get settings of sensor combinations
    int logicalDeviceID = __sensorCount;
    for(auto &settingValue : jsonObj[CUSTOM_KEY_SENSOR_COMBINATIONS])
    {
        std::vector<StereoSensorSetting_T *> sensorSettings;
        //Check if the sensors are installed
        if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_SENSORS))
        {
            //Use "Sensors"
            //Preprocess names
            //We can support duplicated sensors in many ways
            //e.g. If there are two sensors with the same name imx456 and sensor IDs are 3 & 4,
            //     we can config in following ways:
            //     1. Specify all sensor id for every dup sensors
            //        "Sensors": ["IMX123_MIPI_RAW", "IMX456_MIPI_RAW:3", "IMX456_MIPI_RAW:4"]
            //     2. Specify sensor id for dup sensor used
            //        "Sensors": ["IMX123_MIPI_RAW", "IMX456_MIPI_RAW:3"]
            //     3. Only left one sensor ID not specified, need to put all dup sensors in the list
            //        "Sensors": ["IMX123_MIPI_RAW", "IMX456_MIPI_RAW", "IMX456_MIPI_RAW:4"]
            //        "Sensors": ["IMX123_MIPI_RAW", "IMX456_MIPI_RAW:3", "IMX456_MIPI_RAW"]
            //     4. Not specify sensor ID, need to put all dup sensors in the list,
            //        we'll assume you want to use them in order of sensor ID
            //        "Sensors": ["IMX123_MIPI_RAW", "IMX456_MIPI_RAW" "IMX456_MIPI_RAW"]
            bool hasError = false;
            std::unordered_map<string, vector<int32_t>> leftIdsMap;
            std::vector<std::string> namesInSetting = settingValue[CUSTOM_KEY_SENSORS].get<std::vector<std::string>>();
            std::vector<int32_t> sensorIds(namesInSetting.size());
            std::vector<std::string> sensorNames(namesInSetting.size());
            std::unordered_map<string, size_t> nameCount;
            for(int settingIndex = 0; settingIndex < namesInSetting.size(); ++settingIndex)
            {
                auto name = namesInSetting[settingIndex];
                int customizedId = __parseAndRefineSensorName(name);
                sensorNames[settingIndex] = name;
                auto nameIt = __sensorNameMap.find(name);
                if(nameIt == __sensorNameMap.end())
                {
                    hasError = true;
                    break;
                }
                else
                {
                    auto leftIdIt = leftIdsMap.find(name);
                    if(leftIdIt == leftIdsMap.end())
                    {
                        auto ret = leftIdsMap.emplace(name, nameIt->second);
                        leftIdIt = ret.first;
                    }

                    {
                        auto countIt = nameCount.find(name);
                        if(countIt == nameCount.end())
                        {
                            nameCount.emplace(name, 1);
                        }
                        else
                        {
                            countIt->second++;
                        }
                    }

                    auto &ids = nameIt->second;
                    auto &leftIds = leftIdIt->second;
                    if(ids.size() == 1)
                    {
                        if(leftIds.size() < 1)
                        {
                            MY_LOGE("[Parse Sensor Combination]There are more than one \"%s\" in the setting, only one installed on the device",
                                    name.c_str());
                            hasError = true;
                            break;
                        }

                        sensorIds[settingIndex] = ids[0];
                        leftIds.clear();
                    }
                    else
                    {
                        if(customizedId < 0)
                        {
                            // This is the first step checking, need to check later since the ID may be used later
                            auto idIt = leftIdsMap.find(name);
                            if(idIt != leftIdsMap.end()) {
                                if(idIt->second.size() == 1) {
                                    sensorIds[settingIndex] = idIt->second[0];
                                    idIt->second.clear();
                                } else if(idIt->second.size() == 0) {
                                    MY_LOGE("[Parse Sensor Combination]There are more than one \"%s\" in the setting, please correct setting",
                                            namesInSetting[settingIndex].c_str());
                                    hasError = true;
                                    break;
                                } else {
                                    sensorIds[settingIndex] = -1;
                                }
                            }
                        }
                        else
                        {
                            // Sensor ID is specified, must be in installed list
                            if(leftIds.size() == 0)
                            {
                                MY_LOGE("[Parse Sensor Combination]Please remove duplicated sensor setting of \"%s\"",
                                        namesInSetting[settingIndex].c_str());
                                hasError = true;
                                break;
                            }
                            else
                            {
                                auto cusIdIt = std::find(leftIds.begin(), leftIds.end(), customizedId);
                                if(cusIdIt != leftIds.end())
                                {
                                    sensorIds[settingIndex] = customizedId;
                                    leftIds.erase(cusIdIt);
                                }
                                else
                                {
                                    std::ostringstream oss;
                                    oss << "[Parse Sensor Combination] Need to correct sensor id for sensor setting: \""
                                        << name << DELIMITER << customizedId << "\"" << endl
                                        << " Available settings:" << endl;
                                    for(auto &id : ids) {
                                        oss << "    \"" << name << DELIMITER << id << "\"" << endl;
                                    }
                                    MY_LOGE("%s", oss.str().c_str());
                                    hasError = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if(hasError) {
                continue;
            }

            // Scan if there are ID not given or other errors
            hasError = false;
            for(int idIndex = 0; idIndex < sensorIds.size(); ++idIndex)
            {
                auto &name = sensorNames[idIndex];
                if(sensorIds[idIndex] < 0)
                {
                    auto idIt = leftIdsMap.find(name);
                    if(idIt == leftIdsMap.end() ||
                       idIt->second.size() == 0)
                    {
                        MY_LOGE("[Parse Sensor Combination]Please remove the \"%s\" without ID specified", sensorNames[idIndex].c_str());
                        hasError = true;
                        break;
                    }
                    else if(idIt->second.size() == 1)
                    {
                        sensorIds[idIndex] = idIt->second[0];
                        idIt->second.clear();
                    }
                    else
                    {
                        auto nameIt = __sensorNameMap.find(name);
                        if(nameIt->second.size() == idIt->second.size() &&
                           nameCount[name] == nameIt->second.size())
                        {
                            //Give sensor IDs to all dup sensor without id-specified in sensor index order
                            auto &ids = nameIt->second;
                            int idx = 1;
                            sensorIds[idIndex] = ids[0];
                            for(int dupIdx = idIndex+1; dupIdx < sensorIds.size(); ++dupIdx)
                            {
                                if(sensorNames[dupIdx] == name)
                                {
                                    sensorIds[dupIdx] = ids[idx++];
                                }
                            }
                            idIt->second.clear();
                        }
                        else
                        {
                            MY_LOGE("[Parse Sensor Combination]Need to correct setting for %s,\n"
                                    "  Avaliable solutions:\n"
                                    "  1. Specify sensor ID if only one of the duplicated sensors are used\n"
                                    "  2. Remove all ID specified, we'll use them in sensor order\n"
                                    "  3. Left only one without specified and specify for the others",
                                    name.c_str());
                            hasError = true;
                            break;
                        }
                    }
                }
            }

            if(hasError) {
                continue;
            }

            for(int s = 0; s < sensorIds.size(); ++s)
            {
                auto sensorId = sensorIds[s];
                auto name = sensorNames[s] + DELIMITER + std::to_string(sensorId);
                auto sensorSettingIter = __sensorSettings.find(name);
                if(sensorSettingIter != __sensorSettings.end())
                {
                    sensorSettings.push_back(&(sensorSettingIter->second));
                    hasError = false;
                }
                else
                {
                    MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Combination]Sensor %s is not installed",
                                            sensorNames[s].c_str());
                }
            }

            if(sensorSettings.size() != namesInSetting.size())
            {
                continue;
            }
        }
        else
        {
            MY_LOGE("No sensor is defined");
            continue;
        }

        StereoSensorConbinationSetting_T setting;

        //Use from_json(const json &settingJson, StereoSensorConbinationSetting_T& setting)
        setting = settingValue;
        if(setting.moduleType == 0 &&
           sensorSettings.size() == 2 &&
           (    (setting.logicalDeviceFeatureSet & E_STEREO_FEATURE_CAPTURE)
             || (setting.logicalDeviceFeatureSet & E_STEREO_FEATURE_VSDOF)
             || (setting.logicalDeviceFeatureSet & E_STEREO_FEATURE_THIRD_PARTY)
             || (setting.logicalDeviceFeatureSet & E_STEREO_FEATURE_MTK_DEPTHMAP)
           ))
        {
            MY_LOGE("Module type must be set for stereo features");
        }

        // Automatically fill lens pose translation if possible
        if(sensorSettings.size() == 1)
        {
            if(sensorSettings[0]->lensPoseTranslation.size() == 0) {
                sensorSettings[0]->lensPoseTranslation.resize(3, 0.0f);
            }
        }
        else if(sensorSettings.size() != 2) //For sensors > 2, we only check if the data is filled
        {
            for(auto &sensorSetting: sensorSettings)
            {
                if(sensorSetting->lensPoseTranslation.size() == 0)
                {
                    MY_LOGE("%s of sensor %s(%d) is not set in multicam setting, please add for it, sample:\n"
                            "  \"%s\": [-0.05, 0, 0] (If a sensor ID is specified) or\n"
                            "  \"%s\": [[-0.05, 0, 0], [-1.05, 0, 0]] (For duplicated sensors share the same sensor setting)\n"
                            " Please fill the values in Google format, you can refer to:\n"
                            " https://developer.android.com/reference/android/hardware/camera2/CameraCharacteristics.html#LENS_POSE_TRANSLATION",
                            CUSTOM_KEY_LENS_POSE_TRANSLATION,
                            sensorSetting->name.c_str(), sensorSetting->index,
                            CUSTOM_KEY_LENS_POSE_TRANSLATION, CUSTOM_KEY_LENS_POSE_TRANSLATION);
                }
            }
        }
        else if(setting.moduleType > 0 &&
                setting.baseline > 0.0f)
        {
            auto minSensorId = std::min(sensorSettings[0]->index, sensorSettings[1]->index);
            for(auto &sensorSetting: sensorSettings)
            {
                if(sensorSetting->lensPoseTranslation.size() == LENS_POSE_TRANSLATION_SIZE)
                {
                    continue;
                }

                if(sensorSetting->index == minSensorId)
                {
                    sensorSetting->lensPoseTranslation.resize(3, 0.0f);
                }
                else
                {
                    float baseline = setting.baseline/100.0f;  //in meter

                    //Exchange direction if sensor index [0] > [1]
                    if(sensorSetting->index != sensorSettings[1]->index) {
                        baseline = -baseline;
                    }

                    if(0 == sensorSettings[0]->staticInfo.facingDirection)
                    {
                        switch(setting.moduleType)
                        {
                            case 0:
                            case 1:
                            default:
                                sensorSetting->lensPoseTranslation = {baseline, 0, 0};
                                break;
                            case 2:
                                sensorSetting->lensPoseTranslation = {0, -baseline, 0};
                                break;
                            case 3:
                                sensorSetting->lensPoseTranslation = {-baseline, 0, 0};
                                break;
                            case 4:
                                sensorSetting->lensPoseTranslation = {0, baseline, 0};
                                break;
                        }
                    }
                    else
                    {
                        switch(setting.moduleType)
                        {
                            case 0:
                            case 1:
                            case 2:
                            default:
                                sensorSetting->lensPoseTranslation = {-baseline, 0, 0};
                                break;
                            case 3:
                            case 4:
                                sensorSetting->lensPoseTranslation = {baseline, 0, 0};
                                break;
                        }
                    }
                }

                MY_LOGD("%s of sensor %s(%d) init to [%.5f %.5f %.5f]",
                        CUSTOM_KEY_LENS_POSE_TRANSLATION,
                        sensorSetting->name.c_str(), sensorSetting->index,
                        sensorSetting->lensPoseTranslation[0],
                        sensorSetting->lensPoseTranslation[1],
                        sensorSetting->lensPoseTranslation[2]);
            }
        }

        setting.sensorSettings  = std::move(sensorSettings);
        setting.logicalDeviceID = logicalDeviceID;

        //Parse overwrite sensor scenario
        if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_SENSOR_SCENARIO_ZSD))
        {
            const json &config = settingValue[CUSTOM_KEY_SENSOR_SCENARIO_ZSD];
            if(config.size() == setting.sensorSettings.size())
            {
                for(json::const_iterator it = config.begin(); it != config.end(); ++it)
                {
                    setting.sensorScenariosZSD.push_back(stringToSensorScenario(it.value().get<std::string>()));
                }
            }
            else
            {
                MY_LOGE("ZSD sensor scenario size(%zu) != sensor size(%zu), ignore setting", config.size(), setting.sensorSettings.size());
            }
        }

        if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_SENSOR_SCENARIO_REC))
        {
            const json &config = settingValue[CUSTOM_KEY_SENSOR_SCENARIO_REC];
            if(config.size() == setting.sensorSettings.size())
            {
                for(json::const_iterator it = config.begin(); it != config.end(); ++it)
                {
                    setting.sensorScenariosRecord.push_back(stringToSensorScenario(it.value().get<std::string>()));
                }
            }
            else
            {
                MY_LOGE("Record sensor scenario size(%zu) != sensor size(%zu), ignore setting", config.size(), setting.sensorSettings.size());
            }
        }

        __sensorCombinationSettings[logicalDeviceID] = std::move(setting);
        logicalDeviceID++;
    }

    //Get callback buffer list(optional)
    if(LogicalCamJSONUtil::HasMember(jsonObj, CUSTOM_KEY_CALLBACK_BUFFER_LIST)) {
        json &callbackSetting = jsonObj[CUSTOM_KEY_CALLBACK_BUFFER_LIST];
        if(LogicalCamJSONUtil::HasMember(callbackSetting, CUSTOM_KEY_VALUE)) {
            __callbackBufferListString = callbackSetting[CUSTOM_KEY_VALUE].get<std::string>();
        }
    }

    //Get depthmap format(optional, default is Y8)
    if(LogicalCamJSONUtil::HasMember(jsonObj, CUSTOM_KEY_DEPTHMAP_FORMAT)) {
        std::string formatStr = jsonObj[CUSTOM_KEY_DEPTHMAP_FORMAT].get<std::string>();
        if(formatStr == "Y16" ||
           formatStr == "y16")
        {
            __depthmapFormat = NSCam::eImgFmt_Y16;
        }
    }

    //Get Main2 output frequency
    if(LogicalCamJSONUtil::HasMember(jsonObj, CUSTOM_KEY_MAIN2_OUTPUT_FREQ)) {
        json &main2FreqSetting = jsonObj[CUSTOM_KEY_MAIN2_OUTPUT_FREQ];
        for(auto &s : main2FreqSetting.items()) {
            std::string key = s.key();
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            if(key.find("pre") == 0) {
                __main2OutputFrequency[0] = s.value().get<int>();
            } else if(key.find("cap") == 0) {
                __main2OutputFrequency[1] = s.value().get<int>();
            }
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
    for(auto &item : __sensorSettings) {
        auto &s = item.second;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        s.devIndex = sensorList->querySensorDevIdx(s.index);
        sensorList->querySensorStaticInfo(s.devIndex, &sensorStaticInfo);
        s.staticInfo = sensorStaticInfo;

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

                MY_LOGD("Convert diagonal FOV %f to H(%f) & V(%f), sensor size %dx%d",
                        s.fovDiagonal, s.fovHorizontal, s.fovVertical, rSensorCropInfo.full_w, rSensorCropInfo.full_h);
            }
            else
            {
                MY_LOGE("Cannot get sensor crop win info");
            }

            pIHalSensor->destroyInstance(LOG_TAG);
        }

        //sensorStaticInfo.horizontalViewAngle & verticalViewAngle is deprecated
        //If we want to get default FOV, need to calculate from focal length
    }
}

void
StereoSettingProviderKernel::__loadSettingsFromCalibration()
{
    AutoProfileUtil profile(LOG_TAG, "  Load calibration data");

    //Update calibration distance
    CAM_CAL_DATA_STRUCT calibrationData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MUINT32 queryResult;
    for(auto &item : __sensorSettings) {
        auto &s = item.second;
        if(0 == s.distanceMacro ||
           0 == s.distanceInfinite)
        {
            queryResult = pCamCalDrvObj->GetCamCalCalData(s.devIndex, CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&calibrationData);
            s.distanceMacro    = calibrationData.Single2A.S2aAF_t.AF_Macro_pattern_distance;    //unit: mm
            s.distanceInfinite = calibrationData.Single2A.S2aAF_t.AF_infinite_pattern_distance; //unit: mm
        }
    }
    pCamCalDrvObj->destroyInstance();
}

void
StereoSettingProviderKernel::logSettings()
{
    __logger
    .updateLogStatus()
    .FastLogD("=======================")
    .FastLogD("    Sensor Settings")
    .FastLogD("=======================");

    size_t size = __sensorCount;
    for(auto &item : __sensorSettings) {
        item.second.log(__logger);
        if(size-- > 1) {
            __logger.FastLogD("----------------------------------------------");
        }
    }

    __logger
    .FastLogD("=======================")
    .FastLogD("  Sensor Combinations")
    .FastLogD("=======================");
    for(auto &item : __sensorCombinationSettings) {
        item.second.log(__logger);
        __logger .FastLogD("----------------------------------------------");
    }

    __logger
    .FastLogD("Callback Buffer List: %s", __callbackBufferListString.c_str())
    .FastLogD("----------------------------------------------");

    __logger.print();
}

void
StereoSettingProviderKernel::__reset()
{
    __sensorSettings.clear();
    __sensorCombinationSettings.clear();
}
