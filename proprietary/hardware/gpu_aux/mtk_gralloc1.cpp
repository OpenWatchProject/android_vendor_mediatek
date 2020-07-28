#include "mtk_gralloc1.h"
#include "utils.h"

MTKGralloc1::MTKGralloc1()
    : m_pModule(0),
      m_pDevice(0)
{
    int err;
    const hw_module_t *pModule;

    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pModule);
    if (err != 0) {
        MTK_LOGE("Error hw_get_module: %d", err);
    }
    m_pModule = (hw_module_t *)pModule;

    memset(&mDispatch, 0, sizeof(mDispatch));

    if (0 != m_pModule) {
        if (0 == m_pDevice) {
            if (gralloc1_open(m_pModule, &m_pDevice)) {
                MTK_LOGE("Failed to open gralloc1 device");
            }
            initDispatch();
            usage = GRALLOC1_CONSUMER_USAGE_CPU_READ | GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE;
        }
    }
}

MTKGralloc1::~MTKGralloc1()
{
    if (0 != m_pDevice) {
        gralloc1_close(m_pDevice);
        m_pDevice = 0;
    }
}

template<typename T>
void MTKGralloc1::initDispatch(gralloc1_function_descriptor_t desc, T *outPfn)
{
    auto pfn = m_pDevice->getFunction(m_pDevice, desc);
    if (!pfn) {
        MTK_LOGE("Failed to get gralloc1 function : %d", desc);
    } else {
        *outPfn = reinterpret_cast<T>(pfn);
    }
}

void MTKGralloc1::initDispatch()
{
    initDispatch(GRALLOC1_FUNCTION_DUMP, &mDispatch.dump);
    initDispatch(GRALLOC1_FUNCTION_CREATE_DESCRIPTOR, &mDispatch.createDescriptor);
    initDispatch(GRALLOC1_FUNCTION_DESTROY_DESCRIPTOR, &mDispatch.destroyDescriptor);
    initDispatch(GRALLOC1_FUNCTION_SET_DIMENSIONS, &mDispatch.setDimensions);
    initDispatch(GRALLOC1_FUNCTION_SET_FORMAT, &mDispatch.setFormat);
    //initDispatch(GRALLOC1_FUNCTION_SET_LAYER_COUNT, &mDispatch.setLayerCount);
    initDispatch(GRALLOC1_FUNCTION_SET_CONSUMER_USAGE, &mDispatch.setConsumerUsage);
    initDispatch(GRALLOC1_FUNCTION_SET_PRODUCER_USAGE, &mDispatch.setProducerUsage);
    initDispatch(GRALLOC1_FUNCTION_ALLOCATE, &mDispatch.allocate);
    initDispatch(GRALLOC1_FUNCTION_RELEASE, &mDispatch.release);
    initDispatch(GRALLOC1_FUNCTION_LOCK, &mDispatch.lock);
    initDispatch(GRALLOC1_FUNCTION_UNLOCK, &mDispatch.unlock);
}

buffer_handle_t MTKGralloc1::get_buffer_handle(android_native_buffer_t *pBuffer)
{
    if ((0 != pBuffer) &&
        (pBuffer->common.magic   == ANDROID_NATIVE_BUFFER_MAGIC) &&
        (pBuffer->common.version == sizeof(android_native_buffer_t))) {
        return pBuffer->handle;
    } else {
        if (0 != pBuffer) {
            MTK_LOGD("Buffer magic 0x%08x, version %d, width %d, height %d, stride %d",
                     pBuffer->common.magic, pBuffer->common.version, pBuffer->width, pBuffer->height, pBuffer->stride);
        }

        MTK_LOGE("Invalid native buffer %p", pBuffer);
    }

    return 0;
}

int32_t MTKGralloc1::allocBuffer(uint32_t                width,
                                 uint32_t                height,
                                 uint32_t                format,
                                 uint64_t                usage,
                                 android_native_buffer_t **ppBuffer)
{
    gralloc1_buffer_descriptor_t descriptor;
    int32_t status;

    if (0 != m_pDevice && mDispatch.createDescriptor != NULL
        && mDispatch.setDimensions != NULL
        && mDispatch.setFormat != NULL
        && mDispatch.setConsumerUsage != NULL
        && mDispatch.setProducerUsage != NULL
        && mDispatch.allocate != NULL) {
        status = mDispatch.createDescriptor(m_pDevice, &descriptor);
        if (status != GRALLOC1_ERROR_NONE) {
            MTK_LOGE("Failed to createDescriptor : %d", status);
            return status;
        }

        if (width % 2) { width += 1; }
        if (height % 2) { height += 1; }

        status = mDispatch.setDimensions(m_pDevice, descriptor, width, height);
        if (status != GRALLOC1_ERROR_NONE) {
            MTK_LOGE("Failed to set dimensions to (%u, %u) : %d", width, height, status);
            return status;
        }

        status = mDispatch.setFormat(m_pDevice, descriptor, format);
        if (status != GRALLOC1_ERROR_NONE) {
            MTK_LOGE("Failed to set format to %d : %d", format, status);
            return status;
        }

        status = mDispatch.setProducerUsage(m_pDevice, descriptor, usage);
        if (status != GRALLOC1_ERROR_NONE) {
            MTK_LOGE("Failed to set producer usage to %" PRIx64 " : %d", usage, status);
            return status;
        }

        status = mDispatch.setConsumerUsage(m_pDevice, descriptor, usage);
        if (status != GRALLOC1_ERROR_NONE) {
            MTK_LOGE("Failed to set consumer usage to %" PRIx64 " : %d", usage, status);
            return status;
        }

        status = mDispatch.allocate(m_pDevice, 1, &descriptor, &((*ppBuffer)->handle));
        if (status >= 0) {
            (*ppBuffer)->width  = width;
            (*ppBuffer)->height = height;
            (*ppBuffer)->format = format;
            (*ppBuffer)->usage  = usage;
        } else {
            MTK_LOGE("Failed to allocate %dx%d f(0x%08x) buffer : %d", width, height, format, status);
        }
    } else {
        MTK_LOGE("Invalid gralloc device");
        status = -1;
    }

    return status;
}

int32_t MTKGralloc1::freeBuffer(android_native_buffer_t *pBuffer)
{
    int32_t status = -1;

    if (0 != m_pDevice && mDispatch.release != NULL) {
        status = mDispatch.release(m_pDevice, pBuffer->handle);
        if (status >= 0) {
            pBuffer->width  = 0;
            pBuffer->height = 0;
            pBuffer->format = 0;
            pBuffer->usage  = 0;
        } else {
            MTK_LOGE("Failed to free buffer : %d", status);
        }
    } else {
        MTK_LOGE("Invalid gralloc device");
        status = -1;
    }

    return status;
}

int32_t MTKGralloc1::lockBuffer(android_native_buffer_t *pBuffer,
                                uint64_t                usage,
                                void                    **ppVA)
{
    buffer_handle_t         handle;
    gralloc1_rect_t accessRegion;
    int32_t     status;
    int         fenceFd = -1;

    if (0 != m_pModule && mDispatch.lock != NULL) {
        handle = get_buffer_handle(pBuffer);
        if (0 == handle) {
            MTK_LOGE("Failed to get buffer handle before lock");
            return -1;
        }

        accessRegion.left = 0;
        accessRegion.top = 0;
        accessRegion.width = pBuffer->width;
        accessRegion.height = pBuffer->height;

        status = mDispatch.lock(m_pDevice, handle, usage, usage, &accessRegion, ppVA, fenceFd);

        if (status < 0) {
            MTK_LOGE("Failed to lock buffer %p : %d", pBuffer, status);
        }
    } else {
        MTK_LOGE("Invalid gralloc module");
        status = -1;
    }

    return status;
}

int32_t MTKGralloc1::unlockBuffer(android_native_buffer_t *pBuffer)
{
    buffer_handle_t handle;
    int32_t     fenceFd = -1;
    int32_t         status;

    if (0 != m_pModule && mDispatch.unlock != NULL) {
        handle = get_buffer_handle(pBuffer);
        if (0 == handle) {
            MTK_LOGE("Failed to Get buffer handle before unlock");
            return -1;
        }

        status = mDispatch.unlock(m_pDevice, handle, &fenceFd);
        if (status < 0) {
            MTK_LOGE("Failed to unlock buffer %p : %d", pBuffer, status);
        }
    } else {
        MTK_LOGE("Invalid gralloc module");
        status = -1;
    }

    return status;
}

GPUAUXBufferInfo MTKGralloc1::getBufferInfo(android_native_buffer_t *pBuffer)
{
    GPUAUXBufferInfo info;
    int err = GRALLOC_EXTRA_OK;
    buffer_handle_t handle;

    handle = get_buffer_handle(pBuffer);

    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ION_FD, &info.ion_fd);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_WIDTH,  &info.width);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_HEIGHT, &info.height);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_FORMAT, &info.format);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_USAGE,  &info.usage);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_STRIDE, &info.stride);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_VERTICAL_STRIDE, &info.vertical_stride);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &info.alloc_size);

    if (GRALLOC_EXTRA_OK == err) {
        gralloc_extra_ion_sf_info_t sf_info;
        err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        info.status = sf_info.status;
        info.pool_id = sf_info.pool_id;
        info.timestamp = sf_info.timestamp;
        info.status2 = sf_info.status2;
        info.videobuffer_status = sf_info.videobuffer_status;
    }

    if (GRALLOC_EXTRA_OK != err) {
        MTK_LOGE("gralloc extra query error");
    }

    info.err = err;
    return info;
}


int32_t MTKGralloc1::setBufferPara(android_native_buffer_t *pBuffer,
                                   uint32_t                mask,
                                   uint32_t                value)
{
    buffer_handle_t handle;

    handle = get_buffer_handle(pBuffer);
    if (0 == handle) {
        MTK_LOGE("Failed to get buffer handle before setBufferPara");
        return -1;
    }

    if (handle) {
        gralloc_extra_ion_sf_info_t sf_info;

        gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        gralloc_extra_sf_set_status(&sf_info, mask, value);

        gralloc_extra_perform(handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
    }

    return 0;
}
