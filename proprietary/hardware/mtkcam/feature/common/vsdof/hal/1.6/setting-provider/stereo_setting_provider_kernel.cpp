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

Mutex StereoSettingProviderKernel::__instanceLock;
StereoSettingProviderKernel * StereoSettingProviderKernel::__instance = NULL;

#define PERPERTY_SETTING_LOG          PROPERTY_ENABLE_LOG".setting"

#define PROPERTY_ENABLE_CUSTOM_SETTING  "vendor.STEREO.custom_setting"   //rea/write SETTING_FILE_PATH
#define PROPERTY_EXPORT_SETTING         "vendor.STEREO.custom_setting.exp" //write SETTING_HEADER_PATH

#define SETTING_FILE_PATH        "/sdcard/stereo_setting.json"
#define SETTING_HEADER_PATH      "/sdcard/camera_custom_stereo_setting.h"

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
    for(int i = __sensorSettings.size() - 1; i >= 0; --i) {
        if(sensorIndex == __sensorSettings[i].index) {
            return &__sensorSettings[i];
        }
    }

    MY_LOGE("Cannot get sensor setting of sensor %d", sensorIndex);
    return NULL;
}

StereoSensorConbinationSetting_T *
StereoSettingProviderKernel::getSensorCombinationSetting(SensorUID_T uidMain1, SensorUID_T uidMain2)
{
    if(1 == __sensorCombinationSettings.size()) {
        return &__sensorCombinationSettings[0];
    }

    for(int i = __sensorCombinationSettings.size() - 1; i >= 0; --i) {
        if((uidMain1 == __sensorCombinationSettings[i].uidMain1 &&
            uidMain2 == __sensorCombinationSettings[i].uidMain2) ||
           (uidMain2 == __sensorCombinationSettings[i].uidMain1 &&
            uidMain1 == __sensorCombinationSettings[i].uidMain2))
        {
            return &__sensorCombinationSettings[i];
        }
    }

    MY_LOGE("Sensor combination not found: 0x%X + 0x%X", uidMain1, uidMain2);
    return NULL;
}

StereoSettingProviderKernel::StereoSettingProviderKernel()
    : __logger(LOG_TAG, PERPERTY_SETTING_LOG)
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
    Mutex::Autolock lock(__instanceLock);

    AutoProfileUtil profile(LOG_TAG, "init");
    LOG_ENABLED = checkStereoProperty(PERPERTY_SETTING_LOG);

    const bool CUSTOM_SETTING_ENABLED = (1 == checkStereoProperty(PROPERTY_ENABLE_CUSTOM_SETTING));

    //Init
    struct stat st;
    Document loadDocument(kObjectType);
    bool useCustomSetting = true;
    if(CUSTOM_SETTING_ENABLED) {
        if(0 == stat(SETTING_FILE_PATH, &st))
        {
            if(__loadStereoSettingsFromDocument(loadDocument)) {
                useCustomSetting = false;
            } else {
                MY_LOGW("Cannot load setting from file, use default setting instead");
            }
        }
    }

    if(useCustomSetting) {
#if !defined(STEREO_NOT_SUPPORT)
        MY_LOGD_IF(LOG_ENABLED, "Parse setting from custom header");
        loadDocument.Parse(DEFAULT_STEREO_SETTING);
        if(loadDocument.HasParseError()) {
            MY_LOGE("Fail to parse build-in setting, error: %s (offset: %zu)", GetParseError_En(loadDocument.GetParseError()), loadDocument.GetErrorOffset());
        }
#endif
    }

    if(0 == __sensorSettings.size() ||      //First init
       !useCustomSetting)                   //Use stereo_setting.json
    {
        __reset();

        __parseDocument(loadDocument);
        __loadSettingsFromSensorHAL();
        __loadSettingsFromCalibration();
    }

    __logSettings();

    if(CUSTOM_SETTING_ENABLED &&
       (1 == checkStereoProperty(PROPERTY_EXPORT_SETTING) ||
        useCustomSetting))
    {
        __saveSettingToFile(loadDocument);
    }
}

void
StereoSettingProviderKernel::__saveSettingToFile(Document &document)
{
    Document::AllocatorType& allocator = document.GetAllocator();

    char fileName[PATH_MAX+1];
    //Save to json
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    // writer.SetFormatOptions(kFormatSingleLineArray); //LDC table will exceeds line buffer
    document.Accept(writer);
    char *data = (char *)sb.GetString();

    //Write to SETTING_FILE_PATH if not exist
    struct stat st;
    if(stat(SETTING_FILE_PATH, &st)) {
        FILE *fp = fopen(SETTING_FILE_PATH, "w");
        if(fp) {
            fwrite(data, 1, strlen(data), fp);

            fflush(fp);
            fclose(fp);
        }
    }

    if(1 == checkStereoProperty(PROPERTY_EXPORT_SETTING)) {
        FILE *fp = fopen(SETTING_HEADER_PATH, "w");
        if(fp) {
            //Write copy right
            const char *COPY_RIGHT =
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
                       line[i] != ' ')
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
            strncpy(lineBuffer, LAST_LINE, strlen(LAST_LINE));
            fwrite(lineBuffer, 1, strlen(lineBuffer), fp);

            fflush(fp);
            fclose(fp);
        }
    }
}

bool
StereoSettingProviderKernel::__loadStereoSettingsFromDocument(Document &document)
{
    AutoProfileUtil profile(LOG_TAG, "  Load settings from file");

    FILE *fp = fopen(SETTING_FILE_PATH, "r");
    if(fp) {
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        document.ParseStream(is);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s, err: %s", SETTING_FILE_PATH, strerror(errno));
    }

    return true;
}

void
StereoSettingProviderKernel::__parseDocument(Document &document)
{
    AutoProfileUtil profile(LOG_TAG, "  Parse docuemnt");

    //Get settings of sensors
    Value& sensorValues = document[CUSTOM_KEY_SENSORS];
    const int  FEATURE_COUNT = 5;
    const char *FEATURE_NAME[FEATURE_COUNT] = //Same order as StereoFeatureMode
    {
        CUSTOM_KEY_STEREO_CAPTURE,
        CUSTOM_KEY_VSDOF,
        CUSTOM_KEY_DENOISE,
        CUSTOM_KEY_3RD_Party,
        CUSTOM_KEY_ZOOM,
    };

    const int FEATURE_ID[FEATURE_COUNT] = //Same order as FEATURE_NAME
    {
        E_STEREO_FEATURE_CAPTURE,
        E_STEREO_FEATURE_VSDOF,
        E_STEREO_FEATURE_DENOISE,
        E_STEREO_FEATURE_THIRD_PARTY,
        E_DUALCAM_FEATURE_ZOOM,
    };

    for(SizeType i = 0; i < sensorValues.Size(); i++) {
        const Value &settingValue = sensorValues[i];
        StereoSensorSetting_T setting;
        //Get Name
        const char *uidString = settingValue[CUSTOM_KEY_UID].GetString();
        sscanf(uidString, "0x%X", &setting.uid);
        setting.name = settingValue[CUSTOM_KEY_NAME].GetString();

        //Get Capture size
        const Value &capSizeValue = settingValue[CUSTOM_KEY_CAPTURE_SIZE];
        const Value &capSize_16_9_Value = capSizeValue[CUSTOM_KEY_RATIO_16_9];
        const char *capSizeString;
        MSize size;
        for (Value::ConstValueIterator it = capSize_16_9_Value.Begin(); it != capSize_16_9_Value.End(); ++it) {
            capSizeString = it->GetString();
            sscanf(capSizeString, "%dx%d", &size.w, &size.h);
            setting.captureSizes_16_9.push_back(size);
        }

        const Value &capSize_4_3_Value = capSizeValue[CUSTOM_KEY_RATIO_4_3];
        for (Value::ConstValueIterator it = capSize_4_3_Value.Begin(); it != capSize_4_3_Value.End(); ++it) {
            capSizeString = it->GetString();
            sscanf(capSizeString, "%dx%d", &size.w, &size.h);
            setting.captureSizes_4_3.push_back(size);
        }
        //Get cusomized IMGO YUV size
        if(settingValue.HasMember(CUSTOM_KEY_IMGOYUV_SIZE)) {
            const Value &imgoYuvSizeValue = settingValue[CUSTOM_KEY_IMGOYUV_SIZE];
            const char *imgoYuvSizeString = imgoYuvSizeValue[CUSTOM_KEY_RATIO_16_9].GetString();
            if(imgoYuvSizeString) {
                sscanf(imgoYuvSizeString, "%dx%d", &setting.imgoYuvSize_16_9.w, &setting.imgoYuvSize_16_9.h);
            }

            imgoYuvSizeString = imgoYuvSizeValue[CUSTOM_KEY_RATIO_4_3].GetString();
            if(imgoYuvSizeString) {
                sscanf(imgoYuvSizeString, "%dx%d", &setting.imgoYuvSize_4_3.w, &setting.imgoYuvSize_4_3.h);
            }
        }
        //Get cusomized RRZO YUV size
        if(settingValue.HasMember(CUSTOM_KEY_RRZOYUV_SIZE)) {
            const Value &rrzoYuvSizeValue = settingValue[CUSTOM_KEY_RRZOYUV_SIZE];
            const char *rrzoYuvSizeString = rrzoYuvSizeValue[CUSTOM_KEY_RATIO_16_9].GetString();
            if(rrzoYuvSizeString) {
                sscanf(rrzoYuvSizeString, "%dx%d", &setting.rrzoYuvSize_16_9.w, &setting.rrzoYuvSize_16_9.h);
            }

            rrzoYuvSizeString = rrzoYuvSizeValue[CUSTOM_KEY_RATIO_4_3].GetString();
            if(rrzoYuvSizeString) {
                sscanf(rrzoYuvSizeString, "%dx%d", &setting.rrzoYuvSize_4_3.w, &setting.rrzoYuvSize_4_3.h);
            }
        }
        //Get sensor scenario
        const Value &sensorScenarioZSDValue = settingValue[CUSTOM_KEY_SENSOR_SCENARIO_ZSD];
        for(int fi = 0; fi < FEATURE_COUNT; ++fi) {
            if(sensorScenarioZSDValue.HasMember(FEATURE_NAME[fi])) {
                setting.sensorScenarioMapZSD[FEATURE_ID[fi]] = __parseSensorScenario(sensorScenarioZSDValue[FEATURE_NAME[fi]].GetString());
            } else {
                setting.sensorScenarioMapZSD[FEATURE_ID[fi]] = SENSOR_SCENARIO_ID_UNNAMED_START;
            }
        }
        const Value &sensorScenarioRecordValue = settingValue[CUSTOM_KEY_SENSOR_SCENARIO_REC];
        for(int fi = 0; fi < FEATURE_COUNT; ++fi) {
            if(sensorScenarioRecordValue.HasMember(FEATURE_NAME[fi])) {
                setting.sensorScenarioMapRecord[FEATURE_ID[fi]] = __parseSensorScenario(sensorScenarioRecordValue[FEATURE_NAME[fi]].GetString());
            } else {
                setting.sensorScenarioMapRecord[FEATURE_ID[fi]] = SENSOR_SCENARIO_ID_UNNAMED_START;
            }
        }

        //Get FOV
        if(settingValue.HasMember(CUSTOM_KEY_FOV)) {
            const Value &fovValue = settingValue[CUSTOM_KEY_FOV];
            setting.fovHorizontal = fovValue[CUSTOM_KEY_FOV_H].GetFloat();
            setting.fovVertical   = fovValue[CUSTOM_KEY_FOV_V].GetFloat();
        }

        //Get calibration data
        if(settingValue.HasMember(CUSTOM_KEY_CALIBRATION)) {
            const Value &calibrationValue = settingValue[CUSTOM_KEY_CALIBRATION];
            setting.distanceMacro    = calibrationValue[CUSTOM_KEY_MACRO_DISTANCE].GetInt();
            setting.distanceInfinite = calibrationValue[CUSTOM_KEY_INFINITE_DISTANCE].GetInt();
        }

        __sensorSettings.push_back(setting);
    }

    //Get settings of sensor combinations
    Value& sensorCombValues = document[CUSTOM_KEY_SENSOR_COMBINATIONS];
    for(SizeType ci = 0; ci < sensorCombValues.Size(); ci++) {
        const Value &settingValue = sensorCombValues[ci];
        StereoSensorConbinationSetting_T setting;
        float fov[2];

        //uidMain1
        const char *main1Name = settingValue[CUSTOM_KEY_MAIN1_NAME].GetString();
        size_t nameSize = (NULL != main1Name) ? strlen(main1Name) : 0;
        bool isSettingFound = false;
        if(nameSize > 0) {
            isSettingFound = false;
            for(auto &s : __sensorSettings) {
                if(!strncmp(main1Name, s.name.c_str(), nameSize)) {
                    fov[0] = s.fovHorizontal;
                    setting.uidMain1 = s.uid;
                    isSettingFound = true;
                    break;
                }
            }

            if(!isSettingFound) {
                MY_LOGE("Main1 %s not found in sensor settings", main1Name);
            }
        } else {
            MY_LOGE("Sensor combination error: Main1 name is not set");
        }

        //uidMain2
        if(isSettingFound) {
            const char *main2Name = settingValue[CUSTOM_KEY_MAIN2_NAME].GetString();
            nameSize = (NULL != main2Name) ? strlen(main2Name) : 0;
            if(nameSize > 0) {
                isSettingFound = false;
                for(auto &s : __sensorSettings) {
                    if(!strncmp(main2Name, s.name.c_str(), nameSize)) {
                        fov[1] = s.fovHorizontal;
                        setting.uidMain2 = s.uid;
                        isSettingFound = true;
                        break;
                    }
                }

                if(!isSettingFound) {
                    MY_LOGE("Main2 %s not found in sensor settings", main2Name);
                }
            } else {
                MY_LOGE("Sensor combination error: Main1 name is not set");
            }
        }

        if(isSettingFound)
        {
            setting.moduleType = settingValue[CUSTOM_KEY_MODULE_TYPE].GetInt();
            setting.baseline = settingValue[CUSTOM_KEY_BASELINE].GetFloat();
            setting.enableLDC = (fabs(fov[0] - fov[1]) > 0.01f);

            //Get FOV crop(optional)
            if(settingValue.HasMember(CUSTOM_KEY_FOV_CROP)) {
                const Value &fovCropValue = settingValue[CUSTOM_KEY_FOV_CROP];
                float cropDegree = fovCropValue[CUSTOM_KEY_DEGREE].GetFloat();
                float cropRatio = fovCropValue[CUSTOM_KEY_RATIO].GetFloat();
#if !defined(STEREO_NOT_SUPPORT)
                setting.fovCrop = CUST_FOV_CROP_T(cropDegree, cropRatio);
#endif
            }
            //Get cusomized depthmap size(optional)
            if(settingValue.HasMember(CUSTOM_KEY_DEPTHMAP_SIZE)) {
                const Value &depthSizeValue = settingValue[CUSTOM_KEY_DEPTHMAP_SIZE];
                const char *depthSizeString = depthSizeValue[CUSTOM_KEY_RATIO_16_9].GetString();
                if(depthSizeString) {
                    sscanf(depthSizeString, "%dx%d", &setting.depthmapSize_16_9.w, &setting.depthmapSize_16_9.h);
                }

                depthSizeString = depthSizeValue[CUSTOM_KEY_RATIO_4_3].GetString();
                if(depthSizeString) {
                    sscanf(depthSizeString, "%dx%d", &setting.depthmapSize_4_3.w, &setting.depthmapSize_4_3.h);
                }
            }
            __parseLDC(settingValue[CUSTOM_KEY_LDC], setting.LDC);

            __sensorCombinationSettings.push_back(setting);
        }
    }
}

void
StereoSettingProviderKernel::__parseLDC(const Value &ldcValues, vector<float> &ldc)
{
    AutoProfileUtil profile(LOG_TAG, "    Parse LDC");

    stringstream ss;
    float number;
    for(SizeType i = 0; i < ldcValues.Size(); i++) {
        const Value &ldcLine = ldcValues[i];
        char *start = (char *)ldcLine.GetString();
        if(start) {
            char *end = NULL;
            do {
                number = ::strtof(start, &end);
                if  ( start == end ) {
                    MY_LOGD("No LDC data: %s", start);
                    break;
                }
                ldc.push_back(number);
                start = end + 1;
            } while ( end && *end );
        }
    }
}

MUINT
StereoSettingProviderKernel::__parseSensorScenario(const char *scenario)
{
    string s(scenario);
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
    int sensorDevIndex;
    int32_t sensorCount = sensorList->queryNumberOfSensors();
    for(int index = 0; index < sensorCount; index++) {
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        sensorDevIndex = sensorList->querySensorDevIdx(index);
        sensorList->querySensorStaticInfo(sensorDevIndex, &sensorStaticInfo);

        bool isSensorFound = false;
        for(auto &s : __sensorSettings) {
            if(sensorStaticInfo.sensorDevID == s.uid) {
                s.index    = index;
                s.devIndex = sensorDevIndex;

                if(0.0f == s.fovHorizontal) {
                    s.fovHorizontal = sensorStaticInfo.horizontalViewAngle;
                }

                if(0.0f == s.fovVertical) {
                    s.fovVertical = sensorStaticInfo.verticalViewAngle;
                }

                s.staticInfo  = sensorStaticInfo;
                isSensorFound = true;
                MY_LOGD_IF(LOG_ENABLED, "Set index %d to sensor %s", s.index, s.name.c_str());
                break;
            }
        }

        if(!isSensorFound) {
            MY_LOGD("Sensor 0x%X(%d) not found in sensor settings", sensorStaticInfo.sensorDevID, index);
        }
    }

    //Remove unused sensor setting and combinations
    __sensorSettings.erase(std::remove_if(__sensorSettings.begin(), __sensorSettings.end(),
                           [](auto &s) { return s.index < 0; }),
                           __sensorSettings.end());
#if !defined(STEREO_NOT_SUPPORT)
    __sensorCombinationSettings.erase(std::remove_if(__sensorCombinationSettings.begin(), __sensorCombinationSettings.end(),
                            [&](auto &c) {
                                bool isMain1SensorFound = false;
                                bool isMain2SensorFound = false;
                                for(auto &s : __sensorSettings) {
                                    if(s.uid == c.uidMain1) {
                                        isMain1SensorFound = true;
                                    } else if(s.uid == c.uidMain2) {
                                        isMain2SensorFound = true;
                                    }
                                }

                                return !(isMain1SensorFound && isMain2SensorFound);
                            }),
                            __sensorCombinationSettings.end());
#endif
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
    for(auto &s : __sensorSettings) {
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
StereoSettingProviderKernel::__logSettings()
{
    __logger
    .FastLogD("=======================")
    .FastLogD("    Sensor Settings")
    .FastLogD("=======================");

    size_t size = __sensorSettings.size();
    for(auto &s : __sensorSettings) {
        s.log(__logger);

        if(size-- > 1) {
            __logger .FastLogD("-----------------------");
        }
    }

    __logger
    .FastLogD("=======================")
    .FastLogD("  Sensor Combinations")
    .FastLogD("=======================");
    for(auto &c : __sensorCombinationSettings) {
        c.log(__logger);
        __logger .FastLogD("-----------------------");
    }

    __logger.print();
}

void
StereoSettingProviderKernel::__reset()
{
    __sensorSettings.clear();
    __sensorCombinationSettings.clear();
}
