#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_BGSERVICE_V1_0_BGSERVICE_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_BGSERVICE_V1_0_BGSERVICE_H

#include <vendor/mediatek/hardware/camera/bgservice/1.1/IBGService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/Mutex.h> // android::Mutex

#include <mtkcam/utils/std/Sync.h>
#include <mtkcam/utils/std/Trace.h>
#include <utils/String8.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace bgservice {
namespace V1_1 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct BGService
    : public IBGService
    , public android::hardware::hidl_death_recipient
{
    // Methods from ::vendor::mediatek::hardware::camera::bgservice::V1_0::IBGService follow.
    Return<int32_t> setEventCallback(int32_t ImgReaderId, const sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback>& callback) override;

    // Methods from ::vendor::mediatek::hardware::camera::bgservice::V1_1::IBGService follow.
    Return<int32_t> setEventCallback_1_1(int32_t ImgReaderId, const sp<::vendor::mediatek::hardware::camera::bgservice::V1_1::IEventCallback>& callback) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Methods from ::android::hardware::hidl_death_recipient
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void                serviceDied( uint64_t cookie,
                                             const android::wp<android::hidl::base::V1_0::IBase>& who) override;
};

// FIXME: most likely delete, this is only for passthrough implementations
extern "C" IBGService* HIDL_FETCH_IBGService(const char* name);

/*
* Wrapper for calling BGServiceMgr
*/
class BGServiceWrap{

public:
    static BGServiceWrap* getInstance();

    Return<int32_t> setEventCallback(const sp<IEventCallback>& callback, std::string& version);
    Return<int32_t> setEventCallback(const sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback>& callback, std::string& version);

    void resetCallback();
    void getCallback(sp<IEventCallback>& callback);
    void getCallback(sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback>& callback);
    
    bool onRequestCallback( int32_t const ImgReaderId,
                            int32_t  const frameNumber,
                            uint32_t            status,
                            uint64_t const timestamp);

    static bool requestCallback( int32_t const ImgReaderId,
                                 int32_t  const frameNumber,
                                 uint32_t            status,
                                 uint64_t const timestamp);

private:
     BGServiceWrap() {};
    ~BGServiceWrap() {};



private:
    sp<IEventCallback>                       mv11EventCallback = nullptr;
    sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback> mEventCallback = nullptr;
    std::string                              mVersion;
    mutable android::Mutex                   mLock;
};// class BGServiceWrap


}  // namespace implementation
}  // namespace V1_0
}  // namespace bgservice
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_BGSERVICE_V1_0_BGSERVICE_H
