#ifndef HWC_GRALLOC_DEV_H
#define HWC_GRALLOC_DEV_H

#include <utils/Singleton.h>

#ifdef USES_GRALLOC1
#include <hardware/gralloc1.h>
#else
#include <hardware/gralloc.h>
#endif // USES_GRALLOC1

using namespace android;

#ifdef USES_GRALLOC1
class GrallocDevice : public Singleton<GrallocDevice>
{
public:
    GrallocDevice();
    ~GrallocDevice();

    template<typename T>
    void initDispatch(gralloc1_function_descriptor_t desc, T* outPfn);
    void initDispatch();

    struct AllocParam
    {
        AllocParam()
            : width(0), height(0), format(0)
            , usage(0), handle(NULL), stride(0)
        { }

        unsigned int width;
        unsigned int height;
        int format;
        int usage;

        buffer_handle_t handle;
        int stride;
    };

    // allocate memory by gralloc driver
    status_t alloc(AllocParam& param);

    // free a previously allocated buffer
    status_t free(buffer_handle_t handle);

    status_t createDescriptor(const AllocParam& param,
    gralloc1_buffer_descriptor_t* outDescriptor);

    // dump information of allocated buffers
    void dump() const;

private:
    gralloc1_device_t* m_dev;
    struct GrallocPfn{
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

        GrallocPfn()
            : dump(nullptr)
            , createDescriptor(nullptr)
            , destroyDescriptor(nullptr)
            , setDimensions(nullptr)
            , setFormat(nullptr)
            , setLayerCount(nullptr)
            , setConsumerUsage(nullptr)
            , setProducerUsage(nullptr)
            , allocate(nullptr)
            , release(nullptr)
            , lock(nullptr)
            , unlock(nullptr)
        {}
    } m_dispatch;
};

#else // USES_GRALLOC1

class GrallocDevice : public Singleton<GrallocDevice>
{
public:
    GrallocDevice();
    ~GrallocDevice();

    struct AllocParam
    {
        AllocParam()
            : width(0), height(0), format(0)
            , usage(0), handle(NULL), stride(0)
        { }

        unsigned int width;
        unsigned int height;
        int format;
        int usage;

        buffer_handle_t handle;
        int stride;
    };

    // allocate memory by gralloc driver
    status_t alloc(AllocParam& param);

    // free a previously allocated buffer
    status_t free(buffer_handle_t handle);

    // dump information of allocated buffers
    void dump() const;

private:
    alloc_device_t* m_dev;
};
#endif // USES_GRALLOC1

#endif
