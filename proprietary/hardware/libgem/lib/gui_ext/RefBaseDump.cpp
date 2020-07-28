#define LOG_TAG "RefBaseDump"

#if 0
#define RBD_LOGV(x, ...) ALOGV(x, ##__VA_ARGS__)
#define RBD_LOGD(x, ...) ALOGD(x, ##__VA_ARGS__)
#define RBD_LOGI(x, ...) ALOGI(x, ##__VA_ARGS__)
#define RBD_LOGW(x, ...) ALOGW(x, ##__VA_ARGS__)
#define RBD_LOGE(x, ...) ALOGE(x, ##__VA_ARGS__)
#else
//#define RBD_LOGV(x, ...)
//#define RBD_LOGD(x, ...)
#define RBD_LOGI(x, ...)
//#define RBD_LOGW(x, ...)
//#define RBD_LOGE(x, ...)
#endif

#include <cutils/log.h>
#include <cutils/properties.h>
#include <mediatek/RefBaseDump.h>

namespace android {


IMPLEMENT_META_GUIEXTMONITOR(RefBaseMonitor, RefBase*, "RB");

RefBaseMonitor::RefBaseMonitor() {
    char value[PROPERTY_VALUE_MAX];
    RBD_LOGI("RefBaseMonitor ctor - %p", this);
    getProcessName();
    property_get("debug.rb.dump", value, "Mary had a little lamb");
    mIsTracking = (-1 != mProcessName.find(value));
}


status_t RefBaseMonitor::monitor(RefBase* pRb) {
    if (mIsTracking) {
        pRb->trackMe(true, false);
    }
    return GuiExtMonitor<RefBaseMonitor, RefBase*>::monitor(pRb);
}


status_t RefBaseMonitor::dump(String8& result, const char* /*prefix*/) {
    size_t listSz;
    RefBase* pRb;
    Mutex::Autolock _l(mLock);
    RBD_LOGI("RefBaseMonitor Dump - %p", this);
    listSz = mItemList.size();
    result.appendFormat("\t  [%8p]    RefCnt   %s", this, mProcessName.string());
    result.append(mIsTracking ? " <- tracking\n" : "\n");
    result.append("\t  -----------------------\n");

    for (size_t i = 0; i < listSz; i++) {
        pRb = mItemList.keyAt(i);
        if (mIsTracking) {
            pRb->printRefs();
        }
        result.appendFormat("\t   %2zu) %8p %4d\n", i, pRb, pRb->getStrongCount());
    }
    result.append("\t*****************************************************\n");
    return NO_ERROR;
}


String8 RefBaseMonitor::getKeyName() const {
    return String8::format("RB-%p", this);
}


}; // namespace android
