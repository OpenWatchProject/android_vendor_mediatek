#define LOG_TAG "GPU-Test"

#include <stdio.h>
#include <cstdlib>
#include <time.h>
#include <iostream>
#include <log/log.h>
#include <cutils/properties.h>
//#include <WindowSurface.h>
#include <ui/GraphicBuffer.h>
//#include <EGLUtils.h>
#include <math.h>

//#include <ged/ged.h>
//#include "gralloc_extra.h"

#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>
using ::android::hardware::hidl_handle;

using vendor::mediatek::hardware::gpu::V1_0::IGraphicExt;
using vendor::mediatek::hardware::gpu::V1_0::IGraphicExtCallback;
using vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_HANDLE;
using vendor::mediatek::hardware::gpu::V1_0::GED_KPI_HANDLE;
using vendor::mediatek::hardware::gpu::V1_0::GED_HIDL_ERROR_TAG;
using vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM;
using vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY;
using vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_SECURE_BUFFER_TYPE;
using vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT;
using vendor::mediatek::hardware::gpu::V1_0::ge_sf_info_t;
using vendor::mediatek::hardware::gpu::V1_0::ge_ion_debug_t;

using namespace std;
using namespace android;

static int is_sendVsyncOffsetFunc_pass = 0;

#define PRINTD(fmt, args...) \
    ALOGD("%s: " fmt, __func__, ##args)

//	printf("%s: " fmt, __func__, ##args)

class GraphicExtCallback : public IGraphicExtCallback {
public:
    android::hardware::Return<void> sendVsyncOffsetFunc(int32_t protocol, int32_t i32Data);
};

GraphicExtCallback *pGraphicExtCallback  = NULL;

android::hardware::Return<void>  GraphicExtCallback::sendVsyncOffsetFunc(int32_t protocol, int32_t i32Data)
{
    PRINTD("protocol = %d, i32Data = %d\n", protocol, i32Data);
    is_sendVsyncOffsetFunc_pass = 1;
    return android::hardware::Void();
}

int ged_test(sp<IGraphicExt> service)
{
    int isFail = 0;
    GED_HIDL_ERROR_TAG retval;
    GED_HIDL_HANDLE hGed;
    android::hardware::Return<void> ret;

    // ged_create() test
    ret = service->ged_create(
        [&] (GED_HIDL_ERROR_TAG retval, GED_HIDL_HANDLE ged_handle) {
        if (retval == GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            hGed = ged_handle;

            PRINTD("ged_create(), i32BridgeFD %d, tid %d\n",
                hGed.i32BridgeFD, hGed.tid);
        } else {
            isFail = -1;
            PRINTD("ged_create() return fail, retval = %d\n", retval);
        }
    });
    if (!ret.isOk()) {
        isFail = -1;
        PRINTD("ged_create() call fail\n");
    }

    if (isFail) {
        // if ged_create() fail, we can't do the following test.
        return isFail;
    }

    // ged_vsync_calibration() test
    retval = service->ged_vsync_calibration(hGed, 0, 80000000);
    if (retval != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
        isFail = -1;
        PRINTD("ged_vsync_calibration() return fail, retval = %d\n", retval);
    }

    // ged_is_gpu_need_vsync_calibration() test
    ret = service->ged_is_gpu_need_vsync_calibration(
        [&] (GED_HIDL_ERROR_TAG retval, uint32_t isNeed) {
        if (retval == GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            PRINTD("ged_is_gpu_need_vsync_calibration() return %d\n", isNeed);
        } else {
            isFail = -1;
            PRINTD("ged_is_gpu_need_vsync_calibration() return fail, retval = %d\n", retval);
        }
    });
    if (!ret.isOk()) {
        isFail = -1;
        PRINTD("ged_is_gpu_need_vsync_calibration() call fail\n");
    }

    // ged_destroy() test
    retval = service->ged_destroy(hGed);
    if (retval != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
        isFail = -1;
        PRINTD("ged_destroy() return fail, retval = %d\n", retval);
    }

    return isFail;
}

int ged_kpi_test(sp<IGraphicExt> service)
{
    int isFail = 0;
    uint64_t bbqID = 555;
    //GED_HIDL_ERROR_TAG retval;
    android::hardware::Return<GED_HIDL_ERROR_TAG> ret2(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);

    // ged_kpi_create() test
    ret2 = service->ged_kpi_create(bbqID);
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_create() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        } else {
            PRINTD("ged_kpi_create pass\n");
        }
    } else {
        PRINTD("ged_kpi_dequeue_buffer_tag() hidl fail\n");
    }

    if (isFail) {
        // if ged_kpi_create() fail, we can't do the following test.
        return isFail;
    }

    // ged_kpi_acquire_buffer_tag() test
    ret2 = service->ged_kpi_buffer_connect(bbqID, 0, getpid());
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_buffer_connect() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        }
    } else {
        PRINTD("ged_kpi_buffer_connect() hidl fail\n");
    }

    // ged_kpi_dequeue_buffer_tag() test
    ret2 = service->ged_kpi_dequeue_buffer_tag(bbqID, 0, 0);
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_dequeue_buffer_tag() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        }
    } else {
        PRINTD("ged_kpi_dequeue_buffer_tag() hidl fail\n");
    }

    // ged_kpi_queue_buffer_tag() test
    ret2 = service->ged_kpi_queue_buffer_tag(bbqID, 0, 0, 0);
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_queue_buffer_tag() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        }
    } else {
        PRINTD("ged_kpi_queue_buffer_tag() hidl fail\n");
    }

    // ged_kpi_acquire_buffer_tag() test
    ret2 = service->ged_kpi_acquire_buffer_tag(bbqID, 0);
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_acquire_buffer_tag() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        }
    } else {
        PRINTD("ged_kpi_acquire_buffer_tag() hidl fail\n");
    }

    ret2 = service->ged_kpi_buffer_disconnect(bbqID);
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_buffer_disconnect() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        }
    } else {
        PRINTD("ged_kpi_buffer_disconnect() hidl fail\n");
    }

    // ged_kpi_destroy() test
    ret2 = service->ged_kpi_destroy(bbqID);
    if (ret2.isOk()) {
        if (ret2 != GED_HIDL_ERROR_TAG::GED_HIDL_OK) {
            isFail = -1;
            PRINTD("ged_kpi_destroy() return fail, retval = %d\n", (int)(GED_HIDL_ERROR_TAG)ret2);
        }
    } else {
        PRINTD("ged_kpi_destroy() hidl fail\n");
    }

    return isFail;
}

int IGraphicExtCallback_test(sp<IGraphicExt> service)
{
    int isFail = 0;
    android::hardware::Return<int32_t> res(0);

    pGraphicExtCallback = new GraphicExtCallback();

    res = service->setupSendVsyncOffsetFuncCallback(pGraphicExtCallback);
    if (!res.isOk()) {
        isFail = -1;
        PRINTD("setupSendVsyncOffsetFuncCallback() return fail\n");
    }

    return isFail;
}

int gralloc_extra_test(sp<IGraphicExt> service)
{
    int isFail = 0;
    android::hardware::Return<void> ret;

    int32_t retval;
    const int yuvTexWidth = 608;
    const int yuvTexHeight = 480;
    const int yuvTexUsage = GraphicBuffer::USAGE_HW_TEXTURE |
        GraphicBuffer::USAGE_SW_WRITE_RARELY;
    const int yuvTexFormat = HAL_PIXEL_FORMAT_YV12;
    static sp<GraphicBuffer> yuvTexBuffer;

    yuvTexBuffer = new GraphicBuffer(yuvTexWidth, yuvTexHeight, yuvTexFormat,
            yuvTexUsage);
    if (yuvTexBuffer == NULL) {
        isFail = -1;
        PRINTD("new GraphicBuffer fail");
        return isFail;
    }

    auto hidlHandle = hidl_handle(yuvTexBuffer->getNativeBuffer()->handle);

    vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY attr_query;
    vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM attr_perform;

    attr_query = vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_ION_FD;
    ret = service->gralloc_extra_query(hidlHandle, attr_query,
        [&] (int32_t retval, hidl_handle dataHandle) {
        if (retval == (int32_t)vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK) {
            int32_t *out = (int32_t*)&(dataHandle->data[0]);
            PRINTD("gralloc_extra_query(), attr:%d, out = %d\n", attr_query, *out);
        } else {
            isFail = -1;
            PRINTD("gralloc_extra_query(), attr %d, return fail, retval = %d\n", attr_query, retval);
        }
    });
    if (!ret.isOk()) {
        PRINTD("gralloc_extra_query attr %d failed!", attr_query);
        isFail = -1;
    }

    attr_query = vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SF_INFO;
    ret = service->gralloc_extra_query(hidlHandle, attr_query,
        [&] (int32_t retval, hidl_handle dataHandle) {
        if (retval == (int32_t)vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK) {
            int32_t intCount = ceil(static_cast<float>(sizeof(ge_sf_info_t)) / sizeof(int));

            if (dataHandle->numInts != intCount) {
                PRINTD("gralloc_extra_query(), attr:%d, size error intCount %d, dataHandle->numInts %d\n",
                    attr_query, intCount, dataHandle->numInts);
            } else {
                ge_sf_info_t *sf_info = (ge_sf_info_t *)(&(dataHandle->data[0]));
                PRINTD("gralloc_extra_query(), attr:%d, out = %d\n", attr_query, sf_info->magic);
            }
        } else {
            isFail = -1;
            PRINTD("gralloc_extra_query(), attr %d, return fail, retval = %d\n", attr_query, retval);
        }
    });
    if (!ret.isOk()) {
        PRINTD("gralloc_extra_query attr %d failed!", attr_query);
        isFail = -1;
    }

    attr_perform = vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM::GRALLOC_EXTRA_SET_TIMESTAMP;

    int32_t intCount = ceil(static_cast<float>(sizeof(uint64_t)) / sizeof(int));
    //int32_t intCount2 = ceil((sizeof(uint64_t)) / sizeof(int));
    native_handle_t* nativeHandle = native_handle_create(0, intCount);
    if (nativeHandle == NULL) {
        PRINTD("gralloc_extra_perform() nativeHandle NULL \n");
    }

    uint64_t *pValue = (uint64_t*)(&(nativeHandle->data[0]));
    *pValue = 87654;

    android::hardware::Return<int32_t> ret2(0);

    auto qqq = hidl_handle(nativeHandle);

    ret2 = service->gralloc_extra_perform(hidlHandle, attr_perform, qqq);
    if(ret2.isOk()) {
        retval = ret2;

        if (retval == (int32_t)vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK) {
            PRINTD("gralloc_extra_perform(), attr:%d pass\n", attr_perform);
        } else {
            isFail = -1;
            PRINTD("gralloc_extra_perform(), attr %d, return fail, retval = %d\n", attr_perform, retval);
        }
    }

    ret2 = service->gralloc_extra_perform_uint64(hidlHandle, attr_perform, 87654);
    if(ret2.isOk()) {
        if (ret2 == (int32_t)vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK) {
            PRINTD("gralloc_extra_perform(), attr:%d pass\n", attr_perform);
        } else {
            isFail = -1;
            PRINTD("gralloc_extra_perform(), attr %d, return fail, retval = %d\n", attr_perform, (int32_t)ret2);
        }
    } else {
        isFail = -1;
        PRINTD("gralloc_extra_perform(), attr %d, hidl fail\n", attr_perform);
    }

    attr_perform = vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_PERFORM::GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG;
    ge_ion_debug_t geIonDebug;

    ret2 = service->gralloc_extra_perform_ion_debug(hidlHandle, attr_perform, geIonDebug);
    if(ret2.isOk()) {
        if (ret2 == (int32_t)vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK) {
            PRINTD("gralloc_extra_perform(), attr:%d pass\n", attr_perform);
        } else {
            isFail = -1;
            PRINTD("gralloc_extra_perform(), attr %d, return fail, retval = %d\n", attr_perform, (int32_t)ret2);
        }
    } else {
        isFail = -1;
        PRINTD("gralloc_extra_perform(), attr %d, hidl fail\n", attr_perform);
    }

    //if (nativeHandle) {
    //    native_handle_close(nativeHandle);
    //    native_handle_delete(nativeHandle);
    //}


    return isFail;
}

uint64_t GetTimeStamp() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

int performanceTest()
{
    int isFail = 0;
    int testTime = 1000;
    android::hardware::Return<void> ret;
    //int32_t retval;
    const int yuvTexWidth = 608;
    const int yuvTexHeight = 480;
    const int yuvTexUsage = GraphicBuffer::USAGE_HW_TEXTURE |
        GraphicBuffer::USAGE_SW_WRITE_RARELY;
    const int yuvTexFormat = HAL_PIXEL_FORMAT_YV12;
    static sp<GraphicBuffer> yuvTexBuffer;

    yuvTexBuffer = new GraphicBuffer(yuvTexWidth, yuvTexHeight, yuvTexFormat,
            yuvTexUsage);
    if (yuvTexBuffer == NULL) {
        isFail = -1;
        PRINTD("new GraphicBuffer fail");
        return isFail;
    }

    uint64_t s,e;

    // 1.
    s = GetTimeStamp();
    sp<IGraphicExt> service = IGraphicExt::getService();
    if (service == nullptr) {
        PRINTD("failed to get HW service\n");
        return -1;
    }
    e = GetTimeStamp();
    PRINTD("HIDL get 00 service time:%llu\n", (unsigned long long)(e - s));

    auto hidlHandle = hidl_handle(yuvTexBuffer->getNativeBuffer()->handle);

    s = GetTimeStamp();
    for (int i = 0; i < testTime; i++) {
        // Query
        ret = service->gralloc_extra_query(hidlHandle, vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_ION_FD,
            [&] (int32_t retval, hidl_handle dataHandle) {
        });
        if (!ret.isOk()) {
            PRINTD("don't care 0");
        }
    }
    e = GetTimeStamp();
    PRINTD("HIDL one int time * 1000:%llu\n", (unsigned long long)(e - s));

    s = GetTimeStamp();
    for (int i = 0; i < testTime; i++) {
        ret = service->gralloc_extra_query(hidlHandle, vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SF_INFO,
            [&] (int32_t retval, hidl_handle dataHandle) {
        });
        if (!ret.isOk()) {
            PRINTD("don't care <1>[%d]\n", i);
        }
    }
    e = GetTimeStamp();
    PRINTD("HIDL sf_info time * 1000:%llu\n", (unsigned long long)(e - s));

    // 2.
    /*
    int32_t out;
    s = GetTimeStamp();
    for (int i = 0; i < testTime; i++) {
        gralloc_extra_query(yuvTexBuffer->getNativeBuffer()->handle, GRALLOC_EXTRA_GET_ION_FD, &out);
    }
    e = GetTimeStamp();
    PRINTD("lib time * 1000:%llu\n", (e - s));
    */

    // 3.
    GED_HIDL_HANDLE hGed;

    // ged_create() test
    s = GetTimeStamp();
    ret = service->ged_create(
        [&] (GED_HIDL_ERROR_TAG retval, GED_HIDL_HANDLE ged_handle) {
    });
    if (!ret.isOk()) {
        PRINTD("don't care 2 ");
    }
    e = GetTimeStamp();
    PRINTD("HIDL ged_create:%llu\n", (unsigned long long)(e - s));

    android::hardware::Return<GED_HIDL_ERROR_TAG> ret2(GED_HIDL_ERROR_TAG::GED_HIDL_ERROR_FAIL);
    // ged_vsync_calibration() test
    s = GetTimeStamp();
    for (int i = 0; i < testTime; i++) {
        ret2 = service->ged_vsync_calibration(hGed, 0, 80000000);
        if (!ret2.isOk()) {
            PRINTD("don't care 3");
        }
    }
    e = GetTimeStamp();
    PRINTD("HIDL ged_vsync_calibration() * 1000:%llu\n", (unsigned long long)(e - s));

    /*
    GED_HANDLE g_hGed = NULL;
    s = GetTimeStamp();
    g_hGed = ged_create();
    e = GetTimeStamp();
    PRINTD("ged_create:%llu\n", (e - s));

    s = GetTimeStamp();
    for (int i = 0; i < testTime; i++) {
        ged_vsync_calibration(g_hGed, 0, 80000000);
    }
    e = GetTimeStamp();
    PRINTD("ged_vsync_calibration() * 1000:%llu\n", (e - s));
    */

    return isFail;
}

int main(int, char *[])
{
    int isFail = 0;

    PRINTD("=== gpu_test start ===\n");

    sp<IGraphicExt> service = IGraphicExt::getService();
    if (service == nullptr) {
        PRINTD("failed to get HW service\n");
        return 0;
    }

    if (ged_test(service) == -1) {
        isFail = -1; // mark as test fail
    }

    if (ged_kpi_test(service) == -1) {
        isFail = -1; // mark as test fail
    }

    if (gralloc_extra_test(service) == -1) {
        isFail = -1; // mark as test fail
    }

    if (IGraphicExtCallback_test(service) == -1) {
        isFail = -1; // mark as test fail
    }

    if (performanceTest() == -1) {
        PRINTD("performanceTest() have error\n");
    }

    PRINTD("waiting IGraphicExtCallback, press 'q' to exit\n");
    while (!is_sendVsyncOffsetFunc_pass) {
        char c = getchar();

        if (c =='q') {
            break;
        }
    }

    if (pGraphicExtCallback) {
        delete pGraphicExtCallback;
        pGraphicExtCallback = NULL;
    }

    return 0;
}
