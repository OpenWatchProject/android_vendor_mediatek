#ifndef _MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSOR_TYPE_H_
#define _MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSOR_TYPE_H_

namespace NSCam {
namespace Utils {

struct SensorData
{
    SensorData()
        : acceleration{0}
        , gyro{0}
        , light(0)
        , timestamp(0){}
    MFLOAT acceleration[3];
    MFLOAT gyro[3];
    MFLOAT light;
    MINT64 timestamp;
};

enum eSensorType
{
    SENSOR_TYPE_GYRO = 0,
    SENSOR_TYPE_ACCELERATION,
    SENSOR_TYPE_LIGHT,
    SENSOR_TYPE_COUNT,
};

enum eSensorStatus
{
    STATUS_UNINITIALIZED = 0,
    STATUS_INITIALIZED,
    STATUS_ERROR,
};

} // Utils
} // NSCam

#endif // _MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSOR_TYPE_H_

