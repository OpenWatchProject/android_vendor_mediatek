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
#ifndef STEREO_SETTING_PROVIDER_KERNEL_H_
#define STEREO_SETTING_PROVIDER_KERNEL_H_

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <utils/Mutex.h>
#include <map>
#include <climits>

#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>

#include "stereo_setting_def.h"

#include <mtkcam3/feature/stereo/hal/FastLogger.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/mem/cam_cal_drv.h> //For CAM_CAL_DATA_STRUCT
#include <mtkcam/aaa/IHal3A.h>
// #include <af_param.h>          // For AF_ROI_SEL_FD
#include "fov_crop_util.h"

using namespace std;
using namespace StereoHAL;
using namespace NSCam;
using namespace NS3Av3;
using namespace android;
using android::Mutex;

#define PROPERTY_ENABLE_CUSTOM_SETTING  STEREO_PROPERTY_PREFIX"custom_setting"
#define PROPERTY_EXPORT_SETTING         STEREO_PROPERTY_PREFIX"custom_setting.exp"  //write SETTING_HEADER_PATH
#define PROPERTY_SETTING_LOG            PROPERTY_ENABLE_LOG".setting"

class StereoSettingProviderKernel
{
public:
    static StereoSettingProviderKernel *getInstance();
    static void destroyInstance();

    void init();

    StereoSensorSetting_T *getSensorSetting(int sensorIndex);
    StereoSensorConbinationSetting_T *getSensorCombinationSetting(MUINT32 logicalDeviceID=StereoSettingProvider::getLogicalDeviceID());
    string getCallbackBufferListString() { return __callbackBufferListString; }
    NSCam::EImageFormat getDepthmapFormat() { return __depthmapFormat; }
    std::vector<float> getLensPoseTranslation(int32_t sensorId);
    void logSettings();

    MUINT32 getMain2OutputFrequecy(ENUM_STEREO_SCENARIO scenario)
    {
        if(eSTEREO_SCENARIO_CAPTURE == scenario) {
            return __main2OutputFrequency[1];
        }

        return __main2OutputFrequency[0];
    }

protected:
    StereoSettingProviderKernel();
    virtual ~StereoSettingProviderKernel();

private:
    void __reset();

    void __parseDocument(json &jsonObj);
    void __loadSettingsFromSensorHAL();
    void __loadSettingsFromCalibration();

    void __saveSettingToFile(KeepOrderJSON &jsonObj);

private:
    static Mutex    __instanceLock;
    static StereoSettingProviderKernel *__instance;
    const bool LOG_ENABLED = checkStereoProperty(PROPERTY_SETTING_LOG);

    FastLogger      __logger;

    // name -> setting
    // For duplicated sensors, the sensor index will append to name like "IMX386_MIPI_RAW:3"
    unordered_map<string, StereoSensorSetting_T>         __sensorSettings;
    unordered_map<int, StereoSensorConbinationSetting_T> __sensorCombinationSettings; // id -> setting
    unordered_map<string, vector<int32_t>>               __sensorNameMap;    //name -> sensor index list
    string                  __callbackBufferListString;
    NSCam::EImageFormat     __depthmapFormat=NSCam::eImgFmt_Y8;
    MUINT32                 __main2OutputFrequency[2] = {1, UINT_MAX};  //0: preview, 1: Capture
    size_t                  __sensorCount = 0;

    //Do not init with properties here since this instance will keep until camera is killed
    bool    __IS_CUSTOM_SETTING_ENABLED = false;
    bool    __IS_EXPORT_ENABLED         = false;

    json __json;
};

#endif