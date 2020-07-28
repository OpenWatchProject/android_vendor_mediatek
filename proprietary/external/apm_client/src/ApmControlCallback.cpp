#include "ApmControlCallback.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace apmonitor {
namespace V2_0 {
namespace implementation {

ApmControlCallback::ApmControlCallback():
        m_pUpdateKpiSubStateListener(NULL), m_pQueryKpiListener(NULL),
        m_pUpdateKpiSubStateParam(NULL), m_pQueryKpiParam(NULL) {
}

// Methods from ::vendor::mediatek::hardware::apmonitor::V2_0::IApmControlCallback follow.
Return<apmResult_e> ApmControlCallback::onUpdateKpiSubscriptionState(const hidl_vec<uint16_t>& subscribedMessages) {
    if (m_pUpdateKpiSubStateListener == NULL) {
        return apmResult_e::APM_NOT_INITIALIZED;
    }

    m_pUpdateKpiSubStateListener(subscribedMessages, m_pUpdateKpiSubStateParam);
    return apmResult_e::APM_SUCCESS;
}

Return<apmResult_e> ApmControlCallback::onQueryKpi(uint16_t msgId) {
    if (m_pQueryKpiListener == NULL) {
        return apmResult_e::APM_NOT_INITIALIZED;
    }
    m_pQueryKpiListener(msgId, m_pUpdateKpiSubStateParam);
    return apmResult_e::APM_SUCCESS;
}

void ApmControlCallback::registerUpdateKpiSubStateListener(onUpdateKpiSubStateListener listener, void *param) {
    m_pUpdateKpiSubStateListener = listener;
    m_pUpdateKpiSubStateParam = param;
}

void ApmControlCallback::registerQueryKpiListener(onQueryKpiListener listener, void *param) {
    m_pQueryKpiListener = listener;
    m_pQueryKpiParam = param;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

//IApmControlCallback* HIDL_FETCH_IApmControlCallback(const char* /* name */) {
    //return new ApmControlCallback();
//}
//
}  // namespace implementation
}  // namespace V2_0
}  // namespace apmonitor
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
