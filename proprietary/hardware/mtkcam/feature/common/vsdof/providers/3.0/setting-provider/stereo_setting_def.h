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
#ifndef STEREO_SETTING_DEF_H_
#define STEREO_SETTING_DEF_H_

#include <vector>   //for std::vector
#include <utility>  //for std::pair

#include <camera_custom_stereo.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/FastLogger.h>
#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/aaa/IHal3A.h>

#include <mtkcam/feature/stereo/hal/StereoArea.h>
#include <stereo_crop_util.h>
#include <sstream>  //For ostringstream

using namespace NSCam;
using namespace NS3Av3;
using namespace StereoHAL;
using namespace NSCam::v1::Stereo;

//Keys in custom file settings
#define CUSTOM_KEY_SENSORS              "Sensors"
#define CUSTOM_KEY_NAME                 "Name"
#define CUSTOM_KEY_CAPTURE_SIZE         "Capture Size"
#define CUSTOM_KEY_IMGOYUV_SIZE         "IMGOYUV Size"
#define CUSTOM_KEY_RRZOYUV_SIZE         "RRZOYUV Size"
#define CUSTOM_KEY_IRRRZO_SIZE          "IR RRZO Size"
#define CUSTOM_KEY_DEPTHMAP_SIZE        "Depthmap Size"
#define CUSTOM_KEY_POSTVIEW_SIZE        "PostView Size"

//Can be ignored of the feature if the feature does not use the sensor
// Value can be one of: Preview, Capture, Video, Custom1, Custom2
#define CUSTOM_KEY_SENSOR_SCENARIO_ZSD  "SensorScenarioZSD"
#define CUSTOM_KEY_SENSOR_SCENARIO_REC  "SensorScenarioRecording"
#define CUSTOM_KEY_STEREO_CAPTURE       "StereoCapture"
#define CUSTOM_KEY_VSDOF                "VSDoF"
#define CUSTOM_KEY_DENOISE              "Denoise"
#define CUSTOM_KEY_3RD_Party            "3rdParty"
#define CUSTOM_KEY_ZOOM                 "Zoom"
#define CUSTOM_KEY_MTK_DEPTHMAP         "MtkDepthmap"
#define CUSTOM_KEY_ACTIVE_STEREO        "ActiveStereo"

//Optional, if not set, StereoSettingProvider will return FOV from sensor driver
#define CUSTOM_KEY_FOV                  "FOV"
#define CUSTOM_KEY_FOV_H                "H"
#define CUSTOM_KEY_FOV_V                "V"

//Optional, if not set, StereoSettingProvider will return result from calibration driver
#define CUSTOM_KEY_CALIBRATION          "Calibration"
#define CUSTOM_KEY_MACRO_DISTANCE       "Macro Distance"
#define CUSTOM_KEY_INFINITE_DISTANCE    "Infinite Distance"

#define CUSTOM_KEY_SENSOR_COMBINATIONS  "Sensor Combinations"
#define CUSTOM_KEY_MAIN1_NAME           "Main1 Name"
#define CUSTOM_KEY_MAIN2_NAME           "Main2 Name"
#define CUSTOM_KEY_MODULE_TYPE          "Module Type"
#define CUSTOM_KEY_BASELINE             "Baseline"
#define CUSTOM_KEY_FOV_CROP             "FOV Crop"
#define CUSTOM_KEY_CENTER_CROP          "Center Crop"
#define CUSTOM_KEY_DISABLE_CROP         "Disable Crop"
#define CUSTOM_KEY_MODULE_VARIATION     "Module Variation"
#define CUSTOM_KEY_WORKING_RANGE        "Working Range"
#define CUSTOM_KEY_LDC                  "LDC"

#define CUSTOM_KEY_CALLBACK_BUFFER_LIST "Callback Buffer List"
#define CUSTOM_KEY_VALUE                "Value"

#define CUSTOM_KEY_SIZE_CONFIG          "Size Config"
#define CUSTOM_KEY_CONTENT_SIZE         "Content Size"
#define CUSTOM_KEY_PADDING              "Padding"

typedef MUINT32 SensorUID_T;
typedef std::map<int, int> SensorScenarioMap_T;

struct StereoCropSetting_T
{
    float keepRatio   = 1.0f; //keep ratio, i.e., 1.0 means no crop
    float cropRatio   = 0.5f; //min ratio of cropped edge
    float cropDegreeH = 0.0f;
    float cropDegreeV = 0.0f;

    CUST_FOV_CROP_T toCusCrop()
    {
        return CUST_FOV_CROP_T(cropDegreeH, cropRatio);
    }
};

const std::map<int, const char *> SENSOR_SCENARIO_TO_STRING_MAP =
{
    {SENSOR_SCENARIO_ID_NORMAL_PREVIEW, "SENSOR_SCENARIO_ID_NORMAL_PREVIEW"},
    {SENSOR_SCENARIO_ID_NORMAL_CAPTURE, "SENSOR_SCENARIO_ID_NORMAL_CAPTURE"},
    {SENSOR_SCENARIO_ID_NORMAL_VIDEO,   "SENSOR_SCENARIO_ID_NORMAL_VIDEO"},
    {SENSOR_SCENARIO_ID_SLIM_VIDEO1,    "SENSOR_SCENARIO_ID_SLIM_VIDEO1"},
    {SENSOR_SCENARIO_ID_SLIM_VIDEO2,    "SENSOR_SCENARIO_ID_SLIM_VIDEO2"},
    {SENSOR_SCENARIO_ID_CUSTOM1,        "SENSOR_SCENARIO_ID_CUSTOM1"},
    {SENSOR_SCENARIO_ID_CUSTOM2,        "SENSOR_SCENARIO_ID_CUSTOM2"},
    {SENSOR_SCENARIO_ID_CUSTOM3,        "SENSOR_SCENARIO_ID_CUSTOM3"},
    {SENSOR_SCENARIO_ID_CUSTOM4,        "SENSOR_SCENARIO_ID_CUSTOM4"},
    {SENSOR_SCENARIO_ID_CUSTOM5,        "SENSOR_SCENARIO_ID_CUSTOM5"},
    {SENSOR_SCENARIO_ID_UNNAMED_START,  "Unknown"},
};

const std::map<int, const char *> FEATURE_MODE_TO_STRING_MAP =
{
    { E_STEREO_FEATURE_CAPTURE,      "STEREO_FEATURE_CAPTURE" },
    { E_STEREO_FEATURE_VSDOF,        "STEREO_FEATURE_VSDOF" },
    { E_STEREO_FEATURE_DENOISE,      "STEREO_FEATURE_DENOISE" },
    { E_STEREO_FEATURE_THIRD_PARTY,  "STEREO_FEATURE_THIRD_PARTY" },
    { E_DUALCAM_FEATURE_ZOOM,        "DUALCAM_FEATURE_ZOOM" },
    { E_STEREO_FEATURE_MTK_DEPTHMAP, "STEREO_FEATURE_MTK_DEPTHMAP" },
    { E_STEREO_FEATURE_ACTIVE_STEREO,"STEREO_FEATURE_ACTIVE_STEREO" },
};

struct StereoSensorSetting_T
{
    int             index             = -1; //0, 1, 2, 3
    int             devIndex          = 0;  //1, 2, 4, 8
    std::string     name;

    float           fovHorizontal     = 0.0f;
    float           fovVertical       = 0.0f;
    float           fovDiagonal       = 0.0f;

    float           fovHRuntime       = 0.0f;   //Adjust after setting feature mode
    float           fovVRuntime       = 0.0f;

    std::map<ENUM_STEREO_RATIO, std::vector<NSCam::MSize>> captureSizes;

    std::map<ENUM_STEREO_RATIO, NSCam::MSize> imgoYuvSize;
    std::map<ENUM_STEREO_RATIO, NSCam::MSize> rrzoYuvSize;
    std::map<ENUM_STEREO_RATIO, NSCam::MSize> postviewSize;

    std::map<ENUM_STEREO_RATIO, bool>  supportedImageRatios;   //according to capture sizes

    std::map<ENUM_STEREO_RATIO, NSCam::MSize> irRrzoSize = { {eRatio_16_9, NSCam::MSize(1280, 720)} ,
                                                             {eRatio_4_3,  NSCam::MSize(1280, 960)} };

    int             distanceMacro     = 0;  //calibration distance of macro, unit: mm
    int             distanceInfinite  = 0;  //calibration distance of inf, unit: mm

    SensorScenarioMap_T sensorScenarioMapZSD;           //Feature -> sensor scenario(pipeline: ZSD)
    SensorScenarioMap_T sensorScenarioMapRecord;        //Feature -> sensor scenario(pipeline: RECORDING)

    NSCam::SensorStaticInfo staticInfo;

    //Calculated by FOVCropUtil
    StereoCropSetting_T cropSetting;

    ~StereoSensorSetting_T()
    {
    }

    std::string stereoFeatureModeToString(int featureMode) const
    {
        std::string name;
        if(FEATURE_MODE_TO_STRING_MAP.find(featureMode) != FEATURE_MODE_TO_STRING_MAP.end()) {
            name = FEATURE_MODE_TO_STRING_MAP.at(featureMode);
        }

        return name;
    }

    void log(FastLogger &logger) const
    {
        logger
        .FastLogD("Address:  %p", this)
        .FastLogD("Index:    %d", index)
        .FastLogD("DevIndex: %d", devIndex)
        .FastLogD("UID:      0x%X", staticInfo.sensorDevID)
        .FastLogD("Name:     %s", name.c_str())
        .FastLogD("Position: %s", (staticInfo.facingDirection==0) ? "Rear" : "Front")
        .FastLogD("FOV       H: %.2f V: %.2f", fovHorizontal, fovVertical);

        //Capture size
        int m, n;
        char msg[256];
        for(auto &m1 : captureSizes) {
            imageRatioMToN(m1.first, m, n);
            sprintf(msg, "Capture Size of %2d:%-2d:", m, n);
            logger.FastLogD(msg);

            for(auto &s : m1.second) {
                logger.FastLogD("    %dx%d", s.w, s.h);
            }
        }

        //IMGO
        for(auto &m1 : imgoYuvSize) {
            imageRatioMToN(m1.first, m, n);
            sprintf(msg, "IMGO Yuv Size of %2d:%-2d: %dx%d", m, n, m1.second.w, m1.second.h);
            logger.FastLogD(msg);
        }

        //RRZO
        for(auto &m1 : rrzoYuvSize) {
            imageRatioMToN(m1.first, m, n);
            sprintf(msg, "RRZO YUV Size of %2d:%-2d: %dx%d", m, n, m1.second.w, m1.second.h);
            logger.FastLogD(msg);
        }

        //IR RRZO
        for(auto &m1 : irRrzoSize) {
            imageRatioMToN(m1.first, m, n);
            sprintf(msg, "IR RRZO YUV Size of %2d:%-2d: %dx%d", m, n, m1.second.w, m1.second.h);
            logger.FastLogD(msg);
        }

        if(sensorScenarioMapZSD.size() > 0) {
            logger.FastLogD("---- Sensor Scenario for ZSD ----");
            for(auto &s : sensorScenarioMapZSD) {
                std::string featureName = stereoFeatureModeToString(s.first);
                logger.FastLogD("  %-20s: %-3d(%s)", featureName.c_str(), s.second, SENSOR_SCENARIO_TO_STRING_MAP.at(s.second));
            }
        }

        if(sensorScenarioMapRecord.size() > 0) {
            logger.FastLogD("---- Sensor Scenario for Record ----");
            for(auto &s : sensorScenarioMapRecord) {
                std::string featureName = stereoFeatureModeToString(s.first);
                logger.FastLogD("  %-20s: %-3d(%s)", featureName.c_str(), s.second, SENSOR_SCENARIO_TO_STRING_MAP.at(s.second));
            }
        }
    }

    MINT32 getSensorScenario(MINT32 featureMode, MINT32 pipelineMode=PipelineMode_ZSD)
    {
        const int FEATURE_ID[] = //Upper feature has higher priority
        {
            NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP,
            NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF,
            NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE,
            NSCam::v1::Stereo::E_STEREO_FEATURE_DENOISE,
            NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM,
            NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY,
            NSCam::v1::Stereo::E_STEREO_FEATURE_ACTIVE_STEREO,
        };

        SensorScenarioMap_T *scenarioMap = NULL;
        if(pipelineMode == PipelineMode_ZSD) {
            scenarioMap = &sensorScenarioMapZSD;
        } else if(pipelineMode == PipelineMode_RECORDING) {
            scenarioMap = &sensorScenarioMapRecord;
        } else {
            ALOGE("Only support pipeline mode: PipelineMode_ZSD or PipelineMode_RECORDING");
        }

        if(scenarioMap) {
            for(auto searchFeature : FEATURE_ID) {
                if((featureMode & searchFeature) > 0 &&
                   scenarioMap->find(searchFeature) != scenarioMap->end())
                {
                    return (*scenarioMap)[searchFeature];
                }
            }
        }

        ALOGW("Cannot find sensor scenario for sensor %d, featureMode %d, pipeline mode %d",
              index, featureMode, pipelineMode);

        return SENSOR_SCENARIO_ID_UNNAMED_START;
    }

    bool updateRuntimeFOV(int featureMode, MINT32 pipelineMode=PipelineMode_ZSD)
    {
        int sensorScenario = getSensorScenario(featureMode, pipelineMode);
        if(sensorScenario == SENSOR_SCENARIO_ID_UNNAMED_START) {
            return false;
        }

        IHalSensorList* sensorList = nullptr;
        sensorList = MAKE_HalSensorList();
        if(NULL == sensorList) {
            ALOGE("Cannot get sensor list");
            return false;
        }
        IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, index);
        if(NULL == pIHalSensor) {
            ALOGE("Cannot get hal sensor");
            return false;
        }

        //Get sensor crop win info
        SensorCropWinInfo sensorCropInfo;
        MINT32 err = pIHalSensor->sendCommand(devIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&sensorScenario, (MUINTPTR)&sensorCropInfo, 0);
        if(err) {
            ALOGE("Cannot get sensor crop win info");
            return false;
        }

        //We only support CENTER CROP by sensor
        float ratio = 1.0f;
        if(sensorCropInfo.w2_tg_size != sensorCropInfo.scale_w ||
           sensorCropInfo.w0_size != sensorCropInfo.full_w)
        {
            ratio = (float)sensorCropInfo.w2_tg_size/(float)sensorCropInfo.scale_w *
                    (float)sensorCropInfo.w0_size/(float)sensorCropInfo.full_w;
            fovHRuntime = radiansToDegree(2.0f*atan(ratio * tan(degreeToRadians(fovHorizontal/2.0f))));
        } else {
            fovHRuntime = fovHorizontal;
        }

        if(sensorCropInfo.h2_tg_size != sensorCropInfo.scale_h ||
           sensorCropInfo.h0_size != sensorCropInfo.full_h)
        {
            ratio = (float)sensorCropInfo.h2_tg_size/(float)sensorCropInfo.scale_h *
                    (float)sensorCropInfo.h0_size/(float)sensorCropInfo.full_h;
            fovVRuntime = radiansToDegree(2.0f*atan(ratio * tan(degreeToRadians(fovVertical/2.0f))));
        } else {
            fovVRuntime = fovVertical;
        }

        return true;
    }
};

#define DEFAULT_MODULE_VARIATION (1.0f)     //degree
#define DEFAULT_WORKING_RANGE    (20.0f)    //cm

struct StereoSensorConbinationSetting_T
{
    std::vector<std::string> sensorNames;
    std::vector<StereoSensorSetting_T *> sensorSettings;
    MUINT32         moduleType      = 0;     //1-4
    float           baseline        = 0.95f;  //cm
    bool            enableLDC       = false;
    std::vector<float> LDC;

    bool            isCenterCrop    = false;
    bool            disableCrop     = false;
    float           moduleVariation = DEFAULT_MODULE_VARIATION;
    float           workingRange    = DEFAULT_WORKING_RANGE;

    //For 3rd party
    std::map<ENUM_STEREO_RATIO, NSCam::MSize> depthmapSize;

    //Customized VSDoF size
    bool            hasSizeConfig = false;
    std::map<ENUM_STEREO_RATIO, StereoHAL::StereoArea> baseSize;

    //Update after getting index from HAL sensor
    ENUM_STEREO_SENSOR_PROFILE profile  = STEREO_SENSOR_PROFILE_UNKNOWN;

    //Offline calibration
    std::vector<MUINT8> calibrationData;

    void log(FastLogger &logger)
    {
        logger
        .FastLogD("Address:          %p", this)
        .FastLogD("Sensors:");
        for(auto &setting : sensorSettings) {
            logger.FastLogD("    %s(%d)", setting->name.c_str(), setting->index);
        }

        logger
        .FastLogD("Module Type:      %d(%s, %s)", moduleType,
                                                (2==moduleType || 4==moduleType) ? "Horizontal" : "Vertical",
                                                (1==moduleType || 2==moduleType) ? "Main on left" : "Main on right")
        .FastLogD("Baseline:         %.2f cm", baseline)
        .FastLogD("DisableCrop:      %s", (disableCrop)?"Yes":"No")
        .FastLogD("CenterCrop:       %s", (isCenterCrop)?"Yes":"No")
        .FastLogD("Module Variation: %.1f degrees", moduleVariation)
        .FastLogD("Working Range:    %.1f cm", workingRange);

        int m, n;
        char msg[256];
        for(auto &m1 : depthmapSize) {
            imageRatioMToN(m1.first, m, n);
            sprintf(msg, "Depthmap Size of %2d:%-2d: %dx%d", m, n, m1.second.w, m1.second.h);
            logger.FastLogD(msg);
        }

        if(hasSizeConfig) {
            for(auto &m1 : depthmapSize) {
                imageRatioMToN(m1.first, m, n);
                StereoArea area = m1.second;
                sprintf(msg, "Size config for %2d:%-2d:   Content %dx%d, Padding %dx%d", m, n,
                        area.contentSize().w, area.contentSize().h,
                        area.padding.w,area.padding.h);
                logger.FastLogD(msg);
            }
        }

        if(LDC.size() == 0) {
            logger.FastLogD("LDC disabled");
        } else {
            logger.FastLogD("LDC enabled, table size %zu:", LDC.size());

            if(LDC.size() < 16) {
                for(size_t i = 0; i < LDC.size(); i++) {
                    logger.FastLogD(" % *.10f", 2, LDC[i]);
                }
            } else {
                const float *element = &LDC[0];
                for(int k = 0; k < 2; k++) {
                    int line = (int)*element++;
                    int sizePerLine = (int)*element++;
                    logger.FastLogD(" %d %d\n", line, sizePerLine);
                    for(int i = 1; i <= line; i++) {
                        logger.FastLogD("[%02d]% *.10f % *.10f ... % *.10f % *.10f %*d %*d\n",
                                        i,
                                        2, *element, 2, *(element+1),
                                        2, *(element+sizePerLine-2), 2, *(element+sizePerLine-1),
                                        4, (int)*(element+sizePerLine),
                                        2, (int)*(element+sizePerLine+1));
                        element += (sizePerLine+2);
                    }
                }
            }
        }

        if(calibrationData.size() > 0) {
            logger.FastLogD("Offline calibration size %d", calibrationData.size());
        }
    }
};

#endif