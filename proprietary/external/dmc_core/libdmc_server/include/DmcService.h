#ifndef VENDOR_MEDIATEK_HARDWARE_DMC_V1_1_DMCSERVICE_H
#define VENDOR_MEDIATEK_HARDWARE_DMC_V1_1_DMCSERVICE_H

#include "IDmcCore.h"
#include "ObjId.h"
#include <vendor/mediatek/hardware/dmc/1.1/IDmcService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace dmc {
namespace V1_1 {
namespace implementation {

using vendor::mediatek::hardware::dmc::V1_0::dmcResult_e;
using vendor::mediatek::hardware::dmc::V1_0::dmcCommand_e;
using vendor::mediatek::hardware::dmc::V1_0::dmcObjectId_t;
using vendor::mediatek::hardware::dmc::V1_0::IDmcEventCallback;
using vendor::mediatek::hardware::dmc::V1_0::dmcSessionStats_t;
using vendor::mediatek::hardware::dmc::V1_0::dmcSessionConfig_t;
using vendor::mediatek::hardware::dmc::V1_0::dmcValue_t;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

#define DMC_HIDL_SVC_NAME "dmc_hidl_service"

class DmcService : public IDmcService {
public:
    DmcService(IDmcCore *pDmcCore);
    virtual ~DmcService() {};
    // Methods from ::vendor::mediatek::hardware::dmc::V1_0::IDmcService follow.
    Return<void> createSession(const dmcSessionConfig_t& config, createSession_cb _hidl_cb) override;
    Return<dmcResult_e> closeSession(int32_t session) override;
    Return<dmcResult_e> registerEventCallback(int32_t session, const sp<IDmcEventCallback>& callback) override;
    Return<dmcResult_e> invokeInt(int32_t session, dmcCommand_e command, const dmcObjectId_t& objectId, int32_t param) override;
    Return<dmcResult_e> invokeString(int32_t session, dmcCommand_e command, const dmcObjectId_t& objectId, const hidl_string& param) override;
    Return<void> queryEvent(int32_t session, const dmcObjectId_t& objectId, queryEvent_cb _hidl_cb) override;
    Return<dmcResult_e> subscribe(int32_t session, const dmcObjectId_t& objectId) override;
    Return<dmcResult_e> unsubscribe(int32_t session, const dmcObjectId_t& objectId) override;
    Return<void> getSessionStats(int32_t sessionId, int8_t group, getSessionStats_cb _hidl_cb) override;
    // Methods from ::vendor::mediatek::hardware::dmc::V1_1::IDmcService follow.
    Return<dmcResult_e> queryEventAsync(int32_t sessionId, uint8_t simId, const dmcObjectId_t& objectId) override;

    bool registDmcHidlServiceAsyc();
    bool isServiceRegisted();
private:
    static DMC_RESULT_CODE eventCallback(
            int32_t sessionId, KPI_GROUP_TYPE group, ObjId &objId, KpiObj &obj, void *param);
    android::sp<IDmcEventCallback> m_pEventCallback;
    IDmcCore *m_pDmcCore;
    pthread_t m_thread;
    volatile bool mIsServiceRegisted;
    static void *threadRunService(void *obj);

    void objIdCoreToHidl(const ObjId *oidIn, dmcObjectId_t *oidOut);
    void objIdHidlToCore(const dmcObjectId_t *oidIn, ObjId *oidOut);
    void kpiObjCoreToHidl(const KpiObj *objIn, dmcValue_t *objOut);
};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IDmcService* HIDL_FETCH_IDmcService(const char* name);

}  // namespace implementation
}  // namespace V1_1
}  // namespace dmc
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_DMC_V1_1_DMCSERVICE_H
