#ifndef VENDOR_MEDIATEK_HARDWARE_HDMI_V1_0_MTKHDMISERVICE_H
#define VENDOR_MEDIATEK_HARDWARE_HDMI_V1_0_MTKHDMISERVICE_H

#include <vendor/mediatek/hardware/hdmi/1.0/IMtkHdmiService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <utils/threads.h>
#include <log/log.h>

#include <map>
#include <string>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace hdmi {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::hdmi::V1_0::IMtkHdmiService;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::Thread;
using ::android::Mutex;

struct MtkHdmiService : public IMtkHdmiService {

    MtkHdmiService();
    ~MtkHdmiService();

    // Methods from ::vendor::mediatek::hardware::hdmi::V1_0::IMtkHdmiService follow.
    Return<void> get_resolution_mask(get_resolution_mask_cb _hidl_cb) override;

    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> enable_hdcp(bool enable) override;
    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> enable_hdmi(bool enable) override;
    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> set_video_resolution(int32_t resolution) override;
    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> enable_hdmi_hdr(bool enable) override;
    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> set_auto_mode(bool enable) override;
    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> set_color_format(int32_t color_format) override;
    Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> set_color_depth(int32_t color_depth) override;

    static void refreshEdid();
    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    void startObserving();
    bool setDrmKey();
    int enableHDMIInit(int value);
private:
    mutable Mutex mLock;

};

// FIXME: most likely delete, this is only for passthrough implementations
extern "C" IMtkHdmiService* HIDL_FETCH_IMtkHdmiService(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace hdmi
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_HDMI_V1_0_MTKHDMISERVICE_H
