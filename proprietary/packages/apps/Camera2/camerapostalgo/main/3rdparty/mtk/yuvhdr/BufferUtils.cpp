#define LOG_TAG "BufferUtils"
#include <utils/std/Format.h>
#include "BufferUtils.h"
#include <utils/std/Log.h>

using ::vendor::mediatek::hardware::mms::V1_2::IMms;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using android::hardware::hidl_handle;
using android::sp;
using NSCam::MSize;
using NSCam::IImageBufferAllocator;

using namespace NSCam::Utils::Format;
using namespace NSCam;
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)
//
#include <cutils/properties.h>
#define LOG_DETAILS "debug.camerapostalgo.feature.yuvhdr.logdetails"
static int8_t gLogDetail = ::property_get_bool(LOG_DETAILS, 0);
#define FUNCTION_IN   do { if (gLogDetail) MY_LOGD("%s +", __FUNCTION__); } while(0)
#define FUNCTION_OUT  do { if (gLogDetail) MY_LOGD("%s -", __FUNCTION__); } while(0)
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define YHDR_TRACE_CALL()                      ATRACE_CALL()
#define YHDR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define YHDR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define YHDR_TRACE_END()                       ATRACE_END()
#else
#define YHDR_TRACE_CALL()
#define YHDR_TRACE_NAME(name)
#define YHDR_TRACE_BEGIN(name)
#define YHDR_TRACE_END()
#endif
AHardwareBuffer* BufferUtils::getAHWBuffer(IImageBuffer *buf)
{
    if(buf != NULL && buf->getImageBufferHeap())
    {
        return (AHardwareBuffer*)buf->getImageBufferHeap()->getHWBuffer();
    }
    return NULL;
}

hidl_handle BufferUtils::getHidlHandle(IImageBuffer *buf)
{
    return hidl_handle(
               AHardwareBuffer_getNativeHandle(getAHWBuffer(buf)));
}


sp<IMms> BufferUtils::mIMmsService = IMms::getService();
int BufferUtils::mdpCopyBit(const HwCopybitParam* copybitParam)
{
    FUNCTION_IN;
    if (mIMmsService == nullptr)
    {
        MY_LOGE("cannot find IMms_service from getService, return false");
        FUNCTION_OUT;
        return false;
    }
    int res = mIMmsService->copybit(*copybitParam);
    FUNCTION_OUT;
    return res;
}

int BufferUtils::mdpConvertFormatAndResize(IImageBuffer* inBuffer, IImageBuffer* outBuffer)
{
    FUNCTION_IN;

    HwCopybitParam param;
    memset(&param, 0, sizeof(HwCopybitParam));

    // input params of copybit param
    param.inputWidth = inBuffer->getImgSize().w;
    param.inputHeight = inBuffer->getImgSize().h;
    param.inputFormat = inBuffer->getImgFormat();
    param.inputHandle = getHidlHandle(inBuffer);
    //output params of copybit param
    param.outputWidth = outBuffer->getImgSize().w;
    param.outputHeight = outBuffer->getImgSize().h;
    param.outputFormat = outBuffer->getImgFormat();
    param.outputHandle = getHidlHandle(outBuffer);
    // convert format
    int res = mdpCopyBit(&param);

    FUNCTION_OUT;
    return res;
}
