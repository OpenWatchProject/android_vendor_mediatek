#ifndef VENDOR_MEDIATEK_HARDWARE_MDMONITOR_V1_0_MDMONITORSERVICE_H
#define VENDOR_MEDIATEK_HARDWARE_MDMONITOR_V1_0_MDMONITORSERVICE_H

#include <vendor/mediatek/hardware/mdmonitor/1.0/IMDMonitorService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <string>
#include "HIDLHandlerInterface.h"
#include "CommandInterface.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mdmonitor {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::mediatek::hardware::mdmonitor::V1_0::IMDMonitorClientCallback;
using ::vendor::mediatek::hardware::mdmonitor::V1_0::MtkModemBinType;
using namespace libmdmonitor;


class MDMonitorService : public IMDMonitorService {
public:
    MDMonitorService(int maxThreads, std::string serviceName);
    virtual ~MDMonitorService();

    // HIDL APIs, the Methods from IMDMonitorService 
    virtual Return<void> sendMCPData(const hidl_vec<uint8_t>& data, sendMCPData_cb _hidl_cb) override;
    virtual Return<void> registerTrapCallback(const hidl_string& servername, const sp<IMDMonitorClientCallback>& callback) override;
    virtual Return<void> getModemBinData(MtkModemBinType type, uint32_t offset, uint32_t size, getModemBinData_cb _hidl_cb) override;

    // other methods
    void SetMCPHandler(HIDLHandlerInterface *handlerInstance);
    void SetCMDHandler(CommandInterface *cmdRunner);

    // service control methods
    int startService();
    void stopService();

private:
    static void* threadRun(void *obj);
    void runService();
  
private:
    // HIDL thread variable
    int m_maxThreads;
    std::string m_serviceName;
    pthread_t m_thread;
    volatile bool m_isServiceStopped;

    // Call back to process data from HIDL service APIs
    DataQueue m_retMCPData;
    HIDLHandlerInterface *m_mcpHandlerInstance;
    CommandInterface *m_CmdInterface;
    
};


}  // namespace implementation
}  // namespace V1_0
}  // namespace mdmonitor
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_MDMONITOR_V1_0_MDMONITORSERVICE_H