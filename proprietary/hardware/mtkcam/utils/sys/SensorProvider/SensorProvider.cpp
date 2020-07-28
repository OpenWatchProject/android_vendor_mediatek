#define LOG_TAG "MtkCam/SensorProvider"

#include <semaphore.h>
#include <fcntl.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <utils/Errors.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>

#include "SensorProviderService.h"
#include <mtkcam/utils/sys/SensorProvider.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace android;
using std::map;
using std::vector;

namespace NSCam {
namespace Utils {

#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

sp<SensorProvider> SensorProvider::createInstance(const char* user)
{
    if (user == NULL)
    {
        MY_LOGW("User ID can not be NULL");
        return NULL;
    }
    sp<SensorProvider> provider = new SensorProvider(user);
    return provider;
}

SensorProvider::SensorProvider(const char* user)
    : mUser(user)
{
    MY_LOGD("%s SensorProvider constructor", user);

    mpSensorProviderService = SensorProviderService::getInstance();
}

SensorProvider::~SensorProvider()
{
    Mutex::Autolock lock(mLock);
    if (mpSensorProviderService != NULL)
    {
        for (map<eSensorType, SensorState>::iterator it = mSensorState.begin(); it != mSensorState.end(); ++it)
        {
            try {
                mpSensorProviderService->disableSensor(mUser.c_str(), this, it->first);
            } catch (const std::ios_base::failure & e){
                MY_LOGE("%s: disableSensor failed", __FUNCTION__);
            }
        }
    }
    mpSensorProviderService = NULL;

    MY_LOGD("%s SensorProvider destructor", mUser.c_str());
}

MBOOL SensorProvider::isEnabled(const eSensorType sensorType)
{
    Mutex::Autolock lock(mLock);
    if (sensorType >= SENSOR_TYPE_COUNT)
    {
        MY_LOGW("Invalid sensorType %d", sensorType);
        return false;
    }

    if (mpSensorProviderService != NULL)
    {
        return mpSensorProviderService->isEnabled(sensorType);
    }
    else
    {
        MY_LOGE("mpSensorProviderService is NULL!");
        return false;
    }
}

MBOOL SensorProvider::enableSensor(const eSensorType sensorType, const MUINT32 intervalInMs)
{
    Mutex::Autolock lock(mLock);
    if ((intervalInMs == 0) || (sensorType >= SENSOR_TYPE_COUNT) || mSensorState.count(sensorType) == 1)
    {
        MY_LOGW("enableSensor failed! interval=%d, type=%d, enabled=%zu", intervalInMs, sensorType,
            mSensorState.count(sensorType));
        return false;
    }

    SensorState state;
    state.interval = intervalInMs;

    mSensorState[sensorType] = state;

    if (mpSensorProviderService != NULL)
    {
        return mpSensorProviderService->enableSensor(mUser.c_str(), this, sensorType, intervalInMs);
    }
    else
    {
        MY_LOGE("mpSensorProviderService is NULL!");
        return false;
    }
}

MBOOL SensorProvider::disableSensor(const eSensorType sensorType)
{
    Mutex::Autolock lock(mLock);
    if (sensorType >= SENSOR_TYPE_COUNT || mSensorState.count(sensorType) == 0)
    {
        MY_LOGW("disableSensor failed! type=%d, enabled=%zu", sensorType, mSensorState.count(sensorType));
        return false;
    }

    mSensorState.erase(sensorType);

    if (mpSensorProviderService != NULL)
    {
        return mpSensorProviderService->disableSensor(mUser.c_str(), this, sensorType);
    }
    else
    {
        MY_LOGE("mpSensorProviderService is NULL!");
        return false;
    }
}

MBOOL SensorProvider::getLatestSensorData(const eSensorType sensorType, SensorData& sensorData)
{
    Mutex::Autolock lock(mLock);
    if (sensorType >= SENSOR_TYPE_COUNT || mSensorState.count(sensorType) == 0)
    {
        MY_LOGW("getLatestSensorData failed! type=%d, enabled=%zu", sensorType, mSensorState.count(sensorType));
        return false;
    }

    if (mpSensorProviderService != NULL)
    {
        return mpSensorProviderService->getLatestSensorData(sensorType, sensorData);
    }
    else
    {
        MY_LOGE("mpSensorProviderService is NULL!");
        return false;
    }
}

MBOOL SensorProvider::getAllSensorData(const eSensorType sensorType, vector<SensorData>& sensorData)
{
    Mutex::Autolock lock(mLock);

    sensorData.clear();
    if (sensorType >= SENSOR_TYPE_COUNT || mSensorState.count(sensorType) == 0)
    {
        MY_LOGW("getAllSensorData failed! type=%d, enabled=%zu", sensorType, mSensorState.count(sensorType));
        return false;
    }

    if (mpSensorProviderService != NULL)
    {
        return mpSensorProviderService->getAllSensorData(sensorType, sensorData,
            mSensorState[sensorType].serialNum);
    }
    else
    {
        MY_LOGE("mpSensorProviderService is NULL!");
        return false;
    }
}
} //Utils
} //NSCAM

