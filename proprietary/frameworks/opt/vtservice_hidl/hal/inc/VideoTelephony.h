#ifndef VENDOR_MEDIATEK_HARDWARE_VIDEOTELEPHONY_V1_0_VIDEOTELEPHONY_H
#define VENDOR_MEDIATEK_HARDWARE_VIDEOTELEPHONY_V1_0_VIDEOTELEPHONY_H

#include <vendor/mediatek/hardware/videotelephony/1.0/IVideoTelephony.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <fmq/MessageQueue.h>

#include <android/log.h>                                        /* for __android_log_print usage */
#include <utils/RefBase.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace videotelephony {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::videotelephony::V1_0::IVideoTelephony;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::MessageQueue;
using ::android::hardware::kSynchronizedReadWrite;
using ::android::hardware::kUnsynchronizedWrite;
using ::android::hardware::MQDescriptorSync;
using ::android::hardware::MQDescriptorUnsync;

// log function define
#define VT_HIDL_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "VT HIDL", __VA_ARGS__)
#define VT_HIDL_LOGV(...) //__android_log_print(ANDROID_LOG_VERBOSE, "VT", __VA_ARGS__)
#define VT_HIDL_LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "VT HIDL", __VA_ARGS__)
#define VT_HIDL_LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "VT HIDL", __VA_ARGS__)
#define VT_HIDL_LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "VT HIDL", __VA_ARGS__)

#define VT_HIDL_IS_SET(s, f)                    (s & f)
#define VT_HIDL_SET(s, f)                       (s |= f)

#define VT_HIDL_MAX_DATA_SIZE                   (102400)

#define VT_HIDL_TRUE                            (1)
#define VT_HIDL_FALSE                           (0)

#define VT_HIDL_INIT_STATUS_SET_MODE_DONE       (1)
#define VT_HIDL_INIT_STATUS_SET_ReadQ_DONE      (2)
#define VT_HIDL_INIT_STATUS_SET_WriteQ_DONE     (4)
#define VT_HIDL_INIT_STATUS_ALL_DONE            (7)

#define VT_HIDL_READ_WRITE_WAITTING_TIME        (0) // o mean wait forever

typedef MessageQueue<uint8_t, kSynchronizedReadWrite> vtHiDLFmq;

struct VideoTelephony : public IVideoTelephony {
    // Methods from ::vendor::mediatek::hardware::videotelephony::V1_0::IVideoTelephony follow.
    Return<uint16_t> configureFmqMode(uint16_t mode) override;
    Return<void> configureFmqSyncRead(configureFmqSyncRead_cb _hidl_cb) override;
    Return<void> configureFmqSyncWrite(configureFmqSyncWrite_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

   public:

    static VideoTelephony* getInstance();

    int getMode();

    static int isImsVideoCallon();
    static int isDynamicImsVideoCallon();
    static int isIMCBrun();

    std::unique_ptr<vtHiDLFmq>          mWriteFmq;
    std::unique_ptr<vtHiDLFmq>          mReadFmq;

    unsigned int                        mInitStatus;

   private:

    int initialization();

    static VideoTelephony*              sVideoTelephony;

    int                                 mMode;
};

static void restartProcess(void);

static void *VT_Reader_Thread(void *arg);
static void *VT_IMCB_Thread(void *arg);

static void VT_Bind(int *fd, const char *des);

static pthread_t  g_vt_vts_thd = 0;

static int g_vt_imcb_fd;
static pthread_t  g_vt_imcb_thd = 0;

extern "C" IVideoTelephony* HIDL_FETCH_IVideoTelephony(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace videotelephony
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_VIDEOTELEPHONY_V1_0_VIDEOTELEPHONY_H
