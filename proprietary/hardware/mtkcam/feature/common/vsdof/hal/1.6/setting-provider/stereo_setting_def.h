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

using namespace NSCam;
using namespace NS3Av3;

//Keys in custom file settings
#define CUSTOM_KEY_SENSORS              "Sensors"
#define CUSTOM_KEY_UID                  "UID"
#define CUSTOM_KEY_NAME                 "Name"
#define CUSTOM_KEY_CAPTURE_SIZE         "Capture Size"
#define CUSTOM_KEY_IMGOYUV_SIZE         "IMGOYUV Size"
#define CUSTOM_KEY_RRZOYUV_SIZE         "RRZOYUV Size"
#define CUSTOM_KEY_DEPTHMAP_SIZE        "Depthmap Size"
#define CUSTOM_KEY_RATIO_4_3            "4:3"
#define CUSTOM_KEY_RATIO_16_9           "16:9"

//Can be ignored of the feature if the feature does not use the sensor
// Value can be one of: Preview, Capture, Video, Custom1, Custom2
#define CUSTOM_KEY_SENSOR_SCENARIO_ZSD  "SensorScenarioZSD"
#define CUSTOM_KEY_SENSOR_SCENARIO_REC  "SensorScenarioRecording"
#define CUSTOM_KEY_STEREO_CAPTURE       "StereoCapture"
#define CUSTOM_KEY_VSDOF                "VSDoF"
#define CUSTOM_KEY_DENOISE              "Denoise"
#define CUSTOM_KEY_3RD_Party            "3rdParty"
#define CUSTOM_KEY_ZOOM                 "Zoom"

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
#define CUSTOM_KEY_DEGREE               "Degree"
#define CUSTOM_KEY_RATIO                "Ratio"
#define CUSTOM_KEY_LDC                  "LDC"

typedef MUINT32 SensorUID_T;
typedef std::map<int, MINT32> SensorScenarioMap_T;

struct StereoSensorSetting_T
{
    int             index             = -1;  //0, 1, 2, 3
    int             devIndex          = 0;  //1, 2, 4, 8
    SensorUID_T     uid               = 0;  //0x258, 0x214, etc
    std::string     name;

    float           fovHorizontal     = 0.0f;
    float           fovVertical       = 0.0f;

    std::vector<NSCam::MSize>   captureSizes_16_9;
    std::vector<NSCam::MSize>   captureSizes_4_3;

    NSCam::MSize                imgoYuvSize_16_9;
    NSCam::MSize                imgoYuvSize_4_3;

    NSCam::MSize                rrzoYuvSize_16_9;
    NSCam::MSize                rrzoYuvSize_4_3;

    int             distanceMacro     = 0;  //calibration distance of macro, unit: mm
    int             distanceInfinite  = 0;  //calibration distance of inf, unit: mm

    SensorScenarioMap_T sensorScenarioMapZSD;       //Feature -> sensor scenario(pipeline: ZSD)
    SensorScenarioMap_T sensorScenarioMapRecord;    //Feature -> sensor scenario(pipeline: RECORDING)

    NSCam::SensorStaticInfo staticInfo;

    ~StereoSensorSetting_T()
    {
    }

    void log(FastLogger &logger)
    {
        logger
        .FastLogD("Address:  %p", this)
        .FastLogD("Index:    %d", index)
        .FastLogD("DevIndex: %d", devIndex)
        .FastLogD("UID:      0x%X", uid)
        .FastLogD("Name:     %s", name.c_str())
        .FastLogD("FOV       H: %.2f V: %.2f", fovHorizontal, fovVertical)
        .FastLogD("Capture Size of 16:9:");
        for(auto &s : captureSizes_16_9) {
            logger.FastLogD("    %dx%d", s.w, s.h);
        }
        logger.FastLogD("Capture Size of 4:3:");
        for(auto &s : captureSizes_4_3) {
            logger.FastLogD("    %dx%d", s.w, s.h);
        }
        if(imgoYuvSize_16_9.w > 0 &&
           imgoYuvSize_16_9.h > 0)
        {
            logger.FastLogD("IMGO Yuv Size of 16:9: %dx%d", imgoYuvSize_16_9.w, imgoYuvSize_16_9.h);
        }
        if(imgoYuvSize_4_3.w > 0 &&
           imgoYuvSize_4_3.h > 0)
        {
            logger.FastLogD("IMGO Yuv Size of 4:3:  %dx%d", imgoYuvSize_4_3.w, imgoYuvSize_4_3.h);
        }
        if(rrzoYuvSize_16_9.w > 0 &&
           rrzoYuvSize_16_9.h > 0)
        {
            logger.FastLogD("RRZO Yuv Size of 16:9: %dx%d", rrzoYuvSize_16_9.w, rrzoYuvSize_16_9.h);
        }
        if(rrzoYuvSize_4_3.w > 0 &&
           rrzoYuvSize_4_3.h > 0)
        {
            logger.FastLogD("RRZO Yuv Size of 4:3:  %dx%d", rrzoYuvSize_4_3.w, rrzoYuvSize_4_3.h);
        }
        logger
        .FastLogD("Distance Macro: %d, Inf %d", distanceMacro, distanceInfinite);
    }

    MINT32 getSensorScenario(MINT32 featureMode, MINT32 pipelineMode)
    {
        const int FEATURE_ID[] = //Upper feature has higher priority
        {
            NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF,
            NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE,
            NSCam::v1::Stereo::E_STEREO_FEATURE_DENOISE,
            NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM,
            NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY,
        };

        int searchFeature = 0;
        if(pipelineMode == PipelineMode_ZSD) {
            for(auto searchFeature : FEATURE_ID) {
                if((featureMode & searchFeature) > 0 &&
                   sensorScenarioMapZSD.find(searchFeature) != sensorScenarioMapZSD.end())
                {
                    return sensorScenarioMapZSD[searchFeature];
                }
            }
        } else if(pipelineMode == PipelineMode_RECORDING) {
            for(auto searchFeature : FEATURE_ID) {
                if((featureMode & searchFeature) > 0 &&
                   sensorScenarioMapRecord.find(searchFeature) != sensorScenarioMapRecord.end())
                {
                    return sensorScenarioMapRecord[searchFeature];
                }
            }
        } else {
            ALOGE("Only support pipeline model of PipelineMode_ZSD and PipelineMode_RECORDING");
        }

        ALOGE("Cannot find sensor scenario for sensor %d, featureMode %d, pipeline model %d",
                index, featureMode, pipelineMode);

        return SENSOR_SCENARIO_ID_UNNAMED_START;
    }
};

struct StereoSensorConbinationSetting_T
{
    SensorUID_T     uidMain1   = 0;
    SensorUID_T     uidMain2   = 0;
    MUINT32         moduleType = 0;     //1-4
    float           baseline   = 1.0f;  //cm
    CUST_FOV_CROP_T fovCrop;
    bool            enableLDC;
    std::vector<float>   LDC;

    //For 3rd party
    NSCam::MSize    depthmapSize_16_9;
    NSCam::MSize    depthmapSize_4_3;

    void log(FastLogger &logger)
    {
        logger
        .FastLogD("Address:     %p", this)
        .FastLogD("Main1 UID:   0x%X", uidMain1)
        .FastLogD("Main2 UID:   0x%X", uidMain2)
        .FastLogD("Module Type: %d", moduleType)
        .FastLogD("Baseline:    %.2f", baseline);

        if(depthmapSize_16_9.w > 0 &&
           depthmapSize_16_9.h > 0)
        {
            logger.FastLogD("Depthmap Size of 16:9: %dx%d", depthmapSize_16_9.w, depthmapSize_16_9.h);
        }
        if(depthmapSize_4_3.w > 0 &&
           depthmapSize_4_3.h > 0)
        {
            logger.FastLogD("Depthmap Size of 4:3:  %dx%d", depthmapSize_4_3.w, depthmapSize_4_3.h);
        }

        if(LDC.size() > 3) {
            // logger.FastLogD("LDC(size %d):\n  %d %d\n  %f %f...",
            //                 LDC.size(), (int)LDC[0], (int)LDC[1], LDC[2], LDC[3]);
            logger.FastLogD("LDC table size %zu:", LDC.size());

            float *element = &LDC[0];
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
        } else {
            logger.FastLogD("LDC: Empty");
        }
    }
};

#endif