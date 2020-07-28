#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_lomoeffect_V1_0_LomoEffect_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_lomoeffect_V1_0_LomoEffect_H

#include <vendor/mediatek/hardware/camera/lomoeffect/1.0/ILomoEffect.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

// === lomo_hal_jni header
#include <lomo_hal_jni.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace lomoeffect {
namespace V1_0 {
namespace implementation {

using ::android::hardware::camera::common::V1_0::Status;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::android::hidl::memory::V1_0::IMemory;
using ::vendor::mediatek::hardware::camera::lomoeffect::V1_0::ILomoEffect;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::hidl_handle;

using ::android::sp;


// === ion related: start ===
#include <ion/ion.h>
#include <linux/ion_drv.h>
#include "libion_mtk/include/ion.h"
#include <sys/mman.h>
// === ion related: end ===

struct LomoIonInfo
{
public:
    LomoIonInfo();
    ~LomoIonInfo();
    void cleanup(const int ion_dev_fd);
    void print(const int idx, const int logLevel);
    void reset();
// member var
    unsigned char *m_pBuffer = NULL;
    int m_bufLen = 0;
    int m_ionShareFd = -1;
    ion_user_handle_t m_ionImportHandle = -1;
};

struct LomoEffect : public ILomoEffect {
    Return<Status> init() override;
    Return<Status> uninit() override;
    Return<Status> allocLomoSrcImage(uint32_t pvCBWidth, uint32_t pvCBHeight, uint32_t pvCBformat, uint32_t ppSrcImgWidth, uint32_t ppSrcImgHeight) override;
    Return<Status> allocLomoDstImage(uint32_t Number, int32_t size, const hidl_array<uint32_t, 3>& arrayPlaneOffset, const hidl_handle& dstHidlShareFd) override;
    Return<Status> uploadLomoSrcImage(const hidl_handle& srcHidlHandleShareFD, const hidl_array<uint32_t, 3>& arrayPlaneOffset) override;
    Return<Status> freeLomoSrcImage() override;
    Return<Status> lomoImageEnque(uint32_t Number, const hidl_array<uint32_t, 3>& arrayPlaneOffset, int32_t ppEffectIdx) override;
    Return<Status> lomoImageDeque(uint32_t Number, const hidl_array<uint32_t, 3>& arrayPlaneOffset, int32_t ppEffectIdx) override;
    Return<Status> freeLomoDstImage() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

public:
private: // member func
    int dlopenLomoHal(void);
private: // member  variables

#if MTKCAM_LOMO_SUPPORT
    LomoIonInfo m_dstIonInfo[48];
    LomoIonInfo m_srcIonInfo;
    int m_ion_dev_fd = -1;
    int m_isInited = 0;

    LomoHalJni *mpLomoHalJni = NULL;
    int m_srcImgWidth = 0;
    int m_srcImgHeight = 0;

    void *m_lomoLib = NULL;
    void *m_pLomofnEntry = NULL;
    // static LomoHalJniImp singleton;
    int m_logLevel = 0;
#endif  // MTKCAM_LOMO_SUPPORT
};

extern "C" ILomoEffect* HIDL_FETCH_ILomoEffect(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace lomoeffect
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_lomoeffect_V1_0_LomoEffect_H
