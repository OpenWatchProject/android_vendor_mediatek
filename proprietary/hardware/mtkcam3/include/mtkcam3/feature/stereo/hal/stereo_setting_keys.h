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
#ifndef STEREO_SETTING_KEYS_H_
#define STEREO_SETTING_KEYS_H_

//Keys in custom file settings
#define CUSTOM_KEY_SENSORS                  "Sensors"
#define CUSTOM_KEY_NAME                     "Name"
#define CUSTOM_KEY_IMGOYUV_SIZE             "IMGOYUV Size"
#define CUSTOM_KEY_RRZOYUV_SIZE             "RRZOYUV Size"
#define CUSTOM_KEY_DEPTHMAP_SIZE            "Depthmap Size"
#define CUSTOM_KEY_DEPTHMAP_CAPTURE_SIZE    "Depthmap Size Capture"

//Can be ignored of the feature if the feature does not use the sensor
// Value can be one of: Preview, Capture, Video, Custom1, Custom2
#define CUSTOM_KEY_SENSOR_SCENARIO_ZSD  "SensorScenarioZSD"
#define CUSTOM_KEY_SENSOR_SCENARIO_REC  "SensorScenarioRecording"
#define CUSTOM_KEY_STEREO_CAPTURE       "StereoCapture"

//Feature key must be lower case here, but setting is case-insensitive
#define CUSTOM_KEY_VSDOF                "vsdof"
#define CUSTOM_KEY_DENOISE              "denoise"
#define CUSTOM_KEY_3RD_Party            "3rdparty"
#define CUSTOM_KEY_ZOOM                 "zoom"
#define CUSTOM_KEY_MTK_DEPTHMAP         "mtkdepthmap"
#define CUSTOM_KEY_MULTI_CAM            "multicam"
#define CUSTOM_KEY_SECURE_CAMERA        "securecamera"

//Optional, if not set, StereoSettingProvider will return FOV from sensor driver
#define CUSTOM_KEY_FOV                  "FOV"
#define CUSTOM_KEY_FOV_H                "H"
#define CUSTOM_KEY_FOV_V                "V"
#define CUSTOM_KEY_FOV_D                "D" //Calculate H&V by sensor size

//Optional, if not set, StereoSettingProvider will return result from calibration driver
#define CUSTOM_KEY_CALIBRATION          "Calibration"
#define CUSTOM_KEY_MACRO_DISTANCE       "Macro Distance"
#define CUSTOM_KEY_INFINITE_DISTANCE    "Infinite Distance"
#define CUSTOM_KEY_LENS_POSE_TRANSLATION "Lens Pose Translation"

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

#define CUSTOM_KEY_LOGICAL_DEVICE       "Logical Device"
#define CUSTOM_KEY_FEATURES             "Features"

#define CUSTOM_KEY_DEPTHMAP_FORMAT      "Depthmap Format"

#define CUSTOM_KEY_MAIN2_OUTPUT_FREQ    "Main2 Freq"

#define CUSTOM_KEY_REFINE_LEVEL         "Refine Level"

#define CUSTOM_KEY_MULTICAM_ZOOM_RANGE  "Zoom Range"
#define CUSTOM_KEY_MULTICAM_ZOOM_STEPS  "Zoom Steps"

#define CUSTOM_KEY_FRZ_RATIO            "FRZ Ratio"

#endif
