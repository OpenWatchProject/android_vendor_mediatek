#include "ApmService.h"
#include <hidl/LegacySupport.h>
#include "dmc_utils.h"
#include <inttypes.h>
#include <vector>
#include <list>

#undef TAG
#define TAG "DMC-ApmService"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace apmonitor {
namespace V2_0 {
namespace implementation {

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using namespace std;

uint8_t ApmClientInfo::sSessionCount = 0;

ApmClientDeathRecipient::ApmClientDeathRecipient(sp<IApmService> service) {
    mApmService = service;
}

void ApmClientDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    UNUSED(who);
    SID sessionId = (SID)cookie;
    DMC_LOGW(TAG, "APM client[%d] died!", sessionId);
    mApmService->closeSession(sessionId);
}

ApmService::ApmService(IApMonitor *pApMonitor): mIsServiceRegisted(false) {
    DMC_LOGI(TAG, "constructor");
    m_pApMonitor = pApMonitor;
    pthread_mutex_init(&mMutex, NULL);
}

ApmService::~ApmService() {
    pthread_mutex_destroy(&mMutex);
}

// Methods from ::vendor::mediatek::hardware::apmonitor::V2_0::IApmService follow.
Return<void> ApmService::createSession(createSession_cb _hidl_cb) {
    if (ApmClientInfo::getSessionCounter() == MAX_APM_CLIENT_NUM) {
        DMC_LOGW(TAG, "Reach max APM clients: %d, reset session counter", MAX_APM_CLIENT_NUM);
        ApmClientInfo::resetSessionCounter();
    }

    int64_t param = m_pApMonitor->getApmSessionParam();

    pthread_mutex_lock(&mMutex);
        ApmClientInfo *pClientInfo = new ApmClientInfo();
        pClientInfo->sessionId = ApmClientInfo::incSessionCounter();
        pClientInfo->pControlCallback = nullptr;
        mApmClientMap.insert(std::pair<SID, ApmClientInfo *>(pClientInfo->sessionId, pClientInfo));

        DMC_LOGI(TAG, "createSession() success for SID = %d, param = %" PRId64,
                pClientInfo->sessionId, param);
        _hidl_cb(apmResult_e::APM_SUCCESS, pClientInfo->sessionId, param);
    pthread_mutex_unlock(&mMutex);
    return Void();
}

Return<apmResult_e> ApmService::closeSession(SID sessionId) {
    apmResult_e ret = apmResult_e::APM_SUCCESS;
    pthread_mutex_lock(&mMutex);
        do {
            std::map<SID, ApmClientInfo *>::iterator itor;
            itor = mApmClientMap.find(sessionId);
            if (itor == mApmClientMap.end()) {
                DMC_LOGE(TAG, "Can't find APM client with SID = %d", sessionId);
                ret = apmResult_e::APM_FAILED;
                break;
            }
            ApmClientInfo *pClientInfo = (ApmClientInfo *)itor->second;
            pClientInfo->pControlCallback = nullptr;
            if (mApmClientMap.erase(sessionId) == 0) {
                DMC_LOGE(TAG, "Faild to close APM client with SID = %d", sessionId);
                ret = apmResult_e::APM_FAILED;
                break;
            }
        } while(false);

        DMC_LOGI(TAG, "closeSession() success for SID = %d", sessionId);
    pthread_mutex_unlock(&mMutex);

    return ret;
}

Return<void> ApmService::getSubscribedMessages(SID sessionId, getSubscribedMessages_cb _hidl_cb) {
    std::vector<APM_MSGID> subscribedMsgs;
    if (!isValidSession(sessionId)) {
        DMC_LOGE(TAG, "getSubscribedMessages() failed with invalid SID = %d", sessionId);
        _hidl_cb(apmResult_e::APM_INVALID_ARGUMENTS, subscribedMsgs);
        return Void();
    }
    m_pApMonitor->getSubscribedMsgs(subscribedMsgs);
    _hidl_cb(apmResult_e::APM_SUCCESS, subscribedMsgs);
    return Void();
}

bool ApmService::isValidSession(SID sessionId) {
    bool bFound = false;
    pthread_mutex_lock(&mMutex);
        std::map<SID, ApmClientInfo *>::iterator itor;
        itor = mApmClientMap.find(sessionId);
        if (itor != mApmClientMap.end()) {
            bFound = true;
        }
    pthread_mutex_unlock(&mMutex);
    return bFound;
}

Return<uint64_t> ApmService::getCurrentTimestamp() {
    // Java framework only supports to milliseconds.
    return m_pApMonitor->getTimestampUs()/1000;
}

Return<apmResult_e> ApmService::registerControlCallback(SID sessionId, const sp<IApmControlCallback>& callback) {
    if (!isValidSession(sessionId)) {
        DMC_LOGE(TAG, "registerControlCallback() failed with invalid SID = %d", sessionId);
        return apmResult_e::APM_INVALID_ARGUMENTS;
    }

    pthread_mutex_lock(&mMutex);
        std::map<SID, ApmClientInfo *>::iterator itor = mApmClientMap.find(sessionId);
        ApmClientInfo *pClientInfo = (ApmClientInfo *)itor->second;

        pClientInfo->pControlCallback = callback;
        if (pClientInfo->pControlCallback != nullptr) {
            DMC_LOGD(TAG, "registerControlCallback() with SID = %d", sessionId);
            pClientInfo->pClientDeathRecipient = new ApmClientDeathRecipient(this);
            pClientInfo->pControlCallback->linkToDeath(pClientInfo->pClientDeathRecipient, sessionId);
        }
    pthread_mutex_unlock(&mMutex);

    return apmResult_e::APM_SUCCESS;
}


Return<apmResult_e> ApmService::submitKpi(
        const char *func, SID sessionId, const apmKpiHeader_t& apmHeader, uint32_t len, const void *kpi) {
    if (!isValidSession(sessionId)) {
        DMC_LOGE(TAG, "%s() failed with invalid SID = %d", func, sessionId);
        return apmResult_e::APM_INVALID_ARGUMENTS;
    }

    KpiHeader kpiHeader = {0};
    kpiHeaderHidlToCore(&apmHeader, &kpiHeader);

    DMC_RESULT_CODE ret = m_pApMonitor->handleKpi(kpiHeader, len, kpi);

    return (ret != DMC_RESULT_SUCCESS)? apmResult_e::APM_FAILED: apmResult_e::APM_SUCCESS;
}

// Common Metrics API
Return<apmResult_e> ApmService::submitRawEvent(int32_t sessionId, const apmKpiHeader_t& apmHeader,
        uint32_t len, const hidl_vec<uint8_t>& kpi) {
    uint8_t *data = (uint8_t *)kpi.data();
    return submitKpi("submitRawEvent", sessionId, apmHeader, len, (const void *)data);
}

// To be phased out in the future, use submitRawEvent instead.
Return<apmResult_e> ApmService::submitFooBarEvent(
        SID sessionId, const apmKpiHeader_t& apmHeader, const FooBar_t& kpi) {
    return submitKpi("submitFooBarEvent", sessionId, apmHeader,
            sizeof(FooBar_t), (const void *)&kpi);
}
// To be phased out in the future, use submitRawEvent instead.
Return<apmResult_e> ApmService::submitPacketEvent(
        SID sessionId, const apmKpiHeader_t& apmHeader, const PacketEvent_t& kpi) {
    DMC_LOGE(TAG, "submitPacketEvent() phased out!");
    return apmResult_e::APM_FAILED;
}

//=============== IDmcService internal interface implementation ===============
bool ApmService::registApmHidlServiceAsyc() {
    DMC_LOGD(TAG, "registApmHidlServiceAsyc");
    if (!isServiceRegisted()) {
        if (pthread_create(&m_thread, NULL, ApmService::threadRunService, this)) {
            DMC_LOGE(TAG, "pthread_create() failed! err = %s", strerror(errno));
            return false;
        }
    } else {
        DMC_LOGD(TAG, "APM service is already registed~");
    }
    return true;
}

bool ApmService::isServiceRegisted() {
    return mIsServiceRegisted;
}

void ApmService::updateSubscriptionState(std::vector<APM_MSGID> subscribedMsgs) {
    // Notify each APM clients subscription status
    std::map<SID, ApmClientInfo *>::iterator itor;
    list<SID> deathClientList;

    pthread_mutex_lock(&mMutex);
        for (itor = mApmClientMap.begin(); itor != mApmClientMap.end(); itor++) {
            SID sid = (SID)itor->first;
            ApmClientInfo *pClientInfo = (ApmClientInfo *)itor->second;
            if (pClientInfo != NULL && pClientInfo->pControlCallback != nullptr) {
                auto check = pClientInfo->pControlCallback->onUpdateKpiSubscriptionState(subscribedMsgs);
                if (!check.isOk()) {
                    DMC_LOGW(TAG, "APM client[%d] died!", sid);
                    deathClientList.push_back(sid);
                    continue;
                }
                apmResult_e ret = (apmResult_e)check;
                if (ret != apmResult_e::APM_SUCCESS) {
                    DMC_LOGE(TAG, "onUpdateKpiSubscriptionState() failed with SID = %d, ret = %d", sid, ret);
                    continue;
                } else {
                    DMC_LOGD(TAG, "onUpdateKpiSubscriptionState() to SID = %d success", sid);
                }
            } else {
                DMC_LOGE(TAG, "invalid APM client entry with SID = %d", sid);
            }
        }
    pthread_mutex_unlock(&mMutex);

    if (!deathClientList.empty()) {
        // deathClientList.unique();
        list<SID>::iterator i;
        for (i = deathClientList.begin(); i != deathClientList.end(); i++) {
            SID sid = (SID)(*i);
            closeSession(sid);
        }
    }
}

void *ApmService::threadRunService(void *obj) {
    ApmService *me = reinterpret_cast<ApmService*>(obj);

    DMC_LOGD(TAG, "start registerAsService(%s)", APM_HIDL_SVC_NAME);

    if (me->registerAsService(APM_HIDL_SVC_NAME)) {
        DMC_LOGE(TAG, "runService: registerAsService %s failed", APM_HIDL_SVC_NAME);
        return NULL;
    }
    me->mIsServiceRegisted = true;
    return NULL;
}

void ApmService::kpiHeaderHidlToCore(const apmKpiHeader_t *headerIn, KpiHeader *headerOut) {
    headerOut->msgId = (APM_MSGID)headerIn->msgId;
    headerOut->simId = headerIn->simId;
    headerOut->timestampMs = headerIn->timestampMs;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

//IApmService* HIDL_FETCH_IApmService(const char* /* name */) {
    //return new ApmService();
//}
//
}  // namespace implementation
}  // namespace V2_0
}  // namespace apmonitor
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
