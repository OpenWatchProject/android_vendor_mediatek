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

using namespace StereoHAL;

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

#define SENSOR_DRVNAME_PREFIX std::string("SENSOR_DRVNAME_")

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

    LOG_ENABLED = checkStereoProperty(PERPERTY_SETTING_LOG);

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

    const bool CUSTOM_SETTING_ENABLED = (1 == checkStereoProperty(PROPERTY_ENABLE_CUSTOM_SETTING));

    //Init
    struct stat st;
    Document loadDocument(kObjectType);
    bool useCustomSetting = true;
    if(CUSTOM_SETTING_ENABLED &&
       0 == stat(SETTING_FILE_PATH, &st))
    {
        if(__loadStereoSettingsFromDocument(loadDocument)) {
            useCustomSetting = false;
        } else {
            MY_LOGW("Cannot load setting from file, use default setting instead");
        }
    }

    if(useCustomSetting) {
        MY_LOGD_IF(LOG_ENABLED, "Parse setting from custom header");
        loadDocument.Parse(DEFAULT_STEREO_SETTING);
        if(loadDocument.HasParseError()) {
            MY_LOGE("Fail to parse build-in setting, error: %s (offset: %zu)", GetParseError_En(loadDocument.GetParseError()), loadDocument.GetErrorOffset());
        }
    }

    if(0 == __sensorCombinationSettings.size() ||   //First init
       !useCustomSetting ||                         //Use stereo_setting.json
       loadDocument != __document)                  //Setting changes
    {
        __reset();

        __parseDocument(loadDocument);
        __loadSettingsFromSensorHAL();
        __loadSettingsFromCalibration();

        __document.CopyFrom(loadDocument, __document.GetAllocator());
    }

    // logSettings();

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
            size_t LAST_LINE_SIZE = strlen(LAST_LINE);
            ::memcpy(lineBuffer, LAST_LINE, LAST_LINE_SIZE);
            fwrite(lineBuffer, 1, LAST_LINE_SIZE, fp);

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
    const int  FEATURE_COUNT = 7;
    const char *FEATURE_NAME[FEATURE_COUNT] = //Same order as StereoFeatureMode
    {
        CUSTOM_KEY_STEREO_CAPTURE,
        CUSTOM_KEY_VSDOF,
        CUSTOM_KEY_DENOISE,
        CUSTOM_KEY_3RD_Party,
        CUSTOM_KEY_ZOOM,
        CUSTOM_KEY_MTK_DEPTHMAP,
        CUSTOM_KEY_ACTIVE_STEREO,
    };

    const int FEATURE_ID[FEATURE_COUNT] = //Same order as FEATURE_NAME
    {
        E_STEREO_FEATURE_CAPTURE,
        E_STEREO_FEATURE_VSDOF,
        E_STEREO_FEATURE_DENOISE,
        E_STEREO_FEATURE_THIRD_PARTY,
        E_DUALCAM_FEATURE_ZOOM,
        E_STEREO_FEATURE_MTK_DEPTHMAP,
        E_STEREO_FEATURE_ACTIVE_STEREO,
    };

    const char *strRatio = NULL;
    const char *strSize = NULL;
    uint32_t m, n;
    MSize size;
    ENUM_STEREO_RATIO ratio;
    for(SizeType i = 0; i < sensorValues.Size(); i++) {
        const Value &settingValue = sensorValues[i];
        StereoSensorSetting_T setting;
        //Get Name(MUST)
        //  Name must copy from kernel-4.9\drivers\misc\mediatek\imgsensor\inc\kd_imgsensor.h,
        //  definition of SENSOR_DRVNAME_<sensor name>
        std::string name = settingValue[CUSTOM_KEY_NAME].GetString();
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        setting.name = name;

        if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
            name = SENSOR_DRVNAME_PREFIX + name;
        }

        if(__sensorNameMap.find(name) == __sensorNameMap.end()) {
            MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Setting]Sensor %s is not installed on this device",
                       settingValue[CUSTOM_KEY_NAME].GetString());
            continue;
        }

        setting.index = __sensorNameMap[name];
        MY_LOGD_IF(LOG_ENABLED, "Set index %d to sensor %s", setting.index, setting.name.c_str());

        //Get Capture size(MUST for main cam)
        if(settingValue.HasMember(CUSTOM_KEY_CAPTURE_SIZE)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_CAPTURE_SIZE];

            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                const Value &sizeValue = it->value;
                vector<MSize> sizes;

                for (Value::ConstValueIterator it = sizeValue.Begin(); it != sizeValue.End(); ++it) {
                    strSize = it->GetString();
                    sscanf(strSize, "%dx%d", &size.w, &size.h);
                    sizes.push_back(size);
                }
                setting.captureSizes[ratio] = sizes;

                if(sizes.size() > 0) {
                    setting.supportedImageRatios[ratio] = true;
                }
            }
        }

        //Get cusomized IMGO YUV size(optional, used by pure 3rd party flow)
        if(settingValue.HasMember(CUSTOM_KEY_IMGOYUV_SIZE)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_IMGOYUV_SIZE];

            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                strSize = it->value.GetString();
                sscanf(strSize, "%dx%d", &size.w, &size.h);
                setting.imgoYuvSize[ratio] = size;
            }
        }

        //Get cusomized RRZO YUV size(optional, used by pure 3rd party flow)
        if(settingValue.HasMember(CUSTOM_KEY_RRZOYUV_SIZE)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_RRZOYUV_SIZE];

            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                strSize = it->value.GetString();
                sscanf(strSize, "%dx%d", &size.w, &size.h);
                setting.rrzoYuvSize[ratio] = size;
            }
        }

        //Get cusomized PostView size(optional)
        if(settingValue.HasMember(CUSTOM_KEY_POSTVIEW_SIZE)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_POSTVIEW_SIZE];

            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                strSize = it->value.GetString();
                sscanf(strSize, "%dx%d", &size.w, &size.h);
                setting.postviewSize[ratio] = size;
            }
        }

        //Get cusomized IR RRZO YUV size(optional, used by active stereo)
        if(settingValue.HasMember(CUSTOM_KEY_IRRRZO_SIZE)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_IRRRZO_SIZE];

            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                strSize = it->value.GetString();
                sscanf(strSize, "%dx%d", &size.w, &size.h);
                setting.irRrzoSize[ratio] = size;
            }
        }

        //Get sensor scenario of ZSD mode(optional)
        if(settingValue.HasMember(CUSTOM_KEY_SENSOR_SCENARIO_ZSD)) {
            const Value &sensorScenarioZSDValue = settingValue[CUSTOM_KEY_SENSOR_SCENARIO_ZSD];
            for(int fi = 0; fi < FEATURE_COUNT; ++fi) {
                if(sensorScenarioZSDValue.HasMember(FEATURE_NAME[fi])) {
                    setting.sensorScenarioMapZSD[FEATURE_ID[fi]] = __parseSensorScenario(sensorScenarioZSDValue[FEATURE_NAME[fi]].GetString());
                } else {
                    setting.sensorScenarioMapZSD[FEATURE_ID[fi]] = SENSOR_SCENARIO_ID_UNNAMED_START;
                }
            }
        }

        //Get sensor scenario of Recording mode(optional)
        if(settingValue.HasMember(CUSTOM_KEY_SENSOR_SCENARIO_REC)) {
            const Value &sensorScenarioRecordValue = settingValue[CUSTOM_KEY_SENSOR_SCENARIO_REC];
            for(int fi = 0; fi < FEATURE_COUNT; ++fi) {
                if(sensorScenarioRecordValue.HasMember(FEATURE_NAME[fi])) {
                    setting.sensorScenarioMapRecord[FEATURE_ID[fi]] = __parseSensorScenario(sensorScenarioRecordValue[FEATURE_NAME[fi]].GetString());
                } else {
                    setting.sensorScenarioMapRecord[FEATURE_ID[fi]] = SENSOR_SCENARIO_ID_UNNAMED_START;
                }
            }
        }

        //Get FOV(optional, use static info(integer) if not set)
        if(settingValue.HasMember(CUSTOM_KEY_FOV)) {
            const Value &fovValue = settingValue[CUSTOM_KEY_FOV];
            setting.fovHorizontal = fovValue[CUSTOM_KEY_FOV_H].GetFloat();
            setting.fovVertical   = fovValue[CUSTOM_KEY_FOV_V].GetFloat();
        }

        //Get calibration data(optional)
        if(settingValue.HasMember(CUSTOM_KEY_CALIBRATION)) {
            const Value &calibrationValue = settingValue[CUSTOM_KEY_CALIBRATION];
            setting.distanceMacro    = calibrationValue[CUSTOM_KEY_MACRO_DISTANCE].GetInt();
            setting.distanceInfinite = calibrationValue[CUSTOM_KEY_INFINITE_DISTANCE].GetInt();
        }

        __sensorSettings[name] = std::move(setting);
    }

    //Get settings of sensor combinations
    Value& sensorCombValues = document[CUSTOM_KEY_SENSOR_COMBINATIONS];
    for(SizeType ci = 0; ci < sensorCombValues.Size(); ci++) {
        bool hasError = false;
        const Value &settingValue = sensorCombValues[ci];
        StereoSensorConbinationSetting_T setting;

        std::vector<std::string> names;
        if(settingValue.HasMember(CUSTOM_KEY_SENSORS))
        {
            //Use "Sensors"
            for(SizeType si = 0; si < settingValue[CUSTOM_KEY_SENSORS].Size(); si++) {
                const char *name = settingValue[CUSTOM_KEY_SENSORS][si].GetString();
                if(name) {
                    names.push_back(std::string(name));
                }
            }
        }
        else if(settingValue.HasMember(CUSTOM_KEY_MAIN1_NAME) &&
                settingValue.HasMember(CUSTOM_KEY_MAIN2_NAME))
        {
            //uidMain1
            names.push_back(std::string(settingValue[CUSTOM_KEY_MAIN1_NAME].GetString()));
            names.push_back(std::string(settingValue[CUSTOM_KEY_MAIN2_NAME].GetString()));
        }

        std::map<std::string, StereoSensorSetting_T>::iterator sensorIt;
        for(auto &name : names) {
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);
            if(std::string::npos == name.find(SENSOR_DRVNAME_PREFIX)) {
                name = SENSOR_DRVNAME_PREFIX + name;
            }

            sensorIt = __sensorSettings.find(name);
            if(__sensorNameMap.find(name) == __sensorNameMap.end() ||
               sensorIt == __sensorSettings.end())
            {
                MY_LOGD_IF(LOG_ENABLED, "[Parse Sensor Combination]Sensor %s is not installed on this device",
                           name.c_str());
                hasError = true;
                break;
            }

            setting.sensorSettings.push_back(&sensorIt->second);
        }

        if(hasError) {
            continue;
        }

        setting.profile = STEREO_SENSOR_PROFILE_REAR_FRONT;
        if(setting.sensorSettings[0]->staticInfo.facingDirection == 0) {
            if(setting.sensorSettings[1]->staticInfo.facingDirection == 0) {
                setting.profile = STEREO_SENSOR_PROFILE_REAR_REAR;
            }
        } else {
            if(setting.sensorSettings[1]->staticInfo.facingDirection == 1) {
                setting.profile = STEREO_SENSOR_PROFILE_FRONT_FRONT;
            }
        }

        //Get module type(MUST)
        setting.moduleType = settingValue[CUSTOM_KEY_MODULE_TYPE].GetInt();

        //Get Baseline(optional for pure 3rd party)
        if(settingValue.HasMember(CUSTOM_KEY_BASELINE)) {
            setting.baseline = settingValue[CUSTOM_KEY_BASELINE].GetFloat();
        }

        //Get module variation(optional)
        if(settingValue.HasMember(CUSTOM_KEY_MODULE_VARIATION)) {
            setting.moduleVariation = settingValue[CUSTOM_KEY_MODULE_VARIATION].GetFloat();
            if(setting.moduleVariation < 0.0f) {
                setting.moduleVariation = 0.0f;
            }
        }

        //Get working range(optional, should adjust for W+T VSDoF)
        if(settingValue.HasMember(CUSTOM_KEY_WORKING_RANGE)) {
            setting.workingRange = settingValue[CUSTOM_KEY_WORKING_RANGE].GetFloat();
            if(setting.workingRange <= 0) {
                setting.workingRange = DEFAULT_WORKING_RANGE;
            }
        }

        //Get FOV crop(optional)
        if(settingValue.HasMember(CUSTOM_KEY_FOV_CROP)) {
            const Value &fovCropValue = settingValue[CUSTOM_KEY_FOV_CROP];
            if(fovCropValue.HasMember(CUSTOM_KEY_CENTER_CROP)) {
                setting.isCenterCrop = !!(fovCropValue[CUSTOM_KEY_CENTER_CROP].GetInt());
            }

            if(fovCropValue.HasMember(CUSTOM_KEY_DISABLE_CROP)) {
                setting.disableCrop = !!(fovCropValue[CUSTOM_KEY_DISABLE_CROP].GetInt());
            }
        }

        //Get customized depthmap size(optional)
        if(settingValue.HasMember(CUSTOM_KEY_DEPTHMAP_SIZE)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_DEPTHMAP_SIZE];

            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                strSize = it->value.GetString();
                sscanf(strSize, "%dx%d", &size.w, &size.h);
                setting.depthmapSize[ratio] = size;
            }
        }

        //Get LDC(optional)
        if(settingValue.HasMember(CUSTOM_KEY_LDC)) {
            setting.LDC.clear();
            __parseLDC(settingValue[CUSTOM_KEY_LDC], setting.LDC);
            setting.enableLDC = (setting.LDC.size() > 0);
        }

        //Get offline calibration(optional)
        if(settingValue.HasMember(CUSTOM_KEY_CALIBRATION)) {
            setting.calibrationData.clear();
            __parseCalibration(settingValue[CUSTOM_KEY_CALIBRATION], setting.calibrationData);
        }

        //Get customized base size(optional)
        if(settingValue.HasMember(CUSTOM_KEY_SIZE_CONFIG)) {
            const Value &sizeValue = settingValue[CUSTOM_KEY_SIZE_CONFIG];

            StereoArea area;
            MSize contentSize;
            for (Value::ConstMemberIterator it = sizeValue.MemberBegin(); it != sizeValue.MemberEnd(); ++it)
            {
                strRatio = it->name.GetString();
                sscanf(strRatio, "%d:%d", &m, &n);
                ratio = MToNToImageRatio(m, n);
                const Value &sizeValue = it->value;

                StereoArea area;
                strSize = sizeValue[CUSTOM_KEY_PADDING].GetString();
                sscanf(strSize, "%dx%d", &area.padding.w, &area.padding.h);
                area.startPt.x = area.padding.w/2;
                area.startPt.y = area.padding.h/2;

                strSize = sizeValue[CUSTOM_KEY_CONTENT_SIZE].GetString();
                sscanf(strSize, "%dx%d", &contentSize.w, &contentSize.h);
                if(0 != contentSize.w % 16) {
                    MSize newSize = contentSize;
                    //Width must be 16-align
                    applyNAlign(16, newSize.w);

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

                area.size = area.padding + contentSize;
                setting.baseSize[ratio] = area;

                setting.hasSizeConfig = true;
            }
        }

        __sensorCombinationSettings.push_back(setting);
    }

    //Get callback buffer list(optional)
    if(document.HasMember(CUSTOM_KEY_CALLBACK_BUFFER_LIST)) {
        const Value &callbackSetting = document[CUSTOM_KEY_CALLBACK_BUFFER_LIST];
        if(callbackSetting.HasMember(CUSTOM_KEY_VALUE)) {
            __callbackBufferListString = std::string(callbackSetting[CUSTOM_KEY_VALUE].GetString());
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
                    // MY_LOGD("No LDC data: %s", start);
                    break;
                }
                ldc.push_back(number);
                start = end + 1;
            } while ( end && *end );
        }
    }
}

void base64Decode(const char *base64Str, vector<MUINT8> &buffer)
{
    size_t base64_len = strlen(base64Str);
    size_t out_len = base64_len/4 * 3;
    for(size_t i = base64_len-1; i >= 0; --i) {
        if('=' == base64Str[i]) {
            out_len--;
            base64_len--;
        } else {
            break;
        }
    }
    MY_LOGD("Base64 size %zu, out size %zu", base64_len, out_len);
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

void
StereoSettingProviderKernel::__parseCalibration(const Value &calValue, vector<MUINT8> &calibration)
{
    AutoProfileUtil profile(LOG_TAG, "  Parse offline calibration data");
    base64Decode(calValue.GetString(), calibration);
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