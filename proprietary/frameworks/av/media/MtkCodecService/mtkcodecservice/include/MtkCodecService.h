#ifndef HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_MTKCODECSERVICE_V1_0_MTKCODECSERVICE_H_
#define HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_MTKCODECSERVICE_V1_0_MTKCODECSERVICE_H_

#include <vendor/mediatek/hardware/mtkcodecservice/1.1/IMtkCodecService.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>

#include <CAPEWrapper.h>
#include <hidlmemory/mapping.h>
#include <android/hidl/allocator/1.0/IAllocator.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mtkcodecservice {
namespace V1_1 {
namespace implementation {


using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::Thread;
using ::android::Mutex;
using ::android::status_t;
using ::android::hardware::Parcel;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::allocator::V1_0::IAllocator;

class MtkCodecService:public IMtkCodecService{
public:
    MtkCodecService ();
    Return<void> Create(Create_cb _hidl_cb) override;
    Return<void> Destory(int32_t codecid) override;
    Return<void> Init(const apeInitParams& params, int32_t codecid) override;
    Return<void> DeInit(int32_t codecid) override;
    Return<void> Decode(const apeDecParams& params_in, int32_t codecid, Decode_cb _hidl_cb) override;
    Return<void> Reset(const apeResetParams& params, int32_t codecid) override;
    Return<void> setCallback(const sp<IMtkCodecServiceCallback>& callback) override;

    class apeHidlDeathRecipient : public hidl_death_recipient {
    public:
        apeHidlDeathRecipient(const sp<MtkCodecService> ape_s) : mApes(ape_s) {
        }
        virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override {
            mApes->handleHidlDeath();
        }
    private:
        sp<MtkCodecService> mApes;
    };
    virtual void handleHidlDeath();
protected:
    sp<IMtkCodecServiceCallback> mapeCallback;
    sp<apeHidlDeathRecipient> mapeHidlDeathRecipient;
private:
    sp<IMemory> out_dec_mem;
    hidl_memory m_hidl_memory_out;
    KeyedVector<int32_t, CAPEWrapper*> mvCodec;
    int32_t mCodecId;
};

extern "C" IMtkCodecService* HIDL_FETCH_IMtkCodecService(const char* name);

}    // namespace implementation
}    // namespace V1_1
}    // namespace mtkcodecservice
}    // namespace hardware
}    // namespace mediatek
}    // namespace vendor

#endif
