#ifndef __MTK_GRALLOC1_H__
#define __MTK_GRALLOC1_H__

#include <stdlib.h>
#include <hardware/gralloc1.h>
#include <ui/gralloc_extra.h>
#include <system/window.h>

#include "mtk_gralloc.h"

class MTKGralloc1: public MTKGralloc
{
public:
    MTKGralloc1();
    ~MTKGralloc1();

    static void destroyInstance();

    int32_t allocBuffer(uint32_t                width,
                        uint32_t                height,
                        uint32_t                format,
                        uint64_t                usage,
                        android_native_buffer_t **ppBuffer);

    int32_t freeBuffer(android_native_buffer_t *pBuffer);

    int32_t lockBuffer(android_native_buffer_t *pBuffer,
                       uint64_t                usage,
                       void                    **ppVA);

    int32_t unlockBuffer(android_native_buffer_t *pBuffer);

    GPUAUXBufferInfo getBufferInfo(android_native_buffer_t *pBuffer);

    int32_t setBufferPara(android_native_buffer_t *pBuffer,
                          uint32_t                mask,
                          uint32_t                value);

private:
    template<typename T>
    void initDispatch(gralloc1_function_descriptor_t desc, T *outPfn);
    void initDispatch();
    buffer_handle_t get_buffer_handle(android_native_buffer_t *pBuffer);
    hw_module_t       *m_pModule;
    gralloc1_device_t *m_pDevice;

    struct {
        GRALLOC1_PFN_DUMP dump;
        GRALLOC1_PFN_CREATE_DESCRIPTOR createDescriptor;
        GRALLOC1_PFN_DESTROY_DESCRIPTOR destroyDescriptor;
        GRALLOC1_PFN_SET_DIMENSIONS setDimensions;
        GRALLOC1_PFN_SET_FORMAT setFormat;
        GRALLOC1_PFN_SET_LAYER_COUNT setLayerCount;
        GRALLOC1_PFN_SET_CONSUMER_USAGE setConsumerUsage;
        GRALLOC1_PFN_SET_PRODUCER_USAGE setProducerUsage;
        GRALLOC1_PFN_ALLOCATE allocate;
        GRALLOC1_PFN_RELEASE release;
        GRALLOC1_PFN_LOCK lock;
        GRALLOC1_PFN_UNLOCK unlock;
    } mDispatch;
};

#endif  // __MTK_GRALLOC1_H__
