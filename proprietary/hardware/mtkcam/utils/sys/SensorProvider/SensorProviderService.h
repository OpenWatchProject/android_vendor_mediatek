#ifndef _MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSOR_PROVIDER_SERVICE_H_
#define _MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSOR_PROVIDER_SERVICE_H_

#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include <mtkcam/utils/sys/sensor_type.h>
#include <string.h>
#include <vector>
#include <queue>

#include <ALooper.h>
#include <ASensorEventQueue.h>
#include <ASensorManager.h>

//-----------------------------------------------------------------------------
namespace NSCam {
namespace Utils {

class SensorProviderService : public android::RefBase
{
    friend class SensorProvider;
    friend class WorkThread;
private:
    struct SensorConfig
    {
        std::string  userName;
        void*        userAddr;
        eSensorType  type;
        MUINT32      interval;
    };

    struct SensorQueueInfo
    {
        MUINT32 index;
        MINT64  latestSerialNum;
    };

    static android::sp<SensorProviderService> getInstance();

    MVOID initThread();

    MBOOL isEnabled(const eSensorType sensorType);

    MBOOL enableSensor(const char* userName, void* userAddr, const eSensorType sensorType,
        const MUINT32 intervalInMs);

    MBOOL disableSensor(const char* userName, void* userAddr, const eSensorType sensorType);

    MBOOL getLatestSensorData(const eSensorType sensorType, SensorData& sensorData);

    MBOOL getAllSensorData(const eSensorType sensorType, std::vector<SensorData>& sensorData,
        MINT64& clientSerialNum);

private:
    using ISensorManager = android::frameworks::sensorservice::V1_0::ISensorManager;
    using IEventQueueCallback = android::frameworks::sensorservice::V1_0::IEventQueueCallback;
    using Return = android::hardware::Return<void>;
    using Event = android::hardware::sensors::V1_0::Event;
    using IEventQueue = android::frameworks::sensorservice::V1_0::IEventQueue;

    class WorkThread : public android::Thread
    {
    friend class SensorProviderService;
    public:
        WorkThread(const android::sp<SensorProviderService>& outer)
            : wpService(outer) {}
    private:
        android::status_t readyToRun();
        bool threadLoop();
        android::wp<SensorProviderService> wpService;
        std::queue<SensorConfig>  mWorkQueue;
        android::Condition mWorkCond;
        android::Mutex mWorkQueueLock;
    };

    class LoadingDebugger : public android::Thread
    {
    public:
        LoadingDebugger(int loading)
            : mLoading(loading)
            , mLastTime(0) {}
        int mLoading;
        MUINT64 mLastTime;
    private:
        bool threadLoop();
    };

    class SensorProviderListener : public IEventQueueCallback
    {
    public:
        SensorProviderListener(const android::sp<SensorProviderService>& outer)
            : wpService(outer) {}
        Return onEvent(const Event &e);
    private:
        android::wp<SensorProviderService> wpService;
    };

    struct SensorDeathRecipient : public android::hardware::hidl_death_recipient
    {
    public:
        SensorDeathRecipient(const android::sp<SensorProviderService>& outer)
            : wpService(outer) {}
        // hidl_death_recipient interface
        virtual void serviceDied(uint64_t cookie,
                const ::android::wp<::android::hidl::base::V1_0::IBase>& who) override;
    private:
        android::wp<SensorProviderService> wpService;
    };

    struct SensorContext
    {
        MINT32 mSensorHandle;
        eSensorStatus mSensorStatus;
        android::KeyedVector<void*, MUINT32> mActiveUsers;
        android::SortedVector<MUINT32>       mIntervals;
        MUINT32                     mCurrentInterval;
    };

    static const int SENSOR_QUEUE_SIZE    = 200; // Maximum sensor frequency is 200 Hz, keep 1 sec data

    static const int UNINITIALIZED_INTERVAL = 1000;

    SensorProviderService();
    virtual ~SensorProviderService();
    MBOOL configUserInfo(const SensorConfig& config);
    MVOID doEnableSensor(const SensorConfig& config);
    MBOOL doDisableSensor(const char* userName, void* userAddr, const eSensorType sensorType);
    MBOOL  isSupport(void);
    static bool setThreadPriority(int policy, int priority);
    static bool getThreadPriority(int& policy, int& priority);

    static android::Mutex sSingletonLock;
    android::Mutex        mDataLock;
    static android::Mutex sUserLock;

    static android::wp<SensorProviderService> sService;
    android::sp<WorkThread> mpThread;
    android::sp<LoadingDebugger> mpLoadingDebugger;
    android::sp<SensorProviderListener> mpListener;
    android::sp<ISensorManager> mpSensorManager;
    android::sp<IEventQueue>    mpEventQueue;
    SensorContext      mSensorContext[SENSOR_TYPE_COUNT];
    android::sp<SensorDeathRecipient> mpDeathRecipient;

    SensorData                        mSensorDataQueue[SENSOR_TYPE_COUNT][SENSOR_QUEUE_SIZE];
    SensorData                        mLatestSensorData[SENSOR_TYPE_COUNT];
    SensorQueueInfo                   mSensorQueueInfo[SENSOR_TYPE_COUNT];
    MINT32                            mLogLevel;
    MINT32                            mLoadingLevel;
    std::vector<SensorData>           mGyroHistory;
};

} //Utils
} //NSCAM

#endif // _MTKCAM_INCLUDE_MTKCAM_UTILS_SYS_SENSOR_PROVIDER_SERVICE_H_

