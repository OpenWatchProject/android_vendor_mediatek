#ifndef VENDOR_MEDIATEK_HARDWARE_APM_V1_0_APMCONTROLCALLBACK_H
#define VENDOR_MEDIATEK_HARDWARE_APM_V1_0_APMCONTROLCALLBACK_H

#include <vendor/mediatek/hardware/apmonitor/2.0/IApmControlCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vector>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace apmonitor {
namespace V2_0 {
namespace implementation {

using vendor::mediatek::hardware::apmonitor::V2_0::apmResult_e;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

typedef void (*onUpdateKpiSubStateListener)(const std::vector<uint16_t> &subscribedMessages, void *param);
typedef void (*onQueryKpiListener)(uint16_t msgId, void *param);


class ApmControlCallback : public IApmControlCallback {
    // Methods from ::vendor::mediatek::hardware::apmonitor::V2_0::IApmControlCallback follow.
public:
    ApmControlCallback();
    virtual ~ApmControlCallback() {};
    Return<apmResult_e> onUpdateKpiSubscriptionState(const hidl_vec<uint16_t>& subscribedMessages) override;
    Return<apmResult_e> onQueryKpi(uint16_t msgId) override;

    void registerUpdateKpiSubStateListener(onUpdateKpiSubStateListener listener, void *param);
    void registerQueryKpiListener(onQueryKpiListener listener, void *param);
private:
    onUpdateKpiSubStateListener m_pUpdateKpiSubStateListener;
    onQueryKpiListener m_pQueryKpiListener;
    void *m_pUpdateKpiSubStateParam;
    void *m_pQueryKpiParam;
    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IApmControlCallback* HIDL_FETCH_IApmControlCallback(const char* name);

}  // namespace implementation
}  // namespace V2_0
}  // namespace apmonitor
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_APM_V1_0_APMCONTROLCALLBACK_H
