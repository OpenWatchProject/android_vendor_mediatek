#define LOG_TAG "MtkCam/TestSensorProvider"

#include <semaphore.h>
#include <fcntl.h>
#include <def/common.h>
#include <utils/std/Log.h>
#include <vector>

#include <utils/Errors.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <gtest/gtest.h>

using namespace android;

#include <utils/sys/SensorProvider.h>
#include "TestSensorProvider.h"

using std::vector;

namespace NSCam {
namespace Utils {

#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

TestSensorProvider::TestSensorProvider()
    : mpProvider(SensorProvider::createInstance("TestSensorProvider"))
{
}

TestSensorProvider::TestThread::TestThread(const sp<SensorProvider>& provider, int interval,
        int testTime, UsageType usage)
    : mpInnerProvider(provider)
    , mInterval(interval)
    , mTestTime(testTime)
    , mUsage(usage)
{
}

void TestSensorProvider::test(int interval, int testTime, UsageType usage)
{
    sp<TestThread> thread = new TestThread(mpProvider, interval, testTime, usage);
    thread->run("TestThread");

    mThreads.push_back(thread);
}

void TestSensorProvider::waitExit()
{
    for (size_t i = 0; i < mThreads.size(); i++)
    {
        mThreads[i]->join();
    }
}

bool TestSensorProvider::TestThread::threadLoop()
{
    if (mUsage == USAGE_GET_LATEST)
    {
        testGetLatest();
    }
    else if (mUsage == USAGE_GET_ALL)
    {
        testGetAll();
    }
    else
    {
        monkeyTest();
    }

    return false;
}

void TestSensorProvider::TestThread::testGetLatest()
{
    MBOOL ret = 0;
    int frameNum = 0, fps = 30, retry = 0;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;

    ret = mpInnerProvider->enableSensor(SENSOR_TYPE_GYRO, mInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    while (frameNum < (fps * mTestTime))
    {
        usleep(frameInterval);

        SensorData data;
        ret = mpInnerProvider->getLatestSensorData(SENSOR_TYPE_GYRO, data);
        EXPECT_EQ(ret, true);
        if (data.timestamp == previousT && retry == 0)
        {
            ++retry;
            ++frameNum;
            continue;
        }
        EXPECT_TRUE(data.timestamp > previousT);
        //MY_LOGD("sensorprovider test data.timestamp=%lld, preT=%lld", data.timestamp, previousT);

        previousT = data.timestamp;
        ++frameNum;
        retry = 0;
    }
}

void TestSensorProvider::TestThread::testGetAll()
{
    MBOOL ret = 0;
    int frameNum = 0, gyroDataNum = 0, acceDataNum = 0, fps = 30, retry = 0;
    MINT64 previousT = 0;

    MINT64 frameInterval = ((float)1 / fps) * 1000000;

    ret = mpInnerProvider->enableSensor(SENSOR_TYPE_GYRO, mInterval);
    EXPECT_EQ(ret, true);
    usleep(100000); //100ms

    while (frameNum < (fps * mTestTime))
    {
        usleep(frameInterval);

        vector<SensorData> data;
        ret = mpInnerProvider->getAllSensorData(SENSOR_TYPE_GYRO, data);
        if (ret == false && retry == 0)
        {
            ++retry;
            ++frameNum;
            continue;
        }
        EXPECT_EQ(ret, true);
        EXPECT_TRUE(data.size() > 0);
        //MY_LOGD("sensorprovider test data.size=%d", data.size());
        EXPECT_TRUE(data[0].timestamp > previousT);
        gyroDataNum += data.size();

        previousT = data[0].timestamp;
        ++frameNum;
        retry = 0;
    }

    int idealDataNum = ((float)1000 / mInterval) * mTestTime;
    int lowerBound = (float)idealDataNum * 0.95;
    int upperBound = (float)idealDataNum * 1.05;
    EXPECT_TRUE((gyroDataNum > lowerBound && gyroDataNum < upperBound));
}

void TestSensorProvider::TestThread::monkeyTest()
{
    std::srand(std::time(NULL));
    int round = 3000, event = 0;

    for (int i = 0; i < round; i++)
    {
        event = std::rand() % 4;

        switch (event)
        {
        case 0:
            randomEnableSensor();
            break;
        case 1:
            randomDisableSensor();
            break;
        case 2:
            randomGetLatest();
            break;
        case 3:
            randomGetAll();
            break;
        default:
            break;
        }
    }
}

void TestSensorProvider::TestThread::randomEnableSensor()
{
    eSensorType type = (rand() % 2 == 0) ? SENSOR_TYPE_GYRO : SENSOR_TYPE_ACCELERATION;
    int interval = rand() % 100;

    MY_LOGD("type=%d, interval=%d", type, interval);
    mpInnerProvider->enableSensor(type, interval);
}
void TestSensorProvider::TestThread::randomDisableSensor()
{
    eSensorType type = (rand() % 2 == 0) ? SENSOR_TYPE_GYRO : SENSOR_TYPE_ACCELERATION;

    MY_LOGD("type=%d", type);
    mpInnerProvider->disableSensor(type);
}
void TestSensorProvider::TestThread::randomGetLatest()
{
    eSensorType type = (rand() % 2 == 0) ? SENSOR_TYPE_GYRO : SENSOR_TYPE_ACCELERATION;

    MY_LOGD("type=%d", type);
    SensorData data;
    mpInnerProvider->getLatestSensorData(type, data);
}
void TestSensorProvider::TestThread::randomGetAll()
{
    eSensorType type = (rand() % 2 == 0) ? SENSOR_TYPE_GYRO : SENSOR_TYPE_ACCELERATION;

    MY_LOGD("type=%d", type);
    vector<SensorData> data;
    mpInnerProvider->getAllSensorData(SENSOR_TYPE_GYRO, data);
}

} //Utils
} //NSCam

