
#include <sys/stat.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Profile.h>
#include <mtkcam/utils/exif/StdExif.h>
#include "../include/JpegEncodeThread.h"

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_JPEG_ENCODE_THREAD
#define PIPE_CLASS_TAG "JpegEncodeThread"
#include "../include/PipeLog.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

using namespace android;
using namespace NSCam::NSIoPipe;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

#define ALLOC_BUFFER_NUM 3

Mutex JpegEncodeThread::sSingletonLock;
wp<JpegEncodeThread> JpegEncodeThread::sEncoder = NULL;
static inline MUINT32 _align16(MUINT32 x)
{
    const MUINT32 MCU_ALIGN_LENGTH = 16;
    return (((x) + (MCU_ALIGN_LENGTH - 1)) / MCU_ALIGN_LENGTH) * MCU_ALIGN_LENGTH;
}

bool JpegEncodeThread::WorkThread::threadLoop()
{
    sp<IIBuffer> buffer = NULL;

    while (!Thread::exitPending())
    {
        {
            Mutex::Autolock lock(mFullImgLock);
            if (mFullImgQueue.empty())
            {
                mFullImgCond.wait(mFullImgLock);
                continue;
            }
            buffer = mFullImgQueue.front();
            mFullImgQueue.pop();
        }

        sp<JpegEncodeThread> encoder = wpEncoder.promote();
        if (encoder == NULL)
        {
            MY_LOGE("JpegEncodeThread promote failed");
            return false;
        }
        encoder->encode(buffer);
        buffer = NULL;
    }

    return false;
}

sp<JpegEncodeThread> JpegEncodeThread::getInstance(const MSizeF& finalCrop, const char* filename)
{
    Mutex::Autolock lock(sSingletonLock);

    sp<JpegEncodeThread> encoder = sEncoder.promote();
    if (encoder == NULL)
    {
        encoder = new JpegEncodeThread(finalCrop, filename);
        sEncoder = encoder;
        encoder->init();
    }
    else
    {
        MY_LOGW("JpegEncodeThread is occupied by other user");
        return NULL;
    }

    return encoder;
}

JpegEncodeThread::JpegEncodeThread(const MSizeF& finalCrop, const char* filename)
{
    MY_LOGD("JpegEncodeThread constructor, crop=(%.2f, %.2f)", finalCrop.w, finalCrop.h);
    if (filename != NULL)
    {
        mFilePath.assign(filename);
    }
    else
    {
        char fileName[100];
        int64_t timeNow = Utils::getTimeInMs();
        if (mkdir("/data/vendor/camera_dump/jpegencodethread", S_IRWXU | S_IRWXG))
        {
            MY_LOGW("mkdir for jpegencodethread failed");
        }
        sprintf(fileName, "/data/vendor/camera_dump/jpegencodethread/%" PRId64 "", timeNow);
        if (mkdir(fileName, S_IRWXU | S_IRWXG))
        {
            MY_LOGW("mkdir for jpegencodethread file folder failed");
        }
        sprintf(fileName, "/data/vendor/camera_dump/jpegencodethread/%" PRId64 "/%" PRId64 ".jpg", timeNow, timeNow);
        mFilePath.assign(fileName);
    }

    mpJpegFp = fopen(mFilePath.c_str(), "ab");
    mFinalCrop = finalCrop;
    if (mpJpegFp == NULL)
    {
        MY_LOGE("fopen mpJpegFp failed");
    }
}

JpegEncodeThread::~JpegEncodeThread()
{
    MY_LOGD("JpegEncodeThread destructor");
    mpThread->requestExit();
    mpThread->mFullImgCond.signal();
    mpThread->join();

    if (mpJpegFp != NULL)
    {
        fclose(mpJpegFp);
        mpJpegFp = NULL;
    }
    IBufferPool::destroy(mpFullImgPool);
    mpAllocator->free(mpJpegBuf.get());
}

bool JpegEncodeThread::init()
{
    mpThread = new WorkThread(this);
    mpThread->run("FpipeJpgEnc");

    return true;
}

bool JpegEncodeThread::makeExifHeader(MUINT32 width, MUINT32 height, MUINT8* exifBuf, size_t* exifSize)
{
    StdExif camExif;
    ExifParams exifParam;
    exifParam.u4ImageWidth = width;
    exifParam.u4ImageHeight = height;
    camExif.init(exifParam,  false);
    camExif.make((MUINTPTR)exifBuf, *exifSize);
    camExif.uninit();
    return true;
}

bool JpegEncodeThread::prepareBuffers(MUINT32 width, MUINT32 height, EImageFormat format)
{
    if (mpAllocator == NULL)
    {
        mpAllocator = IImageBufferAllocator::getInstance();
        IImageBufferAllocator::ImgParam imgParam((width * height * 2) + DBG_EXIF_SIZE, 0);
        mpJpegBuf = mpAllocator->alloc("FpipeJpgEnc", imgParam);
        makeExifHeader(width, height, mExifBuf, &mExifSize);
    }
    if (mpFullImgPool == NULL)
    {
        mpFullImgPool = ImageBufferPool::create("FpipeJpgEnc", width, height, format, ImageBufferPool::USAGE_SW);
        mStrideSize.w = width;
        mStrideSize.h = height;
    }

    if (mpFullImgPool->peakAvailableSize() <= 1)
    {
        MY_LOGD("allocate %d buffers (%dx%d)", ALLOC_BUFFER_NUM, width, height);
        mpFullImgPool->allocate(ALLOC_BUFFER_NUM);
    }

    return true;
}

MBOOL JpegEncodeThread::compressJpeg(const sp<IIBuffer>& sourceBuffer, bool markFrame)
{
    if (sourceBuffer == NULL)
    {
        return false;
    }
    sp<IImageBuffer> srcBuffer = sourceBuffer->getImageBuffer();
    MSize yPlaneSize = srcBuffer->getImgSize();
    EImageFormat format = (EImageFormat)srcBuffer->getImgFormat();

    MUINT32 pbpp = srcBuffer->getPlaneBitsPerPixel(0);
    MUINT32 ibpp = srcBuffer->getImgBitsPerPixel();
    MUINT32 stride = srcBuffer->getBufStridesInBytes(0);
    pbpp = pbpp ? pbpp : 8;
    MSize strideSize;
    strideSize.w = stride * 8 / pbpp;
    strideSize.w = strideSize.w ? strideSize.w : 1;
    ibpp = ibpp ? ibpp : 8;
    strideSize.h = srcBuffer->getBufSizeInBytes(0) / strideSize.w;
    if( srcBuffer->getPlaneCount() == 1 )
    {
      strideSize.h = strideSize.h * 8 / ibpp;
    }

    prepareBuffers(strideSize.w, strideSize.h, format);

    sp<IIBuffer> dstBuffer = mpFullImgPool->requestIIBuffer();
    if (dstBuffer == NULL)
    {
        MY_LOGE("request dstBuffer failed");
        return false;
    }
    dstBuffer->getImageBuffer()->setExtParam(yPlaneSize);
    dstBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);

    TRACE_FUNC("compressJpeg yPlaneSize=%dx%d, stride=%d, pbpp=%d, ibpp=%d, size=%d",
        yPlaneSize.w, yPlaneSize.h,
        srcBuffer->getBufStridesInBytes(0),
        srcBuffer->getPlaneBitsPerPixel(0),
        srcBuffer->getImgBitsPerPixel(),
        srcBuffer->getBufSizeInBytes(0));

    for (size_t plane = 0 ; plane < srcBuffer->getPlaneCount() ; plane++)
    {
        if (format == eImgFmt_YV12)
        {
            if (markFrame)
            {
                memset((void*)srcBuffer->getBufVA(plane), 0,
                    (plane == 0) ? (strideSize.w * strideSize.h) : (strideSize.w * strideSize.h)/4);
            }
            memcpy((void*)dstBuffer->getImageBuffer()->getBufVA(plane),
                   (void*)srcBuffer->getBufVA(plane),
                   (plane == 0) ? (strideSize.w * strideSize.h) : (strideSize.w * strideSize.h)/4);
        }
        else if (format == eImgFmt_YUY2)
        {
            if (markFrame)
            {
                memset((void*)srcBuffer->getBufVA(plane), 0, strideSize.w * strideSize.h * 2);
            }
            memcpy((void*)dstBuffer->getImageBuffer()->getBufVA(plane),
                   (void*)srcBuffer->getBufVA(plane),
                   strideSize.w * strideSize.h * 2);
        }
        else if (format == eImgFmt_NV21)
        {
            if (markFrame)
            {
                memset((void*)srcBuffer->getBufVA(plane), 0,
                    (plane == 0) ? (strideSize.w * strideSize.h) : (strideSize.w * strideSize.h)/2);
            }
            memcpy((void*)dstBuffer->getImageBuffer()->getBufVA(plane),
                   (void*)srcBuffer->getBufVA(plane),
                   (plane == 0) ? (strideSize.w * strideSize.h) : (strideSize.w * strideSize.h)/2);
        }
    }

    Mutex::Autolock lock(mpThread->mFullImgLock);
    mpThread->mFullImgQueue.push(dstBuffer);
    mpThread->mFullImgCond.signal();

    return true;
}

bool JpegEncodeThread::encode(const sp<IIBuffer>& srcBuf)
{
    if (srcBuf == NULL || mpJpegFp == NULL || mpJpegBuf == NULL)
    {
        MY_LOGE("NULL value, %p %p %p", srcBuf.get(), mpJpegFp, mpJpegBuf.get());
        return false;
    }

    int64_t t1 = Utils::getTimeInMs();

    MUINT32 jpegSize = 0;
    sp<IImageBuffer> srcBuffer = srcBuf->getImageBuffer();
    MSize imgSize = srcBuffer->getImgSize();
    MSize encodeSize = mStrideSize;
    MINT32 format = srcBuffer->getImgFormat();

    if(!mJpgHal.LevelLock(JpgEncHal::JPEG_ENC_LOCK_SW_ONLY))
    {
        MY_LOGE("can't lock jpeg resource");
        return false;
    }

    if (format == eImgFmt_YV12)
    {
        mJpgHal.setEncSize(encodeSize.w, encodeSize.h, JpgEncHal::kENC_YV12_Format);
        mJpgHal.setSrcAddr(
            (void*)srcBuffer->getBufVA(0),
            (void*)srcBuffer->getBufVA(2),
            (void*)srcBuffer->getBufVA(1));
        mJpgHal.setSrcBufSize(
            _align16(encodeSize.w),
            encodeSize.w * encodeSize.h,
            (encodeSize.w * encodeSize.h)/4,
            (encodeSize.w * encodeSize.h)/4,
            _align16(encodeSize.w)/2);
    }
    else if (format == eImgFmt_YUY2)
    {
        mJpgHal.setEncSize(encodeSize.w, encodeSize.h, JpgEncHal::kENC_YUY2_Format);
        mJpgHal.setSrcAddr((void*)srcBuffer->getBufVA(0), NULL);
        mJpgHal.setSrcBufSize(mJpgHal.getSrcBufMinStride() ,encodeSize.w * encodeSize.h * 2, 0);
    }
    else if (format == eImgFmt_NV21)
    {
        mJpgHal.setEncSize(encodeSize.w, encodeSize.h, JpgEncHal::kENC_NV21_Format);
        mJpgHal.setSrcAddr(
            (void*)srcBuffer->getBufVA(0),
            (void*)srcBuffer->getBufVA(1));
        mJpgHal.setSrcBufSize(
            _align16(encodeSize.w),
            encodeSize.w * encodeSize.h,
            (encodeSize.w * encodeSize.h)/2);
    }
    else
    {
        MY_LOGE("format(0x%x) not supported", format);
        mJpgHal.unlock();
        return false;
    }

    mpJpegBuf->syncCache(eCACHECTRL_INVALID);
    if (!mpJpegBuf->lockBuf("FpipeJpgEnc", (eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK)))
    {
        MY_LOGE("jpegBuf operation failed!");
        mJpgHal.unlock();
        return false;
    }
    mJpgHal.setQuality(50); //1~100
    mJpgHal.setDstAddr((void*)mpJpegBuf->getBufVA(0));
    mJpgHal.setDstSize(mpJpegBuf->getBufSizeInBytes(0));
    mJpgHal.enableSOI(0);
    if (format == eImgFmt_YV12 || format == eImgFmt_NV21)
    {
        mJpgHal.setIonMode(0);
    }
    else
    {
        mJpgHal.setIonMode(1);
        mJpgHal.setSrcFD(srcBuffer->getFD(0), -1);
        mJpgHal.setDstFD(mpJpegBuf->getFD(0));
    }

    if (!mJpgHal.start(&jpegSize))
    {
        MY_LOGE("Jpeg encode failed!");
        return false;
    }

    if (!mCropInfoSet)
    {
        fwrite(&mFinalCrop.w, sizeof(mFinalCrop.w), 1, mpJpegFp);
        fwrite(&mFinalCrop.h, sizeof(mFinalCrop.h), 1, mpJpegFp);
        fwrite(&imgSize.w, sizeof(imgSize.w), 1, mpJpegFp);
        fwrite(&imgSize.h, sizeof(imgSize.h), 1, mpJpegFp);
        fwrite(&encodeSize.w, sizeof(encodeSize.w), 1, mpJpegFp);
        fwrite(&encodeSize.h, sizeof(encodeSize.h), 1, mpJpegFp);
        mCropInfoSet = true;
    }

    MUINT32 writeSize = mExifSize + jpegSize;
    fwrite(&writeSize, sizeof(MUINT32), 1, mpJpegFp);
    fwrite(mExifBuf, mExifSize, 1, mpJpegFp);
    fwrite((void*)mpJpegBuf->getBufVA(0), jpegSize, 1, mpJpegFp);

    mpJpegBuf->unlockBuf("FpipeJpgEnc");
    mJpgHal.unlock();
    mFrameNum++;

    int64_t t2 = Utils::getTimeInMs();
    MY_LOGD("encode done[t %" PRId64 "], imageSize=(%d, %d), crop=(%.2f, %.2f), encodeSize=(%d, %d) %d frames written",
        t2 - t1, imgSize.w, imgSize.h, mFinalCrop.w, mFinalCrop.h, encodeSize.w, encodeSize.h, mFrameNum);
    return true;
}

}
}
}

