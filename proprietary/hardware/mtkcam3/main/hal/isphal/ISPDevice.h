#ifndef HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_CAMERA_ISPHAL_V1_0_ISPDEVICE_H
#define HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_CAMERA_ISPHAL_V1_0_ISPDEVICE_H

#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>
#include <vendor/mediatek/hardware/camera/isphal/1.0/IDevice.h>

#include <mtkcam3/main/standalone_isp/IISPPipelineModel.h>

#include <hidl/Status.h>
//#include <utils/Mutex.h> // android::Mutex
#include <mutex>
#include "IISPManager.h"
#include "callback/ISPCallback.h"
#include "meta_converter/ISPMetadataConverter.h"
#include "streamcontroller/ISPStreamController.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace isphal {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

//using ::vendor::mediatek::hardware::camera::isphal;
using ::vendor::mediatek::hardware::camera::isphal::V1_0::IDevice;
using ::vendor::mediatek::hardware::camera::isphal::V1_0::Status;
using namespace NSCam;
using namespace NSCam::ISPHal;
using namespace NSCam::v3::isppipeline::model;
//using ::android::hardware::camera::device::V3_2::CameraMetadata;

class ISPDevice
    : public IDevice
    , public android::hardware::hidl_death_recipient
{
public:
    virtual             ~ISPDevice();
                        ISPDevice(int32_t id);
public:
    virtual ::android::hardware::Return<void> getISPSupportOutputFormat(getISPSupportOutputFormat_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> getISPSupportInputFormat(getISPSupportInputFormat_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> getISPSupportFeatures(getISPSupportFeatures_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> queryFeatureSetting( const ::vendor::mediatek::hardware::camera::isphal::V1_0::StatisticsInfo& info
                                                                   , queryFeatureSetting_cb _hidl_cb) override;

    virtual ::android::hardware::Return<int32_t> getISPTuningDataSize() override;

    virtual ::android::hardware::Return<Status> configureISPDevice(const ::vendor::mediatek::hardware::camera::isphal::V1_0::ISPStreamConfiguration& config
                                                                  , const ::android::sp<::vendor::mediatek::hardware::camera::isphal::V1_0::ICallback>& Cb) override;

    virtual ::android::hardware::Return<Status> closeISPDevice() override;

    virtual ::android::hardware::Return<Status> processImgRequest(const hidl_vec<::vendor::mediatek::hardware::camera::isphal::V1_0::ISPRequest>& requests) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Methods from ::android::hardware::hidl_death_recipient
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void                serviceDied( uint64_t cookie,
                                             const android::wp<android::hidl::base::V1_0::IBase>& who) override;
private:
    auto                        checkReconfigure(
                                                // input
                                                const std::vector<std::shared_ptr<UserRequestParams>> &requests,
                                                const std::shared_ptr<UserConfigurationParams> &newParam,
                                                // output
                                                std::shared_ptr<UserConfigurationParams> &currentParam) -> MBOOL;
private:
    int32_t mId = 0;
    std::timed_mutex                        mLock;
    android::sp<IISPPipelineModel>          mPipelineModel = nullptr;
    android::sp<IISPPipelineModelCallback>  mPipelineCallback = nullptr;
    android::sp<ISPMetadataConverter>         mMetaConverter = nullptr;
    android::sp<ISPStreamController>          mStreamCtrl = nullptr;
    android::sp<::vendor::mediatek::hardware::camera::isphal::V1_0::ICallback> mHidlCallback = nullptr;
    int32_t mCamId = -1;
    std::vector<int32_t> mSensorIds;
    std::shared_ptr<UserConfigurationParams> mConfigParams;

};

}  // namespace implementation
}  // namespace V1_0
}  // namespace isphal
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_CAMERA_ISPHAL_V1_0_ISPDEVICE_H
