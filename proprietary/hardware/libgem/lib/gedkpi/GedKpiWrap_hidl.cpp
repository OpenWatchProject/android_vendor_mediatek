#include <cutils/log.h>
#include <gedkpi/GedKpiWrap.h>

#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>

using vendor::mediatek::hardware::gpu::V1_0::IGraphicExt;
using vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_ERROR_TAG;
using namespace android;

sp<IGraphicExt> g_graphic_service = NULL;

int32_t ged_kpi_create_wrap(uint64_t BBQ_ID)
{
 #ifndef MTK_DO_NOT_USE_GPU_EXT
    int32_t err = (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;
    android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);

    if (g_graphic_service == NULL) {
        g_graphic_service = IGraphicExt::getService();
        if (g_graphic_service == nullptr) {
            ALOGD("failed to get IGraphicExt service\n");
            return err;
        }
    }

    retval = g_graphic_service->ged_kpi_create(BBQ_ID);
    if (retval.isOk()) {
        err = (int32_t)(vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_ERROR_TAG)retval;
    } else {
        ALOGD("ged_kpi_create failed!\n");
    }

    return err;
 #else
    (void)BBQ_ID;
    ALOGD("Use dummy ged handle");
    return (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_OK;
 #endif
}

void ged_kpi_destroy_wrap(uint64_t BBQ_ID)
{
 #ifndef MTK_DO_NOT_USE_GPU_EXT
    if (g_graphic_service) {
        android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);
        retval = g_graphic_service->ged_kpi_destroy(BBQ_ID);
    }
    g_graphic_service = NULL;
 #else
    (void)BBQ_ID;
 #endif
}

int32_t ged_kpi_dequeue_buffer_tag_wrap(uint64_t BBQ_ID, int32_t fence, intptr_t buffer_addr)
{
 #ifndef MTK_DO_NOT_USE_GPU_EXT
    int32_t err = (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;

    if (g_graphic_service) {
        android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);

        retval = g_graphic_service->ged_kpi_dequeue_buffer_tag(BBQ_ID, fence, (int32_t)buffer_addr);
        if (retval.isOk()) {
            err = (int32_t)(vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_ERROR_TAG)retval;
        }
    }
    return err;
 #else
    (void)BBQ_ID;
    (void)fence;
    (void)buffer_addr;
    return (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_OK;
 #endif
}

int32_t ged_kpi_queue_buffer_tag_wrap(uint64_t BBQ_ID, int32_t fence, int32_t QedBuffer_length, intptr_t buffer_addr)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    int32_t err = (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;

    if (g_graphic_service) {
        android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);

        retval = g_graphic_service->ged_kpi_queue_buffer_tag(BBQ_ID, fence, QedBuffer_length, (int32_t)buffer_addr);
        if (retval.isOk()) {
            err = (int32_t)(vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_ERROR_TAG)retval;
        }
    }
    return err;
#else
    (void)BBQ_ID;
    (void)fence;
    (void)QedBuffer_length;
    (void)buffer_addr;
    return (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_OK;
#endif
}

int32_t ged_kpi_acquire_buffer_tag_wrap(uint64_t BBQ_ID, intptr_t buffer_addr)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    int32_t err = (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL;

    if (g_graphic_service) {
        android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);

        retval = g_graphic_service->ged_kpi_acquire_buffer_tag(BBQ_ID, (int32_t)buffer_addr);
        if (retval.isOk()) {
            err = (int32_t)(vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_ERROR_TAG)retval;
        }

    }
    return err;
#else
    (void)BBQ_ID;
    (void)buffer_addr;
    return (int32_t)GED_HIDL_ERROR_TAG::GED_HIDL_OK;
#endif
}

void ged_kpi_buffer_connect(uint64_t BBQ_ID, int32_t BBQ_api_type, int32_t pid)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
        if (g_graphic_service) {
            android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);
    
            retval = g_graphic_service->ged_kpi_buffer_connect(BBQ_ID, BBQ_api_type, pid);
        }
#else
        (void)BBQ_ID;
        (void)buffer_addr;
#endif
}

void ged_kpi_buffer_disconnect(uint64_t BBQ_ID)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
        if (g_graphic_service) {
            android::hardware::Return<GED_HIDL_ERROR_TAG> retval(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);
    
            retval = g_graphic_service->ged_kpi_buffer_disconnect(BBQ_ID);
        }
#else
        (void)BBQ_ID;
        (void)buffer_addr;
#endif

}

