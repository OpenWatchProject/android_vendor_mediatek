#ifndef VENDOR_MEDIATEK_HARDWARE_NVRAM_V1_1_NVRAM_H
#define VENDOR_MEDIATEK_HARDWARE_NVRAM_V1_1_NVRAM_H

#include <vendor/mediatek/hardware/nvram/1.1/INvram.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace nvram {
namespace V1_1 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::nvram::V1_1::INvram;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Nvram : public INvram {
    // Methods from ::vendor::mediatek::hardware::nvram::V1_0::INvram follow.
    Return<void> readFileByName(const hidl_string& filename, uint32_t size, readFileByName_cb _hidl_cb) override;
    Return<int8_t> writeFileByNamevec(const hidl_string& filename, uint32_t size, const hidl_vec<uint8_t>& data) override;

    // Methods from ::vendor::mediatek::hardware::nvram::V1_1::INvram follow.
    Return<void> readFileBylid(int32_t lid, uint16_t para, readFileBylid_cb _hidl_cb) override;
	Return<void> writeFileBylid(int32_t lid, uint16_t para, const hidl_vec<uint8_t>& data,writeFileBylid_cb _hidl_cb) override;
	Return<bool> AddBackupFileNum(int32_t lid) override;
	Return<bool> ResetFileToDefault(int32_t lid) override;
	Return<void> ReadFileVerInfo(int32_t lid, ReadFileVerInfo_cb _hidl_cb) override;
	Return<bool> BackupData_Special(const hidl_vec<uint8_t>& data, uint32_t count, uint32_t mode) override;
	Return<bool> BackupAll_NvRam() override;
	Return<bool> RestoreAll_NvRam() override;
	Return<bool> BackupToBinRegion_All() override;
	Return<bool> BackupToBinRegion_All_Exx(const hidl_vec<uint8_t>& data) override;
    Return<bool> BackupToBinRegion_All_Ex(uint32_t value) override;		

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
extern "C" INvram* HIDL_FETCH_INvram(const char* name);

}  // namespace implementation
}  // namespace V1_1
}  // namespace nvram
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_NVRAM_V1_1_NVRAM_H
