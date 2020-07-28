
#include "aux/GpuAux.h"

#include "GuiExtAux.h"

#include "utils.h"
#ifdef MTK_GPUD_SUPPORT
#include <gpud/gpud_api.h>
#endif

static void incRef(android_native_buffer_t *buffer)
{
    buffer->common.incRef(&buffer->common);
}
static void decRef(android_native_buffer_t *buffer)
{
    buffer->common.decRef(&buffer->common);
}

struct GPUAUXContext {
private:
    GuiExtAuxBufferQueueHandle bq;

    struct {
        int buffer_id;
        android_native_buffer_t *src_buffer;
        android_native_buffer_t *aux_buffer;
        int crop_width;
        int crop_height;
    } current;

    void makeCurrent(int buffer_id, android_native_buffer_t *src, android_native_buffer_t *aux)
    {
        if (current.src_buffer) {
            decRef(current.src_buffer);
        }
        if (current.aux_buffer) {
            decRef(current.aux_buffer);
        }

        current.buffer_id = buffer_id;
        current.src_buffer = src;
        current.aux_buffer = aux;

        //MTK_LOGI("this: %p, buffer_id: %d, src: %p aux: %p", this, buffer_id, src, aux);

        if (current.src_buffer) {
            incRef(current.src_buffer);
        }
        if (current.aux_buffer) {
            incRef(current.aux_buffer);
        }
    }

public:
    GPUAUXContext(int outputFormat, int numBuffers)
    {
#ifdef MTK_GPUD_SUPPORT
        GPUD_API_ENTRY(AuxHackAUXImageDstFormat, &outputFormat);
#endif
        bq = GuiExtAuxCreateBufferQueue(0, 0, outputFormat, numBuffers);

        current.buffer_id = -1;
        current.src_buffer = NULL;
        current.aux_buffer = NULL;
        current.crop_width = 0;
        current.crop_height = 0;

        MTK_LOGI("create: %p", this);
    }

    ~GPUAUXContext()
    {
        GuiExtAuxDestroyBufferQueue(bq);
        makeCurrent(-1, NULL, NULL);

        MTK_LOGI("destory: %p", this);
    }

    int getIdx()
    {
        return current.buffer_id;
    }

    android_native_buffer_t *getSourceBuffer()
    {
        return current.src_buffer;
    }

    android_native_buffer_t *getAuxBuffer()
    {
        return current.aux_buffer;
    }

    int prepare(android_native_buffer_t *srcBuffer)
    {
        if (current.src_buffer != srcBuffer) {
            if (current.buffer_id != -1) {
                GuiExtAuxReleaseBuffer(bq, current.buffer_id, -1);
            }

            GuiExtAuxKickConversion(bq, srcBuffer);

            int buffer_id = -1, fence_fd = -1;
            GuiExtAuxAcquireBuffer(bq, &buffer_id, &fence_fd);

            if (fence_fd >= 0) {
                close(fence_fd);
                fence_fd = -1;
            }

            if (buffer_id < 0) {
                MTK_LOGE("acquireBuffer with invalid buffer_id! \n");
                return  -1;
            }

            makeCurrent(buffer_id, srcBuffer, GuiExtAuxRequestBuffer(bq, buffer_id));
            GuiExtSetDstColorSpace(getAuxBuffer(), getSourceBuffer());
        }
        return 0;
    }

    int doConversionIfNeed()
    {
        int ConversionStatus = GuiExtAuxDoConversionIfNeed(bq, current.aux_buffer, current.src_buffer, current.crop_width, current.crop_height);
#ifdef MTK_GPUD_SUPPORT
        GPUD_API_ENTRY(AuxShowConversionStatus, ConversionStatus);
#endif
        return ConversionStatus;
    }

    void setBufferCropSize(int attr_crop_width, int attr_crop_height)
    {
        // attrib_list: top, left, right, bottom
        if ((attr_crop_width > 0) && (attr_crop_height > 0)) {
            current.crop_width = attr_crop_width;
            current.crop_height = attr_crop_height;
        } else {
            current.crop_width = 0;
            current.crop_height = 0;
        }
        MTK_LOGI("current.crop_width(%d), current.crop_height(%d)\n", current.crop_width, current.crop_height);
    }
};

int GpuAuxIsSupportFormat(android_native_buffer_t *buf)
{
    return GuiExtAuxIsSupportFormat(buf);
}

int GpuAuxIsUIPQFormat(android_native_buffer_t *buf)
{
    return GuiExtAuxIsUIPQFormat(buf);
}

int GpuAuxIsSupportPQWhiteList()
{
    return GuiExtAuxIsSupportPQWhiteList();
}

int GpuAuxCheckAuxPath(android_native_buffer_t *buf, void *gpu_info)
{
    return GuiExtAuxCheckAuxPath(buf, gpu_info);
}

extern "C"
GPUAUXContextHandle GpuAuxCreateContext(int outputFormat, int numBuffers)
{
    return new GPUAUXContext(outputFormat, numBuffers);
}

extern "C"
int GpuAuxPrepareBuffer(GPUAUXContextHandle ctx, android_native_buffer_t *srcBuffer)
{
    return ctx->prepare(srcBuffer);
}

extern "C"
int GpuAuxGetCurrentIdx(GPUAUXContextHandle ctx)
{
    return ctx->getIdx();
}

extern "C"
android_native_buffer_t *GpuAuxGetCurrentAuxBuffer(GPUAUXContextHandle ctx)
{
    return ctx->getAuxBuffer();
}

extern "C"
android_native_buffer_t *GpuAuxGetCurrentSourceBuffer(GPUAUXContextHandle ctx)
{
    return ctx->getSourceBuffer();
}

extern "C"
int GpuAuxDoConversionIfNeed(GPUAUXContextHandle ctx)
{
    return ctx->doConversionIfNeed();
}

extern "C"
int GpuAuxDestoryContext(GPUAUXContextHandle ctx)
{
    delete ctx;
    return 0;
}

extern "C"
void GpuAuxSetBufferCropSize(GPUAUXContextHandle ctx, int attr_crop_width, int attr_crop_height)
{
    return ctx->setBufferCropSize(attr_crop_width, attr_crop_height);
}

