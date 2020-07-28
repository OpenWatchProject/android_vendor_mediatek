#pragma GCC system_header
#ifndef ANDROID_REFBASE_DUMP_H__
#define ANDROID_REFBASE_DUMP_H__

#include <utils/String8.h>
#include <mediatek/GuiExtMonitor.h>

namespace android {

//-------------------------------------------------------------------------
// RefBaseMonitor
//-------------------------------------------------------------------------
class RefBaseMonitor : public GuiExtMonitor<RefBaseMonitor, RefBase*> {
public:
    RefBaseMonitor();
    ~RefBaseMonitor() {}

    // add refbase to the monitored list
    status_t monitor(RefBase* pRb);

    // dump all elements in the monitored list and call printRefs if mIsTracking equals 1
    status_t dump(String8& result, const char* prefix);

protected:
    // build the key name to register in GuiExt Service
    String8 getKeyName() const;

private:
    // if trackMe needed
    bool mIsTracking;
};


}; // namespace android
#endif
