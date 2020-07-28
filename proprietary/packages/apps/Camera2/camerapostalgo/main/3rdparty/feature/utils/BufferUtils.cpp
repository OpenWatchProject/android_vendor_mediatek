#include <utils/std/Format.h>
#include <sys/stat.h>
#include "BufferUtils.h"
#include "core/GraphicBufferPool.h"
#include "LogUtils.h"
#define LOG_TAG "FeatureUtils/BufferUtils"

using ::vendor::mediatek::hardware::mms::V1_2::IMms;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using android::hardware::hidl_handle;
using android::sp;
using NSCam::MSize;
using NSCam::IImageBufferAllocator;

using namespace NSCam::Utils::Format;
using namespace NSCam;
using namespace com::mediatek::campostalgo::NSBufferPool;

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

IImageBuffer* BufferUtils::acquireWorkingBuffer(MSize imgSize, MINT32 imgfmt)
{
    FUNCTION_IN;
    MINT planecount = queryPlaneCount(imgfmt);
    MINT usage = eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MINT32 bufStridesInBytes[3];
    for(MINT i=0; i<planecount; i++) {
        bufStridesInBytes[i] = (queryPlaneWidthInPixels(imgfmt,i,imgSize.w)*queryPlaneBitsPerPixel(imgfmt,i))>>3;
    }

    IImageBufferAllocator::ImgParam imgParam(imgfmt,imgSize,bufStridesInBytes,bufBoundaryInBytes,planecount);
    IImageBufferAllocator::ExtraParam extParam(usage);
    IImageBuffer * mpImageBuffer = IImageBufferAllocator::getInstance()->alloc_gb(LOG_TAG,imgParam,extParam);
    if (mpImageBuffer == nullptr)
    {
        MY_LOGE("acquireWorkingBuffer: create image buffer failed");
    } else if (!(mpImageBuffer->lockBuf(LOG_TAG, usage)))
    {
        MY_LOGE("acquireWorkingBuffer: lock image buffer failed");
    }
    FUNCTION_OUT;
    return mpImageBuffer;
}


void BufferUtils::deallocBuffer(IImageBuffer* pBuf) {
    if (pBuf == NULL) {
        CAM_LOGD("pBuf is NULL, do nothing");
        return;
    }
    // unlock image buffer
    pBuf->unlockBuf(LOG_TAG);
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    allocator->free(pBuf);
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

int BufferUtils::mdpResizeBuffer(IImageBuffer* inBuffer,
        MINT32 outputW, MINT32 outputH, IImageBuffer* outBuffer)
{
    FUNCTION_IN;

    HwCopybitParam param;
    memset(&param, 0, sizeof(HwCopybitParam));

    // input params of copybit param
    param.inputWidth = inBuffer->getImgSize().w;
    param.inputHeight = inBuffer->getImgSize().h;
    param.inputFormat = BufferUtils::toPixelFormat((EImageFormat)inBuffer->getImgFormat());
    param.inputHandle = getHidlHandle(inBuffer);
    //output params of copybit param
    param.outputWidth = outputW;
    param.outputHeight = outputH;
    param.outputFormat = BufferUtils::toPixelFormat((EImageFormat)outBuffer->getImgFormat());;
    param.outputHandle = getHidlHandle(outBuffer);
    // convert format
    int res = mdpCopyBit(&param);

    FUNCTION_OUT;
    return res;
}

int BufferUtils::mdpResizeAndConvert(IImageBuffer* inBuffer, IImageBuffer* outBuffer)
{
    FUNCTION_IN;

    HwCopybitParam param;
    memset(&param, 0, sizeof(HwCopybitParam));

    // input params of copybit param
    param.inputWidth = inBuffer->getImgSize().w;
    param.inputHeight = inBuffer->getImgSize().h;
    param.inputFormat = BufferUtils::toPixelFormat((EImageFormat)inBuffer->getImgFormat());
    param.inputHandle = getHidlHandle(inBuffer);
    //output params of copybit param
    param.outputWidth = outBuffer->getImgSize().w;
    param.outputHeight = outBuffer->getImgSize().h;
    param.outputFormat = BufferUtils::toPixelFormat((EImageFormat)outBuffer->getImgFormat());
    param.outputHandle = getHidlHandle(outBuffer);
    // convert format
    int res = mdpCopyBit(&param);

    FUNCTION_OUT;
    return res;
}

int BufferUtils::mdpResizeAndConvert(IImageBuffer* inBuffer, IImageBuffer* outBuffer, int
    rotationDegree)
{
    FUNCTION_IN;

    HwCopybitParam param;
    memset(&param, 0, sizeof(HwCopybitParam));

    // input params of copybit param
    param.inputWidth = inBuffer->getImgSize().w;
    param.inputHeight = inBuffer->getImgSize().h;
    param.inputFormat = BufferUtils::toPixelFormat((EImageFormat)inBuffer->getImgFormat());
    param.inputHandle = getHidlHandle(inBuffer);
    //output params of copybit param
    param.outputWidth = outBuffer->getImgSize().w;
    param.outputHeight = outBuffer->getImgSize().h;
    param.outputFormat = BufferUtils::toPixelFormat((EImageFormat)outBuffer->getImgFormat());;
    param.outputHandle = getHidlHandle(outBuffer);
    param.rotation = ((rotationDegree + 45) / 90) * 90;
    // convert format
    int res = mdpCopyBit(&param);

    FUNCTION_OUT;
    return res;
}

int BufferUtils::mdpResizeAndConvert(IImageBuffer* inBuffer, IImageBuffer* outBuffer, int
    rotationDegree, bool mirror)
{
    FUNCTION_IN;

    HwCopybitParam param;
    memset(&param, 0, sizeof(HwCopybitParam));

    // input params of copybit param
    param.inputWidth = inBuffer->getImgSize().w;
    param.inputHeight = inBuffer->getImgSize().h;
    param.inputFormat = BufferUtils::toPixelFormat((EImageFormat)inBuffer->getImgFormat());
    param.inputHandle = getHidlHandle(inBuffer);
    //output params of copybit param
    param.outputWidth = outBuffer->getImgSize().w;
    param.outputHeight = outBuffer->getImgSize().h;
    param.outputFormat = BufferUtils::toPixelFormat((EImageFormat)outBuffer->getImgFormat());;
    param.outputHandle = getHidlHandle(outBuffer);
    param.rotation = ((rotationDegree + 45) / 90) * 90;
    param.flipMirror = mirror;
    // convert format
    int res = mdpCopyBit(&param);

    FUNCTION_OUT;
    return res;
}

IImageBuffer* BufferUtils::mdpConvertWithoutOutputBuffer(IImageBuffer* inBuffer, MINT32 outFormat)
{
    FUNCTION_IN;

    IImageBuffer* outBuffer = acquireWorkingBuffer(inBuffer->getImgSize(), outFormat);

    mdpResizeAndConvert(inBuffer, outBuffer);

    FUNCTION_OUT;

    return outBuffer;

}

void BufferUtils::dumpBuffer(IImageBuffer* buffer, char* fileNamePrefix) {

    static int dumpCount = 0;

    char path[128] = "/data/camera_post_algo_buffer_dump";
    int result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    if ((result == -1) && (errno != EEXIST)) {
        MY_LOGE("mkdir fail, error %d, return", errno);
        return;
    }

    char fileName[128];
    MSize size = buffer->getImgSize();
    sprintf(fileName, "%s/%d_%s_f%d_w%d_h%d_s%d.raw", path, ++dumpCount, fileNamePrefix,
        buffer->getImgFormat(), size.w, size.h, size.w);
    FILE *fp = fopen(fileName, "wb");
    if (NULL == fp) {
        MY_LOGE("fail to open file %s", fileName);
    } else {
        size_t planecount = buffer->getPlaneCount();
        for (size_t i = 0; i < planecount; i++) {
            unsigned char* planeBuffer = (unsigned char *)buffer->getBufVA(i);
            int planeBufferSize = buffer->getBufSizeInBytes(i);
            int total_write = 0;
            while(total_write < planeBufferSize) {
                int write_size = fwrite(planeBuffer+total_write, 1, planeBufferSize-total_write, fp);
                if (write_size <= 0) {
                    MY_LOGE("write_size = %d, fileName = %s", write_size, fileName);
                }
                total_write += write_size;
            }
        }
        fclose(fp);
    }
}

void BufferUtils::dumpBufferX(unsigned char* buffer, char* fileNamePrefix, int size) {
    static int dumpCount = 0;

    char path[128] = "/data/camera_post_algo_buffer_dump/xxx";
    int result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    if ((result == -1) && (errno != EEXIST)) {
        MY_LOGE("mkdir fail, error %d, return", errno);
        return;
    }

    char fileName[128];
    sprintf(fileName, "%s/%d_%s_f%d_w%d_h%d_s%d.yv12", path, ++dumpCount, fileNamePrefix,
        eImgFmt_RGBA8888, 280, 280, 280);
    FILE *fp = fopen(fileName, "wb");
    if (NULL == fp) {
        MY_LOGE("fail to open file %s", fileName);
    } else {
        int total_write = 0;
        while(total_write < size) {
            int write_size = fwrite(buffer+total_write, 1, size-total_write, fp);
            if (write_size <= 0) {
                MY_LOGE("write_size = %d, fileName = %s", write_size, fileName);
            }
            total_write += write_size;
        }
        fclose(fp);
    }
}

int32_t BufferUtils::toPixelFormat(NSCam::EImageFormat fmt)
{
  switch(fmt)
  {
      case eImgFmt_RGBA8888:        return HAL_PIXEL_FORMAT_RGBA_8888;
      case eImgFmt_YV12:            return HAL_PIXEL_FORMAT_YV12;
      case eImgFmt_RAW16:           return HAL_PIXEL_FORMAT_RAW16;
      case eImgFmt_RAW_OPAQUE:      return HAL_PIXEL_FORMAT_RAW_OPAQUE;
      case eImgFmt_BLOB:            return HAL_PIXEL_FORMAT_BLOB;
      case eImgFmt_RGBX8888:        return HAL_PIXEL_FORMAT_RGBX_8888;
      case eImgFmt_RGB888:          return HAL_PIXEL_FORMAT_RGB_888;
      case eImgFmt_RGB565:          return HAL_PIXEL_FORMAT_RGB_565;
      case eImgFmt_BGRA8888:        return HAL_PIXEL_FORMAT_BGRA_8888;
      case eImgFmt_YUY2:            return HAL_PIXEL_FORMAT_YCbCr_422_I;
      case eImgFmt_NV16:            return HAL_PIXEL_FORMAT_YCbCr_422_SP;
      case eImgFmt_NV21:            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
      case eImgFmt_NV12:            return HAL_PIXEL_FORMAT_YCrCb_420_SP;
      case eImgFmt_Y8:              return HAL_PIXEL_FORMAT_Y8;
      case eImgFmt_Y16:             return HAL_PIXEL_FORMAT_Y16;
      case eImgFmt_I420:            return HAL_PIXEL_FORMAT_I420;
      default:
        return (android_pixel_format)-1;
  };
}
