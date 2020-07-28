#ifndef MTK_HARDWARE_CAMERA_ISPHAL_ISPCALLBACK_H
#define MTK_HARDWARE_CAMERA_ISPHAL_ISPCALLBACK_H

#include <vendor/mediatek/hardware/camera/isphal/1.0/ICallback.h>
#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>

#include <hidl/Status.h>
#include <utils/Mutex.h> // android::Mutex
#include <utils/Condition.h> // android::Condition
#include <mtkcam3/main/standalone_isp/IISPPipelineModelCallback.h>
#include "../meta_converter/ISPMetadataConverter.h"

#include <map>
#include <memory>
#include <vector>

namespace NSCam {
namespace ISPHal {

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

//using ::vendor::mediatek::hardware::camera::isphal;
using ::vendor::mediatek::hardware::camera::isphal::V1_0::ICallback;
using ::vendor::mediatek::hardware::camera::isphal::V1_0::Status;
using namespace NSCam::ISPHal;
using namespace NSCam::v3;
using namespace NSCam::v3::isppipeline::model;

class ISPCallback
    : public IISPPipelineModelCallback
{
public:
    virtual             ~ISPCallback();
                        ISPCallback(android::sp<ICallback> cb, android::sp<ISPMetadataConverter> converter);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Operations.
    virtual auto        onFrameUpdated(
                            UserOnFrameUpdated const& params
                            ) -> void;
    virtual auto        registerReq(
                            uint32_t const& req
                            ) -> void;

    virtual auto        waitForReqDrained(
                            int32_t const& operation __unused
                            ) -> void;
private:
    auto handleCallbacktoHidl(uint32_t reqNo) -> void;
    auto handlePartialResult(uint32_t reqNo, std::vector<android::sp<IMetaStreamBuffer>> vOutMeta) -> bool;
    auto handleEarlyCallback(uint32_t reqNo, const IMetadata *pMetadata) -> void;
private:
    android::sp<ICallback>             mISPCallback = nullptr;
    android::sp<ISPMetadataConverter>  mMetadataConverter = nullptr;
    mutable ::android::Mutex              mLock;
    android::Condition                    mWaitCond;
    struct CallbackResult
    {
        bool hasError = false;
        std::shared_ptr<IMetadata> data = nullptr;
        CallbackResult(std::shared_ptr<IMetadata> metadata)
            : data(metadata) {}
    };
    std::unordered_map<uint32_t, CallbackResult > mResultMetaMap;
};

}  // namespace ISPHal
}  // namespace NSCam

#endif  // MTK_HARDWARE_CAMERA_ISPHAL_ISPCALLBACK_H