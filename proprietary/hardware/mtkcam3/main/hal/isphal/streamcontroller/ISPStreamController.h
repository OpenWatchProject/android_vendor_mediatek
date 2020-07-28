#ifndef _MTK_HARDWARE_MTKCAM_ISPHIDL_STREAMCONTROLLER_H_
#define _MTK_HARDWARE_MTKCAM_ISPHIDL_STREAMCONTROLLER_H_

#include <hidl/Status.h>
#include <mutex>
#include <utils/Mutex.h> // android::Mutex

#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>
#include <mtkcam3/main/standalone_isp/types.h>
#include "../meta_converter/ISPMetadataConverter.h"
//
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h>
//
#include "ISPStreamBuffers.h"

namespace NSCam {
namespace ISPHal {

using android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using namespace vendor::mediatek::hardware::camera::isphal;
using namespace NSCam::v3::isppipeline::model;
using namespace NSCam::v3;
using UserConfigurationParams = NSCam::v3::isppipeline::model::UserConfigurationParams;
using MetaStreamInfo = NSCam::v3::Utils::MetaStreamInfo;
using ImageStreamInfo = NSCam::v3::Utils::ImageStreamInfo;
using IImageStreamInfo = NSCam::v3::IImageStreamInfo;
using ISPStreamBufferHandle = NSCam::v3::ISPImageStreamBuffer::ISPStreamBufferHandle;

class ISPStreamController
    : public virtual android::RefBase
{
public:
    virtual             ~ISPStreamController();
                        ISPStreamController(int32_t id, android::sp<ISPMetadataConverter> converter);
public:
    virtual auto    configure(
                        const V1_0::ISPStreamConfiguration& config,
                        std::shared_ptr<UserConfigurationParams>& params /*out*/
                        ) -> int;
    virtual auto    parseRequests(
                        const hidl_vec<V1_0::ISPRequest>& hidl_reqs,
                        std::vector<std::shared_ptr<UserRequestParams>>& parsed_reqs /*out*/,
                        std::shared_ptr<UserConfigurationParams>& params /*out*/
                        ) -> int;

struct ProprietaryRawInfo
{
    int32_t format;
    int32_t stride = 0;
    int32_t width = 0;
    int32_t height = 0;
};

private:
    int32_t                                                         mId = 0;
    int32_t                                                         mLogLevel = 0;
    std::timed_mutex                                                mLock;
    android::sp<ISPMetadataConverter>                               mMetaConverter;
    mutable android::Mutex                                          mStreamConfigMapLock;
    android::KeyedVector<StreamId_T, android::sp<IImageStreamInfo>> mStreamConfigMap;
    mutable android::Mutex                                          mMetaConfigMapLock;
    android::KeyedVector<StreamId_T, android::sp<IMetaStreamInfo>>  mMetaConfigMap;

    //create HAL image stream info for certain streamId
    virtual auto    createImageStreamInfo(
                        const uint64_t& srcStreamId,
                        const MUINT32& streamType
                    ) -> android::sp<IImageStreamInfo>;

    //create image stream info for the stream configured from App
    virtual auto    createImageStreamInfo(
                        const V1_0::ISPStream& srcStream,
                        const MUINT32& streamType
                    ) -> android::sp<IImageStreamInfo>;

    virtual auto    queryPlanesInfo(
                        const ISPStream& stream,
                        ImageStreamInfo::BufPlanes_t& planes
                    ) -> int;

    virtual auto    convertImageStreamBuffer(
                        const std::string& bufferName,
                        const hidl_handle& bufferHandle,
                        const uint64_t streamId,
                        android::sp<ISPImageStreamBuffer>& pStreamBuffer /*output*/,
                        const ProprietaryRawInfo& rawInfo
                    )   const -> int;

    virtual auto    createImageStreamBuffer(
                        const std::string& bufferName,
                        const android::sp<IImageStreamInfo>& streamInfo,
                        buffer_handle_t bufferHandle,
                        std::shared_ptr<ISPStreamBufferHandle> ispBufferHandle,
                        int const acquire_fence,
                        int const release_fence,
                        const ProprietaryRawInfo& rawInfo
                    )   const -> ISPImageStreamBuffer*;

     virtual auto    createMetaStreamBuffer(
                        android::sp<IMetaStreamInfo> pStreamInfo,
                        IMetadata const& rSettings
                    )   const -> ISPMetaStreamBuffer*;

     virtual auto    convertTuningMetaBuffer(
                        const std::string& bufferName,
                        const ISPBuffer& streamBuffer,
                        android::sp<IImageBufferHeap>& pImgBufHeap, /*output*/
                        buffer_handle_t& importedTuningBufferHandle /*output*/
                     )  const -> int;
};

};  // namespace ISPHal
};  // namespace NSCam

#endif  // _MTK_HARDWARE_MTKCAM_ISPHIDL_STREAMCONTROLLER_H_
