#ifndef _MTKCAM_UTILS_SYS_SENSOR_PROVIDER_H_
#define _MTKCAM_UTILS_SYS_SENSOR_PROVIDER_H_

#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <mtkcam/utils/sys/sensor_type.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>

namespace NSCam {
namespace Utils {
class SensorProviderService;

class SensorProvider : public android::RefBase
{
public:
    /** Create SensorProvider instance.
      */
    static android::sp<SensorProvider> createInstance(const char* user);

    /** Check whether sensor enabled successfully
      *   @sensorType: sensorType (Gyro/Acceleration)
      *   @This function is Thread-safe
      */
    MBOOL isEnabled(const eSensorType sensorType);

    /** Register and enable sensor
      *   @sensorType: sensorType (Gyro/Acceleration)
      *   @intervalInMs: frequency of sensor event callbacks
      *   @This function is Thread-safe
      */
    MBOOL enableSensor(const eSensorType sensorType, const MUINT32 intervalInMs);

    /** Unregister and disable sensor
      *   @sensorType: sonsorType (Gyro/Acceleration)
      *   @This function is Thread-safe
      */
    MBOOL disableSensor(const eSensorType sensorType);

    /** Get latest sensor data
      *   @sensorType: sensorType (Gyro/Acceleration)
      *   @sensorData: structure used for storing sensor data
      *   @This function is Thread-safe
      */
    MBOOL getLatestSensorData(const eSensorType sensorType, SensorData& sensorData);

    /** Get all sensor data in queue and clear queue
      *   @sensorType: sensorType (Gyro/Acceleration)
      *   @sensorData: vector of structure used for storing sensor data
      *   @This function is Thread-safe
      */
    MBOOL getAllSensorData(const eSensorType sensorType, std::vector<SensorData>& sensorData);

private:
    struct SensorState
    {
        SensorState()
            : interval(0)
            , serialNum(-1){}
        MUINT32     interval;
        MINT64      serialNum;
    };

    SensorProvider(const char* user);

    virtual ~SensorProvider();

    android::sp<SensorProviderService> mpSensorProviderService;
    std::string  mUser;
    std::map<eSensorType, SensorState> mSensorState;
    android::Mutex mLock;
};
} // Utils
} //NSCAM

#endif // _MTKCAM_UTILS_SYS_SENSOR_PROVIDER_H_

