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
#ifndef _STEREO_SETTING_PROVIDER_H_
#define _STEREO_SETTING_PROVIDER_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>                // For property
#include <string>
#include <camera_custom_stereo.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>
#include "stereo_common.h"
#include <ctime>
#include <sstream>

//NOTICE: property has 31 characters limitation
#define PROPERTY_ENABLE_LOG         STEREO_PROPERTY_PREFIX"log"
#define PROPERTY_ENABLE_DUMP        STEREO_PROPERTY_PREFIX"dump"
#define PERPERTY_PASS1_LOG          PROPERTY_ENABLE_LOG".Pass1"
#define PERPERTY_DEPTHMAP_NODE_LOG  PROPERTY_ENABLE_LOG".DepthMapNode"
#define PERPERTY_BOKEH_NODE_LOG     PROPERTY_ENABLE_LOG".BokehNode"
#define PERPERTY_BMDENOISE_NODE_LOG PROPERTY_ENABLE_LOG".BMDenoiseNode"
#define PERPERTY_DCMF_NODE_LOG      PROPERTY_ENABLE_LOG".DualCamMFNode"
#define PERPERTY_DIT_NODE_LOG       PROPERTY_ENABLE_LOG".DITNode"
#define PERPERTY_JENC_NODE_LOG      PROPERTY_ENABLE_LOG".JpgEncNode"
#define PERPERTY_VENC_NODE_LOG      PROPERTY_ENABLE_LOG".VEncNode"
#define PROPERTY_ENABLE_PROFILE_LOG PROPERTY_ENABLE_LOG".Profile"

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef MINT32 N3D_SCENEINFO_TYPE;

enum {
    PipelineMode_PREVIEW,
    PipelineMode_RECORDING,
    PipelineMode_ZSD
};

struct STEREO_PARAMS_T
{
    std::string jpsSize;          // stereo picture size
    std::string jpsSizesStr;      // supported stereo picture size
    std::string refocusSize;      // refocus picture size
    std::string refocusSizesStr;  // supported refocus picture size
    std::string postviewSizesStr; // supported postview size
    std::string depthmapSizeStr;  // supported depthmap size
    int n3dSizes;      // n3d size
    int extraSizes;    // extra size
};

#if (20 == STEREO_HAL_VER)
//=====================
// WPE buffer
//=====================
class WARPING_BUFFER_CONFIG_T
{
public:
    static const size_t MAX_WARPING_BUFFER_PLANE = 3;

    NSCam::MSize  BUFFER_SIZE;   //max: 640x480
    MUINT32       PLANE_COUNT;   //2~3

    WARPING_BUFFER_CONFIG_T(NSCam::MSize s=NSCam::MSize(0, 0), MUINT32 p=0)
        : BUFFER_SIZE(s)
        , PLANE_COUNT(p)
    {
    }

    WARPING_BUFFER_CONFIG_T(const WARPING_BUFFER_CONFIG_T &config)
        : BUFFER_SIZE(config.BUFFER_SIZE)
        , PLANE_COUNT(config.PLANE_COUNT)
    {
    }

    inline size_t getPlaneSizeInBytes() const {
        return (BUFFER_SIZE.w * BUFFER_SIZE.h * sizeof(MUINT32));
    }
};

struct WARPING_BUFFER_T
{
    const WARPING_BUFFER_CONFIG_T CONFIG;
    android::sp<NSCam::IImageBuffer>    planeBuffer[3];

    WARPING_BUFFER_T(WARPING_BUFFER_CONFIG_T &config);
    ~WARPING_BUFFER_T();
};
#endif

#define WIDE_TELE_VSDOF_FOV_DIFF (10.0f)

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

/**
 * \brief This class provides static stereo settings
 * \details Make sure you have turned on sensors before using any sensor related API
 *
 */
class StereoSettingProvider
{
public:
    /**
     * \brief Get stereo sensor index
     * \details In most of the time, main1 index is 0, main2 index is 2.
     *
     * \param  main1 sensor index
     * \param  main2 sensor index
     * \param  stereo profile
     *
     * \return true if successfully get both index; otherwise false
     * \see stereoProfile()
     */
    static bool getStereoSensorIndex(int32_t &main1Idx, int32_t &main2Idx, ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());
    /**
     * \brief Get sensor device index
     * \details In most of the time, main1 index is 1, main2 index is 4.
     *
     * \param  main1 Saves the device index of main stereo sensor
     * \param  main2 Saves the device index of another stereo sensor
     * \param  stereo profile
     *
     * \return true if successfully get both index; otherwise false
     * \see stereoProfile()
     */
    static bool getStereoSensorDevIndex(int32_t &main1DevIdx, int32_t &main2DevIdx, ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Get image ratio
     *
     * \return Ratio of image
     */
    static ENUM_STEREO_RATIO imageRatio() { return m_imageRatio; }

    /**
     * \brief Get De-Noise setting
     * \return True if running de-noise feature
     */
    static bool isDeNoise();

    /**
     * \brief Get 3rd-Party setting
     * \return True if running de-noise feature
     */
    static bool is3rdParty() { return (m_stereoFeatureMode & NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY); }

    /**
     * \brief Check if feature mode is to provide MTK depthmap
     * \return true if feature mode is MTK depthmap mode
     */
    static bool isMTKDepthmapMode() { return (m_stereoFeatureMode & NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP); }

    /**
     * \brief Check if feature mode is active stereo
     * \return true if feature mode is active stereo
     */
    static bool isActiveStereo() { return (m_stereoFeatureMode & NSCam::v1::Stereo::E_STEREO_FEATURE_ACTIVE_STEREO); }

    /**
     * \brief Get stereo profile
     * \details There could be more than one set of stereo camera on the device,
     *          different profile has different sensor id, baseline, FOV, sizes, etc.
     * \return Stereo profile
     */
    static ENUM_STEREO_SENSOR_PROFILE stereoProfile() { return m_stereoProfile; }

    /**
     * \brief Check if the device has hardware feature extraction component
     * \return true if the device has hardware feature extraction component
     */
    static bool hasHWFE();

    /**
     * \brief Get FE/FM block size
     * \details Block sizes are different in each FE/FM stage
     *
     * \param FE_MODE Block size of mode 0: 32, mode 1: 16, mode 2: 8
     * \return Block size
     */
    static MUINT32 fefmBlockSize(const int FE_MODE);

#if (16 == STEREO_HAL_VER || 17 == STEREO_HAL_VER || STEREO_HAL_VER >= 30)
    /**
     * \brief Get LDC table of current profile
     * \return LDC table
     */
    static std::vector<float> &getLDCTable();

    /**
     * \brief Enable LDC or not
     * \return true if enable LDC
     */
    static bool LDCEnabled();

    /**
     * \brief Get FOV crop setting of current profile
     * \return FOV crop setting
     */
    static CUST_FOV_CROP_T getFOVCropSetting();

    /**
     * \brief Get calibration distance of sensor
     * \details Calibration distances came from two sources: calibration driver or custom setting
     *          User can customize this field in camera_custom_stereo_setting.h, e.g.
     *          "\"Calibration\": {"
     *              "\"Macro Distance\": 100,"
     *              "\"Infinite Distance\": 5000"
     *          "}"
     *
     * \param sensor Sensor to query
     * \param macroDistance Calibration distance of macro
     * \param infiniteDistance Calibration distance of infinite
     * \return true if success
     */
    static bool getCalibrationDistance(StereoHAL::ENUM_STEREO_SENSOR sensor, MUINT32 &macroDistance, MUINT32 &infiniteDistance);
#endif

    /**
     * \brief Get FOV(field of view) of stereo sensors
     *
     * \param mainFOV FOV of main1 sensor
     * \param main2FOV FOV of main2 sensor
     * \param profile Profile of stereo camera, can be rear-rear or front-front
     *
     * \return true if success
     * \see getStereoFOV
     */
    static bool getStereoCameraFOV(SensorFOV &mainFOV, SensorFOV &main2FOV, ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Get FOV ratio of main1/main2
     * \details The ratio is decided by the difference of two sensors:
     *          diff >= 20, ratio = 1.4
     *          15 <= diff < 20, ratio = 1.3
     *          10 <= diff < 15, ratio = 1.2
     *          5 <= diff < 10, ratio = 1.1
     *          diff < 5, ratio = 1.0
     *
     * \param stereoProfile Profile of stereo camera, can be rear-rear or front-front
     *
     * \return FOV ratio
     */
    static float getStereoCameraFOVRatio(ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());  //main2_fov / main1_fov

    /**
     * \brief Get module rotation of current stereo profile
     * \details User needs to assign stereo profile first
     *
     * \param profile Profile of stereo camera, can be rear-rear or front-front
     *
     * \return 0, 90, 180, 270 clockwise degree
     * \see stereoProfile()
     */
    static ENUM_ROTATION getModuleRotation(ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Get sensor relative position
     * \return 0: main-main2 (main in L)
     *         1: main2-main (main in R)
     */
    static ENUM_STEREO_SENSOR_RELATIVE_POSITION getSensorRelativePosition(ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Query if the sensor is AF or FF
     *
     * \param SENSOR_INDEX sensor index
     * \return true if the sensor is AF
     */
    static bool isSensorAF(const int SENSOR_INDEX);

    /**
     * \brief Enable stereo log, each node can decide to log or not
     * \details This equals setprop vendor.STEREO.log 1
     * \return true if success
     * \see disableLog()
     */
    static bool enableLog();

    /**
     * \brief Enable log by a property
     * \details This equals setprop LOG_PROPERTY_NAME 1
     *
     * \param LOG_PROPERTY_NAME Log property to enable
     * \return true if success
     */
    static bool enableLog(const char *LOG_PROPERTY_NAME);

    /**
     * \brief Disable stereo log, each node can decide to log or not
     * \details This equals setprop vendor.STEREO.log 1
     * \return true if success
     * \see enableLog()
     */
    static bool disableLog();   //Globally disable log

    /**
     * \brief Check if global log is enabled
     * \details This equals getprop vendor.STEREO.log
     * \return true if log is enabled
     */
    static bool isLogEnabled();

    /**
     * \brief Check if a log property is enabled or not
     * \details This equals getprop LOG_PROPERTY_NAME
     *
     * \param LOG_PROPERTY_NAME The log property to check
     * \return true if log is enabled
     */
    static bool isLogEnabled(const char *LOG_PROPERTY_NAME);   //Check log status of each node, refers to global log switch

    /**
     * \brief Check is profile log is enabled or not
     * \details This equals getprop vendor.STEREO.Profile
     * \return true if profile log is enabled
     */
    static bool isProfileLogEnabled(); //Check if global profile log is enabled

    /**
     * \brief Get maximum extra data buffer size
     * \details We provides this API for AP to pass extra data buffer to middleware,
     *          NOTICE: if we need to add new data to extra data, or change current data size,
     *                  we need to review the returned size again.
     * \return Estimated size of extra data
     */
    static MUINT32 getExtraDataBufferSizeInBytes();

    /**
     * \brief Get MAX size of warping matrix output by N3D HAL
     * \details This size is for each sensor
     * \return Maxium warping matrix in bytes
     */
    static MUINT32 getMaxWarpingMatrixBufferSizeInBytes();

    /**
     * \brief Get MAX size of Scene Info output by N3D HAL
     * \details This size is for each sensor
     * \return Maxium B+M High Resolution in bytes
     */
    static MUINT32 getMaxSceneInfoBufferSizeInBytes();

    /**
     * \brief Provides stereo picture size
     * \details This API provides picture size for AP to save image.
     *          The size is encoded in string.
     *
     * \param param Output parameters
     * \return true if success
     */
    static bool getStereoParams(STEREO_PARAMS_T &param);

    /**
     * \brief Get baseline of current profile, unit: cm
     * \details Should set stereo profile before using this API
     *
     * \param profile profile to get baseline
     * \return baseline of current profile, unit: cm
     */
    static float getStereoBaseline(ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Get sensor output format in raw domain
     * \details User should power on the sensor before using this API
     * \param SENSOR_INDEX Sensor index, 0 for main sensor
     * \return enum
     *         {
     *             SENSOR_RAW_Bayer = 0x0,
     *             SENSOR_RAW_MONO,
     *             SENSOR_RAW_RWB,
     *             SENSOR_RAW_FMT_NONE = 0xFF,
     *         };
     * \see getStereoSensorIndex
     */
    static MUINT getSensorRawFormat(const int SENSOR_INDEX);

    /**
     * \brief Check if stereo camera is Bayer+Mono
     * \details Mono sensor will be main2, therefore we don't need to pass sensor index
     * \return true if stereo camera is Bayer+Mono
     */
    static bool isBayerPlusMono();

    /**
     * \brief Check if current feature is Bayer+Mono VSDoF
     * \details Condition: 1. B+M denoise is off 2. Main2 is mono sensor
     * \return true if Bayer+Mono VSDoF is enabled
     */
    static bool isBMVSDoF();

    /**
     * \brief Check if current feature use dual camera
     * \details Dual camera feature excludes PIP and includes zoom, vsdof, denoise, etc
     * \return true if it is dual cam
     */
    static bool isDualCamMode();

    /**
     * \brief Set stereo profile
     * \details There could be more than one set of stereo camera on the device,
     *          different profile has different sensor id, baseline, FOV, sizes, etc.
     *
     * \param profile Stereo profile
     */
    static void setStereoProfile(ENUM_STEREO_SENSOR_PROFILE profile);

    /**
     * \brief Set stereo profile by opened sensor device
     * \details There could be more than one set of stereo camera on the device,
     *          different profile has different sensor id, baseline, FOV, sizes, etc.
     *
     * \param sensorDev Sensor device id
     * \param sensorNum Total sensors
     */
    static void setStereoProfile(const int32_t sensorDev, const int32_t sensorNum);

    /**
     * \brief Set image ratio
     *
     * \param ratio Image ratio
     */
    static void setImageRatio(ENUM_STEREO_RATIO ratio);

    /**
     * \brief Set stereo feature mode and portrait mode
     * \details Stereo feature mode can be combination of ENUM_STEREO_FEATURE_MODE, default is -1
     *
     * \param stereoMode stereo feature mode
     * \param isPortrait true for portrait mode, default is false
     *                   NOTICE: only vsdof and mtkdepthmap support portrait mode now
     */
    static void setStereoFeatureMode(MINT32 stereoMode, bool isPortrait=false);

    /**
     * \brief Get crop ratio if sensor FOV is different to target FOV
     * \return Crop ratio, default is 1.0f
     */
    static float getFOVCropRatio();

    /**
     * \brief Get current shot mode
     * \return Shot mode, currenly only eShotMode_ZsdShot is returned
     */
    static NSCam::EShotMode getShotMode();

    /**
     * \brief Set current sensor module type in stereo mode
     */
    static void setStereoModuleType(MINT32 moduleType);

    /**
     * \brief Get current sensor module type
     * \return return stereo module type
     */
    static int getStereoModuleType() {return m_stereoModuleType;}

    /**
     * \brief Get current feature type
     * \return return stereo feature type
     */
    static int getStereoFeatureMode() {return m_stereoFeatureMode;}

#if (20 == STEREO_HAL_VER)
    /**
     * \brief Get warping buffer for N3D
     *
     * \param sensor Sensor to get
     *
     * \return WARPING_BUFFER_CONFIG_T, if user want to get size of each plan in byte, please call WARPING_BUFFER_CONFIG_T::getPlaneSizeInBytes()
     */
    static WARPING_BUFFER_CONFIG_T getWarpingBufferConfig(StereoHAL::ENUM_STEREO_SENSOR sensor);
#endif

    /**
     * \brief get stereo shot mode
     *
     * \return return current stereo shot mode
     */
    static MUINT getStereoShotMode() {return m_stereoShotMode;}

    /**
     * \brief set stereo shot mode
     */
    static void setStereoShotMode(MUINT32 stereoShotMode);

    /**
     * \brief Get DPE round for capture
     * \return DPE round for capture
     */
    static size_t getDPECaptureRound();

    /**
     * \brief Get max N3D capture debug buffer in bytes
     * \details N3D debug buffer will pack several buffers into one, this API help to get the total size in bytes
     * \return Max N3D debug buffer size in bytes
     */
    static size_t getMaxN3DDebugBufferSizeInBytes();

    /**
     * \brief Check if the sensor combination is Wide+Tele (by FOV diff)
     *
     * \param profile The stereo set to query
     * \return true if the sensor combination is Wide+Tele
     */
    static bool isWidePlusTele(ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Check if Wide+Tele VSDoF
     *
     * \param profile The stereo set to query
     * \return true if Wide+Tele VSDoF
     */
    static bool isWideTeleVSDoF(ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());

    /**
     * \brief Get sensor scenario
     * \details Use current parameters to query and update sensor scenarios
     *
     * \param stereoMode Stereo Mode, such as VSDoF, Denoise, etc
     * \param sensorModuleType B+B or B+M
     * \param pipelineMode Photo mode(Capture+Preview) or Record mode
     *
     * \param sensorScenarioMain1 Updated sensor scenario of main1
     * \param sensorScenarioMain2 Updated sensor scenario of main2
     */
    static bool getSensorScenario(MINT32 stereoMode,
                                  MINT32 sensorModuleType,
                                  MINT32 pipelineMode,
                                  //Below are output
                                  MUINT &sensorScenarioMain1,
                                  MUINT &sensorScenarioMain2);

    /**
     * \brief Update sensor scenario
     * \details Update result from getSensorScenario
     *
     * \param sensorScenarioMain1 Updated sensor scenario of main1
     * \param sensorScenarioMain2 Updated sensor scenario of main2
     */
    static void updateSensorScenario(MUINT sensorScenarioMain1,
                                     MUINT sensorScenarioMain2);

    /**
     * \brief Get current sensor scenario of main1
     * \details Need to call updateSensorScenario to update before this
     *
     * \return current sensor scenario of main1
     */
    static MUINT getSensorScenarioMain1() { return __sensorScenarioMain1; }

    /**
     * \brief Get current sensor scenario of main2
     * \details Need to call updateSensorScenario to update before this
     *
     * \return current sensor scenario of main2
     */
    static MUINT getSensorScenarioMain2() { return __sensorScenarioMain2; }

    /**
     * \brief Get hardware FEO size
     *
     * \param inputSize Center cropped input image size, can query from pass2 size in size providers
     * \param blockSize Get from fefmBlockSize
     * \param feoSize Output size of
     * \return size of buffer in byes
     */
    static size_t queryHWFEOBufferSize(NSCam::MSize inputSize, MUINT blockSize, NSCam::MSize &feoSize)
    {
        feoSize.w = inputSize.w/blockSize*40;
        feoSize.h = inputSize.h/blockSize;
        return feoSize.w * feoSize.h;
    }

    /**
     * \brief Get hardware FMO size
     *
     * \param feoSize get from queryHWFEOBufferSize
     * \param fmoSize fmo buffer size
     *
     * \return size of buffer in bytes
     */
    static size_t queryHWFMOBufferSize(NSCam::MSize feoSize, NSCam::MSize &fmoSize)
    {
        fmoSize.w = (feoSize.w/40) * 2;
        fmoSize.h = feoSize.h;

        return fmoSize.w * fmoSize.h;
    }

#if (16 == STEREO_HAL_VER || 17 == STEREO_HAL_VER || STEREO_HAL_VER >= 30)
    /**
     * \brief Set current VSDoF scenario
     * \details Should be only set by middleware
     *
     * \param scenario Scenario to set, enum is defined in camera_custom_stereo.h
     */
    static void setVSDoFScenario(ENUM_STEREO_CAM_SCENARIO scenario)
    {
        __vsdofScenario = scenario;
    }

    /**
     * \brief Get current VSDoF scenario
     * \details Get the vsdof scenario set by middlware
     * \return Scenario to get, enum is defined in camera_custom_stereo.h
     */
    static ENUM_STEREO_CAM_SCENARIO getVSDoFScenario()
    {
        return __vsdofScenario;
    }

    /**
     * \brief Get callback buffers to AP in brief string
     * \details Buffers:
     *      ci: Clean Image
     *      bi:Bokeh Image
     *      mbd: MTK Bokeh Depth
     *      mdb: MTK Debug Buffer
     *      mbm: MTK Bokeh Metadata
     * \return Callback buffer list in string, e.g. ci,bi,mbd,mdb,mbm
     */
    static std::string getCallbackBufferList();
#endif

#if (17 == STEREO_HAL_VER || STEREO_HAL_VER >= 30)
    /**
     * \brief Get main1 sensor scenario
     * \details Use current parameters to query and update sensor scenarios
     *
     * \param stereoMode Stereo Mode, such as VSDoF, Denoise, etc
     * \param pipelineMode Photo mode(Capture+Preview) or Record mode
     *
     * \param sensorScenarioMain1 Updated sensor scenario of main1
     */
    static bool getMain1SensorScenario(MINT32 stereoMode,
                                       MINT32 pipelineMode,
                                       //Below are output
                                       MUINT &sensorScenarioMain1);

    /**
     * \brief Get main2 sensor scenario
     * \details Use current parameters to query and update sensor scenarios
     *
     * \param stereoMode Stereo Mode, such as VSDoF, Denoise, etc
     * \param pipelineMode Photo mode(Capture+Preview) or Record mode
     *
     * \param sensorScenarioMain2 Updated sensor scenario of main2
     */
    static bool getMain2SensorScenario(MINT32 stereoMode,
                                       MINT32 pipelineMode,
                                       //Below are output
                                       MUINT &sensorScenarioMain2);

    /**
     * \brief Update main1 sensor scenario
     * \details Update result from getSensorScenario
     *
     * \param main1SensorScenario Updated sensor scenario of main1
     */
    static void updateMain1SensorScenario(MUINT main1SensorScenario);

    /**
     * \brief Update main2 sensor scenario
     * \details Update result from getSensorScenario
     *
     * \param main1SensorScenario Updated sensor scenario of main1
     */
    static void updateMain2SensorScenario(MUINT main2SensorScenario);

    /**
     * \brief Get Main1 FOV crop ratio
     * \details Must get after set stereo profile and feature mode
     * \return FOV crop ratio, 1.0 if no crop
     */
    static float getMain1FOVCropRatio();

    /**
     * \brief Get Main2 FOV crop ratio
     * \details Must get after set stereo profile and feature mode
     * \return FOV crop ratio, 1.0 if no crop
     */
    static float getMain2FOVCropRatio();
#endif

#if (17 == STEREO_HAL_VER || 30 == STEREO_HAL_VER)
    static void setPreviewSize(NSCam::MSize pvSize);

    static size_t getCalibrationData(void *calibrationData, ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());
#endif

#if (30 == STEREO_HAL_VER)
    static size_t getWarpMapSizeInBytes();
#endif

    /**
     * \brief Get dump path for ISP 4.x, file will dump to /sdcard/vsdof/
     *
     * \param isCapture If capture, then dump to cature folder, otherwise dump to preview folder
     * \param timestamp timestamp of the dump
     * \param request pipeline request number
     * \return full path of dump folder, will create the folder before return
     */
    static std::string getDumpFolder(bool isCapture, std::time_t timestamp, int request)
    {
        std::ostringstream oss;
        if(isCapture) {
            oss << "/sdcard/vsdof/capture/";
        } else {
            oss << "/sdcard/vsdof/preview/";
        }

        oss << __featureStartTime << "/" << timestamp << "_" << request;

        NSCam::Utils::makePath(oss.str().c_str(), 0660);
        return oss.str();
    }

    //For UT or debugging, suggest to enable when UT
    static void enableTestMode() { m_isTestMode = true; }
    static bool isTestMode() { return m_isTestMode; }

protected:

private:
    static void _updateImageSettings();
    static bool _getOriginalSensorIndex(int32_t &main1Idx, int32_t &main2Idx, ENUM_STEREO_SENSOR_PROFILE profile=stereoProfile());
    static std::string _getStereoFeatureModeString(int stereoMode);
    static std::string __getStereoProfileString(ENUM_STEREO_SENSOR_PROFILE profile)
    {
        std::string profileStr;
        switch(profile) {
            case STEREO_SENSOR_PROFILE_UNKNOWN:
            default:
                profileStr = "Unknown";
            break;
            case STEREO_SENSOR_PROFILE_REAR_REAR:
                profileStr = "Rear+Rear";
            break;
            case STEREO_SENSOR_PROFILE_FRONT_FRONT:
                profileStr = "Front+Front";
            break;
            case STEREO_SENSOR_PROFILE_REAR_FRONT:
                profileStr = "Rear+Front";
            break;
        }

        return profileStr;
    }

private:
    static ENUM_STEREO_SENSOR_PROFILE   m_stereoProfile;
    static ENUM_STEREO_RATIO            m_imageRatio;
    static MINT32                       m_stereoFeatureMode;
    static MINT32                       m_stereoModuleType;
    static MUINT32                      m_stereoShotMode;
    static bool                         m_isTestMode;

    static MUINT                        __sensorScenarioMain1;
    static MUINT                        __sensorScenarioMain2;

    static MUINT32                      __featureStartTime;

#if (16 == STEREO_HAL_VER || 17 == STEREO_HAL_VER || STEREO_HAL_VER >= 30)
    static ENUM_STEREO_CAM_SCENARIO     __vsdofScenario;
#endif
};

#endif  // _STEREO_SETTING_PROVIDER_H_

