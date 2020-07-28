#ifndef VENDOR_MEDIATEK_HARDWARE_APM_V1_0_APMSERVICE_H
#define VENDOR_MEDIATEK_HARDWARE_APM_V1_0_APMSERVICE_H

#include "IDmcCore.h"
#include "IApMonitor.h"
#include <vendor/mediatek/hardware/apmonitor/2.0/IApmService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <map>
#include <pthread.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace apmonitor {
namespace V2_0 {
namespace implementation {

#define APM_HIDL_SVC_NAME "apm_hidl_service"

using vendor::mediatek::hardware::apmonitor::V2_0::apmResult_e;
using vendor::mediatek::hardware::apmonitor::V2_0::IApmControlCallback;
using vendor::mediatek::hardware::apmonitor::V2_0::apmKpiHeader_t;

using vendor::mediatek::hardware::apmonitor::V2_0::FooBar_t;
using vendor::mediatek::hardware::apmonitor::V2_0::PacketEvent_t;

using android::hidl::base::V1_0::IBase;
using android::hardware::hidl_array;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_memory;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::hardware::Return;
using android::hardware::Void;
using android::sp;
using android::wp;


typedef int32_t SID;
typedef uint16_t APM_MSGID;
#define INVALID_SESSION_ID (0)
#define MAX_APM_CLIENT_NUM (100)

// Binder death recipient handler class
class ApmClientDeathRecipient : public hidl_death_recipient {
public:
    ApmClientDeathRecipient(sp<IApmService> service);
    void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
    sp<IApmService> mApmService;
};

class ApmClientInfo {
public:
    ApmClientInfo():sessionId(INVALID_SESSION_ID) {};
    virtual ~ApmClientInfo() {};

    static SID getSessionCounter() {
        return sSessionCount;
    }
    static SID incSessionCounter() {
        sSessionCount++;
        return sSessionCount;
    }
    static void resetSessionCounter() {
        sSessionCount = 0;
    }

    SID sessionId;
    sp<IApmControlCallback> pControlCallback;
    sp<ApmClientDeathRecipient> pClientDeathRecipient;

private:
    static uint8_t sSessionCount;
};

class ApmService : public IApmService {
public:
    ApmService(IApMonitor *pApMonitor);
    virtual ~ApmService();

    // Methods from ::vendor::mediatek::hardware::apmonitor::V2_0::IApmService follow.
    Return<void> createSession(createSession_cb _hidl_cb) override;
    Return<apmResult_e> closeSession(SID sessionId) override;
    Return<apmResult_e> registerControlCallback(SID sessionId, const sp<IApmControlCallback>& callback) override;
    Return<void> getSubscribedMessages(SID sessionId, getSubscribedMessages_cb _hidl_cb) override;
    Return<uint64_t> getCurrentTimestamp() override;

    bool registApmHidlServiceAsyc();
    bool isServiceRegisted();
    bool isValidSession(SID sessionId);
    void updateSubscriptionState(std::vector<APM_MSGID> subscribedMsgs);

    // Common Metrics API
    Return<apmResult_e> submitRawEvent(int32_t sessionId, const apmKpiHeader_t& apmHeader,
            uint32_t len, const hidl_vec<uint8_t>& kpi) override;

    // Metrics
    Return<apmResult_e> submitFooBarEvent(SID sessionId, const apmKpiHeader_t& apmHeader,
            const FooBar_t& kpi) override;
    Return<apmResult_e> submitPacketEvent(SID sessionId, const apmKpiHeader_t& apmHeader,
            const PacketEvent_t& kpi) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    IApMonitor *m_pApMonitor;
    pthread_t m_thread;
    volatile bool mIsServiceRegisted;
    static void *threadRunService(void *obj);
    Return<apmResult_e> submitKpi(
            const char *func, SID sessionId, const apmKpiHeader_t& apmHeader,
            uint32_t len, const void *kpi);
    void kpiHeaderHidlToCore(const apmKpiHeader_t *headerIn, KpiHeader *headerOut);
    std::map<SID, ApmClientInfo *> mApmClientMap;
    pthread_mutex_t mMutex;
};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IApmService* HIDL_FETCH_IApmService(const char* name);

}  // namespace implementation
}  // namespace V2_0
}  // namespace apmonitor
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_APM_V1_0_APMSERVICE_H
