#ifndef VENDOR_MEDIATEK_HARDWARE_MDMONITOR_V1_0_MDMONITORCLIENTCALLBACK_H
#define VENDOR_MEDIATEK_HARDWARE_MDMONITOR_V1_0_MDMONITORCLIENTCALLBACK_H

#include <vendor/mediatek/hardware/mdmonitor/1.0/IMDMonitorClientCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "HIDLHandlerInterface.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mdmonitor {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::mediatek::hardware::mdmonitor::V1_0::IMDMonitorClientCallback;
using namespace libmdmonitor;


class MDMonitorClientCallback : public IMDMonitorClientCallback {
public:
    MDMonitorClientCallback();
    virtual ~MDMonitorClientCallback();

    // Methods from IMDMonitorClientCallback follow.
    virtual Return<bool> MCPReceiver(const hidl_vec<uint8_t>& data) override;
    
    // other methods
    void SetMCPHandler(HIDLHandlerInterface *handlerInstance);


private:
    DataQueue m_retMCPData; // no used now, trap MCP doesn't need reply MCP data back
    HIDLHandlerInterface *m_mcpHandlerInstance;
};


}  // namespace implementation
}  // namespace V1_0
}  // namespace mdmonitor
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_MDMONITOR_V1_0_MDMONITORCLIENTCALLBACK_H