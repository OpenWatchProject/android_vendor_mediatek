#include <ged/ged_kpi_sys.h>
#include <cutils/log.h>
#include <gedkpi/GedKpiWrap.h>

int32_t ged_kpi_create_wrap(uint64_t surfaceHandle, uint64_t BBQ_ID)

{
 #ifndef MTK_DO_NOT_USE_GPU_EXT
    return ged_kpi_create_sys(surfaceHandle, BBQ_ID);
 #else
    ALOGD("Use dummy ged handle");
    (void)surfaceHandle;
    (void)BBQ_ID;
    return GED_OK;
 #endif
}

void ged_kpi_destroy_wrap(uint64_t surfaceHandle)

{
 #ifndef MTK_DO_NOT_USE_GPU_EXT
    ged_kpi_destroy_sys(surfaceHandle);
 #else
    (void)surfaceHandle;
 #endif
}

int32_t ged_kpi_dequeue_buffer_tag_wrap(uint64_t surfaceHandle, int32_t fence, intptr_t buffer_addr)
{
 #ifndef MTK_DO_NOT_USE_GPU_EXT
    return ged_kpi_dequeue_buffer_tag_sys(surfaceHandle, fence, buffer_addr);
 #else
    (void)surfaceHandle;
    (void)fence;
    (void)buffer_addr;
    return GED_OK;
 #endif
}

int32_t ged_kpi_queue_buffer_tag_wrap(uint64_t surfaceHandle, int32_t fence, int32_t QedBuffer_length, intptr_t buffer_addr)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    return ged_kpi_queue_buffer_tag_sys(surfaceHandle, fence, QedBuffer_length, buffer_addr);
#else
    (void)surfaceHandle;
    (void)fence;
    (void)QedBuffer_length;
    (void)buffer_addr;
    return GED_OK;
#endif
}

int32_t ged_kpi_acquire_buffer_tag_wrap(uint64_t surfaceHandle, intptr_t buffer_addr)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    return ged_kpi_acquire_buffer_tag_sys(surfaceHandle, buffer_addr);
#else
    (void)surfaceHandle;
    (void)buffer_addr;
    return GED_OK;
#endif
}

void ged_kpi_buffer_connect(uint64_t surfaceHandle, int32_t BBQ_api_type, int32_t pid)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    ged_kpi_buffer_connect_sys(surfaceHandle, BBQ_api_type, pid);
#else
    (void)surfaceHandle;
    (void)BBQ_api_type;
    (void)pid;
#endif
}

void ged_kpi_buffer_disconnect(uint64_t surfaceHandle)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    ged_kpi_buffer_disconnect_sys(surfaceHandle);
#else
    (void)surfaceHandle;
#endif
}

