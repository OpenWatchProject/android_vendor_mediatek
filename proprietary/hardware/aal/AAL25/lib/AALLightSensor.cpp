#define LOG_TAG "AALLightSensor"

#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <android/sensor.h>
#include <fcntl.h>
#include <time.h>
#include "linux/hwmsensor.h"
#include "AALLightSensor.h"

using android::frameworks::sensorservice::V1_0::Result;
using android::frameworks::sensorservice::V1_0::IEventQueueCallback;
using android::hardware::Return;
using android::hardware::sensors::V1_0::Event;
using android::hardware::sensors::V1_0::SensorInfo;

using ::android::Mutex;
using ::android::status_t;

#define MILLI_TO_NANO(v) (static_cast<nsecs_t>(v) * static_cast<nsecs_t>(1000L * 1000L))
#define MILLI_TO_MICRO(v) (static_cast<nsecs_t>(v) * static_cast<nsecs_t>(1000L))

#define UNUSED_AAL(expr) do { (void)(expr); } while (0)

static millisec_t DEBOUNCE_TIME = 4000L;
static millisec_t SHORT_TERM_PERIOD = 100L;
static millisec_t LONG_TERM_PERIOD = 500L;

static Mutex gLock;

static int gDebouncedLux = -1;
static bool gIsEnabled = false;
static volatile bool gReconnectSensorService = false;
static ListenerCallback gListenerCallback;
static void *gListenerUser;

static millisec_t getTime()
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return static_cast<millisec_t>(time.tv_sec) * 1000L +
        static_cast<millisec_t>(time.tv_usec / 1000L);
}

class AALLightCallback : public IEventQueueCallback {
private:
    millisec_t mlogPrintTime;
    millisec_t mLastObservedLuxTime;
    int mShortTermAverageLux;
    int mLongTermAverageLux;
    millisec_t mDebounceLuxTime;
    int mNotifiedLux;

    void updateLuxValue(int lux)
    {
        millisec_t currentTime = getTime();

        if (mShortTermAverageLux < 0) {
            mShortTermAverageLux = lux;
            mLongTermAverageLux = lux;
            gDebouncedLux = lux;
        } else {
            millisec_t timeDelta = currentTime - mLastObservedLuxTime;
            if (timeDelta > 0) {
                mShortTermAverageLux =
                    (mShortTermAverageLux * SHORT_TERM_PERIOD + lux * timeDelta) /
                    (SHORT_TERM_PERIOD + timeDelta);
                mLongTermAverageLux =
                    (mLongTermAverageLux * LONG_TERM_PERIOD + lux * timeDelta) /
                    (LONG_TERM_PERIOD + timeDelta);
            }
        }

        mLastObservedLuxTime = currentTime;

        // Check brightening or darkening
        int brighteningThreshold = gDebouncedLux + gDebouncedLux / 5;     // * 1.2
        int darkeningThreshold = gDebouncedLux - (gDebouncedLux * 2 / 5); // * 0.6

        bool brightening, darkening;
        brightening = (mShortTermAverageLux > brighteningThreshold &&
            mLongTermAverageLux > brighteningThreshold);
        darkening = (mShortTermAverageLux < darkeningThreshold &&
            mLongTermAverageLux < darkeningThreshold);

        if (!(brightening || darkening)) {
            mDebounceLuxTime = currentTime;
            return;
        }

        // Check debounce time
        millisec_t debounceTime = mDebounceLuxTime + DEBOUNCE_TIME;
        if (currentTime < debounceTime)
            return;

        mDebounceLuxTime = currentTime;
        gDebouncedLux = mShortTermAverageLux;
    }

public:
    AALLightCallback() {
        mlogPrintTime = 0;
        mLastObservedLuxTime = 0;
        mShortTermAverageLux = -1;
        mLongTermAverageLux = -1;
        mDebounceLuxTime = 0;
        mNotifiedLux = -1;
    }

    void resetLuxValue()
    {
        mShortTermAverageLux = -1;
        mLongTermAverageLux = -1;
        mDebounceLuxTime = 0;
    }

    Return<void> onEvent(const Event &e) {
        sensors_event_t sensorEvent;
        android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);
        int debouncedLux;

        {
            Mutex::Autolock _l(gLock);
            int OriginalLux = (int)sensorEvent.data[0];

            if (gIsEnabled) {
                int newLux = gDebouncedLux;

                updateLuxValue(OriginalLux);

                millisec_t current = getTime();
                if (current - mlogPrintTime >= 5000) { // 5 sec
                    ALOGD("newLux = %d, [%d, %d] -> %d",
                        newLux, mShortTermAverageLux, mLongTermAverageLux, gDebouncedLux);
                        mlogPrintTime = current;
                }
            }
            debouncedLux = gDebouncedLux;
        }

        if (gListenerCallback != NULL && debouncedLux != mNotifiedLux) {
            gListenerCallback(gListenerUser, debouncedLux);
            mNotifiedLux = debouncedLux;
        }

        return android::hardware::Void();
    }
};

struct SensorDeathRecipient : public android::hardware::hidl_death_recipient
{
public:
    // hidl_death_recipient interface
    virtual void serviceDied(uint64_t cookie,
        const ::android::wp<::android::hidl::base::V1_0::IBase>& who){
        ALOGD("sensor service died");
        {
            Mutex::Autolock _l(gLock);
            gReconnectSensorService = true;
        }

        UNUSED_AAL(cookie);
        UNUSED_AAL(who);
    }
};

static sp<AALLightCallback> gpLightCallback;
static sp<SensorDeathRecipient> gpDeathRecipient;

AALLightSensor::AALLightSensor() :
    mSensorMgr(NULL),
    mSensorEventQueue(NULL),
    mLightSensorHandle(-1),
    mPrepareEnable(false)
{
    mContRunning = false;
}


AALLightSensor::~AALLightSensor()
{
    mContRunning = false;
    setEnabled(false);
    mWaitCond.broadcast();

    join();
}

void AALLightSensor::loadCustParameters(CustParameters &cust) {
    if (cust.isGood()) {
        unsigned long alsParam;

        if (cust.loadVar("AlsDebounceTime", &alsParam))
            DEBOUNCE_TIME = (long long)alsParam;
        if (cust.loadVar("AlsShortTermPeriod", &alsParam))
            SHORT_TERM_PERIOD = (long long)alsParam;
        if (cust.loadVar("AlsLongTermPeriod", &alsParam))
            LONG_TERM_PERIOD = (long long)alsParam;
    }
}

void AALLightSensor::setListener(ListenerCallback callback, void *user) {
    gListenerCallback = callback;
    gListenerUser = user;
}

bool AALLightSensor::isEnabled() {
    return gIsEnabled;
}

void AALLightSensor::setEnabled(bool enabled)
{
    ALOGD("AALLightSensor setEnabled %d-->%d", gIsEnabled, enabled);
    if (enabled != gIsEnabled) {
        Mutex::Autolock _l(gLock);

        if (enabled) {
            if (gpLightCallback != NULL)
            {
                gpLightCallback->resetLuxValue();
            }
        }

        int enableVal = (enabled ? 1 : 0);
        Result ret = Result::OK;
        if (enableVal == 1) {
            mPrepareEnable = true;
            if (getTid() == -1) {
                run("AALLightSensor");
            } else {
                if (mContRunning == true && (!gReconnectSensorService)) {
                    ret = mSensorEventQueue->enableSensor(mLightSensorHandle, 200 * 1000 /* sample period */, 0 /* latency */);
                    ALOGD("AALLightSensor Enabled ret=%d", ret);
                } else {
                    ALOGE("connect sensor service fail");
                }
            }
        } else {
            mPrepareEnable = false;
            if (getTid() == -1) {
                /* no action */
            } else {
                if (mContRunning == true && (!gReconnectSensorService)) {
                    ret = mSensorEventQueue->disableSensor(mLightSensorHandle);
                    ALOGD("AALLightSensor Disabled ret=%d", ret);
                } else {
                    ALOGE("connect sensor service fail");
                }
            }
        }

        if (ret == Result::OK) {
            gDebouncedLux = -1;
            gIsEnabled = enabled;
            mWaitCond.broadcast();
        }

    }
}

bool AALLightSensor::threadLoop()
{
INIT:
    mContRunning = false;
    mSensorMgr = NULL;
    mSensorEventQueue = NULL;
    mLightSensorHandle = -1;

    do
    {
        mSensorMgr = ISensorManager::tryGetService();
        if (mSensorMgr == NULL)
        {
            ALOGD("sensorservice is not ready\n");
            mContRunning = false;
            sleep(1);
        }
    } while (mSensorMgr == NULL);
    ALOGD("get SensorManager SUCCESS!\n");

    if (gpDeathRecipient == NULL)
    {
        gpDeathRecipient = new SensorDeathRecipient();
    }
    ::android::hardware::Return<bool> linked = mSensorMgr->linkToDeath(gpDeathRecipient, /*cookie*/ 0);
    if (!linked || !linked.isOk()) {
        ALOGD("Unable to link to sensor service death notifications");
        goto INIT;
    }

    mSensorMgr->getDefaultSensor((::android::hardware::sensors::V1_0::SensorType)SENSOR_TYPE_LIGHT,
        [&](const SensorInfo& sensor, Result ret) {
            ret == Result::OK ? mLightSensorHandle = sensor.sensorHandle : mLightSensorHandle = -1;
        });
    if (mLightSensorHandle == -1)
    {
        ALOGD("get DefaultSensor FAIL! %d", mLightSensorHandle);
        goto INIT;
    }

    if (gpLightCallback == NULL)
    {
        gpLightCallback = new AALLightCallback();
    }
    mSensorMgr->createEventQueue(gpLightCallback,
        [&](const sp<IEventQueue>& queue, Result ret) {
            ret == Result::OK ? mSensorEventQueue = queue : mSensorEventQueue = NULL;
        });
    if (mSensorEventQueue == NULL)
    {
        ALOGD("createEventQueue FAIL!");
        goto INIT;
    }

    if (mPrepareEnable == true)
    {
        //enable sensor
        if (mSensorEventQueue->enableSensor(mLightSensorHandle, 200 * 1000 , 0) != Result::OK)
        {
            ALOGD("enable Sensor FAIL!");
            goto INIT;
        }
    }

    gReconnectSensorService = false;
    mContRunning = true;

    while (mContRunning) {
        if (gReconnectSensorService)
        {
            goto INIT;
        }

        {
            Mutex::Autolock _l(gLock);

            if (gIsEnabled) {
                if (gDebouncedLux < 0)
                    mWaitCond.waitRelative(gLock, MILLI_TO_NANO(100));
                else
                    mWaitCond.waitRelative(gLock, MILLI_TO_NANO(200));
            } else {
                // 10 sec
                mWaitCond.waitRelative(gLock, MILLI_TO_NANO(10 * 1000));
            }
        }

    }

    return mContRunning;
}

