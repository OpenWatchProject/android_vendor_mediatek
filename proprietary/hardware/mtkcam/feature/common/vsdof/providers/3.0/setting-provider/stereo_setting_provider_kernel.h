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

//========= Using rapidjson ==========
#include <vsdof/hal/rapidjson/rapidjson.h>
#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/rapidjson/stringbuffer.h>
#include <vsdof/hal/rapidjson/document.h>     // rapidjson's DOM-style API
#include <vsdof/hal/rapidjson/prettywriter.h> // for stringify JSON
#include <vsdof/hal/rapidjson/filewritestream.h>
#include <vsdof/hal/rapidjson/filereadstream.h>
#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/extradata_def.h>
#include <vsdof/hal/rapidjson/error/en.h>     // RapidJSON's error msg
using namespace rapidjson;
//====================================

#include "stereo_setting_def.h"

#include <mtkcam/feature/stereo/hal/FastLogger.h>

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

class StereoSettingProviderKernel
{
public:
    static StereoSettingProviderKernel *getInstance();
    static void destroyInstance();

    void init();

    StereoSensorSetting_T *getSensorSetting(int sensorIndex);
    StereoSensorConbinationSetting_T *getSensorCombinationSetting(ENUM_STEREO_SENSOR_PROFILE profile);
    bool getSensorIndexesOfProfile(ENUM_STEREO_SENSOR_PROFILE profile, int &sensorIdx1, int &sensorIdx2);
    string getCallbackBufferListString() { return __callbackBufferListString; }
    void logSettings();

protected:
    StereoSettingProviderKernel();
    virtual ~StereoSettingProviderKernel();

private:
    void __reset();

    bool __loadStereoSettingsFromDocument(Document &document);
    void __parseDocument(Document &document);
    void __parseLDC(const Value &ldcValue, vector<float> &ldc);
    void __parseCalibration(const Value &calValue, vector<MUINT8> &calibration);
    MUINT __parseSensorScenario(const char *scenario);
    void __loadSettingsFromSensorHAL();
    void __loadSettingsFromCalibration();

    void __saveSettingToFile(Document &document);

private:
    static Mutex    __instanceLock;
    static StereoSettingProviderKernel *__instance;
    bool LOG_ENABLED = false;

    FastLogger      __logger;

    std::map<std::string, StereoSensorSetting_T>    __sensorSettings;
    std::vector<StereoSensorConbinationSetting_T>   __sensorCombinationSettings;
    std::string                                     __callbackBufferListString;
    std::map<std::string, int>                      __sensorNameMap;    //name -> sensor index

    Document __document;
};

#endif