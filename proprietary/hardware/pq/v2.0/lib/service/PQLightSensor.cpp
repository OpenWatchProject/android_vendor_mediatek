#define LOG_TAG "PQLightSensor"

#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/sensors_io.h>
#include "PQLightSensor.h"

#define MILLI_TO_NANO(v) (static_cast<nsecs_t>(v) * static_cast<nsecs_t>(1000L * 1000L))

PQLightSensor::PQLightSensor() :
    mIsEnabled(false),
    mIsSensorInited(false),
    mContRunning(false),
    mListenerCallback(NULL),
    mListenerUser(NULL),
    mCustPQSensorInitFun(NULL),
    mCustPQSensorDeInitFun(NULL),
    mCustPQSensorEnableFun(NULL),
    mPQSensorCallback(NULL),
    mPQDiedSensorCallback(NULL)

{
    mLib = dlopen("librgbwlightsensor.so", RTLD_GLOBAL);

    if (isRGBWLibGood()) {
        mCustPQSensorInitFun = getFunction<PQSensorInit>("pqLightSensorInit");
        mCustPQSensorDeInitFun = getFunction<PQSensorDeInit>("pqLightSensorDeInit");
        mCustPQSensorEnableFun = getFunction<PQSensorEnable>("pqLightSensorEnable");
        mPQSensorCallback = getFunction<PQSensorCallback>("pqSetListener");
        mPQDiedSensorCallback = getFunction<PQDiedSensorCallback>("pqSetDiedListener");

        mPQSensorCallback(sensorListener,this); // set "sensorListener" as sensor listener callback
        mPQDiedSensorCallback(diedSensorListener,this); // set "diedSensorListener" as sensor listener callback

        mContRunning = true;
    }
}


PQLightSensor::~PQLightSensor()
{
    setEnabled(false);

    if (mLib != NULL) {
        if (mIsSensorInited == true) {
            mCustPQSensorDeInitFun();
        }
        dlclose(mLib);
    }
    mWaitCond.broadcast();
    join();
}

void PQLightSensor::setEnabled(bool enabled)
{
    int ret;
    if (mContRunning != true) {
        ALOGD("setEnabled fail: mLib[%d] mContRunning[%d]", isRGBWLibGood(), mContRunning);
        return;
    }

    ALOGD("setEnabled %d-->%d", mIsEnabled, enabled);
    if (mIsEnabled != enabled) {
        Mutex::Autolock _l(mLock);

        if (getTid() == -1 && enabled == 1)
        {
            run("PQLightSensor");
        }
        else if (mIsSensorInited == true)
        {
            ret = mCustPQSensorEnableFun(enabled);
            if (ret != 1)
            {
                ALOGD("[PQLightSensor] setEnabled failed ret[%d]", ret);
            }
            else
            {
                mIsEnabled = enabled;
                mWaitCond.broadcast();
            }
        }
    }
}


void PQLightSensor::updateLuxValue(int alsR, int alsG, int alsB, int alsW)
{
    //ALOGD("[PQLightSensor] updateLuxValue alsR[%d] alsG[%d] alsB[%d] alsWR[%d]", alsR, alsG, alsB, alsW);
    if (mListenerCallback != NULL)
        mListenerCallback(mListenerUser, alsR, alsG, alsB, alsW);
}


void PQLightSensor::sensorListener(void* obj, int alsR, int alsG, int alsB, int alsW)
{
    //ALOGD("[PQLightSensor] sensorListener alsR[%d] alsG[%d] alsB[%d] alsWR[%d]", alsR, alsG, alsB, alsW);
    PQLightSensor *service = static_cast<PQLightSensor*>(obj);
    service->updateLuxValue(alsR, alsG, alsB, alsW);
}

void PQLightSensor::diedSensorListener(void *obj, bool isDied)
{
    ALOGD("[PQLightSensor] diedSensorListener isDied[%d]", (int)isDied);
    PQLightSensor *service = static_cast<PQLightSensor*>(obj);

    service->mIsSensorInited = false;
}

bool PQLightSensor::threadLoop()
{
    int rgbwSensorInited = 0;

    while (mContRunning) {
        Mutex::Autolock _l(mLock);
        if (mIsSensorInited == 1)
        {
            mWaitCond.waitRelative(mLock, MILLI_TO_NANO(60 * 1000));
        }
        else
        {
            rgbwSensorInited = mCustPQSensorInitFun();

            if (rgbwSensorInited == 1)
            {
                mIsSensorInited = true;
            }
            else
            {
                mWaitCond.waitRelative(mLock, MILLI_TO_NANO(10 * 1000));
            }
        }
    }

    return mContRunning;
}

