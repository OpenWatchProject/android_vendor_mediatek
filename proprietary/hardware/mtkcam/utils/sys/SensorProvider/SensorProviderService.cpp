#define LOG_TAG "MtkCam/SensorProviderService"

#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Profile.h>
#include <utils/Trace.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>

#include "SensorProviderService.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace android;
using std::string;
using std::queue;
using std::vector;

using android::frameworks::sensorservice::V1_0::ISensorManager;
using android::frameworks::sensorservice::V1_0::Result;
using android::hardware::sensors::V1_0::SensorInfo;
using android::hardware::sensors::V1_0::SensorType;
using android::frameworks::sensorservice::V1_0::IEventQueueCallback;
using android::hardware::Return;
using android::hardware::sensors::V1_0::Event;
using android::frameworks::sensorservice::V1_0::IEventQueue;

#undef  ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS

namespace NSCam {
namespace Utils {

#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

#define IF_NULL_RETURN_VALUE(input, ret) \
    if (input == NULL) { \
        MY_LOGE("NULL value!"); \
        return ret; \
    }

#define SENSOR_PROVIDER_FOLDER_PATH "/data/vendor/camera_dump/sensorprovider"

Mutex         SensorProviderService::sSingletonLock;
Mutex         SensorProviderService::sUserLock;
wp<SensorProviderService> SensorProviderService::sService = NULL;

Return<void> SensorProviderService::SensorProviderListener::onEvent(const Event &e)
{
    sp<SensorProviderService> spService = wpService.promote();
    if (spService == NULL)
    {
        return android::hardware::Void();
    }

    int queueIndex = 0;
    eSensorType sensorType;
    SensorData sensorData;
    sensors_event_t sensorEvent;
    android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);

    Mutex::Autolock lock(spService->mDataLock);

    switch(e.sensorType)
    {
        case SensorType::GYROSCOPE:
            sensorData.gyro[0] = sensorEvent.gyro.x;
            sensorData.gyro[1] = sensorEvent.gyro.y;
            sensorData.gyro[2] = sensorEvent.gyro.z;
            sensorData.timestamp = sensorEvent.timestamp;

            sensorType = SENSOR_TYPE_GYRO;
            queueIndex = spService->mSensorQueueInfo[sensorType].index;
            spService->mLatestSensorData[sensorType] = sensorData;
            spService->mSensorDataQueue[sensorType][queueIndex] = sensorData;
            spService->mSensorQueueInfo[sensorType].index = (queueIndex + 1) % SENSOR_QUEUE_SIZE;
            ++spService->mSensorQueueInfo[sensorType].latestSerialNum;
            if (spService->mLogLevel >= 1)
            {
                std::vector<SensorData>& gyroHistory = spService->mGyroHistory;
                gyroHistory.push_back(sensorData);

                if (gyroHistory.size() % 1000 == 0)
                {
                    size_t calcNum = 1000 - 1;
                    vector<MFLOAT> intervalData(calcNum);
                    MFLOAT sumData[5] = {0};
                    MFLOAT debugData[5] = {0};
                    for (size_t i = 0; i < calcNum; i++)
                    {
                        intervalData[i] = gyroHistory[i+1].timestamp - gyroHistory[i].timestamp;
                        sumData[0] += gyroHistory[i].gyro[0];
                        sumData[1] += gyroHistory[i].gyro[1];
                        sumData[2] += gyroHistory[i].gyro[2];
                        sumData[3] += intervalData[i];
                    }
                    debugData[0] = sumData[0] / calcNum; // avg x
                    debugData[1] = sumData[1] / calcNum; // avg y
                    debugData[2] = sumData[2] / calcNum; // avg z
                    debugData[3] = sumData[3] / calcNum; // avg inteval

                    for (size_t i = 0; i < calcNum; i++)
                    {
                        sumData[4] += pow(intervalData[i] - debugData[3], 2);
                    }
                    debugData[4] = sqrt(sumData[4]/calcNum-1); // var inteval
                    FILE* pFp = fopen(SENSOR_PROVIDER_FOLDER_PATH"/GYRO_DEBUG.bin", "ab");
                    IF_NULL_RETURN_VALUE(pFp, android::hardware::Void());

                    fwrite(debugData, sizeof(debugData), 1, pFp);
                    fflush(pFp);
                    fclose(pFp);
                    gyroHistory.clear();
                }

                if (spService->mLogLevel >= 2)
                {
                    MY_LOGD("SensorType::GYROSCOPE time:%" PRId64 ", (x, y, z)=(%f, %f, %f)", sensorEvent.timestamp,
                        sensorEvent.gyro.x, sensorEvent.gyro.y, sensorEvent.gyro.z);
                }
            }
            break;

        case SensorType::ACCELEROMETER:
            sensorData.acceleration[0] = sensorEvent.acceleration.x;
            sensorData.acceleration[1] = sensorEvent.acceleration.y;
            sensorData.acceleration[2] = sensorEvent.acceleration.z;
            sensorData.timestamp = sensorEvent.timestamp;

            sensorType = SENSOR_TYPE_ACCELERATION;
            queueIndex = spService->mSensorQueueInfo[sensorType].index;
            spService->mLatestSensorData[sensorType] = sensorData;
            spService->mSensorDataQueue[sensorType][queueIndex] = sensorData;
            spService->mSensorQueueInfo[sensorType].index = (queueIndex + 1) % SENSOR_QUEUE_SIZE;
            ++spService->mSensorQueueInfo[sensorType].latestSerialNum;
            if (spService->mLogLevel >= 2)
            {
                MY_LOGD("SensorType::ACCELEROMETER time:%" PRId64 ", (x, y, z)=(%f, %f, %f)", sensorEvent.timestamp,
                    sensorEvent.acceleration.x, sensorEvent.acceleration.y, sensorEvent.acceleration.z);
            }
            break;

        case SensorType::LIGHT:
            sensorData.light = sensorEvent.light;
            sensorData.timestamp = sensorEvent.timestamp;

            sensorType = SENSOR_TYPE_LIGHT;
            queueIndex = spService->mSensorQueueInfo[sensorType].index;
            spService->mLatestSensorData[sensorType] = sensorData;
            spService->mSensorDataQueue[sensorType][queueIndex] = sensorData;
            spService->mSensorQueueInfo[sensorType].index = (queueIndex + 1) % SENSOR_QUEUE_SIZE;
            ++spService->mSensorQueueInfo[sensorType].latestSerialNum;
            if (spService->mLogLevel >= 2)
            {
                MY_LOGD("SensorType::LIGHT time:%" PRId64 ", light =%f", sensorEvent.timestamp, sensorEvent.light);
            }
            break;

        default:
            MY_LOGW("unknown type(%d)", e.sensorType);
            return android::hardware::Void();
    }

    return android::hardware::Void();
}

bool SensorProviderService::setThreadPriority(int policy, int priority)
{
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = priority;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, policy, &sched_p);
    }

    return true;
}

bool SensorProviderService::getThreadPriority(int& policy, int& priority)
{
    policy = ::sched_getscheduler(0);
    if (policy == SCHED_OTHER) {
        // a conventional process has only the static priority
        priority = ::getpriority(PRIO_PROCESS, 0);
    } else {
        // a real-time process has both the static priority and real-time priority.
        struct sched_param sched_p;
        ::sched_getparam(0, &sched_p);
        priority = sched_p.sched_priority;
    }

    return true;
}

sp<SensorProviderService> SensorProviderService::getInstance()
{
    Mutex::Autolock lock(sSingletonLock);

    sp<SensorProviderService> service = sService.promote();
    if (service == NULL)
    {
        service = new SensorProviderService();
        sService = service;

        service->initThread();
    }

    return service;
}

MVOID SensorProviderService::initThread()
{
    mpThread = new WorkThread(this);
    mpThread->run("SensorProviderService");

    if (mLoadingLevel >= 0)
    {
        mpLoadingDebugger = new LoadingDebugger(mLoadingLevel);
        mpLoadingDebugger->run("LoadingDebugger");
    }
}

status_t SensorProviderService::WorkThread::readyToRun()
{
    int policy = SCHED_OTHER, priority = 0;
    sp<SensorProviderService> service = wpService.promote();
    IF_NULL_RETURN_VALUE(service, UNKNOWN_ERROR);

    ::prctl(PR_SET_NAME,"Mtkcam@SensorProviderThread", 0, 0, 0);

    service->setThreadPriority(policy, priority);
    service->getThreadPriority(policy, priority);

    return NO_ERROR;
}

bool SensorProviderService::WorkThread::threadLoop()
{
    SensorConfig config;

    while (!Thread::exitPending())
    {
        {
            Mutex::Autolock lock(mWorkQueueLock);
            if (mWorkQueue.empty())
            {
                mWorkCond.wait(mWorkQueueLock);
                continue;
            }
            config = mWorkQueue.front();
            mWorkQueue.pop();
        }

        Mutex::Autolock lock(sUserLock);
        sp<SensorProviderService> service = wpService.promote();
        IF_NULL_RETURN_VALUE(service, false);
        if (!service->configUserInfo(config))
        {
            continue;
        }
        else
        {
            service->doEnableSensor(config);
        }
    }

    return false;
}

bool SensorProviderService::LoadingDebugger::threadLoop()
{
    char levelStr[20];
    MUINT32 level = 0;
    volatile MUINT64 sum = 0, loadingUnit = 1250000;

    while (!Thread::exitPending())
    {
        //auto mode, wait for 10 sec and then increase loading per 5 sec
        if (mLoading == 0)
        {
            if (mLastTime == 0)
            {
                mLastTime = getTimeInMs();
            }

            if (level == 0 && (getTimeInMs() - mLastTime >= 10 * 1000))
            {
                mLastTime = getTimeInMs();
                level++;
                sprintf(levelStr, "level=%d", level);
                ATRACE_BEGIN(levelStr);
                MY_LOGI("start producing loading %d", level);
            }
            else if (level != 0 && (getTimeInMs() - mLastTime >= 5 * 1000))
            {
                mLastTime = getTimeInMs();
                level++;
                ATRACE_END();
                MY_LOGI("change loading level to %d", level);
            }
        }
        else if (mLoading >= 1) //manual mode
        {
            level = mLoading;
            MY_LOGI("loading level is %d", level);
        }
        else
        {
            MY_LOGW("Should not be here!");
        }

        for (MUINT64 volatile m = 0; m < (level * loadingUnit); m++)
        {
            sum = m * (m + 1);
        }
        usleep(50 * 1000); // 50ms
    }

    return false;
}

SensorProviderService::SensorProviderService()
    : mpThread(NULL)
    , mpLoadingDebugger(NULL)
    , mpListener(NULL)
    , mpSensorManager(NULL)
    , mpEventQueue(NULL)
    , mpDeathRecipient(NULL)
    , mLogLevel(0)
    , mLoadingLevel(-1)
{
    MY_LOGD("SensorProviderService constructor");

    for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
    {
        mSensorContext[i].mSensorHandle = 0;
        mSensorContext[i].mSensorStatus = STATUS_UNINITIALIZED;
        mSensorContext[i].mCurrentInterval = UNINITIALIZED_INTERVAL;

        mSensorQueueInfo[i].index = 0;
        mSensorQueueInfo[i].latestSerialNum = -1;

        memset(mSensorDataQueue[i], 0, sizeof(SensorData) * SENSOR_QUEUE_SIZE);
        memset(&mLatestSensorData[i], 0, sizeof(SensorData));
    }

    mLogLevel = ::property_get_int32("debug.sensorprovider.dump", 0);
    mLoadingLevel = ::property_get_int32("debug.sensorprovider.loading", -1);

    if (mLogLevel >= 1)
    {
        if (mkdir(SENSOR_PROVIDER_FOLDER_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
        {
            MY_LOGW("mkdir %s failed", SENSOR_PROVIDER_FOLDER_PATH);
        }
    }
}

SensorProviderService::~SensorProviderService()
{
    mpThread->requestExit();
    mpThread->mWorkCond.signal();
    mpThread->join();

    if (mLoadingLevel >= 0)
    {
        mpLoadingDebugger->requestExit();
        mpLoadingDebugger->join();
        mpLoadingDebugger = NULL;
    }

    mpThread = NULL;
    mpListener = NULL;
    mpSensorManager = NULL;
    mpEventQueue = NULL;
    mpDeathRecipient = NULL;

    MY_LOGD("SensorProviderService destructor");
}

MBOOL SensorProviderService::isEnabled(const eSensorType sensorType)
{
    Mutex::Autolock lock(sUserLock);
    return mSensorContext[sensorType].mSensorStatus == STATUS_INITIALIZED;
}

MBOOL SensorProviderService::enableSensor(const char* userName, void* userAddr,
    const eSensorType sensorType, const MUINT32 intervalInMs)
{
    if (userName == NULL || userAddr == NULL || sensorType >= SENSOR_TYPE_COUNT || intervalInMs == 0)
    {
        MY_LOGW("Invalid parameter!");
        return false;
    }

    MY_LOGD("enableSensor user=%s type=%d interval=%d", userName, sensorType, intervalInMs);

    SensorConfig config;
    config.userName = userName;
    config.userAddr = userAddr;
    config.type = sensorType;
    config.interval = intervalInMs;

    Mutex::Autolock lock(mpThread->mWorkQueueLock);
    mpThread->mWorkQueue.push(config);
    mpThread->mWorkCond.signal();

    return true;
}

MBOOL SensorProviderService::configUserInfo(const SensorConfig& config)
{
    if (mSensorContext[config.type].mActiveUsers.indexOfKey(config.userAddr) >= 0)
    {
        MY_LOGD("user %s already enabled sensorType %d", config.userName.c_str(), config.type);
        return false;
    }
    else
    {
        mSensorContext[config.type].mActiveUsers.add(config.userAddr, config.interval);
        MY_LOGD("user %s added, sensorType %d", config.userName.c_str(), config.type);
        return true;
    }
}

void SensorProviderService::SensorDeathRecipient::serviceDied(
        uint64_t, const wp<::android::hidl::base::V1_0::IBase>&) {
    MY_LOGW("Sensor service died. Cleanup sensor manager instance!");
    Mutex::Autolock autoLock(sUserLock);

    sp<SensorProviderService> spService = wpService.promote();
    if (spService == NULL)
    {
        MY_LOGW("SensorProviderService is NULL!");
        return;
    }
    spService->mpSensorManager = NULL;
    spService->mpEventQueue = NULL;
}

MVOID SensorProviderService::doEnableSensor(const SensorConfig& config)
{
    if (!isSupport())
    {
        return;
    }

    android::hardware::sensors::V1_0::SensorType type = SensorType::GYROSCOPE;
    //HIDL process
    if (mSensorContext[config.type].mSensorStatus == STATUS_UNINITIALIZED)
    {
        mpSensorManager = ISensorManager::getService();

        switch(config.type)
        {
            case SENSOR_TYPE_GYRO:
                type = SensorType::GYROSCOPE;
                break;
            case SENSOR_TYPE_ACCELERATION:
                type = SensorType::ACCELEROMETER;
                break;
            case SENSOR_TYPE_LIGHT:
                type = SensorType::LIGHT;
                break;
            default:
                MY_LOGW("unknown type(%d)", config.type);
                break;
        }

        IF_NULL_RETURN_VALUE(mpSensorManager, void());
        if (mpDeathRecipient == NULL)
        {
            mpDeathRecipient = new SensorDeathRecipient(this);
            ::android::hardware::Return<bool> linked = mpSensorManager->linkToDeath(mpDeathRecipient, /*cookie*/ 0);
            if (!linked || !linked.isOk()) {
                MY_LOGE("Unable to link to sensor service death notifications");
                return;
            }
        }

        ::android::hardware::Return<void> result = mpSensorManager->getDefaultSensor(type,
            [&](const SensorInfo& sensor, Result ret) {
                if (ret == Result::OK)
                {
                    mSensorContext[config.type].mSensorHandle = sensor.sensorHandle;
                }
                else
                {
                    mSensorContext[config.type].mSensorStatus = STATUS_ERROR;
                }
            });

        if (!result.isOk() || mSensorContext[config.type].mSensorStatus == STATUS_ERROR)
        {
            MY_LOGI("Unable to get Default Sensor... do nothing and return");
            return;
        }
    }

    if (mpListener == NULL)
    {
        //create SensorEventQueue and register callback
        mpListener = new SensorProviderListener(this);
        IF_NULL_RETURN_VALUE(mpSensorManager, void());

        ::android::hardware::Return<void> result = mpSensorManager->createEventQueue(mpListener,
            [&](const sp<IEventQueue>& queue, Result ret) {
                ret == Result::OK ? mpEventQueue = queue : mpEventQueue = NULL;
            });

        if (!result.isOk() || mpEventQueue == NULL)
        {
            MY_LOGE("createEventQueue FAIL!");
            return;
        }
    }

    if (config.interval < mSensorContext[config.type].mCurrentInterval)
    {
        IF_NULL_RETURN_VALUE(mpEventQueue, void());
        ::android::hardware::Return<Result> ret = mpEventQueue->enableSensor(mSensorContext[config.type].mSensorHandle, config.interval * 1000, 0 /* latency */);
        if (!ret.isOk())
        {
            MY_LOGE("enable Sensor FAIL!");
            return;
        }
        if (mLogLevel >= 2)
        {
            MY_LOGD("Sensor enabled, interval=%d ms, type=%d", config.interval, config.type);
        }
        mSensorContext[config.type].mCurrentInterval = config.interval;
        mSensorContext[config.type].mSensorStatus = STATUS_INITIALIZED;
    }
}

MBOOL SensorProviderService::disableSensor(const char* userName, void* userAddr,
    const eSensorType sensorType)
{
    if (userName == NULL || userAddr == NULL || sensorType >= SENSOR_TYPE_COUNT)
    {
        MY_LOGW("Invalid parameter");
        return false;
    }

    return doDisableSensor(userName, userAddr, sensorType);
}

MBOOL SensorProviderService::doDisableSensor(const char* userName, void* userAddr,
    const eSensorType sensorType)
{
    IF_NULL_RETURN_VALUE(mpEventQueue, UNKNOWN_ERROR);

    Mutex::Autolock lock(sUserLock);

    if (mSensorContext[sensorType].mActiveUsers.indexOfKey(userAddr) < 0)
    {
        MY_LOGW("user %s not exist", userName);
        return false;
    }

    if (mSensorContext[sensorType].mActiveUsers.indexOfKey(userAddr) >= 0)
    {
        mSensorContext[sensorType].mActiveUsers.removeItem(userAddr);
        MY_LOGD("user %s removed, sensorType = %d", userName, sensorType);
    }

    IF_NULL_RETURN_VALUE(mpEventQueue, false);

    // Disable sensor if necessary
    if (mSensorContext[sensorType].mActiveUsers.size() == 0)
    {
        ::android::hardware::Return<Result> ret = mpEventQueue->disableSensor(mSensorContext[sensorType].mSensorHandle);
        if (!ret.isOk())
        {
            MY_LOGE("disable sensor fail");
        }
        mSensorContext[sensorType].mSensorStatus = STATUS_UNINITIALIZED;
        mSensorContext[sensorType].mSensorHandle = 0;
        mSensorContext[sensorType].mCurrentInterval = UNINITIALIZED_INTERVAL;

        mSensorQueueInfo[sensorType].index = 0;
        mSensorQueueInfo[sensorType].latestSerialNum = -1;

        memset(mSensorDataQueue[sensorType], 0, sizeof(SensorData) * SENSOR_QUEUE_SIZE);
        memset(&mLatestSensorData[sensorType], 0, sizeof(SensorData));
    }
    else
    {
        mSensorContext[sensorType].mIntervals.clear();
        for (size_t i = 0; i < mSensorContext[sensorType].mActiveUsers.size(); i++)
        {
            mSensorContext[sensorType].mIntervals.add(mSensorContext[sensorType].mActiveUsers.valueAt(i));
        }

        if (mSensorContext[sensorType].mIntervals.itemAt(0) > mSensorContext[sensorType].mCurrentInterval)
        {
            ::android::hardware::Return<Result> ret = mpEventQueue->enableSensor(mSensorContext[sensorType].mSensorHandle,
                    mSensorContext[sensorType].mIntervals.itemAt(0) * 1000 , 0 /* latency */);
            if (!ret.isOk())
            {
                MY_LOGE("change sensor rate FAIL!");
                return false;
            }
            mSensorContext[sensorType].mCurrentInterval = mSensorContext[sensorType].mIntervals.itemAt(0);
        }
    }

    return true;
}

MBOOL SensorProviderService::getLatestSensorData(const eSensorType sensorType, SensorData& sensorData)
{
    if (sensorType >= SENSOR_TYPE_COUNT)
    {
        MY_LOGW("unknown sensor type %d", sensorType);
        return false;
    }

    Mutex::Autolock lock(mDataLock);

    if (mLatestSensorData[sensorType].timestamp == 0)
    {
        return false;
    }

    memcpy(&sensorData, &mLatestSensorData[sensorType], sizeof(SensorData));
    return true;
}

MBOOL SensorProviderService::getAllSensorData(const eSensorType sensorType,
    vector<SensorData>& sensorData, MINT64& clientSerialNum)
{
    if (sensorType >= SENSOR_TYPE_COUNT)
    {
        MY_LOGW("unknown sensor type %d", sensorType);
        return false;
    }

    Mutex::Autolock lock(mDataLock);

    MINT64 latestSerialNum = mSensorQueueInfo[sensorType].latestSerialNum;
    MINT64 serialDiff = latestSerialNum - clientSerialNum;
    MUINT32 index = mSensorQueueInfo[sensorType].index + SENSOR_QUEUE_SIZE - 1;

    if (serialDiff == 0) //no available data
    {
        return false;
    }

    if (serialDiff < SENSOR_QUEUE_SIZE)
    {
        for (MINT32 i = 0; i < serialDiff; i++)
        {
            sensorData.push_back(mSensorDataQueue[sensorType][index % SENSOR_QUEUE_SIZE]);
            --index;
        }
    }
    else
    {
        for (MINT32 i = 0; i < SENSOR_QUEUE_SIZE; i++)
        {
            sensorData.push_back(mSensorDataQueue[sensorType][index % SENSOR_QUEUE_SIZE]);
            --index;
        }
    }

    clientSerialNum = latestSerialNum;

    return true;
}

MBOOL SensorProviderService::isSupport()
{
    int fd;
    ssize_t s;
    char boot_mode[4] = {'0'};
    //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
    //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5
    fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
    if (fd < 0)
    {
        MY_LOGE("fail to open: %s", "/sys/class/BOOT/BOOT/boot/boot_mode");
        return false;
    }

    s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
    close(fd);

    if(s <= 0)
    {
        MY_LOGE("could not read boot mode sys file");
        return false;
    }

    boot_mode[s] = '\0';
    if( atoi(boot_mode) == 1 ||
        atoi(boot_mode) == 4)
    {
        MY_LOGD("Boot Mode %d, return false",atoi(boot_mode));
        return false;
    }
    else
    {
        return true;
    }
}
} //Utils
} //NSCAM

