#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_CALLBACKCLIENT_V1_0_MTKCALLBACKCLIENT_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_CALLBACKCLIENT_V1_0_MTKCALLBACKCLIENT_H

#include <vendor/mediatek/hardware/camera/callbackclient/1.1/IMtkCallbackClient.h>
#include <vendor/mediatek/hardware/camera/callbackclient/1.1/IMtkBufferCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <mutex>
#include <list>
#include <utils/KeyedVector.h>

#include "HandleImporter.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace callbackclient {
namespace V1_1 {
namespace implementation {

using namespace android;

using ::android::hardware::camera::common::V1_0::Status;
using ::android::hardware::camera::common::V1_0::helper::HandleImporter;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkCallbackClient;
using ::vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkBufferCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct MtkCallbackClient : public IMtkCallbackClient {
    // Methods from ::vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkCallbackClient follow.
    Return<Status> existCallbackClient  () override;
    Return<Status> createCallbackClient (int64_t timestamp) override;
    Return<Status> setOutputSurfaces    (int64_t timestamp, const sp<vendor::mediatek::hardware::camera::callbackclient::V1_0::IMtkBufferCallback>& bufferCB, const hidl_vec<int32_t>& msgType) override;
    Return<Status> destroyCallbackClient(int64_t timestamp) override;
    // Methods from IMtkCallbackClient follow.
    Return<::android::hardware::camera::common::V1_0::Status> sendCommand(int64_t timestamp, MtkCBClientCmdType cmd, int32_t arg1, int32_t arg2) override;
    // Methods from ::android::hidl::base::V1_0::IBase follow.
};

extern "C" IMtkCallbackClient* HIDL_FETCH_IMtkCallbackClient(const char* name);

/*
* Wrapper for calling CallbackClientMgr
*/
class CallbackClientWrap {

public:
    static CallbackClientWrap* getInstance();

    Return<Status> createCallbackClient (int64_t timestamp);
    Return<Status> setOutputSurfaces    (int64_t timestamp, const sp<IMtkBufferCallback>& bufferCB, int32_t size);
    Return<Status> destroyCallbackClient(int64_t timestamp);
    Return<Status> sendCommand          (int64_t timestamp, MtkCBClientCmdType cmd, int32_t arg1, int32_t arg2);
    bool onBufferCallback( int64_t const   i8Timestamp,
                           uint32_t const  u4BitstreamSize,
                           uint8_t const*  puBitstreamBuf,
                           uint32_t const  u4CallbackIndex,
                           bool            fgIsFinalImage,
                           uint32_t const  msgType,
                           uint32_t const  u4BufOffset4Ts,
                           uint32_t const u4CbId);

    bool onPostviewCallback(int64_t const  i8Timestamp,
                            uint32_t const planeCount,
                            uint32_t const width,
                            uint32_t const height,
                            uint32_t const fmt,
                            uint8_t* const u4Bitstream[],
                            uint32_t const widthInBytes[],
                            uint32_t const strideInBytes[],
                            uint32_t const scanlines[],
                            uint32_t const u4BufferOffsetForTimeStamp,
                            uint32_t const u4CbId
                            );

    static bool bufferCallback( int64_t const   i8Timestamp,
                                uint32_t const  u4BitstreamSize,
                                uint8_t const*  puBitstreamBuf,
                                uint32_t const  u4CallbackIndex,
                                bool            fgIsFinalImage,
                                uint32_t const  msgType,
                                uint32_t const  u4BufOffset4Ts,
                                uint32_t const u4CbId);

    static bool postviewCallback (int64_t const  i8Timestamp,
                                  uint32_t const planeCount,
                                  uint32_t const width,
                                  uint32_t const height,
                                  uint32_t const fmt,
                                  uint8_t* const u4Bitstream[],
                                  uint32_t const widthInBytes[],
                                  uint32_t const strideInBytes[],
                                  uint32_t const scanlines[],
                                  uint32_t const u4BufferOffsetForTimeStamp,
                                  uint32_t const u4CbId
                                  );


private:
     CallbackClientWrap() {};
    ~CallbackClientWrap() {};

private:
    static HandleImporter                        sHandleImporter;
    std::mutex                                   mCbMutex;            // mutex for onBufferCallback and postviewCallback
    std::mutex                                   mMutex;              // mutex for below members
    KeyedVector<int64_t, sp<IMtkBufferCallback>> mBufferCallbackMap;  //<timestamp, IMtkBufferCallback>
    KeyedVector<int64_t, uint32_t>               mCbIdMap;            //<timestamp, callback id>
    std::list<int64_t>                           mlTimestamp;         //timestamp to setCallbackClient

};// class CallbackClientWrap

}  // namespace implementation
}  // namespace V1_0
}  // namespace callbackclient
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor


#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_CALLBACKCLIENT_V1_0_MTKCALLBACKCLIENT_H
