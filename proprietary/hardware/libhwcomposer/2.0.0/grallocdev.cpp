#define DEBUG_LOG_TAG "GrallocDev"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "grallocdev.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <hardware/gralloc.h>

#include "utils/debug.h"
#include "utils/tools.h"

#ifdef USES_GRALLOC1
ANDROID_SINGLETON_STATIC_INSTANCE(GrallocDevice);

GrallocDevice::GrallocDevice()
    : m_dev(NULL)
{
    const hw_module_t* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (err)
    {
        HWC_LOGE("Failed to open gralloc device: %s (%d)", strerror(-err), err);
        return;
    }

    err = gralloc1_open(module, &m_dev);
    if (err)
    {
        HWC_LOGE("Failed to call gralloc1_open(): %s (%d)", strerror(-err), err);
    }

    initDispatch();
}

GrallocDevice::~GrallocDevice()
{
    if (m_dev) gralloc1_close(m_dev);
}

template<typename T>
void GrallocDevice::initDispatch(gralloc1_function_descriptor_t desc, T* outPfn)
{
    auto pfn = m_dev->getFunction(m_dev, desc);
    if (!pfn) {
        HWC_LOGE("Failed to get gralloc1 function : %d", desc);
    }
    *outPfn = reinterpret_cast<T>(pfn);
}

void GrallocDevice::initDispatch()
{
    initDispatch(GRALLOC1_FUNCTION_DUMP, &m_dispatch.dump);
    initDispatch(GRALLOC1_FUNCTION_CREATE_DESCRIPTOR, &m_dispatch.createDescriptor);
    initDispatch(GRALLOC1_FUNCTION_DESTROY_DESCRIPTOR, &m_dispatch.destroyDescriptor);
    initDispatch(GRALLOC1_FUNCTION_SET_DIMENSIONS, &m_dispatch.setDimensions);
    initDispatch(GRALLOC1_FUNCTION_SET_FORMAT, &m_dispatch.setFormat);
    initDispatch(GRALLOC1_FUNCTION_SET_CONSUMER_USAGE, &m_dispatch.setConsumerUsage);
    initDispatch(GRALLOC1_FUNCTION_SET_PRODUCER_USAGE, &m_dispatch.setProducerUsage);
    initDispatch(GRALLOC1_FUNCTION_ALLOCATE, &m_dispatch.allocate);
    initDispatch(GRALLOC1_FUNCTION_RELEASE, &m_dispatch.release);
    initDispatch(GRALLOC1_FUNCTION_LOCK, &m_dispatch.lock);
    initDispatch(GRALLOC1_FUNCTION_UNLOCK, &m_dispatch.unlock);
}

status_t GrallocDevice::alloc(AllocParam& param)
{
    HWC_ATRACE_CALL();
    status_t err;

    if (!m_dev) return NO_INIT;

    if ((param.width == 0) || (param.height == 0))
    {
        HWC_LOGE("Empty buffer(w=%u h=%u) allocation is not allowed",
                param.width, param.height);
        return INVALID_OPERATION;
    }

    gralloc1_buffer_descriptor_t descriptor;
    err = createDescriptor(param, &descriptor);
    if (err != GRALLOC1_ERROR_NONE)
    {
        HWC_LOGE("Failed to create descriptor buffer(w=%u h=%u f=%d usage=%08x): %s (%d)",
                param.width, param.height, param.format, param.usage,
                strerror(-err), err);
    }

    err = m_dispatch.allocate(m_dev, 1, &descriptor, &param.handle);

    if (err == GRALLOC1_ERROR_NONE)
    {
        HWC_LOGD("alloc gralloc memory(%p)", param.handle);
        // TODO: add allocated buffer record
    }
    else
    {
        HWC_LOGE("Failed to allocate buffer(w=%u h=%u f=%d usage=%08x): %s (%d)",
                param.width, param.height, param.format, param.usage,
                strerror(-err), err);
    }

    return err;
}

status_t GrallocDevice::free(buffer_handle_t handle)
{
    HWC_ATRACE_CALL();
    status_t err;

    if (!m_dev) return NO_INIT;

    err = m_dispatch.release(m_dev, handle);
    if (err == GRALLOC1_ERROR_NONE)
    {
        HWC_LOGD("free gralloc memory(%p)", handle);
        // TODO: remove allocated buffer record
    }
    else
    {
        HWC_LOGE("Failed to free buffer handle(%p): %s (%d)",
                handle, strerror(-err), err);
    }

    return err;
}

status_t GrallocDevice::createDescriptor(const AllocParam& param,
    gralloc1_buffer_descriptor_t* outDescriptor) {
    gralloc1_buffer_descriptor_t descriptor;

    status_t err = m_dispatch.createDescriptor(m_dev, &descriptor);

    if (err == GRALLOC1_ERROR_NONE)
    {
        err = m_dispatch.setDimensions(m_dev, descriptor, param.width, param.height);
    }

    if (err == GRALLOC1_ERROR_NONE) {
        err = m_dispatch.setFormat(m_dev, descriptor,
                                    static_cast<int32_t>(param.format));
    }

    if (err == GRALLOC1_ERROR_NONE) {
        err = m_dispatch.setProducerUsage(m_dev, descriptor, param.usage);
    }
    if (err == GRALLOC1_ERROR_NONE) {
        err = m_dispatch.setConsumerUsage(m_dev, descriptor, param.usage);
    }

    if (err == GRALLOC1_ERROR_NONE) {
        *outDescriptor = descriptor;
    } else {
        m_dispatch.destroyDescriptor(m_dev, descriptor);
    }

    return err;
}


void GrallocDevice::dump() const
{
    // TODO: dump allocated buffer record
}

#else // USES_GRALLOC1

ANDROID_SINGLETON_STATIC_INSTANCE(GrallocDevice);

GrallocDevice::GrallocDevice()
    : m_dev(NULL)
{
    const hw_module_t* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (err)
    {
        HWC_LOGE("Failed to open gralloc device: %s (%d)", strerror(-err), err);
        return;
    }

    gralloc_open(module, &m_dev);
}

GrallocDevice::~GrallocDevice()
{
    if (m_dev) gralloc_close(m_dev);
}

status_t GrallocDevice::alloc(AllocParam& param)
{
    HWC_ATRACE_CALL();
    status_t err;

    if (!m_dev) return NO_INIT;

    if ((param.width == 0) || (param.height == 0))
    {
        HWC_LOGE("Empty buffer(w=%u h=%u) allocation is not allowed",
                param.width, param.height);
        return INVALID_OPERATION;
    }

    err = m_dev->alloc(m_dev,
        param.width, param.height, param.format,
        param.usage, &param.handle, &param.stride);
    if (err == NO_ERROR)
    {
        HWC_LOGD("alloc gralloc memory(%p)", param.handle);
        // TODO: add allocated buffer record
    }
    else
    {
        HWC_LOGE("Failed to allocate buffer(w=%u h=%u f=%d usage=%08x): %s (%d)",
                param.width, param.height, param.format, param.usage,
                strerror(-err), err);
    }

    return err;
}

status_t GrallocDevice::free(buffer_handle_t handle)
{
    HWC_ATRACE_CALL();
    status_t err;

    if (!m_dev) return NO_INIT;

    err = m_dev->free(m_dev, handle);
    if (err == NO_ERROR)
    {
        HWC_LOGD("free gralloc memory(%p)", handle);
        // TODO: remove allocated buffer record
    }
    else
    {
        HWC_LOGE("Failed to free buffer handle(%p): %s (%d)",
                handle, strerror(-err), err);
    }

    return err;
}

void GrallocDevice::dump() const
{
    // TODO: dump allocated buffer record
}
#endif // USES_GRALLOC1
