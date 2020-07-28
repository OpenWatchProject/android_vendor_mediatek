#define LOG_TAG "MtkCodecService"
#include "MtkCodecService.h"

#include <utils/Log.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <hidlmemory/mapping.h>

#include <utils/String16.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <math.h>


namespace vendor {
namespace mediatek {
namespace hardware {
namespace mtkcodecservice {
namespace V1_1 {
namespace implementation {

MtkCodecService::MtkCodecService():mapeHidlDeathRecipient(new apeHidlDeathRecipient(this)){
    mapeCallback = nullptr;
    mCodecId = 0;
}


Return<void> MtkCodecService::Create(Create_cb _hidl_cb){
    ALOGE("mtkcodecservice create");
    CAPEWrapper *mAPEDec = new CAPEWrapper();
    mCodecId++;
    mvCodec.add(mCodecId, mAPEDec);

    apeCreateParams params;
    mAPEDec->Create(params);
    params.codec_id = mCodecId;
    _hidl_cb(params);
    return Void();
}

Return<void> MtkCodecService::Destory(int32_t codecid) {
    ALOGE("mtkcodecservice Destory");
    int32_t pCodecid = codecid;
    CAPEWrapper *mAPEDec = mvCodec.valueFor(pCodecid);
    mAPEDec->Destory();
    delete mAPEDec;
    mvCodec.removeItem(pCodecid);
    return Void();
}

Return<void> MtkCodecService::Init(const apeInitParams& params, int32_t codecid) {
    ALOGE("mtkcodecservice Init");
    int32_t pCodecid = codecid;
    CAPEWrapper *mAPEDec = mvCodec.valueFor(pCodecid);
    mAPEDec->Init(params);
    do {
        sp<IAllocator> ashmem = IAllocator::getService("ashmem");
        if (ashmem == NULL) {
            ALOGD("[MtkCodecService] failed to get IAllocator HW service");
            break;
        }
        ashmem->allocate(mAPEDec->getOutSize(),
                        [&](bool success, const hidl_memory &memory_out) {
            if (success == true) {
                m_hidl_memory_out = memory_out;
                out_dec_mem = mapMemory(m_hidl_memory_out);
                ALOGD("[MtkCodecService] Get HIDL output memory success %d.",(int)mAPEDec->getOutSize());
            }
        });
    } while (0);
    return Void();
}

Return<void> MtkCodecService::DeInit(int32_t codecid) {
    ALOGE("mtkcodecservice DeInit");
    int32_t pCodecid = codecid;
    CAPEWrapper *mAPEDec = mvCodec.valueFor(pCodecid);
    mAPEDec->DeInit();
    return Void();
}

Return<void> MtkCodecService::Decode(const apeDecParams& params_in, int32_t codecid, Decode_cb _hidl_cb) {
    apeDecRetParams params_out;
    //map input hidl_mem to IMemory
    sp<IMemory> in_dec_mem;
    in_dec_mem = mapMemory(params_in.in_mem);
    //return params out_mem point to decode hidl_memory(IMemory)
    params_out.out_mem = m_hidl_memory_out;
    //DO decode and write decode IMemory
    out_dec_mem->update();

    int32_t pCodecid = codecid;
    CAPEWrapper *mAPEDec = mvCodec.valueFor(pCodecid);
    mAPEDec->DoCodec(params_in,params_out,in_dec_mem,out_dec_mem);

    out_dec_mem->commit();
    _hidl_cb(params_out);
    return Void();
}

Return<void> MtkCodecService::Reset(const apeResetParams& params, int32_t codecid) {
    ALOGE("mtkcodecservice Reset");
    int32_t pCodecid = codecid;
    CAPEWrapper *mAPEDec = mvCodec.valueFor(pCodecid);
    mAPEDec->Reset(params);
    return Void();
}

Return<void> MtkCodecService::setCallback(const sp<IMtkCodecServiceCallback>& callback) {
    LOGD("setCallback()");
    mapeCallback = callback;
    mapeCallback->linkToDeath(mapeHidlDeathRecipient, 0);
    return Void();
}

void MtkCodecService::handleHidlDeath() {
    ALOGD("apeHidlService handleHidlDeath()");
    mapeCallback = nullptr;
}

IMtkCodecService* HIDL_FETCH_IMtkCodecService(const char* /* name */){
    ALOGE("MtkCodecService service create");
    return new MtkCodecService();
}

}    // namespace implementation
}    // namespace V1_1
}    // namespace mtkcodecservice
}    // namespace hardware
}    // namespace mediatek
}    // namespace vendor
