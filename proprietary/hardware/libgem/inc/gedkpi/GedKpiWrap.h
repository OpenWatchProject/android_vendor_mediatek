#ifndef __GPU_KPI_WRAP_H
#define __GPU_KPI_WRAP_H

#include <stdint.h>

extern "C"
{
int32_t ged_kpi_create_wrap(uint64_t surfaceHandle, uint64_t BBQ_ID);
void ged_kpi_destroy_wrap(uint64_t surfaceHandle);
int32_t ged_kpi_dequeue_buffer_tag_wrap(uint64_t surfaceHandle, int32_t fence, intptr_t buffer_addr);
int32_t ged_kpi_queue_buffer_tag_wrap(uint64_t surfaceHandle, int32_t fence, int32_t QedBuffer_length, intptr_t buffer_addr);
int32_t ged_kpi_acquire_buffer_tag_wrap(uint64_t surfaceHandle, intptr_t buffer_addr);
void ged_kpi_buffer_connect(uint64_t surfaceHandle, int32_t BBQ_api_type, int32_t pid);
void ged_kpi_buffer_disconnect(uint64_t surfaceHandle);
}

#endif
