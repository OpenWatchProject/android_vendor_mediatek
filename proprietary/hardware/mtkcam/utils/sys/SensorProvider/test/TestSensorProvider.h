#ifndef _MTKCAM_UTILS_SYS_SENSOR_PROVIDER_TEST_TESTSENSORPROVIDER_H_
#define _MTKCAM_UTILS_SYS_SENSOR_PROVIDER_TEST_TESTSENSORPROVIDER_H_

#include <utils/threads.h>

namespace NSCam {
namespace Utils {
class SensorProvider;

class TestSensorProvider : public RefBase
{
public:
    enum UsageType {
        USAGE_GET_LATEST,
        USAGE_GET_ALL,
        USAGE_MONKEY,
    };
    TestSensorProvider();
    void test(int interval, int testTime, UsageType usage);
    void waitExit();

private:
    class TestThread : public Thread
    {
    public:
        TestThread(const sp<SensorProvider>& provider, int interval, int testTime, UsageType usage);

    private:
        bool threadLoop();
        void testGetLatest();
        void testGetAll();
        void monkeyTest();
        void randomEnableSensor();
        void randomDisableSensor();
        void randomGetLatest();
        void randomGetAll();
        sp<SensorProvider> mpInnerProvider;
        int mInterval;
        int mTestTime;
        UsageType mUsage;
    };

    std::vector<sp<TestThread> > mThreads;
    sp<SensorProvider> mpProvider;
};
}
}
#endif // _MTKCAM_UTILS_SYS_SENSOR_PROVIDER_TEST_TESTSENSORPROVIDER_H_

