#define LOG_TAG "ImageBufferUtils"

#include <mtkcam3/feature/hdr/1.0/utils/ImageBufferUtils.h>

#include <utils/StrongPointer.h>

#include <mtkcam/Log.h>

#include <mtkcam/utils/Format.h>
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

using namespace NSCam;
using namespace NSCam::Utils::Format;

// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(ImageBufferUtils);

MBOOL ImageBufferUtils::allocBuffer(
        IImageBuffer** ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt)
{
    MBOOL ret = MTRUE;

    IImageBuffer* pBuf = NULL;

    // to avoid non-continuous multi-plane memory,
    // allocate ION memory and map it to ImageBuffer
    MUINT32 plane = queryPlaneCount(fmt);

    MUINT32 mSize = 0;
    for (MUINT32 i=0; i<plane; i++) {
        mSize +=
            ((queryPlaneWidthInPixels(fmt,i, w) *
              queryPlaneBitsPerPixel(fmt,i) + 7) / 8) *
              queryPlaneHeightInPixels(fmt, i, h);
    }
    MY_LOGD("allocBuffer mSize(%d)", mSize);

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 strideInBytes[3] = {0};
    for (MUINT32 i = 0; i < plane; i++) {
        strideInBytes[i] =
            (queryPlaneWidthInPixels(fmt,i, w) *
             queryPlaneBitsPerPixel(fmt, i) + 7) / 8;
        MY_LOGD("allocBuffer strideInBytes[%d](%d)",i, strideInBytes[i]);
    }
    #if 0
    IImageBufferAllocator::ImgParam imgParam(fmt
                                            , MSize(w,h)
                                            , strideInBytes
                                            , bufBoundaryInBytes
                                            , plane
                                            );
    #else
    IIonImageBufferHeap::AllocImgParam_t imgParam(
        fmt, MSize(w,h),
        strideInBytes, bufBoundaryInBytes,
        plane
    );
    #endif

    sp<IIonImageBufferHeap> pHeap = IIonImageBufferHeap::create(LOG_TAG
                                                        , imgParam
                                                        ,IIonImageBufferHeap::AllocExtraParam()
                                                        ,MFALSE
                                                        );
    if(pHeap == 0) {
        MY_LOGE("pHeap is NULL");
        return MFALSE;
    }

    //
    pBuf = pHeap->createImageBuffer();
    if(pBuf == NULL) {
        MY_LOGE("pBuf is NULL");
        return MFALSE;
    }
    //pBuf->incStrong(pBuf);
    if (!pBuf ||
        !pBuf->lockBuf(LOG_TAG,
            eBUFFER_USAGE_HW_CAMERA_READWRITE |
            eBUFFER_USAGE_SW_READ_OFTEN |
            eBUFFER_USAGE_SW_WRITE_OFTEN ))
    {
        MY_LOGE("Null allocated or lock Buffer failed\n");
        ret = MFALSE;

    }
    else
    {
        // flush
        //pBuf->syncCache(eCACHECTRL_INVALID);  //hw->cpu
        MY_LOGD("allocBuffer addr(%p) width(%d) height(%d) format(0x%x)", pBuf, w, h, fmt);

        for (MUINT32 i=0; i<plane; i++) {
            MY_LOGD("allocBuffer getBufVA(%d)(%zu)", i,pBuf->getBufVA(i));
        }
        *ppBuf = pBuf;
    }
lbExit:
    return ret;
}

void ImageBufferUtils::deallocBuffer(IImageBuffer* pBuf)
{
    if (!pBuf)
    {
        MY_LOGD("free a null buffer");
        return;
    }

    // unlock image buffer
    pBuf->unlockBuf(LOG_TAG);
    pBuf = NULL;
}
