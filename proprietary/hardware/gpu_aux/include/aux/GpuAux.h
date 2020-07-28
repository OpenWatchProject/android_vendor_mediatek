
#ifndef __GPUAUX_H__
#define __GPUAUX_H__

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <system/window.h>
#include <system/graphics.h>

__BEGIN_DECLS

typedef enum {
    GPU_MALI,
    GPU_IMG,
} GPU_VENDOR;

typedef enum {
	PATH_NO_AUX = 0,
	PATH_AUX_VIDEO,
	PATH_AUX_UIPQ,
	PATH_AUX_INVALID,
} AUX_PATH;

typedef enum {
    AUX_CONVERSION_GET_BUFFER_INFO_FAIL = -7,
    AUX_CONVERSION_INVALIDE_DIMENSION,
    AUX_CONVERSION_SRC_FORMAT_NOT_SUPPORT,
    AUX_CONVERSION_CREATE_JOB_FAIL,
    AUX_CONVERSION_LOCK_SRC_BUFFER_FAIL,
    AUX_CONVERSION_DST_FORMAT_NOT_SUPPORT,
    AUX_CONVERSION_DST_NOT_ION_BUFFER,
    AUX_CONVERSION_SUCCESS = 0,
    AUX_CONVERSION_INVALIDATE_FAIL,
    AUX_CONVERSION_SKIP_PROTECT_BUFFER,
    AUX_CONVERSION_SKIP_NOT_DIRTY,
    AUX_CONVERSION_PATH_NO_AUX,
} AUX_CONVERSION_STATUS;

typedef struct AuxGPUInfo {
    int vendor;
    bool is_nv21_blk_support;
    bool is_vedio_PQ_support;
} AuxGPUInfo;

struct GPUAUXContext;
typedef struct GPUAUXContext *GPUAUXContextHandle;

int GpuAuxIsSupportFormat(android_native_buffer_t *buf);
int GpuAuxIsUIPQFormat(android_native_buffer_t *buf);
int GpuAuxIsSupportPQWhiteList();
int GpuAuxCheckAuxPath(android_native_buffer_t *buf, void *gpu_info);

GPUAUXContextHandle GpuAuxCreateContext(int outputFormat, int numBuffers);

int GpuAuxPrepareBuffer(GPUAUXContextHandle ctx, android_native_buffer_t *srcBuffer);

int GpuAuxGetCurrentIdx(GPUAUXContextHandle ctx);
android_native_buffer_t *GpuAuxGetCurrentAuxBuffer(GPUAUXContextHandle ctx);
android_native_buffer_t *GpuAuxGetCurrentSourceBuffer(GPUAUXContextHandle ctx);

int GpuAuxDoConversionIfNeed(GPUAUXContextHandle ctx);

void GpuAuxSetBufferCropSize(GPUAUXContextHandle ctx, int attr_crop_width, int attr_crop_height);

int GpuAuxDestoryContext(GPUAUXContextHandle ctx);
__END_DECLS

#endif

