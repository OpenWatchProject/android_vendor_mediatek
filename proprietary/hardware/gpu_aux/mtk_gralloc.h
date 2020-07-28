#ifndef __MTK_GRALLOC_H__
#define __MTK_GRALLOC_H__

#include <system/window.h>
#include <stdlib.h>
#include <utils/Mutex.h>

typedef struct GPUAUXBufferInfo_t {
    int err;

    int ion_fd;
    int width;
    int height;
    int format;
    int usage;
    int stride;
    int vertical_stride;
    int alloc_size;

    int status;
    int pool_id;
    int timestamp;
    int status2;
    int videobuffer_status;

    GPUAUXBufferInfo_t() :
        err(0), ion_fd(-1),
        width(0), height(0),
        format(0), usage(0),
        stride(0), vertical_stride(0),
        alloc_size(0), status(0),
        pool_id(0), timestamp(0),
        status2(0), videobuffer_status(0)
    {}
} GPUAUXBufferInfo;

class MTKGralloc
{
public:
    virtual ~MTKGralloc() {};

    virtual int32_t allocBuffer(uint32_t                width,
                                uint32_t                height,
                                uint32_t                format,
                                uint64_t                usage,
                                android_native_buffer_t **ppBuffer);

    virtual int32_t freeBuffer(android_native_buffer_t *pBuffer);

    virtual int32_t lockBuffer(android_native_buffer_t *pBuffer,
                               uint64_t                usage,
                               void                    **ppVA);

    virtual int32_t unlockBuffer(android_native_buffer_t *pBuffer);

    virtual GPUAUXBufferInfo getBufferInfo(android_native_buffer_t *pBuffer);

    virtual int32_t setBufferPara(android_native_buffer_t *pBuffer,
                                  uint32_t                mask,
                                  uint32_t                value);

    static MTKGralloc *getInstance();

    static void destroyInstance();

    uint64_t getUsage();

    uint64_t usage;

private:
    static MTKGralloc *s_pInst;
    static android::Mutex s_mutex;

};

#endif // __MTK_GRALLOC_H__