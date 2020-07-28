#include "LomoEffect.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "LomoEffectDummy"

#include <cutils/log.h>

#include <android/hidl/memory/1.0/IMemory.h>
//#include <android/hidl/allocator/1.0/IAllocator.h>
//#include <libhidl/base/include/hidl/HidlSupport.h>
#include <libhidl/libhidlmemory/include/hidlmemory/mapping.h>
#include <cutils/properties.h>

#include <dlfcn.h> // for dlopen

using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::Status;

using namespace android;
using namespace android::hardware;

// === mkdbg: start ===
#define DEBUG_HIDL_LOMO_ENABLED "debug.hidl.lomo.enabled"

#ifdef FUNC_START
#undef FUNC_START
#endif
#ifdef FUNC_START
#undef FUNC_END
#endif

#if 1
#define FUNC_START if (m_logLevel) { ALOGD("%s +", __FUNCTION__); }
#define FUNC_END if (m_logLevel) { ALOGD("%s -", __FUNCTION__); }
#else
#define FUNC_START
#define FUNC_END
#endif

#define MK_ION_MMAP_SHARE_PER_FRAME 0 // 1: OK, 0:  looks OK when NO allocLomoDst flow, but NG with it
#define MK_UT_HIDLLOMO_DUMP_SRC 0
#define MK_UT_DST_BUF_WO_P2_ENQUE 0
#define DEBUG_LOMO_LOG_LEVEL "vendor.debug.lomo.loglevel"
#define DEBUG_LOMO_DUMPSRC_ENABLED "vendor.debug.lomo.dumpsrc.enabled"
#define DEBUG_LOMO_DUMPSRC_COUNT "vendor.debug.lomo.dumpsrc.yv12.count"

#define LOMO_EFFECT_DST_BUFF_NUM 48 // this should align lomo_hal_jni.cpp
// === mkdbg: end ===
#define MY_DUMMY_LOG(fmt, arg...)        ALOGD("[%s] %s: " fmt,  LOG_TAG, __FUNCTION__, ##arg)

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace lomoeffect {
namespace V1_0 {
namespace implementation {

LomoIonInfo::LomoIonInfo()
{
    MY_DUMMY_LOG();
}


LomoIonInfo::~LomoIonInfo()
{
    MY_DUMMY_LOG();
}

void LomoIonInfo::reset()
{
    MY_DUMMY_LOG();
}

void LomoIonInfo::cleanup(const int ion_dev_fd)
{
    (void)ion_dev_fd;
    MY_DUMMY_LOG();
}

void LomoIonInfo::print(const int /*idx*/, const int /*logLevel*/)
{
    MY_DUMMY_LOG();
}

int LomoEffect::dlopenLomoHal(void)
{
    MY_DUMMY_LOG();
    return 0;
}

Return<Status> LomoEffect::init() {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::uninit() {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::allocLomoSrcImage(uint32_t /*pvCBWidth*/, uint32_t /*pvCBHeight*/, uint32_t /*pvCBformat*/, uint32_t /*ppSrcImgWidth*/, uint32_t /*ppSrcImgHeight*/) {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::allocLomoDstImage(uint32_t /*Number*/, int32_t /*size*/, const hidl_array<uint32_t, 3>& /*arrayPlaneOffset*/, const hidl_handle& /*dstHidlShareFd*/) {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}


// hidl_memory changed to hidl_handle
Return<Status> LomoEffect::uploadLomoSrcImage(const hidl_handle& /*srcHidlHandleShareFD*/, const hidl_array<uint32_t, 3>& /*arrayPlaneOffset*/) {
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::freeLomoSrcImage() {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::lomoImageEnque(uint32_t /*Number*/, const hidl_array<uint32_t, 3>& /*arrayPlaneOffset*/, int32_t /*ppEffectIdx*/) {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::lomoImageDeque(uint32_t /*Number*/, const hidl_array<uint32_t, 3>& /*arrayPlaneOffset*/, int32_t /*ppEffectIdx*/) {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::freeLomoDstImage() {
    MY_DUMMY_LOG();
    return ::android::hardware::camera::common::V1_0::Status {};
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

ILomoEffect* HIDL_FETCH_ILomoEffect(const char* /* name */) {
    MY_DUMMY_LOG();
    return new LomoEffect();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace hidl_lomo_hal
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
