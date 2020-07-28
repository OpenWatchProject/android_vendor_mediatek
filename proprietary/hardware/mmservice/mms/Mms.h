#ifndef VENDOR_MEDIATEK_HARDWARE_MMS_V1_5_MMS_H
#define VENDOR_MEDIATEK_HARDWARE_MMS_V1_5_MMS_H

#include <vendor/mediatek/hardware/mms/1.5/IMms.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <utils/Mutex.h>

#ifdef MMS_SUPPORT_JPG_ENC
#ifndef JUINT32
typedef unsigned int JUINT32;
#endif

#ifndef JINT32
typedef int JINT32;
#endif

#ifndef JUINT8
typedef unsigned char JUINT8;
#endif

typedef int ion_user_handle_t;
#endif

class DpAsyncBlitStream;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mms {
namespace V1_5 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::Mutex;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using ::vendor::mediatek::hardware::mms::V1_1::HwMDPParam;
using ::vendor::mediatek::hardware::mms::V1_2::MDPParam;
using ::vendor::mediatek::hardware::mms::V1_2::MMS_MEDIA_TYPE_ENUM;
using ::vendor::mediatek::hardware::mms::V1_2::MMS_PROFILE_ENUM;
using ::vendor::mediatek::hardware::mms::V1_3::MDPParamFD;
using ::vendor::mediatek::hardware::mms::V1_4::HwCopybitParamProfile;
using ::vendor::mediatek::hardware::mms::V1_5::HwMDPParamAUX;

struct Mms : public IMms {
    Mms();
    virtual ~Mms();

    // Methods from ::vendor::mediatek::hardware::mms::V1_0::IMms follow.
    Return<bool> isCopybitSupportedOutFormat(int32_t format) override;
    Return<int32_t> getCopybitDefaultOutFormat() override;
    Return<int32_t> getCopybitOutUsage(uint32_t usage) override;
    Return<int32_t> copybit(const ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam& copybitparam) override;
    Return<void> jpegencode(const ::vendor::mediatek::hardware::mms::V1_0::HwJpegEncoderConfig& cfg, jpegencode_cb _hidl_cb) override;
    Return<int32_t> jpegOpenClient(uint32_t width, uint32_t height) override;
    Return<int32_t> jpegCloseClient() override;
    // Methods from ::vendor::mediatek::hardware::mms::V1_1::IMms follow.
    Return<int32_t> mdp_run(const ::vendor::mediatek::hardware::mms::V1_1::HwMDPParam& mdpparam) override;
    // Methods from ::vendor::mediatek::hardware::mms::V1_2::IMms follow.
    Return<int32_t> BlitStream(const ::vendor::mediatek::hardware::mms::V1_2::MDPParam& mdpparam) override;
    // Methods from ::vendor::mediatek::hardware::mms::V1_3::IMms follow.
    Return<int32_t> BlitStreamFD(const ::vendor::mediatek::hardware::mms::V1_3::MDPParamFD& mdpparamfd) override;
    // Methods from ::vendor::mediatek::hardware::mms::V1_4::IMms follow.
    Return<int32_t> copybitprofile(const ::vendor::mediatek::hardware::mms::V1_4::HwCopybitParamProfile& copybitparamprofile) override;
    // Methods from ::vendor::mediatek::hardware::mms::V1_5::IMms follow.
    Return<int32_t> BlitStreamAUX(const ::vendor::mediatek::hardware::mms::V1_5::HwMDPParamAUX& mdpparam) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

protected:
    Mutex mLock;
    DpAsyncBlitStream *mAsyncBlitStream;
};

void copybit_Bufsize(uint32_t *size, int32_t colorFormat, int32_t planes, uint32_t width, uint32_t height);

#ifdef MMS_SUPPORT_JPG_ENC
bool mmsJpeg_query_buffer(JINT32 ion_client, ion_user_handle_t handle, JUINT32 *PAddr, JUINT32 *bufferSize, JINT32 MODULE_ID_ENUM);
bool mmsJpeg_getVA(JINT32 bufFD, JUINT32 size, void **VA);
void mmsJpeg_freeVA(JUINT32 size, void **VA);
#endif

// FIXME: most likely delete, this is only for passthrough implementations
extern "C" IMms* HIDL_FETCH_IMms(const char* name);

}  // namespace implementation
}  // namespace V1_5
}  // namespace mms
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_MMS_V1_5_MMS_H
