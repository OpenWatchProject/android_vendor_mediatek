#ifndef _LSC_MGR_DEFAULT_LOG_HELPER_H_
#define _LSC_MGR_DEFAULT_LOG_HELPER_H_

#include <string>
#include <aaa_types.h>
#include "LscMgrStruct.h"

namespace NSIspTuning {
class LogHelper
{
public:
    LogHelper() : m_bEnableLog(MTRUE) {}
    ~LogHelper() {}

    MVOID           logSensorInfo(MUINT32 u4NumSensorModes, const SENSOR_RESOLUTION_INFO_T& rSensorResolution);
    MVOID           logSensorCropVariables(const SensorCropCalculateVars_T& cropVar);
    MVOID           logOnOff(const MBOOL& enableLog);

private:
    MVOID           appendLogByVar(std::string *log, const std::string& varName, MUINT32 var);
    MVOID           appendLogByVar(std::string *log, const std::string& varName, const std::string& var);
    std::string     floatToStr(MFLOAT num);

    MBOOL           m_bEnableLog;
};
};  // namespace NSIspTuning

#endif //_LSC_MGR_DEFAULT_LOG_HELPER_H_




