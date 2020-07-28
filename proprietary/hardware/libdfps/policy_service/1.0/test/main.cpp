#include <inttypes.h>

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <dfps/FpsPolicy.h>
#include <dfps/FpsInfo.h>
#include <dfps/FpsVsync.h>

#define __STDC_FORMAT_MACROS 1

using namespace android;

class TestClass {
public:
    TestClass(int32_t id) { mId = id; }
    ~TestClass() {}
    void onSwVsyncChange(int32_t mode, int32_t fps);
private:
    int32_t mId;
};

void TestClass::onSwVsyncChange(int32_t mode, int32_t fps)
{
    ALOGI("TestClass_%d: mode%d  fps:%d", mId, mode, fps);
}

void qwe(int32_t mode, int32_t fps)
{
    ALOGI("qwe: mode:%d  fps:%d", mode, fps);
}

int main(int /*argc*/, char** /*argv*/)
{
    sp<ProcessState> ps(ProcessState::self());
    ps->setThreadPoolMaxThreadCount(4);
    ps->startThreadPool();

    String8 string("");
    string.appendFormat("test-policy");
    sp<FpsPolicy> policy = new FpsPolicy(FpsPolicy::API_THERMAL, string);
    int number;
    FpsRange range;
    policy->getPanelInfoNumber(&number);
    ALOGI("total fps: %d", number);
    for (int i = 0; i < number; i++) {
        policy->getFpsRange(i, &range);
        ALOGI("+ panel fps: [%d] %d~%d", i, range.min, range.max);
    }
    class TestClass* tt = new TestClass(1234);
    //vsync->registerSwVsyncChangeCallback(tt->onSwVsyncChange);
    //auto hotplugHook = std::bind(&HWComposer::hotplug, this, std::placeholders::_1, std::placeholders::_2);
    //auto swVSyncChangeHook = std::bind(&TestClass::onSwVsyncChange, tt, std::placeholders::_1, std::placeholders::_2);
    //vsync->registerSwVsyncChangeCallback(swVSyncChangeHook);
    //vsync->enableTracker(1);

    uint64_t glContextId;
    glContextId = 0x1234ffff;
    policy->setFps(40, FpsPolicy::MODE_INTERNAL_SW, 456, glContextId);


    int i = 0;
    printf("press any key to test fps info\n");
    scanf("%d", &i);

    sp<FpsInfo> info = new FpsInfo();

    printf("press any key to exit\n");
    scanf("%d", &i);

    return 0;
}
