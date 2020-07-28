#ifndef __AAL_LIGHT_SENSOR__
#define __AAL_LIGHT_SENSOR__

#include <utils/threads.h>
#include <AAL20/AALCust.h>
#include <ASensorEventQueue.h>
#include <ASensorManager.h>

typedef long long millisec_t;
typedef void (*ListenerCallback)(void *user, int ali);

using android::frameworks::sensorservice::V1_0::ISensorManager;
using android::frameworks::sensorservice::V1_0::IEventQueue;

using android::sp;
using ::android::Thread;
using ::android::Condition;

class AALLightSensor : public Thread
{
private:
    Condition mWaitCond;
    volatile bool mContRunning;

    sp<ISensorManager> mSensorMgr;
    sp<IEventQueue> mSensorEventQueue;
    int32_t mLightSensorHandle;
    bool mPrepareEnable;

public:

    AALLightSensor();
    ~AALLightSensor();

    static void loadCustParameters(CustParameters &cust);

    void setListener(ListenerCallback callback, void *user);

    bool isEnabled();

    void setEnabled(bool enabled);

    virtual bool threadLoop();
};

#endif

