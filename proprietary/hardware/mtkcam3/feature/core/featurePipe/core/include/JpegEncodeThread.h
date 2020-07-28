#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_JPEG_ENCODER_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_JPEG_ENCODER_H_

#include <utils/RefBase.h>
#include <utils/threads.h>
#include "ImageBufferPool.h"
#include <mtkcam3/feature/featurePipe/SFPIO.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include "enc/jpeg_hal.h"
#include <queue>

class JpgEncHal;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class JpegEncodeThread : public android::RefBase
{
public:
    static android::sp<JpegEncodeThread> getInstance(const MSizeF& finalCrop, const char* filename = NULL);

    MBOOL compressJpeg(const android::sp<IIBuffer>& sourceBuffer, bool markFrame = false);

private:
    class WorkThread : public android::Thread
    {
    friend class JpegEncodeThread;
    public:
        WorkThread(const android::sp<JpegEncodeThread>& outer)
            : wpEncoder(outer) {}
    private:
        bool threadLoop();
        std::queue<android::sp<IIBuffer> > mFullImgQueue;
        android::wp<JpegEncodeThread> wpEncoder;
        android::Condition mFullImgCond;
        android::Mutex mFullImgLock;
    };

    JpegEncodeThread(const MSizeF& finalCrop, const char* filename = NULL);

    virtual ~JpegEncodeThread();

    bool init();

    bool prepareBuffers(MUINT32 width, MUINT32 height, EImageFormat format);

    bool makeExifHeader(MUINT32 width, MUINT32 height, MUINT8* exifBuf, size_t* exifSize);

    bool encode(const android::sp<IIBuffer>& srcBuf);

    static android::Mutex sSingletonLock;
    static android::wp<JpegEncodeThread> sEncoder;
    std::string mFilePath;
    android::sp<WorkThread> mpThread = NULL;
    android::sp<IBufferPool> mpFullImgPool = NULL;
    android::sp<IImageBuffer> mpJpegBuf = NULL;
    IImageBufferAllocator* mpAllocator = NULL;
    JpgEncHal mJpgHal;
    MUINT8 mExifBuf[DBG_EXIF_SIZE] = {0};
    FILE*  mpJpegFp = NULL;
    MSizeF mFinalCrop;
    MSize  mStrideSize;
    size_t mExifSize = 0;
    bool mCropInfoSet = false;
    MUINT32 mFrameNum = 0;
};
}
}
}

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_JPEG_ENCODER_H_

