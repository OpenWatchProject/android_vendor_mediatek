#ifndef VENDOR_MEDIATEK_HARDWARE_GPU_V1_0_GRAPHICEXT_H
#define VENDOR_MEDIATEK_HARDWARE_GPU_V1_0_GRAPHICEXT_H

#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>
#include <string.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/threads.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace gpu {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_HANDLE;
using ::vendor::mediatek::hardware::gpu::V1_0::GED_KPI_HANDLE;
using ::vendor::mediatek::hardware::gpu::V1_0::_crop_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_smvr_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_sf_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_hwc_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_ion_debug_t;
//using ::vendor::mediatek::hardware::gpu::V1_0::gralloc_rotate_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::gralloc_gpu_compression_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_hdr_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_video_info_t;
//using ::vendor::mediatek::hardware::gpu::V1_0::ge_misc_info_t;
using ::vendor::mediatek::hardware::gpu::V1_0::ge_timestamp_info_t;

class GraphicExtService : public IGraphicExt {
public:
    GraphicExtService();
    ~GraphicExtService();

    // Methods from IGraphicExt follow.
    Return<void> ged_create(ged_create_cb _hidl_cb) override;
    Return<GED_HIDL_ERROR_TAG> ged_destroy(const GED_HIDL_HANDLE& ged_handle) override;
    Return<GED_HIDL_ERROR_TAG> ged_vsync_calibration(const GED_HIDL_HANDLE& ged_handle, int32_t i32Delay, uint64_t nsVsync_period) override;
    Return<void> ged_is_gpu_need_vsync_calibration(ged_is_gpu_need_vsync_calibration_cb _hidl_cb) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_create(uint64_t BBQ_id) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_destroy(uint64_t BBQ_id) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_dequeue_buffer_tag(uint64_t BBQ_id, int32_t fence, uint32_t buffer_addr) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_queue_buffer_tag(uint64_t BBQ_id, int32_t fence, int32_t QedBuffer_length, uint32_t buffer_addr) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_acquire_buffer_tag(uint64_t BBQ_id, uint32_t buffer_addr) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_buffer_connect(uint64_t BBQ_id, int32_t BBQ_api_type, int32_t pid) override;
    Return<GED_HIDL_ERROR_TAG> ged_kpi_buffer_disconnect(uint64_t BBQ_id) override;
    Return<void> gralloc_extra_query(const android::hardware::hidl_handle& bufferHandle, ::vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, gralloc_extra_query_cb _hidl_cb) override;
    Return<int32_t> gralloc_extra_perform(const android::hardware::hidl_handle& bufferHandle, ::vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, const android::hardware::hidl_handle& dataHandle) override;
    Return<int32_t> gralloc_extra_perform_uint64(const android::hardware::hidl_handle& bufferHandle, ::vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, uint64_t in) override;
    Return<int32_t> gralloc_extra_perform_ion_debug(const android::hardware::hidl_handle& bufferHandle, ::vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, const ge_ion_debug_t& in) override;
    Return<int32_t> setupSendVsyncOffsetFuncCallback(const sp<IGraphicExtCallback>& callback) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

    int init_ged_service();
    int destroy_ged_service();
    static android::status_t updateVsyncOffset(int ns);
    static android::status_t disableHWC(int disable);
private:
    struct _GedKpiData {
        void* psKPI;
        int32_t mConnectedApi;
        int32_t mProducerPid;
        _GedKpiData()
        {
            psKPI = NULL;
            mConnectedApi = 0;
            mProducerPid = -1;
        }
    };

    int ged_service_init;
    static sp<IGraphicExtCallback> mGraphicExtCallback;
    // ged kpi handle list B
    android::Mutex mGedKpiHandleMapLock;
    std::map<uint64_t, _GedKpiData> mGedKpiHandleMap;
    int addGedKpiHandle(uint64_t BBQ_id, void *psKPI);
    _GedKpiData removeGedKpiHandle(uint64_t BBQ_id);
    _GedKpiData getGedKpiHandle(uint64_t BBQ_id);
    int connectGedKpiHandle(uint64_t BBQ_id, int32_t connectedApi, int32_t producerPid);
    int disconnectGedKpiHandle(uint64_t BBQ_id);
    // ged kpi handle list E
};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IGraphicExt* HIDL_FETCH_IGraphicExt(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace gpu
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_GPU_V1_0_GRAPHICEXT_H
