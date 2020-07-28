#include "DmcService.h"
#include "dmc_utils.h"
#include <hidl/LegacySupport.h>
#include <vector>
#include <stdio.h>
#include <stddef.h>
#include <string.h>


#undef TAG
#define TAG "DMC-DmcService"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace dmc {
namespace V1_1 {
namespace implementation {

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;


DmcService::DmcService(IDmcCore *pDmcCore): mIsServiceRegisted(false) {
    DMC_LOGD(TAG, "constructor");
    m_pEventCallback = nullptr;
    m_pDmcCore = pDmcCore;
}

//=============== IDmcService public interface implementation ===============
Return<void> DmcService::createSession(const dmcSessionConfig_t& config, createSession_cb _hidl_cb) {
    int32_t sessionId = 0;
    SessionInfo *pInfo = new SessionInfo();
    if (pInfo == NULL) {
        DMC_LOGE(TAG, "createSession() new SessionInfo failed!");
        _hidl_cb(dmcResult_e::DMC_FAILED, 0);
        return Void();
    }

    strncpy(pInfo->identity, config.identity.c_str(), MAX_IDENTITY_LEN);
    strncpy(pInfo->targetVersion, config.version.c_str(), MAX_VERSION_LEN);
    pInfo->decryptPacket = config.decryptPacket;

    DMC_RESULT_CODE ret = m_pDmcCore->createSession(pInfo, sessionId);
    dmcResult_e result = (DMC_RESULT_SUCCESS != ret)? dmcResult_e::DMC_FAILED : dmcResult_e::DMC_SUCCESS;
    _hidl_cb(result, sessionId);

    delete pInfo;
    return Void();
}

Return<dmcResult_e> DmcService::closeSession(int32_t session) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;
    if (m_pDmcCore->isValidSession(session)) {
        DMC_RESULT_CODE ret = m_pDmcCore->closeSession(session);
        result = (DMC_RESULT_SUCCESS != ret)? dmcResult_e::DMC_FAILED : dmcResult_e::DMC_SUCCESS;
    } else {
        DMC_LOGE(TAG, "closeSession() with invalid SID = %d", session);
        result = dmcResult_e::DMC_INVALID_ARGUMENTS;
    }

    return result;
}

Return<dmcResult_e> DmcService::registerEventCallback(int32_t session, const sp<IDmcEventCallback>& callback) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;
    if (m_pDmcCore->isValidSession(session)) {
        m_pEventCallback = callback;
        m_pDmcCore->registerEventCallback(eventCallback, (void *)this);
        result = dmcResult_e::DMC_SUCCESS;
    } else {
        DMC_LOGE(TAG, "registerEventCallback() with invalid SID = %d", session);
        result = dmcResult_e::DMC_INVALID_ARGUMENTS;
    }

    return result;
}

Return<dmcResult_e> DmcService::invokeInt(
        int32_t session, dmcCommand_e command, const dmcObjectId_t& objectId, int32_t param) {

    if (!m_pDmcCore->isValidSession(session)) {
        DMC_LOGE(TAG, "invokeInt(%d) with invalid SID = %d", command, session);
        return dmcResult_e::DMC_INVALID_ARGUMENTS;
    }
    DMC_LOGI(TAG, "invokeInt(%d) with SID = %d", command, session);

    ObjId *pOid = NULL;
    switch (command) {
        case dmcCommand_e::DMC_CMD_START_REPORT_EVENT: {
            m_pDmcCore->enableSubscription();
            break;
        }
        case dmcCommand_e::DMC_CMD_STOP_REPORT_EVENT: {
            m_pDmcCore->disableSubscription();
            break;
        }
        case dmcCommand_e::DMC_CMD_SET_MAX_PAYLOAD_SIZE: {
            pOid = new ObjId();
            if (pOid == NULL) {
                DMC_LOGE(TAG, "invokeInt() new ObjId failed!");
                return dmcResult_e::DMC_FAILED;
            }

            objIdHidlToCore(&objectId, pOid);

            if (param < 0) {
                DMC_LOGW(TAG, "Set DMC_CMD_SET_MAX_PAYLOAD_SIZE invalid size = %d", param);
                param = 0;
            }
            m_pDmcCore->setPacketPayloadSize(pOid, (uint32_t)param);
            if (NULL != pOid) {
                delete pOid;
            }
            break;
        }
        case dmcCommand_e::DMC_CMD_SET_PACKET_TYPE: {
            pOid = new ObjId();
            if (pOid == NULL) {
                DMC_LOGE(TAG, "invokeInt() new ObjId failed!");
                return dmcResult_e::DMC_FAILED;
            }

            objIdHidlToCore(&objectId, pOid);

            OTA_PACKET_TYPE type = (OTA_PACKET_TYPE)param;
            if (type != PACKET_TYPE_HEADER_ONLY && type != PACKET_TYPE_WITH_PAYLOAD) {
                DMC_LOGW(TAG, "DMC_CMD_SET_PACKET_TYPE unexpected type = %d", type);
                type = PACKET_TYPE_DEFAULT;
            }
            m_pDmcCore->setpacketpayloadType(pOid, type);
            if (NULL != pOid) {
                  delete pOid;
            }
            break;
        }
        default:
            return dmcResult_e::DMC_INVALID_ARGUMENTS;
    }
    return dmcResult_e::DMC_SUCCESS;
}

Return<dmcResult_e> DmcService::invokeString(
        int32_t session, dmcCommand_e command, const dmcObjectId_t& objectId, const hidl_string& param) {
    // TODO implement
    UNUSED(session);
    UNUSED(command);
    UNUSED(objectId);
    UNUSED(param);

    return dmcResult_e {};
}

Return<void> DmcService::queryEvent(int32_t session, const dmcObjectId_t& objectId, queryEvent_cb _hidl_cb) {
    dmcValue_t dmcValue = {0};
    DMC_LOGE(TAG, "queryEvent() not supported!");
    _hidl_cb(dmcResult_e::DMC_FAILED, dmcValue);

    return Void();
}

Return<dmcResult_e> DmcService::queryEventAsync(
        int32_t sessionId, uint8_t simId, const dmcObjectId_t& objectId) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;
    dmcValue_t dmcValue = {0};

    if (m_pDmcCore->isValidSession(sessionId)) {
        ObjId *pOid = new ObjId();
        if (pOid == NULL) {
            DMC_LOGE(TAG, "queryEventAsync() new ObjId failed!");
            return result;
        }

        objIdHidlToCore(&objectId, pOid);

        DMC_RESULT_CODE ret = m_pDmcCore->queryEventAsync(sessionId, simId, pOid);

        DMC_LOGI(TAG, "[%" PRIu8"]queryEventAsync OID = %s, ret = %d", simId, pOid->getOidString(), ret);
        result = (DMC_RESULT_SUCCESS != ret)? dmcResult_e::DMC_FAILED : dmcResult_e::DMC_SUCCESS;

        if (NULL != pOid) {
            delete pOid;
        }
    } else {
        DMC_LOGE(TAG, "queryEventAsync() with invalid SID = %d", sessionId);
    }

    return result;
}

Return<dmcResult_e> DmcService::subscribe(int32_t session, const dmcObjectId_t& objectId) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;

    if (m_pDmcCore->isValidSession(session)) {
        ObjId *pOid = new ObjId();
        if (pOid == NULL) {
            DMC_LOGE(TAG, "subscribe() new ObjId failed!");
            return dmcResult_e::DMC_FAILED;
        }

        objIdHidlToCore(&objectId, pOid);
        DMC_RESULT_CODE ret = m_pDmcCore->subscribe(pOid);
        DMC_LOGI(TAG, "subscribe OID = %s", pOid->getOidString());
        result = (DMC_RESULT_SUCCESS != ret)? dmcResult_e::DMC_FAILED : dmcResult_e::DMC_SUCCESS;
        if (NULL != pOid) {
            delete pOid;
        }
    } else {
        DMC_LOGE(TAG, "subscribe() with invalid SID = %d", session);
        result = dmcResult_e::DMC_INVALID_ARGUMENTS;
    }

    return result;
}

Return<dmcResult_e> DmcService::unsubscribe(int32_t session, const dmcObjectId_t& objectId) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;

    if (m_pDmcCore->isValidSession(session)) {
        ObjId *pOid = new ObjId();
        if (pOid == NULL) {
            DMC_LOGE(TAG, "unsubscribe() new ObjId failed!");
            return dmcResult_e::DMC_FAILED;
        }

        objIdHidlToCore(&objectId, pOid);
        DMC_RESULT_CODE ret = m_pDmcCore->unSubscribe(pOid);
        DMC_LOGI(TAG, "unSubscribe OID = %s", pOid->getOidString());
        result = (DMC_RESULT_SUCCESS != ret)? dmcResult_e::DMC_FAILED : dmcResult_e::DMC_SUCCESS;
        if (NULL != pOid) {
            delete pOid;
        }
    } else {
        DMC_LOGE(TAG, "unsubscribe() with invalid SID = %d", session);
        result = dmcResult_e::DMC_INVALID_ARGUMENTS;
    }

    return result;
}

Return<void> DmcService::getSessionStats(int32_t session, int8_t group, getSessionStats_cb _hidl_cb) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;
    dmcSessionStats_t dmcSessionStats = {
        .totalEvents = 0,
        .droppedEvents = 0,
        .eventBufferCapacity = 0,
        .eventBufferInUse = 0
    };

    SessionStats_t sessionStats = {
        .totalEvents = 0,
        .droppedEvents = 0,
        .eventBufferCapacity = 0,
        .eventBufferInUse = 0
    };

    if (m_pDmcCore->isValidSession(session)) {
        DMC_RESULT_CODE ret = m_pDmcCore->getSessionStats(session, group, sessionStats);
        DMC_LOGI(TAG, "getSessionStats(%d)", group);
        dmcSessionStats.totalEvents = sessionStats.totalEvents;
        dmcSessionStats.droppedEvents = sessionStats.droppedEvents;
        dmcSessionStats.eventBufferCapacity = sessionStats.eventBufferCapacity;
        dmcSessionStats.eventBufferInUse = sessionStats.eventBufferInUse;
        result = (DMC_RESULT_SUCCESS != ret)? dmcResult_e::DMC_FAILED : dmcResult_e::DMC_SUCCESS;
        _hidl_cb(result, dmcSessionStats);
    } else {
        DMC_LOGE(TAG, "getSessionStats(%d) with invalid SID = %d", group, session);
        result = dmcResult_e::DMC_INVALID_ARGUMENTS;
        _hidl_cb(result, dmcSessionStats);
    }
    return Void();
}

DMC_RESULT_CODE DmcService::eventCallback(
        int32_t sessionId, KPI_GROUP_TYPE group, ObjId &objId, KpiObj &kpiObj, void *param) {
    DmcService *me = reinterpret_cast<DmcService*>(param);
    DMC_RESULT_CODE result = DMC_RESULT_FAIL;
    // DMC_LOGD(TAG, "eventCallback() enter");

    if (me->m_pDmcCore->isValidSession(sessionId)) {
        if (me->m_pEventCallback == nullptr) {
            DMC_LOGE(TAG, "eventCallback() failed! NULL event callback");
            return DMC_RESULT_FAIL;
        }
        // The KPI OID
        dmcObjectId_t dmcOid = {0};
        me->objIdCoreToHidl(&objId, &dmcOid);
        dmcOid.group = group;
        // The KPI event value
        dmcValue_t dmcValue = {0};
        me->kpiObjCoreToHidl(&kpiObj, &dmcValue);

        auto check = me->m_pEventCallback->onHandleEvent(sessionId, dmcOid, dmcValue);
        if (!check.isOk()) {
            DMC_LOGW(TAG, "DMC client[%d] died!", sessionId);
            me->closeSession(sessionId);
            return DMC_RESULT_FATAL;
        }

        result = DMC_RESULT_SUCCESS;
    } else {
        result = DMC_RESULT_INVALID_ARGUMENTS;
    }

    return DMC_RESULT_SUCCESS;
}

//=============== IDmcService internal interface implementation ===============
bool DmcService::registDmcHidlServiceAsyc() {
    DMC_LOGD(TAG, "registDmcHidlServiceAsyc");

    if (!isServiceRegisted()) {
        if (pthread_create(&m_thread, NULL, DmcService::threadRunService, this)) {
            DMC_LOGE(TAG, "pthread_create(%s)", strerror(errno));
            return false;
        }
    } else {
        DMC_LOGD(TAG, "DMC service is already registed~");
    }
    return true;
}

bool DmcService::isServiceRegisted() {
    return mIsServiceRegisted;
}

void* DmcService::threadRunService(void *obj) {
    DmcService *me = reinterpret_cast<DmcService*>(obj);

    DMC_LOGD(TAG, "start registerAsService(%s)", DMC_HIDL_SVC_NAME);

    if (me->registerAsService(DMC_HIDL_SVC_NAME)) {
        DMC_LOGE(TAG, "registerAsService %s failed", DMC_HIDL_SVC_NAME);
        return NULL;
    }
    me->mIsServiceRegisted = true;
    return NULL;
}

void DmcService::objIdCoreToHidl(const ObjId *oidIn, dmcObjectId_t *oidOut) {
    uint32_t *oid = oidIn->getObjId();
    oidOut->length = oidIn->getObjLength();
    oidOut->ids.setToExternal(oid, oidOut->length);
}

void DmcService::objIdHidlToCore(const dmcObjectId_t *oidIn, ObjId *oidOut) {
    unsigned int len = oidIn->length;
    std::vector<uint32_t> idsVec = oidIn->ids;
    oidOut->setIdsVec(len, idsVec);
}

void DmcService::kpiObjCoreToHidl(const KpiObj *objIn, dmcValue_t *objOut) {
    size_t len = 0;
    unsigned char *data = (unsigned char *)objIn->GetByteArray(len);
    objOut->length = len;
    objOut->data.setToExternal(data, len);
}


// Methods from ::android::hidl::base::V1_1::IBase follow.

//IDmcService* HIDL_FETCH_IDmcService(const char* /* name */) {
    //return new DmcService();
//}
//
}  // namespace implementation
}  // namespace V1_1
}  // namespace dmc
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
