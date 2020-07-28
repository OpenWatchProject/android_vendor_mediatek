#define LOG_TAG "lsc_mgr_dft_log_helper"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include "LogHelper.h"

#define LSC_LOG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)

MVOID
NSIspTuning::LogHelper::
logSensorInfo(MUINT32 u4NumSensorModes, const SENSOR_RESOLUTION_INFO_T& rSensorResolution)
{
    if(MFALSE == m_bEnableLog) return;
    std::string sensorInfo = "";

    appendLogByVar(&sensorInfo, "u4NumSensorModes +++" , u4NumSensorModes);
    appendLogByVar(&sensorInfo, "i4SensorPreviewWidth" , rSensorResolution.u4SensorPreviewWidth );
    appendLogByVar(&sensorInfo, "i4SensorPreviewHeight", rSensorResolution.u4SensorPreviewHeight);
    appendLogByVar(&sensorInfo, "i4SensorCaptureWidth" , rSensorResolution.u4SensorCaptureWidth );
    appendLogByVar(&sensorInfo, "i4SensorCaptureHeight", rSensorResolution.u4SensorCaptureHeight);
    appendLogByVar(&sensorInfo, "i4SensorVideoWidth"   , rSensorResolution.u4SensorVideoWidth   );
    appendLogByVar(&sensorInfo, "i4SensorVideoHeight"  , rSensorResolution.u4SensorVideoHeight  );
    appendLogByVar(&sensorInfo, "i4SensorVideo1Width"  , rSensorResolution.u4SensorVideo1Width  );
    appendLogByVar(&sensorInfo, "i4SensorVideo1Height" , rSensorResolution.u4SensorVideo1Height );
    appendLogByVar(&sensorInfo, "i4SensorVideo2Width"  , rSensorResolution.u4SensorVideo2Width  );
    appendLogByVar(&sensorInfo, "i4SensorVideo2Height" , rSensorResolution.u4SensorVideo2Height );
    appendLogByVar(&sensorInfo, "i4SensorCustom1Width" , rSensorResolution.u4SensorCustom1Width );
    appendLogByVar(&sensorInfo, "i4SensorCustom1Height", rSensorResolution.u4SensorCustom1Height);
    appendLogByVar(&sensorInfo, "i4SensorCustom2Width" , rSensorResolution.u4SensorCustom2Width );
    appendLogByVar(&sensorInfo, "i4SensorCustom2Height", rSensorResolution.u4SensorCustom2Height);
    appendLogByVar(&sensorInfo, "i4SensorCustom3Width" , rSensorResolution.u4SensorCustom3Width );
    appendLogByVar(&sensorInfo, "i4SensorCustom3Height", rSensorResolution.u4SensorCustom3Height);
    appendLogByVar(&sensorInfo, "i4SensorCustom4Width" , rSensorResolution.u4SensorCustom4Width );
    appendLogByVar(&sensorInfo, "i4SensorCustom4Height", rSensorResolution.u4SensorCustom4Height);
    appendLogByVar(&sensorInfo, "i4SensorCustom5Width" , rSensorResolution.u4SensorCustom5Width );
    appendLogByVar(&sensorInfo, "i4SensorCustom5Height", rSensorResolution.u4SensorCustom5Height);

    LSC_LOG("Sensor info: %s", sensorInfo.c_str());
}

MVOID
NSIspTuning::LogHelper::
logSensorCropVariables(const SensorCropCalculateVars_T& cropVar)
{
    if(MFALSE == m_bEnableLog) return;
    std::string cropInfo = "";

    appendLogByVar(&cropInfo, "rwc",  floatToStr(cropVar.rwc));
    appendLogByVar(&cropInfo, "rhc",  floatToStr(cropVar.rhc));
    appendLogByVar(&cropInfo, "x2c_", floatToStr(cropVar.x2c_));
    appendLogByVar(&cropInfo, "y2c_", floatToStr(cropVar.y2c_));

    appendLogByVar(&cropInfo, "rws",  floatToStr(cropVar.rws));
    appendLogByVar(&cropInfo, "rhs",  floatToStr(cropVar.rhs));
    appendLogByVar(&cropInfo, "x2s_", floatToStr(cropVar.x2s_));
    appendLogByVar(&cropInfo, "y2s_", floatToStr(cropVar.y2c_));

    appendLogByVar(&cropInfo, "x2c__", floatToStr(cropVar.x2c__));
    appendLogByVar(&cropInfo, "y2c__", floatToStr(cropVar.y2c__));
    appendLogByVar(&cropInfo, "x_sc",  floatToStr(cropVar.x_sc));
    appendLogByVar(&cropInfo, "y_sc",  floatToStr(cropVar.y_sc));
    appendLogByVar(&cropInfo, "out_w", floatToStr(cropVar.out_w));
    appendLogByVar(&cropInfo, "out_h", floatToStr(cropVar.out_h));

    LSC_LOG("Crop info: %s", cropInfo.c_str());
}

MVOID
NSIspTuning::LogHelper::
logOnOff(const MBOOL& enableLog)
{
    m_bEnableLog = enableLog;
}

MVOID
NSIspTuning::LogHelper::
appendLogByVar(std::string *log, const std::string& varName, MUINT32 var)
{
    if(log == nullptr) return;
    if(log -> length() == 0)
        *log = *log + varName + " = " + std::to_string(var);
    else
        *log = *log + ", " + varName + " = " + std::to_string(var);
}

MVOID
NSIspTuning::LogHelper::
appendLogByVar(std::string *log, const std::string& varName, const std::string& var)
{
    if(log == nullptr) return;
    if(log -> length() == 0)
        *log = *log + varName + " = " + var;
    else
        *log = *log + ", " + varName + " = " + var;
}

std::string
NSIspTuning::LogHelper::
floatToStr(MFLOAT num)
{
    char buf[32] = "";
    sprintf(buf, "%3.3f", num);
    return buf;
}
