#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include "vsync_hint/VsyncHint.h"

#include <cutils/log.h>
#include <utils/Trace.h>
#include <dlfcn.h>
#include <linux/types.h>
#include <pthread.h>
#undef LOG_TAG
#define LOG_TAG "VsyncHint"

static int (*fpNotifyVsync)(__u32) = nullptr;
static void *handle = nullptr, *func = nullptr;

typedef int (*FpNotifyVsync)(__u32);

#define LIB_FULL_NAME "libperfctl.so"

void fbcDestroy()
{
    fpNotifyVsync = NULL;
    if (handle)
        dlclose(handle);
}

void fbcInit()
{
    static bool inited = false;
    static pthread_mutex_t mMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mMutex);
    if (inited) {
        pthread_mutex_unlock(&mMutex);
        return;
    }

    /*
     * Consider if library or funcion is missing, re-try helps
     * nothing but lots of error logs. Thus, just init once no
     * matter if anything is well-prepared or not. However,
     * entire init flow should be lock-protected.
     */
    inited = true;

    handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
    if (handle == NULL) {
        ALOGE("Can't load library: %s", dlerror());
        pthread_mutex_unlock(&mMutex);
        return;
    }

    func = dlsym(handle, "xgfNotifyVsync");
    fpNotifyVsync = reinterpret_cast<FpNotifyVsync>(func);

    if (fpNotifyVsync == NULL) {
        ALOGE("xgfNotifyVsync error: %s", dlerror());
        goto err_fbcInit;
    }

    pthread_mutex_unlock(&mMutex);
    return;

err_fbcInit:
    fpNotifyVsync = NULL;
    dlclose(handle);
    pthread_mutex_unlock(&mMutex);
}

namespace android {

VsyncHint::VsyncHint()
{
    fbcInit();
}

VsyncHint::~VsyncHint()
{
    fbcDestroy();
}

void VsyncHint::notifyVsync(int type, nsecs_t /*period*/)
{
    if (type == VSYNC_TYPE_SF) {
        ATRACE_NAME("calculateDelayTime");
        if (fpNotifyVsync)
            fpNotifyVsync(1);
    }
}

VsyncHintApi* createVsyncHint()
{
    return new VsyncHint();
}

}
