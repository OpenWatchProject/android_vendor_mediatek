#include "mtk_gralloc0.h"
#include "utils.h"

MTKGralloc0::MTKGralloc0()
    : m_pModule(0),
      m_pDevice(0)
{
    int err;
    const hw_module_t *pModule;

    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pModule);
    if (err != 0) {
        MTK_LOGE("Error hw_get_module: %d", err);
    }
    m_pModule = (gralloc_module_t *)pModule;

    if (0 != pModule) {
        if (0 == m_pDevice) {
            gralloc_open(pModule, &m_pDevice);
            usage = GRALLOC_USAGE_SW_READ_RARELY | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE;
        }
    }
}

MTKGralloc0::~MTKGralloc0()
{
    if (0 != m_pDevice) {
        gralloc_close(m_pDevice);
        m_pDevice = 0;
    }
}

buffer_handle_t MTKGralloc0::get_buffer_handle(android_native_buffer_t *pBuffer)
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

int32_t MTKGralloc0::allocBuffer(uint32_t                width,
                                 uint32_t                height,
                                 uint32_t                format,
                                 uint64_t                usage,
                                 android_native_buffer_t **ppBuffer)
{
    int32_t status;

    if (0 != m_pDevice) {
        if (width % 2) { width += 1; }
        if (height % 2) { height += 1; }

        status = m_pDevice->alloc(m_pDevice,
                                  width,
                                  height,
                                  format,
                                  (uint32_t)usage,
                                  &((*ppBuffer)->handle),
                                  &((*ppBuffer)->stride));
        if (status >= 0) {
            (*ppBuffer)->width  = width;
            (*ppBuffer)->height = height;
            (*ppBuffer)->format = format;
            (*ppBuffer)->usage  = (uint32_t)usage;
        } else {
            MTK_LOGE("Allocate %dx%d f(0x%08x) buffer failed %d", width, height, format, status);
        }
    } else {
        MTK_LOGE("Invalid gralloc device");
        status = -1;
    }

    return status;
}

int32_t MTKGralloc0::freeBuffer(android_native_buffer_t *pBuffer)
{
    int32_t status = -1;

    if (0 != m_pDevice) {
        status = m_pDevice->free(m_pDevice, pBuffer->handle);
        if (status >= 0) {
            pBuffer->width  = 0;
            pBuffer->height = 0;
            pBuffer->format = 0;
            pBuffer->usage  = 0;
        } else {
            MTK_LOGE("Free buffer failed %d", status);
        }
    } else {
        MTK_LOGE("Invalid gralloc device");
        status = -1;
    }

    return status;
}

int32_t MTKGralloc0::lockBuffer(android_native_buffer_t *pBuffer,
                                uint64_t                usage,
                                void                    **ppVA)
{
    int32_t                 status;
    buffer_handle_t         handle;

    if (0 != m_pModule) {
        handle = get_buffer_handle(pBuffer);
        if (0 == handle) {
            MTK_LOGE("Get buffer handle failed");
            return -1;
        }

        status = m_pModule->lock(m_pModule,
                                 handle,
                                 (uint32_t)usage,
                                 0,
                                 0,
                                 pBuffer->width,
                                 pBuffer->height,
                                 ppVA);
        if (status < 0) {
            MTK_LOGE("Lock buffer %p failed %d", pBuffer, status);
        }
    } else {
        MTK_LOGE("Invalid extra device");
        status = -1;
    }

    return status;
}

int32_t MTKGralloc0::unlockBuffer(android_native_buffer_t *pBuffer)
{
    int32_t         status;
    buffer_handle_t handle;

    if (0 != m_pModule) {
        handle = get_buffer_handle(pBuffer);
        if (0 == handle) {
            MTK_LOGE("Get buffer handle failed");
            return -1;
        }

        status = m_pModule->unlock(m_pModule, handle);
        if (status < 0) {
            MTK_LOGE("Unlock buffer %p failed %d", pBuffer, status);
        }
    } else {
        MTK_LOGE("Invalid extra device");
        status = -1;
    }

    return status;
}

GPUAUXBufferInfo MTKGralloc0::getBufferInfo(android_native_buffer_t *pBuffer)
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
        MTK_LOGE("error found");
    }

    info.err = err;
    return info;
}


int32_t MTKGralloc0::setBufferPara(android_native_buffer_t *pBuffer,
                                   uint32_t                mask,
                                   uint32_t                value)
{
    buffer_handle_t handle;

    handle = get_buffer_handle(pBuffer);
    if (0 == handle) {
        MTK_LOGE("Get buffer handle failed");
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
