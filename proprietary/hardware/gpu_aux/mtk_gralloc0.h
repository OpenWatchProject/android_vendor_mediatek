#ifndef __MTK_GRALLOC0_H__
#define __MTK_GRALLOC0_H__

#include <stdlib.h>
#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
#include <system/window.h>

#include "mtk_gralloc.h"

class MTKGralloc0: public MTKGralloc
{
public:
    MTKGralloc0();
    ~MTKGralloc0();

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
    buffer_handle_t get_buffer_handle(android_native_buffer_t *pBuffer);
    gralloc_module_t  *m_pModule;
    alloc_device_t    *m_pDevice;
};

#endif  // __MTK_GRALLOC0_H__
