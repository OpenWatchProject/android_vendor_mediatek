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

//Notice:
// You must add "LOCAL_CPPFLAGS += -fexceptions" to your Android.mk for exception handling
#ifndef LOGICALCAM_JSON_UTIL_H_
#define LOGICALCAM_JSON_UTIL_H_

#include <sstream>  //For ostringstream
#include <mtkcam/utils/json/json.hpp>
#include <mtkcam/utils/json/fifo_map.hpp>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_stereo.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <mtkcam/drv/IHalSensor.h>
#pragma GCC diagnostic pop
#include <mtkcam/utils/LogicalCam/Type.h>
#include <mtkcam/utils/LogicalCam/LogicalCamJSONKeys.h>
#include <cutils/properties.h>

#ifndef CAM_ULOG_MODULE_ID
#define CAM_ULOGMV(fmt, arg...)        ALOGV(fmt, ##arg)
#define CAM_ULOGMD(fmt, arg...)        ALOGD(fmt, ##arg)
#define CAM_ULOGMI(fmt, arg...)        ALOGI(fmt, ##arg)
#define CAM_ULOGMW(fmt, arg...)        ALOGW(fmt, ##arg)
#define CAM_ULOGME(fmt, arg...)        ALOGE(fmt, ##arg)
#endif

using namespace nlohmann;
// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
template<class K, class V, class dummy_compare, class A>
using KeepOrderJsonMap = fifo_map<K, V, fifo_map_compare<K>, A>;
using KeepOrderJSON = basic_json<KeepOrderJsonMap>;

#define STEREO_SETTING_FILE_PATH   "/sdcard/stereo_setting.json"
#define STEREO_TUNING_FILE_PATH    "/sdcard/stereo_tuning.json"
#define STEREO_CALIBRATION_FILE_PATH "/sdcard/calibration.json"

#define SENSOR_DRVNAME_PREFIX std::string("SENSOR_DRVNAME_")
#define DELIMITER std::string(":")

namespace NSCam {

enum ENUM_JSON_PARSE_STATUS
{
    E_JSON_PARSE_FAILED        = 0,
    E_JSON_PARSE_SUCCEED       = 1,
    E_JSON_PARSE_FROM_DEFAULT  = 1<<1,
    E_JSON_PARSE_FROM_EXTERNAL = 1<<2,
};

#define IS_JSON_FROM_EXTERNAL(status) (status & E_JSON_PARSE_FROM_EXTERNAL)
#define IS_JSON_FROM_DEFAULT(status) (status & E_JSON_PARSE_FROM_DEFAULT)

class LogicalCamJSONUtil
{
public:
    static bool HasMember(const json &obj, const char *key)
    {
        return (obj.find(key) != obj.end());
    }

    static bool HasMember(json &obj, const char *key)
    {
        return (obj.find(key) != obj.end());
    }

    static bool HasMember(const json &obj, std::string &key)
    {
        return (obj.find(key) != obj.end());
    }

    static bool HasMember(json &obj, std::string &key)
    {
        return (obj.find(key) != obj.end());
    }

    template<class Json_T>
    static int parseLogicalCamCustomSetting(Json_T &output)
    {
        //Init
        int result = E_JSON_PARSE_SUCCEED;
        struct stat st;
        bool useCustomSetting = true;
        if(0 == stat(STEREO_SETTING_FILE_PATH, &st)) {
            std::ifstream fin(STEREO_SETTING_FILE_PATH);
            if(fin) {
                try
                {
                    output = KeepOrderJSON::parse(fin, nullptr, false);
                    result = E_JSON_PARSE_SUCCEED|E_JSON_PARSE_FROM_EXTERNAL;
                    useCustomSetting = false;
                }
                catch (json::exception& e)
                {
                    CAM_ULOGMW("Parse error: %s", e.what());
                }

                if(!output.is_discarded()) {
                    useCustomSetting = false;
                }
            }

            if(useCustomSetting) {
                CAM_ULOGMW("Cannot load setting from file, use default setting instead");
            }
        }

        if(useCustomSetting) {
            CAM_ULOGMD("Parse DEFAULT_STEREO_SETTING in camera_custom_stereo_setting.h");
            try
            {
                output = Json_T::parse(DEFAULT_STEREO_SETTING);
                result = E_JSON_PARSE_SUCCEED|E_JSON_PARSE_FROM_DEFAULT;
            }
            catch (json::exception& e)
            {
                CAM_ULOGMW("Parse error: %s", e.what());
                result = E_JSON_PARSE_FAILED;
            }

            if(output.is_discarded()) {
                result = E_JSON_PARSE_FAILED;
            }
        }

        return result;
    }

    template<class Json_T>
    static int parseStereoCustomTuning(Json_T &output)
    {
        //Init
        int result = E_JSON_PARSE_SUCCEED;
        struct stat st;
        bool useCustomTuning = true;
        if(0 == stat(STEREO_TUNING_FILE_PATH, &st)) {
            std::ifstream fin(STEREO_TUNING_FILE_PATH);
            if(fin) {
                try
                {
                    output = KeepOrderJSON::parse(fin, nullptr, false);
                    result = E_JSON_PARSE_SUCCEED|E_JSON_PARSE_FROM_EXTERNAL;
                    useCustomTuning = false;
                }
                catch (json::exception& e)
                {
                    CAM_ULOGMW("Parse error: %s", e.what());
                }

                if(!output.is_discarded()) {
                    useCustomTuning = false;
                }
            }

            if(useCustomTuning) {
                CAM_ULOGMW("Cannot load tuning from file, use default tuning instead");
            }
        }

        if(useCustomTuning) {
            CAM_ULOGMD("Parse DEFAULT_STEREO_TUNING in camera_custom_stereo_tuning.h");
            try
            {
                output = Json_T::parse(DEFAULT_STEREO_TUNING);
                result = E_JSON_PARSE_SUCCEED|E_JSON_PARSE_FROM_DEFAULT;
            }
            catch (json::exception& e)
            {
                CAM_ULOGMW("Parse error: %s", e.what());
                result = E_JSON_PARSE_FAILED;
            }

            if(output.is_discarded()) {
                result = E_JSON_PARSE_FAILED;
            }
        }

        return result;
    }

    template<class Json_T>
    static int parseStereoCalibration(Json_T &output)
    {
        //Init
        int result = E_JSON_PARSE_FAILED;
        struct stat st;
        bool useCustomCalibation = true;
        if(0 == stat(STEREO_CALIBRATION_FILE_PATH, &st)) {
            std::ifstream fin(STEREO_CALIBRATION_FILE_PATH);
            if(fin) {
                try
                {
                    output = KeepOrderJSON::parse(fin, nullptr, false);
                    result = E_JSON_PARSE_SUCCEED|E_JSON_PARSE_FROM_EXTERNAL;
                    useCustomCalibation = false;
                }
                catch (json::exception& e)
                {
                    CAM_ULOGMW("Parse error: %s", e.what());
                }

                if(!output.is_discarded()) {
                    useCustomCalibation = false;
                }
            }

            // if(useCustomCalibation) {
            //     CAM_ULOGMW("Cannot load calibration from %s, use default tuning instead", STEREO_CALIBRATION_FILE_PATH);
            // }
        }

        // if(useCustomCalibation) {
        //     CAM_ULOGMD("Parse DEFAULT_STEREO_CALBIRATION in camera_custom_stereo_calibration.h");
        //     try
        //     {
        //         output = Json_T::parse(DEFAULT_STEREO_CALIBRATION);
        //         result = E_JSON_PARSE_SUCCEED|E_JSON_PARSE_FROM_DEFAULT;
        //     }
        //     catch (json::exception& e)
        //     {
        //         CAM_ULOGMW("Parse error: %s", e.what());
        //         result = E_JSON_PARSE_FAILED;
        //     }

        //     if(output.is_discarded()) {
        //         result = E_JSON_PARSE_FAILED;
        //     }
        // }

        return result;
    }

    static std::vector<NSCam::LogicalSensorStruct> getLogicalDevices(void)
    {
        std::vector<struct NSCam::LogicalSensorStruct> gCustomDevList;
        NSCam::IHalSensorList *pSensorList = MAKE_HalSensorList();
        if (NULL == pSensorList) {
            CAM_ULOGME("Cannot get sensor list");
            return gCustomDevList;
        }

        auto parseAndRefineSensorName = [&](std::string &name)->int32_t
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
        };

        //Build sensor name -> list of sensor indexes
        std::unordered_map<std::string, std::vector<int32_t>> sensorNameMap;
        int32_t sensorCount = pSensorList->queryNumberOfSensors();
        for(int index = 0; index < sensorCount; ++index) {
            std::string s(pSensorList->queryDriverName(index));
            parseAndRefineSensorName(s);
            auto iter = sensorNameMap.find(s);
            if(iter == sensorNameMap.end())
            {
                std::vector<int32_t> v;
                v.push_back(index);
                sensorNameMap.emplace(s, v);
            }
            else
            {
                iter->second.push_back(index);
            }
        }

        gCustomDevList.clear();

        json jsonObj;
        LogicalCamJSONUtil::parseLogicalCamCustomSetting(jsonObj);

        // Sensor Combinations
        for(auto &settingValue : jsonObj[CUSTOM_KEY_SENSOR_COMBINATIONS]) {
            // Logical Device
            if(LogicalCamJSONUtil::HasMember(settingValue, CUSTOM_KEY_LOGICAL_DEVICE)) {
                const json &logcalDeviceJson = settingValue[CUSTOM_KEY_LOGICAL_DEVICE];
                NSCam::LogicalSensorStruct logicalDevice;
                // Name
                logicalDevice.Name = logcalDeviceJson[CUSTOM_KEY_NAME].get<std::string>();
                CAM_ULOGMD("Logical Device Name: %s", logicalDevice.Name.c_str());

                logicalDevice.NumofCombinSensor = 0;
                logicalDevice.NumofDefinition   = 1;

                // Sensors
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
                //        "Sensors": ["IMX123_MIPI_RAW", "IMX456_MIPI_RAW", "IMX456_MIPI_RAW"]
                bool hasError = false;
                std::unordered_map<std::string, std::vector<int32_t>> leftIdsMap;
                std::vector<std::string> namesInSetting = settingValue[CUSTOM_KEY_SENSORS].get<std::vector<std::string>>();
                std::vector<int32_t> sensorIds(namesInSetting.size());
                std::vector<std::string> sensorNames(namesInSetting.size());
                std::unordered_map<std::string, size_t> nameCount;
                for(int settingIndex = 0; settingIndex < namesInSetting.size(); ++settingIndex)
                {
                    auto name = namesInSetting[settingIndex];
                    int customizedId = parseAndRefineSensorName(name);
                    sensorNames[settingIndex] = name;
                    auto nameIt = sensorNameMap.find(name);
                    if(nameIt == sensorNameMap.end())
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
                                CAM_ULOGME("[Parse Sensor Combination]There are more than one \"%s\" in the setting, only one installed on the device\n",
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
                                        CAM_ULOGME("[Parse Sensor Combination]There are more than one \"%s\" in the setting, please correct setting\n",
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
                                if(leftIds.size() == 0)
                                {
                                    CAM_ULOGME("[Parse Sensor Combination]Please remove duplicated sensor setting of \"%s\"\n",
                                            namesInSetting[settingIndex].c_str());
                                    hasError = true;
                                    break;
                                }
                                else
                                {
                                    // Sensor ID is specified, must be in installed list
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
                                            << name << DELIMITER << customizedId << "\"" << std::endl
                                            << " Available settings:" << std::endl;
                                        for(auto &id : ids) {
                                            oss << "    \"" << name << DELIMITER << id << "\"" << std::endl;
                                        }
                                        CAM_ULOGME("%s", oss.str().c_str());
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
                            CAM_ULOGME("[Parse Sensor Combination]Please remove the \"%s\" without ID specified", sensorNames[idIndex].c_str());
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
                            auto nameIt = sensorNameMap.find(name);
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
                                CAM_ULOGME("[Parse Sensor Combination]Need to correct setting for %s,\n"
                                           "  Avaliable solutions:\n"
                                           "  1. Specify sensor ID if only one of the duplicated sensors are used\n"
                                           "  2. Remove all ID specified, we'll use them in sensor order\n"
                                           "  3. Left only one without specified and specify for the others\n",
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
                    if(sensorId >= 0)
                    {
                        logicalDevice.SensorIDs.push_back(sensorId);
                        CAM_ULOGMD("Sensor #%d: %s(ID: %d)", logicalDevice.NumofCombinSensor, sensorNames[s].c_str(), sensorId);
                        logicalDevice.NumofCombinSensor++;
                    }
                    else
                    {
                        CAM_ULOGME("Wrong sensor ID sepcified for \"%s\"\n", sensorNames[s].c_str());
                    }
                }

                if(0 == settingValue[CUSTOM_KEY_SENSORS].size() ||
                   (size_t)logicalDevice.NumofCombinSensor < settingValue[CUSTOM_KEY_SENSORS].size())
                {
                    CAM_ULOGMD("---------------");
                    continue;
                }

                // Features
                logicalDevice.Feature = 0;
                for(auto &feature : logcalDeviceJson[CUSTOM_KEY_FEATURES])
                {
                    std::string name = feature.get<std::string>();
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

                    if(name == CUSTOM_KEY_VSDOF)
                    {
                        logicalDevice.Feature |= NSCam::DEVICE_FEATURE_VSDOF;
                        CAM_ULOGMD("%s", CUSTOM_KEY_VSDOF);
                    }
                    else if(name == CUSTOM_KEY_ZOOM)
                    {
                        logicalDevice.Feature |= NSCam::DEVICE_FEATURE_ZOOM;
                        CAM_ULOGMD("%s", CUSTOM_KEY_ZOOM);
                    }
                    else if(name == CUSTOM_KEY_DENOISE)
                    {
                        logicalDevice.Feature |= NSCam::DEVICE_FEATURE_DENOISE;
                        CAM_ULOGMD("%s", CUSTOM_KEY_DENOISE);
                    }
                    else if(name == CUSTOM_KEY_3RD_Party)
                    {
                        logicalDevice.Feature |= NSCam::DEVICE_FEATURE_VSDOF;//NSCam::DEVICE_FEATURE_3RD_PARTY;
                        CAM_ULOGMD("%s(%s)", CUSTOM_KEY_VSDOF, CUSTOM_KEY_3RD_Party);
                    }
                    else if(name == CUSTOM_KEY_MTK_DEPTHMAP)
                    {
                        logicalDevice.Feature |= NSCam::DEVICE_FEATURE_VSDOF;//NSCam::DEVICE_FEATURE_MTK_DEPTHMAP;
                        CAM_ULOGMD("%s(%s)", CUSTOM_KEY_VSDOF, CUSTOM_KEY_MTK_DEPTHMAP);
                    }
                    else if(name == CUSTOM_KEY_SECURE_CAMERA)
                    {
                        logicalDevice.Feature |= NSCam::DEVICE_FEATURE_SECURE_CAMERA;
                        CAM_ULOGMD("%s", CUSTOM_KEY_SECURE_CAMERA);
                    }
                    else
                    {
                        CAM_ULOGMD("Unsupported feature: %s", name.c_str());
                    }
                }

                //Check is logical multicam disabled by user
                if( logicalDevice.NumofCombinSensor < 2 ||
                    (LogicalCamJSONUtil::HasMember(logcalDeviceJson, CUSTOM_KEY_DISABLE_LOGICAL_MULTICAM) &&
                     logcalDeviceJson[CUSTOM_KEY_DISABLE_LOGICAL_MULTICAM].get<int>()) )
                {
                    logicalDevice.isLogicalMulticam = false;
                    CAM_ULOGMD("Logical multicam disabled");
                }

                CAM_ULOGMD("---------------");

                gCustomDevList.push_back(logicalDevice);
            }
        }

        return gCustomDevList;
    }
};

};
#endif
