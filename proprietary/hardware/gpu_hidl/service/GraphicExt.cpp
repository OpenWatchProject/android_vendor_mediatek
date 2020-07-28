#include <log/log.h>
//#include <cutils/properties.h>
#include <math.h>

#include "GraphicExt.h"
#include "ged/ged_kpi.h"
#include "ged/ged2.h"
#include "gralloc_extra.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace gpu {
namespace V1_0 {
namespace implementation {

using ::android::status_t;

sp<IGraphicExtCallback> GraphicExtService::mGraphicExtCallback = NULL;

GraphicExtService::GraphicExtService() {
    status_t status;
    status = this->registerAsService();

    if(status != 0) {
        ALOGE("registerAsService() GraphicExtService failed status=[%d]\n", status);
    } else {
        ALOGV("registerAsService() GraphicExtService successful status=[%d]\n", status);
    }

    ged_service_init = 0;
}

GraphicExtService::~GraphicExtService() {
    mGraphicExtCallback = NULL;
    destroy_ged_service();
}

// Methods from IGraphicExtService follow.
Return<void> GraphicExtService::ged_create(ged_create_cb _hidl_cb) {
    GED_HIDL_ERROR_TAG retval = GED_HIDL_ERROR_TAG::GED_HIDL_OK;
    GED_HIDL_HANDLE ged_handle;

    ged_handle = ged_create_2();

    if (ged_handle.i32BridgeFD == 0) {
        retval = GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;
    }
    _hidl_cb(retval, ged_handle);
    return Void();
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_destroy(const GED_HIDL_HANDLE& ged_handle) {
    ::ged_destroy_2(ged_handle);
    return GED_HIDL_ERROR_TAG::GED_HIDL_OK;
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_vsync_calibration(const GED_HIDL_HANDLE& ged_handle, int32_t i32Delay, uint64_t nsVsync_period) {
    ::GED_ERROR err;
    GED_HIDL_ERROR_TAG retval = GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;

    err = ::ged_vsync_calibration((::GED_HANDLE)&ged_handle, i32Delay, nsVsync_period);
    if (err == GED_OK) {
        retval = GED_HIDL_ERROR_TAG::GED_HIDL_OK;
    }

    return retval;
}

Return<void> GraphicExtService::ged_is_gpu_need_vsync_calibration(ged_is_gpu_need_vsync_calibration_cb _hidl_cb) {
    GED_HIDL_ERROR_TAG retval = GED_HIDL_ERROR_TAG::GED_HIDL_OK;
    uint32_t isNeed = 0;

    // We no need vsync calibration on Android Q
    //char propbuf[PROP_VALUE_MAX];
    //property_get("ro.mediatek.platform", propbuf, "");
    //if (strstr(propbuf, "MT6750") != NULL) {
        //isNeed = 1;
    //}

    _hidl_cb(retval, isNeed);
    return Void();
}

/**
 * add a ged kpi handle to the map
 *
 * @BBQ_id the bufferqueueProducer ID
 * @psKPI the handle of gedKpi
 * @return 1 denote success. 0 denote fail
 */
int GraphicExtService::addGedKpiHandle(uint64_t BBQ_id, void *psKPI) {
    android::Mutex::Autolock _l(mGedKpiHandleMapLock);

    if (psKPI == NULL) {
        return 0;
    }

    _GedKpiData gedKpiData;

    gedKpiData.psKPI = psKPI;
    gedKpiData.mConnectedApi = 0;
    gedKpiData.mProducerPid = -1;

    mGedKpiHandleMap[BBQ_id] = gedKpiData;

    return 1;
}

/**
 * remove a ged kpi handle frome the map
 *
 * @BBQ_id the bufferqueueProducer ID
 * @return the handle that need be deleted. pass this to ged_kpi_destroy()
 */
GraphicExtService::_GedKpiData GraphicExtService::removeGedKpiHandle(uint64_t BBQ_id) {
    android::Mutex::Autolock _l(mGedKpiHandleMapLock);

    _GedKpiData gedKpiData;
    std::map<uint64_t, _GedKpiData>::iterator it;

    it = mGedKpiHandleMap.find(BBQ_id);
    if (it != mGedKpiHandleMap.end()) {
        gedKpiData = it->second;
        mGedKpiHandleMap.erase(it);
    }

    return gedKpiData;
}

GraphicExtService::_GedKpiData GraphicExtService::getGedKpiHandle(uint64_t BBQ_id) {
    android::Mutex::Autolock _l(mGedKpiHandleMapLock);

    _GedKpiData gedKpiData;
    std::map<uint64_t, _GedKpiData>::iterator it;

    it = mGedKpiHandleMap.find(BBQ_id);
    if (it != mGedKpiHandleMap.end()) {
        gedKpiData = it->second;
    }

    return gedKpiData;
}

/**
 * update connectedApi and producerPid
 *
 * @BBQ_id the bufferqueueProducer ID
 * @connectedApi connectedApi
 * @producerPid producerPid
 * @return 1 denote success. 0 denote fail
 */
int GraphicExtService::connectGedKpiHandle(uint64_t BBQ_id, int32_t connectedApi, int32_t producerPid) {
    android::Mutex::Autolock _l(mGedKpiHandleMapLock);

    std::map<uint64_t, _GedKpiData>::iterator it;

    it = mGedKpiHandleMap.find(BBQ_id);
    if (it != mGedKpiHandleMap.end()) {
        it->second.mConnectedApi = connectedApi;
        it->second.mProducerPid = producerPid;
        return 1;
    }
    return 0;
}

/**
 * buffer disconnect.
 * just set mProducerPid to -1
 *
 * @BBQ_id the bufferqueueProducer ID
 * @producerPid producerPid
 * @return 1 denote success. 0 denote fail
 */
int GraphicExtService::disconnectGedKpiHandle(uint64_t BBQ_id) {
    android::Mutex::Autolock _l(mGedKpiHandleMapLock);

    std::map<uint64_t, _GedKpiData>::iterator it;

    it = mGedKpiHandleMap.find(BBQ_id);
    if (it != mGedKpiHandleMap.end()) {
        it->second.mProducerPid = -1;
        return 1;
    }
    return 0;
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_create(uint64_t BBQ_id) {
    GED_HIDL_ERROR_TAG retval = GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;
    void *psKPI = NULL;

    psKPI = ::ged_kpi_create(BBQ_id);
    if (psKPI != NULL && psKPI != (void*)1) {
        if (addGedKpiHandle(BBQ_id, psKPI)) {
            retval = GED_HIDL_ERROR_TAG::GED_HIDL_OK;
        }
    }

    return retval;
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_destroy(uint64_t BBQ_id) {
    _GedKpiData gedKpiData = removeGedKpiHandle(BBQ_id);

    ::ged_kpi_destroy(gedKpiData.psKPI);

    return GED_HIDL_ERROR_TAG::GED_HIDL_OK;
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_dequeue_buffer_tag(uint64_t BBQ_id, int32_t fence, uint32_t buffer_addr) {
    _GedKpiData gedKpiData = getGedKpiHandle(BBQ_id);

    return (GED_HIDL_ERROR_TAG)::ged_kpi_dequeue_buffer_tag(gedKpiData.psKPI, gedKpiData.mConnectedApi, fence, gedKpiData.mProducerPid, buffer_addr);
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_queue_buffer_tag(uint64_t BBQ_id, int32_t fence, int32_t QedBuffer_length, uint32_t buffer_addr) {
    _GedKpiData gedKpiData = getGedKpiHandle(BBQ_id);

    return (GED_HIDL_ERROR_TAG)::ged_kpi_queue_buffer_tag(gedKpiData.psKPI, gedKpiData.mConnectedApi, fence, gedKpiData.mProducerPid, QedBuffer_length, buffer_addr);
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_acquire_buffer_tag(uint64_t BBQ_id, uint32_t buffer_addr) {
    _GedKpiData gedKpiData = getGedKpiHandle(BBQ_id);

    return (GED_HIDL_ERROR_TAG)::ged_kpi_acquire_buffer_tag(gedKpiData.psKPI, gedKpiData.mProducerPid, buffer_addr);
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_buffer_connect(uint64_t BBQ_id, int32_t BBQ_api_type, int32_t pid) {
    connectGedKpiHandle(BBQ_id, BBQ_api_type, pid);

    return GED_HIDL_ERROR_TAG::GED_HIDL_OK;
}

Return<GED_HIDL_ERROR_TAG> GraphicExtService::ged_kpi_buffer_disconnect(uint64_t BBQ_id)
{
    disconnectGedKpiHandle(BBQ_id);

    return GED_HIDL_ERROR_TAG::GED_HIDL_OK;
}

int32_t getDataIntCount(GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute) {
    int32_t intCount = 0;

    if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_ION_FD
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_WIDTH
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_HEIGHT
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_STRIDE
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_VERTICAL_STRIDE
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_ALLOC_SIZE
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_FORMAT
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_USAGE
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_VERTICAL_2ND_STRIDE
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE
     || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_ORIENTATION
    ) {
        intCount = ceil(static_cast<float>(sizeof(int32_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SECURE_HANDLE
            || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC
    ) {
        intCount =  ceil(static_cast<float>(sizeof(uint32_t)) / sizeof(int));
     } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_ID
             || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_CONSUMER_USAGE
             || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_PRODUCER_USAGE
             || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_TIMESTAMP
    ) {
        intCount = ceil(static_cast<float>(sizeof(uint64_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SMVR_INFO) {
        intCount = ceil(static_cast<float>(sizeof(ge_smvr_info_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SF_INFO
            || attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO) {
        intCount = ceil(static_cast<float>(sizeof(ge_sf_info_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_HWC_INFO) {
        intCount = ceil(static_cast<float>(sizeof(ge_hwc_info_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG) {
        intCount = ceil(static_cast<float>(sizeof(ge_ion_debug_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_GPU_COMPRESSION_INFO) {
        intCount = ceil(static_cast<float>(sizeof(gralloc_gpu_compression_info_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_HDR_INFO) {
        intCount = ceil(static_cast<float>(sizeof(ge_hdr_info_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_VIDEO_INFO) {
        intCount = ceil(static_cast<float>(sizeof(ge_video_info_t)) / sizeof(int));
    } else if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_TIMESTAMP_INFO) {
        intCount = ceil(static_cast<float>(sizeof(ge_timestamp_info_t)) / sizeof(int));
    }

    return intCount;
}

Return<void> GraphicExtService::gralloc_extra_query(const hidl_handle& bufferHandle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute,
    gralloc_extra_query_cb _hidl_cb) {
    int32_t retval = (int32_t )GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_INVALIDE_PARAMS;

    int intCount = getDataIntCount(attribute);
    native_handle_t* nativeHandle = native_handle_create(0, intCount);

    if (nativeHandle != NULL) {
        if (attribute == GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG) {
            ge_ion_debug_t *out = (ge_ion_debug_t*)&(nativeHandle->data[0]);
            ::ge_ion_debug_t ged_out;
            retval = ::gralloc_extra_query(bufferHandle, (::GRALLOC_EXTRA_ATTRIBUTE_QUERY)attribute, &ged_out);
            if (retval == (int32_t)GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK) {
                memcpy(&(out->data), &(ged_out.data), sizeof(int32_t) * 4);
                out->name = std::string(ged_out.name);
            }
        } else {
            retval = ::gralloc_extra_query(bufferHandle, (::GRALLOC_EXTRA_ATTRIBUTE_QUERY)attribute, (void*)&(nativeHandle->data[0]));
        }
    } else {
        retval = (int32_t )GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_ERROR;
    }

    _hidl_cb(retval, nativeHandle);

    if (nativeHandle) {
        native_handle_close(nativeHandle);
        native_handle_delete(nativeHandle);
    }

    return Void();
}

Return<int32_t> GraphicExtService::gralloc_extra_perform(const hidl_handle& bufferHandle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, const hidl_handle& dataHandle) {
    int32_t retval = (int32_t)GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_INVALIDE_PARAMS;
    void *in = (void *)&(dataHandle->data[0]);

    retval = ::gralloc_extra_perform(bufferHandle, (::GRALLOC_EXTRA_ATTRIBUTE_PERFORM)attribute, in);

    return retval;
}

Return<int32_t> GraphicExtService::gralloc_extra_perform_uint64(const hidl_handle& bufferHandle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, uint64_t in) {
    int32_t retval = (int32_t )GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_INVALIDE_PARAMS;

    if (attribute == GRALLOC_EXTRA_ATTRIBUTE_PERFORM::GRALLOC_EXTRA_SET_TIMESTAMP) {
        retval = ::gralloc_extra_perform(bufferHandle, (::GRALLOC_EXTRA_ATTRIBUTE_PERFORM)attribute, (void*)&in);
    }

    return retval;
}

Return<int32_t> GraphicExtService::gralloc_extra_perform_ion_debug(const hidl_handle& bufferHandle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, const ge_ion_debug_t& in) {
    int32_t retval = (int32_t )GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_INVALIDE_PARAMS;

    ::ge_ion_debug_t ged_in;
    if (attribute == GRALLOC_EXTRA_ATTRIBUTE_PERFORM::GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG) {
        memcpy(&(ged_in.data), &(in.data), sizeof(int32_t) * 4);
        strlcpy(ged_in.name, in.name.c_str(), sizeof(ged_in.name));
        ged_in.name[sizeof(ged_in.name) - 1] = '\0';
        retval = ::gralloc_extra_perform(bufferHandle, (::GRALLOC_EXTRA_ATTRIBUTE_PERFORM)attribute, &ged_in);
    }

    return retval;
}

Return<int32_t> GraphicExtService::setupSendVsyncOffsetFuncCallback(const sp<IGraphicExtCallback>& callback) {
    mGraphicExtCallback = callback;

    if (!ged_service_init) {
        // create vsync_offset_worker to send callback to client.
        init_ged_service();
        ged_service_init = 1;
    }

    return 0;
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IGraphicExt* HIDL_FETCH_IGraphicExt(const char* /* name */) {
    //return new GraphicExt();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace gpu
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
