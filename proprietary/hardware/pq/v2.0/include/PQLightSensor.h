#ifndef __PQ_LIGHT_SENSOR__
#define __PQ_LIGHT_SENSOR__

#include <utils/threads.h>
#include <fcntl.h>
#include <dlfcn.h>

using ::android::Thread;
using ::android::Mutex;
using ::android::Condition;

typedef long long millisec_t;

class PQLightSensor : public Thread
{
public:
    typedef void (*ListenerCallback)(void *user, int32_t aliR, int32_t aliG, int32_t aliB, int32_t aliW);
    typedef int  (*PQSensorInit)(void);
    typedef void (*PQSensorDeInit)(void);
    typedef int  (*PQSensorEnable)(int enable);
    typedef void (*PQSensorListenser)(void* user, int aliR, int aliG, int aliB, int aliW);
    typedef void (*PQDiedSensorListenser)(void* user, bool isDied);
    typedef void (*PQSensorCallback)(PQSensorListenser pqSensorListenser, void *user);
    typedef void (*PQDiedSensorCallback)(PQDiedSensorListenser pqSensorListenser, void *user);

private:
    Mutex mLock;
    bool mIsEnabled;
    bool mIsSensorInited;
    Condition mWaitCond;

    volatile bool mContRunning;

    void updateLuxValue(int alsR, int alsG, int alsB, int alsW);
    static void sensorListener(void *obj, int alsR, int alsG, int alsB, int alsW);
    static void diedSensorListener(void *obj, bool isDied);
    template <typename T>
    inline T getFunction(const char *name) {
        return reinterpret_cast<T>(dlsym(mLib, name));
    }

    ListenerCallback mListenerCallback;
    void *mListenerUser;
    void *mLib;

    PQSensorInit mCustPQSensorInitFun;
    PQSensorDeInit mCustPQSensorDeInitFun;
    PQSensorEnable mCustPQSensorEnableFun;
    PQSensorCallback mPQSensorCallback;
    PQDiedSensorCallback mPQDiedSensorCallback;

public:

    PQLightSensor();
    ~PQLightSensor();

    void setListener(ListenerCallback callback, void *user) {
        mListenerCallback = callback;
        mListenerUser = user;
    }

    bool isEnabled() {
        return mIsEnabled;
    }

    bool isRGBWLibGood() {
        return (mLib != NULL);
    }

    void setEnabled(bool enabled);

    virtual bool threadLoop();
};

#endif

